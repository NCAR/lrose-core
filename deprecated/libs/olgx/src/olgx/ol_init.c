/*#ident "@(#)ol_init.c	1.60 93/06/28 SMI" */
/*
 * Copyright 1990 Sun Microsystems
 */

/*
 * OPEN LOOK object drawing package Sun Microsystems, Inc.,
 */

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <olgx_private/olgx_impl.h>
#include <olgx_private/busy.h>
#include <olgx_private/grey.h>

/*
 * olgx_real_main_initialize() Mallocs a ginfos struct,initilaises it
 * with the respective values passed and returns a pointer to the
 * ginfo struct
 */

/* ARGSUSED */
#ifdef OW_I18N
static Graphics_info  *
olgx_real_main_initialize(fs_or_not, dpy, screen, depth, d_flag,
			  glyphfont_struct, utextfont, pixvals,
			  stipple_pixmaps)
    Bool            fs_or_not;
#else
Graphics_info  *
olgx_main_initialize(dpy, screen, depth, d_flag, glyphfont_struct,
		     textfont_struct, pixvals, stipple_pixmaps)
#endif
    Display        *dpy;
    int             screen;
    unsigned int    depth;
    int             d_flag;
    XFontStruct    *glyphfont_struct;
#ifdef OW_I18N
    Olgx_font_or_fs utextfont;
#else
    XFontStruct    *textfont_struct;
#endif
    unsigned long   pixvals[];
    Pixmap          stipple_pixmaps[];

{
    Graphics_info  *info;
    int             direct, ascent, descent;
    XCharStruct     overall;
    char            string[2];
    int             i;
    int             num_colors;


    /* Malloc a ginfo struct  */

    info = (Graphics_info *) malloc(sizeof(Graphics_info));


    /*
     * Initialise the dpy,screen,d_flag,glyph_font,text_font infos and
     * pixvals
     */

    info->dpy = dpy;
    info->scrn = screen;
    info->three_d = d_flag;
    info->glyphfont = glyphfont_struct;
#ifdef OW_I18N
    /*
     * Since, compiler does not allow me to use Olgx_flags(info) as lvalue.
     *	"ol_init.c", line 72: left operand must be modifiable lvalue: op "="
     */
    info->_Olgx_Flags = fs_or_not ? OLGX_FONTSET : 0;
    if (Olgx_Flags(info) & OLGX_FONTSET)
	info->textfontset = utextfont.fontset;
    else
	info->textfont = utextfont.fontstruct;
#else
    info->textfont = textfont_struct;
#endif

    /*
     * OLGX_3D_MONO has been nuked.. this space inthe data strcuture is used
     * for storing the drawable associated with the Ginfo
     * info->stipple_pixmaps[3] has been changed to info->drawable[3] and
     * info->drawable[0] is used for storing the drawable. This is to avoid
     * binary compatiblity problem between 2.0 and 3.0
     * 
     * if (d_flag == OLGX_3D_MONO) { info->stipple_pixmaps[0] =
     * stipple_pixmaps[0]; info->stipple_pixmaps[1] = stipple_pixmaps[1];
     * info->stipple_pixmaps[2] = stipple_pixmaps[2]; }
     * 
     */

    num_colors = (d_flag) ? OLGX_NUM_COLORS : 2;

    for (i = 0; i < num_colors; i++)
	info->pixvals[i] = pixvals[i];

    /* Set the depth to the passed depth */


    info->depth = depth;	/* depth is cached for checking the GC pool
				 * to get the GC of the correct depth */


     /* Feb-25/91. we are passing the depth instead of drawable to create a ginfo.
      * With the given depth, we create a 1x1 pixmap, which is used to create
      * all the GCs. This solves a particular problem for XView,whereby 
      * the same Ginfo can be used even thpugh the XID which created the
      * Ginfo is no longer there.
      */

    info->drawable[0] = XCreatePixmap(dpy,RootWindow(dpy,screen),1,1,depth);
    if (!info->drawable[0]) 
	olgx_error("olgx:Unable to create Pixmap of size 1x1\n");
    

    /*
     * Initialise the gcrec's with proper values only the ones which are
     * needed immediately (sp?)
     */

    olgx_initialise_gcrec(info, OLGX_WHITE);
    olgx_initialise_gcrec(info, OLGX_BLACK);

    if (info->three_d) {

	olgx_initialise_gcrec(info, OLGX_BG1);
	olgx_initialise_gcrec(info, OLGX_BG2);
	olgx_initialise_gcrec(info, OLGX_BG3);

    }
    for (i = (info->three_d) ? 5 : 2; i < 9; i++)
	info->gc_rec[i] = NULL;


    /*
     * Now initialise all the OPEN LOOK values associted with the glyph font
     */

    /* get the button height from the size of the endcaps */

    string[0] = BUTTON_UL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->button_height = (overall.ascent + overall.descent);
    info->endcap_width = overall.width;

    /* get the elevator height and width  and the abbsb height */

    string[0] = VERT_SB_UL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->sb_height = (overall.ascent + overall.descent);
    info->sb_width = overall.width;
    info->abbsb_height = info->sb_height - ((info->sb_height - 2) / 3);

    /*
     * get the abbreviated menu button  width The height can be calculated
     * from the width
     */

    string[0] = ABBREV_MENU_UL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->abbrev_width = overall.width;

    /* get the slider height and width */

    string[0] = HORIZ_SLIDER_CONTROL_UL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->slider_height = (overall.ascent + overall.descent);
    info->slider_width = overall.width;

    /* get the menu mark height and width */

    string[0] = HORIZ_MENU_MARK_FILL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->mm_height = (overall.ascent + overall.descent);
    info->mm_width = overall.width;

    /* get the slider height and width */

    string[0] = HORIZ_SLIDER_UL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->se_height = (overall.ascent + overall.descent);
    info->se_width = overall.width;

    /*
     * get the gauge height and width -endcap And the offset dist from the
     * endcap to the inner channel
     */

    string[0] = HORIZ_GAUGE_LEFT_ENDCAP_OUTLINE;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->gauge_height = (overall.ascent + overall.descent);
    info->gauge_width = overall.width;
    info->gauge_endcapOffset = (info->gauge_height < 17) ?
	(info->gauge_height - 3) : 13;

    /* get the base_off height */

    string[0] = BASE_OFF_SPECIALCHAR;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->base_off = (overall.ascent + overall.descent);

    /* get the slider_offset height */

    string[0] = SLIDER_CHANNEL_OFFSET;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->slider_offset = overall.width;

    /* get the push pin height and width */

    string[0] = PUSHPIN_OUT_BOTTOM;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    /*
     * 1 has been added to get the correct rect height works with all point
     * sizes
     */

    info->pp_height = overall.ascent + overall.descent + 1;
    info->pp_width = overall.width;

    /* get the check box height and width */

    string[0] = UNCHECKED_BOX_UL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->cb_height = (overall.ascent + overall.descent);
    info->cb_width = overall.width;

    /*
     * get the texts scroll button width height can be got from the width
     */

    string[0] = TEXTSCROLLBUTTON_LEFT;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->textscbu_width = (overall.ascent + overall.descent);

    /* Get the numscroll button width from textscroll button width  */

    info->numscbu_width = (info->textscbu_width << 1) - ((info->three_d) ? 0:2);


    /*
     * get the reisize arm width the height is the same as the width
     */

    info->resize_arm_width = (info->pp_height > 14) ? 6 : 5;


    /* Get the Point size of the Glyph Font */

    info->point_size = Abbrev_MenuButton_Width(info) -
	((Abbrev_MenuButton_Width(info) > 20) ? 2 : 4);


    /* Drop Target dimensions */

    if (info->point_size < 13) { /* 10 & 12 */
      info->dtarget_height = info->point_size + 9;
      info->dtarget_width  = info->point_size + 4;
      info->dtarget_ewidth = 3;
      info->dtarget_swidth = 1; 
    }

    else if (info->point_size == 14) { /* 14 */
      info->dtarget_height = info->point_size + 9;
      info->dtarget_width  = info->point_size + 3;
      info->dtarget_ewidth = 4;
      info->dtarget_swidth = 1; 
    }else { /* 19 */
      info->dtarget_height = info->point_size + 11;
      info->dtarget_width  = info->point_size + 3;
      info->dtarget_ewidth = 5;
      info->dtarget_swidth = 1; 
    }
      
    
    /* Store the Scrollbar Cable info */

    info->cable_offset = ((ScrollbarElevator_Width(info) - 3) >> 1) -
	((info->three_d) ? 1 : 0);

    info->cable_width = (info->cable_offset & 8) ? 5 : 3;


    /* return the created ginfo */

    return info;

}


