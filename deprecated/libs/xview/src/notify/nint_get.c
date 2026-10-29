#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)nint_get.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Nint_get.c - Implement the nint_get_func private interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <xview_private/nint.h>

pkg_private     Notify_func
nint_get_func(cond)
    register NTFY_CONDITION *cond;
{
    Notify_func     func;

    if (cond->func_count > 1)
	func = cond->callout.functions[cond->func_count - 1];
    else
	func = cond->callout.function;
    return (func);
}
