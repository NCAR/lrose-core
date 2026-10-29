#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)cnvs_scrol.c 20.23 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/cnvs_impl.h>
#include <xview/scrollbar.h>

Pkg_private void
canvas_set_scrollbar_object_length(canvas, direction, sb)
    register Canvas_info *canvas;
    Scrollbar_setting direction;
    Scrollbar       sb;
{
    int             is_vertical = direction == SCROLLBAR_VERTICAL;
    int             pixels_per;
    long unsigned   current_length, new_length;

    if (sb) {
	pixels_per = (int) xv_get(sb, SCROLLBAR_PIXELS_PER_UNIT);

	if (pixels_per == 0) {
	    pixels_per = 1;
	}
	new_length = ((is_vertical) ?
		      canvas->height : canvas->width) / pixels_per;
	current_length = (long unsigned) xv_get(sb, SCROLLBAR_OBJECT_LENGTH);

	if (new_length != current_length) {
	    /* let the scrollbar know how big the scrolling area is */
	    (void) xv_set(sb,
			  SCROLLBAR_OBJECT_LENGTH, new_length,
			  NULL);
	}
    }
}


Pkg_private void
canvas_update_scrollbars(canvas)
    Canvas_info    *canvas;
{
    Canvas          canvas_public = CANVAS_PUBLIC(canvas);
    Xv_Window       view;
    Scrollbar       sb;

    OPENWIN_EACH_VIEW(canvas_public, view)
	sb = (Scrollbar) xv_get(canvas_public, OPENWIN_VERTICAL_SCROLLBAR, view);
        if (sb) {
		canvas_set_scrollbar_object_length(canvas, SCROLLBAR_VERTICAL, sb);
		canvas_scroll(xv_get(view, CANVAS_VIEW_PAINT_WINDOW, NULL), sb);
	}
        sb = (Scrollbar) xv_get(canvas_public, OPENWIN_HORIZONTAL_SCROLLBAR, view);
        if (sb) {
		canvas_set_scrollbar_object_length(canvas, SCROLLBAR_HORIZONTAL, sb);
		canvas_scroll(xv_get(view, CANVAS_VIEW_PAINT_WINDOW, NULL), sb);
	}		
    OPENWIN_END_EACH
}

/*
 * scroll the canvas according to LAST_VIEW_START and VIEW_START.
 */
Pkg_private void
canvas_scroll(paint_window, sb)
    Xv_Window       paint_window;
    Scrollbar       sb;
{
    int             offset = (int) xv_get(sb, SCROLLBAR_VIEW_START);
    int             old_offset = (int) xv_get(sb, SCROLLBAR_LAST_VIEW_START);
    int             is_vertical;
    int             pixels_per;

    if (offset == old_offset)
	return;
    is_vertical = (Scrollbar_setting) xv_get(sb, SCROLLBAR_DIRECTION) == SCROLLBAR_VERTICAL;
    pixels_per = (int) xv_get(sb, SCROLLBAR_PIXELS_PER_UNIT);
    xv_set(paint_window, is_vertical ? XV_Y : XV_X, -(offset * pixels_per), NULL);

}
