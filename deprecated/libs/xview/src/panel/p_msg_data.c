#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_msg_data.c 1.15 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */
#include <xview_private/panel_impl.h>

Pkg_private int panel_message_init();

Xv_pkg          xv_panel_message_pkg = {
    "Message Item", ATTR_PKG_PANEL,
    sizeof(Xv_panel_message),
    &xv_panel_item_pkg,
    panel_message_init,
    NULL,
    NULL,
    NULL,
    NULL			/* no find proc */
};
