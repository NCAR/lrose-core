#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sb_pos.c 1.48 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Module:	sb_pos.c
 * 
 * Description:
 * 
 * positions elements in the scrollbar
 * 
 */


/*
 * Include files:
 */
#include <xview_private/sb_impl.h>

Pkg_private void sb_minimum();
Pkg_private void sb_abbreviated();
Pkg_private void sb_full_size();


Pkg_private void
sb_resize(sb)
Xv_scrollbar_info *sb;
{
    Rect r;
    int anchors_height;

    r = *(Rect *)xv_get(SCROLLBAR_PUBLIC(sb), WIN_RECT);
    sb_normalize_rect(sb, &r);
    sb->length = r.r_height;

    anchors_height = 2 * (sb_marker_height(sb) + SCROLLBAR_CABLE_GAP);
    sb->cable_height = r.r_height - anchors_height;
    
    if ((anchors_height + sb_elevator_height(sb, SCROLLBAR_ABBREVIATED)) > sb->length)
      sb_minimum(sb);
    else if (sb->cable_height <= sb_elevator_height(sb, SCROLLBAR_FULL_SIZE))
      sb_abbreviated(sb);
    else
      sb_full_size(sb);
}

Pkg_private void
scrollbar_init_positions(sb)
	Xv_scrollbar_info *sb;
{
	sb->scale = 
	  (Frame_rescale_state)xv_get(xv_get(SCROLLBAR_PUBLIC(sb), XV_FONT), FONT_SCALE);

	sb_resize(sb);

	if (sb->object_length == SCROLLBAR_DEFAULT_LENGTH)
	  sb->object_length = sb->length / sb->pixels_per_unit;
	if (sb->view_length == SCROLLBAR_DEFAULT_LENGTH)
	  sb->view_length = sb->length / sb->pixels_per_unit;
	
	sb->cable_start = sb_marker_height(sb) + SCROLLBAR_CABLE_GAP;
	sb->cable_height = sb->length - 2 * (sb_marker_height(sb) + SCROLLBAR_CABLE_GAP);

	/* initialize elevator rect */
	if (sb->size == SCROLLBAR_FULL_SIZE)
	  sb->elevator_rect.r_top = sb->cable_start;
	sb->elevator_rect.r_left = sb_margin(sb);
	sb->elevator_rect.r_width = ScrollbarElevator_Width(sb->ginfo);

	xv_set(SCROLLBAR_PUBLIC(sb),
	       (SB_VERTICAL(sb) ? WIN_WIDTH : WIN_HEIGHT),
	           scrollbar_width_for_scale(sb->scale),
	       NULL);
}


Pkg_private void
scrollbar_position_elevator(sb, paint, motion)
	Xv_scrollbar_info *sb;
	int                paint;	/* TRUE or FALSE */
	Scroll_motion      motion;
{
    int             available_cable;
    int             new_top;

    available_cable = scrollbar_available_cable(sb);
    /* Bounds checking */
    if (sb->view_start > Max_offset(sb)) {
	sb->view_start = Max_offset(sb);
    }
    if ((sb->size == SCROLLBAR_FULL_SIZE) && (motion != SCROLLBAR_ABSOLUTE)) {
	if (sb->view_start == 0 || sb->object_length <= sb->view_length) {
	    /* At beginning */
	    new_top = sb->cable_start;
	} else {
	    new_top = (double)sb->view_start * (double)available_cable /(double) Max_offset(sb);
	    if (new_top < 3) {
		/* Not at beginning, leave anchor a little */
		new_top = (available_cable < 3) ? available_cable : 3;
	    } else if (sb->view_start < Max_offset(sb)
		       && new_top > available_cable - 3 && available_cable > 3) {
		/* Not at end, leave anchor a little */
		new_top = available_cable - 3;
	    }
	    new_top += sb->cable_start;
	}
    } else
      /*
       * User has positioned (e.g., dragged) the elevator to a specific
       * point.  We don't want to jump the elevator to the "true" position
       * (to indicate the view start) because this is poor user interface.
       */
      new_top = sb->elevator_rect.r_top;
    
    if (paint)
      scrollbar_paint_elevator_move(sb, new_top);
    else
      sb->elevator_rect.r_top = new_top;
}

