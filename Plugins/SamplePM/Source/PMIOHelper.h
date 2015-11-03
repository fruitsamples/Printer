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
	PMIOHelper.h
	
	Contains:	Routines for buffered async i/o.  These routines are necessary for maximal 
                        performance over usb, but might not be needed for other i/o types where the i/o
                        subsystem performs buffered async writes.

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.
 
	Mods:

 ***************************************************************************/

#ifndef __PMIOHelper__
#define __PMIOHelper__

typedef struct  PMIOHelper *PMIOHelperRef;

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
 

PMIOHelperRef PMIOCreateHelper(int firstBufferSize, int nextBufferSize, void *job, PMIOWriteProcPtr writeProc);

/*
 * PMIOWrite -- Writes data of size bytes to the io channel, may not actually cause i/o immediately.
 */
OSStatus PMIOWrite(PMIOHelperRef h, char *data, int size);

/*
 * PMIOFlush -- Schedule helper buffer to written. Does not block.
 */
 
OSStatus PMIOFlush(PMIOHelperRef h);

/*
 * PMIOFlushSync -- Schedule helper buffer to written. Blocks until writeProc returns.
 */
OSStatus PMIOFlushWait(PMIOHelperRef h);

/*
 * PMIODelete -- Flushes all data, waits for completion, tears down the helper, and frees its memory.
 */
OSStatus PMIODelete(PMIOHelperRef h);

#endif

