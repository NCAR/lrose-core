#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)attr.c 20.24 90/12/04";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <stdio.h>
#include <xview_private/i18n_impl.h>
#include <xview/pkg_public.h>
#include <xview/xv_error.h>
#include <xview_private/attr_impl.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif /* SVR4 */


Xv_private FILE *xv_help_find_file();

#define ANBUFSIZE 64
static char     attr_name_buf[ANBUFSIZE];	/* to hold name of attribute */

Attr_avlist attr_make_count( listhead, listlen, valist, countptr )
    Attr_avlist     listhead;
    int             listlen;
    va_list         valist;
    int            *countptr;
{
    return( copy_va_to_av( valist, listhead, XV_NULL ));
}

/*
 * attr_name - return a pointer to a string containing the name of
 * the specified attribute, or its hex value if not found.
 */
Xv_private char *
attr_name(attr)
    Attr_attribute attr;
{
    int		    attr_value;
    FILE	   *file_ptr;
    int		    found = FALSE;

    attr_name_buf[0] = 0;
    file_ptr = xv_help_find_file("attr_names");
    if (file_ptr) {
	while (fscanf(file_ptr, "%x %s\n", &attr_value, attr_name_buf) != EOF) {
	    if ((found = (attr_value == attr)))
		break;
	}
	fclose(file_ptr);
    }
    if (!found)
	sprintf(attr_name_buf, "attr # 0x%08lx", attr);
    return attr_name_buf;
}


/* copy_va_to_av copies a varargs parameter list into an avlist. If the
   avlist parameter is NULL a new avlist is malloced and returned after
   the varargs list is copied into it. Attr1 exists because ANSI C requires
   that all varargs be preceded by a normal parameter, but some xview
   functions have only a varargs list as their parameters. So those 
   functions declare the first vararg parameter as a normal parameter, and
   pass it in to copy_va_to_av as attr1.
*/

Attr_attribute avlist_tmp[ATTR_STANDARD_SIZE];

