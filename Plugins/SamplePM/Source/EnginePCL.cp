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

/*
	File: EnginePCL.cp
	
	Contains:
		Example code for driving a PCL engine. This is low level code and is
        expected to be replaced by each vendor's version of his own control 
        code to create compressed data ready for the printer. We've included
        it here to make sure you have the complete story of how we've created
        the PCL data for the printer.
        
        This code is not optimized for Tioga. Use with caution
        				
 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Copyright (C) 2000-2002, Apple Computer, Inc., all rights reserved.

*/

#include <stdio.h>		// added for printf debugging
#include <ApplicationServices/ApplicationServices.h>

#include "EnginePCL.h"
#include "RasterDefines.h"
#include "Exception.h"
#include <PrintCore/PMPrinterModule.h>		// For definition of paper types and qualities.


// Constants
//#define kRasterPMDebug

// Globals

// prototypes
	
/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::TEnginePCL()
	Description: Constructor for the base class of engine control code we use
        in support of the higher level functions defined in RasterModule.cp. 
        This code is here for completeness, though we fully expect developers
        will want to replace it with their own carefully crafted code.
	Input: None.
	Output: None.
	Return:
*/
TEnginePCL::TEnginePCL( )
{
	
	// Initialize members as necessary.
	fTablesInitialized = false;		// set init flag
	
	fSubSampleDelta = nil;
	fBlackBlk = nil ;
	fNormalModeBlackBlk = nil;
	fCMYBlk = nil ;		
	fCmpDest = nil ;

	fCmpDestBlk = nil;				
 
	fLastPageDepth = 0 ;			// Force update for new page depth at open page.

	fCyanTRCHndl = nil;				// Handle to RGB2CMYK Cyan TRC table
	fMgtaTRCHndl = nil;				// Handle to RGB2CMYK Magenta TRC table
	fyeloTRCHndl = nil;				// Handle to RGB2CMYK Yellow TRC table
	fBlkTRCHndl = nil;				// Handle to RGB2CMYK Black TRC table
        
	// Clear raster pointer so we know when they're allocated.
	this->ClearRasterPointers() ;

}
//------------------------------------------------------------------------------

/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::~TEnginePCL()
	Description: Desctructor for base class of EnginePCL
	Input: None.
	Output: None.
	Return:

*/
TEnginePCL::~TEnginePCL()
{
	// Free up memory we may have allocated, just in case ClosePage doesn't
	// get called.
	
	TRY
		this->FreeCommonMemory() ;
	NOHANDLER 
}
//------------------------------------------------------------------------------

/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::ClearRasterPointers()
	Description: We clear our raster pointers in our constructor and at the end
		of each page so we can determine our depth and only allocate the blocks
		and pointers we need for the current depth. This also allows us to 
		clear the rasters that have really been allocated.
        NOTE: This may not be a good practice for printer modules under OSX, but we've
        left this code as it was on OS9 to save ourselves the trouble of debugging
        a new more advanced piece of code. 
	Input: None
	Output: None.
	Return:

*/
void TEnginePCL::ClearRasterPointers()
{
	fCyanRstr1 = nil ;				

	fMgtaRstr1 = nil ;				

	fYeloRstr1 = nil ;				

	fBlkRstr1 = nil ;

}
//------------------------------------------------------------------------------

/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::FreeCommonMemory()
	Description: Deallocates all memory that must be released on a page by
		page basis. 
        NOTE: Again, this may not be the right thing to do, but it's what our
        code did under OS9, so we're going to just leave it as is for now.
	Input: None.
	Output: None.
	Return:
*/
void TEnginePCL::FreeCommonMemory()
{
	if (fBlackBlk)
	{
		DisposePtr( fBlackBlk ) ;
		fBlackBlk = nil;
	}
	
	if (fNormalModeBlackBlk)	
	{
		DisposePtr( fNormalModeBlackBlk ) ;
		fNormalModeBlackBlk = nil;
	}

	if (fCmpDestBlk)	
	{
		DisposePtr( fCmpDestBlk ) ;
		fCmpDestBlk = nil;
	}

	if (fCMYBlk)
	{
		DisposePtr( fCMYBlk ) ;
		fCMYBlk = nil;
	}
	
	// Clear the raster pointers associated with the blocks so we'll know when
	// fresh rasters have been allocated.
	
	this->ClearRasterPointers() ;
	
	
	// Dispose subsampling's delta table and white scan line
	DisposeNonNullHdl( fSubSampleDelta ) ;

	// Dispose rgb2cmyk trc handles
	DisposeNonNullHdl( fCyanTRCHndl ) ;
	DisposeNonNullHdl( fMgtaTRCHndl ) ;
	DisposeNonNullHdl( fyeloTRCHndl ) ;
	DisposeNonNullHdl( fBlkTRCHndl ) ;

}
//------------------------------------------------------------------------------

/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::OpenJob()
	Description: Called when w job begins. Use this as an example of what you 
        might wish to do when a job starts. This code is called hpStartEngine()
        in the PCLUtils.cp file. 
    Input: paper type and print quality.
	Output: None.
	Return:
*/
void TEnginePCL::OpenJob( UInt16 paperType, UInt16 printQuality, const PMIOProcs *IOProcs)
{
	
	// Call the base class first in case it needs to do anything.
	inherited::OpenJob( paperType, printQuality, IOProcs);
	
	// Set some fields we know will be used fresh for this page.
	fPaperType = paperType;
	fPrintQuality = printQuality;
	fIOProcs = *IOProcs;

    // create the io helper
    ioh = PMIOCreateHelper(4096, 32768,  fJobID, fIOProcs.PMIOWriteProc);
    
	fPageDepth = 32 ;				// Maximum depth for this page.
	fPixelBuf = nil ;				// Compression output buffer
	fLastBandScan = -1 ;			// Must point to just above the band.
	fLastBandDepth = 0;				// Set to invalid depth for PrintBand

	// initiate a job with the printer
	this->PCLSendCommand( &PCLEnterPCL3 ) ;
	this->PCLSendCommand( &PCLReset ) ;
	this->PCLSendCommand( &PCLTrayOneInput ) ;	

}


//------------------------------------------------------------------------------
// AllocateCommonMemory: Used to allocate the memory needed for all bands. This
// code allocates memory for compression buffers and buffers for PCL data going
// to the printer. Please use this as an example only, not necessarily the right
// thing to do for your particular printer. The particulars of our method for
// allocating memory and setting up our raster scan buffers is specific to the
// engine and our "recycled" code. Please don't take this as the "right" way to
// do things on OSX.

