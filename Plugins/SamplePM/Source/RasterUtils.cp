/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under AppleÍs copyrights in this original Apple software (the
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

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	File: RasterUtils.cp

	Contains: Implements internal utility routines for the Sample Raster Printer Module
			
 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

 	Bugs?:		For bug reports, consult the following page on
 				the World Wide Web:
 
 					http://developer.apple.com/bugreporter/
	Change History:

	To Do:
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include <stdio.h>
#include <dirent.h>

#include <ApplicationServices/ApplicationServices.h>

#include <PrintCore/PMPrinterModule.h>
#include <PrintCore/PMTicket.h>
#include <PrintCore/PMTemplate.h>
#include <PrintCore/PMRaster.h>
#include "RasterModule.h"
#include <unistd.h>	// for sleep

OSErr AddPaperToList( PMTicketRef ourListTicket, SInt32 index, CFStringRef paperName, double pageLeft, 
                        double pageTop, double pageRight, double pageBottom, double paperLeft,
                        double paperTop, double paperRight, double paperBottom );

#define DOCUSTOMPAPER	0
#define DUPLEXING_PRINTER 0

#if DOCUSTOMPAPER
static OSStatus addCustomPaperToTemplate(PMTemplateRef pmTemplate);
#endif

/*--------------------------------------------------------------------------------------------
	Function: hpCreateIconData()

	Description:
		Opens Sample.icns file in Resources folder and returns its data to caller as CFDataRef.
		
	Parameters:
		cfIconData - Reference to the CFData where icon data is stored.
						
		return value - OSStatus, contains errors if there were any.
		
	History:

--------------------------------------------------------------------------------------------*/
OSStatus hpCreateIconData( CFDataRef *cfIconData)
{
	OSStatus 			osStatus = noErr;
	CFDataRef 			cfDataRef = NULL;
	CFBundleRef			bundleRef = NULL;
	CFURLRef			bundleUrl = NULL, resUrl = NULL;
	CFStringRef			bundlePath = NULL, resPath = NULL;
	FILE 				*fileNum = NULL;
	Ptr 				buffer = NULL;
	char 				fullName[MAXNAMLEN];		
	UInt32 				fileSize = 0;
 
	// Get URLs for bundle folder and Resources folder
	bundleRef = CFBundleGetBundleWithIdentifier( kRasterPMBundleID);
   	if (bundleRef != NULL)
	{
		bundleUrl = CFBundleCopyBundleURL( bundleRef);
		resUrl = CFBundleCopyResourcesDirectoryURL( bundleRef);
   	}
	
	// Convert to actual path names
	if (bundleUrl != NULL && resUrl != NULL)
	{
		bundlePath = CFURLCopyFileSystemPath( bundleUrl, kCFURLPOSIXPathStyle);
		resPath = CFURLCopyFileSystemPath( resUrl, kCFURLPOSIXPathStyle);

		CFRelease( bundleUrl);
		CFRelease( resUrl);
   	}
	
	// Convert to C strings, append to make a full path name to the icon file.
	if (bundlePath != NULL && resPath != NULL)
	{
		char resStr[MAXNAMLEN];		 
		
		CFStringGetCString( bundlePath, fullName, MAXNAMLEN-1, kCFStringEncodingUTF8);
		CFStringGetCString( resPath, resStr, MAXNAMLEN-1 , kCFStringEncodingUTF8);
		CFRelease( bundlePath);
		CFRelease( resPath);

		strcat(fullName, "/");
		strcat(fullName, resStr);
		strcat(fullName, "/");
		strcat(fullName, kRasterPMIconFileName);

		// open icon file now
		fileNum = fopen( fullName, "r");
		if (fileNum == NULL)
			osStatus = errno; 

	}
	
	if (osStatus == noErr && fileNum != NULL)
	{
		// get file size, allocate a buffer of the same size, and read into it at once
		osStatus = fseek( fileNum, 0, 2 ) ;					// Seek to the end of the file.
		if ( osStatus == noErr )
		{
			fileSize = ftell( fileNum ) ;			// Find size
			osStatus = fseek( fileNum, 0, 0 ) ;				// Rewind.
			buffer = (Ptr)malloc( fileSize);
			if (buffer != NULL)
				fileSize = (UInt32) fread( buffer, 1, (size_t) fileSize, fileNum);
			else
				osStatus = memFullErr;
		}
		fclose( fileNum);
	}

	// we read the data, now it is time to make a CFData out of it and pass it backup
	if (osStatus == noErr && buffer != NULL)
	{
		cfDataRef = CFDataCreate( CFAllocatorGetDefault(), (const UInt8 *)buffer, fileSize);
		if (cfIconData != NULL)
			*cfIconData = cfDataRef;
    	else
			osStatus = memFullErr;

		free (buffer);
	}
		
	return osStatus;
 }
                 
/*--------------------------------------------------------------------------------------------
	Function: hpCreateProfileDict()

	Description:
		Opens <Vendor-Model>.Profiles.xml file in Resources folder and returns its data to
		caller as CFDictionaryRef.
        
        The dictionary will contain key-value pairs for:
            the default profile ID
            an array of profile dictionaries - each of which will have:
                the profile ID
                the profile name
                the profile URL
                
        It will look something like this:
        
        <dict>
            <key>PMColorDeviceDefaultProfileID</key>
            <integer>1</integer>
            <key>PMColorDeviceProfiles</key>
            <array>
                <dict>
                    <key>PMColorDeviceProfileID</key>
                    <integer>1</integer>
                    <key>PMColorDeviceProfileName</key>
                    <string>Plain Paper</string>
                    <key>PMColorDeviceProfileURL</key>
                    <string>file://localhost/...path to profile</string>
                </dict>
                
                    ...more profile dicts
                
            </array>
        </dict>
		
	Parameters:
		pmProfileDict - ptr to CFDictionaryRef in which profile data is stored - REQUIRED
						
		return value - OSStatus, contains errors if there were any.
		
	History:
		
--------------------------------------------------------------------------------------------*/
OSStatus hpCreateProfileDict( CFDictionaryRef *pmProfileDict)
{
	CFAllocatorRef		allocator			= CFAllocatorGetDefault();
	CFDataRef 			dataRef				= NULL;
	CFBundleRef			bundleRef			= NULL;
	CFURLRef			bundleUrl			= NULL;
	CFURLRef			resUrl				= NULL;
	CFURLRef			plistUrl			= NULL;
	CFStringRef			resPath				= NULL;
	CFStringRef			bundlePath			= NULL;
	CFMutableStringRef	bundleAndResPath	= NULL;
	OSStatus 			osStatus			= noErr;
	SInt32				errorCode			= 0;
	#define				mCFRelease(cfo)		if(NULL!=cfo){CFRelease(cfo);cfo=NULL;}
    
    // First check then zero the return CFDictionaryRef param
    if (pmProfileDict == NULL)
        return (OSStatus) paramErr;
        
    *pmProfileDict = NULL;
 
	// Get URLs for bundle folder and Resources folder.
    // The Bundle path will be a full path - from volume.
    // The Resources path will be partial, relative to the Bundle.
    //
	bundleRef = CFBundleGetBundleWithIdentifier( kRasterPMBundleID);
   	if (bundleRef != NULL)
	{
		resUrl = CFBundleCopyResourcesDirectoryURL( bundleRef);
		bundleUrl = CFBundleCopyBundleURL( bundleRef);
   	}
    
	// Convert resource and Bundle URL to path component CFString
	if (bundleUrl != NULL && resUrl != NULL)
	{
		resPath = CFURLCopyFileSystemPath( resUrl, kCFURLPOSIXPathStyle);
		bundlePath = CFURLCopyFileSystemPath( bundleUrl, kCFURLPOSIXPathStyle);
		
		// Release URLs
		mCFRelease(resUrl);
		mCFRelease(bundleUrl);
   	}
	
   	// Copy the bundlePath StringRef to a mutable StringRef - to append to it
	if (bundlePath != NULL)
	{
        bundleAndResPath = CFStringCreateMutableCopy(allocator, 0, bundlePath);
		mCFRelease(bundlePath);
	}
	
   	// Combine StringRefs to get full path to resources dir
    if (bundleAndResPath != NULL && resPath != NULL)
	{
        // Unfortunately, the resources path doesn't start with "/"
        CFStringAppend(bundleAndResPath, CFSTR("/"));
        CFStringAppend(bundleAndResPath, resPath);
		mCFRelease(resPath);
    }
    
    // Now create the full URL
	if (bundleAndResPath != NULL)
	{
        // The "base URL" needs to be provided - file://localhost
        // otherwise CFURLCreateDataAndPropertiesFromResource
        // returns kCFURLImproperArgumentsError.
        // 
        resUrl = CFURLCreateWithString(allocator, bundleAndResPath, 
        CFURLCreateWithString(allocator, CFSTR("file://localhost"), NULL));
		mCFRelease(bundleAndResPath);
	}
   	
	// Append file name of plist to make a full path URL to the plist file.
	if (resUrl != NULL)
	{
   		plistUrl = CFURLCreateCopyAppendingPathComponent(allocator,
   														resUrl,
   														CFSTR(kRasterPMProfileListFileName),
   														false );
        mCFRelease(resUrl);
	}

   	
	//
	// If there is no data in this file, we get nil back
	// and an error code of -10: kCFURLUnknownError.
    //
	if (plistUrl != NULL)
	{
		CFURLCreateDataAndPropertiesFromResource (
					allocator,
					plistUrl,
					&dataRef,		// get file data here
					NULL,			// don't need properties
					NULL,			// no desired properties
					&errorCode );

		mCFRelease(plistUrl);        
	}	
   	
	if (dataRef != NULL && errorCode == 0)
	{
		// Create a propertyList and pass it back.
		// If empty return value, set errorCode
		*pmProfileDict = (CFDictionaryRef)
                        CFPropertyListCreateFromXMLData (
						allocator,
						dataRef,
						kCFPropertyListImmutable,
						NULL );
		
        mCFRelease(dataRef);
		if (*pmProfileDict == NULL)
			errorCode = kPMProfileDictLoadFailure;
	}
    else errorCode = kPMProfileDictNotFound;
	
	// Release any locally allocated objects.  We do this
    // here to catch anything we might have missed above.
    //
	mCFRelease(dataRef);
	mCFRelease(resPath);
	mCFRelease(bundlePath);
	mCFRelease(bundleAndResPath);
	mCFRelease(resUrl);
	mCFRelease(bundleUrl);
	mCFRelease(plistUrl);
	
    osStatus = (OSStatus) errorCode;
	return osStatus;
 }
                 
/*--------------------------------------------------------------------------------------------
	Function: hpCopyEntriesFromProfileDict()

	Description:
		Queries CFDictionaryRef for standard key-values.
		
	Parameters:
		pmProfileDict		- CFDictionaryRef in which profile data is stored.
		pmDefaultProfileID	- SInt32* to be returned with the value found
		pmProfileArray		- CFArrayRef* to be returned with the value found
		pmProfileCount		- SInt32* to be returned with the CFArray count (of profiles)
		pmProfileIDList		- SInt32**
						
		return value - OSStatus, contains errors if there were any.
		
	History:
		
--------------------------------------------------------------------------------------------*/
OSStatus hpCopyEntriesFromProfileDict( CFDictionaryRef pmProfileDict,
										SInt32*			pmDefaultProfileID,
										CFArrayRef*		pmProfileArray,
										SInt32*			pmProfileCount,
										SInt32**		pmProfileIDList )
{
	OSStatus 			osStatus = noErr;
	CFDictionaryRef		aProfileDict;
	CFNumberRef			aProfileID;
	CFArrayRef			profileArray;
	SInt32				i, count, num;
	SInt32*				ourProfileIDList = NULL;
	Boolean				found = true;
    
    // Ensure the passed dict exists
    if (NULL == pmProfileDict)
        osStatus = paramErr;
 
	// Find the default profileID entry
	if (noErr == osStatus)
	{
        found = CFDictionaryGetValueIfPresent (	pmProfileDict,
                                                kPMColorDeviceDefaultProfileID,
                                                (const void**) &aProfileID );
        if (found && pmDefaultProfileID)
            found = CFNumberGetValue(aProfileID, kCFNumberSInt32Type, pmDefaultProfileID);
    
        if (!found)	// Use specific error codes to track failures
            osStatus = kPMProfileDictProfileIDFailure;
    }
		
	if (noErr == osStatus)
	{
		// Find the profile array entry
		found = CFDictionaryGetValueIfPresent (	pmProfileDict,
												kPMColorDeviceProfiles,
												(const void**) &profileArray );
		// If we got it, return the count.
		if (found)
		{
			if (pmProfileArray)
				*pmProfileArray = (CFArrayRef)CFRetain(profileArray);	// hpCopyEntriesFromProfileDict needs to do a copy
			count = (SInt32) CFArrayGetCount(profileArray);
			if (pmProfileCount)
				*pmProfileCount = count;
		}
		else	// Error getting array of profiles
			osStatus = kPMProfileDictProfileArrayFailure;
	}
	
	// Get the list of IDs from the profile dict - if asked for
	if (noErr == osStatus && pmProfileIDList)
	{
		ourProfileIDList = (SInt32 *) malloc(count * sizeof(SInt32));

		if (NULL != ourProfileIDList)
		{
			// Pass array back to caller
			*pmProfileIDList = ourProfileIDList;
			
			for (i = 0; i < count; i++)
			{
                // Clear the slot first
                ourProfileIDList[i] = 0;
                
                // Get the dictionary object from the array
				aProfileDict = (CFDictionaryRef) CFArrayGetValueAtIndex(profileArray, (CFIndex) i);
				if (NULL == aProfileDict)
					found = false;
				else
					// Get the profile ID entry
					found = CFDictionaryGetValueIfPresent (	aProfileDict,
															kPMColorDeviceProfileID,
															(const void**) &aProfileID );
                                                            
				// Store the ID if we got a value and it can be converted to SInt32
				if (found && CFNumberGetValue(aProfileID, kCFNumberSInt32Type, &num))
                    ourProfileIDList[i] = num;
			}
		}
		else	// No memory for list
			osStatus = memFullErr;
	}
			
	if (!found)	// Error getting list of profileIDs
		osStatus = kPMProfileDictProfileIDListFailure;
	
	return osStatus;
 }

/*--------------------------------------------------------------------------------------------
	Function: hpCreatePrinterInfo()

	Description:
		Creates a new Printer Info ticket and returns it to the caller. This printer info ticket
		supplies information about the printer's characteristics such as recommended application
		drawing resolutions, device resolution, colors available, profiles, input and output 
		trays, etc. Unlike PMTemplates, PrinterInfo entries are not used in verifying job ticket 
		entries.
		
	Parameters:
		pmContext - our private context data so there can be multiple sessions going at once.
						
		printerInfo - Pointer to ticket ref for our ticket once we've allocated it.
												
		return value - OSStatus, contains errors if there were any.
		
	History:
		
	To Do: 

--------------------------------------------------------------------------------------------*/
OSStatus hpCreatePrinterInfo(PMContext pmContext, PMTicketRef *printerInfo )
	{
	OSStatus			result = noErr ;
	CFMutableArrayRef	tempFileArray = NULL ;				// CFArray to hold our file type strings.
	CFArrayRef			ourProfiles	= NULL;					// CFArray of ColorSync profile dicts
	CFDictionaryRef		ourProfileDict = NULL;				// Holds data on our profiles.
	PMResolution		ourRes[1] ;							// Holds values before Template code gets them.
	
	
	// Start by allocating a ticket to return and filling it from our array in our header file.
	
	result = PMTicketCreate( CFAllocatorGetDefault(), kPMPrinterInfoTicket, printerInfo ) ;
										
	// Set our printer long name (this string may be used in UI where we have lots of space)
	if ( result == noErr )
		result = PMTicketSetCFString( *printerInfo, kRasterPMBundleID, kPMPrinterLongNameKey, 
						kRasterPrinterLongName, kPMLocked) ;

	// Set our printer short name (this string is used in UI where we have little space)
	if ( result == noErr )
		result = PMTicketSetCFString( *printerInfo, kRasterPMBundleID, kPMPrinterShortNameKey, 
						kRasterPrinterShortName, kPMLocked) ;

	// Set our product name
	if ( result == noErr )
		result = PMTicketSetCFString( *printerInfo, kRasterPMBundleID, kPMMakeAndModelNameKey, 
						kRasterPrinterProductName, kPMLocked) ;
		
	// Set color capability to true
	if ( result == noErr )
		result = PMTicketSetBoolean( *printerInfo, kRasterPMBundleID, kPMSupportsColorKey, true, kPMLocked);

	// Set copies capability to false (our PM and printer are unable to do multi copies on its own)
	if ( result == noErr )
		result = PMTicketSetBoolean( *printerInfo, kRasterPMBundleID, kPMDoesCopiesKey, false, kPMLocked);

	// Set copy collate capability to false (cannot do collation)
	if ( result == noErr )
		result = PMTicketSetBoolean( *printerInfo, kRasterPMBundleID, kPMDoesCopyCollateKey, false, kPMLocked);

	// Set reverse order capability to false (cannot do reverse order printing)
	if ( result == noErr )
		result = PMTicketSetBoolean( *printerInfo, kRasterPMBundleID, kPMDoesReverseOrderKey, false, kPMLocked);

	// Set supported file types
	// We can handle two different types of input - deep raster data (RGB for now) and 
	// "already processed engine" data. This already processed data would be the result of a 
	// previous print job that went through this same module and printed to disk.
	
	if ( result == noErr )
		{
		tempFileArray = CFArrayCreateMutable( CFAllocatorGetDefault(), NULL, &kCFTypeArrayCallBacks ) ;
		if ( tempFileArray == NULL )
			result = memFullErr ;
		}
		
	if ( result == noErr )
		{
		CFArrayAppendValue( tempFileArray, kPMDataFormatRaster ) ;
		CFArrayAppendValue( tempFileArray, kPMDataFormat ) ;
		result = PMTicketSetCFArray( *printerInfo, kRasterPMBundleID, kPMInputFileTypeListKey,
										tempFileArray, kPMUnlocked ) ;
		CFRelease( tempFileArray ) ;
		}
		
	// All printers are resolution independent now, in the same fashion that PostScript devices used to
	// be. This is because Quartz takes care of any necessary scaling required to get from the application's
	// drawing resolution to the resolution requested by the PM at conversion time. 
	// We start out with the suggested resolution for this printer - the resolution(s) you would most 
	// like to see the application drawing art and images with. This can be an array of discreete
	// resolutions, but are just recommendations to the application. It will also be allowed to 
	// pick any resolution between the min and max given below.
	
	if ( result == noErr )
		{
		ourRes[0].hRes = 300.0 ;
		ourRes[0].vRes = 300.0 ;
		result = PMTicketSetPMResolutionArray( *printerInfo, kRasterPMBundleID, kPMPrinterSuggestedResKey, 
												&ourRes[0], 1, false );
		}
		
	
	// Continue by setting the printer's minimum and maximum resolutions. We're suggesting reasonable
	// values here, but you may wish to modify them. Keep in mind that the "default" drawing res for
	// application is 72dpi.
	
	if ( result == noErr )
		{
		ourRes[0].hRes = 25.0 ;
		ourRes[0].vRes = 25.0 ;
		result = PMTicketSetPMResolutionArray( *printerInfo, kRasterPMBundleID, kPMPrinterMinResKey, 
												&ourRes[0], 1, false );
		}
	if ( result == noErr )
		{
		ourRes[0].hRes = 2500.0 ;
		ourRes[0].vRes = 2500.0 ;
		result = PMTicketSetPMResolutionArray( *printerInfo, kRasterPMBundleID, kPMPrinterMaxResKey, 
												&ourRes[0], 1, false );
		}
		
		
	// Get a list of ColorSync profile info dictionaries.
	if ( result == noErr )
        result = hpCreateProfileDict( &ourProfileDict );
        
	if ( result == noErr )
	{
		//	Add the profile list to the printer info ticket
		//	Our list of profiles is defined as an array of CFDictionaries.
		//	The keys in each profile dictionary are:
		//
		//	key							value
		//	---							-----
		//	PMColorDeviceProfileID		CFString	(convert to number)
		//	PMColorDeviceProfileName	CFString	(for menus or other HI)
		//	PMColorDeviceProfileURL		CFURL		(complete location specifier for profile)
		//
	
		// Get the profile array only
		result = hpCopyEntriesFromProfileDict( ourProfileDict, NULL, &ourProfiles, NULL, NULL);
                CFRelease(ourProfileDict);
                ourProfileDict = NULL;
	
		// Make the profile array entry
		if ( result == noErr ) {
			result = PMTicketSetCFArray( *printerInfo, kRasterPMBundleID, kPMColorSyncProfilesKey, ourProfiles, kPMLocked);
          	CFRelease(ourProfiles);
           	ourProfiles = NULL;
         }
     }
        else if ( result == kPMProfileDictNotFound ) 
		result = noErr;

	// Let go of the printerInfo if we had any trouble up to this point.
	
	if ( result != noErr )
		{
		if ( *printerInfo != NULL )
			PMTicketReleaseAndClear( printerInfo ) ;
		}

	return( result ) ;
	}

/*--------------------------------------------------------------------------------------------
	Function: AddPaperToList()

	Description:
		Adds a paper info ticket to the list of paper info tickets. This utility was created
        simply to make the code easier to read and to facilitate testing by adding various
        paper sizes quickly.
		
	Parameters:
				
--------------------------------------------------------------------------------------------*/
OSErr AddPaperToList( PMTicketRef ourListTicket, SInt32 index, CFStringRef paperName, double pageLeft, 
                        double pageTop, double pageRight, double pageBottom, double paperLeft,
                        double paperTop, double paperRight, double paperBottom )
    {
    OSErr	result = noErr ;
    PMTicketRef		ourPaperTicket ;
    PMRect	tempRect ;
    
    // Create a paper info ticket to store all this in.
    result = PMTicketCreate( CFAllocatorGetDefault(), kPMPaperInfoTicket, &ourPaperTicket ) ;
    if ( result == noErr )
        result = PMTicketSetCFString( ourPaperTicket, kRasterPMBundleID, kPMPaperNameKey, paperName, kPMLocked ) ;
    tempRect.top = paperTop ;
    tempRect.left = paperLeft ;
    tempRect.bottom = paperBottom ;
    tempRect.right = paperRight ;
    if ( result == noErr )
        result = PMTicketSetPMRect( ourPaperTicket, kRasterPMBundleID, kPMUnadjustedPaperRectKey, &tempRect, kPMLocked ) ;
    tempRect.top = pageTop ;
    tempRect.left = pageLeft ;
    tempRect.bottom = pageBottom ;
    tempRect.right = pageRight ;
    if ( result == noErr )
        result = PMTicketSetPMRect( ourPaperTicket, kRasterPMBundleID, kPMUnadjustedPageRectKey, &tempRect, kPMLocked ) ;
    if ( result == noErr )
        result = PMTicketSetTicket( ourListTicket, ourPaperTicket, index ) ;
    if ( ourPaperTicket != NULL )
        PMTicketReleaseAndClear( &ourPaperTicket ) ;
    return ( result ) ;
    }


/*!
 * @function	addDuplexValue
 * @abstract	Convert the provided value to a CFNumber and add it to the provided
 *		array.
 */
static OSStatus addDuplexValue(CFMutableArrayRef duplexArray, SInt32 duplexValue)
{
    OSStatus err = noErr;
    CFNumberRef duplexRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &duplexValue);
    if (duplexRef != NULL) {
	CFArrayAppendValue(duplexArray, duplexRef);
	CFRelease(duplexRef);
	duplexRef = NULL;
    } else {
	err = memFullErr;
    }
    
    return err;
}


