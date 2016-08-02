#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sb_event.c 1.67 91/05/23";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Module:	sb_event.c
 * 
 * Description:
 * 
 * Maps events into actions
 * 
 */


/*
 * Include files:
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <xview_private/sb_impl.h>
#include <xview/canvas.h>
#include <xview/frame.h>
#include <xview/notify.h>
#include <xview/openwin.h>
#include <xview/fullscreen.h>
#include <xview/rectlist.h>
#include <xview/screen.h>
#include <xview/server.h>
#include <xview/win_notify.h>
#include <xview_private/draw_impl.h>

Xv_private char xv_iso_cancel;
Xv_private char xv_iso_default_action;
Xv_private char xv_iso_input_focus_help;
Xv_private char xv_iso_next_element;
Xv_private char xv_iso_select;

Xv_private Xv_object xv_input_readevent();
int	ignore_drag_count;
int	ignore_drag_max;

/*
 * Definitions
 */
#define SPLIT_VIEW_MENU_ITEM_NBR	5
#define JOIN_VIEWS_MENU_ITEM_NBR	6
#define BORDER_WIDTH			1

/*
 * Declaration of Functions Defined in This File (in order):
 */

Pkg_private Notify_value scrollbar_handle_event();
Pkg_private void scrollbar_line_to_top();
Pkg_private void scrollbar_top_to_line();
Pkg_private void scrollbar_split_view_from_menu();
Pkg_private void scrollbar_join_view_from_menu();
Pkg_private void scrollbar_last_position();
Pkg_private int scrollbar_left_mouse_up();
Pkg_private Menu scrollbar_gen_menu();

static Notify_value scrollbar_timed_out();
static int      scrollbar_handle_elevator_event();
static void     scrollbar_handle_timed_page_event();
static void     scrollbar_handle_timed_line_event();
static void     scrollbar_position_mouse();
static Scroll_motion scrollbar_translate_scrollbar_event_to_motion();
static int      scrollbar_translate_to_elevator_event();
static void     scrollbar_timer_start();
static void     scrollbar_timer_stop();
static void     scrollbar_set_intransit();
static int      scrollbar_preview_split();
static int      scrollbar_invoke_split();
static void     scrollbar_destroy_split();
static int      scrollbar_multiclick();
static int	scrollbar_find_view_nbr();

/*
 * additions for the delay time between repeat action events
 */
#define	SB_SET_FOR_LINE		1
#define	SB_SET_FOR_PAGE		2

static int      sb_delay_time;		/* delay before first action repeat 	 */
static int      sb_page_interval;	/* delay between paging repeats 	 */
static int      sb_line_interval;	/* delay between line   repeats 	 */

/******************************************************************/

