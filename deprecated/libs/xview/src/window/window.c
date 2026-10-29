#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)window.c 20.156 93/06/28";
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

#ifdef SVR4
#include <sys/types.h>
#endif /* SVR4 */

#include <stdio.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <signal.h>

#include <xview_private/i18n_impl.h>

#include <xview/notify.h>
#include <xview_private/windowimpl.h>
#include <xview/server.h>
#include <xview/screen.h>
#include <xview/window.h>
#include <xview/cms.h>
#include <xview_private/draw_impl.h>
#include <xview_private/win_keymap.h>
#include <xview_private/win_info.h>
#ifdef OW_I18N
#include <xview/frame.h>
#endif /* OW_I18N */

/*
 * Private
 */

static int      parent_dying = 0;	/* Don't destroy this window if */
/* its parent window is dying.  */
/* Server will do this for us.  */
Pkg_private int window_init();
Pkg_private int window_destroy_win_struct();
Pkg_private XID window_new();
Pkg_private void window_get_grab_flag();

#define eexit(msg, error_msg) \
  if (error_msg) { \
      char dummy[128]; \
      (void) sprintf(dummy, "%s\n%s", msg, error_msg); \
      xv_error(XV_ZERO, \
          ERROR_SEVERITY, ERROR_NON_RECOVERABLE, \
          ERROR_STRING, dummy, ERROR_PKG, WINDOW, NULL); \
  } else { \
      xv_error(XV_ZERO, \
          ERROR_STRING, msg, ERROR_PKG, WINDOW, NULL); \
      return XV_ERROR; \
  }

#define WIN_NOT_DEFINED -1

/*
 * Initialize win as a window.  Return XV_ERROR if error and no WIN_ERROR_MSG
 * is specified. Caller is required to pass a screen as the parent_public
 * when creating a root window.
 */
