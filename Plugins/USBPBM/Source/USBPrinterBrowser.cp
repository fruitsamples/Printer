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
	File:		USBPrinterBrowser.cp
	Contains: 	Implementation of simple USB Printer Browser module.
	
	Description:
		Implementation of USB Printer Browser module.
		
		This module is compiled as a shared library, which is then loaded by
		PrintCenter when the Printer Browser window is opened. 
		
		USBPrinterBrowser draws its controls in the Printer Browser window
		supplied by PrintCenter.

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
#include <HIToolbox/Appearance.h>
#include <ApplicationServices/ApplicationServices.h>
#include <HIToolbox/ControlDefinitions.h>
#include <HIToolbox/Controls.h>

#undef __USB__

#include <PrintCore/PMIOModule.h>
#include "USBPrinterBrowser.h"
#include "USBBrowserListView.h"

/*
**	this constant is owned by Apple. It will move into the headers in the future.
*/
#define kPMUSBBrowserBundleID                  CFSTR("com.pmvendor.print.pbm.USB")
/*
 * You can use the command genstrings to create a strings file from this source. Use:
 *	genstrings -s CopyLocalizedStringFromPlugin USBPrinterBrowser.cp
 *
 * You may get some complaints from genstrings about this macro, but it will create the file.
 */
#define CopyLocalizedStringFromPlugin(key, comment, pluginBundleRef) \
    CFBundleCopyLocalizedString((pluginBundleRef), (key), (key), NULL)

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Constants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
Control and view...

Dimensions of view content requested...
*/
#define		kMyViewMaxH			44
#define		kMyViewMaxV			24

/*
Create CFUUIDRef constants from the UUID strings...
*/
const CFUUIDRef		kPBMTypeID		= ::CFUUIDCreateFromString(::CFAllocatorGetDefault(),
										CFSTR(kPMPrBrowserPlugInType));
const CFUUIDRef		kUSBPBFactoryID	= ::CFUUIDCreateFromString(::CFAllocatorGetDefault(),
										CFSTR("651BD228-AB65-11D4-AC04-0030656DE360")); //CustomInfo.plist
const CFUUIDRef		kAPIVersionID	= ::CFUUIDCreateFromString(::CFAllocatorGetDefault(),
										CFSTR(kPMInterfaceAPIVersion));
const CFUUIDRef		kPrBrowserID	= ::CFUUIDCreateFromString(::CFAllocatorGetDefault(),
										CFSTR(kPMInterfacePrBrowser));
									
/*
USB Printer Browser Module bundle identifier.
*/
const CFStringRef	kUSBPBBundleID	= kPMUSBBrowserBundleID;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Type definitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
USB Printer Browser Module Type, returned by CFPlugIn Factory function. 

The USBPBType is essentially a C implementation of a COM-compliant C++ class
which inherits two sets of functions. Eventually, when the compilers on OS X
can actually generate COM-compliant code, a lot of this won't be necessary.

Note that an Instance of a Type has a reference counter. We use a single counter
for the Instance, rather than a separate one for each Interface.
*/
struct USBPBType
{
	PMInterfaceAPIVersionPtr	apiInterface;	// APIVersion() Interface.
	PMInterfacePrBrowserPtr		pbmInterface;	// General PBM Interface.
	
	CFUUIDRef					myFactory;		// ID of generating Factory.
	ULONG						refCount;		// Reference count for Instance.
};

typedef struct USBPBType	USBPBType;
typedef USBPBType		*USBPBTypePtr;
typedef USBPBType		**USBPBTypeHdl;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Function prototypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#pragma export on

