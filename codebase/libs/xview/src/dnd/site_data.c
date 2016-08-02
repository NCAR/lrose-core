#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)site_data.c 1.4 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *      file for terms of the license.
 */

#include <xview_private/site_impl.h>
Pkg_private int dnd_site_init();
Pkg_private Xv_opaque dnd_site_set_avlist();
Pkg_private Xv_opaque dnd_site_get_attr();
Pkg_private int dnd_site_destroy();

Xv_pkg		xv_drop_site_item = {
    "DropSite", ATTR_PKG_DND,
    sizeof(Xv_drop_site_struct),
    &xv_generic_pkg,
    dnd_site_init,
    dnd_site_set_avlist,
    dnd_site_get_attr,
    dnd_site_destroy,
    NULL
};
