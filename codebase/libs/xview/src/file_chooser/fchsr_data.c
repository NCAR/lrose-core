#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fchsr_data.c 1.3 93/06/28";
#endif
#endif
 

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *	file for terms of the license.
 */


/*
 * File_Chooser package information
 */

#include <xview/xview.h>
#include <xview_private/fchsr_impl.h>

extern int 		file_chooser_init();
extern Xv_opaque	file_chooser_set();
extern Xv_opaque	file_chooser_get();
extern int 		file_chooser_destroy();

Xv_pkg file_chooser_pkg = {
    "File_chooser",
    ATTR_PKG_FILE_CHOOSER,
    sizeof(File_chooser_public),
    FRAME_CMD,
    file_chooser_init,
    file_chooser_set,
    file_chooser_get,
    file_chooser_destroy,
    NULL                     /* no find */
};
