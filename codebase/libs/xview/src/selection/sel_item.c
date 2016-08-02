#ifndef lint
#ifdef SCCS
static char     sccsid[] = "@(#)sel_item.c 1.11 91/04/18";
#endif
#endif

/*
 *	(c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview/window.h>
#include <xview_private/sel_impl.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif /* SVR4 */

Pkg_private char *xv_sel_atom_to_str(/* display, atom */);
Pkg_private Atom xv_sel_str_to_atom(/* display, string */);


/*ARGSUSED*/
Pkg_private int
sel_item_init(parent, sel_item_public, avlist)
    Selection_owner parent;
    Selection_item  sel_item_public;
    Attr_avlist	    avlist;
{
    Sel_item_info   *ip;
    Sel_item_info   *sel_item;
    Xv_sel_item	    *sel_item_object = (Xv_sel_item *) sel_item_public;
    Sel_owner_info  *sel_owner = SEL_OWNER_PRIVATE(parent);
    XID             xid = (XID) xv_get( parent, XV_XID );

    /* Allocate and clear private data */
    sel_item = xv_alloc(Sel_item_info);

    /* Link private and public data */
    sel_item_object->private_data = (Xv_opaque) sel_item;
    sel_item->public_self = sel_item_public;

    /* Append item to end of Selection_owner (== parent) item list */
    if (!sel_owner->first_item) {
	sel_owner->first_item = sel_item;
    } else {
        sel_owner->last_item->next = sel_item;
        sel_item->previous         = sel_owner->last_item;
    }
    sel_owner->last_item = sel_item;

    /* Initialize private data */
    sel_item->format = 8;
    sel_item->owner = sel_owner;
    sel_item->type = XA_STRING;
    sel_item->copy = TRUE;
    sel_item->type_name = xv_sel_atom_to_str( sel_owner->dpy, sel_item->type, xid );

    return XV_OK;
}


/*ARGSUSED*/
Pkg_private Xv_opaque
sel_item_set_avlist(sel_item_public, avlist)
    Selection_item  sel_item_public;
    Attr_avlist	    avlist;

{
    Attr_avlist	    attrs;
    Xv_opaque	    data = XV_ZERO;
    int		    data_set = FALSE;
    int		    length_set = FALSE;
    int		    nbr_bytes;
    Sel_item_info   *sel_item = SEL_ITEM_PRIVATE(sel_item_public);
    int		    type_set = FALSE;
    int		    type_name_set = FALSE;
    Sel_owner_info  *sel_owner;
    XID             xid;
    
    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (attrs[0]) {
	  case SEL_COPY:
	    sel_item->copy = (Bool) attrs[1];
	    break;
	  case SEL_DATA:
	    data = (Xv_opaque) attrs[1];
	    data_set = TRUE;
	    break;
	  case SEL_FORMAT:
	    sel_item->format = (int) attrs[1];
	    break;
	  case SEL_LENGTH:
	    sel_item->length = (unsigned long) attrs[1];
	    length_set = TRUE;
	    break;
	  case SEL_TYPE:
	    sel_item->type = (Atom) attrs[1];
	    type_set = TRUE;
	    break;
	  case SEL_TYPE_NAME:
	    sel_item->type_name = (char *) attrs[1];
	    type_name_set = TRUE;
	    break;
        }
    }

    sel_owner = ( Sel_owner_info * ) sel_item->owner;
    xid = (XID) xv_get( sel_item_public, XV_XID );

    if (type_name_set && !type_set)
	sel_item->type = xv_sel_str_to_atom( sel_owner->dpy, sel_item->type_name, xid );

    if (data_set) {
	if (data && !length_set &&
	    (!strcmp(sel_item->type_name, "STRING") ||
	     !strcmp(sel_item->type_name, "FILE_NAME") ||
	     !strcmp(sel_item->type_name, "HOST_NAME"))) {
	    sel_item->length = strlen( (char *) data);
	}
	if (sel_item->copy) {
	    if (sel_item->data)
		XFree( (char *) sel_item->data );
	    if (data && sel_item->length > 0) {
		nbr_bytes = BYTE_SIZE( sel_item->length, sel_item->format );
		sel_item->data = (Xv_opaque) xv_malloc(nbr_bytes);
		XV_BCOPY( (char *) data, (char *) sel_item->data, nbr_bytes);
	    } else
		sel_item->data = data;
	} else
	    sel_item->data = data;
    }

    return XV_OK;
}


/*ARGSUSED*/
Pkg_private Xv_opaque
sel_item_get_attr(sel_item_public, status, attr, valist)
    Selection_item  sel_item_public;
    int		   *status;
    Attr_attribute  attr;
    va_list	    valist;
{
    Sel_item_info  *sel_item = SEL_ITEM_PRIVATE(sel_item_public);

    switch (attr) {
      case SEL_COPY:
	return (Xv_opaque) sel_item->copy;
      case SEL_DATA:
	return sel_item->data;
      case SEL_FORMAT:
	return (Xv_opaque) sel_item->format;
      case SEL_LENGTH:
	return (Xv_opaque) sel_item->length;
      case SEL_TYPE:
	return (Xv_opaque) sel_item->type;
      case SEL_TYPE_NAME:
        if (!sel_item->type_name){
           /* part of delayed realisation for type_name */

           Sel_owner_info  *sel_owner;
           XID             xid;

           sel_owner = ( Sel_owner_info * ) sel_item->owner;
           xid = (XID) xv_get( sel_item_public, XV_XID );
           sel_item->type_name = xv_sel_atom_to_str( sel_owner->dpy, sel_item->type, xid);
         }

	return (Xv_opaque) sel_item->type_name;
      default:
	if ( xv_check_bad_attr( &xv_sel_item_pkg, attr ) == XV_ERROR ) 
	    *status = XV_ERROR;
	return (Xv_opaque) 0;
    }
}


Pkg_private int
sel_item_destroy(sel_item_public, status)
    Selection_item  sel_item_public;
    Destroy_status  status;
{
    Sel_item_info  *sel_item = SEL_ITEM_PRIVATE(sel_item_public);
    Sel_item_info  *previous = sel_item->previous;

    if (status == DESTROY_CHECKING || status == DESTROY_SAVE_YOURSELF 
        || status == DESTROY_PROCESS_DEATH)
	return XV_OK;

    /* Remove Selection_item for Selection_owner */
    if (previous)
	previous->next = sel_item->next;
    else
	sel_item->owner->first_item = sel_item->next;
    if (sel_item->next)
	sel_item->next->previous = previous;
    else
	sel_item->owner->last_item = previous;

    /* Free up malloc'ed storage */
    free(sel_item);

    return XV_OK;
}

