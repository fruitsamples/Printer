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
	File:		USBBrowserListView.cp
	Contains: 	Implementation of CBrowserListView class.
	
	Description:
		The CBrowserListView displays a list of printers in the Printer 
		Browser View.
		
		The base class displays two columns of data: the printer name (and icon)
		and the printer kind.
		
		The display of the data is handled by the Browser Control. The
		CBrowserListView simply supplies the data and handles display
		bookkeeping.
                
		This browser view also handles hot-plugged USB devices.

	Copyright (C) 2000-2002, Apple Computer, Inc., all rights reserved.

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
		
		
*******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Pragmas
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Includes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <HIToolbox/HIToolbox.h>
#include <PrintCore/PMErrors.h>

#include "USBBrowserListView.h"
#include "USBItemList.h"

/*
 * The text and labels are localized using a strings file.
 * You can use the command genstrings to create a strings file from this source. Use:
 *	genstrings -s CopyLocalizedStringFromPlugin ATBrowserListView.cp
 *
 * You may get some complaints from genstrings about this macro, but it will create the file.
 */
#define CopyLocalizedStringFromPlugin(key, comment, pluginBundleRef) \
    CFBundleCopyLocalizedString((pluginBundleRef), (key), (key), NULL)

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Constants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
const char			*kMatchUSBDevicesString = "IOUSBDevice";
/*
Column header constants.

Column numbers (also used for column title indices...
*/
const SInt16			kStatusColumn		=	1;
const SInt16			kNameColumn		=	2;
const SInt16			kKindColumn		=	3;

/*
Number of columns and their property IDs...
*/
const DataBrowserPropertyID		kStatusProperty	=	'stat';
const DataBrowserPropertyID		kNameProperty	=	'name';
const DataBrowserPropertyID		kKindProperty	=	'kind';

/*
Column widths in pixels.
*/
const SInt16		kStatusColumnMinWidth		=	35;
const SInt16		kStatusColumnMaxWidth		=	90;
const SInt16		kStatusColumnDefaultWidth	=	35;
const SInt16		kNameColumnMinWidth			=	30;
const SInt16		kNameColumnMaxWidth			=	300;
const SInt16		kNameColumnDefaultWidth		=	200;
const SInt16		kKindColumnMinWidth			=	30;
const SInt16		kKindColumnMaxWidth			=	300;
const SInt16		kKindColumnDefaultWidth		=	120;

/*
Header button and row heights in pixels.
*/
const SInt16		kRowHeight			=	18;
const SInt16		kHeaderHeight		=	20;

#define			kNullPString		"\p"


/*
Control layout constants, used to calculate control rectangles within the
view bounds.
*/
const SInt16	kViewLeftMargin		=	1;
const SInt16	kViewTopMargin		=	1;
const SInt16	kViewRightMargin	=	1;
const SInt16	kViewBottomMargin	=	1;
const SInt16	kControlGap			=	16;

/*
Resource IDs.
*/

/*
Constants for use with CBrowserListView::CheckForPrinterSelection().
*/
const Boolean	kPrinterItemDoubleClicked	= true;
const Boolean	kPrinterItemSelected		= false;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Type definitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Function prototypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static pascal OSStatus			DataCallback(
									ControlRef			browser, 
									DataBrowserItemID		item,
									DataBrowserPropertyID		property,
									DataBrowserItemDataRef		itemData,
									Boolean						setValue);
static pascal Boolean			CompareCallback	(
									ControlRef			browser, 
									DataBrowserItemID		itemOne, 
									DataBrowserItemID		itemTwo, 
									DataBrowserPropertyID		sortProperty);
static pascal void			ItemNotificationCallback(
									ControlRef			browser,
									DataBrowserItemID		item, 
									DataBrowserItemNotification	message);
static void				USBNotifications(
                                                                        void				*refcon,
                                                                        io_iterator_t			iterator );

