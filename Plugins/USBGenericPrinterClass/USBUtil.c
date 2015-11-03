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
	File:		USBUtil.c
	Contains: 	Utility Functions to access USB with IOKit.

	Copyright 1999-2002 by Apple Computer, Inc., all rights reserved.

	Description:
		
		
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Pragmas
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Includes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h> // for malloc, free
#include <stdio.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <PrintCore/PMIOModule.h>

#include "USBUtil.h"
#include "DeviceID.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Constants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Type definitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Function prototypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static CFStringRef CreateEncodedCFString(CFStringRef string);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Global variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Inlines
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Private Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if DEBUG==2  

#if defined(__cplusplus)
extern "C" {
#endif

static char
hexdigit( char c )
{
    return ( c < 0 || c > 15 )? '?': (c < 10)? c + '0': c - 10 + 'A';
}

static char
asciidigit( char c )
{
    return (c< 20 || c > 0x7E)? '.': c;
}

void
dump( char *text, void *s, int len )
{
    int i;
    char *p = (char *) s;
    char m[1+2*16+1+16+1];

    fprintf( stderr, "%s pointer %x len %d\n", text, (unsigned int) p, len );

    for ( ; len > 0; len -= 16 )
    {
        char *q = p;
        char *out = m;
        *out++ = '\t';
        for ( i = 0; i < 16 && i < len; ++i, ++p )
        {
            *out++ = hexdigit( (*p >> 4) & 0x0F );
                                 *out++ = hexdigit( *p & 0x0F );
        }
        for ( ;i < 16; ++i )
        {
            *out++ = ' ';
            *out++ = ' ';
        }
        *out++ = '\t';
        for ( i = 0; i < 16 && i < len; ++i, ++q )
            *out++ = asciidigit( *q );
        *out = 0;
        m[ strlen( m ) ] = '\0';
        fprintf( stderr,  "%s\n", m );
    }
}

void 
printcfs( char *text, CFStringRef s )
{
    char dest[1024];
    if ( s != NULL )
    {
        if ( CFStringGetCString(s, dest, sizeof(dest), kCFStringEncodingUTF8) )
            fprintf( stderr,  "%s <%s>\n", text, dest );
        else
            fprintf( stderr,  "%s [Unknown string]\n", text );
    } else {
       fprintf( stderr,  "%s [NULL]\n", text );
    }

}

void
cmpcfs( char *text, CFStringRef a, CFStringRef b )
{
    CFRange found = {0, 0};
    
    printcfs( text, a );
    printcfs( " ", b );

    if (a != NULL && b != NULL) {
        found = CFStringFind( a, b, kCFCompareCaseInsensitive );
	
    } else if (a == NULL && b == NULL) {
        found.length = 1;	// Match
        found.location = 0;
    } else {
        found.length = 0;	// No match.
    }
    
    if ( found.length > 0 )
        fprintf( stderr,  "matched @%d:%d\n", (int) found.location, (int) found.length);
    else
        fprintf( stderr,  "not matched\n" );
}
#if defined(__cplusplus)
}   
#endif
#endif

/******************************************************************************/

