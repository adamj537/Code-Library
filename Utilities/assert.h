/******************************************************************************
 *
 *	Filename:		Assert.h
 *
 *	Description:	If something fails to assert (which basically means it's
 *					equal to zero), we wait in an infinite loop.  If we're in a
 *					debugger, then we can break and see where the code failed.
 *					If we're not debugging, and ARE using a watchdog, then
 *					eventually the watchdog will reset the processor.  If we're
 *					not debugging, and are NOT using a watchdog, we can compile
 *					the assert out.
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#ifndef ASSERT_H
#define ASSERT_H

// Only include the ASSERT macro when we want to.
#ifdef INCLUDE_ASSERT

// This deserves some explanation:
//
// The do-while part just allows us to wrap the macro in something that needs a
// semicolon at the end.  This way when we use the ASSERT macro we put a
// semicolon after it just like a function, which looks nicer.
//
// ASSERT basically just checks if an expression or variable equals zero.  If
// it equals zero, then we say the ASSERT failed, and we do something to alert
// the user that something terrible has happened.  In an embedded system, the
// best we can do is loop forever.  So that's what the while(1) part does.
#define ASSERT(x) do {(if(!x)) while(1);} while(0)

// When we don't want the macro to do anything, we define it as nothing.
#else
#define ASSERT(x)
#endif

#endif