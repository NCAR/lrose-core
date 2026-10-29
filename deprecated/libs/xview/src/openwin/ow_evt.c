#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)ow_evt.c 1.30 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Module:	ow_evt.c
 * 
 * Package:     openwin
 * 
 * Description: Handles events to both the openwin window, and the view window.
 * 
 */

#include <xview_private/ow_impl.h>

/*
 * Package private functions
 */
Pkg_private Notify_value openwin_event();
Pkg_private Notify_value openwin_view_event();

/*-------------------Function Definitions-------------------*/

/*
 * openwin_event - event handler for openwin
 */
Pkg_private Notify_value
openwin_event(owin_public, event, arg, type)
    Openwin         owin_public;
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    Xv_openwin_info *owin = OPENWIN_PRIVATE(owin_public);
    Rect            r;

    switch (event_action(event)) {
#ifdef SELECTABLE_VIEWS
      case ACTION_SELECT:
	if (event_is_down(event))
	    openwin_select(owin_public, event);
	break;
#endif /* SELECTABLE_VIEWS */

      case ACTION_RESCALE:
 	openwin_rescale(owin_public, (int) arg);
	break;

      case WIN_RESIZE:
	r = *(Rect *) xv_get(owin_public, WIN_RECT);
	openwin_adjust_views(owin, &r);
	owin->cached_rect = r;
	break;
      case WIN_REPAINT:
	/* Enable painting in openwin_paint_border */
	STATUS_SET(owin, mapped);
#ifdef SELECTABLE_VIEWS
	if (STATUS(owin, show_borders))
	    openwin_paint_borders(owin_public);
#endif /* SELECTABLE_VIEWS */
	break;
      default:
	break;
    }

    return notify_next_event_func(owin_public, (Notify_event) event, arg, type);
}

/*
 * openwin_view_event - event handler for openwin views
 */
Pkg_private Notify_value
openwin_view_event(window_public, event, arg, type)
    Xv_Window       window_public;
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{

    switch (event_action(event)) {
      case WIN_REPAINT:
	{
	    /* clear the damaged area */
	    Openwin_view_info *view = (Openwin_view_info *) xv_get(window_public,
				     XV_KEY_DATA, openwin_view_context_key);

	    if (STATUS(view->owin, auto_clear)) {
		openwin_clear_damage(window_public, win_get_damage(window_public));
	    }
	    break;
	}
      case ACTION_SPLIT_HORIZONTAL:
	{
	    Openwin_view_info *view = (Openwin_view_info *) xv_get(window_public,
				     XV_KEY_DATA, openwin_view_context_key);

	    xv_set(OPENWIN_PUBLIC(view->owin),
		   OPENWIN_SPLIT,
		   OPENWIN_SPLIT_VIEW, view->view,
		   OPENWIN_SPLIT_DIRECTION, OPENWIN_SPLIT_HORIZONTAL,
		   OPENWIN_SPLIT_POSITION, (int) arg,
		   0,
		   NULL);
	    break;
	}
      case ACTION_SPLIT_VERTICAL:
	{
	    Openwin_view_info *view = (Openwin_view_info *) xv_get(window_public,
				     XV_KEY_DATA, openwin_view_context_key);

	    xv_set(OPENWIN_PUBLIC(view->owin),
		   OPENWIN_SPLIT,
		   OPENWIN_SPLIT_VIEW, view->view,
		   OPENWIN_SPLIT_DIRECTION, OPENWIN_SPLIT_VERTICAL,
		   OPENWIN_SPLIT_POSITION, (int) arg,
		   0,
		   NULL);
	    break;
	}
      case ACTION_SPLIT_DESTROY:
	{
	    Openwin_view_info *view = (Openwin_view_info *) xv_get(window_public,
				     XV_KEY_DATA, openwin_view_context_key);

#ifdef SELECTABLE_VIEWS
	    if (view->owin->seln_view != NULL)
	      openwin_select_view(OPENWIN_PUBLIC(view->owin), NULL);
#endif /* SELECTABLE_VIEWS */
	    if (openwin_count_views(view->owin) > 1)
		xv_destroy_safe(window_public);
	    return (NOTIFY_DONE);
	}
      default:
	break;
    }
    return notify_next_event_func(window_public, (Notify_event) event, arg, type);
}
