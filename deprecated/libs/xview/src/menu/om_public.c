#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)om_public.c 20.146 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1985 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview/win_input.h>	/* includes types & time */
#ifndef FILE
#undef NULL
#include <stdio.h>
#endif				/* FILE */
#include <fcntl.h>
#ifdef SVR4
#include <stdlib.h>
#endif /* SVR4 */

#include <pixrect/pixrect.h>
#include <pixrect/pr_util.h>

#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

#include <X11/Xlib.h>
#include <xview_private/portable.h>
#include <xview_private/fm_impl.h>
#include <xview/xview.h>

#include <xview/defaults.h>
#include <xview/font.h>
#include <xview/panel.h>
#include <xview/win_struct.h>
#include <xview/rectlist.h>

#include <xview/screen.h>
#include <xview/server.h>
#include <xview/fullscreen.h>
#include <xview/notify.h>

#include <xview_private/draw_impl.h>
#include <xview_private/om_impl.h>

/* ------------------------------------------------------------------------- */

/*
 * Public
 */
Xv_public Xv_opaque menu_return_value();
Xv_public Xv_opaque menu_return_item();
Xv_public void  menu_default_pin_proc();

Sv1_public Xv_opaque menu_get();
Sv1_public void menu_destroy_with_proc();

/*
 * Toolkit private
 */
Xv_private void frame_get_rect();
Xv_private void frame_set_rect();
Xv_private void menu_return_default();
Xv_private void menu_select_default();
Xv_private void menu_item_set_parent();
Xv_private void menu_accelerator_notify_proc();

/*
 * Package private
 */
Pkg_private int menu_create_internal();
Pkg_private int menu_create_item_internal();
Pkg_private int menu_item_destroy_internal();
Pkg_private Xv_opaque menu_pullright_return_result();
Pkg_private Xv_opaque menu_sets();
Pkg_private Xv_opaque menu_gets();
Pkg_private Xv_opaque menu_pkg_find();
Pkg_private Xv_opaque menu_item_sets();
Pkg_private Xv_opaque menu_item_gets();
Pkg_private void menu_return_no_value();
Pkg_private void menu_done();
Pkg_private void menu_render();
Pkg_private void menu_set_pin_window();
Pkg_private void menu_window_event_proc();
Pkg_private void menu_shadow_event_proc();
Pkg_private Notify_value menu_client_window_event_proc();

/* destroy routines for menus and menu_items */
Pkg_private void menu_destroys(), menu_item_destroys();

/* Pkg_private (server XV_KEY_DATA keys) */
int    menu_active_menu_key;

/*
 * Private
 */
static void menu_create_pin_window();
static int menu_group_info_key;
static Xv_opaque menu_return_result();

/* Cache the standard menu data obtained from the defaults database */
static Xv_menu_info *m_cache;

extern int panel_item_destroy_flag;

/*
 * Private defs
 */
#define	MENU_DEFAULT_MARGIN		1
#define	MENU_DEFAULT_LEFT_MARGIN	16
#define	MENU_DEFAULT_RIGHT_MARGIN	6
#define	Null_status	(int *)0
#define MENU_KEY 1
#define MENU_ITEM_KEY 2

/* ------------------------------------------------------------------------- */


/*
 * Display the menu, get the menu item, and call notify proc. Default proc
 * returns a pointer to the item selected or NULL.
 */
/* VARARGS3 */
Xv_public void
#ifdef ANSI_FUNC_PROTO
menu_show(Menu menu_public, Xv_Window win, struct inputevent *iep, ...)
#else
menu_show(menu_public, win, iep, va_alist)
    Menu            menu_public;
    Xv_Window       win;
    struct inputevent *iep;
va_dcl		/*** WARNING: menu_show does not support ATTR_LIST. ***/
#endif
{
    AVLIST_DECL;
    va_list         valist;
    Xv_Drawable_info *info;
    Display        *display;
    Rect           *enable_rectp = 0;	/* rectangle in which menu stays up.
					 * Also describes position. 0=> popup
					 * menu */
    Xv_menu_group_info *group_info;
    Xv_menu_info   *menu;
    Rect           *position_rectp = 0;	/* describes menu position only */
    Xv_Server	    server;
    Screen_visual  *screen_visual = NULL;

    if (!menu_public) {
	xv_error(0,
		 ERROR_STRING, 
		    XV_MSG("menu_show: no menu specified"),
		 ERROR_PKG, MENU,
		 NULL);
	return;
    }
    menu = MENU_PRIVATE(menu_public);
    server = xv_get(menu_public, XV_OWNER);
    if (!server)
	server = xv_default_server;
    if (server != XV_SERVER_FROM_WINDOW(win)) {
	xv_error(0,
		 ERROR_STRING, 
		    XV_MSG("menu_show: menu not owned by this server"),
		 ERROR_PKG, MENU,
		 NULL);
	goto menu_show_error;
    }

    VA_START(valist, iep);
    MAKE_AVLIST( valist, avlist );
    va_end(valist);

    for (; *avlist; avlist = attr_next(avlist)) {
	switch (avlist[0]) {

	  case MENU_ENABLE_RECT:
	    enable_rectp = (Rect *) avlist[1];
	    break;

	  case MENU_BUTTON:
	    event_set_id(iep, (int) avlist[1]);
	    break;

	  case MENU_POS:
	    iep->ie_locx = (int) avlist[1], iep->ie_locy = (int) avlist[2];
	    break;

	  case MENU_POSITION_RECT:
	    position_rectp = (Rect *) avlist[1];
	    break;

	  case MENU_PULLDOWN:
	    menu->pulldown = (int) avlist[1];
	    break;

	  default:
	    (void) xv_check_bad_attr(MENU, avlist[0]);
	    break;
	}
    }

    DRAWABLE_INFO_MACRO(win, info);
    display = xv_display(info);

    /*
     * Grab all input and disable anybody but us from writing to screen while
     * we are violating window overlapping.
     */
    if (XGrabPointer(display, xv_get(win, XV_XID),
		     False,	/* owner events: report events relative to
				 * grab window */
		     ButtonPressMask | ButtonReleaseMask |
			 ButtonMotionMask,	/* event mask */
		     GrabModeAsync,	/* pointer mode */
		     GrabModeAsync,	/* keyboard mode */
		     None,	/* confine to: don't confine pointer */
		     0,		/* cursor: use default window cursor */
		     CurrentTime)	/* time */
	!=GrabSuccess) {
	xv_error(0,
		 ERROR_STRING, 
		    XV_MSG("menu_show: unable to grab pointer"),
		 ERROR_PKG, MENU,
		 NULL);
	goto menu_show_error;
    }
    if (XGrabKeyboard(display, xv_get(win, XV_XID),
		     False,	/* owner events: report events relative to
				 * grab window */
		     GrabModeAsync,	/* pointer mode */
		     GrabModeAsync,	/* keyboard mode */
		     CurrentTime)	/* time */
	!=GrabSuccess) {
	xv_error(0,
		 ERROR_STRING, 
		    XV_MSG("menu_show: unable to grab keyboard"),
		 ERROR_PKG, MENU,
		 NULL);
	XUngrabPointer(display, CurrentTime);
	goto menu_show_error;
    }
#ifdef OW_I18N
    menu->ic_was_active = FALSE;
    if (xv_get(win, WIN_USE_IM) == TRUE
     && (menu->ic_was_active = xv_get(win, WIN_IC_ACTIVE)) == TRUE) {
	(void) xv_set(win, WIN_IC_ACTIVE, FALSE, NULL);
	menu->client_window = win;
    }
#endif

    menu->state = MENU_STATE_INIT;
    if (!menu_active_menu_key)
	menu_active_menu_key = xv_unique_key();
    xv_set(server, XV_KEY_DATA, menu_active_menu_key, menu, NULL);
    if (!menu_group_info_key)
	menu_group_info_key = xv_unique_key();
    group_info = (Xv_menu_group_info *) xv_get(server,
	XV_KEY_DATA, menu_group_info_key);
    if (!group_info) {
	/* Allocate and initialize menu group information */
	group_info = (Xv_menu_group_info *) xv_malloc(sizeof(Xv_menu_group_info));
	if (group_info == NULL) {
	    xv_error(0,
		     ERROR_STRING, 
			XV_MSG("menu_show: unable to allocate group_info"),
		     ERROR_PKG, MENU,
		     NULL);
	    XUngrabPointer(display, CurrentTime);
	    XUngrabKeyboard(display, CurrentTime);
#ifdef OW_I18N
	    if (menu->ic_was_active == TRUE)
		xv_set(win, WIN_IC_ACTIVE, TRUE, NULL);
#endif
	    goto menu_show_error;
	}
	xv_set(server, XV_KEY_DATA, menu_group_info_key, group_info, NULL);
	group_info->server = server;
	group_info->three_d_override = FALSE;
    }
    group_info->client_window = win;
    group_info->color_index = menu->color_index;
    group_info->depth = 0;
    group_info->first_event = *iep;
    group_info->last_event = *iep;
    group_info->menu_down_event.action = 0;
	/* no MENU-down event received yet */
    group_info->selected_menu = menu;
    if (menu->vinfo_mask)
      screen_visual = (Screen_visual *)xv_get(xv_screen(info), SCREEN_VISUAL, 
					      menu->vinfo_mask, &menu->vinfo_template);
    if (!screen_visual)
      screen_visual = xv_visual(info);

    group_info->vinfo = screen_visual->vinfo;

    if (group_info->vinfo->depth > 1)
	group_info->three_d = defaults_get_boolean("OpenWindows.3DLook.Color",
	    "OpenWindows.3DLook.Color", TRUE);
    else
#ifdef MONO3D
	group_info->three_d =
	    defaults_get_boolean("OpenWindows.3DLook.Monochrome",
	    "OpenWindows.3DLook.Monochrome", FALSE);
#else
	group_info->three_d = FALSE;
#endif
    /* martin-2.buck@student.uni-ulm.de */
    if (group_info->three_d_override) {
	group_info->three_d = FALSE;
    }

    if (enable_rectp)
	menu->enable_rect = *enable_rectp;
    else
	/* tell process_event and compute_rects that there's no enable rect */
	menu->enable_rect.r_width = 0;
    if (position_rectp)
	menu->position_rect = *position_rectp;
    menu->popup = (enable_rectp || position_rectp) ?
	FALSE			/* menu is a pulldown or pullright */
	: TRUE;			/* menu is a popup */

    menu->stay_up = FALSE;		/* assume non-stay-up mode */

    /*
     * Interpose an event function on the client window to catch the mouse
     * button events.
     * Note: fixed to make xv_window_loop work for menus
     */
    notify_interpose_event_func(win, menu_client_window_event_proc,
         			WIN_IS_IN_LOOP ? NOTIFY_IMMEDIATE :
				NOTIFY_SAFE);

    menu_render(menu, group_info, (Xv_menu_item_info *) 0);
    return;

menu_show_error:
    menu->notify_status = XV_ERROR;
    if (menu->done_proc)
	(menu->done_proc) (menu_public, MENU_NO_VALUE);
}


