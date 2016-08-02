#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_select.c 20.81 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <string.h>
#include <xview_private/panel_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/scrollbar.h>
#include <xview/sel_attrs.h>
#include <xview/sel_svc.h>
#include <xview/font.h>

Xv_public char xv_iso_cancel;
Xv_public char xv_iso_default_action;
Xv_public char xv_iso_input_focus_help;
Xv_public char xv_iso_next_element;
Xv_public char xv_iso_select;

extern int panel_item_destroy_flag;

static int      event_in_view_window();
static int	panel_event_is_drag();
static Item_info *panel_find_item();

#define	CTRL_D_KEY	'\004'
#define	CTRL_G_KEY	'\007'

#define UP_CURSOR_KEY           (KEY_RIGHT(8))
#define DOWN_CURSOR_KEY         (KEY_RIGHT(14))
#define RIGHT_CURSOR_KEY        (KEY_RIGHT(12))
#define LEFT_CURSOR_KEY         (KEY_RIGHT(10))

static int
panel_event_is_drag(event)
    Event	   *event;
{
    switch (event_action(event)) {
      case LOC_DRAG:
      case ACTION_DRAG_COPY:
      case ACTION_DRAG_MOVE:
      case ACTION_DRAG_PREVIEW:
	return TRUE;
      default:
	return FALSE;
    }
}


/*ARGSUSED*/
Pkg_private int
panel_duplicate_key_is_down(panel, event)
    Panel_info	   *panel;
    Event	   *event;
{
    if (event_shiftmask(event) & panel->duplicate_shiftmask)
	return TRUE;
    else
	return FALSE;
}


Pkg_private int
panel_event_is_xview_semantic(event)
    register Event *event;
{
    switch (event_action(event)) {
      case ACTION_CANCEL:   /* == ACTION_STOP */
      case ACTION_AGAIN:
      case ACTION_PROPS:
      case ACTION_UNDO:
      case ACTION_FRONT:
      case ACTION_BACK:
      case ACTION_COPY:
      case ACTION_OPEN:
      case ACTION_CLOSE:
      case ACTION_PASTE:
      case ACTION_FIND_BACKWARD:
      case ACTION_FIND_FORWARD:
      case ACTION_CUT:
	return 1;
      default:
	return 0;
    }
}


Pkg_private int
panel_erase_action(event)
    Event	   *event;
{
    switch (event_action(event)) {
      case ACTION_ERASE_CHAR_BACKWARD:
      case ACTION_ERASE_CHAR_FORWARD:
      case ACTION_ERASE_WORD_BACKWARD:
      case ACTION_ERASE_WORD_FORWARD:
      case ACTION_ERASE_LINE_BACKWARD:
      case ACTION_ERASE_LINE_END:
      case ACTION_ERASE_LINE:
	return 1;
      default:
	return 0;
    }
}


Pkg_private int
panel_navigation_action(event)
    Event	   *event;
{
    switch (event_action(event)) {
      case ACTION_GO_CHAR_BACKWARD:
      case ACTION_GO_CHAR_FORWARD:
      case ACTION_GO_WORD_BACKWARD:
      case ACTION_GO_WORD_END:
      case ACTION_GO_WORD_FORWARD:
      case ACTION_GO_LINE_BACKWARD:
      case ACTION_GO_LINE_END:
      case ACTION_GO_LINE_FORWARD:
      case ACTION_UP:
      case ACTION_DOWN:
	return 1;
      default:
	return 0;
    }
}


