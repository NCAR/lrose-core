#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)scrn_get.c 20.54 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>
#include <xview_private/scrn_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/notify.h>
#include <xview/win_input.h>
#include <xview/win_screen.h>
#include <xview/base.h>
#include <xview/font.h>
#include <xview/server.h>
/* mbuck@debian.org */
#if 1
#include <X11/Xlibint.h>
#endif
#define X11R6

/* Bitmap used for the inactive GC */
static unsigned short screen_gray50_bitmap[16] = {   /* 50% gray pattern */
    0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555,
    0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555
};

Xv_private Xv_Window screen_get_cached_window();
Xv_private GC *screen_get_cached_gc_list();

/* Caller turns varargs into va_list that has already been va_start'd */
/*ARGSUSED*/
Pkg_private     Xv_opaque
screen_get_attr(screen_public, status, attr, args)
    Xv_Screen       screen_public;
    int            *status;	/* initialized by caller */
    Attr_attribute  attr;
    va_list         args;
{
    Screen_info *screen = SCREEN_PRIVATE(screen_public);
    Xv_opaque    value = (Xv_opaque)NULL;
    
    switch (attr) {
      /* Public Attributes */

      case SCREEN_SERVER:
	value = (Xv_opaque)screen->server;
	break;
	
      case SCREEN_OLGC_LIST:
	{
	    Xv_opaque window;
	    
	    window = va_arg(args, Xv_opaque);
	    if (xv_get(window, XV_IS_SUBTYPE_OF, WINDOW))
	      value = (Xv_opaque)screen_get_cached_gc_list(screen, window);
	    else
	      *status = XV_ERROR;
	}
	break;

      case SCREEN_NUMBER:
	value = (Xv_opaque)screen->number;
	break;
	
      case XV_ROOT:
	value = (Xv_opaque)screen->root_window;
	break;

      /* Private Attributes */

      case SCREEN_DEFAULT_CMS:
	value = (Xv_opaque)(screen->default_cms);
	break;
	
      case SCREEN_RETAIN_WINDOWS:
	value = (Xv_opaque)screen->retain_windows;
	break;
	

      case SCREEN_DEFAULT_VISUAL:
	value = (Xv_opaque)&(screen->screen_visuals[0]);
	break;

      case SCREEN_VISUAL:
	{
	    XVisualInfo *vinfo;
	    XVisualInfo *template;
	    long vinfo_mask;
	    
	    vinfo_mask = va_arg(args, long);
	    template = (XVisualInfo *)va_arg(args, Xv_opaque);
	    vinfo = screen_match_visual_info(screen, vinfo_mask, template);
	    value = (Xv_opaque)screen_get_visual(
				 (Display *)xv_get(screen->server, XV_DISPLAY),
				 screen, vinfo);
	}
	break;
	
      case SCREEN_IMAGE_VISUAL:
	{
	    XID		 xid;
	    unsigned int depth;
	    Display     *display = (Display *)xv_get(screen->server, XV_DISPLAY);
	    
	    xid = va_arg(args, XID);
	    depth = va_arg(args, unsigned int);
	    value = (Xv_opaque)screen_get_image_visual(display, screen, xid,
						       depth);
	}
	break;
	
      case SCREEN_SUN_WINDOW_STATE:
        value = screen_check_sun_wm_protocols(screen_public, 
		            (Atom)xv_get(screen->server, SERVER_ATOM,
					"_SUN_WINDOW_STATE"));
	break;

      case SCREEN_SELECTION_STATE:
        value = (Xv_opaque)screen->sel_state;
	break;
	
      default:
	if (xv_check_bad_attr(&xv_screen_pkg, attr) == XV_ERROR) {
	    *status = XV_ERROR;
	}
    }
    return(value);
}

/* 
 * screen_match_visual_info - screen wrapper around XGetVisualInfo, but only
 *     using the visualid, depth and class of the template/mask.
 */
Pkg_private XVisualInfo *
screen_match_visual_info(screen, mask, template)
    Screen_info *screen;
    long         mask;
    XVisualInfo *template;
{
    XVisualInfo *best_match = (XVisualInfo *)NULL;
    XVisualInfo *match;
    int visual;
    int default_depth;

    if (screen->screen_visuals)
      default_depth = screen->screen_visuals->depth;
    else
      default_depth = DefaultDepth((Display *)xv_get(screen->server, XV_DISPLAY), 
				   screen->number);
    
    for (visual = 0; visual < screen->num_visuals; visual++) {
	/* Check visualid */
	if ((mask & VisualIDMask) && 
	    (template->visualid != screen->visual_infos[visual].visualid))
	  continue;

	/* Check class */
	if ((mask & VisualClassMask) &&
	    (template->class != screen->visual_infos[visual].class))
	  continue;

	/* Check depth */
	if ((mask & VisualDepthMask) &&
	    (template->depth != screen->visual_infos[visual].depth))
	  continue;
	
	/* Found one */
	match = &(screen->visual_infos[visual]);
	if (!best_match)
	  best_match = match;

	/* 
	 * If they asked for a specific Visual ID, or they specified both the
	 * depth and class, just return the first one found.  Otherwise, keep 
	 * searching.
	 */
	if ((mask & VisualIDMask) || 
	    ((mask & VisualDepthMask) && (mask & VisualClassMask)))
	  return(best_match);
	else if (match != best_match) {
	    if (mask & VisualClassMask) {
		/* They only specified the class, so favor the default depth,
		 * or the highest depth visual with that class.
		 */
		if (match->depth == default_depth)
		  best_match = match;
		else if ((best_match->depth != default_depth) &&
			 (best_match->depth < match->depth))
		  best_match = match;
	    } else {
		/* They only specified the depth, so favor the highest class
		 * visual with that depth. However, we special case TrueColor
		 * to be favored over DirectColor because if someone was just
		 * trying to get at a 24 bit visual, they most likely want the
		 * TrueColor visual, and not the DirectColor.
		 */
		if ((best_match->class == DirectColor) &&
		    (match->class == TrueColor))
		  best_match = match;
		else if ((best_match->class < match->class) &&
			 ((best_match->class != TrueColor) ||
			  (match->class != DirectColor)))
		  best_match = &(screen->visual_infos[visual]);
	    }
	    
	}
    }
    return(best_match);
}


