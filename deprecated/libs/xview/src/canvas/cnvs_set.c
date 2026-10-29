#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)cnvs_set.c 20.48 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/cnvs_impl.h>
#include <xview/scrollbar.h>
#include <xview/attr.h>
#include <xview_private/draw_impl.h>
#include <X11/Xlib.h>

extern void     window_set_bit_gravity();

static void     canvas_set_bit_gravity();
static void     canvas_append_paint_attrs();


Pkg_private Xv_opaque
canvas_set_avlist(canvas_public, avlist)
    Canvas          canvas_public;
    Attr_avlist     avlist;
{
    Canvas_info    *canvas = CANVAS_PRIVATE(canvas_public);
    Attr_attribute  attr;
    int             width = 0;
    int             height = 0;
    int             vsb_set = 0, hsb_set = 0;
    Scrollbar       vsb = XV_NULL, hsb = XV_NULL;
    short           new_paint_size = FALSE;
    short           recheck_paint_size = FALSE;
    int             ok = TRUE;
    Xv_Window       paint_window;
    Rect            pw_rect;

    for (attr = avlist[0]; attr;
	 avlist = attr_next(avlist), attr = avlist[0]) {
	switch (attr) {
	  case CANVAS_WIDTH:
	    if (canvas->width != (int) avlist[1]) {
		width = (int) avlist[1];
		new_paint_size = TRUE;
	    }
	    break;

	  case CANVAS_HEIGHT:
	    if (canvas->height != (int) avlist[1]) {
		height = (int) avlist[1];
		new_paint_size = TRUE;
	    }
	    break;

	  case CANVAS_MIN_PAINT_WIDTH:
	    if (canvas->min_paint_width != (int) avlist[1]) {
		canvas->min_paint_width = (int) avlist[1];
		new_paint_size = TRUE;
	    }
	    break;

	  case CANVAS_MIN_PAINT_HEIGHT:
	    if (canvas->min_paint_height != (int) avlist[1]) {
		canvas->min_paint_height = (int) avlist[1];
		new_paint_size = TRUE;
	    }
	    break;

	  case CANVAS_VIEW_MARGIN:
	    /* This is a hold over from SunView, it is just a no-op for
	     * the time being
	     */
	    break;

	  case CANVAS_X_PAINT_WINDOW:
	    if ((int) avlist[1] == status(canvas, x_canvas)) {
		break;
	    }
	    if (avlist[1]) {
		status_set(canvas, x_canvas);
	    } else {
		status_reset(canvas, x_canvas);
	    }
	    CANVAS_EACH_PAINT_WINDOW(canvas_public, paint_window)
		xv_set(paint_window, WIN_X_PAINT_WINDOW, avlist[1], NULL);
	    CANVAS_END_EACH
		break;

	  case CANVAS_NO_CLIPPING:
	      if ((int) avlist[1] == status(canvas, no_clipping)) {
		  break;
	      }
	      if (avlist[1]) {
		status_set(canvas, no_clipping);
	      } else {
		status_reset(canvas, no_clipping);
	      }
	      CANVAS_EACH_PAINT_WINDOW(canvas_public, paint_window)
		  xv_set(paint_window, WIN_NO_CLIPPING, avlist[1], NULL);
	      CANVAS_END_EACH
	      break;

	  case CANVAS_REPAINT_PROC:
	    canvas->repaint_proc = (Function) avlist[1];
	    break;

	  case CANVAS_RESIZE_PROC:
	    canvas->resize_proc = (Function) avlist[1];
	    break;

	  case CANVAS_AUTO_EXPAND:
	    if ((int) avlist[1] == status(canvas, auto_expand))
		break;
	    if (avlist[1])
		status_set(canvas, auto_expand);
	    else
		status_reset(canvas, auto_expand);
	    recheck_paint_size = TRUE;
	    break;

	  case CANVAS_AUTO_SHRINK:
	    if ((int) avlist[1] == status(canvas, auto_shrink))
		break;
	    if (avlist[1])
		status_set(canvas, auto_shrink);
	    else
		status_reset(canvas, auto_shrink);
	    recheck_paint_size = TRUE;
	    break;

	  case CANVAS_RETAINED:
	    if ((int) avlist[1] == status(canvas, retained)) {
		break;
	    }
	    if (avlist[1]) {
		status_set(canvas, retained);
	    } else {
		status_reset(canvas, retained);
	    }
	    CANVAS_EACH_PAINT_WINDOW(canvas_public, paint_window)
		xv_set(paint_window, WIN_RETAINED, avlist[1], NULL);
	    CANVAS_END_EACH
		break;

	  case CANVAS_CMS_REPAINT:
	    if (avlist[1]) {
		status_set(canvas, cms_repaint);
	    } else {
		status_reset(canvas, cms_repaint);
	    }
	    break;

	  case CANVAS_FIXED_IMAGE:
	    /* don't do anything if no change */
	    if (status(canvas, fixed_image) != (int) avlist[1]) {
		if ((int) avlist[1]) {
		    status_set(canvas, fixed_image);
		} else {
		    status_reset(canvas, fixed_image);
		}
		canvas_set_bit_gravity(canvas);
	    }
	    break;

	  case WIN_VERTICAL_SCROLLBAR:
	  case OPENWIN_VERTICAL_SCROLLBAR:
	    vsb = (Scrollbar) avlist[1];
	    vsb_set = TRUE;
	    break;

	  case WIN_HORIZONTAL_SCROLLBAR:
	  case OPENWIN_HORIZONTAL_SCROLLBAR:
	    hsb = (Scrollbar) avlist[1];
	    hsb_set = TRUE;
	    break;

	  case WIN_SET_FOCUS:
	    ATTR_CONSUME(avlist[0]);
	    ok = FALSE;
	    CANVAS_EACH_PAINT_WINDOW(canvas_public, paint_window)
		Xv_Drawable_info *pw_info;
		DRAWABLE_INFO_MACRO(paint_window, pw_info);
		if (!xv_no_focus(pw_info) &&
		    win_getinputcodebit((Inputmask *) xv_get(paint_window,
			WIN_INPUT_MASK), KBD_USE)) {
		    (void)win_set_kbd_focus(paint_window, xv_xid(pw_info));
		    ok = TRUE;
		    break;
		}
	    CANVAS_END_EACH
	    break;

	  case CANVAS_PAINTWINDOW_ATTRS:
	    if (status(canvas, created)) {
		CANVAS_EACH_PAINT_WINDOW(canvas_public, paint_window)
		    xv_set_avlist(paint_window, &(avlist[1]));
		CANVAS_END_EACH
	    } else {
		canvas_append_paint_attrs(canvas, &(avlist[1]));
	    }
	    break;

	  case WIN_CMS_CHANGE:
	     if (status(canvas, created)) {
		 Xv_Drawable_info	*info;
		 Xv_Window      	view_public;
		 Canvas_view_info 	*view;
		 Cms			cms;
		 int			cms_fg, cms_bg;

		 DRAWABLE_INFO_MACRO(canvas_public, info);
		 cms = xv_cms(info);
		 cms_fg = xv_cms_fg(info);
		 cms_bg = xv_cms_bg(info);
		 OPENWIN_EACH_VIEW(canvas_public, view_public)
		     view = CANVAS_VIEW_PRIVATE(view_public);
		     (void)window_set_cms(view_public, cms, cms_bg, cms_fg);
		     window_set_cms(view->paint_window, cms, cms_bg, cms_fg);
		 OPENWIN_END_EACH
	     }
	     break;

#ifdef OW_I18N
	  case WIN_IC:
	     canvas->ic = (XIC) avlist[1];
	     break;
#endif /*OW_I18N*/

	  case XV_END_CREATE:

	    /* adjust paint window here rather then view end */
	    /* create because canvas_resize_paint_window */
	    /* assumes view window is known to canvas */
	    paint_window = (Xv_Window) xv_get(canvas_public, CANVAS_NTH_PAINT_WINDOW, NULL);
	    pw_rect = *(Rect *) xv_get(paint_window, WIN_RECT);
	    canvas_resize_paint_window(canvas, pw_rect.r_width, pw_rect.r_height);

	    if (status(canvas, no_clipping)) {
		CANVAS_EACH_PAINT_WINDOW(canvas_public, paint_window)
		    xv_set(paint_window, WIN_NO_CLIPPING, TRUE, NULL);
		CANVAS_END_EACH
	    }
	    break;

	  default:
	    xv_check_bad_attr(&xv_canvas_pkg, attr);
	    break;
	}
    }

    if (!status(canvas, created)) {
	/* copy width and height if set */
	if (width != 0) {
	    canvas->width = width;
	}
	if (height != 0) {
	    canvas->height = height;
	}

    } else {

	if (new_paint_size) {
	    canvas_resize_paint_window(canvas, width, height);
	} else if (recheck_paint_size) {
	    canvas_resize_paint_window(canvas, canvas->width, canvas->height);
	}
    }

    if (vsb_set) {
	canvas_set_scrollbar_object_length(canvas, SCROLLBAR_VERTICAL, vsb);
    }
    if (hsb_set) {
	canvas_set_scrollbar_object_length(canvas, SCROLLBAR_HORIZONTAL, hsb);
    }
    return (Xv_opaque) (ok ? XV_OK : XV_ERROR);
}

static void
canvas_set_bit_gravity(canvas)
    Canvas_info    *canvas;
{
    Xv_Window       paint_window;
    int             bit_value;

    if (status(canvas, fixed_image)) {
	bit_value = NorthWestGravity;
    } else {
	bit_value = ForgetGravity;
    }

    CANVAS_EACH_PAINT_WINDOW(CANVAS_PUBLIC(canvas), paint_window)
	window_set_bit_gravity(paint_window, bit_value);
    CANVAS_END_EACH
}

static void
canvas_append_paint_attrs(canvas, argv)
    Canvas_info    *canvas;
    Attr_avlist     argv;
{
    if (canvas->paint_avlist == NULL) {
	canvas->paint_avlist = (Attr_avlist) xv_alloc_n(Canvas_attribute, ATTR_STANDARD_SIZE);
	canvas->paint_end_avlist = canvas->paint_avlist;
    }
    canvas->paint_end_avlist = (Attr_avlist) attr_copy_avlist(canvas->paint_end_avlist, argv);
}
