#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)attr_util.c 20.18 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/portable.h>
#include <xview/attr.h>

/*
 * attr_create_list creates an avlist from the VARARGS passed on the stack.
 * The storage is always allocated.
 */
Attr_avlist
#ifdef ANSI_FUNC_PROTO
attr_create_list(Attr_attribute attr1, ...)
#else
attr_create_list(attr1, va_alist)
    Attr_attribute attr1;
va_dcl
#endif
{
    va_list         valist;
    Attr_avlist     avlist = (Attr_avlist) malloc( ATTR_STANDARD_SIZE
                                                   * sizeof( Attr_attribute ));

    VA_START(valist, attr1);
    copy_va_to_av( valist, avlist, attr1 );
    va_end(valist);
    return avlist;
}

/*
 * attr_find searches and avlist for the first occurrence of a specified
 * attribute.
 */
Attr_avlist
attr_find(attrs, attr)
    register Attr_avlist attrs;
    register Attr_attribute attr;
{
    for (; *attrs; attrs = attr_next(attrs)) {
	if (*attrs == attr)
	    break;
    }
    return (attrs);
}
