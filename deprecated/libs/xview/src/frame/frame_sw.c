#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)frame_sw.c 20.26 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/fm_impl.h>

#ifdef OW_I18N
#ifdef FULL_R5
Pkg_private void frame_update_status_subwindows();
#endif
#endif

/*
 * some parts of the system cannot deal with zero sized subwindows.  So, when
 * we must cut the size of a subwindow to less than this constant, we use
 * this constant. This will cause a loss of frame borders.
 */
#define	MIN_SW_SIZE	1
/*
 * Layout subwindows in frame. Here we preserve extend-to-edgeness of the
 * subwindows: if a subwindow's width or height is extend-to-edge, the
 * corresponding value in the subwindows's rect is extended to the border of
 * the frame. If the subwindow crosses the edge of the frame, cut back the
 * width/height. If the subwindow is not as big as it should be (desired
 * width/height), expand the subwindow to tile the frame.
 */
Pkg_private void
frame_layout_subwindows(frame_public)
    Frame           frame_public;
{
    Frame_class_info *frame = FRAME_CLASS_PRIVATE(frame_public);
    register Xv_Window sw;
    Rect            rect, old_rect, rconstrain;
    int             width, height;
    register short  right, bottom, outer_right, outer_bottom, avs;
    short           hole, need_constraint;
    int             desired_width, desired_height;
    Pkg_private void frame_compute_constraint();

    width = (int)xv_get(frame_public, XV_WIDTH);
    height = (int)xv_get(frame_public, XV_HEIGHT);

    /*
     * The server generates ConfigureNotify events even when the frame is
     * moved, restacked etc. Therefore, check to see if size has really
     * changed before further processing. Note - This used to be done in
     * xevent_to_event() to avoid the overhead of posting redundant RESIZE
     * events through the notifier, but this was changed to accomodate the
     * splitting of the library into intrinsics.
     */
    if ((frame->oldrect.r_width == width) &&
	(frame->oldrect.r_height == height)) {
	return;
    } else {
	frame->oldrect.r_width = width;
	frame->oldrect.r_height = height;
    }


    /*
     * compute outer dimensions in subwindow space. i.e. (0, 0) is just below
     * the name stripe.
     */
    outer_right = width;
    outer_bottom = height;

    right = outer_right - 1;	/* FRAME_BORDER_WIDTH - 1; */
    bottom = outer_bottom - 1;	/* FRAME_BORDER_WIDTH - 1; */

    FRAME_EACH_SHOWN_SUBWINDOW(frame, sw) {
		/* We have to adjust for the border under certain cases.
		 * When the desired_width/height is set, it does not account
		 * for the border.  So as an example, if we check to see
		 * if the avs will fit the desired_width, we must make
		 * sure there is also enough room for the border.  See
		 * 1065462 for more details.
		 */
	int bw_adjustment = xv_get(sw, WIN_BORDER) * 2;

        /* 
         * Skip icons 
         */ 
        if (xv_get(sw, XV_IS_SUBTYPE_OF, ICON))  {
            continue;
        }

        /*
         * window_get(sw, WIN_RECT) is the right call here but win_getrect is
         * compatible with win_setrect.
         */
	(void) win_get_outer_rect(sw, &rect);
        old_rect = rect;
        need_constraint = TRUE;

        /* adjust the width */
        if (rect.r_left < outer_right) {
	    avs = right - rect.r_left + 1;
	    if (avs < MIN_SW_SIZE)
	        avs = MIN_SW_SIZE;
	    desired_width = (int) window_get(sw, WIN_DESIRED_WIDTH);
	    if (desired_width == WIN_EXTEND_TO_EDGE)
	        /* preserve extend-to-edgeness */
	        rect.r_width = avs;
	    else if (rect_right(&rect) >= right)
	        /* cut the width back to save the border */
	        rect.r_width = MIN(avs, desired_width + bw_adjustment);
	    else if (rect.r_width < desired_width + bw_adjustment) {
	        /* try to expand the width to desired width */
	        frame_compute_constraint(frame, sw, &rconstrain);
	        need_constraint = FALSE;
	        /* rect_marginadjust(&rect, swsp); */
	        hole = rect_right(&rconstrain) - rect_right(&rect);
	        if (hole > 0)
		    rect.r_width += hole;
	        /* rect_marginadjust(&rect, -swsp); */
	        if (rect.r_width > desired_width + bw_adjustment)
		    rect.r_width = desired_width + bw_adjustment;
	    }
        }
        /* adjust the height */
        if (rect.r_top < outer_bottom) {
	    avs = bottom - rect.r_top + 1;
	    if (avs < MIN_SW_SIZE)
	        avs = MIN_SW_SIZE;
	    desired_height = (int) window_get(sw, WIN_DESIRED_HEIGHT);
	    if (desired_height == WIN_EXTEND_TO_EDGE)
	        /* preserve extend-to-edgeness */
	        rect.r_height = avs;
	    else if (rect_bottom(&rect) >= bottom)
	        /* cut the height back to save the border */
	        rect.r_height = MIN(avs, desired_height + bw_adjustment);
	    else if (rect.r_height < desired_height + bw_adjustment) {
	        /* try to expand the height to desired height */
	        if (need_constraint) {
		    frame_compute_constraint(frame, sw, &rconstrain);
	        }
	        /* rect_marginadjust(&rect, swsp); */
	        hole = rect_bottom(&rconstrain) - rect_bottom(&rect);
	        if (hole > 0)
		    rect.r_height += hole;
	        /* rect_marginadjust(&rect, -swsp); */
	        if (rect.r_height > desired_height + bw_adjustment)
		    rect.r_height = desired_height + bw_adjustment;
	    }
        }
        if (!rect_equal(&rect, &old_rect)) {
	    /*
	     * window_set(sw, WIN_RECT, &rect, 0) is the right call here but
	     * that changes WIN_DESIRED_WIDTH and as a result WIN_EXTEND_TO_EDGE
	     * functionality is lost.
	     */
	    (void) win_set_outer_rect(sw, &rect);
        }
#ifdef OW_I18N
#ifdef FULL_R5
       /* 
        * When frame gets resized make sure that all subwindows
        * which have IM on and IM status style of XIMStatusArea
        * resets the XNArea for status region.
        */
       if (status_get(frame, show_imstatus))
           (void) frame_update_status_subwindows(frame_public, sw);
#endif
#endif
        FRAME_END_EACH
    }
}

