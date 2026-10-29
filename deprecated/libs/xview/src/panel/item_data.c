#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)item_data.c 1.15 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/panel_impl.h>
Pkg_private int item_init();
Pkg_private int item_destroy();


Xv_pkg          xv_panel_item_pkg = {
    "Item", ATTR_PKG_PANEL,
    sizeof(Xv_item),
    &xv_generic_pkg,
    item_init,
    item_set_avlist,
    item_get_attr,
    item_destroy,
    NULL			/* No find proc */
};
