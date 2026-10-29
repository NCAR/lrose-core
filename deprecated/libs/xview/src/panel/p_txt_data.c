#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_txt_data.c 1.14 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */
#include <xview_private/panel_impl.h>

Pkg_private int text_init();
Pkg_private Xv_opaque text_set_avlist();
Pkg_private Xv_opaque text_get_attr();
Pkg_private int text_destroy();

Xv_pkg          xv_panel_text_pkg = {
    "Text Item", ATTR_PKG_PANEL,
    sizeof(Xv_panel_text),
    &xv_panel_item_pkg,
    text_init,
    text_set_avlist,
    text_get_attr,
    text_destroy,
    NULL			/* no find proc */
};
