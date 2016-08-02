#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_ambtn.c 1.21 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/panel_impl.h>
#include <xview/openmenu.h>
#include <xview_private/draw_impl.h>

/*
 * Abbreviated Menu Button Panel Item
 */


#define AMBTN_PRIVATE(item) \
	XV_PRIVATE(Ambtn_info, Xv_panel_ambtn, item)
#define AMBTN_FROM_ITEM(ip)	AMBTN_PRIVATE(ITEM_PUBLIC(ip))

/* Declare all ambtn item handler procedures used in the Ops Vector Table */
static void     ambtn_begin_preview(), ambtn_cancel_preview(),
		ambtn_accept_preview(), ambtn_accept_menu(), ambtn_accept_key(),
		ambtn_paint(), ambtn_remove(), ambtn_accept_kbd_focus(),
		ambtn_yield_kbd_focus();

/* Local routines */
static void ambtn_menu_busy_proc();
static void ambtn_menu_done_proc();
static void ambtn_paint_value();


/*
 * Panel Operations Vector Table for this item.
 * If any of the operations do not apply, then use NULL.
 */
static Panel_ops ops = {
    panel_default_handle_event,		/* handle_event() */
    ambtn_begin_preview,		/* begin_preview() */
    NULL,				/* update_preview() */
    ambtn_cancel_preview,		/* cancel_preview() */
    ambtn_accept_preview,		/* accept_preview() */
    ambtn_accept_menu,			/* accept_menu() */
    ambtn_accept_key,			/* accept_key() */
    panel_default_clear_item,		/* clear() */
    ambtn_paint,			/* paint() */
    NULL,				/* resize() */
    ambtn_remove,			/* remove() */
    NULL,				/* restore() */
    NULL,				/* layout() */
    ambtn_accept_kbd_focus,		/* accept_kbd_focus() */
    ambtn_yield_kbd_focus,		/* yield_kbd_focus() */
    NULL				/* extension: reserved for future use */
};


typedef struct ambtn_info {
    Panel_item      public_self;/* back pointer to object */
} Ambtn_info;


/* Item specific definitions */
#define AMB_OFFSET	4	/* gap between label and Abbrev. Menu Button */


/* ========================================================================= */

/* -------------------- XView Functions  -------------------- */
/*ARGSUSED*/
Pkg_private int
panel_ambtn_init(panel_public, item_public, avlist)
    Panel           panel_public;
    Panel_item      item_public;
    Attr_avlist     avlist;
{
    Panel_info     *panel = PANEL_PRIVATE(panel_public);
    register Item_info *ip = ITEM_PRIVATE(item_public);
    Xv_panel_ambtn *item_object = (Xv_panel_ambtn *) item_public;
    Ambtn_info	   *dp;

    dp = xv_alloc(Ambtn_info);

    item_object->private_data = (Xv_opaque) dp;
    dp->public_self = item_public;
    
    ip->ops = ops;
    if (panel->event_proc)
	ip->ops.panel_op_handle_event = (void (*) ()) panel->event_proc;
    ip->item_type = PANEL_ABBREV_MENU_BUTTON_ITEM;
    panel_set_bold_label_font(ip);

    if (panel->status.mouseless)
	ip->flags |= WANTS_KEY;

    return XV_OK;
}


Pkg_private Xv_opaque
panel_ambtn_set_avlist(item_public, avlist)
    Panel_item	    item_public;
    Attr_avlist	    avlist;
{
    Item_info	   *ip = ITEM_PRIVATE(item_public);
    Xv_opaque       result;

    /* Call generic item set code to handle layout attributes.
     * Prevent panel_redisplay_item from being called in item_set_avlist.
     */
    if (*avlist != XV_END_CREATE) {
        ip->panel->no_redisplay_item = TRUE;
        result = xv_super_set_avlist(item_public, &xv_panel_ambtn_pkg, avlist);
        ip->panel->no_redisplay_item = FALSE;
        if (result != XV_OK)
            return result;
    }
 
    /* Parse Attribute-Value List.  Complete initialization upon
     * receipt of XV_END_CREATE.
     */
    for ( ; *avlist; avlist = attr_next(avlist)) {
        switch ((int) avlist[0]) {
	  case XV_END_CREATE:
	    ip->value_rect.r_width = AMB_OFFSET +
		Abbrev_MenuButton_Width(ip->panel->ginfo);
	    ip->value_rect.r_height =
		Abbrev_MenuButton_Height(ip->panel->ginfo);
	    ip->rect = panel_enclosing_rect(&ip->label_rect, &ip->value_rect);
	    break;
	  default:
	    break;
	}
    }
    return XV_OK;	/* return XV_ERROR if something went wrong... */
}