/*
 * olgx_initialize() is present only for backward compatiblity reasons..
 */

Graphics_info  *
olgx_initialize(dpy, screen, d_flag, glyphfont_struct,
		textfont_struct, pixvals, stipple_pixmaps)
    Display        *dpy;
    int             screen;
    int             d_flag;
    XFontStruct    *glyphfont_struct;
    XFontStruct    *textfont_struct;
    unsigned long   pixvals[];
    Pixmap          stipple_pixmaps[];

{
    return (olgx_main_initialize(dpy, screen, DefaultDepth(dpy, screen), d_flag,
				 glyphfont_struct, textfont_struct, pixvals,
				 stipple_pixmaps));

}


#ifdef OW_I18N
Graphics_info *
olgx_i18n_initialize(dpy, screen, depth, d_flag, glyphfont_struct,
		     textfont_set, pixvals, stipple_pixmaps)
    Display        *dpy;
    int             screen;
    unsigned int    depth;
    int             d_flag;
    XFontStruct    *glyphfont_struct;
    XFontSet        textfont_set;
    unsigned long   pixvals[];
    Pixmap          stipple_pixmaps[];
{
    Olgx_font_or_fs utextfont;

    utextfont.fontset = textfont_set;
    return olgx_real_main_initialize(True, dpy, screen, depth, d_flag,
				     glyphfont_struct, utextfont, pixvals,
				     stipple_pixmaps);
}

    
Graphics_info *
olgx_main_initialize(dpy, screen, depth, d_flag, glyphfont_struct,
		     textfont_struct, pixvals, stipple_pixmaps)
    Display        *dpy;
    int             screen;
    unsigned int    depth;
    int             d_flag;
    XFontStruct    *glyphfont_struct;
    XFontStruct    *textfont_struct;
    unsigned long   pixvals[];
    Pixmap          stipple_pixmaps[];
{
    Olgx_font_or_fs utextfont;

    utextfont.fontstruct = textfont_struct;
    return olgx_real_main_initialize(False, dpy, screen, depth, d_flag,
				     glyphfont_struct, utextfont,
				     pixvals, stipple_pixmaps);
}
#endif /* OW_I18N */
    

/*
 * olgx_set_glyph_font(info, font_info,flag)
 * 
 * Sets the glyph font associated with the given Graphics_info structure, and
 * determines the sizes of certain OPEN LOOK items associated with that font.
 */

void
olgx_set_glyph_font(info, font_info, flag)
    Graphics_info  *info;
    XFontStruct    *font_info;
    int             flag;
{
    int             direct, ascent, descent;
    XCharStruct     overall;
    char            string[2];
    register int    i;
    register int    num_colors;
    GC_rec         *existing_gcrec;
    per_disp_res_ptr perdispl_res_ptr;
    XGCValues       values;

    /* don't bother setting this information if it's already valid */

    if (olgx_cmp_fonts(font_info, info->glyphfont)) {
	return;
    }
    /* set the font info */

    info->glyphfont = font_info;

    if (flag & OLGX_SPECIAL) {

	/*
	 * Don't worry if the GC 's are shared or not.. Change them by brute
	 * force
	 */

	num_colors = (info->three_d) ? OLGX_NUM_COLORS : 2;

	for (i = 0; i < num_colors; i++) {

	    /* Need to change the values also */

	    XSetFont(info->dpy, info->gc_rec[i]->gc, font_info->fid);
	    info->gc_rec[i]->values.font = font_info->fid;

	}

    } else {

	/*
	 * If the GC's are shared,create a new GC with the new glyphfont and
	 * attach the new GC's to the ginfo
	 */

	perdispl_res_ptr = olgx_get_perdisplay_list(info->dpy, info->scrn);
	num_colors = (info->three_d) ? OLGX_NUM_COLORS : 2;

	for (i = 0; i < num_colors; i++) {
	    values = info->gc_rec[i]->values;
	    values.font = font_info->fid;
	    if (info->gc_rec[i]->ref_count > 1) {

		/*
		 * The GC is shared, so reduce the reference count and create
		 * a GC
		 */

		info->gc_rec[i]->ref_count -= 1;

		/* Create a GC */

		info->gc_rec[i] = olgx_get_gcrec(perdispl_res_ptr, 
                                                 info->drawable[0], info->depth,
						 info->gc_rec[i]->valuemask,
						 &values);

	    } else {

		/*
		 * The GC is not shared , Check if there is someother GC with
		 * the new values If NO Change the values of this GC to the
		 * new one Should take care of values also If YES Destroy
		 * this GC and attach that GC to the ginfo
		 */


		existing_gcrec = olgx_gcrec_available(perdispl_res_ptr,
						 info->gc_rec[i]->valuemask,
						      &values);

		if (!existing_gcrec)
		    XSetFont(info->dpy, info->gc_rec[i]->gc, 
                             info->glyphfont->fid);

		else {

		    if (existing_gcrec != info->gc_rec[i]) {

			olgx_destroy_gcrec(perdispl_res_ptr, info->gc_rec[i]);
			existing_gcrec->ref_count += 1;
			info->gc_rec[i] = existing_gcrec;

		    }
		}

	    }

	}
    }

    /*
     * Now initialise all the OPEN LOOK values associted with the glyph font
     */

    /* get the button height from the size of the endcaps */

    string[0] = BUTTON_UL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->button_height = (overall.ascent + overall.descent);
    info->endcap_width = overall.width;

    /* get the scrollbar height and width  and the abbsb height */

    string[0] = VERT_SB_UL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->sb_height = (overall.ascent + overall.descent);
    info->sb_width = overall.width;
    info->abbsb_height = info->sb_height - ((info->sb_height - 2) / 3);

    /*
     * get the abbreviated menu button  width The height can be calculated
     * from the width
     */

    string[0] = ABBREV_MENU_UL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->abbrev_width = overall.width;

    /* get the slider height and width */

    string[0] = HORIZ_SLIDER_CONTROL_UL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->slider_height = (overall.ascent + overall.descent);
    info->slider_width = overall.width;

    /* get the menu mark height and width */

    string[0] = HORIZ_MENU_MARK_FILL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->mm_height = (overall.ascent + overall.descent);
    info->mm_width = overall.width;

    /* get the slider height and width */

    string[0] = HORIZ_SLIDER_UL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->se_height = (overall.ascent + overall.descent);
    info->se_width = overall.width;

    /*
     * get the gauge height and width -endcap And the offset dist from the
     * endcap to the inner channel
     */

    string[0] = HORIZ_GAUGE_LEFT_ENDCAP_OUTLINE;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->gauge_height = (overall.ascent + overall.descent);
    info->gauge_width = overall.width;
    info->gauge_endcapOffset = (info->gauge_height < 17) ?
	(info->gauge_height - 3) : 13;

    /* get the base_off height */

    string[0] = BASE_OFF_SPECIALCHAR;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->base_off = (overall.ascent + overall.descent);

    /* get the slider_offset height */

    string[0] = SLIDER_CHANNEL_OFFSET;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->slider_offset = overall.width;

    /* get the push pin height and width */

    string[0] = PUSHPIN_OUT_BOTTOM;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    /*
     * 1 has been added to get the correct rect height works with all point
     * sizes
     */

    info->pp_height = overall.ascent + overall.descent + 1;
    info->pp_width = overall.width;

    /* get the check box height and width */

    string[0] = UNCHECKED_BOX_UL;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->cb_height = (overall.ascent + overall.descent);
    info->cb_width = overall.width;

    /*
     * get the texts scroll button width height can be got from the width
     */

    string[0] = TEXTSCROLLBUTTON_LEFT;
    XTextExtents(info->glyphfont, string, 1, &direct, &ascent, &descent,
		 &overall);
    info->textscbu_width = (overall.ascent + overall.descent);

    /* Get the numscroll button width from textscroll button width  */

    info->numscbu_width = (info->textscbu_width << 1) - 2;


    /*
     * get the reisize arm width the height is the same as the width
     */

    info->resize_arm_width = (info->pp_width > 14) ? 6 : 5;


    /* Get the Point size of the Glyph Font */

    info->point_size = Abbrev_MenuButton_Width(info) -
	((Abbrev_MenuButton_Width(info) > 20) ? 2 : 4);

    /* Store the Scrollbar Cable info */

    info->cable_offset = ((ScrollbarElevator_Width(info) - 3) >> 1) -
	((info->three_d) ? 1 : 0);
    info->cable_width = (info->cable_offset & 8) ? 5 : 3;


}


