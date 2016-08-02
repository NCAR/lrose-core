#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)windowdata.c 1.16 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/windowimpl.h>

Pkg_private int window_init();
Pkg_private int window_destroy_win_struct();
Xv_pkg          xv_window_pkg = {
    "Window", ATTR_PKG_WIN,
    sizeof(Xv_window_struct),
    &xv_drawable_pkg,
    window_init,
    window_set_avlist,
    window_get_attr,
    window_destroy_win_struct,
    NULL
};
