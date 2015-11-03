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
	File:		USBIOPluginInterface.cp
	Contains: 	Implementation of the IOM plugin wrapper.
	
	Description:
		Implements the wrapper routines necessary to make the IOM a loadable CFPlugin
		
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
		
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ApplicationServices/ApplicationServices.h> 
#include "PMIOModule.h"
#include "USBIOInternals.h"

// Factory ID for USB IO Plugin
#define kIOMFactoryIDStr	"AC9D524C-AAD5-11D4-8D35-0030656DE360"

/*
The following Structure maintains the context between the IUnknown plugin and 
the MAC OS X system (CFPlugin). By adding a context structure to hold 
variables that are global in nature we are adding support for multi threaded 
operation.
*/
typedef struct
{
    // Pointer to the IUnknown vtable:
    IUnknownVTbl*			vtable;

    // IUnknown vtable storage:
    IUnknownVTbl			vtableStorage;

    // Reference counter:
    ULONG					refCount;

} IUnknownInstance;

// 
typedef struct
{
    // Pointer to the PrinterModule vtable:
	IOMProcs*				vtable;

	// Printer Module vtable storage
	IOMProcs				vtableStorage;

    // Reference counter:
	UInt32					refCount;    

} IOMInterfaceImpl;

// Prototypes
extern "C"
{
    void* IOPluginFactory ( CFAllocatorRef allocator, CFUUIDRef typeID );
}

/*
	The next two routines(IUnknownAddRef, IUnknownRelease) and QueryInterface are 
	required by CFPlugin to instantiate or create an instance of a base class 
	(to put it in C++ terminology).
*/
static ULONG 	IUnknownAddRef( void* obj );
static ULONG 	IUnknownRelease( void* obj );
static HRESULT 	QueryInterface( void* obj, REFIID iID, LPVOID* intfPtr );

static OSStatus CreateIOMInterface( IOMInterfaceImpl** objPtr );
static OSStatus Retain( PMPlugInHeaderInterface* obj );
static OSStatus Release( PMPlugInHeaderInterface** objPtr );
static OSStatus GetUID( PMPlugInHeaderInterface* objPtr, CFUUIDRef* idPtr  );
static OSStatus GetAPIVersion( PMPlugInHeaderInterface* obj, PMPlugInAPIVersion* versionPtr );

/*****************************************************************************
	Name:			IOPluginFactory

	Input Parameters:
		
	Output Parameters:

	Description:
			Called by CF to determine if a requested "Type" is supported by this plugin.
			It checks if requested type is kIOModuleTypeIDStr, it creates and returns an "Instance". 
			Otherwise, it returns a nil instance.

*****************************************************************************/
void* IOPluginFactory ( CFAllocatorRef allocator, CFUUIDRef reqTypeID )
{
    IUnknownInstance*	instance = NULL;
    CFUUIDRef			myTypeID = NULL;
    CFUUIDRef			myFactoryID = NULL;

    // If the requested type matches our plugin type (it should!)
    // have a plugin instance created which will query us for
    // interfaces:
    myTypeID = CFUUIDCreateFromString( CFAllocatorGetDefault(), CFSTR(kIOModuleTypeIDStr) );

    if( CFEqual( reqTypeID, myTypeID ))
	{
		instance = (IUnknownInstance*) NewPtrClear( sizeof( IUnknownInstance ));
		if (instance != NULL)
		{
		    // Assign all members:
		    instance->vtable = &instance->vtableStorage;

		    instance->vtableStorage.QueryInterface = QueryInterface;
		    instance->vtableStorage.AddRef = IUnknownAddRef;
		    instance->vtableStorage.Release = IUnknownRelease;

			myFactoryID = CFUUIDCreateFromString( CFAllocatorGetDefault(), CFSTR(kIOMFactoryIDStr) );
		    instance->refCount = 1;

			CFPlugInAddInstanceForFactory( myFactoryID );	// Register the newly created instance
		}													// for our factory with CoreFoundation:

	}
	
	if (myTypeID != NULL)
		CFRelease( myTypeID );
	if (myFactoryID != NULL)
		CFRelease( myFactoryID );

   return (void*) instance;
}

