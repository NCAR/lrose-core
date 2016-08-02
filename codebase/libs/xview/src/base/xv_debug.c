#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_debug.c 20.18 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <stdio.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>
#include <xview/pkg.h>
#include <xview_private/xv_debug.h>

int             xv_abort_fatal_error = FALSE;

#ifdef _XV_DEBUG
/*
 * Following temporaries are for use by implementors when they need storage
 * for variables whose address is passed to X query routines.
 */
long unsigned   xv_debug_xid[20];
int             xv_debug_any[100];

int             xv_ask_for_debugger = FALSE;
int             xv_got_debugger = FALSE;
unsigned char   xv_debug_flags[((int) _svdebug_last_plus_one) / 8] = {
    0x80, 0x00
};

/*
 * Allows _svdebug_last_plus_one debug flags Initialized to have the 0th flag
 * turned on and the rest off. xv_debug.h's definition of _svdebug_always_on
 * depends on this initialization.
 */

/*
 * SV error handlers when running with debugging on. All handlers are defined
 * via a procedure pointer so that they can be replaced if necessary.
 */
static void     _xview_abort();
static void     _xview_dprintf();
static int      _xview_take_breakpoint();
void            (*xv_abort) () = _xview_abort;
void            (*xv_dprintf) () = _xview_dprintf;
int             (*xv_take_breakpoint) () = _xview_take_breakpoint;

/* Following are all Xv_private */
int             xv_dprintf_open_error_posted = FALSE;
char           *xv_dprintf_file_name = "xv_debug.log";
char           *xv_dprintf_standard_name = 0;

static void
_xview_abort()
{
    abort();
}

static void
#ifdef ANSI_FUNC_PROTO
_xview_dprintf(FILE *file, char *fmt, ...)
#else
_xview_dprintf(file, fmt, va_alist)
    FILE           *file;
    char           *fmt;
va_dcl
#endif
{
    int             i, we_opened;
    va_list         args;
    char           *file_name;

    we_opened = FALSE;
    if (file == NULL) {
	file = fopen(xv_dprintf_file_name, "a");
	we_opened = TRUE;
    }
    if (file == NULL) {
	if (!xv_dprintf_open_error_posted) {
	    xv_dprintf_open_error_posted = TRUE;
	    (void) fprintf(stderr,
			   XV_MSG("xv_dprintf: cannot open debugging file %s\n"),
			   xv_dprintf_file_name);
	}
	return;
    }
    VA_START(args, fmt);
    _doprnt(fmt, args, file);
    va_end(args);

    fflush(file);
    if (we_opened)
	fclose(file);
}

Xv_private int
xv_set_debug_flag(flag, bool)
    int             flag, bool;
{
    if (flag < 0 || flag >= 8 * sizeof(xv_debug_flags)) {
	return (TRUE);
    } else {
	if (bool) {
	    xv_debug_flags[flag / 8] |= 0x80 >> (flag % 8);
	} else {
	    xv_debug_flags[flag / 8] &= ~(0x80 >> (flag % 8));
	}
	return (FALSE);
    }
}

static int
_xview_take_breakpoint()
/*
 * Following routine body appears to be susceptible to optimization. This is
 * not true, because it is expected that various global variables will be
 * manipulated by the programmer via a debugger.
 */
{
    if (xv_ask_for_debugger) {
	FILE           *console = fopen("/dev/console", "a");
	if (console) {
	    (void) fprintf(console,
			   XV_MSG("xv_take_breakpoint: please debug process %d\n"),
			   getpid());
	    fflush(console);
	    fclose(console);
	    while (!xv_got_debugger) {
		sleep(3);
	    }
	    if (xv_ask_for_debugger) {
		xv_got_debugger = FALSE;
	    }
	}
    }
    return (TRUE);
}


#else


void _xview_dprintf()
{
}


#endif
