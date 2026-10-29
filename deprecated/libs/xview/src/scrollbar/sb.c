#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sb.c 1.53 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Module:	sb.c
 * 
 * 
 * Include files:
 */

#include <xview_private/sb_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/openwin.h>
#include <xview/panel.h>
#include <xview/xv_error.h>
#include <xview/defaults.h>
#include <xview/svrimage.h>
#include <xview/screen.h>
#include <xview/textsw.h>
#include <xview/termsw.h>
#include <X11/Xlib.h>

/*
 * Declaration of Functions Defined in This File (in order):
 */
Pkg_private int            scrollbar_create_internal();

/*
 * Package global data
 */
Attr_attribute  sb_context_key;
Attr_attribute  sb_split_view_menu_item_key;
Attr_attribute  sb_join_view_menu_item_key;

Xv_private void	win_set_no_focus();
Xv_private Graphics_info	*xv_init_olgx();
#ifndef __linux
Xv_private_data Defaults_pairs xv_kbd_cmds_value_pairs[4];
#else
extern Defaults_pairs xv_kbd_cmds_value_pairs[4];
#endif

/******************************************************************/

/*ARGSUSED*/ /*VARARGS2*/
Pkg_private int
scrollbar_create_internal(parent, sb_public, avlist)
    Xv_opaque       parent;
    Xv_opaque       sb_public;
    Xv_opaque      *avlist;
{
    Xv_scrollbar      *scrollbar = (Xv_scrollbar *) sb_public;
    Xv_scrollbar_info *sb;
    int               delay_time;
    int               page_interval, line_interval;
    Xv_Drawable_info  *info;
    int		      three_d = FALSE;

    sb = xv_alloc(Xv_scrollbar_info);
    sb->public_self = sb_public;
    scrollbar->private_data = (Xv_opaque) sb;
    sb->managee = parent;

    DRAWABLE_INFO_MACRO(sb_public, info);
    sb->direction = SCROLLBAR_VERTICAL;
    sb->elevator_state = 0;
    sb->size = SCROLLBAR_FULL_SIZE;
    sb->can_split = FALSE;
    sb->compute_scroll_proc = scrollbar_default_compute_scroll_proc;
    sb->creating = TRUE;
    if (xv_depth(info) > 1)
      three_d = defaults_get_boolean("OpenWindows.3DLook.Color",
				     "OpenWindows.3DLook.Color", TRUE);
    sb->ginfo = xv_init_olgx(SCROLLBAR_PUBLIC(sb), &three_d,
			     xv_get(SCROLLBAR_PUBLIC(sb), XV_FONT));
    sb->last_view_start = 0;
    sb->menu = (Menu) 0;	/* Delay menu creation until first show */
    if(defaults_exists("openWindows.scrollbarjumpCursor",
			"OpenWindows.ScrollbarJumpCursor")){
    	sb->jump_pointer = defaults_get_boolean("OpenWindows.scrollbarjumpCursor",
					    "OpenWindows.ScrollbarJumpCursor", 
					    (Bool) TRUE);
    }
    else {
    	sb->jump_pointer = defaults_get_boolean("scrollbar.jumpCursor",
					    "Scrollbar.JumpCursor", 
					    (Bool) TRUE);
    }
    sb->length = 1;
    sb->multiclick_timeout = 100 * 
      		defaults_get_integer_check("openWindows.scrollbarRepeatTimeout",
				 "OpenWindows.ScrollbarRepeatTimeout", 3, 1, 9);
    sb->object_length = 1 /*SCROLLBAR_DEFAULT_LENGTH */ ;
    sb->page_length = SCROLLBAR_DEFAULT_LENGTH;
    sb->pixels_per_unit = 1;
    sb->view_length = SCROLLBAR_DEFAULT_LENGTH;
    sb->view_start = 0;
    sb->length = 1;
    sb->window = xv_xid(info);
    sb->top_anchor_inverted = FALSE;
    sb->bottom_anchor_inverted = FALSE;
    sb->size = SCROLLBAR_FULL_SIZE;
    sb->transit_motion = SCROLLBAR_NONE;
    sb->last_motion = SCROLLBAR_NONE;
    sb->drag_repaint_percent = 100;
    
    /* create keys so client can hang data off objects */
    if (sb_context_key == (Attr_attribute) 0) {
	sb_context_key = xv_unique_key();
	sb_split_view_menu_item_key = xv_unique_key();
	sb_join_view_menu_item_key = xv_unique_key();
    }

    if (defaults_get_enum("openWindows.keyboardCommands",
			  "OpenWindows.KeyboardCommands",
			  xv_kbd_cmds_value_pairs) < KBD_CMDS_FULL ||
	xv_get(parent, XV_IS_SUBTYPE_OF, TERMSW) ||
	(parent && xv_get(parent, XV_IS_SUBTYPE_OF, TEXTSW))) {
	/* Mouseless operations disabled, or we're attached to a textsw
	 * (which does it's own keyboard-based scrolling):
	 * Don't allow the Scrollbar window to accept the keyboard focus.
	 * Otherwise, scrolling via clicking SELECT will remove the
	 * keyboard focus from it's client (e.g., a textsw).
	 * Note: Consuming ACTION_HELP sets KBD_USE in the input mask,
	 * which allows keyboard focus to be set on the Scrollbar Window.
	 * (See call to win_set_kbd_focus() in xevent_to_event() for more
	 * information.)
	 */
	win_set_no_focus(sb_public, TRUE);
    }

    xv_set(sb_public,
	   XV_SHOW, FALSE,
	   WIN_NOTIFY_SAFE_EVENT_PROC, scrollbar_handle_event,
	   WIN_NOTIFY_IMMEDIATE_EVENT_PROC, scrollbar_handle_event,
	   WIN_BIT_GRAVITY, ForgetGravity,
	   WIN_CONSUME_EVENTS,
	       ACTION_HELP,
	       WIN_UP_EVENTS,
	       LOC_DRAG,
	       WIN_MOUSE_BUTTONS,
	       0,
	   WIN_RETAINED,
	       xv_get(xv_get(sb_public, XV_SCREEN, NULL),
		      SCREEN_RETAIN_WINDOWS),
	   NULL);
    
    /*
     * load delay times for scrollbar repeat from the .Xdefaults file and
     * make the call to set static variables in sb_event.c
     */
    delay_time = defaults_get_integer_check("scrollbar.repeatDelay",
					    "Scrollbar.RepeatDelay",
					    SCROLLBAR_REPEAT_DELAY, 0, 999);
    page_interval = defaults_get_integer_check("scrollbar.pageInterval", 
					       "Scrollbar.PageInterval",
					       SCROLLBAR_REPEAT_PAGE_INTERVAL, 0, 999);
    line_interval = defaults_get_integer_check("scrollbar.lineInterval",
					       "Scrollbar.LineInterval",
					       SCROLLBAR_REPEAT_LINE_INTERVAL, 0, 999);
    scrollbar_init_delay_values(delay_time, page_interval, line_interval);
    return XV_OK;
}

