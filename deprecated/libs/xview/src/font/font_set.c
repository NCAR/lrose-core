#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)font_set.c 20.28 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifdef OW_I18N
#include <euc.h>
#endif /* OW_I18N */
#include <xview_private/i18n_impl.h>
#include <xview_private/font_impl.h>

/*
 * Private
 */

Pkg_private     Xv_opaque
font_set_avlist(font_public, avlist)
    Xv_Font         font_public;
    Attr_attribute  avlist[];
{
    register Font_info *font = FONT_PRIVATE(font_public);
    register Attr_attribute *attrs;

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (attrs[0]) {
	  case FONT_TYPE:
	    font->type = (Font_type) attrs[1];
	    break;
	  case XV_END_CREATE:{
#ifdef OW_I18N
		if (!multibyte) {
		    Font_string_dims dims;

		    (void) xv_get(font_public, FONT_STRING_DIMS, "n", &dims, NULL);
		    if ((dims.width > 0) && (dims.width < font->def_char_width))
		        font->def_char_width = dims.width;
		}
#else
		Font_string_dims dims;

		(void) xv_get(font_public, FONT_STRING_DIMS, "n", &dims, NULL);
		if ((dims.width > 0) && (dims.width < font->def_char_width))
		    font->def_char_width = dims.width;
#endif /* OW_I18N */
#ifdef CHECK_OVERLAPPING_CHARS
		if (font->overlapping_chars && font->type == FONT_TYPE_TEXT) {
		    char            dummy[128];

		    sprintf(dummy, 
		    XV_MSG("Font '%s' has overlapping characters;\n\
character painting errors may occur."), font->name);
		    xv_error(XV_ZERO,
			     ERROR_STRING, dummy,
			     ERROR_PKG, FONT,
			     NULL);
		}
#endif				/* CHECK_OVERLAPPING_CHARS */
		break;
	    }
	  default:
	    xv_check_bad_attr(&xv_font_pkg, attrs[0]);
	    /* BUG: should we return attrs[0] here? */
	    break;

	}
    }

    return (XV_OK);
}
