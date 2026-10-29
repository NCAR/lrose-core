#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)ow_get.c 1.25 90/06/21";
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
 * Module:      ow_get.c
 *
 * Description: Return values for openwin attributes
 * 
 */

#include <xview_private/ow_impl.h>
#include <xview_private/draw_impl.h>
#include <xview_private/portable.h>

/*
 * Package Private Functions
 */
Pkg_private Xv_opaque openwin_get();

/*-------------------Function Definitions-------------------*/

/*
 * openwin_get - return value for given attribute(s)
 */
Pkg_private Xv_opaque
openwin_get(owin_public, get_status, attr, valist)
    Openwin         owin_public;
    int            *get_status;
    Openwin_attribute attr;
    va_list         valist;
{
    Xv_openwin_info *owin = OPENWIN_PRIVATE(owin_public);
    Openwin_view_info *view;
    Xv_opaque v = 0;

    switch (attr) {
      case OPENWIN_NTH_VIEW:
	view = openwin_nth_view(owin, va_arg(valist, int));
	if (view != NULL) {
	    v = (Xv_opaque) view->view;
	} else {
	    v = (Xv_opaque) NULL;
	}
	break;
      case OPENWIN_SHOW_BORDERS:
	v = (Xv_opaque) STATUS(owin, show_borders);
	break;
      case WIN_VERTICAL_SCROLLBAR:
	view = openwin_nth_view(owin, 0);
	if (view == NULL)
	  v = (Xv_opaque) NULL;
	else
	  v = (Xv_opaque) openwin_sb(view, SCROLLBAR_VERTICAL);
	break;
      case OPENWIN_NVIEWS:
	v = (Xv_opaque) openwin_count_views(owin);
	break;
      case OPENWIN_VERTICAL_SCROLLBAR:
	view = (Openwin_view_info *) xv_get(va_arg(valist, Xv_Window),
					    XV_KEY_DATA, openwin_view_context_key);
	if ((view == NULL) && ((view = openwin_nth_view(owin, 0)) == NULL))
	  v = (Xv_opaque) NULL;
	else 
	  v = (Xv_opaque) openwin_sb(view, SCROLLBAR_VERTICAL);
	break;
      case OPENWIN_HORIZONTAL_SCROLLBAR:
	view = (Openwin_view_info *) xv_get(va_arg(valist, Xv_Window), 
					    XV_KEY_DATA, openwin_view_context_key);
	if ((view == NULL) && ((view = openwin_nth_view(owin, 0)) == NULL))
	  v = (Xv_opaque) NULL;
	else
	  v = (Xv_opaque) openwin_sb(view, SCROLLBAR_HORIZONTAL);
	break;
      case OPENWIN_AUTO_CLEAR:
	v = (Xv_opaque) STATUS(owin, auto_clear);
	break;
      case WIN_HORIZONTAL_SCROLLBAR:
	view = openwin_nth_view(owin, 0);
	if (view == NULL)
	  v = (Xv_opaque) NULL;
	else 
	  v = (Xv_opaque) openwin_sb(view, SCROLLBAR_HORIZONTAL);
	break;
      case OPENWIN_ADJUST_FOR_VERTICAL_SCROLLBAR:
	v = (Xv_opaque) STATUS(owin, adjust_vertical);
	break;
      case OPENWIN_ADJUST_FOR_HORIZONTAL_SCROLLBAR:
	v = (Xv_opaque) STATUS(owin, adjust_horizontal);
	break;
      case OPENWIN_VIEW_CLASS:
	v = (Xv_opaque) owin->view_class;
	break;
      case OPENWIN_NO_MARGIN:
	v = (Xv_opaque) STATUS(owin, no_margin);
	break;
      case OPENWIN_SELECTED_VIEW:
#ifdef SELECTABLE_VIEWS
	v = (Xv_opaque) (owin->seln_view ? owin->seln_view->view : NULL);
	break;
#else 
	v = (Xv_opaque) NULL;
	break;
#endif /* SELECTABLE_VIEWS */
      case OPENWIN_SPLIT_INIT_PROC:
	v = (Xv_opaque) (owin->split_init_proc);
	break;
      case OPENWIN_SPLIT_DESTROY_PROC:
	v = (Xv_opaque) (owin->split_destroy_proc);
	break;
      default:
	xv_check_bad_attr(OPENWIN, attr);
	*get_status = XV_ERROR;
    }
    return(v);
}