Pkg_private void
scrollbar_absolute_position_elevator(sb, pos)
    Xv_scrollbar_info *sb;
    int             pos;
{
    int available_cable = scrollbar_available_cable(sb);

    if (pos < 0 || available_cable <= 0) {
	pos = 0;
    } else if (pos > available_cable) {
	pos = available_cable;
    }
    /* add back in cable_start offset */
    pos += sb->cable_start;

    scrollbar_paint_elevator_move(sb, pos);
}

/*
 * scrollbar_top_anchor_rect - return the normalized rect of the top anchor
 */
Pkg_private void
scrollbar_top_anchor_rect(sb, r)
    Xv_scrollbar_info *sb;
    Rect              *r;	/* RETURN VALUE */
{
    r->r_left = sb_margin(sb);
    r->r_width = Vertsb_Endbox_Width(sb->ginfo);
    r->r_height = Vertsb_Endbox_Height(sb->ginfo);
    if (sb->size != SCROLLBAR_FULL_SIZE)
      r->r_top = sb->elevator_rect.r_top - SCROLLBAR_CABLE_GAP - r->r_height;
    else 
      r->r_top = 0;
}


/*
 * scrollbar_bottom_anchor_rect - return the normalized rect of the bottom anchor
 */
Pkg_private void
scrollbar_bottom_anchor_rect(sb, r)
    Xv_scrollbar_info *sb;
    Rect              *r;	/* RETURN VALUE */
{
    r->r_left = sb_margin(sb);
    if (sb->size == SCROLLBAR_FULL_SIZE)
      r->r_top = sb->length - Vertsb_Endbox_Height(sb->ginfo);
    else 
      r->r_top = rect_bottom(&sb->elevator_rect) + 1 + SCROLLBAR_CABLE_GAP;
    r->r_width = Vertsb_Endbox_Width(sb->ginfo);
    r->r_height = Vertsb_Endbox_Height(sb->ginfo);
}

Pkg_private void
sb_normalize_rect(sb, r)
    Xv_scrollbar_info *sb;
    Rect           *r;
{
    int             temp;

    if (!SB_VERTICAL(sb)) {
	temp = r->r_top;
	r->r_top = r->r_left;
	r->r_left = temp;
	temp = r->r_width;
	r->r_width = r->r_height;
	r->r_height = temp;
    }
}

Pkg_private int
scrollbar_available_cable(sb)
    Xv_scrollbar_info *sb;
{
    int            available;

    available = sb->cable_height - sb->elevator_rect.r_height;
    if (available < 0)
      available = 0;
    return available;
}

Pkg_private void
sb_minimum(sb)
    Xv_scrollbar_info *sb;
{
    sb_abbreviated(sb);
    sb->size = SCROLLBAR_MINIMUM;
}

Pkg_private void
sb_abbreviated(sb)
    Xv_scrollbar_info *sb;
{
    sb->size = SCROLLBAR_ABBREVIATED;
    sb->elevator_rect.r_height = sb_elevator_height(sb, sb->size);
    if((sb->elevator_rect.r_top = (sb->length / 2) - 
	(sb->elevator_rect.r_height / 2)) < 0)
	sb->elevator_rect.r_top = 0;
    sb->length = sb->elevator_rect.r_height;
    scrollbar_top_anchor_rect(sb, &sb->top_anchor_rect);
    scrollbar_bottom_anchor_rect(sb, &sb->bottom_anchor_rect);
}

Pkg_private void
sb_full_size(sb)
    Xv_scrollbar_info *sb;
{
    sb->size = SCROLLBAR_FULL_SIZE;
    sb->elevator_rect.r_height = sb_elevator_height(sb, sb->size);
    scrollbar_top_anchor_rect(sb, &sb->top_anchor_rect);
    scrollbar_bottom_anchor_rect(sb, &sb->bottom_anchor_rect);
}

Pkg_private int
sb_margin(sb)
    Xv_scrollbar_info *sb;
{
    switch (sb->scale) {
      case WIN_SCALE_SMALL:
	return SB_SMALL_MARGIN;
      case WIN_SCALE_MEDIUM:
      default:
	return SB_MEDIUM_MARGIN;
      case WIN_SCALE_LARGE:
	return SB_LARGE_MARGIN;
      case WIN_SCALE_EXTRALARGE:
	return SB_XLARGE_MARGIN;
    }
}

