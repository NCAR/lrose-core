#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)path_data.c 1.4 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *	file for terms of the license.
 */


/*
 * Path package information
 */

#include <xview/xview.h>
#include <xview/panel.h>
#include <xview_private/path_impl.h>

extern int 		path_init_avlist();
extern Xv_opaque	path_set_avlist();
extern Xv_opaque	path_get_attr();
extern int 		path_destroy_private();

Xv_pkg path_pkg = {
    "Path_name",
    ATTR_PKG_PATH,
    sizeof(Path_public),
    PANEL_TEXT,
    path_init_avlist,
    path_set_avlist,
    path_get_attr,
    path_destroy_private,
    NULL                     /* no find */
};
