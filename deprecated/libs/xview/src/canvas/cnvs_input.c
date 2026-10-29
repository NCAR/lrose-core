#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)cnvs_input.c 20.62 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/cnvs_impl.h>
#include <xview/canvas.h>
#include <xview/frame.h>
#include <xview/openmenu.h>
#include <xview/scrollbar.h>
#include <xview/xv_xrect.h>
#include <xview_private/draw_impl.h>
#ifdef OW_I18N
#include <xview/xv_i18n.h>
#endif /*OW_I18N*/

#define window_event_proc(win, event, arg) \
    (((int (*)())(window_get(win, WIN_EVENT_PROC)))(win, event, arg))

Xv_private Xv_xrectlist *screen_get_clip_rects();
Xv_private int server_get_fullscreen();

extern Rectlist *win_get_damage();

static void canvas_clear_damage();
static void canvas_inform_resize();

/*
 * handle events posted to the view window.
 */

/* ARGSUSED */
Pkg_private Notify_value
canvas_view_event(view_public, event, arg, type)
    Canvas_view     view_public;
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    Canvas_view_info *view = CANVAS_VIEW_PRIVATE(view_public);
    Canvas_info    *canvas = view->private_canvas;
    Xv_Window       paint_window = view->paint_window;
    Notify_value    result;
    Rect            paint_rect;

    result = notify_next_event_func(view_public, (Notify_event) event, arg, type);

    switch (event_id(event)) {
      case WIN_RESIZE:
	paint_rect = *(Rect *) xv_get(paint_window, WIN_RECT);
	canvas_resize_paint_window(canvas, paint_rect.r_width, paint_rect.r_height);
	break;
      case SCROLLBAR_REQUEST:
	canvas_scroll(paint_window, (Scrollbar) arg);
	break;
      default:
	break;
    }

    return (result);
}

/*
 * Handle events for the paint window.  These events are passed on to the
 * canvas client CANVAS_EVENT_PROC.
 */

/* Save some memory space here, since these variables are never
 * used simulataneously.
 */
#define next_pw	    next_view
#define nth_pw	    nth_view
#define previous_pw previous_view
#define pw_nbr	    view_nbr

/* ARGSUSED */
Pkg_private Notify_value
canvas_paint_event(window_public, event, arg, type)
    Xv_Window       window_public;
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    Canvas_info    *canvas;
    Canvas          canvas_public;
    char           *help_data;
    Xv_Window	    next_view;
    Xv_Window	    nth_view;
    Xv_Window	    previous_view;
    Notify_value    result;
    Scrollbar	    sb;
    Xv_Window	    view;
    int		    view_nbr;

#ifdef OW_I18N
    XIC             ic;
    XID	            xid;
    XPointer        client_data;
    Xv_object       paint_public;
#endif /*OW_I18N*/

    result = notify_next_event_func(window_public, (Notify_event) event, arg,
				    type);

#ifdef OW_I18N1
    ic = (XIC) xv_get(window_public, WIN_IC);
    if (ic && (XGetICValues(ic, XNFocusWindow, &xid, NULL) == NULL) 
        && xid ) {
	paint_public = (Canvas) win_data(XDisplayOfIM(XIMOfIC(ic)),xid);
        canvas = (Canvas_info *) xv_get(paint_public, XV_KEY_DATA,
                               canvas_context_key);
        canvas_public = CANVAS_PUBLIC(canvas);
    } else {
        canvas = (Canvas_info *) xv_get(window_public, XV_KEY_DATA,
                               canvas_context_key);
/* ISSUE: client_data is not initialized */
        canvas_public = (Canvas)client_data;
    }
#else 
    canvas = (Canvas_info *) xv_get(window_public, XV_KEY_DATA,
				    canvas_context_key);
    canvas_public = CANVAS_PUBLIC(canvas);
