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

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	File: PCLUtils.cp

	Contains: Implements internal utility routines for the HP Raster Printer Module
			
 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

 	Bugs?:		For bug reports, consult the following page on
 				the World Wide Web:
 
 					http://developer.apple.com/bugreporter/
	Change History:

	To Do:
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include <strings.h>

#include <ApplicationServices/ApplicationServices.h>

#include <PrintCore/PMPrinterModule.h>
#include <PrintCore/PMTicket.h>
#include <PrintCore/PMRaster.h>
#include "RasterModule.h"
#include "DeviceID.h"

#define	kUSBIOMGetDeviceID			CFSTR( "USBIOMGetDeviceID" )

// Possible error codes that can be returned in recoverable and non-recoverable error conditions
#define	kPaperJamErr 	-1000
#define	kOutOfPaperErr 	-1001
#define	kCoverOpenErr 	-1002

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Method: hpCreateEventDict

        Input Parameters:
                event
                
        Output Parameters:
                CFMutableDictionaryRef

        Description:
                Creates a dictionary for events containing the event code and error code.

        Change History:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CFMutableDictionaryRef hpCreateEventDict(SInt32 event, SInt32 statusCode)
{

	// Create the event dict
   	CFMutableDictionaryRef  returnDict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0,&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    if ( returnDict )
    {
		// create a CFNumber out of the event code and stuff into dict
		CFNumberRef cfNumber = CFNumberCreate(NULL, kCFNumberSInt32Type, &event);
		if (cfNumber != NULL)
		{
			CFDictionarySetValue(returnDict, kPMEventCodeKey, cfNumber );
			CFRelease( cfNumber);
		}

		// create a CFNumber out of the error code and stuff into dict
		if (statusCode != noErr)
		{
			CFNumberRef cfNumber = CFNumberCreate(NULL, kCFNumberSInt32Type, &statusCode);
			if (cfNumber != NULL)
			{
				CFDictionarySetValue(returnDict, kPMErrorCodeKey, cfNumber );
				CFRelease( cfNumber);
			}
		}
    }
    return returnDict;
        
} // hpCreateEventDict

