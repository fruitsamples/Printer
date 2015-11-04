/*******************************************************************************
	File:		OutputBinsPDE.m
	Contains: 	Implementation of an example printer module printing dialog extension

	Description:
		Implementation of an example PPD based printer module printing dialog extension

	Copyright © 2000-2007 Apple Inc. All Rights Reserved.
*******************************************************************************/
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


#import "OutputBinsPDE.h"
#import <Print/PMPrintingDialogExtensions.h>

#import <cups/cups.h>
#import <Carbon/Carbon.h>

@implementation OutputBinsPDEPlugIn

- (BOOL)initWithBundle:(NSBundle*)theBundle {
	// We don't need the bundle here but the
	// printing system needs this method defined.
	return YES;
}

- (NSArray*)PDEPanelsForType:(NSString*)pdeType withHostInfo:(id)host
{
	PDEPluginCallback* callback = (PDEPluginCallback*)host;

	NSMutableArray* pdes = [NSMutableArray array];
	if ([pdeType isEqual:(NSString*)kPrinterModuleTypeIDStr])
	{
		OutputBinsPDE* outputBinsPDE = [[[OutputBinsPDE alloc] initWithCallback:callback] autorelease];
		if( outputBinsPDE != nil )
			[pdes addObject:outputBinsPDE];
		else{
			pdes = nil;
		}
	}

	return pdes;
}
@end


@interface OutputBinsPDE (no_warnings)
- (void) updateUI;
- (void) setDefaults;
- (void) setNewUserChoice: (NSString *)newChoice;
- (void) initializePopup;

// Methods used only when running on Tiger
- (ppd_file_t *) getPPDOnTiger;
- (void) getPPDSettingsFromPrintSettingsOnTiger;
- (void) writePPDSettingsToPrintSettingsOnTiger;


#if PDE_SUPPORTS_NON_PPD_SETTINGS
- (void) getNonPPDSettingsFromPrintSettings;
- (void) writeNonPPDSettingsToPrintSettings;
#endif	/* PDE_SUPPORTS_NON_PPD_SETTINGS */

@end

@implementation OutputBinsPDE


- (id)initWithCallback:(PDEPluginCallback*)callback
{
	self = [super init];
		
	// init instance variables
	thePDEName = nil;
	userChoice = nil;
	outputBinOption = nil;
	ppd = nil;
	popUpIsInited = NO;

	pdePluginCallbackObject = [ callback retain ]; 
	
	// Obtain the bundle for use in translating strings.
	pdeBundle = [ [NSBundle bundleForClass:[self class]] retain ];
		
	// This test whether the constraint handling support is in place, i.e. 10.5 and later. 
	useConstraintHandling = [ pdePluginCallbackObject respondsToSelector:@selector(willChangePPDOptionKeyValue:ppdChoice:) ];
	
	if(useConstraintHandling)
		ppd = [ pdePluginCallbackObject ppdFile ];
	else{
		ppd = [ self getPPDOnTiger ];
	}

	// Check to see whether the current PPD supports the OutputBin option/feature
	if(ppd){
		 outputBinOption = ppdFindOption(ppd, kOutputBinKey);
	}
	
	if( outputBinOption != nil ){
	    // Create a default userChoice value, independent of any particular settings.
		[ self setDefaults ] ;
		
		// Register the help book.
		FSRef fsref;
		NSString * path = [pdeBundle bundlePath];
		NSURL * url = [NSURL fileURLWithPath:path];
		if(CFURLGetFSRef((CFURLRef)url, &fsref)){
			OSStatus err = AHRegisterHelpBook(&fsref);
			if(err)
				NSLog(@"OutputBins PDE got an error %d registering its help book!", err);
		}
	}else{
		// This PPD doesn't support the OutputBin option so there is no need to continue.
	    [ pdePluginCallbackObject release ];
	    pdePluginCallbackObject = nil;
		
		[pdeBundle release];
		pdeBundle = nil;

		// release ourselves because nobody else will.
		[ self release ];
		self = nil;
	}
	
	return self;
}

