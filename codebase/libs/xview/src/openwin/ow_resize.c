#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)ow_resize.c 1.40 93/06/28";
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
 * Module:	ow_resize.c
 * 
 * Description: adjusts sizes of views in accordance with a resize
 * 
 */

#include <xview_private/ow_impl.h>
#include <xview/font.h>

/*
 * Package private functions
 */
Pkg_private int	 openwin_adjust_views();
Pkg_private void openwin_adjust_view();
Pkg_private void openwin_place_scrollbar();
Pkg_private int  openwin_border_width();
Pkg_private void openwin_view_rect_from_avail_rect();

/*
 * Module private functions
 */
static void openwin_adjust_view_rect();
static void openwin_adjust_view_scrollbars();
static void openwin_adjust_view_by_margins();

/*-------------------Function Definitions-------------------*/

/*
 * openwin_adjust_views - resize all views of a openwin to fit the rect
 */
Pkg_private int
openwin_adjust_views(owin, owin_rect)
    Xv_openwin_info *owin;
    Rect           *owin_rect;
{
    Openwin_view_info *view = owin->views;
    Rect            r;
    int		    adjust_rect;
    
    /* find the views that are on the vertical edge */
    for (view = owin->views; view != NULL; view = view->next_view) {
	adjust_rect = FALSE;
	r = view->enclosing_rect;

	/* See if view is visable in the owin */
	if ((owin_rect->r_width > r.r_left) &&
	    (owin_rect->r_height > r.r_top)) {
	    
	    if (view->right_edge) {
		r.r_width = owin_rect->r_width - r.r_left;
		if (r.r_width <= 0) {
		    r.r_width = 1;
		}
		adjust_rect = TRUE;
	    }
	    
	    if (view->bottom_edge) {
		r.r_height = owin_rect->r_height - r.r_top;
		if (r.r_height <= 0)
		  r.r_height = 1;
		adjust_rect = TRUE;
	    }

	    if (adjust_rect)
	      openwin_adjust_view(owin, view, &r);
	}
    }
}

/* 
 * openwin_adjust_view - resize the view to fit the rect
 */
Pkg_private void
openwin_adjust_view(owin, view, view_rect)
    Xv_openwin_info *owin;
    Openwin_view_info *view;
    Rect           *view_rect;
{
    Rect            r, sb_r;
    Scrollbar       sb;

    r = view->enclosing_rect = *view_rect;

    openwin_view_rect_from_avail_rect(owin, view, &r);

    if (r.r_width <= 0) {
	r.r_width = view_rect->r_width;
    } else if (r.r_height <= 0) {
	r.r_height = view_rect->r_height;
    }
    /* place the scrollbars */
    if ((sb = openwin_sb(view, SCROLLBAR_VERTICAL)) != XV_ZERO) {
	openwin_place_scrollbar(OPENWIN_PUBLIC(owin), view->view,
		   openwin_sb(view, SCROLLBAR_VERTICAL), SCROLLBAR_VERTICAL,
				&r, &sb_r);
	xv_set(sb, WIN_RECT, &sb_r, NULL);
    }
    if ((sb = openwin_sb(view, SCROLLBAR_HORIZONTAL)) != XV_ZERO) {

	openwin_place_scrollbar(OPENWIN_PUBLIC(owin), view->view,
	       openwin_sb(view, SCROLLBAR_HORIZONTAL), SCROLLBAR_HORIZONTAL,
				&r, &sb_r);
	xv_set(sb, WIN_RECT, &sb_r, NULL);
    }
    /*
     * now place the view.  Must do this after placing the sb's because if
     * the sb's are moved after the view has been resized, they cause an
     * exposure to be sent to the view, causing a second repaint.
     */
    openwin_adjust_view_rect(owin, view, &r);
}

/*
 * openwin_place_scrollbar - position the scrollbar inside the openwin
 */
Pkg_private void
openwin_place_scrollbar(owin_public, view_public, sb, direction, r, sb_r)
    Xv_object	    owin_public;
    Scrollbar       sb;
    Xv_opaque       view_public;
    Scrollbar_setting direction;
    Rect           *r, *sb_r;
{
    Xv_openwin_info *owin = OPENWIN_PRIVATE(owin_public);    
    int             border_width;

    if (sb == XV_ZERO)
	return;
    border_width = openwin_border_width(owin_public, view_public);

    if (direction == SCROLLBAR_VERTICAL) {
	sb_r->r_width = scrollbar_width_for_scale(xv_get(xv_get(owin_public, XV_FONT),
							 FONT_SCALE));
	sb_r->r_height = r->r_height + (2 * border_width);
	sb_r->r_top = r->r_top;
	if (STATUS(owin, left_scrollbars))
	  sb_r->r_left = r->r_left - sb_r->r_width;
	else
	  sb_r->r_left = r->r_left + r->r_width + (2 * border_width);
    } else {
	sb_r->r_left = r->r_left;
	sb_r->r_top = r->r_top + r->r_height + (2 * border_width);
	sb_r->r_width = r->r_width + (2 * border_width);
	sb_r->r_height = scrollbar_width_for_scale(xv_get(xv_get(owin_public,
						         XV_FONT), FONT_SCALE));
    }
}

