#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_rop.c 20.55 89/07/31";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * xv_rop.c: Implements pw_write functions of the pixwin.h interface for X
 * server.
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <xview_private/pw_impl.h>

#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

#include <xview_private/cms_impl.h>
#include <xview_private/svrim_impl.h>
#include <xview_private/i18n_impl.h>

int   GC_CHAIN_KEY;

#include <xview/xv_xrect.h>

#define INVALID_XID		0

extern Xv_xrectlist *screen_get_clip_rects();

Xv_private_data int xv_to_xop[16];

Xv_private void
xv_init_x_pr()
{
    xv_to_xop[PIX_CLR >> PIX_OP_SHIFT] = GXclear;
    xv_to_xop[PIX_SET >> PIX_OP_SHIFT] = GXset;
    xv_to_xop[PIX_DST >> PIX_OP_SHIFT] = GXnoop;
    xv_to_xop[PIX_NOT(PIX_DST) >> PIX_OP_SHIFT] = GXinvert;
    xv_to_xop[PIX_SRC >> PIX_OP_SHIFT] = GXcopy;
    xv_to_xop[PIX_NOT(PIX_SRC) >> PIX_OP_SHIFT] = GXcopyInverted;
    xv_to_xop[(PIX_SRC & PIX_DST) >> PIX_OP_SHIFT] = GXand;
    xv_to_xop[(PIX_SRC & PIX_NOT(PIX_DST)) >> PIX_OP_SHIFT] = GXandReverse;
    xv_to_xop[(PIX_NOT(PIX_SRC) & PIX_DST) >> PIX_OP_SHIFT] = GXandInverted;
    xv_to_xop[(PIX_SRC ^ PIX_DST) >> PIX_OP_SHIFT] = GXxor;
    xv_to_xop[(PIX_SRC | PIX_DST) >> PIX_OP_SHIFT] = GXor;
    xv_to_xop[(PIX_NOT(PIX_SRC) & PIX_NOT(PIX_DST))
	      >> PIX_OP_SHIFT] = GXnor;
    xv_to_xop[(PIX_NOT(PIX_SRC) ^ PIX_DST) >> PIX_OP_SHIFT] = GXequiv;
    xv_to_xop[(PIX_SRC | PIX_NOT(PIX_DST)) >> PIX_OP_SHIFT] = GXorReverse;
    xv_to_xop[(PIX_NOT(PIX_SRC) | PIX_DST) >> PIX_OP_SHIFT] = GXorInverted;
    xv_to_xop[(PIX_NOT(PIX_SRC) | PIX_NOT(PIX_DST))
	      >> PIX_OP_SHIFT] = GXnand;
}

Xv_private void
xv_set_gc_op(display, info, gc, op, fg_mode, fg_bg)
    Display        *display;
    Xv_Drawable_info *info;
    GC              gc;
    int             op;
    short           fg_mode;
    int             fg_bg;
{
    unsigned long   	value_mask;
    XGCValues       	val;
    Cms_info         	*cms = CMS_PRIVATE(xv_cms(info));

    val.function = XV_TO_XOP(op);
    value_mask = GCForeground | GCBackground | GCFunction | GCPlaneMask;
    val.plane_mask = xv_plane_mask(info);

    if (info->is_bitmap) {		/* restrict bitmap colors to 1 and 0 */
	val.foreground = (fg_bg == XV_DEFAULT_FG_BG) ? 1 : 0;
	val.background = (fg_bg == XV_DEFAULT_FG_BG) ? 0 : 1;
    } else if (fg_mode == XV_USE_OP_FG) {
	if (fg_bg == XV_DEFAULT_FG_BG) {
	    val.foreground = XV_TO_X_PIXEL(PIX_OPCOLOR(op), cms);
	    val.background = xv_bg(info);
	} else {
	    val.background = XV_TO_X_PIXEL(PIX_OPCOLOR(op), cms);
	    val.foreground = xv_bg(info);
	}
    } else {
	if (fg_bg == XV_DEFAULT_FG_BG) {
	    val.foreground = xv_fg(info);
	    val.background = xv_bg(info);
	} else {
	    val.background = xv_fg(info);
	    val.foreground = xv_bg(info);
	}
    }

    switch (val.function) {
      case GXclear:
	val.foreground = val.background;
	val.function = GXcopy;
	break;

      case GXset:
	val.foreground = xv_fg(info);
	val.function = GXcopy;
	break;

      case GXxor:
	val.foreground = val.foreground ^ val.background;
	val.background = 0;
	break;

      case GXinvert:
	if (val.foreground == val.background) {
	    val.foreground = xv_fg(info);
	    val.background = xv_bg(info);
	}
	val.plane_mask = val.foreground ^ val.background;
	break;
    }

    XChangeGC(display, gc, value_mask, &val);
}

