/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under AppleÕs copyrights in this original Apple software (the
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
 *  GenericClassDriver.c
 *  Plugins
 *
 *  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 */
#include "PrinterClass.h"
#include "USBUtil.h"

/*-----------------------------------------------------------------------------*

	USBCharactersToHostCharacters
	
	Desc:	USB strings are UniChar, but always little-endian.
			Here we convert the string to the correct endianness for the host.
			
			
			We're isolated from the host architecture by the USB convenience
			function USBToHostWord.
			
	In:
		p		Pointer to the UniChar array
		len		length in UniChars

	Out:
		p		modified in place.

*-----------------------------------------------------------------------------*/

static void
UsbCharactersToHostCharacters( UniChar *p, UInt16 len )
{
	for ( ; len > 0; --len, ++p )
		*p = USBToHostWord( *p );
}

/*-----------------------------------------------------------------------------*

	UsbDeviceRequest

	Desc:	Pass a device request to this device.
	
			This exposes the devices requests to a printer module through
			GetAttribute.
	
	Usage from PrinterModule:
		USBIODeviceRequest	ioparam;
		OSStatus	result = (ourContext->ioProcs).PMIOGetAttributeProc( job, kUSBIOMVendorRequest, &ioparam );

	In:
		USBIOContext *context->usbInterfaceRef		identify the printer

	Out:
		If p->requestType in USBIn category
			p->length, and contents of p->buffer

*-----------------------------------------------------------------------------*/
static kern_return_t
DeviceRequest( USBPrinterClassContext **printingclass, USBIODeviceRequest *p, UInt16 timeout )
{
    IOUSBDevRequestTO	rq;
    IOReturn			kr = -1;
    USBPrinterInterface	intf = (*printingclass)->interface;

    if ( NULL != intf )
    {
        rq.bmRequestType = p->requestType;
        rq.bRequest = p->request;
        rq.wValue = p->value;
        rq.wIndex = p->index;
        rq.wLength = p->length;
        rq.pData = p->buffer;		// data pointer
        rq.completionTimeout = 0;
        rq.noDataTimeout = timeout;
    
        kr = (*intf)->ControlRequestTO( intf, (UInt8) 0, &rq );
    
        p->length = rq.wLenDone;
        DEBUG_ERR( kr, "DeviceRequest %x\n" );
    }

    return kr;
}

/*-----------------------------------------------------------------------------*

        Open

        Desc:	Using the locationID in  USBPrinterInfo,
                open the printer for exclusive acess
                    and set the (unidirectional, bidirectional) protocol

        In:		USBPrinterInfo *printingclass	locationID
                UInt8 							protocol

        Out:	printingclass->interface non-NULL if printer is open

		Note changes to classic arbitration.
*-----------------------------------------------------------------------------*/
static kern_return_t
Open( USBPrinterClassContext **printingclass, UInt32 targetLocation, UInt8 protocol )
{
    kern_return_t		kr = kUSBPrinterClassDeviceNotOpen;
    USBPrinterInterface	interface = (*printingclass)->interface;

    DEBUG_ERR( kIOReturnSuccess, "GenericClass:Open %x\n" );
    if ( NULL == interface )
    {
        mach_port_t			master_device_port = 0;
        io_service_t		usbInterface = 0;
        io_iterator_t 		iter = 0;
        // lazy initialization of class driver and interface
        do
        {
    
            kr = IOMasterPort( bootstrap_port, &master_device_port );
            DEBUG_ERR( kr, "GenericClass:Open IOMasterPort %x\n" );
            if(kIOReturnSuccess != kr)  break;
    
            {
                CFDictionaryRef 	usbMatch = NULL;
                
                // iterate over all interfaces. 
                usbMatch = IOServiceMatching(kIOUSBInterfaceClassName);
                if ( !usbMatch ) break;
                DEBUG_ERR( 0, "GenericClass:Open IOServiceMatching %x\n" );
            
                // IOServiceGetMatchingServices() consumes the usbMatch reference so we don't need to release it.
                kr = IOServiceGetMatchingServices(master_device_port, usbMatch, &iter);
                usbMatch = NULL;
                
                DEBUG_ERR( kr, "GenericClass:Open  IOServiceGetMatchingServices %x\n" );
                if(kIOReturnSuccess != kr || iter == NULL)  break;
            }
            
            while (  NULL != (usbInterface = IOIteratorNext(iter))  )
            {
                IOCFPlugInInterface 	**iodev;
                SInt32 					score;
                CFMutableDictionaryRef	properties;
                CFStringRef				classDriver = NULL;
    
                kr = IORegistryEntryCreateCFProperties (usbInterface, &properties, kCFAllocatorDefault, kNilOptions);
                if ( kIOReturnSuccess == kr && NULL != properties)
                {
                    classDriver = (CFStringRef) CFDictionaryGetValue( properties, kUSBClassDriverProperty );
                    if ( NULL != classDriver )
                        CFRetain( classDriver );
                    CFRelease( properties );
                }    
    
                kr = IOCreatePlugInInterfaceForService(	usbInterface,
                                                            kIOUSBInterfaceUserClientTypeID, 
                                                            kIOCFPlugInInterfaceID,
                                                            &iodev,
                                                            &score);
                    
                DEBUG_ERR( kr, "GenericClass:Open IOCreatePlugInInterfaceForService %x\n" );
                if ( kIOReturnSuccess == kr )
                {
                    UInt32				location;
                    USBPrinterInterface	intf;
                    HRESULT 			res;
    
                    res = (*iodev)->QueryInterface( iodev, USB_INTERFACE_KIND, (LPVOID *) &intf);
                    DEBUG_ERR( (kern_return_t) res, "GenericClass:Open QueryInterface %x\n" );
    
                    (*iodev)->Release(iodev);
                    if ( noErr != res ) break;

                    if ( NULL != intf )
                    {
                        kr = (*intf)->GetLocationID( intf, &location );
                        if ( kIOReturnSuccess ==  kr )
                        {
                            if ( location == targetLocation )
                            {
                                (*printingclass)->location = targetLocation;
                                (*printingclass)->interface = interface = intf;
                                IOObjectRelease(usbInterface);
                                break;
                            }
                        }
 
                        kr = (*intf)->Release(intf);
                    }
                } // if IOCreatePlugInInterfaceForService
                
                IOObjectRelease(usbInterface);
                usbInterface = NULL;
                
            } // while, there's an interface on the USB bus
        } while ( 0 );
    }
    if ( NULL != interface && protocol != kUSBPrintingProtocolNoOpen )
    {
        //
        //	open the interface and identify the pipes
        //
        //
        //	(Radar 2684463 Printer Offline when printing results in error)
        //	(Radar 2615118 A USB Print job blocked by an off printer doesn't restart)
        //	err == e00002c5 == -536870203 device is opened with exclusive status
        //	err == e00002fd == -536870163 printer is not responding
        //
        //	if another device has the printer open, or printer is off
        //		wait indefinitely until it's available to proceed
        //
        UInt8	i;
        UInt8	numPipes;
        UInt8	intProtocol;
        UInt8	alternateIndex = ~0,
                lastIndex = ~0;		// assume that no printer will have this as a default index!
        int		deviceNotAvailable;	// retry until the device is available
 
        numPipes = 0;
        (*printingclass)->outpipe = 0;	// pipe refnum 0 is invalid for i/o
        (*printingclass)->inpipe = 0;

        do 
        {

            kr = (*interface)->USBInterfaceOpen( interface );
            DEBUG_ERR( kr, "GenericClass:Open USBInterfaceOpen %x\n" );
			if ( kIOReturnExclusiveAccess == kr )
			{
				// Classic has the device open, and should release it for this call
				sleep(1);
				kr = (*interface)->USBInterfaceOpen( interface );
				DEBUG_ERR( kr, "GenericClass:Open USBInterfaceOpen %x\n" );
			}
            if ( kIOReturnSuccess ==  kr )
			{
                kr = (*interface)->GetAlternateSetting(interface, &alternateIndex );
				DEBUG_ERR( kr, "GenericClass:Open GetAlternateSetting %x\n" );
				if ( alternateIndex == lastIndex )	// if this fails we would loop indefinitely
					break;
				do 
				{
					lastIndex = alternateIndex;
					if ( kIOReturnSuccess ==  kr )
					{
						kr = (*interface)->SetAlternateInterface(interface, ++alternateIndex );
						DEBUG_ERR( kr, "GenericClass:Open SetAlternateInterface %x\n" );
					}
					if ( kIOReturnSuccess ==  kr )
						kr = (*interface)->GetInterfaceProtocol(interface, &intProtocol);
				} while ( kIOReturnSuccess ==  kr && protocol != intProtocol );
				//
				//	stash away some constants for class transactions that want them
				//
				if ( kIOReturnSuccess ==  kr )
				{
					kr = (*interface)->GetInterfaceNumber( interface, &(*printingclass)->interfaceNumber );
					if ( kIOReturnSuccess ==  kr )
						kr = (*interface)->GetDeviceVendor( interface, &(*printingclass)->vendorID );
					if ( kIOReturnSuccess ==  kr )
						kr = (*interface)->GetDeviceProduct( interface, &(*printingclass)->productID );
				}
				//
				//	notice that if we can't find the protocol
				//		we close the interface (the pipes aren't open)
				//		we'll return kIOReturnSuccess, but with printingclass->interface == NULL
				//
				if ( kIOReturnSuccess !=  kr || protocol != intProtocol)
				{
					kr = (*interface)->USBInterfaceClose( interface );
					(*printingclass)->interface =
					interface = NULL;
				}
				else
				{
					kr = (*interface)->GetNumEndpoints(interface, &numPipes);
					DEBUG_ERR( kr, "GenericClass:Open GetNumEndpoints %x\n");
					for (i=1; kIOReturnSuccess == kr && i<=numPipes; i++)
					{
						UInt8	number, direction, transferType, interval;
						UInt16	mps;
						kr = (*interface)->GetPipeProperties(interface, i, &direction, &number, &transferType, &mps, &interval);                    
						if ( kIOReturnSuccess == kr && kUSBBulk == transferType )
						{
							if ( kUSBIn == direction )
								(*printingclass)->inpipe = i;
							else	
								(*printingclass)->outpipe = i;
						}
					}
				}
			}
			
            deviceNotAvailable = kIOReturnExclusiveAccess == kr
                                    || kIOReturnNotResponding == kr;
            if ( deviceNotAvailable )
			{
                sleep( 5 );
				fprintf( stderr, "INFO: GenericClass: waiting for device\n" );
			}
        } while ( deviceNotAvailable );
 
    }
    return kr;
}

static kern_return_t
Read( USBPrinterClassContext **printer, UInt8 *buffer, UInt32 *size )
{
    kern_return_t	kr = kUSBPrinterClassDeviceNotOpen;
	DEBUG_ERR( kIOReturnSuccess, "GenericClass: Read %x\n" );
    if ( NULL != printer && NULL != (*printer)->interface )
        kr = (*(*printer)->interface)->ReadPipe( (*printer)->interface, (*printer)->inpipe, buffer, size );

    return kr;
}

static kern_return_t
Write( USBPrinterClassContext **printer, UInt8 *buffer, UInt32 *size, Boolean eoj )
{
    kern_return_t	kr = kUSBPrinterClassDeviceNotOpen;
	DEBUG_ERR( kIOReturnSuccess, "GenericClass: Write %x\n" );
    if ( NULL != printer && NULL != (*printer)->interface )
        kr = (*(*printer)->interface)->WritePipe( (*printer)->interface, (*printer)->outpipe, buffer, *size );

    return kr;
}

static kern_return_t
Abort( USBPrinterClassContext **printer )
{
    //
    //	terminate any pending IO before closing the connection
    //	Abort might leave the data toggle incorrect, so clear any
    //	pending pipe stalls
    //
    kern_return_t	kr = kUSBPrinterClassDeviceNotOpen;
	DEBUG_ERR( kIOReturnSuccess, "GenericClass: Abort %x\n" );
    if ( NULL != printer && NULL != (*printer)->interface )
    {
        kr = kIOReturnSuccess;
        if ( (*printer)->inpipe )
        {
            if ( kIOReturnSuccess == kr ) kr = (*(*printer)->interface)->AbortPipe( (*printer)->interface, (*printer)->inpipe );
            if ( kIOReturnSuccess == kr ) kr =(*(*printer)->interface)->ClearPipeStall( (*printer)->interface, (*printer)->inpipe );
        } 
        if ( (*printer)->outpipe )
        {
            if ( kIOReturnSuccess == kr ) kr =(*(*printer)->interface)->AbortPipe( (*printer)->interface, (*printer)->outpipe );
            if ( kIOReturnSuccess == kr ) kr =(*(*printer)->interface)->ClearPipeStall( (*printer)->interface, (*printer)->outpipe );
        }
    }
    return kr;
}

static kern_return_t
Close( USBPrinterClassContext **printer )
{
    kern_return_t	kr = kUSBPrinterClassDeviceNotOpen;
	DEBUG_ERR( kIOReturnSuccess, "GenericClass: Close %x\n" );
    if( NULL != printer && NULL != (*printer)->interface )
    {
		USBPrinterInterface	intf = (*printer)->interface;
        kr =(*intf)->USBInterfaceClose( intf );
        if ( kIOReturnSuccess == kr )
            kr =(*intf)->Release( intf );
    }
    
    return kr;
}

/*-----------------------------------------------------------------------------*

	UsbGetDeviceID

	Desc:	Get a 1284 device id string from this device (which must be a 
			printer class device!)

			Find the length of the deviceID string.
			Allocate a string buffer of the full length, and read it in.

			We attempt a saving of i/o by allocating a buffer on the
			stack, and if the initial fetch of the deviceID is a complete fetch,
			don't retry the i/o.
			
	Usage from PrinterModule:
		    CFStringRef deviceId;
			OSStatus	result = (ourContext->ioProcs).PMIOGetAttributeProc( job, kUSBIOMGetDeviceID, (CFTypeRef *) &deviceId );

	In:
		USBIOContext *context->usbInterfaceRef		identify the printer
					context->configIndex			the active config
					context->interfaceIndex			the active interface
					context->alternateIndex			the active alternate interface
	Out:
		CFStringRef	result							the IEEE-1284 DeviceID string
*-----------------------------------------------------------------------------*/

static kern_return_t
GetDeviceID( USBPrinterClassContext **printerclass, CFStringRef *result, UInt16 timeout )
{
    char				findsize[256];  //  a small string used at minimum to find the real string length
    kern_return_t		kr = -1;
    UInt8				configValue = 1,
                        alternateIndex = 0;
    USBPrinterInterface	interface = NULL != printerclass? (*printerclass)->interface: NULL;
    USBIODeviceRequest	request;

	DEBUG_ERR( kIOReturnSuccess, "GenericClass: GetDeviceID  %x\n" );
    *result = NULL;	// assume string doesn't exist
    if ( NULL != interface )
    {
        kr = (*interface)->GetConfigurationValue( interface, &configValue );
        DEBUG_ERR( kr, "GetDeviceID GetConfigurationValue %d\n" );
        {
            kr = (*interface)->GetInterfaceNumber( interface, &(*printerclass)->interfaceNumber );
            DEBUG_ERR( kr, "GetDeviceID GetInterfaceNumber %d\n" );
        }
        if ( kIOReturnSuccess == kr )
        {
            kr = (*interface)->GetAlternateSetting( interface, &alternateIndex );
            DEBUG_ERR( kr, "GetDeviceID GetAlternateSetting %d\n" );
        }
        if ( kIOReturnSuccess == kr )
        {
            request.requestType = USBmakebmRequestType(kUSBIn, kUSBClass, kUSBInterface);
            request.request = kUSBPrintClassGetDeviceID;
            request.value = configValue-1;
            request.index = ((*printerclass)->interfaceNumber<<8) | alternateIndex;
            request.length = sizeof(findsize);
            request.buffer = findsize;
            kr = (*printerclass)->DeviceRequest( printerclass, &request, timeout );
            DEBUG_ERR( kr, "GetDeviceID DeviceRequest 1 %d\n" );
        }
     
        if ( kIOReturnSuccess == kr && request.length > 1)
        {
            UInt16 result_length;
            
            result_length = ((unsigned char) findsize[0] << 8);		// note this length does not include the length bytes
            result_length |= (unsigned char) findsize[1];
        
            if ( result_length < sizeof(findsize)-2 )
            {
                // Happily, we don't need to do additional i/o: we've already read in the complete device id string.
                // We just need to move it from the stack into the buffer we've allocated.
                //
                //	We use the byte constructor here because the result could contain NUL bytes
                //
                DEBUG_DUMP( "GetDeviceID DeviceID 1:", findsize, result_length );
                *result = CFStringCreateWithBytes( kCFAllocatorDefault, (UInt8 *) &findsize[2], result_length - 2, kCFStringEncodingUTF8, 0 );
                DEBUG_CFString( "GetDeviceID result:", *result );
            }
            else
            {
                // This is a long id we didn't allocate enough stack to read completely on the first i/o request.
                // Read the target device id directly into the allocated buffer.
                char *result_string = (char *) malloc( result_length );
                if ( result_string != NULL )
                {
                    request.requestType = USBmakebmRequestType(kUSBIn, kUSBClass, kUSBInterface);
                    request.request = kUSBPrintClassGetDeviceID;
                    request.value = configValue-1;
                    request.index = ((*printerclass)->interfaceNumber<<8) | alternateIndex;
                    request.length = result_length;
                    request.buffer = result_string;

                    kr = (*printerclass)->DeviceRequest( printerclass, &request, timeout );
                    DEBUG_ERR( kr, "GetDeviceID DeviceRequest 2 %d\n" );
                    if ( kIOReturnSuccess == kr && request.length > 0)
                    {
                        DEBUG_DUMP( "GetDeviceID DeviceID 2:", result_string, result_length );
                        result_length = ((unsigned char) result_string[0] << 8);
                        result_length |= (unsigned char) result_string[1];
                        *result = CFStringCreateWithBytes( kCFAllocatorDefault, (UInt8 *) &result_string[2], result_length-2, kCFStringEncodingUTF8, 0 );
                        DEBUG_CFString( "GetDeviceID UsbGetDeviceID2 result:", *result );       
                    }
                    free( result_string );
                }
            }
        }
    }
    DEBUG_ERR( kr, "GetDeviceID end %d\n" );
    return kr;
}

/*-----------------------------------------------------------------------------*

	UsbGetCentronicsStatus

	Desc:	Get the Centronics Status byte from this device (which must be a 
			printer class device!)

			This exposes the printer class request to a printer module through
			GetAttribute.
	
	Usage from PrinterModule:
		CentronicsStatusByte  p;
		OSStatus	result = (ourContext->ioProcs).PMIOGetAttributeProc( job, kUSBIOMGetDeviceID, (CFTypeRef *) &p.b );

	In:
		printer		identify the printer and the active interface
	Out:
		CentronicsStatusByte	*p;				single byte Centronics Status

*-----------------------------------------------------------------------------*/
static kern_return_t
GetCentronicsStatus( USBPrinterClassContext **printerclass, CentronicsStatusByte *p, UInt16 timeout )
{
    USBIODeviceRequest	request;
    kern_return_t		kr = -1;

	p->b = '\0';		// return a default result

    request.requestType = USBmakebmRequestType(kUSBIn, kUSBClass, kUSBInterface);
	request.request = kUSBPrintClassGetCentronicsStatus;
    request.index = 0;
    request.length = 0;
    request.buffer = &p->b;
    request.value = (*printerclass)->interfaceNumber;

    kr = (*printerclass)->DeviceRequest( printerclass, &request, timeout );

    DEBUG_ERR( kr, "GetCentronicsStatus %x\n" );
    return kr;
}

/*-----------------------------------------------------------------------------*

	UsbSoftReset

	Desc:	Perform a soft reset on this device (which must be a printer 
			class device!)

			This exposes the printer class request to a printer module through
			SetAttribute.
	
	Usage from PrinterModule:
		OSStatus	result = (ourContext->ioProcs).PMIOSetAttributeProc( job, kUSBIOMSoftReset, NULL );

	In:
		USBPrinterInterface	intf		identify the printer

	Out:
		<none>

*-----------------------------------------------------------------------------*/
static kern_return_t
SoftReset( USBPrinterClassContext **printerclass, UInt16 timeout )
{
    UInt16				len = 0;
    USBIODeviceRequest	request;
    kern_return_t		kr = -1;

    request.requestType = USBmakebmRequestType(kUSBOut, kUSBClass, kUSBInterface);
	request.request = kUSBPrintClassSoftReset;
    request.index = 0;
    request.length = len;
    request.buffer = NULL;
    request.value = (*printerclass)->interfaceNumber;
    
    kr = (*printerclass)->DeviceRequest( printerclass, &request, timeout );
    DEBUG_ERR( kr, "SoftReset %x\n" );
    return kr;
}

/*-----------------------------------------------------------------------------*

	GetString

	Desc:	Get a string from this device.

			USB strings are Unicode; we correct to make sure the endianness
			agrees with our platform.
			
			Also, the strings are prefixed with the USB string descriptor type
			(Byte 0x3) and the length of the string (byte). We try to read in
			the complete string.

			Find the length of the target string
			Allocate a string of the full length
			Read in the target string

	In:
		devRefNum
		string_id
		language

	Out:
		CFStringRef		the Unicode string

*-----------------------------------------------------------------------------*/

static kern_return_t
GetUSBString( USBPrinterClassContext **printerclass, UInt8 string_id, UInt16 language, UInt16 timeout, CFStringRef *result )
{
	kern_return_t		kr = -1;
	UInt8				findsize[258];  //  a small string used at minimum to find the real string length
	UInt16				result_length;
    USBIODeviceRequest	request;

	DEBUG_ERR( kIOReturnSuccess, "GenericClass: GetUSBString %x\n" );
    *result = NULL;	// assume string doesn't exist
	if ( string_id != 0 )
	{
		result_length = sizeof(findsize);  // start with a string that's statically allocated
        request.requestType = USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
        request.request = kUSBRqGetDescriptor;
        request.value = (kUSBStringDesc<<8) | string_id;
        request.index = language;
        request.length = result_length;
        request.buffer = findsize;
        kr = (*printerclass)->DeviceRequest( printerclass, &request, timeout );

		if ( kIOReturnSuccess == kr && result_length > 0 && result_length <= sizeof(findsize) )
		{
		
			result_length = findsize[0]; // string descriptor, string length, text
			if (  0 < result_length && result_length <= sizeof(findsize) )
			{
				// USB strings are Little-Endian: convert to Host-Endian
				// don't swap the byte the descriptor type (should always be 3), or the length
				UsbCharactersToHostCharacters( ((UniChar *) findsize) + 1, (result_length - 2)>>1 );

 				// Happily, we don't need to do additional i/o: we've already read in the complete string.
				// We just need to move it from the stack into the buffer we've allocated.
                *result = CFStringCreateWithCharacters( kCFAllocatorDefault, ((UniChar *) findsize) + 1, (result_length - 2)>>1 );
			}
		}
	}

	return kr;
}


static void
deallocate(USBPrinterClassType *printer)
{
	CFUUIDRef factoryID = printer->factoryID;
	
	DEBUG_ERR( kIOReturnSuccess, "GenericClass: deallocate %x\n" );
	if ( printer->classdriver )
	{
		free( printer->classdriver );
		printer->classdriver = NULL;
	}
	free(printer);
	if (factoryID)
	{
		CFPlugInRemoveInstanceForFactory(factoryID);
		CFRelease(factoryID);
	}
	
}

static ULONG
AddRef(void *printer)
{
	DEBUG_ERR( kIOReturnSuccess, "GenericClass: AddRef %x\n" );
	((USBPrinterClassType *)printer)->refCount += 1;
	return ((USBPrinterClassType *)printer)->refCount;
}

static ULONG
Release(void *printer)
{
	ULONG result;	// not available after deallocate
	DEBUG_ERR( kIOReturnSuccess, "GenericClass: Release %x\n" );
	((USBPrinterClassType *)printer)->refCount -= 1;
	result = ((USBPrinterClassType *)printer)->refCount;
	if ( result == 0)
	{
		deallocate((USBPrinterClassType *)printer);
	}
	return result;
}

static HRESULT
QueryInterface(void *context, REFIID iid, LPVOID *ppv)
{
	CFUUIDRef				interfaceID = CFUUIDCreateFromUUIDBytes(NULL, iid);
	USBPrinterClassType		*printer = (USBPrinterClassType *) context;

	DEBUG_ERR( kIOReturnSuccess, "GenericClass: QueryInterface %x\n" );
	if (CFEqual(interfaceID, kUSBPrinterClassInterfaceID))
	{
		printer->classdriver->AddRef(printer);
		*ppv = context;
		return S_OK;
	}
	else if (CFEqual(interfaceID, IUnknownUUID))
	{
		printer->classdriver->AddRef(printer);
		*ppv = context;
		return S_OK;
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
}

static kern_return_t
Terminate( USBPrinterClassContext **printerclass )
{
    //
    //	unload any loaded class driver
    //

    if ( NULL != (*printerclass)->plugin )
        CFRelease( (*printerclass)->plugin );
    (*printerclass)->plugin = NULL;

    return kIOReturnSuccess;
}

static kern_return_t
Initialize( USBPrinterClassContext **printerclass, USBPrinterClassContext **baseclass )
{
	//	
	// as the default printer class
	//		base class should be NULL and we ignore it
	// this duplicates what we do in allocate
	//		normally, the class driver would copy over baseclass items to fill out it's table
	//
	DEBUG_ERR( kIOReturnSuccess, "GenericClass: Initialize %x\n" );
    if ( NULL != printerclass )
    {
        (*printerclass)->outpipe = 0;				// mandatory bulkOut pipe
        (*printerclass)->inpipe = 0;				// optional bulkIn pipe    
        (*printerclass)->interface = NULL;			// identify the device to IOKit
        (*printerclass)->vendorReference = NULL;	// class specific usage
    
        (*printerclass)->AddRef = AddRef;
        (*printerclass)->Release = Release;
        (*printerclass)->QueryInterface = QueryInterface;

        (*printerclass)->DeviceRequest = DeviceRequest;
        (*printerclass)->ReadPipe = Read;
        (*printerclass)->WritePipe = Write;
        (*printerclass)->Open = Open;
        (*printerclass)->Abort = Abort;
        (*printerclass)->Close = Close;
        (*printerclass)->GetDeviceID = GetDeviceID;
        (*printerclass)->GetCentronicsStatus = GetCentronicsStatus;
        (*printerclass)->SoftReset = SoftReset;
        (*printerclass)->GetString = GetUSBString;

        (*printerclass)->Initialize = Initialize; // Initialize can't be overridden
        (*printerclass)->Terminate = Terminate;
    }
    return kIOReturnSuccess;
}

static USBPrinterClassType *
allocate(CFUUIDRef factoryID)
{
	USBPrinterClassType *newOne = (USBPrinterClassType *)calloc(1, sizeof(USBPrinterClassType));
	
	DEBUG_ERR( kIOReturnSuccess, "GenericClass: allocate %x\n" );
	if ( NULL != newOne )
	{
		newOne->classdriver = (USBPrinterClassContext *)calloc(1, sizeof(USBPrinterClassContext));
		if ( newOne->classdriver )
		{
			Initialize( &newOne->classdriver, NULL );
		}
		if (factoryID)
		{
			newOne->factoryID = CFRetain(factoryID);
			CFPlugInAddInstanceForFactory(factoryID);
		}
		// One ref (on IUnknown)
		newOne->refCount = 1;
	}
	return newOne;
}

void *
USBPrinterClassFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
	DEBUG_ERR( kIOReturnSuccess, "GenericClass: USBPrinterClassFactory %x\n" );
	if (CFEqual(typeID, kUSBPrinterClassTypeID))
	{
		USBPrinterClassType *result = allocate(kUSBPrinterClassFactoryID);
		return result;
	}
	else 
	{
		return NULL;
	}
}

/* eof */
