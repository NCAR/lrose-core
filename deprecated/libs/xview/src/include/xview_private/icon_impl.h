/*	@(#)icon_impl.h 20.12 90/02/26 SMI	*/

/*****************************************************************************/
/*                                 icon_impl.h                               */
/*	
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license. 
 */
/*****************************************************************************/

#ifndef _xview_icon_impl_h_already_included
#define _xview_icon_impl_h_already_included

#include <xview/icon.h>
#include <xview/pkg.h>
#include <xview/attrol.h>
#ifdef OW_I18N
#include <xview_private/i18n_impl.h>
#include <X11/Xresource.h>
#endif

/*****************************************************************************/
/* icon struct                                                               */
/*****************************************************************************/

typedef struct {
	Icon		public_self;	/* Back pointer */
	Rect		ic_gfxrect;	/* where the graphic goes */
	struct pixrect *ic_mpr;		/* the graphic (a memory pixrect) */
	Rect		ic_textrect;	/* where text goes */
#ifdef OW_I18N
        wchar_t        *ic_text_wcs;    /* primary text data */
#endif
	char	       *ic_text;	/* the text */
	int		ic_flags;
	Xv_opaque	frame;		/* frame Icon is assoc w/ */
	Server_image    ic_mask;        /* graphic mask (pixmap) */
	unsigned long	workspace_pixel; /* The pixel value of the workspace */
        char           *workspace_color; /* wrk space color string */
} Xv_icon_info;

/* flag values */
#define ICON_PAINTED	 0x20		/* icon window has been painted */
#define ICON_BKGDTRANS   0x40            /* transparent window */
#define ICON_TRANSLABEL  0x80            /* transparent labels */
#define	ICON_FIRSTPRIV	 0x0100		/* start of private flags range */
#define	ICON_LASTPRIV	 0x8000		/* end of private flags range */

/*****************************************************************************/
/* typedefs                                                                  */
/*****************************************************************************/

typedef Xv_icon_info *icon_handle;

/*	Other Macros 	*/
#define ICON_PRIVATE(icon) \
	XV_PRIVATE(Xv_icon_info, Xv_icon, icon)
#define ICON_PUBLIC(icon)	XV_PUBLIC(icon)

#define ICON_IS_TRANSPARENT(icon) \
  ((icon)->icon_mask || ((icon)->ic_flags & ICON_BKGTRANS))

/* from icon_object.c */
Pkg_private int 	icon_init();
Pkg_private Xv_opaque icon_set_internal();
Pkg_private Xv_opaque icon_get_internal();
Pkg_private int          icon_destroy_internal();


#endif /* ~_xview_icon_impl_h_already_included */