/*-----------------------------------------------------------------------------*

CompareSameString

Desc:	Return the CFCompare result for two strings, either or both of which
        can be NULL.

In:
        a		current value
        b		last value

Out:
        0		if the strings match
        non-zero	if the strings don't match

*-----------------------------------------------------------------------------*/
static int
CompareSameString( const CFStringRef a, const CFStringRef b )
{
    if ( NULL == a && NULL == b )
        return 0;
    else if ( NULL != a && NULL != b )
        return CFStringCompare( a, b, kCFCompareAnchored );
    else
        return 1;	// one of a or b is NULL this time, but wasn't last time
}

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Public
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/
kern_return_t
UsbLoadClassDriver( USBPrinterInfo *printer, CFUUIDRef interfaceID, CFStringRef classDriverBundle )
{
	kern_return_t	kr = kUSBPrinterClassDeviceNotOpen;
	if ( NULL == classDriverBundle )
	{
		//
		//	supply the generic (raster) or generic PostScript class driver
		//
		CFRange	result = DeviceIDSupportedValue( printer->address.command, CFSTR("CMD:PostScript"), kDeviceIDKeyCommandAbbrev, kDeviceIDKeyCommand );
		classDriverBundle = result.length > 0 ? kUSBPostScriptPrinterClassDriver: kUSBGenericPrinterClassDriver;
	}
	DEBUG_CFString( "UsbLoadClassDriver classDriverBundle", classDriverBundle );
    if ( NULL != classDriverBundle )
    {
        USBPrinterClassContext	**classdriver = NULL;
        CFURLRef				classDriverURL = CFURLCreateWithFileSystemPath( NULL, classDriverBundle, kCFURLPOSIXPathStyle, TRUE );
        CFPlugInRef 			plugin = NULL == classDriverURL? NULL: CFPlugInCreate( NULL, classDriverURL );
        if ( NULL != plugin)
        {
            // See if this plug-in implements the Test type.
            CFArrayRef factories =  CFPlugInFindFactoriesForPlugInType( kUSBPrinterClassTypeID );

            // If there are factories for the requested type, attempt to
            // get the IUnknown interface.
			DEBUG_ERR( kr, "UsbLoadClassDriver plugin %x\n" );
            if (NULL != factories && CFArrayGetCount(factories) > 0) 
            {
                // Get the factory ID for the first location in the array of IDs.
                CFUUIDRef factoryID = CFArrayGetValueAtIndex( factories, 0 );
                // Use the factory ID to get an IUnknown interface.
                // Here the code for the PlugIn is loaded.
                IUnknownVTbl **iunknown = CFPlugInInstanceCreate( NULL, factoryID, kUSBPrinterClassTypeID );
                // If this is an IUnknown interface, query for the Test interface.
				DEBUG_ERR( kr, "UsbLoadClassDriver factories %x\n" );
                if (NULL != iunknown)
                {
					DEBUG_ERR( kr, "UsbLoadClassDriver CFPlugInInstanceCreate %x\n" );
                    kr = (*iunknown)->QueryInterface( iunknown, CFUUIDGetUUIDBytes(interfaceID), (LPVOID *) &classdriver );

                    (*iunknown)->Release( iunknown );
                    if ( S_OK == kr && NULL != classdriver )
                    {
						DEBUG_ERR( kr, "UsbLoadClassDriver QueryInterface %x\n" );
						printer->plugin = plugin;
                        kr = (*classdriver)->Initialize( classdriver, printer->classdriver );
						
						kr = kIOReturnSuccess;
						printer->classdriver = classdriver;
                    }
					else
					{
						DEBUG_ERR( kr, "UsbLoadClassDriver QueryInterface FAILED %x\n" );
					}
                }
				else
				{
					DEBUG_ERR( kr, "UsbLoadClassDriver CFPlugInInstanceCreate FAILED %x\n" );
				}
            }
			else
			{
				DEBUG_ERR( kr, "UsbLoadClassDriver factories FAILED %x\n" );
			}
        }
		else
		{
			DEBUG_ERR( kr, "UsbLoadClassDriver plugin FAILED %x\n" );
		}
        if ( kr != kIOReturnSuccess || NULL == plugin || NULL == classdriver )
        {
            UsbUnloadClassDriver( printer );
        }
    }
	
	return kr;
}


kern_return_t
UsbUnloadClassDriver( USBPrinterInfo *printer )
{
	DEBUG_ERR( kIOReturnSuccess, "UsbUnloadClassDriver %x\n" );
	if ( NULL != printer->classdriver )
		(*printer->classdriver)->Release( printer->classdriver );
	printer->classdriver = NULL;
	
	if ( NULL != printer->plugin )
		CFRelease( printer->plugin );
	printer->plugin = NULL;
	
	return kIOReturnSuccess;
}

/*-----------------------------------------------------------------------------*

	UsbSupportedPrinter

	Desc:	Check if this printer is supported by the current module.
	
			The moduleDeviceID is one or more key-value pairs. Since we've already
			isolated the values from the printer's CMD, MDL, and MFG, we need to
			find if the moduleDeviceID contains a (e.g.) CMD. If it does, then if
			any value in the module's CMD occurs in the printer's command, we
			have a match.
			
			e.g., address->command = "PCL 6 Emulation, Postscript Level 2 Emulation"
				  moduleDeviceID = "CMD:PostScript"
			is a match, since PostScript occurs in address->command.
				
			Similarly, address->product = "DESKJET 970C"
					moduleDeviceID = "MDL:DeskJet"
			is a match.

	In:		address		the printer address we're checking

	Out:	<none>

*-----------------------------------------------------------------------------*/
int
UsbSupportedPrinter(USBPrinterAddress *address, CFStringRef moduleDeviceID )
{
	CFRange	result = DeviceIDSupportedValue( address->command, moduleDeviceID, kDeviceIDKeyCommandAbbrev, kDeviceIDKeyCommand );

	if ( result.length <= 0 )
    {
		result = DeviceIDSupportedValue( address->product, moduleDeviceID, kDeviceIDKeyModelAbbrev, kDeviceIDKeyModel );
        //
        //	if the model isn't supported directly, see if the model is compatible with an existing driver
        //
        if ( result.length > 0 && NULL != address->compatible )
            result = DeviceIDSupportedValue( address->compatible, moduleDeviceID, kDeviceIDKeyCompatibleAbbrev, kDeviceIDKeyCompatible );
    }

	if ( result.length <= 0 )
		result = DeviceIDSupportedValue( address->manufacturer, moduleDeviceID, kDeviceIDKeyManufacturerAbbrev, kDeviceIDKeyManufacturer );

	return result.length <= 0? 0: 1;
}


