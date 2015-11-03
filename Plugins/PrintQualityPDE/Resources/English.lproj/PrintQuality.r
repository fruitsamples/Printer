/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple�s copyrights in this original Apple software (the
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
    PrintQuality.r
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 */
#include <Carbon/Carbon.r>

#include "PrintQuality.h"

resource 'CNTL' (kPrintQualityGroupControlID, "PrintQuality Radio Group", purgeable) {
{120, 72, 260, 272},						/* Rect Top, Left, Bottom, Right */
 1, 										/* initial value */
 invisible,									/* attributes */
 7,											/* max */
 1,											/* min */
 kControlRadioGroupProc,					/* Type */
 0,											/* refcon */
 "PrintQuality Radio Group"					/* Title */
};

resource 'CNTL' (kPrintQualityLowestControlID, "Lowest Quality", purgeable) {
{120, 72, 136, 144},						/* Rect Top, Left, Bottom, Right */
 0, 										/* initial value */
 invisible,									/* attributes */
 1,											/* max */
 0,											/* min */
 kControlRadioButtonAutoToggleProc,			/* Type */
 0,											/* refcon */
 "Lowest"									/* Title */
};

resource 'CNTL' (kPrintQualityInkSaverControlID, "Lowest Quality", purgeable) {
{140, 72, 156, 144},						/* Rect Top, Left, Bottom, Right */
 0, 										/* initial value */
 invisible,									/* attributes */
 1,											/* max */
 0,											/* min */
 kControlRadioButtonAutoToggleProc,			/* Type */
 0,											/* refcon */
 "InkSaver"									/* Title */
};

resource 'CNTL' (kPrintQualityDraftControlID, "Draft Quality", purgeable) {
{160, 72, 176, 144},						/* Rect Top, Left, Bottom, Right */
 0, 										/* initial value */
 invisible,									/* attributes */
 1,											/* max */
 0,											/* min */
 kControlRadioButtonAutoToggleProc,			/* Type */
 0,											/* refcon */
 "Draft"									/* Title */
};

resource 'CNTL' (kPrintQualityNormalControlID, "Normal Quality", purgeable) {
{180, 72, 196, 144},						/* Rect Top, Left, Bottom, Right */
 1, 										/* initial value */
 invisible,									/* attributes */
 1,											/* max */
 0,											/* min */
 kControlRadioButtonAutoToggleProc,			/* Type */
 0,											/* refcon */
 "Normal"									/* Title */
};

resource 'CNTL' (kPrintQualityPhotoControlID, "Photo", purgeable) {
{200, 72, 216, 144},						/* Rect Top, Left, Bottom, Right */
 1, 										/* initial value */
 invisible,									/* attributes */
 1,											/* max */
 0,											/* min */
 kControlRadioButtonAutoToggleProc,			/* Type */
 0,											/* refcon */
 "Photo"									/* Title */
};

resource 'CNTL' (kPrintQualityBestControlID, "Best Quality", purgeable) {
{220, 72, 236, 144},						/* Rect Top, Left, Bottom, Right */
 0, 										/* initial value */
 invisible,									/* attributes */
 1,											/* max */
 0,											/* min */
 kControlRadioButtonAutoToggleProc,			/* Type */
 0,											/* refcon */
 "Best"										/* Title */
};

resource 'CNTL' (kPrintQualityHighestControlID, "Highest Quality", purgeable) {
{240, 72, 256, 144},						/* Rect Top, Left, Bottom, Right */
 0, 										/* initial value */
 invisible,									/* attributes */
 1,											/* max */
 0,											/* min */
 kControlRadioButtonAutoToggleProc,			/* Type */
 0,											/* refcon */
 "Highest"									/* Title */
};


