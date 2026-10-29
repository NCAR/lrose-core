#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_num_btn.c 20.13 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */
 
#include <xview_private/panel_impl.h>
#include <xview/openmenu.h>
#include <X11/Xlib.h>
#include <xview_private/draw_impl.h>

Pkg_private int panel_num_up_button_init();
Pkg_private int panel_num_down_button_init();

/*
 * up buttons (10, 12, 14, and 19)
 */
#ifndef SVR4
static short up_button_10_image[] = {
#else SVR4
static unsigned short up_button_10_image[] = {
#endif
#include <images/up_button_10.pr>
};
static mpr_static(up_button_10_pr, 16, 13, 1, up_button_10_image);

#ifndef SVR4
static short up_button_12_image[] = {
#else SVR4
static unsigned short up_button_12_image[] = {
#endif SVR4
#include <images/up_button_12.pr>
};
static mpr_static(up_button_12_pr, 16, 15, 1, up_button_12_image);

#ifndef SVR4
static short up_button_14_image[] = {
#else SVR4
static unsigned short up_button_14_image[] = {
#endif SVR4
#include <images/up_button_14.pr>
};
static mpr_static(up_button_14_pr, 16, 17, 1, up_button_14_image);

#ifndef SVR4
static short up_button_19_image[] = {
#else SVR4
static unsigned short up_button_19_image[] = {
#endif SVR4
#include <images/up_button_19.pr>
};
static mpr_static(up_button_19_pr, 32, 25, 1, up_button_19_image);


/*
 * down buttons (10, 12, 14, and 19)
 */
#ifndef SVR4
static short down_button_10_image[] = {
#else SVR4
static unsigned short down_button_10_image[] = {
#endif SVR4
#include <images/down_button_10.pr>
};
static mpr_static(down_button_10_pr, 16, 13, 1, down_button_10_image);

#ifndef SVR4
static short down_button_12_image[] = {
#else SVR4
static unsigned short down_button_12_image[] = {
#endif SVR4
#include <images/down_button_12.pr>
};
static mpr_static(down_button_12_pr, 16, 15, 1, down_button_12_image);

#ifndef SVR4
static short down_button_14_image[] = {
#else SVR4
static unsigned short down_button_14_image[] = {
#endif SVR4
#include <images/down_button_14.pr>
};
static mpr_static(down_button_14_pr, 16, 17, 1, down_button_14_image);

#ifndef SVR4
static short down_button_19_image[] = {
#else SVR4
static unsigned short down_button_19_image[] = {
#endif SVR4
#include <images/down_button_19.pr>
};
static mpr_static(down_button_19_pr, 32, 25, 1, down_button_19_image);

/*
 * ops
 */
static void num_btn_begin_preview();
static void num_btn_cancel_preview();
static void num_btn_accept_preview();
static void num_btn_paint();

static Panel_ops ops = {
    (void (*) ()) panel_default_handle_event,	/* handle_event() */
    (void (*) ()) num_btn_begin_preview,	/* begin_preview() */
    (void (*) ()) panel_nullproc,		/* update_preview() */
    (void (*) ()) num_btn_cancel_preview,	/* cancel_preview() */
    (void (*) ()) num_btn_accept_preview,	/* accept_preview() */
    (void (*) ()) panel_nullproc,		/* accept_menu() */
    (void (*) ()) panel_nullproc,		/* accept_key() */
    (void (*) ()) num_btn_paint,		/* paint() */
    (void (*) ()) panel_nullproc,		/* remove() */
    (void (*) ()) panel_nullproc,		/* restore() */
    (void (*) ()) panel_nullproc,		/* layout() */
    (void (*) ()) panel_nullproc,		/* accept_kbd_focus() */
    (void (*) ()) panel_nullproc		/* yield_kbd_focus() */
};

typedef enum {
    NUM_UP, NUM_DOWN
}     Button_Kind;

typedef struct num_button_info {
    Panel_item public_self;	/* back pointer to object */
    Button_Kind kind;
    int		state;
}     Num_button_info;

#define BUTTON_PRIVATE(item)\
    XV_PRIVATE(Num_button_info, Xv_panel_button, item)
#define BUTTON_FROM_ITEM(ip)    BUTTON_PRIVATE(ITEM_PUBLIC(ip))


static void
create_button(panel_public, item_public, kind)
    Panel panel_public;
    Panel_item item_public;
    Button_Kind kind;
{
    Panel_info *panel = PANEL_PRIVATE(panel_public);
    register Item_info *ip = ITEM_PRIVATE(item_public);
    Xv_panel_button *item_object = (Xv_panel_button *) item_public;
    Num_button_info *dp = xv_alloc(Num_button_info);

    item_object->private_data = (Xv_opaque) dp;
    dp->public_self = item_public;
    dp->kind = kind;
    dp->state = OLGX_NORMAL | OLGX_ERASE;

    if (ops_set(ip))
	*ip->ops = ops;		/* copy the button ops vector */
    else
	ip->ops = &ops;		/* use the static button ops vector */

    if (panel->event_proc)
	ip->ops->handle_event = (void (*) ())panel->event_proc;

    ip->item_type = PANEL_BUTTON_ITEM;

    ip->label_rect.r_width = Abbrev_MenuButton_Width(panel->ginfo);
    ip->label_rect.r_height = Abbrev_MenuButton_Height(panel->ginfo);

    (void) panel_append(ip);
}


/*ARGSUSED*/
Pkg_private int
panel_num_up_button_init(panel_public, item_public, avlist)
    Panel panel_public;
    Panel_item item_public;
    Attr_avlist avlist;
{
    create_button(panel_public, item_public, NUM_UP);

    return XV_OK;
}


/*ARGSUSED*/
Pkg_private int
panel_num_down_button_init(panel_public, item_public, avlist)
    Panel panel_public;
    Panel_item item_public;
    Attr_avlist avlist;
{
    create_button(panel_public, item_public, NUM_DOWN);

    return XV_OK;
}

static void
invert_inside(ip)
    Item_info *ip;
{
    Num_button_info *dp = BUTTON_FROM_ITEM(ip);

    if (dp->state & OLGX_INVOKED)
	dp->state = OLGX_NORMAL | OLGX_ERASE;
    else
	dp->state = OLGX_INVOKED | OLGX_ERASE;
    num_btn_paint(ip);
}

static void
num_btn_paint(ip)
    Item_info *ip;
{
    Xv_Window pw;
    register Rect *rect = &(ip->label_rect);
    Num_button_info *dp = BUTTON_FROM_ITEM(ip);
    Xv_Drawable_info *info;
    unsigned long   pixvals[OLGX_NUM_COLORS];
    int	    state = dp->state;

    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
	if (!ip->panel->three_d && xv_depth(info) > 1) {
	    pixvals[OLGX_WHITE] = xv_bg(info);
	    pixvals[OLGX_BLACK] = ip->color_index < 0 ? xv_fg(info) :
		(unsigned long) xv_get(xv_cms(info),
				       CMS_PIXEL, ip->color_index);
	    olgx_set_color(ip->panel->ginfo, pixvals);
	}
	if (dp->kind == NUM_UP)
	    state |= OLGX_VERT_BACK_MENU_MARK;
	if (inactive(ip))
	    state |= OLGX_INACTIVE;
	olgx_draw_abbrev_button(ip->panel->ginfo, xv_xid(info),
	    rect->r_left, rect->r_top, state);
    PANEL_END_EACH_PAINT_WINDOW
}


/*ARGSUSED*/
static void
num_btn_begin_preview(ip, event)
    Item_info *ip;
    Event *event;
{
    invert_inside(ip);
}


static void
num_btn_cancel_preview(ip, event)
    Item_info *ip;
    Event *event;
{
    /*
     * If button and event is SELECT-down, then undo the inversion.
     */
    if (action_select_is_down(event))
	invert_inside(ip);
}


static void
num_btn_accept_preview(ip, event)
    Item_info *ip;
    Event *event;
{
    /* undo the inversion */
    invert_inside(ip);

    /* don't mask in the 'working' feedback  it's too quick */

    /* notify the client */
    ip->notify_status = XV_OK;
    (*ip->notify) (ITEM_PUBLIC(ip), event);

    num_btn_paint(ip);
}
