#ifndef lint
#ifdef SCCS
static char     sccsid[] = "@(#)selection.c 1.11 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#if defined(__linux) || defined(__APPLE__)
/* Kludge to prevent multiple variables with same name */
#define __DEFINE_SEL_IMPL_VARS
#endif
#include <xview_private/sel_impl.h>
#if defined(__linux) || defined(__APPLE__)
#undef __DEFINE_SEL_IMPL_VARS
#endif
#include <xview/window.h>

Pkg_private char *xv_sel_atom_to_str(/* display, atom */);
Pkg_private Atom xv_sel_str_to_atom(/* display, string */);


/*ARGSUSED*/
Pkg_private int
sel_init(parent, sel_public, avlist)
    Xv_Window	    parent;
    Selection	    sel_public;
    Attr_avlist	    avlist;
{
	Display    *dpy;
	Sel_info   *sel;
	Xv_sel	   *sel_object = (Xv_sel *) sel_public;
	XID        xid = (XID) xv_get( parent, XV_XID );

	/* Allocate and clear private data */
	sel = xv_alloc(Sel_info);

	/* Link private and public data */
	sel_object->private_data = (Xv_opaque) sel;
	sel->public_self = sel_public;

	dpy = XV_DISPLAY_FROM_WINDOW( parent );

	/* Initialize private data */
	sel->dpy = dpy;
	sel->rank = XA_PRIMARY;
	sel->rank_name = xv_sel_atom_to_str( dpy, sel->rank, xid );
	sel->timeout = defaults_get_integer("selection.timeout",
					    "Selection.Timeout", 3);

#ifdef WIN_SEL_EVENT_PROC
    /* Register selection event handler */
    xv_set(parent,
	   WIN_SEL_EVENT_PROC, sel_event_proc,
	   NULL);
#endif

    return XV_OK;
}


/*ARGSUSED*/
Pkg_private Xv_opaque
sel_set_avlist(sel_public, avlist)
    Selection	    sel_public;
    Attr_avlist	    avlist;
{
    Attr_avlist	    attrs;
    int		    rank_set = FALSE;
    int		    rank_name_set = FALSE;
    Sel_info	    *sel = SEL_PRIVATE(sel_public);
    XID             xid=0;
    
    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
        switch (attrs[0]) {
	  case SEL_RANK:
	    sel->rank = (Atom) attrs[1];
	    rank_set = TRUE;
	    break;
	  case SEL_RANK_NAME:
	    sel->rank_name = (char *) attrs[1];
	    rank_name_set = TRUE;
	    break;
	  case SEL_TIME:
	    sel->time = *(struct timeval *) attrs[1];
	    break;
	  case SEL_TIMEOUT_VALUE:
	    sel->timeout = (int) attrs[1];
	    break;
        }
    }
/*
    xid = (XID) xv_get( sel_public, XV_XID );
*/
    if (rank_set && !rank_name_set) 
        sel->rank_name = xv_sel_atom_to_str( sel->dpy, sel->rank, xid );
    else if (rank_name_set && !rank_set)
        sel->rank = xv_sel_str_to_atom( sel->dpy, sel->rank_name, xid );

    return XV_OK;
}


/*ARGSUSED*/
Pkg_private Xv_opaque
sel_get_attr(sel_public, status, attr, valist)
    Selection	    sel_public;
    int		   *status;
    Attr_attribute  attr;
    va_list	    valist;
{
    Sel_info	   *sel = SEL_PRIVATE(sel_public);

    switch (attr) {
      case SEL_RANK:
	return (Xv_opaque) sel->rank;
      case SEL_RANK_NAME:
	return (Xv_opaque) sel->rank_name;
      case SEL_TIME:
	return (Xv_opaque) &sel->time;
      case SEL_TIMEOUT_VALUE:
	return (Xv_opaque) sel->timeout;
      default:
	if ( xv_check_bad_attr( &xv_sel_pkg, attr ) == XV_ERROR ) 
	    *status = XV_ERROR;
	return (Xv_opaque) 0;
    }
}


Pkg_private int
sel_destroy(sel_public, status)
    Selection	    sel_public;
    Destroy_status  status;
{
    Sel_info	   *sel = SEL_PRIVATE(sel_public);

    if (status == DESTROY_CHECKING || status == DESTROY_SAVE_YOURSELF
        || status == DESTROY_PROCESS_DEATH)
	return XV_OK;

    /* Free up malloc'ed storage */
    free(sel);

    return XV_OK;
}
