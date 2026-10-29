#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)nintideath.c 20.14 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Nint_i_death.c - Implement the notify_interpose_destroy_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <xview_private/nint.h>

/*
 * Following indirection added to allow XView to redefine
 * notify_interpose_destroy_func() when Notifier used as just a part of
 * XView, while still allowing applications to use the Notifier
 * independent of the rest of XView.
 * 
 * To make live easy, we add the following typedef for a pointer to a function
 * returning a Notify_value.
 */
typedef         Notify_error(*Notify_error_func) ();
static Notify_error default_interpose_destroy_func();
static Notify_error_func nint_destroy_interposer =
default_interpose_destroy_func;

extern          Notify_error
notify_interpose_destroy_func(nclient, func)
    Notify_client   nclient;
    Notify_func     func;
{
    return (nint_destroy_interposer(nclient, func));
}

extern          Notify_error_func
notify_set_destroy_interposer(func)
    Notify_error_func func;
{
    Notify_error_func result = nint_destroy_interposer;

    nint_destroy_interposer = (func) ? func
	: default_interpose_destroy_func;
    return (result);
}

static          Notify_error
default_interpose_destroy_func(nclient, func)
    Notify_client   nclient;
    Notify_func     func;
{
    return (nint_interpose_func(nclient, func, NTFY_DESTROY, NTFY_DATA_NULL,
				NTFY_IGNORE_DATA));
}
