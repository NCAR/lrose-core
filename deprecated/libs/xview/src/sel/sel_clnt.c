#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sel_clnt.c 20.41 93/06/29";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>
#include <xview/server.h>
#include <xview_private/seln_impl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <xview/sel_compat.h>
#include <xview/defaults.h>


/*
 * Sel_clnt:	client (selection-holder) routines for talking to the
 * selection service, and to other holders.
 * 
 * There may be several selections active at a time, distinguished by their
 * "rank" (primary, secondary, shelf).
 * 
 * Terminology:  The old service / client distinction doesn't work too well
 * here, since the real service does very little, and the "clients" may both
 * make and respond to calls symmetrically.  So: The clearinghouse process
 * that helps application programs get in touch with each other, and
 * maintains the state of the user interface, is the "service". The remainder
 * of this process (outside the selection routines) is the "client." A
 * process (this or another) that currently holds a selection is a "holder";
 * a process that wants to communicate with a holder about the selection is a
 * "requester."
 */


typedef enum {
    Seln_seized_self, Seln_seized_ok, Seln_seized_blind
}               Seln_seize_result;


/*
 * Procedures
 */
/*
 * static void			seln_client_remove();
 */
static Seln_result seln_svc_hold_file();
static Seln_result seln_local_request();
static Seln_seize_result seln_seize();
static Seln_function_buffer process_svc_inform(),
                execute_fn();

Pkg_private Seln_result selection_send_yield_internal();
Pkg_private Seln_result selection_send_yield();
Pkg_private void seln_give_up_selection();
Pkg_private Seln_result selection_agent_acquire();
Pkg_private void selection_init_holder_info();

int seln_debug;


/*
 * External procedures (called by our client)
 * 
 * 
 * Create:  generate & store client identification; initialize per-process
 * information (socket, service transport handle, ...) if this is the first
 * client.
 */
/*ARGSUSED*/
Xv_public Seln_client
selection_create(server, function_proc, request_proc, client_data)
    Xv_Server       server;
    void            (*function_proc) ();
Seln_result(*request_proc) ();
    char           *client_data;
{
    register Seln_client_node *handle;
    static unsigned num_of_clients;	/* incremented each time a new client
					 * calls seln_create. So each client
					 * has a unique id for varification
					 * of existence. */

    if ((handle = (Seln_client_node *)
	 xv_alloc(Seln_client_node))
	== (Seln_client_node *) NULL) {
	return (char *) NULL;
    }
    handle->client_num = ++num_of_clients;
    handle->ops.do_function = function_proc;
    handle->ops.do_request = request_proc;
    handle->access.pid = getpid();
    handle->access.client = (char *) handle;
    handle->client_data = client_data;

    seln_debug = defaults_get_boolean("selection.debug", "Selection.Debug",
									 False);
    return (char *) handle;
}

/*
 * Destroy:	destroy a client instance;
 */
/*ARGSUSED*/
Xv_public void
selection_destroy(server, client)
    Xv_Server       server;
    char           *client;
{
    int             i;

    if (client == (char *) NULL) {
	(void) complain("Selection library asked to destroy a 0 client.");
	return;
    }
    /*
     * BUG ALERT: Should iterate on all the servers this client is talking
     * to.
     */
    for (i = 1; i < SELN_RANKS; i++) {
	(void) selection_done(xv_default_server, client, (Seln_rank) i);
    }
    xv_free(client);
}

/*
 * Next batch are calls to the selection service
 * 
 * Acquire:	acquire the indicated selection. (The client may pass in
 * SELN_UNSPECIFIED; this means acquire the secondary selection if a function
 * key is down, else the primary.) Return the socket on which to listen for
 * RPC requests; NULL on failure.
 */