Pkg_private     Notify_value
scrollbar_handle_event(sb_public, event, arg, type)
    Xv_Window       sb_public;
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    int               do_split;
    Xv_Window	      focus_win;
    Frame	      frame;
    char	      *help_key;
    Scroll_motion     motion;
    Xv_window	      openwin;
    Scrollbar	      other_sb;  /* other Scrollbar attached to this openwin */
    Xv_Window	      paint_window;
    Rect	      r;
    Xv_scrollbar_info *sb = SCROLLBAR_PRIVATE(sb_public);
    Pkg_private void  sb_minimum();
    Pkg_private void  sb_abbreviated();
    Pkg_private void  sb_full_size();
    Event             split_event;
    int		      view_nbr;
    Xv_Window	      view_window;

    if (event_action(event) == xv_iso_input_focus_help)
	event_set_action(event, ACTION_INPUT_FOCUS_HELP);
    else if (event_action(event) == xv_iso_next_element) {
	if (event_shift_is_down(event))
	    event_set_action(event, ACTION_PREVIOUS_ELEMENT);
	else
	    event_set_action(event, ACTION_NEXT_ELEMENT);
    }

    switch (event_action(event)) {
      case ACTION_RESCALE:
	sb_rescale(sb, (Frame_rescale_state) arg);
	(void)notify_next_event_func(sb_public, (Notify_event)event, arg, type);
	break;

      case WIN_RESIZE:
	sb_resize(sb);
	break;

      case WIN_REPAINT:
	scrollbar_paint(sb_public);
	sb->painted = TRUE;
	break;

      case ACTION_HELP:
      case ACTION_MORE_HELP:
      case ACTION_TEXT_HELP:
      case ACTION_MORE_TEXT_HELP:
      case ACTION_INPUT_FOCUS_HELP:
	if (event_is_down(event)) {
	    motion = scrollbar_translate_scrollbar_event_to_motion(sb, event);
	    switch (motion) {
	      case SCROLLBAR_TO_END:
		help_key = "xview:scrollbarBottom";
		break;
	      case SCROLLBAR_TO_START:
		help_key = "xview:scrollbarTop";
		break;
	      case SCROLLBAR_PAGE_FORWARD:
		help_key = "xview:scrollbarPageForward";
		break;
	      case SCROLLBAR_PAGE_BACKWARD:
		help_key = "xview:scrollbarPageBackward";
		break;
	      case SCROLLBAR_LINE_FORWARD:
		help_key = "xview:scrollbarLineForward";
		break;
	      case SCROLLBAR_LINE_BACKWARD:
		help_key = "xview:scrollbarLineBackward";
		break;
	      case SCROLLBAR_ABSOLUTE:
		help_key = "xview:scrollbarDrag";
		break;
	      case SCROLLBAR_NONE:
		help_key = "xview:scrollbarPageBackward";
		break;
	    }
	    xv_help_show(sb_public, help_key, event);
	}
	break;

      case KBD_USE:
	frame = xv_get(sb_public, WIN_FRAME);
	focus_win = xv_get(frame, FRAME_FOCUS_WIN);
	xv_set(frame,
	       FRAME_FOCUS_DIRECTION,
		   SB_VERTICAL(sb) ? FRAME_FOCUS_RIGHT : FRAME_FOCUS_UP,
	       NULL);
	xv_set(focus_win,
	       WIN_PARENT, sb_public,
	       XV_X, 0,
	       XV_Y, 0,
	       NULL);
	frame_kbd_use(frame, xv_get(sb_public, XV_OWNER), sb_public);
        xv_set(focus_win, XV_SHOW, TRUE, NULL);
	break;

      case KBD_DONE:
	frame = xv_get(sb_public, WIN_FRAME);
	focus_win = xv_get(frame, FRAME_FOCUS_WIN);
	xv_set(focus_win,
	       XV_SHOW, FALSE,
	       NULL);
	frame_kbd_done(frame, xv_get(sb_public, XV_OWNER));
	break;

      case ACTION_NEXT_PANE:
	/* Order of precedence:
	 *	Scrollbar -> next paint or view window's first element
	 *	Scrollbar -> next pane
	 */
        if (event_is_down(event)) {
	    openwin = xv_get(sb_public, XV_OWNER);
	    if (openwin) {
		view_nbr = scrollbar_find_view_nbr(sb, openwin);
		/* Scrollbar -> next paint or view window's first element */
		if (xv_get(openwin, XV_IS_SUBTYPE_OF, CANVAS)) {
		    /* Next paint window receives input focus.  Find the paint
		     * window that corresponds to the view window.
		     */
		    paint_window = xv_get(openwin,
					  CANVAS_NTH_PAINT_WINDOW, view_nbr+1);
		    if (paint_window) {
			xv_set(paint_window, WIN_SET_FOCUS, NULL);
			xv_set(openwin, XV_FOCUS_ELEMENT, 0, NULL);
		    } else
			openwin = XV_ZERO;
		} else {
		    /* Next view window receives input focus */
		    view_window = xv_get(openwin, OPENWIN_NTH_VIEW, view_nbr+1);
		    if (view_window)
			xv_set(view_window, WIN_SET_FOCUS, NULL);
		    else
			openwin = XV_ZERO;
		}
	    }
	    if (!openwin) {
		frame = xv_get(sb_public, WIN_FRAME);
		xv_set(frame, FRAME_NEXT_PANE, NULL);
	    }
	}
	break;

      case ACTION_PREVIOUS_PANE:
	/* Order of precedence:
	 *	Scrollbar -> scrollbar's paint or view window's last element
	 *	Scrollbar -> previous pane
	 */
        if (event_is_down(event)) {
	    openwin = xv_get(sb_public, XV_OWNER);
	    if (openwin) {
		view_nbr = scrollbar_find_view_nbr(sb, openwin);
		/* Scrollbar -> scrollbar's paint or view window */
		if (xv_get(openwin, XV_IS_SUBTYPE_OF, CANVAS)) {
		    /* Paint window receives input focus.  Find the paint
		     * window that corresponds to the view window.  Set the
		     * focus to the last element in the paint window.
		     */
		    xv_set(xv_get(openwin, CANVAS_NTH_PAINT_WINDOW, view_nbr),
			   WIN_SET_FOCUS,
			   NULL);
		    xv_set(openwin, XV_FOCUS_ELEMENT, -1, NULL);
		} else
		    /* View window receives input focus */
		    xv_set(sb->managee, WIN_SET_FOCUS, NULL);
	    } else {
		frame = xv_get(sb_public, WIN_FRAME);
		xv_set(frame, FRAME_PREVIOUS_PANE, NULL);
	    }
	}
	break;

      case ACTION_NEXT_ELEMENT:
	/* Order of precedence:
	 *	Vertical scrollbar -> Horizontal scrollbar
	 *	Any scrollbar -> next paint or view window's first element
	 *	Any scrollbar -> next pane
	 */
        if (event_is_down(event)) {
	    openwin = xv_get(sb_public, XV_OWNER);
	    if (openwin) {
		view_nbr = scrollbar_find_view_nbr(sb, openwin);
		if (SB_VERTICAL(sb)) {
		    view_window = xv_get(openwin, OPENWIN_NTH_VIEW, view_nbr);
		    other_sb = xv_get(openwin, OPENWIN_HORIZONTAL_SCROLLBAR,
			view_window);
		    if (other_sb) {
			xv_set(other_sb, WIN_SET_FOCUS, NULL);
			break;
		    }
		}
		/* Any scrollbar -> next paint or view window's first element */
		if (xv_get(openwin, XV_IS_SUBTYPE_OF, CANVAS)) {
		    /* Next paint window receives input focus.  Find the paint
		     * window that corresponds to the view window.
		     */
		    paint_window = xv_get(openwin,
					  CANVAS_NTH_PAINT_WINDOW, view_nbr+1);
		    if (paint_window) {
			xv_set(paint_window, WIN_SET_FOCUS, NULL);
			xv_set(openwin, XV_FOCUS_ELEMENT, 0, NULL);
		    } else
			openwin = XV_ZERO;
		} else {
		    /* Next view window receives input focus */
		    view_window = xv_get(openwin, OPENWIN_NTH_VIEW, view_nbr+1);
		    if (view_window)
			xv_set(view_window, WIN_SET_FOCUS, NULL);
		    else
			openwin = XV_ZERO;
		}
	    }
	    if (!openwin) {
		frame = xv_get(sb_public, WIN_FRAME);
		xv_set(frame, FRAME_NEXT_PANE, NULL);
	    }
	}
	break;

      case ACTION_PREVIOUS_ELEMENT:
	/* Order of precedence:
	 *	Horizontal scrollbar -> Vertical scrollbar
	 *	Any scrollbar -> scrollbar's paint or view window
	 *	Any scrollbar -> last element in previous pane
	 */
        if (event_is_down(event))  {
	    openwin = xv_get(sb_public, XV_OWNER);
	    if (openwin) {
		view_nbr = scrollbar_find_view_nbr(sb, openwin);
		if (!SB_VERTICAL(sb)) {
		    view_window = xv_get(openwin, OPENWIN_NTH_VIEW, view_nbr);
		    other_sb = xv_get(openwin, OPENWIN_VERTICAL_SCROLLBAR,
			view_window);
		    if (other_sb) {
			/* Horizontal scrollbar -> Vertical scrollbar */
			xv_set(other_sb, WIN_SET_FOCUS, NULL);
			break;
		    }
		}
		/* Any scrollbar -> scrollbar's paint or view window */
		if (xv_get(openwin, XV_IS_SUBTYPE_OF, CANVAS)) {
		    /* Paint window receives input focus.  Find the paint
		     * window that corresponds to the view window.  Set the
		     * focus to the last element in the paint window.
		     */
		    xv_set(xv_get(openwin, CANVAS_NTH_PAINT_WINDOW, view_nbr),
			   WIN_SET_FOCUS,
			   NULL);
		    xv_set(openwin, XV_FOCUS_ELEMENT, -1, NULL);
		} else
		    /* View window receives input focus */
		    xv_set(sb->managee, WIN_SET_FOCUS, NULL);
	    } else {
		/* Any scrollbar -> last element in previous pane */
		frame = xv_get(sb_public, WIN_FRAME);
		xv_set(frame, FRAME_PREVIOUS_ELEMENT, NULL);
	    }
	}
	break;

      case ACTION_UP:
	if (event_is_up(event) || !SB_VERTICAL(sb))
	    break;
	scrollbar_scroll(sb, 0, SCROLLBAR_LINE_BACKWARD);
	break;

      case ACTION_DOWN:
	if (event_is_up(event) || !SB_VERTICAL(sb))
	    break;
	scrollbar_scroll(sb, 0, SCROLLBAR_LINE_FORWARD);
	break;

      case ACTION_LEFT:
	if (event_is_up(event) || SB_VERTICAL(sb))
	    break;
	scrollbar_scroll(sb, 0, SCROLLBAR_LINE_BACKWARD);
	break;

      case ACTION_RIGHT:
	if (event_is_up(event) || SB_VERTICAL(sb))
	    break;
	scrollbar_scroll(sb, 0, SCROLLBAR_LINE_FORWARD);
	break;

      case ACTION_JUMP_UP:
	if (event_is_up(event) || !SB_VERTICAL(sb))
	    break;
	scrollbar_scroll(sb, 0, SCROLLBAR_PAGE_BACKWARD);
	break;
    
      case ACTION_JUMP_DOWN:
	if (event_is_up(event) || !SB_VERTICAL(sb))
	    break;
	scrollbar_scroll(sb, 0, SCROLLBAR_PAGE_FORWARD);
	break;

      case ACTION_JUMP_LEFT:
	if (event_is_up(event) || SB_VERTICAL(sb))
	    break;
	scrollbar_scroll(sb, 0, SCROLLBAR_PAGE_BACKWARD);
	break;

      case ACTION_JUMP_RIGHT:
	if (event_is_up(event) || SB_VERTICAL(sb))
	    break;
	scrollbar_scroll(sb, 0, SCROLLBAR_PAGE_FORWARD);
	break;

      case ACTION_DATA_START:
	if (event_is_down(event))
	    scrollbar_scroll(sb, 0, SCROLLBAR_TO_START);
	break;

      case ACTION_DATA_END:
	if (event_is_down(event))
	    scrollbar_scroll(sb, 0, SCROLLBAR_TO_END);
	break;

      case ACTION_JUMP_MOUSE_TO_INPUT_FOCUS:
	xv_set(sb_public,
	       WIN_MOUSE_XY, 0, 0,
	       NULL);
	break;

      case ACTION_SELECT:
	if (sb->inactive)
	    break;
	if (event_is_down(event)) {
		motion = scrollbar_multiclick(sb, event) ? sb->transit_motion :
		  scrollbar_translate_scrollbar_event_to_motion(sb, event);
		 sb->last_motion = motion;
		switch (motion) {
		      case SCROLLBAR_TO_END:
		      case SCROLLBAR_TO_START:
			scrollbar_invert_region(sb, motion);
			break;

		      case SCROLLBAR_PAGE_FORWARD:
		      case SCROLLBAR_PAGE_BACKWARD:
			scrollbar_timer_start(SCROLLBAR_PUBLIC(sb), SB_SET_FOR_PAGE);
			break;
			
		      case SCROLLBAR_LINE_FORWARD:
		      case SCROLLBAR_LINE_BACKWARD:
		      case SCROLLBAR_ABSOLUTE:
			/* translate event to elevator space */
			scrollbar_translate_to_elevator_event(sb, event);
			scrollbar_handle_elevator_event(sb, event, motion);
			break;
			
		      default:
			break;
		}
		scrollbar_set_intransit(sb, motion, event);
	} else {
		switch (sb->transit_motion) {
		      case SCROLLBAR_TO_END:
		      case SCROLLBAR_TO_START:
			scrollbar_invert_region(sb, sb->transit_motion);
			(void) scrollbar_scroll(sb, 0, sb->transit_motion);
			break;

		      case SCROLLBAR_PAGE_FORWARD:
		      case SCROLLBAR_PAGE_BACKWARD:
			if (!sb->transit_occurred) {
				scrollbar_handle_timed_page_event(sb, sb->transit_motion);
			}
			break;
			
		      case SCROLLBAR_LINE_FORWARD:
		      case SCROLLBAR_LINE_BACKWARD:
		      case SCROLLBAR_ABSOLUTE:
			/* translate event to elevator space */
			scrollbar_translate_to_elevator_event(sb, event);
			scrollbar_handle_elevator_event(sb, event, sb->transit_motion);
			break;
			
		      default:
		break;
		}
		scrollbar_timer_stop(SCROLLBAR_PUBLIC(sb));
	}
        sb->last_select_time = event->ie_time;
	break;
	
      case ACTION_MENU:
	if (sb->inactive)
	  break;
	if (event_is_down(event)) {
		scrollbar_set_intransit(sb, SCROLLBAR_NONE, event);
		if (!sb->menu) {
			scrollbar_create_standard_menu(sb);
		}
		menu_show(sb->menu, SCROLLBAR_PUBLIC(sb), event, NULL);
	}
	break;
	
      case LOC_DRAG:
	if (sb->inactive)
	  break;
	if ((sb->transit_motion == SCROLLBAR_TO_END
	     || sb->transit_motion == SCROLLBAR_TO_START)
	    && sb->can_split) {
		/* when user pulls out of split bar start split */
		motion = scrollbar_translate_scrollbar_event_to_motion(sb, event);
		if (motion != sb->transit_motion) {
			r = *(Rect *) xv_get(sb->managee, WIN_RECT);
			do_split = scrollbar_preview_split(sb, event, &r, &split_event);
			/* split event may cause this sb to be destroy */
			/* therefore all painting must be done first */
			scrollbar_invert_region(sb, sb->transit_motion);
			if (do_split == XV_OK) {
				scrollbar_invoke_split(sb, &split_event);
			}
			scrollbar_set_intransit(sb, SCROLLBAR_NONE, event);
		}
	} else if (sb->transit_motion == SCROLLBAR_ABSOLUTE) {
		/* translate event to elevator space */
		scrollbar_translate_to_elevator_event(sb, event);
		scrollbar_handle_elevator_event(sb, event, sb->transit_motion);
	} 
	break;
	
      default:
	break;
    }
    
    return NOTIFY_DONE;
}