/*ARGSUSED*/
Pkg_private int
panel_ambtn_destroy(item_public, status)
    Panel_item      item_public;
    Destroy_status  status;
{
    Ambtn_info	   *dp = AMBTN_PRIVATE(item_public);

    if ((status == DESTROY_CHECKING) || (status == DESTROY_SAVE_YOURSELF))
	return XV_OK;
    ambtn_remove(item_public);
    free(dp);
    return XV_OK;
}



/* --------------------  Panel Item Operations  -------------------- */
/*ARGSUSED*/
static void
ambtn_begin_preview(item_public, event)	 /* + update_preview() */
    Panel_item	    item_public;
    Event          *event;
{
    int		    amb_invoked;
    Rect	    amb_rect;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    
    if (!event_is_button(event))
	amb_invoked = TRUE;
    else {
	rect_construct(&amb_rect,
		       ip->value_rect.r_left + AMB_OFFSET,
		       ip->value_rect.r_top,
		       ip->value_rect.r_width - AMB_OFFSET,
		       ip->value_rect.r_height);
	amb_invoked =  rect_includespoint(&amb_rect, event_x(event),
				          event_y(event));
    }
    if (amb_invoked) {
	ambtn_paint_value(ip, OLGX_INVOKED);
	ip->flags |= INVOKED;
    }
}


/*ARGSUSED*/
static void
ambtn_cancel_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);

    if (invoked(ip)) {
	ip->flags &= ~INVOKED;
	ambtn_paint_value(ip,
	    ip->panel->status.three_d ? OLGX_NORMAL : OLGX_ERASE | OLGX_NORMAL);
    }
}


/*ARGSUSED*/
static void
ambtn_accept_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);

    if (invoked(ip)) {
	ip->flags &= ~INVOKED;
	ambtn_paint_value(ip, OLGX_BUSY);
	panel_btn_accepted(ip, event);
	if (!hidden(ip))
	    ambtn_paint_value(ip,
		ip->panel->status.three_d ? OLGX_NORMAL :
		OLGX_ERASE | OLGX_NORMAL);
    }
}


/*ARGSUSED*/
static void
ambtn_accept_menu(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Rect	    rect;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    
    rect_construct(&rect,
		   ip->value_rect.r_left + AMB_OFFSET,
		   ip->value_rect.r_top,
		   ip->value_rect.r_width - AMB_OFFSET,
		   ip->value_rect.r_height);
    if (event_is_button(event) &&
	!rect_includespoint(&rect, event_x(event), event_y(event)))
	return;

    ambtn_paint_value(ip, OLGX_INVOKED);

    /*
     * Notify the client.  This callback allows the client to dynamically
     * generate the menu.
     */
    (*ip->notify) (ITEM_PUBLIC(ip), event);

    /*
     * Save public panel handle and current menu done proc. Switch to
     * Abbreviated Menu Button's menu done proc.
     */
    xv_set(ip->menu,
	   XV_KEY_DATA, PANEL_FIRST_ITEM, ip,
	   XV_KEY_DATA, MENU_DONE_PROC, xv_get(ip->menu, MENU_DONE_PROC),
	   MENU_BUSY_PROC, ambtn_menu_busy_proc,
	   MENU_DONE_PROC, ambtn_menu_done_proc,
	   NULL);

    /* Show the menu */
    rect.r_width = 0;	/* paint menu flush left */
    ip->panel->status.current_item_active = TRUE;
    menu_show(ip->menu, event_window(event), event,
	      MENU_POSITION_RECT, &rect,
	      MENU_PULLDOWN, TRUE,
	      NULL);
}


static void
ambtn_accept_key(item_public, event)
    Panel_item	    item_public;
    Event	   *event;
{
    Item_info	   *ip = ITEM_PRIVATE(item_public);

    if (ip->menu && event_action(event) == ACTION_DOWN)
	panel_accept_menu(item_public, event);
}


