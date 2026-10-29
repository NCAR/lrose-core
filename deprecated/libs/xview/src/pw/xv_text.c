#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_text.c 20.26 89/07/31";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * Xv_text.c: Implements pw_char/text functions of the pixwin.h interface for
 * X server.
 */
#include <xview_private/pw_impl.h>
#include <xview/window.h>
#include <pixrect/pixfont.h>

PIXFONT        *xv_pf_sys;

extern          xv_pf_ttext(), xv_pf_text();
extern struct pr_size xv_pf_textwidth();

Xv_public int
pw_char(pw, xw, yw, op, pixfont, c)
    Xv_opaque       pw;
    int             op, xw, yw;
    char            c;
    struct pixfont *pixfont;
{
    char            str[2];

    str[0] = c;
    str[1] = XV_ZERO;
    (void) xv_text(pw, xw, yw, op, (Xv_opaque)pixfont, str);
}

Xv_public int
xv_ttext(window, xbasew, ybasew, op, pixfont, str)
    Xv_opaque       window;
    int             op;
    register int    xbasew, ybasew;
    Xv_opaque       pixfont;
    char           *str;
{
    Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;
    GC              gc;
    XID             font;
    int             len;

    if ((len = strlen(str)) == 0) {
	return 0;
    }

    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    d = xv_xid(info);
    gc = xv_find_proper_gc(display, info, PW_TEXT);

    /* SunView1.X incompatibility: NULL pixfont meant use system_font. */
    if (pixfont == 0) {
	pixfont = xv_get(window, WIN_FONT);
    }
    /*
     * Since this is transparent text, we always paint it with the background
     * color.
     */
    xv_set_gc_op(display, info, gc, op, XV_USE_CMS_FG,
		 XV_INVERTED_FG_BG);
    font = (XID) xv_get(pixfont, XV_XID);
    XSetFont(display, gc, font);
    XDrawString(display, d, gc, xbasew, ybasew, str, len);

    return 0;

}

Xv_public int
xv_text(window, xbasew, ybasew, op, pixfont, str)
    Xv_opaque       window;
    int             op;
    register int    xbasew, ybasew;
    Xv_opaque       pixfont;
    char           *str;
{
    Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;
    GC              gc;
    XID             font;
    int             len;

    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    d = xv_xid(info);
    gc = xv_find_proper_gc(display, info, PW_TEXT);

    if ((len = strlen(str)) == 0) {
	return 0;
    }

    /* SunView1.X incompatibility: NULL pixfont meant use system_font. */
    if (pixfont == 0) {
	pixfont = xv_get(window, WIN_FONT);
    }
    if (PIX_OP(op) == PIX_NOT(PIX_SRC)) {
	xv_set_gc_op(display, info, gc, op, 
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_INVERTED_FG_BG);
    } else {
	xv_set_gc_op(display, info, gc, op, 
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_DEFAULT_FG_BG);
    }
    font = (XID) xv_get(pixfont, XV_XID);
    XSetFont(display, gc, font);

    if (PIX_OP(op) == PIX_SRC || PIX_OP(op) == PIX_NOT(PIX_SRC)) {
	XDrawImageString(display, d, gc, xbasew, ybasew, str, len);
    } else
      XDrawString(display, d, gc, xbasew, ybasew, str, len);

    return 0;
}

Xv_public int
xv_glyph_char(window, x, y, width, height, pixfont, c, color_index)
    Xv_opaque       window;
    register int    x, y;
    int             width, height;
    Pixfont        *pixfont;
    char            c;
    int		    color_index;   /* -1 => use default foreground color */
{
    Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;
    GC              gc, gc1;
    XID             font;
    char            s[2];
    int		    color_op;
    int		    fg_type;	/* foreground type */

    if (color_index < 0) {
	color_op = PIX_SRC;
	fg_type = XV_USE_CMS_FG;
    } else {
	color_op = PIX_SRC | PIX_COLOR(color_index);
	fg_type = XV_USE_OP_FG;
    }

    DRAWABLE_INFO_MACRO(window, info);
    display = xv_display(info);
    d = (Drawable) xv_xid(info);
    s[0] = c;
    s[1] = XV_ZERO;
    gc = xv_find_proper_gc(display, info, PW_TEXT);
    gc1 = xv_find_proper_gc(display, info, PW_ROP_NULL_SRC);
    /*
     * Note - It is far cheaper not to do any clipping, but to draw the
     * entire glyph char. This will have to be changed.
     */
    xv_set_gc_op(display, info, gc, color_op, fg_type,
		 XV_DEFAULT_FG_BG);
    xv_set_gc_op(display, info, gc1, color_op, fg_type,
		 XV_INVERTED_FG_BG);

    font = (XID) xv_get((Xv_opaque)pixfont, XV_XID);
    XSetFont(display, gc, font);

    XFillRectangle(display, d, gc1, x, y, width, height);
    XDrawString(display, d, gc, x, y, s, 1);
}

/*
 * Shared system pixfont.
 */
Xv_public PIXFONT        *
pw_pfsysopen()
{
    if (xv_pf_sys == 0) {
	/* xv_pf_sys should be set by side-effect of xv_pf_open() */
	(void) xv_pf_open((char *) 0);
#ifdef _XV_DEBUG
	if (xv_pf_sys == 0)
	    abort();
#endif
    }
    return (xv_pf_sys);
}

Xv_public int
pw_pfsysclose()
{
}