/* The dealloc method is called on object deallocation when garbage collection is not in effect.
    Therefore this is the place to release objects that we've allocated or retained.
*/
- (void)dealloc
{
	[ userChoice release ];
	[ pdePluginCallbackObject release ];
	[ pdeBundle release ];
	[ thePDEName release ];
	
	// We need to release the view since we created it.
	[ outputBinsView release ];

	[super dealloc];
}


/*
    The finalize method is only needed when you have resources or data that
    needs to be released when garbage collection is in effect. Since the NSObjects
	represented by our instance variables are garbage collected, there is no
	need to release them in the finalize method. Since this code example has no
	other resources that need to be deallocated when garbage collection is in effect,
	there is no need for a finalize method in this code.
	
	If your code needs to explicitly deallocate resources that are not garbage
	collected even when garbage collection is in place, remove the ifdef here and add
	your needed deallocations prior to calling the superclass'es finalize.
*/
#if 0
- (void)finalize {
    // Release your resources that are not garbage collected.
    // ...
    
    // Then finalize the superclass.
    [super finalize];
}
#endif

// return the NSView for this PDE. This method has "Get" semantics so
// this code continues to have a reference to the NSView it returns.
- (NSView*)panelView
{
	if (outputBinsView == NULL)
		[NSBundle loadNibNamed:@"OutputBinsPDE" owner:self];
		
	return outputBinsView;
}

// Return the localized name for this PDE. This will be used as its title in the
// menu of printing panes.
- (NSString*)panelName
{
	if (thePDEName == NULL){
		thePDEName = [ [ pdeBundle localizedStringForKey:kNSOutputBinPanelNameKey value:nil table:nil] retain ];
	}
		
	return thePDEName;
}

// return the kind ID for this PDE
- (NSString*)panelKind
{
	return (NSString*)kOutputBinsKindID;
}

// return an NSArray of PPD option keywords supported by this PDE
- (NSArray *)supportedPPDOptionKeys;
{
	return [NSArray arrayWithObjects: kNSOutputBinKey, nil];
}

// This method enables you to catch the help button and determine
// the default print panel help should be shown or not.
// Returning YES will display the default help, returning NO will suppress it
// allowing you to show your own help.
-(BOOL)shouldShowHelp
{
	BOOL showDefaultHelp = true;
	NSString *helpBookName = (NSString *)[ pdeBundle objectForInfoDictionaryKey: @"CFBundleHelpBookName" ] ;
	if(helpBookName){
		OSStatus err = AHGotoPage((CFStringRef)helpBookName, NULL, NULL);
		if(!err)
			showDefaultHelp = false;
	}
	
	return showDefaultHelp;
}

// the PDE panel is about to be shown
- (void)willShow
{
	[self initializePopup];
	[self updateUI];
}		

- (BOOL)shouldHide
{
	return YES;
}

// This method is called when the system has marked a new PPD option/choice. The passed
// in choice is the new setting for this PPD option.
- (void) PPDOptionKeyValueDidChange:(NSString *)option ppdChoice:(NSString *)choice
{
	if(option != nil && choice != nil && [ option isEqualToString: kNSOutputBinKey ] ){
	    [ self setNewUserChoice: choice ];
	}
}

// read the new settings values and update our internal PDE's settings accordingly
- (BOOL)restoreValuesAndReturnError:(NSError **)error
{
	[self setDefaults];
	
	if(useConstraintHandling){
		// In this case, settings corresponding to PPD options are maintained by the implementation and do not come from 
		// the print settings but instead from the cups ppd calls. The currently marked choice is the setting for the
		// PPD option(s).
		ppd_choice_t *currentChoice = ppdFindMarkedChoice(ppd, kOutputBinKey);
		if(currentChoice){
			NSString *choice = [ NSString stringWithCString: currentChoice->choice encoding: NSASCIIStringEncoding ];
			[ self setNewUserChoice: choice ];
		}
	}else{
		[ self getPPDSettingsFromPrintSettingsOnTiger ];
	}

#if PDE_SUPPORTS_NON_PPD_SETTINGS
	[ self getNonPPDSettingsFromPrintSettings ]; 
#endif

	// Now have UI reflect the current settings.
	[self updateUI];
		
	return YES;
}

