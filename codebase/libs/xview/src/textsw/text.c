#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)text.c 20.31 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <xview/textsw.h>
#include <xview/win_struct.h>
#include <xview/window.h>
#include <xview/text.h>

Pkg_private Textsw_folio textsw_init_internal();

Pkg_private int textsw_init();
Pkg_private Xv_opaque textsw_get();
Pkg_private Xv_opaque textsw_set();
Pkg_private int textsw_folio_destroy();

/* Pkg_private */ int	text_notice_key;



Pkg_private int
textsw_init(parent, textsw_public, avlist)
    Xv_Window       parent;
    Textsw          textsw_public;
    Attr_attribute  avlist[];
{
    Attr_avlist     attrs;
    Textsw_status   dummy_status;
    Textsw_status  *status = &dummy_status;
    Xv_textsw      *textsw_object = (Xv_textsw *) textsw_public;
    Textsw_folio    folio = NEW(struct textsw_object);

    if (!text_notice_key)  {
	text_notice_key = xv_unique_key();
    }

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (*attrs) {
	  case TEXTSW_STATUS:
	    status = (Textsw_status *) attrs[1];
	    break;
	  default:
	    break;
	}
    }

    if (!folio) {
	*status = TEXTSW_STATUS_CANNOT_ALLOCATE;
	return (XV_ERROR);
    }
    /* link to object */
    textsw_object->private_data = (Xv_opaque) folio;
    folio->public_self = textsw_public;

    folio = textsw_init_internal(folio, status, textsw_default_notify, avlist);

    /*
     * BUG: Note the folio is not really initialized until the first view is
     * created.
     */
    return (folio ? XV_OK : XV_ERROR);
}