static OSStatus USBEventHandler( EventHandlerCallRef inHandlerRef, EventRef inEvent, void *userData );
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Global variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Inlines
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Public
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::CBrowserListView

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Default constructor.
		
		This constructor simply initializes the non-UI-related data
		structures: the printer list. The printer list is populated with
		any printers on the USB bus.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CBrowserListView::CBrowserListView(
	CFBundleRef					bundleRef,
	PMPrBrowserRef				ref,
	PMPrBrowserCallbacksPtr		callbacks,
	UInt32						numLookupSpecs)
{
	/*
	Initialize the fields.
	*/
	fRef = ref;
	fBundleRef = bundleRef;
	fCallbacks = callbacks;
	fNumSpecs = numLookupSpecs;

	/*
	The rest of these have to do with user interface instantiations,
	and can be cleared.
	*/
	fUserPane = NULL;
	fView = NULL;
	fWindow = NULL;
	fLookupSpecs = NULL;
	fWorksetList = NULL;
	fPrinters = NULL;
	fOldBrowserHT = NULL;
 
	fAddedDevice = NULL;
	fAddNotification = NULL;
	fRemovedDevice = NULL;
	fRemoveNotification = NULL;

	fUserOptionEventHandlerUPP = NULL;
	fUserOptionEventHandlerRef = NULL;
	
 	/*
	Get the lookup specification records so that we know what printers
	to browser for.
	*/
	this->InitLookupSpecs();
	
	/*
	Scan for printers.
	*/
	this->InitPrinterList();	
        this->InitHotplugNotifications();
	
}	// CBrowserListView::CBrowserListView

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::CBrowserListView

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Default Constructor
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CBrowserListView::CBrowserListView(
	ControlRef					pbUserPaneCtlHdl,
	CFBundleRef					bundleRef,
	PMPrBrowserRef				ref,
	PMPrBrowserCallbacksPtr		callbacks,
	UInt32						numLookupSpecs
	)
{
	/*
	Initialize the fields.
	*/
	fWindow = GetControlOwner(pbUserPaneCtlHdl);
	fUserPane = pbUserPaneCtlHdl;
	fView = NULL;
	fRef = ref;
	fBundleRef = bundleRef;
	fCallbacks = callbacks;
	fNumSpecs = numLookupSpecs;
	fLookupSpecs = NULL;
	fWorksetList = NULL;
	fPrinters = NULL;
	fOldBrowserHT = NULL;

	fAddedDevice = NULL;
	fAddNotification = NULL;
	fRemovedDevice = NULL;
	fRemoveNotification = NULL;
	fUserOptionEventHandlerUPP = NULL;
	fUserOptionEventHandlerRef = NULL;
	
	// Sets up:
	//	fUserOptionEventHandlerUPP	
	this->InitBrowserViewPane();
	
	this->InitHotplugNotifications();
	
}	// CBrowserListView::CBrowserListView

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::~CBrowserListView

	Input Parameters:
		<none>
		
	Output Parameters:
		<none>
		
	Description:
		Destructor
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
CBrowserListView::~CBrowserListView(void)
{

	/*
	Dispose of the event handler for the user pane
	*/
	if (fUserOptionEventHandlerUPP != NULL)
	{
		/*
		Remove the event handler from the view pane
		*/
		(void)::RemoveEventHandler(fUserOptionEventHandlerRef);
	
		DisposeEventHandlerUPP(fUserOptionEventHandlerUPP);
		fUserOptionEventHandlerUPP = NULL;
	}

	/*
	Dispose of the Browser control. 
	*/  
	if (NULL != fView)
	{
		/*
		* Clear out the Browser callbacks since we are going away
		*/
		DataBrowserCallbacks		callbacks;
		callbacks.version = kDataBrowserLatestCallbacks;
		::InitDataBrowserCallbacks(&callbacks);
		::SetDataBrowserCallbacks(fView, &callbacks);

		/*
		Dispose of the control.
		*/
		::DisposeControl(fView);
		fView = NULL;
	}
	
	/*
	Dispose of the printer data.
	*/
	if (NULL != fPrinters)
	{
		delete fPrinters;
		fPrinters = NULL;
	}
		
        /*
        Release hotplugging notification resources
        */       
        if ( fAddedDevice ) IOObjectRelease( fAddedDevice );
        if ( fAddNotification ) IONotificationPortDestroy( fAddNotification );

        if ( fRemovedDevice ) IOObjectRelease( fRemovedDevice );
        if ( fRemoveNotification ) IONotificationPortDestroy( fRemoveNotification );

	/*
	Dispose of the lookup specs array.
	*/
	if (NULL != fLookupSpecs)
	{
		for (UInt32 i = 0; i < fNumSpecs; i++)
			::CFRelease(fLookupSpecs[i]);
			
		::DisposePtr((Ptr) fLookupSpecs);
		fLookupSpecs = NULL;
	}
			
}	// CBrowserListView::~CBrowserListView

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::GetSelectedPrinters

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		This routine returns information about each selected printer in the 
		browser view. 
		
		If there are no selected printers, the function returns 
		kPrinterBrowserNoPrinters and printers is set to NULL.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
OSStatus
CBrowserListView::GetSelectedPrinters(
	CFArrayRef*		printers)
{
	UInt32				numPrinters;	// Number of selected printers.
	Handle				hSelPrinters;	// Handle to IDs of selected printers.
	CFMutableArrayRef	prArray;		// Array of printer IDs.
	CPrinter*			thePrinter;		// Pointer to current printer.
	UInt32				curPrinter;		// Printer loop index.
	OSStatus			err;			// Error condition.
	
	hSelPrinters = NULL;
	numPrinters = 0;
	thePrinter = NULL;
	err = noErr;
	
	*printers = NULL;
	
	/*
	Allocate a CFArray to hold the printer IDs of the selected
	printers.
	
	NOTE: PrintCenter will call ::CFRelease() on the array when 
	it is no longer needed.
	*/
	prArray = ::CFArrayCreateMutable(::CFAllocatorGetDefault(), 0, 
									 &kCFTypeArrayCallBacks);
	if (NULL == prArray)
		Debugger();
	
	if (NULL != fView)
	{	
		/*
		Get the number of selected printers from the data browser.
		*/
		err = ::GetDataBrowserItemCount(fView, (DataBrowserItemID) fPrinters,
										false, kDataBrowserItemIsSelected,
										&numPrinters);
		if (noErr != err)
   DebugStr("\pCBrowserListView::GetSelectedPrinters GetDataBrowserItemCount");

		if (0 < numPrinters)
		{
			/*
			Allocate a Handle to hold the selected item IDs. The data browser
			expects this to be an empty handle; it will resize it appropriately.
			*/
			hSelPrinters = NewHandle(0);

			err = MemError();
			
			if (NULL == hSelPrinters)
	DebugStr("\pCBrowserListView::GetSelectedPrinters hSelPrinters ==NULL");

			if ((noErr == err) && (NULL != hSelPrinters))
			{
				/*
				Get the list of selected items from the data browser.
				This will be a handle to a block of item IDs, each of which
				is actually a pointer to a CPrinter object.
				*/
				err = ::GetDataBrowserItems(fView, (DataBrowserItemID) fPrinters,
											false, kDataBrowserItemIsSelected,
											hSelPrinters);
				if (noErr != err)
   DebugStr("\pCBrowserListView::GetSelectedPrinters GetDataBrowserItems");
   
				if (noErr == err)
				{
					/*
					Add each printer's printerID to the array.
					*/
					for (curPrinter = 0; curPrinter < numPrinters; curPrinter++)
					{
						/*
						Get a pointer to the current printer.
						*/
						thePrinter = (*((CPrinter***) (hSelPrinters)))[curPrinter];
						
                        ::CFArrayAppendValue(prArray, (void*) (thePrinter->PrinterID()));
					}
				}
				
				::DisposeHandle(hSelPrinters);
			}
		}
	}
	else
	{
		/*
		There is no databrowser control present, so the printer browser module
		must have been loaded without a UI. Just return the entire set of
		printers by adding each printer's printerID to the array.
        
        Do not return printers which have no associated printer module.
		*/
		thePrinter = (CPrinter*) (fPrinters->Head());
		
		while (NULL != thePrinter)
		{
            if ( thePrinter->PrinterID() )	// non-NULL iff there's a printer module
                ::CFArrayAppendValue(prArray, (void*) (thePrinter->PrinterID()));
			
			thePrinter = (CPrinter*) (thePrinter->Next());
		}
	}
	
	*printers = prArray;
	
	if (noErr != err)
		err = kPMNoSelectedPrinters;
		
	return (err);

}	// CBrowserListView::GetSelectedPrinters

