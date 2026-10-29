#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sel_data.c 1.6 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview/sel_pkg.h>

Pkg_private int sel_init();
Pkg_private Xv_opaque sel_set_avlist();
Pkg_private Xv_opaque sel_get_attr();
Pkg_private int sel_destroy();

Pkg_private int sel_owner_init();
Pkg_private Xv_opaque sel_owner_set_avlist();
Pkg_private Xv_opaque sel_owner_get_attr();
Pkg_private int sel_owner_destroy();

Pkg_private int sel_req_init();
Pkg_private Xv_opaque sel_req_set_avlist();
Pkg_private Xv_opaque sel_req_get_attr();
Pkg_private int sel_req_destroy();

Pkg_private int sel_item_init();
Pkg_private Xv_opaque sel_item_set_avlist();
Pkg_private Xv_opaque sel_item_get_attr();
Pkg_private int sel_item_destroy();

Xv_pkg xv_sel_pkg = {
    "Selection",
    ATTR_PKG_SELECTION,
    sizeof(Xv_sel),
    &xv_generic_pkg,
    sel_init,
    sel_set_avlist,
    sel_get_attr,
    sel_destroy,
    NULL			/* no find proc */
};

Xv_pkg xv_sel_owner_pkg = {
    "Selection Owner",
    ATTR_PKG_SELECTION,
    sizeof(Xv_sel_owner),
    &xv_sel_pkg,
    sel_owner_init,
    sel_owner_set_avlist,
    sel_owner_get_attr,
    sel_owner_destroy,
    NULL			/* no find proc */
};

Xv_pkg xv_sel_requestor_pkg = {
    "Selection Requestor",
    ATTR_PKG_SELECTION,
    sizeof(Xv_sel_requestor),
    &xv_sel_pkg,
    sel_req_init,
    sel_req_set_avlist,
    sel_req_get_attr,
    sel_req_destroy,
    NULL			/* no find proc */
};

Xv_pkg xv_sel_item_pkg = {
    "Selection Item",
    ATTR_PKG_SELECTION,
    sizeof(Xv_sel_item),
    &xv_generic_pkg,
    sel_item_init,
    sel_item_set_avlist,
    sel_item_get_attr,
    sel_item_destroy,
    NULL			/* no find proc */
};



