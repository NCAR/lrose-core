#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)cms_data.c 1.12 89/08/18";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/cms_impl.h>

Xv_pkg          xv_cms_pkg = {
    "Color", ATTR_PKG_CMS,
    sizeof(Xv_cms_struct),
    &xv_generic_pkg,
    cms_init,
    cms_set_avlist,
    cms_get_attr,
    cms_destroy,
    cms_find_cms    
};