/******************************************************************************/

#if 0
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::Hit

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Draws contents of pane.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
OSStatus
CBrowserListView::Hit(
	ControlRef		whichControl,
	Point			where,
	ControlPartCode	part)
{
    part = ::HandleControlClick(whichControl, where, GetCurrentKeyModifiers(), (ControlActionUPP) -1);

    /*
    The control clicked was (presumably) the data browser, in which
    case the event was handled by one of the callback routines,
    so we do nothing.
    */

        return noErr;

}	// CBrowserListView::Hit
#endif
/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::Resize

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Draws contents of pane.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
OSStatus
CBrowserListView::Resize(
	const Rect&		frameRect)
{
	Rect			listRect;		// List view bounding rectangle.

	/*
	The frame has changed size, so resize the controls to fit it.
	*/
	this->CalcControlRects(frameRect, &listRect);
	
	::SizeControl(fView, listRect.right - listRect.left,
						 listRect.bottom - listRect.top);

	return (noErr);

}	// CBrowserListView::Resize

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::WorksetPrinters

	Input Parameters:
		printers	:	Array of info for printers already in Workset.
		
	Output Parameters:
		<method>	:	Error condition.
		
	Description:
		Given a list of selection specification records for printers already 
		in PrintCenter's Workset, this method scans the list of available 
		printers and marks those that are in the Workset.
		
		This function can be called at any time by PrintCenter; generally
		it is called whenever the Workset is updated while the Printer
		Browser window is open.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
OSStatus
CBrowserListView::WorksetPrinters(
	CFArrayRef			printers)
{
	/*
	If there is already a list of Workset printers, replace it with this
	new one.
	*/
	if (NULL != fWorksetList)
		::CFRelease(fWorksetList);
		
	fWorksetList = printers;
	
	/*
	If the printers list has been populated, traverse it and mark the
	printers already added.
	*/
	this->MarkWorksetPrinters();

	return (noErr);

}	// CBrowserListView::WorksetPrinters

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Browser Control Callbacks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::Compare

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<function>	:	true if itemOneID is less than itemTwoID, false
						otherwise.
		
	Description:
		Callback function to compare two items for list sorting purposes.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