/*
 * olgx_set_text_font(info, font_info,flag)
 * 
 * Sets the text font associated with the given Graphics_info structure. Will
 * create the text GC if necessary.
 */

#ifdef OW_I18N
static void
olgx_real_set_text_font(info, ufont_info, flag)
    Graphics_info  *info;
    Olgx_font_or_fs ufont_info;
    int             flag;
#else
void
olgx_set_text_font(info, font_info, flag)
    Graphics_info  *info;
    XFontStruct    *font_info;
    int             flag;
#endif
{
    XGCValues       values;
    GC_rec         *existing_gcrec;
    per_disp_res_ptr perdispl_res_ptr;

    /*
     * We are making a major assumption that in 2D ,TEXTGC and TEXTGC_REV are
     * twins and any change should be accompanied by a similar change to the
     * other.
     */


    /*
     * Do a smart check whether the fonts are same by comparing the fields in
     * min_bounds and max_bounds return without doing anything if it is the
     * same
     */

#ifdef OW_I18N
    if (Olgx_Flags(info) & OLGX_FONTSET) {
        if (olgx_cmp_fontsets(ufont_info.fontset, info->textfontset))
	    return;

	info->textfontset = ufont_info.fontset;
    } else {
	if (olgx_cmp_fonts(ufont_info.fontstruct, info->textfont))
	    return;

	info->textfont = ufont_info.fontstruct;
    }
#else /* OW_I18N */
    if (olgx_cmp_fonts(font_info, info->textfont))
	return;

    info->textfont = font_info;
#endif /* OW_I18N */

    /*
     * Return if the TextGc has not been created yet
     */

    if (!info->gc_rec[OLGX_TEXTGC])
	return;
    if (flag & OLGX_SPECIAL) {

#ifdef OW_I18N
	if (! (Olgx_Flags(info) & OLGX_FONTSET)) {
	    /*
	     * Don't worry if the GC 's are shared or not.. Change
	     * them by brute force
	     */

	    XSetFont(info->dpy, info->gc_rec[OLGX_TEXTGC]->gc,
		     info->textfont->fid);

            /* Even if !three_d, OLGX_TEXTGC_REV doesn't necessarily exist,
             * so we only set the font, if it does. I don't know if this is
             * the right place to change it, but this fixes a bug reported
             * on alt.toolkits.xview.
             * martin-2.buck@student.uni-ulm.de
             */
#if 1
	    if (!info->three_d && info->gc_rec[OLGX_TEXTGC_REV])
#else
	    if (!info->three_d)
#endif
		/* Only 2d has TEXTGC_REV */
		XSetFont(info->dpy, info->gc_rec[OLGX_TEXTGC_REV]->gc,
			 info->textfont->fid);
	}
#else /* OW_I18N */
	/*
	 * Don't worry if the GC 's are shared or not.. Change them by brute
	 * force
	 */

	XSetFont(info->dpy, info->gc_rec[OLGX_TEXTGC]->gc, font_info->fid);

        /* See comment above.
         * martin-2.buck@student.uni-ulm.de
         */
#if 1
	if (!info->three_d && info->gc_rec[OLGX_TEXTGC_REV])
#else
	if (!info->three_d)
#endif
	    /* Only 2d has TEXTGC_REV */
          XSetFont(info->dpy, info->gc_rec[OLGX_TEXTGC_REV]->gc, font_info->fid);
#endif /* OW_I18N */

    } else { /* not special */

	/*
	 * If the GC's are shared,create a new GC with the new glyphfont and
	 * attach the new GC's to the ginfo
	 */

	perdispl_res_ptr = olgx_get_perdisplay_list(info->dpy, info->scrn);

	if (info->gc_rec[OLGX_TEXTGC]->ref_count > 1) {
	    /*
	     * The GC is shared, so reduce the reference count and create a
	     * GC
	     */

	    info->gc_rec[OLGX_TEXTGC]->ref_count -= 1;
	    values = info->gc_rec[OLGX_TEXTGC]->values;
#ifdef OW_I18N
	    if (Olgx_Flags(info) & OLGX_FONTSET) {
		info->gc_rec[OLGX_TEXTGC]
			= olgx_get_gcrec(perdispl_res_ptr, 
				info->drawable[0],info->depth,
				info->gc_rec[OLGX_TEXTGC]->valuemask & ~GCFont,
				&values);
	    } else {
		values.font = info->textfont->fid;
		info->gc_rec[OLGX_TEXTGC]
			= olgx_get_gcrec(perdispl_res_ptr,
				info->drawable[0], 
                                info->depth,
				info->gc_rec[OLGX_TEXTGC]->valuemask,
				&values);
	    }
#else
	    values.font = font_info->fid;
	    info->gc_rec[OLGX_TEXTGC] = olgx_get_gcrec(perdispl_res_ptr,
                                                       info->drawable[0], 
                                                       info->depth,
				       info->gc_rec[OLGX_TEXTGC]->valuemask,
						       &values);
#endif

	} else {

	    /*
	     * The GC is not shared , Check if there is someother GC with the
	     * new values If NO Change the values of this GC to the new one
	     * Should take care of values also If YES Destroy this GC and
	     * attach that GC to the ginfo
	     */

	    values = info->gc_rec[OLGX_TEXTGC]->values;
#ifdef OW_I18N
	    if (Olgx_Flags(info) & OLGX_FONTSET) {
		existing_gcrec = olgx_gcrec_available(perdispl_res_ptr,
			       info->gc_rec[OLGX_TEXTGC]->valuemask & ~GCFont,
			       &values);
	    } else {
		values.font = info->textfont->fid;
		existing_gcrec = olgx_gcrec_available(perdispl_res_ptr,
			       info->gc_rec[OLGX_TEXTGC]->valuemask,
			       &values);
	    }
#else /* OW_I18N */
	    values.font = font_info->fid;
	    existing_gcrec = olgx_gcrec_available(perdispl_res_ptr,
				       info->gc_rec[OLGX_TEXTGC]->valuemask,
						  &values);
#endif /* OW_I18N */

	    if (!existing_gcrec) {

		/* Nothing similar exists in the Pool */

#ifdef OW_I18N
		if (! (Olgx_Flags(info) & OLGX_FONTSET)) {
#endif
		XSetFont(info->dpy, info->gc_rec[OLGX_TEXTGC]->gc,
                         info->textfont->fid);

#ifdef OW_I18N
		}
#endif

	    } else {

		/*
		 * Something similar exists already inthe Pool So attach that
		 * GC to the ginfo and destroy the one we are looking into
		 * and increase the reference count of the added existing
		 * gc_rec
		 */

		if (existing_gcrec != info->gc_rec[OLGX_TEXTGC]) {

		    olgx_destroy_gcrec(perdispl_res_ptr, 
                                       info->gc_rec[OLGX_TEXTGC]);
		    existing_gcrec->ref_count += 1;
		    info->gc_rec[OLGX_TEXTGC] = existing_gcrec;
		}
	    }

	}
     if (!info->three_d) {

         if (!info->gc_rec[OLGX_TEXTGC_REV]){
            return;
         }
         if (info->gc_rec[OLGX_TEXTGC_REV]->ref_count > 1) { /* GC is shared */

                info->gc_rec[OLGX_TEXTGC_REV]->ref_count -= 1;
                values = info->gc_rec[OLGX_TEXTGC_REV]->values;
#ifdef OW_I18N
                if (Olgx_Flags(info) & OLGX_FONTSET) {
                    info->gc_rec[OLGX_TEXTGC_REV]
                        = olgx_get_gcrec(perdispl_res_ptr,
                            info->drawable[0],
                            info->depth,
                            info->gc_rec[OLGX_TEXTGC_REV]->valuemask & ~GCFont,
                            &values);
                } else {
                    values.font = info->textfont->fid;
                    info->gc_rec[OLGX_TEXTGC_REV]
                        = olgx_get_gcrec(perdispl_res_ptr,
                            info->drawable[0],
                            info->depth,
                            info->gc_rec[OLGX_TEXTGC_REV]->valuemask,
                            &values);
                }
#else /* OW_I18N */
                values.font = font_info->fid;
                info->gc_rec[OLGX_TEXTGC_REV] = olgx_get_gcrec(perdispl_res_ptr,
                                                               info->drawable[0],
                                                               info->depth,
                                   info->gc_rec[OLGX_TEXTGC_REV]->valuemask,
                                                               &values);
#endif /* OW_I18N */
 
         } else {

	    /*
	     * The GC is not shared , Check if there is someother GC with the
	     * new values If NO Change the values of this GC to the new one
	     * Should take care of values also If YES Destroy this GC and
	     * attach that GC to the ginfo
	     */

	    values = info->gc_rec[OLGX_TEXTGC_REV]->values;
#ifdef OW_I18N
	    if (Olgx_Flags(info) & OLGX_FONTSET) {
		existing_gcrec = olgx_gcrec_available(perdispl_res_ptr,
			       info->gc_rec[OLGX_TEXTGC_REV]->valuemask & ~GCFont,
			       &values);
	    } else {
		values.font = info->textfont->fid;
		existing_gcrec = olgx_gcrec_available(perdispl_res_ptr,
			       info->gc_rec[OLGX_TEXTGC_REV]->valuemask,
			       &values);
	    }
#else /* OW_I18N */
	    values.font = font_info->fid;
	    existing_gcrec = olgx_gcrec_available(perdispl_res_ptr,
				       info->gc_rec[OLGX_TEXTGC_REV]->valuemask,
						  &values);
#endif /* OW_I18N */

	    if (!existing_gcrec) {

		/* Nothing similar exists in the Pool */

#ifdef OW_I18N
		if (! (Olgx_Flags(info) & OLGX_FONTSET)) {
#endif
		XSetFont(info->dpy, info->gc_rec[OLGX_TEXTGC_REV]->gc,
                         info->textfont->fid);

#ifdef OW_I18N
		}
#endif

	    } else {

		/*
		 * Something similar exists already inthe Pool So attach that
		 * GC to the ginfo and destroy the one we are looking into
		 * and increase the reference count of the added existing
		 * gc_rec
		 */

		if (existing_gcrec != info->gc_rec[OLGX_TEXTGC_REV]) {

		    olgx_destroy_gcrec(perdispl_res_ptr, 
                                       info->gc_rec[OLGX_TEXTGC_REV]);
		    existing_gcrec->ref_count += 1;
		    info->gc_rec[OLGX_TEXTGC_REV] = existing_gcrec;

		}
	    }

	}
      }
    }


#ifdef OW_I18N
    if (! (Olgx_Flags(info) & OLGX_FONTSET)) {
#endif
    info->gc_rec[OLGX_TEXTGC]->values.font = info->textfont->fid;
    /* See comment above.
     * martin-2.buck@student.uni-ulm.de
     */
#if 1
    if (!info->three_d && info->gc_rec[OLGX_TEXTGC_REV])
#else
    if (!info->three_d)
#endif
	info->gc_rec[OLGX_TEXTGC_REV]->values.font = info->textfont->fid;
#ifdef OW_I18N
    }
#endif
}


