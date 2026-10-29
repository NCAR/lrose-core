#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_event.c 20.62 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */


#include <xview_private/panel_impl.h>
#include <xview/font.h>
#include <xview_private/draw_impl.h>

static Notify_value panel_itimer_expired();

/* ARGSUSED */
Pkg_private     Notify_value
panel_notify_panel_event(window, event, arg, type)
    Xv_Window       window;
    register Event *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    Panel_info     *panel = PANEL_PRIVATE(window);

    switch (event_action(event)) {
      case ACTION_RESCALE:
	panel_refont(panel, (int) arg);
	return notify_next_event_func(window, (Notify_event)event, arg, type);
      default:
	return notify_next_event_func(window, (Notify_event)event, arg, type);
    }
}


Pkg_private     Notify_value
panel_notify_event(paint_window, event, arg, type)
    Xv_Window       paint_window;
    register Event *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    register Panel_info *panel = (Panel_info *) xv_get(paint_window,
					    XV_KEY_DATA, panel_context_key);
    Item_info	   *ip;
    int		   event_handled = TRUE;

    switch (event_action(event)) {
      case WIN_REPAINT:
	if (!panel->paint_window->view) {
	    /* Panel */
	    Rectlist       *win_damage, damage;

	    if (!(win_damage = win_get_damage(paint_window))) {
		win_damage = &rl_null;
	    }
	    damage = rl_null;
	    rl_copy(win_damage, &damage);
	    panel_redisplay(paint_window, paint_window, &damage);
	    rl_free(&damage);
	}
	break;

      case KBD_USE:
#ifdef  OW_I18N
	/* This probably should not be set here, because
	 * the XNFocusWindow does not change with the
	 * keyboard focus.  Unless splitting views
	 * in the scrollable panel makes a difference.
	 * Possibly this should be set in panel_init,
	 * when ic is being created/cached
	 */
        if ((xv_get(PANEL_PUBLIC(panel), WIN_USE_IM)) && panel->ic ) {
                Xv_Drawable_info *info;

                DRAWABLE_INFO_MACRO(paint_window, info);
		window_set_ic_focus_win(paint_window, panel->ic, xv_xid(info));
	}
#endif  /* OW_I18N */
	frame_kbd_use(xv_get(PANEL_PUBLIC(panel), WIN_FRAME),
		      PANEL_PUBLIC(panel), PANEL_PUBLIC(panel));
	panel->focus_pw = paint_window;
	if (!panel->kbd_focus_item) {
	    /* Set keyboard focus to first item that wants it. */
	    panel->kbd_focus_item = panel->last_item;
	    ip = panel_next_kbd_focus(panel, TRUE);
	    if (ip)
		panel->kbd_focus_item = ip;
	    else if (panel->kbd_focus_item &&
		     (!wants_key(panel->kbd_focus_item) ||
		      hidden(panel->kbd_focus_item) ||
		      inactive(panel->kbd_focus_item))) {
		/* Note: The following only occurs if there is no
		 * panel item that wants keyboard focus, and
		 * PANEL_ACCEPT_KEYSTROKE has been set to TRUE on the panel.
		 * DevGuide does this.
		 */ 
		panel->kbd_focus_item = NULL;
	    }
	}
	if (panel->kbd_focus_item &&
	    xv_get(ITEM_PUBLIC(panel->kbd_focus_item), XV_FOCUS_RANK) !=
		XV_FOCUS_PRIMARY) {
	    if (panel->status.focus_item_set)
		panel->status.focus_item_set = FALSE;
	    else {
		/* Set the focus to the primary focus item that most recently
		 * had it.  If no primary focus items ever had the focus, then
		 * set the focus to the first primary focus item.
		 * If there are no primary focus items, then leave the focus
		 * where it is.
		 */
		if (!panel->primary_focus_item) {
		    /* Find first primary focus item */
		    for (ip = panel->items;
			 ip;
			 ip = ip->next) {
			if (wants_key(ip) && !hidden(ip) && !inactive(ip) &&
			    xv_get(ITEM_PUBLIC(ip), XV_FOCUS_RANK) ==
			    XV_FOCUS_PRIMARY) {
			    panel->primary_focus_item = ip;
			    break;
			}
		    }
		}
		if (panel->primary_focus_item)
		    panel_set_kbd_focus(panel, panel->primary_focus_item);
	    }
	}
	if (panel->kbd_focus_item &&
	    panel->kbd_focus_item->item_type == PANEL_TEXT_ITEM) {
	    /* Clear gray diamond caret */
	    panel_text_caret_on(panel, FALSE);
	    if (panel->status.blinking)
		panel_itimer_set(PANEL_PUBLIC(panel), panel->timer_full);
	}
	panel->caret = OLGX_ACTIVE_CARET;
	panel->caret_ascent = panel->active_caret_ascent;
	panel->caret_height = panel->active_caret_height;
	panel->caret_width = panel->active_caret_width;
	panel->status.has_input_focus = TRUE;
	if (panel->kbd_focus_item && !deaf(panel->kbd_focus_item))
	    panel_accept_kbd_focus(panel);
	break;

      case KBD_DONE:
	(void) frame_kbd_done((Xv_Window) xv_get(PANEL_PUBLIC(panel),
					   WIN_FRAME), PANEL_PUBLIC(panel));
	if (panel->kbd_focus_item &&
	    panel->kbd_focus_item->item_type == PANEL_TEXT_ITEM) {
	    panel_text_caret_on(panel, FALSE);
	    if (panel->status.blinking)
		panel_itimer_set(PANEL_PUBLIC(panel), NOTIFY_NO_ITIMER);
	}
	panel->caret = OLGX_INACTIVE_CARET;
	panel->caret_ascent = panel->inactive_caret_ascent;
	panel->caret_height = panel->inactive_caret_height;
	panel->caret_width = panel->inactive_caret_width;
	panel->status.has_input_focus = FALSE;
	if (panel->kbd_focus_item) {
	    if (panel->kbd_focus_item->item_type == PANEL_TEXT_ITEM)
		panel_text_caret_on(panel, TRUE);
	    else if (!deaf(panel->kbd_focus_item))
		panel_yield_kbd_focus(panel);
	}
	break;

      case ACTION_NEXT_PANE:
	if (!panel->paint_window->view && event_is_down(event)) {
	    xv_set(xv_get(PANEL_PUBLIC(panel), WIN_FRAME),
		   FRAME_NEXT_PANE,
		   NULL);
	}
	break;

      case ACTION_PREVIOUS_PANE:
	if (!panel->paint_window->view && event_is_down(event)) {
	    xv_set(xv_get(PANEL_PUBLIC(panel), WIN_FRAME),
		   FRAME_PREVIOUS_PANE,
		   NULL);
	}
	break;

      case ACTION_RESCALE:{
	    panel_refont(panel, (int) arg);
	    break;
	}

      /* Pass on selection events to the next event handler */
      case SEL_CLEAR:
      case SEL_REQUEST:
      case SEL_NOTIFY:
	break;

      case WIN_RESIZE:
	/* Call the resize routine of each top-level (i.e., application created)
	 * panel item.
	 */
	for (ip = panel->items; ip; ip = ip->next) {
	    if (!ip->owner && ip->ops.panel_op_resize)
		(*ip->ops.panel_op_resize)(ITEM_PUBLIC(ip));
	}
#ifdef OW_I18N
#ifdef FULL_R5
        if (panel->ic) {
            XRectangle	    x_rect;
    	    XVaNestedList   preedit_nested_list;
    	    
    	    preedit_nested_list = NULL;
    	    
            if  (panel->xim_style & (XIMPreeditPosition | XIMPreeditArea)) {
                if ((panel->kbd_focus_item) && (panel->kbd_focus_item->item_type == PANEL_TEXT_ITEM)) {
                    x_rect.x = panel->kbd_focus_item->value_rect.r_left;
                    x_rect.y = panel->kbd_focus_item->value_rect.r_top;
                    x_rect.width = panel->kbd_focus_item->value_rect.r_width;
                    x_rect.height = panel->kbd_focus_item->value_rect.r_height;
      	
                    preedit_nested_list = XVaCreateNestedList(NULL, 
					     XNArea, &x_rect, 
					     NULL);
		}
            }
        
	    if (preedit_nested_list) {
	        XSetICValues(panel->ic, XNPreeditAttributes, preedit_nested_list, 
        	     NULL);
        	XFree(preedit_nested_list);
	    }
        }
#endif /* FULL_R5 */	    	
#endif /* OW_I18N */	    	
	break;

      /* Ignore graphics expose events */
      case WIN_GRAPHICS_EXPOSE:
      case WIN_NO_EXPOSE:
      /* Ignore modifier KeyPress & KeyRelease events */
      case SHIFT_LEFT:
      case SHIFT_RIGHT:
      case SHIFT_LEFTCTRL:
      case SHIFT_RIGHTCTRL:
      case SHIFT_META:
	break;

      default:
	event_handled = FALSE;
	break;
    }

    /* Process the event and send it on (for WIN_EVENT_PROC) */
    if ( !event_handled )
	(void) panel_default_event(PANEL_PUBLIC(panel), event, arg);

    return notify_next_event_func(paint_window, (Notify_event)event,
                                      arg, type);
}


/* ARGSUSED */
static          Notify_value
panel_itimer_expired(panel_public, which)
    Panel           panel_public;
    int             which;
{
    register Panel_info *panel = PANEL_PRIVATE(panel_public);

    if (panel->status.blinking) {
	if (panel->kbd_focus_item &&
	    panel->kbd_focus_item->item_type == PANEL_TEXT_ITEM)
	    panel_text_caret_on(panel, panel->caret_on ? FALSE : TRUE);
    } else {
	/* turn itimer off */
	panel_itimer_set(panel_public, NOTIFY_NO_ITIMER);
    }
    return NOTIFY_DONE;
}

Pkg_private void
panel_itimer_set(panel_public, timer)
    register Panel panel_public;
    struct itimerval timer;
{

    (void) notify_set_itimer_func((Notify_client) panel_public,
	 panel_itimer_expired, ITIMER_REAL, &timer, (struct itimerval *) 0);
}