Boolean
CBrowserListView::Compare(
	DataBrowserItemID		itemOne,
	DataBrowserItemID		itemTwo,
	DataBrowserPropertyID	sortProperty)
{
	CItem*			theItemOne;		// itemOneID cast to CItem*.
	CItem*			theItemTwo;		// itemOneID cast to CItem*.
	Boolean			oneLessThanTwo;	// true if itemOne < itemTwo.
	
	oneLessThanTwo = false;
	
	/*
	For readability, cast the IDs to pointers to actual objects.
	*/
	theItemOne = (CItem*) itemOne;
	theItemTwo = (CItem*) itemTwo;
	
	/*
	Compare the specified properties of the two items.
	*/
	switch (sortProperty)
	{
		case kStatusProperty:
		{
			/*
			The Status column is not sortable.
			*/
		}
		break;
		
		case kNameProperty:
		{
			/*
			Compare the name strings.
			*/
			oneLessThanTwo
				= (kCFCompareLessThan == ::CFStringCompare(theItemOne->Name(),
														   theItemTwo->Name(),
														   kCFCompareCaseInsensitive));
		}
		break;
		
		case kKindProperty:
		{
			/*
			Compare the kind strings.
			*/
			oneLessThanTwo
				= (kCFCompareLessThan == ::CFStringCompare(theItemOne->Kind(),
														   theItemTwo->Kind(),
														   kCFCompareCaseInsensitive));
		}
		break;

	}
	
	return (oneLessThanTwo);

}	// CBrowserListView::Compare

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::Data

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Callback function to obtain data for a given browser item.
		
		For now, the 
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
OSStatus
CBrowserListView::Data(
	DataBrowserItemID			item,
	DataBrowserPropertyID		property,
	DataBrowserItemDataRef		itemData,
	Boolean						setValue)
{
	#pragma unused(setValue)
	
	OSStatus					err;		// Error condition.
	CItem*						theItem;	// Pointer to the actual CItem.
	CPrinter*					printer;	// theItem cast to a Printer.
	
	/*
	item is a pointer to the item itself.
	*/
	theItem = (CItem*) item;
	printer = (CPrinter*) theItem;
	
	switch (property)
	{
		case kStatusProperty:
		{
			IconRef		theIcon;	// Reference to icon to display.
			
			theIcon = NULL;

			/*
			If the printer is already in the workset, so indicate. PrintCenter
			supplies us with a standard icon to use in this case, already
			registered with Icon Services; use it.
			*/
			if (printer->InWorkset())
			{
				err = ::GetIconRef(kOnSystemDisk, kPMPrBrowserPCCreator,
								   kPMPrBrowserWorksetPrinterIconType,
								   &theIcon);
				if (noErr != err)
					Debugger();
			}
			
			err = ::SetDataBrowserItemDataIcon(itemData, theIcon);

			/*
			Release any icon reference when we're done (the data browser
			acquires it for itself).
			*/
			if (NULL != theIcon)
				(void) ::ReleaseIconRef(theIcon);
		}
		break;
		
		case kNameProperty:
		{
			/*
			Get the item's icon reference. The data browser increments the IconRef's
			reference count automatically, so we don't need to.
			*/
			err = ::SetDataBrowserItemDataIcon(itemData, theItem->Icon());
			
			/*
			Copy the item's name string into the buffer.
			*/
			err = ::SetDataBrowserItemDataText(itemData, theItem->Name());
		}
		break;
		
		case kKindProperty:
		{
			/*
			Copy the item's kind string into the buffer.
			*/
			err = ::SetDataBrowserItemDataText(itemData, theItem->Kind());
		}
		break;
		
		case kDataBrowserItemIsActiveProperty:
		case kDataBrowserItemIsContainerProperty:
		case kDataBrowserContainerIsSortableProperty:
		{
			err = ::SetDataBrowserItemDataBooleanValue(itemData, true);
		}
		break;

		case kDataBrowserItemIsSelectableProperty:
		{
            // Unsupported printers are not selectable. They have NULL PrinterID.
			err = ::SetDataBrowserItemDataBooleanValue(itemData, printer->PrinterID()?true: false);
		}
        break;

		case kDataBrowserItemIsEditableProperty:
		case kDataBrowserContainerIsOpenableProperty:
		case kDataBrowserContainerIsClosableProperty:
		{
			err = ::SetDataBrowserItemDataBooleanValue(itemData, false);
		}
		break;

		default:
		{
			/*
			Do nothing.
			*/
		}
		break;		
	}
	
	return (noErr);
	
}	// CBrowserListView::Data

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::ItemNotification

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Callback function to get notified when certain events occur to
		an item.
				
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
CBrowserListView::ItemNotification(
	DataBrowserItemID			item, 
	DataBrowserItemNotification	message)
{
	DataBrowserItemID	target;			// Current browser target.
	OSStatus		err;			// Error condition.	

	/*
	The state of an item in the browser control has been changed. Determine
	what kind of item it is, and handle the notification appropriately.
	
	To determine what kind of item this is, check the browser target.
	*/
	err = ::GetDataBrowserTarget(fView, &target);
	if (noErr != err)
	{
            DebugStr("\pCBrowserListView::ItemNotification GetDataBrowserTarget");
	}
		
	/*
	Check the message and respond accordingly.
	*/
	switch (message)
	{
		case kDataBrowserItemSelected:
		case kDataBrowserItemDeselected:
		{
                    /*
                    Notify PrintCenter that the user has changed the selection
                    status of at least one printer item, so that it can adjust
                    the appropriate menu items.
                    */
                    err = this->CheckForPrinterSelection(kPrinterItemSelected);
		}
		break;
		
		case kDataBrowserItemDoubleClicked:
		{
                    /*
                    The control clicked was (presumably) the data browser.
                    If a printer has been double-clicked, notify PrintCenter.
                    */
                    err = this->CheckForPrinterSelection(kPrinterItemDoubleClicked);
		}
		break;	// kDataBrowserItemDoubleClicked
		
		default:
		break;
	}
	
}	// CBrowserListView::ItemNotification

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Protected
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Private
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::CalcControlRects

	Input Parameters:
		viewRect	:	Bounding rectangle of browser view.
		
	Output Parameters:
		popUpRect	:	Bounding rectangle for pop-up menu control.
		listRect	:	Bounding rectangle for list view control.
		
	Description:
		Given the browser view bounding rectangle, calculates the bounding
		rectangles for the controls that will appear within the view.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
CBrowserListView::CalcControlRects(
	const Rect&		viewRect,
	Rect*			listRect)
{
	/*
	The list view fills the lower portion of the view rectangle.
	*/
	::SetRect(&(*listRect), viewRect.left + kViewLeftMargin,
                                                        viewRect.top + kViewTopMargin,
							viewRect.right - kViewRightMargin,
							viewRect.bottom - kViewBottomMargin);

}	// CBrowserListView::CalcControlRects
 
