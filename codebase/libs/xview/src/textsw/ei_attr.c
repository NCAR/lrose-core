#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)ei_attr.c 20.20 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Attribute support for entity interpreters.
 */

#include <sys/types.h>
#include <xview/attrol.h>
#include <xview/pkg.h>
#include <xview_private/primal.h>

#include <xview_private/ei.h>

Pkg_private int
#ifdef ANSI_FUNC_PROTO
ei_set(Ei_handle eih, ...)
#else
ei_set(eih, va_alist)
    register Ei_handle eih;
va_dcl
#endif
{
    va_list  valist;
    AVLIST_DECL;

    VA_START( valist, eih );
    MAKE_AVLIST( valist, avlist );
    va_end( valist );
    return (eih->ops->set(eih, avlist));
}
