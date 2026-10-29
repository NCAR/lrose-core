#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndetsexcep.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_s_except.c - Implement notify_set_exception_func call.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>

extern          Notify_func
notify_set_exception_func(nclient, func, fd)
    Notify_client   nclient;
    Notify_func     func;
    int             fd;
{
    return (ndet_set_fd_func(nclient, func, fd, NTFY_EXCEPTION));
}