#ifdef OW_I18N
void
olgx_set_text_fontset(info, font_set, flag)
    Graphics_info  *info;
    XFontSet        font_set;
    int             flag;
{
    Olgx_font_or_fs ufont;

    ufont.fontset = font_set;
    olgx_real_set_text_font(info, ufont, flag);
}


void
olgx_set_text_font(info, font_info, flag)
    Graphics_info  *info;
    XFontStruct    *font_info;
    int             flag;
{
    Olgx_font_or_fs ufont;

    ufont.fontstruct = font_info;
    olgx_real_set_text_font(info, ufont, flag);
}
#endif /* OW_I18N */


/*
 * olgx_error (string)
 * 
 * routine which prints out error messages Private Routine
 * 
 */

void
olgx_error(string)
    char           *string;
{
    (void) fprintf(stderr, "olgx Error:  %s\n", string);

/*
 * ifdef because EXIT_FAILURE is not defined anywhere on
 * SunOS4.x
 */
#ifdef SVR4
    (void)exit(EXIT_FAILURE);
#else
    (void)exit(1);
#endif /* SVR4 */
}




/*
 * olgx_set_single_color(info,index,pixval,flag)
 * 
 * 
 * Change the pixval of a single index on the fly
 * 
 */

void
olgx_set_single_color(info, index, pixval, flag)
    Graphics_info  *info;
    int             index;
    unsigned long   pixval;
    int             flag;

