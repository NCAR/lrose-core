#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)cnvs_get.c 20.26 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/cnvs_impl.h>

/* ARGSUSED */
Pkg_private Xv_opaque
canvas_get_attr(canvas_public, stat, attr, valist)
    Canvas          canvas_public;
    int            *stat;
    Attr_attribute  attr;
    va_list         valist;
{
    Canvas_info    *canvas = CANVAS_PRIVATE(canvas_public);
    Canvas_view_info *view;
    Xv_Window       view_public, paint_window;
    Rect            view_rect, *canvas_rect;

    switch (attr) {
      case CANVAS_NTH_PAINT_WINDOW:
	view_public = (Xv_Window) xv_get(canvas_public, OPENWIN_NTH_VIEW, va_arg(valist, int));
	if (view_public != XV_NULL) {
	    return ((Xv_opaque) CANVAS_VIEW_PRIVATE(view_public)->paint_window);
	} else {
	    return ((Xv_opaque) NULL);
	}
      case CANVAS_HEIGHT:
	return (Xv_opaque) canvas->height;

      case CANVAS_WIDTH:
	return (Xv_opaque) canvas->width;

      case CANVAS_REPAINT_PROC:
	return (Xv_opaque) canvas->repaint_proc;

      case CANVAS_RESIZE_PROC:
	return (Xv_opaque) canvas->resize_proc;

      case CANVAS_AUTO_EXPAND:
	return (Xv_opaque) status(canvas, auto_expand);

      case CANVAS_AUTO_SHRINK:
	return (Xv_opaque) status(canvas, auto_shrink);

      case CANVAS_RETAINED:
	return (Xv_opaque) status(canvas, retained);

      case CANVAS_CMS_REPAINT:
	return (Xv_opaque) status(canvas, cms_repaint);

      case CANVAS_FIXED_IMAGE:
	return (Xv_opaque) status(canvas, fixed_image);

      case CANVAS_NO_CLIPPING:
       return (Xv_opaque) status(canvas, no_clipping);

      case CANVAS_VIEWABLE_RECT:
	paint_window = va_arg(valist, Xv_Window);
	if (paint_window != XV_NULL) {
	    view = CANVAS_VIEW_PRIVATE((Canvas_view) xv_get(paint_window, XV_OWNER));
	    if (view == NULL) {
		return (Xv_opaque) NULL;
	    }
	    view_rect = *(Rect *) xv_get(CANVAS_VIEW_PUBLIC(view), WIN_RECT);
	    canvas_rect = (Rect *) xv_get(paint_window, WIN_RECT);
	    canvas_rect->r_left = -canvas_rect->r_left;
	    canvas_rect->r_top = -canvas_rect->r_top;
	    canvas_rect->r_width = view_rect.r_width;
	    canvas_rect->r_height = view_rect.r_height;
	    return (Xv_opaque) canvas_rect;
	} else {
	    return (Xv_opaque) NULL;
	}

      case CANVAS_MIN_PAINT_WIDTH:
	return (Xv_opaque) canvas->min_paint_width;

      case CANVAS_MIN_PAINT_HEIGHT:
	return (Xv_opaque) canvas->min_paint_height;

#ifdef OW_I18N
      case CANVAS_IM_PREEDIT_FRAME:
	if (canvas->pew)
	    return (Xv_opaque) canvas->pew->frame; 
	else
	    return (Xv_opaque) NULL;
#endif
      case WIN_TYPE:		/* SunView1.X compatibility */
	return (Xv_opaque) CANVAS_TYPE;

      case OPENWIN_VIEW_CLASS:
	return (Xv_opaque) CANVAS_VIEW;

      default:
	xv_check_bad_attr(&xv_canvas_pkg, attr);
	*stat = XV_ERROR;
	return (Xv_opaque) 0;
    }
}
