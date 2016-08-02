#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_btn.c 20.110 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/panel_impl.h>
#include <xview/font.h>
#include <xview/openmenu.h>
#include <xview/pixwin.h>
#include <xview/server.h>
#include <xview/svrimage.h>
#include <xview/cms.h>
#include <X11/Xlib.h>
#include <xview_private/draw_impl.h>


#define BUTTON_PRIVATE(item) \
	XV_PRIVATE(Button_info, Xv_panel_button, item)
#define BUTTON_FROM_ITEM(ip)	BUTTON_PRIVATE(ITEM_PUBLIC(ip))


static void     btn_begin_preview(), btn_cancel_preview(), btn_accept_preview(),
		btn_accept_menu(), btn_accept_key(), btn_paint(), btn_remove(),
		btn_accept_kbd_focus(), btn_yield_kbd_focus();


static Panel_ops ops = {
    panel_default_handle_event,		/* handle_event() */
    btn_begin_preview,			/* begin_preview() */
    NULL,				/* update_preview() */
    btn_cancel_preview,			/* cancel_preview() */
    btn_accept_preview,			/* accept_preview() */
    btn_accept_menu,			/* accept_menu() */
    btn_accept_key,			/* accept_key() */
    panel_default_clear_item,		/* clear() */
    btn_paint,				/* paint() */
    NULL,				/* resize() */
    btn_remove,				/* remove() */
    NULL,				/* restore() */
    NULL,				/* layout() */
    btn_accept_kbd_focus,		/* accept_kbd_focus() */
    btn_yield_kbd_focus,		/* yield_kbd_focus() */
    NULL				/* extension: reserved for future use */
};

typedef struct button_info {
    Panel_item      public_self;/* back pointer to object */
    int		    clear_button_rect;
    int		    default_menu_item_inactive;
    Server_image    pin_in_image;
}               Button_info;


/*
 * Function declarations
 */
Xv_private void menu_save_pin_window_rect();
Xv_private void menu_item_set_parent();

Pkg_private int panel_button_init();

static void     button_menu_busy_proc();
static void     button_menu_done_proc();
static Menu     generate_menu();
static void     panel_paint_button_image();
static void     take_down_cmd_frame();
int 		panel_item_destroy_flag;
#ifdef OW_I18N
extern wchar_t _xv_null_string_wc[];
#endif /* OW_I18N */


/* ========================================================================= */

/* -------------------- XView Functions  -------------------- */
/* ARGSUSED */
Pkg_private int
panel_button_init(panel_public, item_public, avlist)
    Panel           panel_public;
    Panel_item      item_public;
    Attr_avlist     avlist;
{
    Panel_info     *panel = PANEL_PRIVATE(panel_public);
    register Item_info *ip = ITEM_PRIVATE(item_public);
    Xv_panel_button *item_object = (Xv_panel_button *) item_public;
    Button_info    *dp;

    dp = xv_alloc(Button_info);

    item_object->private_data = (Xv_opaque) dp;
    dp->public_self = item_public;

    ip->ops = ops;
    if (panel->event_proc)
	ip->ops.panel_op_handle_event = (void (*) ()) panel->event_proc;
    ip->item_type = PANEL_BUTTON_ITEM;

    if (panel->status.mouseless)
	ip->flags |= WANTS_KEY;

    return XV_OK;
}


/*ARGSUSED*/
Pkg_private int
panel_button_destroy(item_public, status)
    Panel_item      item_public;
    Destroy_status  status;
{
    Button_info	   *dp = BUTTON_PRIVATE(item_public);

    if ((status == DESTROY_CHECKING) || (status == DESTROY_SAVE_YOURSELF))
	return XV_OK;
    btn_remove(item_public);
    free(dp);
    return XV_OK;
}