static int
scrollbar_handle_elevator_event(sb, event, motion)
    Xv_scrollbar_info *sb;
    Event          *event;
    Scroll_motion   motion;
{
    int             y, x, transit_y;

    switch (event_action(event)) {
      case ACTION_SELECT:
	if (event_is_down(event)) {
	    y = SB_VERTICAL(sb) ? event_y(event) : event_x(event);
	    scrollbar_invert_region(sb, motion);
	    switch (motion) {
	      case SCROLLBAR_LINE_FORWARD:
	      case SCROLLBAR_LINE_BACKWARD:
		scrollbar_timer_start(SCROLLBAR_PUBLIC(sb), SB_SET_FOR_LINE);
		break;
	      case SCROLLBAR_ABSOLUTE:
		if(sb->drag_repaint_percent)
		    ignore_drag_max = 100/sb->drag_repaint_percent; 
		ignore_drag_count = 1;
		break;
	      default:
		break;
	    }
	} else {
	    scrollbar_invert_region(sb, sb->transit_motion);
	    switch (sb->transit_motion) {

	      case SCROLLBAR_LINE_FORWARD:
	      case SCROLLBAR_LINE_BACKWARD:
		scrollbar_timer_stop(SCROLLBAR_PUBLIC(sb));
		if (!sb->transit_occurred) {
		    scrollbar_handle_timed_line_event(sb, sb->transit_motion);
		}
		break;

	      case SCROLLBAR_ABSOLUTE:
		/* 
		 * compute absolute position by top of elevator
		 * in the coordinate space of the cable
		 */
		x = scrollbar_available_cable(sb);
		y = sb->elevator_rect.r_top - sb->cable_start;
		if (y < 0) {
		    y = 0;
		} else if (y > x) {
		    y = x;
		}
		(void) scrollbar_scroll(sb, y, sb->transit_motion);
		break;

	      default:
		break;
	    }
	    scrollbar_timer_stop(SCROLLBAR_PUBLIC(sb));
	}
	break;
      case LOC_DRAG:
	if (sb->transit_motion == SCROLLBAR_ABSOLUTE) {
		if (SB_VERTICAL(sb)) {
			y = event_y(event);
			transit_y = event_y(&sb->transit_event);
		} else {
			y = event_x(event);
			transit_y = event_x(&sb->transit_event);
		}
		if (transit_y != y) {
			/* 
			 * compute absolute position by top of elevator
			 * in the coordinate space of the cable
			 */
			x = scrollbar_available_cable(sb);
			y = sb->elevator_rect.r_top + (y - transit_y) - sb->cable_start;
			if (y < 0) {
				y = 0;
			} else if (y > x) {
				y = x;
			}
			scrollbar_absolute_position_elevator(sb, y);
			if((ignore_drag_count > ignore_drag_max) 
			   && sb->drag_repaint_percent){
			  (void) scrollbar_scroll(sb, y, sb->transit_motion);
			  ignore_drag_count = 1;
		      }
			ignore_drag_count++;
		}
	}
	break;
      default:
	break;
    }

    return XV_OK;
}

