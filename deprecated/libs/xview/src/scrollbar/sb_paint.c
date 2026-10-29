#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sb_paint.c 1.67 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Module:	sb_paint.c
 * 
 * Description:
 * 
 * Maps events into actions
 * 
 */

/*
 * Include files:
 */

#include <xview_private/sb_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/rectlist.h>
#include <xview/window.h>
#include <xview/svrimage.h>
#include <X11/Xlib.h>

/*
 * Declaration of Functions Defined in This File (in order):
 */

Xv_public void  scrollbar_paint();

Pkg_private void scrollbar_clear_damage();
Pkg_private void scrollbar_paint_elevator();
Pkg_private void scrollbar_paint_elevator_move();
Pkg_private void scrollbar_invert_region();
static void     scrollbar_proportional_indicator();
static void     scrollbar_paint_anchor();

/******************************************************************/


Xv_public void
scrollbar_paint(sb_public)
    Scrollbar sb_public;
{
    Xv_scrollbar_info *sb = SCROLLBAR_PRIVATE(sb_public);

    /* Calculate correct position of elevator.  Paint cable and elevator. */
    sb->last_state = 0;
    scrollbar_position_elevator(sb, TRUE, SCROLLBAR_NONE);

    if (sb->size != SCROLLBAR_MINIMUM) {
	/* Paint top cable anchor */
	scrollbar_paint_anchor(sb, &sb->top_anchor_rect, sb->top_anchor_inverted);
    
	/* Paint bottom cable anchor */
	scrollbar_paint_anchor(sb, &sb->bottom_anchor_rect, sb->bottom_anchor_inverted);
    }
}

Pkg_private void
scrollbar_paint_elevator(sb)
    Xv_scrollbar_info *sb;
{
    scrollbar_paint_elevator_move(sb, sb->elevator_rect.r_top);
}

Pkg_private void
scrollbar_paint_elevator_move(sb, new_pos)
    Xv_scrollbar_info *sb;
    int new_pos;
{
    int x, y, old_pos;
    int prop_pos, prop_length, state;
    int scroll_backward, scroll_forward;
    
    state = (sb->size == SCROLLBAR_FULL_SIZE) ? 
      (OLGX_NORMAL | OLGX_ERASE | OLGX_UPDATE) : OLGX_ABBREV;
    state |= sb->elevator_state | OLGX_ERASE;
    old_pos = sb->elevator_rect.r_top;
    
    if (sb->direction == SCROLLBAR_VERTICAL) {
	state |= OLGX_VERTICAL;
	x = sb->elevator_rect.r_left;
	if (sb->size != SCROLLBAR_FULL_SIZE)
	  y = sb->elevator_rect.r_top;
	else
	  y = 0;

    } else {
	state |= OLGX_HORIZONTAL;
	if (sb->size != SCROLLBAR_FULL_SIZE)
	  x = sb->elevator_rect.r_top;
	else
	  x = 0;
	y = sb->elevator_rect.r_left;
    }
    if (!(state & OLGX_INACTIVE) &&
	(!(state & (OLGX_SCROLL_BACKWARD | 
		    OLGX_SCROLL_FORWARD | 
		    OLGX_SCROLL_ABSOLUTE)))) {
	scroll_forward = (sb->view_start < Max_offset(sb));
	scroll_backward = (sb->view_start > 0);
	if (!scroll_forward && !scroll_backward)
	  state |= OLGX_INACTIVE;
	else if (!scroll_backward)
	  state |= OLGX_SCROLL_NO_BACKWARD;
	else if (!scroll_forward)
	  state |= OLGX_SCROLL_NO_FORWARD;
    }
    scrollbar_proportional_indicator(sb, new_pos, &prop_pos, &prop_length);
    
    if ((sb->last_pos != new_pos) || 
	(sb->last_prop_pos != prop_pos) ||
	(sb->last_prop_length != prop_length) ||
	(sb->last_state != state)) {
	olgx_draw_scrollbar(sb->ginfo, sb->window, x, y, sb->length,
			    new_pos, old_pos, prop_pos, prop_length, state);
	sb->last_pos = new_pos;
	sb->last_prop_pos = prop_pos;
	sb->last_prop_length = prop_length;
	sb->last_state = state;
	sb->elevator_rect.r_top = new_pos;
    }
}	    

