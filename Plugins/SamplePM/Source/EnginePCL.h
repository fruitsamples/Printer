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
	File:				EnginePCL.h

	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Change History:
*/

#ifndef __EnginePCL__
#define	__EnginePCL__

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Includes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Engine.h"
extern "C" {
#include "PMIOHelper.h"
}

// Constants & Typedefs & Structs
typedef struct TPCLCmd
{
	char	length ;					// Store size of the command.
	char	cmd[100] ;					// Maximum size of command is 40 for now. //AQ changed to 100
} TCPLCmd ;


enum compression_state
{
	MODE2_START = 0,
	MODE2_IMAGING,
	MODE2_REPEATING,
	MODE2_SINGLE
};

#define	override	

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:			TEnginePCL

	Description:
		This class implements a common PCL Engine class. The class is
		derived either from the TEngine or the TEnginePyg class, depending
		on whether a Sneaker or a Pygmalion driver is being built. This allows
		Pygmalion drivers to override various methods.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class TEnginePCL	: public TEngine
{
	public:
		// Constructor, Destructor, and Initialize functions. The Basics
							TEnginePCL();
		virtual			~TEnginePCL();

		// Basic functions for job control and our main printing interface. Note,
		// for current PCL printers, we don't ever have a non-color head, so let
		// the base class "FindHeadType" be active, no need to override.
		override	void	OpenJob						(	UInt16 		paperType, 
																UInt16 		printQuality,
																const PMIOProcs 	*IOProcs );
		override	void	OpenPage						(	int			copies,
																short			pageDepth );
		override	void	ClosePage					(	Boolean		abort = false );
		override	void	CloseJob						(	void );
		override	void	PrintBand					(	Ptr 			bandBuffer, 
																UInt32 		bandBufferSize, 
																UInt32 		bandWidth, 
																UInt32 		pageWidth,
																short 		bandDepth) ;

	protected:
		typedef		TEngine			inherited;
	
		// Free up memory allocated for a page.
		override	void	FreeCommonMemory			(	void );
		
		override void	AllocateCommonMemory		(	UInt32 		pageWidth);
																
		void		ClearRasterPointers				(	void );
	

		// Direct I/O functions.
		OSStatus 	PCLSendCommand							( 	const TPCLCmd 	*ourPCLCmd) ;
																	
		OSStatus 	PCLSendData								(	UInt8				*data,
																		Size 				length) ;
																	
		void		PrintCMYK							(	Ptr 			bandBuffer,
																UInt32 		bandBufferSize,
																UInt32 		bandWidth,
																UInt32 		bandHeight) ;

		void		RGB2CMYK								(	 
																Ptr 			rgbBuffer,
																UInt32 		bandWidth,
																UInt32 		bandHeight);	
																	
		void		SetBitsPerPixel					(	int			*numBlack,
																int			*numColor ) ;
																
		void		SetUpMultiDotTables				(	Handle			*multiDotCHdl,
																Handle			*multiDotMHdl,
																Handle			*multiDotYHdl,
																Handle			*multiDotKHdl );
																
		void		SetUpGammaCorrection				(	Handle		*gammaHdl );
		
		void		SetUpUCRBG							(	Handle			*blackGenHdl,
																Handle			*underColorHdl );
		
		void		SetUpTRCTables						(	Handle 			*TRCCyanHdl,
																Handle			*TRCMagentaHdl,
																Handle			*TRCYellowHdl,
																Handle			*TRCBlackHdl );
																
		void		LoadTableSet						(	Handle			*cyanHdl,
																Handle			*magentaHdl,
																Handle			*yellowHdl,
																Handle			*blackHdl,
																ResType			tableResType,
																short			baseResID	);
		void		IssueEngineCmds					(  void );
		Boolean 	EncodeLineMode2					(	UInt8 *src, 
																UInt8 *dst,
																short srcBytes,
						 										short *dstSize);	

        void	skipWhite(int count);
		void		printNonWhite							(	Ptr 			bandBuffer,
																UInt32 		bandBufferSize,
																UInt32 		bandWidth,
																UInt32 		bandHeight) ;
	protected:
                PMIOHelperRef ioh;
		int			fLastBandScan ;			// Last scanline of previous band.
		Boolean		fTablesInitialized;		// Tables initialized flag
		short			fPageDepth;					// the current page's maximum depth
		short			fLastPageDepth ;			// Page depth of the previous page.
		UInt8			*fPixelBuf;					// pointer to memory to encode into
		int			fLastBandDepth;			// the pixel depth of the last band
		int			fNumBlackBitsPerPixel ;	// # of values for each black pixel we send.
		int			fNumColorBitsPerPixel ;	// # of values for each color pixel we send.
		Handle		fSubSampleDelta;			// table of 256 uchars for subsampling



		// Members to keep track of scanlines and blocks

		Ptr			fBlackBlk ;					// Pntr to block for black data.
		Ptr			fCMYBlk ;					// Pntr to block for color data.
		Ptr			fNormalModeBlackBlk;		// Pntr to doubleKscanbuff in normal mode

		UInt8			*fCyanRstr1 ;				// We need pointers into our blocks
		UInt8			*fCyanRstr2 ;				// For Simplicity I've numbered them 
	
		UInt8			*fMgtaRstr1 ;				// Magenta pointers into CMYBlock
		UInt8			*fMgtaRstr2 ;

		UInt8			*fYeloRstr1 ;				// Yellow pointers into CMYBlock
		UInt8			*fYeloRstr2 ;

		UInt8			*fBlkRstr1 ;				// Black pointers into CMYBlock


		// Members to control compression
		
		Ptr			fCmpDestBlk ;	//AQ		// Block for our compression destination
		UInt8			*fCmpDest ;					// Destination for compression.

		// members of rgb2cmyk operation
		Handle		fCyanTRCHndl;				// Handle to Cyan TRC table
		Handle		fMgtaTRCHndl;				// Handle to Magenta TRC table
		Handle		fyeloTRCHndl;				// Handle to Yellow TRC table
		Handle		fBlkTRCHndl;				// Handle to Black TRC table
	};

#endif