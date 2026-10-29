#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fm_data.c 1.17 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */
/*
 * Frame packages are in this module. Changes made for shared libraries.
 */

#include <xview_private/fm_impl.h>
#include <xview_private/frame_help.h>
#include <xview_private/frame_cmd.h>
#include <xview_private/frame_base.h>

#ifndef NULL
#define NULL 0
#endif

Xv_pkg          xv_frame_class_pkg = {
    "Frame", (Attr_pkg) ATTR_PKG_FRAME,
    sizeof(Xv_frame_class),
    &xv_window_pkg,
    frame_init,
    frame_set_avlist,
    frame_get_attr,
    frame_destroy,
    NULL			/* no find proc */
};



Xv_pkg          xv_frame_base_pkg = {
    "Frame_base", (Attr_pkg) ATTR_PKG_FRAME,
    sizeof(Xv_frame_base),
    &xv_frame_class_pkg,
    frame_base_init,
    frame_base_set_avlist,
    frame_base_get_attr,
    frame_base_destroy,
    NULL			/* no find proc */
};


Xv_pkg          xv_frame_cmd_pkg = {
    "Frame_cmd", (Attr_pkg) ATTR_PKG_FRAME,
    sizeof(Xv_frame_cmd),
    &xv_frame_class_pkg,
    frame_cmd_init,
    frame_cmd_set_avlist,
    frame_cmd_get_attr,
    frame_cmd_destroy,
    NULL			/* no find proc */
};

#define	xv_frame_props_pkg	xv_frame_cmd_pkg;


Xv_pkg          xv_frame_help_pkg = {
    "Frame_help", (Attr_pkg) ATTR_PKG_FRAME,
    sizeof(Xv_frame_help),
    &xv_frame_class_pkg,
    frame_help_init,
    frame_help_set_avlist,
    frame_help_get_attr,
    frame_help_destroy,
    NULL			/* no find proc */
};