void TEnginePCL::AllocateCommonMemory( UInt32 bandWidth)
{
	long	numBytesForOne ;			// Bytes for a single scan or raster
	long	size ;						// Number passed to banker for allocate.
	UInt8	*blockMemPtr ;				// Pointer to a block of memory from banker.

	TRY

		// Determine the page width, based on the print record's copy of the
		// device page. 		

		// properly to find the correct number of bytes for a single scanline. 
		numBytesForOne = bandWidth / 8 ;
			
		// Allocate a block for the black data and fail if we can't get it.		
		fBlackBlk = NewPtrClear( numBytesForOne );		
		FAILNULL( fBlackBlk, memFullErr) ;
		
		// Set up member pointers to the 1st black raster and the 1st previous
		// black raster ( our minimum).
		fBlkRstr1 = (UInt8*)fBlackBlk;	
		
		// for normal printing, we need an extra line buffer for upsampling	black data
		if (fPrintQuality == kPMQualityNormal)
		{	size = numBytesForOne * 2;
			fNormalModeBlackBlk = NewPtrClear( size);
			FAILNULL( fNormalModeBlackBlk, memFullErr ) ;
		}
		else
			size = numBytesForOne;	
			
		// we need to allocate a buffer for our compression code
		fCmpDestBlk = NewPtrClear( size ) ;		
		FAILNULL( fCmpDestBlk, memFullErr ) ;


		// Color is set up in a similar fashion.  		
		if ( fNumColorBitsPerPixel > 0 )
		{
			// We need 1 raster for each plane if in 1 bit mode			
			if ( fNumColorBitsPerPixel == 1 )
				size = numBytesForOne * 3 ;

			fCMYBlk = NewPtrClear( size ) ;		
			FAILNULL( fCMYBlk, memFullErr ) ;

			
			// Set up pointers for all the color data, including the current and previous
			// for each of the two rasters for each plane.
			blockMemPtr = (UInt8*)fCMYBlk;	//AQ
			fCyanRstr1 = blockMemPtr ;		blockMemPtr += numBytesForOne ;
			fMgtaRstr1 = blockMemPtr ;		blockMemPtr += numBytesForOne ;
			fYeloRstr1 = blockMemPtr ;		blockMemPtr += numBytesForOne ;
			
		}
			
	EXCEPTION
		if (fBlackBlk)
			DisposePtr( fBlackBlk ) ;
		if (fNormalModeBlackBlk)
			DisposePtr( fNormalModeBlackBlk);
		if (fCmpDestBlk)
			DisposePtr( fCmpDestBlk ) ;
		if (fCMYBlk)
			DisposePtr( fCMYBlk ) ;
		PASSEXCEPTION() ;
	ENDEXCEPTION
}

/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::OpenPage()
	Description: Called when a page begins. Typically we'll prefeed a page and 
		load gama resources. The gama resources are used in our conversion from
        RGB to CMYK and shouldn't be considered correct or even necessary for your 
        particular printer.
	Input:
		copies - Number of copies we should print once page is in engine.
		pageDepth - Maximum depth for this page.
	Output: None.
	Return:
