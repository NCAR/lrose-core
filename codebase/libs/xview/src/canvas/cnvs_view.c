#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)cnvs_view.c 20.56 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/cnvs_impl.h>
#include <xview_private/win_keymap.h>
#include <xview/scrollbar.h>
#include <xview/rect.h>
#include <xview/rectlist.h>

#ifdef  OW_I18N
#include <xview_private/draw_impl.h>
#include <xview/font.h>

Pkg_private Notify_value	canvas_pew_event_proc();

extern wchar_t	_xv_null_string_wc[];

#endif /*OW_I18N*/

static          canvas_view_create_paint_window();

/* ARGSUSED */
Pkg_private int
canvas_view_init(parent, view_public, avlist)
    Canvas          parent;
    Canvas_view     view_public;
    Attr_attribute  avlist[];
{
    Xv_canvas_view *view_object = (Xv_canvas_view *) view_public;
    Canvas_view_info *view;
    int             ret;

    view = xv_alloc(Canvas_view_info);

    /* link to object */
    view_object->private_data = (Xv_opaque) view;
    view->public_self = view_public;
    view->private_canvas = CANVAS_PRIVATE(parent);

#ifdef OW_I18N
    if ((ret = canvas_view_create_paint_window(view, avlist)) != XV_OK) {
#else
    if ((ret = canvas_view_create_paint_window(view)) != XV_OK) {
#endif /*OW_I18N*/
	xv_free(view);
	return (ret);
    }
    xv_set(view_public,
	   WIN_NOTIFY_SAFE_EVENT_PROC, canvas_view_event,
	   WIN_NOTIFY_IMMEDIATE_EVENT_PROC, canvas_view_event,
	   WIN_CONSUME_PICK_EVENTS,
	   WIN_RESIZE, 0,
	   NULL);

    return XV_OK;
}

/*ARGSUSED*/ /*VARARGS*/
Pkg_private Xv_opaque
canvas_view_get(view_public, stat, attr, valist)
    Canvas_view     view_public;
    int            *stat;
    Attr_attribute  attr;
    va_list         valist;
{
    Canvas_view_info *view = CANVAS_VIEW_PRIVATE(view_public);

    *stat = XV_OK;

    switch (attr) {
      case CANVAS_VIEW_PAINT_WINDOW:
	return ((Xv_opaque) view->paint_window);

      case CANVAS_VIEW_CANVAS_WINDOW:
	return ((Xv_opaque) CANVAS_PUBLIC(view->private_canvas));

#ifdef OW_I18N
      case WIN_IC:
	ATTR_CONSUME(attr);
	return (Xv_opaque) view->private_canvas->ic;
#endif /*OW_I18N*/

      default:
	xv_check_bad_attr(&xv_canvas_view_pkg, attr);
	*stat = XV_ERROR;
	return (Xv_opaque) 0;
    }
}


/*ARGSUSED*/ /*VARARGS*/
Pkg_private Xv_opaque
canvas_paint_get(paint_public, stat, attr, valist)
    Canvas_paint_window paint_public;
    int            *stat;
    Attr_attribute  attr;
    va_list         valist;
{
    Canvas_view_info *view;
    Canvas_info    *canvas;

    switch (attr) {
      case CANVAS_PAINT_CANVAS_WINDOW:
	canvas = (Canvas_info *) xv_get(paint_public,
					XV_KEY_DATA, canvas_context_key);
	return (Xv_opaque) CANVAS_PUBLIC(canvas);

      case CANVAS_PAINT_VIEW_WINDOW:
	view = (Canvas_view_info *) xv_get(paint_public,
					   XV_KEY_DATA, canvas_view_context_key);
	return (Xv_opaque) CANVAS_VIEW_PUBLIC(view);

#ifdef OW_I18N
      case WIN_IC:
	ATTR_CONSUME(attr);
	canvas = (Canvas_info *) xv_get(paint_public,
					XV_KEY_DATA, canvas_context_key);
	return (Xv_opaque) canvas->ic;
#endif /*OW_I18N*/

      default:
	xv_check_bad_attr(&xv_canvas_pw_pkg, attr);
	*stat = XV_ERROR;
	return (Xv_opaque) 0;
    }
}

/*ARGSUSED*/ /*VARARGS*/
Pkg_private Xv_opaque
canvas_paint_set(paint_public, avlist)
    Canvas_paint_window paint_public;
    Attr_avlist		avlist;
{
    Attr_attribute	attr;
    Xv_opaque           result = XV_OK;

#ifdef OW_I18N
    Canvas_info 	*canvas = (Canvas_info *) xv_get(paint_public,
                                            XV_KEY_DATA, canvas_context_key);
#else
    Canvas_info    	*canvas;
#endif /*OW_I18N*/

    for (attr = avlist[0]; attr;
         avlist = attr_next(avlist), attr = avlist[0]) {
        switch (attr) {
	    case WIN_CMS_CHANGE: 
#ifndef OW_I18N
		canvas = (Canvas_info *) xv_get(paint_public,
                                XV_KEY_DATA, canvas_context_key);
#endif /*OW_I18N*/
                if (status(canvas, cms_repaint)) {
                    Rect                rect;
                    Rectlist    rl;
                    rect.r_left = 0;
                    rect.r_top = 0;
                    rect.r_width = (short)xv_get(paint_public, WIN_WIDTH);
                    rect.r_height = (short)xv_get(paint_public, WIN_HEIGHT);
                    rl = rl_null;
                    rl_rectunion(&rect, &rl, &rl);

                    win_set_damage(paint_public, &rl);
                    canvas_inform_repaint(canvas, paint_public);
                    win_clear_damage(paint_public);
                }
                break;

#ifdef OW_I18N
	    case WIN_IC:
		canvas->ic = (XIC) avlist[1];
		break;
	    
	    case XV_END_CREATE: {
		Frame		frame_public;
		Xv_pkg		*object_type;
		Xv_pkg  	*frame_type;
		Canvas		canvas_public;
		Xv_object 	serverobj;


		if (!xv_get(paint_public, WIN_USE_IM))
		    break;
		if (canvas->ic)
		    break;

		canvas_public = CANVAS_PUBLIC(canvas);

		object_type = (Xv_pkg *) xv_get(canvas_public, XV_TYPE);
		if (object_type->attr_id == (Attr_pkg) ATTR_PKG_PANEL)
		    break;

		/*
		 * Do we really have an IM ?
		 */
		serverobj = XV_SERVER_FROM_WINDOW(canvas_public);
		if ((XIM) xv_get(serverobj, XV_IM) == NULL)
		    break;

		frame_public = (Frame) xv_get(canvas_public, WIN_FRAME);
		frame_type = (Xv_pkg *) xv_get(frame_public, XV_TYPE);

#ifdef notdef
		/*
		 * Here is code to create the pew only on the base
		 * frame.  This is currently #ifdef out to allow popup
		 * frame to have a own pew.
		 */
		if (!strcmp(frame_type->name, "Frame_cmd")) {        
		    frame_public = (Frame)xv_get(frame_public, XV_OWNER);
		    frame_type = (Xv_pkg *)xv_get(frame_public, XV_TYPE);
		}

		if (strcmp(frame_type->name, "Frame_base"))
		    break;
#endif

		canvas->pew = (Canvas_pew *) xv_get(frame_public,
					     XV_KEY_DATA, canvas_pew_key);
		if (canvas->pew == NULL) {
		    /*
		     * This is the first time creating canvas with IM
		     * on this paricular frame.
		     */
 		    canvas->pew = canvas_create_pew(frame_public);

		    /*
		     * Get the pe_cache from panel which panel
		     * created.  We do not have to duplicates.
		     */
		    canvas->pe_cache = panel_get_preedit(
		       ((Xv_panel_or_item *) canvas->pew->ptxt)->private_data);

		    /*
		     * If the frame is popup frame (FRAME_CMD), we
		     * need to catch the WIN_CLOSE and WIN_OPEN event
		     * to sync up with base frame operation (ie, if
		     * this popup frame closes, pew should be close as
		     * well).  However this does not happen
		     * automatically, since pew will be child of the
		     * base frame.
		     */
		    if (strcmp(frame_type->name, "Frame_cmd") == 0) {
		        notify_interpose_event_func(frame_public,
						    canvas_pew_event_proc,
						    NOTIFY_SAFE);
		    }
		} else {
		    canvas->pe_cache =
			(XIMPreeditDrawCallbackStruct *)
			    xv_alloc(XIMPreeditDrawCallbackStruct);
		    canvas->pe_cache->text = (XIMText *) xv_alloc(XIMText);
		    canvas->pe_cache->text->encoding_is_wchar = True;
		    /*
		     * We have to set some value in string, otherwise
		     * panel code dumps.
		     */
		    canvas->pe_cache->text->string.wide_char
					= wsdup(_xv_null_string_wc);
		}

	        canvas->pew->reference_count++;

		canvas->ic = (XIC) xv_get(canvas_public, WIN_IC);
		if (canvas->ic == NULL)
		    break;

#ifdef FULL_R5
		XGetICValues(canvas->ic, XNInputStyle, &canvas->xim_style, NULL);

#endif /* FULL_R5 */

		/*
		 * DEPEND_ON_BUG_1102972: This "#ifdef notdef" (else
		 * part) is a workaround for the bug 1102972.  We have
		 * to delay the setting of the XNFocusWindow to much
		 * later (after all event mask was set to paint window.
		 * Actually in this case will do in the event_proc with
		 * KBD_USE).
		 */
#ifdef notdef
		canvas->focus_pwin = paint_public;
		/* 
		 * Cache the XNFocusWindow whenever it is set 
		 */
		 window_set_ic_focus_win(view_public, canvas->ic,
		 			xv_get(paint_public, XV_XID));
#else
		canvas->focus_pwin = 0;
#endif

		break;
	    }
#endif /*OW_I18N*/
	    default:
		xv_check_bad_attr(&xv_canvas_pw_pkg, attr);
		break;
	}
    }


    return(result);
}
		
Pkg_private int
canvas_view_destroy(view_public, stat)
    Canvas_view     view_public;
    Destroy_status  stat;
{
    Canvas_view_info *view = CANVAS_VIEW_PRIVATE(view_public);

    if ((stat == DESTROY_CLEANUP) || (stat == DESTROY_PROCESS_DEATH)) {
#ifdef OW_I18N
	Canvas_info		*canvas = view->private_canvas;
	Xv_Drawable_info	*info;


	/*
	 * Make sure that XNFocusWindow does not pointing to the
	 * window which being destroyed.
	 */
	if (canvas->ic) {
	    if (view->paint_window == canvas->focus_pwin) {
		/*
		 * Current XNFocusWindow is the one which we are
		 * destroying right now.  Try to find other paint
		 * window (if we are destroying the last view/paint,
		 * then we do not need worry about this).
		 */
	        Xv_Window	pw;

		CANVAS_EACH_PAINT_WINDOW(CANVAS_PUBLIC(canvas), pw)
		    if (pw != view->paint_window) {
			DRAWABLE_INFO_MACRO(pw, info);
			/* 
			 * Cache the XNFocusWindow whenever it is set 
			 */
			window_set_ic_focus_win(view_public, canvas->ic, 
						xv_xid(info));
			break;
		    }
		CANVAS_END_EACH
	    }
	}
#endif /* OW_I18N */
	if (xv_destroy_status(view->paint_window, stat) != XV_OK) {
	    return XV_ERROR;
	}
	if (stat == DESTROY_CLEANUP)
	    free((char *) view);
    }
    return XV_OK;
}

#ifdef OW_I18N
static int
canvas_view_create_paint_window(view, avlist)
    Canvas_view_info	*view;
    Attr_avlist		avlist;
#else
static int
canvas_view_create_paint_window(view)
    Canvas_view_info	*view;
#endif /*OW_I18N*/
{
    Canvas_view     view_public = CANVAS_VIEW_PUBLIC(view);
    Canvas_info    *canvas = view->private_canvas;
    Canvas          canvas_public = CANVAS_PUBLIC(canvas);
    Xv_Window       split_paint;
    Scrollbar       sb;

#ifdef OW_I18N
    Bool	    use_im;
    Attr_avlist	    attrs;
#endif /*OW_I18N*/

    if (canvas->width == 0) {
	canvas->width = (int) xv_get(view_public, WIN_WIDTH);
    }
    if (canvas->height == 0) {
	canvas->height = (int) xv_get(view_public, WIN_HEIGHT);
    }
#ifdef OW_I18N
    use_im = (Bool) xv_get(canvas_public, WIN_USE_IM);

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
        switch (attrs[0]) {

	    case WIN_USE_IM:
		use_im = (int) attrs[1];
		break;

            default:
                break;
 
        }
    }
#endif /*OW_I18N*/

    if (canvas->paint_avlist == NULL) {
	view->paint_window = xv_create(view_public, CANVAS_PAINT_WINDOW,
#ifdef OW_I18N
				       WIN_USE_IM, use_im,
#endif /*OW_I18N*/
				       WIN_WIDTH, canvas->width,
				       WIN_HEIGHT, canvas->height,
			     WIN_NOTIFY_SAFE_EVENT_PROC, canvas_paint_event,
			WIN_NOTIFY_IMMEDIATE_EVENT_PROC, canvas_paint_event,
				     WIN_RETAINED, status(canvas, retained),
			       WIN_X_PAINT_WINDOW, status(canvas, x_canvas),
				    XV_KEY_DATA, canvas_context_key, canvas,
				    XV_KEY_DATA, canvas_view_context_key, view,
				       NULL);
    } else {
	view->paint_window = xv_create(view_public, CANVAS_PAINT_WINDOW,
				       ATTR_LIST, canvas->paint_avlist,
#ifdef OW_I18N
					WIN_USE_IM, use_im,
#endif /*OW_I18N*/
				       WIN_WIDTH, canvas->width,
				       WIN_HEIGHT, canvas->height,
			     WIN_NOTIFY_SAFE_EVENT_PROC, canvas_paint_event,
			WIN_NOTIFY_IMMEDIATE_EVENT_PROC, canvas_paint_event,
				     WIN_RETAINED, status(canvas, retained),
			       WIN_X_PAINT_WINDOW, status(canvas, x_canvas),
				    XV_KEY_DATA, canvas_context_key, canvas,
				    XV_KEY_DATA, canvas_view_context_key, view,
				       NULL);
	xv_free(canvas->paint_avlist);
	canvas->paint_avlist = canvas->paint_end_avlist = NULL;
    }

    if (view->paint_window == XV_NULL) {
	return ((int) XV_ERROR);
    }

#ifdef OW_I18N
    if (canvas->ic)
        xv_set(view->paint_window, WIN_IC, canvas->ic, NULL);
#endif
    
    if (status(canvas, created)) {
	split_paint = (Xv_Window) xv_get(canvas_public, CANVAS_NTH_PAINT_WINDOW, NULL);
	if (split_paint != XV_NULL) {
	    Xv_opaque   defaults_array[ATTR_STANDARD_SIZE];
	    Attr_avlist defaults = defaults_array;
	    Xv_opaque   value;
	    
	    /* inherit certain attributes from the split window */
	    value = xv_get(split_paint, WIN_BACKGROUND_PIXMAP, NULL);
	    if (value) {
		*defaults++ = (Xv_opaque)WIN_BACKGROUND_PIXMAP;
		*defaults++ = xv_get(split_paint, WIN_BACKGROUND_PIXMAP, NULL);
	    }		
	    
	    *defaults++ = (Xv_opaque)WIN_BIT_GRAVITY;
	    *defaults++ = xv_get(split_paint, WIN_BIT_GRAVITY, NULL);

	    *defaults++ = (Xv_opaque)WIN_COLOR_INFO;
	    *defaults++ = xv_get(split_paint, WIN_COLOR_INFO, NULL);
	    
            *defaults++ = (Xv_opaque)WIN_COLUMN_GAP;
	    *defaults++ = xv_get(split_paint, WIN_COLUMN_GAP, NULL);

            *defaults++ = (Xv_opaque)WIN_COLUMN_WIDTH;
	    *defaults++ = xv_get(split_paint, WIN_COLUMN_WIDTH, NULL);

            *defaults++ = (Xv_opaque)WIN_CURSOR;
	    *defaults++ = xv_get(split_paint, WIN_CURSOR, NULL);

	    *defaults++ = (Xv_opaque)WIN_EVENT_PROC;
	    *defaults++ = xv_get(split_paint, WIN_EVENT_PROC, NULL);

	    *defaults++ = (Xv_opaque)WIN_ROW_GAP;
	    *defaults++ = xv_get(split_paint, WIN_ROW_GAP, NULL);

	    *defaults++ = (Xv_opaque)WIN_ROW_HEIGHT;
	    *defaults++ = xv_get(split_paint, WIN_ROW_HEIGHT, NULL);

	    *defaults++ = (Xv_opaque)WIN_WINDOW_GRAVITY;
	    *defaults++ = xv_get(split_paint, WIN_WINDOW_GRAVITY, NULL);

	    *defaults++ = (Xv_opaque)WIN_X_EVENT_MASK;
	    *defaults++ = xv_get(split_paint, WIN_X_EVENT_MASK, NULL);

	    /* null terminate the list */
	    *defaults   = (Xv_opaque)0;
	    
	    /* propagate the attrs to the new paint window */
	    (void)xv_set_avlist(view->paint_window, defaults_array);
	    
	    /* Deal with possible scrollbars */
	    sb = (Scrollbar)xv_get(canvas_public, OPENWIN_VERTICAL_SCROLLBAR,
				   view_public);
	    if (sb != XV_NULL) {
		canvas_scroll(view->paint_window, sb);
	    }
	    sb = (Scrollbar)xv_get(canvas_public, OPENWIN_HORIZONTAL_SCROLLBAR,
				   view_public);
	    if (sb != XV_NULL) {
		canvas_scroll(view->paint_window, sb);
	    }
	}
    } else {
	xv_set(view->paint_window,
	       WIN_BIT_GRAVITY,
	            status(canvas, fixed_image) ? NorthWestGravity : ForgetGravity,
	       WIN_CONSUME_EVENTS,
	           KBD_USE,
	           KBD_DONE,
	           WIN_ASCII_EVENTS,
	           ACTION_HELP,
	           WIN_MOUSE_BUTTONS,
	           0,
	       NULL);
	status_set(canvas, created);
    }

    return (XV_OK);
}
