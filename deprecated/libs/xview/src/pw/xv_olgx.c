#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_olgx.c 1.34 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <X11/X.h>
#ifdef OW_I18N
#include <xview/xv_i18n.h>
#endif /* OW_I18N */
#include <olgx/olgx.h>
#include <xview/defaults.h>
#include <xview/font.h>
#include <xview/screen.h>
#include <xview/window.h>
#include <xview/cms.h>
#include <xview_private/draw_impl.h>

#ifdef MONO3D
#include <images/bg1.xbm>
#include <images/bg2.xbm>
#include <images/bg3.xbm>
#endif


typedef struct ginfo_list_struct {
    Cms		    cms;
    int		    depth;
    Graphics_info  *ginfo;
    Xv_Font	    glyph_font;
    Xv_Font	    text_font;
    struct ginfo_list_struct  *next;
} Ginfo_list;


Xv_private Cms
xv_set_control_cms(window_public, info, cms_status)
    Xv_Window		window_public;
    Xv_Drawable_info   *info;
    int			cms_status;
{
    Cms			    cms;
    char		    cms_name[25];
    Xv_singlecolor	   *cms_colors;
    Xv_singlecolor	    color; 
    Visual		   *visual;

    /* Copy the foreground color from the current CMS to the new
     * Control CMS.
     */
    if (CMS_STATUS(cms_status, CMS_STATUS_DEFAULT)) {
	color.red = color.green = color.blue = 0;
    } else {
	cms_colors = (Xv_singlecolor *) xv_malloc(sizeof(Xv_singlecolor) *
	    xv_get(xv_cms(info), CMS_SIZE));
	(void) xv_get(xv_cms(info), CMS_COLORS, cms_colors);
	    /* ... stores result in cms_colors */
	color.red = cms_colors[1].red;
	color.green = cms_colors[1].green;
	color.blue = cms_colors[1].blue;
	free(cms_colors);
    }
    visual = (Visual *)xv_get(window_public, XV_VISUAL, NULL);
    CMS_CONTROL_CMS_NAME(cms_name, visual, color);
    cms = (Cms)xv_find(xv_screen(info), CMS,
		CMS_NAME, cms_name,
		CMS_CONTROL_CMS, TRUE,
		CMS_SIZE, CMS_CONTROL_COLORS + 1,
		CMS_COLORS, &color,
		XV_VISUAL, visual,
		NULL);

    if ( cms )
      /* Set the window's CMS to the new Control CMS */
      xv_set(window_public, WIN_CMS, cms, NULL);

    return ( cms );
}


Xv_private Graphics_info *
xv_init_olgx(win, three_d, text_font)
    register Xv_Window win;
    int		   *three_d;	/* TRUE: 3D, FALSE: 2D.  May be modified. */
    Xv_Font	    text_font;
{
    Cms		    cms;
    unsigned long   cms_size;
    int		    cms_status;
    Display	   *display;
    Ginfo_list	   *first;
    Ginfo_list	   *ginfo_list;
    Xv_Font	    glyph_font;
    XFontStruct	   *glyph_font_struct;
    unsigned long  *index_table;
    Xv_Drawable_info *info;
    Ginfo_list	   *last;
    int		    control_cms = FALSE;
    unsigned long   pixvals[OLGX_NUM_COLORS];
    Xv_Screen	    screen;
    int		    screen_number;
    Pixmap	    stipple_pixmaps[3];
#ifdef OW_I18N
    XFontSet	    font_set;
#else
    XFontStruct	   *text_font_struct;
#endif /* OW_I18N */
    int		    three_d_state;

    /*
     * Initialize the OPEN LOOK Graphics Library
     */
    DRAWABLE_INFO_MACRO(win, info);
    cms = xv_cms(info);

    if (xv_depth(info) > 1 && *three_d) {
	cms_status = (int)xv_get(cms, CMS_STATUS_BITS);
	*three_d = CMS_STATUS(cms_status, CMS_STATUS_DEFAULT) ||
		   CMS_STATUS(cms_status, CMS_STATUS_FRAME) ||
		   CMS_STATUS(cms_status, CMS_STATUS_CONTROL);
    }

    if (xv_depth(info) > 1 && *three_d) {
	if (!CMS_STATUS(cms_status, CMS_STATUS_CONTROL)) {
	    cms = xv_set_control_cms(win, info, cms_status);
	    if ( cms ) {
		control_cms = TRUE;
		index_table = (unsigned long *)xv_get(cms, CMS_INDEX_TABLE);
		pixvals[OLGX_BLACK] = index_table[CMS_CONTROL_COLORS];
	    } else {
		*three_d = FALSE;
		cms = xv_cms(info);
	    }
	} else {
	    control_cms = TRUE;
	    cms_size = (unsigned long)xv_get(cms, CMS_SIZE);
	    index_table = (unsigned long *)xv_get(cms, CMS_INDEX_TABLE);
	    pixvals[OLGX_BLACK] = index_table[cms_size - 1];
	    xv_set(win, 
	        WIN_FOREGROUND_COLOR, cms_size - 1,
		WIN_BACKGROUND_COLOR, 0,
		NULL);
	}
    }

    if ( control_cms ) {
        pixvals[OLGX_BG1]   = index_table[CMS_CONTROL_BG1];
        pixvals[OLGX_BG2]   = index_table[CMS_CONTROL_BG2];
        pixvals[OLGX_BG3]   = index_table[CMS_CONTROL_BG3];
        pixvals[OLGX_WHITE] = index_table[CMS_CONTROL_HIGHLIGHT];
    } else {	
   	/*
       	 * Either monochrome, or application set a non-control
         * cms on the panel - default to 2D.
         */
        pixvals[OLGX_WHITE] = xv_bg(info);
        pixvals[OLGX_BLACK] = xv_fg(info);
        pixvals[OLGX_BG1]   = pixvals[OLGX_BLACK];
        pixvals[OLGX_BG2]   = pixvals[OLGX_BLACK];
        pixvals[OLGX_BG3]   = pixvals[OLGX_BLACK];
    }
    
    /* See if a Graphics Info has already been created for this CMS.
     * If so, use that one.  If not, create a new one and add it to
     * the ginfo_list.
     */

#ifdef OW_I18N
    text_font = (Xv_Font) xv_get(win, XV_FONT);
    font_set = (XFontSet) xv_get(text_font, FONT_SET_ID);
#endif /* OW_I18N */

    glyph_font = xv_get(win, WIN_GLYPH_FONT);
#ifdef OW_I18N
    glyph_font_struct = (XFontStruct *) xv_get(glyph_font, FONT_INFO);
#endif /* OW_I18N */
    screen = xv_screen(info);
    first = (Ginfo_list *) xv_get(screen, XV_KEY_DATA, SCREEN_GINFO);
    last = first;
    for (ginfo_list = first; ginfo_list; ginfo_list = ginfo_list->next) {
	if (ginfo_list->cms == cms &&
	    ginfo_list->depth == xv_depth(info) &&
#ifdef OW_I18N
	    TextFont_Set(ginfo_list->ginfo) == font_set &&
	    ginfo_list->ginfo->glyphfont == glyph_font_struct)
#else
	    ginfo_list->glyph_font == glyph_font &&
	    ginfo_list->text_font == text_font)
#endif /* OW_I18N */
	{
	    *three_d = (int)xv_get(cms, CMS_CONTROL_CMS, NULL);
	    return (ginfo_list->ginfo);
	}
	last = ginfo_list;
    }
    ginfo_list = xv_alloc(Ginfo_list);
    ginfo_list->cms = cms;
    ginfo_list->depth = xv_depth(info);
    ginfo_list->glyph_font = glyph_font;
    ginfo_list->text_font = text_font;
    if (last)
	last->next = ginfo_list;
    else
	xv_set(screen, XV_KEY_DATA, SCREEN_GINFO, ginfo_list, NULL);

    display = xv_display(info);
    screen_number = (int) xv_get(screen, SCREEN_NUMBER);
    if (*three_d) {
	if (xv_depth(info) > 1)
	    three_d_state = OLGX_3D_COLOR;
	else
#ifdef MONO3D
	    three_d_state = OLGX_3D_MONO;
#else
	    three_d_state = OLGX_2D;
#endif
    } else
	three_d_state = OLGX_2D;

