#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)dnd_data.c 1.4 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *      file for terms of the license.
 */

#include <xview_private/dndimpl.h>
Pkg_private int dnd_init();
Pkg_private Xv_opaque dnd_set_avlist();
Pkg_private Xv_opaque dnd_get_attr();
Pkg_private int dnd_destroy();


Xv_pkg		xv_dnd_pkg = {
    "Drag & Drop", ATTR_PKG_DND,
    sizeof(Xv_dnd_struct),
    &xv_sel_owner_pkg,
    dnd_init,
    dnd_set_avlist,
    dnd_get_attr,
    dnd_destroy,
    NULL		/* BUG: Need find */
};