#if __cplusplus
extern "C" {
#endif

/*
CFPlugIn functions. USBPBFactory() is exported directly. The others are
referenced through the IUnknownVTbl structure contained in the
PMTypePrBrowser structure, a pointer to which is returned as an Instance 
by USBPBFactory().

First, the Factory, which returns an Instance of the Type.
*/
void*		USBPBFactory						(
				CFAllocatorRef				allocator, 
				CFUUIDRef					typeID);

/*
The "base" implementations of the IUnknown API.
*/
HRESULT		USBPBQuery						(
				USBPBTypePtr					instance, 
				REFIID						iid, 
				LPVOID*						ppv);
ULONG		USBPBAddRef						(
				USBPBTypePtr					instance);
ULONG		USBPBRelease						(
				USBPBTypePtr					instance);

/*
Interface-specific implemntations of IUnknown API...
*/
HRESULT		USBPB_APIInterfaceQuery			(
				void*						thisPointer, 
				REFIID						iid, 
				LPVOID*						ppv);
ULONG		USBPB_APIInterfaceAddRef		(
				void*						thisPointer);
ULONG		USBPB_APIInterfaceRelease		(
				void*						thisPointer);

HRESULT		USBPB_PBMInterfaceQuery			(
				void*						thisPointer, 
				REFIID						iid, 
				LPVOID*						ppv);
ULONG		USBPB_PBMInterfaceAddRef		(
				void*						thisPointer);
ULONG		USBPB_PBMInterfaceRelease		(
				void*						thisPointer);

/*
The following define the Printer Browser Module API...

Required entry points...
*/
UInt32		APIVersion						(void);
OSStatus	GetSelectedPrinters				(
				PMPrBrowserContext			context, 
				CFArrayRef*					printers);
OSStatus	Initialize						( 
				PMPrBrowserContext			context, 
				PMPrBrowserRef				ref, 
				PMPrBrowserCallbacksPtr		callbacks, 
				ControlRef					pbUserPaneCtlHdl,
				UInt32						numLookupSpecs);
OSStatus	Prologue						( 
				PMPrBrowserContext*			context, 
				PMPrBrowserFlags			pcFlags,
				CFStringRef*				title,
				UInt32*						minH, 
				UInt32*						minV,
				UInt32*						maxH, 
				UInt32*						maxV);
OSStatus	Resize							( 
				PMPrBrowserContext			context, 
				const Rect*					frameRect);
OSStatus	Sync							( 
				PMPrBrowserContext			context);
OSStatus	Terminate						( 
				PMPrBrowserContext			context, 
				OSStatus					status);
OSStatus	WorksetPrinters					(
				PMPrBrowserContext			context,
				CFArrayRef					printers);

#if __cplusplus
}
#endif

#pragma export off

/*
Global initialization function.
*/
static OSStatus		InitContext							(
						PMPrBrowserFlags			pcFlags,
						USBPrinterBrowserContextPtr*	context);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Global variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Inlines
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Public
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	USBPBFactory

	Input Parameters:
		allocator		:	Reference to CF allocator.
		typeID			:	Type of CFPlugIn to be allocated by Factory.
		
	Output Parameters:
		<function>		:	Pointer to Instance of Type.

	Description:
		If the requested Type is that of a Printer Browser Module, the
		function returns a pointer to an Instance of that Type. Otherwise,
		the function returns NULL.
		
		The Instance can be used by the caller to obtain an Interface, which
		is a table of function pointers to the exported Printer Browser
		Module API.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void*
