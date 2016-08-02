#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndet_dodis.c 20.13 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_do_dis.c - In order for "background" dispatching to be done,
 * notify_do_dispatch must be called.  Background dispatching is that
 * dispatching which is done when a read or select is called before calling
 * notify_start.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>

/* If this is set to a valid fd, exclude that fd from implicit dispatching */
int notify_exclude_fd = -5;

extern          Notify_error
notify_do_dispatch()
{
    ndet_flags |= NDET_DISPATCH;
    return (NOTIFY_OK);
}
