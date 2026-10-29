#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fmcmd_get.c 1.27 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/fm_impl.h>
#include <xview_private/frame_cmd.h>

/* ARGSUSED */
Pkg_private     Xv_opaque
frame_cmd_get_attr(frame_public, status, attr, valist)
    Frame           frame_public;
    int            *status;
    Frame_attribute attr;
    va_list         valist;
{
    register Frame_cmd_info *frame = FRAME_CMD_PRIVATE(frame_public);

    switch (attr) {

      case FRAME_SHOW_LABEL:
	attr = (Frame_attribute) ATTR_NOP(attr);
	return (Xv_opaque) status_get(frame, show_label);

      case FRAME_SHOW_RESIZE_CORNER:
	attr = (Frame_attribute) ATTR_NOP(attr);
	return (Xv_opaque) status_get(frame, show_resize_corner);

      case FRAME_CMD_PUSHPIN_IN:  /* attr. here only for compatibility */
      case FRAME_CMD_PIN_STATE:
	attr = (Frame_attribute) ATTR_NOP(attr);
	return (Xv_opaque) status_get(frame, pushpin_in);

      case FRAME_CMD_DEFAULT_PIN_STATE:
	attr = (Frame_attribute) ATTR_NOP(attr);
	return (Xv_opaque) status_get(frame, default_pin_state);

      case FRAME_CMD_PANEL:
	return (Xv_opaque) frame->panel;

      case FRAME_SCALE_STATE:
	attr = (Frame_attribute) ATTR_NOP(attr);
	/*
	 * WAIT FOR NAYEEM return (Xv_opaque)
	 * window_get_rescale_state(frame_public);
	 */
	return (Xv_opaque) 0;

      default:
	*status = XV_ERROR;
	return (Xv_opaque) 0;
    }
}
