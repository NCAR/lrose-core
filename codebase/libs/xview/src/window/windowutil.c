#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)windowutil.c 20.102 93/06/28";
#endif
#endif
/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifdef _XV_DEBUG
#include <xview_private/xv_debug.h>
#endif

#include <stdio.h>
#include <ctype.h>
#ifdef SVR4
#include <sys/types.h>
#include <stdlib.h>
#endif /* SVR4 */
#include <sys/file.h>
#include <xview/notify.h>
#include <xview_private/windowimpl.h>
#include <xview_private/draw_impl.h>
#include <xview/server.h>
#include <xview/screen.h>
#include <xview/frame.h>
#include <xview/font.h>
#include <xview/sel_svc.h>
#include <xview/sel_attrs.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <xview_private/i18n_impl.h>

#define WIN_SET_FLAG(deaf, win, flag) \
	((deaf) ? WIN_SET_DEAF(win, flag) : WIN_SET_LOOP(win, flag))
#define WIN_IS_FLAG(deaf, win) ((deaf) ? WIN_IS_DEAF(win) : WIN_IS_LOOP(win))


extern void     print_request();
Xv_private void win_get_cmdline_option();
Xv_private void win_set_wm_command_prop();
Xv_private char		*xv_app_name;
Xv_private char		*xv_instance_app_name;

/*
 * Static functions
 */
static void     adjust_rect_obj();


void
window_release_event_lock(window)
    Xv_Window       window;
{
    /*
     * BUG: get rid of this? (void) win_release_event_lock(window);
     */
}

void
window_refuse_kbd_focus(window)
    Xv_Window       window;
{
    (void) win_refuse_kbd_focus(window);
}

int
window_read_event(window, event)
    Xv_Window       window;
    Event          *event;
{
    return input_readevent(window, event);
}


void
window_bell(window)
    Xv_Window       window;
{
    struct timeval  tv;

    tv.tv_sec = 0;
    tv.tv_usec = 100000;	/* 1/10 second */
    (void) win_bell(window, tv, window);
}


Pkg_private int
win_appeal_to_owner(adjust, win, op, d1, d2)
    int             adjust;
    register Window_info *win;
    caddr_t         op;
    caddr_t         d1, d2;
{
    /* BUG: is adjust use right here? */

    /*
     * BUG: hack for the root window.  Call the default layout to allow
     * clients to get WIN_RECT/X/Y/etc. for the root window.
     */
    if (!win->owner) {
	/* must be a root window */
	window_layout(WIN_PUBLIC(win), WIN_PUBLIC(win), op, d1, d2);
	return adjust;
    }
    if (win->owner && win->owner->layout_proc) {
	(win->owner->layout_proc) (WIN_PUBLIC(win->owner), WIN_PUBLIC(win),
				   op, d1, d2);




	return adjust;
    }
    return FALSE;
}

/* NOTE: this proc is the bottom-level event_func for all windows. */

Pkg_private     Notify_value
window_default_event_func(win_public, event, arg, type)
    Xv_Window       win_public;
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    register Window_info *win = WIN_PRIVATE(win_public);
	Xv_opaque            server_public;
	Xv_Drawable_info    *info;

	DRAWABLE_INFO_MACRO(win_public, info);
	server_public = xv_server(info);

    switch (event_action(event)) {
      case ACTION_RESCALE: {
          Xv_Font  new_font = (win->font) ?
				(Xv_Font) xv_find(win_public, FONT,
			  			FONT_RESCALE_OF, win->font, arg,
						NULL) :
				(Xv_Font) 0;

          if (new_font)
	      xv_set(win_public, XV_FONT, new_font, NULL);
	}
	break;
      case KBD_USE:
      case KBD_DONE: {
#ifdef OW_I18N
        if (event_action(event) == KBD_DONE) {
	    if (win->win_use_im && win->xic)
		XUnsetICFocus(win->xic);
            check_lang_mode(NULL,NULL,NULL);
	} else {
	    if (win->win_use_im && win->xic) {
	        if (win->ic_active)
	       	    XSetICFocus(win->xic);
		else 
		    XUnsetICFocus(win->xic);
	    }
	}
#else
        if (event_action(event) == KBD_DONE)
            check_lang_mode(NULL,NULL,NULL);
#endif /* OW_I18N */
	if (win->softkey_flag) {
	    register Xv_Drawable_info 	*info;
	    unsigned long 		 TimeStamp;
	    unsigned long     		 ldata[2];
	    Xv_server 			 server;
	    XID             		 xv_get_softkey_xid(),
            				 sfk_win;

	    /* send a _OL_SOFT_KEY_LABELS message asking the soft key process
	     * update its labels 
	     */
	    DRAWABLE_INFO_MACRO(win_public,info);		
	    server = (Xv_server) xv_server(info);
            TimeStamp = xv_get(server,SERVER_FOCUS_TIMESTAMP);
	    ldata[0] = (event_action(event) == KBD_USE) ? xv_xid(info) : 0 ;
	    ldata[1] = TimeStamp; 

	    sfk_win = (Window) xv_get_softkey_xid(server, xv_display(info));
	    if (sfk_win != None) 
	        xv_send_message(win_public, sfk_win, "_OL_SOFTKEY_LABELS", 32,
				ldata, 16);
	}
      }
      break;

      case SEL_CLEAR:{
	    XSelectionClearEvent *clear_event;
		
		clear_event = (XSelectionClearEvent *) event->ie_xevent;

	    if (!xv_sel_handle_selection_clear(clear_event))
		selection_agent_clear(server_public, clear_event);
	    break;
	}


      case SEL_REQUEST:{
	    XSelectionRequestEvent *req_event;
		
		req_event = (XSelectionRequestEvent *)event->ie_xevent;

	    if (!xv_sel_handle_selection_request(req_event))
		selection_agent_selectionrequest(server_public, req_event);
	    break;
	}

      case SEL_NOTIFY:{
	    XSelectionEvent *sel_event;
		
		sel_event = (XSelectionEvent *) event->ie_xevent;
	    xv_sel_handle_selection_notify(sel_event);
	    break;
	}
      default:
	break;
    }
    if (win->event_proc)
	(win->event_proc) (win_public, event, arg);
    return NOTIFY_DONE;
}


