#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fm_win.c 20.31 90/11/08";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifdef SVR4
#include <sys/types.h>
#endif /* SVR4 */
#include <sys/file.h>
#include <X11/Xlib.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/fm_impl.h>
#include <xview_private/draw_impl.h>

/*
 * use the window manager to set the position and trim the size of the base
 * frame.
 */
/* ARGSUSED */
Pkg_private void
frame_set_position(parent, frame)
    unsigned long   parent;
    Frame_class_info *frame;
{
    Frame           frame_public = FRAME_PUBLIC(frame);
    Rect            rectnormal;
    /*
     * int		rect_info = (int) xv_get(frame_public,
     * WIN_RECT_INFO);
     */

    win_getrect(frame_public, &rectnormal);

    /* if the position is not set, then let the window manager set it */

    /*
     * csk 5/3/89: rect_info is never set to anything but 0.  Should it be
     * removed?  If anything, it should test rectnormal.r_* for set values...
     * This code needs to be reviewed by Mr. Ng!!
     * 
     * if (!(rect_info & WIN_X_SET)) rectnormal.r_left = 0; if (!(rect_info &
     * WIN_Y_SET)) rectnormal.r_top = 0;
     */


    win_setrect(frame_public, &rectnormal);
    frame->rectcache = rectnormal;

    /*
     * Cached rect is self relative.
     */
    frame->rectcache.r_left = frame->rectcache.r_top = 0;
}

Pkg_private int
frame_is_exposed(frame)
    Frame           frame;
{
    XID             root, frame_xid, tmp;
    XID            *children;
    Display        *display;
    Rect            child_rect, frame_rect;
    Xv_Drawable_info *frame_info;
    Xv_Drawable_info *root_info;
    Xv_object       rootwindow;
    unsigned int    nchildren;

    DRAWABLE_INFO_MACRO(frame, frame_info);
    frame_xid = xv_xid(frame_info);
    (void) win_getrect(frame, &frame_rect);

    /*
     * It is assumed that the frame is a child of root window
     */
    rootwindow = xv_get(frame, XV_ROOT);
    DRAWABLE_INFO_MACRO(rootwindow, root_info);
    root = xv_xid(root_info);
    display = xv_display(frame_info);


    if (XQueryTree(display, root, &tmp, &tmp, &children, &nchildren) == 0) {
	xv_error(frame,
		 ERROR_STRING, 
		 XV_MSG("frame_is_exposed(): XQueryTree failed!"),
		 ERROR_PKG, FRAME,
		 NULL);
	goto deallocate;
    }
    if (nchildren) {
	register XID   *c;

	/*
	 * Find the frame_xid among the children. No frame_xid among Children
	 * indicates error
	 */
	for (c = children; nchildren && (*c != frame_xid); nchildren--, c++);
	if (*c++ != frame_xid) {
	    xv_error(frame,
		     ERROR_STRING, 
		     XV_MSG("frame_is_exposed(): window not in tree"),
		     ERROR_PKG, FRAME,
		     NULL);
	    goto deallocate;
	}
	/*
	 * Scan through all the children of the root window that are above
	 * this frame
	 */
	for (--nchildren; nchildren; nchildren--, c++) {
	    if (win_view_state(display, *c) != IsViewable)
		continue;
	    win_x_getrect(display, *c, &child_rect);
	    if (rect_intersectsrect(&frame_rect, &child_rect))
		break;

	}
	return (nchildren ? FALSE : TRUE);
    }
deallocate:
    if (children)
	xv_free(children);
    return (FALSE);
}