USBPBFactory(
	CFAllocatorRef		allocator, 
	CFUUIDRef			typeID) 
{
    USBPBType*		instance;	// Pointer to Instance.

	/*
	Check whether the type specified is one supported by this Factory.
	*/
    if (::CFEqual(typeID, kPBMTypeID))
    {
	   	/*
    	Allocate an Instance of the Printer Browser Module Type.
    	*/
    	instance = new USBPBType;
    	
    	instance->apiInterface = new PMInterfaceAPIVersion;
    	instance->pbmInterface = new PMInterfacePrBrowser;
    	
    	/*
    	Set up the IUnknown function pointers in each Interface.
    	*/
    	instance->apiInterface->u.QueryInterface = USBPB_APIInterfaceQuery;
    	instance->apiInterface->u.AddRef = USBPB_APIInterfaceAddRef;
    	instance->apiInterface->u.Release = USBPB_APIInterfaceRelease;
    	
    	instance->pbmInterface->u.QueryInterface = USBPB_PBMInterfaceQuery;
    	instance->pbmInterface->u.AddRef = USBPB_PBMInterfaceAddRef;
    	instance->pbmInterface->u.Release = USBPB_PBMInterfaceRelease;
    	
    	/*
    	Record the function pointers for the various supported
    	Interfaces...
    	*/
    	instance->apiInterface->apiVersion = APIVersion;
    	
		instance->pbmInterface->getSelectedPrinters = GetSelectedPrinters;
		instance->pbmInterface->initialize = Initialize;
		instance->pbmInterface->prologue = Prologue;
		instance->pbmInterface->resize = Resize;
		instance->pbmInterface->sync = Sync;
		instance->pbmInterface->terminate = Terminate;
		instance->pbmInterface->worksetPrinters = WorksetPrinters;
    	
    	/*
    	Record the Factory ID and set the refCount to 1.
    	*/
    	instance->myFactory = kUSBPBFactoryID;
    	instance->refCount = 1;
    }
    else
        instance = NULL;
    
     return (instance);
    
}	// USBPBFactory


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	USBPBQuery

	Input Parameters:
		instance		:	Pointer to Instance.
		
	Output Parameters:
		<function>		:	Printer Browser Module API version.

	Description:
		Returns the latest version of the Printer Browser Module API supported 
		by this module.
		
		It is assumed that thisPointer is not NULL.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
HRESULT
USBPBQuery(
	USBPBTypePtr	instance, 
	REFIID		iid, 
	LPVOID*		ppv)
{
    CFUUIDRef		interfaceID;	// Specifies type of Interface to instantiate.
    HRESULT		result;			// Result code.
    
    /*
    Assume the result is OK unless proven otherwise.
    */
    result = S_OK;
    
    /*
    Get the CFUUID from the ID data.
    */
    interfaceID = ::CFUUIDCreateFromUUIDBytes(::CFAllocatorGetDefault(), iid);

    if (::CFEqual(interfaceID, IUnknownUUID) ||
		::CFEqual(interfaceID, kAPIVersionID))
    {
    	/*
    	Return an Interface for the APIVersion() function.
    	(If an IUnknown API is requested, the result is the same, since
    	the first functions in the APIVersion() Interface are the
    	IUnknown functions.)
    	*/
        (void) USBPBAddRef(instance);
        *ppv = instance->apiInterface;
    } 
    else if (::CFEqual(interfaceID, kPrBrowserID)) 
    {
     	/*
    	Return an Interface for the Printer Browser Module API.
    	*/
        (void) USBPBAddRef(instance);
        *ppv = instance->pbmInterface;
    } 
    else 
    {
        *ppv = NULL;
	result = E_NOINTERFACE;
    }

	return (result);

}	// USBPBQuery


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	USBPBAddRef

	Input Parameters:
		instance		:	Pointer to Instance.
		
	Output Parameters:
		<function>		:	New reference count.

	Description:
		Increments the reference counter associated with the given Interface
		and returns the new reference count.
		
		It is assumed that thisPointer is not NULL.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
ULONG
USBPBAddRef(
	USBPBTypePtr	instance)
{
	return (++(instance->refCount));

}	// USBPBAddRef


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	USBPBRelease

	Input Parameters:
		instance		:	Pointer to Instance.
		
	Output Parameters:
		<function>		:	New reference count.

	Description:
		Decrements the reference counter associated with the given Interface
		and returns the new reference count.
		
		If the reference count reaches 0, the Interface is deallocated.
		
		It is assumed that thisPointer is not NULL.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
ULONG
USBPBRelease(
	USBPBTypePtr	instance)
{
	ULONG		count;		// New reference count.
	
	count = --(instance->refCount);
	
	/*
	If the refCount hits 0, delete the Instance.
	*/
	if (0 == count)
	{
		delete instance->apiInterface;
		delete instance->pbmInterface;
		delete instance;
	}
	
	return (count);

}	// USBPBRelease


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	USBPB_APIInterfaceQuery

	Input Parameters:
		thisPointer		:	Pointer to Interface.
		
	Output Parameters:
		<function>		:	New reference count.

	Description:
		Calculates the location of the Instance and calls USBPBAddRef().
		
		If thisPointer or ppv is NULL, no action is taken E_INVALIDARG is
		returned.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