Xv_private      GC
window_private_gc(win_public)
    Xv_Window       win_public;
{
    return ((GC) xv_get(win_public, XV_KEY_DATA, XV_GC));
}

/*
 * Only called for siblings -- hence not necessary to go to server Compute
 * the rect of source in the coordinate space of dest_child's parent. This is
 * used to get e.g. the rect of a subwindow relative to a subframe. The
 * interface anomaly of specify a child of the desired destination rather
 * than the destination is because all the callers are WIN_OWNERs, not
 * necessarily WIN_PARENTs, of the managed dest_child. If dest_child is NULL,
 * the root window of the source is used as the dest.
 */
Xv_private int
window_getrelrect(dest_child, source, source_rect)
    Xv_object       dest_child, source;
    register Rect  *source_rect;
{
    Xv_object       dest;
    register Xv_Drawable_info *dest_info;
    register Xv_Drawable_info *source_info;

    if ((Xv_object) xv_get(dest_child, WIN_PARENT) !=
	(Xv_object) xv_get(dest_child, WIN_PARENT))
	goto Error_Return;

    DRAWABLE_INFO_MACRO(source, source_info);
    dest = dest_child ?
	         ((Xv_object) xv_get(dest_child, WIN_PARENT)) :
	         xv_root(source_info);
    DRAWABLE_INFO_MACRO(dest, dest_info);
    if (xv_display(source_info) != xv_display(dest_info)) {
	/* Windows are on different X servers => caller error */
	goto Error_Return;
    }
    (void) win_get_outer_rect(source, source_rect);
    return XV_OK;

Error_Return:
    *source_rect = rect_null;
    return XV_ERROR;
}


/* convert number of rows to pixels */
Xv_public int
xv_rows(win_public, rows)
    Xv_Window       win_public;
    int             rows;
{
    register Window_info *win = WIN_PRIVATE(win_public);

    return rows * (actual_row_height(win) + win->row_gap);
}


/* convert row position to pixels */
Xv_public int
xv_row(win_public, row)
    Xv_Window       win_public;
    int             row;
{
    Window_info    *win = WIN_PRIVATE(win_public);

    return win->top_margin + xv_rows(win_public, row);
}


/* convert number of columns to pixels */
Xv_public int
xv_cols(win_public, cols)
    Xv_Window       win_public;
    int             cols;
{
    register Window_info *win = WIN_PRIVATE(win_public);

    return cols * (actual_column_width(win) + win->column_gap);
}

/* convert col position to pixels */
Xv_public int
xv_col(win_public, col)
    Xv_Window       win_public;
    int             col;
{
    Window_info    *win = WIN_PRIVATE(win_public);

    return win->left_margin + xv_cols(win_public, col);
}

Xv_private void
window_get_outer_rect(window, rect)
    Xv_Window       window;
    Rect           *rect;
{
    Window_info    *win = WIN_PRIVATE(window);
    *rect = win->cache_rect;
    if (win->has_border)
	rect_borderadjust(rect, WIN_DEFAULT_BORDER_WIDTH);
}

Xv_private void
window_set_outer_rect(window, rect)
    Xv_Window       window;
    Rect           *rect;
{
    Window_info    *win = WIN_PRIVATE(window);
    int             old_rect_info, rect_info = ( WIN_X_SET | WIN_Y_SET | WIN_WIDTH_SET | WIN_HEIGHT_SET);
    if (win->has_border)
	rect_borderadjust(rect, -(WIN_DEFAULT_BORDER_WIDTH));
	
    old_rect_info = win->rect_info;
    win->rect_info = rect_info;
    window_set_cache_rect(window, rect);
    win->rect_info = old_rect_info;
}


Xv_private void
window_set_client_message(window, msg)
    Xv_Window       window;
    XClientMessageEvent *msg;
{
    Window_info    *win = WIN_PRIVATE(window);

    win->client_message.type = (Xv_opaque) msg->message_type;
    win->client_message.format = (unsigned char) msg->format;
    XV_BCOPY (msg->data.b, win->client_message.data.b, WIN_MESSAGE_DATA_SIZE);
}

Xv_private void
window_get_cache_rect(window, rect)
    Xv_Window       window;
    Rect           *rect;
{
    *rect = WIN_PRIVATE(window)->cache_rect;
}

/* this updates the inner rect */

Xv_private void
window_update_cache_rect(window, rect)
    Xv_Window       window;
    Rect           *rect;
{
    WIN_PRIVATE(window)->cache_rect = *rect;
}

/*
 * this sets the inner rect and translates the coordinates to what X expects
 * before calling XConfigureWindow
 */