/* --------------------  Panel Item Operations  -------------------- */
/* ARGSUSED */
static void
btn_begin_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Menu            default_menu;
    Menu_item       default_menu_item;
    Button_info	   *dp = BUTTON_PRIVATE(item_public);
    int		    height;
    Panel_image     image;
    Xv_Drawable_info *info;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Xv_opaque	    label;
    Menu_class      menu_class;
    Menu_item	  (*mi_gen_proc)();
    int		    olgx_state = OLGX_NORMAL;
    int             pin_is_default;	/* boolean */
    Pixlabel	    pixlabel;
    int		    pushpin_width;
    Xv_Window       pw;
    Menu            submenu;

    dp->clear_button_rect = FALSE;
    ip->flags |= INVOKED;
    if (ip->menu) {

	/* Get the Server Image or string of the default item */
	submenu = ip->menu;
	if ((default_menu = generate_menu(submenu)) == XV_ZERO)
	    return;
	default_menu_item = (Menu_item) xv_get(default_menu, MENU_DEFAULT_ITEM);
	if (!default_menu_item)
	    /* Invalid menu attached: ignore begin_preview request */
	    return;
	mi_gen_proc = (Menu_item (*) ()) xv_get(default_menu_item,
	    MENU_GEN_PROC);
	if (mi_gen_proc) {
	    if (!xv_get (default_menu_item, MENU_PARENT))
		menu_item_set_parent (default_menu_item, default_menu);
	    default_menu_item = mi_gen_proc(default_menu_item, MENU_DISPLAY);
        }
	if (!default_menu_item)
	    /* Invalid menu attached: ignore begin_preview request */
	    return;
	pin_is_default = xv_get(default_menu, MENU_PIN) &&
	    xv_get(default_menu_item, MENU_TITLE);
	height = 0;
#ifdef OW_I18N
        if (pin_is_default) {
            image.im_type = PIT_STRING;
            image_string_wc(&image) = _xv_null_string_wc;
            label = (Xv_opaque) _xv_null_string_wc;
	    olgx_state |= OLGX_LABEL_IS_WCS;
        } else if (!(image_string_wc(&image) = (CHAR *) 
		xv_get(default_menu_item, MENU_STRING_WCS))) {
            olgx_state |= OLGX_LABEL_IS_PIXMAP;
            image.im_type = PIT_SVRIM;
            image_svrim(&image) = xv_get(default_menu_item, MENU_IMAGE);
            pixlabel.pixmap = (XID) xv_get(image_svrim(&image), XV_XID);
            pixlabel.width = ((Pixrect *)image_svrim(&image))->pr_width;
            pixlabel.height = ((Pixrect *)image_svrim(&image))->pr_height;
            label = (Xv_opaque) &pixlabel;
            height = Button_Height(ip->panel->ginfo);
            dp->clear_button_rect = TRUE;
        } else {
            image.im_type = PIT_STRING;
            image_font(&image) = image_font(&ip->label);
            image_bold(&image) = image_bold(&ip->label);
            label = (Xv_opaque) image_string_wc(&image);
	    olgx_state |= OLGX_LABEL_IS_WCS;
        }
#else
	if (pin_is_default) {
	    image.im_type = PIT_STRING;
	    image_string(&image) = NULL;
/* Alpha compatibility, mbuck@debian.org */
#if 1
	    label = (Xv_opaque) ((char *)"");
#else
	    label = "";
#endif
	} else if (!(image_string(&image) = (char *) xv_get(default_menu_item,
		MENU_STRING))) {
	    olgx_state |= OLGX_LABEL_IS_PIXMAP;
	    image.im_type = PIT_SVRIM;
	    image_svrim(&image) = xv_get(default_menu_item, MENU_IMAGE);
	    pixlabel.pixmap = (XID) xv_get(image_svrim(&image), XV_XID);
	    pixlabel.width = ((Pixrect *)image_svrim(&image))->pr_width;
	    pixlabel.height = ((Pixrect *)image_svrim(&image))->pr_height;
	    label = (Xv_opaque) &pixlabel;
	    height = Button_Height(ip->panel->ginfo);
	    dp->clear_button_rect = TRUE;
	} else {
	    image.im_type = PIT_STRING;
	    image_font(&image) = image_font(&ip->label);
	    image_bold(&image) = image_bold(&ip->label);
	    label = (Xv_opaque) image_string(&image);
	}
#endif /* OW_I18N */
	image_ginfo(&image) = image_ginfo(&ip->label);
	/*
	 * Replace the menu button with the default item Server Image or
	 * string.  If it's a string, truncate the string to the width of the
	 * original button stack string.
	 */
	dp->default_menu_item_inactive = (int) xv_get(default_menu_item,
						      MENU_INACTIVE);
	menu_class = (Menu_class) xv_get(default_menu, MENU_CLASS);
	switch (menu_class) {
	  case MENU_COMMAND:
	    panel_paint_button_image(ip, &image, dp->default_menu_item_inactive,
				     (Xv_opaque) NULL, height);
	    if (pin_is_default) {
		if (xv_get(default_menu_item, MENU_INACTIVE)) {
		    olgx_state = OLGX_PUSHPIN_OUT | OLGX_DEFAULT |
			OLGX_INACTIVE;
		    pushpin_width = 26; /* ??? point sizes != 12 */
		} else {
		    olgx_state = OLGX_PUSHPIN_IN;
		    pushpin_width = 13; /* ??? point sizes != 12 */
		}
		PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
		    DRAWABLE_INFO_MACRO(pw, info);
		    olgx_draw_pushpin(ip->panel->ginfo, xv_xid(info),
			ip->label_rect.r_left + (ip->label_rect.r_width -
			    pushpin_width) /2,
			ip->label_rect.r_top + 1, olgx_state);
		PANEL_END_EACH_PAINT_WINDOW
	    }
	    break;

	  case MENU_CHOICE:
	  case MENU_TOGGLE:
	    if (menu_class == MENU_CHOICE ||
		!xv_get(default_menu_item, MENU_SELECTED))
		/* Choice item: always selected
		 * Toggle item: selected if previous state is "not selected"
		 */
		olgx_state |= OLGX_INVOKED;
	    panel_clear_rect(ip->panel, ip->label_rect);
	    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
		DRAWABLE_INFO_MACRO(pw, info);
		olgx_draw_choice_item(ip->panel->ginfo, xv_xid(info),
		    ip->label_rect.r_left, ip->label_rect.r_top,
		    ip->label_rect.r_width, ip->label_rect.r_height,
		    label, olgx_state);
	    PANEL_END_EACH_PAINT_WINDOW
	    dp->clear_button_rect = TRUE;
	}
    } else
	/* Invert the button interior */
	panel_paint_button_image(ip, &ip->label, inactive(ip), ip->menu, 0);
}


