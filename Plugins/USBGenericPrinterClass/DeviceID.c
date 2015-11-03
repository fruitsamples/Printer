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
/*******************************************************************************
	File:		DeviceID.c
	Contains: 	Utilities to work with IEEE-1284 DeviceID as a CFString.

	Copyright 1999-2002 by Apple Computer, Inc., all rights reserved.

	Description:
	
		See DeviceID.h for a description of IEEE-1284 DeviceID string.
		
		Although this is work references printers, there should be nothing
		printer specific in the code function.
		
		
*******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Pragmas
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Includes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include "DeviceID.h"

#include "USBUtil.h"	// DEBUG macros

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Constants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Type definitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Function prototypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static CFRange
DelimitSubstring( CFStringRef stringToSearch, CFStringRef delim, CFRange bounds, CFStringCompareFlags options );

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Global variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Inlines
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Private Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*-----------------------------------------------------------------------------*

        DelimitSubstring

        Desc:	Search a string from a starting location, looking for a given
        		delimiter. Return the range from the start of the search to the
        		delimiter, or end of string (whichever is shorter).

        In:		stringToSearch		string which contains a substring that we search
        		delim				string which marks the end of the string 
        		bounds				start and length of substring of stringToSearch
        		options				case sensitive, anchored, etc.

        Out:	Range up to the delimiter.

*-----------------------------------------------------------------------------*/
static CFRange
DelimitSubstring( CFStringRef stringToSearch, CFStringRef delim, CFRange bounds, CFStringCompareFlags options )
{
    CFRange 	where_delim,	// where the delimiter was found
                value;
    //
    //	trim leading space by changing bounds
    //
    while ( bounds.length > 0 && CFStringFindWithOptions( stringToSearch, CFSTR(" "), bounds, kCFCompareAnchored, &where_delim ) )
    {
        ++bounds.location;	// drop a leading ' '
        --bounds.length;
    }
    value = bounds;			// assume match to the end of string, may be NULL
    //
    //	find the delimiter in the remaining string
    //
    if (  bounds.length > 0 && CFStringFindWithOptions( stringToSearch, delim, bounds, options, &where_delim ) )
    {
        //
        // match to the delimiter
        //
        value.length = where_delim.location /* delim */ - bounds.location /* start of search */;
    }
    DEBUG_CFString( "\tFind target", stringToSearch );
    DEBUG_CFString( "\tFind pattern", delim );
    DEBUG_ERR( (int) value.location, "\t\tFound %d\n" );
    DEBUG_ERR( (int) value.length, " length %d"  );

    return value;
}

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Public
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*-----------------------------------------------------------------------------*

        DeviceIDCreateValueList

        Desc:	Create a new string for the value list of the specified key.
        		The key may be specified as two strings (an abbreviated form
        		and a standard form). NULL can be passed for either form of 
        		the key.
        		
        		(Although passing NULL for both forms of the key is considered
        		 bad form[!] it is handled correctly.)

        In:		deviceID	the device's IEEE-1284 DeviceID key-value list
        		abbrevKey	the key we're interested in (NULL allowed)
        		key			

        Out:	CFString	the value list 
        		or NULL		key wasn't found in deviceID

*-----------------------------------------------------------------------------*/
CFStringRef
DeviceIDCreateValueList( const CFStringRef deviceID, const CFStringRef abbrevKey, const CFStringRef key )
{
    CFRange 	found = CFRangeMake( -1,0);   // note CFStringFind sets length 0 if string not found
    CFStringRef	valueList = NULL;

	DEBUG_CFString( "---------DeviceIDCreateValueList DeviceID:", deviceID );
    DEBUG_CFString( "---------DeviceIDCreateValueList key:", key );
	DEBUG_CFString( "---------DeviceIDCreateValueList abbrevkey:", abbrevKey );
   if ( NULL != deviceID && NULL != abbrevKey )
        found = CFStringFind( deviceID, abbrevKey, kCFCompareCaseInsensitive );
    if (  NULL != deviceID && NULL != key && found.length <= 0 )
        found = CFStringFind( deviceID, key, kCFCompareCaseInsensitive );
    if ( found.length > 0 )
    {
        // the key is at found
        // the value follows the key until we reach the semi-colon, or end of string
        //
        CFRange	search = CFRangeMake( found.location + found.length,
                                  CFStringGetLength( deviceID ) - (found.location + found.length) );
        //
        // finally extract the string
        //
        valueList = CFStringCreateWithSubstring ( kCFAllocatorDefault, deviceID,
                                                  DelimitSubstring( deviceID, kDeviceIDKeyValuePairDelimiter, search, kCFCompareCaseInsensitive ) );
	DEBUG_CFString( "---------DeviceIDCreateValueList:", valueList );
    }
    return valueList;

}

