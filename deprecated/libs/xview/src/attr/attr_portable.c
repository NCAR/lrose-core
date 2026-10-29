#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)attr_portable.c 20.12 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/attr_impl.h>

#ifndef NON_PORTABLE

static int      valist_count_recursive();
static va_list  valist_skip_value();

#define	valist_get(valist)	va_arg(valist, caddr_t)

/*
 * Copy count elements from list to dest. Advance both list and dest to the
 * next element after the last one copied.
 */
#define	valist_copy_many(valist, dest, count)	\
    { \
	register caddr_t *last = dest + count; \
	\
	while (dest < last) \
	    *dest++ = valist_get(valist); \
    }


/*
 * A macro to copy attribute values count is the number of caddr_t size
 * chunks to copy.
 */
#define	valist_copy_values(src, dest, count) \
    if (count == 1) \
        *dest++ = valist_get(src); \
    else { \
	valist_copy_many(src, dest, count); \
    }


/*
 * copy the var-args list from * valist to dest.  Recursive lists are
 * collapsed into dest.
 */

Attr_avlist
attr_copy_valist(dest, valist)
    register Attr_avlist dest;
    register va_list valist;
{
    register Attr_attribute attr;
    register u_int  cardinality;

    while (attr = (Attr_attribute) valist_get(valist)) {
	cardinality = ATTR_CARDINALITY(attr);
	switch (ATTR_LIST_TYPE(attr)) {
	  case ATTR_NONE:	/* not a list */
	    *dest++ = (caddr_t) attr;
	    valist_copy_values(valist, dest, cardinality);
	    break;

	  case ATTR_NULL:	/* null terminated list */
	    *dest++ = (caddr_t) attr;
	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:
		/*
		 * Note that this only checks the first four bytes for the
		 * null termination. Copy each value element until we have
		 * copied the null termination.
		 */
		do {
		    valist_copy_values(valist, dest, cardinality);
		} while (*(dest - 1));
		break;

	      case ATTR_LIST_IS_PTR:
		*dest++ = valist_get(valist);
		break;
	    }
	    break;

	  case ATTR_COUNTED:	/* counted list */
	    *dest++ = (caddr_t) attr;
	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:{
		    register u_int  count;

		    *dest = valist_get(valist);	/* copy the count */
		    count = ((u_int) * dest++) * cardinality;
		    valist_copy_values(valist, dest, count);
		    break;
		}

	      case ATTR_LIST_IS_PTR:
		*dest++ = valist_get(valist);
		break;
	    }
	    break;

	  case ATTR_RECURSIVE:	/* recursive attribute-value list */
	    if (cardinality != 0)	/* don't strip it */
		*dest++ = (caddr_t) attr;

	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:
		dest = attr_copy_valist(dest, valist);
		if (cardinality != 0)	/* don't strip it */
		    dest++;	/* move past the null terminator */
		valist = valist_skip_value(attr, valist);
		break;

	      case ATTR_LIST_IS_PTR:
		if (cardinality != 0)	/* don't collapse inline */
		    *dest++ = valist_get(valist);
		else {
		    Attr_avlist     new_avlist = (Attr_avlist) valist_get(valist);
		    if (new_avlist)
			/*
			 * Copy the list inline -- don't move past the null
			 * termintor. Here both the attribute and null
			 * terminator will be stripped away.
			 */
			dest = attr_copy_avlist(dest, new_avlist);
		}
		break;
	    }
	    break;
	}
    }
    *dest = 0;
    return (dest);
}

/*
 * return a pointer to the attribute after the value pointed to by list.
 * attr should be the attribute which describes the value at list.
 */
static          va_list
valist_skip_value(attr, valist)
    register Attr_attribute attr;
    register va_list valist;
{
    switch (ATTR_LIST_TYPE(attr)) {
      case ATTR_NULL:
	if (ATTR_LIST_PTR_TYPE(attr) == ATTR_LIST_IS_PTR)
	    (void) valist_get(valist);
	else
	    while (valist_get(valist));
	break;

      case ATTR_RECURSIVE:
	if (ATTR_LIST_PTR_TYPE(attr) == ATTR_LIST_IS_PTR)
	    (void) valist_get(valist);
	else
	    while (attr = (Attr_attribute) valist_get(valist))
		valist = valist_skip_value(attr, valist);
	break;

      case ATTR_COUNTED:
	if (ATTR_LIST_PTR_TYPE(attr) == ATTR_LIST_IS_PTR) {
	    (void) valist_get(valist);
	    break;
	}
	/* else fall through ... */
      case ATTR_NONE:{
	    register u_int  count = ATTR_CARDINALITY(attr);

	    if (ATTR_LIST_TYPE(attr) == ATTR_COUNTED)
		/* use the count */
		count *= (u_int) valist_get(valist);
	    while (count--)
		(void) valist_get(valist);
	}
	break;
    }
    return valist;
}


/*
 * valist_count counts the number of slots in the varargs-list valist.
 * Recursive lists are counted as being collapsed inline.
 */
int
valist_count(valist)
    va_list         valist;
{
    /* count the null termination */
    return (valist_count_recursive(valist, 0) + 1);
}

static int
valist_count_recursive(valist, last_attr)
    register va_list valist;
    register Attr_attribute last_attr;
{
    register Attr_attribute attr;
    register u_int  count = 0;
    register u_int  num;
    register u_int  cardinality;

    while (attr = (Attr_attribute) valist_get(valist)) {
	count++;		/* count the attribute */
	cardinality = ATTR_CARDINALITY(attr);
	attr_check_pkg(last_attr, attr);
	last_attr = attr;
	switch (ATTR_LIST_TYPE(attr)) {
	  case ATTR_NONE:	/* not a list */
	    count += cardinality;
	    valist = valist_skip_value(attr, valist);
	    break;

	  case ATTR_NULL:	/* null terminated list */
	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:
		/*
		 * Note that this only checks the first four bytes for the
		 * null termination.
		 */
		while (valist_get(valist))
		    count++;
		count++;	/* count the null terminator */
		break;

	      case ATTR_LIST_IS_PTR:
		count++;
		(void) valist_get(valist);
		break;
	    }
	    break;

	  case ATTR_COUNTED:	/* counted list */
	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:{
		    va_list         orig_list = valist;

		    num = ((u_int) (valist_get(valist))) * cardinality + 1;
		    count += num;
		    valist = valist_skip_value(attr, orig_list);
		    break;
		}
	      case ATTR_LIST_IS_PTR:
		count++;
		(void) valist_get(valist);
		break;
	    }
	    break;

	  case ATTR_RECURSIVE:	/* recursive attribute-value list */
	    if (cardinality == 0)	/* don't include the attribute */
		count--;

	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:
		count += valist_count_recursive(valist, attr);
		if (cardinality != 0)	/* count the null terminator */
		    count++;
		valist = valist_skip_value(attr, valist);
		break;

	      case ATTR_LIST_IS_PTR:
		if (cardinality != 0) {	/* don't collapse inline */
		    count++;
		    (void) valist_get(valist);
		} else {
		    Attr_avlist     new_avlist = (Attr_avlist) valist_get(valist);

		    if (new_avlist)
			/*
			 * Here we count the elements of the recursive list
			 * as being inline. Don't count the null terminator.
			 */
			count += attr_count_avlist(new_avlist, attr);
		}
		break;
	    }
	    break;
	}
    }
    return count;
}

#endif	/* not NON_PORTABLE */