Xv_private int
xv_rop_internal(display, d, gc, x, y, width, height, src, xr, yr, dest_info)
    Display        *display;
    Drawable        d;
    GC              gc;
    int             x, y, width, height;
    Xv_opaque       src;
    int             xr, yr;
    Xv_Drawable_info *dest_info;
{
    Xv_Drawable_info *src_info;
    Drawable        src_d;
    XGCValues       changes;
    unsigned long   changes_mask = 0;

    if (width == 0 || height == 0 || !src) {
	return (XV_ERROR);
    }
    /*
     * If src is not a client pixrect, it can either be a window or a
     * server_image.
     */
    if (PR_NOT_MPR(((Pixrect *) src))) {

	DRAWABLE_INFO_MACRO(src, src_info);
	src_d = (Drawable) xv_xid(src_info);

	if (PR_IS_SERVER_IMAGE((Pixrect *) src)) {
	    /*
	     * Since src is a server image, avoid the overhead of NoExpose
	     * events by doing stippling/tiling.
	     */
	    changes.ts_x_origin = x;
	    changes.ts_y_origin = y;
	    changes_mask = GCTileStipXOrigin | GCTileStipYOrigin;
	    
	    /* clip to source dimensions */
	    width = (width > ((Pixrect *) src)->pr_size.x) ?
		((Pixrect *) src)->pr_size.x : width;
	    height = (height > ((Pixrect *) src)->pr_size.y) ?
		((Pixrect *) src)->pr_size.y : height;

	    /* stipple only if we have a bitmap src */
	    if (xv_depth(src_info) == 1) {
	      changes.stipple = xv_xid(src_info);
	      changes.fill_style = FillOpaqueStippled;
	      changes_mask |=  GCStipple | GCFillStyle;
	    } else if(xv_depth(dest_info) == xv_depth(src_info)) {
	      changes.tile = xv_xid(src_info);
	      changes.fill_style = FillTiled;
	      changes_mask |=  GCTile | GCFillStyle;
	    } else {
		xv_error(XV_ZERO,
			 ERROR_STRING,
			     XV_MSG("xv_rop: can't handle drawables of different depth"),
			 NULL);
		return (XV_ERROR);
	    }		
	    if (changes_mask) 
	      XChangeGC(display, gc, changes_mask, &changes);
	    XFillRectangle(display, d, gc, x, y, width, height);
	} else {
	    /* src is a window */
	    if (xv_depth(dest_info) == xv_depth(src_info)) {
		XCopyArea(display, src_d, d, gc, xr, yr, width, height, x, y);
	    } else {
		xv_error(XV_ZERO,
			 ERROR_STRING,
			     XV_MSG("xv_rop: Windows of different depth, can't rop"),
			 NULL);
		return (XV_ERROR);
	    }
	}
    } else {
	if (xv_rop_mpr_internal(display, d, gc, x, y, width, height, src, 
		xr, yr, dest_info, TRUE) == XV_ERROR)
	    return(XV_ERROR);
    }

    return(XV_OK);
}	