/*-----------------------------------------------------------------------------*

        DeviceIDSupportedValue

        Desc:	If the lookup contains either the abbrevKey or key
        			isolate the value list from the lookup string.
        		If any value in this value list is on the
        			printerValueList, the printer is supported.
        			
        		We use a case-insensitive match. This provides best compatibility
        		with existing printers.
        		
        		Note the value returned is the complete range for this
        		value from the hardware's value list. This might be useful
        		in diagnostics, or used by the printer module.
  
  	     		Note that lookup with value length 0 is a special case match
  	     		to allow printer modules to match to IEEE1284-non-compliant printers:
  	     		we allow these printer modules to wild-card match to the NULL
  	     		returned from the printer. However, unlike matching a NULL string,
  	     		these modules won't match other printers.

  	     		i.e., If a printer is 1284-non-compliant, it won't return
  	     		DeviceID. In particular, MFG and MDL are NULL. To match to these
  	     		printers, the printer module should specify either "MDL:" or "MFG:"
  	     		We special case to ensure that a 1284-compliant printer won't
  	     		be able to use a printer module designed to match 1284-non-compliant
  	     		printers.
 
 
        In:		
        		printerValueList	list of values from the printer which
        							match key or abbrevKey
        		lookup				a key-value_list from the printer software
        							module which may or may not contain the
        							key
        		abbrevKey, key		keys which describe the printerValueList
                    NOTE: Specifying key in printer module requires an anchored, exact
                        match for the value, where the abbrevKey requires a partial match.
                    e.g., "CMD:PostScript" will match "PostScript" anywhere in the COMMAND SET
                                HW: "CMD:PostScript Lvl II,PCL 6"  SUCCEEDS
                       	  "MDL:DeskJet" will match any "DeskJet" printer
                                HW: "MODEL: Deskjet 660C" SUCCEEDS
                          "MODEL:DeskJet 810,DeskJet 880" specifies an exact match of one value
                                HW: "MDL:DeskJet 660C" FAILS
                                HW: "MDL:DeskJet 880C" FAILS
                                HW: "MDL"DeskJet 810"  SUCCEEDS

        Out:	Return the range in the printerValueList which was matched.
        
        						           1         2         3         4         5
        						 012345678901234567890123456789012345678901234567890

       e.g., PrinterValueList	"PCL 6 Emulation, PostScript Level 2 Emulation"
        	  lookup			"CMD:PostScript"
        	  abbrevKey			"CMD:"
        	  key				"COMMAND SET:"
        	  
        	  returns CFRangeMake( 17, 44 - 17 + 1 )
        	  			i.e., "PostScript Level 2 Emulation"

*-----------------------------------------------------------------------------*/