static void
btn_cancel_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Button_info	   *dp = BUTTON_PRIVATE(item_public);
    Menu	    default_menu;
    Menu_item	    default_menu_item;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Menu_item	  (*mi_gen_proc)();

    if (dp->clear_button_rect) {
	dp->clear_button_rect = FALSE;
	panel_clear_rect(ip->panel, ip->label_rect);
    }

    /*
     * If menu button or event is SELECT-down, then repaint button.
     */
    ip->flags &= ~INVOKED;
    if (ip->menu || action_select_is_down(event))
	panel_paint_button_image(ip, &ip->label, inactive(ip), ip->menu, 0);
    if (ip->menu) {
	if ((default_menu = generate_menu(ip->menu)) == XV_ZERO)
	    return;
	default_menu_item = (Menu_item) xv_get(default_menu, MENU_DEFAULT_ITEM);
	if (!default_menu_item)
	    return;
	mi_gen_proc = (Menu_item (*) ()) xv_get(default_menu_item,
	    MENU_GEN_PROC);
	if (mi_gen_proc)
	    mi_gen_proc(default_menu_item, MENU_DISPLAY_DONE);
    }
}


static void
btn_accept_preview(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Button_info	   *dp = BUTTON_PRIVATE(item_public);
    Xv_Drawable_info *info;
    Item_info      *ip = ITEM_PRIVATE(item_public);

    if (!invoked(ip)) {
	/* SELECT-down didn't occur over button: ignore SELECT-up */
	return;
    }

    if (dp->clear_button_rect) {
	dp->clear_button_rect = FALSE;
	panel_clear_rect(ip->panel, ip->label_rect);
    }

    /* Set the BUSY flag and clear the INVOKED flag.
     * Clear the BUSY_MODIFIED flag so that we can find out if the notify proc
     * does an xv_set on PANEL_BUSY.
     */
    ip->flags |= BUSY;
    ip->flags &= ~(INVOKED | BUSY_MODIFIED);

    if (ip->menu && dp->default_menu_item_inactive) {
	xv_set(PANEL_PUBLIC(ip->panel), WIN_ALARM, NULL);
    } else {
	/*
	 * Repaint original button image with busy feedback.  Sync server to
	 * force busy pattern to be painted before entering notify procedure.
	 */
	panel_paint_button_image(ip, &ip->label, inactive(ip), ip->menu, 0);
	DRAWABLE_INFO_MACRO(PANEL_PUBLIC(ip->panel), info);
	XSync(xv_display(info), False);

	panel_item_destroy_flag = 0;
	panel_btn_accepted(ip, event);
	if (panel_item_destroy_flag == 2)
	    return;
	panel_item_destroy_flag = 0;
    }

    /* Clear the BUSY flag unless the notify proc has modified it by doing an
     * xv_set on PANEL_BUSY.
     */
    if (!busy_modified(ip))
	ip->flags &= ~(BUSY | BUSY_MODIFIED);
    else
	ip->flags &= ~BUSY_MODIFIED;

    if (!hidden(ip) && !busy(ip))
	panel_paint_button_image(ip, &ip->label, inactive(ip), ip->menu, 0);
}