/*ARGSUSED*/
static          Notify_value
scrollbar_timed_out(client, which)
    Notify_client   client;
    int             which;
{
    Xv_scrollbar_info *sb = SCROLLBAR_PRIVATE(client);

    /* if the event has happened, but the user is no longer physically */
    /* pressing the Left Mouse Button, we skip the action. This is a   */
    /* method to use the current programmatic paradigm w/o processing  */
    /* past events/timeouts that have triggered but are no longer valid */
    /* jcb -- 5/3/89 */
    if (scrollbar_left_mouse_up(sb))
	return NOTIFY_DONE;


    /* tell myself to repeat a scroll */
    switch (sb->transit_motion) {

      case SCROLLBAR_LINE_BACKWARD:
	if (sb->view_start) {
	    scrollbar_handle_timed_line_event(sb, sb->transit_motion);
	    sb->transit_occurred = TRUE;
	}
	break;

      case SCROLLBAR_LINE_FORWARD:
	scrollbar_handle_timed_line_event(sb, sb->transit_motion);
	sb->transit_occurred = TRUE;
	break;

      case SCROLLBAR_PAGE_FORWARD:
	scrollbar_handle_timed_page_event(sb, sb->transit_motion);
	sb->transit_occurred = TRUE;
	break;

      case SCROLLBAR_PAGE_BACKWARD:
	if (sb->view_start) {
	    scrollbar_handle_timed_page_event(sb, sb->transit_motion);
	    sb->transit_occurred = TRUE;
	}
	break;

      default:
	break;
    }

    return NOTIFY_DONE;
}


/*ARGSUSED*/
Pkg_private void
scrollbar_line_to_top(menu, item)
    Menu            menu;
    Menu_item       item;
{
    Xv_scrollbar_info *sb;
    int    pos;

    sb = (Xv_scrollbar_info *) xv_get(menu, XV_KEY_DATA, sb_context_key, NULL);

    /* look at transit event to see where the mouse button went down */
    pos = SB_VERTICAL(sb) ? event_y(&sb->transit_event)	: event_x(&sb->transit_event);
    scrollbar_scroll(sb, pos, SCROLLBAR_POINT_TO_MIN);
}

/*ARGSUSED*/
Pkg_private void
scrollbar_top_to_line(menu, item)
    Menu            menu;
    Menu_item       item;
{
    Xv_scrollbar_info *sb;
    int    pos;

    sb = (Xv_scrollbar_info *) xv_get(menu, XV_KEY_DATA, sb_context_key);

    /* look at transit event to see where the mouse button went down */
    pos = SB_VERTICAL(sb) ? event_y(&sb->transit_event) : event_x(&sb->transit_event);
    scrollbar_scroll(sb, pos, SCROLLBAR_MIN_TO_POINT);
}


