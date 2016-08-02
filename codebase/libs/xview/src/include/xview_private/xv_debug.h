/*      @(#)xv_debug.h 20.14 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef _xv_debug_h_already_included
#define _xv_debug_h_already_included

#ifndef FILE
#if !defined(SVR4) && !defined(__linux) && !defined(__APPLE__)
#undef NULL
#endif /* SVR4 */
#include <stdio.h>
#endif /* FILE */
#include <xview/pkg.h>		/* needed to get definition of Xv_private */
  /* 
   * Not strictly necessary to include <stdio.h> here, but eliminates
   * the need of clients who xv_dprintf to stdout or stderr to include
   * both xv_debug.h and stdio.h.
   */

Xv_private void	(*xv_abort)();
Xv_private void	(*xv_dprintf)();
Xv_private int	(*xv_take_breakpoint)();

#ifdef _XV_DEBUG

#define DEBUG_ONLY(x)		x

Xv_private int		xv_ask_for_debugger, xv_got_debugger;
Xv_private unsigned char	xv_debug_flags[];
Xv_private int		xv_set_debug_flag();
#define DEBUG_FLAG_ON(flag)	\
	(xv_debug_flags[((unsigned)(flag))/8] & \
	 (0x80>>(((unsigned)(flag))%8)) )
    /*
     * The 0th flag will always be on.  This depends on initialization
     * of xv_debug_flags performed in xv_debug.c.
     */
typedef enum {
    _svdebug_always_on					= 0,
    _svdebug_obsolete_code				= 1,
    _svdebug_synchronous_server				= 2,

    /* Enablers for code that when turned on crashes X alpha2 server */
    _svdebug_fill_opaque_stippled			= 100,
    _svdebug_null_src_is_white				= 101,

    /* Temporaries not expected to show in code that is checked in */
    _svdebug_alok					= 475,
    _svdebug_carl					= 500,
    _svdebug_mark					= 525,
    _svdebug_pete					= 550,

    _svdebug_last_plus_one				= 1024
} Xv_debug_flag;

#define AN_ERROR(expr, flag)	((expr) && \
				 DEBUG_FLAG_ON(flag) && \
				 xv_take_breakpoint())
#define ASSERT(expr, flag)	if ((expr) && DEBUG_FLAG_ON(flag)) \
				xv_abort()
#define ASSUME(expr, flag)	if ((expr) && DEBUG_FLAG_ON(flag)) \
				xv_take_breakpoint()
#define FATAL_ERROR(exit_code)	xv_abort()

/*
 * Examples of expected usage of these macros follows:
 *	Calling checking procedure(s) when running debugged
 * DEBUG_ONLY(check_xxx(...); check_yyy(...))
 *
 *   xv_debug_flags exists to allow fine control over the bear-traps that
 * the following macros implement.
 *	Checking for an error and taking breakpoint when running debugged
 * if AN_ERROR(check_xxx(...), xxx_debug_flag) { recovery code }
 *	Checking that a vital invariant is true
 * ASSERT(invariant, invariant_debug_flag)
 *	Checking that a non-vital invariant is true.  It is expected that
 *	the programmer has set a breakpoint on _xview_take_breakpoint();
 * ASSUME(invariant, invariant_debug_flag)
 *
 *   xv_ask_for_debugger is initially FALSE, but when set to TRUE causes
 * xv_take_breakpoint to write to the console and hang waiting to be
 * attached to by a debugger.  The process can be continued by setting
 * xv_got_debugger to TRUE from the debugger.
 */

#else /* _XV_DEBUG */

#define DEBUG_ONLY(x)
#define AN_ERROR(expr, flag)	(expr)
#define ASSERT(expr, flag)
#define ASSUME(expr, flag)
#define FATAL_ERROR(exit_code)	if (xv_abort_fatal_error) abort() \
				else exit(exit_code)

#endif /* _XV_DEBUG */

#endif /* _xv_debug_h_already_included */