#endif /*OW_I18N*/

    switch (event_action(event)) {
      case WIN_REPAINT:
      case WIN_GRAPHICS_EXPOSE:
	canvas_inform_repaint(canvas, window_public);
	break;

      case WIN_RESIZE:
	/* scrollbars have already been updated */
	/* tell the client the paint window changed size */
	canvas_inform_resize(canvas);
	break;

      case ACTION_HELP:
      case ACTION_MORE_HELP:
      case ACTION_TEXT_HELP:
      case ACTION_MORE_TEXT_HELP:
      case ACTION_INPUT_FOCUS_HELP:
#ifdef OW_I18N1
        if (event_is_down(event)) {
            if ((Attr_pkg) xv_get(window_public, WIN_TYPE) == CANVAS_TYPE) {
                help_data = (char *) xv_get(window_public, XV_HELP_DATA);
                if (help_data)
                    xv_help_show(window_public,help_data, event);
            }
        }
        break;

#else
	if (event_is_down(event)) {
	    if ((Attr_pkg) xv_get(canvas_public, WIN_TYPE) == CANVAS_TYPE) {
		help_data = (char *) xv_get(canvas_public, XV_HELP_DATA);
		if (help_data)
		    xv_help_show(window_public, help_data, event);
	    }
	}
	break;
#endif /* OW_I18N */
      case ACTION_NEXT_PANE:
	if (event_is_down(event)) {
	    for (pw_nbr = 0;
		 nth_pw = xv_get(canvas_public, CANVAS_NTH_PAINT_WINDOW,
				 pw_nbr);
		 pw_nbr++) {
		if (nth_pw == window_public)
		    break;
	    }
	    next_pw = xv_get(canvas_public, CANVAS_NTH_PAINT_WINDOW,
			     pw_nbr + 1);
	    if (next_pw) {
		/* Set focus to first element in next paint window */
		xv_set(next_pw, WIN_SET_FOCUS, NULL);
		xv_set(canvas_public, XV_FOCUS_ELEMENT, 0, NULL);
	    } else
		xv_set(xv_get(canvas_public, WIN_FRAME), FRAME_NEXT_PANE, NULL);
	}
	break;

      case ACTION_PREVIOUS_PANE:
	if (event_is_down(event)) {
	    for (pw_nbr = 0;
		 nth_pw = xv_get(canvas_public, CANVAS_NTH_PAINT_WINDOW,
				 pw_nbr);
		 pw_nbr++) {
		if (nth_pw == window_public)
		    break;
		previous_pw = nth_pw;
	    }
	    if (pw_nbr > 0) {
		/* Set focus to last element in previous paint window */
		xv_set(previous_pw, WIN_SET_FOCUS, NULL);
		xv_set(canvas_public, XV_FOCUS_ELEMENT, -1, NULL);
	    } else {
		xv_set(xv_get(canvas_public, WIN_FRAME),
		       FRAME_PREVIOUS_PANE,
		       NULL);
	    }
	}
	break;

      case ACTION_PREVIOUS_ELEMENT:
	/* Order of precedence:
	 * 	previous paint window's horizontal scrollbar
	 *	previous paint window's vertical scrollbar
	 *	last element in previous frame subwindow
	 */
	if (event_is_down(event)) {
	    view = CANVAS_VIEW_PUBLIC((Canvas_view_info *) xv_get(window_public,
	    	XV_KEY_DATA, canvas_view_context_key));
	    for (view_nbr = 0;
		 nth_view = xv_get(canvas_public, OPENWIN_NTH_VIEW, view_nbr);
		 view_nbr++) {
		if (nth_view == view)
		    break;
		previous_view = nth_view;
	    }
	    if (view_nbr > 0) {
		sb = xv_get(canvas_public,
			    OPENWIN_HORIZONTAL_SCROLLBAR, previous_view);
		if (!sb)
		    sb = xv_get(canvas_public,
				OPENWIN_VERTICAL_SCROLLBAR, previous_view);
		xv_set(sb, WIN_SET_FOCUS, NULL);
	    } else {
		/* Go to last element in previous frame subwindow */
		xv_set(xv_get(canvas_public, WIN_FRAME),
		       FRAME_PREVIOUS_ELEMENT,
		       NULL);
	    }
	}
	break;

      case ACTION_NEXT_ELEMENT:
	/* Order of precedence:
	 *	paint window's vertical scrollbar
	 *	paint window's horizontal scrollbar
	 *	next frame subwindow
	 */
	if (event_is_down(event)) {
	    view = CANVAS_VIEW_PUBLIC((Canvas_view_info *) xv_get(window_public,
	    	XV_KEY_DATA, canvas_view_context_key));
	    sb = xv_get(canvas_public, OPENWIN_VERTICAL_SCROLLBAR, view);
	    if (!sb)
		sb = xv_get(canvas_public, OPENWIN_HORIZONTAL_SCROLLBAR, view);
	    if (sb) {
		xv_set(sb, WIN_SET_FOCUS, NULL);
		break;
	    }
	    /* There is no scrollbar attached: go to next pane */
	    xv_set(xv_get(canvas_public, WIN_FRAME), FRAME_NEXT_PANE, NULL);
	}
	break;

      case ACTION_VERTICAL_SCROLLBAR_MENU:
      case ACTION_HORIZONTAL_SCROLLBAR_MENU:
	view = CANVAS_VIEW_PUBLIC((Canvas_view_info *) xv_get(window_public,
	    XV_KEY_DATA, canvas_view_context_key));
	if (event_action(event) == ACTION_VERTICAL_SCROLLBAR_MENU)
	    sb = xv_get(canvas_public, OPENWIN_VERTICAL_SCROLLBAR, view);
	else
	    sb = xv_get(canvas_public, OPENWIN_HORIZONTAL_SCROLLBAR, view);
	if (sb) {
	    Event	    sb_event;

	    event_init(&sb_event);
	    event_set_action(&sb_event, ACTION_MENU);
	    event_set_window(&sb_event, sb);
	    sb_event.ie_flags = event->ie_flags; /* set up/down flag */
	    win_post_event(sb, &sb_event, NOTIFY_SAFE);
	}
	break;

      case ACTION_JUMP_MOUSE_TO_INPUT_FOCUS:
	view = CANVAS_VIEW_PUBLIC((Canvas_view_info *) xv_get(window_public,
	    XV_KEY_DATA, canvas_view_context_key));
	xv_set(view,
	       WIN_MOUSE_XY, 0, 0,
	       NULL);
	/* BUG ALERT:  Clicking MENU at this point does not send ACTION_MENU
	 *	       to the canvas paint window.  Instead, an Window Manager
	 *	       Window menu is brought up.
	 */
	break;

#ifdef OW_I18N
      case KBD_USE: {
	XID			 xwin;

	if (canvas->ic) {
	    if (canvas->focus_pwin != window_public) {
		/* 
		 * Set XNFocusWindow and cache the value.
		 */
		window_set_ic_focus_win(window_public, canvas->ic,
			xv_get(window_public, XV_XID));
		canvas->focus_pwin = window_public;
	    }

	    /*
	     * Update the preedit display.
	     */
	    panel_preedit_display(
		((Xv_panel_or_item *) canvas->pew->ptxt)->private_data,
		canvas->pe_cache, TRUE);
	}
	(void) frame_kbd_use(xv_get(canvas_public, WIN_FRAME), canvas_public,
			     canvas_public);
        break;
      }

      case KBD_DONE: {
        Xv_panel_or_item	*pi;

	if (canvas->ic) {
	    pi = (Xv_panel_or_item *) canvas->pew->ptxt;
            canvas->pe_cache = panel_get_preedit(pi->private_data);
	}
	(void) frame_kbd_done(xv_get(canvas_public, WIN_FRAME), canvas_public);
      	break;
      }
#else
      case KBD_USE:
	(void) frame_kbd_use(xv_get(canvas_public, WIN_FRAME), canvas_public,
			     canvas_public);
	break;

      case KBD_DONE:
	(void) frame_kbd_done(xv_get(canvas_public, WIN_FRAME), canvas_public);
	break;

#endif /*OW_I18N*/
      default:
	break;
    }

    return (result);
}