{

    per_disp_res_ptr perdispl_res_ptr;

    if (info->gc_rec[index] != NULL) {
	if (info->pixvals[index] == pixval)
	    return;
	else
	    info->pixvals[index] = pixval;
    }
    perdispl_res_ptr = olgx_get_perdisplay_list(info->dpy, info->scrn);

    info->gc_rec[index] = olgx_set_color_smart(info, perdispl_res_ptr,
					       info->gc_rec[index],
					       1, pixval, flag);

    /* Depending on the index.. change the appropriate GC */

    switch (index) {

      case OLGX_WHITE:

	if (info->three_d != OLGX_3D_COLOR) {
	    info->gc_rec[OLGX_GREY_OUT] = olgx_set_color_smart(info,
                                                               perdispl_res_ptr,
						info->gc_rec[OLGX_GREY_OUT],
							       1, pixval, flag);

	    info->gc_rec[OLGX_SCROLL_GREY_GC] = olgx_set_color_smart(info, 
								perdispl_res_ptr,
					  info->gc_rec[OLGX_SCROLL_GREY_GC],
							        1, pixval, flag);

	    info->gc_rec[OLGX_BUSYGC] = olgx_set_color_smart(info,
                                                             perdispl_res_ptr,
					       info->gc_rec[OLGX_BUSYGC], 1,
							     pixval, flag);
	    if (!info->three_d) {
		/* 2D */
		info->gc_rec[OLGX_BLACK] = olgx_set_color_smart(info, 
                                                                perdispl_res_ptr,
						info->gc_rec[OLGX_BLACK], 0,
							        pixval, flag);

		info->gc_rec[OLGX_TEXTGC_REV] = olgx_set_color_smart(info,
                                                                perdispl_res_ptr,
					      info->gc_rec[OLGX_TEXTGC_REV],
							        1, pixval, flag);

		info->gc_rec[OLGX_TEXTGC] = olgx_set_color_smart(info,
                                                                perdispl_res_ptr,
						       info->gc_rec[OLGX_TEXTGC],
							        0, pixval, flag);

	    } else {

		/* 3d -Mono */

		info->gc_rec[OLGX_WHITE] = olgx_set_color_smart(info, 
                                                                perdispl_res_ptr,
						        info->gc_rec[OLGX_WHITE],
							        0, pixval, flag);

		info->gc_rec[OLGX_BG1] = olgx_set_color_smart(info,
                                                              perdispl_res_ptr,
           					       info->gc_rec[OLGX_BG1], 0,
							      pixval, flag);
	    }
	}
	break;

      case OLGX_BLACK:


	info->gc_rec[OLGX_TEXTGC] = olgx_set_color_smart(info,
                                                         perdispl_res_ptr,
					       info->gc_rec[OLGX_TEXTGC], 1,
							 pixval, flag);
	info->gc_rec[OLGX_BUSYGC] = olgx_set_color_smart(info,
                                                         perdispl_res_ptr,
					       info->gc_rec[OLGX_BUSYGC], 0,
							 pixval, flag);

	if (info->three_d != OLGX_3D_COLOR) {

	    info->gc_rec[OLGX_SCROLL_GREY_GC] = olgx_set_color_smart(info,
                                                                perdispl_res_ptr,
					       info->gc_rec[OLGX_SCROLL_GREY_GC],
							        1, pixval, flag);
	    if (!info->three_d) {

		info->gc_rec[OLGX_WHITE] = olgx_set_color_smart(info,
                                                                perdispl_res_ptr,
						info->gc_rec[OLGX_WHITE], 0,
							        pixval, flag);
		if (info->gc_rec[OLGX_TEXTGC_REV])
		    info->gc_rec[OLGX_TEXTGC_REV] = olgx_set_color_smart(info,
							   perdispl_res_ptr,
					      info->gc_rec[OLGX_TEXTGC_REV],
							   0, pixval, flag);

	    }
	}
	break;

      case OLGX_BG1:

	if (info->three_d) {

	    info->gc_rec[OLGX_TEXTGC] = olgx_set_color_smart(info,
                                                             perdispl_res_ptr,
					       info->gc_rec[OLGX_TEXTGC], 0,
							     pixval, flag);

	    if (info->three_d == OLGX_3D_COLOR) {

		info->gc_rec[OLGX_WHITE] = olgx_set_color_smart(info,
                                                                perdispl_res_ptr,
						info->gc_rec[OLGX_WHITE], 0,
							        pixval, flag);
		info->gc_rec[OLGX_BLACK] = olgx_set_color_smart(info,
                                                                perdispl_res_ptr,
						info->gc_rec[OLGX_BLACK], 0,
							        pixval, flag);
		info->gc_rec[OLGX_BG1] = olgx_set_color_smart(info,
                                                              perdispl_res_ptr,
						  info->gc_rec[OLGX_BG1], 0,
							      pixval, flag);
		info->gc_rec[OLGX_BG2] = olgx_set_color_smart(info,
                                                              perdispl_res_ptr,
						  info->gc_rec[OLGX_BG2], 0,
							      pixval, flag);
		info->gc_rec[OLGX_BG3] = olgx_set_color_smart(info,
                                                              perdispl_res_ptr,
						  info->gc_rec[OLGX_BG3], 0,
							      pixval, flag);
		info->gc_rec[OLGX_GREY_OUT] = olgx_set_color_smart(info,
                                                                perdispl_res_ptr,
						info->gc_rec[OLGX_GREY_OUT],
							        1, pixval, flag);
		info->gc_rec[OLGX_SCROLL_GREY_GC] = olgx_set_color_smart(info,
                                                                perdispl_res_ptr,
  					       info->gc_rec[OLGX_SCROLL_GREY_GC],
					  		        0, pixval, flag);
		info->gc_rec[OLGX_BUSYGC] = olgx_set_color_smart(info,
                                                               perdispl_res_ptr,
						       info->gc_rec[OLGX_BUSYGC],
						               1, pixval, flag);
	    }
	}
	break;

      case OLGX_BG2:

	/* do nothing */

	break;

      case OLGX_BG3:

	if (info->three_d == OLGX_3D_COLOR)
	    info->gc_rec[OLGX_SCROLL_GREY_GC] = olgx_set_color_smart(info, 
                                                                perdispl_res_ptr,
					       info->gc_rec[OLGX_SCROLL_GREY_GC],
					        		1, pixval, flag);
	break;

    }
}


/*
 * olgx_get_single_color(info,index)
 * 
 * Get the pixval of a single index
 * 
 */

unsigned long
olgx_get_single_color(info, index)
    Graphics_info  *info;
    int             index;
{

    if (info->gc_rec[index] != NULL)
	return (info->pixvals[index]);
    return 0;
}


/*
 * olgx_destroy(info)
 * 
 * Destroy all the information and server resources associated with a
 * Graphics_info structure, then free the structure itself.
 * 
 */


void
olgx_destroy(info)
    Graphics_info  *info;
{

    per_disp_res_ptr per_displ_res_ptr;
    register int    i;

    /*
     * Traverse the graphics_info gcrecs and either destroy them or reduce
     * its ref count, and free the greaphics_info struct
     */

    per_displ_res_ptr = olgx_get_perdisplay_list(info->dpy, info->scrn);
    for (i = 0; i < OLGX_NUM_GCS; i++)
	olgx_destroy_gcrec(per_displ_res_ptr, info->gc_rec[i]);


    free(info);
    info = NULL;

}