Pkg_private int
window_init(parent_public, win_public, avlist)
    Xv_Window       parent_public, win_public;
    Attr_avlist     avlist;
{
    Window_info    		*parent;
    Xv_window_struct 		*win_object = (Xv_window_struct *) win_public;
    register Window_info 	*win;
    register Attr_avlist	attrs;
    char           		*error_msg = NULL;
    unsigned char		is_root = FALSE, is_sel_window = FALSE;
    unsigned char             	input_only = FALSE, transparent = FALSE;
    unsigned char 		is_client_pane = FALSE;
    int		 		border = FALSE, no_decor = FALSE,
				convert_cu = FALSE, 
                                inherit_colors = WIN_NOT_DEFINED;
    Display        		*display;
    Xv_opaque       		screen, server;
    Screen_visual		*default_visual, *visual;
    XVisualInfo			vinfo_template;
    long			vinfo_mask = 0;
    char			cms_name[100];
    Colormap			cmap_id;
    Xv_Cursor       		default_cursor;
    register Xv_Drawable_info 	*info;
    register Xv_Drawable_info 	*parent_info = NULL;
#ifdef OW_I18N
    int				is_wcs_error_msg = FALSE;	
    int                         win_use_im = TRUE;
    int				win_ic_active = TRUE;
    Xv_private void    		_xv_status_start(), _xv_status_done(), _xv_status_draw();
#ifdef FULL_R5
    XIMStyle			win_im_style_mask = NULL;
#endif 

    /* Inherit WIN_USE_IM from parent */
    if (parent_public && 
	((Xv_pkg *)xv_get(parent_public, XV_IS_SUBTYPE_OF, WINDOW)))  {
        win_use_im = xv_get(parent_public, WIN_USE_IM);
    }
#endif /* OW_I18N */

    /*
     * Initialize flags that control whether passive grabs are done
     */
    window_get_grab_flag();

    win = xv_alloc(Window_info);
    rect_construct(&win->cache_rect, EMPTY_VALUE, EMPTY_VALUE,
				     EMPTY_VALUE, EMPTY_VALUE);

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch ((int) attrs[0]) {
	  case WIN_IS_ROOT:	/* create a root window */
	    is_root = TRUE;
	    ATTR_CONSUME(attrs[0]);
	    break;

	  case WIN_SELECTION_WINDOW:
	    is_sel_window = TRUE;
	    ATTR_CONSUME(attrs[0]);
	    break;

	  case WIN_INHERIT_COLORS:
	    inherit_colors = (int)attrs[1];
	    ATTR_CONSUME(attrs[0]);
	    break;

	  case WIN_INPUT_ONLY:	/* create an input only window */
	    input_only = TRUE;
	    ATTR_CONSUME(attrs[0]);
	    break;

	  case WIN_TRANSPARENT:/* create a "see through" window */
	    transparent = TRUE;
	    ATTR_CONSUME(attrs[0]);
	    break;

	  case WIN_IS_CLIENT_PANE:
	    is_client_pane = TRUE;
	    ATTR_CONSUME(attrs[0]);
	    break;

	  case XV_DEPTH:
	  case WIN_DEPTH:
	    vinfo_template.depth = (unsigned int)attrs[1];
	    vinfo_mask |= VisualDepthMask;
	    ATTR_CONSUME(attrs[0]);
	    break;

	  case WIN_BORDER:
	    border = (int) attrs[1];
	    ATTR_CONSUME(attrs[0]);
	    break;

	  case WIN_DYNAMIC_VISUAL:
	    /* This is an out-dated attribute that is only here
	     * for binary compatibility.  It was used to use
	     * the PseudoColor visual when the StaticColor visual
	     * was the default on OW1.0.  The application should now
	     * use XV_VISUAL, or XV_VISUAL_CLASS/XV_DEPTH to specify
	     * the visual they wish to create the window with.
	     */ 
	    if (!(vinfo_mask & VisualClassMask)) {
		vinfo_template.class = PseudoColor;
		vinfo_mask |= VisualClassMask;
	    }
	    ATTR_CONSUME(attrs[0]);
	    break;

	  case XV_VISUAL_CLASS:
	    vinfo_template.class = (int)attrs[1];
	    vinfo_mask |= VisualClassMask;
	    ATTR_CONSUME(attrs[0]);
	    break;

	  case XV_VISUAL:
	    vinfo_template.visualid = XVisualIDFromVisual((Visual *)attrs[1]);
	    vinfo_mask |= VisualIDMask;
	    ATTR_CONSUME(attrs[0]);
	    break;

#ifdef OW_I18N
          case WIN_USE_IM:
            win_use_im = (int) attrs[1];
            ATTR_CONSUME(attrs[0]);
            break;

	  case WIN_IC_ACTIVE:
	    win_ic_active = (int) attrs[1];
            ATTR_CONSUME(attrs[0]);
            break;

          case WIN_ERROR_MSG_WCS:
            error_msg = (char *) _xv_wcstombsdup((wchar_t *)attrs[1]);
	    is_wcs_error_msg = TRUE;
            ATTR_CONSUME(attrs[0]);
            break;

#ifdef FULL_R5
	  case WIN_X_IM_STYLE_MASK:
	    win_im_style_mask = (XIMStyle) attrs[1];
	    ATTR_CONSUME(attrs[0]);
	    break;
#endif /* FULL_R5 */
#endif /* OW_I18N */

	  case WIN_ERROR_MSG:
	    error_msg = (char *) attrs[1];
	    ATTR_CONSUME(attrs[0]);
	    break;

	    /*
	     * for compatiblity we check these to see if a character unit
	     * macro (ATTR_ROW/COL) has been used.  If so, then we must
	     * replace all of the character units in the avlist before any of
	     * the set procs are called.
	     */
	  case WIN_WIDTH:
	    convert_cu |= attr_is_cu((int) attrs[1]);
	    if (!convert_cu)
	    	win->cache_rect.r_width = (short) attrs[1];
	    break;
	  case WIN_HEIGHT:
	    convert_cu |= attr_is_cu((int) attrs[1]);
	    if (!convert_cu)
	        win->cache_rect.r_height = (short) attrs[1];
	    break;
	  case WIN_X:
	    convert_cu |= attr_is_cu((int) attrs[1]);
	    if (!convert_cu)
	        win->cache_rect.r_left = (short) attrs[1];
	    break;
	  case WIN_Y:
	    convert_cu |= attr_is_cu((int) attrs[1]);
	    if (!convert_cu)
	        win->cache_rect.r_top = (short) attrs[1];
	    break;
	  case WIN_RECT: {
	    Rect rect;
	    rect = *(Rect *) (attrs[1]);
	    win->cache_rect.r_width = rect.r_width;
	    win->cache_rect.r_height = rect.r_height;
	    win->cache_rect.r_left = rect.r_left;
	    win->cache_rect.r_top = rect.r_top;
	    break;
	  }
	  case WIN_FIT_WIDTH:
	  case WIN_FIT_HEIGHT:
	    convert_cu |= attr_is_cu((int) attrs[1]);
	    break;

	  case WIN_MOUSE_XY:
	    convert_cu |= attr_is_cu((int) attrs[1]) |
		attr_is_cu((int) attrs[2]);
	    break;

	  default:
	    break;
	}
    }

    if (is_root) {
	parent = 0;
	win->top_level = TRUE;
    } else {
	if (!parent_public) {
	    /* xv_create ensures that xv_default_screen is valid. */
	    parent_public = xv_get(xv_default_screen, XV_ROOT);
	    win->top_level = TRUE;
	} else if (xv_get(parent_public, WIN_IS_ROOT))
	    win->top_level = TRUE;
	else if (!xv_get(parent_public, XV_IS_SUBTYPE_OF, WINDOW)) {
	    xv_error(XV_ZERO,
        	     ERROR_SEVERITY, ERROR_NON_RECOVERABLE,
        	     ERROR_STRING, 
			XV_MSG("Subwindow owner is not a window\n"),
		     ERROR_PKG, WINDOW,
		     NULL);
	} else
	    win->top_level = FALSE;

	parent = WIN_PRIVATE(parent_public);
	if (!parent->layout_proc) {
	    eexit(
		XV_MSG("Parent window does not support children"), 
		error_msg);
	}
	DRAWABLE_INFO_MACRO(parent_public, parent_info);
    }

    screen = (is_root) ? parent_public : xv_screen(parent_info);
    server = xv_get(screen, SCREEN_SERVER);
    display = (Display *) xv_get(server, XV_DISPLAY);
    DRAWABLE_INFO_MACRO(win_public, info);


    win_object->private_data = (Xv_opaque) win;
    win->public_self = win_public;
    win->top_level_no_decor = no_decor;
    win->created = FALSE;
    win->has_border = border;
    win->input_only = input_only;
    win->transparent = transparent;
    win->is_client_pane = is_client_pane;
    win->in_fullscreen_mode = FALSE;
    win->being_rescaled = FALSE;
    win->scale = (int) WIN_SCALE_MEDIUM;
    win->map = TRUE;
    win->layout_proc = window_layout;
    win->xmask = ExposureMask;
    win->collapse_exposures = TRUE;
    win->collapse_motion_events = FALSE;
    win->background_pixmap = (Pixmap) NULL;
    win->deaf = FALSE;
    win->window_loop = FALSE;
    win->normal_cursor = (Xv_object)NULL;
    win->owner = parent;
    win->dropSites = NULL;
    win->dropInterest = NULL;
    win->softkey_flag = FALSE;
    /* caching the parent for performance reasons */
    win->parent = parent_public;
    win->cmdline = (char *)NULL;

