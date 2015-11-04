/*
	File: InkTool.m
	
	Copyright © 2006-7 Apple Inc. All Rights Reserved
    
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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

	This sample code assumes that you have read Technical Note TN2144 which discusses
	how a tool such as this should behave.  
	See http://developer.apple.com/technotes/tn2005/tn2144.html

*/
#import <stdio.h>
#import <Foundation/Foundation.h>
#import <ApplicationServices/ApplicationServices.h>

#import "InkTool.h"

static void writeSupplies(CFDictionaryRef suppliesDict);
static NSDictionary *inkLevels();
static PMPrinter createPrinterWithID(const char *cStr);

static char *progname;

static void sigterm_handler(int sig)
{
	NSLog(@"%s received SIGTERM", progname);
	exit(1);
}

int main(int argc, char ** argv)
{	
	/* The printer id is always the last parameter.
	 * Any options come before the printer id.
	 */
	progname = strrchr( argv[0], '/' );
	if ( progname )
		progname++; // ignore path separator
	else
		progname = argv[0];

	signal(SIGTERM, sigterm_handler);

	if (argc > 1) {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		PMPrinter			printer = createPrinterWithID(argv[1]); /*	argv[1] is the printer id... */

		// for our sample tool, inkLevel isn't dependent on the PMPrinter
		NSDictionary *levels = inkLevels();
		if (levels != nil)
			writeSupplies((CFDictionaryRef)levels);

		if ( printer )
		{
			PMRelease(printer);
			printer = NULL;
		}
		[pool release];
	}
	else {
		fprintf(stderr, "%s: A printer id must be supplied.\n", progname);
		fprintf(stderr, "Usage: %s <printerID>\n", progname);
	}
	
	return 0;
}

/*!
 * @function		createPrinterWithID
 * @abstract		Return a PMPrinter for the queue with the supplied
 *					printer ID.
 */
static PMPrinter createPrinterWithID(const char *printerIDCStr)
{
	CFStringRef printerID = NULL;
	PMPrinter printer = NULL;
	
	printerID = CFStringCreateWithCString(kCFAllocatorDefault, printerIDCStr, kCFStringEncodingUTF8);
	if (printerID != NULL) {
	
		printer = PMPrinterCreateFromPrinterID(printerID);

		CFRelease(printerID);
		printerID = NULL;
	} else {
		if (printerIDCStr == NULL) printerIDCStr = "(NULL)";
		fprintf(stderr, "%s: createPrinterWithID() failed to create CFString from %s\n", progname, printerIDCStr);
	}
	
	return printer;
}

static void writeSupplies(CFDictionaryRef suppliesDict)
{
	CFDataRef xmlData = NULL;
	if (suppliesDict != NULL) {	
		xmlData = CFPropertyListCreateXMLData(kCFAllocatorDefault, suppliesDict);
		if (xmlData != NULL) {
			ssize_t bytesWritten = 0;
			const UInt8 *buffer = CFDataGetBytePtr(xmlData);
			size_t numBytes = CFDataGetLength(xmlData);
			
			if (buffer != NULL && numBytes > 0) {
				bytesWritten = fwrite(buffer, sizeof(*buffer), numBytes, stdout);
				if (bytesWritten != numBytes) {
					fprintf(stderr, "%s: writeSupplies() write failed.\n", progname );
					if (bytesWritten < 0) fprintf(stderr, "%s: writeSupplies():", progname );
				}
			}
			CFRelease(xmlData);
		} else {
			fprintf(stderr, "%s: writeSupplies() could not create the XML data.\n", progname );
		}
	}
}


static NSDictionary *inkLevels(PMPrinter printer)
{
	#  define kMaxInkColors  16
	NSMutableDictionary *inkDictionary = nil;
	
    NSString *theColors[]={
						@"Black", 
						@"Color/Photo", 
						@"Color/Photo 2",
						@"Color/Photo 3",
						@"Color/Photo 4",
						@"Color/Photo 5",
						@"Color/Photo 6",
						@"Color/Photo 7",
						@"Color/Photo 8",
						@"Color/Photo 9",
						@"Color/Photo 10",
						@"Color/Photo 11",
						@"Color/Photo 12",
						@"Color/Photo 13",
						@"Color/Photo 14",
						@"Color/Photo 15",
						0};
	int i;
	int level[kMaxInkColors];
	
	int cartridgeCount = (rand() % kMaxInkColors) + 1;

	// This code produces a random ink level between 1 and 100 for each
	// color. Your code should determine the ink level for each level based
	// on the actual device being tested.
	for (i = 0; i < kMaxInkColors; i++) {
		level[i] = (rand() % 100) + 1;
	}
	
	inkDictionary = [NSMutableDictionary dictionaryWithCapacity:1];
	NSMutableArray *cartridges = [NSMutableArray arrayWithCapacity:cartridgeCount];
	for (i=0;i < cartridgeCount;i++)
	{
		NSMutableDictionary *cartridge = [NSMutableDictionary dictionaryWithCapacity:7];
		
		// See Technical Note TN2144 for the meaning of these keys and the numbers that should be associated
		// with them: http://developer.apple.com/technotes/tn2005/tn2144.html

		[cartridge setObject:theColors[i] forKey:kPrtMarkerSuppliesDescription];
		[cartridge setObject:[NSNumber numberWithInt:3] forKey:kPrtMarkerSuppliesClass ];
		[cartridge setObject:[NSNumber numberWithInt:6] forKey:kPrtMarkerSuppliesType ];
		[cartridge setObject:[NSNumber numberWithInt:7] forKey:kPrtMarkerSuppliesSupplyUnit];
		[cartridge setObject:[NSNumber numberWithInt:100] forKey:kPrtMarkerSuppliesMaxCapacity];
		
		[cartridge setObject:[NSNumber numberWithInt:level[i]] forKey:kPrtMarkerSuppliesLevel];
		
		[cartridges addObject:cartridge];
	}
	
	[inkDictionary setObject:cartridges forKey:kPrtMarkerSuppliesTable];
//	[inkDictionary setObject:@"Your mileage may vary." forKey:kPrtMarkerSuppliesDisclaimer];
		
	return inkDictionary;
}