Xv_private Attr_avlist copy_va_to_av( valist1, avlist1, attr1 )
   va_list valist1;
   Attr_avlist avlist1;
   Attr_attribute attr1;
{
   register Attr_attribute attr;
   register unsigned cardinality;
   register int i;
   int avlist_size;
   static va_list valist;
   static Attr_avlist avlist;
   static int arg_count = 0, arg_count_max = ATTR_STANDARD_SIZE,
              recursion_count = 0;

   recursion_count++;

   /* These two variables are used instead of the paramters so that the
      position in the lists is maintained after a recursive call. 
   */
   va_copy(valist, valist1);
   avlist = avlist1;

   if( !avlist )
      avlist = avlist_tmp;

   if( attr1 )
      attr = attr1;
   else
      attr = va_arg( valist, Attr_attribute );

   while( attr )
   {
      if( ++arg_count > arg_count_max )
      {
         xv_error( XV_NULL,
                   ERROR_STRING, XV_MSG(
              "A/V list more than 250 elements long, extra elements ignored"),
                   NULL);
         return( avlist1 );
      }
      cardinality = ATTR_CARDINALITY(attr);

      switch (ATTR_LIST_TYPE(attr))
      {
         case ATTR_NONE:       /* not a list */
            *avlist++ = attr;

            if(( arg_count += cardinality ) > arg_count_max )
            {
               xv_error( XV_NULL,
                         ERROR_STRING, XV_MSG(
               "A/V list more than 250 elements long, extra elements ignored"),
                         NULL);
               return( avlist1 );
            }
            for( i=0; i < cardinality; i++ )
               *avlist++ = va_arg( valist, Attr_attribute );
            break;

         case ATTR_NULL:       /* null terminated list */
            *avlist++ = attr; 
            switch (ATTR_LIST_PTR_TYPE(attr))
            {
               case ATTR_LIST_IS_INLINE:
                  /*
                   * Note that this only checks the first four bytes for the
                   * null termination. Copy each value element until we have
                   * copied the null termination.
                   */
                  do {
                      if(( arg_count += cardinality ) > arg_count_max )
                      {
                         xv_error( XV_NULL,
                                   ERROR_STRING, XV_MSG(
              "A/V list more than 250 elements long, extra elements ignored"),
                                   NULL);
                         return( avlist1 );
                      }
                      for( i=0; i < cardinality; i++ )
                         *avlist++ = va_arg( valist, Attr_attribute );
                  } while (*(avlist - 1));
                  break;

               case ATTR_LIST_IS_PTR:
                  if( ++arg_count  > arg_count_max )
                  {
                     xv_error( XV_NULL,
                               ERROR_STRING, XV_MSG(
              "A/V list more than 250 elements long, extra elements ignored"),
                               NULL);
                     return( avlist1 );
                  }
                  *avlist++ = va_arg( valist, Attr_attribute );
                  break;
            }
            break;

         case ATTR_COUNTED:    /* counted list */
            *avlist++ = attr;
            switch (ATTR_LIST_PTR_TYPE(attr)) {
               case ATTR_LIST_IS_INLINE:
               {
                  register unsigned count;

                  if( ++arg_count  > arg_count_max )
                  {
                     xv_error( XV_NULL,
                               ERROR_STRING, XV_MSG(
              "A/V list more than 250 elements long, extra elements ignored"),
                               NULL);
                     return( avlist1 );
                  }
                  *avlist = va_arg( valist, Attr_attribute ); /*copy the count*/
                  count = ((unsigned) *avlist++) * cardinality;

                  if(( arg_count += count ) > arg_count_max )
                  {
                     xv_error( XV_NULL,
                               ERROR_STRING, XV_MSG(
              "A/V list more than 250 elements long, extra elements ignored"),
                               NULL);
                     return( avlist1 );
                  }
                  for( i=0; i < count; i++ )
                     *avlist++ = va_arg( valist, Attr_attribute );
                }
                break;

                case ATTR_LIST_IS_PTR:
                   if( ++arg_count  > arg_count_max )
                   {
                      xv_error( XV_NULL,
                                ERROR_STRING, XV_MSG(
              "A/V list more than 250 elements long, extra elements ignored"),
                                NULL);
                      return( avlist1 );
                   }
                   *avlist++ = va_arg( valist, Attr_attribute );
                   break;
             }
             break;

          case ATTR_RECURSIVE:  /* recursive attribute-value list */
             if (cardinality != 0)       /* don't strip it */
                *avlist++ = attr;

             switch (ATTR_LIST_PTR_TYPE(attr))
             {
                case ATTR_LIST_IS_INLINE:
                   (void) copy_va_to_av(valist, avlist, NULL);
                   if (cardinality != 0)   /* don't strip it */
                      avlist++;     /* move past the null terminator */
                   break;

                case ATTR_LIST_IS_PTR:
                   if (cardinality != 0)   /* don't collapse inline */
                   {
                      if( ++arg_count  > arg_count_max )
                      {
                         xv_error( XV_NULL,
                                   ERROR_STRING, XV_MSG(
              "A/V list more than 250 elements long, extra elements ignored"),
                                   NULL);
                         return( avlist1 );
                      }
                      *avlist++ = va_arg( valist, Attr_attribute );
                   }
                   else
                   {
                      attr = va_arg( valist, Attr_attribute );
                      if (attr)
                         /*
                          * Copy the list inline -- don't move past the null
                          * termintor. Here both the attribute and null
                          * terminator will be stripped away.
                          */
                         avlist = attr_copy_avlist(avlist, (Attr_avlist) attr);
                  }
                  break;
            }
            break;
      }

      attr = va_arg( valist, Attr_attribute );
   }    

   *avlist = 0;
   if( !avlist1 )
   {
      avlist_size = (( avlist - avlist_tmp ) + 1 ) * sizeof( Attr_attribute );
      avlist1 = xv_malloc( avlist_size );
      XV_BCOPY( avlist_tmp, avlist1, avlist_size );
   }

   if( --recursion_count == 0 )
   {
      arg_count = 0;
      arg_count_max = ATTR_STANDARD_SIZE;
   }

   return( avlist1 );
}



/*
 * attr_copy_avlist copies the attribute-value list from avlist to dest.
 * Recursive lists are collapsed into dest.
 */