#ifdef OW_I18N
    /** FIX ME! this needs to be generalized beyond the C locale
     **  since many locales may not use an IM */
    /* if input lang is C, cannot have an IM */
    if (strcmp((char *)xv_get(server, XV_LC_INPUT_LANG),"C")==0)
        win_use_im = FALSE;
    win->win_use_im = win_use_im;
    win->ic_active = win_ic_active;
    win->ic_conversion = FALSE;
    win->ic_created = FALSE;
    win->xic = (XIC)NULL;
    win->win_ic_committed = (char *)NULL;
    win->win_ic_committed_wcs = (wchar_t *)NULL;
    win->active_grab = FALSE;
    win->passive_grab = FALSE;

    if (win->win_use_im) {
	Frame			frame_public;
	Xv_Drawable_info 	*info;
	
	frame_public = (Frame)xv_get(win_public, WIN_FRAME);
	DRAWABLE_INFO_MACRO(frame_public, info);

    	/* Initialize callback structs */
    	win->start_pecb_struct.callback =  (XIMProc) NULL;
    	win->start_pecb_struct.client_data = (XPointer) NULL;
    	win->draw_pecb_struct.callback = (XIMProc) NULL;
    	win->draw_pecb_struct.client_data = (XPointer) NULL;
    	win->caret_pecb_struct.callback = (XIMProc) NULL;
    	win->caret_pecb_struct.client_data = (XPointer) NULL;
    	win->done_pecb_struct.callback = (XIMProc) NULL;
    	win->done_pecb_struct.client_data = (XPointer) NULL;

    	/* Set default status callbacks */
    	win->start_stcb_struct.callback = (XIMProc)_xv_status_start;
    	win->draw_stcb_struct.callback = (XIMProc)_xv_status_draw;
    	win->done_stcb_struct.callback = (XIMProc)_xv_status_done;
    	win->start_stcb_struct.client_data = 
    	win->draw_stcb_struct.client_data = 
    	win->done_stcb_struct.client_data = (XPointer)frame_public;

    	win->ic_focus_win = NULL;
    	win->tmp_ic_focus_win =  xv_xid(info);
#ifdef FULL_R5
	win->x_im_style_mask = (XIMStyle)win_im_style_mask;
#endif /* FULL_R5 */
    }