Pkg_private void
menu_done(m)
    register Xv_menu_info *m;
{
    Display        *display;
    Xv_Drawable_info *info;
    Xv_opaque       result;
    Xv_Server       server;

    DRAWABLE_INFO_MACRO(m->group_info->client_window, info);
    display = xv_display(info);
    server = xv_server(info);

    XUngrabPointer(display, CurrentTime);
    XUngrabKeyboard(display, CurrentTime);
#ifdef OW_I18N
    if (m->ic_was_active == TRUE)
	xv_set(m->group_info->client_window, WIN_IC_ACTIVE, TRUE, NULL);
#endif

    if (m->status == MENU_STATUS_PIN)
	(m->group_info->pinned_menu->pin_proc) (
	    MENU_PUBLIC(m->group_info->pinned_menu),
	    m->group_info->pinned_menu->pin_window_pos.x,
	    m->group_info->pinned_menu->pin_window_pos.y);

    /*
     * Call the generate and notify procedures.
     * Should handle special case of selection = 0.
     */
    XSync(display, False); /* Sync the server */
    m->group_info->notify_proc = m->notify_proc;
    if (!m->group_info->notify_proc)
	m->group_info->notify_proc = MENU_DEFAULT_NOTIFY_PROC;
    if (m->status == MENU_STATUS_DONE) {
	m->group_info->selected_menu->notify_status = XV_OK;
	    /* Assume success; dismiss a popup */
	result = menu_return_result(m->group_info->selected_menu,
				    m->group_info, m->group_info->selected_menu->parent);
    } else {
	m->group_info->selected_menu->notify_status = XV_ERROR;
	    /* Assume failure; don't dismiss a popup */
	m->valid_result = FALSE;
	result = MENU_NO_VALUE;
    }
    m->notify_status = m->group_info->selected_menu->notify_status;

    /*
     * Call menu-done procedure, if any.
     */
    if (m->done_proc)
	(m->done_proc) (MENU_PUBLIC(m), result);

    /* fixed to make xv_window_loop work for menus */
    notify_remove_event_func(m->group_info->client_window,
			     menu_client_window_event_proc, 
			     WIN_IS_IN_LOOP ? NOTIFY_IMMEDIATE :
                             NOTIFY_SAFE);
    m->group_info = NULL;
    xv_set(server, XV_KEY_DATA, menu_active_menu_key, NULL, NULL);
}


