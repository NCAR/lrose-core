#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)scrn_data.c 1.19 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/scrn_impl.h>

Pkg_private int screen_init();
Pkg_private int screen_destroy();

Xv_pkg          xv_screen_pkg = {
    "Screen", ATTR_PKG_SCREEN,
    sizeof(Xv_screen_struct),
    &xv_generic_pkg,
    screen_init,
    screen_set_avlist,
    screen_get_attr,
    screen_destroy,
    NULL			/* no find proc */
};