static void
btn_accept_menu(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Xv_Window       paint_window = event_window(event);

    if (ip->menu == XV_ZERO || paint_window == XV_ZERO)
	return;

    /*
     * Notify the client.  This callback allows the client to dynamically
     * generate the menu.
     */
    (*ip->notify) (ITEM_PUBLIC(ip), event);

    /*
     * Save public panel handle and current menu color, busy proc and done proc.
     * Switch to button's color and menu done proc.
     */
    xv_set(ip->menu,
	   XV_KEY_DATA, PANEL_BUTTON, ITEM_PUBLIC(ip),
	   XV_KEY_DATA, MENU_BUSY_PROC, xv_get(ip->menu, MENU_BUSY_PROC),
	   XV_KEY_DATA, MENU_DONE_PROC, xv_get(ip->menu, MENU_DONE_PROC),
	   XV_KEY_DATA, MENU_COLOR, xv_get(ip->menu, MENU_COLOR),
	   MENU_BUSY_PROC, button_menu_busy_proc,
	   MENU_DONE_PROC, button_menu_done_proc,
	   MENU_COLOR, ip->color_index,
	   NULL);

    /* Repaint the button in the invoked state */
    ip->flags |= INVOKED;
    panel_paint_button_image(ip, &ip->label, inactive(ip), ip->menu, 0);

    /* Show menu */
    ip->panel->status.current_item_active = TRUE;
    menu_show(ip->menu, paint_window, event,
	      MENU_POSITION_RECT, &ip->label_rect,
	      MENU_PULLDOWN, ip->panel->layout == PANEL_HORIZONTAL,
	      NULL);
}


static void
btn_accept_key(item_public, event)
    Panel_item	    item_public;
    Event          *event;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Panel_info	   *panel = ip->panel;

    if (panel->layout == PANEL_VERTICAL) {
	switch (event_action(event)) {
	  case ACTION_RIGHT:
	    if (ip->menu)
		panel_accept_menu(ITEM_PUBLIC(ip), event);
	    break;
	  case ACTION_UP:
	    if (event_is_down(event) && is_menu_item(ip))
		panel_set_kbd_focus(panel,
				    panel_previous_kbd_focus(panel, TRUE));
	    break;
	  case ACTION_DOWN:
	    if (event_is_down(event) && is_menu_item(ip))
		panel_set_kbd_focus(panel,
				    panel_next_kbd_focus(panel, TRUE));
	    break;
	}
    } else if (ip->menu && event_action(event) == ACTION_DOWN)
	panel_accept_menu(ITEM_PUBLIC(ip), event);
}


static void
btn_paint(item_public)
    Panel_item	    item_public;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);

    panel_paint_button_image(ip, &ip->label, inactive(ip), ip->menu, 0);
}


static void
btn_remove(item_public)
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
btn_accept_kbd_focus(item_public)
    Panel_item	    item_public;
{
    Frame	    frame;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    int		    x;
    int		    y;

    frame = xv_get(PANEL_PUBLIC(ip->panel), WIN_FRAME);
    if (ip->panel->layout == PANEL_HORIZONTAL) {
	xv_set(frame, FRAME_FOCUS_DIRECTION, FRAME_FOCUS_UP, NULL);
	x = ip->rect.r_left +
	    (ip->rect.r_width - FRAME_FOCUS_UP_WIDTH)/2;
	y = ip->rect.r_top + ip->rect.r_height - FRAME_FOCUS_UP_HEIGHT/2;
    } else {
	xv_set(frame, FRAME_FOCUS_DIRECTION, FRAME_FOCUS_RIGHT, NULL);
	x = ip->rect.r_left - FRAME_FOCUS_RIGHT_WIDTH/2;
	y = ip->rect.r_top +
	    (ip->rect.r_height - FRAME_FOCUS_RIGHT_HEIGHT)/2;
    }
    if (x < 0)
	x = 0;
    if (y < 0)
	y = 0;
    panel_show_focus_win(item_public, frame, x, y);
}


