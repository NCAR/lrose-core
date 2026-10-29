#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sb_compat.c 1.19 91/03/19";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Module:	sb_compat.c
 * 
 * 
 * Include files:
 */

#include <xview_private/sb_impl.h>
#include <xview_private/portable.h>


Sv1_public      Scrollbar
#ifdef ANSI_FUNC_PROTO
scrollbar_create(Attr_attribute attr1, ...)
#else
scrollbar_create(attr1, va_alist)
    Attr_attribute attr1;
va_dcl
#endif
{
    va_list         args;
    Attr_attribute  avarray[ATTR_STANDARD_SIZE];
    Attr_avlist     avlist = avarray;

    if( attr1 )
    {
        VA_START(args, attr1);
        copy_va_to_av( args, avlist, attr1 );
        va_end(args);
    }
    else
        avlist[0] = XV_ZERO;

    return (Scrollbar) xv_create(0, SCROLLBAR,
				 ATTR_LIST, avlist,
				 NULL);
}

Sv1_public int
#ifdef ANSI_FUNC_PROTO
scrollbar_set(Scrollbar sb_public, ...)
#else
scrollbar_set(sb_public, va_alist)
    Scrollbar       sb_public;
va_dcl
#endif
{
    va_list         args;
    AVLIST_DECL;

    VA_START(args, sb_public);
    MAKE_AVLIST( args, avlist );
    va_end(args);
    return (int) xv_set_avlist(sb_public, avlist);
}


Sv1_public      Xv_opaque
scrollbar_get(sb_public, attr)
    Scrollbar       sb_public;
    Attr_attribute  attr;
{
    return xv_get(sb_public, attr);
}


Sv1_public int
scrollbar_destroy(sb_public)
    Scrollbar       sb_public;
{
    xv_destroy(sb_public);
    return 0;
}

Sv1_public void
scrollbar_scroll_to(sb_public, new_start)
    Scrollbar       sb_public;
    unsigned long   new_start;
{
    (void) xv_set(sb_public, SCROLLBAR_VIEW_START, new_start, NULL);
}
