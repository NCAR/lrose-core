#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)cursor.c 20.55 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * cursor.c: Routines for creating & modifying a cursor.
 * 
 */

#include <X11/Xlib.h>
#include <xview_private/portable.h>
#include <xview_private/curs_impl.h>
#include <xview/font.h>
#include <xview/notify.h>
#include <xview/svrimage.h>
#include <xview/window.h>
#include <xview/screen.h>


Xv_private Attr_avlist attr_find();

Pkg_private unsigned long cursor_make_x();
Pkg_private unsigned long cursor_make_x_font();
Pkg_private void cursor_free_x();
Pkg_private void cursor_set_cursor_internal();

static Xv_opaque create_text_cursor();

extern unsigned char txtmove_bits[];
extern unsigned char txtmove_mask_bits[];
#define txtmove_width  64
#define txtmove_height 64
extern unsigned char txtmove_more_bits[];
#define txtmove_more_width  64
#define txtmove_more_height 64
extern unsigned char txtdup_bits[];
extern unsigned char txtdup_mask_bits[];
#define txtdup_width  64
#define txtdup_height 64
extern unsigned char txtdup_more_bits[];
#define txtdup_more_width  64
#define txtdup_more_height 64
extern unsigned char txtmove_accept_bits[];
extern unsigned char txtmove_accept_mask_bits[];
#define txtmove_accept_width  64
#define txtmove_accept_height 64
extern unsigned char txtmove_accept_more_bits[];
#define txtmove_accept_more_width  64
#define txtmove_accept_more_height 64
extern unsigned char txtdup_accept_bits[];
extern unsigned char txtdup_accept_mask_bits[];
#define txtdup_accept_width  64
#define txtdup_accept_height 64
extern unsigned char txtdup_accept_more_bits[];
#define txtdup_accept_more_width  64
#define txtdup_accept_more_height 64

/* The cursor table indices are [drag_state][drag_type][more_arrow] */
static Cursor_table_entry cursor_table[2][2][2] = {
    /* CURSOR_NEUTRAL, CURSOR_MOVE, no more arrow */
    txtmove_bits, txtmove_mask_bits,
    txtmove_width, txtmove_height, 20, 29,
    /* CURSOR_NEUTRAL, CURSOR_MOVE, more arrow */
    txtmove_more_bits, txtmove_mask_bits,
    txtmove_more_width, txtmove_more_height, 20, 29,
    /* CURSOR_NEUTRAL, CURSOR_DUPLICATE, no more arrow */
    txtdup_bits, txtdup_mask_bits,
    txtdup_width, txtdup_height, 20, 29,
    /* CURSOR_NEUTRAL, CURSOR_DUPLICATE, more arrow */
    txtdup_more_bits, txtdup_mask_bits,
    txtdup_more_width, txtdup_more_height, 20, 29,
    /* CURSOR_ACCEPT, CURSOR_MOVE, no more arrow */
    txtmove_accept_bits, txtmove_accept_mask_bits,
    txtmove_accept_width, txtmove_accept_height, 20, 29,
    /* CURSOR_ACCEPT, CURSOR_MOVE, more arrow */
    txtmove_accept_more_bits, txtmove_accept_mask_bits,
    txtmove_accept_more_width, txtmove_accept_more_height, 20, 29,
    /* CURSOR_ACCEPT, CURSOR_DUPLICATE, no more arrow */
    txtdup_accept_bits, txtdup_accept_mask_bits,
    txtdup_accept_width, txtdup_accept_height, 20, 29,
    /* CURSOR_ACCEPT, CURSOR_DUPLICATE, more arrow */
    txtdup_accept_more_bits, txtdup_accept_mask_bits,
    txtdup_accept_more_width, txtdup_accept_more_height, 20, 29,
};

#define CURSOR_TEXT_XHOT 9
#define CURSOR_TEXT_YHOT 9

Pkg_private int
cursor_create_internal(parent, object, avlist)
    Xv_Screen       parent;
    Xv_Cursor       object;
    Attr_avlist     avlist;
