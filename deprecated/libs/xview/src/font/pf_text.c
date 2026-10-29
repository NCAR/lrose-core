#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)pf_text.c 20.34 93/06/28 SMI";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Rasterop up a text string in a specified Pixfont.
 */

#include <sys/types.h>
#include <pixrect/pixrect.h>
#include <pixrect/pixfont.h>
#ifdef OW_I18N
#include <xview/xv_i18n.h>
#include <xview_private/font_impl.h>
#endif /* OW_I18N */
#include <xview/font.h>
#include <xview_private/portable.h>
#include <xview/xview_xvin.h>
#include <X11/Xlib.h>

struct pr_size  xv_pf_textbatch();

extern Display *xv_default_display;
extern struct pixrectops mem_ops;

#define	SAFER_NOTMPR(pr)	((pr)->pr_ops != &mem_ops)

Xv_public int
xv_pf_text(prpos, op, pf, str)
    struct pr_prpos prpos;
    int             op;
    Pixfont        *pf;
    char           *str;
{
    register int    rmdr = strlen(str), errors = 0;
    Font_string_dims string_dims;
    static Pixmap   glyph_pixmap;
    static int      glyph_width;
    static int      glyph_height;
    static GC       glyph_gc;
    int             needed_width, needed_height;
    int             root_window = RootWindow(xv_default_display,
					 DefaultScreen(xv_default_display));

    if (SAFER_NOTMPR((Pixrect *) prpos.pr)) {
	/*
	 * If the pixrect is a server_image, then hand it off to server_image
	 * object to handle the text ropping
	 */
	errors = server_image_pf_text(prpos, op, pf, str);
	return errors;
    }
    /*
     * otherwise, assuming the pixrect is a memory pixrect
     */
    /*
     * Use new FONT_STRING_DIMS attr instead needed_width  = rmdr *
     * pf->pf_defaultsize.x; needed_height = pf->pf_defaultsize.y;
     */
    (void) xv_get((Xv_opaque)pf, FONT_STRING_DIMS, str, &string_dims);
    needed_width = string_dims.width;
    needed_height = string_dims.height;
    if (glyph_width < needed_width || glyph_height < needed_height) {
	if (glyph_pixmap) {
	    XFreePixmap(xv_default_display, glyph_pixmap);
	}
	glyph_width = MAX(needed_width, glyph_width);
	glyph_height = MAX(needed_height, glyph_height);
	/*
	 * always create the pixmap on the default display because it doesn't
	 * matter which display we create it on, we are just going to bring
	 * back the image
	 */
	glyph_pixmap = XCreatePixmap(xv_default_display, root_window,
				     glyph_width, glyph_height, 1);
    }
    if (!glyph_gc) {
	XGCValues       v;

	v.foreground = XBlackPixel(xv_default_display, 0);
	v.background = XWhitePixel(xv_default_display, 0);
	glyph_gc = XCreateGC(
	 xv_default_display, glyph_pixmap, GCForeground | GCBackground, &v);
    }
    XSetFont(xv_default_display, glyph_gc, (Font)xv_get((Xv_opaque)pf, XV_XID));
    XDrawImageString(xv_default_display, glyph_pixmap, glyph_gc,
      pf->pf_char[*str].pc_home.x, -pf->pf_char[*str].pc_home.y, str, rmdr);
    errors = xv_read_internal(prpos.pr, prpos.pos.x - pf->pf_char[*str].pc_home.x,
		    prpos.pos.y + pf->pf_char[*str].pc_home.y, needed_width,
	    needed_height, PIX_SRC, xv_default_display, glyph_pixmap, 0, 0);
    return errors;
}

Xv_public struct pr_size
xv_pf_textbatch(dst, lenp, pf, str)
    register struct pr_prpos *dst;
    int            *lenp;
    Pixfont        *pf;
    register char  *str;
{
    register int    basex = 0, basey = 0;
    register struct pixchar *pc;
    struct pr_size  pr_size;
    register int    len = *lenp;

    pr_size.x = 0;
    pr_size.y = 0;
    /*
     * Place each character.  Basex and basey keep track of the base position
     * change which resulted from the previous character.
     */
    while (len > 0) {
	if (*str == 0) {
	    *lenp -= len;
	    break;
	}
	len--;
	pc = &pf->pf_char[(u_char) * str++];
	/* if (pc->pc_pr) { */
	dst->pr = pc->pc_pr;
	/*
	 * Character begins at its home location.
	 */
	dst->pos.x = basex + pc->pc_home.x;
	dst->pos.y = basey + pc->pc_home.y;
	dst++;
	/*
	 * Next character must specify an offset to return to the baseline (-
	 * home terms) and account for the width of the character (advance
	 * terms).
	 */
	basex = pc->pc_adv.x - pc->pc_home.x;
	basey = pc->pc_adv.y - pc->pc_home.y;
	/* } else { */
	/*
	 * Skip character, but offset next by its advance.
	 */
	/*
	 * (*lenp)--; basex += pc->pc_adv.x; basey += pc->pc_adv.y; }
	 */
	/*
	 * Accumulate advances for caller.
	 */
	pr_size.x += pc->pc_adv.x;
	pr_size.y += pc->pc_adv.y;
    }
    return (pr_size);
}

