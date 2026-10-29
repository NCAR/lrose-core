#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)attr_cust.c 1.7 91/03/25";
#endif
#endif

/*
 *	(c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include  <X11/X.h>
#include  <X11/Xlib.h>
#include  <X11/Xresource.h>
#include  <xview_private/attr_impl.h>
#include  <xview/pkg.h>
#include  <xview/server.h>
#include  <xview/window.h>
#include  <xview/canvas.h>
#include  <xview/panel.h>

EXTERN_FUNCTION( Xv_opaque db_get_data, ( XID db, XrmQuarkList instance_qlist, char *attr_name, Attr_attribute attr, Xv_opaque default_value ));

Xv_private XrmQuarkList generic_create_instance_qlist();
static Attr_avlist attr_copy_customize();

/*
 * Typedefs for storing customizable attributes
 */

/*
 * Node for storing ONE customizable attribute
 */
typedef struct cust_attrs  {
    Attr_attribute	attr;
    char		*attr_name;
    struct cust_attrs	*next;
}Cust_attrs;


/*
 * Node for storing customizable attributes for one pkg
 */
typedef struct cust_pkgs  {
    Xv_pkg		*pkg;
    Cust_attrs		*attr_list;
    struct cust_pkgs	*next;
}Cust_pkgs;

/*
 * Ptr to list of nodes of customizable pkgs
 */
static Cust_pkgs	*customizable_pkgs = (Cust_pkgs *)NULL;

/*
 * Routines for manipulating customizable attributes
 */

/*
 * attr_new_cust_pkg
 * allocs new Cust_pkgs node and returns it
 */
static Cust_pkgs *
attr_new_cust_pkg(pkg)
Xv_pkg		*pkg;
{
    Cust_pkgs	*new;

    new = (Cust_pkgs *)xv_malloc(sizeof(Cust_pkgs));

    if (!new)  {
	return((Cust_pkgs *)NULL);
    }

    new->pkg = pkg;
    new->attr_list = (Cust_attrs *)NULL;
    new->next = (Cust_pkgs *)NULL;

    return(new);
}

/*
 * attr_new_cust_attr
 * allocs new Cust_attrs node and returns it
 */
static Cust_attrs *
attr_new_cust_attr(attr, attr_name)
Attr_attribute	attr;
char		*attr_name;
{
    Cust_attrs	*new;

    new = (Cust_attrs *)xv_malloc(sizeof(Cust_attrs));

    if (!new)  {
	return((Cust_attrs *)NULL);
    }

    new->attr = attr;
    new->attr_name = attr_name;
    new->next = (Cust_attrs *)NULL;

    return(new);
}

/*
 * attr_search_cust_attr
 * searches 'attr_list' for node containing attribute 'attr'
 * and returns it
 */
static Cust_attrs *
attr_search_cust_attr(attr_list, attr)
Cust_attrs	*attr_list;
Attr_attribute	attr;
{
    Cust_attrs		*cur_attr = (Cust_attrs *)NULL;

    for (cur_attr = attr_list; cur_attr; 
		cur_attr = cur_attr->next)  {
	/*
	 * If found match, return right away
	 */
	if (cur_attr->attr == attr)  {
	    return(cur_attr);
	}
    }

    return(cur_attr);
}

/*
 * attr_search_cust_pkg
 * searches customizable_pkgs list for node corresponding to 'pkg'
 * and returns it
 */
static Cust_pkgs *
attr_search_cust_pkg(pkg)
Xv_pkg		*pkg;
{
    Cust_pkgs		*cur_pkg = (Cust_pkgs *)NULL;

    for (cur_pkg = customizable_pkgs; cur_pkg; 
		cur_pkg = cur_pkg->next)  {
	if (cur_pkg->pkg == pkg)  {
	    return(cur_pkg);
	}
    }

    return(cur_pkg);
}

/*
 * attr_find_cust_pkg
 * searches customizable_pkgs list for node corresponding to
 * 'pkg' and returns it.
 * If it is not found, a new one is created.
 */
static Cust_pkgs *
attr_find_cust_pkg(pkg)
Xv_pkg		*pkg;
{
    Cust_pkgs		*cur_pkg = (Cust_pkgs *)NULL;

    cur_pkg = attr_search_cust_pkg(pkg);

    if (cur_pkg)  {
	return(cur_pkg);
    }

    cur_pkg = attr_new_cust_pkg(pkg);
    cur_pkg->next = customizable_pkgs;
    customizable_pkgs = cur_pkg;

    return(cur_pkg);
}
 