static void
btn_yield_kbd_focus(item_public)
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
button_menu_busy_proc(menu)
    Menu	    menu;
{
    Panel_item	    item = xv_get(menu, XV_KEY_DATA, PANEL_BUTTON);
    Item_info	   *ip = ITEM_PRIVATE(item);

    /* MENU-up: Menu is in Click-Move-Click mode.  Show button in busy state. */
    ip->flags &= ~INVOKED;
    ip->flags |= BUSY;
    panel_paint_button_image(ip, &ip->label, inactive(ip), ip->menu, 0);
}


static void
button_menu_done_proc(menu, result)
    Menu            menu;
    Xv_opaque       result;
{
    void            (*orig_done_proc) ();	/* original menu-done
						 * procedure */
    Panel_item	    item = xv_get(menu, XV_KEY_DATA, PANEL_BUTTON);
    Item_info	   *ip = ITEM_PRIVATE(item);
    Panel_info     *panel = ip->panel;
    Panel           panel_public = PANEL_PUBLIC(panel);

    /* Restore menu button to normal state */
    ip->flags &= ~(INVOKED | BUSY);
    if (!hidden(ip))
        panel_paint_button_image(ip, &ip->label, inactive(ip), ip->menu, 0);

    if (xv_get(menu, MENU_NOTIFY_STATUS) == XV_OK)
	take_down_cmd_frame(panel_public);

    panel->current = NULL;

    /* Restore menu color and original menu busy and done procs. */
    orig_done_proc = (void (*) ()) xv_get(menu, XV_KEY_DATA, MENU_DONE_PROC);
    xv_set(menu,
	MENU_BUSY_PROC, xv_get(menu, XV_KEY_DATA, MENU_BUSY_PROC),
	MENU_DONE_PROC, orig_done_proc,
	MENU_COLOR, xv_get(menu, XV_KEY_DATA, MENU_COLOR),
	NULL);

    /* Invoke original menu done proc (if any) */
    if (orig_done_proc)
	(orig_done_proc) (menu, result);

    ip->panel->status.current_item_active = FALSE;
}


static          Menu
generate_menu(menu)
    Menu            menu;
{
    Menu            gen_menu, (*gen_proc) ();	/* generated menu and
						 * procedure */

    if (gen_proc = (Menu(*) ()) xv_get(menu, MENU_GEN_PROC)) {
	gen_menu = gen_proc(menu, MENU_DISPLAY);
	if (gen_menu == XV_ZERO) {
	    xv_error(menu,
		     ERROR_STRING,
		 XV_MSG("begin_preview: menu's gen_proc failed to generate a menu"),
		     ERROR_PKG, PANEL,
		     NULL);
	    return (XV_ZERO);
	}
	return (gen_menu);
    } else
	return (menu);
}


Pkg_private void
panel_btn_accepted(ip, event)
    Item_info	   *ip;
    Event	   *event;
{
    Menu            default_menu, submenu, topmenu;
    Menu_item       default_menu_item;
    int             menu_depth = 0;
    void            (*pin_proc) ();	/* pin procedure for default menu */
    int             pin_is_default;	/* boolean */
    int             notify_status;	/* XV_OK or XV_ERROR */

    /* notify the client */
    ip->notify_status = XV_OK;
    panel_item_destroy_flag = 0;
    if (ip->flags & IS_MENU_ITEM)
        panel_item_destroy_flag = 1;
    (*ip->notify) (ITEM_PUBLIC(ip), event);
    if (panel_item_destroy_flag == 2)
	return; 
    panel_item_destroy_flag = 0;

    if ((ip->menu) && (submenu = topmenu = generate_menu(ip->menu))) {
	/* Select the default entry */
	menu_select_default(submenu);
	do {
	    menu_depth++;
	    default_menu = submenu;
	    default_menu_item = (Menu_item) xv_get(default_menu,
						   MENU_DEFAULT_ITEM);
	} while (default_menu_item &&
		 (submenu = (Menu) xv_get(default_menu_item, MENU_PULLRIGHT)));
	if (default_menu_item) {
	    pin_is_default = xv_get(default_menu, MENU_PIN) &&
		xv_get(default_menu_item, MENU_TITLE);
	    if (pin_is_default) {
		if (xv_get(default_menu_item, MENU_INACTIVE))
		    notify_status = XV_ERROR;
		else {
		    pin_proc = (void (*) ()) xv_get(default_menu,
						    MENU_PIN_PROC);
		    pin_proc(default_menu, event_x(event), event_y(event));
		    notify_status = XV_OK;
		}
	    } else {
		/*
		 * Invoke the appropriate notify procedure(s) for the menu
		 * selection.
		 */
		menu_return_default(topmenu, menu_depth, event);
		notify_status = (int) xv_get(topmenu, MENU_NOTIFY_STATUS);
	    }
	} else
	    notify_status = XV_ERROR;
    } else
	notify_status = ip->notify_status;

    if (notify_status == XV_OK)
	take_down_cmd_frame(PANEL_PUBLIC(ip->panel));
}