Xv_private int
xv_rop_mpr_internal(display, d, gc, x, y, width, height, src, xr, yr, 
	dest_info, mpr_bits)
    Display        	*display;
    Drawable        	d;
    GC              	gc;
    int             	x, y, width, height;
    Xv_opaque       	src;
    int             	xr, yr;
    Xv_Drawable_info 	*dest_info;
    short		mpr_bits;
{
    int             		 src_depth;
    XImage         		*ximage;
    Cms_info			*cms = CMS_PRIVATE(xv_cms(dest_info));
    static unsigned char	*data = (unsigned char *)NULL;
    static unsigned int		 last_size = 0;

    src_depth = ((Pixrect *) src)->pr_depth;
    /* 
     * In Sunview, this case is handled by setting all non-zero color values 
     * to 1's. This is currently a NO-OP in XView. This case must be 
     * handled by creating a separate array of data bits of setting non-zero 
     * pixel values to 1's.
     */
    if ((xv_depth(dest_info) == 1) && (src_depth > 1)) {
	return(XV_ERROR);
    }

    if (src_depth == 1) {
	if (!(ximage = xv_image_bitmap(dest_info))) {
	    Screen_visual     *visual;

	    visual = (Screen_visual *)xv_get(xv_screen(dest_info), SCREEN_DEFAULT_VISUAL);
	    xv_image_bitmap(dest_info) = ximage = 
		(XImage *) XCreateImage(display, visual->vinfo->visual,
				1, XYBitmap, 0,
				(char *) mpr_d(((Pixrect *) src))->md_image, 
				0, 0, MPR_LINEBITPAD,
				mpr_d(((Pixrect *) src))->md_linebytes);
	    if (!ximage) {
		return (XV_ERROR);
	    }
	}
    } else if ((src_depth == 8) && (xv_depth(dest_info) == 8)) {
        if (!(ximage = xv_image_pixmap(dest_info))) {
	    Screen_visual *visual;
	    
	    visual = (Screen_visual *)xv_get(xv_screen(dest_info), SCREEN_DEFAULT_VISUAL);
	    xv_image_pixmap(dest_info) = ximage = 
		(XImage *) XCreateImage(display, visual->vinfo->visual,
				8, ZPixmap, 0,
				(char *) mpr_d(((Pixrect *) src))->md_image,
				0, 0, MPR_LINEBITPAD,
				mpr_d(((Pixrect *) src))->md_linebytes);
	    if (!ximage) {
		return (XV_ERROR);
	    }
	}
    } else {
	return (XV_ERROR);
    }
					
    ximage->bitmap_unit = MPR_LINEBITPAD;
    ximage->bitmap_pad = MPR_LINEBITPAD;
    ximage->height = ((Pixrect *) src)->pr_height;
    ximage->width = ((Pixrect *) src)->pr_width;
    ximage->bytes_per_line = mpr_d(((Pixrect *) src))->md_linebytes;
    ximage->data = (char *) mpr_d(((Pixrect *) src))->md_image;

    /* 
     * The bitmap data being passed in might be in either of 2 formats:
     *    1. memory pixrect format.
     *    2. Xlib bitmap format.
     */
    if (mpr_bits == TRUE) {
    /* bitmap data is in memory pixrect format */
#ifdef i386
        ximage->byte_order = LSBFirst;
        /*
         * Check to see if the pixrect data was set by mpr_static(), or by
         * actually creating the pixrect with mem_create() and drawing into
         * it.
         */
        if (mpr_d((Pixrect *) src)->md_flags & MP_I386)
	    ximage->bitmap_bit_order = LSBFirst;
        else
	    ximage->bitmap_bit_order = MSBFirst;
#else
#if defined(ultrix) || defined(__alpha)
        ximage->byte_order = LSBFirst;
        ximage->bitmap_bit_order = MSBFirst;
#else
        ximage->byte_order = MSBFirst;
        ximage->bitmap_bit_order = MSBFirst;
#endif /* ~VAX */
#endif /* ~i386 */
    } else {
    /* bitmap data is in Xlib bitmap format */
	ximage->byte_order = LSBFirst;
	ximage->bitmap_bit_order = LSBFirst;
        if (src_depth == 1) 
          ximage->bytes_per_line = (width + 7) >> 3;
    }

    if (src_depth == 1) {
	XPutImage(display, d, gc, ximage, xr, yr, x, y,
	    MIN(width, ximage->width), MIN(height, ximage->height));
    } else {
	register int     i, j;
	unsigned long    index;
	unsigned int	 size;
	char		*image_data;
	
	/*
	 * Create any space needed to convert the image data to pixel values
	 */
	size = ximage->height * ximage->bytes_per_line;
	if (size > last_size) {
	    if (data)
	      xv_free(data);
	    data = (unsigned char *)xv_malloc(size);
	    last_size = size;
	}
	
	/* 
	 * convert image from cms indices to X pixel values.
	 */
	image_data = ximage->data;
	for (i = 0; i < ximage->height; i++) {
	    for (j = 0; j < ximage->bytes_per_line; j++) {
		index = j + i * ximage->bytes_per_line;
		data[index] = cms->index_table[(unsigned char)image_data[index]];
	    }
	}
	
	ximage->data = (char *)data;
	XPutImage(display, d, gc, ximage, xr, yr, x, y,
		  MIN(width, ximage->width), MIN(height, ximage->height));
	ximage->data = image_data;
    }
    return (XV_OK);
}