/*ARGSUSED*/
static void
ambtn_paint(item_public)
    Panel_item	    item_public;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);

    /* Paint the label */
    panel_paint_image(ip->panel, &ip->label, &ip->label_rect, inactive(ip),
		      ip->color_index);

    /* Paint the value */
    ambtn_paint_value(ip,
	ip->panel->status.three_d ? OLGX_NORMAL : OLGX_ERASE | OLGX_NORMAL);
}


static void
ambtn_remove(item_public)
    Panel_item	    item_public;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Panel_info	   *panel = ip->panel;

    /*
     * Only reassign the keyboard focus to another item if the panel isn't
     * being destroyed.
     */
    if (!panel->status.destroying && panel->kbd_focus_item == ip) {
	panel->kbd_focus_item = panel_next_kbd_focus(panel, TRUE);
	panel_accept_kbd_focus(panel);
    }

    return;
}


static void
ambtn_accept_kbd_focus(item_public)
    Panel_item	    item_public;
{
    Frame	    frame;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    int		    x;
    int		    y;

    frame = xv_get(PANEL_PUBLIC(ip->panel), WIN_FRAME);
    if (ip->panel->layout == PANEL_HORIZONTAL) {
	xv_set(frame, FRAME_FOCUS_DIRECTION, FRAME_FOCUS_UP, NULL);
	x = ip->value_rect.r_left + AMB_OFFSET +
	    (ip->value_rect.r_width - AMB_OFFSET - FRAME_FOCUS_UP_WIDTH)/2;
	y = ip->value_rect.r_top + ip->value_rect.r_height;
    } else {
	xv_set(frame, FRAME_FOCUS_DIRECTION, FRAME_FOCUS_RIGHT, NULL);
	x = ip->value_rect.r_left - FRAME_FOCUS_RIGHT_WIDTH/2;
	y = ip->value_rect.r_top +
	    (ip->value_rect.r_height - FRAME_FOCUS_RIGHT_HEIGHT)/2;
    }
    if (x < 0)
	x = 0;
    if (y < 0)
	y = 0;
    panel_show_focus_win(item_public, frame, x, y);
}


static void
ambtn_yield_kbd_focus(item_public)
    Panel_item	    item_public;
{
    Xv_Window	    focus_win;
    Frame	    frame;
    Item_info      *ip = ITEM_PRIVATE(item_public);

    frame = xv_get(PANEL_PUBLIC(ip->panel), WIN_FRAME);
    focus_win = xv_get(frame, FRAME_FOCUS_WIN);
    xv_set(focus_win, XV_SHOW, FALSE, NULL);
}



/* --------------------  Local Routines  -------------------- */

static void
ambtn_menu_busy_proc(menu)
    Menu	    menu;
{
    Item_info	   *ip;
    
    ip = (Item_info *) xv_get(menu, XV_KEY_DATA, PANEL_FIRST_ITEM);
    ambtn_paint_value(ip, OLGX_BUSY);
}


static void
ambtn_menu_done_proc(menu, result)
    Menu            menu;
    Xv_opaque       result;
{
    Item_info	   *ip;
    void          (*orig_done_proc) ();	/* original menu-done procedure */
    
    ip = (Item_info *) xv_get(menu, XV_KEY_DATA, PANEL_FIRST_ITEM);
    ambtn_paint_value(ip,
	ip->panel->status.three_d ? OLGX_NORMAL : OLGX_ERASE | OLGX_NORMAL);

    /* Restore original menu done proc. */
    orig_done_proc = (void (*) ()) xv_get(menu, XV_KEY_DATA, MENU_DONE_PROC);
    xv_set(menu,
	MENU_DONE_PROC, orig_done_proc,
	NULL);

    /* Invoke original menu done proc (if any) */
    if (orig_done_proc)
	(orig_done_proc) (menu, result);

    ip->panel->status.current_item_active = FALSE;
}


static void
ambtn_paint_value(ip, state)
    Item_info      *ip;
    int		    state;
{
    Xv_Drawable_info *info;
    Xv_Window	    pw;

    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
	olgx_draw_abbrev_button(ip->panel->ginfo, xv_xid(info),
	    ip->value_rect.r_left + AMB_OFFSET,
	    ip->value_rect.r_top, state);
    PANEL_END_EACH_PAINT_WINDOW
}
