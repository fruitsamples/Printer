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
	File: RasterModule.cp

	Contains: Implements the Printer Module API for a single raster printer, intended to
		bes used as a basis for 3rd party printer modules. 
		
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

#include <Carbon/Carbon.h>
#include <PrintCore/PMPrinterModule.h>
#include <PrintCore/PMIOModule.h>
#include <PrintCore/PMTicket.h>
#include <PrintCore/PMErrors.h>
#include <PrintCore/PMRaster.h>
#include "RasterDefines.h"

// FUNCTIONS

/*--------------------------------------------------------------------------------------------
	Function: CreatePrinterBrowserModuleInfo()

	Description:
		For a given connection type, returns a ticket containing printer browser
		information about the supported printers that can be browsed for on that
		connection.  This info includes: icons, printer model and kind (language).
		If the connection is not supported, kPMUnsupportedConnection is returned.

		This API is called at printer discovery time and before a printer address is
		found.  Therefore, it can be called outside of Initialize/Terminate.
		
	Parameters:
		connectionType - 		Connection type on which browser info is requested.

		printerBrowserInfo - 	Browser information which includes PrinterModel, language,
								and icon data.
																		
		return value - OSStatus, contains errors if there were any.
		
	History:
		
--------------------------------------------------------------------------------------------*/

