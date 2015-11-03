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
	File:		USBUtil.h
	Contains: 	Utility Functions to access USB with IOKit.

	Written by:	Olav Andrade
	
	Copyright 1999-2002 by Apple Computer, Inc., all rights reserved.


	To Do:
		
*******************************************************************************/
#ifndef	__USBUtil__
#define	__USBUtil__

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Pragmas
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <unistd.h>
#include <pthread.h>	// Used for writeMutex

#include <CoreFoundation/CFData.h>
#include <ApplicationServices/ApplicationServices.h>

#include "PrinterClass.h"
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Constants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define		kIOModuleName			"\pUSB IOM"	 // should be name of code frag
#define		kIOPrinterBrowserModuleName	"\pApple USB PB"
#define		kIOMajorVersion			1
#define		kIOMinorVersion			0
#define		kIOCreator				'aapl'


#define		kMaxDataSize			4096
#define		kGeneralError			-1
#define		kEOF					-2
#define		kOutOfMemory			-3
#define		kNoPrinterName			-4

/*
**	Attributes
*/
#define		kUSBIOMGetDeviceID			CFSTR( "USBIOMGetDeviceID" )
#define		kUSBIOMGetCentronicsStatus	CFSTR( "USBIOMGetCentronicsStatus" )
#define		kUSBIOMSetSoftReset			CFSTR( "USBIOMSoftReset" )
#define		kUSBIOMVendorRequest		CFSTR( "USBIOMVendorRequest" )
#define		kUSBIOMResetOnAbort			CFSTR( "USBIOMResetOnAbort" )
#define		kUSBIOMSetAltInterface		CFSTR( "USBIOMSetAltInterface"  )
#define		kUSBIOMSetControlTimeout	CFSTR( "USBIOMSetControlTimeout"  )


#define		kDefaultControlTimeoutValue	60000		/*msec = 60s before we timeout*/

/*
    Debugging output to Console
    DEBUG undefined or
    DEBUG=0		production code: suppress console output

    DEBUG=1		report errors (non-zero results)
    DEBUG=2		report all results, generate dumps
*/
#if DEBUG==2
#define DEBUG_ERR(c, x)						fprintf(stderr, x, c)
#define DEBUG_DUMP( text, buf, len )		dump( text, buf, len )
#define DEBUG_CFString( text, a )			printcfs( text, a )
#define DEBUG_CFCompareString( text, a, b )	cmpcfs( text, a, b )
#elif DEBUG==1
#define DEBUG_ERR(c, x)						if (c) fprintf(stderr, x, c)
#define DEBUG_DUMP( text, buf, len )
#define DEBUG_CFString( text, a )
#define DEBUG_CFCompareString( text, a, b )
#else
#define DEBUG_ERR(c, x)
#define DEBUG_DUMP( text, buf, len )
#define DEBUG_CFString( text, a )
#define DEBUG_CFCompareString( text, a, b )
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Type Definitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct
{
    //
    //	Tagged/Tranparent Binary Communications Protocol
    //	TBCP read
    Boolean					tbcpQuoteReads;			// enable tbcp on reads
    Boolean					escapeNextRead;			// last char of last read buffer was escape
    UInt8					*tbcpReadData;			// read buffer
    UInt32					readLength;				// read buffer length (all used)
    int						match_endoffset,		// partial match of end TBCP sequence
                            match_startoffset;		// partial match of start TBCP sequence
    // TBCP write
    UInt8					*tbcpWriteData;			// write buffer
    UInt32					tbcpBufferLength,		// write buffer allocated length
                            tbcpBufferRemaining;	// write buffer not used

    Boolean					sendStatusNextWrite;

} PostScriptData;

typedef struct 
{
    CFPlugInRef				plugin;			// valid until plugin is release 
    USBPrinterClassContext	**classdriver;	// usb printer class in user space
    CFStringRef				bundle;			// class driver URI
    UInt32					location;		// unique location in USB topology
    USBPrinterAddress		address;		// topology independent bus address
    CFURLRef				reference;		// internal use
} USBPrinterInfo;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if DEBUG==2
#if defined(__cplusplus)
extern "C" {
#endif
void dump( char *text, void *s, int len );
void printcfs( char *text, CFStringRef s );
void cmpcfs( char *text, CFStringRef a, CFStringRef b );
#if defined(__cplusplus)
}
#endif
#endif

/*
**	IOKit to CF functions
*/
#if defined(__cplusplus)
extern "C" {
#endif
USBPrinterInfo		*UsbCopyPrinter( USBPrinterInfo *aPrinter );
CFMutableArrayRef	UsbGetAllPrinters( void );
void				UsbReleasePrinter( USBPrinterInfo *aPrinter );
void				UsbReleaseAllPrinters( CFMutableArrayRef printers );
kern_return_t		UsbRegistryOpen( USBPrinterAddress *usbAddress, USBPrinterInfo **result );
kern_return_t		UsbUnloadClassDriver( USBPrinterInfo *printer );
kern_return_t		UsbLoadClassDriver( USBPrinterInfo *printer, CFUUIDRef interfaceID, CFStringRef classDriverBundle );

int			UsbSamePrinter( const USBPrinterAddress *lastTime, const USBPrinterAddress *thisTime ); 

int			UsbSupportedPrinter(USBPrinterAddress *address, CFStringRef moduleDeviceID );
void		UsbAddressDispose( USBPrinterAddress *address );
OSStatus	UsbAddressRecover( CFDataRef pmaddr, USBPrinterAddress *address );

CFDataRef 	UsbCreateExternalAddress( const USBPrinterAddress *address, Boolean fUseGeneric);		//bg

OSStatus	UsbGetPrinterAddress( USBPrinterInfo *thePrinter, USBPrinterAddress *address, UInt16 timeout );

Boolean 	StripEOJs( Ptr buffer, UInt32* nBytesP);		// jl131 bg862
#if defined(__cplusplus)
}
#endif

#endif	// __USBUtil__
// eof