HRESULT
USBPB_APIInterfaceQuery(
	void*		thisPointer, 
	REFIID		iid, 
	LPVOID*		ppv)
{
	HRESULT			result;		// Result code.
	
	if ((NULL != thisPointer) && (NULL != ppv))
		result = USBPBQuery((USBPBTypePtr) thisPointer, iid, ppv);
	else
		result = E_INVALIDARG;
	
	return (result);

}	// USBPB_APIInterfaceQuery


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	USBPB_APIInterfaceAddRef

	Input Parameters:
		thisPointer		:	Pointer to Interface.
		
	Output Parameters:
		<function>		:	New reference count.

	Description:
		Calculates the location of the Instance and calls USBPBAddRef().
		
		If thisPointer is NULL, no action is taken and 0 is returned.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
ULONG
USBPB_APIInterfaceAddRef(
	void*	thisPointer)
{
	ULONG		count;		// New reference count.
	
	if (NULL != thisPointer)
		count = USBPBAddRef((USBPBTypePtr) thisPointer);
	else
		count = 0;
	
	return (count);

}	// USBPB_APIInterfaceAddRef


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	USBPB_APIInterfaceRelease

	Input Parameters:
		thisPointer		:	Pointer to Interface.
		
	Output Parameters:
		<function>		:	New reference count.

	Description:
		Calculates the location of the Instance and calls USBPBRelease().
		
		If thisPointer is NULL, no action is taken and 0 is returned.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
ULONG
USBPB_APIInterfaceRelease(
	void*	thisPointer)
{
	ULONG		count;		// New reference count.
	
	if (NULL != thisPointer)
		count = USBPBRelease((USBPBTypePtr) thisPointer);
	else
		count = 0;
	
	return (count);

}	// USBPB_APIInterfaceRelease


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	USBPB_PBMInterfaceQuery

	Input Parameters:
		thisPointer		:	Pointer to Interface.
		
	Output Parameters:
		<function>		:	New reference count.

	Description:
		Calculates the location of the Instance and calls USBPBAddRef().
		
		If thisPointer or ppv is NULL, no action is taken E_INVALIDARG is
		returned.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
HRESULT
USBPB_PBMInterfaceQuery(
	void*		thisPointer, 
	REFIID		iid, 
	LPVOID*		ppv)
{
	PMInterfacePrBrowserPtr*	thisPtr;	// Typecast thisPointer.
	HRESULT						result;		// Result code.
	
	if ((NULL != thisPointer) && (NULL != ppv))
	{
		thisPtr = (PMInterfacePrBrowserPtr*) thisPointer;
		result = USBPBQuery((USBPBTypePtr) (thisPtr - 1), iid, ppv);
	}
	else
		result = E_INVALIDARG;
	
	return (result);

}	// USBPB_PBMInterfaceQuery


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	USBPB_PBMInterfaceAddRef

	Input Parameters:
		thisPointer		:	Pointer to Interface.
		
	Output Parameters:
		<function>		:	New reference count.

	Description:
		Calculates the location of the Instance and calls USBPBAddRef().
		
		If thisPointer is NULL, no action is taken and 0 is returned.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
ULONG
USBPB_PBMInterfaceAddRef(
	void*	thisPointer)
{
	PMInterfacePrBrowserPtr*	thisPtr;	// Typecast thisPointer.
	ULONG						count;		// New reference count.
	
	if (NULL != thisPointer)
	{
		thisPtr = (PMInterfacePrBrowserPtr*) thisPointer;
		count = USBPBAddRef((USBPBTypePtr) (thisPtr - 1));
	}
	else
		count = 0;
	
	return (count);

}	// USBPB_PBMInterfaceAddRef


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	USBPB_PBMInterfaceRelease

	Input Parameters:
		thisPointer		:	Pointer to Interface.
		
	Output Parameters:
		<function>		:	New reference count.

	Description:
		Calculates the location of the Instance and calls USBPBRelease().
		
		If thisPointer is NULL, no action is taken and 0 is returned.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