Pkg_private Notify_value
panel_default_event(p_public, event, arg)
    Panel           p_public;
    register Event *event;
    Notify_arg      arg;
{
    Item_info	   *ip;
    Item_info      *new = NULL;
    Xv_Window       paint_window = event_window(event);
    register Panel_info *panel;
    Panel           panel_public;

    XV_OBJECT_TO_STANDARD(p_public, "panel_default_event", panel_public);
    panel = PANEL_PRIVATE(panel_public);
    if (!is_panel(panel)) {
	panel = ITEM_PRIVATE(panel_public)->panel;
    }

    if (!panel->status.pointer_grabbed && !panel->status.current_item_active) {

	if (panel->kbd_focus_item && !wants_iso(panel->kbd_focus_item)) {
	    if (event_action(event) == xv_iso_cancel)
		event_set_action(event, ACTION_CANCEL);
	    else if (event_action(event) == xv_iso_default_action)
		event_set_action(event, ACTION_DEFAULT_ACTION);
	    else if (event_action(event) == xv_iso_input_focus_help)
		event_set_action(event, ACTION_INPUT_FOCUS_HELP);
	    else if (event_action(event) == xv_iso_next_element) {
		if (event_shift_is_down(event))
		    event_set_action(event, ACTION_PREVIOUS_ELEMENT);
		else
		    event_set_action(event, ACTION_NEXT_ELEMENT);
	    } else if (event_action(event) == xv_iso_select)
		event_set_action(event, ACTION_SELECT);
	}

	switch (event_action(event)) {
    	  case ACTION_NEXT_ELEMENT:
	    if (event_is_down(event)) {
		ip = panel_next_kbd_focus(panel, FALSE);
		if (ip)
		    panel_set_kbd_focus(panel, ip);
		else if (panel->paint_window->view)
		    return NOTIFY_IGNORED; /* let canvas handle event */
		else
		    xv_set(xv_get(panel_public, WIN_FRAME),
			   FRAME_NEXT_PANE,
			   NULL);
	    }
	    return (int) NOTIFY_DONE;
	  case ACTION_PREVIOUS_ELEMENT:
	    if (event_is_down(event)) {
		ip = panel_previous_kbd_focus(panel, FALSE);
		if (ip)
		    panel_set_kbd_focus(panel, ip);
		else if (panel->paint_window->view)
		    return NOTIFY_IGNORED; /* let canvas handle event */
		else
		    xv_set(xv_get(panel_public, WIN_FRAME),
			   FRAME_PREVIOUS_ELEMENT,
			   NULL);
	    }
	    return (int) NOTIFY_DONE;
	  case ACTION_PANEL_START:
	    if (event_is_down(event) && panel->items) {
		for (ip = panel->items; ip; ip = ip->next) {
		    if (wants_key(ip) && !hidden(ip) && !inactive(ip)) {
			panel_set_kbd_focus(panel, ip);
			return (int) NOTIFY_DONE;
		    }
		}
	    }
	    return (int) NOTIFY_DONE;
	  case ACTION_PANEL_END:
	    if (event_is_down(event) && panel->items) {
		for (ip = panel->items; ip->next; ip = ip->next);
		for (; ip; ip = ip->previous) {
		    if (wants_key(ip) && !hidden(ip) && !inactive(ip)) {
			panel_set_kbd_focus(panel, ip);
			return (int) NOTIFY_DONE;
		    }
		}
	    }
	    return (int) NOTIFY_DONE;
	  case ACTION_JUMP_MOUSE_TO_INPUT_FOCUS:
	    if (panel->kbd_focus_item) {
		xv_set(panel->focus_pw,
		       WIN_MOUSE_XY,
			    panel->kbd_focus_item->rect.r_left,
			    panel->kbd_focus_item->rect.r_top,
		       NULL);
	    }
	    return (int) NOTIFY_DONE;
	  case ACTION_DEFAULT_ACTION:
	    if (panel->kbd_focus_item && panel->kbd_focus_item->menu)
		event_set_action(event, ACTION_MENU);
	    else if (event_is_down(event) && panel->default_item) {


		/* make sure the focus item sees the down event */
		if (panel->kbd_focus_item)
		    panel_handle_event(ITEM_PUBLIC(panel->kbd_focus_item), event);

		event_set_action(event, ACTION_SELECT);
		new = ITEM_PRIVATE(panel->default_item);
		if (new != panel->current) {
		    if (panel->current)
			panel_cancel(ITEM_PUBLIC(panel->current), event);
		    panel->current = new;
		}

		panel_handle_event(ITEM_PUBLIC(new), event); /* SELECT-down */
		event_set_up(event);
		panel_handle_event(ITEM_PUBLIC(new), event); /* SELECT-up */
		return (int) NOTIFY_DONE;
	    }
	    break;
	  case ACTION_ACCELERATOR: /* only received on down (KeyPress) events */
	    event_set_action(event, ACTION_SELECT);
	    new = ITEM_PRIVATE((Panel_item)arg);
	    if (new != panel->current) {
		if (panel->current)
		    panel_cancel(ITEM_PUBLIC(panel->current), event);
		panel->current = new;
	    }
	    panel_handle_event(ITEM_PUBLIC(new), event); /* SELECT-down */
	    event_set_up(event);
	    panel_handle_event(ITEM_PUBLIC(new), event); /* SELECT-up */
	    return (int) NOTIFY_DONE;
	  case SCROLLBAR_REQUEST:
	    new = (Item_info *) xv_get(arg, XV_KEY_DATA, PANEL_LIST);
	    break;
	  case ACTION_DRAG_COPY:
	  case ACTION_DRAG_MOVE:
	    if (dnd_is_forwarded(event))
		new = panel->default_drop_site_item;
	    else
		new = panel_find_item(panel, event);
	    break;
	  case ACTION_PASTE:	/* for quick-copy */
	    /* panel->current is active item, panel will propagate paste
	     * otherwise, preview gets canceled below
	     */
	    new = (Item_info *) panel;
	    break;
	  default:
	    /* Find out who's under the locator */
	    new = panel_find_item(panel, event);
	    break;
	}
    } else
	new = panel->current;

    /* Use the panel if not over some item */
    if (!new)
	new = (Item_info *) panel;

    /* Set Quick Move status */
    if (event_is_quick_move(event))
	panel->status.quick_move = TRUE;
    else
	panel->status.quick_move = FALSE;

    /* cancel the old item if needed */
    if ( new != panel->current ) {
	if (panel->current &&
	    ((panel->current != (Item_info *) panel &&
	      panel->current->item_type == PANEL_DROP_TARGET_ITEM) ||
	     (event_action(event) != ACTION_DRAG_COPY &&
	      event_action(event) != ACTION_DRAG_MOVE &&
	      event_action(event) != ACTION_DRAG_PREVIEW)))
	    panel_cancel(ITEM_PUBLIC(panel->current), event);
	panel->current = new;
    }
    /* If help event, call help request function. */
    if (event_action(event) == ACTION_HELP ||
	event_action(event) == ACTION_MORE_HELP ||
	event_action(event) == ACTION_TEXT_HELP ||
	event_action(event) == ACTION_MORE_TEXT_HELP ||
	event_action(event) == ACTION_INPUT_FOCUS_HELP) {
#ifdef OW_I18N
    /*  For some reason the down event is not passed
     *  back to panel, so for now we're looking for
     *  the up event.  When the down event is passed
     *  to panel again we should remove this code.
     *  As they say, a hack for now.
     */
	if (event_is_up(event))
#else
	if (event_is_down(event))
#endif /* OW_I18N */
	{
	    char   *panel_help = (char *) xv_get(panel_public, XV_HELP_DATA);
	    char   *item_help = 0;

	    if (event_action(event) == ACTION_INPUT_FOCUS_HELP)
		new = panel->kbd_focus_item;
	    if (new && new != (Item_info *) panel)
		item_help = (char *) xv_get(ITEM_PUBLIC(new), XV_HELP_DATA);
	    if (item_help)
		panel_help = item_help;
	    if (panel_help) {
		xv_help_show(paint_window, panel_help, event);
		return (int) NOTIFY_DONE;
	    } else
		return (int) NOTIFY_IGNORED;
	}
    } else if (!event_is_button(event) &&
	       !panel_event_is_drag(event) &&
	       event_action(event) != SCROLLBAR_REQUEST) {
	if (panel->kbd_focus_item)
	    /* Give the key event to the keyboard focus item */
	    panel_handle_event(ITEM_PUBLIC(panel->kbd_focus_item), event);
	else if (wants_key(panel))
	    /* Give the key event to the panel background proc */
	    panel_handle_event(PANEL_PUBLIC(panel), event);
    } else {
	/* Give the non-key event to the item under the pointer */
	(void) panel_handle_event(ITEM_PUBLIC(new), event);
    }
    return (int) NOTIFY_DONE;
}


