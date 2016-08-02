#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)openwin.c 1.37 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Package:     openwin
 *
 * Module:      openwin.c
 * 
 * Description: Implements general creation and initialization for openwin
 * 
 */

#include <stdio.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/ow_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/defaults.h>
#include <xview/cms.h>

/*
 * Package private functions
 */ 
Pkg_private int openwin_init();
Pkg_private int openwin_destroy();
#ifndef NO_OPENWIN_PAINT_BG
Pkg_private void openwin_set_bg_color();
#endif /* NO_OPENWIN_PAINT_BG */

/*
 * Module private functions
 */
static int  openwin_layout();

/*
 * Global Data
 */
static Defaults_pairs sb_placement_pairs[] = 
{
    "Left",  OPENWIN_SCROLLBAR_LEFT,
    "left",  OPENWIN_SCROLLBAR_LEFT,
    "Right", OPENWIN_SCROLLBAR_RIGHT,
    "right", OPENWIN_SCROLLBAR_RIGHT,
    NULL,    OPENWIN_SCROLLBAR_RIGHT,
};

/*-------------------Function Definitions-------------------*/

/*
 * openwin_init - initialize the openwin data structure
 */
/*ARGSUSED*/
Pkg_private int
openwin_init(parent, owin_public, avlist)
    Xv_opaque       parent;

    Xv_opaque       owin_public;
    Xv_opaque      *avlist;
{
    Xv_openwin     *openwin = (Xv_openwin *) owin_public;
    Xv_openwin_info *owin;
#ifndef NO_OPENWIN_PAINT_BG
    Xv_Drawable_info *info;
#endif /* NO_OPENWIN_PAINT_BG */

    if (!(owin = xv_alloc(Xv_openwin_info))) {
	fprintf(stderr, 
	    XV_MSG("can't allocate openwin structure. Abort\n"));
	return XV_ERROR;
    }
    owin->public_self = owin_public;
    openwin->private_data = (Xv_opaque) owin;
    owin->margin = OPENWIN_REGULAR_VIEW_MARGIN;
    owin->view_class = (Xv_pkg *) WINDOW;
    owin->cached_rect = *(Rect *) xv_get(owin_public, WIN_RECT);
#ifdef SELECTABLE_VIEWS
    owin->seln_client =	seln_create(openwin_seln_function, openwin_seln_reply, 
				    (char *) owin);
#endif /* SELECTABLE_VIEWS */
    STATUS_SET(owin, auto_clear);
    STATUS_SET(owin, show_borders);

    if (defaults_get_enum("openWindows.scrollbarPlacement",
			  "OpenWindows.ScrollbarPlacement",
			  sb_placement_pairs) == OPENWIN_SCROLLBAR_LEFT)
      STATUS_SET(owin, left_scrollbars);
    else
      STATUS_RESET(owin, left_scrollbars);

    owin->layout_proc = (int (*) ()) xv_get(owin_public, WIN_LAYOUT_PROC);

#ifndef NO_OPENWIN_PAINT_BG
    DRAWABLE_INFO_MACRO(owin_public, info);
    if ((xv_depth(info) > 1) && 
	 defaults_get_boolean("OpenWindows.3DLook.Color", 
			      "OpenWindows.3DLook.Color",
			      TRUE)) {

	    STATUS_SET(owin, paint_bg);
	    XParseColor(xv_display(info), 
			(Colormap)xv_get(xv_cms(info), XV_XID, NULL),
			defaults_get_string("openWindows.windowColor",
					    "OpenWindows.WindowColor", "#cccccc"),
			&(owin->background));
	    openwin_set_bg_color(owin_public);
    } else
      STATUS_RESET(owin, paint_bg);
#endif /* NO_OPENWIN_PAINT_BG */

    /*
     * For performance reasons, the openwin borders are always being painted
     * using X borders. This might change when border highlighting is
     * implemented for pane selection. WIN_CONSUME_PICK_EVENT, MS_LEFT &
     * WIN_REPAINT will have to turned on here to implement border
     * highlighting.
     */
    xv_set(owin_public,
	   WIN_NOTIFY_SAFE_EVENT_PROC, openwin_event,
	   WIN_NOTIFY_IMMEDIATE_EVENT_PROC, openwin_event,
#ifdef SELECTABLE_VIEWS
	   WIN_CONSUME_EVENT, ACTION_SELECT,
#endif /* SELECTABLE_VIEWS */
	   /*
	   WIN_INHERIT_COLORS, TRUE,
	   */
	   WIN_LAYOUT_PROC, openwin_layout,
	   NULL);

    return XV_OK;
}

