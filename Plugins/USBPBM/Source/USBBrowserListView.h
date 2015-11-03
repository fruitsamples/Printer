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

#ifndef __USBBrowserListView__
#define	__USBBrowserListView__
/*******************************************************************************
	File:		USBBrowserListView.h
	Contains: 	Public header for CBrowserListView class.
	
	Copyright (C) 2000-2002, Apple Computer, Inc., all rights reserved.

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
				
*******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Includes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define __USB__ 1
#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>
#include <HIToolbox/MacWindows.h>

#undef __USB__

#include <Print/PMPrinterBrowsers.h>
#include "USBPrinterList.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Constants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Type definitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Function prototypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Global variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Classes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:		CBrowserListView

	Description:
		<description>

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class CBrowserListView
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		Methods
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	public:
						CBrowserListView				(
							CFBundleRef					bundleRef,
							PMPrBrowserRef				ref,
							PMPrBrowserCallbacksPtr		callbacks,
							UInt32						numLookupSpecs);
						CBrowserListView				(
							ControlRef					pbUserPaneCtlHdl,
							CFBundleRef					bundleRef,
							PMPrBrowserRef				ref,
							PMPrBrowserCallbacksPtr		callbacks,
							UInt32						numLookupSpecs);
		virtual			~CBrowserListView				(void);
		
		/*
		Interface methods...
		*/
		OSStatus		GetSelectedPrinters				(
							CFArrayRef*					printers);
#if 0
// Replaced with the Carbon Event model.
		OSStatus		Hit								(
							ControlRef					whichControl,
							Point						where,
							ControlPartCode				part);
#endif
		OSStatus		Resize							(
							const Rect&					frameRect);
		OSStatus		WorksetPrinters					(
							CFArrayRef					printers);

		/*
		Data Browser contol callback handlers (public because they are
		called by the actual callback entry points, which are not
		methods of this object).
		*/
		Boolean			Compare							(
							DataBrowserItemID			itemOneID,
							DataBrowserItemID			itemTwoID,
							DataBrowserPropertyID		sortProperty);
		OSStatus		Data							(
							DataBrowserItemID			item,
							DataBrowserPropertyID		property,
							DataBrowserItemDataRef		itemData,
							Boolean						setValue);
		void			ItemNotification				(
							DataBrowserItemID			item, 
							DataBrowserItemNotification	message);

              void			UpdatePrinters					(void);
  	protected:	// None.		

	private:
		/*
		Browser control initialization...
		*/
                void			InitHotplugNotifications			(void);
 		void			CalcControlRects				( const Rect&	viewRect,
                                                                                          Rect*		listRect);
                void			SetUpBrowserView				(void);
		void			InitBrowserControl				( const Rect&	listRect);
		void			InitBrowserHeader				(void);
		void			InitBrowserViewPane				(void);
		void			InitLookupSpecs					(void);
		void			LoadBrowserView					( CItemList*	theList);
		void			MarkWorksetPrinters				(void);
              void			InitPrinterList					(void);
		
		/*
		Other internal methods...
		*/
		OSStatus		CheckForPrinterSelection			(  Boolean	doubleClick);
                
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		Fields
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	public:		// None.
		ControlRef					fUserPane;		// The user pane
		ControlRef					fView;			// Data browser control reference.
		CFBundleRef					fBundleRef;		// The plugin's bundle
	protected:	// None.
	private:
//                int					fInvalidPrinters;
                io_iterator_t				fAddedDevice;		// hotplugged devices added
                io_iterator_t				fRemovedDevice;		// hotplugged devices removed
                IONotificationPortRef			fAddNotification;	// hotplugging notifications from kernel
                IONotificationPortRef			fRemoveNotification;	// hotplugging notifications from kernel
		WindowRef					fWindow;		// Window containing browser view.
		PMPrBrowserRef				fRef;			// IO Browser reference.				
		PMPrBrowserCallbacksPtr		fCallbacks;		// PrintCenter callbacks.
		UInt32						fNumSpecs;		// Number of lookup specs.
		CFDictionaryRef*			fLookupSpecs;	// Lookup specification records.
		CFArrayRef					fWorksetList;	// Array of specs of printers already in Workset.
		CPrinterList*				fPrinters;		// Pointer to list of USB printers.
		ControlUserPaneHitTestUPP	fOldBrowserHT;	// Pointer to original data browser hit test.
		EventHandlerUPP				fUserOptionEventHandlerUPP;
		EventHandlerRef				fUserOptionEventHandlerRef;

}; // CBrowserListView

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Inline functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

#endif	// __CBrowserListView__