Sv1_public void
panel_handle_event(client_object, event)
    Panel_item      client_object;	/* could be a Panel */
    Event          *event;
{
    Item_info      *object = ITEM_PRIVATE(client_object);

    if (!object || !object->ops.panel_op_handle_event)
	return;

    /* this flag can only be set for panel items */
    if ( post_events(object) )
	notify_post_event(client_object, (Notify_event)event, NOTIFY_IMMEDIATE);
    else
	(*object->ops.panel_op_handle_event) (client_object, event);
}


Sv1_public void
panel_begin_preview(client_object, event)
    Panel_item      client_object;	/* could be a Panel */
    Event          *event;
{
    Item_info      *object = ITEM_PRIVATE(client_object);

    if (!object)
	return;

    if (object->ops.panel_op_begin_preview)
	(*object->ops.panel_op_begin_preview) (client_object, event);
}


Sv1_public void
panel_update_preview(client_object, event)
    Panel_item      client_object;	/* could be a Panel */
    Event          *event;
{
    Item_info      *object = ITEM_PRIVATE(client_object);

    if (!object)
	return;

    if (object->ops.panel_op_update_preview)
	(*object->ops.panel_op_update_preview) (client_object, event);
}


Sv1_public void
panel_accept_preview(client_object, event)
    Panel_item      client_object;	/* could be a Panel */
    Event          *event;
{
    Item_info      *object = ITEM_PRIVATE(client_object);
    Panel_info     *panel;

    if (!object)
	return;

    panel_item_destroy_flag = 0;
    if (object->ops.panel_op_accept_preview) {
	(*object->ops.panel_op_accept_preview) (client_object, event);
	if (panel_item_destroy_flag == 2) {
	    panel_item_destroy_flag = 0;
	    return;
	}
    }
    panel_item_destroy_flag = 0;

    if (is_item(object))
	panel = object->panel;
    else
	panel = PANEL_PRIVATE(client_object);
    panel->current = NULL;
}