OSStatus CreatePrinterBrowserModuleInfo(CFStringRef connectionType, CFArrayRef *printerBrowserInfo)
{
	OSStatus 				osStatus = noErr;
	CFMutableDictionaryRef 	browserInfo1 = NULL, browserInfo2 = NULL;
	CFDataRef				cfIconData = NULL;
	CFMutableArrayRef		cfArrayOfPrinters = NULL;
 
	if (printerBrowserInfo != NULL)
	{
		*printerBrowserInfo =  NULL;

		// Only USB is supported by this printer
		if ((CFStringCompare ( connectionType, kPMUSBConnection, 
				kCFCompareCaseInsensitive) == kCFCompareEqualTo))
		{

			// Load icon data from our resource file which we will later stuff into browserInfo dictionary
			osStatus = hpCreateIconData( &cfIconData);

			// create a dictionary to hold BrowserInfo for a single printer
			if (osStatus == noErr)			
				browserInfo1 = CFDictionaryCreateMutable( CFAllocatorGetDefault(), 0, 
						&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
						
			if ( browserInfo1 != NULL)
			{
				// Set our printermodel and printerlanguage into the dictionary
				CFDictionarySetValue( browserInfo1, kPMPrinterBrowserKindKey, kRasterPrinterBrowserKindValue);
				CFDictionarySetValue( browserInfo1, kPMPrinterBrowserInfoKey, kRasterPrinterBrowserInfoValue);
				CFDictionarySetValue( browserInfo1, kPMPrinterBrowserDeviceIDKey, kRasterPrinterBrowserDeviceIDValue );
	
				// Stuff icon data into dictionary as well
				CFDictionarySetValue( browserInfo1, kPMPrinterBrowserIconsKey, cfIconData);
			}
			else
				osStatus = memFullErr;
				
			// create another dictionary to hold BrowserInfo for another printer - just to illustrate how this 
			// is done.  In fact, I'm adding info to an old DeskWriter that may not be even be supported by this PM.
			// Ideally, each dict should have browser info to browse for a single printer model with
			// unique device ID, icon, model, etc..
			if (osStatus == noErr)			
				browserInfo2 = CFDictionaryCreateMutable( CFAllocatorGetDefault(), 0, 
						&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
						
			if ( browserInfo2 != NULL)
			{
				// Set our printermodel and printerlanguage into the dictionary
				CFDictionarySetValue( browserInfo2, kPMPrinterBrowserKindKey, kRasterPrinterBrowserKindValue);
				CFDictionarySetValue( browserInfo2, kPMPrinterBrowserInfoKey, kRasterPrinterBrowserInfoValue);
				CFDictionarySetValue( browserInfo2, kPMPrinterBrowserDeviceIDKey, CFSTR("MDL:DeskWriter") );
	
				// stuff same icon data into second dictionary
				CFDictionarySetValue( browserInfo2, kPMPrinterBrowserIconsKey, cfIconData);
			}
			else
				osStatus = memFullErr;
				
			// Create an array with a capacity for many BrowserInfo dictionaries. 
			if (osStatus == noErr)			
				cfArrayOfPrinters = CFArrayCreateMutable( CFAllocatorGetDefault(), 0, &kCFTypeArrayCallBacks);
	
			// stuff our dictionary into it.
			if ( cfArrayOfPrinters != NULL)
			{
				CFArrayAppendValue( cfArrayOfPrinters, (const void *) browserInfo1);
				CFArrayAppendValue( cfArrayOfPrinters, (const void *) browserInfo2);
			}
			else
				osStatus = memFullErr;
				
			// release icon data
			if (cfIconData != NULL)
				CFRelease ( cfIconData);
			// Release the browser Info dict(s)
			if (browserInfo1 != NULL)
				CFRelease( browserInfo1);
			if (browserInfo2 != NULL)
				CFRelease( browserInfo2);
		
			// return the array to caller - which may be null if we failed to construct the array above
			*printerBrowserInfo =  cfArrayOfPrinters;
		}
		else
			osStatus = kPMUnsupportedConnection;
	}
	else
		osStatus = kPMInvalidParameter;

	return osStatus;
}


/*--------------------------------------------------------------------------------------------
	Function: Initialize()

	Description:
		Create a new instance of a Raster Printer Module and place it's reference in a
		caller supplied variable. The new printer module is bound to the printer address supplied
		by the caller. We'll receive this context each time a new job is submitted, or when printer
		tickets/PDE paths are requested.
		
		The private data block is used for storing internal printer module data and Printing Manager 
		parameters passed to this routine that are needed in subsequent PM calls; such as pointers
		to IO module routines, pointer to a notification callback routine, and printer address.
		
	Parameters:
		printerAddr 	- The printer which we'll be printing to. 
						
		jobContext		- Identifies the job this PM is going to be used with. 
						
		PMIOProcs 		- IO functions to use when communicating with printer.
		
		PMNotificationProc - Called to report printer status to Printing Manager when needed.
																
		PMContext 		- Memory allocated by the caller to hold a PMContext created by this
							function. 
												
		return value - OSStatus, contains errors if there were any.
		
	History:
		
--------------------------------------------------------------------------------------------*/
OSStatus Initialize( const CFDataRef printerAddress, const void *jobContext, const PMIOProcs *pmIOProcs, 
	const PMNotificationProcPtr pmNotificationProc, PMContext *pmContext )
	{
	OSStatus		result = noErr ;
	PrivateData		*ourContext = NULL ;
	
	
	// Start by allocating our private data structure that serves as the context. We'll fill it
	// in with the relevent stuff later.
	
	ourContext = (PrivateData*)CFAllocatorAllocate( CFAllocatorGetDefault(), sizeof(*ourContext), kNoHints ) ;
	if ( ourContext == NULL)
		result = memFullErr ;

	// store the IOProcs, notification proc and its context into our private data so we can report job 
	// status in PrintPage/PrintJob, and possibly in CreatePrinterTickets and CreatePrintingDialogExtensionsPaths
	if ( result == noErr )
	{
		ourContext->ioProcs = *pmIOProcs ;	
		ourContext->notifyProc = pmNotificationProc ;	// Copy in the notification proc pointer.
	}

	if ( result == noErr )
    {
        //
        //	This printer module only works with USB connections --
        //		we know that the printerAddress is actually a CFStringRef.
        //
        //	DeskJet 800-series printers will not be queried for status.
        //
#define kPMDeskJet8 CFSTR("DESKJET 8")

        CFStringRef	printer = CFStringCreateFromExternalRepresentation( kCFAllocatorDefault, printerAddress, kCFStringEncodingUTF8 );
        
        ourContext->deviceIdQueryOkay = true;	// assume we can query
        if ( NULL != printer )
        {
			CFRange		bounds;
			CFRange 	where;

			bounds = CFRangeMake( 0, CFStringGetLength( printer));
            if ( CFStringFindWithOptions( printer, kPMDeskJet8, bounds, kCFCompareCaseInsensitive, &where ) )
                ourContext->deviceIdQueryOkay = false;
			CFRelease( printer );
        }
    }
    
	// reset the abort flag before any job starts.
	if ( result == noErr )
		ourContext->abort = false;

	// Allocate an array of references to images for ImageAccess processing
	if ( result == noErr )
	{
		ourContext->imageRefArray = CFArrayCreateMutable( CFAllocatorGetDefault(), 0, &kCFTypeArrayCallBacks);
		if ( ourContext->imageRefArray == NULL)
			result = memFullErr ;
	}
	
	// reset error status flags
	if ( result == noErr )
	{
		ourContext->isIdle = false;
		ourContext->isBusy = false;
		ourContext->isCoverOpen = false;
		ourContext->isPaperOut = false;
		ourContext->isPaperJam = false;
	}
	
	// Check for errors at this point. If there are any, then free up whatever we allocated, 
	// including the printer address block whose pointer is supposed to be in our context.
	if ( result != noErr )
	{
		if ( ourContext != NULL )
			CFAllocatorDeallocate( CFAllocatorGetDefault(), ourContext ) ;
		
		*pmContext = NULL ;
	}
	else 
		// Return our context for the caller.
		*pmContext = (PMContext) ourContext;

	return( result ) ;
}

/*--------------------------------------------------------------------------------------------
	Function: CreatePrintingDialogExtensionsPaths()

	Description:
		return one or more paths of this Printer Module's Print Dialog Extension(s).	
		RasterPM PDE(s) is/are located in the same folder as the PM itself. 
		All third party PDEs live in /Library/Printers/ path so returned paths should
		be relative to that path.
	
		This API is called after Initialize to allow the PM to return different PDE's per
		printer model/address.

	Parameters:
		pmContext 	- our private context data.
														
		pdePaths - 	Array of CFStrings each specifying a path name to a PDE plugin.
																		
		return value - OSStatus, contains errors if there were any.
		
	History:
		
--------------------------------------------------------------------------------------------*/

OSStatus CreatePrintingDialogExtensionsPaths(PMContext pmContext, CFArrayRef *pdePaths)
{
	OSStatus 	osStatus = noErr;

	CFMutableArrayRef cfArray = CFArrayCreateMutable( CFAllocatorGetDefault(), 0, &kCFTypeArrayCallBacks);

	// stuff our PDEs into it.
	if ( cfArray != NULL && pdePaths != NULL)
	{
		CFArrayAppendValue( cfArray, (const void *) CFSTR("SamplePM/PrintQuality.plugin"));
		*pdePaths =  cfArray;
	}
	else
		osStatus = memFullErr;
	     
	return osStatus;
}


/*--------------------------------------------------------------------------------------------
	Function: CreatePrinterTickets()

	Description:
		returns printer module's template and printerinfo tickets to the caller.  
		A PrinterInfo ticket holds capability and product ID information whereas
		a Template holds range and default information about various print settings.
		Both the ticket and the template objects are created and populated with APIs 
		in PMTicket.h and PMTemplate.h printing headers.
		
	Parameters:
		pmContext 	- our private context data so there can be multiple sessions going at once.
						
		printerInfo - Pointer to ticket ref for our ticket once we've allocated it.

		pmTemplate 	- Pointer to template ref.
												
		return value - OSStatus, contains errors if there were any.
		
	History:
		
--------------------------------------------------------------------------------------------*/
OSStatus CreatePrinterTickets(PMContext pmContext, PMTicketRef *printerInfo, PMTemplateRef *pmTemplate)
{
	OSStatus		result = noErr ;

	// We use internal routines for PrinterInfo and Template creation
	if ( result == noErr && printerInfo != NULL)
            result = hpCreatePrinterInfo( pmContext, printerInfo);
            
	if ( result == noErr && pmTemplate != NULL)
            result = hpCreatePMTemplate( pmContext, pmTemplate);
            
	return result;
}


/*--------------------------------------------------------------------------------------------
	Function: BeginJob()

	Description:
		Called by the Printing Manager to initiate a job. This function is intended as a
		"setup" function, so all the information needed to set up for printing is supplied.
		
	Parameters:
		pmContext 	- our private context data so there can be multiple sessions going at once.
						
		jobContext	- Identifies the job this PM is being used with.
	
		jobTicket 	- The control parameters for this job, as defined by the application and user.
						
		converterSetup - Allows the PM to setup a converter via standard ticket tags.

		return value - OSStatus, contains errors if there were any.
		
	History:
		
	To Do: 
		
--------------------------------------------------------------------------------------------*/
OSStatus BeginJob( PMContext pmContext, const void *jobContext, PMTicketRef jobTicket, 
	PMTicketRef *converterSetup)
	{
	OSStatus	 	result = noErr ;
	PrivateData		*ourContext = NULL ;
	SInt32			quality = 0 ;					// What quality mode are we working with.
	SInt32			paperType = 0 ;					// What type of paper are we using
	SInt32			profileID = 0 ;					// What ColorSync profile are we using
	CFStringRef		inputFormat = NULL ;			// Format of incoming data - Raster or 
	CFRange					compareRange ;			// Range of characters to compare.
	CFComparisonResult		compareResult ;			// Holds less than, equal, or greater.
	Boolean			rasterFormat = false ;			// Set to true if we find incoming data is Raster.
	Boolean 		myPMFormat = false ;			// Set to true if we find incoming file is native
	
	// Get the internal data context
	ourContext = (PrivateData*)pmContext ;				
	if ( ourContext == NULL )
		result = kPMInvalidPMContext ;
		
	// Before we go anywhere, we need to confirm that the file type / data type
	// coming in matches the kind we need. This PM will handle either raster data or its native format 
	// data, so this is the place to check that kind of thing.
	if ( result == noErr )
		result =  PMTicketGetCFString( jobTicket, kPMTopLevel, kPMTopLevel, 
							kPMPrinterModuleFormatKey, &inputFormat ) ;
	
	// If we have one of our two good input types, then we can proceed with setting up for 
	// this job. Later on, we'll check to see if this was our favorite format - deep rasters - but
	// for now we just need to have one of the two - raster or native PM format.
	// Note: Until Tioga allows for Drag and Drop printing, native PM formats are not supported.
	if ( result == noErr )
		{
		compareRange.location = 0 ;
		compareRange.length = CFStringGetLength( inputFormat ) ;
		compareResult = CFStringCompareWithOptions( kPMDataFormatRaster, inputFormat, 
								 				compareRange,  NULL ) ;
		if ( kCFCompareEqualTo == compareResult )
			rasterFormat = true ;
			
			
		// If we didn't find raster format, then take another look to see if we got our native format.
		// If neither one of these is found to be true, then we'll fall through with both
		// flags false.
		
		else
			{
			compareRange.location = 0 ;
			compareRange.length = CFStringGetLength( inputFormat ) ;
			compareResult = CFStringCompareWithOptions( kPMDataFormat, inputFormat, 
									 				compareRange,  NULL ) ;
			if ( kCFCompareEqualTo == compareResult )
				myPMFormat = true ;
			}
			
		if ( !myPMFormat && !rasterFormat )
			result = kPMInvalidFileType ;
		}
	
	
	// Find the paper type and quality so we can get our engine code going on the right
	// track. This allows the proper initialization of tables and resources for efficient
	// processing in the C++ code.
	if ( result == noErr )
	{
		// Get paperType from job ticket, if not there default to plain
		result = PMTicketGetSInt32( jobTicket, kPMTopLevel, kPMTopLevel, kPMPaperTypeKey, &paperType ) ;
		if ( result != noErr )
		{
			paperType = kPMPaperTypePlain;
			result = noErr;
		}
	}

	if ( result == noErr )
	{
		// Get printQuality from job ticket, if not there default to draft mode.
		result = PMTicketGetSInt32( jobTicket, kPMTopLevel, kPMTopLevel, kPMQualityKey, &quality ) ;
		if ( result != noErr )
		{
			quality = kPMQualityDraft;
			result = noErr;
		}
		
		// Note: This PM does not handle normal and best print modes at this time.  This is why
		// I'm ignoring the PDE selection and forcing draft mode;
		quality = kPMQualityDraft;
	}


	if ( result == noErr )
	{
		// Get profile ID from job ticket, if not there set to zero.
        // The PM can override this ID when setting up the converter.

		result = PMTicketGetSInt32( jobTicket, kPMTopLevel, kPMTopLevel, kPMColorSyncProfileIDKey, &profileID ) ;
		if ( result == kPMKeyNotFound )
		{
			profileID = 0;
			result = noErr;
		}
	}

	// tell the printing mananger how we want our data to be generated/converted.
	if ( result == noErr )
		result = hpCreateConverterSetup ( jobTicket, paperType, quality, profileID, converterSetup);

#if !NOOUTPUT
	// Start things off with the printer by Opening the IO channel as quickly as we can.
	if ( result == noErr )
		result = ourContext->ioProcs.PMIOOpenProc( jobContext ) ;
	
	// Finally, start Engine to do the real work for us. We do this step only if
	// we're working with raster data. There is no need to reprocess the incoming PM ready data if 
	// that's what we've been given.
	if ( result == noErr && rasterFormat )
		result = hpStartEngine( (PMContext*)ourContext, jobContext, paperType, quality);
#endif


	// Open our module resource file using our bundle identifier.
	if (result == noErr)
	{
		ourContext->bundleRef = CFBundleGetBundleWithIdentifier( kRasterPMBundleID);

		if (ourContext->bundleRef != NULL)
			ourContext->resFile = CFBundleOpenBundleResourceMap( ourContext->bundleRef);

		result = ResError();
	}

	return( result ) ;
}

/*--------------------------------------------------------------------------------------------
	Function: ImageAccess()

	Description:
		This API is called when an image is encountered during conversion.  This allows the 
		Printer Module to access and modify the image data (imageRef) and drawing context before
		the image is rasterized into the page bands.  Modified image should be returned in 
		outImageRefPtr..
				
	Parameters:
		
		return value - OSStatus, contains errors if there were any.
		
	History:
		
	To Do: 
		
--------------------------------------------------------------------------------------------*/
// Callback function to frees up Bitmap data buffer used in the ImageAccess routine below
void ReleaseDataProc(void *info, const void *data, size_t size);
void ReleaseDataProc(void *info, const void *data, size_t size)
{
	if (data != NULL)
		free((void *)data);
}

OSStatus ImageAccess( PMContext pmContext, const void* jobContext, CFStringRef grafBase, 
			PMDrawingCtx drawingCtx, PMImageRef imageRef, PMImageRef* outImageRefPtr )
{
	OSStatus 		result = noErr ;
	PrivateData		*ourContext = NULL ;		// Our context block, keeps track from call to call
	
	// Double check our incoming context parameter.
	if ( pmContext == NULL)
		result = kPMInvalidParameter ;
	else
		ourContext = (PrivateData*)pmContext ;

	// Make sure we can handle this type of image.  Currently the only type is supported is CG.
	if ( result == noErr && kCFCompareEqualTo == CFStringCompare(grafBase, kPMGraphicsContextCoreGraphics,
												  kCFCompareCaseInsensitive) )
	{
		// Get CGImage reference from the PMImage reference
		CGImageRef cgImage = (CGImageRef) imageRef;

		// make sure this is a new image that we have not seen before on the page.
		// We check this by comparing the imageRef against an array of previously
		// processed images which we retain in an internal array.  This array is 
		// created in Initialize, added to below, and released in Terminate.
		// If we don't check, we will end up enhancing each image n-times per page;
		// where n is the number of bands on the page.
		CFDictionaryRef aDict;
		CGImageRef img;
		Boolean processedImage = false;
		CFIndex index, size = CFArrayGetCount( ourContext->imageRefArray);
		for (index = 0; index < size && processedImage==false; index++)
		{
			aDict = (CFDictionaryRef) CFArrayGetValueAtIndex( ourContext->imageRefArray, index);
			img = (CGImageRef)CFDictionaryGetValue( aDict, CFSTR("Src Img"));
			if (img == cgImage)
				processedImage = true;
		}
		
		if (processedImage)
		{
			// This image has been processed before.  Return the output image which was 
			// stored in the dictionary above.
			img = (CGImageRef)CFDictionaryGetValue( aDict, CFSTR("Dst Img"));
			CGImageRetain( img);
			*outImageRefPtr = img;
		}
		else
		{
			// New image is here.  Make sure we can handle its data. This sample code handles 
			// 24-bit RGB data; skips all other depths including rgb with alpha.
			size_t bitsPerPixel = CGImageGetBitsPerPixel( cgImage);
			size_t bitsPerComponent = CGImageGetBitsPerComponent( cgImage);
			if ( bitsPerPixel != 24 || bitsPerComponent != 8)
			{
				// Cannot enhance this image.  Return it as is
				*outImageRefPtr = (PMImageRef* )cgImage;
			}
			else
			{
				// We have received a new 24-bit deep image.  Lets enhance it.
				// Warning: Error checking is left as an exercise to the developer!!.
				
				// Get info about the image to be rendered
				size_t imageWidth = CGImageGetWidth( cgImage);
				size_t imageHeight = CGImageGetHeight( cgImage);
				const float *decodeArray = CGImageGetDecode( cgImage);
				int shouldInterpolate = CGImageGetShouldInterpolate( cgImage);
				CGColorRenderingIntent intent = CGImageGetRenderingIntent( cgImage);
				
				// Create a bitmap to draw the image into.  We need to allocate a buffer to fit the entire
				// image data - which may not be ideal for large a image where it's best to allocate a small
				// buffer and draw the image in chunks.
				size_t bytesPerRow = imageWidth * 4;
				size_t imageSize = bytesPerRow * imageHeight;	// Enough for an additional alpha channel
				void *dataBuffer = malloc( imageSize);

				CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
                                
                                // Preserve alpha channel when we make the CGBitMapContext
				CGContextRef cgBitMap = CGBitmapContextCreate( dataBuffer, imageWidth, imageHeight, 
						bitsPerComponent, bytesPerRow, colorSpace, kCGImageAlphaPremultipliedLast);
									
				// Draw the image into our bitmap.
				CGRect rect = CGRectMake( 0.0, 0.0, (float) imageWidth, (float) imageHeight);
                                CGContextClearRect( cgBitMap, rect);
				CGContextDrawImage( cgBitMap, rect, cgImage);
			
				// Now that we have the image drawn onto our bitmap, we will go thru the bitmap buffer data and 
				// modify it (i.e. enhance the data) as we like.  For illustration, the following code reduces 
				// the intensity of all color components by 10.  Printed image should come out slightly darker.
				// Value can be increased to dramatize the effect and make it more visible.
				size_t i;
				unsigned char *buffPtr = (unsigned char *)dataBuffer;
				for ( i=0; i< imageSize; i++, buffPtr++)
				{
					if (*buffPtr < 10)
						*buffPtr = 0;
					else
						*buffPtr -= 10;
				}
				
				// We have enhanced the data, it's time to create a new image based on it.
				// We do that by creating a CGDataProvider and passing that to CGCreateImage
				CGDataProviderRef dataProvider = CGDataProviderCreateWithData( NULL, (const void *)dataBuffer, 
													imageSize, ReleaseDataProc);
				CGImageRef myImage = CGImageCreate( imageWidth, imageHeight, bitsPerComponent, 32, 
						bytesPerRow, colorSpace, kCGImageAlphaPremultipliedLast, dataProvider, decodeArray, 
													shouldInterpolate, intent);
				
				// Free up data provider and the color space
				CGDataProviderRelease( dataProvider);
				CGColorSpaceRelease(colorSpace);
				// if all goes well, save both image refs into array of processed images
				// create a dict and stuff both imagerefs into it.  We also stuff the pointer to the
				// bitmap data buffer so it can disposed of it later.
				CFMutableDictionaryRef imgDict = CFDictionaryCreateMutable( CFAllocatorGetDefault(), 0, 
							&kCFTypeDictionaryKeyCallBacks, NULL);
				CFDictionarySetValue( imgDict, CFSTR("Src Img"), cgImage);
				CFDictionarySetValue( imgDict, CFSTR("Dst Img"), myImage);

				// Add dict to array then release dict
				CFArrayAppendValue( ourContext->imageRefArray, (const void *) imgDict);
				CFRelease( imgDict);
				
				// Ask CG to retain the output image ref so we can return it per band
				CGImageRetain( (CGImageRef) myImage );
				*outImageRefPtr = (PMImageRef)myImage;
			}
		}
	}
	
    return result;
}


/*--------------------------------------------------------------------------------------------
	Function: PrintJob()

	Description:
		Begin sending print data to the printer. The Printing Manager calls this API when page
		boundaries in the job can not be determined (i.e PM specific format).  It's the responsibility of 
		this PM to handle paginating the job to the printer, if necessary.  It'll continue to 
		process the job until it's told to cancel or it reaches the end. 
		
	Parameters:
		pmContext 	- our private context data so there can be multiple sessions going at once.
						
		jobContext	- Identifies the job this PM is being used with.
	
		jobTicket 	- The control parameters for this job, as defined by the application and user.
		
		inDataProcs - IO functions to use when receiving data we are to send to the printer.
		
		return value - OSStatus, contains errors if there were any.
		
	History:
		
--------------------------------------------------------------------------------------------*/
OSStatus PrintJob( PMContext pmContext, const void *jobContext, PMTicketRef jobTicket,
			 const PMJobStreamProcs *inDataProcs )
{
	OSStatus 	osStatus = noErr;

	// Drag and drop printing is not supported yet in Tioga so this API is not called.
	return( osStatus ) ;
}


/*--------------------------------------------------------------------------------------------
	Function: PrintPage()

	Description:
		Send only a specific page to the printer. This function is called when the Converter
		sends raster (deep) data to the Printer Module. The PM then needs to process the data
		and make it ready for the printer hardware. This API is called once per page until we
		reach end of document.
				
	Parameters:
		pmContext 	- our private context data so there can be multiple sessions going at once.
						
		jobContext	- Identifies the job this PM is being used with.
	
		jobTicket 	- The control parameters for this job, as defined by the application and user.
		
		pmJobStreamGetNextBandProc - Callback function we call to get the next band of data.
		
		return value - OSStatus, contains errors if there were any.
		
	History:
		
	To Do: 
		
--------------------------------------------------------------------------------------------*/
OSStatus PrintPage( PMContext pmContext, const void *jobContext, PMTicketRef jobTicket,
		const PMJobStreamGetNextBandProcPtr pmJobStreamGetNextBandProc )
	{
	OSStatus 		result = noErr ;
	PrivateData		*ourContext = NULL ;		// Our context block, keeps track from call to call
	SInt16 			origResFile = CurResFile();	// So we can open the PM's res file then restore.
	CFStringRef		inputFormat = NULL ;		// Format of incoming data - Raster or 
	CFRange			compareRange ;				// Range of characters to compare.
	CFComparisonResult	compareResult ;			// Holds less than, equal, or greater.
	
	
	// Double check our incoming context parameter.
	
	if ( pmContext == NULL )
		result = kPMInvalidParameter ;
	else
		ourContext = (PrivateData*)pmContext ;
		

	// Find out what type of data is coming in. For this call to work, the caller must
	// give us raster data.
	if ( result == noErr )
		{
		result =  PMTicketGetCFString( jobTicket, kPMTopLevel, kPMTopLevel, kPMPrinterModuleFormatKey, 
											&inputFormat ) ;
		if ( result == noErr )
			{
			compareRange.location = 0 ;
			compareRange.length = CFStringGetLength( inputFormat ) ;
			compareResult = CFStringCompareWithOptions( kPMDataFormatRaster, inputFormat, 
											 				compareRange,  NULL ) ;
			if ( kCFCompareEqualTo != compareResult )
				result = kPMInvalidFileType ; 
			}
		}
	
	// Print all the bands in this page	
	if ( result == noErr )
		result = hpPrintPageBands( (PMContext*)ourContext, jobContext, jobTicket, pmJobStreamGetNextBandProc);
			
	// Restore the original resource file.
	UseResFile( origResFile);

	return( result ) ;
}

/*--------------------------------------------------------------------------------------------
	Function: CancelJob()

	Description:
		Cancel the currently printing job.  Does the cancelling by setting an internal flag that
		is checked by PrintJob and PrintPage to abort printing of any current job/page.
		
	Parameters:
		pmContext 	- our private context data.
						
		jobContext	- Identifies the job this PM is being used with.
		
	History:
		
--------------------------------------------------------------------------------------------*/
OSStatus CancelJob( PMContext	pmContext,  const void *jobContext )
{
	OSStatus 		result = noErr ;
	PrivateData		*ourContext = NULL ;			

	if ( pmContext == NULL )
		result = kPMInvalidParameter ;
	else
	{	ourContext = (PrivateData*)pmContext ;
		ourContext->abort = true;
	}
	
	return( result ) ;
}


/*--------------------------------------------------------------------------------------------
	Function: EndJob()

	Description:
		Finish up the previously printed job. All job data should be in the printer.
		
	Parameters:
		pmContext 	- our private context data.
						
		jobContext	- Identifies the job this PM is being used with.
		
	History:
	
	To Do:
		
--------------------------------------------------------------------------------------------*/
OSStatus EndJob( PMContext pmContext, const void *jobContext )
	{
	OSStatus	 	result = noErr ;
	PrivateData		*ourContext = NULL ;			// Our private structure for context.
	
	
	// Double check the incoming parameters.
	
	if ( pmContext == NULL )
		result = kPMInvalidParameter ;
	else
		ourContext = (PrivateData*)pmContext ;

#if !NOOUTPUT
	// If this job used Engine, then close job and dispose of the object.
	if ( result == noErr )
		result = hpStopEngine( (PMContext*)ourContext);
		
	// Close the IO path for incoming data.
	if ( result == noErr )
		result = ourContext->ioProcs.PMIOCloseProc( jobContext ) ;
#endif
		
	
	// To avoid using the IOProcs any more (which aren't supposed to be valid after EndJob )
	// we'll clear them in our context block.

	if ( result == noErr )
		memset(&ourContext->ioProcs, 0, sizeof(ourContext->ioProcs));

	// Close the resource fork.
	if (ourContext->bundleRef != NULL)
		CFBundleCloseBundleResourceMap(ourContext->bundleRef, ourContext->resFile);

	return( result ) ;
}
	
/*--------------------------------------------------------------------------------------------
	Function: Terminate()

	Description:
		Dispose of our private data block and release any memory allocated in the
		last session.
		
	Parameters:
		pmContext 	- our private context data.
						
		jobContext	- Identifies the job this PM is being used with.
		
	History:
		
--------------------------------------------------------------------------------------------*/
OSStatus Terminate( PMContext *pmContext, const void *jobContext )
	{
	OSStatus 		result = noErr ;
	PrivateData		*ourContext = NULL ;
	
	if ( pmContext == NULL )
		result = kPMInvalidParameter ;
	else
	{
		ourContext = (PrivateData*) *pmContext ;

		// Deallocate the array of image references and bitmap data used in ImageAccess processing
		if ( ourContext->imageRefArray != NULL)
		{
			// To deallocate bitmap data, we need to go thru all the dictionaries in the
			// array and release each image which will call our ReleaseDataProc callback to
			// release the bitmaps we allocated for these images.
			CFDictionaryRef aDict;
			CGImageRef img;
			CFIndex index, size = CFArrayGetCount( ourContext->imageRefArray);
			for (index = 0; index < size; index++)
			{
				aDict = (CFDictionaryRef) CFArrayGetValueAtIndex( ourContext->imageRefArray, index);
				img = (CGImageRef)CFDictionaryGetValue( aDict, CFSTR("Dst Img"));
				CGImageRelease( img );
			}

			// Now release the array itself
			CFRelease( ourContext->imageRefArray);
		}
		
		CFAllocatorDeallocate( CFAllocatorGetDefault(), ourContext ) ;
	}

	return( result ) ;
}