Xv_private void
window_set_cache_rect(window, rect)
    Xv_Window       window;
    Rect           *rect;
{
    Window_info    *win;
    Xv_Drawable_info *info;
    long unsigned   change_mask = 0;
    XWindowChanges  changes;

    win = WIN_PRIVATE(window);
    
    if (!win->rect_info)
        return;

    /*
     * X does not allow 0 width or height so arbitrarily set it to 1 (yeah!)
     */
    if (!rect->r_height || !rect->r_width) {
	xv_error(window,
		 ERROR_STRING, 
		     XV_MSG("window: zero width or height specified"),
		 ERROR_PKG, WINDOW,
		 NULL);
	if (!rect->r_height)
	    rect->r_height = 1;
	if (!rect->r_width)
	    rect->r_width = 1;
    }
    /* If the rect's are equal, no need to resize any of the subwindows. */
    if (rect_equal(&(win->cache_rect), rect))
	return;

    
    if ((win->rect_info & (int) WIN_X_SET)  &&
        (rect->r_left != EMPTY_VALUE)) {
	if (WIN_DEFAULT_RECT(&win->cache_rect)) {
	    change_mask |= CWX;
            win->cache_rect.r_left = rect->r_left;
	} else if (win->cache_rect.r_left != rect->r_left) {	
            change_mask |= CWX;
            win->cache_rect.r_left = rect->r_left;
	}
    }    
    if ((win->rect_info & (int) WIN_Y_SET)  &&
        (rect->r_top != EMPTY_VALUE)) {
	if (WIN_DEFAULT_RECT(&win->cache_rect)) {
	    change_mask |= CWY;
            win->cache_rect.r_top = rect->r_top;
	} else if (win->cache_rect.r_top != rect->r_top) {	
            change_mask |= CWY;
            win->cache_rect.r_top = rect->r_top;
	}
    }    

    if ((win->rect_info & (int) WIN_WIDTH_SET)  &&
        (rect->r_width != EMPTY_VALUE)) {
	if (WIN_DEFAULT_RECT(&win->cache_rect)) {
	    change_mask |= CWWidth;
            win->cache_rect.r_width = rect->r_width;
	} else if (win->cache_rect.r_width != rect->r_width) {	
            change_mask |= CWWidth;
            win->cache_rect.r_width = rect->r_width;
	}
    }    

    if ((win->rect_info & (int) WIN_HEIGHT_SET)  &&
        (rect->r_height != EMPTY_VALUE)) {
	if (WIN_DEFAULT_RECT(&win->cache_rect)) {
	    change_mask |= CWHeight;
            win->cache_rect.r_height = rect->r_height;
	} else if (win->cache_rect.r_height != rect->r_height) {	
            change_mask |= CWHeight;
            win->cache_rect.r_height = rect->r_height;
	}
    }    

    
    DRAWABLE_INFO_MACRO(window, info);
    
    changes.x = win->cache_rect.r_left;
    changes.y = win->cache_rect.r_top;
    changes.width = win->cache_rect.r_width;
    changes.height = win->cache_rect.r_height;
    
    XConfigureWindow(xv_display(info), xv_xid(info), change_mask,
		     &changes);
		     
    if ((change_mask & (CWWidth  | CWHeight | CWY | CWX)) &&
					(!win->top_level && win->created)) {
	    Event event;
	    XConfigureEvent cevent;

	    /* We are going to fake an XEvent here to keep people who are
	       monitoring them happy.  I really think the right way to do
	       this is use XSendEvent and handle it in xevent_to_event, but
	       that's too risky to go in now and may require some major
	       reworks.      				                     */

	    cevent.type = ConfigureNotify;
	    cevent.serial = 0L;
	    cevent.send_event = FALSE;
	    cevent.display = xv_display(info);
	    cevent.event = xv_xid(info);
	    cevent.window = xv_xid(info);
	    cevent.x = win->cache_rect.r_left;
	    cevent.y = win->cache_rect.r_top;
	    cevent.width = win->cache_rect.r_width;
	    cevent.height = win->cache_rect.r_height;

	    event_init(&event);
	    event_set_id(&event, WIN_RESIZE);
	    event_set_window(&event, window);
	    event_xevent(&event) = (XEvent *) &cevent;

	    if (win->being_rescaled) {
	    	event_set_id(&event, ACTION_RESCALE);
		win_post_event_arg(window, &event, NOTIFY_IMMEDIATE,
				       win->scale, NOTIFY_COPY_NULL,
				       NOTIFY_RELEASE_NULL);
	    } else
		win_post_event(window, &event, NOTIFY_IMMEDIATE);
	}
}

Xv_private int
window_set_parent(window, parent)
    Xv_Window       window, parent;
{
    Window_info    *win_private = WIN_PRIVATE(window);
    Xv_Drawable_info *info;
    Inputmask       im;

    if (win_private->parent == parent)
	return 0;
    win_private->parent = parent;
    DRAWABLE_INFO_MACRO(window, info);
    if (parent == xv_get(xv_screen(info), XV_ROOT))
	win_private->top_level = TRUE;
    else
	win_private->top_level = FALSE;
    win_xmask_to_im(win_private->xmask, &im);
    xv_set(window, WIN_INPUT_MASK, &im, NULL);
}

Xv_private void
window_set_map_cache(window, flag)
    Xv_Window       window;
    int             flag;
{
    WIN_PRIVATE(window)->map = flag;
}

Xv_private int
window_get_map_cache(window)
    Xv_Window       window;
{
    return(WIN_PRIVATE(window)->map);
}