Pkg_private     Menu
scrollbar_gen_menu(menu, op)
    Menu            menu;
    Menu_generate   op;
{
    int             nbr_menu_items;
    int             nbr_views;
    Xv_scrollbar_info *sb;
    Xv_opaque       openwin;

    if (op == MENU_DISPLAY) {
	nbr_menu_items = (int) xv_get(menu, MENU_NITEMS);
	sb = (Xv_scrollbar_info *) xv_get(menu, XV_KEY_DATA, sb_context_key);
	if (sb->can_split) {
	    if (nbr_menu_items < SPLIT_VIEW_MENU_ITEM_NBR)
		xv_set(menu,
		       MENU_APPEND_ITEM,
		     xv_get(menu, XV_KEY_DATA, sb_split_view_menu_item_key, NULL),
		       NULL);
	    openwin = xv_get(SCROLLBAR_PUBLIC(sb), XV_OWNER);
	    nbr_views = (int) xv_get(openwin, OPENWIN_NVIEWS);
	    if (nbr_views > 1 && nbr_menu_items < JOIN_VIEWS_MENU_ITEM_NBR)
		xv_set(menu,
		       MENU_APPEND_ITEM,
		       xv_get(menu, XV_KEY_DATA, sb_join_view_menu_item_key, NULL),
		       NULL);
	    else if (nbr_views == 1 &&
		     nbr_menu_items == JOIN_VIEWS_MENU_ITEM_NBR)
		xv_set(menu, MENU_REMOVE, JOIN_VIEWS_MENU_ITEM_NBR, NULL);
	} else {
	    if (nbr_menu_items == JOIN_VIEWS_MENU_ITEM_NBR)
		xv_set(menu, MENU_REMOVE, JOIN_VIEWS_MENU_ITEM_NBR, NULL);
	    if (nbr_menu_items >= SPLIT_VIEW_MENU_ITEM_NBR)
		xv_set(menu, MENU_REMOVE, SPLIT_VIEW_MENU_ITEM_NBR, NULL);
	}
    }
    return (menu);
}


/*ARGSUSED*/
Pkg_private void
scrollbar_split_view_from_menu(menu, item)
    Menu            menu;
    Menu_item       item;
{
    Event	    last_menu_event;
    int		    do_split;
    Rect	    managee_rect;
    Xv_scrollbar_info *sb;
    Event	    split_event;

    sb = (Xv_scrollbar_info *) xv_get(menu, XV_KEY_DATA, sb_context_key);
    last_menu_event = *(Event *) xv_get(menu, MENU_LAST_EVENT);
    if (event_is_button(&last_menu_event))
	scrollbar_invoke_split(sb, (Event *) xv_get(menu, MENU_FIRST_EVENT));
    else {
	event_set_x(&last_menu_event, BORDER_WIDTH);
	event_set_y(&last_menu_event, BORDER_WIDTH);
	managee_rect = *(Rect *) xv_get(sb->managee, WIN_RECT);
	do_split = scrollbar_preview_split(sb, &last_menu_event, &managee_rect,
					   &split_event);
	if (do_split == XV_OK)
	    scrollbar_invoke_split(sb, &split_event);
    }
}


/*ARGSUSED*/
Pkg_private void
scrollbar_join_view_from_menu(menu, item)
    Menu            menu;
    Menu_item       item;
{
    scrollbar_destroy_split(
	   (Xv_scrollbar_info *) xv_get(menu, XV_KEY_DATA, sb_context_key));
}


/*ARGSUSED*/
Pkg_private void
scrollbar_last_position(menu, item)
    Menu            menu;
    Menu_item       item;
{
    Xv_scrollbar_info *sb;

    sb = (Xv_scrollbar_info *) xv_get(menu, XV_KEY_DATA, sb_context_key);

    scrollbar_scroll_to_offset(sb, sb->last_view_start);
}

static void
scrollbar_handle_timed_page_event(sb, motion)
    Xv_scrollbar_info *sb;
    Scroll_motion   motion;
{
	int             x, y, new_y;

    if ((scrollbar_scroll(sb, 0, motion) != SCROLLBAR_POSITION_UNCHANGED) &&
	(sb->jump_pointer)) {
	    /* adjust the mouse if necessary */
	    if (SB_VERTICAL(sb)) {
		    x = event_x(&sb->transit_event);
		    y = event_y(&sb->transit_event);
	    } else {
		    x = event_y(&sb->transit_event);
		    y = event_x(&sb->transit_event);
	    }		    
	    
	    new_y = y;
	    if (sb->transit_motion == SCROLLBAR_PAGE_FORWARD && 
		(y <= rect_bottom(&sb->elevator_rect)))
	      new_y = rect_bottom(&sb->elevator_rect) + 1;
	    else if (sb->transit_motion == SCROLLBAR_PAGE_BACKWARD && 
		     (y >= sb->elevator_rect.r_top))
	      new_y = sb->elevator_rect.r_top - 1;
		    
	    scrollbar_position_mouse(sb, x, new_y);
	    
	    if (new_y != y) 
	      if (SB_VERTICAL(sb))
		event_set_y(&sb->transit_event, y);
	      else
		event_set_x(&sb->transit_event, y);
    }
}


static void
scrollbar_handle_timed_line_event(sb, motion)
	Xv_scrollbar_info *sb;
	Scroll_motion      motion;
{
	int             x, y;
	Rect            active_region;

	if ((scrollbar_scroll(sb, 0, motion) != SCROLLBAR_POSITION_UNCHANGED) &&
	    (sb->jump_pointer)) {
		/* reposition mouse if necessary */
		if (motion == SCROLLBAR_LINE_FORWARD)
		  scrollbar_line_forward_rect(sb, &active_region);
		else 
		  scrollbar_line_backward_rect(sb, &active_region);

		x = active_region.r_left + (active_region.r_width / 2);
		y = active_region.r_top + (active_region.r_height / 2);
		scrollbar_position_mouse(sb, x, y);
	}
}

static void
scrollbar_position_mouse(sb, x, y)
	Xv_scrollbar_info *sb;
	int             x, y;
{
	Xv_Window 	win = SCROLLBAR_PUBLIC(sb);
	Rect           *mouse_rect;
	Rect           *sb_rect;
	
	mouse_rect = (Rect *)xv_get(win, WIN_MOUSE_XY, NULL);
	sb_rect = (Rect *)xv_get(win, XV_RECT, NULL);

	/* move mouse if it is still in the scrollbar window */
	if (mouse_rect->r_left >= 0 &&
	    mouse_rect->r_left < sb_rect->r_width &&
	    mouse_rect->r_top >= 0 &&
	    mouse_rect->r_top < sb_rect->r_height)
	  if (SB_VERTICAL(sb))
	    xv_set(win, WIN_MOUSE_XY, x, y, NULL);
	  else
	    xv_set(win, WIN_MOUSE_XY, y, x, NULL);
}

