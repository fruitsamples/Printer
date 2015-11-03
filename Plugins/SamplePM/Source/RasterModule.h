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
	File: RasterModule.h

	Contains: Prototypes for internal utility routines for the Sample Raster Printer Module
			
 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

 	Bugs?:		For bug reports, consult the following page on
 				the World Wide Web:
 
 					http://developer.apple.com/bugreporter/
	Change History:

	To Do:
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifndef __PMRasterModule__
#define	__PMRasterModule__

#include <PrintCore/PMPrinterModule.h>
#include <PrintCore/PMTicket.h>
#include <PrintCore/PMRaster.h>
#include "RasterDefines.h"

// Main Printer Module Prototypes:
/* CreatePrinterBrowserModuleInfo */
EXTERN_API_C( OSStatus) CreatePrinterBrowserModuleInfo ( CFStringRef connectionType, CFArrayRef *printerLookupInfo);

/* Initialize */
EXTERN_API_C( OSStatus) Initialize( const CFDataRef printerAddress, const void * jobContext, const PMIOProcs *pmIOProcs, const PMNotificationProcPtr pmSetPrinterStatusProc, PMContext * pmContext);

/* CreatePrintingDialogExtensionsPaths */
EXTERN_API_C( OSStatus) CreatePrintingDialogExtensionsPaths(PMContext pmContext, CFArrayRef *pdePaths);

/* CreatePrinterTickets */
EXTERN_API_C( OSStatus) CreatePrinterTickets (PMContext pmContext, PMTicketRef *printerInfo, PMTemplateRef *jobTemplate);

/* BeginJob */
EXTERN_API_C( OSStatus) BeginJob(PMContext pmContext, const void * jobContext, PMTicketRef jobTicket, PMTicketRef *converterSetup);

/* ImageAccess */
EXTERN_API_C( OSStatus) ImageAccess( PMContext pmContext, const void* jobContext, CFStringRef grafBase, PMDrawingCtx drawingCtx, PMImageRef imageRef, PMImageRef* outImageRefPtr);

/* PrintJob */
EXTERN_API_C( OSStatus) PrintJob(PMContext pmContext, const void * jobContext, PMTicketRef jobTicket, const PMJobStreamProcs *inDataProcs);

/* PrintPage */
EXTERN_API_C( OSStatus) PrintPage( PMContext pmContext, const void * jobContext, PMTicketRef jobTicket, const PMJobStreamGetNextBandProcPtr PMJobStreamGetNextBandProc);

/* CancelJob */
EXTERN_API_C( OSStatus) CancelJob( PMContext pmContext, const void * jobContext);
							 
/* EndJob */
EXTERN_API_C( OSStatus) EndJob( PMContext pmContext, const void * jobContext);

/* Terminate */
EXTERN_API_C( OSStatus) Terminate(PMContext * pmContext, const void * jobContext);


// Prototypes for internal functions
OSStatus 	hpCreateIconData	( CFDataRef *cfIconData);

OSStatus 	hpCreatePrinterInfo	(PMContext pmContext, PMTicketRef *printerInfo);
								
OSStatus 	hpCreatePMTemplate	(PMContext pmContext, PMTemplateRef *pmTemplate);
								
OSStatus	hpCreateProfileDict	( CFDictionaryRef *pmProfileDict);

OSStatus	hpCopyEntriesFromProfileDict(	CFDictionaryRef pmProfileDict,
											SInt32*			pmDefaultProfileID,
											CFArrayRef*		pmProfileArray,
											SInt32*			pmProfileCount,
											SInt32**		pmProfileIDList );
                                            
OSStatus	hpVerifyColorSyncProfileID ( PMTicketRef jobTicket, SInt32 paperType, SInt32 quality, SInt32 *profileID );

											
OSStatus 	hpCreateConverterSetup ( PMTicketRef jobTicket, SInt32 paperType, SInt32 quality, SInt32 profileID, PMTicketRef *converterSetup);

OSStatus 	hpStartEngine		( PMContext *Context, const void *jobContext, SInt32 paperType, SInt32 quality);

OSStatus 	hpPrintPageBands	( PMContext *myContext, const void *jobContext, PMTicketRef jobTicket,
								PMJobStreamGetNextBandProcPtr pmJobStreamGetNextBandProc) ;
OSStatus 	hpStopEngine		( PMContext *Context);

#endif //__PMRasterModule__