/*
 * Parent should be either a window or a screen, or any object that will
 * return the root window in response to xv_get(parent, XV_ROOT).
 */
{
    register Cursor_info *cursor;
    register Pixrect *pr;
    Cursor_info    *other_cursor;
    Attr_avlist     copy_attr;

    ((Xv_cursor_struct *) (object))->private_data =
	(Xv_opaque) xv_alloc(Cursor_info);
    if (!(cursor = CURSOR_PRIVATE(object))) {
	return XV_ERROR;
    }
    cursor->public_self = object;
    cursor->cur_src_char = NOFONTCURSOR;
    /* Use default screen if none given (xv_create ensures default ok) */
    cursor->root = xv_get((parent ? parent : xv_default_screen), XV_ROOT);

    copy_attr = attr_find(avlist, XV_COPY_OF);
    if (*copy_attr) {
	other_cursor = CURSOR_PRIVATE(copy_attr[1]);
	*cursor = *other_cursor;

	/* Allocate new shape, copy old, and flag need to free new shape. */
	pr = other_cursor->cur_shape;
	cursor->cur_shape =
	    (Pixrect *) xv_create(xv_get(other_cursor->root, XV_SCREEN),
				  SERVER_IMAGE,
				  XV_WIDTH, pr->pr_width,
				  XV_HEIGHT, pr->pr_height,
				  SERVER_IMAGE_DEPTH, pr->pr_depth,
				  NULL);
	if (!cursor->cur_shape)
	    return XV_ERROR;
	xv_rop(cursor->cur_shape, 0, 0, pr->pr_width, pr->pr_height, PIX_SRC,
	       pr, 0, 0);
	cursor->flags |= FREE_SHAPE;
	cursor->cur_xhot = other_cursor->cur_xhot;
	cursor->cur_yhot = other_cursor->cur_yhot;
	cursor->cur_src_char = other_cursor->cur_src_char;
	cursor->cur_mask_char = other_cursor->cur_mask_char;
	cursor->cur_function = other_cursor->cur_function;
    } else {
	cursor->cur_function = PIX_SRC | PIX_DST;
	cursor->cur_shape = 
	    (Pixrect *) xv_create(xv_get(cursor->root, XV_SCREEN),
				  SERVER_IMAGE,
				  XV_WIDTH, CURSOR_MAX_IMAGE_WORDS,
				  XV_HEIGHT, CURSOR_MAX_IMAGE_WORDS,
				  SERVER_IMAGE_DEPTH, 1,
				  NULL);
	cursor->flags = FREE_SHAPE;
    }
    /* the id will be set the first time through cursor_set() */
    cursor->cursor_id = 0;

    /* default foreground/background color */
    cursor->fg.red = 0;
    cursor->fg.green = 0;
    cursor->fg.blue = 0;
    cursor->bg.red = 255;
    cursor->bg.green = 255;
    cursor->bg.blue = 255;

    cursor->type = CURSOR_TYPE_PIXMAP;
    cursor->drag_state = CURSOR_NEUTRAL;
    cursor->drag_type = CURSOR_MOVE;

    (void) xv_set(object, XV_RESET_REF_COUNT, NULL);	/* Mark as ref counted. */

    return XV_OK;
}


Pkg_private int
cursor_destroy_internal(cursor_public, status)
    Xv_Cursor       cursor_public;
    Destroy_status  status;
{
    Cursor_info    *cursor = CURSOR_PRIVATE(cursor_public);
    Xv_Drawable_info *info;

    if (status == DESTROY_CLEANUP) {
	if (free_shape(cursor))
	    xv_destroy( (Xv_opaque)(cursor->cur_shape) );
	if (cursor->type == CURSOR_TYPE_TEXT && cursor->cursor_id) {
	    DRAWABLE_INFO_MACRO(cursor->root, info);
	    XFreeCursor(xv_display(info), cursor->cursor_id);
#ifdef OW_I18N
	    _xv_free_ps_string_attr_dup(&cursor->string);
#endif
	}
	free((char *) cursor);
    }
    return XV_OK;
}

