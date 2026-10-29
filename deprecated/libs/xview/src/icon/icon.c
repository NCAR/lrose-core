#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)icon.c 20.16 90/02/26";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * icon.c - Display icon.
 */

#include <stdio.h>
/* #include <pixrect/pixrect_hs.h> */
#include <xview_private/i18n_impl.h>
#include <xview/frame.h>
#include <xview/xview.h>
#include <xview/rect.h>
#include <xview/rectlist.h>
#include <xview/pixwin.h>
#include <xview/font.h>
#include <xview_private/icon_impl.h>
#include <xview_private/svrim_impl.h>


#ifdef OW_I18N
static void DrawWCString();
#else
static void DrawString();
#endif
static void FillRect();
static int DrawNonRectIcon();
static void DrawTransparentIcon();
static void icon_draw_label();


Xv_private void
icon_display(icon_public, x, y)
    Icon            icon_public;
    register int    x, y;
{
    extern Pixrect             *frame_bkgrd;
    register Xv_icon_info      *icon = ICON_PRIVATE(icon_public);
    register Xv_Window         pixwin = icon_public;
    register Xv_Drawable_info  *info;
    register Display           *display;
    register XID               xid;
    
    DRAWABLE_INFO_MACRO( pixwin, info );
    display = xv_display( info );
    xid = (XID) xv_xid(info);

    if ( icon->ic_mask )  {   /* we have a icon mask to use */
        FillRect( pixwin, icon->workspace_pixel,
		 icon->ic_gfxrect.r_left, icon->ic_gfxrect.r_top,
		 icon->ic_gfxrect.r_width, icon->ic_gfxrect.r_height);	
	DrawNonRectIcon( display, xid, icon, info, x, y );
    } else {
	if (icon->ic_mpr ) {
	    if ( icon->ic_flags & ICON_BKGDTRANS ) 	
	        DrawTransparentIcon( icon, pixwin, x, y, icon->workspace_pixel );
	    else
	        (void) xv_rop(pixwin,
		      icon->ic_gfxrect.r_left + x, icon->ic_gfxrect.r_top + y,
		      icon->ic_gfxrect.r_width, icon->ic_gfxrect.r_height,
		      PIX_SRC, icon->ic_mpr, 0, 0);
	}
    }
#ifdef OW_I18N
    if (icon->ic_text_wcs && (icon->ic_text_wcs[0] != '\0'))
#else  
    if (icon->ic_text && (icon->ic_text[0] != '\0')) 
#endif
	icon_draw_label( icon, pixwin, info, x, y, icon->workspace_pixel );
    icon->ic_flags |= ICON_PAINTED;
}


