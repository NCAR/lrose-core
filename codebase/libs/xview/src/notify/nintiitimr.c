#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)nintiitimr.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Nint_i_itimer.c - Implement the notify_interpose_itimer_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <xview_private/nint.h>

extern          Notify_error
notify_interpose_itimer_func(nclient, func, which)
    Notify_client   nclient;
    Notify_func     func;
    int             which;
{
    NTFY_TYPE       type;

    /* Check arguments */
    if (ndet_check_which(which, &type))
	return (notify_errno);
    return (nint_interpose_func(nclient, func, type, NTFY_DATA_NULL,
				NTFY_IGNORE_DATA));
}
