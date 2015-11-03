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

/*
	File:		Exception.h

	Contains:	Includes for exception handler

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

	Change History:

	To Do:

*/

#ifndef __Exception__
#define __Exception__

#include <ApplicationServices/ApplicationServices.h>

#ifndef __Thread__

#include <setjmp.h>			// setjmp / longjmp

/**************************************************************************************
	Typedefs
 **************************************************************************************/
typedef	long FAILCode;			// failure code as used by exception handler

struct	ExceptionFrame
	{
	struct ExceptionFrame		*next;
	jmp_buf						jmpbuf;	// different on power mac
	};
# if !defined(__cplusplus)
typedef	struct ExceptionFrame ExceptionFrame;
# endif


/**************************************************************************************
	Routines called by macros defined below
 **************************************************************************************/
void		_installExceptionHandler(ExceptionFrame *frame);
void		_removeExceptionHandler(ExceptionFrame *frame);
void		_raiseException(FAILCode code);
void		_raiseContextException(OSStatus error, short context);
void		_failContextOSErr(OSStatus error, short context);
void		_failErrCode(FAILCode code);
void		_failnull(void *p, short context);


/**************************************************************************************
	Macros
 **************************************************************************************/
// Start an exception frame.  Must be terminated by either a NOHANDLER or 
// EXCEPTION and ENDEXCEPTION
# define	TRY	{ExceptionFrame	__fr;\
				FAILCode		EXCEPTIONCODE;\
				_installExceptionHandler(&__fr);\
				{\
				if ( (EXCEPTIONCODE = setjmp(__fr.jmpbuf)) == 0){

// Specify the start of code which will handle failures.  Must be terminated by
// an ENDEXCEPTION
# define	EXCEPTION	_removeExceptionHandler(&__fr); \
							}else{

// Signifies the end of an exception frame
# define	ENDEXCEPTION	}}}

// Variation on TRY which will automatically set EXCEPTIONCODE to the
// exception error 
# define	TRYR(EXCEPTIONCODE)\
					{ExceptionFrame	__fr;\
						_installExceptionHandler(&__fr);\
						{\
						if ( (EXCEPTIONCODE = setjmp(__fr.jmpbuf)) == 0){

// Use this to terminate a TRY when you don't want to do anything when an
// exception happens
# define	NOHANDLER			_removeExceptionHandler(&__fr); \
							}}}

// Retryable exception frame.  The RETRY macro can be called within this kind of
// exception frame.  Must be terminated by either a NOHANDLER or EXCEPTION and
// ENDEXCEPTION
# define	RTRY {ExceptionFrame	__fr;\
				FAILCode		EXCEPTIONCODE;\
				retryLabel:\
				_installExceptionHandler(&__fr);\
				{\
				if ( (EXCEPTIONCODE = setjmp(__fr.jmpbuf)) == 0){

// Retryable variation on TRY which will automatically set EXCEPTIONCODE to the
// exception error.  The RETRY macro can be called within this kind of
// exception frame.
# define	RTRYR(EXCEPTIONCODE)\
					{ExceptionFrame	__fr;\
						retryLabel:\
						_installExceptionHandler(&__fr);\
						{\
						if ( (EXCEPTIONCODE = setjmp(__fr.jmpbuf)) == 0){


// If a failure happens, you can call RETRY within the EXCEPTION / ENDEXCEPTION.
// This will cause the TRY code to be exectuted again
# define	RETRY()			goto retryLabel

// Call this when you want to fail with the original exception code from within an
// EXCEPTION / ENDEXCEPTION. 
# define	PASSEXCEPTION()	_raiseException(EXCEPTIONCODE)


/**********************************************************************************
	Inlines
 **********************************************************************************/
// Fail with the given error and context
inline void FAIL(OSStatus error, short context = 0)
	{ _raiseContextException(error, context); };

// If the passed long error code is non-zero, fail with it
inline void FAILOSErr(FAILCode code)
	{ if (code) _failErrCode(code); };

// If the pointer is nil, fail with a memFullErr and the specified context
inline void FAILNULL(void *p, short context = 0)
	{ _failnull(p, context); };

// If a mem error occured, fail with it and the specified context
inline void FAILMemErr(short context = 0)
	{ _failContextOSErr(MemError(), context);};

// If a QuickDraw error occurred, fail with it and context. NOTE: Shouldn't call
// unless you're sure we have color quickdraw.
inline void FAILQDErr(short context = 0)
	{ _failContextOSErr(QDError(), context);};

// If a resource error occured, fail with it and the specified context
inline void FAILResErr(short context = 0)
	{ _failContextOSErr(ResError(), context);};

// Extract the context (in the high word) from the specified FAILCode
inline short GetContext(FAILCode failCode)
	{ return ((failCode >> 16) & 0xFFFF); };

// Extract the error (in the low word) from the specified FAILCode
inline OSStatus GetError(FAILCode failCode)
	{ return (short)(failCode & 0xFFFF); };

// If the condition it true, fail with the specified failure code. 
inline void FAILIf(Boolean condition, OSStatus error, short context = 0)
	{ if (condition) FAIL( error, context); };

// Specialized FAILIf which checks if the current exception frame is the outermost
// and only fails if it isn't.  This is to be used when you can't fail because
// there may not be an outer exception frame to catch the failure
# define	FAILIfNotTopHandler(error, context)						\
				if (__fr.next != nil) FAIL(error, context)
				

/**********************************************************************************
	C++ routines
 **********************************************************************************/

// If the pointer is nil, fail with ResErr and the specified context
// Has workaround for Resource Manager behavior which does not set
// ResErr when resource type is unavailable in any open resource file
// Ref IM I-119
void	FAILNULLResource(void *p, short context = 0);

#endif // __Thread__


#endif // __Exception__