Xv_private void
window_set_border(window, width)
    Xv_object       window;
    int             width;
{
    Xv_Drawable_info *info;

    DRAWABLE_INFO_MACRO(window, info);
    XSetWindowBorderWidth(xv_display(info), xv_xid(info), width);
}


/*
 * Decrease the size of the rect given the information in the win struct
 */
Xv_private void
window_outer_to_innerrect(Xv_Window window, Rect *rect)
{
    Window_info    *win = WIN_PRIVATE(window);

    if (win->has_border)
	rect_borderadjust(rect, WIN_DEFAULT_BORDER_WIDTH);

}

Xv_private void
window_inner_to_outerrect(window, rect)
    Xv_Window       window;
    Rect           *rect;
{
    Window_info    *win = WIN_PRIVATE(window);

    if (win->has_border)
	rect_borderadjust(rect, -(WIN_DEFAULT_BORDER_WIDTH));
}

/*
 * set the scale for this window
 */
Xv_private void
window_set_rescale_state(window, state)
    Xv_Window       window;
    int             state;
{

    Window_info    *win = WIN_PRIVATE(window);
    win->scale = state;

}

/*
 * get the scale for this window
 */

Xv_private int
window_get_rescale_state(window)
    Xv_Window       window;
{

    Window_info    *win = WIN_PRIVATE(window);
    return win->scale;
}

Xv_private void
window_end_rescaling(window, state)
    Xv_Window       window;
{
    Window_info    *win = WIN_PRIVATE(window);
    win->being_rescaled = FALSE;
}

Xv_private void
window_start_rescaling(window, state)
    Xv_Window       window;
{
    Window_info    *win = WIN_PRIVATE(window);
    win->being_rescaled = TRUE;
}

/*
 * calculate the new width and height of the window based on new scaling
 * info. When we enter this function the scale and font have already been set
 * by a call to window_default_event_func somewhere. This method will give
 * nasty results if parent and child have different row_gap, top_margin and
 * bottom_margins Setting the row height invalidates rescaling. Should
 * rescale the row_gap row_height etc.
 */

Xv_private void
window_calculate_new_size(parent, window, state, new_width, new_height)
    Xv_Window       parent, window;
    int            *new_width, *new_height;
{
    Window_info    *win = WIN_PRIVATE(window);
    Window_info    *par = WIN_PRIVATE(parent);

    /*
     * no need to do this go from pixelsold to pixels new must use all parent
     * vars.
     */

    int             rows = (int) xv_get(window, WIN_ROWS, NULL);
    int             cols = (int) xv_get(window, WIN_COLUMNS, NULL);

    /*
     * vp = old_rect.r_height; vp -= win->top_margin + win->bottom_margin;
     * rows = vp / (actual_row_height(win) + win->row_gap)); vp =
     * old_rect.r_width; vp -= win->left_margin + win->right_margin; cols =
     * (vp / (actual_column_width(win) + win->column_gap));
     */

    /* now calculate new size from new font */

    *new_height = rows * (actual_rescale_row_height(par, win)
		     + win->row_gap) + win->top_margin + win->bottom_margin;
    *new_width = cols * (actual_rescale_column_width(par, win)
		  + win->column_gap) + win->left_margin + win->right_margin;
}


Xv_private void
window_destroy_rect_obj_list(rect_obj_list)
    Window_rescale_rect_obj *rect_obj_list;
{
    free(rect_obj_list);

}

Xv_private      Window_rescale_rect_obj *
window_create_rect_obj_list(num_elems)
    int             num_elems;
{
    Window_rescale_rect_obj *rect_object;
    rect_object = (Window_rescale_rect_obj *) xv_calloc( /* alloc space */
						     (unsigned) num_elems,
				(unsigned) sizeof(Window_rescale_rect_obj));

    return rect_object;
}

Xv_private void
window_add_to_rect_list(rect_obj_list, window, rect, i)
    Window_rescale_rect_obj *rect_obj_list;
    Xv_Window       window;
    Rect           *rect;
    int             i;
{
    rect_obj_list[i].sw = window;
    rect_obj_list[i].old_rect = *rect;

}

Xv_private int
window_rect_equal_ith_obj(rect_obj_list, rect, i)
    Window_rescale_rect_obj *rect_obj_list;
    int             i;
    Rect           *rect;
{
    if (!rect_equal(&(rect_obj_list[i].old_rect), &(rect_obj_list[i].new_rect))) {
	*rect = rect_obj_list[i].new_rect;
	return TRUE;
    } else {
	rect = RECT_NULL;
	return FALSE;
    }

}

