#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)ow_view.c 1.43 91/04/24";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Module:	ow_view.c Product:	SunView 2.0
 * 
 * Description:
 * 
 * manages creating and destroyomg views
 * 
 */


/*
 * Include files:
 */

#include <stdio.h>
#include <xview_private/ow_impl.h>
#include <xview/cms.h>

Attr_attribute  openwin_view_context_key;

/*
 * Declaration of Functions Defined in This File (in order):
 */

void            openwin_create_initial_view();
void            openwin_destroy_views();
int             openwin_count_views();
Openwin_view_info *openwin_nth_view();
int             openwin_viewdata_for_view();
int             openwin_viewdata_fo_sb();
void            openwin_split_view();
int             openwin_fill_view_gap();
void            openwin_copy_scrollbar();
void            openwin_remove_split();

static          openwin_init_view();
static          openwin_free_view();
static          openwin_remove_scrollbars();
static          openwin_create_viewwindow();
static          openwin_test_for_sb();
static          openwin_link_view();
static int      openwin_unlink_view();
static          openwin_locate_right_viewers();
static          openwin_locate_left_viewers();
static          openwin_locate_bottom_viewers();
static          openwin_locate_top_viewers();
static          openwin_expand_viewers();
static          openwin_register_initial_sb();
static int openwin_check_view(Openwin_view_info *view);


/******************************************************************/

Pkg_private void
openwin_create_initial_view(owin)
    Xv_openwin_info *owin;
{
    Rect            r;
    Openwin_view_info *new_view;

    r = *(Rect *) xv_get(OPENWIN_PUBLIC(owin), WIN_RECT);
    r.r_left = r.r_top = 0;

    openwin_init_view(owin, NULL, OPENWIN_SPLIT_VERTICAL, &r, &new_view);

    /* add scrollbars if we have seen them */
    if (owin->vsb_on_create) {
	openwin_register_initial_sb(owin, new_view, owin->vsb_on_create, SCROLLBAR_VERTICAL);
	owin->vsb_on_create = XV_ZERO;
    }
    if (owin->hsb_on_create) {
	openwin_register_initial_sb(owin, new_view, owin->hsb_on_create, SCROLLBAR_HORIZONTAL);
	owin->hsb_on_create = XV_ZERO;
    }
}

Pkg_private void
openwin_destroy_views(owin)
    Xv_openwin_info *owin;
{
    Openwin_view_info *view, *next_view;

    /* set that sb's being removed so remove code in layout */
    /* won't get called */
    STATUS_SET(owin, removing_scrollbars);

    for (view = owin->views; view != NULL; view = next_view) {
	next_view = view->next_view;
	openwin_free_view(view);
    }
}

/* Linux: openwin_check_views() has turned static in 3.2, but is not
 * called from anywhere. openwin_check_view() is made redundant at the same
 * time. Is this a xview bug or can these routines be removed altogether??
 * Change static to Pkg_private on linux, for now. */
#ifndef __linux
static int
#else
Pkg_private int
#endif
openwin_check_views(owin)
    Xv_openwin_info *owin;
{
    int             ret;
    Openwin_view_info *view;

    for (view = owin->views; view != NULL; view = view->next_view) {
	if ((ret = openwin_check_view(view)) != XV_OK) {
	    return (ret);
	}
    }
    return (XV_OK);
}

Pkg_private int
openwin_count_views(owin)
    Xv_openwin_info *owin;
{
    int             i = 0;
    Openwin_view_info *view = owin->views;

    while (view != NULL) {
	i++;
	view = view->next_view;
    }
    return (i);
}

Pkg_private Openwin_view_info *
openwin_nth_view(owin, place)
    Xv_openwin_info *owin;
{
    int             i = 0;
    Openwin_view_info *view = owin->views;

    for (i = 0; i < place; i++) {
	view = view->next_view;
	if (view == NULL) {
	    return (NULL);
	}
    }
    return (view);
}

Pkg_private int
openwin_viewdata_for_view(window, view)
    Xv_Window       window;
    Openwin_view_info **view;
{
    *view = NULL;
    *view = (Openwin_view_info *) xv_get(window, XV_KEY_DATA, openwin_view_context_key);
    if (*view != NULL) {
	return (XV_OK);
    } else {
	return (XV_ERROR);
    }
}

