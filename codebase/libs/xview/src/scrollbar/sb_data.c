#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sb_data.c 1.17 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Module:	sb_data.c
 * 
 * 
 * Include files:
 */

#include <xview_private/sb_impl.h>

/*
 * Global Defines:
 */
Xv_pkg          xv_scrollbar_pkg = {
    "Scrollbar",			/* seal -> package name */
    (Attr_pkg) ATTR_PKG_SCROLLBAR,	/* scrollbar attr */
    sizeof(Xv_scrollbar),		/* size of the scrollbar data struct */
    &xv_window_pkg,			/* pointer to parent */
    scrollbar_create_internal,		/* init routine for scrollbar */
    scrollbar_set_internal,		/* set routine */
    scrollbar_get_internal,		/* get routine */
    scrollbar_destroy_internal,		/* destroy routine */
    NULL				/* No find proc */
};