Xv_public       Seln_rank
selection_acquire(server, seln_client, asked)
    Xv_Server       server;
    Seln_client     seln_client;
    Seln_rank       asked;
{
    Seln_rank       given;
    Seln_result     result;
    Seln_holder     buffer;
    Seln_agent_info *agent =
	(Seln_agent_info *) xv_get(server, (Attr_attribute)XV_KEY_DATA,
				   SELN_AGENT_INFO);
    Seln_client_node *client;

    if ((int) asked > (int) SELN_UNSPECIFIED)
	return SELN_UNKNOWN;
    client = (Seln_client_node *) seln_client;
    if (client == (Seln_client_node *) NULL) {
	complain("Acquire for a null client");
	return SELN_UNKNOWN;
    }
    /* Determine which if unspecified; take it from current holder if any  */
    if (seln_seize(server, client->client_data, asked, &given)
	== Seln_seized_self) {
	return given;
    }
    buffer.rank = given;
    buffer.state = SELN_EXISTS;
    buffer.access = client->access;

    /* Update the selection holder table */
    if (ord(given) >= ord(SELN_CARET) &&
	ord(given) <= ord(SELN_SHELF)) {
	if (agent->held_file[ord(given)] != 0) {
	    (void) close(agent->held_file[ord(given)]);
	    agent->held_file[ord(given)] = 0;
	}
	agent->client_holder[ord(given)] = buffer;
	/* Tell the server that the agent has the selection */
	result = (Seln_result) selection_agent_acquire(server, given);
    } else {
	result = SELN_FAILED;
    }

    if (result != SELN_SUCCESS) {
	complain("Service wouldn't let us acquire selection");
	(void) fprintf(stderr, 
		XV_MSG("requested selection: %d; result: %d\n"),
		       given, result);
	return SELN_UNKNOWN;
    }
    return given;
}

/*
 * Tell the service we're giving up the selection (e.g. on termination)
 */
Xv_public       Seln_result
selection_done(server, seln_client, rank)
    Xv_Server       server;
    Seln_client     seln_client;
    Seln_rank       rank;
{
    Seln_holder     my_descriptor;
    Seln_result     result;
    Seln_client_node *client;
    Seln_agent_info *agent =
	(Seln_agent_info *) xv_get(server, (Attr_attribute)XV_KEY_DATA,
				   SELN_AGENT_INFO);

    client = (Seln_client_node *) seln_client;
    if (client == (Seln_client_node *) NULL) {
	complain("Done for a null client");
	return SELN_FAILED;
    }
    my_descriptor.rank = rank;
    my_descriptor.access = client->access;
    if (ord(my_descriptor.rank) >= ord(SELN_CARET) &&
	ord(my_descriptor.rank) <= ord(SELN_SHELF) &&
	seln_equal_access(&my_descriptor.access,
		   &agent->client_holder[ord(my_descriptor.rank)].access)) {
	if (agent->client_holder[ord(my_descriptor.rank)].state == SELN_FILE) {
	    (void) close(agent->held_file[ord(my_descriptor.rank)]);
	    agent->held_file[ord(my_descriptor.rank)] = 0;
	}
	seln_give_up_selection(server, my_descriptor.rank);
	result = SELN_SUCCESS;
    } else {
	result = SELN_FAILED;
    }
    return result;
}

/*
 * Get rid of any selections this process holds
 */
Xv_public void
selection_yield_all(server)
    Xv_Server       server;
{
    register int    i;
    union {
	Seln_holder     array[SELN_RANKS];
	struct {
	    Seln_holder     first;
	    Seln_holders_all rest;
	}               rec;
    }               holders;

    holders.rec.rest = selection_inquire_all(server);
    for (i = ord(SELN_CARET); i <= ord(SELN_SHELF); i++) {
	if (holders.array[i].state == SELN_EXISTS &&
	    !selection_equal_agent(server, holders.array[i])) {

	    (void) selection_send_yield(server, (Seln_rank) i,
					&holders.array[i]);
	    /*
	     * seln_give_up_selection(server, (Seln_rank) i);
	     * selection_init_holder_info(server, (Seln_rank) i);
	     */
	}
    }
}

/*
 * We saw a function key change state; tell the service.
 */


Xv_public       Seln_function_buffer
selection_inform(server, seln_client, which, down)
    Xv_Server       server;
    Seln_client     seln_client;
    Seln_function   which;
    int             down;
{
    Seln_inform_args buffer;
    Seln_function_buffer result;
    Seln_client_node *client;


    client = (Seln_client_node *) seln_client;
    buffer.holder.rank = SELN_UNKNOWN;
    buffer.holder.state = SELN_NONE;
    if (client == (Seln_client_node *) NULL) {
	XV_BZERO((char *) &buffer.holder.access, sizeof(Seln_access));
    } else {
	buffer.holder.access = client->access;
    }
    buffer.function = which;
    buffer.down = down;
    result.addressee_rank = SELN_UNKNOWN;
    process_svc_inform(server, &buffer, &result);
    return result;
}



/*
 * Inquire:  ask the service for access info for the holder of the indicated
 * selection.  (Again, UNSPECIFIED means PRIMARY or SECONDARY as
 * appropriate.)
 */