/*
 * olgx_malloc(nbytes)
 * 
 * Allocate a given number of bytes and return a pointer to it.
 */

char           *
olgx_malloc(nbytes)
    unsigned int    nbytes;
{

    return ((char *)calloc(nbytes, 1));

}

/*
 * Public Routine sets the specified clipmask on all the GCs used by the
 * specified ginfo.
 * 
 * void olgx_set_clipmask(ginfo,mask) Graphics_info * ginfo; Pixmap mask; {
 * 
 * } void olgx_set_cliprect(ginfo,rects,numrects) Graphics_info * ginfo; XRects
 * rects[]; int numrects;
 * 
 * { }
 */


/*
 * $void olgx_set_stipple_pixmaps(info, pixmaps) Graphics_info  *info; Pixmap
 * pixmaps[3];
 * 
 * { XGCValues       gcval;$
 */

/* If the GC's have not been created, create them */

/*
 * $gcval.fill_style = FillOpaqueStippled; gcval.stipple = pixmaps[0];
 * XChangeGC(info->dpy, info->gc_rec[OLGX_BG1]->gc, (GCFillStyle |
 * GCStipple), &gcval); info->stipple_pixmaps[0] = pixmaps[0];
 * 
 * gcval.stipple = pixmaps[1]; XChangeGC(info->dpy, info->gc_rec[OLGX_BG2]->gc,
 * (GCFillStyle | GCStipple), &gcval); info->stipple_pixmaps[0] = pixmaps[1];
 * 
 * gcval.stipple = pixmaps[2]; XChangeGC(info->dpy, info->gc_rec[OLGX_BG3]->gc,
 * (GCFillStyle | GCStipple), &gcval); info->stipple_pixmaps[0] = pixmaps[2];
 * 
 * }$ */



/* Resource saving GC manager */



GC_rec         *
olgx_get_gcrec(perdispl_res_ptr, drawable, depth, valuemask, values)
/*
 * Get a matching GC with the same valuemask and values...create one if there
 * is'nt one. Private Routine
 */

    per_disp_res_ptr perdispl_res_ptr;
    Drawable        drawable;
    int             depth;
    unsigned long   valuemask;
    XGCValues      *values;


{


    register GC_rec *cur;
    register GC_rec *prev;


    /* search if a identical Gc is found in the Pool */


    for (cur = perdispl_res_ptr->gc_list_ptr, prev = NULL; cur != NULL;
	 prev = cur, cur = cur->next) {
	if ((cur->valuemask == valuemask)
	    && (cur->depth == depth)
	    && (gc_matches(cur, valuemask, values))) {

	    /* A matching Gc is found */
	    /* Increase the reference count */

	    cur->ref_count++;

	    /* move this to the front of the list */

	    if (prev != NULL) {
		prev->next = cur->next;
		cur->next = perdispl_res_ptr->gc_list_ptr;
		perdispl_res_ptr->gc_list_ptr = cur;
	    }
	    return cur;

	}
    }

    /*
     * Match not found in the GC Pool so create a new one and add it to the
     * pool
     */


    cur = (GC_rec *) malloc(sizeof(GC_rec));
    if (cur == NULL)
	olgx_error("Error in allocating mem\n");

    cur->next = perdispl_res_ptr->gc_list_ptr;
    perdispl_res_ptr->gc_list_ptr = cur;

    cur->ref_count = 1;
    cur->valuemask = valuemask;
    if (values != NULL)
	cur->values = *values;
    cur->depth = depth;
    cur->gc = XCreateGC(perdispl_res_ptr->dpy, drawable, valuemask, values);

    return (cur);
}


/*
 * Private GCmanager routine Compares the values of the new gc to be created
 * with the one already in the list to see if they are the same. Returns True
 * if it matches
 */

int
gc_matches(GCrec, valuemask, values)
    GC_rec         *GCrec;
    unsigned long   valuemask;
    XGCValues      *values;

{

    register XGCValues *p = &(GCrec->values);

#define CheckGCField(MaskBit,fieldName)\
if ((valuemask & MaskBit) && (p->fieldName != values->fieldName))\
return False


    CheckGCField(GCForeground, foreground);
    CheckGCField(GCBackground, background);
    CheckGCField(GCGraphicsExposures, graphics_exposures);
    CheckGCField(GCFont, font);

    /* Are we done yet */

    if (!(valuemask & ~(GCForeground | GCBackground | GCFont | GCGraphicsExposures)))
	return True;

    CheckGCField(GCFillStyle, fill_style);
    CheckGCField(GCStipple, stipple);

    /* all Checks done, so the GC matches */
#undef CheckGCField
    return True;

}


/*
 * Routine searches thru the Pool of GCs and returns the gcrec if a matching
 * GC is found else returns NULL Private Routine
 * 
 */

GC_rec         *
olgx_gcrec_available(perdispl_res_ptr, valuemask, values)
    per_disp_res_ptr perdispl_res_ptr;
    XGCValues      *values;
    unsigned long   valuemask;

{
    register GC_rec *cur;
    register GC_rec *prev;


    /* search if a identical Gc is found in the cache */


    for (cur = perdispl_res_ptr->gc_list_ptr, prev = NULL;
	 cur != NULL; prev = cur, cur = cur->next) {


	if ((cur->valuemask == valuemask)
	    && (gc_matches(cur, valuemask, values))) {

	    /*
	     * A matching Gc is found Move it to the Front of the Pool
	     */

	    if (prev != NULL) {
		prev->next = cur->next;
		cur->next = perdispl_res_ptr->gc_list_ptr;
		perdispl_res_ptr->gc_list_ptr = cur;
	    }
	    return cur;
	}
    }

    /* Nothing Matching Found so return NULL */

    return NULL;
}



Pixmap
olgx_get_busy_stipple(perdispl_res_ptr)
    per_disp_res_ptr perdispl_res_ptr;

/* returns a standard busy  stipple pixmap .. Creates one if nothing exists  */

{



    if (!perdispl_res_ptr->busy_stipple)
	perdispl_res_ptr->busy_stipple = XCreatePixmapFromBitmapData(
                                                              perdispl_res_ptr->dpy,
      		        RootWindow(perdispl_res_ptr->dpy, perdispl_res_ptr->screen),
			   		              (char *)busy_bits, busy_width,
		 					   	     busy_height,
								     1, 0, 1);
    return (perdispl_res_ptr->busy_stipple);


}


Pixmap
olgx_get_grey_stipple(perdispl_res_ptr)
    per_disp_res_ptr perdispl_res_ptr;

/* returns the  std grey_out stipple pixmap.. Create one if nothing exists */

{

    if (!perdispl_res_ptr->grey_stipple)
	perdispl_res_ptr->grey_stipple = XCreatePixmapFromBitmapData(
                                                              perdispl_res_ptr->dpy,
 		        RootWindow(perdispl_res_ptr->dpy, perdispl_res_ptr->screen),
						              (char *)grey_bits,
                                                              grey_width,
							      grey_height, 1, 0, 1);
    return (perdispl_res_ptr->grey_stipple);
}






per_disp_res_ptr
olgx_get_perdisplay_list(dpy, screen)
/*
 * olgx_get_perdisplaylist() returns a pointer to perdisp_res_ptr after
 * searching thru the list of perdisplay_ptr's..creating one if none matching
 * "dpy" is found . Ginfo and GC's are cached on a perdisplay basis..and
 * these constiute the perdisplay resources
 */


    Display        *dpy;
    int             screen;


