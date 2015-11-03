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
	File:		OutputBins.cp
	Contains: 	Implementation of the example printer module printing dialog extension

	Description:
		Implementation of the example  PPD based printer module printing dialog extension
		
		This module is compiled as a CFPlugin, which is then loaded by
		print dialog(s). The application is resonsible for loading the CFPlugin
		with the CFPluginCreate API call. The printing system will take it from there.

	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
*******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Pragmas
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Includes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <CoreFoundation/CoreFoundation.h>
#include <HIToolbox/HIToolbox.h>
#include <PrintCore/PMTicket.h>
#include <Print/PMPrintingDialogExtensions.h>

#include "PPDLib.h"

#include "OutputBins.h"

/*
 * The text and labels for the PDE are localized using a strings file.
 * You can use the command genstrings to create a strings file from this source. Use:
 *	genstrings -s CopyLocalizedStringFromPlugin OutputBins.cp
 *
 * You may get some complaints from genstrings about this macro, but it will create the file.
 */
#define CopyLocalizedStringFromPlugin(key, comment, pluginBundleRef) \
    CFBundleCopyLocalizedString((pluginBundleRef), (key), (key), NULL)

#define kPMPrintingManager 				CFSTR("com.pmvendor.printingmanager")
#define kPMPrintSettingsRef 			CFSTR("PMPrintSettingsTicket")

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Constants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// ---------------------------------------------------------------------------
#define kOutputBinsKindID 			CFSTR("com.pmvendor.print.pde.OutputBinsKind")
#define kOutputBinsBundleID 		CFSTR("com.pmvendor.print.pde.OutputBins")

// define the CFUUID for the factory
#define kOutputBinsIntfFactoryIDStr 	CFSTR("D931EF08-B9C2-11D4-B00E-003065416BA2")

// max size of our drawing area
// NOTE: Horizontal value is fixed so any value
// can be used - it will be ignored, anyway
#define kMAXH		150
#define kMAXV		60

// PPD keywords (pascal)
const unsigned char* kOutputBinKey	= "\pOutputBin";
			
// PPD keywords (CFString)
#define	kCFOutputBinKey				CFSTR("OutputBin")
#define	kUpperStr				CFSTR("Upper")
#define	kLowerStr				CFSTR("Lower")

// PPD info array indexes
enum {
	kNoMatch		= -1,
	kMainIndex	 	= 0,
	kOptionIndex	= 1,
	kAliasIndex	 	= 2,
	kForbiddenIndex	= 3
};

// .nib CFStrings
#define	kNibFileName	CFSTR("OutputBins")
#define	kNibWindowName	CFSTR("Window")

// .nib control signature
#define kSignature	'psob'	// (p)ost(s)cript (o)utput (b)ins

// .nib control IDs
enum {
	kControlOutputBinsPopupID	= 100,
};

enum {
	kSyncDirectionSetTickets = false,				// Set Ticket(s) based on UI
	kSyncDirectionSetUserInterface = true			// Set UI to reflect Ticket(s)
};

const ResType kPlugInCreator = 'pde ';

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Type definitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
The following Structure maintains the context between the IUnknown plugin and 
the MAC OS X system (Core Foundation). By adding a context structure to hold 
variables that are global in nature we are adding support for multi threaded 
operation.
*/
typedef struct
{
    const IUnknownVTbl*	vtable;						// Pointer to the vtable:    
    IUnknownVTbl		vtableStorage;				// Our vtable storage:
    CFUUIDRef			factoryID;					// Factory ID this instance is for:
    ULONG				refCount;					// Reference counter:
} IUnknownInstance;

/*
The following Structure maintains the context between this plugin and the host.
By adding a context structure to hold variables that are global in nature we are
adding support for multi threaded operation.
*/

typedef struct
{
    PlugInIntfVTable*	vtable;
    UInt32				refCount;    				// Reference counter
} OutputBinsPlugInInterface;

// vtable data structure
typedef struct
{
	CFBundleRef 		theBundleRef;				// Our bundle reference
	CFStringRef 		theTitleRef;				// the PDE title
	
	CFMutableArrayRef	PPDInfoArrayRef;			// Array contaning PPD keyword info

	ControlRef			outputBinsPopupControl;		// Menu control ref
	MenuRef				outputBinsMenuRef;			// Menu ref

} OutputBinsContext, *OutputBinsContextPtr, **OutputBinsContextHdl;
								
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Function prototypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus GetTicketFromSession(PMPrintSession printSession, CFStringRef key, void *valuePtr);
static OSStatus GetUIListFromPPD(PPDContext ppdContext, const unsigned char *key, 
                            CFMutableArrayRef *cfMutArray, SInt16 *optionsCount);
static OSStatus PPDIndexToKeywordInfoArray(PPDContext ppdContext, Mindex mainIndex, Oindex optionIndex, 
							unsigned char *optionAlias, Boolean forbidden, CFArrayRef *keyInfoCFArray);
static SInt32 FindPPDKeyPairsMatch(CFMutableArrayRef cfMutArray, CFStringRef mainStr, CFStringRef optionStr);
static OSStatus EmbedAndShowControl(ControlRef inControl, ControlRef  inContainer);
static void AdjustControlBounds (ControlRef control, SInt32 offsetV, SInt32 offsetH);

// Global initialization function.
static OSStatus InitContext(OutputBinsContextPtr* context);

