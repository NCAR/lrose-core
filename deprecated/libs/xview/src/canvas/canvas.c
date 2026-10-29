#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)canvas.c 20.44 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/cnvs_impl.h>
#include <xview_private/win_keymap.h>
#include <xview_private/draw_impl.h> 

#ifdef	OW_I18N
#include <xview/font.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview_private/i18n_impl.h>  


Xv_private_data Attr_attribute  canvas_pew_key;
#endif /*OW_I18N*/

Xv_private_data Attr_attribute  canvas_context_key;
Xv_private_data Attr_attribute  canvas_view_context_key;

/* ARGSUSED */
Pkg_private int
canvas_init(parent, canvas_public, avlist)
    Xv_Window       parent;
    Canvas          canvas_public;
    Attr_attribute  avlist[];
{
    Xv_canvas          *canvas_object = (Xv_canvas *) canvas_public;
    Canvas_info        *canvas;

#ifdef OW_I18N
    Frame	        frame_public;
    Xv_pkg              *frame_type;
#endif /*OW_I18N*/

    if (canvas_context_key == (Attr_attribute) 0) {
	canvas_context_key = xv_unique_key();
#ifdef OW_I18N
	canvas_pew_key = xv_unique_key();
#endif /*OW_I18N*/
    }
    canvas = xv_alloc(Canvas_info);

    /* link to object */
    canvas_object->private_data = (Xv_opaque) canvas;
    canvas->public_self = canvas_public;

    status_set(canvas, fixed_image);
    status_set(canvas, auto_expand);
    status_set(canvas, auto_shrink);
    status_set(canvas, retained);

    /*
     * 1. Make all the paint windows inherit the WIN_DYNAMIC_VISUAL attribute.
     * 2. The Canvas is, by default, a First-Class (primary) focus client.
     */
    xv_set(canvas_public,
        WIN_INHERIT_COLORS, TRUE,
        XV_FOCUS_RANK, XV_FOCUS_PRIMARY,
#ifdef OW_I18N
	WIN_IM_PREEDIT_START,	canvas_preedit_start, canvas_public,
	WIN_IM_PREEDIT_DRAW,	canvas_preedit_draw, canvas_public,
	WIN_IM_PREEDIT_DONE,	canvas_preedit_done, canvas_public,
#endif
        NULL);

    return XV_OK;
}


Pkg_private int
canvas_destroy(canvas_public, stat)
    Canvas          canvas_public;
    Destroy_status  stat;
{
    Canvas_info    *canvas = CANVAS_PRIVATE(canvas_public);

    if (stat == DESTROY_CLEANUP) {
#ifdef OW_I18N
        /*
	 * All the canvases under one frame share the preedit window.
	 * Only when all the canvases have been destroyed, can we
	 * destroy the preedit window. So, we need to keep count of
	 * the canvases.
	 */
	Canvas_pew	   *pew;
	Frame		    frame_public;

	if (canvas->ic) {
	    /*
	     * Get the pew from frame to make sure, pew is still exist
	     * or not (when entire frame get destroy, pew may get
	     * destory first) instead of accessing the private data.
	     */
	    frame_public = (Frame) xv_get(canvas_public, WIN_FRAME);
	    pew = (Canvas_pew *) xv_get(frame_public,
					XV_KEY_DATA, canvas_pew_key);

	    if (pew != NULL) {
	        /*
		 * If the preedit window is still up and not pinned,
		 * make sure it is unmapped.
		 */
		if (status(canvas, preedit_exist)
	         && (--pew->active_count) <= 0) {
		    if (xv_get(pew->frame, FRAME_CMD_PIN_STATE)
						== FRAME_CMD_PIN_OUT) {
		        xv_set(pew->frame, XV_SHOW, FALSE, NULL);
		    }
		}

		/*
		 * If this is last canvas uses pew, and pew is not
		 * destoryed yet, let's destory it.
		 */
		if ((--pew->reference_count) <= 0) {
		    xv_destroy(pew->frame);
		    /*
		     * freeing pew itself and setting null to the pew
		     * will be done in the destroy interpose routine.
		     */
		}
	    }

	    /*
	     * Free the all preedit text cache information.
	     */
	    if (canvas->pe_cache) {
	        if (canvas->pe_cache->text->feedback) {
		    xv_free(canvas->pe_cache->text->feedback);
	        }
	        if (canvas->pe_cache->text->string.wide_char) {
		    xv_free(canvas->pe_cache->text->string.wide_char);
		}
		xv_free(canvas->pe_cache);
	    }
	}
#endif /*OW_I18N*/
	xv_free((char *) canvas);
    }
    return XV_OK;
}