/*
 * openwin_border_width - return the border with in pixels of an openwin
 */
Pkg_private int
openwin_border_width(owin_public, view_public)
    Openwin         owin_public;
    Xv_opaque       view_public;
{
    /*
     * OPENWIN_SHOW_BORDERS & WIN_BORDER are the same now since borders are
     * always drawn using X window borders for performance reasons. However,
     * this might have to change when we implement border highlighting for
     * pane selection.
     */

    if (((int) xv_get(owin_public, OPENWIN_SHOW_BORDERS) == TRUE) ||
	(view_public && (int) xv_get(view_public, WIN_BORDER) == TRUE)) {
	return (WIN_DEFAULT_BORDER_WIDTH);
    } else {
	return (0);
    }
}

/*
 * openwin_view_rect_from_avail_rect - 
 */
Pkg_private void
openwin_view_rect_from_avail_rect(owin, view, r)
    Xv_openwin_info *owin;
    Openwin_view_info *view;
    Rect           *r;
{
    openwin_adjust_view_scrollbars(owin, view, r);
    openwin_adjust_view_by_margins(owin, view, owin->margin, r);
}

/*
 * openwin_adjust_view_rect - 
 */
static void
openwin_adjust_view_rect(owin, view, view_rect)
    Xv_openwin_info *owin;
    Openwin_view_info *view;
    Rect           *view_rect;
{
    Scrollbar       vsb, hsb;

    vsb = openwin_sb(view, SCROLLBAR_VERTICAL);
    hsb = openwin_sb(view, SCROLLBAR_HORIZONTAL);

#ifdef SELECTABLE_VIEWS
    /* clear borders if painted */
    if (STATUS(owin, show_borders))
	openwin_paint_border(OPENWIN_PUBLIC(owin), view, FALSE);
#endif /* SELECTABLE_VIEWS */
    xv_set(view->view, WIN_RECT, view_rect, NULL);
#ifdef SELECTABLE_VIEWS
    /* repaint borders is shown */
    if (STATUS(owin, show_borders))
	openwin_paint_border(OPENWIN_PUBLIC(owin), view, TRUE);
#endif /* SELECTABLE_VIEWS */

    if (vsb != XV_ZERO) {
	xv_set(vsb,
	       SCROLLBAR_VIEW_LENGTH, 
	       view_rect->r_height / (int) xv_get(vsb, SCROLLBAR_PIXELS_PER_UNIT),
	       NULL);
    }
    if (hsb != XV_ZERO) {
	xv_set(hsb,
	       SCROLLBAR_VIEW_LENGTH, 
	       view_rect->r_width / (int) xv_get(hsb, SCROLLBAR_PIXELS_PER_UNIT),
	       NULL);
    }
}

/*
 * openwin_adjust_view_scrollbars - 
 */
static void
openwin_adjust_view_scrollbars(owin, view, avail_rect)
    Xv_openwin_info *owin;
    Openwin_view_info *view;
    Rect           *avail_rect;
{
    int             vsb_w, hsb_h;

    /* no computation if not adjusted or already adjusted */
    if (!STATUS(owin, adjust_vertical) && !STATUS(owin, adjust_horizontal)) {
	return;
    }

    vsb_w = hsb_h = scrollbar_width_for_scale(
	xv_get(xv_get(OPENWIN_PUBLIC(owin), XV_FONT), FONT_SCALE));

    if (STATUS(owin, adjust_vertical) && vsb_w < avail_rect->r_width) {
	avail_rect->r_width -= vsb_w;
	if (STATUS(owin, left_scrollbars))
	    avail_rect->r_left += vsb_w;
    }
    if (STATUS(owin, adjust_horizontal) && hsb_h < avail_rect->r_height) {
	avail_rect->r_height -= hsb_h;
    }
}

/*
 * openwin_adjust_view_by_margins - 
 */
static void
openwin_adjust_view_by_margins(owin, view, margin, view_rect)
    Xv_openwin_info *owin;
    Openwin_view_info *view;
    int             margin;
    Rect           *view_rect;
{
    int             n_vmargins, n_hmargins;
    int             border_width = 0;

    /* set up margins */
    if (STATUS(owin, no_margin)) {
	n_vmargins = n_hmargins = 0;
    } else {
#ifndef SVR4
	n_vmargins = n_hmargins = 1;
#else /* SVR4 */
	n_vmargins = n_hmargins = 2;
#endif /* SVR4 */
    }

    /* get rid of margin if view is on one of the edges, or if there is 
     * a scrollbar 
     */
    if (view->right_edge ||
	(openwin_sb(view, SCROLLBAR_VERTICAL) != XV_ZERO) ||
	STATUS(owin, adjust_vertical)) 
      n_vmargins = 0;
    if (view->bottom_edge ||
	(openwin_sb(view, SCROLLBAR_HORIZONTAL) != XV_ZERO) ||
	STATUS(owin, adjust_horizontal))
      n_hmargins = 0;
    
    border_width = openwin_border_width(OPENWIN_PUBLIC(owin), view->view);

    view_rect->r_width -= n_vmargins * margin + 2 * border_width;
    view_rect->r_height -= n_hmargins * margin + 2 * border_width;
}