// set UI/stored values to the print settings
- (BOOL)saveValuesAndReturnError:(NSError **)error
{
	if(!useConstraintHandling){
		// If using constraint handling, the PPD option choices are handled for us so in that
		// case there is no need to save anything for PPD options. If this code is not
		// running on 10.5 then we need to write the PPD settings in a way that is compatible
		// with Tiger.
	
		[ self writePPDSettingsToPrintSettingsOnTiger ];	
	}

#if PDE_SUPPORTS_NON_PPD_SETTINGS
	[ self writeNonPPDSettingsToPrintSettings ]; 
#endif

	return YES;
}

- (void) setNewUserChoice: (NSString *)newChoice
{
    // Retain the choice we want to keep.
    [ newChoice retain ];

    // Release our current choice.
    [ userChoice release ];
    
    // Now save the choice.
    userChoice = newChoice;
}


// move stored values to the UI
- (void) updateUI
{
	if(popUpIsInited){
		NSInteger currentItem = [ outputBinsPopup indexOfItemWithRepresentedObject: userChoice ];
		[ outputBinsPopup selectItemAtIndex: currentItem ];
	}
}

// set the default stored values
- (void) setDefaults
{
	NSString *choice = [ NSString stringWithCString: outputBinOption->defchoice encoding: NSASCIIStringEncoding ];
	[ self setNewUserChoice: choice ];
}

// initialize the Popup Menu
- (void) initializePopup;
{
	if(!popUpIsInited){
		NSInteger i;
		NSMenu *menu = nil;
		
		// clear out the menu
		[outputBinsPopup removeAllItems];
		// get the menu associated with the popup
		menu = [outputBinsPopup menu];

		// add the choices to the menu
		for( i = 0 ; i < outputBinOption->num_choices; i++)
		{
			ppd_choice_t choice = outputBinOption->choices[i];
			
			// add the item text to the menu
			NSString *itemTitle = [ NSString stringWithCString: choice.text encoding: NSUTF8StringEncoding];
			[outputBinsPopup addItemWithTitle: itemTitle];
			
			// attach the PPD choice NSString object to the menu item
			NSString *itemChoice = [ NSString stringWithCString: choice.choice encoding: NSASCIIStringEncoding];
			[[menu itemAtIndex:i] setRepresentedObject:itemChoice];
		}
		popUpIsInited = YES;
	}
}

// format info for Summary panel
- (NSDictionary *) summaryInfo
{
	NSMutableDictionary *summaryInfoDict = nil;
	const char *userChoiceStr = [ userChoice cStringUsingEncoding: NSASCIIStringEncoding];
	ppd_choice_t *choice = ppdFindChoice(outputBinOption, userChoiceStr);
	if(choice){
		NSString *summaryTitle = [ pdeBundle localizedStringForKey:kNSOutputBinSummaryKey value:nil table:nil];
		summaryInfoDict = [NSMutableDictionary dictionaryWithCapacity:0];
		[summaryInfoDict setObject: [ NSString stringWithCString: choice->text encoding: NSASCIIStringEncoding ] forKey: summaryTitle ];
	}
	return summaryInfoDict;
}

// action method called when a new output bin is selected
- (IBAction)requestNewOutputBin:(id)sender
{
	// Get the currently selected menu item 
	NSInteger index = [(NSPopUpButton *)sender indexOfSelectedItem];
	NSMenuItem *menuItem = [[(NSPopUpButton *)sender menu] itemAtIndex:index];
	NSString *newChoice = [NSString stringWithString: (NSString *)[menuItem representedObject]];

	if(useConstraintHandling)
	{
	    // request the PPD be marked with the new UI setting
	    BOOL reqGranted = [pdePluginCallbackObject willChangePPDOptionKeyValue:kNSOutputBinKey ppdChoice: newChoice];
	    
	    if(reqGranted)
		    // update the stored value to match the new UI
		    [ self setNewUserChoice: newChoice ];
	    else{
			// Restore the old choice.
			index = [ outputBinsPopup indexOfItemWithRepresentedObject: userChoice ];
			[ sender selectItemAtIndex: index ];
		}
	}else{
		// Update our internal value to reflect the UI
	    [ self setNewUserChoice: newChoice ];
	}
}

