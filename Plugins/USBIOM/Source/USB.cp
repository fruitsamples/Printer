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

/*******************************************************************************
	File:		USB.cp

	Copyright 1999-2002, Apple Computer, Inc. All rights reserved.

	Description:
             IO module for USB (USB 1.0)
 	Version:	Technology:	Tioga SDK
 				Release:	2.0
	

          USB printing 1.1 spec allows an interrupt pipe for async status
*******************************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreServices/CoreServices.h>

#include <PrintCore/PMIOModule.h>
#include <PrintCore/PMTicket.h>

#include "USBUtil.h"
#include "USBIOInternals.h"

#define kBrowserPath				"../SDK_USBPBM.plugin"
#define PMCFEqual					CFEqual
/*
**     The following constants are owned by Apple.
*/
#define kAttrAvailable				kPMNoError
#define kIOHasBackChannel			CFSTR("IOHasBackChannel") 
//
//	Since we wait forever to get access to our printers
//		we'll log some information into the console window.
//		Intervals below are specified in seconds.
//
#define PRINTER_POLLING_INTERVAL	5
#define INITIAL_LOG_INTERVAL		(12*PRINTER_POLLING_INTERVAL)
#define SUBSEQUENT_LOG_INTERVAL		(5*INITIAL_LOG_INTERVAL)

typedef struct
{
    USBPrinterAddress		usbAddress;
    USBPrinterInfo			*thePrinter;		// printer abstraction that meets USB spec
    UInt16					timeout;			// control transaction timeouts

	// Mutex data:
    pthread_mutex_t			writeMutex;
    pthread_t				owner;
    
    OSStatus				err;					// while waiting to open device, report status

    Boolean					resetOnAbort;			// true if we need to send SoftReset after aborting a job
} USBIOContext;

//
//	non-localized strings returned by the IOMStatus routine
//	the host module must localize these
//
static const char *kStrRasterBusy = "The printer is busy.";
static const char *kStrRasterOffline = "The printer is not responding.";


/*-----------------------------------------------------------------------------*

	IOMGetConnectionInfo
	
*-----------------------------------------------------------------------------*/

OSStatus	IOMGetConnectionInfo( CFStringRef *connectionType, CFStringRef *pbmPath)
{
	OSStatus err = noErr;
    
    if (connectionType != NULL)
        *connectionType = kPMUSBConnection;
    if (pbmPath != NULL)
        *pbmPath = CFSTR(kBrowserPath);
        
	return err;
}

/*-----------------------------------------------------------------------------*

	IOMInitialize
	
	Desc:	Creates a context to store our variables

	In:		ioModuleContext	- ptr to a variable to store the context
			pmPrinterAddress - the printer address of selected printer

	Out:		ioModuleContext - will contain our global context

*-----------------------------------------------------------------------------*/
OSStatus	IOMInitialize( const CFDataRef printerAddress, IOMContext *ioModuleContextPtr)
{
    OSErr			err	= noErr;
    USBIOContext	*myContext = NULL;

    // check for bad parameters
    if ((ioModuleContextPtr == NULL) || (printerAddress == NULL))
        err = kPMInvalidIOMContext;

    // allocate the memory for the context
    if (err == noErr)
    {
        myContext = (USBIOContext*) calloc(1, sizeof(USBIOContext));

        if(myContext == NULL)
            err = memFullErr;
        else
        {
            myContext->timeout =  kDefaultControlTimeoutValue;
            pthread_mutex_init( &myContext->writeMutex, NULL );

            // get the printer address so we can open it later
            UsbAddressRecover( printerAddress, &myContext->usbAddress );

            *ioModuleContextPtr = (IOMContext ) myContext;
        }
    }
	if (err != noErr)
	{
		if (myContext != NULL)
			free(myContext);
	}
    return err;
}