/*VARARGS*/
Pkg_private int
menu_create_internal(parent, object, avlist)	/*ARGSUSED*/
    Xv_opaque       parent;
    Xv_opaque       object;
    Xv_opaque      *avlist;
{
    register Xv_menu_info *m;
    register Xv_pkg *menu_type;
    register Attr_avlist	attrs;
    /*
     * BUG ALERT:  We need to pass a root window to the menu package in order
     * to use multiple screens.
     */

    ((Xv_menu *) (object))->private_data = (Xv_opaque) xv_alloc(Xv_menu_info);
    if (!(m = MENU_PRIVATE(object))) {
	xv_error(object,
		 ERROR_STRING, 
		    XV_MSG("menu_create: unable to allocate menu structure"),
		 ERROR_PKG, MENU,
		 NULL);
	return (int) XV_ERROR;
    }
    if (!m_cache) {
	if (!(m_cache = xv_alloc(Xv_menu_info))) {
	    xv_error(object,
		     ERROR_STRING,
		         XV_MSG("menu_create: unable to allocate menu structure"),
		     ERROR_PKG, MENU,
		     NULL);
	    return (int) XV_ERROR;
	}
	m_cache->color_index = -1;
	m_cache->column_major = TRUE;
	m_cache->default_image.bold_font = XV_ZERO;
	m_cache->default_image.font = XV_ZERO;
	m_cache->default_image.left_margin = 1;
	m_cache->default_image.margin = 1;
	m_cache->default_image.right_margin = 1;
	m_cache->default_position = 1;	/* default item is always the first
					 * if not specified */
	m_cache->extra_destroy_proc = 0;
	m_cache->notify_proc = MENU_DEFAULT_NOTIFY_PROC;
	m_cache->pin_proc = menu_default_pin_proc;
	m_cache->pullright_delta =
	    defaults_get_integer("openWindows.dragRightDistance",
				 "OpenWindows.DragRightDistance", 100);
	m_cache->select_is_menu = defaults_get_boolean(
	    "openWindows.selectDisplaysMenu",
	    "OpenWindows.SelectDisplaysMenu",
	    FALSE);
    }
    XV_BCOPY(m_cache, m, sizeof(Xv_menu_info));

   /* Above XV_BCOPY() zaps all of m, so must set individual fields after it.*/
    m->public_self = object;
    m->type = (int) MENU_MENU;	/* for verifying menu handle in menu_destroys */

    /*
     * Malloc the menu storage and create the item list
     */
    m->nitems = 0, m->max_nitems = 2 * MENU_FILLER;
    m->item_list = (Xv_menu_item_info **)
	xv_calloc(2 * MENU_FILLER, sizeof(Xv_menu_item_info *));
    if (!m->item_list) {
	xv_error(object,
		 ERROR_LAYER, ERROR_SYSTEM,
		 ERROR_STRING, 
		    XV_MSG("menu_create: unable to allocate an item list"),
		 ERROR_PKG, MENU,
		 NULL);
	return (int) XV_ERROR;
    }
    /*
     * set the class field depending on what type of menu the client created
     */
    menu_type = (Xv_pkg *) xv_get(object, XV_TYPE);
    if (menu_type == MENU_COMMAND_MENU) {
	m->class = MENU_COMMAND;
    } else if (menu_type == MENU_CHOICE_MENU) {
	m->class = MENU_CHOICE;
	m->default_image.left_margin = 3;
	m->default_image.right_margin = 3;
    } else if (menu_type == MENU_TOGGLE_MENU) {
	m->class = MENU_TOGGLE;
    } else {
	xv_error(object,
		 ERROR_STRING, 
		    XV_MSG("Unknown menu type"),
		 ERROR_PKG, MENU,
		 NULL);
    }

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch ((int)attrs[0]) {
	  case XV_VISUAL:
	    if ((int)attrs[1]) {
		m->vinfo_template.visualid = XVisualIDFromVisual((Visual *)attrs[1]);
		m->vinfo_mask |= VisualIDMask;
	    }
	    break;
	    
	  case XV_VISUAL_CLASS:
	    m->vinfo_template.class = (int)attrs[1];
	    m->vinfo_mask |= VisualClassMask;
	    break;

	  case XV_DEPTH:
	    m->vinfo_template.depth = attrs[1];
	    m->vinfo_mask |= VisualDepthMask;
	    break;

	  default:
	    break;
	}	    
    }

    (void) xv_set(object, XV_RESET_REF_COUNT, NULL);	/* Mark as ref counted. */

    return (int) XV_OK;
}

/* VARARGS */
Pkg_private int
menu_create_item_internal(parent, object, avlist)	/*ARGSUSED*/
    Xv_opaque       parent;
    Xv_opaque       object;
    Xv_opaque      *avlist;
{
    Xv_menu_item_info *mi;

    ((Xv_menu_item *) (object))->private_data =
	(Xv_opaque) xv_alloc(Xv_menu_item_info);
    if (!(mi = MENU_ITEM_PRIVATE(object))) {
	xv_error(object,
		 ERROR_STRING, 
		    XV_MSG("Menu_create_item: unable to allocate menu_item"),
		 ERROR_PKG, MENU,
		 NULL);
	return (int) XV_ERROR;
    }
    mi->color_index = -1;
    mi->public_self = object;
    mi->image.free_image = TRUE;

    return (int) XV_OK;
}


/*
 * destroy the menu data.
 */
Pkg_private int
menu_destroy_internal(menu_public, status)
    Menu            menu_public;
    Destroy_status  status;
{
    Xv_menu_info   *menu = MENU_PRIVATE(menu_public);

    if ((status != DESTROY_CHECKING) && (status != DESTROY_SAVE_YOURSELF))
	menu_destroys(menu, menu->extra_destroy_proc);

    return XV_OK;
}


/*
 * destroy the menu item data.
 */
Pkg_private int
menu_item_destroy_internal(menu_item_public, status)
    Menu_item       menu_item_public;
    Destroy_status  status;
{
    Xv_menu_item_info *menu_item = MENU_ITEM_PRIVATE(menu_item_public);

    if ((status != DESTROY_CHECKING) && (status != DESTROY_SAVE_YOURSELF))
	menu_item_destroys(menu_item, menu_item->extra_destroy_proc);

    return XV_OK;
}


static Xv_opaque
menu_return_result(menu, group, parent)
    Xv_menu_info   *menu;
    Xv_menu_group_info *group;
    Xv_menu_item_info *parent;
{
    register Xv_menu_info *m;
    register Xv_menu_item_info *mi;
    Menu	  (*m_gen_proc) ();
    Menu_item	  (*mi_gen_proc) ();
    Xv_opaque	  (*notify_proc) ();
    int             i;
    int		    mask;
    Xv_opaque	    result;
    int		    toggle_value;

    /* Call menu generate procedure with MENU_NOTIFY */
    if (m_gen_proc = menu->gen_proc) {
	m = MENU_PRIVATE((m_gen_proc) (MENU_PUBLIC(menu), MENU_NOTIFY));
	if (m == NULL)
	    return MENU_NO_VALUE;
	m->group_info = group;
	m->parent = parent;
    } else
	m = menu;

    /*
     * Determine the selected menu item and position
     */
    if (m->status != MENU_STATUS_DONE ||
	!range(m->selected_position, 1, m->nitems))
	m->selected_position = m->default_position;
    mi = m->item_list[m->selected_position - 1];

    switch (m->class) {
      case MENU_CHOICE:	/* exclusive choice */
	for (i = 0; i < m->nitems; i++)
	    m->item_list[i]->selected = FALSE;
	mi->selected = TRUE;
	if (mi->panel_item_handle) {
	    if (m->item_list[0]->title)
		i = m->selected_position - 2;
	    else
		i = m->selected_position - 1;
	    xv_set(mi->panel_item_handle,
		   PANEL_VALUE, i,
		   NULL);
	}
	break;

      case MENU_TOGGLE:	/* nonexclusive choice */
	mi->selected = mi->selected ? FALSE : TRUE;
	if (mi->panel_item_handle) {
	    i = 0;
	    if (m->item_list[0]->title)
		i++;
	    mask = 1;
	    toggle_value = 0;
	    for (; i < m->nitems; i++) {
		if (m->item_list[i]->selected)
		    toggle_value |= mask;
		mask <<= 1;
	    }
	    xv_set(mi->panel_item_handle, PANEL_VALUE, toggle_value, NULL);
	}
	break;

      case MENU_COMMAND:
      default:
	mi->selected = m->status == MENU_STATUS_DONE;
	break;
    }

    mi->parent = m;

    if (mi->inactive) {
	m->valid_result = FALSE;
	result = MENU_NO_VALUE;
	goto cleanup;
    }

    /* Call menu item generate procedure with MENU_NOTIFY */
    if (mi_gen_proc = mi->gen_proc) {
	mi = MENU_ITEM_PRIVATE((mi_gen_proc) (MENU_ITEM_PUBLIC(mi),
					      MENU_NOTIFY));
	if (mi == NULL) {
	    m->valid_result = FALSE;
	    result = MENU_NO_VALUE;
	    goto cleanup;
	}
	mi->parent = m;
    }

    /* Call menu item or menu notify procedure */
    notify_proc = mi->notify_proc ? mi->notify_proc
	: m->notify_proc ? m->notify_proc : m->group_info->notify_proc;
    result = (notify_proc) (MENU_PUBLIC(m), MENU_ITEM_PUBLIC(mi));

    /* Call menu item generate procedure with MENU_NOTIFY_DONE */
    if (mi_gen_proc)
	(mi_gen_proc) (MENU_ITEM_PUBLIC(mi), MENU_NOTIFY_DONE);

cleanup:
    /* Call menu generate procedure with MENU_NOTIFY_DONE */
    if (m_gen_proc) {
	(m_gen_proc) (MENU_PUBLIC(m), MENU_NOTIFY_DONE);
    }
    return result;
}