Pkg_private int
openwin_viewdata_for_sb(owin, sb, view, sb_direction, last_sb)
    Xv_openwin_info *owin;
    Scrollbar       sb;
    Openwin_view_info **view;
    Scrollbar_setting *sb_direction;
    int            *last_sb;
{

    /* look vertical first */
    *last_sb = TRUE;
    *sb_direction = SCROLLBAR_VERTICAL;
    *view = NULL;
    openwin_test_for_sb(owin, sb, *sb_direction, view, last_sb);

    if (*view != NULL) {
	/* found it */
	return (XV_OK);
    }
    *last_sb = TRUE;
    *sb_direction = SCROLLBAR_HORIZONTAL;
    *view = NULL;

    return (openwin_test_for_sb(owin, sb, *sb_direction, view, last_sb));
}

static
openwin_test_for_sb(owin, sb, sb_direction, view, last_sb)
    Xv_openwin_info *owin;
    Scrollbar       sb;
    Scrollbar_setting sb_direction;
    Openwin_view_info **view;
    int            *last_sb;
{
    Scrollbar       test_sb;
    Openwin_view_info *test_view;


    for (test_view = owin->views; test_view != NULL; test_view = test_view->next_view) {
	test_sb = openwin_sb(test_view, sb_direction);
	if (test_sb == sb) {
	    *view = test_view;
	} else if (test_sb != XV_ZERO) {
	    *last_sb = FALSE;
	}
    }

    if (*view != NULL) {
	return (XV_OK);
    } else {
	return (XV_ERROR);
    }
}

Pkg_private void
openwin_split_view(owin, view, direction, pos, view_start)
    Xv_openwin_info *owin;
    Openwin_view_info *view;
    Openwin_split_direction direction;
    int             pos;
    int             view_start;
{
    Openwin_view_info *new_view;
    Rect            r, new_r;
    Scrollbar       sb;

    /* compute the new rects for both the new and old views 
     * placing the new view to the left and above the old 
     */
    r = new_r = view->enclosing_rect;
    if (direction == OPENWIN_SPLIT_VERTICAL) {
	r.r_width = pos;
	new_r.r_left += pos;
	new_r.r_width -= pos;
    } else {
	r.r_height = pos;
	new_r.r_top += pos;
	new_r.r_height -= pos;
    }

    /* create new view */
    /* this automatically adjusts the view if needed */
    openwin_init_view(owin, view, direction, &new_r, &new_view);

    /* adjust active views rect */
    openwin_adjust_view(owin, view, &r);

    /* add needed scrollbars */
    if ((sb = openwin_sb(view, SCROLLBAR_VERTICAL)) != XV_ZERO) {
	openwin_copy_scrollbar(owin, sb, new_view);
	if (direction == OPENWIN_SPLIT_HORIZONTAL) {
	    sb = openwin_sb(new_view, SCROLLBAR_VERTICAL);
	    xv_set(sb, SCROLLBAR_VIEW_START, 
		   view_start / (int) xv_get(sb, SCROLLBAR_PIXELS_PER_UNIT), NULL);
	}
    }
    if ((sb = openwin_sb(view, SCROLLBAR_HORIZONTAL)) != XV_ZERO) {
	openwin_copy_scrollbar(owin, sb, new_view);
	if (direction == OPENWIN_SPLIT_VERTICAL) {
	    sb = openwin_sb(new_view, SCROLLBAR_HORIZONTAL);
	    xv_set(sb, SCROLLBAR_VIEW_START, 
		   view_start / (int) xv_get(sb, SCROLLBAR_PIXELS_PER_UNIT), NULL);
	}
    }
#ifdef SELECTABLE_VIEWS
    /* paint borders if needed */
    if (STATUS(owin, show_borders))
	openwin_paint_border(OPENWIN_PUBLIC(owin), new_view, TRUE);
#endif /* SELECTABLE_VIEWS */

    if (owin->split_init_proc) {
	(owin->split_init_proc) (view->view, new_view->view, pos);
    }
}