/*-----------------------------------------------------------------------------*

	IOMOpen
	
	Desc:	Opens the USB port for input and output

	In:
                ioModuleContext	- ptr to our context
                jobTicket 	- ref to the jobticket
                bufferSize	- ptr to return our optimal buffer size for input/output

	Out:	bufferSize - size of the desired buffer used for input/output
	
*-----------------------------------------------------------------------------*/
OSStatus	IOMOpen(IOMContext ioModuleContext, PMTicketRef jobTicket,
                   UInt32* bufferSize)
{
    int				countdown;
    OSStatus		err = noErr;
    USBIOContext	*myContext = (USBIOContext *) ioModuleContext;

    *bufferSize = kMaxDataSize;		// our desired buffer size

    //
    // try and find our printer
    //	keep trying until it's turned on
    //
    countdown = INITIAL_LOG_INTERVAL;	// initial log after 5*12*5 seconds = five minutes
    do
    {
        err = UsbRegistryOpen( &myContext->usbAddress, &myContext->thePrinter );
        myContext->err = err;

        if ( NULL == myContext->thePrinter )
        {
            myContext->err = kIOReturnNotResponding;
            sleep( PRINTER_POLLING_INTERVAL );
            countdown -= PRINTER_POLLING_INTERVAL;
            if ( !countdown )
            {
                // periodically, write to the log so someone knows we're waiting
                fprintf( stderr, "Printing: Waiting for USB printer to respond.\n" );
                countdown = SUBSEQUENT_LOG_INTERVAL;	// subsequent log entries, every 30 minutes
            }
        }
    } while ( NULL == myContext->thePrinter );
    myContext->err = noErr;	// IOMStatus can stop reporting kIOReturnNotResponding

    DEBUG_ERR( "IOMOpenDefault UsbRegistryOpen returned %lx\n", err );
    if( kIOReturnSuccess == err && NULL == myContext->thePrinter )
        err = kPMOpenFailed;

    return err;
}

/*-----------------------------------------------------------------------------*

	IOMRead
	
	Desc:	Attempts to read a series of bytes

	In:
		ioModuleContext - ptr to store our global context
                buffer - ptr to the buffer to receive the data
		size - ptr to the desired size of the data to read. Upon return 
			this will contain the acutal number of bytes read
                eoj - ???
	Out:	size - the number of bytes read
	
*-----------------------------------------------------------------------------*/
OSStatus	IOMRead(IOMContext ioModuleContext, Ptr buffer,
                   UInt32* size, Boolean *eoj)
{
    OSStatus		err = kPMReadFailed;	// assume failure

    if( ioModuleContext != NULL && size && *size > 0)
    {
        USBIOContext			*myContext = (USBIOContext *) ioModuleContext;
        USBPrinterClassContext	**printer = NULL != myContext->thePrinter? myContext->thePrinter->classdriver: NULL;

        if( NULL != printer )
        {
            err = (*printer)->ReadPipe( printer, (UInt8 *)buffer, size );
            DEBUG_ERR( "IOMRead %lx\n", err );
            DEBUG_DUMP( "\tRead data", buffer, *size );
        }
    }
    return err;
}

/*-----------------------------------------------------------------------------*

	IOMWrite
	
	Desc:	Attempts to write a series of bytes

	In:	buffer - ptr to the buffer to send
		size - ptr to the desired size of the data to write. Upon return
                 this will contain the actual number of bytes written
		ioModuleContext - ptr to store our global context
	Out:	size - the number of bytes written

	
*-----------------------------------------------------------------------------*/
OSStatus	IOMWrite( IOMContext ioModuleContext, Ptr buffer,
                     UInt32* size, Boolean eoj)
{
    OSStatus		err = kPMWriteFailed;
 
    if( ioModuleContext != NULL && size && *size > 0)
    {
        USBIOContext			*myContext = (USBIOContext *) ioModuleContext;
        USBPrinterClassContext	**printer = NULL != myContext->thePrinter? myContext->thePrinter->classdriver: NULL;

        if( NULL != printer )
        { 
            pthread_mutex_lock( &myContext->writeMutex );

            DEBUG_ERR( "writing data to pipe %ld\n", *size );
            err = (*printer)->WritePipe(printer, (UInt8 *)buffer, size, eoj );

            pthread_mutex_unlock( &myContext->writeMutex );
        }
   }
   return err;
}