/*--------------------------------------------------------------------------------------------
	Function: addDuplexToTemplate()

	Description:
		Adds duplexing information to a job template.
		
	Parameters:
		templateTicket - the template ticket to add to.
						
		canDuplexNoTumble - A boolean indicating whether the printer can duplexNoTumble without
					any intervention from the operating system.
												
		canDuplexTumble - A boolean indicating whether the printer can duplexTumble without
					any intervention from the operating system.

		requiresDifferentMarginsForOSFlippedSide2 - a boolean indicating whether the OS should
				    adjust the raster for the bottom margin rather than the top margin. 

		haveOurOwnPDE	- a boolean indicating whether we have our own custom PDE to handle
							the duplex layout. If true the printing system will not do its
							own duplex UI.

		return value - OSStatus, contains errors if there were any.
		
	History:
		
--------------------------------------------------------------------------------------------*/
static OSStatus addDuplexToTemplate(PMTemplateRef templateTicket, Boolean canDuplexNoTumble, 
					Boolean canDuplexTumble, Boolean requiresDifferentMarginsForOSFlippedSide2,
					Boolean haveOurOwnPDE)
{
    CFMutableArrayRef duplexArray = NULL;
    OSStatus err = noErr;

    duplexArray = CFArrayCreateMutable(kCFAllocatorDefault, 0 /* capacity */, &kCFTypeArrayCallBacks);
    
    if (duplexArray != NULL) {
	/* If the local host states the printer can do duplex, then we'll enable simplex,
	 * duplex, and duplex tumble. As long as the printer can duplex, we can flip the
	 * page images to get the correct tumble state.
	 */
	err = addDuplexValue(duplexArray, kPMDuplexNone);
	if (!err && canDuplexNoTumble) err = addDuplexValue(duplexArray, kPMDuplexNoTumble);
	if (!err && canDuplexTumble) err = addDuplexValue(duplexArray, kPMDuplexTumble);
    
	/* Create the duplex template entry.
	 */
	if (!err) err = PMTemplateMakeEntry(templateTicket, kPMDuplexingKey, kPMValueSInt32, kPMConstraintList);

	/* Set the list of duplex options.
	 */
	if (!err) err = PMTemplateSetCFArrayConstraintValue(templateTicket, kPMDuplexingKey, duplexArray);

	/* The default is duplex off.
	 */
	if (!err) err = PMTemplateSetSInt32DefaultValue(templateTicket, kPMDuplexingKey, kPMDuplexNone);

#ifdef kPMDuplexingRequiresFlippedMarginAdjustKey	// need to build with updated headers to add this entry
	if(!err){
	    err = PMTemplateMakeEntry( templateTicket, kPMDuplexingRequiresFlippedMarginAdjustKey, 
						kPMValueBoolean, kPMConstraintList) ;
	    if (!err)
		err = PMTemplateSetBooleanDefaultValue( templateTicket, kPMDuplexingRequiresFlippedMarginAdjustKey,
							    requiresDifferentMarginsForOSFlippedSide2) ;
	}
#endif
#ifdef kPMHasCustomDuplexPDEKey
	if(!err){
	    err = PMTemplateMakeEntry( templateTicket, kPMHasCustomDuplexPDEKey, 
						kPMValueBoolean, kPMConstraintList) ;
	    if (!err)
		err = PMTemplateSetBooleanDefaultValue( templateTicket, kPMHasCustomDuplexPDEKey,
							    haveOurOwnPDE) ;
	}

#endif

	CFRelease(duplexArray);
	duplexArray = NULL;
    }
    
    return noErr;
}