/*****************************************************************************
	Name:			QueryInterface

	Input Parameters:
		
	Output Parameters:

	Description:
			Called by CF to determine if a requested "Interface" is supported by this plugin.
			It checks if requested interface is kIOModuleIntfIDStr, it creates and returns an 
			"Interface".  Otherwise, it returns a nil instance.


*****************************************************************************/
static HRESULT QueryInterface( void* obj, REFIID iID, LPVOID* intfPtr )
{
    IUnknownInstance*	instance = (IUnknownInstance*) obj;
    CFUUIDRef			myIntfID = NULL, reqIntfID = NULL;
    HRESULT				err = E_UNEXPECTED;
    IOMInterfaceImpl*	interface = NULL;

   // Get IDs for requested and back end interfaces:
    reqIntfID = CFUUIDCreateFromUUIDBytes( CFAllocatorGetDefault(), iID );
    myIntfID = CFUUIDCreateFromString( CFAllocatorGetDefault(), CFSTR(kIOModuleIntfIDStr) );

	// IUnkown vtable interface request
    if( CFEqual( reqIntfID, IUnknownUUID ) )
    {
        instance->vtable->AddRef( (void*) instance );
        *intfPtr = (LPVOID) instance;
		err = S_OK;
    }
	// IOM interface request:
    else if( CFEqual( reqIntfID, myIntfID ))
    {
        if( CreateIOMInterface ( &interface ) == noErr )
        {
            *intfPtr = (LPVOID) interface;
            err = S_OK;
        }
    }
    // Unknown interface request:
    else
	{
	    *intfPtr = NULL;
	    err = E_NOINTERFACE;
	}

    // Clean up and return status:
	if (reqIntfID != NULL)
		CFRelease( reqIntfID );
	if (myIntfID != NULL)
		CFRelease( myIntfID );

    return err;
}


/*****************************************************************************
	Name:			CreateIOMInterface

	Input Parameters:
		
	Output Parameters:

	Description:
			Creates an IOM "Interface" and returns it to caller.


*****************************************************************************/
static OSStatus CreateIOMInterface ( IOMInterfaceImpl** objPtr )
{
	IOMInterfaceImpl*		intf = NULL;

  	// Allocate object and clear it:
   	intf = (IOMInterfaceImpl*) NewPtrClear( sizeof( IOMInterfaceImpl ));
	
	if (intf != NULL) 
	{
		// Assign all plugin data members:
		intf->refCount = 1;
	    
		intf->vtable = &intf->vtableStorage;

		// Assign all plugin header methods:
		intf->vtable->pluginHeader.Retain 		= Retain;
		intf->vtable->pluginHeader.Release 		= Release;
		intf->vtable->pluginHeader.GetAPIVersion 	= GetAPIVersion;

		// Assign all plugin methods:
		intf->vtable->GetConnectionInfo	= IOMGetConnectionInfo;
		intf->vtable->Initialize	 	= IOMInitialize;
		intf->vtable->Open			 	= IOMOpen;
		intf->vtable->Read			 	= IOMRead;
		intf->vtable->Write		 		= IOMWrite;
		intf->vtable->Status		 	= IOMStatus;
		intf->vtable->Close		 		= IOMClose;
		intf->vtable->GetAttribute		= IOMGetAttribute;
		intf->vtable->SetAttribute		= IOMSetAttribute;
		intf->vtable->Terminate	 		= IOMTerminate;
	}

	// Return results:
	*objPtr = (IOMInterfaceImpl*) intf;
    
	return noErr;
}