/*-----------------------------------------------------------------------------*

	IOMStatus
	
	Desc:	Returns the status port

	In: 	status	- the status string to fill in
			ioModuleContext - ptr to store our global context
			
	Out:	status	- will contain the status string (pstring)
	
*-----------------------------------------------------------------------------*/
OSStatus	IOMStatus(IOMContext ioModuleContext, CFStringRef *status)
{
    USBIOContext	*myContext;
    OSStatus		err	= noErr;

	*status = NULL;

    if( ioModuleContext != NULL )
    {
        myContext = (USBIOContext *) ioModuleContext;
        DEBUG_ERR( "IOMStatus %ld\n", myContext->err );
        //
        //	raster module should localize this result
        //
        switch ( myContext->err )
        {
        case kIOReturnExclusiveAccess:
            *status = CFStringCreateWithCString( kCFAllocatorDefault, kStrRasterBusy, kCFStringEncodingUTF8 );
        case kIOReturnNotResponding:
            *status = CFStringCreateWithCString( kCFAllocatorDefault, kStrRasterOffline, kCFStringEncodingUTF8 );
            break;
        case noErr:
        default:
            break;
        }
    }
    return err;
}

/*-----------------------------------------------------------------------------*

	IOMClose
	
	Desc:	Close the port

	In:		abnormalAbort - terminate pending i/o
			ioModuleContext	- ptr to our global context

	Out:	none
	
*-----------------------------------------------------------------------------*/
OSStatus	IOMClose( IOMContext ioModuleContext, Boolean abort)
{
    OSStatus	err = noErr;

    DEBUG_ERR( "IOMClose %ld...", err );
    if( ioModuleContext != NULL )
    {
        USBIOContext			*myContext = (USBIOContext *) ioModuleContext;
        USBPrinterClassContext	**printer = NULL != myContext->thePrinter? myContext->thePrinter->classdriver: NULL;

        if( NULL != printer )
        {
            if ( abort )
            {
                (*printer)->Abort( printer );
                //
                //	SoftReset should eject last page so that next job starts on a new page
                //	alternatively PM should issue a FormFeed to eject the last page
                //
                if ( myContext->resetOnAbort )
                    (*printer)->SoftReset( printer, myContext->timeout );
            }

            pthread_mutex_lock( &myContext->writeMutex );
            (*printer)->Close( printer );
            pthread_mutex_unlock( &myContext->writeMutex );
        }
     }
     DEBUG_ERR ("...Close %ld\n", err );
     return err;
}

/*-----------------------------------------------------------------------------
 	IOMGetAttributes
 
	Desc:	Checks if the requested attribute is implemeneted by the 
	connection type or not. If the attribute is not implemented, 
	kAttrNotImplementedErr is returned. If the attribute is implemented but 
	not available, kAttrNotAvailable is returned. If the attribute is 
	implemented and available, kAttrAvailable is returned. In some cases 
	when returning kAttrAvailable, "result" is set with the result of the 
	requested attribute (i.e. job ID).
  
	In:		attribute	- 
			result		- 
			ioModuleContext	- our global context
	Out:	none

-----------------------------------------------------------------------------*/
OSStatus	IOMGetAttribute( IOMContext ioModuleContext,
                                CFStringRef attribute, CFTypeRef *result)
{
    USBIOContext		*myContext = (USBIOContext *) ioModuleContext;
    OSStatus			returnValue = kPMIOAttrNotAvailable;

	DEBUG_CFString("Tioga USBIOM: IOMGetAttribute ", attribute );
    if( ioModuleContext != NULL )
    {
        USBPrinterClassContext		**printer = NULL == myContext->thePrinter? NULL: myContext->thePrinter->classdriver;
        if(PMCFEqual(attribute, kIOHasBackChannel))
        {
            returnValue = kAttrAvailable;
            *result = ((*printer)->inpipe) ? kCFBooleanTrue : kCFBooleanFalse;
            DEBUG_ERR( "USBIOM GetAttr kIOHasBackChannel %d\n", *result? 1: 0 );

        }
		else if ( PMCFEqual(attribute, kPM8BitChannelAttr)  )
        {
            // we always enable TBCP over PostScript,
            // and USB is otherwise defined as an 8-bit clear channel
            DEBUG_ERR( "USBIOM GetAttr kPM8BitChannelAttr %d\n", noErr );
            returnValue = kAttrAvailable;
            *result = kCFBooleanTrue;
		}
        else if ( PMCFEqual(attribute, kPMTransparentByteRange) )
        { 
            // we always enable TBCP over PostScript
            DEBUG_ERR( "USBIOM GetAttr kPMTransparentByteRange %d\n", noErr );
            returnValue = kAttrAvailable;
            *result = kCFBooleanTrue;
        }
		else if ( PMCFEqual( attribute, kUSBIOMGetDeviceID ) && NULL != printer )
		{
 			returnValue = (*printer)->GetDeviceID( printer, (CFStringRef *) result, myContext->timeout  );
 		}
		else if (  PMCFEqual( attribute, kUSBIOMGetCentronicsStatus ) && NULL != printer  )
		{
			returnValue = (*printer)->GetCentronicsStatus( printer, (CentronicsStatusByte *) result, myContext->timeout  );
		}
        else if (  PMCFEqual( attribute, kUSBIOMVendorRequest ) && NULL != printer  )
		{
			returnValue = (*printer)->DeviceRequest( printer, (USBIODeviceRequest *) result, myContext->timeout );
		}
    }
    
    return returnValue;

}

