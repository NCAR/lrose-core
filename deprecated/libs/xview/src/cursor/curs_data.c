#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)curs_data.c 1.15 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/curs_impl.h>

Xv_pkg          xv_cursor_pkg = {
    "Cursor",			/* seal -> package name */
    ATTR_PKG_CURSOR,		/* cursor attr */
    sizeof(Xv_cursor_struct),	/* size of the cursor data struct */
    &xv_generic_pkg,		/* pointer to parent */
    cursor_create_internal,	/* init routine for cursor */
    cursor_set_internal,
    cursor_get_internal,
    cursor_destroy_internal,
    NULL			/* no find proc */
};
