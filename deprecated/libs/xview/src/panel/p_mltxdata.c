#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_ntx_data.c 20.6 90/06/21";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/panel_impl.h>

Pkg_private int         panel_mltxt_init();
Pkg_private Xv_opaque   panel_mltxt_set_avlist();
Pkg_private Xv_opaque   panel_mltxt_get_attr();
Pkg_private int         panel_mltxt_destroy();

Xv_pkg xv_panel_multiline_text_pkg = {
    "Multiline Text Item", ATTR_PKG_PANEL,
    sizeof(Xv_panel_multiline_text),
    &xv_panel_item_pkg,
    panel_mltxt_init,
    panel_mltxt_set_avlist,
    panel_mltxt_get_attr,
    panel_mltxt_destroy,
    NULL                        /* no find proc */
};
