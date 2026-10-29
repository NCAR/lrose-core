#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)frame.c 20.73 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <xview_private/fm_impl.h>
#include <xview/server.h>
#include <xview_private/draw_impl.h>
#include <xview_private/windowimpl.h>

Xv_private void
frame_cmdline_help(name)
    char           *name;
{
    xv_usage(name);
}

Xv_public void
frame_get_rect(frame, rect)
    Frame           frame;
    Rect           *rect;
{
    
    XWindowAttributes	xwin_attr;
    Xv_Drawable_info	*info;      
    Frame_class_info	*frame_info;
    Window_info		*win;
    XID   		win_xid;
    register Display	*display;
    int			top = 0, bottom = 0, left = 0, right = 0;
    
    if (xv_get(frame, WIN_TYPE) != (Xv_opaque)FRAME_TYPE) {
	XV_BZERO(rect, sizeof(Rect));
        return;
    }
        
    DRAWABLE_INFO_MACRO(frame, info);
    win = WIN_PRIVATE(frame);
    frame_info = FRAME_PRIVATE(frame);
    display = xv_display(info);
    win_xid = xv_xid(info);

    /* Currently the ICCCM does not provide any means for an application to 
     * determine the size of the window manager decor window.  This makes
     * determining the current position and size of the overall application
     * very difficult.  For now we assume we are running under an OPEN LOOK
     * window manager and hope we get resolution from the ICCCM soon. :-(
     */

    /* Server property _SUN_WM_PROTOCOLS is set only by Sun window managers */

    if (!win->top_level_no_decor &&
	screen_get_sun_wm_protocols(xv_screen(info)))  {
        top = 26;
        bottom = 5;
        left = right = 5;
    }
        
    XGetWindowAttributes(display,  win_xid, &xwin_attr);
    
    if ((xwin_attr.map_state == IsViewable) || status_get(frame_info, iconic)) {
        int		x, y;
        XID		notused;
        XTranslateCoordinates(display, win_xid, 
        		      (XID) xv_get(xv_root(info), XV_XID), 
        		      0, 0, &x, &y, &notused);
        xwin_attr.x = x - left;
        xwin_attr.y = y - top;
    }
    			  
    rect->r_left = xwin_attr.x;
    rect->r_top = xwin_attr.y;
    rect->r_width = xwin_attr.width + left + right;
    rect->r_height = xwin_attr.height + top + bottom;  
}

Xv_public void
frame_set_rect(frame, rect)
    Frame           frame;
    Rect           *rect;
{
    
    XWindowAttributes	xwin_attr;
    Xv_Drawable_info	*info;
    Window_info		*win;
    Frame_class_info	*frame_info;
    register Display	*display; 
    int			top = 0, bottom = 0, left = 0, right = 0;
    int			x_adj=0, y_adj=0;

    if (xv_get(frame, WIN_TYPE) != (Xv_opaque)FRAME_TYPE)
        return;
        
    DRAWABLE_INFO_MACRO(frame, info);
    win = WIN_PRIVATE(frame);
    frame_info = FRAME_PRIVATE(frame);
    display = xv_display(info); 
    
    /* Currently the ICCCM does not provide any means for an application to 
     * determine the size of the window manager decor window.  This makes
     * determining the current position and size of the overall application
     * very difficult.  For now we assume we are running under an OPEN LOOK
     * window manager and hope we get resolution from the ICCCM soon. :-(
     */

    /* Server property _SUN_WM_PROTOCOLS is set only by Sun window managers */

    if (!win->top_level_no_decor &&
	screen_get_sun_wm_protocols(xv_screen(info)))  {

	top = 26;
	bottom = 5;
	left = right = 5;

	/* making frame_set_rect() work on mapped frames requires adjustments */
	
	XGetWindowAttributes(display,  xv_xid(info), &xwin_attr);
	
	if ((xwin_attr.map_state == IsViewable) || 
	    status_get(frame_info, iconic)) {
	    x_adj = 5;
	    y_adj = 26;
	}
    }

    if (!(frame_info->normal_hints.flags & PSize) ||
	!(frame_info->normal_hints.flags & PPosition)) {
        frame_info->normal_hints.flags |= PSize | PPosition;
        XSetWMNormalHints(display, xv_xid(info), &frame_info->normal_hints);
    }

    /*
     *  This is setting the frame x, y relative to the root,
     *  and the width and height adjust for the outer rect.
     *  This will result in setting the window decor with the
     *  correct rect. 
     */
    XMoveResizeWindow(xv_display(info),  xv_xid(info), 
      rect->r_left + x_adj,
      rect->r_top + y_adj,
      rect->r_width - (left + right), 
      rect->r_height - (top + bottom));
}