Pkg_private     Xv_opaque
menu_pullright_return_result(menu_item_public)
    Menu_item       menu_item_public;
{
    register Xv_menu_info *m;
    register Xv_menu_item_info *mi;
    register Xv_menu_info *mn;
    Menu	  (*gen_proc) ();
    Menu	    pullright_menu;
    Xv_opaque       v;

    if (!menu_item_public)
	return XV_ZERO;
    mi = MENU_ITEM_PRIVATE(menu_item_public);
    if (!mi->pullright)
	return XV_ZERO;


    m = mi->parent;

    /* Call menu generate procedure with MENU_NOTIFY */
    if (gen_proc = mi->gen_pullright) {
	pullright_menu = (gen_proc) (menu_item_public, MENU_NOTIFY);
	mn = pullright_menu ? MENU_PRIVATE(pullright_menu) : NULL;
	if (mn == NULL) {
	    menu_return_no_value(MENU_PUBLIC(m));
	    return MENU_NO_VALUE;
	}
    } else {
	mn = MENU_PRIVATE(mi->value);
    }

    if (mn->nitems) {
	v = menu_return_result(mn, m->group_info, mi);
	m->valid_result = mn->valid_result;
    } else {
	v = (Xv_opaque) MENU_NO_VALUE;
	m->valid_result = FALSE;
    }

    if (gen_proc)
	(gen_proc) (menu_item_public, MENU_NOTIFY_DONE);

    return v;
}


Xv_public       Xv_opaque
menu_return_value(menu_public, menu_item_public)
    Menu            menu_public;
    Menu_item       menu_item_public;
{
    register Xv_menu_info *m;
    register Xv_menu_item_info *mi;

    if (!menu_public || !menu_item_public) {	/* No menu or item */
	if (menu_public) {
	    (MENU_PRIVATE(menu_public))->valid_result = FALSE;
	}
	return (Xv_opaque) MENU_NO_VALUE;
    }
    m = MENU_PRIVATE(menu_public);
    mi = MENU_ITEM_PRIVATE(menu_item_public);
    if (mi->pullright)
	return menu_pullright_return_result(menu_item_public);

    m->valid_result = TRUE;
    return (Xv_opaque) mi->value;	/* Return value */
}


Xv_public       Xv_opaque
menu_return_item(menu_public, menu_item_public)
    Menu            menu_public;
    Menu_item       menu_item_public;
{
    register Xv_menu_info *m;
    register Xv_menu_item_info *mi;

    if (!menu_public || !menu_item_public) {	/* No menu or item */
	if (menu_public) {
	    (MENU_PRIVATE(menu_public))->valid_result = FALSE;
	}
	return (Xv_opaque) MENU_NO_ITEM;
    }
    m = MENU_PRIVATE(menu_public);
    mi = MENU_ITEM_PRIVATE(menu_item_public);
    if (mi->pullright)
	return menu_pullright_return_result(menu_item_public);

    m->valid_result = TRUE;
    return MENU_ITEM_PUBLIC(mi);/* Return pointer */
}


Pkg_private void
menu_return_no_value(menu_public)
    Menu            menu_public;
{
    register Xv_menu_info *m;

    if (menu_public) {
	m = MENU_PRIVATE(menu_public);
	m->valid_result = FALSE;
	if (m->gen_proc) {
	    /* Call menu generate procedure with MENU_NOTIFY */
	    (m->gen_proc) (menu_public, MENU_NOTIFY);
	    /* Call menu generate procedure with MENU_NOTIFY_DONE */
	    (m->gen_proc) (menu_public, MENU_NOTIFY_DONE);
	}
    }
}


/* VARARGS1 */
Sv1_public      Menu_item
#ifdef ANSI_FUNC_PROTO
menu_find(Menu menu_public, ...)
#else
menu_find(menu_public, va_alist)
    Menu            menu_public;
va_dcl
#endif			/*** WARNING: menu_find does not support ATTR_LIST. ***/
{
    AVLIST_DECL;
    va_list         valist;

    VA_START(valist, menu_public);
    MAKE_AVLIST( valist, avlist );
    va_end(valist);
    return menu_pkg_find(menu_public, MENUITEM, avlist);
}


/*
 * Find the menu_item specified by the avlist. menu_pkg_find is called from
 * xv_find via the menu package ops vector.
 */
