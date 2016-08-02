#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)notice_pt.c 1.26 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <stdio.h>
#include <xview_private/noticeimpl.h>
#include <olgx/olgx.h>
#include <xview_private/portable.h>
#include <xview_private/draw_impl.h>
#include <xview_private/pw_impl.h>
#include <xview/server.h>
#include <xview/cms.h>

/*
 * XView Public
 */
Xv_public char xv_iso_default_action;
Xv_public char xv_iso_next_element;
Xv_public char xv_iso_select;

/*
 * Table containing valid values for OpenWindows.KeyboardCommands resource
 */
#if !defined(__linux) && !defined(__APPLE__)
Xv_private_data Defaults_pairs xv_kbd_cmds_value_pairs[4];
#else
/* Global already defined and initialized in server/server.c */
extern Defaults_pairs xv_kbd_cmds_value_pairs[4];
#endif

/*
 * --------------------------- Cursor Stuff -------------------------
 */


extern void     xv_help_save_image();

#ifdef OW_I18N
extern struct pr_size xv_pf_textwidth_wc();
#else
extern struct pr_size xv_pf_textwidth();
#endif
Xv_private Time	xv_sel_get_last_event_time();

/*
 * --------------------------- DISPLAY PROCS ------------------------
 */

Pkg_private void     notice_build_button();
Pkg_private int      notice_text_width();
Pkg_private int      notice_button_width();
static void     notice_paint_button();
static void     notice_get_button_pin_points();
static notice_buttons_handle notice_button_for_event();
static void     notice_copy_event();
static Xv_window	notice_get_focus_win();
static int	notice_show_focus_win();

/*
 * --------------------------- STATICS ------------------------------
 */

extern Defaults_pairs bell_types[];

#define NOTICE_INVERT_BUTTON	1
#define NOTICE_NORMAL_BUTTON	0
static notice_buttons_handle	notice_get_prev_button();

static int
  notice_quadrant(Rect notice_screen_rect, int x, int y);

/*
 * --------------------------- Externals ----------------------------
 */

Xv_private Graphics_info *xv_init_olgx();


/*
 * ----------------------- Public Interface -------------------------
 */

/*VARARGS*/
Xv_public int
#ifdef ANSI_FUNC_PROTO
notice_prompt(Xv_Window client_window, Event *event, ...)
#else
notice_prompt(client_window, event, va_alist)
    Xv_Window       client_window;
    Event          *event;
va_dcl
#endif
{
    va_list			valist;
    int				result;
    notice_handle	 	notice;
    AVLIST_DECL;

    if (!client_window) {
	xv_error(XV_ZERO,
		 ERROR_STRING,
	     XV_MSG("NULL parent window passed to notice_prompt(). Not allowed."),
	         ERROR_PKG, NOTICE,
		 NULL);
	return (NOTICE_FAILED);
    }
#ifndef DISABLE_NOTICES
    notice = (struct notice *) xv_calloc(1, sizeof(struct notice));

    if (!notice) {
        xv_error(XV_ZERO,
            ERROR_STRING, 
	    XV_MSG("Malloc failed."),
	    ERROR_PKG, NOTICE,
            NULL);

        return (NOTICE_FAILED);
    }


    /*
     * Get notice default settings
     */
    notice_use_audible_bell = defaults_get_enum("openWindows.beep",
                                "OpenWindows.Beep", bell_types);
    /*
     * Make notice pkg look for new resource OpenWindows.PopupJumpCursor
     * first before Notice.JumpCursor
     */
    if (defaults_exists("openWindows.popupJumpCursor", "OpenWindows.PopupJumpCursor"))  {
        notice_jump_cursor = (int)defaults_get_boolean("openWindows.popupJumpCursor", 
			"OpenWindows.PopupJumpCursor", (Bool) TRUE);
    }
    else  {
        notice_jump_cursor = (int)defaults_get_boolean("notice.jumpCursor", 
			"Notice.JumpCursor", (Bool) TRUE);
    }
    default_beeps = defaults_get_integer("notice.beepCount", 
			"Notice.BeepCount", 1);

    (void) notice_defaults(notice);

    notice->event = (Event *) event;
    notice->client_window = client_window;
    notice->owner_window = notice->fullscreen_window = XV_ZERO;

    VA_START(valist, event);
    MAKE_AVLIST(valist, avlist);
    va_end(valist);

    /*
     * Should consume attributes that should only be used
     * for NOTICE object here
     */

    (void) notice_generic_set(notice, avlist, FALSE);

    if (!notice->notice_font)  {
	notice_determine_font(client_window, notice);
    }

    result = notice_block_popup(notice);

    /*
     * Clean up
     */
    notice_free_button_structs(notice->button_info);
    notice_free_msg_structs(notice->msg_info);
    if (notice->fullscreen_window)  {
	xv_destroy(notice->fullscreen_window);
    }
    free((char *)notice);

    return (result);
#else
    return (NOTICE_YES);
#endif
}

