#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_lst_data.c 1.14 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */
#include <xview_private/panel_impl.h>

Pkg_private int panel_list_init();
Pkg_private int panel_list_destroy();
Pkg_private Xv_opaque panel_list_set_avlist();
Pkg_private Xv_opaque panel_list_get_attr();

Xv_pkg          xv_panel_list_pkg = {
    "Panel_list Item", ATTR_PKG_PANEL,
    sizeof(Xv_panel_list),
    &xv_panel_item_pkg,
    panel_list_init,
    panel_list_set_avlist,
    panel_list_get_attr,
    panel_list_destroy,
    NULL			/* no find proc */
};
