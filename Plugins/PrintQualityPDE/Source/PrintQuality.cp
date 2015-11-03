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
	File:		PrintQuality.cp
	Contains: 	Implementation of the example printer module printing dialog extension

	Description:
		Implementation of the example printer module printing dialog extension
		
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
#include <strings.h>

#include <ApplicationServices/ApplicationServices.h>

#include <PrintCore/PMTemplate.h>
#include <PrintCore/PMTicket.h>
#include <PrintCore/PMPrinterModule.h>
#include <Print/PMPrintingDialogExtensions.h>

#include "PrintQuality.h"


/*
 * The text and labels for the PDE are localized using a strings file.
 * You can use the command genstrings to create a strings file from this source. Use:
 *	genstrings -s CopyLocalizedStringFromPlugin PrintQuality.cp
 *
 * You may get some complaints from genstrings about this macro, but it will create the file.
 */
#define CopyLocalizedStringFromPlugin(key, comment, pluginBundleRef) \
    CFBundleCopyLocalizedString((pluginBundleRef), (key), (key), NULL)

#define kPMPrintingManager 				CFSTR("com.pmvendor.printingmanager")

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Constants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// ---------------------------------------------------------------------------
#define kSamplePMUserOptionKindID 		CFSTR("com.pmvendor.print.pde.PrintQualityKind")
#define kPrintQualityBundleID 			CFSTR("com.pmvendor.print.pde.PrintQuality")

// define the CFUUID for the factory
#define kPrintQualityIntfFactoryIDStr 	CFSTR("26DADC8E-2231-11D4-BD55-0050E4603277")

#define kMAXH							150			// max size of our drawing area
#define kMAXV							90			// should be calculated

// quality radio group values
enum
{
    kLowestRadioButtonControl = 1,
    kInkSaverRadioButtonControl = 2,
    kDraftRadioButtonControl = 3,
    kNormalRadioButtonControl = 4,
    kPhotoRadioButtonControl = 5,
    kBestRadioButtonControl = 6,
    kHighestRadioButtonControl = 7,
};

#define kButtonWidth					80
#define kButtonOffset					20
#define kButtonTop						120
#define kButtonLeft						72

#define kMaxEntries						(kHighestRadioButtonControl + 1)
#define kDefaultQuality					kDraftRadioButtonControl


enum {
	kSyncDirectionSetTickets = false,				// Set Ticket(s) based on UI
	kSyncDirectionSetUserInterface = true			// Set UI to reflect Ticket(s)
};

const ResType kPlugInCreator = 'paul';

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
} PrintQualityPlugInInterface;

typedef struct
{
    SInt16			theResFile;						// Resource FIle of this PDE
													// Here are our controls
    ControlRef		theQualityRadioGroupControl;
	ControlRef		theQualityControls[kMaxEntries];

	CFBundleRef 	theBundleRef;					// Our bundle reference
} PrintQualityContext, *PrintQualityContextPtr, **PrintQualityContextHdl;
								
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Function prototypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus GetTicketRef(PMPrintSession printSession, CFStringRef ticket, void* container);
static SInt16 TicketValueToControlValue(SInt32 theTicketValue);
static SInt32 ControlValueToTicketValue(SInt16 theControlValue);