/*
 * ----------------------PRIVATE PROCS-------------------------------
 */

Pkg_private int
notice_block_popup(notice)
Notice_info	*notice;
{
    register Xv_Drawable_info 	*info;
    notice_buttons_handle 	button;
    Cms				cms = XV_ZERO;
    notice_buttons_handle 	current_button = NULL;
    notice_buttons_handle 	prev_button = NULL;
    notice_buttons_handle 	default_button = NULL;
    Graphics_info  		*ginfo;
    Event           		ie;
    Inputmask       		im;
    int             		is_highlighted = FALSE;
    int             		left_placement;
    int             		ok_to_toggle_buttons = FALSE;
    int             		old_mousex;
    int             		old_mousey;
    Rect           		*old_mouse_position;
    int             		quadrant;
    struct rect     		rect;
    int             		result;
    Xv_opaque       		root_window;
    unsigned short  		this_event;
    unsigned short  		this_id;
    int             		three_d;
    int             		top_placement;
    unsigned short  		trigger;
    int				x;
    int				y;
    int				buttons_width;
    Xv_Window			notice_window;
    Fullscreen			fs;
    Rect			notice_screen_rect;
    Xv_Window       		client_window = notice->client_window;
    Xv_Window       		focus_window = XV_ZERO;
    int      			leftoff, topoff;
    int				mouseless = FALSE, first_repaint_set = FALSE;


    /*
     * Check if mouseless is on
     */
    if (defaults_get_enum("openWindows.keyboardCommands",
			  "OpenWindows.KeyboardCommands",
			  xv_kbd_cmds_value_pairs) == KBD_CMDS_FULL)  {
	mouseless = TRUE;
	focus_window = notice_get_focus_win(notice);
    }

    DRAWABLE_INFO_MACRO(client_window, info);

    input_imnull(&im);
    /*
     * Set im to be used in xv_input_readevent
     */
    win_setinputcodebit(&im, MS_LEFT);
    win_setinputcodebit(&im, MS_MIDDLE);
    win_setinputcodebit(&im, MS_RIGHT);
    win_setinputcodebit(&im, LOC_WINENTER);
    win_setinputcodebit(&im, LOC_WINEXIT);
    win_setinputcodebit(&im, LOC_DRAG);
    win_setinputcodebit(&im, LOC_MOVE);
    win_setinputcodebit(&im, WIN_VISIBILITY_NOTIFY);
    win_setinputcodebit(&im, WIN_REPAINT);
    im.im_flags = IM_ASCII | IM_NEGEVENT;

    root_window = (Xv_object) xv_get(client_window, XV_ROOT);

    (void) win_getrect(root_window, &notice_screen_rect);

    DRAWABLE_INFO_MACRO(client_window, info);

    if (xv_depth(info) > 1)  {
        three_d = notice->three_d = defaults_get_boolean("OpenWindows.3DLook.Color",
            "OpenWindows.3DLook.Color", TRUE);
    } else 
#ifdef MONO3D
        three_d = notice->three_d = defaults_get_boolean("OpenWindows.3DLook.Monochrome",
            "OpenWindows.3DLook.Monochrome", FALSE);
#else
        three_d = notice->three_d = FALSE;
#endif

    /*
     * Use client window CMS
     */
    cms = xv_cms(info);

    if (!notice->fullscreen_window)  {
        notice->fullscreen_window = 
	notice_window = (Xv_Window) xv_create(root_window, WINDOW,
	    WIN_TRANSPARENT,
	    WIN_TOP_LEVEL_NO_DECOR, TRUE,	 /* no wmgr decoration */
	    WIN_SAVE_UNDER, TRUE,		 /* no damage caused */
	    WIN_CMS, cms,
	    XV_VISUAL, xv_get(cms, XV_VISUAL),
	    XV_FONT, notice->notice_font,
	    XV_KEY_DATA, notice_context_key, notice,
	    XV_SHOW, FALSE,
	    NULL);

        ginfo = notice->ginfo = xv_init_olgx(notice_window, &three_d,
			 xv_get(notice_window, XV_FONT));

    }
    else  {
	notice_window = notice->fullscreen_window;
        ginfo = notice->ginfo;
    }

    /*
     * Get mouse positioning info
     */
    if (notice->focus_specified) {
        int new_x, new_y;
        win_translate_xy(client_window, root_window, 
			 notice->focus_x,
			 notice->focus_y,
			 &new_x,
			 &new_y);
	x = old_mousex = new_x;
	y = old_mousey = new_y;
    } else {
	old_mouse_position = (Rect *) xv_get(root_window, WIN_MOUSE_XY);
	x = old_mousex = notice->focus_x = old_mouse_position->r_left;
	y = old_mousey = notice->focus_y = old_mouse_position->r_top;
    }

    /* Get size of rectangle */
    (void) notice_get_notice_size(notice, &rect, &buttons_width);

    (void)notice_get_button_pin_points(notice);

    /*
     * Now offset for shadow
     */
    leftoff = topoff = APEX_DIST(notice->scale);

    /*
     * If x and y position is somehow offscreen, default to center of
     * screen
     */
    if (x < 0)  {
	x = old_mousex = 
	    (notice_screen_rect.r_left + notice_screen_rect.r_width)/2;
    }

    if (y < 0)  {
	y = old_mousey = 
	    (notice_screen_rect.r_top + notice_screen_rect.r_height)/2;
    }

    quadrant = notice_quadrant(notice_screen_rect, x, y);

    switch (quadrant) {
      case 0:
	left_placement = old_mousex;
	top_placement = old_mousey;
	rect.r_left = leftoff + PANE_XY(NOTICE_IS_TOPLEVEL, notice->scale);
	rect.r_top = topoff + PANE_XY(NOTICE_IS_TOPLEVEL, notice->scale);
	break;
      case 1:
	left_placement = old_mousex - (rect.r_width + 
			PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + leftoff);
	top_placement = old_mousey;
	rect.r_left = PANE_XY(NOTICE_IS_TOPLEVEL, notice->scale);
	rect.r_top = topoff + PANE_XY(NOTICE_IS_TOPLEVEL, notice->scale);
	break;
      case 2:
	left_placement = old_mousex - (rect.r_width + 
			PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + leftoff);
	top_placement = old_mousey - (rect.r_height + 
			PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + topoff);
	rect.r_left = PANE_XY(NOTICE_IS_TOPLEVEL, notice->scale);
	rect.r_top = PANE_XY(NOTICE_IS_TOPLEVEL, notice->scale);
	break;
      case 3:
	left_placement = old_mousex;
	top_placement = old_mousey - (rect.r_height + 
			PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + topoff);
	rect.r_left = leftoff + PANE_XY(NOTICE_IS_TOPLEVEL, notice->scale);
	rect.r_top = PANE_XY(NOTICE_IS_TOPLEVEL, notice->scale);
	break;
    }

    xv_set(notice_window,
	    XV_X, left_placement,
	    XV_Y, top_placement,
	    XV_WIDTH, rect.r_width + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + 
			leftoff,
	    XV_HEIGHT, rect.r_height + PANE_NOTICE_DIFF(NOTICE_IS_TOPLEVEL, notice->scale) + 
			topoff,
	    XV_SHOW, TRUE,
	    NULL);

    /*
     * BUG: Should wait for notice window to map
     */

    fs = (Fullscreen)
	xv_create(root_window, FULLSCREEN,
	    FULLSCREEN_INPUT_WINDOW, notice_window,
	    WIN_CONSUME_EVENTS,
		WIN_VISIBILITY_NOTIFY,
		WIN_REPAINT,
		WIN_MOUSE_BUTTONS,
		WIN_ASCII_EVENTS,
		WIN_UP_ASCII_EVENTS,
		LOC_WINENTER, LOC_WINEXIT,
		LOC_DRAG, LOC_MOVE,
		0,
	    NULL);
    if (fs) {
	notice->fullscreen = fs;
    } else {
	return (NOTICE_FAILED);	/* out of memory?? */
    }

    if (xv_get(xv_server(info), SERVER_JOURNALLING))
	(void) xv_set(xv_server(info), SERVER_JOURNAL_SYNC_EVENT, 1, NULL);

    (void) notice_do_bell(notice);

    /*
     * then draw empty box and shadow
     */
    (void) notice_drawbox(notice_window, &rect, quadrant, 
				leftoff, topoff);

    /*
     * now fill in the box with the text AND buttons
     */
    (void) notice_layout(notice, &rect, buttons_width);

    /*
     * Mouseless
     * Draw location cursor
     */
    if (mouseless)  {
        notice_show_focus_win(notice, notice->button_info, focus_window, FALSE);
    }

    /*
     * If notice.jumpCursor is set, and default button exists (always
     * true), calculate (x,y) of center of default button, and warp
     * ptr there. Also save ptr to default button for later use.
     */
    if (notice_jump_cursor && notice->yes_button_exists) {
	notice_buttons_handle curr;
	for (curr = (notice_buttons_handle) notice->button_info;
	     curr != (notice_buttons_handle) NULL;
	     curr = (notice_buttons_handle) curr->next)
	    if (curr->is_yes) {
		default_button = curr;
		x = left_placement + curr->button_rect.r_left +
		    (curr->button_rect.r_width / 2);
		y = top_placement + curr->button_rect.r_top + 
			(curr->button_rect.r_height/2);
			
		(void) xv_set(root_window, WIN_MOUSE_XY, x, y, NULL);
		break;
	    }
    }

    if (!default_button)  {
        notice_buttons_handle curr;

        /*
         * Search for default button on notice
         */
        for (curr = (notice_buttons_handle) notice->button_info;
                curr != (notice_buttons_handle) NULL;
                curr = (notice_buttons_handle) curr->next)  {
            if (curr->is_yes) {
                default_button = curr;
                break;
            }
        }
    }

    /*
     * Stay in fullscreen until a button is pressed, or trigger used
     */
    trigger = notice->default_input_code;

    if (xv_get(xv_server(info), SERVER_JOURNALLING))
	(void) xv_set(xv_server(info), SERVER_JOURNAL_SYNC_EVENT, 1, NULL);

    prev_button = notice->button_info;

    for (;;) {
	int             type, is_select_action, is_stop_key;
	Time		server_time, response_time, repaint_time;
	short		button_or_key_event;

	if (xv_input_readevent(notice_window, &ie, TRUE, TRUE, &im) 
		== (Xv_object)-1) {
	    break;
	}

        type = event_xevent_type(&ie);

	/*
	 * For Button and Key events:
	 * We need to make sure that these types of events are
	 * processed only if they occurred sometime *after* the
	 * notice window is mapped.
	 */

	/*
	 * Init flag to FALSE
	 */
	button_or_key_event = FALSE;

	/*
	 * Get event time for Button/Key events
	 * set flag to TRUE
	 */
	if ((type == ButtonPress) || (type == ButtonRelease))  {
	    XButtonEvent      *eb = (XButtonEvent *) event_xevent(&ie);

	    response_time = eb->time;
	    button_or_key_event = TRUE;
	}

	if ((type == KeyPress) || (type == KeyRelease))  {
	    XKeyEvent      *ek = (XKeyEvent *) event_xevent(&ie);

	    response_time = ek->time;
	    button_or_key_event = TRUE;
	}

	/*
	 * Check if the Button/Key event occurred before the first repaint
	 * event.
	 */
	if (button_or_key_event)  {
	    /*
	     * If the repaint time is not even set, this event should not be
	     * processed
	     */
	    if (!first_repaint_set)  {
		continue;
	    }

	    if (response_time < repaint_time)  {
	        Xv_Drawable_info	*notice_window_info;

		/*
		 * The Button/Key event time is before the repaint time.
		 * 2 possibilities here:
		 *	1. the Button/Key event actually occurred before the
		 *	   repaint time
		 *	2. the server time went over 49.7 days and got rolled
		 *	   over. So, in this case an event occurring after
		 *	   the first repaint might have a time value that is
		 *	   smaller than the repaint time.
		 * To check for (2), we get the current server time, and see if 
		 * it is smaller than the repaint time. If yes, the clock had
		 * rolled over, and the event *did* actually occur after the 
		 * repaint. If no, the clock had not rolled over, and the
		 * event did occcur before the repaint event - such events are 
		 * ignored.
		 */
	        DRAWABLE_INFO_MACRO(notice_window, notice_window_info);

                server_time = 
			xv_sel_get_last_event_time(xv_display(notice_window_info),
				xv_xid(notice_window_info));
		
		if (server_time > repaint_time)  {
		    continue;
		}
		else  {
		    repaint_time = server_time;
		}
	    }
	}

	x = event_x(&ie);
	y = event_y(&ie);

        /* Translate unmodified ISO (ASCII) Mouseless Keyboard Commands
         * used inside a notice.
         */
	if (mouseless)  {
            if (event_action(&ie) == xv_iso_default_action)  {
                event_set_action(&ie, ACTION_DEFAULT_ACTION);
            }
            else  {
                if (event_action(&ie) == xv_iso_next_element) {
                    if (event_shift_is_down(&ie))  {
                        event_set_action(&ie, ACTION_PREVIOUS_ELEMENT);
                    }
                    else  {
                        event_set_action(&ie, ACTION_NEXT_ELEMENT);
                    }
                } else if (event_action(&ie) == xv_iso_select) {
                    event_set_action(&ie, ACTION_SELECT);
                }
            }
        }

	this_event = event_action(&ie);	/* get encoded event */
	this_id = event_id(&ie);/* get unencoded event */

	if (this_event == ACTION_HELP)  {
	    continue;
	}

	is_select_action = ((this_event == (int) ACTION_SELECT) ||
			    (this_id == (int) MS_LEFT))
	    ? 1 : 0;
	is_stop_key = ((this_event == (int) ACTION_STOP) ||
		       (this_id == (int) WIN_STOP))
	    ? 1 : 0;

	/*
	 * Get notice button for this event, given (x,y) position
	 * on notice window
	 */
	button = notice_button_for_event(notice, x, y);

	if (event_action(&ie) == ACTION_NEXT_ELEMENT)  {
            if (event_is_down(&ie))  {
	        if (prev_button)  {
		    button = prev_button->next;
		    if (!button)  {
	                button = notice->button_info;
		    }
	        }
	        else  {
	            prev_button = notice->button_info;

		    if (prev_button->next)  {
		        button = prev_button->next;
		    }
	        }
                notice_show_focus_win(notice, prev_button, focus_window, TRUE);
                notice_do_buttons(notice, &rect, -1, prev_button, buttons_width);
	        prev_button = button;
                notice_show_focus_win(notice, button, focus_window, FALSE);
	        continue;
	    }
	}

	if (event_action(&ie) == ACTION_PREVIOUS_ELEMENT)  {
            if (event_is_down(&ie))  {
	        button = notice_get_prev_button(notice, prev_button);
                notice_show_focus_win(notice, prev_button, focus_window, TRUE);
                notice_do_buttons(notice, &rect, -1, prev_button, buttons_width);
	        prev_button = button;
                notice_show_focus_win(notice, button, focus_window, FALSE);
	        continue;
	    }
	}

	/*
	 * Must use the button selected using mouseless interface
	 * if mouseless on, and event is not mouse-related
	 */
	if (mouseless && !event_is_button(&ie) && (this_event != LOC_DRAG))  {
	    button = prev_button;
	}


	/*
	 * Check if notice is obscured
	 */
	if (this_event == WIN_VISIBILITY_NOTIFY)  {
	    XVisibilityEvent	*xVisEv;
	    xVisEv = (XVisibilityEvent *)event_xevent(&ie);

	    if ((xVisEv->state == VisibilityPartiallyObscured) || 
		(xVisEv->state == VisibilityFullyObscured))  {
		Xv_Drawable_info	*notice_window_info;

		DRAWABLE_INFO_MACRO(notice_window, notice_window_info);
		/*
		 * If notice is obscured, raise it
		 */
		XRaiseWindow(xv_display(notice_window_info), 
				xv_xid(notice_window_info));
	    }

	    continue;
	}

	/*
	 * Check if notice needs to be repainted
	 */
	if (this_event == WIN_REPAINT)  {
	    Xv_Drawable_info	*notice_window_info;
	    DRAWABLE_INFO_MACRO(notice_window, notice_window_info);

	    if (!first_repaint_set)  {
                repaint_time = 
			xv_sel_get_last_event_time(xv_display(notice_window_info),
				xv_xid(notice_window_info));
		first_repaint_set = TRUE;
	    }

            /*
             * draw empty box and shadow
             */
            (void) notice_drawbox(notice_window, &rect, 
                                quadrant, leftoff, topoff);
            /*
             * now fill in the box with the text AND buttons
             */
            (void) notice_layout(notice, &rect, buttons_width);

	    /*
	     * Mouseless
	     * Draw location cursor
	     */
	    if (mouseless)  {
                notice_show_focus_win(notice, button, focus_window, FALSE);
	    }

	    continue;
	}


	if (((this_event == trigger) || (this_id == trigger))
	    && (((trigger == (int) ACTION_SELECT) ||
		 (trigger == (int) MS_LEFT)) ?
		(event_is_up(&ie) && (current_button == NULL))
		: 0)) {
	    /*
	     * catch UP mouse left if missed down below for trigger
	     */
	    notice->result = NOTICE_TRIGGERED;
	    (void) notice_copy_event(notice, &ie);
	    goto Done;
	} else if (((this_event == trigger) || (this_id == trigger))
		   && (((trigger == (int) ACTION_SELECT) ||
			(trigger == (int) MS_LEFT)) ?
		       (event_is_down(&ie) && (button == NULL))
		       : 0)) {
	    /*
	     * catch down mouse left for trigger, check above against
	     * button rather than current_button since current_button
	     * is NULL on SELECT down, but button may be a real button
	     */
	    notice->result = NOTICE_TRIGGERED;
	    (void) notice_copy_event(notice, &ie);
	    goto Done;
	} else if (is_stop_key
		   && notice->no_button_exists) {
	    notice->result = NOTICE_NO;
	    (void) notice_copy_event(notice, &ie);
	    goto Done;
	} else if ((this_event == ACTION_DO_IT
		    || this_event == NOTICE_ACTION_DO_IT)
		   && notice->yes_button_exists) {
	    if (!event_is_down(&ie))  {
		continue;
	    }

	    notice->result = default_button->value;
	    (void) notice_copy_event(notice, &ie);
	    goto Done;
	    /*
	     * NOTE: handle button event processing starting here
	     */
	} else if (is_select_action && notice->button_info) {
	    if (event_is_down(&ie)) {
		if (current_button &&
		    (current_button != button) &&
		    is_highlighted) {
		    notice_paint_button(notice_window,
			current_button, NOTICE_NORMAL_BUTTON, ginfo, three_d);
		    current_button = NULL;
		    is_highlighted = FALSE;
		    ok_to_toggle_buttons = FALSE;
		}
		if (button &&
		    !is_highlighted &&
		    current_button != button) {

		    /* Mouseless */
		    if (mouseless)  {
			/*
			 * Erase focus window over previous button
			 * Redraw previous button
			 */
			if (prev_button)  {
                            notice_show_focus_win(notice, prev_button, focus_window, TRUE);
                            notice_do_buttons(notice, &rect, -1, prev_button, buttons_width);
			}

			/*
			 * Draw focus window over current button
			 */
                        notice_show_focus_win(notice, current_button, focus_window, TRUE);
		    }

		    current_button = button;
		    notice_paint_button(notice_window,
			current_button, NOTICE_INVERT_BUTTON, ginfo, three_d);
		    prev_button = current_button = button;

		    /* Mouseless */
		    if (mouseless)  {
                        notice_show_focus_win(notice, button, focus_window, FALSE);
		    }

		    is_highlighted = TRUE;
		    ok_to_toggle_buttons = TRUE;
		}
	    } else {		/* event_is_up */
		if (button) {
		    if (current_button &&
			(current_button != button) &&
			is_highlighted) {

		        /* Mouseless */
			if (mouseless)  {
                            notice_show_focus_win(notice, current_button, focus_window, TRUE);
			}

			notice_paint_button(notice_window,
			    current_button, NOTICE_NORMAL_BUTTON, ginfo, three_d);
			current_button = NULL;
			is_highlighted = FALSE;
			ok_to_toggle_buttons = FALSE;
		    }
		    notice->result = button->value;
		    (void) notice_copy_event(notice, &ie);
		    goto Done;
		} else {
		    ok_to_toggle_buttons = FALSE;
		}
	    }
	} else if (this_event == LOC_DRAG) {
	    if (current_button && (current_button != button)) {
		notice_paint_button(notice_window,
		    current_button, NOTICE_NORMAL_BUTTON, ginfo, three_d);

		/* Mouseless */
		if (mouseless)  {
                    notice_show_focus_win(notice, current_button, focus_window, FALSE);
		}

		is_highlighted = FALSE;
		current_button = NULL;
		continue;
	    }
	    if (button) {
		if (current_button == button) {
		    continue;	/* already there */
		} else if ((current_button == NULL) && ok_to_toggle_buttons) {
		    /* Mouseless */
		    if (mouseless && prev_button)  {
                        notice_show_focus_win(notice, prev_button, focus_window, TRUE);
                        notice_do_buttons(notice, &rect, -1, prev_button, buttons_width);
		    }

		    notice_paint_button(notice_window,
			button, NOTICE_INVERT_BUTTON, ginfo, three_d);
		    prev_button = current_button = button;

		    /* Mouseless */
		    if (mouseless)  {
                        notice_show_focus_win(notice, button, focus_window, FALSE);
		    }

		    is_highlighted = TRUE;
		    continue;
		}
	    } else if (!button && current_button) {
		/* Mouseless */
		if (mouseless)  {
                    notice_show_focus_win(notice, current_button, focus_window, TRUE);
		}

		notice_paint_button(notice_window,
		    current_button, NOTICE_NORMAL_BUTTON, ginfo, three_d);
		current_button = NULL;
		is_highlighted = FALSE;
		continue;
	    }
	} else if (((this_event==trigger)||(this_id==trigger))
		&& (!is_select_action)) {
	    /*
	     * catch trigger as a last case, trigger can't be select button
	     * here as that case is dealt with above
	     */
	    notice->result = NOTICE_TRIGGERED;
	    (void) notice_copy_event(notice, &ie);
	    goto Done;
	}
    }

Done:
    if (xv_get(xv_server(info), SERVER_JOURNALLING))
	(void) xv_set(xv_server(info), SERVER_JOURNAL_SYNC_EVENT, 1, NULL);
    (void) xv_destroy(fs);

    result = notice->result;

    /*
     * Copy the result to notice->result_ptr if NOTICE_STATUS was specified 
     * i.e. an additional place to put the result
     */
    if (notice->result_ptr)  {
        *(notice->result_ptr) = notice->result;
    }

    if (client_window && (notice->event != (Event *)0)) {
    	int new_x, new_y;
        win_translate_xy(notice_window, client_window, 
			 event_x(notice->event),
			 event_y(notice->event),
			 &new_x,
			 &new_y);
	event_set_x(notice->event, new_x);
	event_set_y(notice->event, new_y);
	event_set_window(notice->event, client_window);
    }
    /* warp mouse back */
    if (notice_jump_cursor && notice->yes_button_exists) {
	if (notice->focus_specified) {
	    (void) xv_set(root_window, WIN_MOUSE_XY,
			  old_mousex, old_mousey,
			  NULL);
	} else {
	    (void) xv_set(root_window, WIN_MOUSE_XY, old_mousex, old_mousey, NULL);
	}
    }

    xv_set(notice_window, XV_SHOW, FALSE, NULL);

    /*
     * BUG: Should wait for notice window to unmap before returning
     */

    return (result);
}