/*--------------------------------------------------------------------------------------------
	Function: hpCheckDeviceIDStatus()

	Description:
		Hewlett-Packard DeskJet printers report some rudimentary status using a VSTATUS key 
		of the USB DeviceID class request. This is a vendor extension of IEEE-1284. It may
		not be available for your printer.

		We use this status since we can issue the DeviceId request at any time during the print job.
		Your printer module may need to be more sophisticated about interweaving status requests
		with the print job and parsing data retreived from the read pipe.

		The status fields we examine in this sample code are basically paper jam, paper out, and cover
		open. Our implementation is very dependent on the non-positional nature of this information in
		VSTATUS -- if the target string is found anywhere in the VSTATUS field, we note the condition
		is present in the printer. While adequate for sample code, this is not the best way to get
		status from the device, and may not account for all permutations of the VSTATUS field. 
        This code may report false errors on some DeskJet printer models, and it may miss other 
        errors.

	Parameters:
		ourContext 	- our private context data.
 		job 		- Identifies the job this PM is being used with.
        deviceId	- the IEEE-1284 which has been retrieved from the printer
        condition	- a value of the VSTATUS field that we're looking for
        errmsg		- the non-localized error message we report if the condition is found
        *state		- track the condition since the last time we reported
		
	History:
--------------------------------------------------------------------------------------------*/
static Boolean
hpCheckDeviceIDStatus( PrivateData *ourContext, void *job, CFStringRef deviceId, CFStringRef condition, CFStringRef errmsg, Boolean *state )
{
    CFRange		range;					// look for substring in the VSTATUS field of DeviceID
    Boolean		newState;				// whether the condition was found or not
    Boolean		change;					// whether the condition has changed since we last looked
	
    range = DeviceIDSupportedValue( condition, deviceId, CFSTR("VSTATUS:"), NULL );
    newState = range.length > 0;
    
    change = newState ^ *state;
    if ( change )
    {
        //
        //	The <condition> string has just been discovered in the deviceId string.
        //
		
        // Put together a notification dictionary with the new status
        // and pass it along to the printing manager.
        CFMutableDictionaryRef 	notificationDict = NULL;
		CFDictionaryRef 		notificationReplyDict = NULL;
		
        /* if the condition is idle or printing, send a normal kPMEventPrinterStatus event and return. */
		if ( (CFStringCompare ( condition, CFSTR("IDLE"), kCFCompareCaseInsensitive) == kCFCompareEqualTo) ||
			  (CFStringCompare ( condition, CFSTR("PRNT"), kCFCompareCaseInsensitive) == kCFCompareEqualTo) )
			notificationDict = hpCreateEventDict(kPMEventPrinterStatus, noErr);
		
		// if the condition is "paper jam", we will consider it a fatal error; i.e. non-recoverable.
		// We will send kPMEventErrorOccurred and stop this job and leave in the queue as a failed job.
		else if (CFStringCompare ( condition, CFSTR("PAPS"), kCFCompareCaseInsensitive) == kCFCompareEqualTo)		
			notificationDict = hpCreateEventDict(kPMEventErrorOccurred, kPaperJamErr);
		
		// if the condition is "cover open", we need to look at the previous state and 
		// send a kPMEventRecoverableErrorOccurred or kPMEventRecoverableErrorCleared event accordingly
		else if (CFStringCompare ( condition, CFSTR("UP"), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
		{
			// check the previous state and the new state and send the right event
			if (*state == false && newState == true)
				notificationDict = hpCreateEventDict( kPMEventRecoverableErrorOccurred, kCoverOpenErr);
			else if (*state == true && newState == false)
				notificationDict = hpCreateEventDict(kPMEventRecoverableErrorCleared, noErr);
					
			// add another key to indicate which error we are clearing.  We are specifically
			// clearing kCoverOpenErr error.  We do that by adding a kPMEventContextKey to the dict.
			SInt32 errToClear = kCoverOpenErr;
			CFNumberRef cfNumber = CFNumberCreate(NULL, kCFNumberSInt32Type, &errToClear);
			if (cfNumber != NULL && notificationDict != NULL)
			{
				CFDictionarySetValue(notificationDict, kPMEventContextKey, cfNumber );
				CFRelease( cfNumber);
			}
		}
		
		// if the condition is "out of paper", we will pretend that we cannot continue until
		// the condition is cleared by the user and we receive a "Continue" reply.  This may not
		// be the best example for demonstrating the use of kPMEventBlockingRecoverableErrorOccurred!
		else if (CFStringCompare ( condition, CFSTR("OOPA"), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
		{
			// check the previous state and the new state and send the right event
			if (*state == false && newState == true)
				notificationDict = hpCreateEventDict(kPMEventRecoverableErrorOccurred, kOutOfPaperErr);
		}
		
        if ( NULL != notificationDict )
        {
            //
            //	When IPP support is enabled, we'll want to iterate through the supplied list
            //	of languages to generate the preferred localized strings for the client.
            //
            //	For now we'll just use the local preference for language.
            //
            CFStringRef localizedErrMessage = CFBundleCopyLocalizedString( ourContext->bundleRef, errmsg, errmsg, NULL );
            CFDictionarySetValue(notificationDict, kPMErrorTextKey, localizedErrMessage );
	
			// send the event to the printing manager via notifyProc.  This routine will return immediately
			// unless we are sending a kPMEventBlockingRecoverableErrorOccurred event.
           (void) (*ourContext->notifyProc)( job, notificationDict, &notificationReplyDict);

			if (notificationReplyDict)
			{
				// Currently, blocking recoverable errors are not supported, so just
				// you shouldn't receive a reply dict. If you do, just release it.				            
				CFRelease( notificationReplyDict );
			}
		
			// release what we allocated here
            CFRelease( localizedErrMessage );
            CFRelease( notificationDict );
        }

        *state = newState;
    }
    
    return change;
}

/*--------------------------------------------------------------------------------------------
	Function: hpRasterGetStatus()

	Description:
        Interpret the status from the printer and map to a known PM error to allow
        the printing manager know what to do with it.

        Translate printer string to English first then localize it before passing back.

        Finally, send status back to the Printing Manager. 

	Parameters:
		ourContext 	- our private context data.
								
		job 		- Identifies the job this PM is being used with.

		
	History:
--------------------------------------------------------------------------------------------*/
static void
hpRasterGetStatus( PrivateData *ourContext, void *job )
{
    CFStringRef deviceId;

    /*
    **	Query the USB connection for the Device ID string
    */
	OSStatus	result = (ourContext->ioProcs).PMIOGetAttributeProc( job, kUSBIOMGetDeviceID, (CFTypeRef *) &deviceId );

    /*
    **	This convenience macro is defined to reorder args so that we can use genstrings to 
    **		generate the localized string list
    */
#define LocalizedDeviceIDStatusString( errorKey, deviceIdCondition, n ) \
    hpCheckDeviceIDStatus( ourContext, job, deviceId, CFSTR(deviceIdCondition), CFSTR(errorKey), n )
   
    if ( noErr == result && NULL != deviceId )
    {
        int changed = 0;

        /* idle */
        changed |= LocalizedDeviceIDStatusString( "Idle", "IDLE", &ourContext->isIdle );

        /* printing */
        changed |= LocalizedDeviceIDStatusString( "Printing job…", "PRNT", &ourContext->isBusy );

        /* cover open */
        changed |= LocalizedDeviceIDStatusString( "Please close the cover.", "UP", &ourContext->isCoverOpen );

		/* out of paper */
        changed |= LocalizedDeviceIDStatusString( "Please add paper.", "OOPA", &ourContext->isPaperOut );

		/* paper jam */
        changed |= LocalizedDeviceIDStatusString( "The paper has jammed. Please clear the paper path.", "PAPS", &ourContext->isPaperJam );
	}
    
    if ( NULL != deviceId )
        CFRelease( deviceId );
}


/*--------------------------------------------------------------------------------------------
	Function: hpStartEngine()

	Description:
		Internal routine used to start enginePCL and start a print job		

	Parameters:
		pmContext 	- our private context data.
								
		jobContext 		- Identifies the job this PM is being used with.

		paperType - Setup engine and printer for this paper type
		
		quality - Setup engine and printer for this print quality
		
	History:
--------------------------------------------------------------------------------------------*/
OSStatus hpStartEngine( PMContext *Context, const void *jobContext, SInt32 paperType, SInt32 quality)
{
	OSStatus 			osStatus = noErr ;
	PrivateData			*ourContext = NULL ;		

	if ( Context == NULL )
		osStatus = kPMInvalidParameter ;
	else
		ourContext = (PrivateData*)Context ;
	
	if (osStatus == noErr)
	{	
		ourContext->enginePCL = new TEnginePCL;
		ourContext->enginePCL->Initialize( (void *)jobContext);
		ourContext->enginePCL->OpenJob( paperType, quality, &ourContext->ioProcs ) ;
	}
	
	return osStatus;
}


/*--------------------------------------------------------------------------------------------
	Function: hpPrintPageBands()

	Description:
		Internal routine used to process the individual bands of a single page. Called by
		PMPrintPage.
		
	Parameters:
		pmContext 	- our private context data.
						
		jobContext 		- Identifies the job this PM is being used with.

		jobTicket 	- The control parameters for this job, as defined by the application and user.

		pmJobStreamGetNextDataRasterBandProc - Procedure called to get a raster band of data from print mgr.
				
	History:
		
--------------------------------------------------------------------------------------------*/
OSStatus hpPrintPageBands( PMContext *Context, const void *jobContext, PMTicketRef jobTicket, 
						PMJobStreamGetNextBandProcPtr pmJobStreamGetNextBandProc)
{
	OSStatus 			result = noErr ;
	UInt32 				scanLine;
	PrivateData			*ourContext = NULL ;		
	PMRasterBand		pmRasterBand;				// Data block coming from Printing Manager.
	SInt32				numCopies = 0 ;				// Number of copies we print of this page.
	UInt32 				pageWidth = 0, pageHeight = 0;
	SInt32				resolution = 0 ;			// # of pixels per inch
	PMRect				unadjustedPage ;			// Holds size of page in 72nds (points)
	SInt32				quality = 0 ;				// draft, normal, and best
    PMTicketRef			printSettingsTicket = NULL, paperInfoTicket = NULL;

	if ( Context == NULL )
		result = kPMInvalidParameter ;
	else
		ourContext = (PrivateData*)Context ;

	// First, we need to get the page dimensions (width and height in pixels).  This requires getting
	// the unadjusted page rect in jobticket->printsettings->paperInfo ticket, determing the width
	// and height of the rectangle (in dpis) and converting to pixels.
	if ( result == noErr )
		result = PMTicketGetTicket( jobTicket, kPMPrintSettingsTicket, kPMTopLevel, &printSettingsTicket );
	if ( result == noErr && printSettingsTicket != NULL)
		result = PMTicketGetTicket( printSettingsTicket, kPMPaperInfoTicket, kPMTopLevel, &paperInfoTicket );
	if ( result == noErr && paperInfoTicket != NULL)
		result = PMTicketGetPMRect( paperInfoTicket, kPMTopLevel, kPMTopLevel, kPMUnadjustedPageRectKey, 
									&unadjustedPage ) ;

	// calculate page width and height in pixels based on the resolution which is based on the resolution
	PMTicketGetSInt32( jobTicket, kPMTopLevel, kPMTopLevel, kPMQualityKey, &quality ) ;
	if (quality == kPMQualityBest)
		resolution = kBestResolution;
	else if (quality == kPMQualityNormal)
		resolution = kNormalResolution;
	else
		resolution = kDraftResolution;
	
	if ( result == noErr )
	{
		pageWidth = (UInt32) (unadjustedPage.right - unadjustedPage.left) ;
		pageWidth = (UInt32) (((float)pageWidth * resolution ) / 72) ;		// Convert to dpi.
	}
		
	if ( result == noErr )
	{
		pageHeight = (UInt32) (unadjustedPage.bottom - unadjustedPage.top);
		pageHeight = (UInt32) (((float)pageHeight * resolution ) / 72) ;
	}

	// Read first data band from the Printing Manager.
	if ( result == noErr )
	{
		result = pmJobStreamGetNextBandProc( jobContext, &pmRasterBand ) ;
		if ( result == noErr || ( result == eofErr && pmRasterBand.size > 0 ))
		{
			// Get number of copies from the job ticket to let EnginePCL know
			PMTicketGetSInt32( jobTicket, kPMTopLevel, kPMTopLevel, kPMCopiesKey, &numCopies ) ;

			// Set res file to ours since we need to load resource at open page time.
         	UseResFile( ourContext->resFile);
#if !NOOUTPUT
			// Open PCL Page
			ourContext->enginePCL->OpenPage( numCopies, pmRasterBand.depth ) ;
#endif
		}
	}
		
		
	// Once the first band is read, we can loop through the remaining bands until there are
	// no more bands to be dealt with. This is taken care of by our utility funciton. 
	
	// Loops thru all bands in current page
	for ( scanLine = pmRasterBand.height, result = noErr; 
			result == noErr && scanLine<=pageHeight && !ourContext->abort; 
			scanLine+= pmRasterBand.height)
	{

#if !NOOUTPUT
		// print the band after doing some processing
		// For "kPMRGBX_32_Sep_Gray_8" color mode, the band depth alternates between
		// RGBX and 8-bit gray. So watch out for the depth of eacch band in PrintBand
		ourContext->enginePCL->PrintBand( pmRasterBand.baseAddress, pmRasterBand.size, pageWidth,
				pmRasterBand.height, pmRasterBand.depth);
		
		// check for printer errors and report status to Printing Manager
        if ( ourContext->deviceIdQueryOkay )
            hpRasterGetStatus( ourContext, (void *)jobContext );
#endif	
		// read next band data, if the current one is not the last
		if (pmRasterBand.order != kPMLastBand && pmRasterBand.order != kPMLoneBand)
		{
			result = pmJobStreamGetNextBandProc( jobContext, &pmRasterBand);
			if (result == eofErr && pmRasterBand.size > 0)
				 result = noErr;
		}
	}
	
	// If the result we got back is end of file, we need to clear the error. That one doesn't count.
	if ( result == eofErr )
		result = noErr ;
		
#if !NOOUTPUT
	// Once that is complete, we ask EnginePCL to close the page.
	ourContext->enginePCL->ClosePage( false ) ;
#endif

	return result;
}

/*--------------------------------------------------------------------------------------------
	Function: hpStopEngine()

	Description:
		Internal routine used to close job and stop enginePCL		

	Parameters:
		pmContext 	- our private context data.
										
	History:
--------------------------------------------------------------------------------------------*/
OSStatus hpStopEngine( PMContext *Context)
{
	OSStatus 			osStatus = noErr ;
	PrivateData			*ourContext = NULL ;		

	if ( Context == NULL )
		osStatus = kPMInvalidParameter ;
	else
		ourContext = (PrivateData*)Context ;
	
	if (osStatus == noErr && ourContext->enginePCL != NULL)
	{
		ourContext->enginePCL->CloseJob() ;
		delete( ourContext->enginePCL ) ;
	}
	
	return osStatus;
}
