#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)curs_cmpat.c 20.24 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * curs_cmpat.c: Compatibility Routines for creating & modifying a cursor.
 * 
 */

#include <xview_private/curs_impl.h>
#include <xview_private/portable.h>
#include <xview/notify.h>
#include <xview/window.h>
#include <xview/screen.h>

Xv_Cursor
#ifdef ANSI_FUNC_PROTO
cursor_create(Attr_attribute attr1, ...)
#else
cursor_create(attr1, va_alist)
    Attr_attribute attr1;
va_dcl
#endif
{
    Xv_opaque       avlist[ATTR_STANDARD_SIZE];
    va_list         valist;

    if( attr1 )
    {
        VA_START(valist, attr1);
        copy_va_to_av( valist, avlist, attr1 );
        va_end(valist);
    }
    else
        avlist[0] = XV_ZERO;

    return (xv_create_avlist(XV_NULL, CURSOR, avlist));
}

void
cursor_destroy(cursor_public)
    Xv_Cursor       cursor_public;
{
    (void) xv_destroy(cursor_public);
}

/* cursor_get returns the current value of which_attr. */
Xv_opaque
cursor_get(cursor_public, which_attr)
    Xv_Cursor       cursor_public;
    Cursor_attribute which_attr;
{
    return (xv_get(cursor_public, (Attr_attribute)which_attr));
}

int
#ifdef ANSI_FUNC_PROTO
cursor_set(Xv_Cursor cursor_public, ...)
#else
cursor_set(cursor_public, va_alist)
    Xv_Cursor       cursor_public;
va_dcl
#endif
{
    AVLIST_DECL;
    va_list         valist;

    VA_START(valist, cursor_public);
    MAKE_AVLIST( valist, avlist );
    va_end(valist);

    return (int) xv_set_avlist(cursor_public, avlist);
}


Xv_Cursor
cursor_copy(cursor_public)
    register Xv_Cursor cursor_public;
{
    return xv_create(xv_get(cursor_public,XV_OWNER),CURSOR,
		     XV_COPY_OF,cursor_public,NULL);
}
