#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)ow_seln.c 1.15 93/06/28";
#endif
#endif

/*
 * Package:     openwin
 *
 * Module:      ow_seln.c
 *
 * Description: Handle selection of openwin views
 */

/*
 * NOTE: until selectable views is implemented, this file really doesn't
 *       need to be compiled.
 */

#ifdef SELECTABLE_VIEWS

#include <xview_private/ow_impl.h>
#include <xview/sun.h>
#include <xview/sel_svc.h>
#include <xview/sel_attrs.h>

/*
 * Package private functions
 */
Pkg_private void      openwin_seln_function();
Pkg_private Xv_opaque openwin_seln_reply();
Pkg_private void      openwin_select();
Pkg_private void      openwin_select_view();

/*-------------------Function Definitions-------------------*/

/* Callback procedure for function key changes (as if I care) */
Pkg_private void
openwin_seln_function(client_data, function)
    char           *client_data;
    Seln_function_buffer *function;
{
}


/* Callback procedure to answer requests from selection service */
Pkg_private Xv_opaque
openwin_seln_reply(item, context, length)
    caddr_t         item;
    Seln_replier_data *context;
    int             length;
{
    Xv_openwin_info *owin = (Xv_openwin_info *) context->client_data;

    switch ((int) item) {
      case SELN_REQ_YIELD:
	if (owin->seln_view != NULL)
	    openwin_select_view(OPENWIN_PUBLIC(owin), NULL);
	*context->response_pointer++ = (char *) SELN_SUCCESS;
	return ((Xv_opaque) SELN_SUCCESS);
      case SELN_REQ_END_REQUEST:
	return ((Xv_opaque) SELN_SUCCESS);
      default:
	return ((Xv_opaque) SELN_UNRECOGNIZED);
    }
}


/* Set the selection for the appropriate view */
Pkg_private void
openwin_select(owin_public, event)
    Openwin         owin_public;
    Event          *event;
{
    Openwin_view_info *view;
    int             x = event_x(event);
    int             y = event_y(event);
    Xv_openwin_info *owin = OPENWIN_PRIVATE(owin_public);


    for (view = owin->views; view != NULL; view = view->next_view) {
	if (rect_includespoint(&view->enclosing_rect, x, y)) {
	    openwin_select_view(owin_public, view);
	    break;
	}
    }
}

/* Select a particular view; if view is NULL unselect any selected view */
Pkg_private void
openwin_select_view(owin_public, view)
    Openwin         owin_public;
    Openwin_view_info *view;
{
    Xv_openwin_info *owin = OPENWIN_PRIVATE(owin_public);


    if (owin->seln_view == view)
	return;

    if (owin->seln_view == NULL) {	/* owin needs to acquire the
					 * selection */
	seln_acquire(owin->seln_client, SELN_PRIMARY);
    } else {			/* one of owin's views already has it */
	openwin_lolite_view(owin->public_self, owin->seln_view);
    }

    if (view == NULL) {		/* yield the selection */
	owin->seln_view = NULL;
	(void) selection_done(xv_default_server, owin->seln_client, SELN_PRIMARY);
    } else {			/* remember who has the selection */
	owin->seln_view = view;
	openwin_hilite_view(owin_public, view);
    }
}

#endif SELECTABLE_VIEWS