Xv_public       Seln_holder
selection_inquire(server, which)
    Xv_Server       server;
    Seln_rank       which;
{
    Seln_holder     result;
    Seln_agent_info *agent =
	(Seln_agent_info *) xv_get(server, (Attr_attribute)XV_KEY_DATA,
				   SELN_AGENT_INFO);

    switch (which) {
      case SELN_UNSPECIFIED:
	if (selection_function_pending(server)) {
	    which = SELN_SECONDARY;
	} else {
	    which = SELN_PRIMARY;
	}			/* FALL THROUGH	 */
      case SELN_CARET:
      case SELN_PRIMARY:
      case SELN_SECONDARY:
      case SELN_SHELF:
	result = agent->client_holder[ord(which)];
	break;
      default:
	result = seln_null_holder;
    }

    /*
     * If the result.state is SELN_NONE get the selection info from the
     * server
     */
    /*
     * if ((result.state == SELN_NONE) && (which != SELN_CARET)) {
     */
    if (result.state == SELN_NONE) {
	selection_agent_get_holder(server, which, &result);
    }
    return result;
}

/*
 * Inquire_all:  ask the service for access info for the holders of all
 * selections.
 */
Xv_public       Seln_holders_all
selection_inquire_all(server)
    Xv_Server       server;
{
    Seln_holders_all result;
    Seln_holder    *rank_holder;
    Seln_agent_info *agent =
	(Seln_agent_info *) xv_get(server, (Attr_attribute)XV_KEY_DATA,
				   SELN_AGENT_INFO);
    int             rank;

    for (rank = 1; rank < SELN_RANKS; rank++) {
	switch (rank) {

	  case SELN_CARET:
	    rank_holder = &result.caret;
	    break;
	  case SELN_PRIMARY:
	    rank_holder = &result.primary;
	    break;
	  case SELN_SECONDARY:
	    rank_holder = &result.secondary;
	    break;
	  case SELN_SHELF:
	    rank_holder = &result.shelf;
	    break;
	  default:
	    continue;
	}
	if (agent->client_holder[rank].rank == (Seln_rank) SELN_NONE)
	    selection_agent_get_holder(server, (Seln_rank) rank, rank_holder);
	else
	    *rank_holder = agent->client_holder[rank];
    }
    return (result);
}


/*
 * Clear_functions:  If there is a current secondary selection, yield that
 * selection.
 */


Xv_public void
selection_clear_functions(server)
    Xv_Server       server;
{
    Seln_holder     secondary_holder;

    secondary_holder = selection_inquire(server, SELN_SECONDARY);
    if (secondary_holder.state == SELN_EXISTS) {
	(void) selection_send_yield(server, SELN_SECONDARY,
				    &secondary_holder);
    }
    seln_function_clear(server);
}



/*
 * Now calls to other holders
 * 
 * Send a request concerning the selection to its holder
 */

/*ARGSUSED*/
Xv_public       Seln_result
selection_request(server, holder, buffer)
    Xv_Server       server;
    Seln_holder    *holder;
    Seln_request   *buffer;
{
    if (seln_holder_same_process(holder)) {
	return seln_local_request(holder, buffer);
    }
    complain("Error, Non local request  ");
    return SELN_FAILED;
}

/*
 * short-circuit net if holder of selection is in the same process as
 * requester
 */
/*ARGSUSED*/
static          Seln_result
seln_local_request(holder, buffer)
    Seln_holder    *holder;
    Seln_request   *buffer;
{
    Seln_request    request;
    Seln_replier_data replier_info;
    Seln_result     result;

    request = *buffer;
    (void) seln_init_reply(&request, buffer, &replier_info);
    if (buffer->requester.consume) {

	do {
	    if (seln_get_reply_buffer(buffer) != SELN_SUCCESS) {
		return SELN_FAILED;
	    }
	    result = buffer->requester.consume(buffer);
	    if (result == SELN_FAILED) {
		return SELN_FAILED;
	    }
	    if (result == SELN_CANCEL) {
		*buffer->replier->request_pointer =
		    (char *) SELN_REQ_END_REQUEST;
		(void) seln_get_reply_buffer(buffer);
		return SELN_SUCCESS;
	    } else if (result == SELN_OVER) {
		return (SELN_SUCCESS);
	    }
	} while (buffer->status == SELN_CONTINUED);
	return SELN_SUCCESS;
    }
    if (seln_get_reply_buffer(buffer) != SELN_SUCCESS) {
	return SELN_FAILED;
    }
    if (buffer->status == SELN_CONTINUED) {
	(void) seln_get_reply_buffer(buffer);
	return SELN_FAILED;
    }
    return SELN_SUCCESS;
}

/*
 * Procedures which get called remotely
 */

/*
 * Client internal routines
 */
