/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple’s copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following text
 and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple. Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
 NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE.  */



/*****************************************************************************
	PMIOHelper.c
	
	Contains:	Routines for buffered async i/o.  These routines are necessary for maximal 
                        performance over usb, but might not be needed for other i/o types where the i/o
                        subsystem performs buffered async writes.

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.
 
	Mods:

 ***************************************************************************/

#include <stdio.h>		// added for printf debugging
#include <pthread.h>
#include <sys/file.h>

#include <ApplicationServices/ApplicationServices.h>
#include <PrintCore/PMPrinterModule.h>

#include "PMIOHelper.h"

//#define Dprintf(x) printf x
#define Dprintf(x)

/*
 * ioThreadData is a private data structure to control the async io thread.  This thread is used to schedule
 * the asynchronous writes.  It is based on standard "producer/consumer" pthreads logic. (Search for pthreads 
 * examples on the web...
 */
typedef struct ioThreadData {
    pthread_mutex_t ioLock;		// lock to protect this data structure
    pthread_cond_t dataReadyCond;	// condition waited on by iothread, signaled by writer
    pthread_cond_t writerIdleCond;	// condition waited on by writer, signaled by iothread
    int dataReady, writerReady, die, hadError;  // state flags to signal conditions
    OSStatus err;			// error latched from last write
    char *buffer;			// buffer to write
    int count;				// count to write
    void *job;				// job id for call to writeProc
    PMIOWriteProcPtr writeProc;		// PMIO write proc
    pthread_t thread;			// async thread running ioThread() loop
    int debug_fd;			// posix fd useful for capturing data to file
} ioThreadData;


typedef struct PMIOHelper {
    char *bufferBase;		// buffer base pointer
    char *bufferPtr;		// pointer to where next data should go
    int bufferLength;		// capacity of buffer
    int initialBuffer;		// first buffer size
    int nextBuffer;		// next buffer size
    void *job;			// job reference for calling write proc
    PMIOWriteProcPtr writeProc;	// write proc for calling io module
   ioThreadData *io;		// async io state
} PMIOHelper;

static void *ioThread(void *data);

static ioThreadData *ioInit(void *job, PMIOWriteProcPtr writeProc)
{
    ioThreadData *io = (ioThreadData *) malloc(sizeof(ioThreadData));
    bzero(io, sizeof(ioThreadData));
    pthread_mutex_init(&io->ioLock, NULL);
    pthread_cond_init(&io->dataReadyCond, NULL);
    pthread_cond_init(&io->writerIdleCond, NULL);
	pthread_create (&io->thread, NULL, ioThread, (void *)io);
    io->job = job;
    io->writeProc = writeProc;
    io->err = noErr;
    io->writerReady = 1;
    io->debug_fd = -1;
#define SAVE_PCL_TO_FILE 1
#if SAVE_PCL_TO_FILE
    io->debug_fd = open("/tmp/hpdebug.pcl", O_CREAT | O_WRONLY | O_TRUNC, 0644);
#endif
    return io;
}
static void *ioThread(void *data)
{
    ioThreadData *io = (ioThreadData *) data;
    char *buffer;
    OSStatus err;
    UInt32 count;
    Dprintf(("ioThread started\n"));
    while(1) {
        pthread_mutex_lock(&io->ioLock);
        // wait for work to be posted
        while(!io->dataReady  && !io->die) {
            pthread_cond_wait(&io->dataReadyCond, &io->ioLock);
        }
        // postpone processing die until next time if data is ready.
        if(io->die && !io->dataReady ) {
            // producer thread wants us to die, just break out of loop
            if (io->debug_fd != -1) close(io->debug_fd);
            break;
        }
        // latch buffer and count for i/o call.
        buffer = io->buffer;
        count = io->count;
        io->dataReady = 0;
        io->buffer = 0;
        io->count = 0;
        io->writerReady = 1; // now that we've latched the data, let the writer proceed
        pthread_cond_signal(&io->writerIdleCond);  // signal that we're ready for new data
        pthread_mutex_unlock(&io->ioLock);	   // drop the lock while we write out the data
        if (!count && !buffer) continue;   	   // a null post will just synchronize us.
        Dprintf(("iothread: writing %d bytes\n", (int)count));
        if (io->debug_fd != -1) write(io->debug_fd, buffer, count);
        err = (*io->writeProc)( io->job, buffer, &count, false);
    
        free(buffer);	// contract with posting thread is that we own the buffer once we get it, so free it here.
        if (err != noErr) {
            fprintf(stderr,"Had error %d!\n", (int)err);
            pthread_mutex_lock(&io->ioLock);
            io->hadError = 1;
            io->err = err;   // latch error for examination by posting thread
            pthread_mutex_unlock(&io->ioLock);
        }
        
    }
    return 0;
}
        
