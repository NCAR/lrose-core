#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fm_geom.c 20.31 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/fm_impl.h>
#include <xview/window.h>
#include <xview/font.h>
#include <pixrect/pixfont.h>

/* OPEN LOOK footer sizes */
/* Pre-subtracted 3 for the size of the resize handle */
#define FRAME_FOOTER_SMALL_HEIGHT	17
#define FRAME_FOOTER_MEDIUM_HEIGHT	19
#define FRAME_FOOTER_LARGE_HEIGHT	21
#define FRAME_FOOTER_EXTRALARGE_HEIGHT	29

/* Pre-subtracted 3 for the size of the resize handle */
#define FRAME_FOOTER_SMALL_BASELINE		4
#define FRAME_FOOTER_MEDIUM_BASELINE		4
#define FRAME_FOOTER_LARGE_BASELINE		5
#define FRAME_FOOTER_EXTRALARGE_BASELINE	8

/* Pre-subtracted 3 for the size of the resize handle */
#define FRAME_FOOTER_SMALL_MARGIN		7
#define FRAME_FOOTER_MEDIUM_MARGIN		9
#define FRAME_FOOTER_LARGE_MARGIN		11
#define FRAME_FOOTER_EXTRALARGE_MARGIN		16

#define FRAME_FOOTER_SMALL_GAP		4
#define FRAME_FOOTER_MEDIUM_GAP		4
#define FRAME_FOOTER_LARGE_GAP	        5
#define FRAME_FOOTER_EXTRALARGE_GAP	7

extern Pixfont *xv_pf_sys;

static int      frame_sw_size_compute();

/* ARGSUSED */
Pkg_private int
frame_height_from_lines(n, show_label)
    int             n, show_label;
{
    return (n * xv_get((Xv_opaque)xv_pf_sys, FONT_DEFAULT_CHAR_HEIGHT)) + FRAME_BORDER_WIDTH;
}

Pkg_private int
frame_width_from_columns(x)
    int             x;
{
    return (x * xv_get((Xv_opaque)xv_pf_sys, FONT_DEFAULT_CHAR_WIDTH)) + 
		(2 * FRAME_BORDER_WIDTH);
}

/*
 * Position subwindow in frame. This is only called by frame_layout for
 * XV_CREATE, so the width and height are always WIN_EXTEND_TO_EDGE.
 */
Pkg_private void
frame_position_sw(frame,nsw, swprevious, width, height, rect)
    Frame_class_info *frame;
    Xv_Window       nsw;
    Xv_Window       swprevious;
    int             width, height;
    Rect           *rect;
{
    Rect            framerect;
    Rect            current_rect;
    unsigned int    rect_info;

    /*
     * Iconic adjustments
     */

    framerect = *(Rect *)xv_get(FRAME_PUBLIC(frame), XV_RECT);
    (void) win_getrect(nsw,&current_rect);
    rect_info = (unsigned int) xv_get(nsw,WIN_RECT_INFO);

    /*
     * Set position: set up new subwindow (nsw) relative to previous
     * subwindow (psw) in the list. If psw.width is default then assume that
     * it extends to right edge and start nsw below it and flush left.
     * Otherwise, put to the right of it. Use outer rect for calculations but
     * return inner rect
     */
    if (swprevious) {
	win_get_outer_rect(swprevious, rect);	/* outer rect */
	if (EXTEND_WIDTH(swprevious)) {
	    rect->r_top += rect->r_height;
	    rect->r_left = FRAME_BORDER_WIDTH;
	} else {
	    rect->r_left += rect->r_width;
	}
    } else {

	rect->r_top = FRAME_BORDER_WIDTH;	/* frame_stripe_height(frame) */
	rect->r_left = FRAME_BORDER_WIDTH;
    }

    /* Reset rect->r_top and rect->r_left if the client has explicitly
     * set WIN_X and WIN_Y
     */

     if (rect_info & WIN_X_SET) 
         rect->r_left = current_rect.r_left;
     if (rect_info & WIN_Y_SET)
	 rect->r_top  = current_rect.r_top;
 
    /*
     * Compute width & height
     */
    rect->r_width = frame_sw_size_compute(width,
					  framerect.r_width - rect->r_left);
    rect->r_height = frame_sw_size_compute(height,
					  framerect.r_height - rect->r_top);
    if (swprevious)
	window_outer_to_innerrect(swprevious, rect);	/* decrease the rect if
							 * the window
							 * (swprevious) has a
							 * border -- get inner
							 * border */
}

/*
 * Set size: set up new subwindow (nsw) size in available space (avs). If avs
 * is <=0 then set nsw to be requested amount. If avs is > 0 then set nsw =
 * min(requested amount, avs). If nsw = default then use avs unless avs = 0
 * inwhich case choose some small constant.  We don't want 0 size.
 */