/*
 * Windows are cached in the screen and shared between menus.
 */
Xv_private Xv_Window
screen_get_cached_window(screen_public, event_proc, borders, visual, new_window)
    Xv_Screen       screen_public;
    Notify_func	    event_proc;
    int             borders;
    Visual	   *visual;
    int		   *new_window;	/* output parameter */
{
    Screen_info    *screen = SCREEN_PRIVATE(screen_public);
    Xv_cached_window *cached_window;

    for (cached_window = screen->cached_windows; cached_window != NULL;
	 cached_window = cached_window->next) {
	if (cached_window->busy == FALSE &&
	    cached_window->borders == (short) borders &&
	    XVisualIDFromVisual(cached_window->visual) == 
	        XVisualIDFromVisual(visual)) {
	    cached_window->busy = TRUE;
	    *new_window = FALSE;
	    return ((Xv_Window) cached_window->window);
	}
    }

    *new_window = TRUE;
    cached_window = (Xv_cached_window *) xv_alloc(Xv_cached_window);
    cached_window->window = (Xv_Window) xv_create(
	xv_get(screen_public, XV_ROOT), WINDOW,
	WIN_BIT_GRAVITY, ForgetGravity,
	WIN_BORDER, borders,
        XV_VISUAL, visual,						  
	WIN_NOTIFY_SAFE_EVENT_PROC, event_proc,
	WIN_TOP_LEVEL_NO_DECOR, TRUE,
	WIN_SAVE_UNDER, TRUE,
	XV_SHOW, FALSE,
	NULL);

    if (screen->cached_windows == NULL) {
	screen->cached_windows = cached_window;
    } else {
	cached_window->next = screen->cached_windows;
	screen->cached_windows = cached_window;
    }
    cached_window->busy = TRUE;
    cached_window->borders = (short) borders;
    cached_window->visual = visual;
    return ((Xv_Window) cached_window->window);
}