static OSStatus postWrite(ioThreadData *io, char *buffer, int count)
{
    pthread_mutex_lock(&io->ioLock);
    if (io->hadError) {
        pthread_mutex_unlock(&io->ioLock);
        return io->err;
    }
    while(!io->writerReady) {
            pthread_cond_wait(&io->writerIdleCond, &io->ioLock);
        }
    io->buffer = buffer;
    io->count = count;
    io->dataReady = 1;
    io->writerReady = 0;
    pthread_cond_signal(&io->dataReadyCond);
    pthread_mutex_unlock(&io->ioLock);
    return noErr;
}    
        
static void postKill(ioThreadData *io)
{
    void *result;
    pthread_mutex_lock(&io->ioLock);
    io->die = 1;
    pthread_mutex_unlock(&io->ioLock);
    pthread_cond_signal(&io->dataReadyCond);
    pthread_join(io->thread, &result);
    free(io);
}    
        



/*
 * PMIOCreateHelper
 * 
 * Creates an IO helper.  An IO helper helps optimize the communication between the 
 * PrinterModule and the IOModule, providing buffering and async writing services.  The
 * PMIOCreateHelper will act as a "front" for all of your writes to the i/o module, collect
 * them into reasonably sized buffers, and schedule them for i/o on a special thread.  Used
 * correctly this allows the main printing and converting threads to not wait on i/o.
 *
 * Parameters:  firstBufferSize, size of first buffer (usually small, like 4K)
 * 		nextBufferSize, size of subsequent buffers (usually larger, like 32K)
 * 		job, jobID to call IO write proc
 *		writeProc, the PMIOWriteProcPtr to send data to
 */

PMIOHelperRef PMIOCreateHelper(int firstBufferSize, int nextBufferSize, void *job, PMIOWriteProcPtr writeProc)

{
    PMIOHelper *h = malloc(sizeof(PMIOHelper));
    bzero(h, sizeof (PMIOHelper));
    h->initialBuffer = firstBufferSize;
    h->nextBuffer = nextBufferSize;
    h->job = job;
    h->writeProc = writeProc;
    return h;
}
/*
 * PMIOWrite -- Writes data of size bytes to the io channel, may not actually cause i/o immediately.
 */
OSStatus PMIOWrite(PMIOHelper *h, char *data, int size)
{

    OSStatus	err = noErr;					
    if (h->bufferLength == 0) {
        h->bufferLength = h->initialBuffer;  
        Dprintf(("Buffer sizes is (first = %d, subsequent = %d)\n", 
                h->bufferLength, h->nextBuffer));
        h->bufferBase = h->bufferPtr = (char *) malloc(h->bufferLength);
    }
    if (h->bufferPtr + size > h->bufferBase + h->bufferLength) {
        int left = h->bufferBase + h->bufferLength - h->bufferPtr;
        bcopy(data, h->bufferPtr, left);
        h->bufferPtr += left;
        data += left;
        size -= left;
        err = PMIOFlush(h);
    }
    /* too big for the buffer, write it directly */
    while (size > h->bufferLength) {
        bcopy(data, h->bufferPtr, h->bufferLength);
        h->bufferPtr += h->bufferLength;
        data += h->bufferLength;
        size -= h->bufferLength;
        err = PMIOFlush(h);
        if (err != noErr) {
        /* unwind here, should actually return bytes written */
            return err;
        }
    }
    bcopy(data, h->bufferPtr, size);
    h->bufferPtr += size;
    return err;
}

/*
 * PMIOFlush -- Schedule helper buffer to written. Does not block.
 */
 
OSStatus PMIOFlush(PMIOHelper *h)
{
    OSStatus	err = noErr;					
    UInt32 writeSize = h->bufferPtr - h->bufferBase;
    if(writeSize) {
#define ASYNC_IO 1
#if ASYNC_IO
        if(!h->io)
            h->io = ioInit(h->job, h->writeProc);
        err = postWrite(h->io, h->bufferBase, writeSize);
        if (h->bufferLength == h->initialBuffer)
            h->bufferLength = h->nextBuffer;
        h->bufferBase = (char *) malloc(h->bufferLength);
        h->bufferPtr = h->bufferBase;
#else
		err = (*h->writeProc)( h->job, h->bufferBase, &writeSize, false);
        if (h->bufferLength == h->initialBuffer) {
            h->bufferLength = h->nextBuffer;
            free(h->buffer);
            h->bufferBase = (char *) malloc(h->bufferLength);
        }
        h->bufferPtr = h->bufferBase;
#endif
    
    }
    return err;
}

/*
 * PMIOFlushSync -- Schedule helper buffer to written. Blocks until writeProc returns.
 */
OSStatus PMIOFlushWait(PMIOHelper *h)
{
    OSStatus	err = noErr;					
    err = PMIOFlush(h);
    if (h->io) err = postWrite(h->io, 0, 0); // post a null write to sync
    return err;
    
}
/*
 * PMIODelete -- Flushes all data, waits for completion, tears down the helper, and frees its memory.
 */
OSStatus PMIODelete(PMIOHelper *h)
{
    
    OSStatus	err = noErr;					
    err = PMIOFlush(h);
    if(h->io) postKill(h->io);  // post kill will wait for io thread to synchronize
    if (h->bufferBase) free(h->bufferBase);
    free(h);
    return err;
}

