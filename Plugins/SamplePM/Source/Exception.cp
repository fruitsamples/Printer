/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under AppleÕs copyrights in this original Apple software (the
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
	File:		Exception.cp

	Contains:	Implement the Sneaker exception handler

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

	Change History:

	To Do:
*/

#include "Exception.h"


/**************************************************************************************
	Globals
 **************************************************************************************/
static	ExceptionFrame	*gExcHandler	= nil;	// pointer to exception frame base


/**************************************************************************************
  _installExceptionHandler
	Abstract:
		Take the passed exception frame pointer and install it into our global
		exception handler queue.  If a FAIL happens and no other frame has been
		installed, the information stored in this frame will be used to handle the
		exception.
	Input:
		frame			-	exception frame to use to install onto our queue
	Output:
		gExcHandler		-	now points to frame making frame the top most exception
							frame.
	Return:
		Nothing.
 **************************************************************************************/
void	_installExceptionHandler(ExceptionFrame *frame)
	{
	// chain the handler
	frame->next			= gExcHandler;
	gExcHandler			= frame;
	}


/**************************************************************************************
  _removeExceptionHandler
	Abstract:
		Remove the passed exception frame from the exception queue.  If we're building
		a DEBUG version and if the passed frame is nil or if the passed frame is not
		the top most in the queue, break into the debugger.
	Input:
		frame			-	exception frame to remove from the queue
	Output:
		gExcHandler		-	now points to the frame pointed to by frame->next
							instead of the frame
	Return:
		Nothing.
 **************************************************************************************/
void	_removeExceptionHandler(ExceptionFrame *frame)
	{
#if	DEBUG
	// check if the current exception handler is null
	if (!frame)
		DebugStr("\p >>>>> Invalid exception handler chain. Null frame Ptr.");
	
	// check if the current exception handler is the right one
	if (frame != gExcHandler)
		DebugStr("\p >>>>> Invalid exception handler chain. Top <> frame Ptr");
#endif	// DEBUG

	// Set current exception handler to the next one in the frame
	gExcHandler			= frame->next;
	}


/**************************************************************************************
  _raiseException
	Abstract:
		Using the passed error code, which incorporates both error and context
		information, FAIL by calling longjmp using the saved setjmp information in
		the top most exception frame.
	Input:
		code			-	error which cause failure and context under which the
							failure happened
	Output:
		gExcHandler		-	now points to the frame pointed to by frame->next
							instead of the frame
	Return:
		Nothing.
 **************************************************************************************/
void	_raiseException(FAILCode code)
	{
	ExceptionFrame		*frame;
	
	// Get the current exception frame, then remove it from the queue
	frame				= gExcHandler;
	gExcHandler			= gExcHandler->next;

	// Do a long jump using the register information which was saved in the
	// setjmp which occured when the exception frame was installed
	longjmp(frame->jmpbuf, (int)code);
	}


/**************************************************************************************
  _raiseContextException
	Abstract:
		Using the passed error and context, FAIL by calling longjmp using the 
		saved setjmp information in the top most exception frame.
	Input:
		error			-	error to use to raise the exception
		context			-	context under which the failure happened
	Output:
		gExcHandler		-	now points to the frame pointed to by frame->next
							instead of the frame
	Return:
		Nothing.
 **************************************************************************************/
void	_raiseContextException(OSStatus error, short context)
	{
	FAILCode	code;
	short		*pfcCode	= (short *)&code;

	// Construct the FAILCode to use with the context in the hi order word and the
	// error in the lo order word
	*pfcCode++				= 	context;
	*pfcCode				= 	error;
	
	// FAIL with the constructed code
	_raiseException(code);
	}


/**************************************************************************************
  _failContextOSErr
	Abstract:
		Using the passed error and context, FAIL if the error is not equal to noErr.
	Input:
		error			-	error to use to raise the exception if not noErr
		context			-	context under which the failure happened
	Output:
		Nothing.
	Return:
		Nothing.
	FAILABLE
 **************************************************************************************/
void	_failContextOSErr(OSStatus error, short context)
	{
	if (error != noErr)
		_raiseContextException(error, context);
	}


/**************************************************************************************
  _failErrCode
	Abstract:
		If the passed error code's error is not noErr, fail
	Input:
		error			-	error to use to raise the exception if not noErr
		context			-	context under which the failure happened
	Output:
		Nothing.
	Return:
		Nothing.
	FAILABLE
 **************************************************************************************/
void	_failErrCode(FAILCode code)
	{
	OSStatus		error	= GetError(code);
	
	// Extract the error from the passed FAILCode and FAIL if not noErr
	if ( error != noErr)
		_raiseContextException(error, GetContext(code));
	}


/**************************************************************************************
  _failnull
	Abstract:
		If the passed pointer is nil, FAIL with a memFullErr as the error and the
		passed context as the context under which the memory failed to allocate
	Input:
		p				-	pointer to check for null
		context			-	context under which memory failed to allocate happened
	Output:
		Nothing.
	Return:
		Nothing.
	FAILABLE
 **************************************************************************************/
void	_failnull(void *p, short context)
	{
	// If the passed pointer is nil, FAIL
	if ( p == nil )
		_raiseContextException(memFullErr, context);
	}


/**************************************************************************************
  FAILNULLResource
	Abstract:
		If the pointer is nil, fail with ResErr and the specified context
		Has workaround for Resource Manager behavior which does not set
		ResErr when resource type is unavailable in any open resource file
		Ref IM I-119
	Input:
		p				-	handle to resource.
		context			-	context to use when failing
	Output:
		Nothing.
	Return:
		Nothing.
	FAILABLE
 **************************************************************************************/
void	FAILNULLResource(void *p, short context)
	{
	if (p == NULL)
		{
		OSStatus	err;
	
		if ((err = ResError()) == noErr)
			err = resNotFound;

		_failContextOSErr(err, context);
		}
	}

