#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sel_common.c 20.36 93/06/29";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/seln_impl.h>
#include <xview/server.h>
#include <X11/Xlib.h>
#include <xview/sel_compat.h>
#include <X11/Xatom.h>


Pkg_private void seln_give_up_selection();

/*
 * sel_common.c:	routines which appear in both the service and the
 * client library.  (See header of sel_clnt.c for discussion of service /
 * client terminology in this package.)
 * 
 */


/*
 * initialized data goes into sharable (read-only) text segment
 */



/*
 * External procedures (called by client code)
 * 
 * First, some predicates
 * 
 * 
 * Holder_same_client: TRUE if argument access struct identifies this client
 * (valid only in the client's address space)
 */

Xv_public int
seln_holder_same_client(holder, client_data)
    register Seln_holder *holder;
    register char  *client_data;
{
    return (holder != (Seln_holder *) NULL &&
	    holder->access.client != NULL &&
	    ((Seln_client_node *) holder->access.client)->client_data
	    == client_data);
}

/*
 * Holder_same_process: Return TRUE if argument access struct identifies same
 * process
 */
Xv_public int
seln_holder_same_process(holder)
    register Seln_holder *holder;
{
    return (holder != (Seln_holder *) NULL);
}

/*
 * Same_holder: Return TRUE if 2 holders are the same (incl. rank & state)
 */
Xv_public int
seln_same_holder(h1, h2)
    register Seln_holder *h1, *h2;
{
    if (h1 == (Seln_holder *) NULL || h2 == (Seln_holder *) NULL) {
	return FALSE;
    }
    return (h1->rank == h2->rank && h1->state == h2->state &&
	    seln_equal_access(&h1->access, &h2->access));
}

/*
 * Return TRUE if two holder structs agree in all elements, else FALSE
 */
Pkg_private int
seln_equal_access(first, second)
    register Seln_access *first;
    register Seln_access *second;
{

    if (first == (Seln_access *) NULL || second == (Seln_access *) NULL) {
	return FALSE;
    }
    return (first->pid == second->pid && first->client == second->client);
}


/*
 * If the holder of the rank (from the perspective of a xview client within
 * this process) is the agent, return TRUE.
 */
Pkg_private int
selection_equal_agent(server, holder)
    Xv_Server       server;
    Seln_holder     holder;
{
    Seln_agent_info *agent =
	(Seln_agent_info *) xv_get(server, (Attr_attribute)XV_KEY_DATA,
				   SELN_AGENT_INFO);
    return (holder.access.client == agent->agent_holder.access.client);

}

/* #ifdef _SELECTION_AGENT_CLIENT */

/*
 * This is used by selection service that deals with selection between
 * Sunview1 and XView
 */
Pkg_private     Xv_opaque
selection_agent_client(server)
    Xv_Server       server;
{
    Seln_agent_info *agent =
	(Seln_agent_info *) xv_get(server, (Attr_attribute)XV_KEY_DATA,
				   SELN_AGENT_INFO);
    return ((Xv_opaque) (agent->agent_holder.access.client));
}

/* #endif _SELECTION_AGENT_CLIENT */

/*
 * Secondary_made:  true if this buffer indicates a secondary selection has
 * been made since the function key went down (whether or not one exists
 * now).
 */
Xv_public int
seln_secondary_made(buffer)
    Seln_function_buffer *buffer;
{
    return (buffer->secondary.access.pid != 0);
}

/*
 * Secondary_exists:  true if this buffer indicates a secondary selection
 * existed at the time the function key went up.
 */
Xv_public int
seln_secondary_exists(buffer)
    Seln_function_buffer *buffer;
{
    return (buffer->secondary.state != SELN_NONE);
}




/*
 * Yield a selection rank
 */

Pkg_private     Seln_result
selection_send_yield_internal(server, rank, holder)
    Xv_Server       server;
    Seln_rank       rank;
    Seln_holder    *holder;
{
    Seln_request    buffer;
    Seln_replier_data replier;
    char          **requestp;
    Seln_result     result;


    buffer.replier = 0;
    buffer.requester.consume = 0;
    buffer.requester.context = 0;
    buffer.addressee = holder->access.client;
    buffer.rank = rank;
    buffer.status = SELN_SUCCESS;
    buffer.buf_size = 3 * sizeof(char *);
    requestp = (char **) buffer.data;
    *requestp++ = (char *) SELN_REQ_YIELD;
    *requestp++ = 0;
    *requestp++ = 0;

    if (seln_holder_same_process(holder)) {
	buffer.replier = &replier;
	replier.client_data =
	    ((Seln_client_node *) holder->access.client)->client_data;
	replier.rank = holder->rank;
	replier.context = 0;
	replier.request_pointer = (char **) buffer.data;
	if (seln_get_reply_buffer(&buffer) != SELN_SUCCESS) {
	    result = SELN_FAILED;
	}
    }
    requestp = (char **) buffer.data;
    if (*requestp++ != (char *) SELN_REQ_YIELD) {
	result = SELN_FAILED;
    } else
	result = (Seln_result) * requestp;
    return (result);
}