Sv1_public void
panel_cancel_preview(client_object, event)
    Panel_item      client_object;	/* could be a Panel */
    Event          *event;
{
    Item_info      *object = ITEM_PRIVATE(client_object);
    Panel_info     *panel;

    if (!object)
	return;

    if (object->ops.panel_op_cancel_preview)
	(*object->ops.panel_op_cancel_preview) (client_object, event);

    if (is_item(object))
	panel = object->panel;
    else
	panel = PANEL_PRIVATE(client_object);
    panel->current = NULL;
}


Sv1_public void
panel_accept_menu(client_object, event)
    Panel_item      client_object;	/* could be a Panel */
    Event          *event;
{
    Item_info      *object = ITEM_PRIVATE(client_object);
    Panel_info     *panel;

    if (!object)
	return;

    if (event_is_down(event)) {
	if (object->ops.panel_op_accept_menu)
	    (*object->ops.panel_op_accept_menu) (client_object, event);
    } else {
	if (is_item(object))
	    panel = object->panel;
	else
	    panel = PANEL_PRIVATE(client_object);
	panel->current = NULL;
    }
}


Sv1_public void
panel_accept_key(client_object, event)
    Panel_item      client_object;	/* could be a Panel */
    Event          *event;
{
    Item_info      *object = ITEM_PRIVATE(client_object);

    if (!object)
	return;

    if (object->ops.panel_op_accept_key)
	(*object->ops.panel_op_accept_key) (client_object, event);
}


Sv1_public void
panel_cancel(client_object, event)
    Panel_item      client_object;	/* could be a Panel */
    Event          *event;
{
    Event           cancel_event;

    if (!client_object)
	return;

    cancel_event = *event;
    cancel_event.action = PANEL_EVENT_CANCEL;
    (void) panel_handle_event(client_object, &cancel_event);
}


