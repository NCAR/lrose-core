#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)nint_r_in.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Nint_r_in.c - Implement the notify_remove_input_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/nint.h>

extern          Notify_error
notify_remove_input_func(nclient, func, fd)
    Notify_client   nclient;
    Notify_func     func;
    int             fd;
{
    return (nint_remove_fd_func(nclient, func, NTFY_INPUT, fd));
}
