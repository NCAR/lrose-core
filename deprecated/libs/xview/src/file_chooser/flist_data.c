#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)flist_data.c 1.3 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *	file for terms of the license.
 */


/*
 * File_List package information
 */

#include <xview/xview.h>
#include <xview_private/flist_impl.h>

extern int 		file_list_init();
extern Xv_opaque	file_list_set();
extern Xv_opaque	file_list_get();
extern int 		file_list_destroy();

Xv_pkg file_list_pkg = {
    "File_list",
    ATTR_PKG_FILE_LIST,
    sizeof(File_list_public),
    PANEL_LIST,
    file_list_init,
    file_list_set,
    file_list_get,
    file_list_destroy,
    NULL                     /* no find */
};