Xv_private void
window_adjust_rects(rect_obj_list, parent_public, num_elems, parent_width, parent_height)
    Window_rescale_rect_obj *rect_obj_list;
    Xv_Window       parent_public;
    int             num_elems, parent_width, parent_height;
{
    int             this_sw;
    int             new_width, new_height;
    int             i;

    for (i = 0; i < num_elems; i++) {
	window_calculate_new_size(parent_public, &(rect_obj_list[i].sw), &new_width, &new_height);
	/* add it to the local version */
	rect_obj_list[i].new_rect.r_width = new_width;
	rect_obj_list[i].new_rect.r_height = new_height;

	/*
	 * add borders to sw geometry
	 */
	{
	    int             diff;
	    if (diff = rect_obj_list[i].new_rect.r_width
		- rect_obj_list[i].old_rect.r_width) {
		rect_obj_list[i].width_change = diff;
	    }
	    if (diff = rect_obj_list[i].new_rect.r_height
		- rect_obj_list[i].old_rect.r_height) {
		rect_obj_list[i].height_change = diff;
	    }
	}
	window_inner_to_outerrect((Xv_Window) rect_obj_list[i].sw, &(rect_obj_list[i].new_rect));

	/* init other params */
	rect_obj_list[i].width_change = 0;
	rect_obj_list[i].height_change = 0;
	rect_obj_list[i].y_change = 0;
	rect_obj_list[i].x_change = 0;

    }

    /*
     * THE MEAT: Each subwindow is adjusted and its position is calculated
     * from the position of the windows above and to the left of it if a
     * window above or to the left has not been adjusted then recursively
     * adjust it.
     */

    for (this_sw = 0; this_sw < num_elems; this_sw++) {
	/*
	 * this_sw refers to the subwindow whose rect we are trying to adjust
	 */

	adjust_rect_obj(num_elems, this_sw, rect_obj_list, parent_width, parent_height);
	window_outer_to_innerrect((Xv_Window) &(rect_obj_list[this_sw]),
				  &rect_obj_list[this_sw].new_rect);
    }
}

static void
adjust_rect_obj(num_elems, this_sw, rect_obj_list, parent_width, parent_height)
    Window_rescale_rect_obj *rect_obj_list;
    int             parent_width, parent_height, this_sw, num_elems;
{
    int             sw_left_of = 0, left_coord, sw_above = 0, top_coord;
    Window_rescale_rect_obj *sw_to_adjust, *temp_sw;
    int             avw, avh, another_sw;
    int             nothing_right_of = TRUE, nothing_below = TRUE;

    sw_to_adjust = &(rect_obj_list[this_sw]);
    if (sw_to_adjust->adjusted)
	return;
    /*
     * to position this window all windows before it must have been
     * positioned
     */

    for (another_sw = 0; another_sw < num_elems; another_sw++) {
	if (this_sw != another_sw) {
	    temp_sw = &(rect_obj_list[another_sw]);
	    if (rect_right_of(&(temp_sw->old_rect), &(sw_to_adjust->old_rect))) {
		if (left_coord < temp_sw->old_rect.r_left) {
		    /*
		     * adjust adjacent rect first
		     */
		    adjust_rect_obj(num_elems, another_sw, rect_obj_list, parent_width, parent_height);
		    left_coord = temp_sw->old_rect.r_left;
		    sw_left_of = another_sw;
		}
	    } else if (rect_right_of(&(sw_to_adjust->old_rect), &(temp_sw->old_rect)))
		nothing_right_of = FALSE;

	    if (rect_below(&(temp_sw->old_rect),
			   &(sw_to_adjust->old_rect))) {
		if (top_coord < temp_sw->old_rect.r_top) {
		    /*
		     * has this rect been modified? if not recursively modify
		     * it
		     */
		    adjust_rect_obj(num_elems, another_sw, rect_obj_list, parent_width, parent_height);
		    top_coord = temp_sw->old_rect.r_top;
		    sw_above = another_sw;
		}
	    } else if (rect_below(&(sw_to_adjust->old_rect), &(temp_sw->old_rect)))
		nothing_below = FALSE;

	    /*
	     * if there is a window above this_sw that is below all other
	     * windows above it
	     */
	    if (sw_above) {
		sw_to_adjust->new_rect.r_top +=
		    rect_obj_list[sw_above].y_change + rect_obj_list[sw_above].height_change;
		sw_to_adjust->y_change +=
		    rect_obj_list[sw_above].y_change + rect_obj_list[sw_above].height_change;
	    }
	    /* if there is a window that is right most to the left of this_sw */
	    if (sw_left_of) {
		sw_to_adjust->new_rect.r_left +=
		    rect_obj_list[sw_left_of].x_change + rect_obj_list[sw_left_of].width_change;
		sw_to_adjust->x_change +=
		    rect_obj_list[sw_left_of].x_change + rect_obj_list[sw_left_of].width_change;
	    }
	    if (nothing_right_of) {
		avw = parent_width -
		    (sw_to_adjust->new_rect.r_left +
		     sw_to_adjust->new_rect.r_width);

		if (avw) {	/* give it whats left */
		    sw_to_adjust->new_rect.r_width += avw;
		    avw = parent_width;
		}
	    }
	    if (nothing_below) {
		avh = parent_width -
		    (sw_to_adjust->new_rect.r_top +
		     sw_to_adjust->new_rect.r_height);

		if (avh) {	/* give it whats left */
		    sw_to_adjust->new_rect.r_height += avh;
		    avh = parent_width;
		}
	    }
	}			/* if (this_sw != another_sw) */
    }				/* end for */
    sw_to_adjust->adjusted = TRUE;
}


/* Get data from a property hanging off RootWindow */
Xv_private int
xv_get_external_data(window, key, data, len, format)
    Xv_object       window;
    char           *key;
    Xv_opaque      *data;
    int            *len;	/* Number of bytes in the external data */
    int            *format;
{
    Display        *display = XV_DISPLAY_FROM_WINDOW(window);
    Atom            actual_type, prop_name;
    unsigned long   nitems, bytes_after;
    unsigned long  *prop;

    if ((prop_name = XInternAtom(display, key, TRUE)) !=
	None) {
	if (XGetWindowProperty(display, DefaultRootWindow(display),
		      prop_name, 0, 1, FALSE, AnyPropertyType, &actual_type,
			       format, &nitems, &bytes_after,
			       (unsigned char **) &prop) == Success) {
	    *len = nitems * (*format) / 8;
	    XV_BCOPY(prop, (char *) data, *len);
	    return (*len);
	}
    }
    return (0);
}


