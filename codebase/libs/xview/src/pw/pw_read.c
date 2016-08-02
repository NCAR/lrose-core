#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)pw_read.c 20.24 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * pw_read.c: Implements the read function of pixwin interface.
 */

#include <xview_private/i18n_impl.h>
#include <xview_private/pw_impl.h>

#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

Xv_public int
xv_read(pr, x, y, width, height, op, window, sx, sy)
    Pixrect        *pr;
    int             op, x, y, width, height;
    Xv_opaque       window;
    int             sx, sy;
{
    if (PR_IS_MPR(pr)) {
	Xv_Drawable_info *info;

	DRAWABLE_INFO_MACRO(window, info);
	xv_read_internal(pr, x, y, width, height, op, xv_display(info),
			 xv_xid(info), sx, sy);
    } else if (PR_IS_SERVER_IMAGE(pr)) {
	xv_rop((Xv_opaque)pr, x, y, width, height, op, (Pixrect *)window, sx, sy);
    } else {
	xv_error(XV_ZERO,
		 ERROR_STRING,
		     XV_MSG("xv_read: attempting to read into an invalid object"),
		 NULL);
    }
}

Xv_public int
xv_read_internal(pr, x, y, width, height, op, display, d, sx, sy)
    Pixrect        *pr;
    int             op, x, y, width, height;
    Display        *display;
    Drawable        d;
    int             sx, sy;
{
    register XImage *image;
    struct mpr_data image_mpr_data;
    struct pixrect  image_mpr;

    image = XGetImage(display, d, sx, sy, width, height,
		      AllPlanes, pr->pr_depth == 1 ? XYPixmap : ZPixmap);
    if (image->depth > pr->pr_depth) {
	xv_error((Xv_opaque)pr,
		 ERROR_SEVERITY, ERROR_NON_RECOVERABLE,
		 ERROR_STRING,
		     XV_MSG("xv_read_internal(): image depth > pixrect depth"),
		 NULL);
    }
    /* Fabricate a memory pixrect from the Xlib image */
    image_mpr.pr_ops = &mem_ops;
    image_mpr.pr_depth = image->depth;
    image_mpr.pr_height = image->height;
    image_mpr.pr_width = image->width;
    image_mpr.pr_data = (caddr_t) (&image_mpr_data);
    image_mpr_data.md_linebytes = image->bytes_per_line;
    image_mpr_data.md_image = (short *) image->data;
    image_mpr_data.md_offset.x = 0;
    image_mpr_data.md_offset.y = 0;
    image_mpr_data.md_primary = 0;
    image_mpr_data.md_flags = 0;
    /* Move the image from the memory pixrect into the destination pr */
    pr_rop(pr, x, y, width, height, op, &image_mpr, 0, 0);
    image->f.destroy_image(image);
}