Pkg_private void
scrollbar_invert_region(sb, motion)
    Xv_scrollbar_info *sb;
    Scroll_motion   motion;
{
    int		    state;
    
    switch (motion) {
      case SCROLLBAR_TO_START:
	state = sb->top_anchor_inverted = !(sb->top_anchor_inverted);
	scrollbar_paint_anchor(sb, &sb->top_anchor_rect, state);
	break;
      case SCROLLBAR_TO_END:
	state = sb->bottom_anchor_inverted = !(sb->bottom_anchor_inverted);
	scrollbar_paint_anchor(sb, &sb->bottom_anchor_rect, state);
	break;
      case SCROLLBAR_LINE_FORWARD:
	sb->elevator_state = (sb->elevator_state == OLGX_SCROLL_FORWARD) ? 0 :
	  OLGX_SCROLL_FORWARD;
	scrollbar_paint_elevator(sb);
	break;
      case SCROLLBAR_LINE_BACKWARD:
	sb->elevator_state = (sb->elevator_state == OLGX_SCROLL_BACKWARD) ? 0 :
	  OLGX_SCROLL_BACKWARD;
	scrollbar_paint_elevator(sb);
	break;
      case SCROLLBAR_ABSOLUTE:
	sb->elevator_state = (sb->elevator_state == OLGX_SCROLL_ABSOLUTE) ? 0 :
	  OLGX_SCROLL_ABSOLUTE;
	scrollbar_paint_elevator(sb);
	break;
    }
}

static void
scrollbar_paint_anchor(sb, r, invoked)
    Xv_scrollbar_info *sb;
    Rect              *r;
    int               invoked;
{
    sb_normalize_rect(sb, r);
    olgx_draw_box(sb->ginfo, sb->window,
		  r->r_left, r->r_top, r->r_width, r->r_height,
		  invoked | OLGX_ERASE, TRUE);
    sb_normalize_rect(sb, r);
}


/*
 * scrollbar_proportional_indicator - return the starting position, and length 
 *   of the scrollbar's cable proportional indicator.
 */
static void
scrollbar_proportional_indicator(sb, elev_pos, position, length)
Xv_scrollbar_info	*sb;
int			elev_pos;	/* the top of the elevator */
int 			*position;	/* RETURN VALUE */
int			*length;	/* RETURN VALUE */
{
    int cable_size = scrollbar_available_cable(sb) - sb->cable_start;
    
    if (sb->size != SCROLLBAR_FULL_SIZE) {
	*position = 0;
	*length = 0;
    } else if ((sb->object_length == 0) || (sb->object_length <= sb->view_length)) {
	*position = sb->cable_start;
	*length = sb->cable_height - 2;
    } else {
	*length = sb->cable_height * sb->view_length / sb->object_length;
	if (*length > sb->cable_height - 2)
	  *length = sb->cable_height - 2;
	
	if (*length > sb->elevator_rect.r_height &&
	    elev_pos > sb->cable_start && cable_size > 0)
	  *position = elev_pos - (elev_pos - sb->cable_start) * 
	    (*length - sb->elevator_rect.r_height) / cable_size;
	else
	  *position = elev_pos;
	
	/* 
	 * If the prop indicator would be covered by the elevator,
	 * make the indicator a 3 point mark above (or below) the
	 * elevator
	 */
	if (*length < sb->elevator_rect.r_height) 
	  if ((elev_pos - 4) >= sb->cable_start) {
	      *position = elev_pos - 4;
	      *length = 2 + sb->elevator_rect.r_height + 1;
	  } else if ((elev_pos + sb->elevator_rect.r_height + 1) <= 
		     (sb->cable_start + sb->cable_height - 1)) {
	      *position = elev_pos;
	      *length = 2 + sb->elevator_rect.r_height;
	  } else
	    *length = 0;
    }
}