Pkg_private void
selection_init_holder_info(server, rank)
    Xv_Server       server;
    Seln_rank       rank;
{
    Seln_agent_info *agent =
	(Seln_agent_info *) xv_get(server, (Attr_attribute)XV_KEY_DATA,
				   SELN_AGENT_INFO);

    agent->agent_holder.state = SELN_NONE;
    agent->agent_holder.rank = SELN_UNKNOWN;
    agent->client_holder[ord(rank)].state = SELN_NONE;
    agent->client_holder[ord(rank)].access = seln_null_holder.access;
    agent->seln_acquired_time[ord(rank)] = 0;
}

Pkg_private void
seln_give_up_selection_without_telling_server(server, rank)
    Xv_Server       server;
    Seln_rank       rank;
{
    Display        *display =
	(Display *) xv_get(server, (Attr_attribute)XV_DISPLAY);
    Seln_agent_info *agent =
	(Seln_agent_info *) xv_get(server, (Attr_attribute)XV_KEY_DATA,
				   SELN_AGENT_INFO);
    Atom            selection = seln_rank_to_selection(rank, agent);

    if (selection != None) {
	if ((XID) XGetSelectionOwner(display, selection) == agent->xid)
	{
	    xv_sel_free_compat_data( display, selection );        
	}
	
	selection_init_holder_info(server, rank);
    }
}

/*
 * Yield a selection rank, but don't actually tell X that the selection
 * has changed since we don't want a SelectionClear to be sent. This
 * routine should only be called if we are going to immediately acquire
 * the selection again.  I.e. this should only be called from seln_seize().
 */
static     Seln_result
selection_send_yield_without_telling_server(server, rank, holder)
    Xv_Server       server;
    Seln_rank       rank;
    Seln_holder    *holder;
{
    Seln_result result;
    result = selection_send_yield_internal(server,rank,holder);
    seln_give_up_selection_without_telling_server(server, rank);
    return (result);
}
static          Seln_seize_result
seln_seize(server, client_data, asked, given)
    Xv_Server       server;
    char           *client_data;
    Seln_rank       asked, *given;
{
    Seln_result     result;
    Seln_holder     holder;

    /* Ask the current holder for it	 */
    holder = selection_inquire(server, asked);
    *given = holder.rank;
    if (holder.state != SELN_EXISTS) {
	return Seln_seized_ok;
    }
    if (seln_holder_same_client(&holder, client_data)) {
	return Seln_seized_self;
    }
    result = (Seln_result) selection_send_yield_without_telling_server(server,
				holder.rank,
				&holder);
    switch (result) {
      case SELN_SUCCESS:
	return Seln_seized_ok;
      case SELN_WRONG_RANK:	/* server's behind; try again	 */
	if (*given == SELN_PRIMARY && asked != SELN_SECONDARY) {
	    *given = SELN_SECONDARY;
	    holder = selection_inquire(server, *given);
	    if (holder.state != SELN_EXISTS) {
		return Seln_seized_ok;
	    }
	    if (seln_holder_same_client(&holder, client_data)) {
		return Seln_seized_self;
	    }
	    result = (Seln_result) selection_send_yield(server, holder.rank, &holder);
	} else {
	    complain("Other holder confused about selection ranks");
	    return Seln_seized_blind;
	}
	if (result == SELN_SUCCESS) {
	    return Seln_seized_ok;
	}			/* else FALL THROUGH  */
    }
    return Seln_seized_blind;
}



/*
 * This is trying to emulate what seln_svc_inform was doing in SunView 1.
 * This routine must make sure that if the agent is holder of any of the
 * selection, then that information must be sent to the appropiate window via
 * the X server.
 */
static          Seln_function_buffer
process_svc_inform(server, buffer, result)
    Xv_Server       server;
    Seln_inform_args *buffer;
    Seln_function_buffer *result;
{

    if (buffer->down) {		/* button-down: note status       */
	if (!selection_function_pending(server)) {
            Seln_holder     secondary;
    	    Seln_agent_info *agent =
		(Seln_agent_info *) xv_get(server,(Attr_attribute)XV_KEY_DATA,
					   SELN_AGENT_INFO);
	    /*
	     * This could possibly be speeded up by calling X directly i.e.
	     * seln_give_up_selection
	     */
	    secondary = selection_inquire(server, SELN_SECONDARY);
	    if (secondary.state == SELN_EXISTS)
		(void) selection_send_yield(server, SELN_SECONDARY, &secondary);
	    agent->client_holder[(int) SELN_SECONDARY].access =
		seln_null_holder.access;
	}
	*result = seln_null_function;
    } else {			/* button-up      */
	seln_function_clear(server);
	if (!selection_function_pending(server)) {	   /* invoke transfer */
	    *result = execute_fn(server, buffer);
	} else {
	    *result = seln_null_function;
	}
    }
    return (*result);
}


