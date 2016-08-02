#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)win_cntral.c 20.20 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Implements library routines for centralized window event management.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <xview_private/windowimpl.h>
#include <xview/notify.h>
#include <xview/rect.h>
#include <xview/win_input.h>
#include <xview/win_notify.h>

static Notify_error win_send();

extern int      errno;
extern Event    xv_last_event;

void            notify_perror();

/*
 * Public interface:
 */

Notify_error
win_post_id(client, id, when)
    Notify_client   client;
    int             id;
    Notify_event_type when;
{
    Event           event;

    event_init(&event);
    event_set_id(&event, id);
    event_set_window(&event, client);
    return (win_send(client, &event, when, (Notify_arg) 0, win_copy_event,
		     win_free_event));
}


Notify_error
win_post_id_and_arg(client, id, when, arg, copy_func, release_func)
    Notify_client   client;
    int           id;
    Notify_event_type when;
    Notify_arg      arg;
    Notify_copy     copy_func;
    Notify_release  release_func;
{
    Event           event;

    event_init(&event);
    event_set_id(&event, id);
    event_set_window(&event, client);
    return (win_send(client, &event, when, arg, copy_func,
		     release_func));
}

Notify_error
win_post_event(client, event, when)
    Notify_client   client;
    Event          *event;
    Notify_event_type when;
{
    /* Send event */
    return (win_send(client, event, when, (Notify_arg) 0,
		     win_copy_event, win_free_event));
}

Notify_error
win_post_event_arg(client, event, when, arg, copy_func, release_func)
    Notify_client   client;
    Event          *event;
    Notify_event_type when;
    Notify_arg      arg;
    Notify_copy     copy_func;
    Notify_release  release_func;
{
    /* Send event */
    return (win_send(client, event, when, arg, copy_func, release_func));
}

/* ARGSUSED */
Notify_arg
win_copy_event(client, arg, event_ptr)
    Notify_client   client;
    Notify_arg      arg;
    Event         **event_ptr;
{
    Event          *event_new;

    if (*event_ptr != EVENT_NULL) {
	event_new = (Event *) (xv_malloc(sizeof(Event)));
	*event_new = **event_ptr;
	*event_ptr = event_new;
    }
    return (arg);
}

/* ARGSUSED */
void
win_free_event(client, arg, event)
    Notify_client   client;
    Notify_arg      arg;
    Event          *event;
{
    if (event != EVENT_NULL)
	free((caddr_t) event);
}

/*
 * Private to this module:
 */

static          Notify_error
win_send(client, event, when, arg, copy_func, release_func)
    Notify_client   client;
    register Event *event;
    Notify_event_type when;
    Notify_arg      arg;
    Notify_copy     copy_func;
    Notify_release  release_func;
{
    Notify_error    error;

    /*
     * keymap the event.  Note that we assume the client is a window.
     */
    /*
     * (void)win_keymap_map(client, event);
     */

    /* Post event */
    /* Post immediately if in xv_window_loop */
    error = notify_post_event_and_arg(client, (Notify_event) event,
				      WIN_IS_IN_LOOP ? NOTIFY_IMMEDIATE : when, 
				      arg, copy_func, release_func);
    /* dixon - suppressing annoying error message */
    /* if (error != NOTIFY_OK)
       notify_perror("win_send"); */
    return (error);
}