#pragma export on
#if __cplusplus
extern "C" {
#endif

/*
The following define prototypes for the PDE API...
*/
static 
OSStatus Prologue(		PMPDEContext	*context,
						OSType 			*creator,
						CFStringRef		*userOptionKind,
						CFStringRef 	*title, 
						UInt32 			*maxH, 
						UInt32 			*maxV);
static
OSStatus Initialize(	PMPDEContext	context, 
						PMPDEFlags*		flags,
						PMPDERef		ref,
						ControlRef		parentUserPane,
						PMPrintSession	printSession);
static 
OSStatus GetSummaryText(PMPDEContext	context, 
						CFArrayRef 		*titleArray,
						CFArrayRef		*summaryArray);

static 
OSStatus Sync(			PMPDEContext	context,
						PMPrintSession	printSession,
						Boolean			reinitializePlugin);
static 
OSStatus Open(			PMPDEContext	context);

static 
OSStatus Close(			PMPDEContext	context);

static 
OSStatus Terminate(		PMPDEContext	context, 
						OSStatus		status);


#if __cplusplus
}
#endif
#pragma export off
					
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Global variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    CFPlugin Specific Routines
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* 	The CustomInfo.plist file and how it relates to PDE's.
	Two associations need to be made when developing a print dialog extension.
 	This association is accomplished by the information contained in the
	CustomInfo.plist file.
	The CustomInfo.plist file for this PDE looks like this:
{
	CFBundleExecutable = "OutputBins";
	CFBundleIdentifier = "com.pmvendor.print.pde.OutputBins";
    CFPlugInDynamicRegistration = NO;
    CFPlugInFactories = 
    {
        "D931EF08-B9C2-11D4-B00E-003065416BA2" = "OutputBinsPluginFactory";
    };
    CFPlugInTypes = 
    {
        "BDB091F4-E57F-11D3-B5CC-0050E4603277" = ("D931EF08-B9C2-11D4-B00E-003065416BA2");
    };
}
	The CFPlugInTypes entry associates the extensions type UUID with this plugins 
	factory UUID. And the CFPlugInFactories entry associates this plugins factory
	UUID with the factory function name.
	
*/

// Factory ID and routine.
// Plugin factory names must not be mangled by C++ compiler.
extern "C"
{
    // Factory function:
    void* OutputBinsPluginFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
}

// More Prototypes
static OSStatus CreatePlugInInterface(PlugInIntf** objPtr);
static ULONG IUnknownAddRef(void* obj);
static ULONG IUnknownRelease(void* obj);

/*
	The next two routines(IUnKnownAddRef, IUnKnownRelease) and QueryInterface are 
	required by  CFPlugin to instantiate or create an instance of a base class 
	(to put it in C++ terminology).
*/

/* =============================================================================
    Name:	IUnknownAddRef()
	Description:
        This function adds a tick to a plugin instance's reference count.
	Input Parameters:
        obj			-	The 'this' pointer.
	Output Parameters:
		None.
	Return Value:
        ULONG			-	Updated value of the reference count, or zero
						in case of an error. 
* ========================================================================== */
static ULONG IUnknownAddRef(void* obj)
{   
    IUnknownInstance* instance = (IUnknownInstance*) obj;
    ULONG refCount = 0;    							// We can't do much with errors here since we can only
													// update reference count value.   
	if (instance != NULL)
    {
		// Get updated refCount value (should be under mutex):
		refCount = ++instance->refCount;
    }
    else
    {
		refCount = 0;
    }

	return refCount;
}
/* =============================================================================
    Name:	IUnknownRelease()
	Description:
        This function takes a tick from a plugin instance's reference count.
		When the reference count goes down to zero the object self-destructs.
	Input Parameters:
        obj			-	The 'this' pointer.
	Output Parameters:
        None.    Return Value:
        ULONG		-	Updated value of the reference count, or zero
                        in case of an error.
* ========================================================================== */
static ULONG IUnknownRelease(void* obj)
{
    IUnknownInstance* instance = (IUnknownInstance*) obj;
    ULONG refCount = 0;
    
    // We can't do much with errors here since we can only return
    // updated reference count value.
	try
    {
		if (instance != NULL)
		{
			// Get updated refCount value (should be under mutex):
			// Make sure refCount is non-zero:
			if (0 == instance->refCount)
			{
				instance = NULL;
				return(refCount);
			}
	
			refCount = --instance->refCount;
			
			// Is it time to self-destruct?
			if (0 == refCount)
			{	    
				// Unregister 'instance for factory' with CoreFoundation:
				CFPlugInRemoveInstanceForFactory(instance->factoryID);														
				// Release used factoryID:				
				CFRelease(instance->factoryID);
				instance->factoryID = NULL;
				
				// Deallocate object's memory block:
				free((void*) instance);
				instance = NULL;
			}
		}
    }    
    catch(...)
    {
		if (instance)
		{
			// Release used factoryID:
			if (instance->factoryID)
				CFRelease(instance->factoryID);
		
			// Deallocate object's memory block:
			free((void*) instance);
		}
		// Return zero refCount value:
		refCount = 0;
    }
	return refCount;    
}
/*
	The next three routines(Retain, Release, GetAPIVersion) are required by 
	the Printing system to host the dialog extensions plugin.
*/

/* =============================================================================
    Name:	Retain()
	
	Description:
        This function controls our plugins instance.
		This function adds a tick to a plugin instance's reference count.

	Input Parameters:
        obj			-	The 'this' pointer.

	Output Parameters:
        None.

	Return Value:
        OSStatus		- 	The error status
* ========================================================================== */
static OSStatus Retain(PMPlugInHeaderInterface* obj)
{
    if (obj != NULL)
    {
		OutputBinsPlugInInterface* plugin = (OutputBinsPlugInInterface*) obj;
	
		// Increment reference count:
		plugin->refCount++;
	}

	return noErr;
}