static          Scroll_motion
scrollbar_translate_scrollbar_event_to_motion(sb, event)
    Xv_scrollbar_info *sb;
    Event          *event;
{
    int             anchor_height = sb_marker_height(sb);
    int             elevator_start;
    int             elevator_end;
    int		    region_height;
    int             pos;

    pos = SB_VERTICAL(sb) ? event_y(event) : event_x(event);

    elevator_start = sb->elevator_rect.r_top;
    elevator_end = elevator_start + sb->elevator_rect.r_height - 1;
    region_height = sb->elevator_rect.r_height;
    region_height /= (sb->size != SCROLLBAR_FULL_SIZE) ? 2 : 3;

    if (sb->size == SCROLLBAR_MINIMUM)
      if ((pos < elevator_start) || (pos > elevator_end))
	return SCROLLBAR_NONE;
      else if (pos < (elevator_start + region_height))
	return SCROLLBAR_LINE_BACKWARD;
      else 
	return SCROLLBAR_LINE_FORWARD;
    else { 
	/* Check for top anchor */

	if (pos <= rect_bottom(&sb->top_anchor_rect))
	    return SCROLLBAR_TO_START;

	/* Check in between top anchor and elevator */
	else if ((sb->size == SCROLLBAR_FULL_SIZE) && (pos <= elevator_start))
	  if (elevator_start <= anchor_height + SCROLLBAR_CABLE_GAP)
	    /* already at the top */
	    return SCROLLBAR_NONE;
	  else
	    return SCROLLBAR_PAGE_BACKWARD;
	
	/* Check for line backward region of elevator */
	else if (pos <= (elevator_start + region_height))
	  return SCROLLBAR_LINE_BACKWARD;
	
	/* Check for absolute scroll region of elevator */
	else if ((sb->size == SCROLLBAR_FULL_SIZE) &&
		 (pos <= (elevator_start + region_height * 2)))
	  return SCROLLBAR_ABSOLUTE;
	
	/* Check for line forward region of elevator */
	else if (pos <= elevator_end)
	  return SCROLLBAR_LINE_FORWARD;
	
	/* Check in between elevator and bottom anchor */
	else if ((sb->size == SCROLLBAR_FULL_SIZE) && (pos <= (sb->length - anchor_height)))
	  if (elevator_end + SCROLLBAR_CABLE_GAP + 1 >= sb->length - anchor_height)
	    /* already at the bottom */
	    return SCROLLBAR_NONE;
	  else
	    return SCROLLBAR_PAGE_FORWARD;
    
	/* Check the bottom anchor */
	else if ((pos > sb->bottom_anchor_rect.r_top) &&
		 (pos <= rect_bottom(&sb->bottom_anchor_rect)))
	  return SCROLLBAR_TO_END;
	else
	  return SCROLLBAR_NONE;
    }
}

static void
scrollbar_timer_start(scrollbar, actiontype)
    Scrollbar       scrollbar;
    int             actiontype;
{
    struct itimerval timer;
    int             interval;

    /* Call timeout routine using pre-set times according to actiontype */
    if (actiontype == SB_SET_FOR_LINE)
	interval = sb_line_interval;
    else
	interval = sb_page_interval;

    timer.it_value.tv_usec = sb_delay_time * 1000;
    timer.it_value.tv_sec = 0;

    /* next time call in `interval' msec		 */
    timer.it_interval.tv_usec = interval * 1000;
    timer.it_interval.tv_sec = 0;

    (void) notify_set_itimer_func((Notify_client) scrollbar,
				  scrollbar_timed_out, ITIMER_REAL, &timer,
				  (struct itimerval *) NULL);
}

static void
scrollbar_timer_stop(scrollbar)
    Scrollbar       scrollbar;
{
    (void) notify_set_itimer_func((Notify_client) scrollbar,
				  scrollbar_timed_out,
				  ITIMER_REAL,
				  (struct itimerval *) NULL,
				  (struct itimerval *) NULL);
}

static void
scrollbar_set_intransit(sb, motion, event)
    Xv_scrollbar_info *sb;
    Scroll_motion   motion;
    Event          *event;
{
    sb->transit_motion = motion;
    sb->transit_event = *event;
    sb->transit_occurred = FALSE;
}

static int
scrollbar_translate_to_elevator_event(sb, event)
    Xv_scrollbar_info *sb;
    Event          *event;
{
    if (SB_VERTICAL(sb)) {
	    event_set_x(event, event_x(event) - sb_margin(sb));
	    event_set_y(event, event_y(event) - sb->elevator_rect.r_top);
    } else {
	    event_set_x(event, event_x(event) - sb->elevator_rect.r_top);
	    event_set_y(event, event_y(event) - sb_margin(sb));
    }		    
}