/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::CheckForPrinterSelection

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
OSStatus
CBrowserListView::CheckForPrinterSelection(
	Boolean		doubleClick)
{
	DataBrowserItemID	target;			// Current data browser target.
	UInt32				numPrinters;	// Number of selected printers.
	OSStatus			err;			// Error condition.
	
	err = noErr;
	
	err = ::GetDataBrowserTarget(fView, &target);
	if ((CItemList*) target == fPrinters)
	{
		/*
		The data browser is displaying a list of printers. Check
		whether any have been selected.
		*/
		err = ::GetDataBrowserItemCount(fView, (DataBrowserItemID) fPrinters,
										false, kDataBrowserItemIsSelected,
										&numPrinters);
		
		/*
		Notify PrintCenter of the selection status.
		*/
		fCallbacks->selStatus(fRef, (numPrinters > 0), doubleClick);
	}
	
	return (err);

}	// CBrowserListView::CheckForPrinterSelection

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::InitBrowserControl

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Initialize the browser control.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
CBrowserListView::InitBrowserControl(
	const Rect&		listRect)
{
	DataBrowserCallbacks		callbacks;
	OSStatus					err;				// Error condition.

	/*
	Create the browser control, making it initially invisible.
	*/
	err = ::CreateDataBrowserControl(fWindow, &listRect,
									 kDataBrowserListView, &fView);
	if (noErr != err)
            DebugStr("\pCBrowserListView::InitBrowserControl CreateDataBrowserControl" );
	
	/*
	Turn off focus ring
	*/
    Boolean frameAndFocus = false;

	err = ::SetControlData(
		fView, kControlNoPart, 
		kControlDataBrowserIncludesFrameAndFocusTag,
		sizeof(frameAndFocus), &frameAndFocus);
	if (noErr != err)
		Debugger();

	/*
	Embed the control in the view.
	*/
	err = ::EmbedControl(fView, fUserPane);
	if (noErr != err)
            DebugStr("\pCBrowserListView::InitBrowserControl EmbedControl" );
		
	err = ::SetKeyboardFocus(fWindow, fView, kControlNoPart);
	if (noErr != err)
            DebugStr("\pCBrowserListView::InitBrowserControl SetKeyboardFocus" );
		
	/*
	Set the control reference to point to this object.
	*/
	::SetControlReference(fView, (SInt32) this);

	/*
	The embedder control is initially invisible, but we want our control to
	become visible when it does, so make it visible and enable it.
	*/
	err = ::SetControlVisibility(fView, true, true);
	if (noErr != err)
            DebugStr("\pCBrowserListView::InitBrowserControl SetControlVisibility" );

	err = ::ActivateControl(fView);
	if (noErr != err)
            DebugStr("\pCBrowserListView::InitBrowserControl ActivateControl" );
	
	/*
	Attach the callback functions.
	
	NOTE: The version number *must* be stored in the callbacks parameter
	block before calling InitDataBrowserCallbacks(), or the call will
	fail and the data browser will not display any data!
	*/
	callbacks.version = kDataBrowserLatestCallbacks;
	err = ::InitDataBrowserCallbacks(&callbacks);

	callbacks.u.v1.itemCompareCallback
		= NewDataBrowserItemCompareUPP(CompareCallback);
		
	callbacks.u.v1.itemDataCallback
		= NewDataBrowserItemDataUPP(DataCallback);
		
	callbacks.u.v1.itemNotificationCallback
		= NewDataBrowserItemNotificationUPP(ItemNotificationCallback);
		
	err = ::SetDataBrowserCallbacks(fView, &callbacks);
	
	// Set the event handler for our user pane
	fUserOptionEventHandlerUPP = NewEventHandlerUPP( USBEventHandler );
	static EventTypeSpec userOptionEvents[] =
	{
		{ kEventClassControl, kEventControlHit },
		{ kEventClassKeyboard, kEventRawKeyUp }
	};
	InstallControlEventHandler( fUserPane, fUserOptionEventHandlerUPP, 2, userOptionEvents, this,  &fUserOptionEventHandlerRef );

	/*
	Initialize browser control header buttons.
	*/
	this->InitBrowserHeader();
	
}	// CBrowserListView::InitBrowserControl


/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::InitBrowserHeader

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Initialize the browser view header.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
CBrowserListView::InitBrowserHeader(void)
{
	DataBrowserListViewColumnDesc	cData;		// Column data block.
	OSStatus						err;		// Error condition.
	
	/*
	Define a font style record with just the basics.
	*/
	ControlFontStyleRec fontStyle = {
										kControlUseFontMask, 
										kControlFontSmallSystemFont, 
										0, 0, 0, 0, 
										{ 0, 0, 0 },
										{ 0, 0, 0 }
									};
									
	::SetControlFontStyle(fView, &fontStyle);
	
	// Set the header button font style
	fontStyle.just = teFlushDefault;
	fontStyle.font = kControlFontViewSystemFont;
	fontStyle.style = normal;
	fontStyle.size = 0;

	/*
	Set up the column titles and properties. The browser view has three
	columns: Status, Name, and Kind. Status is a narrow, 16-pixel column
	on the left, followed by Name, followed by Kind. Default sort is on
	the Name column.
	
	First, the Status column. Status displays an icon and is not sortable.
	Its header button displays nothing.
	*/
	cData.propertyDesc.propertyID = kStatusProperty;
	cData.propertyDesc.propertyType = kDataBrowserIconType;
	cData.propertyDesc.propertyFlags = kDataBrowserDefaultPropertyFlags;

  	cData.headerBtnDesc.version = kDataBrowserListViewLatestHeaderDesc;
	cData.headerBtnDesc.minimumWidth = kStatusColumnMinWidth;
	cData.headerBtnDesc.maximumWidth = kStatusColumnMaxWidth;
	cData.headerBtnDesc.titleOffset = 0;

	cData.headerBtnDesc.btnContentInfo.contentType = kControlContentTextOnly;

	cData.headerBtnDesc.btnContentInfo.u.iconRef = NULL;
		
	cData.headerBtnDesc.btnFontStyle = fontStyle;
	
	/*
	No title for the Status column.
	*/
	cData.headerBtnDesc.titleString = CFSTR("");
	
	/*
	AddDataBrowserListViewColumn() inserts the specified column before the 
	column corresponding to the position parameter, where the first column is
	number 0.
	*/
	err = ::AddDataBrowserListViewColumn(fView, &cData, kStatusColumn - 1);
	if (noErr != err)
		DebugStr("\pError adding Status column!");
		
	/*
	Set the column's default width.
	*/
	err = ::SetDataBrowserTableViewNamedColumnWidth(fView, kStatusProperty, 
													kStatusColumnDefaultWidth);
	if (noErr != err)
		DebugStr("\pError setting Status column width!");
	
	/*
	Next, the Name column. Name displays the name and icon of the printer
	and is sortable and moveable. Its header button displays the "Name" string.
	*/
	cData.propertyDesc.propertyID = kNameProperty;
	cData.propertyDesc.propertyType = kDataBrowserIconAndTextType;
	cData.propertyDesc.propertyFlags = kDataBrowserListViewMovableColumn
									   + kDataBrowserListViewSortableColumn
									   + kDataBrowserListViewSelectionColumn;

  	cData.headerBtnDesc.version = kDataBrowserListViewLatestHeaderDesc;
	cData.headerBtnDesc.minimumWidth = kNameColumnMinWidth;
	cData.headerBtnDesc.maximumWidth = kNameColumnMaxWidth;
	cData.headerBtnDesc.titleOffset = 0;

	cData.headerBtnDesc.btnContentInfo.contentType = kControlContentTextOnly;

	cData.headerBtnDesc.btnContentInfo.u.iconRef = NULL;
		
	cData.headerBtnDesc.btnFontStyle = fontStyle;
	
	/*
	Get the title for the Name column.
	*/
	cData.headerBtnDesc.titleString = CopyLocalizedStringFromPlugin(
			CFSTR("Product"),
			CFSTR("Product column title"),
			fBundleRef);
	
	err = ::AddDataBrowserListViewColumn(fView, &cData, kNameColumn - 1);
	if (noErr != err)
		DebugStr("\pError adding Name column!");
	
	err = ::SetDataBrowserTableViewNamedColumnWidth(fView, kNameProperty, 
													kNameColumnDefaultWidth);
	if (noErr != err)
		DebugStr("\pError setting Name column width!");
	
	/*
	Finally, the Kind column. Kind displays the type of printer and is
	sortable and moveable. Its header button displays the "Kind" string.
	*/
	cData.propertyDesc.propertyID = kKindProperty;
	cData.propertyDesc.propertyType = kDataBrowserTextType;
	cData.propertyDesc.propertyFlags = kDataBrowserListViewMovableColumn
									   + kDataBrowserListViewSortableColumn;

  	cData.headerBtnDesc.version = kDataBrowserListViewLatestHeaderDesc;
	cData.headerBtnDesc.minimumWidth = kKindColumnMinWidth;
	cData.headerBtnDesc.maximumWidth = kKindColumnMaxWidth;
	cData.headerBtnDesc.titleOffset = 0;

	cData.headerBtnDesc.btnContentInfo.contentType = kControlContentTextOnly;
	cData.headerBtnDesc.btnContentInfo.u.iconRef = NULL;
		
	cData.headerBtnDesc.btnFontStyle = fontStyle;
	
	/*
	Get the title for the Kind column.
	*/
	cData.headerBtnDesc.titleString = CopyLocalizedStringFromPlugin(
			CFSTR("Kind"),
			CFSTR("Kind column title"),
			fBundleRef);
	
	
	err = ::AddDataBrowserListViewColumn(fView, &cData, kKindColumn - 1);
	if (noErr != err)
		DebugStr("\pError adding Kind column!");
	
	err = ::SetDataBrowserTableViewNamedColumnWidth(fView, kKindProperty, 
													kKindColumnDefaultWidth);
	if (noErr != err)
		DebugStr("\pError setting Kind column width!");
	
	/*
	Now set the default sorting row and the row and header heights.
	*/
	::SetDataBrowserSortProperty(fView, kNameProperty);
	::SetDataBrowserTableViewRowHeight(fView, kRowHeight);
	
}	// CBrowserListView::InitBrowserHeader

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::InitBrowserViewPane

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Initialize the browser view pane based on information in
		browserInfo.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