/*ARGSUSED*/ /*VARARGS3*/
Pkg_private Xv_opaque
cursor_get_internal(cursor_public, status, which_attr, args)
    Xv_Cursor       cursor_public;
    int            *status;
    Attr_attribute  which_attr;
    va_list         args;
{
    register Cursor_info     *cursor = CURSOR_PRIVATE(cursor_public);

    switch (which_attr) {
      case XV_XID:
	return (Xv_opaque) cursor->cursor_id;

      case XV_SHOW:
	return (Xv_opaque) show_cursor(cursor);

      case CURSOR_STRING:
#ifndef OW_I18N
	return (Xv_opaque) cursor->string;
#else
	return (Xv_opaque) _xv_get_mbs_attr_dup(&cursor->string);

      case CURSOR_STRING_WCS:
	return (Xv_opaque) _xv_get_wcs_attr_dup(&cursor->string);
#endif /* OW_I18N */

      case CURSOR_DRAG_STATE:
	return (Xv_opaque) cursor->drag_state;

      case CURSOR_DRAG_TYPE:
	return (Xv_opaque) cursor->drag_type;

      case CURSOR_SRC_CHAR:
	return (Xv_opaque) cursor->cur_src_char;

      case CURSOR_MASK_CHAR:
	return (Xv_opaque) cursor->cur_mask_char;

      case CURSOR_XHOT:
	return (Xv_opaque) cursor->cur_xhot;

      case CURSOR_YHOT:
	return (Xv_opaque) cursor->cur_yhot;

      case CURSOR_OP:
	return (Xv_opaque) cursor->cur_function;

      case CURSOR_IMAGE:
	return (Xv_opaque) cursor->cur_shape;

      case CURSOR_FOREGROUND_COLOR: 
	 return ((Xv_opaque) &cursor->fg);

      case CURSOR_BACKGROUND_COLOR:
	 return ((Xv_opaque)&cursor->bg);

      default:
	if (xv_check_bad_attr(CURSOR, which_attr) == XV_ERROR) {
	    *status = XV_ERROR;
	}
	return XV_NULL;
    }

}