/*-----------------------------------------------------------------------------*

	UsbAddressDispose

	Desc:	deallocates anything used to create a persistent printer address

	In:	address		the printer address we've created

	Out:	<none>

*-----------------------------------------------------------------------------*/
void
UsbAddressDispose( USBPrinterAddress *address )
{
	if ( address->product != NULL ) CFRelease( address->product );
	if ( address->manufacturer != NULL ) CFRelease( address->manufacturer );
	if ( address->serial != NULL ) CFRelease( address->serial );
	if ( address->command != NULL ) CFRelease( address->command );

	address->product =
	address->manufacturer =
	address->serial =
	address->command = NULL;

}

/*-----------------------------------------------------------------------------*

	UsbGetPrinterAddress

	Desc:	Given a printer we're enumerating, discover it's persistent
	reference.

	A "persistent reference" is one which enables us to identify
	a printer regardless of where it resides on the USB topology,
	and enumeration sequence.

	To do this, we actually construct a reference from information
	buried inside the printer. First we look at the USB device
	descripton: an ideally defined device will support strings for
	manufacturer and product id, and serial number. The serial number
	will be unique for each printer.

	Our prefered identification fetches the IEEE-1284 device id string.
	This transparently handled IEEE-1284 compatible printers which
	connected over a USB-parallel cable. Only if we can't get all the
	information to uniquely identify the printer do we try the strings
	referenced in the printer's USB device descriptor. (These strings
	are typically absent in a USB-parallel cable.)

	If a device doesn't support serial numbers we have a problem:
	we can't distinguish between two identical printers. Unique serial
	numbers allow us to distinguish between two same-model, same-manufacturer
	USB printers.

	In:
		thePrinter		iterator required for fetching device descriptor
		devRefNum		required to configure the interface

	Out:
		address->manufacturer
		address->product
		address->serial
				Any (and all) of these may be NULL if we can't retrieve
				information for IEEE1284 DeviceID or the USB device
				descriptor. Caller should be prepared to handle such a case.
		address->command
				May be updated.

*-----------------------------------------------------------------------------*/
OSStatus
UsbGetPrinterAddress( USBPrinterInfo *thePrinter, USBPrinterAddress *address, UInt16 timeout )
{

	//
	//	start by assuming the device is not IEEE-1284 compliant
	//	and that we can't read in the required strings.
	//
	OSStatus				err;
    CFStringRef				deviceId = NULL;
    USBPrinterClassContext	**printer = NULL == thePrinter? NULL: thePrinter->classdriver;
	
	address->manufacturer =
	address->product =
    address->compatible =
	address->serial =
    address->command = NULL;

    DEBUG_DUMP( "UsbGetPrinterAddress thePrinter", thePrinter, sizeof(USBPrinterInfo) );

    err = (*printer)->GetDeviceID( printer, &deviceId, timeout );
    if ( noErr == err && NULL != deviceId )
    {
        // the strings embedded here are defined in the IEEE1284 spec
        //
        //	use the MFG/MANUFACTURER for the manufacturer
        //	and the MDL/MODEL for the product
        //  there is no serial number defined in IEEE1284
        //		but it's been observed in recent HP printers
        //
        address->command = DeviceIDCreateValueList( deviceId, kDeviceIDKeyCommandAbbrev, kDeviceIDKeyCommand );

        address->product = DeviceIDCreateValueList( deviceId, kDeviceIDKeyModelAbbrev, kDeviceIDKeyModel );
        address->compatible = DeviceIDCreateValueList( deviceId, kDeviceIDKeyCompatibleAbbrev, kDeviceIDKeyCompatible );

        address->manufacturer = DeviceIDCreateValueList( deviceId, kDeviceIDKeyManufacturerAbbrev, kDeviceIDKeyManufacturer );

        address->serial = DeviceIDCreateValueList( deviceId, kDeviceIDKeySerialAbbrev, kDeviceIDKeySerial );
        CFRelease( deviceId );
    }
    DEBUG_CFString( "UsbGetPrinterAddress DeviceID address->product", address->product );
    DEBUG_CFString( "UsbGetPrinterAddress DeviceID address->compatible", address->compatible );
    DEBUG_CFString( "UsbGetPrinterAddress DeviceID address->manufacturer", address->manufacturer );
    DEBUG_CFString( "UsbGetPrinterAddress DeviceID address->serial", address->serial );

    if ( NULL == address->product || NULL == address->manufacturer || NULL == address->serial )
    {
        //
        //	if the manufacturer or the product or serial number were not specified in DeviceID
        //		try to construct the address using USB English string descriptors
        //
        IOUSBDeviceDescriptor	desc;
        USBIODeviceRequest		request;
                                
        request.requestType = USBmakebmRequestType( kUSBIn,  kUSBStandard, kUSBDevice );
        request.request = kUSBRqGetDescriptor;
        request.value = (kUSBDeviceDesc << 8) | 0;
        request.index = 0; 	/* not kUSBLanguageEnglish*/
        request.length = sizeof(desc);
        request.buffer = &desc;
        err = (*printer)->DeviceRequest( printer, &request, timeout );
        DEBUG_ERR( (kern_return_t) err, "UsbGetPrinterAddress: GetDescriptor %x" );
        if ( kIOReturnSuccess == err )
        {
            // once we've retrieved the device descriptor
            //	try to fill in missing pieces of information
            //
            //	Don't override any information already retrieved from DeviceID.

            if ( NULL == address->product)
            {
                err = (*printer)->GetString( printer, desc.iProduct, kUSBLanguageEnglish, timeout, &address->product );
                if ( kIOReturnSuccess != err || address->product == NULL) {
                    address->product = CFSTR("Unknown");
                }                
            }
            DEBUG_CFString( "UsbGetPrinterAddress: UsbGetString address->product\n", address->product );

            if ( NULL == address->manufacturer )
            {
                err = (*printer)->GetString( printer, desc.iManufacturer, kUSBLanguageEnglish, timeout, &address->manufacturer );
                if (kIOReturnSuccess != err || address->manufacturer == NULL) {
                    address->manufacturer = CFSTR("Unknown");
                }
            }
            DEBUG_CFString( "UsbGetPrinterAddress: UsbGetString address->manufacturer\n", address->manufacturer );

            if ( NULL == address->serial )
            {
                err = (*printer)->GetString( printer, desc.iSerialNumber, kUSBLanguageEnglish, timeout, &address->serial );
                // if the printer doesn't have a serial number, use locationId
                if ( NULL == address->serial )
                {
                   address->serial = CFStringCreateWithFormat( NULL, NULL, CFSTR("%lx"), (*printer)->location );
                }
            }
            DEBUG_CFString( "UsbGetPrinterAddress: UsbGetString address->serial\n", address->serial );
        }
    }
    if ( NULL != address->product)
        CFRetain(address->product);         // UsbGetString is really a UsbCopyString.
    if ( NULL != address->manufacturer )
        CFRetain( address->manufacturer );
    if ( NULL != address->serial )
        CFRetain( address->serial );
    return err;
}

