#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_image.c 20.18 89/07/31";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/panel_impl.h>
#include <xview/svrimage.h>

extern struct pr_size xv_pf_textwidth();

/*****************************************************************************/
/* panel_button_image                                                        */
/* panel_button_image creates a button pixrect from the characters in        */
/* 'string'.  'width' is the desired width of the button (in characters).    */
/* If 'font' is NULL, the font in 'panel' is used.                           */
/* *** NOTE:  This routine is retained for compatibility only.				 */
/*****************************************************************************/

Sv1_public Pixrect *
panel_button_image(client_object, string, width, font)
    Panel           client_object;
    char           *string;
    int             width;
    Xv_Font         font;
{
    Panel_info     *object = PANEL_PRIVATE(client_object);
    struct pr_prpos where;	/* where to write the string */
    struct pr_size  size;	/* size of the pixrect */
    Panel_info     *panel;

    /* make sure we were really passed a panel, not an item */
    if (is_panel(object))
	panel = object;
    else if (is_item(object))
	panel = ((Item_info *) object)->panel;
    else
	return NULL;

    if (!font)
	font = xv_get(PANEL_PUBLIC(panel), WIN_FONT);

    size = xv_pf_textwidth(strlen(string), font, string);

    width = panel_col_to_x(font, width);

    if (width < size.x)
	width = size.x;

    where.pr = (Pixrect *) xv_create(0, SERVER_IMAGE,
				     XV_WIDTH, width,
				     XV_HEIGHT, size.y,
				     SERVER_IMAGE_DEPTH, 1, NULL);
    if (!where.pr)
	return (NULL);

    where.pos.x = (width - size.x) / 2;
    where.pos.y = panel_fonthome(font);

    xv_text((Xv_opaque)where.pr, where.pos.x, where.pos.y, PIX_SRC, font, string);

    return (where.pr);
}

