#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)screen.c 20.51 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/i18n_impl.h>
#include <xview_private/scrn_impl.h>
#include <xview/notify.h>
#include <xview/server.h>
#include <xview/cms.h>
#include <xview/window.h>
#include <xview/defaults.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#define VisualClassError -1
static Defaults_pairs visual_class_pairs[] = {
    "StaticGray",  StaticGray,
    "GrayScale",   GrayScale,
    "StaticColor", StaticColor,
    "PseudoColor", PseudoColor,
    "TrueColor",   TrueColor,
    "DirectColor", DirectColor,
    NULL,          VisualClassError
};

static Screen_visual *screen_default_visual();
static XVisualInfo   *screen_default_visual_info();
static void	      screen_input();
static void           screen_update_sun_wm_protocols();

typedef enum {
	XvNewValue,
	XvDeleted
} Xv_sel_state;

Pkg_private int
screen_init(parent, screen_public, avlist)
    Xv_opaque       parent;
    Xv_Screen       screen_public;
    Attr_avlist     avlist;
{
    register Screen_info       *screen;
    register Attr_avlist 	attrs;
    Xv_screen_struct 	       *screen_object;
    Xv_object       		font;
    XVisualInfo			visual_template;
    Display        	       *display;
    char			cms_name[100];
    Atom			type;
    int             		font_id,
    				format;
    unsigned long		nitems,
				bytes_after;

    /* Allocate private data and set up forward/backward links. */
    screen = (Screen_info *)xv_alloc(Screen_info);
    screen->public_self = screen_public;
    screen_object = (Xv_screen_struct *)screen_public;
    screen_object->private_data = (Xv_opaque)screen;

    screen->server = parent ? parent : xv_default_server;
    display = (Display *) xv_get(screen->server, XV_DISPLAY);
    screen->number = DefaultScreen(display);
    screen->gc_list = (Screen_OLGC_List *)NULL;
    screen->sun_wm_protocols = NULL;
    screen->num_sun_wm_protocols = 0;
    screen->sel_state = 0;

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch ((int) attrs[0]) {
	  case SCREEN_NUMBER:
	    if ((int) attrs[1] >= ScreenCount(display)) {
		xv_error(XV_ZERO,
			 ERROR_BAD_VALUE, attrs[1], attrs[0],
			 ERROR_PKG, SCREEN,
			 NULL);
		xv_free(screen);
		return XV_ERROR;
	    }
	    screen->number = (int) attrs[1];
	    ATTR_CONSUME(attrs[0]);
	    break;

	  default:
	    break;
	}
    }

    /* get information about all visuals supported by this window */
    visual_template.screen = screen->number;
    screen->visual_infos = XGetVisualInfo(display, VisualScreenMask, 
					  &visual_template,
					  &(screen->num_visuals));

    /* get the default visual from the list */
    screen->screen_visuals = screen_default_visual(display, screen);

    /* This should be changed to make the default colors controlled by 
     * -fg, and -bg.
     */
    sprintf(cms_name, "xv_default_cms_for_0x%x",
	    screen->screen_visuals->vinfo->visualid);
    screen->default_cms = (Cms)xv_create(screen_public, CMS,
        CMS_NAME, cms_name,
        XV_VISUAL, screen->screen_visuals->vinfo->visual,
        CMS_TYPE, XV_STATIC_CMS,					 
        CMS_SIZE, 2,
        CMS_NAMED_COLORS, "White", "Black", NULL,
        CMS_DEFAULT_CMS, TRUE,					 
        NULL);

    /*
     * By default, monochrome leaf windows (ie. windows into which bits are
     * actually written) are set to non-retained. This can be turned to retained
     *  by a cmdline arg for debugging.
     */
    if (DisplayPlanes(display, screen->number) > 1) {
	screen->retain_windows = FALSE;
    } else {
	screen->retain_windows =
	    !(defaults_get_boolean("window.mono.disableRetained",
				   "Window.Mono.DisableRetained", TRUE));
    }

    /* set the default font in the GC for this screen */
    font = (Xv_object) xv_get(screen->server, SERVER_FONT_WITH_NAME, NULL, NULL);
    if (!font) {
	XFree((char *)(screen->visual_infos));
	xv_free(screen);
	return XV_ERROR;
    }
    font_id = (int) xv_get(font, XV_XID);
    xv_set_default_font(xv_get(screen->server, XV_DISPLAY),
			screen->number, font_id);

    /* NOTE: Do we really need the screen_layout proc? */
    screen->root_window =
	xv_create(screen_public, WINDOW,
		  WIN_IS_ROOT,
		  WIN_LAYOUT_PROC, screen_layout,
		  NULL);

    if (!screen->root_window) {
	XFree((char *)(screen->visual_infos));
	xv_free(screen);
	return XV_ERROR;
    }

    xv_set(screen->server,
		SERVER_PRIVATE_XEVENT_PROC, screen_input, screen->root_window,
		SERVER_PRIVATE_XEVENT_MASK, (XID)xv_get(screen->root_window,
							XV_XID),
					    PropertyChangeMask,
					    screen->root_window,
		NULL);

    /* now tell the server it has a new screen */
    xv_set(screen->server,
	   SERVER_NTH_SCREEN, screen->number, screen_public,
	   NULL);

    /* Store away any sun specific WM_PROTOCOLS. */
    (void)screen_update_sun_wm_protocols(screen->root_window, XvNewValue);

    return XV_OK;
}


