#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fm_menu.c 20.19 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/fm_impl.h>
#include <xview/openmenu.h>

static void     frame_menu_redisplay(),
                frame_menu_quit(), frame_menu_props();

/* ARGSUSED */
static void
frame_menu_redisplay(menu, mi)
    Menu            menu;
    Menu_item       mi;
{
    Frame           frame_public = menu_get(mi, MENU_CLIENT_DATA);
    Frame_class_info *frame = FRAME_CLASS_PRIVATE(frame_public);
    register Xv_Window sw;

    /*
     * BUG: none of this works, since we are not setting the damage clipping
     * list.  Anyway, this code will all go away when the external window
     * manager is implemented.
     */
    /*
     * post a repaint event to each subwindow and the frame. use SAFE since
     * we only have safe event handlers.
     */
    FRAME_EACH_SHOWN_SUBWINDOW(frame, sw)
	win_post_id(sw, WIN_REPAINT, NOTIFY_SAFE);
    FRAME_END_EACH
	win_post_id(frame_public, WIN_REPAINT, NOTIFY_SAFE);
}


/* ARGSUSED */
static void
frame_menu_quit(menu, mi)
    Menu            menu;
    Menu_item       mi;
{
    Frame           frame = menu_get(mi, MENU_CLIENT_DATA);

    (void) xv_destroy(frame);
}



/* ARGSUSED */
static void
frame_menu_props(menu, mi)
    Menu            menu;
    Menu_item       mi;
{
    Frame           frame = menu_get(mi, MENU_CLIENT_DATA);

    frame_handle_props(frame);
}