static void
notice_copy_event(notice, event)
    register notice_handle notice;
    Event          *event;
{
    if (notice->event == (Event *) 0) {
	return;
    } else
	*notice->event = *event;
}

/*
 * --------------------------- Statics ------------------------------
 */

static void
notice_get_button_pin_points(notice)
register notice_handle	notice;
{
    Graphics_info	*ginfo = notice->ginfo;
    notice_buttons_handle curr;
    Xv_Font	this_font = (Xv_Font)(notice->notice_font);

    for (curr = notice->button_info; curr != NULL; curr = curr->next) {
        (void)notice_button_width(this_font, ginfo, curr);
    }
}


/*
 * ----------------------   Misc Utilities   ------------------------
 */

/*
 * font char/pixel conversion routines
 */

Pkg_private int
notice_text_width(font, str)
    Xv_Font        font;
    CHAR           *str;
{
    struct pr_size  size;

#ifdef OW_I18N
    size = xv_pf_textwidth_wc(STRLEN(str), font, str);
#else
    size = xv_pf_textwidth(STRLEN(str), font, str);
#endif

    return (size.x);
}

Pkg_private int
notice_button_width(font, ginfo, button)
    Xv_Font			font;
    Graphics_info  		*ginfo;
    notice_buttons_handle	button;
{
    button->button_rect.r_width = notice_text_width(font, button->string) +
	2*ButtonEndcap_Width(ginfo);
    button->button_rect.r_height = Button_Height(ginfo);
    return (button->button_rect.r_width);
}