static Screen_visual *
screen_default_visual(display, screen)
    Display	*display;
    Screen_info *screen;
{
    Screen_visual *new_visual;
    XVisualInfo   *default_vinfo;
    
    default_vinfo = screen_default_visual_info(display, screen);
    new_visual = screen_new_visual(display, screen, 
				   RootWindow(display, screen->number), 
				   default_vinfo->depth, default_vinfo);
    return(new_visual);
}


static XVisualInfo *
screen_default_visual_info(display, screen)
    Display	*display;
    Screen_info *screen;
{
    XVisualInfo *visual_info = (XVisualInfo *)NULL;
    XVisualInfo template;
    long	mask = 0;
    
    if (defaults_exists("window.visual", "Window.Visual")) {
	template.class = defaults_get_enum("window.visual",
					   "Window.Visual",
					   visual_class_pairs);
	if (template.class != VisualClassError)
	  mask |= VisualClassMask;
	else {
	    char message[1000];
	    
	    sprintf(message, XV_MSG("Unknown visual class \"%s\", using default visual\n"),
	    defaults_get_string("window.visual", "Window.Visual",
				(char *)NULL));
	    xv_error(XV_ZERO,
		     ERROR_STRING, message,
		     ERROR_PKG, SCREEN,
		     NULL);
	}
    }
        
    if (defaults_exists("window.depth", "Window.Depth")) {
	template.depth = 
	  (unsigned int)defaults_get_integer("window.depth", 
					     "Window.Depth",
					     DefaultDepth(display, screen->number));
	mask |= VisualDepthMask;
    }
    
    if (mask)
      visual_info = screen_match_visual_info(screen, mask, &template);
    
    /* If user didn't specify visual, or there was an error getting 
     * the visual requested, then use the screen's default visual.
     */
    if (visual_info == (XVisualInfo *)NULL) {
	template.visualid = XVisualIDFromVisual(DefaultVisual(display, screen->number));
	visual_info = screen_match_visual_info(screen, VisualIDMask, &template);
    }
    return(visual_info);
}


Pkg_private int
screen_destroy(screen_public, status)
    Xv_Screen       screen_public;
    Destroy_status  status;
{
    Screen_info    *screen = SCREEN_PRIVATE(screen_public);

    if (notify_post_destroy(screen->root_window, status, NOTIFY_IMMEDIATE) ==
	NOTIFY_DESTROY_VETOED)
	return XV_ERROR;

    if ((status == DESTROY_CHECKING) || (status == DESTROY_SAVE_YOURSELF) ||
	(status == DESTROY_PROCESS_DEATH))
	return XV_OK;

    /* now tell the server it has lost a screen */
    xv_set(screen->server, SERVER_NTH_SCREEN, screen->number, (Xv_Screen) 0, NULL);
    
    XFree((char *)(screen->visual_infos));
    if (screen->sun_wm_protocols)
        XFree((char *)(screen->sun_wm_protocols));
    free(screen);

    return XV_OK;
}

/* ARGSUSED */
Pkg_private Xv_opaque
screen_set_avlist(screen_public, avlist)
    Xv_Screen       screen_public;
    Attr_attribute  avlist[];
{
    register Attr_avlist    attrs;

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (attrs[0]) {
	    default:
		xv_check_bad_attr(&xv_screen_pkg, attrs[0]);
		break;
	}
    }

    return (XV_OK);
}


Xv_private void
screen_set_clip_rects(screen_public, xrect_array, rect_count)
    Xv_Screen       screen_public;
    XRectangle     *xrect_array;
    int             rect_count;
{
    Screen_info    *screen = SCREEN_PRIVATE(screen_public);
    int             i;

    for (i = 0; i < rect_count; i++)
	screen->clip_xrects.rect_array[i] = xrect_array[i];
    screen->clip_xrects.count = rect_count;
}


Xv_private Xv_xrectlist *
screen_get_clip_rects(screen_public)
    Xv_Screen       screen_public;
{
    Screen_info    *screen = SCREEN_PRIVATE(screen_public);

    return (&screen->clip_xrects);
}


Xv_private void
screen_set_cached_window_busy(screen_public, window, busy)
    Xv_Screen       screen_public;
    Xv_window       window;
    int		    busy;
{
    Screen_info    *screen = SCREEN_PRIVATE(screen_public);
    Xv_cached_window *cached_window;

    for (cached_window = screen->cached_windows; cached_window != NULL;
	 cached_window = cached_window->next) {
	if (cached_window->window == window) {
	    cached_window->busy = busy;
	    break;
	}
    }
    if (cached_window == NULL) {
	xv_error(XV_ZERO,
		 ERROR_STRING, 
		 XV_MSG("Unable to return window to screen cache"),
		 ERROR_PKG, SCREEN,
		 NULL);
    }
}

