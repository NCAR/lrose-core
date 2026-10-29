#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fmcmd_dsty.c 1.19 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/fm_impl.h>
#include <xview_private/frame_cmd.h>

/* Destroy the frame struct */
Pkg_private int
frame_cmd_destroy(frame_public, status)
    Frame           frame_public;
    Destroy_status  status;
{
    Frame_cmd_info *frame = FRAME_CMD_PRIVATE(frame_public);

    if (status == DESTROY_CLEANUP) {
	free((char *) frame);
    }

    return XV_OK;
}