Pkg_private
xv_to_x_convert_image(ximage, val)
    XImage         *ximage;
    int             val;
{
    register int    i, j, index;

    for (i = 0; i < ximage->height; i++) {
	for (j = 0; j < ximage->bytes_per_line; j++) {
	    index = j + i * ximage->bytes_per_line;
	    ximage->data[index] = ((int) ximage->data[index]) + val;
	}
    }
}

Pkg_private     GC
xv_find_proper_gc(display, info, op)
    Display        	*display;
    Xv_Drawable_info 	*info;
    int	        	op;
{
    int             depth = xv_depth(info), i;
    Drawable        xid = xv_xid(info);
    XGCValues       gv;
    Xv_Screen       screen = xv_screen(info);
    Xv_xrectlist   *clip_xrects = screen_get_clip_rects(screen);
    short           xv_in_fullscreen = server_get_fullscreen(xv_server(info));
    struct gc_chain   *gcs, *gc_list, **ops_private_gcs;

    if (!GC_CHAIN_KEY)  
	    GC_CHAIN_KEY = xv_unique_key();
    ops_private_gcs = (struct gc_chain **) xv_get( screen, XV_KEY_DATA, GC_CHAIN_KEY );
    if (!ops_private_gcs) {
		ops_private_gcs = (struct gc_chain **) xv_calloc((PW_NUM_OPS+1),sizeof(struct gc_chain));
	    xv_set( screen, XV_KEY_DATA, GC_CHAIN_KEY, ops_private_gcs, 0 );
    }
    gc_list = ops_private_gcs[op];

    /*
     * If a new clipping rectangle has been set for this drawable since the
     * last invocation of this function, set all xid's in the gc list to an
     * invalid xid.
     */
    if (info->new_clipping) {
#ifndef SVR4
	for (i = 0; i < PW_NUM_OPS; i++)
#else /* SVR4 */
	for (i = 0; i <= PW_NUM_OPS; i++)
#endif /* SVR4 */
	    for (gcs = ops_private_gcs[i]; gcs != NULL; gcs = gcs->next)
		gcs->xid = INVALID_XID;
	info->new_clipping = FALSE;
    }
    if (!gc_list) {
	gc_list = ops_private_gcs[op] =
	    (struct gc_chain *) xv_calloc(1, sizeof(struct gc_chain));
	if (xv_in_fullscreen) {
	    gv.subwindow_mode = IncludeInferiors;
	    gc_list->gc = XCreateGC(display, xid, GCSubwindowMode, &gv);
	} else {
	    gc_list->gc = XCreateGC(display, xid, 0, 0);
	}
	gc_list->clipping_set = FALSE;
	gc_list->depth = depth;
	gc_list->next = NULL;

	/*
	 * Newly created GC. If clipping is enabled on the drawable, set the
	 * GCClipMask for the GC.
	 */
	if (clip_xrects->count) {
	    XSetClipRectangles(display, gc_list->gc, 0, 0,
		     clip_xrects->rect_array, clip_xrects->count, Unsorted);
	    gc_list->clipping_set = TRUE;
	}
	gc_list->xid = xid;
	return (gc_list->gc);
    } else {
	gcs = gc_list;
	while (gcs) {
	    if (gcs->depth == depth) {
		if (xv_in_fullscreen) {
		    gv.subwindow_mode = IncludeInferiors;
		} else {
		    gv.subwindow_mode = ClipByChildren;
		}

		/*
		 * The clipping_set field is redundant. A bug in XChangeGC
		 * (in Xlib) needs to fixed. If the current clip_mask and the
		 * cached clip_mask are both None, the cache need not be
		 * flushed. Remove clipping_set field when this gets fixed.
		 */
		if (gcs->clipping_set && !clip_xrects->count) {
		    gcs->clipping_set = FALSE;
		    gv.clip_mask = None;
		    XChangeGC(display, gcs->gc, GCSubwindowMode | GCClipMask, &gv);
		} else {
		    XChangeGC(display, gcs->gc, GCSubwindowMode, &gv);
		}

		/*
		 * If this is a different drawable since the last invocation
		 * and it has a clipping rectangle enabled, or, the clipping
		 * for the same drawable has changed since the last
		 * invocation, reset the clipping.
		 */
		if (clip_xrects->count && (gcs->xid != xid)) {
		    XSetClipRectangles(display, gcs->gc, 0, 0,
		     clip_xrects->rect_array, clip_xrects->count, Unsorted);
		    gcs->clipping_set = TRUE;
		}
		gcs->xid = xid;
		return (gcs->gc);
	    } else {
		if (gcs->next) {
		    gcs = gcs->next;
		} else {
		    struct gc_chain *new;
		    /*
		     * no gc of the same depth, need to create a new gc
		     */
		    gcs->next = new = (struct gc_chain *) xv_malloc(sizeof(struct gc_chain));
		    if (xv_in_fullscreen) {
			gv.subwindow_mode = IncludeInferiors;
			new->gc = XCreateGC(display, xid, GCSubwindowMode, &gv);
		    } else {
			new->gc = XCreateGC(display, xid, 0, 0);
		    }
		    new->depth = depth;
		    new->next = NULL;

		    /*
		     * Newly created GC. If clipping is enabled on the
		     * window, set the GC.
		     */
		    if (clip_xrects->count) {
			XSetClipRectangles(display, new->gc, 0, 0,
					   clip_xrects->rect_array, clip_xrects->count, Unsorted);
			new->clipping_set = TRUE;
		    }
		    new->xid = xid;
		    return (new->gc);
		}
	    }
	}
    }

}