static          Seln_function_buffer
execute_fn(server, args)
    Xv_Server       server;
    Seln_inform_args *args;
{
    Seln_function_buffer buffer;
    Seln_holder     recipient_holder;
    int             full_notifications = FALSE;

    buffer.function = args->function;
    buffer.caret = selection_inquire(server, SELN_CARET);
    buffer.primary = selection_inquire(server, SELN_PRIMARY);
    buffer.secondary = selection_inquire(server, SELN_SECONDARY);
    buffer.shelf = selection_inquire(server, SELN_SHELF);

    /* For the time being only do the new style */
    if (!full_notifications) {	/* New style: 1 notification  */
	register Seln_rank recipient;

	switch (args->function) {
	  case SELN_FN_GET:
	  case SELN_FN_FIND:
	    recipient = SELN_CARET;
	    break;
	  case SELN_FN_PUT:
	  case SELN_FN_DELETE:
	    if (buffer.secondary.state == SELN_EXISTS) {
		recipient = SELN_SECONDARY;
	    } else {
		recipient = SELN_PRIMARY;
	    }
	    break;
	  default:
	    return (seln_null_function);
	}

	buffer.addressee_rank = recipient;
	recipient_holder = selection_inquire(server, recipient);

	if (recipient_holder.state != SELN_EXISTS)
	    return (seln_null_function);

	/*
	 * if the informer is the intended recipient( It can never be the
	 * agent), tell him in the return value.  Otherwise, send a
	 * notification and give the informer a no-op return.
	 */
	if (seln_equal_access(&args->holder.access, &recipient_holder.access)) {
	    return (buffer);
	} else {
	    register Seln_client_node *client =
			   (Seln_client_node *) recipient_holder.access.client;

	    /*
	     * If the recipient is somebody other than the agent, the
	     * recipient's function call back is called else the agent's
	     * function call back is called.
	     */

	    client->ops.do_function(client->client_data, &buffer);

	    return (seln_null_function);
	}
    } else return (seln_null_function);
}





/*
 * We want the service to take over as holder of a selection whose contents
 * are stored in a file.
 */

Xv_public       Seln_result
selection_hold_file(server, rank, path)
    Xv_Server       server;
    Seln_rank       rank;
    char           *path;
{
    Seln_file_info  buffer;

    buffer.rank = rank;
    buffer.pathname = path;
    return (seln_svc_hold_file(server, buffer));
}

static          Seln_result
seln_svc_hold_file(server, input)
    Xv_Server       server;
    Seln_file_info  input;
{
    int             fd;
    Seln_result     result;
    Seln_agent_info *agent =
	(Seln_agent_info *) xv_get(server, (Attr_attribute)XV_KEY_DATA,
				   SELN_AGENT_INFO);
    Seln_holder     holder;

    if (ord(input.rank) < ord(SELN_PRIMARY) ||
	ord(input.rank) > ord(SELN_SHELF)) {
	complain("Selection service can't hold file");
	(void) fprintf(stderr, 
		XV_MSG("selection # %d\n"), 
		ord(input.rank));
	result = SELN_FAILED;
	return result;
    }
    if ((fd = open(input.pathname, O_RDONLY, 0)) == -1) {
	perror(XV_MSG("Selection service couldn't open selection file"));
	(void) fprintf(stderr, 
		XV_MSG("filename: \"%s\"\n"), 
		input.pathname);
	result = SELN_FAILED;
	return result;
    }
    holder = selection_inquire(server, input.rank);
    /* Don't need to yield if the state is SELN_FILE */
    if (holder.state == SELN_EXISTS) {
	/* Don't need to tell server because we will immediately
	   acquire it again. */
	(void) selection_send_yield_without_telling_server(server,
							   input.rank,&holder);
    }
    (void) selection_acquire(server, agent->agent_holder.access.client, 
                             input.rank);
    agent->held_file[ord(input.rank)] = fd;
    agent->client_holder[ord(input.rank)].state = SELN_FILE;
    agent->client_holder[ord(input.rank)].access = agent->agent_holder.access;
    result = SELN_SUCCESS;
    return (result);
}

Xv_public void
selection_use_timeout(server, seconds)
    Xv_Server       server;
    int             seconds;
{
    Seln_agent_info *agent =
	(Seln_agent_info *) xv_get(server, (Attr_attribute)XV_KEY_DATA,
				   SELN_AGENT_INFO);
    agent->timeout = seconds;
}