Attr_avlist
attr_copy_avlist(dest, avlist)
    register Attr_avlist dest;
    register Attr_avlist avlist;
{
    register Attr_attribute attr;
    register unsigned cardinality;

    while ((attr = (Attr_attribute) avlist_get(avlist))) {
	cardinality = ATTR_CARDINALITY(attr);
	switch (ATTR_LIST_TYPE(attr)) {
	  case ATTR_NONE:	/* not a list */
	    *dest++ = attr;
	    avlist_copy_values(avlist, dest, cardinality);
	    break;

	  case ATTR_NULL:	/* null terminated list */
	    *dest++ = attr;
	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:
		/*
		 * Note that this only checks the first four bytes for the
		 * null termination. Copy each value element until we have
		 * copied the null termination.
		 */
		do {
		    avlist_copy_values(avlist, dest, cardinality);
		} while (*(dest - 1));
		break;

	      case ATTR_LIST_IS_PTR:
		*dest++ = avlist_get(avlist);
		break;
	    }
	    break;

	  case ATTR_COUNTED:	/* counted list */
	    *dest++ = attr;
	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:{
		    register unsigned count;

		    *dest = avlist_get(avlist);	/* copy the count */
		    count = ((unsigned) *dest++) * cardinality;
		    avlist_copy_values(avlist, dest, count);
		}
		break;

	      case ATTR_LIST_IS_PTR:
		*dest++ = avlist_get(avlist);
		break;
	    }
	    break;

	  case ATTR_RECURSIVE:	/* recursive attribute-value list */
	    if (cardinality != 0)	/* don't strip it */
		*dest++ = attr;

	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:
		dest = attr_copy_avlist(dest, avlist);
		if (cardinality != 0)	/* don't strip it */
		    dest++;	/* move past the null terminator */
		avlist = attr_skip(attr, avlist);
		break;

	      case ATTR_LIST_IS_PTR:
		if (cardinality != 0)	/* don't collapse inline */
		    *dest++ = avlist_get(avlist);
		else {
		    Attr_avlist     new_avlist = (Attr_avlist)
		    avlist_get(avlist);
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
 * attr_count counts the number of slots in the av-list avlist. Recursive
 * lists are counted as being collapsed inline.
 */
int
attr_count(avlist)
    Attr_avlist     avlist;
{
    /* count the null termination */
    return (attr_count_avlist(avlist, (Attr_attribute) NULL) + 1);
}


int
attr_count_avlist(avlist, last_attr)
    register Attr_avlist avlist;
    register Attr_attribute last_attr;  /* BUG ALERT: Not used any more */
{
    register Attr_attribute attr;
    register unsigned count = 0;
    register unsigned num;
    register unsigned cardinality;

    while ((attr = (Attr_attribute) * avlist++)) {
	count++;		/* count the attribute */
	cardinality = ATTR_CARDINALITY(attr);
	last_attr = attr;
	switch (ATTR_LIST_TYPE(attr)) {
	  case ATTR_NONE:	/* not a list */
	    count += cardinality;
	    avlist += cardinality;
	    break;

	  case ATTR_NULL:	/* null terminated list */
	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:
		/*
		 * Note that this only checks the first four bytes for the
		 * null termination.
		 */
		while (*avlist++)
		    count++;
		count++;	/* count the null terminator */
		break;

	      case ATTR_LIST_IS_PTR:
		count++;
		avlist++;
		break;
	    }
	    break;

	  case ATTR_COUNTED:	/* counted list */
	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:
		num = ((unsigned) (*avlist)) * cardinality + 1;
		count += num;
		avlist += num;
		break;
	      case ATTR_LIST_IS_PTR:
		count++;
		avlist++;
		break;
	    }
	    break;

	  case ATTR_RECURSIVE:	/* recursive attribute-value list */
	    if (cardinality == 0)	/* don't include the attribute */
		count--;

	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:
		count += attr_count_avlist(avlist, attr);
		if (cardinality != 0)	/* count the null terminator */
		    count++;
		avlist = attr_skip(attr, avlist);
		break;

	      case ATTR_LIST_IS_PTR:
		if (cardinality != 0) {	/* don't collapse inline */
		    count++;
		    avlist++;
		} else if (*avlist)
		    /*
		     * Here we count the elements of the recursive list as
		     * being inline. Don't count the null terminator.
		     */
		    count += attr_count_avlist((Attr_avlist) * avlist++,
					       attr);
		else
		    avlist++;
		break;
	    }
	    break;
	}
    }
    return count;
}


/*
 * attr_skip_value returns a pointer to the attribute after the value pointed
 * to by avlist.  attr should be the attribute which describes the value at
 * avlist.
 */
Attr_avlist
attr_skip_value(attr, avlist)
    register Attr_attribute attr;
    register Attr_avlist avlist;
{
    switch (ATTR_LIST_TYPE(attr)) {
      case ATTR_NULL:
	if (ATTR_LIST_PTR_TYPE(attr) == ATTR_LIST_IS_PTR)
	    avlist++;
	else
	    while (*avlist++);
	break;

      case ATTR_RECURSIVE:
	if (ATTR_LIST_PTR_TYPE(attr) == ATTR_LIST_IS_PTR)
	    avlist++;
	else
	    while ((attr = (Attr_attribute) * avlist++))
		avlist = attr_skip_value(attr, avlist);
	break;

      case ATTR_COUNTED:
	if (ATTR_LIST_PTR_TYPE(attr) == ATTR_LIST_IS_PTR)
	    avlist++;
	else
	    avlist += ((int) *avlist) * ATTR_CARDINALITY(attr) + 1;
	break;

      case ATTR_NONE:
	avlist += ATTR_CARDINALITY(attr);
	break;
    }
    return avlist;
}
