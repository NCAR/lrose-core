#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)attr_copy.c 20.14 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 * 	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE
 *	file for terms of the license.
 */

#include <xview/attr.h>

static int
  copy_1_attr(Attr_attribute attr, Attr_avlist *source, Attr_avlist *dest);

static int
  copy_counted_list(Attr_attribute **source, Attr_attribute **dest);

static int
copy_singleton(Attr_attribute attr, Attr_attribute **source,
               Attr_attribute **dest);

static int
copy_null_list(Attr_attribute attr, Attr_attribute **source,
               Attr_attribute **dest);

/*
 * attr_copy:	copy an attribute list, returning the size in bytes
 */
int
attr_copy(source, dest)
    Attr_avlist *source, *dest;
{
    Attr_attribute  attr;
    int        result, size;

    size = 0;
    do {
	attr = **source;
	result = copy_1_attr(attr, source, dest);
	if (result == -1)
	    return -1;
	size += result;
    } while (attr != 0);
    return size;
}

static int
copy_1_attr(Attr_attribute attr, Attr_avlist *source, Attr_avlist *dest)
{
    int             result, size;

    *source += 1;
    **dest = attr;
    *dest += 1;
    size = sizeof attr;
    if (attr == 0 || ATTR_BASE_TYPE(attr) == ATTR_BASE_NO_VALUE)
	return size;

    switch (ATTR_LIST_TYPE(attr)) {
      case ATTR_NONE:
	result = copy_singleton(attr, source, dest);
	break;
      case ATTR_NULL:
	result = copy_null_list(attr, source, dest);
	break;
      case ATTR_COUNTED:
	result = copy_counted_list(source, dest);
	break;
      case ATTR_RECURSIVE:
	result = attr_copy(source, dest);
	break;
      default:
	return -1;
    }
    if (result == -1)
	return -1;
    else
	return size + result;
}

static int
copy_counted_list(Attr_attribute **source, Attr_attribute **dest)
{
    register unsigned count, n;
    register Attr_attribute *srcp, *destp;

    srcp = *source;
    destp = *dest;
    count = *srcp++;
    *destp++ = count;
    for (n = count; n--;) {
	*destp++ = *srcp++;
    }
    *source = srcp;
    *dest = destp;
    return count + 1;

}

static int
copy_singleton(Attr_attribute attr, Attr_attribute **source,
               Attr_attribute **dest)
{
    register int    count, size;
    register Attr_attribute *srcp, *destp;

    count = ATTR_CARDINALITY(attr);
    size = count * 4;
    srcp = *source;
    destp = *dest;
    while (count-- > 0) {
	*destp++ = *srcp++;
    }
    *source = srcp;
    *dest = destp;
    return size;
}

static int
copy_null_list(Attr_attribute attr, Attr_attribute **source,
               Attr_attribute **dest)

{
    register int    count, size;
    register Attr_attribute *srcp, *destp;

    count = 0;
    size = ATTR_CARDINALITY(attr);
    srcp = *source;
    destp = *dest;
    while (*srcp != 0) {
	register int    i;

	for (i = size; i--;) {
	    *destp++ = *srcp++;
	    count++;
	}
    }
    *destp++ = *srcp++;
    count++;
    *source = srcp;
    *dest = destp;
    return count * sizeof attr;
}