Pkg_private     Xv_opaque
menu_pkg_find(menu_public, pkg, avlist)		/*ARGSUSED*/
    Menu            menu_public;
    Xv_pkg         *pkg;	/* Must be MENUITEM */
    Attr_attribute  avlist[ATTR_STANDARD_SIZE];
{
    register Attr_avlist attrs;
    register Xv_menu_item_info *mi, **mip;
    register int    nitems, correct;
    int             submenus = FALSE, descend_first = FALSE;
    Xv_menu_info   *m;
    Menu(*gen_proc) ();
    Menu_item       mi_public;
    Menu_item(*gen_item_proc) ();
    Xv_menu_info   *m_base;

    if (!menu_public)
	return XV_ZERO;

    m_base = MENU_PRIVATE(menu_public);

    nitems = m_base->nitems;
    for (attrs = avlist; *attrs; attrs = attr_next(attrs))
	if (attrs[0] == MENU_DESCEND_FIRST)
	    descend_first = (int) attrs[1];

    if (gen_proc = m_base->gen_proc) {
	m = MENU_PRIVATE((gen_proc) (menu_public, MENU_DISPLAY));
	if (m == NULL) {
	    xv_error(menu_public,
		     ERROR_STRING,
		         XV_MSG("menu_find: menu's gen_proc failed to generate a menu"),
		     ERROR_PKG, MENU,
		     NULL);
	    return XV_ZERO;
	}
    } else {
	m = m_base;
    }

    nitems = m->nitems;
    for (mip = m->item_list; correct = TRUE, mi = *mip, nitems--; mip++) {

	if (gen_item_proc = mi->gen_proc) {
	    mi = MENU_ITEM_PRIVATE((gen_item_proc) (MENU_ITEM_PUBLIC(mi),
						    MENU_DISPLAY));
	    if (mi == NULL) {
		xv_error(menu_public,
			 ERROR_STRING,
			     XV_MSG("menu_find: menu item's gen_proc failed to generate a menu item"),
			 ERROR_PKG, MENU,
			 NULL);
		goto exit;
	    }
	}
	for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	    switch (attrs[0]) {

	      case MENU_ACTION:/* & case MENU_NOTIFY_PROC: */
		correct = mi->notify_proc == (Xv_opaque(*) ()) attrs[1];
		break;

	      case MENU_CLIENT_DATA:
		correct = mi->client_data == (Xv_opaque) attrs[1];
		break;

	      case MENU_FEEDBACK:
		correct = !mi->no_feedback == (unsigned) attrs[1];
		break;

	      case XV_FONT:
		correct = (mi->image.font ? mi->image.font : 0) == attrs[1];
		break;

	      case MENU_GEN_PROC:
		correct = mi->gen_proc == (Menu_item(*) ()) attrs[1];
		break;

	      case MENU_GEN_PULLRIGHT:
		correct = mi->pullright &&
		    mi->gen_pullright == (Menu(*) ()) attrs[1];
		break;

	      case MENU_IMAGE:
		correct = mi->image.svr_im == (Server_image) attrs[1];
		break;

	      case MENU_INACTIVE:
		correct = mi->inactive == (unsigned) attrs[1];
		break;

	      case MENU_INVERT:
		correct = mi->image.invert == (unsigned) attrs[1];
		break;

	      case MENU_PARENT:
		correct = mi->parent == MENU_PRIVATE(attrs[1]);
		break;

	      case MENU_PULLRIGHT:
		correct = mi->pullright && mi->value == (Xv_opaque) attrs[1];
		break;

#ifdef OW_I18N
              case MENU_STRING:
                if ( ! _xv_is_string_attr_exist_nodup(&mi->image.string)) {
                    correct = 0;
                    break;
                }
		_xv_use_psmbs_value_nodup(&mi->image.string);
		correct = strcmp(mi->image.string.psmbs.value,
						(char *)attrs[1]) == 0;
                break;

              case MENU_STRING_WCS:
                if ( ! _xv_is_string_attr_exist_nodup(&mi->image.string)) {
                    correct = 0;
                    break;
                }
		_xv_use_pswcs_value_nodup(&mi->image.string);
                correct = wscmp(mi->image.string.pswcs.value,
					    (wchar_t *) attrs[1]) == 0;
                break;
#else
	      case MENU_STRING:
		correct = mi->image.string && strcmp(mi->image.string,
		    (char *) attrs[1]) == 0;
		break;
#endif /* OW_I18N */

	      case MENU_VALUE:
		correct = mi->value == (Xv_opaque) attrs[1];
		break;

	    }
	    if (!correct)
		break;
	}

	if (gen_item_proc)
	    (gen_item_proc) (MENU_ITEM_PUBLIC(mi), MENU_DISPLAY_DONE);

	if (correct)
	    goto exit;

	if (mi->pullright)
	    if (descend_first) {
		mi_public = menu_pkg_find(mi->value, MENUITEM, avlist);
		if (mi_public) {
		    mi = MENU_ITEM_PRIVATE(mi_public);
		    goto exit;
		}
	    } else {
		submenus = TRUE;
	    }
    }

    if (submenus) {
	nitems = m->nitems;
	for (mip = m->item_list; mi = *mip, nitems--; mip++)
	    if (mi->pullright) {
		mi_public = menu_pkg_find(mi->value, MENUITEM, avlist);
		if (mi_public) {
		    mi = MENU_ITEM_PRIVATE(mi_public);
		    goto exit;
		}
	    }
    }
    mi = NULL;

exit:
    if (gen_proc)
	(gen_proc) (menu_public, MENU_DISPLAY_DONE);

    return mi ? MENU_ITEM_PUBLIC(mi) : XV_ZERO;
}


Xv_private void
menu_select_default(menu_public)
    Menu            menu_public;
{
    Xv_menu_info   *menu = MENU_PRIVATE(menu_public);
    Xv_menu_item_info *mi;

    if (menu->default_position > menu->nitems)
	/* Menu has no items or only a pushpin: ignore request */
	return;
    menu->selected_position = menu->default_position;
    mi = menu->item_list[menu->selected_position - 1];
    if (mi && mi->pullright && mi->value)
	menu_select_default(mi->value);
}


Xv_private void
menu_return_default(menu_public, depth, event)
    Menu            menu_public;
    int             depth;
    Event	   *event;
{
    Xv_menu_info   *menu = MENU_PRIVATE(menu_public);
    Xv_menu_group_info *group;

    group = xv_alloc(Xv_menu_group_info);
    group->depth = depth;
    group->first_event = *event;
    group->notify_proc = menu->notify_proc;
    if (!group->notify_proc)
	group->notify_proc = MENU_DEFAULT_NOTIFY_PROC;
    menu->notify_status = XV_OK;
    (void) menu_return_result(menu, group, (Xv_menu_item_info *) 0);
    xv_free(group);
}


Xv_public void
menu_default_pin_proc(menu_public, x, y)
    Menu            menu_public;
    int             x, y;	/* fullscreen coordinate of top left corner
				 * of pinned window */
{
    Panel_item      default_panel_item;
    Rect           *frame_rect;
    int             i;
    Xv_menu_info   *menu = MENU_PRIVATE(menu_public);
    Xv_menu_item_info *mi;
    Panel           panel;

#ifdef OW_I18N
    if (!menu->pin_window) {
	_xv_use_pswcs_value_nodup(&menu->pin_window_header);
	menu_create_pin_window(menu_public, menu->pin_parent_frame,
                               menu->pin_window_header.pswcs.value);
    }
#else
    if (!menu->pin_window)
	menu_create_pin_window(menu_public, menu->pin_parent_frame,
			       menu->pin_window_header);
#endif
    	
    /* Call any Pullright Generate procedures */
    for (i = 0; i < menu->nitems; i++) {
	mi = menu->item_list[i];
	if (mi->gen_pullright && !mi->value) {
	    mi->value = (mi->gen_pullright) (MENU_ITEM_PUBLIC(mi),
					     MENU_DISPLAY);
	    if (mi->panel_item_handle)
		xv_set(mi->panel_item_handle,
		       PANEL_ITEM_MENU, mi->value,
		       NULL);
	}
    }

    /* Set the pin window panel default item */
    default_panel_item =
	menu->item_list[menu->default_position - 1]->panel_item_handle;
    if (default_panel_item) {
	panel = xv_get(default_panel_item, XV_OWNER);
	xv_set(panel, PANEL_DEFAULT_ITEM, default_panel_item, NULL);
    }

    /* Set the pin window's position if the window has not already
     * been shown.
     */
    frame_rect = (Rect *) xv_get(menu->pin_window, XV_RECT);
    if (xv_get(menu->pin_window, XV_KEY_DATA, XV_SHOW) != TRUE) {
	frame_rect->r_left = x;
	frame_rect->r_top = y;
#ifdef  OW_I18N
        menu->pin_window_rect.r_left = x;
        menu->pin_window_rect.r_top  = y;
#endif /* OW_I18N */
    } else {
	frame_rect->r_left = menu->pin_window_rect.r_left;
	frame_rect->r_top = menu->pin_window_rect.r_top;
    }
    menu->pin_window_rect.r_width = frame_rect->r_width;
    menu->pin_window_rect.r_height = frame_rect->r_height;
    xv_set(menu->pin_window, XV_RECT, frame_rect, NULL);

    /* Show the pin window.  Set a flag saying that the window has now
     * been shown.
     */
    menu->item_list[0]->inactive = TRUE;  /* first menu item is pushpin-title */
    xv_set(menu->pin_window,
	   FRAME_CMD_PUSHPIN_IN, TRUE,
	   XV_SHOW, TRUE,
	   XV_KEY_DATA, XV_SHOW, TRUE,
	   NULL);
}

