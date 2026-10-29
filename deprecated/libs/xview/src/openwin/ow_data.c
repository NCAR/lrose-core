#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)ow_data.c 1.16 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Global Defines:
 */

#include <xview_private/ow_impl.h>

Xv_pkg          xv_openwin_pkg = {
    "Open Window",		/* seal -> package name */
    (Attr_pkg) ATTR_PKG_OPENWIN,/* openwin attr */
    sizeof(Xv_openwin),		/* size of the openwin data struct */
    &xv_window_pkg,		/* pointer to parent */
    openwin_init,		/* init routine for openwin */
    openwin_set,		/* set routine */
    openwin_get,		/* get routine */
    openwin_destroy,		/* destroy routine */
    NULL			/* No find proc */
};
