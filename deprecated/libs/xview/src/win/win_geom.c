#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)win_geom.c 20.35 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Win_geometry.c: Implement the geometry operation functions of the
 * win_struct.h interface.
 */

#include <xview/rect.h>
#include <xview/win_struct.h>
#include <xview/window.h>
#include <xview_private/draw_impl.h>
#include <xview_private/windowimpl.h>

/*
 * Geometry operations.
 */
Xv_private void
win_x_getrect(display, xid, rect)	/* Low-level access for toolplaces */
    Display        *display;
    XID             xid;
    register Rect  *rect;
{
    XID             root;
    int             x, y;
    unsigned int    w, h, border_w, depth;

    XGetGeometry(display, xid, &root, &x, &y, &w, &h, &border_w, &depth);
    rect->r_left = x;
    rect->r_top = y;
    rect->r_width = w;
    rect->r_height = h;
}

Xv_private void
win_getrect(window, rect)
    Xv_object       window;
    Rect           *rect;
{
    window_get_cache_rect(window, rect);
}

Xv_private void
win_setrect(window, rect)
    Xv_object       window;
    Rect           *rect;
{
    int		    old_rect_info,
		    rect_info = ( WIN_X_SET | WIN_Y_SET |
				  WIN_WIDTH_SET | WIN_HEIGHT_SET);

    old_rect_info = (int)xv_get(window, WIN_RECT_INFO);
    xv_set(window, WIN_RECT_INFO, rect_info, NULL);
    window_set_cache_rect(window, rect);
    xv_set(window, WIN_RECT_INFO, old_rect_info, NULL);
}

/* these next two functions deak with the outer rect */

#define MIN_SW_SIZE 1

Xv_private void
win_set_outer_rect(window, rect)
    Xv_object       window;
    Rect           *rect;
{
    int		    old_rect_info,
		    rect_info = ( WIN_X_SET | WIN_Y_SET |
				  WIN_WIDTH_SET | WIN_HEIGHT_SET);
    Window_info    *win = WIN_PRIVATE(window);

 
    if (win->has_border)
    {
        rect_borderadjust(rect, -WIN_DEFAULT_BORDER_WIDTH);
        if (rect->r_width  <= 0)
            rect->r_width = MIN_SW_SIZE;
        if (rect->r_height  <= 0)
            rect->r_height = MIN_SW_SIZE;
    }

    old_rect_info = (int)xv_get(window, WIN_RECT_INFO);
    xv_set(window, WIN_RECT_INFO, rect_info, NULL);
    window_set_cache_rect(window, rect);
    xv_set(window, WIN_RECT_INFO, old_rect_info, NULL);
}

Xv_private void
win_get_outer_rect(window, rect)
    Xv_object       window;
    Rect           *rect;
{
    window_get_outer_rect(window, rect);
}

/*
 * Utilities
 */
Xv_private void
win_getsize(window, rect)
    Xv_object       window;
    Rect           *rect;
{
    (void) win_getrect(window, rect);
    rect->r_left = 0;
    rect->r_top = 0;
}

Xv_private int
win_get_retained(window)
    Xv_object       window;
{
    register Xv_Drawable_info *info;
    XWindowAttributes wattrs;

    DRAWABLE_INFO_MACRO(window, info);
    XGetWindowAttributes(xv_display(info), xv_xid(info), &wattrs);
    switch (wattrs.backing_store) {
      case Always:
      case WhenMapped:
	return wattrs.backing_store;

      default:
	return FALSE;
    }
}

/* translate coordinates */
Xv_private int
win_translate_xy(src, dst, src_x, src_y, dst_x, dst_y)
    Xv_object       src, dst;
    int             src_x, src_y;
    int            *dst_x, *dst_y;
{
    register Xv_Drawable_info *src_info;
    register Xv_Drawable_info *dst_info;

    DRAWABLE_INFO_MACRO(src, src_info);
    DRAWABLE_INFO_MACRO(dst, dst_info);
    return (win_translate_xy_internal(xv_display(src_info), xv_xid(src_info),
			     xv_xid(dst_info), src_x, src_y, dst_x, dst_y));
}

Xv_private int
win_translate_xy_internal(display, src_id, dst_id, src_x, src_y, dst_x, dst_y)
    Display        *display;
    XID             src_id, dst_id;
    int             src_x, src_y, *dst_x, *dst_y;
{
    XID             child;

    if (XTranslateCoordinates(display, src_id, dst_id, src_x, src_y,
			      dst_x, dst_y, &child))
	return (XV_OK);
    return (XV_ERROR);
}

Xv_private XID
win_pointer_under(window, x, y)
    Xv_opaque       window;
    int             x, y;
{
    /*
     * Given x, y with respect to "window", win_pointer_under() will return
     * the xid of the window lies under that location. If there is no window,
     * 0 will be returned.
     */
    register Xv_Drawable_info *info;
    Window          child;
    int             dst_x, dst_y;
    register Display *display;
    register XID    src_xid, root_xid;
    XID             dst_xid = (XID) 0;


    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    src_xid = xv_xid(info);
    root_xid = (XID) xv_get(xv_root(info), XV_XID);

    /*
     * Make use of the side-effect of XTranslateCoordinates to identify the
     * desired child by translating to self.
     */
    if (XTranslateCoordinates(display, src_xid, root_xid, x, y,
			      &dst_x, &dst_y, &child) == 0)
	return ((XID) 0);

    if (!child)
	return ((XID) 0);

    src_xid = (XID) root_xid;
    x = dst_x;
    y = dst_y;
    dst_xid = (XID) child;

    for (;;) {
	if (XTranslateCoordinates(display, src_xid, dst_xid, x, y,
				  &dst_x, &dst_y, &child) == 0)
	    return ((XID) 0);
	if (!child)
	    break;
	else {
	    src_xid = dst_xid;
	    dst_xid = child;
	    x = dst_x;
	    y = dst_y;
	}
    }

    return (dst_xid);
}