#ifdef OW_I18N
#ifdef FULL_R5
Pkg_private void
frame_update_status_subwindows(frame_public, sw)
    Frame           frame_public;
    Xv_Window	    sw;	
{
    Frame_class_info   	*frame = FRAME_CLASS_PRIVATE(frame_public);
    Rect 		*xv_rect;	
    XRectangle      	x_rect;
    XVaNestedList   	va_nested_list;
    XIC			ic;

    /*
     * If WIN_USE_IM is TRUE, and the current status style
     * is XIMStatusArea, then we should update the status
     * region's XNArea.
     */
    if (xv_get(sw, WIN_USE_IM)) {
	if (((XIMStyle)xv_get(sw, WIN_X_IM_STYLE_MASK) & XIMStatusArea)) {
	      ic = (XIC)xv_get(sw, WIN_IC);
	      if (ic) {
                  /*
                   * Get the new rect for the IM status region
                   */
                  xv_rect = (Rect *)xv_get(frame->imstatus, WIN_RECT);

                  x_rect.x = (short)xv_rect->r_left;
                  x_rect.y = (short)xv_rect->r_top;
                  x_rect.width = (unsigned short)xv_rect->r_width;
                  x_rect.height = (unsigned short)xv_rect->r_height;

                  va_nested_list = XVaCreateNestedList(NULL,
                                                       XNArea, &x_rect,
                                                       NULL);

                  XSetICValues(ic, XNStatusAttributes, va_nested_list, NULL);
    	          XFree(va_nested_list);
	      }
	}	
    }

}
#endif /* FULL_R5 */
#endif /* OW_I18N */