/*
 * Checks if the attribute  'attr' is customizable for the package
 * 'pkg'
 */
static char *
attr_check_custom_pkg(pkg, attr)
Xv_pkg          *pkg;
Attr_attribute  attr;
{
    Cust_pkgs   *c_pkg = (Cust_pkgs *)NULL;
    Cust_attrs  *c_attr = (Cust_attrs *)NULL;

    c_pkg = attr_search_cust_pkg(pkg);

    if (c_pkg)  {
        c_attr = attr_search_cust_attr(c_pkg->attr_list, attr);

        if (c_attr)  {
            return(c_attr->attr_name);
        }
    }    

    return((char *)NULL);
}

/*
 * Checks if the attribute  'attr' is customizable for the package
 * 'pkg'
 */
static char *
attr_check_custom(pkg, attr)
Xv_pkg		*pkg;
Attr_attribute	attr;
{
    Xv_pkg	*orig_pkg;
    char        *attr_name = (char *)NULL;

    orig_pkg = pkg;
    while (orig_pkg)  {
        attr_name = attr_check_custom_pkg(orig_pkg, attr);

        if (attr_name)  {
            return(attr_name);
	}

        orig_pkg = orig_pkg->parent_pkg;
    }

    return((char *)NULL);
}

static Xv_server
attr_get_server(obj, passed_owner)
Xv_object	obj;
Xv_object	passed_owner;
{
    Xv_object	owner;
    Xv_server	server;

    if (!obj)  {
	if (passed_owner)  {
	    obj = passed_owner;
	}
	else  {
	    return(xv_default_server);
	}
    }

    if (xv_get(obj, XV_IS_SUBTYPE_OF, SERVER))  {
        return(obj);
    }

    if (xv_get(obj, XV_IS_SUBTYPE_OF, SCREEN))  {
	server = (Xv_server)xv_get(obj, SCREEN_SERVER);
	return(server);
    }

    if (xv_get(obj, XV_IS_SUBTYPE_OF, WINDOW))  {
        server = XV_SERVER_FROM_WINDOW(obj);
    }
    else  {
	owner = xv_get(obj, XV_OWNER, NULL);
	server = attr_get_server(owner, NULL);
    }

    if (!server)  {
	server = xv_default_server;
    }

    return(server);
}

static Attr_avlist
attr_copy_customize(obj, pkg, instance_name, owner, use_db, dest, avlist)
Xv_object	obj;
Xv_pkg		*pkg;
char		*instance_name;
Xv_object	owner;
int		use_db;
Attr_avlist	dest;
Attr_avlist	avlist;
{
    register unsigned	cardinality;
    char		*attr_name;
    Attr_attribute	attr;
    XID			db;
    XrmQuarkList	instance_qlist = NULL;
    Xv_opaque		server;
    Xv_opaque		temp;
    int			see_db;
    Xv_opaque		default_value;

    if (use_db)  {
	if (obj)  {
            instance_qlist = (XrmQuarkList)xv_get(obj, XV_INSTANCE_QLIST);
	}
	else  {
	    instance_qlist = (XrmQuarkList)generic_create_instance_qlist(owner, 
						instance_name);
	}

        server = attr_get_server(obj, owner);
        db = (XID) xv_get(server, SERVER_RESOURCE_DB);
    }

    while ((attr = (Attr_attribute) avlist_get(avlist))) {
	see_db = (attr == XV_USE_DB);

	cardinality = ATTR_CARDINALITY(attr);

	switch (ATTR_LIST_TYPE(attr)) {
	  case ATTR_NONE:	/* not a list */
	    *dest++ = attr;
	    if (!use_db)  {
	        avlist_copy_values(avlist, dest, cardinality);
	    }
	    else  {
		attr_name = attr_check_custom(pkg, attr);

		if (attr_name)  {
                    default_value = (Xv_opaque)avlist_get(avlist);

                    temp = db_get_data(db, instance_qlist, 
				attr_name, attr, default_value);
                    *dest++ = (Attr_attribute)temp;
		}
		else  {
	            avlist_copy_values(avlist, dest, cardinality);
		}
	    }
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
	    if ((cardinality != 0) && (!see_db)) 	/* don't strip it */
		*dest++ = attr;

	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:
		dest = attr_copy_customize(obj, pkg, instance_name, owner, 
				see_db, dest, avlist);
		if ((cardinality != 0) && (!see_db))	/* don't strip it */
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
			dest = attr_copy_customize(obj, pkg, instance_name, 
					owner, see_db, dest, new_avlist);
		}
		break;
	    }
	    break;
	}
    }
    *dest = 0;

    /*
     * Free instance_qlist if it was created
     */
    if (instance_qlist)  {
	free((char *)instance_qlist);
    }

    return (dest);
}