ULONG
USBPB_PBMInterfaceRelease(
	void*	thisPointer)
{
	PMInterfacePrBrowserPtr*	thisPtr;	// Typecast thisPointer.
	ULONG						count;		// New reference count.
	
	if (NULL != thisPointer)
	{
		thisPtr = (PMInterfacePrBrowserPtr*) thisPointer;	
		count = USBPBRelease((USBPBTypePtr) (thisPtr - 1));
	}
	else
		count = 0;
	
	return (count);

}	// USBPB_PBMInterfaceRelease


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	APIVersion

	Input Parameters:
		<none>
		
	Output Parameters:
		<function>		:	Printer Browser Module API version.

	Description:
		Returns the latest version of the Printer Browser Module API supported 
		by this module.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
UInt32
APIVersion(void)
{
	return (kPMPrBrowserAPIVersion);
	
}	// APIVersion


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	Prologue

	Input Parameters:
		mySpec			:	This shared library's filespec.
		
	Output Parameters:
		maxH			:	Maximum horizontal dimension required by module.
		maxV			:	Maximum vertical dimension required by module.
		<function>		:	Status code.

	Description:
		Performs preliminary initialization. Allocates the module's global
		context, returns dimensions of content region desired by the client.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
OSStatus
Prologue( 
	PMPrBrowserContext*				context, 
	PMPrBrowserFlags				pcFlags,
	CFStringRef*					title,
	UInt32*							minH, 
	UInt32*							minV,
	UInt32*							maxH, 
	UInt32*							maxV )
{
	SInt16						origResFile;// Original resource file.
	USBPrinterBrowserContextPtr	myContext;	// Pointer to global data block.
	OSStatus					err;		// Error condition.

	/*
	Allocate a global data block.
	*/
	err = InitContext(pcFlags, &myContext);
	
	if (noErr == err)
	{
		/*
		Save the global data pointer in the PMPrBrowserContext, so we can
		get at it when we're called.
		*/
		*context = (PMPrBrowserContext) myContext;
		
		/*
		Save the current resource file and use ours.
		*/
		origResFile = ::CurResFile();
		::UseResFile(myContext->myResFile);
		
		/*
		Return the UI display string for the printer browser module
		*/
        *title = CopyLocalizedStringFromPlugin(
                CFSTR("SDK_USBPBM"),
                CFSTR("The popup menu item to show the printer browser connection type"),
                myContext->usbpbBundle);
		
		/*
		Set the desired horizontal and vertical dimensions for the
		Browser view.
		
		*/
		*minH = kMyViewMaxH;
		*minV = kMyViewMaxV;
		*maxH = kMyViewMaxH;
		*maxV = kMyViewMaxV;
		
		/*
		Restore the original resource file.
		*/
		::UseResFile(origResFile);
	}

	return (err);

}	// Prologue

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	Initialize

	Input Parameters:
		context			:	Module's context.
		ref			:	Printer Browser reference from PrintCenter.
		callbacks		:	Pointer to callback pointer block.
		pbWindow		:	Reference to window containing Printer Browser view.
		numLookupSpecs		:	Number of printer lookup specification records.
		
	Output Parameters:
		flags			:	Capability & behavior flags.
		eventMask		:	Desired events, if any (use if Event() call requested).
		<function>		:	Status code.

	Description:
		Initializes client interface.
		
		Note that the callback block is allocated by PrintCenter and is not
		to be deleted or modified.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
