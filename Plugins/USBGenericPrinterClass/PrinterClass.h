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

/*
 *  PrinterClass.h
 *  Plugins
 *
 *  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 */
#ifndef __USBPrinterClass__
#define __USBPrinterClass__ 1

#include <CoreFoundation/CFPlugInCOM.h>

#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <mach/mach.h>				// include mach after IOKit for definitions
#include <mach/mach_error.h>


#define		USB_INTERFACE_KIND 					CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID182)
#define     kUSBLanguageEnglish            		0x409

/*
**	Section 5.3 USB Printing Class spec
*/
#define		kUSBPrintingSubclass				1
#define		kUSBPrintingProtocolNoOpen			0
#define		kUSBPrintingProtocolUnidirectional	1
#define		kUSBPrintingProtocolBidirectional	2

#define 	kUSBPrintClassGetDeviceID			0
#define		kUSBPrintClassGetCentronicsStatus	1
#define		kUSBPrintClassSoftReset				2

/*
**	Apple MacOS X printer-class plugins
*/
#define 	kUSBPrinterClassTypeID				(CFUUIDGetConstantUUIDWithBytes(NULL, 0x06, 0x04, 0x7D, 0x16, 0x53, 0xA2, 0x11, 0xD6, 0x92, 0x06, 0x00, 0x30, 0x65, 0x52, 0x45, 0x92))

#define		kUSBPrinterClassInterfaceID			(CFUUIDGetConstantUUIDWithBytes(NULL, 0x03, 0x34, 0x6D, 0x74, 0x53, 0xA3, 0x11, 0xD6, 0x9E, 0xA1, 0x76, 0x30, 0x65, 0x52, 0x45, 0x92))

#define		kUSBPrinterClassFactoryID			(CFUUIDGetConstantUUIDWithBytes(NULL, 0xF1, 0x50, 0x6F, 0xC0, 0x53, 0xA0, 0x11, 0xD6, 0x89, 0x39, 0x00, 0x30, 0x65, 0x52, 0x45, 0x92))

#define 	kUSBGenericPrinterClassDriver		CFSTR( "/System/Library/Printers/Libraries/USBGenericPrinterClass.plugin" )
#define 	kUSBPostScriptPrinterClassDriver	CFSTR( "/System/Library/Printers/Libraries/USBPostScriptPrinterClass.plugin" )

#define		kUSBClassDriverProperty				CFSTR( "USB Printing Class" )
#define		kUSBPrinterClassDeviceNotOpen		-9664	/*kPMInvalidIOMContext*/

typedef union {
	char			b;
	struct {
		unsigned	reserved0:2;
		unsigned	paperError:1;
		unsigned	select:1;
		unsigned	notError:1;
		unsigned	reserved1:3;
	} status;
} CentronicsStatusByte;

typedef struct
{
    CFStringRef		manufacturer;	// manufacturer name
    CFStringRef		product;		// product name
    CFStringRef		compatible;		// compatible product name
    CFStringRef		serial;			// serial number
    CFStringRef		command;		// command set
    CFStringRef		ppdURL;			// url of the selected PPD, if any
} USBPrinterAddress;

typedef IOUSBInterfaceInterface182	**USBPrinterInterface;

typedef struct 
{
	UInt8		requestType;
	UInt8		request;
	UInt16		value;
	UInt16		index;
	UInt16		length;
	void		*buffer;	
} USBIODeviceRequest;

typedef struct classDriverContext
{
    IUNKNOWN_C_GUTS;
    CFPlugInRef				plugin;				// release plugin
    IUnknownVTbl			**factory;	
    void					*vendorReference;	// vendor class specific usage
    UInt32					location;			// unique location in bus topology
    UInt8					interfaceNumber;
    UInt16					vendorID;
    UInt16					productID;
    USBPrinterInterface		interface;			// identify the device to IOKit
    UInt8		  			outpipe;			// mandatory bulkOut pipe
    UInt8					inpipe;				// optional bulkIn pipe   
    /*
    **	general class requests
    */
    kern_return_t 	(*DeviceRequest)( struct classDriverContext **printer, USBIODeviceRequest *iorequest, UInt16 timeout );
    kern_return_t	(*GetString)( struct classDriverContext **printer, UInt8 whichString, UInt16 language, UInt16 timeout, CFStringRef *result );
    /*
    **	standard printer class requests
    */
    kern_return_t	(*SoftReset)( struct classDriverContext **printer, UInt16 timeout );
    kern_return_t	(*GetCentronicsStatus)( struct classDriverContext **printer, CentronicsStatusByte *result, UInt16 timeout );
    kern_return_t	(*GetDeviceID)( struct classDriverContext **printer, CFStringRef *devid, UInt16 timeout );
    /*
    **	standard bulk device requests
    */
    kern_return_t 	(*ReadPipe)( struct classDriverContext **printer, UInt8 *buffer, UInt32 *count );
    kern_return_t 	(*WritePipe)( struct classDriverContext **printer, UInt8 *buffer, UInt32 *count, Boolean eoj );
    /*
    **	interface requests
    */
    kern_return_t 	(*Open)( struct classDriverContext **printer, UInt32 location, UInt8 protocol );
    kern_return_t 	(*Abort)( struct classDriverContext **printer );
    kern_return_t 	(*Close)( struct classDriverContext **printer );
    /*
    **	initialize and terminate
    */
    kern_return_t 	(*Initialize)( struct classDriverContext **printer, struct classDriverContext **baseclass );
    kern_return_t 	(*Terminate)( struct classDriverContext **printer );
} USBPrinterClassContext;


typedef struct usbPrinterClassType
{
	USBPrinterClassContext	*classdriver;
	CFUUIDRef				factoryID;
	UInt32					refCount;
} USBPrinterClassType;

#endif
/* eof */