Pkg_private int
openwin_fill_view_gap(owin, view)
    Xv_openwin_info *owin;
    Openwin_view_info *view;
{
    Openwin_view_info *bounding_viewers[50];
    Rect            r;

    /* four cases -- look right, left, bottom, top */

    r = view->enclosing_rect;
    if (openwin_locate_right_viewers(owin->views, &r, bounding_viewers)) {
	/* these windows grow horizontally */
	openwin_expand_viewers(owin, view, bounding_viewers, &r, OPENWIN_SPLIT_HORIZONTAL);
    } else if (openwin_locate_left_viewers(owin->views, &r, bounding_viewers)) {
	/* these windows grow horizontally */
	openwin_expand_viewers(owin, view, bounding_viewers, &r, OPENWIN_SPLIT_HORIZONTAL);
    } else if (openwin_locate_bottom_viewers(owin->views, &r, bounding_viewers)) {
	/* these windows grow vertically */
	openwin_expand_viewers(owin, view, bounding_viewers, &r, OPENWIN_SPLIT_VERTICAL);
    } else if (openwin_locate_top_viewers(owin->views, &r, bounding_viewers)) {
	/* these windows grow vertically */
	openwin_expand_viewers(owin, view, bounding_viewers, &r, OPENWIN_SPLIT_VERTICAL);
    } else {
	return (OPENWIN_CANNOT_EXPAND);
    }

    return (XV_OK);
}

Pkg_private void
openwin_copy_scrollbar(owin, sb, to_view)
    Xv_openwin_info *owin;
    Scrollbar       sb;
    Openwin_view_info *to_view;
{
    int             view_length;
    Scrollbar_setting direction = (Scrollbar_setting) xv_get(sb, SCROLLBAR_DIRECTION);
    Rect            sb_r, r;
    Scrollbar       copy_sb;
    int             pixs_per_unit;

    r = *(Rect *) xv_get(to_view->view, WIN_RECT);
    openwin_place_scrollbar(OPENWIN_PUBLIC(owin), to_view->view,
			    sb, direction, &r, &sb_r);

    pixs_per_unit = (int) xv_get(sb, SCROLLBAR_PIXELS_PER_UNIT);
    view_length = (direction == SCROLLBAR_VERTICAL) ? 
      (int)xv_get(to_view->view, WIN_HEIGHT) : (int)xv_get(to_view->view, WIN_WIDTH);
    view_length = view_length / pixs_per_unit;

    copy_sb = xv_create(OPENWIN_PUBLIC(owin), SCROLLBAR,
        SCROLLBAR_DIRECTION, direction,
        SCROLLBAR_PIXELS_PER_UNIT, pixs_per_unit,
	SCROLLBAR_OBJECT_LENGTH, xv_get(sb, SCROLLBAR_OBJECT_LENGTH),
	SCROLLBAR_VIEW_START, xv_get(sb, SCROLLBAR_VIEW_START),
	SCROLLBAR_VIEW_LENGTH, view_length,
	SCROLLBAR_PAGE_LENGTH, xv_get(sb, SCROLLBAR_PAGE_LENGTH),
	SCROLLBAR_NORMALIZE_PROC, xv_get(sb, SCROLLBAR_NORMALIZE_PROC),
	SCROLLBAR_NOTIFY_CLIENT, to_view->view,
	SCROLLBAR_SPLITTABLE, xv_get(sb, SCROLLBAR_SPLITTABLE),
        SCROLLBAR_COMPUTE_SCROLL_PROC, xv_get(sb, SCROLLBAR_COMPUTE_SCROLL_PROC),
	WIN_RECT, &sb_r,
        XV_VISUAL, xv_get(sb, XV_VISUAL),			
        WIN_CMS, xv_get(sb, WIN_CMS),			
	XV_SHOW, TRUE,
	NULL);

    openwin_set_sb(to_view, direction, copy_sb);
}

Pkg_private void
openwin_remove_split(owin, view)
    Xv_openwin_info *owin;
    Openwin_view_info *view;
{
    openwin_unlink_view(owin, view);
    openwin_remove_scrollbars(view);
}

static
openwin_init_view(owin, twin, direction, r, new_view)
    Xv_openwin_info *owin;
    Openwin_view_info *twin;
    Openwin_split_direction direction;
    Rect           *r;
    Openwin_view_info **new_view;
{
    Openwin_view_info *view;

    *new_view = NULL;

    /* allocate the view */
    view = xv_alloc(Openwin_view_info);
    view->owin = owin;

    view->enclosing_rect = *r;

    if (twin == NULL) {
	/* default view -- on create -- add scrollbars */
	if (owin->vsb_on_create) {
	    openwin_set_sb(view, SCROLLBAR_VERTICAL, owin->vsb_on_create);
	}
	if (owin->hsb_on_create) {
	    openwin_set_sb(view, SCROLLBAR_HORIZONTAL, owin->hsb_on_create);
	}
	view->right_edge = view->bottom_edge = TRUE;
    } else {    
	if (direction == OPENWIN_SPLIT_VERTICAL) {
	    view->right_edge = twin->right_edge;
	    twin->right_edge = FALSE;
	    view->bottom_edge = twin->bottom_edge;
	} else {
	    view->bottom_edge = twin->bottom_edge;
	    twin->bottom_edge = FALSE;
	    view->right_edge = twin->right_edge;
	}
    } 

    /* use old view so get border and sb info */
    openwin_view_rect_from_avail_rect(owin, view, r);

    /* create the view window */
    openwin_create_viewwindow(owin, twin, view, r);
    openwin_link_view(owin, view);

    *new_view = view;
}