static int
notice_quadrant(Rect notice_screen_rect, int x, int y)
{
    int             quadrant;

    if ((x <= notice_screen_rect.r_width / 2) && (y <= notice_screen_rect.r_height / 2))
	quadrant = 0;
    else if ((x > notice_screen_rect.r_width / 2) && (y <= notice_screen_rect.r_height / 2))
	quadrant = 1;
    else if ((x > notice_screen_rect.r_width / 2) && (y > notice_screen_rect.r_height / 2))
	quadrant = 2;
    else
	quadrant = 3;

    return (quadrant);
}

Pkg_private void
notice_build_button(pw, x, y, button, ginfo, three_d)
    Xv_Window       pw;
    int             x, y;
    notice_buttons_handle button;
    Graphics_info  *ginfo;
    int		   three_d;
{
    button->button_rect.r_top = y;
    button->button_rect.r_left = x;
    notice_paint_button(pw, button, NOTICE_NORMAL_BUTTON, ginfo, three_d);
}

static void
notice_paint_button(pw, button, invert, ginfo, three_d)
    Xv_Window	    pw;
    notice_buttons_handle button;
    int             invert;
    Graphics_info  *ginfo;
    int		    three_d;
{
    Xv_Drawable_info *info;
    int		    state;

    DRAWABLE_INFO_MACRO(pw, info);
    if (invert)
	state = OLGX_INVOKED;
    else if (three_d)
	state = OLGX_NORMAL;
    else
	state = OLGX_NORMAL | OLGX_ERASE;
    if (button->is_yes)
	state |= OLGX_DEFAULT;
    olgx_draw_button(ginfo, xv_xid(info), button->button_rect.r_left,
	button->button_rect.r_top, button->button_rect.r_width, 0,
#ifdef OW_I18N
	button->string, state | OLGX_LABEL_IS_WCS);
#else
	button->string, state);