OSStatus
Initialize(
    PMPrBrowserContext              context,
    PMPrBrowserRef                  ref,
    PMPrBrowserCallbacksPtr         callbacks,
	ControlRef						pbUserPaneCtlHdl,
    UInt32                          numLookupSpecs)
{
	#pragma	unused(eventMask)
	USBPrinterBrowserContextPtr	myContext;		// Typecast pointer to context.
	SInt16				origResFile;	// Original resource file.
	OSStatus					err;			// Error condition.
	
	err = noErr;
	
	myContext = (USBPrinterBrowserContextPtr) context;
	
	/*
	Save the current resource file and use ours.
	*/
	origResFile = ::CurResFile();
	::UseResFile(myContext->myResFile);
	
	if ((myContext->pcFlags & kPMPrBrowserPCNoUI) == 0)
	{
		/*
		Instantiate and initialize the browser view.
		*/
		myContext->browserView = new CBrowserListView(pbUserPaneCtlHdl, myContext->usbpbBundle, ref, callbacks, numLookupSpecs);
	}
	else
	{
		/*
		PrintCenter is loading the printer browser module without a user
		interface. Just initialize the printer lists and stand by.
		*/
		myContext->browserView = new CBrowserListView(myContext->usbpbBundle, ref, callbacks, numLookupSpecs);
	}

	/*
	Instantiate and initialize the browser view.
	*/
//	myContext->browserView = new CBrowserListView(pbWindow, *frameRect, ref,
//                                               callbacks, numLookupSpecs, &(*flags));
		
	/*
	Restore the original resource file.
	*/
	::UseResFile(origResFile);

	return (noErr);

}	// Initialize

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	Resize

	Input Parameters:
		frameRect	:	New content region bounding rectangle.
		context		:	Module's context.
		
	Output Parameters:
		<function>	:	Status code.

	Description:
		Notifies Printer Browser module that the content bounding rectangle
		has changed (generally because the user has resized the window).
		Resize the controls within the content appropriately.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
OSStatus
Resize(
	PMPrBrowserContext				context,
	const Rect*					frameRect)
{
	SInt16		origResFile;	// Original resource file.
	OSStatus	err;			// Error condition.
	
	err = noErr;
	
	if ((((USBPrinterBrowserContextPtr) context)->pcFlags & kPMPrBrowserPCNoUI) == 0)
	{		
		/*
		Save the current resource file and use ours.
		*/
		origResFile = ::CurResFile();
		::UseResFile(((USBPrinterBrowserContextPtr) context)->myResFile);
			
		/*
		Tell the browser view to resize itself.
		*/
		((USBPrinterBrowserContextPtr) context)->browserView->Resize(*frameRect);
		
		/*
		Restore the original resource file.
		*/
		::UseResFile(origResFile);
	}
	else
		err = kPMPrBrowserNoUI;
	
	return (err);

}	// Resize

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	Sync

	Input Parameters:
		context		:	Module's context.
		
	Output Parameters:
		<function>	:	Status code.

	Description:
		Called following a request from the Printer Browser module for
		data synchronization.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
OSStatus
Sync(
	PMPrBrowserContext			context)
{
	SInt16		origResFile;	// Original resource file.
	
	/*
	Save the current resource file and use ours.
	*/
	origResFile = ::CurResFile();
	::UseResFile(((USBPrinterBrowserContextPtr) context)->myResFile);
	
	/*
	Restore the original resource file.
	*/
	::UseResFile(origResFile);

	return (noErr);

}	// Sync

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	GetSelectedPrinters

	Input Parameters:
		context		:	Module's context.
		
	Output Parameters:
		info		:	Information about a selected printer.
		<function>	:	Status code.

	Description:
		Called after PrintCenter is notified that one or more printers have
		been selected.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
OSStatus
GetSelectedPrinters(
	PMPrBrowserContext			context,
	CFArrayRef*					printers)
{
	USBPrinterBrowserContextPtr	ctxt;		// Typecast pointer to the context.
	SInt16						origResFile;	// Original resource file.
	OSStatus					err;		// Error condition.
	
	ctxt = (USBPrinterBrowserContextPtr) context;

	/*
	Save the current resource file and use ours.
	*/
	origResFile = ::CurResFile();
	::UseResFile(ctxt->myResFile);
	
	/*
	Get information about the next selected printer from the browser view.
	*/
	err = ctxt->browserView->GetSelectedPrinters(&(*printers));
	
	/*
	Restore the original resource file.
	*/
	::UseResFile(origResFile);

	return (err);

}	// GetSelectedPrinters

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	Terminate

	Input Parameters:
		context		:	Module's context.
		status		:	Termination error condition.
		
	Output Parameters:
		<function>	:	Status code.

	Description:
		Called when PrintCenter is closing the Printer Browser module.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