Xv_public int
xv_rop(window, x, y, width, height, op, pr, xr, yr)
    Xv_opaque       window;
    int             op, x, y, width, height;
    Pixrect        *pr;
    int             xr, yr;
{
    register Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;

    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    d = (Drawable) xv_xid(info);
    if (pr == NULL) {
	GC	gc = xv_find_proper_gc(display, info, PW_ROP_NULL_SRC);

	xv_set_gc_op(display, info, gc, op, XV_USE_OP_FG,
		     XV_DEFAULT_FG_BG);
	XFillRectangle(display, d, gc, x, y, width, height);
    } else {
	GC	gc = xv_find_proper_gc(display, info, PW_ROP);

	xv_set_gc_op(display, info, gc, op,
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_DEFAULT_FG_BG);

	if (xv_rop_internal(display, d, gc, x, y, width, height,
			    (Xv_opaque) pr, xr, yr, info) == XV_ERROR) {
	    xv_error(XV_ZERO,
		     ERROR_STRING, 
		     XV_MSG("xv_rop: xv_rop_internal failed"),
		     NULL);
	}
    }
}

Xv_private void
xv_invert_polygon(pw, npts, outline, x, y, w, h, color_index)
    Xv_opaque       pw;		/* pixwin */
    int             npts;	/* number of points */
    struct pr_pos  *outline;	/* array of points */
    int             x, y;	/* top left corner of enclosing rectangle */
    int             w, h;	/* width and height of enclosing rectangle */
    int		    color_index;  /* -1 => use default foreground color */
{
    register int    i;
    register XPoint *points;
    Xv_Drawable_info *info;
    GC              gc;
    int		    color_op;
    int		    fg_type;

    DRAWABLE_INFO_MACRO(pw, info);

    gc = xv_find_proper_gc(xv_display(info), info, PW_ROP);
    if (color_index < 0) {
	color_op = PIX_NOT(PIX_DST);
	fg_type = XV_USE_CMS_FG;
    } else {
	color_op = PIX_NOT(PIX_DST) | PIX_COLOR(color_index);
	fg_type = XV_USE_OP_FG;
    }
    xv_set_gc_op(xv_display(info), info, gc, color_op, fg_type,
		 XV_DEFAULT_FG_BG);

    points = (XPoint *) xv_malloc(npts * sizeof(XPoint));
    /* BUG ALERT: convert int into short, might lose */
    for (i = 0; i < npts; i++) {
	points[i].x = x + outline[i].x;
	if (outline[i].x < 0)
	    points[i].x += w;
	points[i].y = y + outline[i].y;
	if (outline[i].y < 0)
	    points[i].y += h;
    }

    XFillPolygon(xv_display(info), xv_xid(info), gc, points,
		 npts, Convex, CoordModeOrigin);
    free(points);
}