/*
 * openwin_destroy - handle the cleanup and destruction of an openwin
 */
Pkg_private int
openwin_destroy(owin_public, destroy_status)
    Openwin         owin_public;
    Destroy_status  destroy_status;
{
    Xv_openwin_info *owin = OPENWIN_PRIVATE(owin_public);

    if ((destroy_status == DESTROY_CLEANUP) ||
	(destroy_status == DESTROY_PROCESS_DEATH)) {
#ifdef SELECTABLE_VIEWS
	if (owin->seln_client != NULL) {
	    seln_destroy(owin->seln_client);
	    owin->seln_client = NULL;
	}
#endif
	/* unlink layout procs */
	xv_set(owin_public, WIN_LAYOUT_PROC, owin->layout_proc, NULL);
	openwin_destroy_views(owin);

	if (destroy_status == DESTROY_CLEANUP)
	    free((char *) owin);
    }
    return XV_OK;
}

/*
 * openwin_layout - postion the views of the openwin
 */
static int
openwin_layout(owin_public, child, op, d1, d2, d3, d4, d5)
    Openwin         owin_public;
    Xv_Window       child;
    Window_layout_op op;
/* Alpha compatibility, mbuck@debian.org, FIXME: I don't understand this */
#if defined(__alpha)
    unsigned long   d1, d2, d3, d4, d5;
#else
    int             d1, d2, d3, d4, d5;
#endif
{
    Xv_openwin_info *owin = OPENWIN_PRIVATE(owin_public);
    Openwin_view_info *view;
    Scrollbar_setting direction;
    int             last;
    Rect            r;


    switch (op) {
      case WIN_CREATE:
	/* Determine if child is a scrollbar. */
	if (xv_get(child, XV_IS_SUBTYPE_OF, SCROLLBAR)) {
	    direction = (Scrollbar_setting) xv_get(child, SCROLLBAR_DIRECTION);
	    xv_set(owin_public, direction == SCROLLBAR_VERTICAL ?
		   WIN_VERTICAL_SCROLLBAR : WIN_HORIZONTAL_SCROLLBAR,
		   child,
		   NULL);
	}
	break;

      case WIN_DESTROY:
	if (openwin_viewdata_for_view(child, &view) == XV_OK) {
	    void            (*destroy_proc) ();

	    destroy_proc = owin->split_destroy_proc;
	    openwin_remove_split(owin, view);
	    (void) openwin_fill_view_gap(owin, view);
	    xv_free(view);
	    if (destroy_proc) {
		destroy_proc(owin_public);
	    }
	} else if (!STATUS(owin, removing_scrollbars)) {
	    /* must look through data structures since can't */
	    /* do a get on the sb to get information */
	    if (openwin_viewdata_for_sb(owin, child, &view, &direction, &last) == XV_OK) {
		openwin_set_sb(view, direction, XV_ZERO);
		/* only re-adjust if last view with sb */
		if (last) {
		    if (direction == SCROLLBAR_VERTICAL) {
			STATUS_RESET(owin, adjust_vertical);
		    } else {
			STATUS_RESET(owin, adjust_horizontal);
		    }
		    r = *(Rect *) xv_get(OPENWIN_PUBLIC(owin), WIN_RECT);
		    openwin_adjust_views(owin, &r);
		}
	    }
	}
	break;
      default:
	break;
    }

    if (owin->layout_proc != NULL) {
	return (owin->layout_proc(owin_public, child, op, d1, d2, d3, d4, d5));
    } else {
	return TRUE;
    }
}

#ifndef NO_OPENWIN_PAINT_BG
Pkg_private void
openwin_set_bg_color(owin_public)
Openwin owin_public;
{
	Xv_openwin_info  *owin = OPENWIN_PRIVATE(owin_public);
	Xv_Drawable_info *info;

	DRAWABLE_INFO_MACRO(owin_public, info);
	if (XAllocColor(xv_display(info), 
			(Colormap)xv_get(xv_cms(info), XV_XID, NULL),
			&(owin->background)) == 1) {
		XSetWindowBackground(xv_display(info), xv_xid(info), 
				     owin->background.pixel);
		XClearWindow(xv_display(info), xv_xid(info));
	}
}
#endif /* NO_OPENWIN_PAINT_BG */