/*--------------------------------------------------------------------------------------------
	Function: hpCreatePMTemplate()

	Description:
		Creates Job Template and returns it to the caller. This job template describes the
		range of possible settings for this Printer module.
		
	Parameters:
		pmContext - our private context data so there can be multiple sessions going at once.
						
		pmTemplate - Pointer to ticket ref for the template we must return.
												
		return value - OSStatus, contains errors if there were any.
		
	History:
		
--------------------------------------------------------------------------------------------*/
OSStatus hpCreatePMTemplate(PMContext pmContext, PMTemplateRef *pmTemplate)
	{
	OSStatus 				result = noErr ;
	CFDictionaryRef			ourProfileDict = NULL;		// Holds data on our profiles.
	SInt32					ourDefaultProfileID;		// ID of default profile
	SInt32					ourProfileCount;			// Count of profiles
	SInt32*					ourProfileIDList = NULL;	// List of profile IDs
	PMTemplateRef			ourTemplate ;				// The template we're building up.
	SInt32					ourSInt32s[4] ;				// Holding place on the way to template call.
	PMTicketRef				ourPaperInfoTicket = NULL;	// Place to store a paper info ticket
	PMTicketRef				ourListTicket = NULL ;		// Holds the entire series of Paper Info Tickets
	

	
	// Make a template to fill
	
	result = PMTemplateCreate( &ourTemplate ) ;
	
	
	// Start by setting up the color modes.
	
	if ( result == noErr )
		result = PMTemplateMakeEntry( ourTemplate, kPMColorModeKey, kPMValueSInt32, 
										kPMConstraintList ) ;

	// Set the default color mode to color.
	
	if ( result == noErr )
		result = PMTemplateSetSInt32DefaultValue( ourTemplate, kPMColorModeKey, 
													kPMColor ) ;

	// We allow B/W, Grayscale, and Color.
    	
	ourSInt32s[0] = kPMBlackAndWhite ;
	ourSInt32s[1] = kPMGray ;
	ourSInt32s[2] = kPMColor ;
	if ( result == noErr )
		result = PMTemplateSetSInt32ListConstraint( ourTemplate, kPMColorModeKey, 3, &ourSInt32s[0] ) ;

#ifdef kPMColorSpaceModelKey	// need to build with updated headers to add this entry
        if(result == noErr){
            // We allow only RGB.
            PMColorSpaceModel ourSupportedColorSpaceModels[1] = {kPMRGBColorSpaceModel};
            result = PMTemplateMakeEntry( ourTemplate, kPMColorSpaceModelKey, kPMValueSInt32, 
                                                                            kPMConstraintList ) ;

            // Our default color space model is kPMRGBColorSpaceModel.
            
            if ( result == noErr )
                result = PMTemplateSetSInt32DefaultValue( ourTemplate, kPMColorSpaceModelKey, kPMRGBColorSpaceModel ) ;

            
            if ( result == noErr )
                result = PMTemplateSetSInt32ListConstraint( ourTemplate, kPMColorSpaceModelKey, 
                                                            sizeof(ourSupportedColorSpaceModels)/sizeof(PMColorSpaceModel),
                                                            (SInt32 *)&ourSupportedColorSpaceModels[0] ) ;
        }
#endif
	// Set up quality modes - we allow draft, normal, and best modes, with draft as the default.
	
	if ( result == noErr )
		result = PMTemplateMakeEntry( ourTemplate, kPMQualityKey, kPMValueSInt32, kPMConstraintList ) ;
	if ( result == noErr )
		result = PMTemplateSetSInt32DefaultValue( ourTemplate, kPMQualityKey, kPMQualityDraft ) ;
	ourSInt32s[0] = kPMQualityDraft ; 
	ourSInt32s[1] = kPMQualityNormal ;
	ourSInt32s[2] = kPMQualityBest ;
	if ( result == noErr )
		result = PMTemplateSetSInt32ListConstraint( ourTemplate, kPMQualityKey, 3, &ourSInt32s[0] ) ;

    // ColorSync Matching Intent key was removed
#if OLD
	// Setup color matching intent 
	if ( result == noErr )
		result = PMTemplateMakeEntry( ourTemplate, kPMColorSyncIntentKey, kPMValueSInt32, kPMConstraintList ) ;
	if ( result == noErr )
		result = PMTemplateSetSInt32DefaultValue( ourTemplate, kPMColorSyncIntentKey, kPMColorIntentAutomatic ) ;
	ourSInt32s[0] = kPMColorIntentAutomatic ; 
	ourSInt32s[1] = kPMColorIntentPhoto ;
	ourSInt32s[2] = kPMColorIntentBusiness ;
	if ( result == noErr )
		result = PMTemplateSetSInt32ListConstraint( ourTemplate, kPMColorSyncIntentKey, 3, &ourSInt32s[0] ) ;
#endif

#if DUPLEXING_PRINTER
	if(!result){
	    /*
		These settings match the HP Deskjet 970 (and similar) duplexing printers. Specifically: this printer
		feeds the second side of a sheet with the leading edge of the edge that was the bottom of the sheet
		when the first side was fed. This means that printer always "tumbles" the sheet when printing duplex.
		Because this printer cannot do duplexNoTumble without operating system intervention, we so indicate
		here. For this specific configuration the OS will flip the coordinate system when generating duplexNoTumble
		to compensate for the fact that the printer can only produce tumbled output.
		
		The requiresDifferentMarginsForOSFlippedSide2 indicates whether we wish to have the raster supplied to us 
		for the second sheet adjusted when the OS must flip the coordinate system to achieve the resulting duplex request. 
		For our case here by setting this to true, we are saying we want the raster supplied to us for side 2 to BEGIN 
		at a distance from the top of the sheet equivalent to the BOTTOM margin of the imageable area of the paper rather
		than at the TOP margin of the imageable area of the paper as it normally would be. 
		
		The haveOurOwnPDE value reflects whether we have our own PDE for allowing the user to handle 2 sided printing. Apple strongly
		recommends that you use the standard system UI for this purpose but if you provide your own UI you should set this
		true so that the printing system does not present its own UI.  
	    */
	    Boolean printerCanDuplexNoTumble = false;
	    Boolean printerCanDuplexTumble = true;
	    Boolean requiresDifferentMarginsForOSFlippedSide2 = true;
	    Boolean haveOurOwnPDE = false;
	    result = addDuplexToTemplate(ourTemplate, printerCanDuplexNoTumble, printerCanDuplexTumble, 
	    									requiresDifferentMarginsForOSFlippedSide2, haveOurOwnPDE);
	}
#endif

#ifdef kPMDefaultReverseOutputOrderKey
	/*
	    This entry tells the printing system what order the default output paper tray stacks the paper. Specifying a value
	    of true for the default value of this template entry tells the printing system that the printer stacks the paper in reverse
	    order (N, N-1, ..., 2, 1). Specifying a value of false for the default value of this template entry tells the printing system that
	    the printer stacks the paper in Normal order (1, 2, ... N-1, N). The printing system uses this information to determine
	    the appropriate initial default value for reverse order printing for this printer.
	*/
	if(!result){
	    Boolean printerStacksOutputInReverseOrder = true;	// This entry is printer model dependent. Change appropriately for your printer model.
	    result = PMTemplateMakeEntry( ourTemplate, kPMDefaultReverseOutputOrderKey, kPMValueBoolean, kPMConstraintList) ;
	    if (!result)
		result = PMTemplateSetBooleanDefaultValue( ourTemplate, kPMDefaultReverseOutputOrderKey, printerStacksOutputInReverseOrder) ;
	}
#endif


	// Paper Sources. We allow only the standard input tray for now.
	
	if ( result == noErr )
		result = PMTemplateMakeEntry( ourTemplate, kPMPaperSourceKey, kPMValueSInt32, kPMConstraintList ) ;
	if ( result == noErr )
		result = PMTemplateSetSInt32DefaultValue( ourTemplate, kPMPaperSourceKey, 1 ) ;
	ourSInt32s[0] = 1 ; 
	if ( result == noErr )
		result = PMTemplateSetSInt32ListConstraint( ourTemplate, kPMPaperSourceKey, 1, &ourSInt32s[0] ) ;
	
    // Get a list of ColorSync profile info dictionaries.
	if ( result == noErr )
        result = hpCreateProfileDict( &ourProfileDict );
        
	if ( result == noErr )
    {
        // Get the default profile ID, a count of profiles (IDs), and a list of profile IDs
        result = hpCopyEntriesFromProfileDict( ourProfileDict, &ourDefaultProfileID,
                                                NULL, &ourProfileCount, &ourProfileIDList);

        CFRelease(ourProfileDict);
        ourProfileDict = NULL;
    
        // Set the default profile ID and its constraint list
        if ( result == noErr )
                result = PMTemplateMakeEntry( ourTemplate, kPMColorSyncProfileIDKey,
                                            kPMValueSInt32, kPMConstraintList ) ;
        if ( result == noErr )
            result = PMTemplateSetSInt32DefaultValue( ourTemplate, kPMColorSyncProfileIDKey,
                                                        ourDefaultProfileID ) ;
        if ( result == noErr )
            result = PMTemplateSetSInt32ListConstraint( ourTemplate, kPMColorSyncProfileIDKey,
                                                        ourProfileCount, ourProfileIDList ) ;

        // Free the list if it was allocated (by hpCopyEntriesFromProfileDict)
        if (ourProfileIDList != NULL)
            free (ourProfileIDList);
    }
    else if ( result == kPMProfileDictNotFound ) 
		result = noErr;

	// Paper sizes: we'll support letter, legal, A4, B5, and a vendor specific size. 
    // Paper sizes are now localized using the localizable.strings file under the print
    // framework. Please see the constants defined in PMPrinterModule.h for all currently 
	// standard paper names.  
	if ( result == noErr )
		result = PMTemplateMakeEntry( ourTemplate, kPMPaperInfoList, kPMValueTicket, 
										kPMConstraintList ) ;
                                        
	// Create the list ticket of papers.
	if ( result == noErr )
		result = PMTicketCreate( CFAllocatorGetDefault(), kPMTicketList, &ourListTicket ) ;

	// If we were able to create a paper info, then fill it with the values needed to define the
	// letter size paper. This will be our default paper size for the PSPM.
	
    if ( result == noErr )
        result = AddPaperToList( ourListTicket, 1, USLetter, 0.0, 0.0, 576., 756., -18., -18., 594., 774. ) ;
	// Need to add the first paper as the default paper.
    if ( result == noErr )
        result = PMTicketGetTicket( ourListTicket, kPMPaperInfoTicket, 1, &ourPaperInfoTicket ) ;
    if ( result == noErr )
		result = PMTemplateSetPMTicketDefaultValue( ourTemplate, kPMPaperInfoList, ourPaperInfoTicket ) ;

    // Continue to add papers to the template.
    if ( result == noErr )
        result = AddPaperToList( ourListTicket, 2, USLegal, 0.0, 0.0, 576., 972., -18., -18., 594., 990. ) ;
    if ( result == noErr )
        result = AddPaperToList( ourListTicket, 3, A4, 0.0, 0.0, 595., 842., -18., -18., 613., 860. ) ;
    if ( result == noErr )
        result = AddPaperToList( ourListTicket, 4, B5, 0.0, 0.0, 516., 728., -18., -18., 534., 746. ) ;
    if ( result == noErr )
        result = AddPaperToList( ourListTicket, 5, CFSTR("HPPOSTCARD"), 0.0, 0.0, 360., 288., -18., -18., 378., 306. ) ;

	// Ok, we have our paper sizes stored as tickets in our list ticket. From here, we
	// can save this list ticket as the constraint for Paper Info.

	if ( result == noErr )
		result = PMTemplateSetPMTicketListConstraint( ourTemplate, kPMPaperInfoList, ourListTicket ) ;
	if ( result == noErr )
		result = PMTicketReleaseAndClear( &ourListTicket ) ;
	
#if DOCUSTOMPAPER
        if( result == noErr){
            result = addCustomPaperToTemplate(ourTemplate);
            
        }
#endif
        
	// For now, we return the template as far as we have it.
		
	if ( result == noErr )
		*pmTemplate = ourTemplate ;	
	else
		PMTemplateDelete( &ourTemplate ) ;
	return( result ) ;
	}