static void
pin_button_notify_proc(item, event)
    Panel_item      item;
    Event          *event;
{
    register Xv_menu_info *m;
    Menu_item	    (*gen_proc)();
    Menu            menu;
    Menu_item       menu_item;
    void            (*notify_proc) ();

    menu = (Menu) xv_get(item, XV_KEY_DATA, MENU_KEY);
    menu_item = (Menu_item) xv_get(item, XV_KEY_DATA, MENU_ITEM_KEY);
    notify_proc = (void (*) ()) xv_get(item, XV_KEY_DATA, MENU_NOTIFY_PROC);
    gen_proc = (Menu_item (*) ()) xv_get(item, XV_KEY_DATA, MENU_GEN_PROC);

    /* Fake a MENU_FIRST_EVENT */
    m = MENU_PRIVATE(menu);
    m->group_info = xv_alloc(Xv_menu_group_info);
    m->group_info->first_event = *event;

    /* Invoke the menu item action proc */
    m->notify_status = XV_OK;
    if (gen_proc)
	(void) gen_proc(menu_item, MENU_NOTIFY);
    notify_proc(menu, menu_item);
    if (gen_proc)
	(void) gen_proc(menu_item, MENU_NOTIFY_DONE);
    if (panel_item_destroy_flag != 2)
        xv_set(item, PANEL_NOTIFY_STATUS, m->notify_status, NULL);

    xv_free(m->group_info);
}


/*ARGSUSED*/
static void
pin_choice_notify_proc(item, value, event)
    Panel_item	    item;
    int		    value;
    Event	   *event;
{
    int		    i;
    Xv_menu_info   *m;
    int		    mask;
    Menu            menu;
    int		    menu_item_index = 0;
    Xv_menu_item_info *mi = NULL;

    menu = (Menu) xv_get(item, XV_KEY_DATA, MENU_KEY);
    m = MENU_PRIVATE(menu);
    if (m->item_list[0]->title)
	menu_item_index++;
    if (m->class == MENU_CHOICE) {
	/* Update Choice Menu */
	menu_item_index += value;
	for (i=0; i<m->nitems; i++)
	    m->item_list[i]->selected = i == menu_item_index;
    } else {
	/* Update Toggle Menu */
	i = 0;
	if (m->item_list[0]->title)
	    i++;
	mask = 1;
	for (; i<m->nitems; i++) {
	    if (m->item_list[i]->selected != ((value & mask) != 0))
		    menu_item_index = i;
	    m->item_list[i]->selected = ((value & mask) != 0);
	    mask <<= 1;
	}
    }
    mi = m->item_list[menu_item_index];
    if (mi) {
	xv_set(item,
	       XV_KEY_DATA, MENU_ITEM_KEY, MENU_ITEM_PUBLIC(mi),
	       XV_KEY_DATA, MENU_GEN_PROC, mi->gen_proc,
	       XV_KEY_DATA, MENU_NOTIFY_PROC,
			mi->notify_proc ? mi->notify_proc : m->notify_proc,
	       NULL);
	pin_button_notify_proc(item, event);
    }
}


Pkg_private void
menu_create_pin_panel_items(panel, menu)
    Panel           panel;
    Xv_menu_info   *menu;
{
    Panel_item	    choice_item = XV_ZERO;	/* choice or toggle item */
    int		    choice_nbr = 0;
    int             i;
    int		    label_width;
    int		    mask;
    Xv_menu_item_info *mi;
    int		    toggle_value;
    int		    start_new_column = FALSE;
    int		    num_rows;


    if (menu->default_image.font)  {
        xv_set(panel,
#ifdef OW_I18N
           XV_FONT, menu->default_image.font,
#endif /* OW_I18N */
	   PANEL_LAYOUT, PANEL_VERTICAL,
	   PANEL_ITEM_Y_GAP, menu->default_image.margin,
	   NULL);
    }
    else  {
        xv_set(panel,
	   PANEL_LAYOUT, PANEL_VERTICAL,
	   PANEL_ITEM_Y_GAP, menu->default_image.margin,
	   NULL);
    }

    if (menu->class == MENU_CHOICE)
	choice_item = xv_create(panel, PANEL_CHOICE,
				PANEL_ITEM_COLOR, menu->color_index,
				PANEL_LAYOUT, PANEL_VERTICAL,
				PANEL_CHOICE_NCOLS, menu->ncols,
				XV_HELP_DATA,
				    xv_get(MENU_PUBLIC(menu), XV_HELP_DATA),
				NULL);
    else if (menu->class == MENU_TOGGLE)
	choice_item = xv_create(panel, PANEL_TOGGLE,
				PANEL_ITEM_COLOR, menu->color_index,
				PANEL_LAYOUT, PANEL_VERTICAL,
				PANEL_CHOICE_NCOLS, menu->ncols,
				XV_HELP_DATA,
				    xv_get(MENU_PUBLIC(menu), XV_HELP_DATA),
				NULL);

    /* decide how many rows should be on the panel */
    if ( menu->ncols_fixed )
	/* formula from om_render.c:compute_dimensions() */
	num_rows = ((menu->nitems - 1) / menu->ncols) + 1;
    else if ( menu->nrows_fixed )
	num_rows = menu->nrows - 1;
    else
	num_rows = menu->nitems;


    for (i = 0; i < menu->nitems; i++) {
	mi = menu->item_list[i];
	if (mi->title)
	    continue;
	if (choice_item)
	    mi->panel_item_handle = choice_item;
	else {
	    /* ACC_XVIEW */
            /* 
             * button width is calculated using the label only
             * (default_image) because we do not want to display
             * the accelerator strings on a pinned menu.
             * the value for the key position is actually the
             * left edge of the label + the label width. the
             * width of the key is 0.      
             */
		
            if (menu->ginfo)
               label_width = Button_Width(menu->ginfo,
                        menu->default_image.left_edge,
                        menu->default_image.left_edge +
                                menu->default_image.width,
                        0);
            else
                label_width = menu->default_image.button_size.x;

	    /* ACC_XVIEW */
	    if (mi->pullright && menu->ginfo)
		label_width -= 2*MenuMark_Width(menu->ginfo);

	    /* code redundancy because PANEL_NEXT_COL is create-only */
	    if ( start_new_column ) {
		start_new_column = FALSE;
		mi->panel_item_handle = xv_create(panel, PANEL_BUTTON,
			  PANEL_INACTIVE, mi->inactive | mi->no_feedback,
			  PANEL_ITEM_COLOR, menu->color_index,
			  PANEL_LABEL_WIDTH, label_width,
			  PANEL_MENU_ITEM, TRUE,
			  XV_HELP_DATA, xv_get(MENU_ITEM_PUBLIC(mi), XV_HELP_DATA),
			  PANEL_NEXT_COL, -1,
			  NULL);
	    } else
		mi->panel_item_handle = xv_create(panel, PANEL_BUTTON,
			  PANEL_INACTIVE, mi->inactive | mi->no_feedback,
			  PANEL_ITEM_COLOR, menu->color_index,
			  PANEL_LABEL_WIDTH, label_width,
			  PANEL_MENU_ITEM, TRUE,
			  XV_HELP_DATA, xv_get(MENU_ITEM_PUBLIC(mi), XV_HELP_DATA),
			  NULL);

	    if ( (i % num_rows) == 0 )
		start_new_column = TRUE;
	}
	if (mi->image.svr_im) {
	    if (choice_item)
		xv_set(choice_item,
		       PANEL_CHOICE_IMAGE, choice_nbr++, mi->image.svr_im,
		       NULL);
	    else
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_IMAGE, mi->image.svr_im,
		       NULL);
#ifdef  OW_I18N
	} else if (_xv_is_string_attr_exist_nodup(&mi->image.string)) {
	    _xv_use_pswcs_value_nodup(&mi->image.string);
	    if (choice_item)
		xv_set(choice_item,
                       PANEL_CHOICE_STRING_WCS,
				choice_nbr++,
				mi->image.string.pswcs.value,
		       NULL);
	    else
		xv_set(mi->panel_item_handle,
                       PANEL_LABEL_STRING_WCS,	mi->image.string.pswcs.value,
		       NULL);
#else
	} else if (mi->image.string) {
	    if (choice_item)
		xv_set(choice_item,
		       PANEL_CHOICE_STRING, choice_nbr++, mi->image.string,
		       NULL);
	    else
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_STRING, mi->image.string,
		       NULL);
#endif /* OW_I18N */
	} else
	    xv_error(XV_ZERO,
		     ERROR_SEVERITY, ERROR_NON_RECOVERABLE,
		     ERROR_STRING,
			 XV_MSG("menu item does not have a string or image"),
		     ERROR_PKG, MENU,
		     NULL);
	if (mi->pullright) {
	    if (mi->gen_pullright)
		mi->value = (mi->gen_pullright) (MENU_ITEM_PUBLIC(mi),
						 MENU_DISPLAY);
	    xv_set(mi->panel_item_handle,
		   PANEL_ITEM_MENU, mi->value,
		   NULL);
	} else {
	    if (choice_item)
		xv_set(choice_item,
		   PANEL_NOTIFY_PROC, pin_choice_notify_proc,
		   XV_KEY_DATA, MENU_KEY, MENU_PUBLIC(menu),
		   XV_KEY_DATA, MENU_NOTIFY_PROC,
			mi->notify_proc ? mi->notify_proc : menu->notify_proc,
		   XV_KEY_DATA, MENU_GEN_PROC, mi->gen_proc,
		   NULL);
	    else
		xv_set(mi->panel_item_handle,
		   PANEL_NOTIFY_PROC, pin_button_notify_proc,
		   XV_KEY_DATA, MENU_KEY, MENU_PUBLIC(menu),
		   XV_KEY_DATA, MENU_ITEM_KEY, MENU_ITEM_PUBLIC(mi),
		   XV_KEY_DATA, MENU_NOTIFY_PROC,
			mi->notify_proc ? mi->notify_proc : menu->notify_proc,
		   XV_KEY_DATA, MENU_GEN_PROC, mi->gen_proc,
		   NULL);
	}
    }
    if (menu->class == MENU_CHOICE) {
	for (i=0; i < menu->nitems; i++)
	    if (menu->item_list[i]->selected)
		break;
	if (menu->item_list[0]->title)
	    i--;
	xv_set(choice_item, PANEL_VALUE, i, NULL);
    } else if (menu->class == MENU_TOGGLE) {
	i = 0;
	if (menu->item_list[0]->title)
	    i++;
	mask = 1;
	toggle_value = 0;
	for (; i < menu->nitems; i++) {
	    if (menu->item_list[i]->selected)
		toggle_value |= mask;
	    mask <<= 1;
	}
	xv_set(choice_item, PANEL_VALUE, toggle_value, NULL);
    }

    xv_set(panel, WIN_FIT_HEIGHT, 1, WIN_FIT_WIDTH, 1, NULL);
}