static void
canvas_inform_resize(canvas)
    register Canvas_info *canvas;
{
    Canvas          canvas_public = CANVAS_PUBLIC(canvas);

    if (!canvas->resize_proc) {
	return;
    }
    (*canvas->resize_proc) (canvas_public, canvas->width, canvas->height);
}

/*
 * tell the client to repaint the paint window.
 */
Pkg_private
canvas_inform_repaint(canvas, paint_window)
    Canvas_info    *canvas;
    Xv_Window       paint_window;
{
    Rectlist       *win_damage, damage;

    if (!(win_damage = win_get_damage(paint_window))) {
	win_damage = &rl_null;
    }
    damage = rl_null;
    rl_copy(win_damage, &damage);

    if (xv_get(CANVAS_PUBLIC(canvas), OPENWIN_AUTO_CLEAR)) {
	canvas_clear_damage(paint_window, &damage);
    }
    if (canvas->repaint_proc) {
	if (status(canvas, x_canvas)) {
	    Xv_xrectlist    xrects;

	    /*
	     * If there is no damage on the paint window, pass NULL
	     * xrectangle array and a count of zero to let the application
	     * know that there is no clipping.
	     */
	    if (win_damage == &rl_null) {
		(*canvas->repaint_proc) (CANVAS_PUBLIC(canvas), paint_window,
				       XV_DISPLAY_FROM_WINDOW(paint_window),
				     xv_get(paint_window, XV_XID), NULL);
	    } else {
		xrects.count = win_convert_to_x_rectlist(&damage,
					  xrects.rect_array, XV_MAX_XRECTS);
		(*canvas->repaint_proc) (CANVAS_PUBLIC(canvas), paint_window,
				       XV_DISPLAY_FROM_WINDOW(paint_window),
					 xv_get(paint_window, XV_XID),
					 &xrects);
	    }
	} else {
	    (*canvas->repaint_proc) (CANVAS_PUBLIC(canvas), paint_window, &damage);
	}
    }
    rl_free(&damage);
}