static int
attr_check_use_custom(avlist)
Attr_attribute  avlist[];
{
    register unsigned	cardinality;
    Attr_attribute	attr;
    int			found;

    while ((attr = (Attr_attribute) avlist_get(avlist))) {
	if (attr == XV_USE_DB)  {
	    return(TRUE);
	}

	cardinality = ATTR_CARDINALITY(attr);

	switch (ATTR_LIST_TYPE(attr)) {
	  case ATTR_NONE:	/* not a list */
            avlist = attr_skip(attr, avlist);
	  break;

	  case ATTR_NULL:	/* null terminated list */
            avlist = attr_skip(attr, avlist);
	  break;

	  case ATTR_COUNTED:	/* counted list */
            avlist = attr_skip(attr, avlist);
	  break;

	  case ATTR_RECURSIVE:	/* recursive attribute-value list */
	    switch (ATTR_LIST_PTR_TYPE(attr)) {
	      case ATTR_LIST_IS_INLINE:
		/*
		 * For inline recursive list, call 
		 * attr_check_use_custom recursively
		 */
		found = attr_check_use_custom(avlist);

		/*
		 * If found XV_USE_DB, return right away
		 * else continue with rest of list, skipping over the
		 * recursive list
		 */
		if (found)  {
		    return(found);
		}
		else  {
		    avlist = attr_skip(attr, avlist);
		}
		break;

	      case ATTR_LIST_IS_PTR:
		/*
		 * Isa:
		 * For pointer type recursive lists,
		 * only recurse if the cardinality is 0
		 * I don't know why yet - this is to be consistent
		 * with attr_copy_customize and attr_copy
		 */
		if (cardinality == 0)  {
		    Attr_avlist     new_avlist = (Attr_avlist)
		    avlist_get(avlist);

		    if (new_avlist)  {
		        found = attr_check_use_custom(new_avlist);

			/*
			 * If found XV_USE_DB, return right away
			 */
		        if (found)  {
		            return(found);
		        }
		    }
		}
		break;
	    }
	    break;
	}
    }

    /*
     * XV_USE_DB not found if this point reached,
     * return FALSE
     */
    return (FALSE);
}

/*
 * XView private interface to customizable attrs
 */
Xv_private Attr_avlist
attr_customize(obj, pkg, instance_name, owner, listhead, listlen, avlist)
Xv_object	obj;
Xv_pkg		*pkg;
char		*instance_name;
Xv_object	owner;
Attr_attribute	listhead[];
int		listlen;
Attr_attribute  avlist[];
{
    if (!avlist)  {
        return(avlist);
    }

    if (!attr_check_use_custom(avlist))  {
        return(avlist);
    }

    (void)attr_copy_customize(obj, pkg, instance_name, owner, FALSE, 
				listhead, avlist);

    return(listhead);
}

/*
 * Public interface to customizable attributes
 */
Xv_public	void
#ifdef ANSI_FUNC_PROTO
xv_add_custom_attrs(Xv_pkg *pkg, ...)
#else
xv_add_custom_attrs(pkg, va_alist)
   Xv_pkg	   *pkg;
va_dcl
#endif
{
    va_list		list;
    Cust_pkgs		*c_pkg;
    Cust_attrs		*c_attrs;
    Attr_attribute	attr;
    char		*attr_name;

    if (!pkg)  {
	return;
    }

    VA_START(list, pkg);
    attr = va_arg(list, Attr_attribute);
    while (attr != (Attr_attribute)NULL)  {
        attr_name = va_arg(list, char *);

	/*
	 * Find customizable pkg struct
	 */
	c_pkg = attr_find_cust_pkg(pkg);

	/*
	 * Create new cust attr struct with fields filled in
	 */
	c_attrs = attr_new_cust_attr(attr, attr_name);

	/*
	 * Insert new cust attr
	 */
	c_attrs->next = c_pkg->attr_list;
	c_pkg->attr_list = c_attrs;

	/*
	 * Get next customizable attribute
	 */
        attr = va_arg(list, Attr_attribute);
    }
    va_end(list);

}