#ifndef OW_I18N
    text_font_struct = (XFontStruct *) xv_get(text_font, FONT_INFO);
    glyph_font_struct = (XFontStruct *) xv_get(glyph_font, FONT_INFO);
#endif /* OW_I18N */

#ifdef MONO3D
    if (three_d_state == OLGX_3D_MONO) {
	XID	    xid;

	xid = xv_xid(info);
	int fg = BlackPixel(display, screen_number);
	int bg = WhitePixel(display, screen_number);
	stipple_pixmaps[0] = (Pixmap) xv_get(screen, XV_KEY_DATA,
					     SCREEN_BG1_PIXMAP);
	stipple_pixmaps[1] = (Pixmap) xv_get(screen, XV_KEY_DATA,
					     SCREEN_BG2_PIXMAP);
	stipple_pixmaps[2] = (Pixmap) xv_get(screen, XV_KEY_DATA,
					     SCREEN_BG3_PIXMAP);
	if (stipple_pixmaps[0] == (Pixmap) NULL) {
	    stipple_pixmaps[0] = XCreatePixmapFromBitmapData(display,
		xid, bg1_bits, bg1_width, bg1_height, fg, bg, 1);
	    xv_set(screen,
		   XV_KEY_DATA, SCREEN_BG1_PIXMAP, stipple_pixmaps[0],
		   NULL);
	}
	if (stipple_pixmaps[1] == (Pixmap) NULL) {
	    stipple_pixmaps[1] = XCreatePixmapFromBitmapData(display,
		xid, bg2_bits, bg2_width, bg2_height, fg, bg, 1);
	    xv_set(screen,
		   XV_KEY_DATA, SCREEN_BG2_PIXMAP, stipple_pixmaps[1],
		   NULL);
	}
	if (stipple_pixmaps[2] == (Pixmap) NULL) {
	    stipple_pixmaps[2] = XCreatePixmapFromBitmapData(display,
		xid, bg3_bits, bg3_width, bg3_height, fg, bg, 1);
	    xv_set(screen,
		   XV_KEY_DATA, SCREEN_BG3_PIXMAP, stipple_pixmaps[2],
		   NULL);
	}
	olgx_set_stipple_pixmaps(ginfo, stipple_pixmaps);
	xv_set(win, WIN_BACKGROUND_PIXMAP, stipple_pixmaps[0], NULL); 
    }
#endif
#ifdef OW_I18N
    ginfo_list->ginfo = olgx_i18n_initialize(display, screen_number,
					     xv_depth(info),
					     three_d_state, 
					     glyph_font_struct, font_set, 
					     pixvals,
					     stipple_pixmaps);
#else
    ginfo_list->ginfo = olgx_main_initialize(display, screen_number, xv_depth(info),
					     three_d_state, 
					     glyph_font_struct, text_font_struct, 
					     pixvals,
					     stipple_pixmaps);
#endif /* OW_I18N */
    return (ginfo_list->ginfo);
}
