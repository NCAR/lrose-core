#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fmprp_dsty.c 1.17 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/fm_impl.h>
#include <xview_private/fm_props.h>

static void     frame_props_free();

/* Destroy the frame struct */
Pkg_private int
frame_props_destroy(frame_public, status)
    Frame           frame_public;
    Destroy_status  status;
{
    Frame_props_info *frame = FRAME_PROPS_PRIVATE(frame_public);

    if (status == DESTROY_CLEANUP) {	/* waste of time if ...PROCESS_DEATH */
	xv_destroy(frame->panel);
	frame_props_free(frame);
    }
    return XV_OK;
}

/*
 * free the frame struct and all its resources.
 */
static void
frame_props_free(frame)
    Frame_props_info *frame;
{
    /* Free frame struct */
    free((char *) frame);
}
