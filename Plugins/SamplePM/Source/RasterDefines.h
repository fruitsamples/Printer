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

/*****************************************************************************
	RasterDefines.h
	
	Contains:	Raster Printer Module IDs and related constants
				Engine specific commands and related constants  

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.
 
	Mods:

 ***************************************************************************/

#ifndef __RasterDefines__
#define	__RasterDefines__

#include "RasterModule.h"
#include "EnginePCL.h"

// constants and types
#define kRasterPMFactoryIDStr			"E4089DDE-935D-11D6-9F32-003065A0DEE8"
#define kRasterPMBundleID			CFSTR("com.pmvendor.print.sample.pm")
#define kRasterPMIconFileName			"Sample.icns"   
#define kRasterPrinterBrowserKindValue           CFSTR("Sample PCL printer")    
#define kRasterPrinterBrowserInfoValue        	CFSTR("DESKJET")   
#define kRasterPrinterBrowserDeviceIDValue	CFSTR("MDL:DeskJet")

// ColorSync-related tags
#define	kPMColorDeviceDefaultProfileID		CFSTR("PMColorDeviceDefaultProfileID")
#define	kPMColorDeviceProfiles				CFSTR("PMColorDeviceProfiles")
#define	kPMColorDeviceProfileID				CFSTR("PMColorDeviceProfileID")
#define	kPMColorDeviceProfileName			CFSTR("PMColorDeviceProfileName")
#define	kPMColorDeviceProfileURL			CFSTR("PMColorDeviceProfileURL")
#define kRasterPMProfileListFileName		"PMColorDeviceProfiles.xml"
#define	kPMProfileDictNotFound				-100
#define	kPMProfileDictLoadFailure			-101
#define	kPMProfileDictProfileIDFailure		-102
#define	kPMProfileDictProfileArrayFailure	-103
#define	kPMProfileDictProfileIDListFailure	-104

#define		kNoCancel					0	

const CFStringRef	kRasterPrinterLongName		= CFSTR( "Hewlett Packard DeskJet 880c");	
const CFStringRef	kRasterPrinterShortName		= CFSTR( "HP 880c");	
const CFStringRef	kRasterPrinterProductName	= CFSTR( "HP DeskJet 880c");	

const CFStringRef	kPMDataFormat 			= CFSTR( "application/vnd.apple.printing-pcl" );
#define	kDraftResolution			300
#define	kNormalResolution			300
#define	kBestResolution				600

#define	kBestQualityBandSize		32
#define	kNormalQualityBandSize		64
#define	kDraftQualityBandSize		64

static const CFOptionFlags 	kNoHints = NULL ;
static const UInt32 		kIOBufferSize = (1024 * 32) ;  	//32k - Size of our input and output buffers.


// A private data structure to hold various things we'll need to keep track of our printing session.
typedef struct 
{
        TEnginePCL					*enginePCL ;		// Pointer to our EnginePCL object.	
    	UInt32 						bufferSize ;		// The size in bytes of the following bufffer
    	PMIOProcs 					ioProcs ;			// Connection to the IO module
    	PMNotificationProcPtr 		notifyProc ;		// Call this routine with our status.
    	Boolean 					abort ;				// When true, the caller is requesting an abort.
        SInt16						resFile;			// our printer module resfile.
        CFBundleRef 				bundleRef;			// printer module cfbundle ref.
		CFMutableArrayRef			imageRefArray;		// array of image refs for ImageAccess use
        /*
        **	status tracking
        */
        Boolean						deviceIdQueryOkay;	// some DeskJet models have problems w/DeviceID
        Boolean						isCoverOpen;		// cover is open
        Boolean						isPaperJam;			// paper jam
        Boolean						isPaperOut;			// paper out
        Boolean						isBusy;				// busy printing
        Boolean						isIdle;				// idle
} PrivateData ;

// Resource Types for color lookup tables
#define		kMDOTResType			'MDOT'
#define		kGAMAResType			'GAMA'
#define		kTRCTResType			'TRCT'

//-----------------------------------------------------------------------------
//	command IDs used in ECMD resource table AND Engine Code to
//	override commands, as necessary.
//-----------------------------------------------------------------------------
#define		kVoidCmdID				0x00
#define		kBiDirCmdID				'B'
#define		kBlackOnlyOnPageCmdID	'K'
#define		kConfigRasterCmdID		'C'
#define		kGraphicsTypeCmdID		'G'
#define		kPaperSizeSetupCmdID	'Z'
#define		kUnitOfMeasureCmdID		'U'
#define		kPaperFeedCmdID			'F'
#define		kPaperThicknessCmdID	'T'
#define		kPaperTypeCmdID			'P'
#define		kQualityCmdID			'Q'
#define		kQuality2CmdID			'q'