/* cursor_set_attr sets the attributes mentioned in avlist. */
Pkg_private Xv_opaque
cursor_set_internal(cursor_public, avlist)
    Xv_Cursor       cursor_public;
    register Attr_avlist avlist;
{
    register Cursor_info 	*cursor = CURSOR_PRIVATE(cursor_public);
    register Pixrect 		*pr;
    register int    		dirty = FALSE;
    Xv_Drawable_info 		*root_info;
    register Xv_opaque 		arg1;
    int				end_create = FALSE;
    Xv_singlecolor		*fg = NULL, *bg = NULL;	
    XColor			xfg, xbg;

    for (; *avlist; avlist = attr_next(avlist)) {
	arg1 = avlist[1];
	switch (*avlist) {
	  case XV_SHOW:
	    /* BUG: is this used anywhere? */
	    if (arg1)
		cursor->flags &= ~DONT_SHOW_CURSOR;
	    else
		cursor->flags |= DONT_SHOW_CURSOR;
	    dirty = TRUE;
	    break;

	  case CURSOR_STRING:
#ifdef OW_I18N
	    _xv_set_mbs_attr_dup(&cursor->string, (char *) arg1);
#else
	    cursor->string = (char *) arg1;
#endif
	    cursor->type = CURSOR_TYPE_TEXT;
	    break;

#ifdef OW_I18N
	  case CURSOR_STRING_WCS:
	    _xv_set_wcs_attr_dup(&cursor->string, (CHAR *) arg1);
	    cursor->type = CURSOR_TYPE_TEXT;
	    break;
#endif

	  case CURSOR_DRAG_STATE:
	    cursor->drag_state = (Cursor_drag_state) arg1;
	    if (cursor->drag_state > CURSOR_ACCEPT) {
		/* CURSOR_REJECT is not supported yet */
		cursor->drag_state = CURSOR_ACCEPT;
	    }
	    break;

	  case CURSOR_DRAG_TYPE:
	    cursor->drag_type = (Cursor_drag_type) arg1;
	    break;

	  case CURSOR_SRC_CHAR:
	    cursor->cur_src_char = (unsigned int) arg1;
	    cursor->type = CURSOR_TYPE_GLYPH;
	    dirty = TRUE;
	    break;

	  case CURSOR_MASK_CHAR:
	    cursor->cur_mask_char = (unsigned int) arg1;
	    dirty = TRUE;
	    break;

	  case CURSOR_XHOT:
	    cursor->cur_xhot = (int) arg1;
	    dirty = TRUE;
	    break;

	  case CURSOR_YHOT:
	    cursor->cur_yhot = (int) arg1;
	    dirty = TRUE;
	    break;

	  case CURSOR_OP:
	    cursor->cur_function = (int) arg1;
	    dirty = TRUE;
	    break;

	  case CURSOR_IMAGE:
	    if (free_shape(cursor)) {
                    /* destroy the remote image */
	        xv_destroy( (Xv_opaque)(cursor->cur_shape) );
		cursor->flags &= ~FREE_SHAPE;
	    }
	    cursor->cur_shape = (Pixrect *) arg1;
	    cursor->type = CURSOR_TYPE_PIXMAP;
	    dirty = TRUE;
	    break;

	  case CURSOR_FOREGROUND_COLOR:
	     fg = (Xv_singlecolor *)arg1;
	     cursor->fg.red = fg->red;
	     cursor->fg.green = fg->green;
	     cursor->fg.blue = fg->blue;
	     break;

	  case CURSOR_BACKGROUND_COLOR:
	     bg = (Xv_singlecolor *)arg1;
	     cursor->bg.red = bg->red;
	     cursor->bg.green = bg->green;
	     cursor->bg.blue = bg->blue;
	     break;

	  case XV_COPY_OF:
	     dirty = TRUE;
	     break;

	  case XV_END_CREATE:
	    end_create = TRUE;
	    break;

	  default:
	    (void) xv_check_bad_attr(CURSOR, *avlist);
	    break;
	}
    }

    DRAWABLE_INFO_MACRO(cursor->root, root_info);

    if (end_create && cursor->type == CURSOR_TYPE_TEXT) {
	return create_text_cursor(cursor, root_info);
    }

    xfg.red = cursor->fg.red << 8;
    xfg.green = cursor->fg.green << 8;
    xfg.blue = cursor->fg.blue << 8;
    xfg.flags = DoRed | DoGreen | DoBlue;
    xbg.red = cursor->bg.red << 8; 
    xbg.green = cursor->bg.green << 8;
    xbg.blue = cursor->bg.blue << 8;
    xbg.flags = DoRed | DoGreen | DoBlue;

    if (!dirty) {
	if (fg || bg) {
	    XRecolorCursor(xv_display(root_info), cursor->cursor_id,
			   &xfg, &xbg);
	}
	return XV_OK;
    }

    /* make the cursor now */
    if (cursor->cursor_id) {
	cursor_free_x(root_info, cursor->cursor_id);
    }
    if (cursor->cur_src_char != NOFONTCURSOR) {
	cursor->cursor_id = cursor_make_x_font(root_info,
				       (unsigned int) cursor->cur_src_char,
				       (unsigned int) cursor->cur_mask_char,
				       &xfg, &xbg);
    } else {
	pr = cursor->cur_shape;
	cursor->cursor_id = cursor_make_x(root_info,
	   				  pr->pr_size.x, pr->pr_size.y,
					  pr->pr_depth, cursor->cur_function,
				          cursor->cur_xhot, cursor->cur_yhot, 
					  &xfg, &xbg, pr); 
    }

    /* BUG: ok to abort? */
    if (!cursor->cursor_id) {
	xv_error((Xv_object)cursor,
		 ERROR_STRING, 
		 XV_MSG("cursor: can't create cursor"),
		 ERROR_PKG, CURSOR,
		 NULL);
    }
    return (Xv_opaque) XV_OK;
}

Xv_private void
cursor_set_cursor(window, cursor_public)
    Xv_object       window;
    Xv_Cursor       cursor_public;
{
    Cursor_info    *cursor = CURSOR_PRIVATE(cursor_public);
    Xv_Drawable_info *window_info;

    if (xv_get(window, XV_ROOT) != cursor->root) {
	xv_error((Xv_object)cursor,
		 ERROR_STRING,
		   XV_MSG("Window and cursor have different roots! Can't set cursor"),
		 ERROR_PKG, CURSOR,
		 NULL);
    } else {
	DRAWABLE_INFO_MACRO(window, window_info);
	cursor_set_cursor_internal(window_info, cursor->cursor_id);
    }
}