CBrowserListView::InitBrowserViewPane(void)
{
	Rect			listRect;		// List view bounding rectangle.
	Rect			controlRect;
	
	/*
	Register with the Appearance Manager.
	*/
	::RegisterAppearanceClient();
	
	/*
	Calculate the bounding rectangles of the components of the browser
	view: the pop-up menu and the list view.
	*/
	::GetControlBounds(fUserPane, &controlRect);
	this->CalcControlRects(controlRect, &listRect);
	
	/*
	Get the lookup specification records so that we know what printers
	to browser for.
	*/
	this->InitLookupSpecs();
	
	/*
	Initialize and populate the list view.
	*/
	this->InitBrowserControl(listRect);
	this->SetUpBrowserView();
	
}	// CBrowserListView::InitBrowserViewPane

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::InitLookupSpecs

	Input Parameters:
		<none>
		
	Output Parameters:
		<none>
		
	Description:
		Get the printer lookup specification records from PrintCenter
		and store them in a local array.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
CBrowserListView::InitLookupSpecs(void)
{
	UInt32		curSpec;	// Lookup spec record index.
	OSStatus	err;		// Error condition.
	
	err = noErr;
	
	/*
	Allocate enough space for an array of the lookup spec records...
	*/
	fLookupSpecs = (CFDictionaryRef*) ::NewPtr(fNumSpecs * sizeof(CFDictionaryRef));
													 
	if (NULL == fLookupSpecs)
        DebugStr("\pCBrowserListView::InitLookupSpecs NULL == fLookupSpecs" );
	else
	{
		/*
		Get the lookup spec records from PrintCenter...
		*/
		for (curSpec = 0; curSpec < fNumSpecs; curSpec++)
		{
			err = fCallbacks->getLookupSpec(fRef, curSpec, 
											&(fLookupSpecs[curSpec]));
			if (noErr != err)
				::Debugger();
		}
	}
	
}	// CBrowserListView::InitLookupSpecs

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::InitPrinterList

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Initialize the browser view data.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
CBrowserListView::InitPrinterList( void )
{
    /*
    First, get rid of any existing printers list.
    */
    if (NULL != fPrinters)
        delete fPrinters;
    /*
    Create a new list of USB printers.
    */
    fPrinters = new CPrinterList(fNumSpecs, fLookupSpecs, fBundleRef );

    if (NULL != fPrinters)
    {	
        /*
        Find all the printers on the bus
        */
        fPrinters->Populate();
    }
            
		
}	// CBrowserListView::InitPrinterList

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::LoadBrowserView

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
CBrowserListView::LoadBrowserView(
	CItemList*	theList)
{
	CListIterator*		items;		// Iterator for theList.
	DataBrowserItemID	item;		// Browser control item ID.
	OSStatus			err;		// Error condition.
		
	/*
	First remove any existing items from the browser control by
	switching targets for the browser. We make theList the
	current target. Changing targets causes the browser to clear
	its data storage, leaving it ready to be repopulated.
	*/
	err = ::SetDataBrowserTarget(fView, (DataBrowserItemID) theList);
//	if (noErr != err)  DebugStr("\pCBrowserListView::LoadBrowserView SetDataBrowserTarget" );

        if (noErr == err && NULL != theList)
	{
		/*
		Walk the item list and add a new row to the browser view containing
		the name of the item, its icon, and its type.
		*/
		items = new CListIterator(theList->Head());

		while (NULL != items->Current())
		{
			/*
			Store a pointer to each item as its ID in the browser.
			*/
			item = (DataBrowserItemID) (items->Current());
			err = ::AddDataBrowserItems(fView, (DataBrowserItemID) theList,
										1, &item, kDataBrowserItemNoProperty);
			if (noErr != err)
            	break; //DebugStr("\pCBrowserListView::LoadBrowserView AddDataBrowserItems" );

			(void) items->Next();
		}

		delete items;

		/*
		Sort the list view items.
		*/
		err = ::SortDataBrowserContainer(fView, (DataBrowserItemID) theList,
										 false);
	}
	
}	// CBrowserListView::LoadBrowserView

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::MarkWorksetPrinters

	Input Parameters:
		<none>
		
	Output Parameters:
		<none>
		
	Description:
		Walks the list of printers (if it has been allocated), comparing it
		against the contents of the array of printer selection specifications
		stored in fWorksetList, and indicates which printers are already in 
		the PrintCenter Workset. If the user interface is active, the list 
		view is updated, which will result in the display of icons indicating 
		which printers are in the Workset.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