static void
panel_paint_button_image(ip, image, inactive_button, menu, height)
    Item_info      *ip;
    Panel_image    *image;
    int             inactive_button;
    Xv_opaque       menu;	/* NULL => no menu attached to button */
    int		    height;	/* 0 => use height of image */
{
    int             default_button;
    Graphics_info  *ginfo;
    Xv_Drawable_info *info;
    Xv_opaque	    label;
    Panel_info     *panel = ip->panel;
    Pixlabel	    pixlabel;
    Xv_Window       pw;
    int		    save_black;
    int		    state;

    default_button = ITEM_PUBLIC(ip) == panel->default_item;
    state = busy(ip) ? OLGX_BUSY :
	    invoked(ip) ? OLGX_INVOKED :
	    panel->status.three_d ? OLGX_NORMAL :
	    OLGX_NORMAL | OLGX_ERASE;
    if (image_type(image) == PIT_STRING) {
	height = 0;   /* not used by olgx_draw_button */
#ifdef OW_I18N
	label = (Xv_opaque) image_string_wc(image);
	state |= OLGX_LABEL_IS_WCS;
#else
	label = (Xv_opaque) image_string(image);
#endif /* OW_I18N */
    } else {
	if (height == 0)
	    height = ((Pixrect *)image_svrim(image))->pr_height +
		OLGX_VAR_HEIGHT_BTN_MARGIN;
	pixlabel.pixmap = (XID) xv_get(image_svrim(image), XV_XID);
	pixlabel.width = ((Pixrect *)image_svrim(image))->pr_width;
	pixlabel.height = ((Pixrect *)image_svrim(image))->pr_height;
	label = (Xv_opaque) &pixlabel;
	state |= OLGX_LABEL_IS_PIXMAP;
    }
    if (is_menu_item(ip)) {
	state |= OLGX_MENU_ITEM;
	if (!busy(ip) && !invoked(ip))
	    state |= OLGX_ERASE;
    }
    if (default_button)
	state |= OLGX_DEFAULT;
    if (inactive_button)
	state |= OLGX_INACTIVE;
    if (menu)  {
	if (panel->layout == PANEL_VERTICAL)
	    state |= OLGX_HORIZ_MENU_MARK;
	else
	    state |= OLGX_VERT_MENU_MARK;
    }
    if (image_ginfo(image))
	ginfo = image_ginfo(image);
    else
	ginfo = panel->ginfo;
    PANEL_EACH_PAINT_WINDOW(panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
	if (ip->color_index >= 0) {
	    save_black = olgx_get_single_color(ginfo, OLGX_BLACK);
	    olgx_set_single_color(ginfo, OLGX_BLACK,
				  xv_get(xv_cms(info), CMS_PIXEL,
				  	 ip->color_index),
				  OLGX_SPECIAL);
	}
	olgx_draw_button(ginfo, xv_xid(info),
			 ip->label_rect.r_left, ip->label_rect.r_top,
			 ip->label_rect.r_width, height, label, state);
	if (ip->color_index >= 0)
	    olgx_set_single_color(ginfo, OLGX_BLACK, save_black,
				  OLGX_SPECIAL);
    PANEL_END_EACH_PAINT_WINDOW
}


static void
take_down_cmd_frame(panel_public)
    Panel           panel_public;
{
    Frame           frame;

    /*
     * If this is a command frame, and the pin is out, then take down the
     * window.  Note: The frame code checks to see if the pin is out.
     */
    frame = xv_get(panel_public, WIN_FRAME);
    if (xv_get(frame, XV_IS_SUBTYPE_OF, FRAME_CMD)) {
	menu_save_pin_window_rect(frame);
	xv_set(frame, XV_SHOW, FALSE, NULL);
    }
}

