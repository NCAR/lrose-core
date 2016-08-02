#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_drp_data.c 1.3 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */
#include <xview_private/panel_impl.h>

Pkg_private int panel_drop_init();
Pkg_private Xv_opaque panel_drop_set_avlist();
Pkg_private Xv_opaque panel_drop_get_attr();
Pkg_private int panel_drop_destroy();

Xv_pkg          xv_panel_drop_pkg = {
    "Drop Target Item",
    ATTR_PKG_PANEL,
    sizeof(Xv_panel_drop),
    &xv_panel_item_pkg,
    panel_drop_init,
    panel_drop_set_avlist,
    panel_drop_get_attr,
    panel_drop_destroy,
    NULL			/* no find proc */
};