Xv_private void
xv_gray_polygon(pw, npts, outline, x, y, w, h, gray_pr)
    Xv_opaque       pw;		/* pixwin */
    int             npts;	/* number of points */
    struct pr_pos  *outline;	/* array of points */
    int             x, y;	/* top left corner of enclosing rectangle */
    int             w, h;	/* width and height of enclosing rectangle */
    Pixrect        *gray_pr;	/* ptr to gray pixrect */
{
    register int    i;
    register struct pr_pos *vlist;	/* vector list */
    int             npts_array[1];

    vlist = (struct pr_pos *) xv_malloc(npts * sizeof(struct pr_pos));
    for (i = 0; i < npts; i++) {
	vlist[i].x = x + outline[i].x;
	if (outline[i].x < 0)
	    vlist[i].x += w;
	vlist[i].y = y + outline[i].y;
	if (outline[i].y < 0)
	    vlist[i].y += h;
    }
    npts_array[0] = npts;

    pw_polygon_2((struct pixwin *)pw, 0, 0, 1, npts_array, vlist, PIX_SRC | PIX_DST, gray_pr, 0, 0);
    free(vlist);
}


Xv_public int
xv_replrop(window, xw, yw, width, height, op, pr, xr, yr)
    Xv_opaque       window;
    int             op, xw, yw, width, height;
    Pixrect        *pr;
    int             xr, yr;
{
    register Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;

    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    d = (Drawable) xv_xid(info);
    if (pr) {
	GC              gc = xv_find_proper_gc(display, info, PW_ROP);

	xv_set_gc_op(display, info, gc, op,
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_DEFAULT_FG_BG);
	if (xv_replrop_internal(display, info, d, gc, xw, yw, width,
				height, pr, xr, yr, info) == XV_ERROR) {
	    xv_error(XV_ZERO,
		     ERROR_STRING, 
		     XV_MSG("xv_replrop: xv_replrop_internal failed"),
		     NULL);
	}
    } else {
	xv_rop(window, xw, yw, width, height, op, pr, xr, yr);
    }
}


Xv_private int
xv_replrop_internal(display, info, d, gc, xw, yw, width, height, src, xr, yr, dest_info)
    Display        *display;
    Xv_Drawable_info *info;
    GC              gc;
    int             xw, yw, width, height;
    Pixrect        *src;
    int             xr, yr;
    Xv_Drawable_info *dest_info;
{
    GC              tile_gc = NULL, xv_get_temp_gc();
    XID             tile;
    XGCValues       values;
    long unsigned   valuemask;
    Xv_Drawable_info *src_info;

    if (!PR_NOT_MPR(src)) {
	/*
	 * Since X does not support direct replrop of an image, we transfer
	 * the image to a new pixmap which becomes a tile for a rectangle
	 * fill (or a stipple if depth = 1, as tile must be as deep as
	 * destination drawable).
	 */
	if (!(tile = XCreatePixmap(display, d, src->pr_width, src->pr_height,
				   src->pr_depth))) {
	    return (XV_ERROR);
	}
	if (!(tile_gc = XCreateGC(display, tile, 0, 0))) {
		XFreePixmap(display, tile);
		return (XV_ERROR);
	}
	xv_set_gc_op(display, info, tile_gc, PIX_SRC, XV_USE_CMS_FG,
		     XV_DEFAULT_FG_BG);

	if (xv_rop_internal(display, tile, tile_gc, 0, 0, src->pr_width,
			src->pr_height, (Xv_opaque) src, 0, 0, dest_info)
	    == XV_ERROR) {
	    XFreePixmap(display, tile);
	    return (XV_ERROR);
	}
    } else {
	DRAWABLE_INFO_MACRO((Xv_opaque) src, src_info);
	tile = xv_xid(src_info);
    }
    valuemask = GCFillStyle | ((src->pr_depth == 1) ? GCStipple : GCTile) |
	GCTileStipXOrigin | GCTileStipYOrigin;
    if (src->pr_depth == 1) {
	values.fill_style = FillOpaqueStippled;
	values.stipple = tile;
    } else {
	values.fill_style = FillTiled;
	values.tile = tile;
    }
    values.ts_x_origin = xw + xr;
    values.ts_y_origin = yw + yr;
    XChangeGC(display, gc, valuemask, &values);
    XFillRectangle(display, d, gc, xw, yw, width, height);
    if (!PR_NOT_MPR(src)) {
	XFreePixmap(display, tile);
    }
    return (XV_OK);
}