/*
 * Write data in a property hanging off RootWindow. If the property doesn't
 * exist, create one
 */

Xv_private int
xv_write_external_data(window, key, format, data, len)
    Xv_object       window;
    char           *key;
    int             format;
    Xv_opaque      *data;
    int             len;	/* lenght of data in the format */
{
    Display        *display = XV_DISPLAY_FROM_WINDOW(window);
    Atom            atom;

    if ((atom = XInternAtom(display, key, FALSE)) != None) {
	if ((XChangeProperty(display, DefaultRootWindow(display),
			     atom, XA_INTEGER, format, PropModeReplace,
			     (unsigned char *) data, len)) == Success)
	    return (XV_OK);
    }
    return (XV_ERROR);
}


/*
 * Send message to addresse
 */
Xv_public int
xv_send_message(window, addresse, msg_type, format, data, len)
    Xv_object       window;
    Xv_opaque       addresse;
    char           *msg_type;
    int             format;	/* 8, 16, or 32 */
    Xv_opaque      *data;
    int             len;	/* Number of bytes in data */
{
    Display        *display = XV_DISPLAY_FROM_WINDOW(window);
    Atom            registration = XInternAtom(display, msg_type, FALSE);
    XClientMessageEvent client_event;

    client_event.type = ClientMessage;
    client_event.message_type = registration;
    client_event.format = format;
    XV_BCOPY((char *) data, client_event.data.b, len);
    client_event.window = (((int) addresse == PointerWindow) ||
			   ((int) addresse == InputFocus)) ?
	(Window) XV_DUMMY_WINDOW : (Window) addresse;
    XSendEvent(display, addresse, False, NoEventMask, (XEvent *)&client_event);
    XFlush(display);
    return (XV_OK);
}


/* Grab the keys that can be used to start a "quick" selection, namely
 * "Paste" and "Cut".
 */
Xv_private void
win_grab_quick_sel_keys(window)
    Xv_Window	    window; /* window that wants to grab quick selection keys */
{
    Xv_Drawable_info *info;
    int		    keycode;
    KeySym	    keysym;

    DRAWABLE_INFO_MACRO(window, info);

    /* Grab Cut key */
    keycode = XKeysymToKeycode(xv_display(info),
		 	       xv_get(xv_server(info), SERVER_CUT_KEYSYM));
    if (keycode)
	XGrabKey(xv_display(info), keycode, 0, xv_xid(info), False,
		 GrabModeAsync, GrabModeAsync);

    /* Grab Paste key */
    keycode = XKeysymToKeycode(xv_display(info),
		 	       xv_get(xv_server(info), SERVER_PASTE_KEYSYM));
    if (keycode)
	XGrabKey(xv_display(info), keycode, 0, xv_xid(info), False,
		 GrabModeAsync, GrabModeAsync);
#ifdef OW_I18N
    {
	/* 
	 * Should actually create a table of all the keys which have
	 * a passive grab. But for now let's just set a flag, and 
	 * assume we know all cases.
	 */
	Window_info 	*win;

	win = WIN_PRIVATE(window);
	WIN_SET_PASSIVE_GRAB(win, TRUE);
    }
#endif
}


/* Ungrab the keys that can be used to start a "quick" selection, namely
 * "Paste" and "Cut".
 */
Xv_private void
win_ungrab_quick_sel_keys(window)
    Xv_Window	    window;
{
    Xv_Drawable_info *info;
    int		    keycode;
    KeySym	    keysym;

    DRAWABLE_INFO_MACRO(window, info);

    /* ungrab Cut key */
    keycode = XKeysymToKeycode(xv_display(info),
		 	       xv_get(xv_server(info), SERVER_CUT_KEYSYM));

    if (keycode)
	XUngrabKey(xv_display(info), keycode, 0, xv_xid(info));


    /* ungrab Paste key */
    keycode = XKeysymToKeycode(xv_display(info),
		 	       xv_get(xv_server(info), SERVER_PASTE_KEYSYM));
    if (keycode)
	XUngrabKey(xv_display(info), keycode, 0, xv_xid(info));

#ifdef OW_I18N
    {
	/* 
	 * Should actually create a table of all the keys which have
	 * a passive grab. But for now let's just set a flag, and 
	 * assume we know all cases.
	 */
	Window_info 	*win;

	win = WIN_PRIVATE(window);
	WIN_SET_PASSIVE_GRAB(win, FALSE);
    }
#endif
}




