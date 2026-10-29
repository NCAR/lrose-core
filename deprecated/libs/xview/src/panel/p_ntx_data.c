#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_ntx_data.c 20.8 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/panel_impl.h>

Pkg_private int         panel_num_text_init();
Pkg_private Xv_opaque   panel_num_text_set_avlist();
Pkg_private Xv_opaque   panel_num_text_get_attr();
Pkg_private int         panel_num_text_destroy();

Xv_pkg xv_panel_num_text_pkg = {
    "Numeric Text Item", ATTR_PKG_PANEL,
    sizeof(Xv_panel_num_text),
    &xv_panel_item_pkg,
    panel_num_text_init,
    panel_num_text_set_avlist,
    panel_num_text_get_attr,
    panel_num_text_destroy,
    NULL                        /* no find proc */
};
