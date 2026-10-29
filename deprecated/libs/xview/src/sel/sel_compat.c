#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sel_compat.c 1.23 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview/server.h>
#include <xview_private/seln_impl.h>
#include <xview/sel_svc.h>
#include <xview/win_input.h>


Xv_public char *
seln_create(function_proc, request_proc, client_data)
    void	      (*function_proc) ();
    Seln_result	      (*request_proc) ();
    char	       *client_data;
{
    return (selection_create(xv_default_server, function_proc,
			     request_proc, client_data));
}


Xv_public void
seln_destroy(client)
    char *client;
{
    selection_destroy(xv_default_server, client);
}


Xv_public void
seln_report_event(seln_client, event)
    Seln_client     seln_client;
    struct inputevent *event;
{
    selection_report_event(xv_default_server, seln_client, event);
}

Xv_public       Seln_holder
seln_inquire(which)
    Seln_rank       which;
{
    return (selection_inquire(xv_default_server, which));
}


Xv_public void
seln_clear_functions()
{
    selection_clear_functions(xv_default_server);
}




Xv_public       Seln_rank
seln_acquire(seln_client, asked)
    Seln_client     seln_client;
    Seln_rank       asked;
{
    return (selection_acquire(xv_default_server, seln_client, asked));
}


Xv_public       Seln_result
seln_request(holder, buffer)
    Seln_holder    *holder;
    Seln_request   *buffer;
{
    return (selection_request(xv_default_server, holder, buffer));
}



Xv_public       Seln_holders_all
seln_inquire_all()
{
    return (selection_inquire_all(xv_default_server));
}


Xv_public       Seln_result
seln_done(seln_client, rank)
    Seln_client     seln_client;
    Seln_rank       rank;
{
    return (selection_done(xv_default_server, seln_client, rank));
}



Xv_public       Seln_function_buffer
seln_inform(seln_client, which, down)
    Seln_client     seln_client;
    Seln_function   which;
    int             down;
{
    return (selection_inform(xv_default_server, seln_client, which, down));
}


Seln_response
seln_figure_response(buffer, holder)
    Seln_function_buffer *buffer;
    Seln_holder   **holder;
{
    return (selection_figure_response(xv_default_server, buffer, holder));
}

Xv_public       Seln_result
seln_hold_file(rank, path)
    Seln_rank       rank;
    char           *path;
{
    return (selection_hold_file(xv_default_server, rank, path));
}


Xv_public void
seln_yield_all()
{
    selection_yield_all(xv_default_server);
}

Xv_public void
seln_use_timeout(seconds)
    int	seconds;
{
    selection_use_timeout(xv_default_server, seconds);
}