static void
menu_create_pin_window(menu_public, parent_frame, frame_label)
    Menu            menu_public;
    Frame           parent_frame;
    CHAR           *frame_label;
{
    Frame           cmd_frame;
    int		    cms_status;
    Xv_Drawable_info *info;
    Panel           panel;
    Xv_menu_info   *menu = MENU_PRIVATE(menu_public);

    /*
     * Create the Command Frame, and fill in its panel.
     * The Command Frame is owned by the parent frame,
     * but it's X Window parent is the root window.
     * Thus, it's coordinates are expressed in fullscreen coordinates.
     */
    if (menu->group_info) {
          cmd_frame = xv_create(parent_frame, FRAME_CMD,
#ifdef  OW_I18N  
                                WIN_USE_IM,      FALSE,
                                FRAME_LABEL_WCS, frame_label,
#else
                                FRAME_LABEL, frame_label,
#endif /* OW_I18N */
                                XV_SHOW, FALSE,
                                WIN_PARENT, xv_get(parent_frame, XV_ROOT),
                                XV_VISUAL, menu->group_info->vinfo->visual,
                                NULL);
    }
    else {
          cmd_frame = xv_create(parent_frame, FRAME_CMD,
#ifdef  OW_I18N
                                WIN_USE_IM,      FALSE,
                                FRAME_LABEL_WCS, frame_label,
#else
                                FRAME_LABEL, frame_label,
#endif /* OW_I18N */
                                XV_SHOW, FALSE,
                                WIN_PARENT, xv_get(parent_frame, XV_ROOT),
                                NULL);
    }
    panel = xv_get(cmd_frame, FRAME_CMD_PANEL);
    if (menu->group_info && menu->group_info->three_d) {
	DRAWABLE_INFO_MACRO(menu->group_info->client_window, info);
	cms_status = (int) xv_get(xv_cms(info), CMS_STATUS_BITS);
	if (CMS_STATUS(cms_status, CMS_STATUS_CONTROL))
	    /* Use the same Control CMS, foreground and background color
	     * in the pin window's panel as in the client window.
	     */
	    xv_set(panel,
		   WIN_COLOR_INFO, xv_get(menu->group_info->client_window,
		       WIN_COLOR_INFO),
		   NULL);
    }
    xv_set(panel,
	   XV_HELP_DATA, xv_get(menu_public, XV_HELP_DATA),
	   NULL);
    menu_create_pin_panel_items(panel, menu);
    xv_set(cmd_frame, WIN_FIT_HEIGHT, 0, WIN_FIT_WIDTH, 0, NULL);
    menu_set_pin_window(menu, cmd_frame);
}


Xv_private void
menu_save_pin_window_rect(win)
    Xv_Window	    win;
{
    Xv_menu_info   *m = (Xv_menu_info *) xv_get(win, XV_KEY_DATA, MENU_MENU);

    if (m)
	frame_get_rect(win, &m->pin_window_rect);
}


Pkg_private Notify_value
menu_pin_window_event_proc(win, event, arg, type)
    Xv_Window win;
    Event *event;
    Notify_arg arg;
    Notify_event_type type;
{
    int		    i;
    Xv_menu_info   *m;
    Xv_menu_item_info *mi;

    if (event_action(event) == ACTION_DISMISS) {
	menu_save_pin_window_rect(win);
    } else if (event_action(event) == ACTION_CLOSE) {
	m = (Xv_menu_info *) xv_get(win, XV_KEY_DATA, MENU_MENU);
	if (m) {
	    m->item_list[0]->inactive = FALSE;
		/* first menu item is pushpin-title */
	    for (i = 0; i < m->nitems; i++) {
		mi = m->item_list[i];
		if (mi->gen_pullright) {
		    (mi->gen_pullright) (MENU_ITEM_PUBLIC(mi),
					 MENU_DISPLAY_DONE);
		    mi->value = 0;	/* MENU_DISPLAY_DONE complete */
		}
	    }
	}
    }
    return notify_next_event_func(win, (Notify_event) event, arg, type);
}