// Global initialization function.
static OSStatus InitContext(PrintQualityContextPtr* context);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	This simple class saves and sets the current resource file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class  PDE_UseResFile {
public:
    inline PDE_UseResFile(SInt16 theResFile)	 	// Constructor
    {
		mtheResFile = CurResFile();					// Save current resFile
		UseResFile(theResFile);						// Set resFile	    
    }
    
    inline ~PDE_UseResFile()						// Destructor
    {
		UseResFile(mtheResFile);					// restore resFile
    }
private:
    SInt16	mtheResFile;							// storage for resource file
};


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
	CFBundleExecutable = "PrintQuality";
	CFBundleIdentifier = "com.pmvendor.print.pde.PrintQuality";
    CFPlugInDynamicRegistration = NO;
    CFPlugInFactories = 
    {
        "26DADC8E-2231-11D4-BD55-0050E4603277" = "PrintQualityPluginFactory";
    };
    CFPlugInTypes = 
    {
        "BDB091F4-E57F-11D3-B5CC-0050E4603277" = ("26DADC8E-2231-11D4-BD55-0050E4603277");
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
    void* PrintQualityPluginFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
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
		PrintQualityPlugInInterface* plugin = (PrintQualityPlugInInterface*) obj;
	
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
		PrintQualityPlugInInterface* plugin = (PrintQualityPlugInInterface*) *objPtr;
		
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
    PrintQualityPlugInInterface* intf = NULL;
    PlugInIntfVTable* vtable = NULL;    			// Allocate object and clear it:
    intf = (PrintQualityPlugInInterface*) malloc(sizeof(PrintQualityPlugInInterface));

    if (intf != NULL) 
    {
		// init structure to null
		memset(intf, 0, sizeof(PrintQualityPlugInInterface));
	
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
    Name:	PrintQualityPluginFactory()

	Description:
		This is the factory function which should be the only entry point of 
		this plugin. This factory function creates the "base class" for the system.
		The factory functions name (ie "PrintQualityPluginFactory") needs to be
		listed in the CustomInfo.plist to associate the factory function name
		with the factory function UUID. For example, this is how this function
		is associated with the UUID in the CustomInfo.plist file.
		
		CFPlugInFactories = 
		{
        	"26DADC8E-2231-11D4-BD55-0050E4603277" = "PrintQualityPluginFactory";
    	};


    Input Parameters:
        allocator	- the allocator function used by CoreFoundation
        reqTypeID	- requested instance type.
	Output Parameters:
        None.    

	Return Value:

* ========================================================================== */
void* PrintQualityPluginFactory(CFAllocatorRef allocator, CFUUIDRef reqTypeID)
{
    CFUUIDRef			myInstID;
    CFUUIDRef			myFactoryID;
    IUnknownInstance*	instance = NULL;

  // There is not much we can do with errors - just return NULL.
    myInstID = CFUUIDCreateFromString(CFAllocatorGetDefault(), kPrinterModuleTypeIDStr);
    myFactoryID = CFUUIDCreateFromString(CFAllocatorGetDefault(), kPrintQualityIntfFactoryIDStr);

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
OSStatus Prologue(		PMPDEContext	*context,
						OSType 			*creator,
						CFStringRef		*userOptionKind,
						CFStringRef 	*title, 
						UInt32 			*maxH, 
						UInt32 			*maxV)
{
    PrintQualityContextPtr myContext = NULL;
    OSErr err = noErr;
    

	err = InitContext(&myContext);
    
    if (noErr == err)
    {
		PDE_UseResFile theResFile(myContext->theResFile);
		*context = (PMPDEContext) myContext;

		// calculate the maximum amount of screen real estate that this plugin needs.
		*maxH = kMAXH;
		*maxV = kMAXV;

		CFStringRef theTitleRef = CopyLocalizedStringFromPlugin(
				CFSTR("Print Quality"),
				CFSTR("the text of the popup menu"),
				myContext->theBundleRef);

		if (theTitleRef != NULL)
			*title = theTitleRef;
	
		*userOptionKind = kSamplePMUserOptionKindID;
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
    PrintQualityContextPtr	myContext = NULL;		// Pointer to global data block.
    
    myContext = (PrintQualityContextPtr) context;
    
    if ((myContext != NULL) && (printSession != NULL))
    {		
		PDE_UseResFile theResFile(myContext->theResFile);		// get the windowref from the user pane

		// We need the Job Template
		PMTemplateRef 	jobTemplate = NULL;	
		err = GetTicketRef(printSession, kPDE_PMJobTemplateRef , &jobTemplate);
		if (err)
			return err;

		WindowRef theWindow = NULL;
		theWindow = GetControlOwner(parentUserPane);
		
		// the user panes rect is the rect we should use to draw our
		// controls into. The printing system calculates the user pane
		// size based on the maxh and maxv sizes returned from the 
		// Prologue function
		
		// Note that we are using the AutoToggleProc variant of the Radio Button control
		// This allows a hit on this control to be automatically processed by the ControlMgr
	
		// get controls
		myContext->theQualityRadioGroupControl = GetNewControl(kPrintQualityGroupControlID, theWindow);

		myContext->theQualityControls[kLowestRadioButtonControl] =
				GetNewControl(kPrintQualityLowestControlID, theWindow);
		myContext->theQualityControls[kInkSaverRadioButtonControl] =
				GetNewControl(kPrintQualityInkSaverControlID, theWindow);
		myContext->theQualityControls[kDraftRadioButtonControl] =
				GetNewControl(kPrintQualityDraftControlID, theWindow);
		myContext->theQualityControls[kNormalRadioButtonControl] =
				GetNewControl(kPrintQualityNormalControlID, theWindow);
		myContext->theQualityControls[kPhotoRadioButtonControl] =
				GetNewControl(kPrintQualityPhotoControlID, theWindow);
		myContext->theQualityControls[kBestRadioButtonControl] =
				GetNewControl(kPrintQualityBestControlID, theWindow);
		myContext->theQualityControls[kHighestRadioButtonControl] =
				GetNewControl(kPrintQualityHighestControlID, theWindow);
	
		// embed controls
		EmbedControl(myContext->theQualityRadioGroupControl, parentUserPane);
		for (SInt16 controlIndex = kLowestRadioButtonControl; controlIndex <= kHighestRadioButtonControl; controlIndex++)
			EmbedControl(myContext->theQualityControls[controlIndex], myContext->theQualityRadioGroupControl);

		SetControlVisibility(myContext->theQualityRadioGroupControl, true, false);

		// set controls as invisible
		for (SInt16 controlIndex = kLowestRadioButtonControl; controlIndex <= kHighestRadioButtonControl; controlIndex++)
			SetControlVisibility(myContext->theQualityControls[controlIndex],false, false);

        // Ask the template code how many entries we have in the array. This will be stored in count.
        // Once the number of entries is known, we can allocate space for them and loop through
        // the items.
		int count = 0;
        SInt32 *myArray = NULL ;
		err = PMTemplateGetSInt32ListConstraintValue(jobTemplate, kPMQualityKey, &count, NULL);
		if (noErr == err && count > 0)
		{
			myArray = (SInt32*)malloc(count * sizeof(SInt32));
			if ( myArray == NULL )
				err = memFullErr ;
		}
		
		if ( noErr == err && count > 0 )
		{
			err = PMTemplateGetSInt32ListConstraintValue(jobTemplate, kPMQualityKey, &count, myArray);
			if (noErr == err)
			{
				Point wherethecontrolgoes = { kButtonTop, kButtonLeft };
				
				for (SInt16 constraintindex = 0; constraintindex < count; constraintindex++)
				{
					ControlRef theControl = NULL;
					
					SInt16 controlIndex = TicketValueToControlValue(myArray[constraintindex]);
					theControl = myContext->theQualityControls[controlIndex];
					MoveControl(theControl, wherethecontrolgoes.h, wherethecontrolgoes.v);
					SetControlVisibility(theControl, true, false);
					wherethecontrolgoes.v += kButtonOffset;
					if ((wherethecontrolgoes.v - kButtonTop) >= (kMAXV - kButtonOffset))
					{
						wherethecontrolgoes.v = kButtonTop;
						wherethecontrolgoes.h += kButtonWidth;
					}						
				}
			}

            
        // Finally, once we're done we should free the list we had allocated.
        if ( myArray != NULL )
            free( myArray ) ;
        }
	
		// Set default value
		SInt32 qualityValue, qualityGroupControlValue;
		err = PMTemplateGetSInt32DefaultValue(jobTemplate, kPMQualityKey, &qualityValue);

		if (err == noErr)
		{
			qualityGroupControlValue = TicketValueToControlValue(qualityValue);
			SetControlValue(myContext->theQualityRadioGroupControl, qualityGroupControlValue);
		}
		else
		{
			SetControlValue(myContext->theQualityRadioGroupControl, kDefaultQuality);
		}
		
		// Set flags
		*flags = kPMPDENoFlags;
		
		// Initialize this plugins controls based on the information in the 
		// PageSetup or PrintSettings ticket.
		err = Sync(context, printSession, kSyncDirectionSetUserInterface);
		if (err == kPMKeyNotFound)
			err = noErr;
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
    PrintQualityContextPtr myContext = NULL;		// Pointer to global data block.
    
    myContext = (PrintQualityContextPtr) context;
	*titleArray = NULL;
    *summaryArray = NULL;

	if (myContext != NULL)
    {
		PDE_UseResFile theResFile(myContext->theResFile);

		CFMutableArrayRef theTitleArray = NULL;
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
						CFSTR(" Print Quality Output"),
						CFSTR("Summary Title"),
						myContext->theBundleRef);
		
			SInt16	qualityGroupControlValue = -1;
			qualityGroupControlValue = GetControlValue(myContext->theQualityRadioGroupControl);
			switch (qualityGroupControlValue)
			{
				case kLowestRadioButtonControl:
					summaryStringRef = CopyLocalizedStringFromPlugin(
						CFSTR(" Lowest"),
						CFSTR("Summary Text"),
						myContext->theBundleRef);
					break;
	
				case kInkSaverRadioButtonControl:
					summaryStringRef = CopyLocalizedStringFromPlugin(
						CFSTR(" InkSaver"),
						CFSTR("Summary Text"),
						myContext->theBundleRef);
					break;
	

				case kDraftRadioButtonControl:
					summaryStringRef = CopyLocalizedStringFromPlugin(
						CFSTR(" Draft"),
						CFSTR("Summary Text"),
						myContext->theBundleRef);
					break;
	
				case kNormalRadioButtonControl:
					summaryStringRef = CopyLocalizedStringFromPlugin(
						CFSTR(" Normal"),
						CFSTR("Summary Text"),
						myContext->theBundleRef);
					break;
	
				case kPhotoRadioButtonControl:
					summaryStringRef = CopyLocalizedStringFromPlugin(
						CFSTR(" Photo"),
						CFSTR("Summary Text"),
						myContext->theBundleRef);
					break;
	
				case kBestRadioButtonControl:
					summaryStringRef = CopyLocalizedStringFromPlugin(
						CFSTR(" Best"),
						CFSTR("Summary Text"),
						myContext->theBundleRef);
					break;

				case kHighestRadioButtonControl:
					summaryStringRef = CopyLocalizedStringFromPlugin(
						CFSTR(" Highest"),
						CFSTR("Summary Text"),
						myContext->theBundleRef);
					break;
	
			}

			CFArrayAppendValue(theTitleArray, titleStringRef);
			CFArrayAppendValue(theSummaryArray, summaryStringRef);

			*titleArray = theTitleArray;
			*summaryArray = theSummaryArray;
		}
		else
		{
			if (theTitleArray)
				CFRelease(theTitleArray);
			if (theSummaryArray)
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
OSStatus Sync(			PMPDEContext	context,
						PMPrintSession	printSession,
						Boolean			syncDirection)
{
    OSStatus err = noErr;
    PrintQualityContextPtr myContext = NULL;		// Pointer to global data block.

    
    myContext = (PrintQualityContextPtr) context;
    
    if ((myContext != NULL) && (printSession != NULL))
	{
		PDE_UseResFile theResFile(myContext->theResFile);

		PMTicketRef printSettingsContainer = NULL;
		err = GetTicketRef(printSession, kPDE_PMPrintSettingsRef, &printSettingsContainer);

		if (noErr == err)		
		{
			SInt32 qualityValue = 0;
			SInt16 qualityGroupControlValue = -1;

			switch (syncDirection)
			{
				case kSyncDirectionSetUserInterface:
					// Get the initial print quality setting
					err = PMTicketGetSInt32(printSettingsContainer, kPMTopLevel, kPMTopLevel, kPMQualityKey, &qualityValue);
					if (noErr == err)
					{
						qualityGroupControlValue = TicketValueToControlValue(qualityValue);
						SetControlValue(myContext->theQualityRadioGroupControl, qualityGroupControlValue);
					}						
					break;

				case kSyncDirectionSetTickets:
					// capture the print quality setting
					qualityGroupControlValue = GetControlValue(myContext->theQualityRadioGroupControl);
					qualityValue = ControlValueToTicketValue(qualityGroupControlValue);
					err = PMTicketSetSInt32(printSettingsContainer, kPMPrintingManager, kPMQualityKey, qualityValue, kPMUnlocked);
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
		setup dialogs panel	Change History (most recent first):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus Open(PMPDEContext	context)
{
    OSStatus				err = noErr;
    PrintQualityContextPtr	myContext = NULL;		// Pointer to global data block.
    

    myContext = (PrintQualityContextPtr) context;
    if (myContext != NULL)
    {
		PDE_UseResFile theResFile(myContext->theResFile);		// set resFile
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
    PrintQualityContextPtr myContext = NULL;		// Pointer to global data block.
	
	myContext = (PrintQualityContextPtr) context;
    
    if (myContext != NULL)
    {
		PDE_UseResFile theResFile(myContext->theResFile);		// set resFile
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
    PrintQualityContextPtr myContext = NULL;		// Pointer to global data block.
 
    myContext = (PrintQualityContextPtr) context;
    
	if (myContext != NULL)
    {		
		if (myContext->theResFile != -1)
		{											// Close the resource fork
			CFBundleRef theBundleRef = NULL;
			theBundleRef = CFBundleGetBundleWithIdentifier(kPrintQualityBundleID);
			CFBundleCloseBundleResourceMap(theBundleRef, myContext->theResFile);
			myContext->theResFile = -1;
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
InitContext(PrintQualityContextPtr*	context)
{	
    OSStatus	err = noErr;						// Error condition.
    
   /*
    Allocate the global context.
    */
    *context =  (PrintQualityContextPtr) malloc(sizeof(PrintQualityContext));
    
    if (NULL != *context)
    {
		memset(*context, 0, sizeof(PrintQualityContext));
		/*
		Initialize the global data.
		*/
		(*context)->theQualityRadioGroupControl = NULL;

		(*context)->theBundleRef = NULL;

		(*context)->theResFile = -1;
		CFBundleRef theBundleRef = NULL;

		//  Open the resource fork
		theBundleRef = CFBundleGetBundleWithIdentifier(kPrintQualityBundleID);
		(*context)->theResFile =  CFBundleOpenBundleResourceMap(theBundleRef);
		if ((*context)->theResFile == -1)
			err = kPMGeneralError;

		(*context)->theBundleRef = theBundleRef;
	}

    else
		err = kPMInvalidPDEContext;

    return (err);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	GetTicketRef

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
static OSStatus GetTicketRef(PMPrintSession printSession, CFStringRef ticket, void* container)
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
	Name:	TicketValueToControlValue

	Input Parameters
		theTicketValue	:	value of ticket		

	Output Parameters:
		None.

	Return Value:
		controlValue	:	control index of radio group

	Description:
	    This function returns the control value to represent the ticket value
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static SInt16 TicketValueToControlValue(SInt32 theTicketValue)
{
	SInt16 controlValue = 0;
	
	switch (theTicketValue)
	{
		case kPMQualityLowest:			
			controlValue = kLowestRadioButtonControl;
			break;
	
		case kPMQualityInkSaver:
			controlValue = kInkSaverRadioButtonControl;
			break;
	
		case kPMQualityDraft:
			controlValue = kDraftRadioButtonControl;
			break;
	
		case kPMQualityNormal:
			controlValue = kNormalRadioButtonControl;
			break;
	
		case kPMQualityPhoto:
			controlValue = kPhotoRadioButtonControl;
			break;
	
		case kPMQualityBest:
			controlValue = kBestRadioButtonControl;
			break;
	
		case kPMQualityHighest:
			controlValue = kHighestRadioButtonControl;
			break;
			
		default:
			controlValue = kDefaultQuality;
			break;
			
	}
	return controlValue;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	ControlValueToTicketValue

	Input Parameters
		theControlValue	:	value of radio group control		

	Output Parameters:
		None.

	Return Value:
		controlValue	:	control index of radio group

	Description:
	    This function returns the ticket value th control value
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static SInt32 ControlValueToTicketValue(SInt16 theControlValue)
{
	SInt32 theTicketValue = 0;
	
	switch (theControlValue)
	{
		case kLowestRadioButtonControl:
			theTicketValue = kPMQualityLowest;
			break;
	
		case kInkSaverRadioButtonControl:
			theTicketValue = kPMQualityInkSaver;
			break;
	
		case kDraftRadioButtonControl:
			theTicketValue = kPMQualityDraft;
			break;
	
		case kNormalRadioButtonControl:
			theTicketValue = kPMQualityNormal;
			break;
	
		case kPhotoRadioButtonControl:
			theTicketValue = kPMQualityPhoto;
			break;
	
		case kBestRadioButtonControl:
			theTicketValue = kPMQualityBest;
			break;
	
		case kHighestRadioButtonControl:
			theTicketValue = kPMQualityHighest;
			break;
	}
	return theTicketValue;
}