{

    static per_disp_res_ptr olgx_perdisp_res_list = NULL;
    register per_disp_res_ptr cur;
    register per_disp_res_ptr prev;



    /* Start searching thru the list */

    for (cur = olgx_perdisp_res_list, prev = NULL; cur != NULL;
	 prev = cur, cur = cur->next) {

	if ((cur->dpy == dpy)
	    && (cur->screen == screen)) {

	    /*
	     * We have found the perdisplay_res Ptr with the same display ..
	     * so move it to head of the list
	     */

	    if (prev != NULL) {
		prev->next = cur->next;
		cur->next = olgx_perdisp_res_list;
		olgx_perdisp_res_list = cur;
	    }
	    return cur;
	}
    }

    /* Nothing matching found.. so allocate a new one and add it to the head */

    cur = (per_disp_res_ptr) malloc(sizeof(per_disp_res_rec));
    cur->dpy = dpy;
    cur->screen = screen;
    cur->gc_list_ptr = NULL;
    cur->busy_stipple = NULL;
    cur->grey_stipple = NULL;
    cur->next = olgx_perdisp_res_list;
    olgx_perdisp_res_list = cur;

    return cur;
}



/*
 * Go thru the GC pool find the gc_rec specified and destroy it If it is
 * shared reduce the reference count Private Routine
 */


void
olgx_destroy_gcrec(perdisp_res_ptr, gcrec)
    per_disp_res_ptr perdisp_res_ptr;
    GC_rec         *gcrec;

{

    register GC_rec *cur;
    register GC_rec *prev;

    if (gcrec == NULL)
	return;


    for (cur = perdisp_res_ptr->gc_list_ptr, prev = NULL; cur != gcrec;
    /* EMPTY */
	 prev = cur, cur = cur->next);

    if (cur->ref_count > 1)
	cur->ref_count -= 1;
    else {

	if (prev != NULL) {
	    prev->next = cur->next;
	    free(gcrec);
	} else {
	    perdisp_res_ptr->gc_list_ptr = cur = cur->next;
	    free(gcrec);
	}
    }
}



/*
 * olgx_set_color_smart() Private Routine Takes care of checking the
 * ref_count and whether the GC is shared whether somwthing similar already
 * exists in the pool...etc
 */

GC_rec         *
olgx_set_color_smart(info, perdispl_res_ptr, gcrec, fg_flag, pixval, flag)
    Graphics_info  *info;
    per_disp_res_ptr perdispl_res_ptr;
    GC_rec         *gcrec;
    int             fg_flag;
    unsigned long   pixval;
    int             flag;

{

    XGCValues       values;
    GC_rec         *existing_gcrec;


    if (gcrec == NULL)
	/* The GC has not been created yet */
	return NULL;

    if (flag & OLGX_SPECIAL) {
	/*
	 * flag is OLGX_SPECIAL Don't worry if the GC 's are shared or not..
	 * Change them by brute force
	 */

	if (fg_flag) {
	    XSetForeground(info->dpy, gcrec->gc, pixval);
	    gcrec->values.foreground = pixval;
	} else {
	    XSetBackground(info->dpy, gcrec->gc, pixval);
	    gcrec->values.background = pixval;
	}

    } else {

	values = gcrec->values;
	if (fg_flag)
	    values.foreground = pixval;
	else
	    values.background = pixval;

	if (gcrec->ref_count > 1) {

	    /*
	     * This GC is shared.. so reduce the ref_count look if a similar
	     * GC with the new values exists or create a new GC with the new
	     * values
	     */

	    gcrec->ref_count -= 1;
	    gcrec = olgx_get_gcrec(perdispl_res_ptr, info->drawable[0],
                                   info->depth, gcrec->valuemask, &values);

	} else {

	    /* This GC is not shared */

	    existing_gcrec = olgx_gcrec_available(perdispl_res_ptr,
                                                  gcrec->valuemask, &values);

	    if (!existing_gcrec) {

		/* Nothing similar exists in the pool */

		if (fg_flag) {

		    XSetForeground(info->dpy, gcrec->gc, pixval);
		    gcrec->values.foreground = pixval;

		} else {

		    XSetBackground(info->dpy, gcrec->gc, pixval);
		    gcrec->values.background = pixval;

		}
	    } else {

		/* Something similar already exists in the pool */

		if (existing_gcrec != gcrec) {
		    olgx_destroy_gcrec(perdispl_res_ptr, gcrec);
		    existing_gcrec += 1;
		    gcrec = existing_gcrec;
		}
	    }
	}
    }


    return (gcrec);
}



/*
 * olgx_total_gcs() Private Routine Prints the Total number of GC's created
 * so far. Internal debugging routine.
 */

void
olgx_total_gcs(dpy, screen)
    Display        *dpy;
    int             screen;

{

    register GC_rec *cur;
    int             i = 0;
    per_disp_res_ptr per_displ_list;

    per_displ_list = olgx_get_perdisplay_list(dpy, screen);
    for (cur = per_displ_list->gc_list_ptr; cur->next != NULL;
    /* EMPTY */
	 cur = cur->next, ++i);
    printf("Total # GCs created = %d \n", i + 1);

}



/*
 * olgx_initialise_gcrec() Private Routine Initialises the respective GC
 * passed as an index values This routine facilitates creating GC's as when
 * and needed
 */

void
olgx_initialise_gcrec(info, index)
    Graphics_info  *info;
    short           index;

