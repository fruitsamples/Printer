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
	File:		DeviceID.h
	Contains:	Support IEEE-1284 DeviceID as a CFString.

	Copyright 2000-2002 by Apple Computer, Inc., all rights reserved.

	Description:
		IEEE-1284 Device ID is referenced in USB and PPDT (1394.3). It allows
		a computer peripheral to convey information about its required software
		to the host system.
		
		DeviceID is defined as a stream of ASCII bytes, commencing with one 16-bit
		binary integer in Little-Endian format which describes how many bytes
		of data are required by the entire DeviceID.
		
		The stream of bytes is further characterized as a series of
		key-value list pairs. In other words each key can be followed by one
		or more values. Multiple key-value list pairs fill out the DeviceID stream.
		
		Some keys are required: COMMAND SET (or CMD), MANUFACTURER (or MFG),
		and MODEL (or MDL).
		
		One needs to read the first two bytes of DeviceID to allocate storage
		for the complete DeviceID string. Then a second read operation can
		retrieve the entire string.
		
		Often DeviceID is not very large. By allocating a reasonable buffer one
		can fetch most device's DeviceID string on the first read.
	
	A more formal definition of DeviceID.

		<DeviceID> = <Length><Key_ValueList_Pair>+

		<Length> = <low byte of 16 bit integer><high byte of 16 bit integer>
		<Key_ValueList_Pair> = <Key>:<Value>[,<Value>]*;

		<Key> = <ASCII Byte>+
		<Value> = <ASCII Byte>+
		
		Some keys are defined in the standard. The standard specifies that
		keys are case sensitive. White space is allowed in the key.
		
		The standard does not say that values are case-sensitive.
		Lexmark is known to ship printers with mixed-case value:
			i.e., 'CLASS:Printer'

		Required Keys:
			'COMMAND SET' or CMD
			MANUFACTURER or MFG
			MODEL or MDL
		
		Optional Keys:
			CLASS
				Value PRINTER is referenced in the standard.
				
		Observed Keys:
			SN,SERN
				Used by Hewlett-Packard for the serial number.
		
		
*******************************************************************************/
#ifndef __DeviceID__
#define __DeviceID__

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Pragmas
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Includes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <CoreFoundation/CFString.h>

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Constants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define kDeviceIDKeyCommand				CFSTR("COMMAND SET:")
#define kDeviceIDKeyCommandAbbrev		CFSTR( "CMD:" )

#define kDeviceIDKeyManufacturer  		CFSTR("MANUFACTURER:")
#define kDeviceIDKeyManufacturerAbbrev  CFSTR( "MFG:" )

#define kDeviceIDKeyModel  				CFSTR("MODEL:")
#define kDeviceIDKeyModelAbbrev  		CFSTR( "MDL:" )

#define kDeviceIDKeySerial				CFSTR("SN:")
#define kDeviceIDKeySerialAbbrev  		CFSTR("SERN:")

#define kDeviceIDKeyCompatible  		CFSTR("COMPATIBLITY ID:")
#define kDeviceIDKeyCompatibleAbbrev	CFSTR("CID:")

// delimiters
#define kDeviceIDKeyValuePairDelimiter  CFSTR(";")
#define kDeviceIDValueListDelimiter		CFSTR(",")

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Type definitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Function prototypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if defined(__cplusplus)
extern "C" {
#endif
CFStringRef
DeviceIDCreateValueList(	const CFStringRef deviceID,
							const CFStringRef abbrevKey,
							const CFStringRef key );

CFRange
DeviceIDSupportedValue(		const CFStringRef printerValuelist,
							const CFStringRef lookup,
							const CFStringRef abbrevKey,
							const CFStringRef key );

int
DeviceIDSupportedLookup( 	const CFStringRef moduleDeviceID );
#if defined(__cplusplus)
}
#endif


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Global variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Inlines
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#endif