Xv_private      GC
xv_get_temp_gc(display, xid, depth)
    Display        *display;
    unsigned int    xid;
    int             depth;
{
#ifndef SVR4
#define DIFF_DEPTHS	3
#define DEPTH_1		0
#define DEPTH_8		1
#define DEPTH_24	2
#else /* SVR4 */
#define DIFF_DEPTHS     6
#define DEPTH_1         0
#define DEPTH_2         1
#define DEPTH_4         2
#define DEPTH_8         3
#define DEPTH_16        4
#define DEPTH_24        5
#endif /* SVR4 */
    static GC       temp_gcs[DIFF_DEPTHS];
    short           index;

    if (depth == 1) {
	index = DEPTH_1;
#ifdef SVR4
    } else if (depth == 2) {
	index = DEPTH_2;
    } else if (depth == 4) {
	index = DEPTH_4;
#endif /* SVR4 */
    } else if (depth == 8) {
	index = DEPTH_8;
#ifdef SVR4
    } else if (depth == 16) {
	index = DEPTH_16;
#endif /* SVR4 */
    } else if (depth == 24) {
	index = DEPTH_24;
    } else {
	printf(XV_MSG("Unsupported frame buffer depth: %d\n"), depth);
	abort();
    }
    if (temp_gcs[index]) {
	return (temp_gcs[index]);
    } else {
	if (!(temp_gcs[index] = XCreateGC(display, xid, 0, 0))) {
	    printf(XV_MSG("Server probabaly run out of memory in XCreateGC\n"));
	    abort();
	} else {
	    return (temp_gcs[index]);
	}
    }
}


Xv_public Pw_pixel_cache *
pw_save_pixels(pw, rect)
    register Xv_opaque pw;
    register Rect  *rect;
{
    Xv_Drawable_info *info;
    Server_image    rpr;
    Pw_pixel_cache *pc;

    DRAWABLE_INFO_MACRO(pw, info);

    /* Allocate pixel cache */
    pc = xv_alloc(Pw_pixel_cache);
    pc->r = *rect;

    rpr = xv_create(xv_screen(info), SERVER_IMAGE,
		    XV_WIDTH, rect->r_width,
		    XV_HEIGHT, rect->r_height,
		    SERVER_IMAGE_DEPTH, xv_depth(info),
		    NULL);
    if (!rpr) {
	free(pc);
	return PW_PIXEL_CACHE_NULL;
    }
    xv_rop(rpr, 0, 0, rect->r_width, rect->r_height, PIX_SRC,
	   (Pixrect *) pw, rect->r_left, rect->r_top);
    pc->plane_group[0] = (struct pixrect *) rpr;
    return pc;
}

Xv_public void
pw_restore_pixels(pw, pc)
    register Xv_opaque pw;
    Pw_pixel_cache *pc;
{
    register Rect  *r = &pc->r;

    if (pc == PW_PIXEL_CACHE_NULL)
	return;
    xv_rop(pw, r->r_left, r->r_top, r->r_width, r->r_height, PIX_SRC,
	   pc->plane_group[0], 0, 0);

    /* Free the allocated pixmap and the pixel cache */
    xv_destroy((Xv_opaque)pc->plane_group[0]);
    free((caddr_t) pc);
}