static
openwin_free_view(view)
    Openwin_view_info *view;
{
    openwin_remove_scrollbars(view);
    xv_destroy_status(view->view, DESTROY_CLEANUP);

    free((char *) view);
}

static
openwin_remove_scrollbars(view)
    Openwin_view_info *view;
{
    Scrollbar       vsb, hsb;

    vsb = openwin_sb(view, SCROLLBAR_VERTICAL);
    hsb = openwin_sb(view, SCROLLBAR_HORIZONTAL);

    if (vsb != XV_ZERO) {
	xv_destroy_status(vsb, DESTROY_CLEANUP);
    }
    if (hsb != XV_ZERO) {
	xv_destroy_status(hsb, DESTROY_CLEANUP);
    }
}

static
openwin_create_viewwindow(owin, from_view, view, r)
    Xv_openwin_info *owin;
    Openwin_view_info *from_view, *view;
    Rect           *r;
{
    int             xborders;
    Visual	   *visual;
    Cms		    cms;

    if (from_view != NULL) {
	xborders = (int)xv_get(from_view->view, WIN_BORDER);
	visual = (Visual *)xv_get(from_view->view, XV_VISUAL);
	cms = (Cms)xv_get(from_view->view, WIN_CMS);
    } else {
	xborders = STATUS(owin, show_borders) ? TRUE : FALSE;
	visual = (Visual *)xv_get(OPENWIN_PUBLIC(owin), XV_VISUAL);
	cms = (Cms)xv_get(OPENWIN_PUBLIC(owin), WIN_CMS);
    }

    /* create context key so can hang data off objects */
    if (openwin_view_context_key == (Attr_attribute) 0) {
	openwin_view_context_key = xv_unique_key();
    }
    if (owin->view_avlist == NULL) {
	/* parent/child registration process sets the data handle */
	view->view = 
	  (Xv_Window)xv_create(OPENWIN_PUBLIC(owin), owin->view_class,
              WIN_NOTIFY_SAFE_EVENT_PROC, openwin_view_event,
	      WIN_NOTIFY_IMMEDIATE_EVENT_PROC, openwin_view_event,
	      WIN_RECT, r,
	      WIN_BORDER, xborders,
	      XV_VISUAL, visual,
              WIN_CMS, cms,			       
              XV_KEY_DATA, openwin_view_context_key, view,
	      NULL);
    } else {
	/* parent/child registration process sets the data handle */
	view->view = 
	  (Xv_Window)xv_create(OPENWIN_PUBLIC(owin), owin->view_class,
              ATTR_LIST, owin->view_avlist,
              WIN_NOTIFY_SAFE_EVENT_PROC, openwin_view_event,
	      WIN_NOTIFY_IMMEDIATE_EVENT_PROC, openwin_view_event,
	      WIN_RECT, r,
	      WIN_BORDER, xborders,
	      XV_VISUAL, visual,
              WIN_CMS, cms,			       
	      XV_KEY_DATA, openwin_view_context_key, view,
	      NULL);

	/* if client toggled xborders redo rect placement */
	if ((int) xv_get(view->view, WIN_BORDER) != xborders) {
	    *r = view->enclosing_rect;
	    openwin_view_rect_from_avail_rect(owin, view, r);
	    if (!rect_equal(&view->enclosing_rect, r)) {
		xv_set(view->view, WIN_RECT, r, NULL);
	    }
	    /* no xborders is the default so if xborders is TRUE */
	    /* and we each here assume client defaulted */
	    /* WIN_BORDER to mean inherit from splittee. */
	    /* Therefore we do a set to turn the borders on */
	    /* we couldn't do this in the xv_create call */
	    /* because ATTR_LIST must appear first in the avlist */
	    /* This means a split can't override a border true */
	    /* on create. No big deal since it is rare. */
	    if (xborders) {
		xv_set(view->view, WIN_BORDER, xborders, NULL);
	    }
	}
	xv_free(owin->view_avlist);
	owin->view_avlist = NULL;
    }
}