{


    XGCValues       values;
    unsigned long   valuemask;
    per_disp_res_ptr perdispl_res_ptr;


    /* Error Checking */
    if (!info->glyphfont)
	olgx_error("Uninitialized ginfo.linking with an incompatible libolgx ?");

    perdispl_res_ptr = olgx_get_perdisplay_list(info->dpy, info->scrn);

    if (info->three_d) {

	/* 3D Mono or Color */

	if (info->three_d == OLGX_3D_COLOR)
	    values.background = info->pixvals[OLGX_BG1];
	else
	    values.background = info->pixvals[OLGX_WHITE];

    } else
	/* 2D */
	values.background = info->pixvals[OLGX_WHITE];

    values.graphics_exposures = False;
    valuemask = (GCForeground | GCBackground | GCGraphicsExposures | 
		 GCFont | GCStipple);
    values.font = info->glyphfont->fid;
    values.stipple = olgx_get_grey_stipple(perdispl_res_ptr);

    switch (index) {

      case OLGX_WHITE:

	if (!info->three_d)
	    values.background = info->pixvals[OLGX_BLACK];

	values.foreground = info->pixvals[OLGX_WHITE];
	info->gc_rec[OLGX_WHITE] = olgx_get_gcrec(perdispl_res_ptr, 
                                                  info->drawable[0],info->depth, 
                                                  valuemask, &values);

	break;

      case OLGX_BLACK:

	/* get OLGX_BLACK GC */

	values.foreground = info->pixvals[OLGX_BLACK];
	info->gc_rec[OLGX_BLACK] = olgx_get_gcrec(perdispl_res_ptr,
                                                  info->drawable[0],info->depth, 
                                                  valuemask, &values);

	break;

      case OLGX_BG1:
      case OLGX_BG2:
      case OLGX_BG3:

	if (!info->three_d) {

	    /* 2D initialise OLGX_TEXTGC_REV */

	    values.foreground = info->pixvals[OLGX_WHITE];
	    values.background = info->pixvals[OLGX_BLACK];
#ifdef OW_I18N
	    if (Olgx_Flags(info) & OLGX_FONTSET) {
		info->gc_rec[OLGX_TEXTGC_REV]
			= olgx_get_gcrec(perdispl_res_ptr, info->drawable[0],
					 info->depth, valuemask & ~GCFont,
					 &values);
	    } else {
		values.font = info->textfont->fid;
		info->gc_rec[OLGX_TEXTGC_REV]
			= olgx_get_gcrec(perdispl_res_ptr, info->drawable[0],
					 info->depth, valuemask,
					 &values);
	    }
#else /* OW_I18N */
	    values.font = info->textfont->fid;
	    info->gc_rec[OLGX_TEXTGC_REV] = olgx_get_gcrec(perdispl_res_ptr, 
                                                           info->drawable[0],
						          info->depth, valuemask,
							   &values);
#endif /* OW_I18N */

	} else {

	    /* 3D - so initialise BG?  */

	    /*
	     * $if (info->three_d == OLGX_3D_MONO) { values.fill_style =
	     * FillOpaqueStippled; valuemask = (GCForeground | GCBackground |
	     * GCGraphicsExposures | GCFont | GCFillStyle | GCStipple); }$
	     */

	    values.foreground = info->pixvals[index];

	    /*
	     * $if (info->three_d == OLGX_3D_MONO) values.stipple =
	     * info->stipple_pixmaps[index - 2];$
	     */


	    info->gc_rec[index] = olgx_get_gcrec(perdispl_res_ptr,
                                                 info->drawable[0],info->depth,
                                                 valuemask, &values);
	}
	break;

      case OLGX_TEXTGC:

	/* get the OLGX_TEXTGC */

	values.foreground = info->pixvals[OLGX_BLACK];
	values.background = (info->three_d) ? info->pixvals[OLGX_BG1] :
	    info->pixvals[OLGX_WHITE];
#ifdef OW_I18N
	if (Olgx_Flags(info) & OLGX_FONTSET) {
	    info->gc_rec[OLGX_TEXTGC]
		= olgx_get_gcrec(perdispl_res_ptr, info->drawable[0],
				 info->depth, valuemask & ~GCFont, &values);
	} else {
	    values.font = info->textfont->fid;
	    info->gc_rec[OLGX_TEXTGC]
		= olgx_get_gcrec(perdispl_res_ptr, info->drawable[0],
				 info->depth, valuemask, &values);
	}
#else /* OW_I18N */
	values.font = info->textfont->fid;
	info->gc_rec[OLGX_TEXTGC] = olgx_get_gcrec(perdispl_res_ptr,
                                                   info->drawable[0],
                                                   info->depth, valuemask,
                                                   &values);
#endif /* OW_I18N */

	break;

      case OLGX_GREY_OUT:

	/* get the OLGX_GREY_OUT GC */

	valuemask = (GCForeground|GCGraphicsExposures|GCStipple|GCFillStyle);
	values.foreground = (info->three_d == OLGX_3D_COLOR) ? 
                             info->pixvals[OLGX_BG1] : info->pixvals[OLGX_WHITE];
	values.stipple = olgx_get_grey_stipple(perdispl_res_ptr);
	values.fill_style = FillStippled;
	info->gc_rec[OLGX_GREY_OUT] = olgx_get_gcrec(perdispl_res_ptr,
                                                     info->drawable[0],
                                                     info->depth, valuemask,
                                                     &values);

	break;

      case OLGX_BUSYGC:

	/* getthe OLGX_BUSYGC */

	values.foreground = (info->three_d == OLGX_3D_COLOR) ?
                             info->pixvals[OLGX_BG1] : info->pixvals[OLGX_WHITE];

	values.fill_style = FillOpaqueStippled;
	values.stipple = olgx_get_busy_stipple(perdispl_res_ptr);
	values.background = info->pixvals[OLGX_BLACK];
	valuemask = (GCBackground|GCForeground|GCGraphicsExposures
                     |GCFont|GCStipple | GCFillStyle);
	info->gc_rec[OLGX_BUSYGC] = olgx_get_gcrec(perdispl_res_ptr,
						   info->drawable[0],
						   info->depth, valuemask,
 						   &values);

	break;

      case OLGX_SCROLL_GREY_GC:

	/* get the OLGX_SCROLL_GREY_GC */

	values.fill_style = FillOpaqueStippled;
	values.stipple = olgx_get_grey_stipple(perdispl_res_ptr);
	valuemask = (GCBackground | GCForeground | GCGraphicsExposures |
                     GCStipple | GCFillStyle);

	if (info->three_d == OLGX_3D_COLOR) {
	    values.foreground = info->pixvals[OLGX_BG3];
	    values.background = info->pixvals[OLGX_BG1];
	} else {
	    values.foreground = info->pixvals[OLGX_BLACK];
	    values.background = info->pixvals[OLGX_WHITE];
	}

	info->gc_rec[OLGX_SCROLL_GREY_GC] = olgx_get_gcrec(perdispl_res_ptr,
                                                           info->drawable[0], 
                                                          info->depth, valuemask,
							   &values);

	break;
    }

}


/*
 * olgx_cmp_fonts()` Compares two fonts and returns True if equal False if
 * not Compares the overall max_bounds and min_bounds Probably not a good and
 * correct way of comparing fonts Private Routine
 */


int
olgx_cmp_fonts(font_info1, font_info2)
    XFontStruct    *font_info1;
    XFontStruct    *font_info2;

{


    if (font_info1->fid == font_info2->fid)
	return True;

    else
	return False;

    /*
     * #define IsEqual(field) \ if (font_info1->field != font_info2->field)
     * return False
     * 
     * IsEqual(min_bounds.lbearing); IsEqual(min_bounds.rbearing);
     * IsEqual(min_bounds.ascent); IsEqual(min_bounds.descent);
     * 
     * IsEqual(max_bounds.lbearing); IsEqual(max_bounds.rbearing);
     * IsEqual(max_bounds.ascent); IsEqual(max_bounds.descent);
     * 
     * return True;
     */
}


#ifdef OW_I18N
int
olgx_cmp_fontsets(fontset_info1, fontset_info2)
    XFontSet	*fontset_info1;
    XFontSet	*fontset_info2;
{
    if (fontset_info1 == fontset_info2)
	return True;
    else
	return False;
}
#endif /* OW_I18N */


/*
 * Public Utility Routine Initialise bg2,bg3 and white Xcolor structs as
 * dictated by the OPEN LOOK(TM) spec from the bg1 struct values passed in
 */

/*
 * $olgx_calculate_3Dcolors(fg, bg1, bg2, bg3, white) XColor         *fg,
 * *bg1; XColor         *bg2, *bg3, *white;$
 *//* return values */

/* ${$ */

/*
 * Calculate and initialise the bg2,bg3 values from the bg1 value passed and
 * also initialise the white values
 */


/*
 * $bg2->red = (bg1->red * 9) / 10; bg2->green = (bg1->green * 9) / 10;
 * bg2->blue = (bg1->blue * 9) / 10;
 * 
 * bg3->red = bg1->red >> 1; bg3->green = bg1->green >> 1; bg3->blue = bg1->blue
 * >> 1;
 * 
 * white->red = 0.92 * ((unsigned short) ~0); white->green = 0.92 * ((unsigned
 * short) ~0); white->blue = 0.92 * ((unsigned short) ~0);
 * 
 * return; }$
 */
