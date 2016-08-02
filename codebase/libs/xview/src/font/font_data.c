#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)font_data.c 1.15 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */
#include <xview_private/font_impl.h>

Xv_pkg          xv_font_pkg = {
    "Font", ATTR_PKG_FONT,
    sizeof(Xv_font_struct),
    &xv_generic_pkg,
    font_init,
    font_set_avlist,
    font_get_attr,
    font_destroy_struct,
    font_find_font
};
