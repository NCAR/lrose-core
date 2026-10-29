#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)panel_seln.c 20.30 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/panel_impl.h>
#include <xview/sun.h>
#include <xview/sel_attrs.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif SVR4

extern void     (*panel_seln_inform_proc) (),
                (*panel_seln_destroy_proc) ();

static void     panel_seln_destroy_info(),
                panel_seln_function(),
                panel_seln_get();

static void     panel_seln_report_event(), check_cache();


static Seln_result panel_seln_request();

/* Register with the service */
Pkg_private void
panel_seln_init(panel)
    register Panel_info *panel;
{
    /*
     * this is so we only try to contact the selection service once.
     */
    static          no_selection_service;	/* Defaults to FALSE */

    register Panel_selection *primary = panel_seln(panel, SELN_PRIMARY);
    register Panel_selection *secondary = panel_seln(panel, SELN_SECONDARY);
    register Panel_selection *caret = panel_seln(panel, SELN_CARET);
    register Panel_selection *shelf = panel_seln(panel, SELN_SHELF);

    if (no_selection_service)
	return;

    panel->seln_client =
	(Xv_opaque) seln_create(panel_seln_function, panel_seln_request,
		    (char *) panel);

    if (!panel->seln_client) {
	no_selection_service = TRUE;
	return;
    }
    panel_seln_destroy_proc = panel_seln_destroy_info;
    panel_seln_inform_proc = (void (*) ()) panel_seln_report_event;

    primary->rank = SELN_PRIMARY;
    primary->is_null = TRUE;
    primary->ip = (Item_info *) 0;

    secondary->rank = SELN_SECONDARY;
    secondary->is_null = TRUE;
    secondary->ip = (Item_info *) 0;

    caret->rank = SELN_CARET;
    caret->is_null = TRUE;
    caret->ip = (Item_info *) 0;

    shelf->rank = SELN_SHELF;
    shelf->is_null = TRUE;
    shelf->ip = (Item_info *) 0;
}


/* Inquire about the holder of a selection */
Pkg_private     Seln_holder
panel_seln_inquire(rank)
    Seln_rank       rank;
{
    Seln_holder     holder;

    /*
     * always ask the service, even if we have not setup contact before (i.e.
     * no text items). This could happen if some other item has
     * PANEL_ACCEPT_KEYSTROKE on. if (!panel->seln_client) holder.rank =
     * SELN_UNKNOWN; else
     */
    holder = seln_inquire(rank);
    return holder;
}


static void
panel_seln_report_event(panel, event)
    Panel_info     *panel;
    Event          *event;
{
    seln_report_event(panel->seln_client, event);

    if (!panel->seln_client)
	return;

    check_cache(panel, SELN_PRIMARY);
    check_cache(panel, SELN_SECONDARY);
    check_cache(panel, SELN_CARET);
}

static void
check_cache(panel, rank)
    register Panel_info *panel;
    register Seln_rank rank;
{
    Seln_holder     holder;

    if (panel_seln(panel, rank)->ip) {
	holder = seln_inquire(rank);
	if (!seln_holder_same_client(&holder, (char *) panel))
	    panel_seln_cancel(panel, rank);
    }
}



/* Acquire the selection and update state */
Pkg_private void
panel_seln_acquire(panel, ip, rank, is_null)
    register Panel_info *panel;
    Item_info      *ip;
    Seln_rank       rank;
    int             is_null;
{
    register Panel_selection *selection;

    if (!panel->seln_client)
	return;

    switch (rank) {
      case SELN_PRIMARY:
      case SELN_SECONDARY:
      case SELN_CARET:
	selection = panel_seln(panel, rank);
	/*
	 * if we already own the selection, don't ask the service for it.
	 */
	if (ip && selection->ip == ip)
	    break;
	/* otherwise fall through ... */

      default:
	rank = seln_acquire(panel->seln_client, rank);
	switch (rank) {
	  case SELN_PRIMARY:
	  case SELN_SECONDARY:
	  case SELN_CARET:
	  case SELN_SHELF:
	    selection = panel_seln(panel, rank);
	    break;

	  default:
	    return;
	}
	break;
    }

    /* if there was an old selection, de-hilite it */
    if (selection->ip)
	panel_seln_dehilite(selection->ip, rank);

    /* record the selection & hilite it if it's not null */
    selection->ip = ip;
    selection->is_null = is_null;
    if (!is_null)
	panel_seln_hilite(selection);
}

/*
 * Clear out the current selection.
 */