/* =============================================================================
    Name:	Release()

	Description:
        This function controls our plugins instance. It takes a tick from a 
        plugin instance's reference count. When the reference count goes down
        to zero the object self-destructs.

	Input Parameters:
        obj			-	The 'this' pointer.

	Output Parameters:
        None.

	Return Value:
        OSStatus	- 	The error status
* ========================================================================== */
static OSStatus Release(PMPlugInHeaderInterface** objPtr)
{

   if (*objPtr != NULL)
    {
		OutputBinsPlugInInterface* plugin = (OutputBinsPlugInInterface*) *objPtr;
		
		// Clear caller's variable:
		*objPtr = NULL;
	
		// Decrement reference count:
		plugin->refCount--;
	
		// When reference count is one it's time self-destruct:
		if (0 == plugin->refCount)
		{
			// Delete object's vtable:
			free((char *)plugin->vtable);
			
			// Delete object's memory block:
			free((char *)plugin);
		}
    }
    return noErr;
}
/* =============================================================================
    Name:	GetAPIVersion()

	Description:
        This routine returns the plugins interface version

	Input Parameters:
        obj			-	The 'this' pointer.

	Output Parameters:
        versionPtr	- 	The pointer to the API version information

	Return Value:
        OSStatus	- an error code
 * ========================================================================== */
static OSStatus  GetAPIVersion(PMPlugInHeaderInterface* obj, PMPlugInAPIVersion* versionPtr)
{
  // Return versioning info:
    versionPtr->buildVersionMajor = kPDEBuildVersionMajor;
    versionPtr->buildVersionMinor = kPDEBuildVersionMinor;
    versionPtr->baseVersionMajor  = kPDEBaseVersionMajor;
    versionPtr->baseVersionMinor  = kPDEBaseVersionMinor;
    
    return noErr;
}
/* =============================================================================
    Name:	CreatePlugInInterface()

	Description:
        This function creates an interface for our plugin. An interface in this
        sample code consists of both the vtable of pointer to functions and the
        variables that will be global to the routines in this plugin. In this
        sample code we are storing a ControlRef in the interface.

	Input Parameters:
        obj			-	The 'this' pointer.

	Output Parameters:
        None.

	Return Value:
        OSStatus	- an error code
* ========================================================================== */
 
