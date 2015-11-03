/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under AppleÕs copyrights in this original Apple software (the
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
	File: Engine.cp

	Contains: Engine Characteristic functions and
			  Interfaces to 'C' Engine Control routines

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

	Change History:

*/

#include "Engine.h"
#include "Exception.h"

// Globals
TEngine *gEngine = nil ;

/*******************************************************************************
	Routine:	TEngine
	Desc: 		Constructor - zeros internal data members

	History:

 *******************************************************************************/
TEngine::TEngine	(	void )
{
	fGammaHndl			= nil;		// clear gamma table handle
	fKLevelHndl			= nil;		// black removal table used in rgb2cmyk
	fSLevelHndl			= nil;		// saturation table used in rgb2cmyk
	fErrDiffuseScanBlk	= nil;
	fErrDiffuseLastScan	= nil;		// clear ptr to last scan storage for error diffusion

	fMdotCyan 			= nil;		// Clear so desctructor works correctly.
	fMdotMgta 			= nil;		// Clear so desctructor works correctly.
	fMdotYelo 			= nil;		// Clear so desctructor works correctly.
	fMdotBlk 			= nil;		// Clear so desctructor works correctly.
	
	fJobIsOpen			= false;	// flag which indicated whether engine starts a new job.

	gEngine				= this;		// Set up our global.
}

/*******************************************************************************
	Routine:	Initialize()
	Desc:		Initialize. Allocates memory for engine globals

	Input:		nothing
	Output:		nothing

	History:

 *******************************************************************************/
void	TEngine::Initialize		(void *jobID )
{
	// Zero count of actual physical pages printed.
	fPageCount = 0;
	fJobID = jobID;
}

/*******************************************************************************
	Routine:	~TEngine()
	Desc: 		Destructor. Frees memory for engine globals

	Input:		nothing
	Output:		nothing

	History:

 *******************************************************************************/
TEngine::~TEngine	(	void )
{

	// Free error diffusion carry-forward buffer.
	if (fErrDiffuseLastScan)
	{
		DisposePtr( fErrDiffuseScanBlk ) ;

		fErrDiffuseScanBlk = nil;
		
		// set pointer back to nil so DiffusePixmap knows to reallocate
		fErrDiffuseLastScan = nil;
	}

	/*
		Dispose of std gamma table, gray gamma table,
		black generation (k) gamma table,
		and saturation (s) table.
	*/
	DisposeNonNullHdl(fGammaHndl);
	DisposeNonNullHdl(fKLevelHndl);
	DisposeNonNullHdl(fSLevelHndl);

	// Remove multidot tables if loaded.
	
	DisposeNonNullHdl( fMdotCyan ) ;
	DisposeNonNullHdl( fMdotMgta ) ;
	DisposeNonNullHdl( fMdotYelo ) ;
	DisposeNonNullHdl( fMdotBlk ) ;
	
	gEngine = nil ;
}

/*******************************************************************************
	Routine:	OpenJob()
	Desc:  		Marks the start of a print job.  Performs tasks that initialize
				services or resources that will be necessary throughout the job.

	Input:		nothing
	Output:		nothing

	History:

 *******************************************************************************/
void	TEngine::OpenJob	( UInt16 paperType, UInt16 printQuality, const PMIOProcs *IOProcs)
{	
	// turn the flag on 
	fJobIsOpen = true;
	
}

/*******************************************************************************
	Routine:	CloseJob()
	Desc:  		Marks the end of a print job

	Input:		nothing
	Output:		nothing

	History:


 *******************************************************************************/
void	TEngine::CloseJob	(	void )
{

}

/*******************************************************************************
	Routine:	OpenPage
	Desc:  		Marks the start of a new page

	Input:		Number of copies - if engine can reprint
				pageDepth - the maximum pixel depth on the page
	Output:		nothing

	History: 

 *******************************************************************************/
void	TEngine::OpenPage	(int copies, short pageDepth)
{

}

/*******************************************************************************
	Routine:	ClosePage
	Desc:  		Marks the end of a page

	Input:		nothing
	Output:		Whether job was aborted

	History:

 *******************************************************************************/
void	TEngine::ClosePage	(	Boolean abort)
{

	// Increment count of actual physical pages printed.
	fPageCount ++;
}

/*******************************************************************************
	Routine:	DiffusePixMap

	Desc: 		Diffuse error in the pixmap caused by more source pixel depth
				than can be produced with the current halftone cell.
				 
				Now Supports diffusion of 8-bit and 32-bit pixmap.
				Took out check for pixmap to be 32-bit deep.

	Input:		Pointer to band to process the pixels of.
	Output:		Pixmap is modified - error diffused.


	Notes:
				Fails if unable to allocate memory

	Change History:
	
 *******************************************************************************/
void	TEngine::DiffusePixMap	( UInt32  bandHeight, Ptr bandBuffer, UInt32 pageWidth )
{
	mHT				lfmHT;				// parameter block for multidrop screen code
	long	localRowBytes;
	long	size,x;
	short	workingHeight;
	short	j;							// loop variable					
	short	numDiffusionOps = 1;		// how many error diffusion ops to do, default 1
	
	localRowBytes = pageWidth * 4;
		
	// Calculate the size of the error buffer required for this band.
	size = 2*localRowBytes + 10; 		
	

	// If we haven't allocated memory for last scan, do so and zero it out
	if (fErrDiffuseLastScan == nil)
	{
		fErrDiffuseScanBlk = NewPtr( size) ;	
		
		// if we didn't get memory, tell user and fail.
		if (fErrDiffuseScanBlk == 0L)
			FAIL( memFullErr);
		
		fErrDiffuseLastScan = fErrDiffuseScanBlk;
		for (x=0; x< size; x++)
			fErrDiffuseLastScan[x] = 0;
	}

	for (j = 1; j <= numDiffusionOps; j++)
	{
		// Load up parameter block for multi dot error diffusion code. Some 
		// of this will depend on whether or not we've subsampled, some of 
		// it doesn't.
	
		lfmHT.sptr = (short *)fErrDiffuseLastScan;
		lfmHT.cdot = *fMdotCyan;
		lfmHT.mdot = *fMdotMgta;
		lfmHT.ydot = *fMdotYelo;
		lfmHT.kdot = *fMdotBlk;
		lfmHT.width = pageWidth;
		lfmHT.pixworp = 0 ;
		lfmHT.worp = localRowBytes ;
	
		workingHeight = bandHeight;
			
		
		// For normal pixmaps, we simply determine the width and height and set 
		// to walk consecutive pixels in consecutive rows.
		
		lfmHT.iptr = (UInt8 *)bandBuffer;
			
		if (workingHeight > 4)	
			lfmHT.height = 4;
		else
			lfmHT.height = workingHeight;
	
		// While we have blocks of 4 to process, keep calling idle before processing
		// each block.  This works out good for idle calls and also good for
		// subsampled portrait bands which has a minimum of 4 scanlines and will 
		// always have an even number of scanlines.
	
		while (lfmHT.height)
		{
			MultidotCMYK( lfmHT );
	
			// Reset base pointer address
			lfmHT.iptr += (localRowBytes * lfmHT.height);
			
			// Subtract off the height we just finished
			workingHeight -= lfmHT.height;
			if (workingHeight > 4)
				lfmHT.height = 4;
			else
				lfmHT.height = workingHeight;
		}
	}
}

/***************************************************************************
	Function Name:		HalftoneStep

	Input Parameters:
		Assumes all relevant variables are assigned by caller, uses caller's variables:
		dotIndex, horizontalError, *errorBand,random,dotPtr.
	Output Parameters:	None
		Updates random, replaces contone data in *contone with proper dot code,
			updates horizontal and vertical errors.
		
	Description:	Does one step for one component in the above described
						halftone algorithm.
				
		Returns:		Void - See Output Parameters
		
	Change History:

---------------------------------------------------------------------------------*/
inline long	HalftoneStep(	short * myHorizontalError,
							long myRandom,
							UInt8 * myContone,
							short * myErrorBand,
							MDot *myDotPtr)			 
{
	short myDotIndex;
	short myBeta;
	long myRandomTemp;
	short myUnnoisySum;

	myDotIndex = *myContone + *myHorizontalError +  *myErrorBand; /* Dot index is contone value + horizontal +vertical errors */
	myUnnoisySum = myDotIndex;				/* save unnoisy sum to be reduced by dot weight, split h/v */ 

	myRandom = (myRandom<<9)+(myRandom)+1;	/* randomize step */ 
	myRandomTemp = myRandom;															

	myRandomTemp = myRandomTemp>>24;		/* Rescale noise to -128 - 127 */ 
	myDotIndex += ((myDotPtr->noise*myRandomTemp)>>myDotPtr->correction) + myDotPtr->offset; /* rescale noise, */
																/* add noise and offset for negative values */ 
	myDotIndex = myDotPtr->dropVols[myDotIndex>>4];	/* get dot code and weight */ 
	*myContone = myDotIndex&kDotCodeMask;				/* replace contone value with dot encoding in contone buffer */ 

	myUnnoisySum -= (myDotIndex>>6)&kWeightMask;	/* subtract dot weight from unnoisySum */ 
	myRandomTemp = myRandomTemp>>myDotPtr->shift;	/* adjust ratio noise shift */ 
	myBeta = myDotPtr->beta + myRandomTemp;			/* beta is vertical fraction */ 
	myRandomTemp = myBeta*myUnnoisySum>>10;		/* divide by 1024 - rescale error to proper range */ 
	*myErrorBand = myRandomTemp;							/* vertical error buffer update */ 
	*myHorizontalError = myUnnoisySum - myRandomTemp;			/* remainder to horizontal error */ 
	return(myRandom);
}																/* on exit, unnoisySum has dot weight */
/***************************************************************************************
	Function Name:		MultidotCMYK

	Input Parameters:	mHT params (halftoning parameters)

	Output Parameters:	None
		
	Description:	Halftones a cmyk buffer in place according to halftoning info in the
					mHT params. Prior to calling MultidotCMYK, the caller must:
		
		1.	Allocate and fill a CMYK buffer of size (width*height*4) bytes,
			with pointer *contone pointing to it's first byte.

		2.	Allocate and clear an error buffer of size (width+5)*2 bytes,
			with pointer *errorBand pointing to its start.  Each component is signed short.  
			(Width is for the vertical error for each byte of the
			last scanline, +5 is for the horizontal error of each element of the last pixel 
			and a direction byte, *2 for each error being a short rather than a byte.)

		3.	Allocate and initialize the dot strength and encoding tables

		4.	Write the band dimensions (width and height) into the structure

		5.	MultidotCMYK overwrites the input buffer with the table specified dot encoding.
	
	Returns:		Void - See Output Parameters
		
	Change History:


**************************************************************************************/
void TEngine::MultidotCMYK(mHT params)
{
	long random;			/* random number to define threshold and H/V ratio noise */
	long width;				/* width is number of pixels to process */
	short *errorBand;		/* pointer to 16 bit signed error band */
	union contoneType contone;
	HorizontalErrors *errControl;		/* pointer to 5 element HorizontalErrors, accesses the direction and 
									memory locations for the horizontal error components */


	width	=	params.width;
	contone.c = params.iptr ; /* point to contone data buffer */
	errControl = (HorizontalErrors *)params.sptr ; /* first 5 integers are signed short, first is direction, 
										other 4 are cyanH, magentaH,yellowH,and blackH*/
	errorBand = params.sptr + 5; /* after 5 entries, remainder is vertical error buffer */

	random = rand()<<16;
	if((errControl->direction&kDirFlagCMYK)!=0)
	{
	/* begin by processing right to left - move pointers to end (rightmost element) */
		contone.c += width*4;
		errorBand+=width*4;
	}
	
	while (params.height--)
	{
		if((errControl->direction&kDirFlagCMYK)==0)
		{
			while(width--)
			{	/* forwards - left to right */
				if(*contone.l==kWhiteCMYK)
				{
					contone.c +=4;
					errorBand +=4;
				}
				else
				{
						/* cyan */
					random = HalftoneStep(&errControl->cyanH,random,contone.c,errorBand,params.cdot);
					contone.c+=1;
					errorBand+=1;
						/* magenta */
					random = HalftoneStep(&errControl->magentaH,random,contone.c,errorBand,params.mdot);
					contone.c+=1;
					errorBand+=1;
						/* yellow */
					random = HalftoneStep(&errControl->yellowH,random,contone.c,errorBand,params.ydot);
					contone.c+=1;
					errorBand+=1;
						/* black */
					random = HalftoneStep(&errControl->blackH,random,contone.c,errorBand,params.kdot);
					contone.c+=1;
					errorBand+=1;
				}
			}
		}
		else  /* backwards - right to left */
		{
			while(width--)
			{
				if(*(contone.l-1)==kWhiteCMYK)
				{
					contone.c -=4;
					errorBand -=4;
				}
				else
				{
						/* black */
					errorBand-=1;
					contone.c-=1;
					random = HalftoneStep(&errControl->blackH,random,contone.c,errorBand,params.kdot);
						/* yellow */
					errorBand-=1;
					contone.c-=1;
					random = HalftoneStep(&errControl->yellowH,random,contone.c,errorBand,params.ydot);
						/* magenta */
					errorBand-=1;
					contone.c-=1;
					random = HalftoneStep(&errControl->magentaH,random,contone.c,errorBand,params.mdot);
						/* cyan */
					errorBand-=1;
					contone.c-=1;
					random = HalftoneStep(&errControl->cyanH,random,contone.c,errorBand,params.cdot);
				}
			}
		}
		width = params.width ; /* Reset width counter. */
		contone.l += width; /* Update the input buffer pointer. */
		errControl->direction ^= kDirFlagCMYK; /* Reverse direction */
	}
}

/*€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€*/