#if PDE_SUPPORTS_NON_PPD_SETTINGS
/*	
	The purpose of getNonPPDSettingsFromPrintSettings and writeNonPPDSettingsToPrintSettings is
	to show how in a Cocoa PDE to read values from the print settings and how to write them out.

	Missing from this code is setting a default value in case getNonPPDSettingsFromPrintSettings is not called, 
	handling summary information, etc.  

*/

- (void) getNonPPDSettingsFromPrintSettings
{
	// If this PDE handled any settings other than PPD option settings, this would
	// be the place to get those settings from the PMPrintSettings using PMPrintSettingsGetValue. 
	
	PMPrintSettings settings = [pdePluginCallbackObject printSettings];
	if(settings){
	    OSStatus err = PMPrintSettingsGetValue(settings, (CFStringRef)kOurSettingKey, (CFTypeRef *)&ourSettingValue);
	    if(err){
			// No setting for kOurSettingKey in ticket. Use the default value.
			ourSettingValue = kOurSettingDefaultCFStringValue;
			err = noErr;
	    }	
	}
}

- (void) writeNonPPDSettingsToPrintSettings
{
	// If this PDE handled any settings other than PPD option settings, this would
	// be the place to save those settings to the PMPrintSettings using PMPrintSettingsSetValue. 
	
	PMPrintSettings settings = [pdePluginCallbackObject printSettings];
	if(settings){
	    OSStatus err = PMPrintSettingsSetValue(settings, (CFStringRef)kOurSettingKey, ourSettingValue, kPMUnlocked);
		// do appropriate thing when there is an error. Here we write it
		if(err)
			NSLog(@"OutputBins PDE got an error %d setting the value for %@ !", err, kOurSettingKey);
	}
}

#endif		/* PDE_SUPPORTS_NON_PPD_SETTINGS */

/*   ********* Methods only needed on Tiger **************  */

/*
	These functions are only needed when running on 10.4 Tiger. These only
	need to be 32 bit code.
*/
- (ppd_file_t *)getPPDOnTiger
{
	ppd_file_t *currentPPD = nil;
#if !__LP64__
#ifdef __cplusplus
extern "C" {
#endif 
OSStatus	GetPPDRefFromSession(PMPrintSession printSession, ppd_file_t **ppd);
#ifdef __cplusplus
}       
#endif
		OSStatus err = GetPPDRefFromSession( [ pdePluginCallbackObject printSession ], &currentPPD );
		if(err)
			currentPPD = nil;

#endif	/* !__LP64__ */

	return currentPPD;
}

- (void) getPPDSettingsFromPrintSettingsOnTiger
{
#if  !__LP64__
	// For 10.4 execution (which is 32 bit only)
	PMTicketRef printSettings = NULL;
	// obtain the print setings ticket from the session
	OSStatus err = PMSessionGetTicketFromSession([pdePluginCallbackObject printSession], kPDE_PMPrintSettingsRef, &printSettings);
	if (err == noErr)
	{
		NSMutableDictionary	*ppdDict = nil;
		// obtain the current PPD keyword dictionary
		err = PMTicketGetPPDDict(printSettings, kPMTopLevel, kPMTopLevel, (CFMutableDictionaryRef *)&ppdDict);
		if(err == noErr)
		{
			NSString *choice = nil;
			// get the choice keyword value 
			choice = (NSString*) [ppdDict valueForKey: kNSOutputBinKey];
			if(choice)
				[ self setNewUserChoice: choice ];
		}
	}
#endif	/* !__LP64__ */
}

- (void) writePPDSettingsToPrintSettingsOnTiger
{
#if !__LP64__
	PMTicketRef printSettings = NULL;

	// obtain the print setings ticket from the session
	OSStatus err = PMSessionGetTicketFromSession([pdePluginCallbackObject printSession], kPDE_PMPrintSettingsRef, &printSettings);
	if (err == noErr)
	{
		NSMutableDictionary	*ppdDict = nil;
		// set the current choice in the PPD Dict
		err = PMTicketGetPPDDict(printSettings, kPMTopLevel, kPMTopLevel, (CFMutableDictionaryRef *)&ppdDict);
		if (err == noErr)
		{
			// Save the current user choice
			[ppdDict setObject:userChoice forKey: kNSOutputBinKey];
		}
	}
#endif	/* !__LP64__ */
}

@end
