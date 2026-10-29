#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)term_data.c 1.16 93/06/28";
#endif
#endif

/***********************************************************************/
/* term_data.c               	       */
/*	
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license. 
 */
/***********************************************************************/

#include <sys/types.h>
#include <xview_private/term_impl.h>

Pkg_private int termsw_folio_init();
Pkg_private Xv_opaque termsw_folio_set();
Pkg_private Xv_opaque termsw_folio_get();
Pkg_private int termsw_folio_destroy();

Xv_pkg          xv_termsw_pkg = {
    "Termsw",
    (Attr_pkg) ATTR_PKG_TERMSW,
    sizeof(Xv_termsw),
    &xv_openwin_pkg,
    termsw_folio_init,
    termsw_folio_set,
    termsw_folio_get,
    termsw_folio_destroy,
    NULL			/* no find proc */
};

Pkg_private int termsw_view_init();
Pkg_private Xv_opaque termsw_view_set();
Pkg_private Xv_opaque termsw_view_get();
Pkg_private int termsw_view_destroy();
Xv_pkg          xv_termsw_view_pkg = {
    "Termsw_view",
    (Attr_pkg) ATTR_PKG_TERMSW_VIEW,
    sizeof(Xv_termsw_view),
    &xv_window_pkg,
    termsw_view_init,
    termsw_view_set,
    termsw_view_get,
    termsw_view_destroy,
    NULL			/* no find proc */
};