/*
 * translate a canvas paint window-space event to a canvas subwindow-space
 * event.
 */
Xv_private Event          *
canvas_window_event(canvas_public, event)
    Canvas          canvas_public;
    register Event *event;
{
    Xv_Window       paint_window;
    static Event    tmp_event;  /* MAKING STATIC TO AVOID RETURNING LOCAL VARIABLE - DIXON */
    int             x, y;

    paint_window = xv_get(canvas_public, CANVAS_NTH_PAINT_WINDOW, NULL);
    if (paint_window == XV_NULL) {
	/* call xv_error */
	return (event);
    }
    tmp_event = *event;
    win_translate_xy(paint_window, canvas_public,
		     event_x(event), event_y(event), &x, &y);
    event_set_x(&tmp_event, x);
    event_set_y(&tmp_event, y);
    return (&tmp_event);
}

/*
 * translate a window-space event to a canvas-space event.
 */
Xv_private Event          *
canvas_event(canvas_public, event)
    Canvas          canvas_public;
    register Event *event;
{
    Xv_Window       paint_window;
    static Event    tmp_event;  /* MAKING STATIC TO AVOID RETURNING LOCAL VARIABLE - DIXON */
    int             x, y;

    paint_window = xv_get(canvas_public, CANVAS_NTH_PAINT_WINDOW, NULL);
    if (paint_window == XV_NULL) {
	/* call xv_error */
	return (event);
    }
    tmp_event = *event;
    win_translate_xy(paint_window, canvas_public,
		     event_x(event), event_y(event), &x, &y);
    event_set_x(&tmp_event, x);
    event_set_y(&tmp_event, y);
    return (&tmp_event);
}


/* Clear the damaged area */
static void
canvas_clear_damage(window, rl)
    Xv_Window       window;
    Rectlist       *rl;
{
    register Xv_Drawable_info *info;
    Xv_xrectlist   *clip_xrects;
    Display	   *display;
    XGCValues	    gc_values;
    unsigned long   gc_value_mask;
    Xv_Screen      screen;
    GC             *gc_list;
    
    if (!rl)
	return;
    DRAWABLE_INFO_MACRO(window, info);
    clip_xrects = screen_get_clip_rects(xv_screen(info));
    gc_value_mask = GCForeground | GCBackground | GCFunction | GCPlaneMask |
	GCSubwindowMode | GCFillStyle;
    gc_values.background = xv_bg(info);
    gc_values.function = GXcopy;
    gc_values.plane_mask = xv_plane_mask(info);
    if (gc_values.stipple = xv_get(window, WIN_BACKGROUND_PIXMAP)) {
	gc_value_mask |= GCStipple;
	gc_values.foreground = xv_fg(info);
	gc_values.fill_style = FillOpaqueStippled;
    } else {
	gc_values.foreground = xv_bg(info);
	gc_values.fill_style = FillSolid;
    }
    if (server_get_fullscreen(xv_server(info)))
	gc_values.subwindow_mode = IncludeInferiors;
    else
	gc_values.subwindow_mode = ClipByChildren;
    display = xv_display(info);
    screen = xv_screen(info);
    gc_list = (GC *)xv_get(screen, SCREEN_OLGC_LIST, window);
    XChangeGC(display, gc_list[SCREEN_NONSTD_GC], gc_value_mask,
	      &gc_values);
    XSetClipRectangles(display, gc_list[SCREEN_NONSTD_GC],
		       0, 0, clip_xrects->rect_array, clip_xrects->count,
		       Unsorted);
    XFillRectangle(display, xv_xid(info),
		   gc_list[SCREEN_NONSTD_GC], 
		   rl->rl_bound.r_left, rl->rl_bound.r_top,
		   rl->rl_bound.r_width, rl->rl_bound.r_height);
    XSetClipMask(display, gc_list[SCREEN_NONSTD_GC], None);
}