#endif /* OW_I18N */

   if (!is_root) {
	Rect default_rect;
	rect_construct(&default_rect, DEFAULT_X_Y, DEFAULT_X_Y,
			            DEFAULT_WIDTH_HEIGHT, DEFAULT_WIDTH_HEIGHT);
	sync_rect(win, &default_rect, &win->cache_rect);
	/* If it is WIN_EXTEND_TO_EDGE use default dims.  Need to come up
	 * with a way for this code to query its parent for its size.
	 */
	if (win->cache_rect.r_width == WIN_EXTEND_TO_EDGE)
		win->cache_rect.r_width = DEFAULT_WIDTH_HEIGHT;
	if (win->cache_rect.r_height == WIN_EXTEND_TO_EDGE)
		win->cache_rect.r_height = DEFAULT_WIDTH_HEIGHT;
    } else {
	/* Make sure that the root window gets the correct visual */
	vinfo_template.visualid = 
	  XVisualIDFromVisual(DefaultVisual(display, xv_get(screen, SCREEN_NUMBER)));
	vinfo_mask = VisualIDMask;
    }
    
    if (!is_root && !is_sel_window) {
        default_cursor = (Xv_Cursor) xv_get(screen, XV_KEY_DATA, WIN_CURSOR);
        if (!default_cursor) {
	    default_cursor = xv_create(screen, CURSOR,
				   CURSOR_SRC_CHAR, OLC_BASIC_PTR,
				   CURSOR_MASK_CHAR, OLC_BASIC_MASK_PTR,
				   /* Can never free */
				   XV_INCREMENT_REF_COUNT,	
				   NULL);
	    (void) xv_set(screen, XV_KEY_DATA, CURSOR_BASIC_PTR,
							   default_cursor, NULL);
	    (void) xv_set(screen, XV_KEY_DATA, WIN_CURSOR, default_cursor, NULL);
        } 
        (void) xv_set(default_cursor, XV_INCREMENT_REF_COUNT, NULL);
        win->cursor = (Xv_opaque) default_cursor;
    } else {
	win->cursor = (Xv_opaque)NULL;
    }

    /* Determine wether we want to inherit colors (and thus the vinfo)
     * from the parent
     */
    if (inherit_colors != WIN_NOT_DEFINED)
      win->inherit_colors = inherit_colors;
    else if (!(win->top_level) && (parent->inherit_colors))
      win->inherit_colors = TRUE;
    else 
      win->inherit_colors = FALSE;
    
    /* Find the visual and depth for the window creation.
     * If the application specified a visual, visual_class, or depth,
     * try to find the appropriate visual.
     */
    default_visual = (Screen_visual *)xv_get(screen, SCREEN_DEFAULT_VISUAL);
    xv_visual(info) = default_visual;
    if (vinfo_mask) {
	visual = (Screen_visual *)xv_get(screen, SCREEN_VISUAL, vinfo_mask, &vinfo_template);
	if (visual) {
	    xv_visual(info) = visual;
	    if (!is_root &&
		(xv_visual(info)->vinfo->visualid != xv_visual(parent_info)->vinfo->visualid))
	      win->inherit_colors = FALSE;
	}
    } else if (win->inherit_colors) {
	xv_visual(info) = xv_visual(parent_info);
    }
    if (!xv_visual(info)) {
	eexit(
	    XV_MSG("Window creation failed to get new visual"), 
	    error_msg);
    }

    /* By convention, if the visual class is even, it is 
     * static, otherwise it is dynamic 
     */
    xv_dynamic_color(info) = xv_visual(info)->vinfo->class % 2;
    
    /* Set up CMS information */
    if (win->inherit_colors) {
	xv_cms(info) = xv_cms(parent_info);
	xv_cms_bg(info) = xv_cms_bg(parent_info);
	xv_bg(info) = xv_bg(parent_info);
	xv_cms_fg(info) = xv_cms_fg(parent_info);
	xv_fg(info) = xv_fg(parent_info);
    } else {
	sprintf(cms_name, "xv_default_cms_for_0x%x", xv_visual(info)->vinfo->visualid);
	xv_cms(info) = (Cms)xv_find(screen, CMS,
             CMS_NAME, cms_name,				    
             XV_VISUAL, xv_visual(info)->vinfo->visual,
             CMS_DEFAULT_CMS, TRUE,
             CMS_SIZE, 2,
	     CMS_NAMED_COLORS, "white", "black", NULL,
	     NULL);
	xv_cms_bg(info) = 0;
	xv_bg(info) = (unsigned long)xv_get(xv_cms(info), CMS_PIXEL, NULL);
	xv_cms_fg(info) = 1;
	xv_fg(info) = (unsigned long)xv_get(xv_cms(info), CMS_PIXEL, 1);
    }	
    cmap_id = (Colormap)xv_get(xv_cms(info), XV_XID, NULL);

    xv_xid(info) = window_new(display, screen, win, cmap_id, parent_info);
    
    /* Default plane mask is all planes. */
    info->plane_mask = ~0;
    
    /* this is a window, not a bitmap */
    info->is_bitmap = 0;

    if (is_root) {
	/* Patch up the visual's root_window. */
	xv_root(info) = win_public;
	if (xv_visual(info) != default_visual)
	  default_visual->root_window = win_public;
    }
    info->private_gc = FALSE;

    win->font = (Xv_font)NULL;

    /*
     * inherit parent's font
     */
    if (parent_public && 
	((Xv_pkg *)xv_get(parent_public, XV_IS_SUBTYPE_OF, WINDOW)))  {
        win->font = (Xv_font) xv_get(parent_public, XV_FONT);
    }

    /*
     * If parent null or parent's font not available
     * create default font of server
     */
    if (!win->font)  {
        win->font = (Xv_font) xv_font_with_name(server, (char *)NULL);
    }

    (void) xv_set(win->font, XV_INCREMENT_REF_COUNT, NULL);

    /* register with the notifier */
    if (notify_set_event_func(win_public, window_default_event_func,
			      NOTIFY_SAFE) ==
	NOTIFY_FUNC_NULL) {
	eexit(
	    XV_MSG("notify_set_event_func failed in window creation"), 
	    error_msg);
	/*
	 * BUG: do we need this here? (void)notify_remove(win_public);
	 */
    }
    win->notify_safe_event_proc = (void (*) ()) window_default_event_func;

    if (notify_set_event_func(win_public, window_default_event_func,
			      NOTIFY_IMMEDIATE) ==
	NOTIFY_FUNC_NULL) {
	eexit(
	    XV_MSG("notify_set_event_func failed in window creation"), 
	    error_msg);
    }
    win->notify_immediate_event_proc = (void (*) ()) window_default_event_func;

    /* for compatibility */
    if (convert_cu)
	window_scan_and_convert_to_pixels(win_public, avlist);

