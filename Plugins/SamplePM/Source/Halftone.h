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
	File:		Halftone.h

	Contains:	The public interface for Halftone.cp.

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

	Change History:

*/

#ifndef __Halftone__
#define __Halftone__

#include <ApplicationServices/ApplicationServices.h>

/* Defines */
#define kWhiteCMYK		0x00000000L
#define kWhiteGRAY		0xFF
#define kDotCodeMask	0x3f
#define kWeightMask		0x3ff
#define kDirFlagCMYK	0x0001
#define kDirFlagCMY		0x0002
#define kDirFlagK 		0x0004
#define kDirFlagGRAY	0x0008

/* Structures */

/* data structure defining the first 5 elements of the *sptr structure
	This structure is allocated and initialized by the engine code. */
typedef struct
{
	short	direction;	/* one bit allocated per halftoning method */
	short	cyanH;		/* cyan horizontal error */
	short	magentaH;		/* magenta horizontal error */
	short	yellowH;		/* yellow horizontal error */
	short	blackH;		/* black horizontal error */
} HorizontalErrors ;


union contoneType
{
	UInt8 *c;
	UInt32 *l;
};

// Structure for halftone screen access
typedef struct HalftoneScreen
{
	short xSize;
	short ySize;
	UInt8 cel[1];
}	HalftoneScreen, *HalftoneScreenPtr, **HalftoneScreenHdl;


// Channel MDOT resource structure, so we can acces individual components of the
// MDOT structure for a particular plane. Note that while the first field defines
// the number of bits used, we still need the number of levels to tell us if we have
// 3 or 4 levels coinciding with 2 bits, for instance.
typedef struct MDot
{
	short			depth ;		// Number of bits per pixel.
	short			noise ;		// Magnitude of noise threshold.
	short			correction ;// Restores noise to proper magnitude.
	short			offset ;	// Prevents index from going negative.
	short			shift ;		// Controls noise on hoizontal and vertical ratio.
	short			beta ;		// 1024 - alpha.
	short			dropVols[1] ;// Array of drop volumes.
} MDot, **MDotHandle ;

// Multidot scatter structure. Used as parameter to new multidot halftoning code.
typedef	struct	mHT	
{	// band halftone data:
	long			height;		// band height (16 bit)
	long			width;		// band width	 (16 bit) in pixels
	UInt8 		*iptr;		// pointer to input band
	short			*sptr;		// pointer to carry over data
	MDot 			*cdot;		// cyan multidot pointer
	MDot 			*mdot;		// magenta multidot pointer
	MDot 			*ydot;		// yello multidot pointer
	MDot 			*kdot;		// black multidot pointer
	long 			pixworp;	// pixel offset - used to skip pixels in multidotCMY
	long 			worp;		// # bytes from start of one scan to start of next scan
} mHT;

#endif
