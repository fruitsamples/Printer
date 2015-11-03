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
	File: Engine.h

	Contains:
		Definition of Engine Control object and interface

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

	Change History:
    
*/

#ifndef __Engine__
#define __Engine__

#include <stdlib.h>
#include "SneakerObject.h"
#include "Halftone.h"
#include <PrintCore/PMPrinterModule.h>

// defines
#define	DisposeNonNullHdl(h)		if(h!=nil){DisposeHandle((Handle)h);h=nil;}


/**********************************************************************************
 *	Class definition
 **********************************************************************************/
class TEngine : public TSneakerObject
{
	public:
		// Constructor, Destructor, and Initialize functions. The Basics
						TEngine					(	void );
		virtual			~TEngine				(	void );
		virtual	void	Initialize				(	void * fJobID );
		
		// Notify that Engine starts a new job
				Boolean JobIsOpen				(	void ) {	return (fJobIsOpen); };

		// Basic functions for job control and our main printing interface.
		virtual	void	OpenJob					(   UInt16 		paperType, 
													UInt16 		printQuality,
													const PMIOProcs 	*IOProcs);
		virtual	void	CloseJob				(	void );
		virtual	void	OpenPage				(	int				copies,
													short			pageDepth );
		virtual	void	ClosePage				(	Boolean			abort = false );
		virtual	void	PrintBand				(	Ptr 			bandBuffer, 
													UInt32 			bandBufferSize, 
													UInt32 			bandWidth, 
													UInt32 			pageWidth,
													short 			bandDepth ) = 0;
	protected:
	
		// Free up memory allocated for a page.
		virtual void	AllocateCommonMemory	(	UInt32 pageWidth) = 0;
		virtual	void	FreeCommonMemory		(	void ) = 0;
		
		// Halftoning.
		virtual void 	DiffusePixMap			( 	UInt32 		bandHeight, 
													Ptr 		bandBuffer,
													UInt32 		pageWidth );
													
		virtual	void	MultidotCMYK			(mHT params);
		
	private:	// None.
	
	// Fieldsä
	public:		// None.

	protected:
		UInt16			fPaperType;				// Paper type for this job
		UInt16			fPrintQuality;			// Print Quality for this job
		PMIOProcs		fIOProcs;				// IO Procs for communicating with engine
		
		Handle			fGammaHndl;				// Handle to Gamma table if it comes from a resource
		Handle			fDepleteDataOdd;		// Handle to odd depletion lookup table resource
		Handle			fDepleteDataEven;		// Handle to even depletion lookup table resource
		Handle			fKLevelHndl;			// handle to resource to extract Black (K) level from RGB
		Handle			fSLevelHndl;			// handle to resource to extract saturation level from RGB
		short			fLastLandScapeDepth;	// pixel depth of last call to GetLandScapeScan
		short			fPageCount;				// Count of physical pages we've printed.
		Ptr				fErrDiffuseLastScan;	//	pointer to storage of errors on last scan line of a pixmap during error diffusion
		Ptr				fErrDiffuseScanBlk;		// ptr to banker block structure for last scan of error diffusion

		mHT				fmHT;					// parameter block for multidrop screen code
		MDotHandle		fMdotCyan;				// Cyan Multidot resource handle.
		MDotHandle		fMdotMgta;				// Magenta Multidot resource handle.
		MDotHandle		fMdotYelo;				// Yellow Multidot resource handle.
		MDotHandle		fMdotBlk;				// Black Multidot resource handle.
		
		Boolean			fJobIsOpen;			// true if engine starts a new job
		void			*fJobID;			// Printing Manager JobID.
	private:	// None.
	
};

#endif // __Engine__