#ifdef OW_I18N
    if (is_wcs_error_msg && error_msg)
	xv_free(error_msg);
#endif

    return XV_OK;
}


Pkg_private XID
window_new(display, screen, win,  cmap_id, parent_info)
    Display        	*display;
    Xv_opaque       	screen;
    Window_info 	*win;
    int                 cmap_id;
    Xv_Drawable_info 	*parent_info;
{
    Window                  new_window;
    XSetWindowAttributes    attrs;
    Rect            	    rect;
    unsigned long   	    value_mask = 0;     
    Xv_object       	    win_public = win->public_self;
    Xv_Drawable_info        *info;
    int			     transparent;

    DRAWABLE_INFO_MACRO(win_public, info);

    if (!parent_info) {
        new_window = RootWindow(display, (int)xv_get(screen, SCREEN_NUMBER));
        win_x_getrect(display, new_window, &rect);
        window_update_cache_rect(win_public, &rect);
    } else {
	/* A BadMatch will occur if you specify background_pixmap to None with
	 * parent of a different depth.  Therefore we treat this situation as
	 * if the application did not ask for WIN_TRANSPARENT.
	 */
	transparent = (win->transparent && (xv_depth(info) == xv_depth(parent_info)));
        attrs.event_mask = win->xmask;
        value_mask = CWEventMask;
        if (!win->input_only) {
            /* BitGravity/background/Cmap are invalid for InputOnly windows */
            attrs.bit_gravity = NorthWestGravity;
            value_mask |= CWBitGravity;
            attrs.colormap = (cmap_id) ? cmap_id : CopyFromParent;
            value_mask |= CWColormap;
            if (!transparent) {
                attrs.background_pixel = xv_bg(info);
                value_mask |= CWBackPixel;
                attrs.border_pixel = xv_fg(info);
                value_mask |= CWBorderPixel;
            }
        }

        if (transparent) {
            value_mask |= CWBackPixmap;
            attrs.background_pixmap = None;
        }
	if (win->cursor) {
	    value_mask |= CWCursor;
	    attrs.cursor = (Cursor)xv_get(win->cursor, XV_XID);
	}
        new_window = XCreateWindow(display,
		        xv_xid(parent_info),
		        win->cache_rect.r_left,
		        win->cache_rect.r_top,
		        win->cache_rect.r_width,
		        win->cache_rect.r_height,
		        win->has_border ? WIN_DEFAULT_BORDER_WIDTH : 0,
		        win->input_only ? 0 : xv_depth(info),
		        win->input_only ? InputOnly : CopyFromParent,
			xv_visual(info)->vinfo->visual ? xv_visual(info)->vinfo->visual : CopyFromParent,
		        value_mask,
		        &attrs);

    }
    XSaveContext(display, new_window, CONTEXT, (caddr_t)win_public);
    return (new_window);
}