Xv_private int
screen_get_sun_wm_protocols(screen_public)
    Xv_Screen       screen_public;
{
    Screen_info    *screen = SCREEN_PRIVATE(screen_public);

    return (screen->num_sun_wm_protocols);
}

/*
 * _SUN_WM_PROTOCOLS(screen->num_sun_wm_protocols) stores an array 
 * of Atoms.
 * This function checks if 'check_atom' is one of them.
 */
Xv_private int
screen_check_sun_wm_protocols(screen_public, check_atom)
    Xv_Screen       screen_public;
    Atom       	    check_atom;
{
    Screen_info    *screen = SCREEN_PRIVATE(screen_public);
    int		   i;

    for (i = 0; i < screen->num_sun_wm_protocols; i++) {
        if (screen->sun_wm_protocols[i] == check_atom) {
            return(True);
        }
    }

    return(False);
}

static void
screen_update_sun_wm_protocols(window, state)
    Xv_object 	    	window;
    Xv_sel_state        state;
{
    Xv_Screen	    screen_public = XV_SCREEN_FROM_WINDOW(window);
    Screen_info    *screen = SCREEN_PRIVATE(screen_public);
    int             format;
    Atom  	    type;
    unsigned long   nitems,
		    bytes_after;
    Display 	   *dpy = (Display *) xv_get(screen->server, XV_DISPLAY);

    if (screen->sun_wm_protocols) {
	XFree((char *)screen->sun_wm_protocols);
        screen->num_sun_wm_protocols = 0;
    }

    if (state == XvDeleted) {
	screen->sun_wm_protocols = NULL;
	return;
    }

    /* Store away any sun specific WM_PROTOCOLS. */
    if (XGetWindowProperty(dpy, RootWindow(dpy, screen->number),
			     (Atom)xv_get(screen->server,
					  SERVER_ATOM, "_SUN_WM_PROTOCOLS"),
			     0, 100L, False, XA_ATOM, &type, &format,
			     &nitems, &bytes_after,
                             (unsigned char **)(&screen->sun_wm_protocols))
			     != Success) {

	screen->sun_wm_protocols = NULL;
    } else if (type == None) {
	screen->sun_wm_protocols = NULL;
    } else {
        XWindowAttributes xwa;

        screen->num_sun_wm_protocols = nitems;
        XGetWindowAttributes(dpy,RootWindow(dpy,screen->number),&xwa);
        xwa.your_event_mask ^= PropertyChangeMask;
        XSelectInput(dpy,RootWindow(dpy,screen->number),xwa.your_event_mask);
    }
}

/* Receives events from RootWindow.  Specifically looking for PropertyNotify
 * events for the _SUN_WM_PROTOCOLS property.
 */
static void
screen_input(server, dpy, xev, obj)
    Xv_server	  	 server;
    Display		*dpy;
    XPropertyEvent	*xev;	
    Xv_opaque		 obj;
{
    switch(xev->type) {
      case PropertyNotify:
	if (xev->atom == (Atom)xv_get(server, SERVER_ATOM,
					"_SUN_WM_PROTOCOLS")) {
	    Xv_sel_state state = XvNewValue;

	    if (xev->state == PropertyDelete)
		state = XvDeleted;

            (void)screen_update_sun_wm_protocols(obj, state);
        } else if (xev->atom == (Atom)xv_get(server, SERVER_ATOM,
					    "_SUN_QUICK_SELECTION_KEY_STATE")) {
             
             Xv_Screen	    screen_public = XV_SCREEN_FROM_WINDOW(obj);
             Screen_info    *screen = SCREEN_PRIVATE(screen_public);
             XWindowAttributes xwa;
   
             /* The Window manager is not olwm.. so we can shut off
              * property Notify events from the root Window 
              */
             XGetWindowAttributes(dpy,RootWindow(dpy,screen->number),&xwa);
             xwa.your_event_mask ^= PropertyChangeMask;
             XSelectInput(dpy,RootWindow(dpy,screen->number),xwa.your_event_mask);

         /*
          * Screen_info    *screen = SCREEN_PRIVATE(XV_SCREEN_FROM_WINDOW(obj));
	  * Atom           type;
	  * int            format;
	  * unsigned long  nitems,
	  *                bytes_after;
	  * unsigned char *data;
          *
	  * if (xev->state == PropertyDelete)
	  *    screen->sel_state = 0;
	  * else {
	  *	if (XGetWindowProperty((Display *)XV_DISPLAY_FROM_WINDOW(obj),
          *                      (XID)xv_get(screen->root_window, XV_XID),
	  *	   	         (Atom)xv_get(server, SERVER_ATOM,
          *                                 "_SUN_QUICK_SELECTION_KEY_STATE"),
          *                      0L, 1, False, XA_ATOM,
          *                      &type, &format, &nitems, &bytes_after,
          *                      &data) != Success)
          *         screen->sel_state = 0;
          *     else if (type == None) {
          *         screen->sel_state = 0;
          *     } else {
          *         screen->sel_state = *(Atom *)data;
	  *         XFree((char *)data);
	  *	}
	  * }
          */
	} 
      default:
	break;
    }
}