#if DOCUSTOMPAPER
/*
    For this example we are claiming support for custom paper size for widths of
    2 to 11 inches and heights from 2 to 17 inches. The margins that are imposed on
    each custom size are 18 points on the top and bottom and 9 points on the
    left and right side.
*/
static OSStatus addCustomPaperToTemplate(PMTemplateRef pmTemplate)
{
    OSStatus err = noErr;
    PMRect marginRect;
    double minWidth, maxWidth, minHeight, maxHeight;
    
    // NOTE: these must all be POSITIVE values, i.e. these are absolute distances
    marginRect.top = 18;	// required hardware margin for top edge in points
    marginRect.bottom = 18;	// required hardware margin for bottom edge in points	
    marginRect.left = 9;	// required hardware margin for left edge in points
    marginRect.right = 9;	// required hardware margin for right edge in points
    
    minWidth = 2*72;		// minimum paper width in points
    maxWidth = 11*72;		// maximum paper width in points
  
    minHeight = 2*72;		// minimum paper height in points
    maxHeight = 17*72;		// maximum paper height in points

    // make entry for width of custom page size
    if(!err)
	err = PMTemplateMakeEntry( pmTemplate, kPMCustomPageWidthKey, kPMValueDouble, kPMConstraintRange); 
    if (!err)
        err = PMTemplateSetDoubleRangeConstraint( pmTemplate, kPMCustomPageWidthKey, minWidth, maxWidth);

    // make entry for height of custom page size
    if(!err)
	err = PMTemplateMakeEntry( pmTemplate, kPMCustomPageHeightKey,  kPMValueDouble, kPMConstraintRange); 
    if (!err)
        err = PMTemplateSetDoubleRangeConstraint( pmTemplate, kPMCustomPageHeightKey, minHeight, maxHeight);

    // make entry for margins
    if(!err)
	err = PMTemplateMakeEntry( pmTemplate, kPMCustomPageMarginsKey,  kPMValuePMRect,  kPMConstraintPrivate); 

    if(!err)
	err = PMTemplateSetPMRectDefaultValue (pmTemplate, kPMCustomPageMarginsKey, &marginRect);

    return err;
}

