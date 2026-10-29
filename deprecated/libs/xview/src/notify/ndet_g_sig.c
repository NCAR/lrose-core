#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndet_g_sig.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_g_sig.c - Implement the notify_get_signal_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>

extern          Notify_func
notify_get_signal_func(nclient, signal, mode)
    Notify_client   nclient;
    int             signal;
    Notify_signal_mode mode;
{
    NTFY_TYPE       type;

    /* Check arguments */
    if (ndet_check_mode(mode, &type))
	return (NOTIFY_FUNC_NULL);
    if (ndet_check_sig(signal))
	return (NOTIFY_FUNC_NULL);
    return (ndet_get_func(nclient, type, (NTFY_DATA) signal, NTFY_USE_DATA));
}
