#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)scrn_vis.c 20.23 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/scrn_impl.h>
#include <xview/server.h>


Pkg_private Screen_visual *
screen_get_visual(display, screen, visual_info)
    Display	*display;
    Screen_info *screen;
    XVisualInfo *visual_info;
{
    Screen_visual *visual = screen->screen_visuals;
    
    /* Sanity check */
    if (!visual_info) {
	visual = NULL;
    } else {

	/* See if visual has already been created */
	while (visual && (visual->vinfo != visual_info))
	  visual = visual->next;
	
	/* If we didn't find one, create it and add it to the screen's list */
	if (!visual) {
	    visual = screen_new_visual(display, screen, 
				       RootWindow(display, screen->number),
				       visual_info->depth, visual_info);
	    if (visual) {
		/* The first screen visual is the default, which is always there */
		visual->next = screen->screen_visuals->next;
		screen->screen_visuals->next = visual;
	    }
	}
    }
    return(visual);
}


Pkg_private Screen_visual *
screen_get_image_visual(display, screen, xid, depth)
    Display	*display;
    Screen_info *screen;
    XID		 xid;
    unsigned int depth;
{
    Screen_visual *visual = screen->screen_visuals;
    
    /* See if visual has already been created */
    while (visual && (visual->vinfo || (visual->depth != depth)))
      visual = visual->next;
    
    /* If we didn't find one, create it and add it to the screen's list */
    if (!visual) {
	visual = screen_new_visual(display, screen, xid, depth, NULL);
	if (visual) {
	    /* The first screen visual is the default, which is always there */
	    visual->next = screen->screen_visuals->next;
	    screen->screen_visuals->next = visual;
	}
    }
    return(visual);
}


Pkg_private Screen_visual *
screen_new_visual(display, screen, xid, depth, visual_info)
    Display	*display;
    Screen_info *screen;
    XID		 xid;
    unsigned int depth;
    XVisualInfo *visual_info;
{
    Xv_private Xv_opaque cms_default_colormap();

    Screen_visual      *visual;
    GC			gc;
    XGCValues       	gc_values;

    /* Make sure colors are correct in gc. */
    /* BUG ALERT!  This is not handling color properly!!! */
    gc_values.foreground = BlackPixel(display, screen->number);
    gc_values.background = WhitePixel(display, screen->number);
    gc = XCreateGC(display, xid, 
		   GCForeground | GCBackground, &gc_values);
    if (!gc)
      return (Screen_visual *)NULL;

    visual = xv_alloc(Screen_visual);
    visual->screen = SCREEN_PUBLIC(screen);
    visual->server = screen->server;
    visual->display = display;
    visual->root_window = screen->root_window;
    visual->vinfo = visual_info;
    visual->depth = depth;
    if (visual_info == (XVisualInfo *)NULL)
      visual->colormaps = (Xv_opaque)NULL;
    else 
      visual->colormaps = 
	cms_default_colormap(screen->server, display, screen->number, visual_info);
    visual->gc = gc;
    visual->image_bitmap = (XImage *)NULL;
    visual->image_pixmap = (XImage *)NULL;

    visual->next = (Screen_visual *)NULL;
    return (visual);
}