Xv_private void
window_set_bit_gravity(win_public, value)
    Xv_Window       win_public;
    int             value;
{
    register Xv_Drawable_info *info;
    XSetWindowAttributes win_attr;

    win_attr.bit_gravity = value;
    DRAWABLE_INFO_MACRO(win_public, info);
    XChangeWindowAttributes(xv_display(info), xv_xid(info), CWBitGravity, &win_attr);

}

void
xv_main_loop(win_public)
    Xv_Window       win_public;
{
    Window_info    *win = WIN_PRIVATE(win_public);
    register Xv_Drawable_info *info;
    Display        *display;
    extern int      sview_gprof_start;

    DRAWABLE_INFO_MACRO(win_public, info);
    display = xv_display(info);
    /*
     * sync with the server to make sure we have all outstanding
     * ConfigureNotify events in the queue. Then process the events, then
     * finally map the frame.
     */
    xv_set(xv_server(info), SERVER_SYNC_AND_PROCESS_EVENTS, NULL);


    /* install win in tree */
    xv_set(win_public, XV_SHOW, TRUE, NULL);

    /* server is in journalling mode */
    if (xv_get(xv_server(info), SERVER_JOURNALLING))
	xv_set(xv_server(info), SERVER_JOURNAL_SYNC_EVENT, 1, NULL);

    XFlush(display);		/* flush anthing left in the buffer */
    (void) notify_start();	/* and then loop in the notifier */

    if (xv_default_server && xv_get(xv_default_server, SERVER_JOURNALLING))
	xv_set(xv_default_server, SERVER_JOURNAL_SYNC_EVENT, 1, NULL);
}