/*-----------------------------------------------------------------------------*

	UsbAddressRecover

	Desc:	Construct a run-time address using the serialized persistant
			USB address.
 
			A run-time address binds the persistent address to the active
			bus enumeration and placement of the device in the device
			tree.
			
			We use a triple retreived from the printer to identify it:
				the manufacturer		e.g., 'Canon', 'Hewlett-Packard'
				the product				e.g., 'Epson Stylus 800', 'DeskJet 970'
				the serial number 		<any ANSI 7-bit string>
 
			Once we bind the printer here, a hot-unplug will invalidate the
			address we've constructed.

	In:		C-String	the persistent address

	Out:	address		the USB printer address to which we've bound

*-----------------------------------------------------------------------------*/
OSStatus
UsbAddressRecover( CFDataRef pmaddr, USBPrinterAddress *address )
{
	OSStatus err = noErr;
	CFMutableDictionaryRef addressDictRef = NULL;

	addressDictRef = (CFMutableDictionaryRef) CFPropertyListCreateFromXMLData(NULL, pmaddr, kCFPropertyListImmutable, NULL);
	if (addressDictRef == NULL)
		err = -1;	//kPMInvalidPrinterAddress;

	// make sure it's a dictionary
	if (err == noErr && CFGetTypeID(addressDictRef) != CFDictionaryGetTypeID())
		err = -2;	//kPMInvalidPrinterAddress;

	if (err == noErr)
	{
		address->manufacturer	= (CFStringRef) CFDictionaryGetValue(addressDictRef, kDeviceIDKeyManufacturer );
		address->product		= (CFStringRef) CFDictionaryGetValue(addressDictRef, kDeviceIDKeyModel );
		address->serial			= (CFStringRef) CFDictionaryGetValue(addressDictRef, kDeviceIDKeySerialAbbrev );
		address->compatible		= NULL; //(CFStringRef) CFDictionaryGetValue(addressDictRef, kDeviceIDKeyCompatibleAbbrev );
		address->command		= NULL;

		if (address->manufacturer != NULL)
            CFRetain(address->manufacturer);
		if (address->product != NULL)
            CFRetain(address->product);
		if (address->compatible != NULL)
			CFRetain(address->compatible);
		if (address->serial != NULL)
			CFRetain(address->serial);
	}

	if (addressDictRef != NULL)
		CFRelease(addressDictRef);

	return err;
}

