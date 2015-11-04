/*
 *  InkTool.h
 
 *  Copyright © 2006-7 Apple Inc. All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Appleâ€™s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


 * Ink Tool
 * A printer driver can point to a tool that can detect printer supply levels.
 * The driver specifies the appropriate tool via a line in the printer's PPD or
 * an entry in the printer's printer info ticket. When the print system wishes
 * to query the printer for supply levels it will execute the tool passing in the
 * queue id. Other optional parameters may also be specified.
 *
 *	<tool>  <queue_id>
 *
 * When executed the tool should return the XML form of a CF or NSDictionary.
 * The dictionary describes each consumable in the printer and provides
 * information about its current state.
 *
 * Currently there is a single defined key for the top level dictionary.
 * This key is prtMarkerSuppliesTable. The value for the prtMarkerSuppliesTable
 * key is an array of dictionaries. Each in the array describes a consumable
 * through a series of key-value pairs. The defines keys and their values are
 * listed below. See RFC 1759 for additional information about these keys.
 *
 */

//NSString *kPrtMarkerSuppliesTable = @"prtMarkerSuppliesTable";
#define kPrtMarkerSuppliesTable @"prtMarkerSuppliesTable"

enum {
	kUnknown = -2,
	kOther = 1
};

/*
 * REQUIRED
 * Key:	prtMarkerSuppliesClass
 * Value: A CFNumber representing an integer
 *	1 = other
 *	3 = supplyThatIsConsumed
 *	4 = supplyThatIsFilled
 */
//NSString *kPrtMarkerSuppliesClass = @"prtMarkerSuppliesClass";
#define kPrtMarkerSuppliesClass @"prtMarkerSuppliesClass"

enum {
	kSupplyThatIsConsumed = 3,
	kSupplyThatIsFilled = 4,
};

/*
 * REQUIRED
 * Key: prtMarkerSuppliesDescription
 * Value: A CFString
 *  A localized description of the supply.
 */
//NSString *kPrtMarkerSuppliesDescription = @"prtMarkerSuppliesDescription";
#define kPrtMarkerSuppliesDescription @"prtMarkerSuppliesDescription"

/*
 * REQUIRED
 * Key: prtMarkerSuppliesMaxCapacity
 * Value: A CFNumber representing an integer.
 *  The maximum capacity of this supply container/receptacle expressed
 *  in prtMarkerSuppliesSupplyUnit. The value (-1) means other and
 *  specifically indicates that the sub-unit places no restrictions
 *  on this parameter. The value (-2) means unknown.
 */
//NSString *kPrtMarkerSuppliesMaxCapacity = @"prtMarkerSuppliesMaxCapacity";
#define kPrtMarkerSuppliesMaxCapacity @"prtMarkerSuppliesMaxCapacity"

/*
 * REQUIRED
 * Key: prtMarkerSuppliesLevel
 * Value: A CFNumber representing an integer
 *  The current level if this supply is a container; the remaining space
 *  if this supply is a receptacle. The value (-2) means unknown.  A
 *  value of (-3) means that the printer knows that the maximum capacity has
 *  not been reached but the precise level is unknown.
 */
//NSString *kPrtMarkerSuppliesLevel = @"prtMarkerSuppliesLevel";
#define kPrtMarkerSuppliesLevel @"prtMarkerSuppliesLevel"

/*
 * REQUIRED
 * Key: prtMarkerSuppliesType	
 * Value: A CFNumber representing an integer
 *  1 = other
 *  2 = unknown
 *  3 - toner
 *  4 = wasteToner
 *  5 = ink
 *  6 = inkCartridge
 *  7 = inkRibbon
 *  8 = wasteInk
 *  9 = opc
 *  10 = developer
 *  11 = fuserOil
 *  12 = solidWax
 *  13 = ribbonWax
 *  14 = wasteWax
 *	15 = fuser
 *	16 = coronaWire
 *	17 = fuserOilWick
 *	18 = cleanerUnit
 *	19 = fuserCleaningPad
 *	20 = transferUnit
 *	21 = tonerCartridge
 *	22 = fuserOiler
 *	23 = water
 *	24 = wasteWater
 *	25 = glueWaterAdditive
 *	26 = wastePaper
 *	27 = bindingSupply
 *	28 = bandingSupply
 *	29 = stitchingWire
 *	30 = shrinkWrap
 *	31 = paperWrap
 *	32 = staples
 *	33 = inserts
 *	34 = covers
 */
//NSString *kPrtMarkerSuppliesType = @"prtMarkerSuppliesType";
#define kPrtMarkerSuppliesType @"prtMarkerSuppliesType"
#define kMaxKnownSuppliesTypeValue	34

/*
 * OPTIONAL
 * Key: prtMarkerSuppliesSupplyUnit
 * Value: A CFString representing an integer
 *	3 = tenThousandthsOfInches
 *	4 = micrometers
 *	12 = thousandthsOfOunces
 *	13 = tenthsOfGrams
 *	14 = hundrethsOfFluidOunces
 *	15 = tenthsOfMilliliters
 */
//NSString *kPrtMarkerSuppliesSupplyUnit = @"prtMarkerSuppliesSupplyUnit";
#define kPrtMarkerSuppliesSupplyUnit @"prtMarkerSuppliesSupplyUnit"

/*
 * OPTIONAL
 * Key: prtMarkerSRGBRepresentation
 * Value: A CFArray containing three CFNumbers.
 *  The red, green, and blue sRGB values to represent on the display
 *  the color of this supply.
 */
//NSString *kPrtMarkerSRGBRepresentation = @"prtMarkerSRGBRepresentation";
#define kPrtMarkerSRGBRepresentation  @"prtMarkerSRGBRepresentation"

/*
 * OPTIONAL
 * Key: prtMarkerPartNumber
 * Value: A CFString holding the part number of the supply.
 */
//NSString *kPrtMarkerPartNumber = @"prtMarkerPartNumber";
#define kPrtMarkerPartNumber @"prtMarkerPartNumber"

/*
 * OPTIONAL
 * Key: kPrtMarkerSuppliesDisclaimer
 * Value: A CFString holding a disclaimer about supply level accuracy.
 */
//NSString *kPrtMarkerSuppliesDisclaimer = @"prtMarkerSuppliesLegalDisclaimer";
#define kPrtMarkerSuppliesDisclaimer @"prtMarkerSuppliesLegalDisclaimer"

