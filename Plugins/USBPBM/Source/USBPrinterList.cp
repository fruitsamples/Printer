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
	File:		USBPrinterList.cp
	Contains:	Implementation of CPrinterList and CPrinter classes.
	
	Copyright (C) 2000-2002, Apple Computer, Inc., all rights reserved.

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
		
*******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Pragmas
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Includes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <CarbonCore/Folders.h>
#include <Print/PMPrinterBrowsers.h>
#include <PrintCore/PMPrinterModule.h>

#include "USBPrinterList.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Constants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*
Printer item status constants.
*/
const StatusValue	kPrinterNoStatus	=	0;	// No status.
const StatusValue	kPrinterDefault		=	1;	// Printer is default printer.
const StatusValue	kPrinterInWorkset	=	2;	// Printer in PrintCenter Workset.
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Type definitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Function prototypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Global variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Inlines
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 * The text and labels are localized using a strings file.
 * You can use the command genstrings to create a strings file from this source. Use:
 *	genstrings -s CopyLocalizedStringFromPlugin USBPrinterList.cp
 *
 * You may get some complaints from genstrings about this macro, but it will create the file.
 */
#define CopyLocalizedStringFromPlugin(key, comment, pluginBundleRef) \
    CFBundleCopyLocalizedString((pluginBundleRef), (key), (key), NULL)
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	local
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Private Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Public
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CPrinterList::CPrinterList

	Input Parameters:
			numSpecs	:	Number of lookup specification records.
		lookupSpecs	:	Array of lookup specs.
		
	Output Parameters:
		<none>
		
	Description:
		Default constructor.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CPrinterList::CPrinterList(
	UInt32					numSpecs,
	CFDictionaryRef*		lookupSpecs,
    CFBundleRef				bundleRef
    )
{
	fNumSpecs = numSpecs;
	fLookupSpecs = lookupSpecs;
    fBundleRef = bundleRef;
	
} // CPrinterList::CPrinterList

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CPrinterList::~CPrinterList

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Destructor.
		
		Deletes the printer list and all associated memory.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CPrinterList::~CPrinterList(void)
{
	fLookupSpecs = NULL;

} // CPrinterList::~CPrinterList

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CPrinterList::Populate

	Input Parameters:
		<none>
		
	Output Parameters:
		<none>
		
	Description:
		<purpose, data structures, alogrithms, etc.>
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
CPrinterList::Populate(void)
{
    CFMutableArrayRef	usbBusPrinters = UsbGetAllPrinters();
    CFIndex				i, numPrinters = NULL == usbBusPrinters? 0: CFArrayGetCount( usbBusPrinters );

    for ( i = 0; i < numPrinters; ++i )
    {
        USBPrinterInfo	*printer = (USBPrinterInfo *) CFArrayGetValueAtIndex( usbBusPrinters, i );
        if ( NULL != printer )
        {

            UInt32				curLookup;				// Index to current lookup spec.
            CFDataRef			iconData;				// Reference to icon family data.
            CFIndex				iconLen;				// Length of icon data.
            IconFamilyHandle	hFamily=NULL;			// Handle to icon family data.
            IconRef				iconRef;				// Registered icon reference.
            CFTypeRef			lookupRef;				// PrintCenter's lookup reference.
            CFStringRef			kindRef;				// PrinterModule's information for user.
            CFStringRef			deviceIDRef;			// PrinterModule's deviceID to match to printer
            Boolean				printerAdded = false;
        
            /*
            For each type of printer specified in the lookup spec array, find
            all of that type of printer and add the results to the list of found
            printers.
            */
            // create this printer's persistent address
            UsbGetPrinterAddress( printer, &printer->address, 60000L );
            for (curLookup = 0; curLookup < fNumSpecs; curLookup++)
            {
                /*
                Get the icon data from the look-up dictionary.
                The icon data are in the form of an icon family, so register
                this and get an IconRef to it.
                */
                iconData = (CFDataRef) ::CFDictionaryGetValue(fLookupSpecs[curLookup],
                                                            kPMPrinterBrowserIconsKey);
                if (NULL != iconData)
                {
                    iconLen = ::CFDataGetLength(iconData);
                    hFamily = (IconFamilyHandle) ::NewHandle(iconLen);
                    
                    ::HLock((Handle) hFamily);
                    {
                        ::CFDataGetBytes(iconData, ::CFRangeMake(0, iconLen),
                                        (UInt8*) (*hFamily));
                    }
                    ::HUnlock((Handle) hFamily);
                    
                    (void) ::RegisterIconRefFromIconFamily('pbus', (OSType) curLookup,
                                                            hFamily, &iconRef);
                    if( NULL!= hFamily )
                        ::DisposeHandle( (Handle) hFamily );
                }
                
                deviceIDRef = (CFStringRef) ::CFDictionaryGetValue(fLookupSpecs[curLookup],
                                                            kPMPrinterBrowserDeviceIDKey);
                if ( NULL == deviceIDRef || !DeviceIDSupportedLookup( deviceIDRef ) )
                    continue;
        
                /*
                Grab the lookup reference that we have to pass back to PrintCenter
                when the printer is selected.
                */
                lookupRef = (CFTypeRef) ::CFDictionaryGetValue(fLookupSpecs[curLookup],
                                                        kPMPrBrowserLookupRefKey);
                kindRef = (CFStringRef) ::CFDictionaryGetValue(fLookupSpecs[curLookup],
                                                        kPMPrinterBrowserKindKey);

                if ( UsbSupportedPrinter( &printer->address, deviceIDRef ) )
                {
                    this->AddPrinterToList(kPrinterNoStatus, iconRef, 
                                &printer->address, kindRef, lookupRef);
                    printerAdded = true;
                }
                
            }	// For each lookup spec.
            if ( !printerAdded )
            {
                //
                // This printer doesn't have an associated printer module.
                // Show the printer in the list so that the user knows we can see it, even
                //	if we can't print to it.
                //
                // 	Use the "Unknown printer" icon. If that icon has not been registered, 
                //	just ignore the icon.
                //
                //	Elsewhere in this PBM, NULL lookupRef is checked to prevent Print Center
                //	adding the printer to the workset, and the user from selecting tihs printer
                //	while our browser is active.
                IconRef		unknownPrinterIconRef = NULL;		// Use Print Center's registered icon
                OSStatus	err = ::GetIconRef(kOnSystemDisk, 'pctr', kPMPrBrowserUnknownPrinterIconType, &unknownPrinterIconRef);
                CFStringRef	unsupportedKind = CopyLocalizedStringFromPlugin(
                            CFSTR("Unsupported printer"),
                            CFSTR("Kind for printers which are unsupported"),
                            fBundleRef);
                
                if ( noErr != err ) unknownPrinterIconRef = NULL;	// if err, don't use an icon

                this->AddPrinterToList(kPrinterNoStatus, unknownPrinterIconRef, &printer->address, unsupportedKind, NULL/* lookupRef */ );
            }
        } // if there's a printer
	} // for all printers
    UsbReleaseAllPrinters( usbBusPrinters );


} // CPrinterList::Populate

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Protected
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Private
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CPrinterList::AddPrinterToList

	Input Parameters:
		status		:		Initial printer status.
		icon		:		IconRef for printer's icon.
		addr		:		CFStringRef to printer's AppleTalk address.
		lookupRef	:		PrintCenter lookup reference for printer.
		
	Output Parameters:
		<method>	:		Pointer to new printer.
		
	Description:
		Creates a new printer and adds it to the list of printers.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CPrinter*
CPrinterList::AddPrinterToList(
	StatusValue			status,
	IconRef				icon,
	USBPrinterAddress		*addr,
        CFStringRef			kindRef,
	CFTypeRef			lookupRef)
{
	CPrinter	*newPrinter;		// Pointer to new printer object.
	
	newPrinter = new CPrinter(status, icon, addr->product, kindRef, addr, lookupRef);
	if (NULL != newPrinter)
		this->AddItemToList(newPrinter);
	
	return newPrinter;

}	// CPrinterList::AddItemToList				 


// End of "PrinterList.cp"