#define CUR_MAX_TEXT		3

static Xv_opaque
create_text_cursor(cursor, info)	/* returns XV_OK or XV_ERROR */
    Cursor_info	   *cursor;
    Xv_Drawable_info *info;
{
    unsigned int    best_height;
    unsigned int    best_width;
    XColor	    bg;		/* background color of cursor */
    Colormap	    cmap;
    Cursor_table_entry cte;
    Display	   *display;
    XColor	    fg;		/* foreground color of cursor */
    Xv_Font	    font;
    int		    length;
    Pixmap	    mask_pixmap;
    int		    more_arrow;
    int		    screen_nbr;
    Pixmap	    src_pixmap;
    Screen_visual  *visual;
    Status	    status;
    XID		    xid;

    display = xv_display(info);
    xid = xv_xid(info);

#ifdef OW_I18N
    length = STRLEN(cursor->string.pswcs.value);
#else
    length = strlen(cursor->string);
#endif

    if (length > CUR_MAX_TEXT) {
	length = CUR_MAX_TEXT;
	more_arrow = TRUE;
    } else
	more_arrow = FALSE;

    cte = cursor_table[cursor->drag_state][cursor->drag_type][more_arrow];

    /* See if we can create a cursor of this size */
    status = XQueryBestCursor(display, xid, cte.width, cte.height,
			      &best_width, &best_height);
    if (!status || best_width < cte.width || best_height < cte.height)
	return XV_ERROR;

    /* Create mask and source pixmaps */
    mask_pixmap = XCreateBitmapFromData(display, xid, (char *)(cte.mask_bits),
                                        cte.width, cte.height);
    src_pixmap = XCreateBitmapFromData(display, xid, (char *)(cte.src_bits),
                                       cte.width, cte.height);

    /* Draw text into source pixmap */
    visual = (Screen_visual *) xv_get(xv_screen(info),
        SCREEN_IMAGE_VISUAL, src_pixmap, 1);
    font = xv_find(xv_server(info), FONT,
	FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
	FONT_STYLE, FONT_STYLE_DEFAULT,
	FONT_SIZE, FONT_SIZE_DEFAULT,
	NULL);
    if (!font)
	return XV_ERROR;

    XSetFont(display, visual->gc, xv_get(font, XV_XID));
    XSetFillStyle(display, visual->gc, FillSolid);

    /* Draw string into cursor pixmap */
#ifdef OW_I18N
    XwcDrawString(display, src_pixmap, (XFontSet)xv_get(font, FONT_SET_ID),
		  visual->gc, cte.x_offset, cte.y_offset,
    		  cursor->string.pswcs.value, length);
#else
    XDrawString(display, src_pixmap, visual->gc, cte.x_offset, cte.y_offset,
		cursor->string, length);
#endif

    /* Define foreground and background colors */
    screen_nbr = (int) xv_get(xv_screen(info), SCREEN_NUMBER);
    fg.flags = bg.flags = DoRed | DoGreen | DoBlue;
    fg.pixel = BlackPixel(display, screen_nbr);
    cmap = (Colormap) xv_get(xv_cms(info), XV_XID);
    XQueryColor(display, cmap, &fg);
    bg.pixel = WhitePixel(display, screen_nbr);
    XQueryColor(display, cmap, &bg); 

    /* Create Pixmap Cursor */
    cursor->cursor_id = XCreatePixmapCursor(display, src_pixmap, mask_pixmap,
	&fg, &bg, CURSOR_TEXT_XHOT, CURSOR_TEXT_YHOT);

    /* Free the src_pixmap and mask_pixmap */

    XFreePixmap(display,src_pixmap);
    XFreePixmap(display,mask_pixmap);



    if (cursor->cursor_id)
	return XV_OK;
    else
	return XV_ERROR;
}