static int
scrollbar_preview_split(sb, event, r, completion_event)
    Xv_scrollbar_info *sb;
    Event          *event;
    Rect           *r;
    Event          *completion_event;
{
    Fullscreen      fs;
    Xv_Window         paint_window;
    Xv_Drawable_info *info;
    Display	     *display;
    XID		      pw_xid;
    GC		     *gc_list, gc;
    Rect              sb_r;
    int		      x, y;
    int               sb_from_x, sb_from_y, sb_to_x, sb_to_y;
    int               from_x, from_y, to_x, to_y;
    Scrollbar         scrollbar = SCROLLBAR_PUBLIC(sb);
    Inputmask         im;
    int		      i;
    int		      jump_delta;
    int		      update_feedback;
    int		      status;
    int		      done;
    
    /* Create the fullscreen paint window needed to draw the preview */
    fs = xv_create(xv_get(scrollbar, XV_ROOT), FULLSCREEN,
        FULLSCREEN_INPUT_WINDOW, scrollbar,
	FULLSCREEN_CURSOR_WINDOW, scrollbar,
	WIN_CONSUME_EVENTS, 
            WIN_MOUSE_BUTTONS, LOC_DRAG, WIN_UP_EVENTS, NULL,
	NULL);
    paint_window = (Xv_Window)xv_get(fs, FULLSCREEN_PAINT_WINDOW);
    DRAWABLE_INFO_MACRO(paint_window, info);
    display = xv_display(info);
    pw_xid = xv_xid(info);

    /* Get rubberband GC */
    gc_list = (GC *)xv_get(xv_screen(info), SCREEN_OLGC_LIST, paint_window);
    gc = gc_list[SCREEN_RUBBERBAND_GC];
    
    /*
     * Calculate the preview line coordinates in the scrollbar's
     * coordinate space
     */
    sb_r = *(Rect *)xv_get(scrollbar, WIN_RECT);
    if (sb->direction == SCROLLBAR_VERTICAL) {
	sb_from_y = sb_to_y = event_y(event);
	if (sb_r.r_left < r->r_left) {
	    /* left handed scrollbar */
	    sb_from_x = 0;
	    sb_to_x = sb_r.r_width + r->r_width;
	} else {
	    /* right handed scrollbar */
	    sb_from_x = r->r_left - sb_r.r_left;
	    sb_to_x = sb_r.r_width;
	}
	jump_delta = sb_r.r_height / 10;
    } else {
	/* horizontal scrollbar */
	sb_from_x = sb_to_x = event_x(event);
	sb_from_y = r->r_top - sb_r.r_top;
	sb_to_y = sb_r.r_height;
	jump_delta = sb_r.r_width / 10;
    }

    /* 
     * Translate the preview line coordinates from the scrollbar's
     * space to the paint window's coordinate space
     */
    win_translate_xy(scrollbar, paint_window, sb_from_x, sb_from_y, &from_x, &from_y);
    win_translate_xy(scrollbar, paint_window, sb_to_x, sb_to_y, &to_x, &to_y);

    x = event_x(event);
    y = event_y(event);

    /* Must synchronize server before drawing the initial preview */
    xv_set(XV_SERVER_FROM_WINDOW(scrollbar), SERVER_SYNC, 0, NULL);

    /* Draw initial preview line */
    XDrawLine(display, pw_xid, gc, from_x, from_y, to_x, to_y);

    /* Set up the input mask */
    for (i=0; i<IM_MASKSIZE; i++)
	im.im_keycode[i] = 0;
    win_setinputcodebit(&im, MS_LEFT);
    win_setinputcodebit(&im, MS_MIDDLE);
    win_setinputcodebit(&im, MS_RIGHT);
    win_setinputcodebit(&im, LOC_DRAG);
    im.im_flags = IM_ISO | IM_NEGEVENT;

    done = FALSE;
    status = XV_OK;
    while (!done && (status != XV_ERROR)) {
	if (xv_input_readevent((Xv_Window) scrollbar, completion_event, TRUE, TRUE, &im) == -1)
	  status = XV_ERROR;
	else {
	    update_feedback = TRUE;
	    if (event_action(completion_event) == xv_iso_cancel)
	      event_set_action(completion_event, ACTION_CANCEL);
	    else if (event_action(completion_event) == xv_iso_default_action)
	      event_set_action(completion_event, ACTION_DEFAULT_ACTION);
	    else if (event_action(completion_event) == xv_iso_select)
	      event_set_action(completion_event, ACTION_SELECT);

	    switch (event_action(completion_event)) {
	      case ACTION_CANCEL:
		if (event_is_up(completion_event)) {
		    XDrawLine(display, pw_xid, gc, from_x, from_y, to_x, to_y);
		    status = XV_ERROR;
		} else
		  update_feedback = FALSE;
		break;
	      case ACTION_DEFAULT_ACTION:
	      case ACTION_SELECT:
		if (event_is_up(completion_event)) {
		    XDrawLine(display, pw_xid, gc, from_x, from_y, to_x, to_y);
		    done = TRUE;
		} else 
		  update_feedback = FALSE;
		break;
	      case ACTION_DOWN:
		if (event_is_down(completion_event) &&
		    sb->direction == SCROLLBAR_VERTICAL &&
		    y < sb_r.r_height - 2*BORDER_WIDTH)
		  y++;
		else
		  update_feedback = FALSE;
		break;
	      case ACTION_UP:
		if (event_is_down(completion_event) &&
		    sb->direction == SCROLLBAR_VERTICAL &&
		    y > BORDER_WIDTH)
		  y--;
		else
		  update_feedback = FALSE;
		break;
	      case ACTION_LEFT:
		if (event_is_down(completion_event) &&
		    sb->direction == SCROLLBAR_HORIZONTAL &&
		    x > BORDER_WIDTH)
		  x--;
		else
		  update_feedback = FALSE;
		break;
	      case ACTION_RIGHT:
		if (event_is_down(completion_event) &&
		    sb->direction == SCROLLBAR_HORIZONTAL &&
		    x < sb_r.r_width - 2*BORDER_WIDTH)
		  x++;
		else
		  update_feedback = FALSE;
		break;
	      case ACTION_JUMP_DOWN:
		if (event_is_down(completion_event) &&
		    sb->direction == SCROLLBAR_VERTICAL) {
		    y = MIN(y + jump_delta, sb_r.r_height - 2*BORDER_WIDTH);
		} else
		  update_feedback = FALSE;
		break;
	      case ACTION_JUMP_UP:
		if (event_is_down(completion_event) &&
		    sb->direction == SCROLLBAR_VERTICAL) {
		    y = MAX(y - jump_delta, BORDER_WIDTH);
		} else
		  update_feedback = FALSE;
		break;
	      case ACTION_JUMP_RIGHT:
		if (event_is_down(completion_event) &&
		    sb->direction == SCROLLBAR_HORIZONTAL) {
		    x = MIN(x + jump_delta, sb_r.r_width - 2*BORDER_WIDTH);
		} else
		  update_feedback = FALSE;
		break;
	      case ACTION_JUMP_LEFT:
		if (event_is_down(completion_event) &&
		    sb->direction == SCROLLBAR_HORIZONTAL) {
		    x = MAX(x - jump_delta, BORDER_WIDTH);
		} else
		  update_feedback = FALSE;
		break;
	      case ACTION_LINE_START:
	      case ACTION_DATA_START:
		if (sb->direction == SCROLLBAR_VERTICAL)
		  y = BORDER_WIDTH;
		else
		  x = BORDER_WIDTH;
		break;
	      case ACTION_LINE_END:
	      case ACTION_DATA_END:
		if (sb->direction == SCROLLBAR_VERTICAL)
		  y = sb_r.r_height - 2*BORDER_WIDTH;
		else
		  x = sb_r.r_width - 2*BORDER_WIDTH;
		break;
	      case LOC_DRAG:
		x = event_x(completion_event);
		y = event_y(completion_event);
		break;
	      default:
		update_feedback = FALSE;
		break;
	    }
	    if (update_feedback)
	      if (sb->direction == SCROLLBAR_VERTICAL) {
		  if (y >= 0 && y <= sb_r.r_height) {
		      XDrawLine(display, pw_xid, gc, from_x, from_y, to_x, to_y);
		      from_y += y - sb_from_y;
		      to_y = from_y;
		      sb_from_y = y;
		      XDrawLine(display, pw_xid, gc, from_x, from_y, to_x, to_y);
		  }
	      } else {
		  if (x >= 0 && x <= sb_r.r_width) {
		      XDrawLine(display, pw_xid, gc, from_x, from_y, to_x, to_y);
		      from_x += x - sb_from_x;
		      to_x = from_x;
		      sb_from_x = x;
		      XDrawLine(display, pw_xid, gc, from_x, from_y, to_x, to_y);
		  }
	      }
	}
    } /* while (!done && (status != XV_ERROR)) */
    xv_destroy(fs);
    event_set_x(completion_event, x);
    event_set_y(completion_event, y);
    return(status);
}

