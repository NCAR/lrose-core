#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)omi.c 20.55 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Image layout:
 * 
 * Image width = M, Lm, LPr, Pr, Rpr, Rm, M Image height = M, max(LPr, Pr, Rpr),
 * M
 * 
 * for choice & toggle menus: Image width = M, M, LPr, M, STRING, M, M
 * 
 * M = Margin, Lm = Left Margin, LPr = Left Pixrect, Pr = Pixrect or String,
 * Rpr = Right Pixrect, Rm = Right Margin,
 * 
 */

#include <sys/types.h>
#include <stdio.h>

#include <pixrect/pixrect.h>

#include <X11/Xlib.h>

#include <xview_private/om_impl.h>
#include <xview_private/draw_impl.h>
#include <olgx/olgx.h>

#ifdef  OW_I18N
extern struct pr_size xv_pf_textwidth_wc();
#endif
extern struct pr_size xv_pf_textwidth();

#define IMAX(a, b) ((int)(b) > (int)(a) ? (int)(b) : (int)(a))
#define INHERIT_VALUE(f) im->f ? im->f : std_image ? std_image->f : 0

Pkg_private int menu_image_compute_size();


Pkg_private int
menu_image_compute_size(m, im, std_image)	/* returns status */
    Xv_menu_info   *m;
    register struct image *im, *std_image;
{
    Font	    font;
    register int    margin2;
    int             margin, left_margin, right_margin;
    Pixrect	   *pr;


    margin = INHERIT_VALUE(margin);
    left_margin = INHERIT_VALUE(left_margin);
    right_margin = INHERIT_VALUE(right_margin);
    margin2 = margin << 1;

    if (im->svr_im) {
	pr = (Pixrect *) im->svr_im; /* Well, it's faster than xv_get! */
	im->button_size.x = pr->pr_width;
	im->button_size.y = pr->pr_height;
#ifdef OW_I18N
    } else if (_xv_is_string_attr_exist_nodup(&im->string)) {
#else
    } else if (im->string) {
#endif
	if (im->title)
	    font = std_image->bold_font;
	else
	    font = INHERIT_VALUE(font);
#ifdef OW_I18N
	_xv_use_pswcs_value_nodup(&im->string);
	im->button_size = xv_pf_textwidth_wc(
				       wslen(im->string.pswcs.value),
				       font, im->string.pswcs.value);
#else
	im->button_size = xv_pf_textwidth(strlen(im->string), font,
					  im->string);
#endif /* OW_I18N */

	/* make every string menu item with the same font the same height */
	im->button_size.y = Button_Height(m->ginfo);
    } else if (!(m->pin && im->title)) {
	xv_error(XV_ZERO,
		 ERROR_STRING, 
		    XV_MSG("Menu item does not have a string or image"),
		 ERROR_PKG, MENU,
		 NULL);
	return XV_ERROR;
    }
#ifdef OW_I18N
    if (im->svr_im || _xv_is_string_attr_exist_nodup(&im->string)) {
#else
    if (im->svr_im || im->string) {
#endif
	im->width = im->button_size.x;
	im->height = im->button_size.y;

	im->button_pos.y = margin;
	im->height += margin2;
    } else {
	im->height = im->width = 0;
    }
    im->width = IMAX(im->width, std_image->width);
    im->height = IMAX(im->height, std_image->height);
    return XV_OK;
}