Xv_private void
win_set_wm_command(window)
Xv_window	window;
{
    Xv_Drawable_info *info;
    char *appl_cmdline;
    char **appl_cmdline_argv = (char **)NULL;
    char appl_cmdline_argc = 0;
    int		reset = FALSE;

    DRAWABLE_INFO_MACRO(window, info);
    appl_cmdline = (char *)xv_get(window, WIN_CMD_LINE);

    if (xv_get(window, XV_IS_SUBTYPE_OF, FRAME_BASE))  {
        appl_cmdline_argv = (char **)xv_get(window, 
		FRAME_WM_COMMAND_ARGV);
        appl_cmdline_argc = (int)xv_get(window, 
		FRAME_WM_COMMAND_ARGC);
    }
    else  {
	if (!appl_cmdline)  {
            appl_cmdline_argv = (char **)-1;
	}
    }

    if (!xv_app_name || 
	(xv_get(window, XV_OWNER) != xv_get(xv_screen(info), XV_ROOT)))  {
	reset = TRUE;
    }
    else  {
        if (!appl_cmdline_argv && appl_cmdline)  {
	    /*
	     * WIN_CMDLINE set AND FRAME_CMD_LINE_* not used
	     * Use old non-ICCCM compliant way of setting WM_COMMAND
	     */
            if ((int)appl_cmdline != -1) {
                int   len = 1000;
                char *str = NULL;
        
                if (appl_cmdline)
                    len += strlen(appl_cmdline);

                str = xv_malloc(len);
                win_get_cmdline_option(window, str, appl_cmdline);
                win_change_property(window, SERVER_WM_COMMAND, XA_STRING, 8, 
			        str, strlen(str) + 1);
                xv_free(str);
            } 
	    else  {
	        reset = TRUE;
	    }
        }
        else  {
	    /*
	     * WIN_CMDLINE not set AND FRAME_CMD_LINE_* used
	     * Use new ICCCM compliant way of setting WM_COMMAND
	     */
	    if ((int)appl_cmdline_argv != -1)  {
		char	*argv[200];
                win_set_wm_command_prop(window, argv, appl_cmdline_argv,
						appl_cmdline_argc);
	    }
	    else  {
	        reset = TRUE;
	    }
        }
    }

    if (reset)  {
        /*
         * If we can't save the state, perform a zero
         * length append on WM_COMMAND.
         */
        win_change_property(window, SERVER_WM_COMMAND, XA_STRING, 8, NULL, NULL);
    }
}

/*
 * Sets the WM_CLASS property on window
 */
Xv_private void
win_set_wm_class(window)
Xv_window	window;
{
    Xv_Drawable_info	*info;
    XClassHint		class_hints;
    int			len;
    int			i;
    char		*class_name;

    DRAWABLE_INFO_MACRO(window, info);

    /*
     * Get instance name of application
     */
    class_hints.res_name = xv_instance_app_name;

    /*
     * Get argv[0]
     */
    class_name = strdup(xv_app_name);
    len = strlen(class_name);

    /*
     * Make the first lower case letter upper case
     */
    for (i=0; i < len; ++i)  {
        if (islower(xv_app_name[i]))  {
            class_name[i] = toupper(xv_app_name[i]);
	    break;
	}
    }

    /*
     * Set class to argv[0] with first low case letter upper cased
     */
    class_hints.res_class = class_name;

    /*
     * Set WM_CLASS property on window
     */
    XSetClassHint(xv_display(info), xv_xid(info), &class_hints);

    free(class_name);
}

/*
 * window_set_tree_flag(topLevel, pointer, deafBit, on)
 * Sets the flag bit in the private data of the window 'topLevel' 
 * and all it's subwindows to 'flag'. flag is either deaf or window_loop. 
 * This is controlled by 'deafBit'. If other flag's need to be manipulated 
 * by this function, modify the macros WIN_IS_FLAG and WIN_CHECK_FLAG 
 * and how 'deafBit' is interpreted in this function.
 *
 * If 'pointer' is not NULL, all windows in the tree will use it as the new pointer
 * if 'flag' is TRUE. Upon calling this function with 'flag' = FALSE, the old pointer
 * will be restored to all the windows in the tree.
 */
Xv_private int
window_set_tree_flag(topLevel, pointer, deafBit, flag)
Xv_window	topLevel;
Xv_cursor	pointer;
int		deafBit;
Bool		flag;
{
    Window_info			*win;

    if (!topLevel)  {
	return(XV_OK);
    }

    win = WIN_PRIVATE(topLevel);

    /*
     * Return if the window is already in the state we want to set 
     * it to
     */
    if (WIN_IS_FLAG(deafBit, win) == flag)  {
        return(XV_OK);
    }

    /*
     * Otherwise, set bit in window private data
     */
    WIN_SET_FLAG(deafBit, win, flag);

    /*
     * If cursor provided, make window use it
     */
    if (pointer)  {
        (void)window_set_flag_cursor(topLevel, pointer, flag);
    }

    /*
     * Do the same for ALL children/descendants.
     */
    if (window_set_tree_child_flag(topLevel, pointer, deafBit, flag) != XV_OK)  {
	/*
	 * If unsuccessful, return error code
	 */
        return(XV_ERROR);
    }

    return(XV_OK);
}

/*
 * window_set_tree_child_flag(query, pointer, deafBit, flag)
 * Traverses the window tree rooted at 'query', setting the flag bit
 * in the private data of all windows in the tree to 'flag'. flag is either 
 * deaf or window_loop. This is controlled by 'deafBit'. If other flag's 
 * need to be manipulated by this function, modify the macros WIN_IS_FLAG 
 * and WIN_CHECK_FLAG (and how 'deafBit) is interpreted in this function.
 *
 * If 'pointer' is not NULL, all windows in the tree will use it as the new pointer
 * if 'flag' is TRUE. Upon calling this function with 'flag' = FALSE, the old pointer
 * will be restored to all the windows in the tree.
 */
