#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)win_cursor.c 20.24 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Win_cursor.c: Implement the mouse cursor operation functions of the
 * win_struct.h interface.
 */
#include <xview/rect.h>
#include <xview/server.h>
#include <xview/win_struct.h>
#include <xview_private/draw_impl.h>
#include <X11/Xlib.h>

/*
 * Mouse cursor operations.
 */
win_setmouseposition(window, x, y)
    Xv_object       window;
    short           x, y;
{
    Xv_Drawable_info *info;

    DRAWABLE_INFO_MACRO(window, info);
    if (!(xv_get(xv_server(info), SERVER_JOURNALLING)))
	win_setmouseposition_internal(xv_display(info), xv_xid(info), x, y);
}

win_setmouseposition_internal(display, xid, x, y)
    Display        *display;
    unsigned int    xid;
    short           x, y;
{

    /* if the src window is none, the move is independent */
    XWarpPointer(display, None, xid, 0, 0, 0, 0, x, y);
    XSync(display, 0);
    XAllowEvents(display, SyncPointer, CurrentTime);
}

win_getmouseposition(window, x, y)
    Xv_object       window;
    short          *x, *y;
{
    Xv_Drawable_info *info;
    XID             root, child;
    int             rootx, rooty, dummy_x, dummy_y;
    unsigned int             mask;

    DRAWABLE_INFO_MACRO(window, info);
    XQueryPointer(xv_display(info), xv_xid(info), &root, &child,
		  &rootx, &rooty, &dummy_x, &dummy_y, &mask);
    *x = dummy_x;
    *y = dummy_y;
}

XID
win_findintersect(window, x, y)
    Xv_object       window;
    short           x, y;
{
    /*
     * Given x, y with respect to "window", win_findintersect() will return
     * the xid of the window lies under that location, iff it is a child of
     * "window".  Otherwise, WIN_NULLLINK will be returned.  If x, y is in
     * "window", but "window" does not have a child, then it will return
     * WIN_NULLLINK.
     */
    register Xv_Drawable_info *info;
    Window          child;
    int             dst_x, dst_y;
    register Display *display;
    XID             src_xid;
    XID             dst_xid = (XID) WIN_NULLLINK;


    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    src_xid = xv_xid(info);

    if (XTranslateCoordinates(display, src_xid, src_xid, x, y,
			      &dst_x, &dst_y, &child) == 0) {
	return ((XID) WIN_NULLLINK);
    }
    if (child == 0) {
	return ((XID) WIN_NULLLINK);
    }
    dst_xid = (XID) child;

    /*
     * Make use of the side-effect of XTranslateCoordinates to identify the
     * desired child by translating to self.
     */
    for (;;) {
	if (XTranslateCoordinates(display, src_xid, dst_xid, x, y,
				  &dst_x, &dst_y, &child) == 0)
	    return ((XID) WIN_NULLLINK);
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