Xv_private void
menu_item_set_parent(menu_item_public, menu_public)
    Menu_item       menu_item_public;
    Menu            menu_public;
{
    MENU_ITEM_PRIVATE(menu_item_public)->parent = MENU_PRIVATE(menu_public);
}

/* ACC_XVIEW */
Xv_private void
menu_accelerator_notify_proc(accelerator_data, event)
        Frame_accel_data        *accelerator_data;
	Event			*event;
{
 
	Xv_menu_info	 	*menu_private;
	Menu			menu;
        Xv_menu_group_info	*group_info;
	Xv_server		server;
	Xv_menu_item_info 	*menu_item_private, *cur_mi, **mip;
	Menu_item		menu_item;
	Menu			(*m_gen_proc)();
	Menu_item		(*mi_gen_proc)();
	Xv_opaque		(*notify_proc)();
	Xv_opaque		result;
	int			nitems, saved_event = FALSE;
	Event			save_last_event,
				save_first_event;

	if (!accelerator_data)  {
	    return;
	}

	menu = accelerator_data->menu;
	menu_item = accelerator_data->item;

	if (!menu_item || !menu)  {
	    return;
	}

	/*
	 * get handle to menu private data
	 */
	menu_private = MENU_PRIVATE(menu);

	/*
	 * If menu is already visible, don't do accelerators
	 * as it may confuse the gen procs
	 */
	if (menu_private->active)  {
	    return;
	}

	/*
	 * Get server object 
	 * This is done to get/setup the group info
	 */
        server = xv_get(menu, XV_OWNER);
        if (!server)
	    server = xv_default_server;
	

	if (!menu_private->group_info)  {
	    /*
	     * Initialize XV_KEY_DATA key for group info if not
	     * done yet
	     */
            if (!menu_group_info_key)
	        menu_group_info_key = xv_unique_key();

	    /*
	     * Get group info cached on server
	     */
            group_info = (Xv_menu_group_info *) xv_get(server,
	                XV_KEY_DATA, menu_group_info_key);

            if (!group_info) {
	        /* Allocate and initialize menu group information */
	        group_info = (Xv_menu_group_info *) xv_malloc(sizeof(Xv_menu_group_info));
	        if (group_info == NULL) {
	            xv_error(0,
		         ERROR_STRING, 
			    XV_MSG("Unable to allocate group_info"),
		         ERROR_PKG, MENU,
		         NULL);
	        }
	        xv_set(server, XV_KEY_DATA, menu_group_info_key, group_info, NULL);
	        group_info->server = server;
            }

	    /*
	     * Save first/last event
	     */
            save_first_event = group_info->first_event;
            save_last_event = group_info->last_event;
	    saved_event = TRUE;

	    menu_private->group_info = group_info;
	}
	else  {
	    group_info = menu_private->group_info;
	}

	/*
	 * NOTE:
	 * Here we try to make menu accelerators behave as closely as possible to 
	 * actually bringing up the menu, *except* bringing up the menu.
	 * The sequence of actions taken when a menu is popped up was looked at 
	 * to do this. Any change there should be done here as well, if applicable.
	 */

	/*
	 * Set first/last event to be the accelerator event
	 */
        group_info->first_event = *event;
        group_info->last_event = *event;


	m_gen_proc = menu_private->gen_proc;

	/*
	 * Call menu's MENU_GEN_PROC with MENU_DISPLAY
	 */
	if (m_gen_proc)  {
	    menu = m_gen_proc(menu, MENU_DISPLAY);
	    if (!menu)  {
		return;
	    }
	    menu_private = MENU_PRIVATE(menu);
	    menu_private->group_info = group_info;
	}

	/*
	 * For each menu item, call it's gen_proc with
	 * MENU_DISPLAY
	 */
	nitems = menu_private->nitems;
        for (mip = menu_private->item_list; nitems--; mip++) {
	    cur_mi = *mip;
	    cur_mi->parent = menu_private;
	    if (cur_mi->gen_proc)  {
                *mip = cur_mi = 
			MENU_ITEM_PRIVATE((cur_mi->gen_proc) 
				(MENU_ITEM_PUBLIC(cur_mi), MENU_DISPLAY));
	    }
	}

	/*
	 * For each menu item, call it's gen_proc with
	 * MENU_DISPLAY_DONE
	 */
	nitems = menu_private->nitems;
        for (mip = menu_private->item_list; nitems--; mip++) {
	    cur_mi = *mip;
	    cur_mi->parent = menu_private;
	    if (cur_mi->gen_proc)  {
                *mip = cur_mi = 
			MENU_ITEM_PRIVATE((cur_mi->gen_proc) 
				(MENU_ITEM_PUBLIC(cur_mi), MENU_DISPLAY_DONE));
	    }
	}

	/*
	 * Call menu's MENU_GEN_PROC with MENU_DISPLAY_DONE
	 */
	if (m_gen_proc)  {
	    menu = m_gen_proc(menu, MENU_DISPLAY_DONE);
	    if (!menu)  {
		return;
	    }
	    menu_private = MENU_PRIVATE(menu);
	    menu_private->group_info = group_info;
	}

	/*
	 * Call menu's MENU_GEN_PROC with MENU_NOTIFY
	 */
	if (m_gen_proc)  {
	    menu = m_gen_proc(menu, MENU_NOTIFY);
	    if (!menu)  {
		return;
	    }
	    menu_private = MENU_PRIVATE(menu);
	    menu_private->group_info = group_info;
	}

	menu_item_private = MENU_ITEM_PRIVATE(menu_item);

	/*
	 * Don't do anything if menu item is currently inactive
	 */
	if (!menu_item_private->inactive)  {
	    /*
	     * Set MENU_PARENT to be current menu
	     */
	    menu_item_private->parent = menu_private;

	    /*
	     * Call menu item's MENU_GEN_PROC with MENU_NOTIFY
	     */
	    if (mi_gen_proc = menu_item_private->gen_proc)  {
	        menu_item = mi_gen_proc(menu_item, MENU_NOTIFY);
	    }

	    if (menu_item)  {
	        menu_item_private = MENU_ITEM_PRIVATE(menu_item);

	        /*
	         * Set MENU_PARENT to be current menu
	         */
	        menu_item_private->parent = menu_private;

	        /*
	         * Get menu item notify proc
	         * If it doesnt exist, get menu notify proc
	         */
	        notify_proc = menu_item_private->notify_proc ? 
				menu_item_private->notify_proc :
			        menu_private->notify_proc;
	        /* 
	         * Call notify_proc if it exists
	         */
	        if (notify_proc)  {
	            result = notify_proc(menu, menu_item);
	        }

		/*
		 * Call menu item's MENU_GEN_PROC with MENU_NOTIFY_DONE
		 */
	        if (mi_gen_proc)  {
	            mi_gen_proc(menu_item, MENU_NOTIFY_DONE);
	        }
	    }
	}

	/*
	 * Call menu's MENU_GEN_PROC with MENU_NOTIFY_DONE
	 */
	if (m_gen_proc)  {
	    m_gen_proc(menu, MENU_NOTIFY_DONE);
	}

	/*
	 * Call menu's MENU_DONE_PROC with result
	 */
	if (menu_private->done_proc)  {
	    menu_private->done_proc(menu, result);
	}

	/*
	menu_private->group_info = (Xv_menu_group_info *)NULL;
	*/

	/*
	 * Restore first/last event
	 */
	if (saved_event)  {
            group_info->first_event = save_first_event;
            group_info->last_event = save_last_event;
	}

}
/* ACC_XVIEW */
