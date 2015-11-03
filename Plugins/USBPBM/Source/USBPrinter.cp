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
	File:		USBPrinter.cp
	Contains: 	Implementation of CPrinter class.

	Description:
		CPrinter encapsulates information about an AppleTalk printer.

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
#define __USB__ 1
#include <ApplicationServices/ApplicationServices.h>
//#include <HIToolbox/Icons.h>
#include <CarbonCore/MacMemory.h>
#undef __USB__

#include <Print/PMPrinterBrowsers.h>
#include "USBPrinterList.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Constants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

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

/******************************************************************************/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Public
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CPrinter::CPrinter

	Input Parameters:
		status		:	Status of item in browser view.
		icon		:	Icon reference for printer icon.
		name		:	CFStringRef to printer's name string.
		type		:	CFStringRef to printer's model string.
		addr		:	Printer's USB address data.
		lookupRef	:	PrintCenter lookup reference for printer.
		
	Output Parameters:
		<none>
		
	Description:
		Constructor.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CPrinter::CPrinter(
	StatusValue			status,
	IconRef				icon,
	CFStringRef			name,
	CFStringRef			type,
	USBPrinterAddress	*addr,
	CFTypeRef			lookupRef)
	
:	CItem(status, name, type, icon)
{
	fInWorkset = false;
	
    /*
    Increment the reference counts for the appropriate data.
    */
    fAddr = *addr;
    if ( NULL != fAddr.serial ) CFRetain( fAddr.serial );
    if ( NULL != fAddr.product ) CFRetain( fAddr.product );
    if ( NULL != fAddr.manufacturer ) CFRetain( fAddr.manufacturer );
    if ( NULL != fAddr.command ) CFRetain( fAddr.command );

 			
    fLookupRef = lookupRef;
    if (NULL != fLookupRef) CFRetain(fLookupRef);

} // CPrinter::CPrinter

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CPrinter::~CPrinter

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Destructor.
				
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CPrinter::~CPrinter(void)
{
 	if (NULL != fLookupRef)
	{
		::CFRelease(fLookupRef);
		fLookupRef = NULL;
	}
    UsbAddressDispose( &fAddr );
		
} // CPrinter::~CPrinter


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CPrinter::CopyAddr

	Input Parameters:
		<none>
		
	Output Parameters:
		<none>
		
	Description:
		Returns printer address as a CFDataRef.
				
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CFDataRef
CPrinter::CopyAddr(void)
{
	/*
	Get the USB address and convert it to a CFDataRef.
	*/
	return (::UsbCreateExternalAddress( &fAddr, true/*fUseGeneric PPD*/ ));

}	// CPrinter::CopyAddr


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CPrinter::PrinterID

	Input Parameters:
		<none>
		
	Output Parameters:
		<method>	:	Printer selection specification dictionary.
		
	Description:
		Constructs a new printer selection specification dictionary
		from the printer's internal data and returns a reference to it.
				
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CFDictionaryRef
CPrinter::PrinterID(void)
{
	CFMutableDictionaryRef	idDict = NULL;		// Printer selection spec.
	CFDataRef				addrData;	// Printer address info.
	
	/*
	Allocate a new mutable dictionary and populate it with the required
	data references.
	*/
    if ( NULL != fLookupRef )
    {
        idDict = ::CFDictionaryCreateMutable( kCFAllocatorDefault, 0,&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        
        ::CFDictionarySetValue(idDict, kPMPrBrowserSelectNameKey, this->Name());
        ::CFDictionarySetValue(idDict, kPMPrBrowserSelectKindKey, this->Kind());
        
        /*
        Convert the address to a CFData.
        */
        addrData = this->CopyAddr();
        ::CFDictionarySetValue(idDict, kPMPrBrowserSelectAddrKey, addrData);
        ::CFRelease(addrData);
    
        /*
        Store PrintCenter's lookup reference. Note that although the data are
        the same as in the lookup spec, the name of the key in the selection
        spec is different
        */
        ::CFDictionarySetValue(idDict, kPMPrBrowserSelectRefKey, fLookupRef);
    }
    
	return ((CFDictionaryRef) idDict);
		
} // CPrinter::PrinterID

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Protected
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Private
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Private Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

// End of "CPrinter.cp"
