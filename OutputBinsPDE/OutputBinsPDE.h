/*
    OutputBins.h
	Copyright © 2000-2007 Apple Inc. All Rights Reserved.

 */
/*
 IMPORTANT: This Apple software is supplied to you by Apple Inc.,
 ("Apple") in consideration of your agreement to the following terms,
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
 Neither the name, trademarks, service marks or logos of Apple Inc. 
 may be used to endorse or promote products derived from the Apple
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

#import <Cocoa/Cocoa.h>
#import <Print/PDEPluginInterface.h>

/*
	This example PDE does not support any settings
	that are not PPD feature related. Code that could
	be used to read and write settings that are not
	PPD related is ifdef'ed out by this define.
*/
#define PDE_SUPPORTS_NON_PPD_SETTINGS 0

// PPD keyword definitions
static const char* kOutputBinKey = "OutputBin";
#define kNSOutputBinKey @"OutputBin"

// Keys to look up localized data for the UI 
#define kNSOutputBinSummaryKey @"Output Bin" 
#define kNSOutputBinPanelNameKey @"Output Bins" 

// PDE Kind definition
#define kOutputBinsKindID @"com.apple.examples.print.pde.OutputBinsKind"

#if PDE_SUPPORTS_NON_PPD_SETTINGS
//This should be a unique key for our print settings.
#define kOurSettingKey @"com.mycompany.printsettings.ourSettingKey"
// Default value for our key.
#define kOurSettingDefaultCFStringValue @"None"
#endif


@interface OutputBinsPDEPlugIn : NSObject {
}
@end

@interface PDEPluginCallback : NSObject{
}
@end

@interface OutputBinsPDE : NSObject
{
	PDEPluginCallback*  pdePluginCallbackObject;
	NSString*			thePDEName;

	IBOutlet NSPopUpButton *outputBinsPopup;
	IBOutlet NSView *outputBinsView;

	BOOL			useConstraintHandling;
	BOOL			popUpIsInited;
	NSString*		userChoice;
	ppd_file_t		*ppd;
	ppd_option_t	*outputBinOption;
	NSBundle		*pdeBundle;

#if PDE_SUPPORTS_NON_PPD_SETTINGS
	NSString *ourSettingValue;
#endif
}

- (id)initWithCallback:(PDEPluginCallback*) callback;

- (IBAction)requestNewOutputBin:(id)sender;

@end