const Boolean kColorToo = true ;		// Swap color pointers or clear prev color rows.
const Boolean kIgnoreColor = false ;	// Only swap or clear black rows.

// Engine commands under various print modes, color, and paper types

const TPCLCmd PCLPlainPaperTypeCmd		= { 5, "\x1b&l0M" } ;
const TPCLCmd PCLCoatedPaperTypeCmd		= { 5, "\x1b&l2M" } ;
const TPCLCmd PCLTranspPaperTypeCmd		= { 5, "\x1b&l4M" } ;
const TPCLCmd PCLGlossyPaperTypeCmd		= { 5, "\x1b&l3M" } ;

const TPCLCmd PCLDraftQualityCmd		= { 6, "\x1b*o-1M" } ;
const TPCLCmd PCLNormalQualityCmd		= { 5, "\x1b*o0M" } ;
const TPCLCmd PCLBestQualityCmd			= { 5, "\x1b*o1M" } ;

const TPCLCmd PCLGraphicsTypeCmd		= { 10, "\x1b*o5W\x7\x8\x0\x0\x2" } ;

// Resolution,Color commands for Draft/plain/color
const TPCLCmd PCLDraftPlainColorCmd   = { 13, "\x1b*t300R\x1b*r-4U" } ;
const TPCLCmd PCLNormalPlainColorCmd  = { 13, "\x1b*t600R\x1b*r-4U" } ;
const TPCLCmd PCLBestPlainColorCmd    = { 13, "\x1b*t600R\x1b*r-4U" } ;

const TPCLCmd PCLDraftUnitOfMeasureCmd		= { 7, "\x1b&u300D" } ; // send only in Best/Plain and Best/Coated/Color !!!
const TPCLCmd PCLNormalUnitOfMeasureCmd		= { 7, "\x1b&u300D" } ; // send only in Best/Plain and Best/Coated/Color !!!
const TPCLCmd PCLBestUnitOfMeasureCmd		= { 7, "\x1b&u600D" } ; // send only in Best/Plain and Best/Coated/Color !!!


// Our PCL commands. Structures loaded w/ arrary initializers.
// Job Setup & Control Commands, Use to put printer in standard printing mode.
const TPCLCmd PCLEnterPCL3				= { 33, "\x1b%-12345X@PJL ENTER LANGUAGE=PCL\12" } ;

const TPCLCmd PCLTrayOneInput			= { 5, "\x1b&l1H" } ;
const TPCLCmd PCLSetMargins				= { 5, "\x1b&l0E" } ;
const TPCLCmd PCLEndOfJob				= { 9, "\x1b%-12345X" } ;
const TPCLCmd PCLFormFeed				= { 1, "\x0C" } ;
const TPCLCmd PCLReset					= { 2, "\x1B\x45" } ;

// Move CAP Vertical assumes we don't need to ever move CAP horizontal, so it's 
// prepended for our normal case. Once the # of lines is sent, we can send the 
// end command. Number of PCL lines is usually 300 (basic resolution of head) but
// will vary by engine (when basic resolution of head is 600, PCL unit will be 1/600th) ;
const TPCLCmd PCLMoveCAPVertStart		= { 7, "\x1b*p+0x+" } ;
const TPCLCmd PCLMoveCAPVertEnd 		= { 1, "Y" } ;

// Raster control. We'll always start rasters at CAP.
const TPCLCmd PCLSourceRasterWidthPre		= { 3, "\x1b*r"};
const TPCLCmd PCLSourceRasterWidthPost		= { 1, "S"};
const TPCLCmd PCLStartRasterAtCAP		= { 5, "\x1b*r1A" } ;
const TPCLCmd PCLTransferRaster			= { 3, "\x1b*b" } ;
const TPCLCmd PCLTransferRasterPlane		= { 1, "V" } ;
const TPCLCmd PCLTransferRasterPlaneFinal	= { 1, "V" } ;
const TPCLCmd PCLTransferRasterRowFinal		= { 1, "W" } ;
const TPCLCmd PCLEndRaster				= { 4, "\x1b*rC" } ;

// Raster compression methods, two for beginning of band, two for scanline
// level control
const TPCLCmd PCLNoCompression			= { 8, "\x1b*b0M\x1b*b" } ;
const TPCLCmd PCLCompressionTwo			= { 8, "\x1b*b2M\x1b*b" } ;
const TPCLCmd PCLNoCompressThisScan		= { 2, "0m" } ;
const TPCLCmd PCLCompressThisScan		= { 2, "2m" } ;

#endif //__RasterDefines__
