#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ntfyclient.c 20.18 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ntfy_client.c - NTFY_CLIENT specific operations that both the detector and
 * dispatcher share.
 */

/* This file seems to implement some kind of speed hack using b-trees
 * to locate notifier clients. Linux does not have the search code in
 * the libs, so I removed it in the hope that it keeps working, albeit
 * slower. Of course it would be nice if someone added tsearch() & friends
 * for linux (see <search.h> on Solaris). (lmfken) */
/* Newsflash: As of libc-5.0.9, Linux has tsearch() & friends, so we define
 * HAVE_TSEARCH here.                  <martin-2.buck@student.uni-ulm.de>
 */
#define HAVE_TSEARCH

#include <xview_private/ntfy.h>
#include <xview_private/ndis.h>	/* For ndis_default_prioritizer */
#include <xview_private/ndet.h>	
#ifdef HAVE_TSEARCH
#include <search.h>
#endif
#include <xview_private/portable.h>

/* Variables used in paranoid enumerator (see ntfy_condition) */
pkg_private_data NTFY_CLIENT *ntfy_enum_client = 0;
pkg_private_data NTFY_CLIENT *ntfy_enum_client_next = 0;

static NTFY_CLIENT dummy_client;
static void *ndet_rootv;
static void **ndet_root = &ndet_rootv;

#ifdef HAVE_TSEARCH

static int ndet_compar( key1, key2 )
   const void *key1, *key2;
{
   u_int key1a, key2a, tmp, tmp1;

   key1a = (u_int)(((NTFY_CLIENT*)key1)->nclient);
   key2a = (u_int)(((NTFY_CLIENT*)key2)->nclient);
   tmp = key1a >> 16;
   tmp1 = key1a << 21;
   key1a = tmp1 | ((key1a << 5) & 0x001f0000) | tmp;
   tmp = key2a >> 16;
   tmp1 = key2a << 21;
   key2a = tmp1 | ((key2a << 5) & 0x001f0000) | tmp;
   return key1a - key2a;
}
#endif /* HAVE_TSEARCH */

pkg_private NTFY_CLIENT *
ntfy_find_nclient(client_list, nclient, client_latest)
    NTFY_CLIENT    *client_list;
    Notify_client   nclient;
    register NTFY_CLIENT **client_latest;
{
    register NTFY_CLIENT *client;
    NTFY_CLIENT    *next;

    ntfy_assert(NTFY_IN_CRITICAL, 36 /* Unprotected list search */);
    /* See if hint matches */
    if (*client_latest && (*client_latest)->nclient == nclient)
	return (*client_latest);

#ifdef HAVE_TSEARCH
    if(( client_list == ndet_clients ) && ndet_clients ) {
       dummy_client.nclient = nclient;
       /* Find client */
       if( client = (NTFY_CLIENT *)tfind( &dummy_client, ndet_root,
           ndet_compar )) {
	       /* This is a weird hack to get the actual client handle back out
                  of the B tree node returned by tsearch */
           client = client->next;

           *client_latest = client;
           return (client);
       }
    }

    else 
#endif /* HAVE_TSEARCH */
        /* Search entire list */
        for (client = client_list; client; client = next) {
            next = client->next;
            if (client->nclient == nclient) {
                /* Set up hint for next time */
                *client_latest = client;
                return (client);
            }
        }    

    return (NTFY_CLIENT_NULL);
}

/*
 * Find/create client that corresponds to nclient
 */
pkg_private NTFY_CLIENT *
ntfy_new_nclient(client_list, nclient, client_latest)
    NTFY_CLIENT   **client_list;
    Notify_client   nclient;
    NTFY_CLIENT   **client_latest;
{
    register NTFY_CLIENT *client;
    static NTFY_CLIENT *new_client = NTFY_CLIENT_NULL;

#ifdef HAVE_TSEARCH
    if( client_list == &ndet_clients ) {
        if( new_client  == NTFY_CLIENT_NULL ) {
            if ((new_client = ntfy_alloc_client()) == NTFY_CLIENT_NULL)
                return (NTFY_CLIENT_NULL);
        }
        new_client->nclient = nclient;
        client = (NTFY_CLIENT *)tsearch( new_client, ndet_root, ndet_compar );

	    /* This is a weird hack to get the actual client handle back out
               of the B tree node returned by tsearch */
        client = client->next;

        if( client == new_client ) {
            if ((new_client = ntfy_alloc_client()) == NTFY_CLIENT_NULL)
                return (NTFY_CLIENT_NULL);
        }
        else
            return client;
    }

    else
#endif /* HAVE_TSEARCH */
      if ((client = ntfy_find_nclient(*client_list, nclient,
				    client_latest)) != NTFY_CLIENT_NULL)
        return client;
    /* Allocate client */
    else if ((client = ntfy_alloc_client()) == NTFY_CLIENT_NULL)
        return (NTFY_CLIENT_NULL);

    /* Initialize client */
    client->next = NTFY_CLIENT_NULL;
    client->conditions = NTFY_CONDITION_NULL;
    client->condition_latest = NTFY_CONDITION_NULL;
    client->nclient = nclient;
    client->prioritizer = ndis_default_prioritizer;
    client->flags = 0;
    /* Append to client list */
    ntfy_append_client(client_list, client);
    /* Set up client hint */
    *client_latest = client;

    return (client);
}

pkg_private void
ntfy_remove_client(client_list, client, client_latest, who)
    NTFY_CLIENT   **client_list;
    NTFY_CLIENT    *client;
    NTFY_CLIENT   **client_latest;
    NTFY_WHO        who;
{
    register NTFY_CONDITION *condition;
    NTFY_CONDITION *next;

    /* Fixup enumeration variables if client matches one of them */
    if (client == ntfy_enum_client)
	ntfy_enum_client = NTFY_CLIENT_NULL;
    if (client == ntfy_enum_client_next)
	ntfy_enum_client_next = ntfy_enum_client_next->next;
    /* Make sure that remove all conditions */
    for (condition = client->conditions; condition; condition = next) {
	next = condition->next;
	ntfy_remove_condition(client, condition, who);
    }
#ifdef HAVE_TSEARCH
    /* Remove & free client from client_list */
    if( client_list == &ndet_clients )
        tdelete( client, ndet_root, ndet_compar );
#endif /* HAVE_TSEARCH */
    ntfy_remove_node((NTFY_NODE **) client_list, (NTFY_NODE *) client);
    /* Invalidate condition hint */
    *client_latest = NTFY_CLIENT_NULL;
}