static void
icon_draw_label( icon, pixwin, info, x, y, wrk_space_pixel )
register Xv_icon_info      *icon;
register Xv_Window         pixwin;
register int             x, y;
register Xv_Drawable_info  *info;
unsigned long              wrk_space_pixel;
{	
    PIXFONT        *font = (PIXFONT *) xv_get(pixwin, WIN_FONT);
    int            left, top, line_leading = xv_get((Xv_opaque)font, FONT_DEFAULT_CHAR_HEIGHT);
#ifdef OW_I18N
    XFontSet       font_set;
    Display        *dpy;
    XRectangle     overall_ink_extents = {0};
    XRectangle     overall_logical_extents = {0};
#else  
    XFontStruct    *x_font_info;
#endif
    int            descent = 0;
    int            ascent = 0;
    int            direction = 0;
    XCharStruct    overall_return;
    struct rect    textrect;
	
    /*
     * Initialize overall_return to zeros
     * It is not initialized like overall_ink_extents above because the MIT 
     * build (using cc), complains about "no automatic aggregate initialization"
     */
    XV_BZERO(&overall_return, sizeof(XCharStruct));

    if (rect_isnull(&icon->ic_textrect)) 
        /* Set text rect to accomodate 1 line at bottom. */
        rect_construct(&icon->ic_textrect,
		       0, icon->ic_gfxrect.r_height - line_leading,
		       icon->ic_gfxrect.r_width, line_leading);

    if ( (icon->ic_flags & ICON_BKGDTRANS) || icon->ic_mask ) {
	if ( !( icon->ic_flags & ICON_TRANSLABEL) )  /*check for transparent label*/
            FillRect( pixwin, wrk_space_pixel,
		 icon->ic_textrect.r_left + x, icon->ic_textrect.r_top + y - 3,
		 icon->ic_textrect.r_width, icon->ic_textrect.r_height + 3);	
    }
    else
        /* Blank out area onto which text will go. */
        (void) xv_rop(pixwin,
		      icon->ic_textrect.r_left + x, icon->ic_textrect.r_top + y-3,
		      icon->ic_textrect.r_width, icon->ic_textrect.r_height+3,
		      PIX_CLR, (Pixrect *)NULL, 0, 0 );

    /* Format text into textrect */
    textrect = icon->ic_textrect;
    textrect.r_left += x;
    textrect.r_top += y;

#ifdef OW_I18N
    font_set = (XFontSet) xv_get((Xv_opaque)font, FONT_SET_ID);
    dpy = xv_display( info );
 
    XwcTextExtents(font_set, icon->ic_text_wcs, wslen(icon->ic_text_wcs),
	&overall_ink_extents, &overall_logical_extents);
    left = (int)(icon->ic_gfxrect.r_width - overall_logical_extents.width)/2;
    if (left < 0)
        left = 0;
    top = textrect.r_top - overall_logical_extents.y - 3;
#else /* OW_I18N */
    x_font_info = (XFontStruct *) xv_get((Xv_opaque)font, FONT_INFO );

    (void) XTextExtents( x_font_info, icon->ic_text, strlen( icon->ic_text ),
			&direction, &ascent, &descent, &overall_return );

    left = (icon->ic_gfxrect.r_width - overall_return.width)/2;
    if (left < 0)  
	left = 0;

    top = textrect.r_top + x_font_info->ascent -3;
#endif

    if ( (icon->ic_flags & ICON_BKGDTRANS) || icon->ic_mask )
#ifdef OW_I18N
	DrawWCString(pixwin,xv_fg(info),wrk_space_pixel,
		   left,top,font_set,icon->ic_text_wcs);
#else
        DrawString(pixwin, xv_fg(info), wrk_space_pixel,
		   left, top, font, icon->ic_text);
#endif
    else
#ifdef OW_I18N
    {
        GC              gc;
        Drawable        d;
 
        gc = xv_find_proper_gc(dpy, info, PW_TEXT);
        d = xv_xid( info );
        xv_set_gc_op(dpy, info, gc, PIX_SRC,
                PIX_OPCOLOR(PIX_SRC) ? XV_USE_OP_FG : XV_USE_CMS_FG,
                XV_DEFAULT_FG_BG);
        (void) XwcDrawString(dpy, d, font_set, gc,
                left, top, icon->ic_text_wcs, wslen(icon->ic_text_wcs));
    }
#else
        (void) xv_text(pixwin, left, top, PIX_SRC, (Xv_opaque)font, icon->ic_text);
#endif
    
}
    
#ifdef OW_I18N
static void
DrawWCString( win, frg_pixel, bkg_pixel, x, y, font_set, str )
register Xv_Window  win;
unsigned long       frg_pixel, bkg_pixel;
register int        x, y;
XFontSet            font_set;
wchar_t            *str;
{
    register Xv_Drawable_info  *info;
    Display  *display;
    XID      xid;
    GC       gc;
    XGCValues  val;
    unsigned long  val_mask;
    
    DRAWABLE_INFO_MACRO( win, info );
    display = xv_display( info );
    xid = (XID) xv_xid(info);

    gc = xv_find_proper_gc( display, info, PW_TEXT );
    val.function = GXcopy;
    val.foreground = frg_pixel;
    val.background = bkg_pixel;
    val.clip_mask = None;
    val_mask = GCBackground | GCForeground | GCClipMask;
    XChangeGC(display, gc, val_mask, &val );

    XwcDrawString( display, xid, font_set, gc, x, y, str, wslen(str) );
}
#else 
static void
DrawString( win, frg_pixel, bkg_pixel, x, y, pixfont, str )
register Xv_Window  win;
unsigned long       frg_pixel, bkg_pixel;
register int        x, y;
Xv_opaque           pixfont;
char                *str;
{
    register Xv_Drawable_info  *info;
    Display  *display;
    XID      xid, font;
    GC       gc;
    XGCValues  val;
    unsigned long  val_mask;
    
    DRAWABLE_INFO_MACRO( win, info );
    display = xv_display( info );
    xid = (XID) xv_xid(info);
    font = (XID) xv_get( pixfont, XV_XID );

    gc = xv_find_proper_gc( display, info, PW_TEXT );
    val.function = GXcopy;
    val.foreground = frg_pixel;
    val.background = bkg_pixel;
    val.clip_mask = None;
    val_mask = GCBackground | GCForeground | GCClipMask;
    XChangeGC(display, gc, val_mask, &val );
    XSetFont(display, gc, font );

    XDrawString( display, xid, gc, x, y, str, strlen(str) );
}
#endif


static void
FillRect( win, bkg_pixel, x, y, w, h )
register Xv_Window win;
unsigned long      bkg_pixel;
register int  x, y, w, h;
{
    register Xv_Drawable_info  *info;
    Display  *display;
    XID      xid;
    GC       gc;
    XGCValues  val;
    unsigned long  val_mask;
    
    DRAWABLE_INFO_MACRO( win, info );
    display = xv_display( info );
    xid = (XID) xv_xid(info);

    gc = xv_find_proper_gc( display, info, PW_ROP );
    val.function = GXcopy;
    val.foreground = bkg_pixel;
    val.fill_style = FillSolid;
    val.clip_mask = 0;
    val_mask = GCClipMask | GCFillStyle | GCForeground | GCFunction;
    XChangeGC(display, gc, val_mask, &val );
    XFillRectangle( display, xid, gc, x, y, w, h );
}