Xv_public void
panel_default_handle_event(client_object, event)
    Panel_item      client_object;	/* could be a Panel */
    register Event *event;
{
    int             accept;
    Item_info      *object = ITEM_PRIVATE(client_object);
    Item_info      *ip = NULL;
    Panel_info     *panel = PANEL_PRIVATE(client_object);

    if (is_item(object)) {
	if (inactive(object) || busy(object))
	    return;
	ip = object;
	panel = ip->panel;
    }
    switch (event_action(event)) {
      case ACTION_ADJUST:	/* middle button up or down */
	if (panel->status.current_item_active)
	    break;   /* ignore ADJUST during active item operation */
	if (!ip || !wants_adjust(ip)) {
	    panel_user_error(object, event);
	    if (event_is_up(event))
		panel->current = NULL;
	    break;
	} /* else execute ACTION_SELECT code */

      case ACTION_SELECT:	/* left button up or down */
	if (event_action(event) == ACTION_SELECT &&
	    panel->status.select_displays_menu &&
	    !panel->status.current_item_active &&
	    ip && ip->menu) {
	    /* SELECT over menu button => show menu */
	    (void) panel_accept_menu(client_object, event);
	    break;
	}
	if (event_is_up(event)) {
	    if (ip) {
		ip->flags &= ~PREVIEWING;
		/*
		 * If the SELECT button went up inside any of the panel's
		 * view windows, then accept the preview; else, cancel the
		 * preview.
		 */
		accept = !event_is_button(event) ||
		    event_in_view_window(panel, event);
	    } else
		accept = FALSE;
	    if (accept)
		panel_accept_preview(client_object, event);
	    else
		panel_cancel_preview(client_object, event);
	} else if (ip) {
	    /* SELECT or ADJUST down over an item: begin preview */
	    if (ip->item_type == PANEL_TEXT_ITEM &&
		!panel->status.has_input_focus)
		ip->flags |= PREVIEWING;
	    /* Process SELECT or ADJUST down event */

	    /* Move kbd focus to this item if we're not doing a secondary
	     * selection (i.e., Quick Move or Quick Copy), and the kbd focus
	     * is not already there.
	     */
	    if (event_is_button(event) &&
		(event_flags(event) & (IE_QUICK_MOVE | IE_QUICK_COPY)) == 0 &&
		panel->kbd_focus_item != ip &&
		wants_key(ip) && !hidden(ip) && !inactive(ip)) {
		/* Move Location Cursor to current item */
		if (panel->status.has_input_focus)
		    panel_set_kbd_focus(panel, ip);
		else {
		    panel->kbd_focus_item = ip;
		    panel->status.focus_item_set = TRUE;
		}
	    }
	    panel_begin_preview(client_object, event);
	} else if (panel->sel_holder[PANEL_SEL_PRIMARY]) {
	    /* SELECT-down over the panel background when there's a
	     * primary selection: lose the primary selection.
	     */
	    xv_set(panel->sel_owner[PANEL_SEL_PRIMARY], SEL_OWN, FALSE, NULL);
	}
	break;

      case ACTION_MENU:	/* right button up or down */
	if (panel->status.current_item_active)
	    break;   /* ignore MENU during active item operation */
#ifdef NO_BINARY_COMPATIBILITY
	if (!ip || !ip->menu) {
	    panel_user_error(object, event);
	    if (event_is_up(event))
		panel->current = NULL;
	} else
#endif /* NO_BINARY_COMPATIBILITY */
	    (void) panel_accept_menu(client_object, event);
	break;

      case LOC_DRAG:		/* left, middle, or right drag */
	if (action_select_is_down(event) || action_adjust_is_down(event))
	    (void) panel_update_preview(client_object, event);
	if (action_menu_is_down(event))
	    panel_accept_menu(client_object, event);
	break;

      case PANEL_EVENT_CANCEL:
	if (panel->status.current_item_active) {
	    event_set_id(event, LOC_DRAG);	/* pass more events */
	    (void) panel_update_preview(client_object, event);
	    break;
	} else
	    (void) panel_cancel_preview(client_object, event);
	break;

      default:	/* some other event */
	if (event_is_iso(event) ||
	    (event_is_key_right(event) && event_is_down(event)) ||
	    panel_erase_action(event) || panel_navigation_action(event) ||
	    panel_event_is_xview_semantic(event))
	    (void) panel_accept_key(client_object, event);
	break;
    }
}

static Item_info *
panel_find_item(panel, event)
    Panel_info     *panel;
    Event          *event;
{
    register Item_info *ip = panel->current;
    register int    x = event_x(event);
    register int    y = event_y(event);

    if (!panel->items || !event_in_view_window(panel, event))
	return NULL;

    if (ip && is_item(ip) && !hidden(ip) &&
	(rect_includespoint(&ip->rect, x, y) ||
	 (previewing(ip) && ip->item_type == PANEL_TEXT_ITEM)))
	return ip;

    for (ip = (hidden(panel->items) || deaf(panel->items)) ?
	 panel_successor(panel->items) : panel->items;
	 ip && !rect_includespoint(&ip->rect, x, y);
	 ip = panel_successor(ip));

    return ip;
}


/*
 * If the event occurred inside any of the panel's view windows, then return
 * TRUE, else return FALSE.
 */
static int
event_in_view_window(panel, event)
    register Panel_info     *panel;
    register Event          *event;
{
    register Xv_Window       pw;

    PANEL_EACH_PAINT_WINDOW(panel, pw)
	if (rect_includespoint(panel_viewable_rect(panel, pw),
			       event_x(event), event_y(event)))
	    return TRUE;
    PANEL_END_EACH_PAINT_WINDOW
    return FALSE;
}


/*
 * Base handler for Item's that request events to be posted
 * (implementing PANEL_POST_EVENTS).
 */
Xv_private Notify_value 
panel_base_event_handler(client, event, arg, type)
     Notify_client client;
     Notify_event event;
     Notify_arg arg;
     Notify_event_type type; 
{
    Item_info *object = ITEM_PRIVATE(client);

    /* 
     * do what panel_handle_event() would have, had we not
     * posted the event through the notifier.
     */
    if (object->ops.panel_op_handle_event)
	(*object->ops.panel_op_handle_event) ((Panel_item) client, event);

    /*
     * ASSume the Panel handled the event, since the Panel 
     * doesn't return any useful status information...
     */
    return NOTIFY_DONE;
}
