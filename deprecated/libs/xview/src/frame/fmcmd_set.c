#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fmcmd_set.c 1.46 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <xview_private/draw_impl.h>
#include <xview_private/wmgr_decor.h>
#include <xview_private/frame_cmd.h>
#include <xview/panel.h>
#include <xview/server.h>

static int
  update_default_pin_state (Frame_cmd_info * frame, Xv_opaque server_public);

Pkg_private     Xv_opaque
frame_cmd_set_avlist(frame_public, avlist)
    Frame           frame_public;
    Attr_attribute  avlist[];
{
    Attr_avlist     attrs;
    Frame_cmd_info *frame = FRAME_CMD_PRIVATE(frame_public);
    Xv_Drawable_info *info;
    Xv_opaque       server_public;
    int             result = XV_OK;
    int             add_decor, delete_decor, set_win_attr;
    Atom            add_decor_list[WM_MAX_DECOR], delete_decor_list[WM_MAX_DECOR];

    DRAWABLE_INFO_MACRO(frame_public, info);
    server_public = xv_server(info);
    set_win_attr = add_decor = delete_decor = 0;

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (attrs[0]) {

	  case FRAME_CMD_PUSHPIN_IN:  

	    /* This attribute is overloaded for 2 different functionalities:
	       the initial state of the pin, and the current state of the
	       pin.  As a consequence it supports neither functionality 
	       accurately.  It is here only for compatibility reasons. Instead,
	       Use FRAME_CMD_DEFAULT_PIN_STATE and FRAME_CMD_PIN_STATE */

	    /* only change the state of the pin when the window is not map */
	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    if (status_get(frame, pushpin_in) == (int) attrs[1])
		break; 

	    if ((int) attrs[1])
		frame->win_attr.pin_initial_state = WMPushpinIsIn;
	    else
		frame->win_attr.pin_initial_state = WMPushpinIsOut;
	    status_set(frame, pushpin_in, (int) attrs[1]); 
	    set_win_attr = TRUE;
	    break;

	  case FRAME_CMD_DEFAULT_PIN_STATE:
	    /* defines pin state when the frame_cmd becomes mapped */
	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    status_set (frame, default_pin_state, (int) attrs[1]);
	    status_set (frame, default_pin_state_valid, (int) TRUE);
	    if (!xv_get(frame_public, XV_SHOW))
		set_win_attr |= update_default_pin_state (frame, server_public);
	    break;

	  case FRAME_CMD_PIN_STATE:
	    /* changes the current pin state.  Useless when unmapped */

	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    if ((int) attrs[1])
		frame->win_attr.pin_initial_state = WMPushpinIsIn;
	    else
		frame->win_attr.pin_initial_state = WMPushpinIsOut;

	    status_set (frame, pushpin_in, (int) attrs[1]);
	    set_win_attr = TRUE;
	    break;

	  case FRAME_SHOW_LABEL:	/* same as FRAME_SHOW_HEADER */
	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    if (status_get(frame, show_label) == (int) attrs[1])
		break;

	    status_set(frame, show_label, (int) attrs[1]);
	    
	    if ((int) attrs[1])
		add_decor++;
	    else
		delete_decor++;
	    break;

	  case FRAME_SHOW_RESIZE_CORNER:
	    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
	    if (status_get(frame, show_resize_corner) == (int) attrs[1])
		break;

	    status_set(frame, show_resize_corner, (int) attrs[1]);

	    if ((int) attrs[1])
		add_decor++;
	    else
		delete_decor++;
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

	  case XV_LABEL:
	    {
#ifdef OW_I18N
                extern wchar_t    *xv_app_name_wcs;
#endif
		extern char    *xv_app_name;
		Frame_class_info *frame_class = FRAME_CLASS_FROM_CMD(frame);

		*attrs = (Frame_attribute) ATTR_NOP(*attrs);
#ifdef OW_I18N
		if ((char *) attrs[1]) {
		    _xv_set_mbs_attr_dup(&frame_class->label,
					(char *) attrs[1]);
		} else {
		    _xv_set_wcs_attr_dup(&frame_class->label,
					 xv_app_name_wcs);
		}
#else
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
#endif
		(void) frame_display_label(frame_class);
		break;

#ifdef OW_I18N
          case XV_LABEL_WCS:
            {
                extern wchar_t    *xv_app_name_wcs;
                Frame_class_info *frame_class = FRAME_CLASS_FROM_CMD(frame);
 
                *attrs = (Frame_attribute) ATTR_NOP(*attrs);
		_xv_set_wcs_attr_dup(&frame_class->label,
				     (wchar_t *) attrs[1]
				        ? (wchar_t *) attrs[1]
					: xv_app_name_wcs);
                (void) frame_display_label(frame_class);
            }
            break;
#endif

	  case XV_SHOW:
		{
		    Frame_class_info *frame_class = FRAME_CLASS_FROM_CMD(frame);
		    

		    /* ignore this if we are mapping the window */
		    if (!attrs[1]) {
			/*
			 * ignore this if we are in the midst of dismissing
			 * the window
			 */
			if (status_get(frame_class, dismiss)) {
			    status_set(frame_class, dismiss, FALSE);
			    set_win_attr |= update_default_pin_state (frame, 
								     server_public);
			    break;
			}

			/* don't unmap the frame if the pushpin is in */

			if (status_get(frame, pushpin_in))
			    attrs[0] = (Frame_attribute) ATTR_NOP(attrs[0]);
			else {
			    set_win_attr |= update_default_pin_state (frame, 
								     server_public);
			}
		    } else {
		        int			data[6];
		        Rect			*rect;
		        Panel_item      	default_panel_item;
		        int			button_x, button_y;
		        
			/* This deals with the problem where someone does an
			 * XV_SHOW TRUE in the xv_create call.  In this
			 * case the panel has not been created yet, so we do


			 * so here.
			 */
                        if (!frame->panel) 
			    frame->panel = xv_create(frame_public, PANEL, NULL);
#ifdef OW_I18N
                        if (status_get(frame, warp_pointer) == TRUE) {
#endif

		        default_panel_item = (Panel_item) xv_get(frame->panel,
							    PANEL_DEFAULT_ITEM);
		        if (default_panel_item == XV_ZERO)
		            break;
		        rect = (Rect *) xv_get(default_panel_item,
							       PANEL_ITEM_RECT);
		        		        		 
		        win_translate_xy(frame->panel, frame_public, 
		        		 rect->r_left, rect->r_top, &button_x,
								     &button_y);
		        data[0] = button_x + rect->r_width / 2;
		        data[1] = button_y + rect->r_height / 2;
		        data[2] = button_x;
		        data[3] = button_y;
		        data[4] = rect->r_width;
		        data[5] = rect->r_height;

		        win_change_property(frame_public, 
		                            SERVER_WM_DEFAULT_BUTTON,
		                                      XA_INTEGER, 32,  data, 6);
#ifdef OW_I18N
                        }
#endif
		    }
		    break;
		}

#ifdef OW_I18N
          case FRAME_CMD_POINTER_WARP:
            *attrs = (Frame_attribute) ATTR_NOP(*attrs);
            status_set(frame, warp_pointer, attrs[1]);
            break;
#endif

	  case XV_END_CREATE:
		{
		    if (!frame->panel)
		    	frame->panel = xv_create(frame_public, PANEL, NULL);
		    break;
		}

	  default:
		break;

	    }
	}
    }

    if (set_win_attr)
	(void) wmgr_set_win_attr(frame_public, &(frame->win_attr));

    /* recompute wmgr decorations */

    if (add_decor || delete_decor) { 
	add_decor = delete_decor = 0;

	if (status_get(frame, show_label))
	    add_decor_list[add_decor++] =
		(Atom) xv_get(server_public, SERVER_WM_DECOR_HEADER);
	else
	    delete_decor_list[delete_decor++] =
		(Atom) xv_get(server_public, SERVER_WM_DECOR_HEADER);

	if (status_get(frame, show_resize_corner))
	    add_decor_list[add_decor++] =
		(Atom) xv_get(server_public, SERVER_WM_DECOR_RESIZE);
	else
	    delete_decor_list[delete_decor++] =
		(Atom) xv_get(server_public, SERVER_WM_DECOR_RESIZE);

	wmgr_add_decor(frame_public, add_decor_list, add_decor);

	wmgr_delete_decor(frame_public, delete_decor_list, delete_decor);
    }

    return (Xv_opaque) result;
}

static int
update_default_pin_state (Frame_cmd_info * frame, Xv_opaque server_public)
{
    int retval = FALSE;

    if (status_get(frame, default_pin_state_valid)) {
	retval = TRUE;
	if (status_get (frame, default_pin_state))
	    frame->win_attr.pin_initial_state = WMPushpinIsIn;
	else
	    frame->win_attr.pin_initial_state = WMPushpinIsOut;
    }
    return retval;
}