OSStatus
Terminate(
	PMPrBrowserContext				context,
	OSStatus					status)
{
	#pragma	unused(status)
	
	USBPrinterBrowserContextPtr	myContext;		// Typecast pointer to context.
	SInt16				origResFile;	// Original resource file.
	
	myContext = (USBPrinterBrowserContextPtr) context;
	
	/*
	Save the current resource file and use ours.
	*/
	origResFile = ::CurResFile();
	if (-1 != myContext->myResFile)
		::UseResFile(myContext->myResFile);
	
	/*
	Delete the browser view.
	*/
	if (NULL != myContext->browserView)
		delete myContext->browserView;
		
	/*
	Restore the original resource file.
	*/
	::UseResFile(origResFile);
	
	/*
	Close the resource fork.
	*/
	if (NULL != myContext->usbpbBundle)
		//::_CFBundleCloseBundleResourceFork(myContext->usbpbBundle);
		::CFBundleCloseBundleResourceMap(myContext->usbpbBundle, myContext->myResFile);

        /*
	Free the global context.
	*/
	if (NULL != context)
		::DisposePtr((Ptr) myContext);
		
	return (noErr);

}	// Terminate

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	WorksetPrinters

	Input Parameters:
		context		:	Module's context.
		printers	:	Array of printers in Workset.
		
	Output Parameters:
		<function>	:	Status code.

	Description:
		Called by PrintCenter to supply the printer browser module with
		a list of compatible printers already in the Workset.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
OSStatus
WorksetPrinters(
	PMPrBrowserContext				context,
	CFArrayRef						printers)
{
	SInt16			origResFile;	// Original resource file.
	OSStatus		err;			// Error condition.
	
	/*
	Save the current resource file and use ours.
	*/
	origResFile = ::CurResFile();
	::UseResFile(((USBPrinterBrowserContextPtr) context)->myResFile);
	
	/*
	Pass the list of printers to the browser view.
	*/
	err = ((USBPrinterBrowserContextPtr) context)->browserView->WorksetPrinters(printers);
		
	/*
	Restore the original resource file.
	*/
	::UseResFile(origResFile);

	return (err);

}	// WorksetPrinters

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Private Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	InitContext

	Input Parameters:
		mySpec		:	This module's filespec.
		
	Output Parameters:
		context		:	Pointer to global context.
		<function>	:	Status code.

	Description:
		Allocates and initializes the global data block for this module.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static
OSStatus
InitContext(
	PMPrBrowserFlags				pcFlags,
	USBPrinterBrowserContextPtr*		context)
{	
	OSStatus	err;		// Error condition.
	
	err = noErr;
	
	/*
	Allocate the global context.
	*/
	*context = (USBPrinterBrowserContextPtr) ::NewPtrClear(sizeof(USBPrinterBrowserContext));

        if (NULL != *context)
	{
            USBPrinterBrowserContextPtr usbcontext = (USBPrinterBrowserContextPtr) *context;
           /*
            Initialize the global data.
            */
           usbcontext->browserView = NULL;

           /*
            Open our resource fork.
            */
            usbcontext->usbpbBundle = ::CFBundleGetBundleWithIdentifier(kUSBPBBundleID);
            usbcontext->pcFlags = pcFlags;
            usbcontext->myResFile =
                    //::_CFBundleOpenBundleResourceFork(usbcontext->usbpbBundle);
            ::CFBundleOpenBundleResourceMap(usbcontext->usbpbBundle);

            err = noErr; //ResError();
	}
	else
            err = MemError();
	
	return (err);
	
}	// InitContext

/******************************************************************************/

// End of "USBPrinterBrowser.cp"
