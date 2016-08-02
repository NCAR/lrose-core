#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndet_veto.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_veto.c - Implementation of notify_veto_destroy.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>

/* ARGSUSED */
extern          Notify_error
notify_veto_destroy(nclient)
    Notify_client   nclient;
{
    NTFY_BEGIN_CRITICAL;
    ndet_flags |= NDET_VETOED;
    NTFY_END_CRITICAL;
    return (NOTIFY_OK);
}
