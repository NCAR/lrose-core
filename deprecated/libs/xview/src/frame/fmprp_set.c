#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fmprp_set.c 1.36 93/06/28";
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
#include <xview_private/wmgr_decor.h>
#include <xview/panel.h>
#include <xview/server.h>


Pkg_private     Xv_opaque
frame_props_set_avlist(frame_public, avlist)
    Frame           frame_public;
    Frame_attribute avlist[];
{
    Frame_attribute *attrs;
    Frame_props_info *frame = FRAME_PROPS_PRIVATE(frame_public);
    Xv_Drawable_info *info = 0;
    Xv_opaque       server_public;
    int             result = XV_OK;
    int             add_decor, delete_decor, set_win_attr;
    Atom            add_decor_list[WM_MAX_DECOR], delete_decor_list[WM_MAX_DECOR];

    DRAWABLE_INFO_MACRO(frame_public, info);
    server_public = xv_server(info);
    set_win_attr = add_decor = delete_decor = 0;

    for (attrs = avlist; *attrs; attrs = frame_attr_next(attrs)) {
	switch (attrs[0]) {

	  case FRAME_PROPS_PUSHPIN_IN:
	    /*
	     * Only change the state of the pin when the window is not
	     * mapped.
	     */
	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    if ((int) attrs[1])
		frame->win_attr.pin_initial_state = (Atom) xv_get(server_public, SERVER_WM_PIN_IN);
	    else
		frame->win_attr.pin_initial_state = (Atom) xv_get(server_public, SERVER_WM_PIN_OUT);;
	    status_set(frame, pushpin_in, (int) attrs[1]);
	    set_win_attr = TRUE;
	    break;

	  case FRAME_SHOW_LABEL:	/* same as FRAME_SHOW_HEADER */
	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    if (status_get(frame, show_label) == (int) attrs[1])
		break;

	    status_set(frame, show_label, (int) attrs[1]);

	    if ((int) attrs[1])
		add_decor_list[add_decor++] =
		    (Atom) xv_get(server_public, SERVER_WM_DECOR_HEADER);
	    else
		delete_decor_list[delete_decor++] =
		    (Atom) xv_get(server_public, SERVER_WM_DECOR_HEADER);
	    break;

	  case FRAME_SHOW_FOOTER:
	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    if (status_get(frame, show_footer) == (int) attrs[1])
		break;

	    status_set(frame, show_footer, (int) attrs[1]);

	    if ((int) attrs[1])
		add_decor_list[add_decor++] =
		    (Atom) xv_get(server_public, SERVER_WM_DECOR_FOOTER);
	    else
		delete_decor_list[delete_decor++] =
		    (Atom) xv_get(server_public, SERVER_WM_DECOR_FOOTER);
	    break;

	  case FRAME_SCALE_STATE:
	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    /*
	     * set the local rescale state bit, then tell the WM the current
	     * state, and then set the scale of our subwindows
	     */
	    /*
	     * WAIT FOR NAYEEM window_set_rescale_state(frame_public,
	     * attrs[1]);
	     */
	    wmgr_set_rescale_state(frame_public, attrs[1]);
	    frame_rescale_subwindows(frame_public, attrs[1]);
	    break;

	  case FRAME_PROPS_RESET_PROC:
	    frame->reset_proc = (void (*) ()) attrs[1];
	    result = (int) XV_SET_DONE;
	    break;

	  case XV_LABEL:
	    {
		extern char    *xv_app_name;
		Frame_class_info *frame_class = FRAME_CLASS_FROM_PROPS(frame);

		*attrs = (Frame_attribute) ATTR_NOP(*attrs);
		if (frame_class->label) {
		    free(frame_class->label);
		}
		if ((char *) attrs[1]) {
		    frame_class->label = (char *) calloc(1,
					     strlen((char *) attrs[1]) + 1);
		    strcpy(frame_class->label, (char *) attrs[1]);
		} else {
		    if (xv_app_name) {
			frame_class->label = (char *) calloc(1,
						   strlen(xv_app_name) + 1);
			strcpy(frame_class->label, xv_app_name);
		    } else {
			frame_class->label = NULL;
		    }
		}
		(void) frame_display_label(frame_class);
		break;
	    }

	  case XV_SHOW:
	    {
		Frame_class_info *frame_class = FRAME_CLASS_FROM_PROPS(frame);

		/* ignore this if we are mapping the window */
		if (!attrs[1]) {
		    /*
		     * ignore this if we are in the midst of dismissing the
		     * window
		     */
		    if (status_get(frame_class, dismiss)) {
			status_set(frame_class, dismiss, FALSE);
			break;
		    }
		    /* don't unmap the frame if the pushpin is in */
		    if (status_get(frame, pushpin_in))
			attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
		}
		break;
	    }

	  case XV_END_CREATE:
	    set_win_attr = TRUE;
	    frame->panel = xv_create(frame_public, PANEL,
				     NULL);
	    break;

	  default:
	    break;

	}
    }

    if (set_win_attr)
	(void) wmgr_set_win_attr(frame_public, &(frame->win_attr));

    if (add_decor)
	wmgr_add_decor(frame_public, add_decor_list, add_decor);

    if (delete_decor)
	wmgr_add_decor(frame_public, delete_decor_list, delete_decor);


    return (Xv_opaque) result;
}
