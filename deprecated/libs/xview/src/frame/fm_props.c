#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fm_props.c 1.42 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <X11/Xlib.h>
#include <xview_private/fm_impl.h>
#include <xview_private/fm_props.h>
#include <xview_private/draw_impl.h>
#include <xview/cursor.h>
#include <xview_private/svr_atom.h>
#include <xview/server.h>


#include <X11/Xatom.h>

#if defined(WITH_3X_LIBC) || defined(vax)
/* 3.x - 4.0 libc transition code; old (pre-4.0) code must define the symbol */
#define jcsetpgrp(p)  setpgrp((p),(p))
#endif

extern Attr_avlist attr_find();

Pkg_private int
frame_props_init(owner, frame_public, avlist)
    Xv_Window       owner;
    Frame           frame_public;
    Frame_attribute avlist[];
{
    static          pid;
    Xv_frame_props *frame_object = (Xv_frame_props *) frame_public;
    Xv_Drawable_info *info = 0;
    Xv_opaque       server_public;
    Frame_props_info *frame;
    Frame_attribute *attrs;

    DRAWABLE_INFO_MACRO(frame_public, info);
    server_public = xv_server(info);
    frame = xv_alloc(Frame_props_info);

    /* link to object */
    frame_object->private_data = (Xv_opaque) frame;
    frame->public_self = frame_public;


    /* set initial window decoration flags */
    frame->win_attr.win_type = (Atom) xv_get(server_public, SERVER_WM_WT_CMD);
    frame->win_attr.menu_type = (Atom) xv_get(server_public, SERVER_WM_MENU_LIMITED);
    frame->win_attr.pin_initial_state = (Atom) xv_get(server_public, SERVER_WM_PIN_OUT);

    status_set(frame, show_label, TRUE);
    status_set(frame, show_footer, FALSE);
    status_set(frame, pushpin_in, FALSE);
#ifdef OW_I18N
    status_set(frame, show_IMstatus, FALSE);
#endif 

    notify_interpose_event_func(frame_public, frame_props_input, NOTIFY_SAFE);
    notify_interpose_event_func(frame_public, frame_props_input,
				NOTIFY_IMMEDIATE);

    for (attrs = avlist; *attrs; attrs = frame_attr_next(attrs)) {
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


	  default:
	    break;
	}
    }

    return XV_OK;
}

Pkg_private     Notify_value
frame_props_input(frame_public, event, arg, type)
    Frame           frame_public;
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    Frame_props_info *frame = FRAME_PROPS_PRIVATE(frame_public);
    unsigned int    action = event_action(event);

    switch (action) {
      case ACTION_PININ:
	status_set(frame, pushpin_in, TRUE);
	break;
      case ACTION_PINOUT:
	status_set(frame, pushpin_in, FALSE);
	break;
    }

    return notify_next_event_func((Notify_client) frame_public,
				  (Notify_event) event, arg, type);
}
