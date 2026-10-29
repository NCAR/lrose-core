#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndetgdeath.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_g_destroy.c - Implement the notify_get_destroy_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>

extern          Notify_func
notify_get_destroy_func(nclient)
    Notify_client   nclient;
{
    return (ndet_get_func(nclient, NTFY_DESTROY, NTFY_DATA_NULL,
			  NTFY_IGNORE_DATA));
}
