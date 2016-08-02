#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)hist_data.c 1.3 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *	file for terms of the license.
 */


/*
 * Hist package information
 */

#include <xview/xview.h>
#include <xview/hist.h>

extern int 		hist_list_init();
extern Xv_opaque	hist_list_set();
extern Xv_opaque	hist_list_get();
extern Xv_opaque	hist_list_find();
extern int 		hist_list_destroy();

Xv_pkg history_list_pkg = {
    "History List",
    ATTR_PKG_HIST,
    sizeof(History_list_public),
    XV_GENERIC_OBJECT,
    hist_list_init,
    hist_list_set,
    hist_list_get,
    hist_list_destroy,
    hist_list_find,	/* yes, a find method! */
};



extern int 		hist_menu_init();
extern Xv_opaque	hist_menu_set();
extern Xv_opaque	hist_menu_get();
extern int 		hist_menu_destroy();

Xv_pkg history_menu_pkg = {
    "History Menu",
    ATTR_PKG_HIST,
    sizeof(History_menu_public),
    XV_GENERIC_OBJECT,
    hist_menu_init,
    hist_menu_set,
    hist_menu_get,
    hist_menu_destroy,
    NULL			/* no find */
};
