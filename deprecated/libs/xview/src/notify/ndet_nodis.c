#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndet_nodis.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_no_dis.c - To turn off "background" dispatching, notify_no_dispatch
 * must be called.  Background dispatching is that dispatching which is done
 * when a read or select is called before calling notify_start.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>

extern          Notify_error
notify_no_dispatch()
{
    ndet_flags &= ~NDET_DISPATCH;
    return (NOTIFY_OK);
}