#endif
}

static          notice_buttons_handle
notice_button_for_event(notice, x, y)
    register notice_handle notice;
{
    register notice_buttons_handle curr;

    if (notice->button_info == NULL)
	return (NULL);
    for (curr = notice->button_info; curr; curr = curr->next) {
	if ((x >= curr->button_rect.r_left)
	    && (x <= (curr->button_rect.r_left +
		      curr->button_rect.r_width))
	    && (y >= curr->button_rect.r_top)
	    && (y <= (curr->button_rect.r_top
		      + curr->button_rect.r_height))) {
	    return (curr);
	}
    }
    return ((notice_buttons_handle) 0);
}

static          notice_buttons_handle
notice_get_prev_button(notice, button)
    register notice_handle notice;
    notice_buttons_handle	button;
{
    register notice_buttons_handle cur, prev = NULL;
    int last = FALSE;

    if (notice->button_info == NULL)  {
	return (NULL);
    }

    if (notice->number_of_buttons == 1)  {
	return (notice->button_info);
    }

    if (!button)  {
	return (notice->button_info);
    }

    for (cur = notice->button_info; cur; prev = cur, cur = cur->next) {
	if (cur == button)  {
	    if (prev)  {
		return(prev);
	    }
	    else  {
		last = TRUE;
	    }
	}
    }

    if (last)  {
        return ((notice_buttons_handle) prev);
    }
    else  {
        return ((notice_buttons_handle) 0);
    }
}

