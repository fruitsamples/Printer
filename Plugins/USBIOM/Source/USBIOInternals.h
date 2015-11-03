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
	File:		USBIOInternals.h
	
	Description:
		Prototypes for exported IO module routines for USB
		
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
		
*******************************************************************************/
#ifndef __USBIOINTERNALS__
#define __USBIOINTERNALS__

#ifndef __COREFOUNDATION_COREFOUNDATION__
#if TARGET_OS_MAC && SLASH_INCLUDES_UNSUPPORTED
	#include <:CoreFoundation:CoreFoundation.h>
#else
	#include <CoreFoundation/CoreFoundation.h>
#endif
#endif

#include "USBUtil.h"

#ifndef __MACTYPES__
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <stdlib.h>
#include <unistd.h>

#include <CoreServices/CoreServices.h>
extern "C" {
#include <mach/mach_port.h> // for mach_port_allocate
};

#include <string.h>

#include "PMIOModule.h"
#include "PMTicket.h"

#ifdef __cplusplus
extern "C" {
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Prototypes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
EXTERN_API_C( OSStatus) IOMGetConnectionInfo(CFStringRef *connectionType, CFStringRef *pbmPath);

EXTERN_API_C( OSStatus) IOMInitialize(const CFDataRef printerAddress, IOMContext *ioModuleContextPtr);

EXTERN_API_C( OSStatus) IOMOpen(IOMContext ioModuleContext, PMTicketRef jobTicket, UInt32 *bufferSize);

EXTERN_API_C( OSStatus) IOMRead(IOMContext ioModuleContext, Ptr buffer, UInt32 *size, Boolean *eoj);

EXTERN_API_C( OSStatus) IOMWrite(IOMContext ioModuleContext, Ptr buffer, UInt32 *size, Boolean eoj);

EXTERN_API_C( OSStatus) IOMStatus(IOMContext ioModuleContext, CFStringRef *status);

EXTERN_API_C( OSStatus) IOMGetAttribute(IOMContext ioModuleContext, CFStringRef attribute, CFTypeRef *result);

EXTERN_API_C( OSStatus) IOMSetAttribute(IOMContext ioModuleContext, CFStringRef attribute, CFTypeRef data);

EXTERN_API_C( OSStatus) IOMClose(IOMContext ioModuleContext, Boolean abort);

EXTERN_API_C( OSStatus) IOMTerminate(IOMContext *ioModuleContextPtr);

#ifdef __cplusplus
}
#endif

#endif //__USBIOINTERNALS__