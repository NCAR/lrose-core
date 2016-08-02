#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)cnvs_data.c 1.17 90/06/21o";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/cnvs_impl.h>

Xv_pkg          xv_canvas_pkg = {
    "Canvas",
    (Attr_pkg) ATTR_PKG_CANVAS,
    sizeof(Xv_canvas),
    &xv_openwin_pkg,
    canvas_init,
    canvas_set_avlist,
    canvas_get_attr,
    canvas_destroy,
    NULL
};

Xv_pkg          xv_canvas_view_pkg = {
    "Canvas view",
    (Attr_pkg) ATTR_PKG_CANVAS_VIEW,
    sizeof(Xv_canvas_view),
    &xv_window_pkg,
    canvas_view_init,
    NULL,
    canvas_view_get,
    canvas_view_destroy,
    NULL
};

Xv_pkg          xv_canvas_pw_pkg = {
    "Canvas paint window",
    (Attr_pkg) ATTR_PKG_CANVAS_PAINT_WINDOW,
    sizeof(Xv_canvas_pw),
    &xv_window_pkg,
    NULL,
    canvas_paint_set,
    canvas_paint_get,
    NULL,
    NULL
};