static
openwin_link_view(owin, view)
    Xv_openwin_info *owin;
    Openwin_view_info *view;
{
    Openwin_view_info *t_view;

    if (owin->views == NULL) {
	owin->views = view;
    } else {
	for (t_view = owin->views; t_view->next_view != NULL; t_view = t_view->next_view);
	t_view->next_view = view;
    }
}

static int
openwin_unlink_view(owin, view)
    Xv_openwin_info *owin;
    Openwin_view_info *view;
{
    Openwin_view_info *t_view;

    if (owin->views == view) {
	owin->views = view->next_view;
	return (XV_OK);
    } else {
	for (t_view = owin->views; t_view->next_view != NULL; t_view = t_view->next_view) {
	    if (t_view->next_view == view) {
		t_view->next_view = view->next_view;
		return (XV_OK);
	    }
	}
    }
    return (XV_ERROR);
}

static int
openwin_check_view(Openwin_view_info *view)
{
    int             ret;
    Scrollbar       sb;

    ret = xv_destroy_status(view->view, DESTROY_CHECKING);
    if (ret != XV_OK)
	return (ret);

    if ((sb = openwin_sb(view, SCROLLBAR_VERTICAL)) != XV_ZERO) {
	ret = xv_destroy_status(sb, DESTROY_CHECKING);
	if (ret != XV_OK)
	    return (ret);
    }
    if ((sb = openwin_sb(view, SCROLLBAR_HORIZONTAL)) != XV_ZERO) {
	ret = xv_destroy_status(sb, DESTROY_CHECKING);
	if (ret != XV_OK)
	    return (ret);
    }
    return (ret);
}


static int
openwin_locate_right_viewers(views, r, bounders)
    Openwin_view_info *views;
    Rect           *r;
    Openwin_view_info *bounders[];
{
    Openwin_view_info *view;
    Rect            view_r;
    int             num_bound = 0;
    int             found_min = FALSE, found_max = FALSE;

    for (view = views; view != NULL; view = view->next_view) {
	view_r = view->enclosing_rect;

	if ((r->r_left + r->r_width) == view_r.r_left) {
	    /* if views starting point and ending point is */
	    /* between the gap add to list */
	    if (view_r.r_top >= r->r_top) {
		if (view_r.r_top + view_r.r_height <= r->r_top + r->r_height) {
		    bounders[num_bound++] = view;
		} else {
		    /* view extends beyond end of gap */
		    return (FALSE);
		}
	    }
	    if (view_r.r_top == r->r_top) {
		found_min = TRUE;
	    }
	    if (view_r.r_top + view_r.r_height == r->r_top + r->r_height) {
		found_max = TRUE;
	    }
	}
    }

    if (num_bound > 0) {
	bounders[num_bound] = NULL;
    }
    return (found_min && found_max);
}

static int
openwin_locate_left_viewers(views, r, bounders)
    Openwin_view_info *views;
    Rect           *r;
    Openwin_view_info *bounders[];
{
    Openwin_view_info *view;
    Rect            view_r;
    int             num_bound = 0;
    int             found_min = FALSE, found_max = FALSE;

    /* look below/to the right first */
    for (view = views; view != NULL; view = view->next_view) {
	view_r = view->enclosing_rect;

	if (r->r_left == (view_r.r_left + view_r.r_width)) {
	    /* see if view fits in the gap */
	    if (view_r.r_top >= r->r_top) {
		if (view_r.r_top + view_r.r_height <= r->r_top + r->r_height) {
		    bounders[num_bound++] = view;
		} else {
		    /* view extend beyond gap bounds */
		    return (FALSE);
		}
	    }
	    if (view_r.r_top == r->r_top) {
		found_min = TRUE;
	    }
	    if (view_r.r_top + view_r.r_height == r->r_top + r->r_height) {
		found_max = TRUE;
	    }
	}
    }

    if (num_bound > 0) {
	bounders[num_bound] = NULL;
    }
    return (found_min && found_max);
}

