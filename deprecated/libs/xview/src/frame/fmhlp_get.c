#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fmhlp_get.c 1.18 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/fm_impl.h>
#include <xview_private/frame_help.h>

/* ARGSUSED */
Pkg_private     Xv_opaque
frame_help_get_attr(frame_public, status, attr, valist)
    Frame           frame_public;
    int            *status;
    Frame_attribute attr;
    va_list         valist;
{
    register Frame_help_info *frame = FRAME_HELP_PRIVATE(frame_public);

    switch (attr) {

      case FRAME_SHOW_LABEL:
	attr = (Frame_attribute) ATTR_NOP(attr);
	return (Xv_opaque) status_get(frame, show_label);

      default:
	*status = XV_ERROR;
	return (Xv_opaque) 0;
    }
}