static Xv_window
notice_get_focus_win(notice)
Notice_info	*notice;
{
    if (!notice->owner_window)  {
        notice_get_owner_frame(notice);
    }

    return((Xv_window)xv_get(notice->owner_window, FRAME_FOCUS_WIN));
}

static int
notice_show_focus_win(notice, button, focus_window, erase)
Notice_info		*notice;
notice_buttons_handle	button;
Xv_window		focus_window;
int			erase;
{
    Xv_window	fs_win;
    Xv_Drawable_info *image_info;
    Xv_Drawable_info *info;
    Server_image	image;
    GC		gc;
    XGCValues	gc_values;
    unsigned long	valuemask = 0;
    int		x, y, width, height;

    if (!button)  {
	return(XV_ERROR);
    }

    fs_win = notice->fullscreen_window;

    if (!fs_win)  {
	return(XV_ERROR);
    }

    if (!focus_window)  {
	return(XV_ERROR);
    }

    x = button->button_rect.r_left + 
		(button->button_rect.r_width - FRAME_FOCUS_UP_WIDTH)/2;
    y = button->button_rect.r_top + button->button_rect.r_height - FRAME_FOCUS_UP_HEIGHT/2;
    width = FRAME_FOCUS_UP_WIDTH;
    height = FRAME_FOCUS_UP_HEIGHT;


    DRAWABLE_INFO_MACRO(focus_window, info);
    gc = (GC) xv_get(focus_window, XV_KEY_DATA, FRAME_FOCUS_GC);
    if (!gc) {
        /* Create the Graphics Context for the Focus Window */
	/* THIS IS ALSO DONE IN frame_focus_win_event_proc() in fm_input.c*/
        gc_values.fill_style = FillOpaqueStippled;
        gc = XCreateGC(xv_display(info), xv_xid(info), GCFillStyle,
                            &gc_values);
        xv_set(focus_window, XV_KEY_DATA, FRAME_FOCUS_GC, gc, NULL);
    }

    DRAWABLE_INFO_MACRO(fs_win, info);

    if (erase)  {
        gc_values.fill_style = FillSolid;
        gc_values.foreground = xv_bg(info);
    }
    else  {
        image = xv_get(focus_window, XV_KEY_DATA, FRAME_FOCUS_UP_IMAGE);
        DRAWABLE_INFO_MACRO(image, image_info);
        gc_values.fill_style = FillOpaqueStippled;
        gc_values.stipple = xv_xid(image_info);
        gc_values.ts_x_origin = x;
        gc_values.ts_y_origin = y;
        gc_values.background = xv_bg(info);
        gc_values.foreground = xv_fg(info);
	valuemask |= GCStipple | GCTileStipXOrigin | GCTileStipYOrigin
			| GCBackground;
    }

    valuemask |= GCFillStyle | GCForeground;

    XChangeGC(xv_display(info), gc, valuemask, &gc_values);

    XFillRectangle(xv_display(info), xv_xid(info), gc, x, y,
                        width, height);

    if (!erase)  {
        gc_values.ts_x_origin = 0;
        gc_values.ts_y_origin = 0;
        gc_values.fill_style = FillOpaqueStippled;
        XChangeGC(xv_display(info), gc, GCTileStipXOrigin | GCTileStipYOrigin 
		    | GCFillStyle, &gc_values);
    }

    return(XV_OK);
}
