#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)icon_data.c 1.17 93/06/28";
#endif
#endif
/*****************************************************************************/
/*
 * icon_object.c
 *
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */
/*****************************************************************************/

#include <xview_private/icon_impl.h>

Xv_pkg          xv_icon_pkg = {
    "Icon",			/* seal -> package name */
    (Attr_pkg) ATTR_PKG_ICON,	/* icon attr */
    sizeof(Xv_icon),		/* size of the icon data struct */
    &xv_window_pkg,		/* pointer to parent */
    icon_init,			/* init routine for icon */
    icon_set_internal,		/* set routine */
    icon_get_internal,		/* get routine */
    icon_destroy_internal,	/* destroy routine */
    NULL
};
