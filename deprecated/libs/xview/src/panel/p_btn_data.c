#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_btn_data.c 1.17 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */
#include <xview_private/panel_impl.h>

Pkg_private int panel_button_init();
Pkg_private int panel_button_destroy();

Xv_pkg          xv_panel_button_pkg = {
    "Button Item", ATTR_PKG_PANEL,
    sizeof(Xv_panel_button),
    &xv_panel_item_pkg,
    panel_button_init,
    NULL,
    NULL,
    panel_button_destroy,
    NULL			/* no find proc */
};
