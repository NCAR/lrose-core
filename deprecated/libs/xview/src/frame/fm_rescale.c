#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fm_rescale.c 20.21 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/fm_impl.h>
#include <xview/font.h>

/*
 * rescale the sub_tree
 */

Pkg_private void
frame_rescale_subwindows(frame_public, scale)
    Frame           frame_public;
    int             scale;
{
    Frame_class_info *frame = FRAME_CLASS_PRIVATE(frame_public);
    register Xv_Window sw;
    Rect            new_rect;
    Window_rescale_rect_obj *rect_obj_list;
    int             i = 0;
    int             num_sws = 0;
    int             frame_width, frame_height;


    /*
     * this is do able only for the frame
     * xv_set(frame_public,WIN_RESCALE_AND_SIZE,0);
     */


    /*
     * if this is not called from the frame_input function then call it. This
     * gets teh correct font and the correct scale.
     */
    window_default_event_func(frame_public, (Event *) 0, scale, (Notify_event_type) 0);
    window_calculate_new_size(frame_public, frame_public, &frame_height, &frame_width);
#if !defined(__linux) && !defined(__APPLE__)
    xv_set(frame_public, WIN_RECT, NULL); /* This looks like a XView bug to me */
#endif


    /*
     * must rescale inner rect but should layout according to outer rect
     */

    FRAME_EACH_SUBWINDOW(frame, sw)	/* count number of sw's */
	num_sws++;
    FRAME_END_EACH
	rect_obj_list = window_create_rect_obj_list(num_sws);
    FRAME_EACH_SUBWINDOW(frame, sw)
	window_set_rescale_state(sw, scale);
    window_start_rescaling(sw);
    window_add_to_rect_list(rect_obj_list, sw, (Rect *) xv_get(sw, WIN_RECT), i);
    i++;
    FRAME_END_EACH
	window_adjust_rects(rect_obj_list, num_sws, frame_width, frame_height);
    FRAME_EACH_SUBWINDOW(frame, sw)
	if (!window_rect_equal_ith_obj(rect_obj_list, &new_rect, i))
	xv_set(sw, WIN_RECT, &new_rect, NULL);
    window_end_rescaling(sw);
    FRAME_END_EACH
	window_destroy_rect_obj_list(rect_obj_list);
}
