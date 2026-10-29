#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fs_data.c 1.16 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/fs_impl.h>

Xv_pkg          xv_fullscreen_pkg = {
    "Fullscreen", ATTR_PKG_FULLSCREEN,
    sizeof(Xv_fullscreen),
    &xv_generic_pkg,
    fullscreen_init_internal,
    fullscreen_set_avlist,
    fullscreen_get_attr,
    fullscreen_destroy_internal,
    NULL			/* no find proc */
};

/*
 * When not zero will not actually acquire exclusive io lock so that the
 * debugger doesn't get hung.
 */
int             fullscreendebug;

/*
 * When not zero will not grab device server/pointer/keyboard
 */
int             fullscreendebugserver;
int             fullscreendebugptr;
int             fullscreendebugkbd;