*/
void TEnginePCL::OpenPage( int copies, short pageDepth)
{

  	inherited::OpenPage(copies, pageDepth);
			
	// Get print/media mode commands from engineInfo and send them to the printer 
	this->IssueEngineCmds();
}
//------------------------------------------------------------------------------
/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::PrintBand()
	Description: Called w/ a deep band of image data that must be processed and
		sent to the printer.  After processing the band's data, a PCL end raster
		command should be sent synchronously.  This will force the buffer to
		start flushing while the driver goes off and renders another band.`
	Input:
		band - Pointer to our band object w/ deep data.
		jobTicket - jobticket describing the print settings
		bandWidth - with of the band in pixels
	Output: None.
	Return:
*/
void TEnginePCL::PrintBand(	Ptr bandBuffer, UInt32 bandBufferSize, 
							UInt32 bandWidth, UInt32 bandHeight, short bandDepth)
{
	FAILCode	error ;					// Any failure will record exception here.	
	
	TRYR( error ) 

	if (bandDepth != fLastBandDepth)
	{
		this->FreeCommonMemory();

		// Load the necessary MDOT resources for halftoning algorithms.  
		this->SetUpMultiDotTables( (Handle*)&fMdotCyan, (Handle*)&fMdotMgta,
								 (Handle*)&fMdotYelo, (Handle*)&fMdotBlk ) ;
			
		// Set up Gamma Correction Tables & undercolor removal / black generation tables.		
		this->SetUpGammaCorrection(&fGammaHndl);
		this->SetUpUCRBG(&fKLevelHndl, &fSLevelHndl);			

		// Set up TRC tables for the new RGB2CMYK conversion and record our
		// tables as loaded.		
		this->SetUpTRCTables(&fCyanTRCHndl, &fMgtaTRCHndl, &fyeloTRCHndl, &fBlkTRCHndl);

		this->SetBitsPerPixel( &fNumBlackBitsPerPixel, &fNumColorBitsPerPixel ) ;

		this->AllocateCommonMemory( bandWidth);

		fLastBandDepth = bandDepth;
		fTablesInitialized = true;
	}

	// Turning this flag on creates a red scanline between bands - used for debugging
	// Note that this code assumes the alpha channel may be set to FF without causing
	// any problems.			
#define SHOW_BAND_START_WITH_RED 0
#if SHOW_BAND_START_WITH_RED
    {
		// Set up a pointer into our scan buffer, at the first scanline of the band,
		// and loop through that scanline to set pixels to red.
	    
	    UInt32 *bytePtr = (UInt32 *) bandBuffer;
        for (int x = 0; x < bandWidth; x++) 
            *bytePtr++ = 0xffff0000;
    }
#endif

	
	// If we are going to skip white space, then call our function to print only
	// the non-white stuff. This function will first look for white and then print
	// only the non-white.
#define WHITE_SPACE_SKIPPING 1
#if WHITE_SPACE_SKIPPING
    this->printNonWhite( bandBuffer, bandBufferSize, bandWidth, bandHeight) ;


	// If we don't want to skip over white by looking for the white pixels, then
	// simply print all the data.
#else 
    if (bandHeight != 0) 
    {
        this->PCLSendCommand( &PCLStartRasterAtCAP ) ;
        this->PrintCMYK( bandBuffer, bandBufferSize, bandWidth, bandHeight) ;
        this->PCLSendCommand( &PCLEndRaster ) ;
    }
#endif
	
	NOHANDLER ;

	FAILOSErr(error);
}


	
/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::skipWhite()
	Description: Called to advance the head the requested number of scanlines. 
		This is done to skip over white space. 
	Input:
		count - Number of scans to skip.
	Output: None.
*/

void TEnginePCL::skipWhite(int count)
{
	Str15		whiteSizeString ;		// String to hold number of lines to skip.
    NumToString( count, whiteSizeString) ;	
    
    this->PCLSendCommand( &PCLMoveCAPVertStart ) ;						
    this->PCLSendData( &whiteSizeString[1], whiteSizeString[0] ) ;
    this->PCLSendCommand( &PCLMoveCAPVertEnd ) ;						
}
    

/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::printNonWhite()
	Description: Called to find the white space, skip over it, and then print
		the non-white portion from the band. This function is called in place of the
		normal PrintCYMK function.
	Input:
		bandBuffer - Pointer to the band bits
		bandBufferSize - Number of bytes in the band buffer
		bandWidth - Band width in bytes
		bandHeight - Band height in scan lines
	Output: None.
*/
void TEnginePCL::printNonWhite(	 Ptr bandBuffer, UInt32 bandBufferSize,
							UInt32 bandWidth, UInt32 bandHeight)
{
    UInt32 *p = (UInt32 *) bandBuffer;
    int h = bandHeight;
    int w = bandWidth;
    int i,x;
    int lastWhiteLine;
    char *bufStart = bandBuffer;
    int numLines;
    int whiteLine;
    
    numLines = 0;
        
    for (i = 0; i < h; i++, numLines++) {
        whiteLine = 1;
        
        for(x = 0; x < w; x++) {
            if (*p++ != 0xffffffff) {
                whiteLine = 0;
                p += w - x - 1;
                break;
            }
        }
        if (i == 0) {
            lastWhiteLine = whiteLine;
            continue;
        }
        if(lastWhiteLine != whiteLine) {
            if(!whiteLine) {
                this->skipWhite(numLines);
                bufStart = bandBuffer + i*bandWidth*sizeof(UInt32);
                numLines = 0;
            } else {
                this->PrintCMYK( bufStart, numLines*bandWidth, bandWidth, numLines) ;
                numLines = 0;
           }
            lastWhiteLine = whiteLine;
        }
    }
    if(whiteLine) {
        this->skipWhite(numLines);
    } else {
        this->PrintCMYK( bufStart, numLines*bandWidth, bandWidth, numLines) ;
    }
}

// A macro used to output the correct compression field based on the
// current scans compression and the previous scan's compression. Toggles the
// compression based on the new value.
#define mCheckAndToggleCompression(current,previous)			\
	if ( 1 /*current != previous*/ )							\
		{														\
		if ( current == true )									\
			this->PCLSendCommand( &PCLCompressionTwo ) ;		\
		else													\
			this->PCLSendCommand( &PCLNoCompression ) ;	\
		}														

// Masks for 1 bit in each component.
#define kOneBitEachMask	0x01010101		

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::PrintCMYK()
	Description: Takes a 32 bit deep band and sends proper data to the printer.
		This function can be called just for the non-white data or for the 
		entire band.
		Note, please, that most of this is specific to our particular engine
		and the method we chose to control it, largely based on old code for an
		OS9 driver. Your engine is likely to need a very different control
		strategy to be efficient.
	Input:
		band - Pntr to a band object, should be 32 bits deep.
		bandBuffer - band data.
		jobTicket- job ticket for the current job
		bandWidth - Number of pixels per scanline.
	Output: None.
	Return:

*/
void TEnginePCL::PrintCMYK(	 Ptr bandBuffer, UInt32 bandBufferSize,
							UInt32 bandWidth, UInt32 bandHeight)
{
	UInt32		scan ;					// Current scan number in loop.
	UInt32		*CMYKPixelBase;			// Base address of CMYK data for scan row
	UInt32	 	CMYKData ;				// 32 bits of source CMYK data - 4 components.
	UInt32		CMYKLow ;				// Space for bits in our pixel.
	short		compSize ;				// Size of compressed output scanline.
	Boolean		compressed ;			// True if we were able to compress a scan.
	Boolean		prevCompressed = true ;	// True if the previous scanline compressed.
	Str15		rasterSizeString ;		// String to hold number of bytes in raster.
	
	// In addition, for each channel, we'll set up a local pointer so we can add bytes
	// to the output stream as we build the data.
	
	UInt8		*cyanLowBuff ;			// Pntr to cyan low intensity.
	UInt8		*magentaLowBuff ;		// Pntr to magenta low intensity.
	UInt8		*yellowLowBuff ;		// Pntr to yellow low intensity.
	UInt8		*blackLowBuff ;			// Pntr to black low intensity.
	long		scanBytes, rowBytesPad;

#ifdef kRasterPMDebug
	fprintf(stderr,"\nPrintCMYK: bandWidth=%ld; bandHeight=%ld\n ", bandWidth, bandHeight);
#endif

	// First things first, let's convert to CMYK and set our scanline counter so
	// we know we're just starting on this band.
	this->RGB2CMYK( bandBuffer, bandWidth, bandHeight);

#ifdef kRasterPMDebug
	fprintf(stderr,"Done RGB2CMYK\n ");
#endif

//	UInt32 size = bandBufferSize;
//	gIOModuleProcs.IOWriteProc (fJobID, bandBuffer, &size, false);
//	return;

	// Diffuse our band, using the multidot scatter routines.
	this->DiffusePixMap( bandHeight, bandBuffer, bandWidth ) ;

#ifdef kRasterPMDebug
	fprintf(stderr,"Done diffusing\n ");
#endif

	// initlize some local variables
	CMYKData = 0 ;	
	rowBytesPad = 0;
	CMYKPixelBase = (UInt32*)bandBuffer ;
	fCmpDest = (UInt8*)fCmpDestBlk ;	
	scanBytes = bandWidth / 8 ;	// ignore <8 pixels at end until converter generates mult of 8

	// process scanlines one at a time and send to the printer. 

	for ( scan = 0; scan < bandHeight ; scan++ )
	{
#ifdef kRasterPMDebug
	fprintf(stderr,"\nscan #%d; ", scan);
#endif

		CMYKPixelBase = (UInt32*)bandBuffer + (scan * bandWidth);


		// Set up pointers to our output buffers.		
		cyanLowBuff = fCyanRstr1 ;
		magentaLowBuff = fMgtaRstr1 ;
		yellowLowBuff = fYeloRstr1 ;
		blackLowBuff = fBlkRstr1 ;
		
		// For each of the pixels in our scanline, we'll march through and pick
		// up a pixel at a time, turning on the multiple bits for each component
		// one at a time.
		if ( fNumColorBitsPerPixel == 1 )
		{
			for ( int i = 0 ; i < scanBytes ; i++ )		// Scanbytes counts our output.
			{
				CMYKLow = 0 ;							// Make sure we start w/ white.
				for ( int bit = 0 ; bit < 8 ; bit++ )	// Build up 8 bits at a time.
				{
					// Fetch a fresh pixel and make room for the new bits. These pixels
					// have a single bit of relevent information in each 8 bit component.
					
					CMYKData = *CMYKPixelBase++ ;		// Fetch a pixel
					CMYKLow <<= 1;						// Low intensity bits only
					
		
					// Based on the value stored by error diffusion, we need to set 
					// bits in the long word output. This is done w/ masks for the 
					// particular component bits. Once complete, we'll have 4 
					// components set with a full eight pixels worth of data.
						
					if ( CMYKData != nil )
						CMYKLow |= CMYKData & kOneBitEachMask ;
				}
	
		
				// Once the 8 bits are created for the high and low intensity
				// information, we can copy to the buffers used for this scanline.
								
				*blackLowBuff++ = CMYKLow;				// Single Intensity black.
				*yellowLowBuff++ = CMYKLow >> 8;		// Single Intensity yellow.
				*magentaLowBuff++ = CMYKLow >> 16;		// Single Intensity magenta.
				*cyanLowBuff++ = CMYKLow >> 24;			// Single Intensity cyan.
			}
		}	
	
		// We have our raw data built up, so we're ready to send down compressed
		// data. First, though, let's make sure we take into account the rowbytes
		// pad (Should be zero, but you never know ).
		
		CMYKPixelBase += rowBytesPad ;
#ifdef kRasterPMDebug
		fprintf(stderr,"done munging; ");
#endif		
        
        prevCompressed = true;
		if ( fNumColorBitsPerPixel == 1 )
		{
			
			// Compress the raster, allowing a full raster's worth of bytes to be used.
			
			compSize = scanBytes ;								// Allow use of full scans worth of bytes.
			compressed = EncodeLineMode2( fBlkRstr1, fCmpDest, scanBytes, &compSize ) ;

#ifdef kRasterPMDebug
			fprintf(stderr,"done compression; ");
#endif
			// Once compressed, toggle the compression switch to the engine via a 
			// tagged compression excape sequence that is also used to send a raster.
			
			mCheckAndToggleCompression( compressed, prevCompressed ) ;
			prevCompressed = compressed ;						// Save for next time.
			
			
  			NumToString( compSize, rasterSizeString) ;			// Figure out size of raster.
			this->PCLSendData( &rasterSizeString[1], rasterSizeString[0] ) ;
			this->PCLSendCommand( &PCLTransferRasterPlane ) ;						
			this->PCLSendData( fCmpDest, compSize ) ;			// Send the data itself.
			
			// Final components follow without fancy commenting so we can update 
			// them easily. Perhaps I should make this a macro?
			
			// CYAN is next.
			
 			compSize = scanBytes ;
			compressed = EncodeLineMode2( fCyanRstr1, fCmpDest, scanBytes, &compSize ) ;

			mCheckAndToggleCompression( compressed, prevCompressed ) ;
			prevCompressed = compressed ;
 			NumToString( compSize, rasterSizeString) ;	
			this->PCLSendData( &rasterSizeString[1], rasterSizeString[0] ) ;
			this->PCLSendCommand( &PCLTransferRasterPlane ) ;						
			this->PCLSendData( fCmpDest, compSize ) ;


			// Magenta is next.
			
			compSize = scanBytes ;
			compressed = EncodeLineMode2( fMgtaRstr1, fCmpDest, scanBytes, &compSize ) ;

 			mCheckAndToggleCompression( compressed, prevCompressed ) ;
			prevCompressed = compressed ;

 			NumToString( compSize, rasterSizeString) ;	
			this->PCLSendData( &rasterSizeString[1], rasterSizeString[0] ) ;
			this->PCLSendCommand( &PCLTransferRasterPlane ) ;						
			this->PCLSendData( fCmpDest, compSize ) ;


			// And finally Yellow. 
			
			compSize = scanBytes ;
			compressed = EncodeLineMode2( fYeloRstr1, fCmpDest, scanBytes, &compSize ) ;
 
			mCheckAndToggleCompression( compressed, prevCompressed ) ;
			prevCompressed = compressed ;
			NumToString( compSize, rasterSizeString) ;	
			this->PCLSendData( &rasterSizeString[1], rasterSizeString[0] ) ;
			
			
			// Send the final row command. We'll follow this
			// with the final data for the yellow raster.
			
            
            this->PCLSendCommand( &PCLTransferRasterRowFinal ) ;	

			this->PCLSendData( fCmpDest, compSize ) ;
#ifdef kRasterPMDebug
			fprintf(stderr,"done sending; ");
#endif

		}
		
	}
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::SetBitsPerPixel()
	Description: 	
	Input:
	Output:
		*currentBlack - Number of bits per pixel required for each black pixel.
		*currentColor - Number of bits per pixel for each color pixel.
	Return:

*/
void TEnginePCL::SetBitsPerPixel( int *currentBlack, int *currentColor )
	{
	
        // If, we're working with an entirely black
        // page, then we can simply set the depth to 1, no color.

        *currentColor = fPageDepth == 1? 0: 1 ; 
        *currentBlack = 1 ;
	}
//------------------------------------------------------------------------------
/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::ClosePage()
	Description: Called at the end of a page. We may not have sent the last
		band on the page because the translator may not have discovered any
		objects to be drawn all the way at the bottom. 
	Input:
		abort - True if we're aborting the print job.
	Output: None.
	Return:

*/
void TEnginePCL::ClosePage( Boolean abort )
	{
	
	// At the end of the page, we send a form feed command to kick out the paper.
	// Once that's done, we can free up our page memory and call the base class
	// method.
	
	this->PCLSendCommand( &PCLFormFeed  ) ;		// Force flush

	inherited::ClosePage( abort );
	
	// Record this page's (current page we're completing) depth as the previous,
	// now that we're done with it. This keeps the fLastPageDepth field valid
	// throughout the current page.
	
	fLastPageDepth = fPageDepth ;

	}
//------------------------------------------------------------------------------
/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::CloseJob()
	Description: Called at the end of a page. We may not have sent the last
		band on the page because the translator may not have discovered any
		objects to be drawn all the way at the bottom. 
	Input: None.
	Output: None.
	Return:

*/
void TEnginePCL::CloseJob()
{

	// Send EOJ command
	this->PCLSendCommand( &PCLEndOfJob ) ;

	PMIODelete(ioh);
    this->FreeCommonMemory() ;

	inherited::CloseJob();
}

/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::PCLSendCommand()
	Description: Sends a command to the PCL engine.
	Input:
		cmd - Pointer to the Structure containing the command to send (w/ length) 
		async - True if we want asycnronous transfers.
	Output: None.
	Return:
		OSStatus - zero if all is well, osErr otherwise.

*/
OSStatus TEnginePCL::PCLSendCommand( const TPCLCmd *ourPCLCmd )
{
	OSStatus	err = noErr ;				
	UInt32 		writeSize;
	
//	return noErr;
	if (ourPCLCmd->length > 0)
	{
		writeSize = ourPCLCmd->length;
        err = PMIOWrite(ioh, (Ptr)ourPCLCmd->cmd, writeSize);
	}
		
	return( err ) ;
}
//------------------------------------------------------------------------------

/**-----------------------------------------------------------------------------
	Routine: TEnginePCL::PCLSendData()
	Description: Sends a command to the PCL engine. Data is sent as asynchronous.
	Input:
		data - Pointer to the byte stream for the data.
		length - Number of bytes in this data packet.
		async - True if we want asycnronous transfers.
	Output: None.
	Return:
		OSStatus - zero if all is well, osErr otherwise.

*/
OSStatus TEnginePCL::PCLSendData( UInt8 *data, Size length )
{
	OSStatus	err = noErr;					
	UInt32 		writeSize;
	
//	return noErr;
	if (length > 0 )
	{
		writeSize = length;
        err = PMIOWrite(ioh, (char *)data, (int) writeSize);
	}
		
	return( err ) ;
}
//------------------------------------------------------------------------------
/*******************************************************************************
	Routine:	RGB2CMYK

	Input:		Pointer to band, containing data to work on
		
	Output:		nothing
		
	Desc:		Convert a 32bit pixmap from RGB to CMYK, a pixel at a time.
				This method overrides the base class RGB2CMYK method which
				uses a single cmyk lookup table for all four channels - which
				is OK for the StyleWriter and other old color inkjets, but not the 
				HP PCL printers which require a separate TRC for each channel to
				achieve optimal color result.
				
				This function fails if the Black and Saturation tables are not loaded.
				It calls the bas class RGB2CMYK if the CMYK TRC tables are not loaded.
				It uses the gamma table if loaded, and works without it, if not loaded.
		
 *******************************************************************************/
void	TEnginePCL::RGB2CMYK		( Ptr rgbBuffer, UInt32 bandWidth, UInt32 bandHeight )
{
	UInt8	*CMYKpixelPtr;	// pointer to output cmyk data
	UInt8	*RGBpixelPtr;	// pointer to input rgb data
	UInt8	*gammaTable;	// pointer rgb gamma table
	UInt8	*cyanTRC=nil, 	// pointer to cyan trc table
			*mgtaTRC=nil, 	// pointer to magenta trc table
			*yeloTRC=nil, 	// pointer to yellow trc table
			*blkTRC=nil;	// pointer to black trc table
	UInt8	*kLevelPtr;		// pointer to black inversion table
	UInt8	*sLevelPtr;		// pointer to saturation table
	UInt8	r,b,g,c=0,m=0,y=0,k=0,s,w;	// temporary bytes
	long	prevPixel;		// for performance, prev pixel is cached
	long	thisPixel;
	UInt32	col,row;		// loop counters
	int		IdleCount;		// to call Idle every 10 lines


	// Load TRC tables for the four channels.  The four handles
	// are assumed to be locked IF loaded.
	cyanTRC = (UInt8*) *fCyanTRCHndl;
	mgtaTRC = (UInt8*) *fMgtaTRCHndl;
	yeloTRC = (UInt8*) *fyeloTRCHndl;
	blkTRC = (UInt8*) *fBlkTRCHndl;
	
	// Load black and saturation gamma table ptrs from their handles,
	// which are assumed to be loaded & locked.
	kLevelPtr = (UInt8*) *fKLevelHndl;
	sLevelPtr = (UInt8*) *fSLevelHndl;
	
	// Load rgb table pointer from the gamma handle which is locked.
	gammaTable = (UInt8*) *fGammaHndl;

	prevPixel = -1;
	IdleCount = 0;
	
	RGBpixelPtr = (UInt8 *) rgbBuffer;
	CMYKpixelPtr = (UInt8 *)rgbBuffer;
	
	// Loop once per scan line in band
	for (row = 0; row < bandHeight; row++)
	{
		
		// Loop once per pixel in current scan line
		for (col = 0; col < bandWidth; col++, RGBpixelPtr += 4)
		{
			thisPixel = (*(long *)RGBpixelPtr) & 0x00ffffff;

			// check for white and bump to next pixel
			if (thisPixel == 0x00ffffff)
			{
				*(long *)CMYKpixelPtr = 0L;
				CMYKpixelPtr += 4;
				continue;
			}

			// if current pixel is same as last, use saved cmyk values
			// otherwise do the rgb2cmyk thing
			if (thisPixel != prevPixel)
			{
				prevPixel = thisPixel;

				// Get RGB components, skip Alpha channel
				r = thisPixel >> 16;
				g = thisPixel >> 8;
				b = thisPixel;

				// If we have a Gamma table, run RGB through Gamma Table
				if (gammaTable)
				{
					r = *(gammaTable+r);
					g = *(gammaTable+g);
					b = *(gammaTable+b);
				}

				// Convert RGB to CMY

				// Compute white level
				w = r;
				if (g > w) w = g;
				if (b > w) w = b;

				// Look up black and saturation level from white level
				k = kLevelPtr[w];
				s = sLevelPtr[w];

				// convert rgb to cmy - the old simple way
				c = s - r;
				m = s - g;
				y = s - b;

				// Use TRC tables to further improve the cmyk output
				c = *(cyanTRC+c);
				m = *(mgtaTRC+m);
				y = *(yeloTRC+y);
				k = *(blkTRC+k);
				
			}

			// Save converted pixels
			*CMYKpixelPtr++ = c;
			*CMYKpixelPtr++ = m;
			*CMYKpixelPtr++ = y;
			*CMYKpixelPtr++ = k;
		} // end of for(col) loop
	} // end of for(row) loop
}

/*------------------------------------------------------------------------------
	Routine:	SetUpMultiDotTables

	Input:		Pointers for the tables to be stored.
		
	Output:		Pointers filled with resource handles for the CMYK tables.
		
	Desc:		Fetches the correct tables to do the RGB to CMYK translation
				and halftoning. This function takes into account the current
				quality mode and the paper types to determine how the 
				half-toning and RGB conversion will be done.
		
------------------------------------------------------------------------------*/
void	TEnginePCL::SetUpMultiDotTables	(	Handle			*multiDotCHdl,
												Handle			*multiDotMHdl,
												Handle			*multiDotYHdl,
												Handle			*multiDotKHdl )
{
	short	mDotBaseID = 0;		// base resource ID read in from table

	switch (fPaperType)
	{
		case kPMPaperTypeCoated:
			if (fPrintQuality == kPMQualityNormal)
				mDotBaseID = 49;
			else if (fPrintQuality == kPMQualityBest)
				mDotBaseID = 33;
			else				// No such thing: assume draft/plain
				mDotBaseID = 113;
			
			break;
			
		case kPMPaperTypeGlossy:
		case kPMPaperTypePremium:
			if (fPrintQuality == kPMQualityNormal)
				mDotBaseID = 65;
			else if (fPrintQuality == kPMQualityBest)
				mDotBaseID = 97;
			else				// No such thing: assume draft/plain
				mDotBaseID = 113;
			break;
			
		case kPMPaperTypeTransparency:
		case kPMPaperTypeTShirt:
			mDotBaseID = 129;
			break;
			
		case kPMPaperTypePlain:
		default:
			if (fPrintQuality == kPMQualityNormal)
				mDotBaseID = 1;
			else if (fPrintQuality == kPMQualityBest)
				mDotBaseID = 17;
			else				// assume draft/plain
				mDotBaseID = 113;
			break;	
	}
	
	
	/* 
		Load the MultiDot table based resource for each color component.
		Call this even if the mDotBaseID is 0, as it will set the value
		of the caller's handles to nil.
	*/
	if (mDotBaseID)
		this->LoadTableSet( multiDotCHdl, multiDotMHdl, 
						multiDotYHdl, multiDotKHdl, 
						kMDOTResType, mDotBaseID );
}


/*-----------------------------------------------------------------------------*
 *
 *	SetUpGammaCorrection
 *
 *	Inputs:			gammaHdl		which we load with a handle to a gamma table
 *									given the current Info fields.
 *
 *
 *	Output:			Handle to table set.
 *
 *	Description:	This routine will be called to retrieve the gamma correction
 *					resources necessary for the current state of the imaging
 *					parameters.
 *					Ensure the IDInfo table has been loaded and properly searched,
 *					then get the gammaTable ID from the fIDInfo data member.
 *					
 *					If the resource can't be loaded, we will fail.
 *
 *
 *-----------------------------------------------------------------------------*/
void	TEnginePCL::SetUpGammaCorrection	(	Handle		*gammaHdl )
{
	short	gammaTableID = 0;
	Handle	gammaTableHdl;
	
	/*
		Init passed handle to nil.
	*/
	*gammaHdl = nil;

	switch (fPaperType)
	{
		case kPMPaperTypeCoated:
			if (fPrintQuality == kPMQualityNormal)
				gammaTableID = 5;
			else if (fPrintQuality == kPMQualityBest)
				gammaTableID = 6;
			else				// No such thing: assume draft/plain
				gammaTableID = 2;
			
			break;
			
		case kPMPaperTypeGlossy:
		case kPMPaperTypeTShirt:
			if (fPrintQuality == kPMQualityNormal)
				gammaTableID = 5;
			else if (fPrintQuality == kPMQualityBest)
				gammaTableID = 9;
			else				// No such thing: assume draft/plain
				gammaTableID = 2;
			break;
		
		case kPMPaperTypePremium:
			if (fPrintQuality == kPMQualityNormal)
				gammaTableID = 5;
			else if (fPrintQuality == kPMQualityBest)
				gammaTableID = 6;
			else				// No such thing: assume draft/plain
				gammaTableID = 2;
			break;
				
		case kPMPaperTypeTransparency:
			if (fPrintQuality == kPMQualityNormal)
				gammaTableID = 5;
			else if (fPrintQuality == kPMQualityBest)
				gammaTableID = 17;
			else				// No such thing: assume draft/plain
				gammaTableID = 2;
			break;
			
		case kPMPaperTypePlain:
		default:
			if (fPrintQuality == kPMQualityNormal)
				gammaTableID = 3;
			else if (fPrintQuality == kPMQualityBest)
				gammaTableID = 4;
			else				// assume draft/plain
				gammaTableID = 2;
			break;	
	}


	/*
		Now load the gamma correction resource - if the ID is non-zero.
	*/
	if (gammaTableID != 0)
	{
		gammaTableHdl = GetResource('GAMA',gammaTableID);
		FAILNULL(gammaTableHdl, ResError());
	
		/*
			If we got it, detach the resource,
			lock the handle, and assign it to parameter.
		*/
		DetachResource(gammaTableHdl);
		HLockHi(gammaTableHdl);		
		*gammaHdl = gammaTableHdl;
	}
}

/*-----------------------------------------------------------------------------*
 *
 *	SetUpUCRBG
 *		
 *	Inputs:			blackGenHdl - ptr to Handle to set to Black Generation table
 *					underColorHdl - ptr to Handle to set to UnderColor Removal table
 *
 *
 *	Output:			Passed handles are set if resources are successfully loaded.
 *
 *	Description:	Loads the tables used to do Black Generation and UnderColor
 *					Removal, when converting from RGB to CMYK.
 *					
 *
 *-----------------------------------------------------------------------------*/
void	TEnginePCL::SetUpUCRBG		(	Handle			*blackGenHdl,
										Handle			*underColorHdl )
{
	short		blackGenID = 0;		// ID of BG table to load
	short		underColorID = 0;	// ID of UCR table to load
	volatile 	Handle		bgHdl;			// local copy of resource handle
	volatile 	Handle		ucrHdl;			// local copy of resource handle
	
	/*
		Init return parameters to nil so there's
		no question whether they were loaded or not.
	*/
	*blackGenHdl = nil;
	*underColorHdl = nil;
	
	/*
		Do the same for the local copies.
		Previously, declared them volatile.
	*/
	bgHdl = nil;
	ucrHdl = nil;
		
	switch (fPaperType)
	{
		case kPMPaperTypeCoated:
			if (fPrintQuality == kPMQualityNormal)
				blackGenID = 53;
			else if (fPrintQuality == kPMQualityBest)
				blackGenID = 37;
			else				// No such thing: assume draft/plain
				blackGenID = 117;
			
			break;
			
		case kPMPaperTypeGlossy:
		case kPMPaperTypeTShirt:
		case kPMPaperTypePremium:
			if (fPrintQuality == kPMQualityNormal)
				blackGenID = 69;
			else if (fPrintQuality == kPMQualityBest)
				blackGenID = 101;
			else				// No such thing: assume draft/plain
				blackGenID = 117;
						
			break;
		
		case kPMPaperTypeTransparency:
			blackGenID = 104;
			
			break;
			
		case kPMPaperTypePlain:
		default:
			if (fPrintQuality == kPMQualityNormal)
				blackGenID = 5;
			else if (fPrintQuality == kPMQualityBest)
				blackGenID = 21;
			else				// No such thing: assume draft/plain
				blackGenID = 117;
			
			break;
	}

	if (blackGenID != 0)
		underColorID = blackGenID+1;

	/*
		Attempt to load resources, fail if not successful.
		Otherwise, detach them and lock them high in the heap.
	*/
	TRY
	{
		if (blackGenID != 0)
		{
			bgHdl = GetResource('TRCT',blackGenID);
			FAILNULL(bgHdl, ResError());
			DetachResource(bgHdl);
			HLockHi(bgHdl);	
		}
		
		/*
			Same drill for UCR Table.
		*/
		if (underColorID != 0)
		{
			ucrHdl = GetResource('TRCT',underColorID);
			FAILNULL(ucrHdl, ResError());
			DetachResource(ucrHdl);
			HLockHi(ucrHdl);	
		}
	}
	EXCEPTION
	{
		/*
			Get rid of anything that was loaded.
			If something was successfully loaded
			then it will also have been detached,
			so we can just dispose of it.
		*/
		DisposeNonNullHdl(bgHdl);
		DisposeNonNullHdl(ucrHdl);
		
		/*
			Now REALLY fail.
		*/
		PASSEXCEPTION();
	}
	ENDEXCEPTION
	
	/*
		Set the return parameters.
	*/
	*blackGenHdl = bgHdl;
	*underColorHdl = ucrHdl;
}

/*-----------------------------------------------------------------------------*
 *
 *	SetUpTRCTables
 *					
 *	Inputs:			TRCCyanHdl		Address of a handle to a TRC resource table
 *									for cyan
 *					TRCMagentaHdl	Address of a handle to a TRC resource table
 *									for magenta
 *					TRCYellowHdl	Address of a handle to a TRC resource table 
 *									for yellow
 *					TRCBlackHdl		Address of a handle to a TRC resource table
 *									for black
 *
 *					** NOTE if a parameter passed in is nil, the resource table for
 *							that parameter is not loaded.
 *
 *
 *	Output:			Handle parameters set
 *
 *
 *	Description:	Loads the TRC tables, based on the current imaging settings, which
 *					are used to do any color correction on a component when RGB is
 *					converted to CMYK.  The "TRCBaseID" is read from the ERID table.  The
 *					TRC resType and the baseID are then sent to the LoadTableSet()
 *					method to actually do the reading of the resource tables.  The
 *					generic load method computes the resource number of the TRC
 *					tables as explained in the description of the LoadTableSet()
 *					method..
 *
 *-----------------------------------------------------------------------------*/
void	TEnginePCL::SetUpTRCTables			(	Handle			*TRCCyanHdl,
												Handle			*TRCMagentaHdl,
												Handle			*TRCYellowHdl,
												Handle			*TRCBlackHdl )
{
	short	TRCBaseID = 0;				// base resorce ID obtained from table
	
	switch (fPaperType)
	{
		case kPMPaperTypeCoated:
			if (fPrintQuality == kPMQualityNormal)
				TRCBaseID = 49;
			else if (fPrintQuality == kPMQualityBest)
				TRCBaseID = 33;
			else				// No such thing: assume draft/plain
				TRCBaseID = 113;
			
			break;
			
		case kPMPaperTypeGlossy:
		case kPMPaperTypeTShirt:
		case kPMPaperTypePremium:
			if (fPrintQuality == kPMQualityNormal)
				TRCBaseID = 65;
			else if (fPrintQuality == kPMQualityBest)
				TRCBaseID = 97;
			else				// No such thing: assume draft/plain
				TRCBaseID = 113;
						
			break;
		
		case kPMPaperTypeTransparency:
			TRCBaseID = 104;
			
			break;
			
		case kPMPaperTypePlain:
		default:
			if (fPrintQuality == kPMQualityNormal)
				TRCBaseID = 1;
			else if (fPrintQuality == kPMQualityBest)
				TRCBaseID = 17;
			else				
				TRCBaseID = 113;
			
			break;
	}

	/*	
		Load the TRC table based resource for each color component.
		Call this even if the mDotBaseID is 0, as it will set the value
		of the caller's handles to nil.
	*/
	if (TRCBaseID)
		this->LoadTableSet ( TRCCyanHdl, TRCMagentaHdl, TRCYellowHdl, 
								TRCBlackHdl, kTRCTResType, TRCBaseID );
}

/*-----------------------------------------------------------------------------*
 *
 *	LoadTableSet
 *					
 *	Inputs:			cyanHdl			Address of a handle to a TRC resource table
 *									for cyan
 *					magentaHdl		Address of a handle to a TRC resource table
 *									for magenta
 *					yellowHdl		Address of a handle to a TRC resource table 
 *									for yellow
 *					blackHdl		Address of a handle to a TRC resource table
 *									for black
 *					tableResType	Resource type of tables to load
 *					baseResID		Resource base ID of tables to load
 *
 *					** NOTE if a parameter passed in is nil, the resource table for
 *							that parameter is not loaded.
 *
 *
 *	Output:			Handle parameters set
 *
 *
 *	Description:	Loads for resources whose resType is tableResType and resource
 					ID is baseResID.
 *
 *-----------------------------------------------------------------------------*/
void	TEnginePCL::LoadTableSet	(	Handle			*cyanHdl,
										Handle			*magentaHdl,
										Handle			*yellowHdl,
										Handle			*blackHdl,
										ResType			tableResType,
										short			baseResID	)
{
	Boolean	loadCTable = false;	// indicates whether cyan table should be loaded
	Boolean	loadMTable = false;	// indicates whether magenta table should be loaded
	Boolean	loadYTable = false;	// indicates whether yellow table should be loaded
	Boolean	loadKTable = false;	// indicates whether black table should be loaded
	
	//following declared volatile, since there are used in the exception frame
	volatile 	Handle	cHdl = nil;	// local copy of return parameter
	volatile 	Handle	mHdl = nil;	// local copy of return parameter
	volatile 	Handle	yHdl = nil;	// local copy of return parameter
	volatile 	Handle	kHdl = nil;	// local copy of return parameter

        /*
		If address of handles passed in are non-zero, set corresponding handles to 
		nil to insure that the caller will not try to access them in the case that
		the resource was not found or the base ID was zero.
	*/
	if (cyanHdl != nil)
	{
		loadCTable = true;
		
		// If the handle isn't nil already, then free it before setting to
		// nil prior to our loading of new handle.
		
		DisposeNonNullHdl( *cyanHdl ) ;
		*cyanHdl = nil;
	}
	
	if (magentaHdl != nil)
	{
		loadMTable = true;

		// If the handle isn't nil already, then free it before setting to
		// nil prior to our loading of new handle.
		
		DisposeNonNullHdl( *magentaHdl ) ;
		*magentaHdl = nil;
	}
	
	if (yellowHdl != nil)
	{
		loadYTable = true;

		// If the handle isn't nil already, then free it before setting to
		// nil prior to our loading of new handle.
		
		DisposeNonNullHdl( *yellowHdl ) ;
		*yellowHdl = nil;
	}
	
	if (blackHdl != nil)
	{
		loadKTable = true;

		// If the handle isn't nil already, then free it before setting to
		// nil prior to our loading of new handle.
		
		DisposeNonNullHdl( *blackHdl ) ;
		*blackHdl = nil;
	}
	
	/*
		Now attempt to load the resource tables - if any are specified.
	*/
	if (baseResID != 0)
	{
		TRY
		{			
			if (loadKTable == true)
			{
				// Load the "tableResType" table for the black component
				kHdl = GetResource(tableResType, baseResID);
				FAILNULL(kHdl, ResError());
			
				/*
					If we got it, detach the resource and lock
					the handle and set the return parameter.
				*/
				DetachResource(kHdl);
				HLockHi(kHdl);
				*blackHdl = kHdl;
				baseResID++;
			}
			
			if (loadCTable == true)
			{
				// Load the "tableResType" table for the cyan component
				cHdl = GetResource(tableResType, baseResID);
				FAILNULL(cHdl, ResError());
			
				/*
					If we got it, detach the resource and lock
					the handle and set the return parameter.
				*/
				DetachResource(cHdl);
				HLockHi(cHdl);
				*cyanHdl = cHdl;
				baseResID++;
			}
			
			if (loadMTable == true)
			{
				// Load the "tableResType" table for the cyan component
				mHdl = GetResource(tableResType, baseResID);
				FAILNULL(mHdl, ResError());
			
				/*
					If we got it, detach the resource and lock
					the handle and set the return parameter.
				*/
				DetachResource(mHdl);
				HLockHi(mHdl);
				*magentaHdl = mHdl;
				baseResID++;
			}
	
			if (loadYTable == true)
			{
				// Load the "tableResType" table for the yellow component
				yHdl = GetResource(tableResType, baseResID);
				FAILNULL(yHdl, ResError());
			
				/*
					If we got it, detach the resource and lock
					the handle and set the return parameter.
				*/
				DetachResource(yHdl);
				HLockHi(yHdl);
				*yellowHdl = yHdl;
			}
		}
		EXCEPTION
		{
			/*
				In each failure case, any resource that was
				successfully loaded will have been detached,
				so we should just dispose of it (and set it
				to nil - which the DisposeNonNullHdl Macro does).
			*/
			DisposeNonNullHdl(kHdl);
			DisposeNonNullHdl(cHdl);
			DisposeNonNullHdl(mHdl);
			DisposeNonNullHdl(yHdl);
			/*
				Now REALLY fail.
			*/
			PASSEXCEPTION();
		}
		ENDEXCEPTION
	}
}

/*******************************************************************************
 *
 *	IssueEngineCmds
 *
 *	Inputs:			None.
 *
 *
 *	Output:			None
 *
 *
 *	Description:	Issues Engine Setup command based on jobTicketExt information.
 * 					This was a handy way to get the proper PCL commands generated
 					at the beginning of a page, with all the requisite decisions
 					made in one place.
 *

 *******************************************************************************/
void		TEnginePCL::IssueEngineCmds		()
{
	
	/*
		send paper type command.
	*/
	switch (fPaperType)
	{
		case kPMPaperTypeCoated:
			this->PCLSendCommand( &PCLCoatedPaperTypeCmd ) ;
			break;
			
		case kPMPaperTypeGlossy:
		case kPMPaperTypePremium:
			this->PCLSendCommand( &PCLGlossyPaperTypeCmd ) ;
			break;
			
		case kPMPaperTypeTransparency:
		case kPMPaperTypeTShirt:
			this->PCLSendCommand( &PCLTranspPaperTypeCmd ) ;
			break;
		
		case kPMPaperTypePlain:
		default:
			this->PCLSendCommand( &PCLPlainPaperTypeCmd ) ;
			break;
	}
	
	/*
		send printquality command.
	*/
	switch (fPrintQuality)
	{
		case kPMQualityBest:
			this->PCLSendCommand( &PCLBestQualityCmd ) ;
			this->PCLSendCommand( &PCLBestUnitOfMeasureCmd ) ;
			break;
			
		case kPMQualityNormal:
			this->PCLSendCommand( &PCLNormalQualityCmd) ;
			break;
		
		case kPMQualityDraft:
		default:
			this->PCLSendCommand( &PCLDraftQualityCmd ) ;
			break;
	}
		
	/*
		send graphicsType command.
	*/
	this->PCLSendCommand( &PCLGraphicsTypeCmd ) ;
		
	/*
		send ConfigRaster command.
	*/
	switch (fPrintQuality)
	{
		case kPMQualityBest:			
            this->PCLSendCommand( &PCLBestPlainColorCmd );
			break;
			
		case kPMQualityNormal:
            this->PCLSendCommand( &PCLNormalPlainColorCmd );
			break;
		
		case kPMQualityDraft :
		default:
            this->PCLSendCommand( &PCLDraftPlainColorCmd );
			break;
	}
}

/*******************************************************************************
 *
 *	EncodeLineMode2
 *
 *	Inputs:			
 *			src		- pointer to source data to compress
 *			dst		- pointer to destination data to compress into
 *			srcBytes- size of input data in bytes
 *
 *
 *	Output:	dstSize- size of output compressed data in bytes
 *
 *
 *	Description:	Compresses PCL data using Mode2 (!!) This is an example of 
 * 					data compression for PCL engines. Your engines will likely 
 *					need different compression algorithms. We make not claim to
 *					have the most efficient compression methodology, so use this
 *					with caution.
 *
 *******************************************************************************/
Boolean	TEnginePCL::EncodeLineMode2( UInt8 *src, UInt8 *dst, short srcBytes, short *dstSize)
{
	compression_state	mode2_state;
	char				last_byte = 0;
	char				this_byte = 0;
	char				repeat_count = 0;
	UInt8				*command_ptr = NULL;
	UInt8				*old_dst_ptr = dst;
	short				i;

	mode2_state = MODE2_START;
	for(i=0; i++<srcBytes; )
	{
		this_byte = *src++;
		if (mode2_state == MODE2_IMAGING)
		{
			// This check makes sure that we don't pop into REPEATING mode just for two bytes.
			// Doing so would make the compression worse by one command byte.  In order to check
			// the future byte we must make sure that (i < srcBytes) so that we don't look at an 
			// illegal byte. 
			if  ((last_byte == this_byte) && (i < srcBytes) && (this_byte == *src))
			{
				dst--;
				*command_ptr = repeat_count-1;		// Don't "IMAGE" the preceeding byte
				
				mode2_state = MODE2_REPEATING;
				repeat_count = 1;					// Starting with 2 bytes 
				command_ptr = dst++;
			}
			else
			{
				last_byte = *dst++ = this_byte;
				if (++repeat_count > 126)
				{
					*command_ptr = repeat_count;
					mode2_state = MODE2_START;
				}
			}
		}
		else if (mode2_state == MODE2_REPEATING)
		{
			if (last_byte == this_byte)
			{
				if (++repeat_count > 126)
				{
					*command_ptr = -repeat_count;
					*dst++ = last_byte;
					mode2_state = MODE2_START;
				}
			}
			else
			{
				*command_ptr = -repeat_count;
				*dst++ = last_byte;
				
				command_ptr = dst++;
				last_byte = this_byte;
				mode2_state = MODE2_SINGLE;
			}
		} 
		else if (mode2_state == MODE2_START)
		{
			command_ptr = dst++;
			last_byte = this_byte;
			mode2_state = MODE2_SINGLE;
		}			
		else 							// MODE2_SINGLE
		{
			if (last_byte == this_byte) 
			{
				mode2_state = MODE2_REPEATING;
				repeat_count = 1;  		// Starting with 2 bytes
			}
			else
			{
				mode2_state = MODE2_IMAGING;
				*dst++ = last_byte;
				last_byte = *dst++ = this_byte;
				repeat_count = 1; 		// Starting with 2 bytes
			}
		}				
		
	}  

	if (mode2_state == MODE2_SINGLE)
	{
		if (this_byte)
		{
			*command_ptr = 0;
			*dst++ = this_byte;
		}
		else
			dst = command_ptr;			// Ignore trailing zeros
	} 
	else if (mode2_state == MODE2_IMAGING)
	{
		// No need for terminating conditions other than this_byte == 0 because in order
		// to be here there has to be more than one byte that is different.  Not all of the
		// bytes can be zero. 
		while (this_byte == 0)
		{
			dst--;						// Ignore trailing zeros
			repeat_count--;
			this_byte = *(dst - 1);
		}
		*command_ptr = repeat_count;
	} 
	else if (mode2_state == MODE2_REPEATING)
	{
		if (last_byte)
		{
			*command_ptr = -repeat_count;
			*dst++ = last_byte;
		}
		else
			dst = command_ptr;   		// Ignore trailing zeros
	}
	
	*dstSize = dst - old_dst_ptr;
	return( *dstSize > 0);

}