Xv_private int
window_set_tree_child_flag(query, pointer, deafBit, flag)
Xv_window	query;
Xv_cursor	pointer;
int		deafBit;
Bool		flag;
{
    Display			*display;
    Xv_Drawable_info		*info;
    Window			queryXid,
				root, 
				parent, 
				*children;
    unsigned int		numchildren = 0;
    int				return_code = XV_OK;
    int				i;

    if (!query)  {
	return(XV_OK);
    }

    DRAWABLE_INFO_MACRO(query, info);
    display = xv_display(info);
    queryXid = xv_xid(info);

    /*
     * Query for all children of parent window
     */
    if (!(XQueryTree(display, queryXid, &root, &parent, &children, &numchildren)))  {
        xv_error(query,
            ERROR_STRING, 
		XV_MSG("Attempt to query the window tree failed"), 
            NULL);

	return(XV_ERROR);
    }

    if (numchildren == 0)  {
	return(XV_OK);
    }

    /*
     * For each child:
     */
    for (i=0; i<numchildren; ++i)  {
	Xv_window	win_obj;

	if (win_obj = win_data(display, children[i]))  {
	    Window_info		*win_private = WIN_PRIVATE(win_obj);
	    
            /*
             * Do only if the window is not already in the state we want to set 
             * it to
             */
            if (WIN_IS_FLAG(deafBit, win_private) != flag)  {
		/*
		 * Set bit in window private data
		 */
	        WIN_SET_FLAG(deafBit, win_private, flag);

		/*
		 * Set cursor to busy cursor
		 */
		if (pointer)  {
                    (void)window_set_flag_cursor(win_obj, pointer, flag);
		}

		/*
		 * make children of this window deaf also
		 */
                if (window_set_tree_child_flag(win_obj, pointer, deafBit, flag) != XV_OK)  {
	            return_code = XV_ERROR;
	        }
            }
	}

    }

    XFree((char *)children);

    return(return_code);
}

/*
 * window_set_flag_cursor(window, pointer, flag)
 * Makes 'window' use 'pointer' as a cursor if 'flag' is TRUE
 */
Xv_private int
window_set_flag_cursor(window, pointer, flag)
Xv_window	window;
Xv_cursor	pointer;
Bool		flag;
{
    Window_info		*win_private;

    if (!window)  {
        return(XV_OK);
    }

    win_private = WIN_PRIVATE(window);

    if (flag)  {
	if (pointer)  {
            win_private->normal_cursor = win_private->cursor;
            xv_set(window, WIN_CURSOR, pointer, NULL);
	}
    }
    else  {
	if (win_private->normal_cursor)  {
            xv_set(window, WIN_CURSOR, win_private->normal_cursor, NULL);
	}
    }

    return(XV_OK);
}

#ifdef OW_I18N
/*
 * Current implementation of XUnsetICFocus() does not disable the IC
 * completely, so we need to set the focus window temporarily to a 
 * dummy window.
 *
 * Whenever setting XNFocusWindow make sure to cache the real focus
 * window, so that we can reset it properly upon calling XSetICFocus()
 */
Xv_private int
window_set_ic_focus_win(window, ic, focus_win)
    Xv_window	window;
    XIC		ic;
    XID		focus_win;
{
    Window_info		*win;
    XID                 current_focus;

    if (!window)  {
        return(XV_OK);
    }

    win = WIN_PRIVATE(window);

    win->ic_focus_win = focus_win;

    /*
     * Query current focus window to see if it really needs to be set.
     * This is the safe way of checking, but for better performance we
     * could actually trigger off of the win->ic_active value:
     *
     * 	WIN_IC_ACTIVE TRUE  => XNFocusWindow set to win->ic_focus_win.
     *  WIN_IC_ACTIVE FALSE => XNFocusWindow set to win->tmp_ic_focus_win
     *
     * This has been added to help alleviate the tool hang problem (1110677).
     * This will reduce number of times XNFocusWindow is being set
     * in panel (1111354) and in ttysw (1111352).
     */
    XGetICValues(ic, XNFocusWindow, &current_focus, NULL);

    if (win->ic_active) {
	if (focus_win && (focus_win != current_focus)) 
            XSetICValues(ic, XNFocusWindow, focus_win, NULL);
    } else  {
	if (win->tmp_ic_focus_win && (win->tmp_ic_focus_win != current_focus))
            XSetICValues(ic, XNFocusWindow, win->tmp_ic_focus_win, NULL);
    }

    return(XV_OK);
}

Xv_private int
window_set_xungrabkeyboard(window, display, time)
    Xv_window   window;
    Display     *display;
    Time        time;
{
    Window_info         *win;
 
    if (!window)  {
        return(XV_OK);
    }
 
    win = WIN_PRIVATE(window);
 	
    XUngrabKeyboard(display, time);
    WIN_SET_GRAB(win, FALSE);
 
}

Xv_private int   
window_set_xgrabkeyboard(window, dpy, grab_window, owner_events,
                     ptr_mode, kbd_mode, time)
    Xv_window   window;
    Display     *dpy; 
    Bool        owner_events;
    int         ptr_mode;   
    int         kbd_mode;
    Time        time;
{
    Window_info         *win;
    int                 grab_status;
 
    if (!window)  {
        return(XV_OK);
    }
 
    win = WIN_PRIVATE(window);
 
    grab_status =  XGrabKeyboard(dpy, grab_window, owner_events,
ptr_mode,
                                 kbd_mode, time);
    if (grab_status == GrabSuccess)
        WIN_SET_GRAB(win, TRUE);
 
    return(grab_status);
 
}
#endif /* OW_I18N */