#endif


/*****************************************************************************
	Name:			hpCreateConverterSetup
	
		Setup raster converter with resolution, banding, color format information.
		Uses the JobTicket to determine the right converterSetup settings.
		
	Parameters:
		jobTicket 		- The control parameters for this job, as defined by the application and user.
						
		paperType 		- Type of paper we are printing on to; it might effect converterSetup. 

		quality 		- Print quality impacts converter setup.
        
        profileID		- ID of default profile, to be verified and put in converter settings.

		converterSetup 	- Ticket that holds the converter settings. 
		
		return value 	- OSStatus, contains errors if there were any.
		

	Description:

	History:

*****************************************************************************/
OSStatus hpCreateConverterSetup ( PMTicketRef jobTicket, SInt32 paperType, SInt32 quality, SInt32 profileID,
								PMTicketRef *converterSetup)
{

	OSStatus 				osStatus = noErr;
	float 					xResolution, yResolution;
	PMTicketRef				converterSetupTicket=NULL;
       
	if( converterSetup == NULL)
		osStatus  = kPMInvalidTicket;

	// Create the ConverterSetup ticket
	if (osStatus == noErr)
		osStatus = PMTicketCreate(CFAllocatorGetDefault(), kPMConverterSetupTicket, &converterSetupTicket);

	if (quality == kPMQualityBest)
		xResolution = yResolution = kBestResolution;
	else if (quality == kPMQualityNormal)
		xResolution = yResolution = kNormalResolution;
	else 
		xResolution = yResolution = kDraftResolution;

	// Set resolution based on print quality mode as set by our PDE.  	
  	if( osStatus == noErr && converterSetupTicket != NULL)
	{	
		osStatus = PMTicketSetDouble( converterSetupTicket, kRasterPMBundleID, 
                        kPMConverterResHorizontalKey, xResolution, kPMLocked);
		osStatus = PMTicketSetDouble( converterSetupTicket, kRasterPMBundleID, 
                        kPMConverterResVerticalKey, yResolution, kPMLocked);
	}

	// Set banding & band height:
  	if( osStatus == noErr  && converterSetupTicket != NULL)
	{
		osStatus = PMTicketSetBoolean( converterSetupTicket, kRasterPMBundleID, 
                            kPMBandingRequestedKey, true, kPMLocked);

 		osStatus = PMTicketSetSInt32 ( converterSetupTicket, kRasterPMBundleID, 
                        kPMRequiredBandHeightKey, kDraftQualityBandSize, kPMLocked );
	}
 
	// Set Pixel format and layout.
  	if( osStatus == noErr && converterSetupTicket != NULL)
	{
		osStatus = PMTicketSetSInt32( converterSetupTicket, kRasterPMBundleID,
                        kPMRequestedPixelFormatKey, kPMXRGB_32, kPMLocked );

		osStatus = PMTicketSetSInt32( converterSetupTicket, kRasterPMBundleID, 
                        kPMRequestedPixelLayoutKey, kPMDataChunky, kPMLocked );
	}
    
	// Set ColorSync Profile ID.
    // Here is where the PM can override the setting passed in, which is based
    // on defaults.  The profile ID that we store in the converterSetupTicket
    // is the one that will actually be used when doing color matching.
    //
  	if( osStatus == noErr && converterSetupTicket != NULL)
	{
        osStatus = hpVerifyColorSyncProfileID(jobTicket, paperType, quality, &profileID);
        if (osStatus == noErr && profileID != 0)
            osStatus = PMTicketSetSInt32( converterSetupTicket, kRasterPMBundleID,
                        kPMCVColorSyncProfileIDKey, profileID, kPMLocked );
	}
    
	// Set the passed argument to our converterSetup ticket just created
	if( osStatus == noErr)
		*converterSetup = converterSetupTicket;
	else
	{
		if (converterSetupTicket != NULL)
			PMTicketReleaseAndClear( &converterSetupTicket);
		*converterSetup = NULL;
	}
		
	return osStatus;
}