CBrowserListView::MarkWorksetPrinters(void)
{
	CListIterator*		iPrinter;		// Printer list iterator.
	CPrinter*			curPrinter;		// Pointer to current printer item.
	CFIndex				numWSPrinters;	// Number of printers in Workset array.
	CFIndex				wsIndex;		// Loop index for array traversal.
	CFDictionaryRef		specDict;		// Selection spec dictionary.
	CFDataRef			prAddr;			// Printer address from selection spec.
	Boolean				update;			// true if display should be updated.
	OSStatus			err;			// Error condition.
	
	update = false;
	err = noErr;
	
	if ((NULL != fPrinters) && (NULL != fWorksetList))
	{
		/*
		Clear the workset flags in the printers list first.
		*/
		iPrinter = new CListIterator(fPrinters->Head());
		
		while (NULL != iPrinter->Current())
		{
			((CPrinter*) (iPrinter->Current()))->SetInWorkset(false);
			(void) iPrinter->Next();
		}
		
		delete iPrinter;
		
		/*
		Now scan the printers list and mark those printers that are in 
		the workset.
		*/
		numWSPrinters = ::CFArrayGetCount(fWorksetList);
		
		iPrinter = new CListIterator(fPrinters->Head());

		while (NULL != iPrinter->Current())
		{
			curPrinter = (CPrinter*) (iPrinter->Current());
			
			/*
			The USB address of the printer uniquely identifies it for
			our purposes. So walk the array looking for an entry with 
			the same address as that of the current printer.
			*/
			for (wsIndex = 0; wsIndex < numWSPrinters; wsIndex++)
			{
				specDict = (CFDictionaryRef) ::CFArrayGetValueAtIndex(fWorksetList,
																	  wsIndex);
				if (NULL != specDict)
				{
                                        //CopyAddr can return NULL for an unsupported printer.
                                        CFDataRef currentPrinterAddr = curPrinter->CopyAddr();
					prAddr = (CFDataRef) ::CFDictionaryGetValue(specDict, kPMPrBrowserSelectAddrKey);
                                        
                                        if (NULL != prAddr && NULL != currentPrinterAddr
                                                           && ::CFEqual(prAddr, currentPrinterAddr)) {
                                            
                                                curPrinter->SetInWorkset(true);
                                                update = true;
					}
                                        
                                        if (currentPrinterAddr != NULL) {
                                            CFRelease(currentPrinterAddr);
                                            currentPrinterAddr = NULL;
                                        }
				}
			}
			
			(void) iPrinter->Next();
		}
		
		delete iPrinter;
	}
	
	if ((update) && (NULL != fView))
	{
		/*
		The UI is active, and at least one printer is already in the Workset,
		so force an update of the list.
		*/
		err = ::UpdateDataBrowserItems(fView, kDataBrowserNoItem, 0, NULL,
									   kDataBrowserItemNoProperty,
									   kDataBrowserItemNoProperty);
	}
	
	if (noErr != err)
		Debugger();

}	// CBrowserListView::MarkWorksetPrinters

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CBrowserListView::SetUpBrowserView

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Sets the browser view to display the appropriate data.		
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
CBrowserListView::SetUpBrowserView( void )
{
	OSStatus		err;		// Error condition.

        this->InitPrinterList();
        this->LoadBrowserView(fPrinters);

        /*
        Set the item selection flags for a list of printers. Printers
        can be drag-selected and discontiguous, and a selection can
        be added to using the Shift key.
        */
        err = ::SetDataBrowserSelectionFlags(fView,   kDataBrowserCmdTogglesSelection + kDataBrowserDragSelect);
	
	/*
	Force an update of the browser control.
	*/

	::DrawOneControl(fView);
	
	/*
	At this point, tell PrintCenter that nothing is selected.
	*/
	fCallbacks->selStatus(fRef, false, false);

}	// CBrowserListView::SetUpBrowserView

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Private Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	CompareCallback

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Handle the hit test from the Control Manager.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static pascal Boolean
CompareCallback(
	ControlRef			browser, 
	DataBrowserItemID		itemOne, 
	DataBrowserItemID		itemTwo, 
	DataBrowserPropertyID	sortProperty)
{
	CBrowserListView*	browserView;		// Pointer to CBrowserListView.

         browserView = (CBrowserListView*) ::GetControlReference(browser);
	
        return browserView == NULL? false: browserView->Compare(itemOne, itemTwo, sortProperty);
}

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	DataCallback

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Handle the hit test from the Control Manager.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static pascal OSStatus
DataCallback(
	ControlRef					browser, 
	DataBrowserItemID			item,
	DataBrowserPropertyID		property,
	DataBrowserItemDataRef		itemData,
	Boolean						setValue)
{
	CBrowserListView*	browserView;	// Pointer to CBrowserListView.
	OSStatus			err;

	browserView = (CBrowserListView*) ::GetControlReference(browser);
	
	if (NULL != browserView) 
		err = browserView->Data(item, property, itemData, setValue);

	return (err);
}

