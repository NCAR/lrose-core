#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sel_policy.c 20.20 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/seln_impl.h>
#include <xview/sel_svc.h>
#include <xview/sel_compat.h>
#include <xview/rect.h>
#include <xview/win_input.h>
#include <xview/server.h>

/*
 * sel_policy.c:	implement the user interface policy in the
 * selection_service
 */

static int      seln_non_null_primary();


/*
 * Figure how to respond to a function buffer from the server
 */
/*ARGSUSED*/
Xv_public       Seln_response
selection_figure_response(server, buffer, holder)
    Xv_Server       server;
    Seln_function_buffer *buffer;
    Seln_holder   **holder;
{
    Seln_holder    *me;
    char           *client;

    if (buffer->function == SELN_FN_ERROR) {
	return SELN_IGNORE;
    }
    switch (buffer->addressee_rank) {
      case SELN_CARET:
	me = &buffer->caret;
	break;
      case SELN_PRIMARY:
	me = &buffer->primary;
	break;
      case SELN_SECONDARY:
	me = &buffer->secondary;
	break;
      case SELN_SHELF:
	me = &buffer->shelf;
	break;
      default:
	complain("figure_response got a malformed buffer.");
	return SELN_IGNORE;
    }
    client = ((Seln_client_node *) (me->access.client))->client_data;
    switch (buffer->function) {
      case SELN_FN_GET:
	if (!seln_holder_same_client(&buffer->caret, client)) {
	    return SELN_IGNORE;
	}
	if (seln_secondary_made(buffer)) {
	    *holder = &buffer->secondary;
	} else {
	    *holder = &buffer->shelf;
	}
	if ((*holder)->state == SELN_NONE) {
	    return SELN_IGNORE;
	} else {
	    buffer->addressee_rank = SELN_CARET;
	    return SELN_REQUEST;
	}

      case SELN_FN_PUT:
	if (seln_secondary_exists(buffer)) {
	    if (!seln_holder_same_client(&buffer->secondary, client)) {
		return SELN_IGNORE;
	    } else {
		*holder = &buffer->primary;
		buffer->addressee_rank = SELN_SECONDARY;
		return SELN_REQUEST;
	    }
	}
	if (seln_secondary_made(buffer) ||	/* made & canceled */
	    !seln_holder_same_client(&buffer->primary, client)) {
	    return SELN_IGNORE;
	}
	*holder = &buffer->shelf;
	buffer->addressee_rank = SELN_PRIMARY;
	return SELN_SHELVE;

      case SELN_FN_FIND:
	if (!seln_holder_same_client(&buffer->caret, client)) {
	    return SELN_IGNORE;
	}
	buffer->addressee_rank = SELN_CARET;
	if (!seln_secondary_made(buffer)) {
	    if (seln_non_null_primary(&buffer->primary)) {
		*holder = &buffer->primary;
		return SELN_FIND;
	    }
	    *holder = &buffer->shelf;
	    return SELN_FIND;
	}
	if (!seln_secondary_exists(buffer)) {	/* made & canceled */
	    return SELN_IGNORE;
	}
	*holder = &buffer->secondary;	/* secondary exists  */
	return SELN_FIND;

      case SELN_FN_DELETE:
	if (seln_secondary_exists(buffer)) {
	    if (!seln_holder_same_client(&buffer->secondary, client)) {
		return SELN_IGNORE;
	    } else {
		*holder = &buffer->shelf;
		buffer->addressee_rank = SELN_SECONDARY;
		return SELN_DELETE;
	    }
	}
	if (seln_secondary_made(buffer) ||	/* made & canceled */
	    !seln_holder_same_client(&buffer->primary, client)) {
	    return SELN_IGNORE;
	}
	*holder = &buffer->shelf;
	buffer->addressee_rank = SELN_PRIMARY;
	return SELN_DELETE;
      default:
	complain("figure_response got a malformed buffer.");
	return SELN_IGNORE;
    }
}



Xv_public void
selection_report_event(server, seln_client, event)
    Xv_Server       server;
    Seln_client     seln_client;
    struct inputevent *event;
{
    Seln_function   func;
    Seln_function_buffer buffer;
    Seln_client_node *client;

    client = (Seln_client_node *) seln_client;
    switch (event_action(event)) {
      case ACTION_COPY:	/* formerly KEY_LEFT(6): */
	func = SELN_FN_PUT;
	break;
      case ACTION_PASTE:	/* formerly KEY_LEFT(8): */
	func = SELN_FN_GET;
	break;
      case ACTION_FIND_FORWARD:/* formerly KEY_LEFT(9): */
      case ACTION_FIND_BACKWARD:
	func = SELN_FN_FIND;
	break;
      case ACTION_CUT:		/* formerly KEY_LEFT(10): */
	func = SELN_FN_DELETE;
	break;
      default:
	return;
    }
    buffer = selection_inform(server, (Seln_client) client, func,
			      win_inputposevent(event));
    if (buffer.function != SELN_FN_ERROR &&
	client != (Seln_client_node *) NULL) {
	client->ops.do_function(client->client_data, &buffer);
    }
}

static int
seln_non_null_primary(holder)
    Seln_holder    *holder;
{
    Seln_request    buffer;
    u_int          *bufp = (u_int *) buffer.data;

#ifdef OW_I18N
/*
 * This function is called when Find(L9) is pressed in textsw.
 * SELN_REQ_BYTESIZE requires index conversion from characters to bytes in
 * textsw. It's bit expensive. The following code is for performance up.
 */
    seln_init_request(&buffer, holder, SELN_REQ_CHARSIZE, 0, NULL);
    if (seln_request(holder, &buffer) == SELN_SUCCESS) {
        if (*bufp++ != (u_int) SELN_REQ_CHARSIZE || *bufp == 0)
            return FALSE;
        else
            return TRUE;
    }
#endif /* OW_I18N */
    seln_init_request(&buffer, holder, SELN_REQ_BYTESIZE, 0, NULL);
    if (seln_request(holder, &buffer) != SELN_SUCCESS ||
	*bufp++ != (u_int) SELN_REQ_BYTESIZE ||
	*bufp == 0)
	return FALSE;
    else
	return TRUE;
}