Xv_public int
xv_pf_textbound(bound, len, pf, str)
    struct pr_subregion *bound;
    register int    len;
    register Pixfont *pf;
    register char  *str;
/*
 * bound is set to bounding box for str, with origin at left-most point on
 * the baseline for the first character. bound->pos is top-left corner,
 * bound->size.x is width, bound->size.y is height. bound->pr is not
 * modified. NOTE: xv_pf_textbound must duplicate what pf_text & pf_textbatch
 * do!
 */
{
    register int    basex = 0, basey = 0;
    register struct pixchar *pc;
    int             dstposx, dstposy;

    bound->pos.x = bound->pos.y = 0;
    bound->size.x = bound->size.y = 0;
    /*
     * Place each character.  Basex and basey keep track of the base position
     * which resulted from the previous character.
     */
    while (len > 0) {
	len--;
	pc = &pf->pf_char[(u_char) * str++];
	/*
	 * Character begins at its home location.
	 */
	dstposx = basex + pc->pc_home.x;
	dstposy = basey + pc->pc_home.y;
	if (dstposx < bound->pos.x)
	    bound->pos.x = dstposx;
	if (dstposy < bound->pos.y)
	    bound->pos.y = dstposy;
	/*
	 * Character ends where pixrect does
	 */
	dstposx += pc->pc_pr->pr_width /* (pc->pc_adv.x - pc->pc_home.x) */ ;
	dstposy += pc->pc_pr->pr_height /* (pc->pc_adv.y - pc->pc_home.y) */ ;
	if (dstposx > bound->pos.x + bound->size.x)
	    bound->size.x = dstposx - bound->pos.x;
	if (dstposy > bound->pos.y + bound->size.y)
	    bound->size.y = dstposy - bound->pos.y;
	basex += pc->pc_adv.x;
	basey += pc->pc_adv.y;
    }
}

Xv_public struct pr_size
xv_pf_textwidth(len, pf, str)
    int             len;
    register Xv_font pf;
    register char  *str;
{
    struct pr_size  size;
    XFontStruct    *font_info = (XFontStruct *) xv_get(pf, FONT_INFO);
    int             direction = 0;
    int             ascent = 0;
    int             descent = 0;
    XCharStruct     overall_return;
#ifdef OW_I18N
    Font_info	  *font = FONT_PRIVATE(pf);
    XFontSet	   font_set = (XFontSet)xv_get(pf, FONT_SET_ID);
    XRectangle	   overall_ink_extents = {0};
    XRectangle	   overall_logical_extents = {0};
#endif /* OW_I18N */

    /*
     * Initialize overall_return to zeros
     * It is not initialized like overall_ink_extents above because the MIT 
     * build (using cc), complains about "no automatic aggregate initialization"
     */
    XV_BZERO(&overall_return, sizeof(XCharStruct));

#ifdef OW_I18N
    if (font->type == FONT_TYPE_TEXT)  {
        XmbTextExtents(font_set, str, len, &overall_ink_extents, &overall_logical_extents);
        size.x = overall_logical_extents.width;
        size.y = overall_logical_extents.height;	/* max height so won't clip */
    }
    else  {
        (void) XTextExtents(font_info, str, len,
			&direction, &ascent, &descent, &overall_return);
        size.x = overall_return.width;
        size.y = ascent + descent;	/* max height so won't clip */
    }
#else
    (void) XTextExtents(font_info, str, len,
			&direction, &ascent, &descent, &overall_return);
    size.x = overall_return.width;
    size.y = ascent + descent;	/* max height so won't clip */
			
#endif
    return (size);
}

#ifdef OW_I18N
Xv_public struct pr_size
xv_pf_textwidth_wc(len, pf, ws)
    int             	len;
    register Xv_font 	pf;
    register wchar_t  	*ws;
{
    struct pr_size  size;
    XRectangle	overall_ink_extents = {0};
    XRectangle 	overall_logical_extents = {0};
    XFontSet	    font_set = (XFontSet)xv_get(pf, FONT_SET_ID);

    XwcTextExtents(font_set, ws, len, &overall_ink_extents, &overall_logical_extents);
    size.x = overall_logical_extents.width;
    size.y = overall_logical_extents.height;	/* max height so won't clip */   
    return (size);
}
#endif /*OW_I18N*/

