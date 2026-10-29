#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)nint_i_out.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Nint_i_out.c - Implement the notify_interpose_output_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/nint.h>

extern          Notify_error
notify_interpose_output_func(nclient, func, fd)
    Notify_client   nclient;
    Notify_func     func;
    int             fd;
{
    return (nint_interpose_fd_func(nclient, func, NTFY_OUTPUT, fd));
}
