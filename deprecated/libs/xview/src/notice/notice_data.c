#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)notice_data.c 1.3 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/noticeimpl.h>

Xv_pkg          xv_notice_pkg = {
    "Notice", ATTR_PKG_NOTICE,
    sizeof(Xv_notice_struct),
    &xv_generic_pkg,		/* subclass of generic */
    notice_init_internal,
    notice_set_avlist,
    notice_get_attr,
    notice_destroy_internal,
    NULL			/* no find proc */
};