/*-----------------------------------------------------------------------------

 	IOMSetAttributes
 
	Desc:	Checks if the requested attribute is implemeneted by the connection 
	type or not. If the attribute is not implemented, kAttrNotImplementedErr is 
	returned. If the attribute is implemented but not available, kAttrNotAvailable 
	is returned. If the attribute is implemented and available, kAttrAvailable 
	is returned. In some cases when returning kAttrAvailable, "data" may used 
	to set the the requested attribute (i.e. USB T status channel).
  
	In:		attribute	- 
			data		- 
			ioModuleContext	- our global context
	Out:	none

-----------------------------------------------------------------------------*/
OSStatus	IOMSetAttribute ( IOMContext ioModuleContext,
                                 CFStringRef attribute, CFTypeRef data)
{
    USBIOContext		*myContext = (USBIOContext *) ioModuleContext;
    OSStatus			returnValue = kPMIOAttrNotAvailable;
    
    if ( NULL != myContext)
    {
        USBPrinterClassContext		**printer = NULL == myContext->thePrinter? NULL: myContext->thePrinter->classdriver;
        if ( PMCFEqual( attribute, kUSBIOMSetSoftReset ) && NULL != printer )
        {
            returnValue = (*printer)->SoftReset( printer, myContext->timeout );
        } else if (PMCFEqual( attribute, kUSBIOMResetOnAbort )) {
            if (PMCFEqual(data, kCFBooleanTrue)) {
                myContext->resetOnAbort = true;
                returnValue = noErr;
    
            } else if (PMCFEqual(data, kCFBooleanFalse)) {
                myContext->resetOnAbort = false;
                returnValue = noErr;
            }
        } else if (PMCFEqual( attribute, kUSBIOMSetAltInterface ) && NULL != data  && NULL != printer ) {
            //
            //	changing the protocol may surrender the use of the printer to another process,
            //	but the Open will wait until we have control again.
            //
            SInt32	protocol;
            UInt32	location = myContext->thePrinter->location;
            returnValue = CFNumberGetValue( (CFNumberRef) data, kCFNumberSInt32Type, &protocol );
            if ( noErr == returnValue )
                returnValue = (*printer)->Close( printer );
            if ( noErr == returnValue )
                returnValue = (*printer)->Open( printer, location, (UInt8) protocol );
        } else if (CFEqual( attribute, kUSBIOMSetControlTimeout ) && NULL != data) {
            SInt32	timeout;
            returnValue = CFNumberGetValue( (CFNumberRef) data, kCFNumberSInt32Type, &timeout );
            myContext->timeout = noErr == returnValue? timeout: kDefaultControlTimeoutValue;
        }
    }
    return returnValue;
}

/*-----------------------------------------------------------------------------

 	IOMTerminate
 
	Desc:	Frees the global context we created

	In:	ioModuleContext	- address of variable containing our context
	Out:	none

-----------------------------------------------------------------------------*/
OSStatus	IOMTerminate(IOMContext *ioModuleContextPtr)
{
    USBIOContext *myContext;

    if( ioModuleContextPtr != NULL && *ioModuleContextPtr != NULL)
    {
        myContext = (USBIOContext *) *ioModuleContextPtr;

        // free our global context that we created
        UsbAddressDispose( &myContext->usbAddress );
        pthread_mutex_destroy( &myContext->writeMutex );
        free( myContext );
        *ioModuleContextPtr = NULL;
    }
    return noErr;
}
/* eof */