static int
openwin_locate_bottom_viewers(views, r, bounders)
    Openwin_view_info *views;
    Rect           *r;
    Openwin_view_info *bounders[];
{
    Openwin_view_info *view;
    Rect            view_r;
    int             num_bound = 0;
    int             found_min = FALSE, found_max = FALSE;

    /* look below/to the right first */
    for (view = views; view != NULL; view = view->next_view) {
	view_r = view->enclosing_rect;

	if ((r->r_top + r->r_height) == view_r.r_top) {
	    /* see if view fits in the gap */
	    if (view_r.r_left >= r->r_left) {
		if (view_r.r_left + view_r.r_width <= r->r_left + r->r_width) {
		    bounders[num_bound++] = view;
		} else {
		    /* view extend beyond gap bounds */
		    return (FALSE);
		}
	    }
	    if (view_r.r_left == r->r_left) {
		found_min = TRUE;
	    }
	    if (view_r.r_left + view_r.r_width == r->r_left + r->r_width) {
		found_max = TRUE;
	    }
	}
    }

    if (num_bound > 0) {
	bounders[num_bound] = NULL;
    }
    return (found_min && found_max);
}

static int
openwin_locate_top_viewers(views, r, bounders)
    Openwin_view_info *views;
    Rect           *r;
    Openwin_view_info *bounders[];
{
    Openwin_view_info *view;
    Rect            view_r;
    int             num_bound = 0;
    int             found_min = FALSE, found_max = FALSE;

    /* look below/to the right first */
    for (view = views; view != NULL; view = view->next_view) {
	view_r = view->enclosing_rect;

	if (r->r_top == (view_r.r_top + view_r.r_height)) {
	    /* see if view fits in the gap */
	    if (view_r.r_left >= r->r_left) {
		if (view_r.r_left + view_r.r_width <= r->r_left + r->r_width) {
		    bounders[num_bound++] = view;
		} else {
		    /* view extend beyond gap bounds */
		    return (FALSE);
		}
	    }
	    if (view_r.r_left == r->r_left) {
		found_min = TRUE;
	    }
	    if (view_r.r_left + view_r.r_width == r->r_left + r->r_width) {
		found_max = TRUE;
	    }
	}
    }

    if (num_bound > 0) {
	bounders[num_bound] = NULL;
    }
    return (found_min && found_max);
}

static
openwin_expand_viewers(owin, old_view, viewers, r, direction)
    Xv_openwin_info *owin;
    Openwin_view_info *old_view;
    Openwin_view_info **viewers;
    Rect           *r;
    Openwin_split_direction direction;
{
    Rect            view_r;
    Openwin_view_info *view;

    for (view = *viewers; view != NULL; view = *(++viewers)) {
	view_r = view->enclosing_rect;
	if (direction == OPENWIN_SPLIT_VERTICAL) {
	    if (view_r.r_top > r->r_top) {
		view_r.r_top = r->r_top;
	    }
	    view_r.r_height += r->r_height;

	    if (old_view->bottom_edge) {
		view->bottom_edge = TRUE;
	    }
	} else {
	    if (view_r.r_left > r->r_left) {
		view_r.r_left = r->r_left;
	    }
	    view_r.r_width += r->r_width;

	    if (old_view->right_edge) {
		view->right_edge = TRUE;
	    }
	}
	openwin_adjust_view(owin, view, &view_r);
    }
}

static
openwin_register_initial_sb(owin, view, sb, direction)
    Xv_openwin_info *owin;
    Openwin_view_info *view;
    Scrollbar       sb;
    Scrollbar_setting direction;
{
    Rect            r, sb_r;
    long unsigned   view_length;

    r = *(Rect *) xv_get(view->view, WIN_RECT);

    openwin_place_scrollbar(OPENWIN_PUBLIC(owin), view->view, sb, direction,
			    &r, &sb_r);

    view_length = (direction == SCROLLBAR_VERTICAL) ? r.r_height : r.r_width;
    view_length = view_length / (int) xv_get(sb, SCROLLBAR_PIXELS_PER_UNIT);

    if (xv_get(sb, WIN_PARENT) != OPENWIN_PUBLIC(owin) ||
	xv_get(sb, XV_OWNER) != OPENWIN_PUBLIC(owin))
	xv_set(sb,
	       WIN_PARENT, OPENWIN_PUBLIC(owin),
	       XV_OWNER, OPENWIN_PUBLIC(owin),
	       NULL);
    xv_set(sb,
	   WIN_RECT, &sb_r,
	   SCROLLBAR_DIRECTION, direction,
	   SCROLLBAR_VIEW_LENGTH, view_length,
	   SCROLLBAR_NOTIFY_CLIENT, view->view,
	   XV_SHOW, TRUE,
	   NULL);
}