static int
scrollbar_invoke_split(sb, event)
    Xv_scrollbar_info *sb;
    Event          *event;
{
    int             split_loc;
    Scroll_motion   motion;
    extern Notify_arg win_copy_event();
    extern void     win_free_event();

    /* make sure the event is in the scrollbar */
    split_loc = (sb->direction == SCROLLBAR_VERTICAL) ? event_y(event)
	: event_x(event);

    motion = scrollbar_translate_scrollbar_event_to_motion(sb, event);
    if (motion != sb->transit_motion) {
	if ((sb->transit_motion == SCROLLBAR_TO_START && motion == SCROLLBAR_TO_END) ||
	    (sb->transit_motion == SCROLLBAR_TO_END && motion == SCROLLBAR_TO_START)) {
	    /* destroy the split */
	    scrollbar_destroy_split(sb);
	} else {
	    /* create the split */
	    (void) win_post_id_and_arg(sb->managee,
	     (sb->direction == SCROLLBAR_VERTICAL) ? ACTION_SPLIT_HORIZONTAL
				       : ACTION_SPLIT_VERTICAL,
				       NOTIFY_SAFE, split_loc,
				       win_copy_event, win_free_event);
	}
    }
}


static void
scrollbar_destroy_split(sb)
    Xv_scrollbar_info *sb;
{
    extern Notify_arg win_copy_event();
    extern void     win_free_event();

    win_post_id_and_arg(sb->managee,
			ACTION_SPLIT_DESTROY, NOTIFY_SAFE, XV_ZERO,
			win_copy_event, win_free_event);
}


/*
 * scrollbar_multiclick returns TRUE if this (ACTION_SELECT) event has
 * occurred within sb->multiclick_timeout msec of the last ACTION_SELECT
 * event; otherwise, it returns FALSE.
 */
static int
scrollbar_multiclick(sb, event)
    Xv_scrollbar_info *sb;
    Event          *event;
{
	unsigned int	sec_delta,
			usec_delta;

	sec_delta = event->ie_time.tv_sec - sb->last_select_time.tv_sec;
	usec_delta = event->ie_time.tv_usec - sb->last_select_time.tv_usec;
	if (sec_delta > 0) {		/* normalize the time difference */
		sec_delta--;
		usec_delta += 1000000;
	}

	/* 
	 * Compare the delta time with the multiclick timeout (in msec).
	 * We can't just convert the delta time to msec because we may
	 * very well overflow the machine's integer format with unpredictable
	 * results.
	 */
	if (sec_delta != (sb->multiclick_timeout / 1000)) {
		return (sec_delta < (sb->multiclick_timeout / 1000) );
	}
	else {
		return (usec_delta <= (sb->multiclick_timeout * 1000) );
	}
}


/*
 * setup the values for the repeat status. This routine is called at
 * initialization time (sb_init.c) after .Xdefault values are read.
 */

Pkg_private void
scrollbar_init_delay_values(delay_time, page_interval, line_interval)
    int             delay_time;
    int             page_interval, line_interval;
{
    sb_delay_time = delay_time;
    sb_page_interval = page_interval;
    sb_line_interval = line_interval;
}

/*
 * Poll the state of the mouse to see if the user still has it physically
 * depressed. This is used with the current paradigm to insure speed of user
 * scrolling while also preventing overflow processing after the mouse is
 * released. The other solution is to slow the scroll action so it works
 * correctly on the _slowest_ case, rather than taking advantage of the
 * faster hardware that is available. jcb 5/3/89
 */
Pkg_private int
scrollbar_left_mouse_up(sb)
    Xv_scrollbar_info *sb;

{
    Scrollbar       sb_public = SCROLLBAR_PUBLIC(sb);
    Xv_Drawable_info *info;
    Display        *display;
    Window          window;
    Window          root, child;
    int             root_X, root_Y;
    int             win_X, win_Y;
    unsigned int    key_buttons;

    /* get the root of X information */
    DRAWABLE_INFO_MACRO(sb_public, info);
    display = xv_display(info);
    window = xv_xid(info);

    (void) XQueryPointer(display, window, &root, &child,
			 &root_X, &root_Y, &win_X, &win_Y, &key_buttons);

    return ((key_buttons & Button1Mask) == 0);
}


static int
scrollbar_find_view_nbr(sb, openwin)
    Xv_scrollbar_info *sb;
    Openwin	    openwin;
{
    Xv_Window	    view;
    int		    view_nbr;

    for (view_nbr = 0;
	 (view = xv_get(openwin, OPENWIN_NTH_VIEW, view_nbr))
	  != XV_ZERO;
	 view_nbr++)
	if (view == sb->managee)
	    break;
    return view_nbr;
}