Pkg_private void
panel_seln_cancel(panel, rank)
    Panel_info     *panel;
    Seln_rank       rank;
{
    Panel_selection *selection = panel_seln(panel, rank);

    if (!panel->seln_client || !selection->ip)
	return;

    /* de-hilite the selection */
    panel_seln_dehilite(selection->ip, rank);
    selection->ip = (Item_info *) 0;
    (void) seln_done(panel->seln_client, rank);
}

/* Destroy myself as a selection client */
static void
panel_seln_destroy_info(panel)
    register Panel_info *panel;
{
    if (!panel->seln_client)
	return;

    /*
     * cancel PRIMARY and SECONDARY to get rid of possible highlighting
     */
    panel_seln_cancel(panel, SELN_PRIMARY);
    panel_seln_cancel(panel, SELN_SECONDARY);
    if (panel->shelf) {
	free(panel->shelf);
	panel->shelf = (char *) 0;
    }
    seln_destroy(panel->seln_client);
}

/* Callback routines */

/* A function key has gone up -- do something. */
static void
panel_seln_function(panel, buffer)
    register Panel_info *panel;
    register Seln_function_buffer *buffer;
{
    Seln_holder    *holder;
    Panel_selection *selection;
    char           *selection_string;
    int             selection_length;
    char            save_char;
    register u_char *cp;
    u_char         *char_buf;
    Event           event;

    if (!panel->kbd_focus_item)
	return;

    switch (seln_figure_response(buffer, &holder)) {
      case SELN_IGNORE:
	break;

      case SELN_REQUEST:
	panel_seln_get(panel, holder, buffer->addressee_rank);
	break;

      case SELN_SHELVE:
	if (panel->shelf)
	    free(panel->shelf);
	selection = panel_seln(panel, buffer->addressee_rank);
	if (selection->is_null || !selection->ip) {
	    selection_string = "";
	    selection_length = 0;
	} else
	    panel_get_text_selection(selection->ip, &(u_char *)selection_string,
				     &selection_length, selection->rank);
	if (selection_string) {
	    save_char = *(selection_string + selection_length);
	    *(selection_string + selection_length) = 0;
	    panel->shelf = (char *) panel_strsave((u_char *)selection_string);
	    *(selection_string + selection_length) = save_char;
	} else
	    panel->shelf = (char *) panel_strsave((u_char *)"");
	panel_seln_acquire(panel, (Item_info *) 0,
			   SELN_SHELF, TRUE);
	break;

      case SELN_FIND:
	(void) seln_ask(holder,
			SELN_REQ_COMMIT_PENDING_DELETE,
			SELN_REQ_YIELD, 0,
			NULL);
	break;

      case SELN_DELETE:{

	    if (panel->shelf)
		free(panel->shelf);
	    selection = panel_seln(panel, buffer->addressee_rank);
	    if (selection->is_null || !selection->ip) {
		selection_string = "";
		selection_length = 0;
	    } else
		panel_get_text_selection(selection->ip,
					 &(u_char *)selection_string,
					 &selection_length, selection->rank);
	    if (selection_string) {
		save_char = *(selection_string + selection_length);
		*(selection_string + selection_length) = 0;
		panel->shelf =
		    (char *) panel_strsave((u_char *)selection_string);
		*(selection_string + selection_length) = save_char;
	    } else
		panel->shelf = (char *) panel_strsave((u_char *)"");
	    panel_seln_acquire(panel, (Item_info *) 0,
			       SELN_SHELF, TRUE);

	    if (!selection_string || selection->is_null || !selection->ip)
		break;

	    /* CUT: Delete the selected characters and call the notify proc */
	    panel_seln_delete(selection, TRUE);

	    if (buffer->addressee_rank == SELN_SECONDARY) {	/* "Quick Move"
								 * operation */
		/* PASTE: Copy the shelf to the caret */
		event_init(&event);
		event_set_down(&event);
		event_set_action(&event, ACTION_PASTE);
		panel_handle_event(ITEM_PUBLIC(panel->kbd_focus_item),
				   &event);
		char_buf = cp = (u_char *) malloc(strlen(panel->shelf) + 1);
		strcpy(cp, panel->shelf);
		while (*cp) {
		    event_set_id(&event, (short) *cp++);
		    panel_handle_event(ITEM_PUBLIC(panel->kbd_focus_item),
				       &event);
		}
		free(char_buf);
		event_set_up(&event);
		event_set_action(&event, ACTION_PASTE);
		panel_handle_event(ITEM_PUBLIC(panel->kbd_focus_item),
				   &event);
	    }
	    break;
	}

      default:
	/* ignore anything else */
	break;
    }
}


/*
 * Respond to a request about my selections.
 */
