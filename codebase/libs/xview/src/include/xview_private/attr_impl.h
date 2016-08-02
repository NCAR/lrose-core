/*      @(#)attr_impl.h 20.13 90/11/13 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef _attr_impl_h_already_included
#define	_attr_impl_h_already_included

#include <xview/attr.h>
#include <xview_private/portable.h>

/* size of an attribute */
#define	ATTR_SIZE	(sizeof(Attr_attribute))

#define avlist_get(avlist)      *(avlist)++

/*
 * Copy count elements from list to dest. Advance both list and dest to the
 * next element after the last one copied.
 */
#define	avlist_copy(avlist, dest, count)	\
    { \
        XV_BCOPY((char *) avlist, (char *) dest, (int)(count * ATTR_SIZE)); \
        avlist += count; \
        dest += count; \
    }

/*
 * A macro to copy attribute values count is the number of Xv_opaque size
 * chunks to copy.
 */
#define	avlist_copy_values(avlist, dest, count) \
    if (count == 1) \
        *dest++ = avlist_get(avlist); \
    else { \
	avlist_copy(avlist, dest, count); \
    }


/* package private routines */
extern Attr_avlist	attr_copy_avlist();
extern int		attr_count_avlist();
extern Attr_avlist	attr_copy_valist();

#endif /* _attr_impl_h_already_included */