int				/* bool */
window_done(win)
    Xv_Window       win;
{
    register Xv_Window owner = win;
    register Xv_Window grand_owner;

    /*
     * find the frame for this window. Chase up the owner list until we hit
     * the root window.
     */
    while ((grand_owner = window_get(owner, WIN_OWNER))) {
	/*
	 * remember the current window as the child of the next owner.
	 */
	win = owner;
	owner = grand_owner;
    }
    /*
     * now grand_owner is NULL, owner is a root window, and win is the frame
     * to destroy.
     */
    xv_destroy(win);
    return TRUE;
}

/* A flag to tell us if the parent window is going to be destoryed. */
Pkg_private int
window_get_parent_dying()
{
    return (parent_dying);
}

Pkg_private int
window_set_parent_dying()
{
    parent_dying = TRUE;
}

Pkg_private int
window_unset_parent_dying()
{
    parent_dying = FALSE;
}

Pkg_private int
window_destroy_win_struct(win_public, status)
    register Xv_Window win_public;
    Destroy_status  status;
{
    register Window_info *win = WIN_PRIVATE(win_public);
    register Xv_Drawable_info *info;

    switch (status) {
      case DESTROY_CLEANUP: {
	/* Decrement the ref count on all ref counted objects */
	if (win->cursor)
	    (void) xv_set(win->cursor, XV_DECREMENT_REF_COUNT, NULL);
	if (win->menu)
	    (void) xv_set(win->menu, XV_DECREMENT_REF_COUNT, NULL);
	if (win->font)
	    (void) xv_set(win->font, XV_DECREMENT_REF_COUNT, NULL);

	if (win->cmdline && ((int)win->cmdline != -1))  {
	    free(win->cmdline);
	}

#ifdef OW_I18N
	if (win->win_ic_committed)
	   xv_free(win->win_ic_committed);
	if (win->win_ic_committed_wcs)
	   xv_free(win->win_ic_committed_wcs);
	if (win->ic_created && win->xic && win->win_use_im) {
	   XDestroyIC(win->xic);
	   win->xic = (XIC)NULL;
	}
#endif /* OW_I18N */
	DRAWABLE_INFO_MACRO(win_public, info);
	/*
	 * Remove conditions from notifier.  Could remove just conditions set
	 * in window_init(), but we do everyone a favor here because it is
	 * overwhelmingly the common case.
	 */
	(void) notify_remove(win_public);
	if (win->owner && win->owner->layout_proc)
	    (win->owner->layout_proc) (WIN_PUBLIC(win->owner), win_public,
				       WIN_DESTROY);

	/* Free up the drop site info we have been holding. */
	if (win->dropSites) {
	    Win_drop_site_list *dropSite = win->dropSites;
	    Win_drop_site_list dropSite_save;

            dropSite_save.next = dropSite->next; 
	    while(dropSite = (Win_drop_site_list *)
					(XV_SL_SAFE_NEXT((&dropSite_save)))) {
		/* The drop site linked list that the window maintains
		 * is cleaned up each time a drop site that it owns
		 * is destroyed.  In the end, all we have to do is
		 * free the head of the list, which is never used.
		 */
                dropSite_save.next = dropSite->next;
	        xv_destroy(dropSite->drop_item);
	    }
	    xv_free(win->dropSites);
	}

	/* If this is DESTROY_CLEANUP and the parent window is not going to */
	/* be destroyed, destroy the window.				    */
	if (!window_get_parent_dying()) {
	    win_free(win_public);
	    XFlush(xv_display(info));
	} else
	    XDeleteContext(xv_display(info), xv_xid(info), 1);

	/*
	 * Free private data of window
	 */
	free(win);
	break;
      }
      case DESTROY_PROCESS_DEATH: {
	DRAWABLE_INFO_MACRO(win_public, info);
	(void) notify_remove(win_public);
	XDeleteContext(xv_display(info), xv_xid(info), 1);
	break;
      }
      default:
	break;
    }
    return XV_OK;
}