static void
DrawTransparentIcon( icon, pixwin, x, y, bkg_color )
register Xv_icon_info    *icon;
register Xv_Window       pixwin;
register int             x, y;
unsigned long            bkg_color;
{
    register Xv_Drawable_info  *info, *src_info;
    Display  *display;
    XID      xid;
    GC       gc;
    XGCValues  val;
    unsigned long  val_mask;
    
    DRAWABLE_INFO_MACRO( pixwin, info );
    display = xv_display( info );
    xid = (XID) xv_xid(info);
    
	
    DRAWABLE_INFO_MACRO( (Xv_opaque) icon->ic_mpr, src_info );
    gc = xv_find_proper_gc( display, info, PW_ROP );
    val.function = GXcopy;
    val.plane_mask = xv_plane_mask(info);
    val.background = bkg_color;
    val.foreground = xv_fg(info);
    val.stipple = xv_xid(src_info);
    val.fill_style = FillOpaqueStippled;
    val.ts_x_origin = 0;
    val.ts_y_origin = 0;	
    val_mask = GCForeground | GCBackground | GCFunction | 
               GCPlaneMask | GCFillStyle | GCTileStipXOrigin | 
               GCTileStipYOrigin | GCStipple;
	
    XChangeGC(display, gc, val_mask, &val );
    XFillRectangle( display, xid, gc, icon->ic_gfxrect.r_left + x,
		   icon->ic_gfxrect.r_top + y,
		   icon->ic_gfxrect.r_width, icon->ic_gfxrect.r_height );
}

static int
DrawNonRectIcon( display, xid, icon, info, x, y )
Display                  *display;
XID                      xid;
register Xv_icon_info    *icon;
register Xv_Drawable_info  *info;
register int             x, y;
{
    register Xv_Drawable_info  *src_info, *mask_info;
    GC       gc;
    XGCValues  val;
    unsigned long  val_mask;

    DRAWABLE_INFO_MACRO( (Xv_opaque) icon->ic_mask, mask_info );
    gc = xv_find_proper_gc( display, info, PW_ROP );

    val.function = GXcopy;
    val.plane_mask = xv_plane_mask(info);
    val.background = xv_bg(info);
    val.foreground = xv_fg(info);

    val.fill_style = FillOpaqueStippled;
    val.ts_x_origin = 0;
    val.ts_y_origin = 0;	
    val_mask = GCForeground | GCBackground | GCFunction | 
               GCPlaneMask | GCTileStipXOrigin | 
               GCTileStipYOrigin;
    XChangeGC(display, gc, val_mask, &val );

    if (PR_NOT_MPR(((Pixrect *) icon->ic_mpr)))  {
	DRAWABLE_INFO_MACRO( (Xv_opaque) icon->ic_mpr, src_info );

	/* stipple only if we have a bitmap icon */
	if (xv_depth(src_info) == 1) {
    		val.stipple = xv_xid(src_info);
		val.fill_style = FillOpaqueStippled;
		val_mask = GCFillStyle | GCStipple;
	} else if (xv_depth(info) == xv_depth(src_info)) {
		val.tile = xv_xid(src_info);
		val.fill_style = FillTiled;
		val_mask = GCFillStyle | GCTile;
    	} else {
		xv_error(XV_ZERO,
			ERROR_STRING,
		 	XV_MSG("icon: can't handle drawables of different depth"),
		 	NULL);
		return (XV_ERROR);
    	}	

	val.clip_mask = xv_xid(mask_info);
	val_mask |= GCClipMask;
	XChangeGC(display, gc, val_mask, &val );

	if ( xv_rop_internal( display, xid, gc, icon->ic_gfxrect.r_left + x,
			     icon->ic_gfxrect.r_top + y,
			     icon->ic_gfxrect.r_width, icon->ic_gfxrect.r_height,
			     (Xv_opaque) icon->ic_mpr, 0, 0, info ) == XV_ERROR) {
	    xv_error( XV_ZERO, ERROR_STRING, 
		XV_MSG("xv_rop: xv_rop_internal failed"), 0 );
	}
    }
    else {
	if (xv_rop_mpr_internal( display, xid, gc, icon->ic_gfxrect.r_left + x,
			     icon->ic_gfxrect.r_top + y,
			     icon->ic_gfxrect.r_width, icon->ic_gfxrect.r_height,
			     icon->ic_mpr, 0, 0, info, TRUE) == XV_ERROR)
	return(XV_ERROR);
    }

    return XV_OK;
}
