#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndisdsched.c 20.13 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndis_d_sched.c - Default scheduler for the dispatcher.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndis.h>

pkg_private Notify_value ndis_default_scheduler();
pkg_private Notify_value ndis_special_client_scheduler();
static Notify_value save_sched_func();
static int		special_client_set;
static Notify_client	special_client;

pkg_private     Notify_value
ndis_default_scheduler(n, nclients)
    int             n;
    register Notify_client *nclients;
{
    register Notify_client nclient;
    register int    i;

    for (i = 0; i < n; i++) {
	nclient = *(nclients + i);
	/* Notify client if haven't been done yet */
	if (nclient != NOTIFY_CLIENT_NULL) {
            /* notify_client detects errors from nclients */
            if (notify_client(nclient) != NOTIFY_OK)
                return (NOTIFY_UNEXPECTED);
            /*
             * Null out client entry prevents it from being notified again.
             */
            *(nclients + i) = NOTIFY_CLIENT_NULL;
	}
    }
    return (NOTIFY_DONE);
}

pkg_private     Notify_value
ndis_special_client_scheduler(n, nclients)
    int             n;
    register Notify_client *nclients;
{
    register Notify_client nclient;
    register int    i;

    for (i = 0; i < n; i++) {
	nclient = *(nclients + i);
	/* Notify client if haven't been done yet */
	if (nclient != NOTIFY_CLIENT_NULL) {
	    /*
	     * Check if special client flag set
	     */
	    if (special_client_set)  {
		/* 
		 * If it is, check if the current client IS the special
		 * client
		 */
		if (nclient == special_client)  {
	            /* notify_client detects errors from nclients */
	            if (notify_client(nclient) != NOTIFY_OK)
		        return (NOTIFY_UNEXPECTED);
	            /*
	             * Null out client entry prevents it from being notified again.
	             */
	            *(nclients + i) = NOTIFY_CLIENT_NULL;
	        }
	    }
	    else  {
	        if (notify_client(nclient) != NOTIFY_OK)
		    return (NOTIFY_UNEXPECTED);
	        /*
	         * Null out client entry prevents it from being notified again.
	         */
	        *(nclients + i) = NOTIFY_CLIENT_NULL;
	    }

	}
    }
    return (NOTIFY_DONE);
}

void
ndis_set_special_client(client)
Notify_client	client;
{
    if (!special_client_set)  {
	/*
	 * Should check if client exists first
	 */
	special_client = client;
	special_client_set = 1;
    }
}

void
ndis_unset_special_client()
{
    special_client_set = 0;
    special_client = (Notify_client)NULL;
}