CFRange
DeviceIDSupportedValue( const CFStringRef printerValuelist, const CFStringRef lookup, const CFStringRef abbrevKey, const CFStringRef key )
{
    CFRange			result = CFRangeMake( -1, 0 );	// fail
    CFOptionFlags	searchKind = kCFCompareCaseInsensitive; 
    int				lengthValueListDelimiter = CFStringGetLength( kDeviceIDKeyValuePairDelimiter );

	if ( printerValuelist && lookup )
    {
		// if the key is found in the lookup string
        //	i.e., the printer module is matching on this key or abbreviated key
        //			return the value list which will match this printer module
        //			e.g, lookup = "CMD:PostScript", abbrevKey = "CMD:", key = "COMMAND SET:"
        //				 => softwareList ="PostScript"
        CFStringRef softwareList = DeviceIDCreateValueList( lookup, abbrevKey, key );
        
        //
        //	if the abbrevKey matched, use the unanchored search
        //
        {
            CFRange lookupRange = CFRangeMake(0,CFStringGetLength(lookup)),
                    configureSearchOptions;
            
            if ( CFStringFindWithOptions( lookup, abbrevKey, lookupRange, kCFCompareCaseInsensitive, &configureSearchOptions ) )
            {
                DEBUG_ERR( noErr, "DeviceIDSupportedValue not matching exact %d\n" );
            	searchKind = kCFCompareCaseInsensitive;
            }
            else
            {
                DEBUG_ERR( noErr, "DeviceIDSupportedValue matching EXACT/not found %d\n" );
                searchKind = kCFCompareCaseInsensitive | kCFCompareAnchored;
            }
        }

		if ( softwareList )
		{
            DEBUG_CFString( "DeviceIDSupportedValue softwareList: ", softwareList );
            DEBUG_CFString( "DeviceIDSupportedValue hardwareList: ", printerValuelist );
			//
			//	Special case to allow printer modules to specify that they
			//	deal only with IEEE1284 non-compliant printers.
			//
			// if the specification calls for matching the NULL string
			//		match iff the printerValuelist wasn't retrieved
			//
			if ( CFStringGetLength( softwareList ) == 0 )
			{
				if ( NULL == printerValuelist )
					result = CFRangeMake( 0, 0 );
			}
			else
			{ 
				// for each value in the value list
                //	e.g., softwareList = "DeskJet 810, DeskJet 880"
                //				=> nextSoftwareValue iterates through: [DeskJet 810], [DeskJet 880]
				CFRange	nextSoftwareValue = CFRangeMake( 0, CFStringGetLength( softwareList ) );
				while ( nextSoftwareValue.location < CFStringGetLength( softwareList )  && result.length <= 0 )
				{
                	CFRange softwareSubstring = DelimitSubstring( softwareList, kDeviceIDValueListDelimiter, nextSoftwareValue, kCFCompareCaseInsensitive );
                	CFStringRef	softwareValue = CFStringCreateWithSubstring( kCFAllocatorDefault, softwareList, softwareSubstring );
                    DEBUG_ERR( (int)nextSoftwareValue.location, "DeviceIDSupportedValue Matching %d " );
                    DEBUG_ERR( (int)nextSoftwareValue.length, " length %d\n" );
                    DEBUG_CFString( "DeviceIDSupportedValue softwareValue: ", softwareValue ); 
					if ( softwareValue )
					{
						CFRange	nextHardwareValue = CFRangeMake( 0, CFStringGetLength( printerValuelist ) );
						// if this value occurs in any value of the printerValueList
						while ( nextHardwareValue.location < CFStringGetLength( printerValuelist )  && result.length <= 0 )
						{
                        	CFRange	hardwareSubstring = DelimitSubstring( printerValuelist, kDeviceIDValueListDelimiter, nextHardwareValue, kCFCompareCaseInsensitive );

                            DEBUG_ERR( (int)nextHardwareValue.location, "DeviceIDSupportedValue Hardware %d " );
                            DEBUG_ERR( (int)nextHardwareValue.length, " length %d\n" );
                         	// return not just the part that matched, but the entire value
                            // may be anchored search!
                            if ( CFStringFindWithOptions( printerValuelist, softwareValue, hardwareSubstring, searchKind, &result ) )
                            {
                                //
                                //	if we're using an unanchored search
                                //		partial value matches are allowed
                                //	otherwise, we must match the entire value or ignore it
                                //
                                DEBUG_ERR( (int)result.location, "Found at %d\n"  );
                                DEBUG_ERR( (int)result.length, " length %d\n" );
                                if ( searchKind == kCFCompareCaseInsensitive )
                                    result = nextHardwareValue;
                                else if ( result.length != nextHardwareValue.length )
                                {
                                    result.length = 0;	// ignore partial matches when using anchored search
                                }
                            }
                            nextHardwareValue.location += hardwareSubstring.length + lengthValueListDelimiter;
                            nextHardwareValue.length -= hardwareSubstring.length + lengthValueListDelimiter;
						}
						CFRelease( softwareValue );
					}
                    nextSoftwareValue.location += softwareSubstring.length + lengthValueListDelimiter;
                    nextSoftwareValue.length -= softwareSubstring.length + lengthValueListDelimiter;
				}
			}
            CFRelease( softwareList );
		}
	}

    DEBUG_CFString( "DeviceIDSupportedValue", lookup );
    DEBUG_CFString( " searching: ", abbrevKey );
    DEBUG_CFString( " printerValuelist: ", printerValuelist );

 	return result;
}


/*-----------------------------------------------------------------------------*

        DeviceIDSupportedLookup

        Desc:	Given a string, report if it contains any supported 
        		1284 DeviceID keywords.
        		
        		This is different from determining a valid DeviceID string.
        		We are not concerned about _all_ required keywords being present--
        		just that at least one keyword is present.
        		
        		Keywords we allow module matching on are
        			COMMAND SET, CMD:	e.g., PostScript, PCL
        			MODEL, MDL:			e.g., DeskJet, Epson Stylus 800
        			MANUFACTURER, MFG:	e.g., Hewlett-Packard, Canon

        		The string may contain multiple keywords, and multiple
        		values for each keyword.
        		
        		e.g., CMD: PCL, PostScript; MDL: Optra E300, Lexmark Optra E310, E315
        		(The last match is possibly dangerous! Responsibility lies with the
        		module developer.)

        		It is not necessary to end with a final semi-colon.
        			
        In:		
        		moduleDeviceID

        Out:	Return true if any supported keyword was found.

*-----------------------------------------------------------------------------*/

int
DeviceIDSupportedLookup( const CFStringRef moduleDeviceID )
{
	int		isvalid = 0;
	CFStringRef	valid = NULL;
	
	valid = DeviceIDCreateValueList( moduleDeviceID, kDeviceIDKeyCommandAbbrev, kDeviceIDKeyCommand );
  	if ( !valid ) valid = DeviceIDCreateValueList( moduleDeviceID, kDeviceIDKeyModelAbbrev, kDeviceIDKeyModel );
 	if ( !valid ) valid = DeviceIDCreateValueList( moduleDeviceID, kDeviceIDKeyManufacturerAbbrev, kDeviceIDKeyManufacturer );
 	
	isvalid = (valid != NULL);

    if ( NULL != valid )  CFRelease( valid );
		
	return isvalid;
}

// eof
