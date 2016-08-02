#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)frame_cmd.c 1.48 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <X11/Xlib.h>
#include <xview_private/fm_impl.h>
#include <xview_private/frame_cmd.h>
#include <xview_private/draw_impl.h>
#include <xview/cursor.h>
#include <xview/server.h>
#include <xview_private/svr_atom.h>

#include <X11/Xatom.h>

#if defined(WITH_3X_LIBC) || defined(vax)
/* 3.x - 4.0 libc transition code; old (pre-4.0) code must define the symbol */
#define jcsetpgrp(p)  setpgrp((p),(p))
#endif

extern Attr_avlist attr_find();

/*  static void     cmd_warp_pointer();  */

/* ARGSUSED */
Pkg_private int
frame_cmd_init(owner, frame_public, avlist)
    Xv_Window       owner;
    Frame           frame_public;
    Attr_attribute  avlist[];
{
    Xv_frame_cmd   *frame_object = (Xv_frame_cmd *) frame_public;
    Xv_Drawable_info *info;
    Xv_opaque       server_public;
    Frame_cmd_info *frame;
    Attr_avlist     attrs;
    int		    dont_show_resize_corner = TRUE;

    DRAWABLE_INFO_MACRO(frame_public, info);
    server_public = xv_server(info);
    frame = xv_alloc(Frame_cmd_info);

    /* link to object */
    frame_object->private_data = (Xv_opaque) frame;
    frame->public_self = frame_public;

    status_set(frame, warp_pointer, TRUE);

    /* set initial window decoration flags */
    frame->win_attr.flags = WMWinType | WMMenuType | WMPinState;
    frame->win_attr.win_type = (Atom) xv_get(server_public, SERVER_WM_WT_CMD);
    frame->win_attr.menu_type = (Atom) xv_get(server_public, SERVER_WM_MENU_LIMITED);
    frame->win_attr.pin_initial_state = WMPushpinIsOut;


    status_set(frame, show_label, TRUE);
    status_set(frame, pushpin_in, FALSE);		     
    status_set(frame, default_pin_state, FRAME_CMD_PIN_OUT); /* new attr */

    /* Wmgr default to have resize corner for cmd frame */
    status_set(frame, show_resize_corner, TRUE);   

    notify_interpose_event_func(frame_public, frame_cmd_input, NOTIFY_SAFE);
    notify_interpose_event_func(frame_public, frame_cmd_input,
				NOTIFY_IMMEDIATE);

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (*attrs) {

	  case FRAME_SCALE_STATE:
	    /*
	     * change scale and find the apprioprate size of the font Don't
	     * call frame_rescale_subwindows because they have not created
	     * yet.
	     */
	    /*
	     * WAIT FOR NAYEEM window_set_rescale_state(frame_public,
	     * attrs[1]);
	     */
	    wmgr_set_rescale_state(frame_public, attrs[1]);
	    break;
	  case FRAME_SHOW_RESIZE_CORNER:
	    dont_show_resize_corner = !attrs[1];
	    break;
	  default:
	    break;
	}
    }
    if (dont_show_resize_corner)
        (void)xv_set(frame_public, FRAME_SHOW_RESIZE_CORNER, FALSE, NULL);

    /* Flush the intial _OL_WIN_ATTR.  We were waiting until END_CREATE,
     * but that caused problems because the frame could be mapped before
     * the attr gets flushed.
     */
    (void) wmgr_set_win_attr(frame_public, &(frame->win_attr));

    return XV_OK;
}

Pkg_private     Notify_value
frame_cmd_input(frame_public, event, arg, type)
    Frame           frame_public;
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    Frame_cmd_info *frame = FRAME_CMD_PRIVATE(frame_public);
    unsigned int    action = event_action(event);

    switch (action) {
      case ACTION_PININ:
	status_set(frame, pushpin_in, TRUE);			 
	break;
      case ACTION_PINOUT:
	status_set(frame, pushpin_in, FALSE);			 /* old attr */
	break;
      case WIN_MAP_NOTIFY:
	/* cmd_warp_pointer(frame_public); */
	break;
      case WIN_UNMAP_NOTIFY:
	/*
	 * reset the warp_pointer flag so when the user invokes the popup
	 * again, we will warp the pointer
	 */
	status_set(frame, warp_pointer, TRUE);
	break;
    }

    return notify_next_event_func((Notify_client) frame_public,
				  (Notify_event) event, arg, type);
}

/*
 * Make sure the window is viewable before warping the pointer.  To do this,
 * wait for a WIN_REPAINT before setting the kbd focus.
 */
/* static void
cmd_warp_pointer(frame_public)
    Frame           frame_public;
{
    Frame_cmd_info *frame = FRAME_CMD_PRIVATE(frame_public);
    Rect           *item_rect;
    Panel_item      default_panel_item;

    if (!status_get(frame, warp_pointer))
	return;

    default_panel_item = (Panel_item) xv_get(frame->panel, PANEL_DEFAULT_ITEM);
    if (default_panel_item == NULL)
	return;

    item_rect = (Rect *) xv_get(default_panel_item, PANEL_ITEM_RECT);
    (void) win_setmouseposition(frame->panel, item_rect->r_left + item_rect->r_width / 2,
				item_rect->r_top + item_rect->r_height / 2);
    status_set(frame, warp_pointer, FALSE);
}  */