/*****************************************************************************
	Name:			hpVerifyColorSyncProfileID
	
		The printer module "knows" which is the correct profile ID to use based
        on the other job settings.  The profile ID that is passed in may need
        to be reset to the proper one - hence a pointer to it is passed in.
		
	Parameters:
		jobTicket 		- The control parameters for this job, as defined by the application and user.
						
		paperType 		- Type of paper we are printing on impacts ColorSync Profile Selection. 

		quality 		- Print quality impacts ColorSync Profile Selection.
        
        profileID		- Ptr to ID of default profile, to be changed if need be.

		return value 	- OSStatus, contains errors if there were any.
		

	Description:

	History:

*****************************************************************************/
OSStatus hpVerifyColorSyncProfileID ( PMTicketRef jobTicket, SInt32 paperType, SInt32 quality, SInt32 *profileID )
{

	OSStatus 				osStatus = noErr;
	/*
	    The value of *profileID passed in is that obtained from the print settings or is 0 if it could not be
            obtained from the print settings. The only reason we would override the value in the print settings
            is if it didn't make sense for this print job.
               
            This routine should, based on the job ticket and other data passed in, determine which profileID 
	    should be used to handle this print job. If the data in the jobTicket that we would normally use 
	    to determine which profile to use is not available we should handle that gracefully and 
	    set profileID to an appropriate default.
            
            In all cases we should make sure that the profileID we return is valid for our printer. Since
            this sample code only registers one profile (with ID = 1) we always update the profileID passed
            in to point to our profile.
	*/
        *profileID = 1;	// this is the only profile ID we've registered in our sample PM
    return osStatus;
}