Pkg_private void
scrollbar_create_standard_menu(sb)
    Xv_scrollbar_info *sb;
{
    sb->menu = (Menu) xv_create(XV_SERVER_FROM_WINDOW(SCROLLBAR_PUBLIC(sb)),
	MENU,
	MENU_GEN_PROC, scrollbar_gen_menu,
	XV_HELP_DATA, "xview:scrollbarMenu",
	MENU_TITLE_ITEM, XV_MSG("Scrollbar"),
	MENU_ITEM,
	    MENU_STRING, (sb->direction == SCROLLBAR_VERTICAL) ?
		XV_MSG("Here to top") : 
		XV_MSG("Here to left"),
	    MENU_NOTIFY_PROC, scrollbar_line_to_top,
	    XV_HELP_DATA, (sb->direction == SCROLLBAR_VERTICAL) ?
		"xview:scrollbarHereToTop" : "xview:scrollbarHereToLeft",
	    0,
	MENU_ITEM,
	    MENU_STRING, (sb->direction == SCROLLBAR_VERTICAL) ?
		XV_MSG("Top to here") : 
		XV_MSG("Left to here"),
	    MENU_NOTIFY_PROC, scrollbar_top_to_line,
	    XV_HELP_DATA, (sb->direction == SCROLLBAR_VERTICAL) ?
		"xview:scrollbarTopToHere" : "xview:scrollbarLeftToHere",
	    0,
	MENU_ITEM,
	    MENU_STRING, XV_MSG("Previous"),
	    MENU_NOTIFY_PROC, scrollbar_last_position,
	    XV_HELP_DATA, "xview:scrollbarPrevious",
	    0,
	XV_KEY_DATA, sb_context_key, sb,
	XV_KEY_DATA, sb_split_view_menu_item_key,
	    xv_create(0, MENUITEM,
		MENU_ACTION_ITEM, 
		    XV_MSG("Split View"), 
		    scrollbar_split_view_from_menu,
		XV_HELP_DATA, "xview:scrollbarSplitView",
		NULL),
	XV_KEY_DATA, sb_join_view_menu_item_key,
	    xv_create(0, MENUITEM,
		MENU_ACTION_ITEM, 
		    XV_MSG("Join Views"), 
		    scrollbar_join_view_from_menu,
		XV_HELP_DATA, "xview:scrollbarJoinViews",
		NULL),
	NULL);
}