/* =============================================================================

    Name:	IUnknownAddRef()

    Description:
        This function adds a tick to a plugin instance's reference count.

    Input Parameters:
        obj			-	The 'this' pointer.

    Output Parameters:
		None.

	Return Value:
        ULONG		-	Updated value of the reference count, or zero
						in case of an error.

 * ========================================================================== */

static ULONG IUnknownAddRef( void* obj )
{   
    IUnknownInstance*	instance = (IUnknownInstance*) obj;
	ULONG	refCount = 0;
	
    // We can't do much with errors here since we can only return
    // updated reference count value.

	if( NULL != instance )
	{
        // Get updated refCount value (should be under mutex):
        refCount = ++instance->refCount;
    }
    else
    {
        refCount = 0;
    }	
		
    return ( refCount );
}


/* =============================================================================

    Name:	IUnknownRelease()

    Description:
        This function takes a tick from a plugin instance's reference count.
		When the reference count goes down to zero the object self-destructs.

    Input Parameters:
        obj			-	The 'this' pointer.

    Output Parameters:
        None.

    Return Value:
        ULONG		-	Updated value of the reference count, or zero
                        in case of an error.

 * ========================================================================== */

static ULONG IUnknownRelease( void* obj )
{
    IUnknownInstance*	instance = (IUnknownInstance*) obj;
    ULONG				refCount = 0;

    // We can't do much with errors here since we can only return
    // updated reference count value.

	try 
	{
		if( NULL == instance )
		{
	        // Get updated refCount value (should be under mutex):
	
	        // Make sure refCount is non-zero:
			if (instance->refCount > 0)
			{
				refCount = --instance->refCount;
		
				// Is it time to self-destruct?
				if( refCount == 0 )
				{	    
					// Unregister 'instance for factory' with CoreFoundation:
					CFPlugInRemoveInstanceForFactory( IUnknownUUID );
		
					// Deallocate object's memory block:
					free( (void*) instance );
					instance = NULL;
				}
			}
	    }
    }

    catch( ... )
    {
        if( instance )
        {
            // Deallocate object's memory block:
            free( (void*) instance );
 			instance = NULL;
       }

        // Return zero refCount value:
        refCount = 0;
    }

    return refCount;

}

/*****************************************************************************
	Name:			Retain

	Input Parameters:
		
	Output Parameters:

	Description:


*****************************************************************************/
static OSStatus Retain( PMPlugInHeaderInterface* obj )
{

	if (obj != NULL)
	{
	    IOMInterfaceImpl* plugin = (IOMInterfaceImpl*) obj;

	    // Increment reference count:
	    plugin->refCount++;

	}
    return noErr;
}


/*****************************************************************************
	Name:			Release

	Input Parameters:
		
	Output Parameters:

	Description:

*****************************************************************************/
static OSStatus Release( PMPlugInHeaderInterface** objPtr )
{
	if (*objPtr != NULL)
	{
	    IOMInterfaceImpl* plugin = (IOMInterfaceImpl*) *objPtr;

	    // Clear caller's variable:
	    *objPtr = NULL;

	    // Decrement reference count:
		if (plugin->refCount > 0)
	    {
			plugin->refCount--;
	
			// When reference count is one it's time self-destruct:
			if( plugin->refCount == 0 )
			{
				// Delete object's memory block:
				DisposePtr( (char *)plugin );
			}
		}
	}

    return noErr;
}

/*****************************************************************************
	Name:			GetAPIVersion

	Input Parameters:
		
	Output Parameters:

	Description:


*****************************************************************************/
static OSStatus GetAPIVersion( PMPlugInHeaderInterface* obj, PMPlugInAPIVersion* versionPtr )
{

    // Return versioning info:
    versionPtr->buildVersionMajor = kIOMBuildVersionMajor;
    versionPtr->buildVersionMinor = kIOMBuildVersionMinor;
    versionPtr->baseVersionMajor  = kIOMBaseVersionMajor;
    versionPtr->baseVersionMinor  = kIOMBaseVersionMinor;

    return noErr;
}