static          Seln_result
panel_seln_request(attr, context, max_length)
    Seln_attribute  attr;
    register Seln_replier_data *context;
    int             max_length;
{
    register Panel_info *panel = (Panel_info *) context->client_data;
    register Panel_selection *selection;
    char           *selection_string = (char *) 0;
    int             selection_length;
    Seln_result     result;

    switch (context->rank) {
      case SELN_PRIMARY:
      case SELN_SECONDARY:
	selection = panel_seln(panel, context->rank);
	if (selection->ip)
	{
	    if (!selection->is_null)
		panel_get_text_selection(selection->ip,
		     &(u_char *)selection_string, &selection_length,
		     selection->rank);
	}
	break;

      case SELN_SHELF:
	selection_string = panel->shelf;
	selection_length = (selection_string ? strlen(selection_string) : 0);
	break;

      default:
	break;
    }

    switch (attr) {
      case SELN_REQ_BYTESIZE:
	if (!selection_string)
	    return SELN_DIDNT_HAVE;
	*context->response_pointer++ = (caddr_t) selection_length;
	return SELN_SUCCESS;

      case SELN_REQ_CONTENTS_ASCII:{
	    char           *temp = (char *) context->response_pointer;
	    int             count;

	    if (!selection_string)
		return SELN_DIDNT_HAVE;

	    count = selection_length;
	    if (count <= max_length) {
		XV_BCOPY(selection_string, temp, count);
		temp += count;
		while ((unsigned) temp % sizeof(*context->response_pointer))
		    *temp++ = '\0';
		context->response_pointer = (char **) temp;
		*context->response_pointer++ = 0;
		return SELN_SUCCESS;
	    }
	    return SELN_FAILED;
	}

      case SELN_REQ_YIELD:
	result = SELN_FAILED;
	switch (context->rank) {
	  case SELN_SHELF:
	    if (panel->shelf) {
		result = SELN_SUCCESS;
		free(panel->shelf);
		panel->shelf = 0;
		(void) seln_done(panel->seln_client, SELN_SHELF);
	    }
	    break;

	  default:
	    if (panel_seln(panel, context->rank)->ip) {
		panel_seln_cancel(panel, (Seln_rank) context->rank);
		result = SELN_SUCCESS;
	    }
	    break;
	}
	*context->response_pointer++ = (caddr_t) result;
	return result;

      default:
	return SELN_UNRECOGNIZED;
    }
}


/* Selection utilities */

/*
 * Get the selection from holder and put it in the text item that owns the
 * selection rank.
 */
static void
panel_seln_get(panel, holder, rank)
    Panel_info     *panel;
    Seln_holder    *holder;
    Seln_rank       rank;
{
    Seln_request   *buffer;
    register Attr_avlist avlist;
    register u_char *cp;
    Event           event;
    int             num_chars;
    Panel_selection *selection = panel_seln(panel, rank);
    Item_info      *ip = selection->ip;

    if (!panel->seln_client)
	return;

    /*
     * if the request is too large, drop it on the floor.
     */
    if (holder->rank == SELN_SECONDARY)
	buffer = seln_ask(holder,
			  SELN_REQ_BYTESIZE, 0,
			  SELN_REQ_CONTENTS_ASCII, 0,
			  SELN_REQ_YIELD, 0,
			  NULL);
    else
	buffer = seln_ask(holder,
			  SELN_REQ_BYTESIZE, 0,
			  SELN_REQ_CONTENTS_ASCII, 0,
			  SELN_REQ_COMMIT_PENDING_DELETE,
			  NULL);

    if (buffer->status == SELN_FAILED)
	return;

    avlist = (Attr_avlist) buffer->data;

    if ((Seln_attribute) * avlist++ != SELN_REQ_BYTESIZE)
	return;

    num_chars = (int) *avlist++;

    if ((Seln_attribute) * avlist++ != SELN_REQ_CONTENTS_ASCII)
	return;

    cp = (u_char *) avlist;

    /*
     * make sure the string is null terminated in the last byte of the
     * selection.
     */
    cp[num_chars] = '\0';

    panel_seln_dehilite(ip, rank);

    /* initialize the event */
    event_init(&event);
    event_set_down(&event);
    event_set_action(&event, ACTION_PASTE);
    panel_handle_event(ITEM_PUBLIC(ip), &event);

    while (*cp) {
	event_set_id(&event, (short) *cp++);
	(void) panel_handle_event(ITEM_PUBLIC(ip), &event);
    }

    event_set_up(&event);
    event_set_action(&event, ACTION_PASTE);
    panel_handle_event(ITEM_PUBLIC(ip), &event);
}