Pkg_private     Seln_result
selection_send_yield(server, rank, holder)
    Xv_Server       server;
    Seln_rank       rank;
    Seln_holder    *holder;
{
    Seln_result result;
    result = selection_send_yield_internal(server,rank,holder);
    /*
     * If the owner of the selection already gave up the selection, update
     * the cache.
     */
    seln_give_up_selection(server, rank);
    return (result);
}


Pkg_private     void
seln_init_reply(request, reply, replier)
    Seln_request   *request;
    Seln_request   *reply;
    Seln_replier_data *replier;
{
    *reply = *request;
    reply->status = SELN_SUCCESS;
    reply->requester = request->requester;
    reply->replier = replier;
    replier->client_data = request->addressee ?
	((Seln_client_node *) (request->addressee))->client_data : NULL;
    replier->rank = request->rank;
    replier->context = 0;
    replier->request_pointer = (char **) request->data;
}

Pkg_private     Seln_result
seln_get_reply_buffer(buffer)
    Seln_request   *buffer;
{
    Seln_client_node *client;
    int             buf_len;
    char           *request;
    char           *limit;
    Seln_result	    result;

    client = (Seln_client_node *) buffer->addressee;
    limit = buffer->data + SELN_BUFSIZE - 2 * sizeof(Seln_attribute);
    buffer->replier->response_pointer = (char **) buffer->data;
    while ((request = *buffer->replier->request_pointer++) != 0) {
	if (buffer->status != SELN_CONTINUED) {
	    *buffer->replier->response_pointer++ = request;
	}
	buf_len = limit - (char *) buffer->replier->response_pointer;
	if (client)
	    result = client->ops.do_request(request, buffer->replier, buf_len);
	else
	    result = SELN_FAILED;
	switch (result) {
	  case SELN_SUCCESS:	/* go round again	 */
	    buffer->status = SELN_SUCCESS;
	    break;
	  case SELN_CONTINUED:	/* 1 buffer filled	 */
	    buffer->buf_size = (char *) buffer->replier->response_pointer
		- buffer->data;
	    *buffer->replier->response_pointer++ = 0;
	    buffer->replier->response_pointer = (char **) buffer->data;
	    buffer->replier->request_pointer -= 1;
	    buffer->status = SELN_CONTINUED;
	    return SELN_SUCCESS;
	  case SELN_UNRECOGNIZED:
	    buffer->replier->response_pointer[-1] = (char *) SELN_REQ_UNKNOWN;
	    *buffer->replier->response_pointer++ = request;
	    buffer->status = SELN_SUCCESS;
	    break;
	  case SELN_DIDNT_HAVE:
	    buffer->replier->response_pointer[-1] = 0;
	    buffer->status = SELN_DIDNT_HAVE;
	    return SELN_SUCCESS;
	  default:		/* failure - quit	 */
	    buffer->replier->response_pointer[-1] = 0;
	    buffer->status = SELN_FAILED;
	    return SELN_FAILED;
	}
	buffer->replier->request_pointer =
	    (char **) attr_skip_value((Attr_attribute) request,
    			              (Attr_avlist) buffer->replier->request_pointer);
    }
    (void) client->ops.do_request(SELN_REQ_END_REQUEST, buffer->replier, 0);
    buffer->status = SELN_SUCCESS;
    *buffer->replier->response_pointer++ = 0;
    buffer->buf_size = (char *) buffer->replier->response_pointer
	- buffer->data;
    return SELN_SUCCESS;
}

/*
 * convert a selection atom to a selection service rank.
 */
Pkg_private     Seln_rank
selection_to_rank(selection, agent)
    Atom            selection;
    Seln_agent_info *agent;
{
    if(! agent)
      return SELN_UNKNOWN;

    if (selection == CLIPBOARD(agent))
	return (SELN_SHELF);
    if (selection == CARET(agent))
	return (SELN_CARET);
    switch (selection) {
      case XA_PRIMARY:
	return SELN_PRIMARY;
      case XA_SECONDARY:
	return SELN_SECONDARY;
      default:
	return SELN_UNKNOWN;
    }
}


/*
 * convert a selection service rank to  selection atom
 */
Pkg_private     Atom
seln_rank_to_selection(rank, agent)
    Seln_rank       rank;
    Seln_agent_info *agent;
{
    switch (rank) {
      case SELN_PRIMARY:
	return (XA_PRIMARY);
      case SELN_SECONDARY:
	return (XA_SECONDARY);
      case SELN_SHELF:
	return (CLIPBOARD(agent));
      case SELN_CARET:
	return (CARET(agent));
      default:
	return (None);
    }
}