/*-----------------------------------------------------------------------------*

UsbSamePrinter

        Desc:	match two Usb printer address; return TRUE if they are the same.

        In:		a	the persistent address found last time
                b	the persistent address found this time

        Out:	non-zero iff the addresses are the same

*-----------------------------------------------------------------------------*/
int
UsbSamePrinter( const USBPrinterAddress *a, const USBPrinterAddress *b )
{
    int result = 0;
    DEBUG_CFCompareString( "UsbSamePrinter serial", a->serial, b->serial );
    DEBUG_CFCompareString( "UsbSamePrinter product", a->product, b->product );
    DEBUG_CFCompareString( "UsbSamePrinter manufacturer", a->manufacturer, b->manufacturer );

    result = !CompareSameString( a->serial, b->serial );
    if ( result )  result = !CompareSameString( a->product, b->product );
    if ( result ) result = !CompareSameString( a->manufacturer, b->manufacturer );

    return result;
}

/*-----------------------------------------------------------------------------*

UsbCreateExternalAddress

	Desc:	Construct a serialized persistant USB address using the run-time 
			address.
   
			A persistent address is one which doesn't depend on the order
			of bus enumeration, or placement of the device in the device
			tree.
			
			We use a triple retreived from the printer to identify it:
				the manufacturer		e.g., 'Canon', 'Hewlett-Packard'
				the product				e.g., 'Epson Stylus 800', 'DeskJet 970'
				the serial number 		<any ANSI 7-bit string>
 
			There are some inherent limitations -- if devices don't support
			serial numbers, we'll easily confuse identical printers from the
			same manufacturer.

	In:		address		the USB printer address we're using

	Out:	CF-Strigns	the persistent address


*-----------------------------------------------------------------------------*/
CFDataRef
UsbCreateExternalAddress( const USBPrinterAddress *address, Boolean fUseGeneric )	//bg
{
    CFDataRef				internalData = NULL;
	CFMutableStringRef		url;
	CFStringRef				scratch;
    CFMutableDictionaryRef	internal;

	if (address->manufacturer != NULL && address->product != NULL)
	{
		internal = CFDictionaryCreateMutable( kCFAllocatorDefault, 0,&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		if (internal != NULL)
		{
			CFDictionarySetValue(internal , kDeviceIDKeyManufacturer, address->manufacturer );
			CFDictionarySetValue(internal , kDeviceIDKeyModel, address->product );
			if (address->compatible != NULL)
				CFDictionarySetValue(internal , kDeviceIDKeyCompatibleAbbrev, address->compatible );
			if (address->serial != NULL)
				CFDictionarySetValue(internal , kDeviceIDKeySerialAbbrev, address->serial );
	
			// Since we eventually want addresses to be URLs create one and add it.
			if ((url = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, CFSTR("usb://"))) != NULL)
			{
				if ((scratch = CreateEncodedCFString(address->manufacturer)) != NULL)
				{
					CFStringAppend(url, scratch);
					CFRelease(scratch);

					if ((scratch = CreateEncodedCFString(address->product)) != NULL)
					{
						CFStringAppend(url, CFSTR(":"));
						CFStringAppend(url, scratch);
						CFRelease(scratch);

						if (address->serial != NULL && (scratch = CreateEncodedCFString(address->serial)) != NULL)
						{
							CFStringAppend(url, CFSTR("/"));
							CFStringAppend(url, scratch);
							CFRelease(scratch);
						}
                        DEBUG_CFString( "USBUtils: uri:", url);
						CFDictionarySetValue(internal, kPMPrinterURI, url);
						CFRelease(url);
#if PPD_SUPPORT						
						// if there is a PPD URL ref, add it to the addr dict
						if(fUseGeneric){
							CFDictionarySetValue(internal, kUseGenericPPD, kCFBooleanTrue);		//bg
						}else if(address->ppdURL != NULL) {		//bg
							CFDictionarySetValue(internal, kPMPPDNameKey, address->ppdURL);
						}
#endif
						internalData = CFPropertyListCreateXMLData(NULL, internal);
					}
				}
			}
			CFRelease(internal);
		}
	}

    return internalData;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Method:		UsbMakeFullUriAddress

	Input Parameters:

	Output Parameters:

	Description:
        Fill in missing address information

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CFStringRef
UsbMakeFullUriAddress( USBPrinterInfo *printer, CFStringRef manufacturer, CFStringRef product, CFStringRef serial )
{
    //
    //	fill in missing address information.
    //
    CFMutableStringRef printerUri = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, CFSTR("usb://") );
    if ( NULL != printerUri )
    {
        CFStringAppend(printerUri, NULL == manufacturer? printer->address.manufacturer: manufacturer );
        CFStringAppend(printerUri, CFSTR("/") );

        CFStringAppend(printerUri, NULL == product? printer->address.product: product );
	
        //Handle the case where there is no serial number (S450?)
        if (serial == NULL && printer->address.serial == NULL)
            serial = CFStringCreateWithFormat( NULL, NULL, CFSTR("%lx"), printer->location );

        CFStringAppend(printerUri, CFSTR("/") );
        CFStringAppend(printerUri, NULL == serial? printer->address.serial: serial );
    }
    
    return printerUri;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Method:		UsbGetAllPrinters

	Input Parameters:

	Output Parameters:
		array of all USB printers on the system

	Description:
        Build a list of USB printers by iterating IOKit USB objects

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CFMutableArrayRef
UsbGetAllPrinters( void )
{
    kern_return_t		kr;		// kernel errors
    mach_port_t			master_device_port = 0;
    io_service_t		usbInterface = 0;
    io_iterator_t 		iter = 0;
    CFMutableArrayRef	printers = CFArrayCreateMutable( NULL, 0, NULL );	// all printers

    do
    {

        kr = IOMasterPort( bootstrap_port, &master_device_port );
        DEBUG_ERR( kr, "UsbGetAllPrinters IOMasterPort %x\n" );
        if(kIOReturnSuccess != kr)  break;

        {
            CFDictionaryRef 	usbMatch = NULL;
            
            // iterate over all interfaces. 
            usbMatch = IOServiceMatching(kIOUSBInterfaceClassName);
            if ( !usbMatch ) break;
            DEBUG_ERR( kr, "UsbGetAllPrinters IOServiceMatching %x\n" );
        
            // IOServiceGetMatchingServices() consumes the usbMatch reference so we don't need to release it.
            kr = IOServiceGetMatchingServices(master_device_port, usbMatch, &iter);
            usbMatch = NULL;
            
            DEBUG_ERR( kr, "UsbGetAllPrinters IOServiceGetMatchingServices %x\n" );
            if(kIOReturnSuccess != kr || iter == NULL)  break;
        }
        
        while (  NULL != (usbInterface = IOIteratorNext(iter))  )
        {
            IOCFPlugInInterface 	**iodev;
            USBPrinterInterface		intf;
            HRESULT 				res;
            SInt32 					score;
            CFMutableDictionaryRef	properties;
            CFStringRef				classDriver = NULL;

            kr = IORegistryEntryCreateCFProperties( usbInterface, &properties, kCFAllocatorDefault, kNilOptions);
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
                
            DEBUG_ERR( kr, "UsbGetAllPrinters IOCreatePlugInInterfaceForService %x\n" );
            if ( kIOReturnSuccess == kr )
            {
                UInt8				intfClass = 0;
                UInt8				intfSubClass = 0;
 
                res = (*iodev)->QueryInterface( iodev, USB_INTERFACE_KIND, (LPVOID *) &intf);
                DEBUG_ERR( (kern_return_t) res, "UsbGetAllPrinters QueryInterface %x\n" );

               (*iodev)->Release(iodev);
                if ( noErr != res ) break;
 
                kr = (*intf)->GetInterfaceClass(intf, &intfClass);
                DEBUG_ERR(kr, "UsbGetAllPrinters GetInterfaceClass %x\n");
                if ( kIOReturnSuccess == kr )
                    kr = (*intf)->GetInterfaceSubClass(intf, &intfSubClass);
                DEBUG_ERR(kr, "UsbGetAllPrinters GetInterfaceSubClass %x\n");
                
                if ( kIOReturnSuccess == kr &&
                        kUSBPrintingClass == intfClass &&
                        kUSBPrintingSubclass == intfSubClass )
                {

                    USBPrinterInfo			printer,
                                            *printerInfo;
                    /*
                    For each type of printer specified in the lookup spec array, find
                    all of that type of printer and add the results to the list of found
                    printers.
                    */
                    // create this printer's persistent address
                    memset( &printer, 0, sizeof(USBPrinterInfo) );
                    kr = (*intf)->GetLocationID(intf, &printer.location);
                    DEBUG_ERR(kr, "UsbGetAllPrinters GetLocationID %x\n");
                    if ( kIOReturnSuccess == kr )
                    {
                        kr = UsbLoadClassDriver( &printer, kUSBPrinterClassInterfaceID, classDriver );
                        DEBUG_ERR(kr, "UsbGetAllPrinters UsbLoadClassDriver %x\n");
						if ( kIOReturnSuccess == kr && printer.classdriver )
						{
							(*(printer.classdriver))->interface = intf;
							kr = UsbGetPrinterAddress( &printer, &printer.address, 60000L );
							{ 
								// always unload the driver
								//	but don't mask last error
								kern_return_t unload_err = UsbUnloadClassDriver( &printer );
								if ( kIOReturnSuccess == kr )
									kr = unload_err;
							}
						}
                    }
                    
                    printerInfo = UsbCopyPrinter( &printer );
                    if ( NULL != printerInfo )
                        CFArrayAppendValue( printers, (const void *) printerInfo );		// keep track of it

                 } // if there's a printer
                kr = (*intf)->Release(intf);
            } // if IOCreatePlugInInterfaceForService
            
            IOObjectRelease(usbInterface);
            usbInterface = NULL;
            
        } // while there's an interface
    } while ( 0 );

    if (iter) 
    {
        IOObjectRelease(iter);
        iter = 0;
    }

    if (master_device_port) 
    {
        mach_port_deallocate(mach_task_self(), master_device_port);
        master_device_port = 0;
    }
    return printers;

} // UsbGetAllPrinters

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Method:		UsbReleasePrinter

	Input Parameters:

	Output Parameters:

	Description:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
UsbReleasePrinter( USBPrinterInfo *printer )
{
    if ( printer )
    {
		UsbUnloadClassDriver( printer );
        if ( NULL != printer->address.manufacturer )
            CFRelease( printer->address.manufacturer );
        if ( NULL != printer->address.product )
            CFRelease( printer->address.product );
        if ( NULL != printer->address.serial )
            CFRelease( printer->address.serial );
        if ( NULL != printer->address.command )
            CFRelease( printer->address.command );
        if ( NULL != printer->bundle )
            CFRelease( printer->bundle );
        free( printer );
   }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Method:		UsbReleaseAllPrinters

	Input Parameters:

	Output Parameters:

	Description:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
UsbReleaseAllPrinters( CFMutableArrayRef printers )
{
	if ( NULL != printers )
	{
		CFIndex i;
		for ( i = 0; i < CFArrayGetCount(printers); ++i ) 
			UsbReleasePrinter( (USBPrinterInfo *) CFArrayGetValueAtIndex( printers, i ) );
		CFRelease( printers );			
	}
}

USBPrinterInfo *
UsbCopyPrinter( USBPrinterInfo *aPrinter )
{
    //
    //	note this does not copy interface information, just address information
    //
    USBPrinterInfo *printerInfo = (USBPrinterInfo *) calloc( 1, sizeof(USBPrinterInfo));
    if ( NULL != printerInfo && NULL != aPrinter )
    {
        printerInfo->location = aPrinter->location;
        if ( NULL != (printerInfo->address.manufacturer = aPrinter->address.manufacturer) )
            CFRetain( printerInfo->address.manufacturer );
        if ( NULL != (printerInfo->address.product = aPrinter->address.product) )
            CFRetain( printerInfo->address.product );
        if ( NULL != (printerInfo->address.serial = aPrinter->address.serial) )
            CFRetain( printerInfo->address.serial );
        if ( NULL != (printerInfo->address.command = aPrinter->address.command) )
            CFRetain( printerInfo->address.command );
        if ( NULL != (printerInfo->bundle = aPrinter->bundle) )
            CFRetain( printerInfo->bundle );
    }
    
    return printerInfo;
}

/*-----------------------------------------------------------------------------*

        UsbRegistryOpen

        Desc:	opens the USB printer which matches the supplied printerAddress

        In:		myContext->printerAddress	persistent name which identifies the printer

        Out:	myContext->usbDeviceRef 	current IOKit address of this printer
*-----------------------------------------------------------------------------*/
kern_return_t
UsbRegistryOpen( USBPrinterAddress *usbAddress, USBPrinterInfo **result )
{
    kern_return_t		kr = -1;	// indeterminate failure
    CFMutableArrayRef	printers = UsbGetAllPrinters();
    CFIndex				numPrinters = NULL != printers? CFArrayGetCount( printers): 0;
    CFIndex				i;

    *result = NULL;	// nothing matched
    for ( i = 0; i < numPrinters; ++i )
    {
        USBPrinterInfo	*thisPrinter = (USBPrinterInfo *) CFArrayGetValueAtIndex( printers, i );
        if (  NULL != thisPrinter && UsbSamePrinter( usbAddress, &thisPrinter->address ) ) 
        {
            *result = UsbCopyPrinter( thisPrinter );	// retains reference
            if ( NULL != *result )
            {
                //
                //	if we can't find a bi-di interface, settle for a known uni-directional interface
                //
                USBPrinterClassContext **printer = NULL;
				//
                //	setup the default class driver
                //	If one is specified, allow the vendor driver to override our default implementation
				//
                kr = UsbLoadClassDriver( *result, kUSBPrinterClassInterfaceID, NULL );
				if ( kIOReturnSuccess == kr && (*result)->bundle )
					kr = UsbLoadClassDriver( *result, kUSBPrinterClassInterfaceID, (*result)->bundle );
                if ( kIOReturnSuccess == kr && NULL != (*result)->classdriver )
                {
                    printer = (*result)->classdriver;
                    kr = (*printer)->Open( printer, (*result)->location, kUSBPrintingProtocolBidirectional );
                    if ( kIOReturnSuccess != kr || NULL == (*printer)->interface )
                        kr = (*printer)->Open( printer, (*result)->location, kUSBPrintingProtocolUnidirectional );
                    //	it's possible kIOReturnSuccess == kr && NULL == (*printer)->interface
                    //		in the event that we can't open either Bidirectional or Unidirectional interface
                    if ( kIOReturnSuccess == kr )
                    {
                        if ( NULL == (*printer)->interface )
                        {
                            (*printer)->Close( printer );
                            UsbReleasePrinter( *result );
                            *result = NULL;
                        }
                    }
                }
            }
            break;
        }
    }
    UsbReleaseAllPrinters( printers ); // but, copied printer is retained
    DEBUG_ERR( kr, "UsbRegistryOpen return %x\n" );

    return kr;
}

Boolean StripEOJs( Ptr buffer, UInt32* nBytesP)		// jl131 bg862
/*	Modify buffer to remove x04- Control-D characters.
*/ 
{
	Ptr src, dst;
	SInt32	count;
    Boolean foundEOJ = false;
        
	src = dst = buffer;
	count = *nBytesP + 1;	// +1 for pre-decrement
    
	while (--count > 0)
	{
		if ((*dst++ = *src++) == '\x04')
		{
			dst--;			// Oops, we hit a control-D, backup a character & decrement num bytes
			(*nBytesP)--;
                        foundEOJ = true;
		}
	}
        
        return foundEOJ;
}

/*!
 * @function	CreateEncodedCFString
 *
 * @abstract	Create an encoded version of the string parameter 
 *				so that it can be included in a URI.
 *
 * @param	string	A CFStringRef of the string to be encoded.
 * @result	An encoded CFString.
 *
 * @discussion	This function will change all characters in string into URL acceptable format
 *				by encoding the text using the US-ASCII coded character set.  The following
 *				are invalid characters: the octets 00-1F, 7F, and 80-FF hex.  Also called out
 *				are the chars "<", ">", """, "#", "{", "}", "|", "\", "^", "~", "[", "]", "`".
 *				The reserved characters for URL syntax are also to be encoded: (so don't pass
 *				in a full URL here!) ";", "/", "?", ":", "@", "=", "%", and "&".
 */
static CFStringRef CreateEncodedCFString(CFStringRef string)
{
	CFStringRef result = NULL;
	char *bufferUTF8 = NULL;
	char *bufferEncoded = NULL;

	if (string != NULL)
	{
		CFIndex bufferSizeUTF8 = (3 * CFStringGetLength(string));
		if ((bufferUTF8 = (char*)malloc(bufferSizeUTF8)) != NULL)
		{
			CFStringGetCString(string, bufferUTF8, bufferSizeUTF8, kCFStringEncodingUTF8);
            {
                UInt16 bufferSizeEncoded = (3 * strlen(bufferUTF8)) + 1;
                if ((bufferEncoded = (char*)malloc(bufferSizeEncoded)) != NULL)
                {
                    Boolean textChanged;
                    NSLHexEncodeText(bufferUTF8, strlen(bufferUTF8), bufferEncoded, &bufferSizeEncoded, &textChanged);
                    bufferEncoded[bufferSizeEncoded] = '\0';
                    result = CFStringCreateWithCString(kCFAllocatorDefault, bufferEncoded, kCFStringEncodingUTF8);
                }
            }
		}
	}

	if (bufferUTF8)		free(bufferUTF8);
	if (bufferEncoded)	free(bufferEncoded);

	return result;
}

// eof

