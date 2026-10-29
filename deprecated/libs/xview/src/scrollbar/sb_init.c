#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sb_init.c 1.46 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Module:	sb_init.c
 * 
 * Description:
 * 
 * Initializes scrollbar
 * 
 */


/*
 * Include files:
 */

#include <xview_private/sb_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/defaults.h>
#include <xview/svrimage.h>
#include <X11/Xlib.h>

Xv_private void window_set_bit_gravity();

Attr_attribute  sb_context_key;
Attr_attribute  sb_split_view_menu_item_key;
Attr_attribute  sb_join_view_menu_item_key;

/*
 * Declaration of Functions Defined in This File (in order):
 */

Pkg_private void scrollbar_init_common();
Pkg_private void scrollbar_create_standard_menu();

/******************************************************************/

Pkg_private void
scrollbar_init_common(sb)
    Xv_scrollbar_info *sb;
{
    Xv_Drawable_info *info;
    int              delay_time;
    int              page_interval, line_interval;

    DRAWABLE_INFO_MACRO(SCROLLBAR_PUBLIC(sb), info);
    sb->can_split = FALSE;
    sb->compute_scroll_proc = scrollbar_default_compute_scroll_proc;
    sb->creating = TRUE;
    sb->ginfo = (Graphics_info *) NULL;
    sb->last_view_start = 0;
    sb->menu = (Menu) 0;	/* Delay menu creation until first show */
    sb->jump_pointer = 
      defaults_get_boolean("scrollbar.jumpCursor", "Scrollbar.JumpCursor", 
			   (Bool) TRUE);
    sb->multiclick_timeout = 100 *
	defaults_get_integer_check("openWindows.multiClickTimeout",
				 "OpenWindows.MultiClickTimeout", 4, 1, 10);
    sb->object_length = 1 /*SCROLLBAR_DEFAULT_LENGTH */ ;
    sb->overscroll = 0;
    sb->page_length = SCROLLBAR_DEFAULT_LENGTH;
    sb->pixels_per_unit = 1;
    sb->prev_page = 1;
    sb->view_length = SCROLLBAR_DEFAULT_LENGTH;
    sb->view_start = 0;
    sb->window = xv_xid(info);
    sb->top_anchor_inverted = FALSE;
    sb->bottom_anchor_inverted = FALSE;
    sb->elevator_type = OLGX_NORMAL;
    sb->drag_repaint_percent = 100;
    
    /* create keys so client can hang data off objects */
    if (sb_context_key == (Attr_attribute) 0) {
	sb_context_key = xv_unique_key();
	sb_split_view_menu_item_key = xv_unique_key();
	sb_join_view_menu_item_key = xv_unique_key();
    }
    win_set_no_focus(SCROLLBAR_PUBLIC(sb), TRUE);
    window_set_bit_gravity(SCROLLBAR_PUBLIC(sb), ForgetGravity);

    xv_set(SCROLLBAR_PUBLIC(sb),
	   XV_SHOW, FALSE,
	   WIN_NOTIFY_SAFE_EVENT_PROC, scrollbar_handle_event,
	   WIN_NOTIFY_IMMEDIATE_EVENT_PROC, scrollbar_handle_event,
	   WIN_CONSUME_EVENTS,
	       ACTION_HELP,
	       WIN_UP_EVENTS,
	       LOC_DRAG,
	       WIN_MOUSE_BUTTONS,
	       0,
	   WIN_RETAINED,
	   xv_get(xv_get(SCROLLBAR_PUBLIC(sb), XV_SCREEN, NULL),
		  SCREEN_RETAIN_WINDOWS),
	   NULL);
    
    /* create the GCs that are used for rendering -- jcb 4/17/89 */
    sb_init_GCs(sb);

    /*
     * load delay times for scrollbar repeat from the .Xdefaults file and
     * make the call to set static variables in sb_event.c
     */

    delay_time = defaults_get_integer_check(
			   "scrollbar.repeatDelay", "Scrollbar.RepeatDelay",
					    SCROLLBAR_REPEAT_DELAY, 0, 999);

    page_interval = defaults_get_integer_check(
			 "scrollbar.pageInterval", "Scrollbar.PageInterval",
				    SCROLLBAR_REPEAT_PAGE_INTERVAL, 0, 999);

    line_interval = defaults_get_integer_check(
			 "scrollbar.lineInterval", "Scrollbar.LineInterval",
				    SCROLLBAR_REPEAT_LINE_INTERVAL, 0, 999);

    scrollbar_init_delay_values(delay_time, page_interval, line_interval);
}


Pkg_private void
scrollbar_create_standard_menu(sb)
    Xv_scrollbar_info *sb;
{
    sb->menu = (Menu) xv_create(XV_SERVER_FROM_WINDOW(SCROLLBAR_PUBLIC(sb)),
				MENU,
				MENU_TITLE_ITEM, "Scrollbar",
				MENU_ACTION_ITEM,
		     (sb->direction == SCROLLBAR_VERTICAL) ? "Here to top" :
				"Here to left",
				scrollbar_line_to_top,
				MENU_ACTION_ITEM,
		     (sb->direction == SCROLLBAR_VERTICAL) ? "Top to here" :
				"Left to here",
				scrollbar_top_to_line,
				MENU_ACTION_ITEM,
				"Previous", scrollbar_last_position,
				MENU_GEN_PROC, scrollbar_gen_menu,
				XV_KEY_DATA, sb_context_key, sb,
				XV_KEY_DATA, sb_split_view_menu_item_key,
				xv_create(0, MENUITEM,
	     MENU_ACTION_ITEM, "Split View", scrollbar_split_view_from_menu,
					  NULL),
				XV_KEY_DATA, sb_join_view_menu_item_key,
				xv_create(0, MENUITEM,
	      MENU_ACTION_ITEM, "Join Views", scrollbar_join_view_from_menu,
					  NULL),
				NULL);

}