Xv_private GC *
screen_get_cached_gc_list(screen, window)
    Screen_info    *screen;
    Xv_Window       window;
{
    Xv_Drawable_info *info;
    Screen_OLGC_List   *gc_list = screen->gc_list;
    Screen_OLGC_List   *new_gc_list;	
    XGCValues	      gc_value;
    unsigned long     gc_value_mask;
    int		      gc;
    Xv_Font	      std_font, bold_font;

    /* Search the screen's cached gc_list list and see if
     * there is a suitable list for this window.
     */
    DRAWABLE_INFO_MACRO(window, info);
    while (gc_list && (gc_list->depth != xv_depth(info)))
      gc_list = gc_list->next;
    if (gc_list)
      return(gc_list->gcs);

    /* None was found, so create a new one */
    new_gc_list = (Screen_OLGC_List *)xv_alloc(Screen_OLGC_List);
    new_gc_list->depth = xv_depth(info);
    new_gc_list->next = screen->gc_list;
    screen->gc_list = new_gc_list;

    /* Create each of the GCs for the list */
    std_font = (Xv_Font)xv_get(window, XV_FONT);
    for (gc = 0; gc < SCREEN_OLGC_LIST_SIZE; gc++) {
	gc_value.foreground = xv_fg(info);
	gc_value.background = xv_bg(info);
	gc_value.function = GXcopy;
	gc_value.plane_mask = xv_plane_mask(info);
	gc_value.graphics_exposures = FALSE;
	gc_value_mask = GCForeground | GCBackground | GCFunction |
	                GCPlaneMask | GCGraphicsExposures;
	switch (gc) {
	  case SCREEN_SET_GC:
	  case SCREEN_NONSTD_GC:
	    break;
	  case SCREEN_CLR_GC:
	    gc_value.foreground = xv_bg(info);
	    break;
	  case SCREEN_TEXT_GC:
#ifdef OW_I18N  	
            /* do nothing since using font sets always */
#else /* OW_I18N */
	    gc_value.font = (Font)xv_get(std_font, XV_XID);
	    gc_value_mask |= GCFont;
#endif /* OW_I18N */
	    break;
	  case SCREEN_BOLD_GC:
#ifdef OW_I18N
            /* do nothing since using font sets always */
#else /* OW_I18N */
	    bold_font = (Xv_Font)xv_find(window, FONT,
		FONT_FAMILY, xv_get(std_font, FONT_FAMILY),
		FONT_STYLE, FONT_STYLE_BOLD,
		FONT_SIZE, xv_get(std_font, FONT_SIZE),
		NULL);
	    if (bold_font == XV_ZERO) {
		xv_error(XV_ZERO,
		    ERROR_STRING,
		        XV_MSG("Unable to find bold font; using standard font"),
		    ERROR_PKG, SCREEN,
		    NULL);
		bold_font = std_font;
	    }
	    gc_value.font = (Font)xv_get(bold_font, XV_XID);
	    gc_value_mask |= GCFont;
#endif /* OW_I18N */
	    break;
	  case SCREEN_GLYPH_GC:
	    gc_value.font = (Font)xv_get(xv_get(window, WIN_GLYPH_FONT),
					 XV_XID);
	    gc_value_mask |= GCFont;
	    break;
	  case SCREEN_INACTIVE_GC:
	    gc_value.foreground = xv_bg(info);
	    gc_value.background = xv_fg(info);
	    gc_value.stipple = 
	      XCreateBitmapFromData(xv_display(info), xv_xid(info), 
				    (char *)screen_gray50_bitmap, 16, 16);
	    gc_value.fill_style = FillStippled;
	    gc_value_mask |= GCStipple | GCFillStyle;
	    break;
	  case SCREEN_DIM_GC:
	    gc_value.line_style = LineDoubleDash;
	    gc_value.dashes = 1;
	    gc_value_mask |= GCLineStyle | GCDashList;
	    break;
	  case SCREEN_INVERT_GC:
	    gc_value.function = GXinvert;
	    gc_value.plane_mask =
		gc_value.foreground ^ gc_value.background;
	    break;
	  case SCREEN_RUBBERBAND_GC:
	    gc_value.subwindow_mode = IncludeInferiors;
	    gc_value.function = GXinvert;
	    gc_value_mask |= GCSubwindowMode | GCFunction;
	    break;

	}
	new_gc_list->gcs[gc] = XCreateGC(xv_display(info), xv_xid(info), 
					 gc_value_mask, &gc_value);
    }
    return(new_gc_list->gcs);
}

Xv_private void
screen_adjust_gc_color(window, gc_index)
    Xv_Window   window;
    int		gc_index;
{
    Xv_Drawable_info *info;
    unsigned long    new_fg;
    unsigned long    new_bg;
    unsigned long    new_plane_mask;
    XGCValues	     gc_values;
    GC               *gc_list;
#ifdef X11R6
	/* lumpi@dobag.in-berlin.de */
    XGCValues	gc_tmp;
#endif
    
    DRAWABLE_INFO_MACRO(window, info);
    new_plane_mask = xv_plane_mask(info);

    gc_list = (GC *)xv_get(xv_screen(info), SCREEN_OLGC_LIST, window);
    switch (gc_index) {
      case SCREEN_SET_GC:
      case SCREEN_NONSTD_GC:
      case SCREEN_TEXT_GC:
      case SCREEN_BOLD_GC:
      case SCREEN_GLYPH_GC:
      case SCREEN_DIM_GC:
	new_fg = xv_fg(info);
	new_bg = xv_bg(info);
	break;
      case SCREEN_INVERT_GC:
	new_fg = xv_fg(info);
	new_bg = xv_bg(info);
	new_plane_mask = new_fg ^ new_bg;
	break;
      case SCREEN_CLR_GC:
	new_fg = new_bg = xv_bg(info);
	break;
      case SCREEN_INACTIVE_GC:
	new_fg = xv_bg(info);
	new_bg = xv_fg(info);
	break;
    }
#ifdef X11R6
	/* lumpi@dobag.in-berlin.de */
	XGetGCValues(xv_display(info), gc_list[gc_index],
		     GCPlaneMask|GCForeground|GCBackground,&gc_tmp);

    if ((new_fg != gc_tmp.foreground )||
	(new_bg != gc_tmp.background) || 
	(new_plane_mask != gc_tmp.plane_mask)) {
	gc_tmp.foreground = new_fg;
	gc_tmp.background = new_bg;
	gc_tmp.plane_mask = new_plane_mask;
	XChangeGC(xv_display(info), gc_list[gc_index],
		  GCForeground | GCBackground | GCPlaneMask, &gc_tmp);
    }
#else
    if (new_fg != gc_list[gc_index]->values.foreground ||
	new_bg != gc_list[gc_index]->values.background ||
	new_plane_mask != gc_list[gc_index]->values.plane_mask) {
	gc_values.foreground = new_fg;
	gc_values.background = new_bg;
	gc_values.plane_mask = new_plane_mask;
	XChangeGC(xv_display(info), gc_list[gc_index],
		  GCForeground | GCBackground | GCPlaneMask, &gc_values);
    }
#endif
}