/******************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	ItemNotificationCallback

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:
		Handle the hit test from the Control Manager.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static pascal void
ItemNotificationCallback(
	ControlRef					browser,
	DataBrowserItemID			item, 
	DataBrowserItemNotification	message)
{	
	CBrowserListView*	browserView;	// Pointer to CBrowserListView.
	
	/*
	Get a pointer to the CBrowserListView from the control; it's stored
	in the control reference
	*/
	browserView = (CBrowserListView*) ::GetControlReference(browser);
	
	if (NULL != browserView) 
		browserView->ItemNotification(item, message);

}	// ItemNotificationCallback

/******************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Name:	USBNotifications

        Input Parameters:
                refcon
                iterator

        Output Parameters:
                none

        Description:
                Asynchronous notification from IOKit of USB devices' comings
                and goings. We don't iterate IORegistry (since we need DeviceID
                information which it doesn't contain). Instead we indirectly
                rebuild the browser's list of printers.

                One other thing to note is the Notification will not fire again
                until we iterate the IORegistry! So we iterate through here, just
                get the next notification. Do something with a side-effect so that
                the compiler doesn't optimize the loop out of existance!
                
                Also, since USBNotifications must be outside of any class (the
                kernel can't pass a this pointer!) we split the routine into this
                one which handles the kernel iterator, and a class member functions
                which handles the browser.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void
USBNotifications( void *refcon, io_iterator_t iterator )
{
    io_object_t			obj;

    //
    //	prime for the next notification by iterating through all devices
    //
    while( (obj = IOIteratorNext( iterator )) )
         IOObjectRelease( obj );
    //
    // update the list of printers
    //
    if ( refcon )
        ((CBrowserListView * )refcon)->UpdatePrinters();
}

void
CBrowserListView::UpdatePrinters( void )
{
    this->InitPrinterList();
    if ( NULL != fView )
    {
            /*
            The UI is active, so force a redraw of the list.
            */
           ::RemoveDataBrowserItems( fView, kDataBrowserNoItem, 0, NULL, kDataBrowserItemNoProperty);
           this->LoadBrowserView(fPrinters);
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Name:	InitHotplugNotifications

        Input Parameters:
                none

        Output Parameters:
                none

        Description:
                Hotplugging of USB devices is handled by telling IOKit to 
                pass us notifications for devices of type IOUSBDevice. (Notice
                we'll get superfluous update events for scanners and mice
                that are hot-plugged!)
                
                This routine simply sets up housekeeping variables required by
                IOKit, and then we add our notification proc to the run loop
                for addition and removal of USB devices.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void
CBrowserListView::InitHotplugNotifications( void )
{
    kern_return_t	kr;
    do
    {
        mach_port_t	masterPort;
    
        kr = IOMasterPort( bootstrap_port, &masterPort);
        if ( kIOReturnSuccess != kr ) break;

        fAddNotification = IONotificationPortCreate( masterPort );
        if ( NULL == fAddNotification ) break;

        kr = IOServiceAddMatchingNotification(fAddNotification,
                                                kIOMatchedNotification,
                                                IOServiceMatching( kMatchUSBDevicesString ),
                                                &::USBNotifications,
                                                (void *) this, /* ref con */
                                                &fAddedDevice );
        if ( kIOReturnSuccess != kr ) break;

        fRemoveNotification = IONotificationPortCreate( masterPort );
        if ( NULL == fRemoveNotification ) break;

        kr = IOServiceAddMatchingNotification(fRemoveNotification,
                                                kIOTerminatedNotification,
                                                IOServiceMatching( kMatchUSBDevicesString ),
                                                &::USBNotifications,
                                                (void *) this, /* ref con */
                                                &fRemovedDevice );
        if ( kIOReturnSuccess != kr ) break;
    } while ( 0 );
    if ( kIOReturnSuccess != kr )
    {
        //
        //	clean up, something in IOKit failed
        //
        // masterPort, done at close
        // port, done at close
    }
    else
    {
        //
        // prime the iterators to give us hot-plug events by enumerating the current state
        //
        io_object_t			obj;
    
        //
        // prime for the next notification by iterating through all devices
        // We don't call USBNotifications since that assumes UI
        //
        while( (obj = IOIteratorNext( fAddedDevice )) )
            IOObjectRelease( obj );
        while( (obj = IOIteratorNext( fRemovedDevice )) )
            IOObjectRelease( obj );
        //
        // tell IOKit to pass the hotplug notifications on to us
        //
        CFRunLoopAddSource(CFRunLoopGetCurrent(),
                            IONotificationPortGetRunLoopSource(fAddNotification),
                            kCFRunLoopDefaultMode);
        CFRunLoopAddSource(CFRunLoopGetCurrent(),
                            IONotificationPortGetRunLoopSource(fRemoveNotification),
                            kCFRunLoopDefaultMode);
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:	USBEventHandler()

	Input Parameters:
		<parameter>	:	<description>
		
	Output Parameters:
		<none>
		
	Description:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static OSStatus USBEventHandler( EventHandlerCallRef inHandlerRef, EventRef inEvent, void *userData )
{
	OSStatus result = eventNotHandledErr;
	
	UInt32 eventClass = GetEventClass (inEvent);
	UInt32 eventKind = GetEventKind (inEvent);
//	CBrowserListView* myContext = (CBrowserListView*)userData;
			
	if ((eventClass == kEventClassControl) && (eventKind == kEventControlHit))
	{
		ControlRef controlHit;
		OSStatus status = GetEventParameter( inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof( ControlRef ), NULL, &controlHit );
		if ( (status == noErr) && (controlHit != nil) )
		{
			/* do something */
		}
	}
	else if ((eventClass == kEventClassKeyboard) && (eventKind == kEventRawKeyUp))
	{
			/* do something */
	}

	return result;
}
// End of "CBrowserListView.cp"
