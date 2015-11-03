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

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Name:			TSneakerObject

	Description:
		"Ultimate" base class for all Sneaker objects. This class allows us
		to specify universal methods or fields for Sneaker classes derived
		from it. In particular, operator new() and operator delete()
		are overridden for all classes to force clearing of memory (and also
		because of a bug in the new() and delete() in early versions of the
		standard C++ libraries for PowerPC).
		
		All Sneaker base classes should be declared public TSneakerObject.
		For debugging purposes, global new() and delete() are also overridden,
		and generate a _DebugStr to warn the programmer that he forgot to
		derive his object from TSneakerObject.
		
		Also for debugging purposes, this base class has a 4-byte field
		which contains a "cookie" which can be used to identify objects
		instantiated by Sneaker code.

 	Version:	Technology:	Tioga SDK
 				Release:	1.0
 
	Copyright (C) 2000-2001, Apple Computer, Inc., all rights reserved.

	Change History:
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef	__ExtraTypes__
#define	__ExtraTypes__

#include	<stddef.h>
#include <stdlib.h>


#if defined __cplusplus

/*€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€*/
class TSneakerObject
{
	public:		// None.
	protected:	// None.
	private:
#if	DEBUG
		ulong		fSneakerCookie;	// Identifies Sneaker objects.
#endif
		
	public:
#if	DEBUG
		inline			TSneakerObject (void) { fSneakerCookie = 'snkr'; };
#endif
		inline void*	operator new		(size_t);
		inline void		operator delete	(void *);
		
	protected:	// None.
	private:		// None.
		
}; // TSneakerObject


/*€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€*/
inline void *TSneakerObject::operator new(size_t s)
{
	return malloc((long) s);
	
}	// TSneakerObject::operator new


/*€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€*/
inline void TSneakerObject::operator delete(void *p)
{
	if (p != (void *) NULL)
		free( p);
	
}	// TSneakerObject::operator delete

#endif	// __cplusplus

#endif	// __ExtraTypes__