static int
frame_sw_size_compute(request, avs)
    int             request, avs;
{
    int             nsw;

    if (request == WIN_EXTEND_TO_EDGE) {
	nsw = (avs <= 0) ? 16 : avs;
    } else
	nsw = request;
    if (avs > 0)
	nsw = MIN(nsw, avs);
    return (nsw);
}

Pkg_private int
frame_footer_height(scale)
    Frame_rescale_state scale;
{
    int height;
    
    switch (scale) {
      case WIN_SCALE_SMALL:
	height = FRAME_FOOTER_SMALL_HEIGHT;
	break;
      case WIN_SCALE_MEDIUM:
	height = FRAME_FOOTER_MEDIUM_HEIGHT;
	break;
      case WIN_SCALE_LARGE:
	height = FRAME_FOOTER_LARGE_HEIGHT;
	break;
      case WIN_SCALE_EXTRALARGE:
	height = FRAME_FOOTER_EXTRALARGE_HEIGHT;
	break;
      default:
	height = FRAME_FOOTER_MEDIUM_HEIGHT;
	break;
    }
    return(height);
}

#ifdef OW_I18N
Pkg_private int
frame_IMstatus_height(scale)
    Frame_rescale_state scale;
{
    int height;
    
    switch (scale) {
      case WIN_SCALE_SMALL:
	height = FRAME_FOOTER_SMALL_HEIGHT;
	break;
      case WIN_SCALE_MEDIUM:
	height = FRAME_FOOTER_MEDIUM_HEIGHT;
	break;
      case WIN_SCALE_LARGE:
	height = FRAME_FOOTER_LARGE_HEIGHT;
	break;
      case WIN_SCALE_EXTRALARGE:
	height = FRAME_FOOTER_EXTRALARGE_HEIGHT;
	break;
      default:
	height = FRAME_FOOTER_MEDIUM_HEIGHT;
	break;
    }
    return(height);
}
#endif

/*
 * frame_footer_baseline - return the distance from the bottom 
 *                         of the footer to the baseline of the footer's text
 */
Pkg_private int
frame_footer_baseline(scale)
    Frame_rescale_state scale;
{
    int baseline;
    
    switch (scale) {
      case WIN_SCALE_SMALL:
	baseline = FRAME_FOOTER_SMALL_BASELINE;
	break;
      case WIN_SCALE_MEDIUM:
	baseline = FRAME_FOOTER_MEDIUM_BASELINE;
	break;
      case WIN_SCALE_LARGE:
	baseline = FRAME_FOOTER_LARGE_BASELINE;
	break;
      case WIN_SCALE_EXTRALARGE:
	baseline = FRAME_FOOTER_EXTRALARGE_BASELINE;
	break;
      default:
	baseline = FRAME_FOOTER_MEDIUM_BASELINE;
	break;
    }
    return(baseline);
}

/* 
 * frame_footer_margin - return the size of the left
 *                       and right margin for the footers.
 */
Pkg_private int
frame_footer_margin(scale)
    Frame_rescale_state scale;
{
    int margin;
    
    switch (scale) {
      case WIN_SCALE_SMALL:
	margin = FRAME_FOOTER_SMALL_MARGIN;
	break;
      case WIN_SCALE_MEDIUM:
	margin = FRAME_FOOTER_MEDIUM_MARGIN;
	break;
      case WIN_SCALE_LARGE:
	margin = FRAME_FOOTER_LARGE_MARGIN;
	break;
      case WIN_SCALE_EXTRALARGE:
	margin = FRAME_FOOTER_EXTRALARGE_MARGIN;
	break;
      default:
	margin = FRAME_FOOTER_MEDIUM_MARGIN;
	break;
    }
    return(margin);
}
    
/*
 * frame_inter_footer_gap - return the size of the minimum gap 
 *                          between the footers in pixels
 */
Pkg_private int
frame_inter_footer_gap(scale)
    Frame_rescale_state scale;
{
    int gap;
    
    switch (scale) {
      case WIN_SCALE_SMALL:
	gap = FRAME_FOOTER_SMALL_GAP;
	break;
      case WIN_SCALE_MEDIUM:
	gap = FRAME_FOOTER_MEDIUM_GAP;
	break;
      case WIN_SCALE_LARGE:
	gap = FRAME_FOOTER_LARGE_GAP;
	break;
      case WIN_SCALE_EXTRALARGE:
	gap = FRAME_FOOTER_EXTRALARGE_GAP;
	break;
      default:
	gap = FRAME_FOOTER_MEDIUM_GAP;
	break;
    }
    return(gap);
}