static OSStatus CreatePlugInInterface(PlugInIntf** objPtr)
{
    OutputBinsPlugInInterface* intf = NULL;
    PlugInIntfVTable* vtable = NULL;    			// Allocate object and clear it:

    intf = (OutputBinsPlugInInterface*) malloc(sizeof(OutputBinsPlugInInterface));

    if (intf != NULL) 
    {
		// init structure to null
		memset(intf, 0, sizeof(OutputBinsPlugInInterface));
	
		// Assign all plugin data members:
		intf->refCount = 1;
	
		// Allocate object's vtable and clear it:
		vtable = (PlugInIntfVTable*) malloc(sizeof(PlugInIntfVTable));
		if (vtable != NULL)
		{
			// init structure to null
			memset(vtable, 0, sizeof(PlugInIntfVTable));
	
			// set interface pointer
			intf->vtable = vtable;
	
			// Assign all plugin header methods:
			vtable->plugInHeader.Retain 		= Retain;
			vtable->plugInHeader.Release 		= Release;
			vtable->plugInHeader.GetAPIVersion 	= GetAPIVersion;
			
			// Assign all plugin methods:
			vtable->Prologue 					= Prologue;
			vtable->Initialize 					= Initialize;
			vtable->Sync 						= Sync;
			vtable->GetSummaryText 				= GetSummaryText;
			vtable->Open 						= Open;
			vtable->Close 						= Close;
			vtable->Terminate 					= Terminate;
		}
    }
    
    // Return results:
    *objPtr = (PlugInIntf*) intf;
    return noErr;
}
/* =============================================================================
    Name:	QueryInterface()
	
	Description:
        This function creates and interface for our plugin by calling 
        CreatePluginInterface. If the interface requested is for the
        "base" IUnKnownInterface just bump the refcount.

	Input Parameters:
        obj		-	The 'this' pointer.
        iID		- 	The UUID that describes which interface to return

	Output Parameters:
        intfPtr - 	A Pointer to the requested interface

	Return Value:
        HRESULT	-	Tells Core Foundation our status.
* ========================================================================== */
static HRESULT QueryInterface(void* obj, REFIID iID, LPVOID* intfPtr)
{
    IUnknownInstance* instance = (IUnknownInstance*) obj;
    CFUUIDRef myIntfID = NULL, reqIntfID = NULL;
    HRESULT err = E_UNEXPECTED;
    PlugInIntf* interface;

    // Get IDs for requested and PDE interfaces:
    reqIntfID = CFUUIDCreateFromUUIDBytes(CFAllocatorGetDefault(), iID);
    myIntfID = CFUUIDCreateFromString(CFAllocatorGetDefault(), kDialogExtensionIntfIDStr);
    
    // If we are asked to return the interface for 
    // the IUnkown vtable, which the system already has access to,
    // just increment the refcount value
    if (CFEqual(reqIntfID, IUnknownUUID))
    {
		instance->vtable->AddRef((void*) instance);
		*intfPtr = (LPVOID) instance;
		err = S_OK;
    }
    else if (CFEqual(reqIntfID, myIntfID))			// if we are asked for the PDEs interface,
	{												// lets make one and return it.
		err = CreatePlugInInterface(&interface);
		if (noErr == err)
		{
			*intfPtr = (LPVOID) interface;
			err = S_OK;
		}
    }
    else											// we will return the err = E_NOINTERFACE
	{												//  and a  *intfPtr of NULL
		*intfPtr = NULL;
		err = E_NOINTERFACE;
    }    // Clean up and return status:
    CFRelease(reqIntfID);
    CFRelease(myIntfID);
    
    return err;
}
/* =============================================================================
    Name:	OutputBinsPluginFactory()

	Description:
		This is the factory function which should be the only entry point of 
		this plugin. This factory function creates the "base class" for the system.
		The factory functions name (ie "OutputBinsPluginFactory") needs to be
		listed in the CustomInfo.plist to associate the factory function name
		with the factory function UUID. For example, this is how this function
		is associated with the UUID in the CustomInfo.plist file.
		
		CFPlugInFactories = 
		{
        	"D931EF08-B9C2-11D4-B00E-003065416BA2" = "OutputBinsPluginFactory";
    	};


    Input Parameters:
        allocator	- the allocator function used by CoreFoundation
        reqTypeID	- requested instance type.
	Output Parameters:
        None.    

	Return Value:

* ========================================================================== */
void* OutputBinsPluginFactory(CFAllocatorRef allocator, CFUUIDRef reqTypeID)
{
    CFUUIDRef			myInstID;
    CFUUIDRef			myFactoryID;
    IUnknownInstance*	instance = NULL;
	
	// There is not much we can do with errors - just return NULL.
    myInstID = CFUUIDCreateFromString(CFAllocatorGetDefault(), kPrinterModuleTypeIDStr);
    myFactoryID = CFUUIDCreateFromString(CFAllocatorGetDefault(), kOutputBinsIntfFactoryIDStr);

	// If the requested type matches our plugin type (it should!)
    // have a plugin instance created which will query us for
    // interfaces:
	if (CFEqual(reqTypeID, myInstID))
    {
		instance = (IUnknownInstance*) malloc(sizeof(IUnknownInstance));
		if (instance != NULL)
		{
			// Clear all object memory:
			memset(instance, 0, sizeof(IUnknownInstance));
			
			// Assign all members:
			instance->vtable = &instance->vtableStorage;
			
			instance->vtableStorage.QueryInterface = QueryInterface;
			instance->vtableStorage.AddRef = IUnknownAddRef;
			instance->vtableStorage.Release = IUnknownRelease;
			
			instance->factoryID = myFactoryID;
			instance->refCount = 1;

			// Register the newly created instance
			CFPlugInAddInstanceForFactory(myFactoryID);
		}
	}
    CFRelease(myInstID);

	return ((void*) instance);
}
/******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	Prologue
	
	Input Parameters:
		none

	Output Parameters:
		context			:	The plugins context
		creator			: 	The creator type for this plugin
		userOptionKind	: 	The extension kind for the plugin
		title			: 	The title of this plugin.
		maxH			:	Maximum horizontal dimension required by client.
		maxV			:	Maximum vertical dimension required by client.
		err				:	returns the error status

	Description:
		Returns dimensions of content region desired by the client.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static 
OSStatus Prologue(	PMPDEContext	*context,
					OSType 			*creator,
					CFStringRef		*userOptionKind,
					CFStringRef 	*title, 
					UInt32 			*maxH, 
					UInt32 			*maxV)
{
    OutputBinsContextPtr myContext = NULL;
    OSErr err = noErr;
	
	err = InitContext(&myContext);
    if (noErr == err)
    {
		*context = (PMPDEContext) myContext;

		// calculate the maximum amount of screen 
		// real estate that this plugin needs.
		*maxH = kMAXH;
		*maxV = kMAXV;

		// we need to save the title ref in our
		// context so we can release it later
		myContext->theTitleRef = CopyLocalizedStringFromPlugin(
				CFSTR("Output Bins"),
				CFSTR("the text of the popup menu"),
				myContext->theBundleRef);

		if (myContext->theTitleRef != NULL)
			*title = myContext->theTitleRef;
	
		*userOptionKind = kOutputBinsKindID;
		*creator = kPlugInCreator;    
    }
    else
		err = kPMInvalidPDEContext;						// return an error
	
	return (err);
}
/******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	Initialize

	Input Parameters:
		context				:	The plugins context
		parentUserPane		:	the user pane to your your controls into
		ref					:	the reference to this PDE
		printSession		:	this holds the PM tickets
		
	Output Parameters:
		flags				:	feature flags that are supported by this PDE
		err					:	returns the error status

	Description:
		Initializes client interface. Creates controls and sets initial values
	
	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static 
OSStatus Initialize(	PMPDEContext	context, 
						PMPDEFlags*		flags,
						PMPDERef		ref,
						ControlRef		parentUserPane,
						PMPrintSession	printSession)
{
    OSStatus err = noErr;
    OutputBinsContextPtr	myContext = NULL;		// Pointer to global data block.
	
    myContext = (OutputBinsContextPtr) context;
    
    if ((myContext != NULL) && (printSession != NULL))
    {		
		PMTicketRef 	printerInfo = NULL;
		
		// We need to determine if the compiled PPD is available in the
		// session and if there is an OutputBin keywords in the PPD.
		// If either of these is false, we can return an error and the
		// PDE won't be loaded.

		// Get the PrinterInfo TicketRef from the Session.
		err = GetTicketFromSession(printSession, kPDE_PMPrinterInfoRef, (void *)&printerInfo);

		// Open the PPD, if available, and check it for the OutputBin key
		if(err == noErr)
		{
			FSSpec		tempFSSpec;
			PPDContext	ppdContext;
			Mindex		mIndex;
			SInt16		count = 0;

			// All of the PPD access functions are currently file based. The
			// library function, ppdOpenCompiledPPDFromTicket()  can be used 
			// to take the compiled PPD data from the PrinterInfo ticket, write
			// it to a temporary file, and open it. The corresponding function,
			// ppdCloseCompiledPPDFromTicket(), will close the PPD and delete
			// the temporary file.
			err = ppdOpenCompiledPPDFromTicket(printerInfo, &ppdContext, &tempFSSpec);

			// ****** PPD OutputBins ****** //
			if(err == noErr)
			{
				mIndex = ppdGetMainIndex(ppdContext, (const StringPtr)kOutputBinKey);
				// if the returned Mindex is valid, call ppdGetUIKeyType()
				// to verify that the keyword really exists in this PPD, and
				// call ppdCountUIOptions to make sure options are available.
				//
				// ppdUIkeyType() returns:
				//	 0 if the keyword does not exist in the compiled PPD
				//	 1 if the keyword exists and appeared in UI form
				//	-1 if the keyword exists but appeared in a non-UI form
				if((mIndex == kPPDInvalidIndex) 
						|| (ppdGetUIKeyType(ppdContext, mIndex) != 1) 
						|| (ppdCountUIOptions(ppdContext, mIndex) == 0))
				{
					err = kPMInvalidKey;
				}
				
				if(err == noErr)
				{
					// set the UI constraints
					setInstallableOptions(ppdContext, printerInfo);

					// get the PPD key info and store it in myContext->PPDInfoArrayRef
					myContext->PPDInfoArrayRef = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks );
					err = GetUIListFromPPD(ppdContext, kOutputBinKey, 
											&(myContext->PPDInfoArrayRef), &count);
				}
				
				// close the PPD and delete the file
				ppdCloseCompiledPPDFromTicket(ppdContext, &tempFSSpec);
			}

			if(err == noErr)
			{
				IBNibRef 	nibRef = NULL;
				WindowRef 	nibWindow = NULL;
				ControlID	controlID;
		
				// get a nib reference
				err = CreateNibReferenceWithCFBundle(myContext->theBundleRef, kNibFileName, &nibRef);
				
				// Create a hidden window from the nib
				if(err == noErr)
					err = CreateWindowFromNib(nibRef, kNibWindowName, &nibWindow);
				
				// Get the popup menu control
				if(err == noErr)
				{
					controlID.signature = kSignature;
					controlID.id = kControlOutputBinsPopupID;
					err = GetControlByID(nibWindow, &controlID, &(myContext->outputBinsPopupControl));

					// move, embed and show the popup control
					if(err == noErr)
					{
						Rect	frameRect;
						
						// get the embedder control bounds
						GetControlBounds(parentUserPane, &frameRect);
						
						AdjustControlBounds(myContext->outputBinsPopupControl, frameRect.top, frameRect.left);
						err = EmbedAndShowControl(myContext->outputBinsPopupControl, parentUserPane);
					}
				}
				
				// Dispose the nib window & reference
				if(nibWindow != NULL)
					DisposeWindow(nibWindow);
				
				if(nibRef != NULL)
					DisposeNibReference(nibRef);
				
				// get menu ref and fill in the menu
				if(err == noErr)
				{
					// build the menu based on the data obtained from the PPD
					SInt32		i;
					CFArrayRef 	keyInfoCFArray = NULL;
					CFStringRef	aliasStringRef = NULL;
					
					myContext->outputBinsMenuRef = GetControlPopupMenuHandle(myContext->outputBinsPopupControl);

					// make sure the menu is empty, to start
					DeleteMenuItems(myContext->outputBinsMenuRef, 1, CountMenuItems(myContext->outputBinsMenuRef));

					for(i = 0; i < count; i++)
					{
						keyInfoCFArray = (CFArrayRef)CFArrayGetValueAtIndex(myContext->PPDInfoArrayRef, i);
						aliasStringRef = (CFStringRef)CFArrayGetValueAtIndex(keyInfoCFArray, kAliasIndex);
						AppendMenuItemTextWithCFString(myContext->outputBinsMenuRef, aliasStringRef, NULL, NULL, NULL);

						// if this is an uninstalled option, disable it
						if(CFStringCompare((CFStringRef)CFArrayGetValueAtIndex(keyInfoCFArray, kForbiddenIndex), 
												kUpperStr, 0) == kCFCompareEqualTo)
						{
							DisableMenuItem(myContext->outputBinsMenuRef, CountMenuItems(myContext->outputBinsMenuRef));
						}
					}
	
					// set the control min, max, and value,  
					SetControlMinimum(myContext->outputBinsPopupControl, 1);
					SetControlMaximum(myContext->outputBinsPopupControl, count);
					SetControlValue(myContext->outputBinsPopupControl, 1);
				}
				err = Sync(context, printSession, kSyncDirectionSetUserInterface);
			}
		}
	}
    else
		err = kPMInvalidPDEContext;
	
	return (err);
}
/******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	GetSummaryText

	Input Parameters:
		context			:	The plugins context
		titleArray 		:	an array to store the title of the summary text
		summaryArray	:	an array to store the summary text
		
	Output Parameters:
		titleArray 		:	updated with this plugins summary text title
		summaryArray	:	updated with this plugins summary text
		err				:	returns the error status	Description:
		Returns the status/state of the plugin in textual form
	
	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static 
OSStatus GetSummaryText(PMPDEContext	context, 
						CFArrayRef 		*titleArray,
						CFArrayRef		*summaryArray)
{
    OSStatus err = noErr;
    OutputBinsContextPtr myContext = NULL;		// Pointer to global data block.

    myContext = (OutputBinsContextPtr) context;
	*titleArray = NULL;
    *summaryArray = NULL;

	if (myContext != NULL)
    {
		CFMutableArrayRef theTitleArray = NULL;		// Init CF mutable arrays
		CFMutableArrayRef theSummaryArray = NULL;

		CFStringRef titleStringRef = NULL;			// Init CF strings
		CFStringRef summaryStringRef = NULL;
		
		//  NOTE: if the second parameter to CFArrayCreateMutable 
		//		  is not 0 then the array is a FIXED size
		theTitleArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		theSummaryArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    
		if ((theTitleArray != NULL) && (theSummaryArray != NULL))
		{
			titleStringRef = CopyLocalizedStringFromPlugin(
						CFSTR(" Output Bin"),
						CFSTR("Summary Title"),
						myContext->theBundleRef);
		
			if(titleStringRef != NULL) {
				// get the text of the selected menu item and
				// append it to the summary array
				err = CopyMenuItemTextAsCFString(myContext->outputBinsMenuRef,
								GetControlValue(myContext->outputBinsPopupControl), &summaryStringRef);

				if(err == noErr && summaryStringRef != NULL) {
					CFArrayAppendValue(theTitleArray, titleStringRef);
					CFArrayAppendValue(theSummaryArray, summaryStringRef);
					
					CFRelease(summaryStringRef);
		
					*titleArray = theTitleArray;
					*summaryArray = theSummaryArray;
				}
				CFRelease(titleStringRef);
			}
		}
		else {
			if (theTitleArray != NULL)
				CFRelease(theTitleArray);
	
			if (theSummaryArray != NULL)
				CFRelease(theSummaryArray);

			err = memFullErr;
		}

	}
    else
		err = kPMInvalidPDEContext;

	return (err);
}
/******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	Sync

	Input Parameters: 	
		context				:	The plugins context
		printSession		:	this holds the tickets
		syncDirection		:	A boolean that tells the plugin that it needs to
								do one of two functions. 
								If true the plugin
									fetches the values from the tickets and 
									sets the plugins control values.
								if false the plugin
									does the opposite, it takes the values out of the 
									plugins' controls and sets them into the the
									tickets
			
	Output Parameters:
		err					returns the error status

	Description:
		Sets/Gets values in the PageFormat or PrintSettings tickets

	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static 
OSStatus Sync(	PMPDEContext	context,
				PMPrintSession	printSession,
				Boolean			syncDirection)
{
    OSStatus err = noErr;
    OutputBinsContextPtr myContext = NULL;		// Pointer to global data block.
	
    myContext = (OutputBinsContextPtr) context;
    
    if ((myContext != NULL) && (printSession != NULL))
	{
		PMTicketRef printSettingsContainer = NULL;

		err = GetTicketFromSession(printSession, kPMPrintSettingsRef, &printSettingsContainer);
		if (noErr == err)		
		{
			CFArrayRef				outputBinsSetting = NULL;
			SInt32					keyIndex = kNoMatch;
			CFStringRef				cfOindexStr = NULL;
			CFMutableDictionaryRef	ppdDict = NULL;

			switch (syncDirection)
			{
				case kSyncDirectionSetUserInterface:
					// obtain the current PPD keyword dictionary
					err = PMTicketGetPPDDict(printSettingsContainer, kPMTopLevel, kPMTopLevel, &ppdDict);
					if(err == noErr)
					{
						// see if the OutputBins PPD main keyword is in the dict
						if(CFDictionaryContainsKey(ppdDict, kCFOutputBinKey))
						{
							// it is, so get the option keyword value and make 
							// sure it's a CFStringRef
							cfOindexStr = (CFStringRef)CFDictionaryGetValue(ppdDict, kCFOutputBinKey);
							if(cfOindexStr != NULL && CFStringGetTypeID() == CFGetTypeID(cfOindexStr))
							{
								// look for a match with the PPD info arrays
								keyIndex = FindPPDKeyPairsMatch(myContext->PPDInfoArrayRef, kCFOutputBinKey, cfOindexStr);
								// if a match is found, set the menu value
								// NOTE: remember the array is zero-based while
								// the menu is one-based.
								if(keyIndex != kNoMatch)
									SetControlValue(myContext->outputBinsPopupControl, keyIndex+1);
							}
						}
					}

					// an error probably indicates it wasn't in the ticket so
					// just set the default to be the first enabled item in the menu
					if(err != noErr || cfOindexStr == NULL || keyIndex == kNoMatch)
					{
						SInt32		i;
						SInt32		count = CountMenuItems(myContext->outputBinsMenuRef);

						err = noErr;		// clear any error
						for(i = 1; i <= count ; i++)
						{
							if(IsMenuItemEnabled(myContext->outputBinsMenuRef, i))
							{
								SetControlValue(myContext->outputBinsPopupControl, i);
								break;
							}
						}
					}
					break;

				case kSyncDirectionSetTickets:
					// get the seleceted menu value
					keyIndex = GetControlValue(myContext->outputBinsPopupControl);
					
					// get the corresponding PPD info array
					outputBinsSetting = (CFArrayRef)CFArrayGetValueAtIndex(myContext->PPDInfoArrayRef, keyIndex-1);

					// get the current PPD dictionary
					err = PMTicketGetPPDDict(printSettingsContainer, kPMTopLevel, kPMTopLevel, &ppdDict);
					if(err == noErr)
					{
						// set the PPD main/option keyword pair in the dict. Adding keys
						// to ppdDict adds them to the one held in the ticket
						CFDictionarySetValue(	ppdDict, 
												(CFStringRef)CFArrayGetValueAtIndex(outputBinsSetting, kMainIndex), 
												(CFStringRef)CFArrayGetValueAtIndex(outputBinsSetting, kOptionIndex));
									}
					break;
			}
		}
	}
    else
		err = kPMInvalidPrintSession;
	
	return (err);
}
/******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	Open
	Input Parameters:
		context				:	The plugins context
		
	Output Parameters:
		err					:	returns the error status
		
	Description:
		Do something before the plugins controls are shown in the print/page
		setup dialogs panel
	
	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus Open(PMPDEContext	context)
{
    OSStatus				err = noErr;
    OutputBinsContextPtr	myContext = NULL;		// Pointer to global data block.
    

    myContext = (OutputBinsContextPtr) context;
    if (myContext != NULL)
    {
		//  Do something useful here
    }
    else
		err = kPMInvalidPDEContext;

    return (err);
}

/******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	Close
	Input Parameters:
		context				:	The plugins context
		
	Output Parameters:
		err					:	returns the error status

	Description:
		Do something before control is shifted to another plugin

	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus Close(PMPDEContext	context)
{
    OSStatus err = noErr;
    OutputBinsContextPtr myContext = NULL;		// Pointer to global data block.
	
	myContext = (OutputBinsContextPtr) context;
    
    if (myContext != NULL)
    {
		// Do something useful here
    }
    else
		err = kPMInvalidPDEContext;

	return (err);
}

/******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	Terminate
	Input Parameters:
		context				:	The plugins context
		status				:	Tells us why we are going away
		
	Output Parameters:
		err					:	returns the error status
		
	Description:
		Disposes of controls, and other "things" created by this plugin.
	
	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus Terminate(PMPDEContext context, OSStatus status)
{
    OSStatus err = noErr;
    OutputBinsContextPtr myContext = NULL;		// Pointer to global data block.
	
    myContext = (OutputBinsContextPtr) context;
    
	if (myContext != NULL)
    {		
		if (myContext->theTitleRef != NULL)
		{
			CFRelease(myContext->theTitleRef);
			myContext->theTitleRef = NULL;
		}

		if (myContext->PPDInfoArrayRef != NULL)
		{
			CFRelease(myContext->PPDInfoArrayRef);
			myContext->PPDInfoArrayRef = NULL;
		}

		// Free the global context.
		free((char*) myContext);
		myContext = NULL;
    }
    else
		err = kPMInvalidPDEContext;

	return (err);
}
/******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Private Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	InitContext
	
	Input Parameters:
		
	Output Parameters:
		context		:	Pointer to global context.
		<function>	:	Status code.

	Description:
		Allocates and initializes the global data block for this module.
	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static
OSStatus
InitContext(OutputBinsContextPtr*	context)
{	
    OSStatus	err = noErr;						// Error condition.

    //Allocate the global context.
    *context =  (OutputBinsContextPtr) malloc(sizeof(OutputBinsContext));
    
    if (NULL != *context)
    {
		memset(*context, 0, sizeof(OutputBinsContext));

		//Initialize the global data.
		(*context)->theTitleRef = NULL;
		(*context)->PPDInfoArrayRef = NULL;
		(*context)->outputBinsPopupControl = NULL;
		(*context)->outputBinsMenuRef = NULL;

		(*context)->theBundleRef = CFBundleGetBundleWithIdentifier(kOutputBinsBundleID);
	}
    else
		err = kPMInvalidPDEContext;

    return (err);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	GetTicketFromSession

	Input Parameters
		printSession	:	this holds the PM tickets		
		ticket			:	Ticket we want	

	Output Parameters:
		container		:	container reference to the ticket
	
	Return Value:
			err			:	an error code

	Description:
	    Get the ticket refence from a printsession and ticket
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus GetTicketFromSession(PMPrintSession printSession, CFStringRef ticket, void* container)
{
	OSStatus err = noErr;
	CFNumberRef cfTicketRef = NULL;

	err = PMSessionGetDataFromSession(printSession, ticket, (CFTypeRef *)&cfTicketRef);	
	if ((noErr == err) && (cfTicketRef))
	{
		// This returns a Boolean (lossy data) that we don't care about
		Boolean lossy;
		lossy = CFNumberGetValue(cfTicketRef, kCFNumberSInt32Type, container);
	}
	else
	{
		err = kPMInvalidTicket;
	}
	return err;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	GetUIListFromPPD()

	Input Parameters:
		ppdContext		:	the PPDContext returned at ppdOpen		
		key				:	the PPD main key to get		
		
	Output Parameters:
		cfMutArray		:	array containing PPD info		
		optionsCount	:	count of UI options found		
		
	Description:
		Get a list of UI options available for a PPD keyword. The
		info is returned in a CFMutableArray. Each element is itself a
		CFArray of CFStrings. See the function, PPDIndexToKeywordInfoArray(),
		for the format of the CFArray.
		
	Change History:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus 
GetUIListFromPPD(	PPDContext	ppdContext,
					const unsigned char *key,
					CFMutableArrayRef *cfMutArray,
					SInt16 *optionsCount)
{
	OSStatus		err = noErr;
	SInt16			count;
	SInt32			i;
	Mindex			mainIndex;
	xUIOption		optionUI;
	unsigned char	optionAliasStr[256];
	CFArrayRef		keyInfoCFArray = NULL;
	
	if(*cfMutArray != NULL)
	{
		mainIndex = ppdGetMainIndex(ppdContext, (const StringPtr)key);
		if(mainIndex && (count = ppdCountUIOptions (ppdContext, mainIndex)) != 0)
		{
			optionUI.version = kUIHeaderVersion;
			
			for(i = 0; err == noErr && i < count; i++)
			{
				if(ppdGetIndUIOption(ppdContext, mainIndex, i, &optionUI))
				{
					// get the option translation string
					if(ppdGetOptionAlias(ppdContext, mainIndex, 
											optionUI.optionKeyIndex, optionAliasStr))
					{
						// store the main, option, alias & forbidden strings to a CFArray
						err = PPDIndexToKeywordInfoArray(ppdContext, mainIndex, optionUI.optionKeyIndex, 
														optionAliasStr, optionUI.forbidden, &keyInfoCFArray);
						p2cstrcpy((char *)optionAliasStr, optionAliasStr);
						if(err == noErr)
						{
							// add the key info CFArray to the mutable CFArray
							CFArrayAppendValue(*cfMutArray, (const void *)keyInfoCFArray);
							CFRelease(keyInfoCFArray);
							(*optionsCount)++;
						}
					}
				}
			}	// for
		}
	}

	return err;
}	// GetUIListFromPPD


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	PPDIndexToKeywordInfoArray()

	Input Parameters:
		ppdContext		:	the PPDContext returned at ppdOpen		
		mainIndex		:	the PPD main key to get		
		optionIndex		:	the PPD option key to get		
		alias			:	the PPD translation string		
		forbidden		:	the PPD constraint state		
		
	Output Parameters:
		keyInfoCFArray	:	CFArray of CFStrings containing PPD info		
		
	Description:
		Create a 4 element CFArray of CFStrings containing PPD info.
		The 4 elements are:
			0 - Main key CFString
			1 - Option key CFString
			2 - Translation CFString
			3 - Either "True" or "False" CFString signifying the forbidden state
		
	Change History:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus 
PPDIndexToKeywordInfoArray(	PPDContext ppdContext,
							Mindex mainIndex,
							Oindex optionIndex,
							unsigned char *alias,
							Boolean forbidden,
							CFArrayRef *keyInfoCFArray)
{
	OSStatus		err = noErr;
	CFStringRef		keyInfoCFStr[4];
	unsigned char	keyStr[256];

	ppdGetMainString(ppdContext, mainIndex, keyStr);
	keyInfoCFStr[kMainIndex] = CFStringCreateWithPascalString( kCFAllocatorDefault,  
								keyStr, kCFStringEncodingASCII);

	ppdGetOptionString(ppdContext, optionIndex, keyStr);
	keyInfoCFStr[kOptionIndex] = CFStringCreateWithPascalString( kCFAllocatorDefault,  
								keyStr, kCFStringEncodingASCII);

	keyInfoCFStr[kAliasIndex] = CFStringCreateWithPascalString( kCFAllocatorDefault,  
								alias, kCFStringEncodingASCII);

	keyInfoCFStr[kForbiddenIndex] = forbidden ? kUpperStr : kLowerStr;

	// create an array to hold the key string info
	*keyInfoCFArray = CFArrayCreate(CFAllocatorGetDefault(), (const void **)&keyInfoCFStr,
								4, &kCFTypeArrayCallBacks);

	// Release the strings that we "created" since their refCounts
	// were bumped when they were added to the array.
	CFRelease(keyInfoCFStr[kMainIndex]);
	CFRelease(keyInfoCFStr[kOptionIndex]);
	CFRelease(keyInfoCFStr[kAliasIndex]);
	
	return err;
}	// PPDIndexToKeywordInfoArray


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	FindPPDKeyPairsMatch()

	Input Parameters:
		cfMutArray		:	the PPD info array to search		
		mainStr			:	the PPD main key to search for		
		optionStr		:	the PPD option key to search for		
		
	Output Parameters:
		<none>
	
	Return Value:
			foundAt		:	index of a match or
							-1 if no match found
		
	Description:
		Search for a PPD main key, option key match in the PPD
		info array
		
	Change History:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static SInt32 
FindPPDKeyPairsMatch(	CFMutableArrayRef cfMutArray,
						CFStringRef mainStr,
						CFStringRef optionStr)
{
    SInt32		i, foundAt = kNoMatch;
    CFIndex		count;
    CFArrayRef	cfArray;
    
    count = CFArrayGetCount(cfMutArray);
    
    for(i = 0; i < count; i++)
	{
        cfArray = (CFArrayRef)CFArrayGetValueAtIndex(cfMutArray, i);
        
        if((CFStringCompare((CFStringRef)CFArrayGetValueAtIndex(cfArray, kMainIndex), mainStr, 0) == kCFCompareEqualTo) &&
           (CFStringCompare((CFStringRef)CFArrayGetValueAtIndex(cfArray, kOptionIndex), optionStr, 0) == kCFCompareEqualTo))
		{
            foundAt = i;
			break;
        }
    }

	return foundAt;
}	// FindPPDKeyPairsMatch

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	EmbedAndShowControl()

	Input Parameters:
		inControl	:	reference of the control to be embedded		
		inContainer	:	the container to hold the control		
		
	Output Parameters:
		<none>
	
	Return Value:
		err		:	an error code
		
	Description:
		Embed inControl in inContainer and set it to be visible
		
	Change History:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus 
EmbedAndShowControl (ControlRef inControl,  ControlRef inContainer)
{
	OSStatus err = noErr;
	
	err = EmbedControl (inControl, inContainer);
	if (err == noErr)
		err = SetControlVisibility (inControl, true, false);
		
	return err;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	AdjustControlBounds()

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		
	Change History:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void 
AdjustControlBounds (ControlRef control, SInt32 offsetV, SInt32 offsetH)
{
	Rect	controlRect;
	
	GetControlBounds(control, &controlRect);
	controlRect.top += offsetV; controlRect.bottom += offsetV;
	controlRect.left += offsetH; controlRect.right += offsetH;
	SetControlBounds(control, &controlRect);
}