Pkg_private int
sb_marker_height(sb)
    Xv_scrollbar_info *sb;
{
	int height;

	if (sb->ginfo != (Graphics_info *)NULL) 
	  height = Vertsb_Endbox_Height(sb->ginfo);
	else
	  switch (sb->scale) {
		case WIN_SCALE_SMALL:
		  height = SB_SMALL_MARKER_HEIGHT;
		case WIN_SCALE_MEDIUM:
		default:
		  height = SB_MEDIUM_MARKER_HEIGHT;
		case WIN_SCALE_LARGE:
		  height = SB_LARGE_MARKER_HEIGHT;
		case WIN_SCALE_EXTRALARGE:
		  height = SB_XLARGE_MARKER_HEIGHT;
	  }
	return(height);
}

Pkg_private void
sb_rescale(sb, scale)
    Xv_scrollbar_info *sb;
    Frame_rescale_state scale;
{
    if (sb->scale != scale) {
	sb->scale = scale;
	if (sb->size == SCROLLBAR_ABBREVIATED) {
	    sb_abbreviated(sb);
	} else {
	    sb_full_size(sb);
	}
    }
}

/* 
 * scrollbar_line_backward_rect - compute the normalized backward scroll rect
 */
Pkg_private void
scrollbar_line_backward_rect(sb, r)
	Xv_scrollbar_info *sb;
	Rect              *r;	/* RETURN VALUE */
{
	r->r_left =   sb->elevator_rect.r_left;
	r->r_width =  sb->elevator_rect.r_width;
	r->r_top =    sb->elevator_rect.r_top;
	if (sb->size != SCROLLBAR_ABBREVIATED)
	  r->r_height = sb->elevator_rect.r_height / 3;
	else
	  r->r_height = sb->elevator_rect.r_height / 2;
}

/* 
 * scrollbar_absolute_rect - compute the normalized absolute scroll rect
 */
Pkg_private void
scrollbar_absolute_rect(sb, r)
	Xv_scrollbar_info *sb;
	Rect              *r;	/* RETURN VALUE */
{
	r->r_left =   sb->elevator_rect.r_left;
	r->r_width =  sb->elevator_rect.r_width;
	r->r_top =    sb->elevator_rect.r_top + (sb->elevator_rect.r_height / 3);
	r->r_height = sb->elevator_rect.r_height / 3;
}

/* 
 * scrollbar_line_forward_rect - compute the normalized forward scroll rect
 */
Pkg_private void
scrollbar_line_forward_rect(sb, r)
	Xv_scrollbar_info *sb;
	Rect              *r;	/* RETURN VALUE */
{
	r->r_left = sb->elevator_rect.r_left;
	r->r_width = sb->elevator_rect.r_width;
	if (sb->size != SCROLLBAR_ABBREVIATED) {
		r->r_top = sb->elevator_rect.r_top + (sb->elevator_rect.r_height / 3) * 2;
		r->r_height = sb->elevator_rect.r_height / 3;
	} else {
		r->r_top = sb->elevator_rect.r_top + (sb->elevator_rect.r_height / 2);
		r->r_height = sb->elevator_rect.r_height / 2;
	}
}	

Xv_private int
scrollbar_width_for_scale(scale)
    Frame_rescale_state scale;
{
    switch (scale) {
      case WIN_SCALE_SMALL:
	return SCROLLBAR_SMALL_THICKNESS;
      case WIN_SCALE_MEDIUM:
	return SCROLLBAR_MEDIUM_THICKNESS;
      case WIN_SCALE_LARGE:
	return SCROLLBAR_LARGE_THICKNESS;
      case WIN_SCALE_EXTRALARGE:
	return SCROLLBAR_XLARGE_THICKNESS;
      default:
	return SCROLLBAR_MEDIUM_THICKNESS;
    }
}

Pkg_private int
sb_elevator_height(sb, size)
    Xv_scrollbar_info *sb;
    Scrollbar_size size;
{
    int h = ScrollbarElevator_Height(sb->ginfo);

    if (size != SCROLLBAR_FULL_SIZE)
      h = ((h - 2) / 3) * 2 + 3;
    return (h);
}

Xv_private int
scrollbar_minimum_size(sb_public)
Scrollbar sb_public;
{
    Xv_scrollbar_info *sb = SCROLLBAR_PRIVATE(sb_public);

    return sb_elevator_height(sb, SCROLLBAR_MINIMUM);
}

