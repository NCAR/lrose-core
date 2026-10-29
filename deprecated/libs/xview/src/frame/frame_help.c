#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)frame_help.c 1.27 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <X11/Xlib.h>
#include <xview_private/fm_impl.h>
#include <xview_private/frame_help.h>
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


/* ARGSUSED */
Pkg_private int
frame_help_init(owner, frame_public, avlist)
    Xv_Window       owner;
    Frame           frame_public;
    Frame_attribute avlist[];
{
    Xv_frame_help  *frame_object = (Xv_frame_help *) frame_public;
    Xv_Drawable_info *info;
    Xv_opaque       server_public;
    Frame_help_info *frame;

    DRAWABLE_INFO_MACRO(frame_public, info);
    server_public = xv_server(info);
    frame = xv_alloc(Frame_help_info);

    /* link to object */
    frame_object->private_data = (Xv_opaque) frame;
    frame->public_self = frame_public;

    /* set initial window decoration flags */
    frame->win_attr.flags = WMWinType | WMMenuType | WMPinState;
    frame->win_attr.win_type = (Atom) xv_get(server_public, SERVER_WM_WT_HELP);
    frame->win_attr.menu_type = (Atom) xv_get(server_public,
					      SERVER_WM_MENU_LIMITED);
    frame->win_attr.pin_initial_state = WMPushpinIsIn;

    status_set(frame, show_label, TRUE);

    return XV_OK;
}
