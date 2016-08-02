/*	@(#)curs_impl.h 20.25 93/06/28 SMI	*/

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#ifndef  _xview_cursor_impl_h_already_include
#define  _xview_cursor_impl_h_already_include

#include <xview_private/i18n_impl.h>
#include <xview/cursor.h>
#include <xview_private/draw_impl.h>


#define CURSOR_PRIVATE(cursor_public)	\
	    XV_PRIVATE(Cursor_info, Xv_cursor_struct, cursor_public)
#define CURSOR_PUBLIC(cursor_private)  XV_PRIVATE(cursor_private)

#define	DONT_SHOW_CURSOR	0x00000001
#define	FREE_SHAPE		0x00000080
#define	show_cursor(cursor)		(!((cursor)->flags & DONT_SHOW_CURSOR)) 
#define	free_shape(cursor)		((cursor)->flags & FREE_SHAPE)

typedef enum {
    CURSOR_TYPE_PIXMAP,	/* uses CURSOR_IMAGE */
    CURSOR_TYPE_GLYPH,	/* uses CURSOR_SRC_CHAR */
    CURSOR_TYPE_TEXT	/* uses CURSOR_STRING */
} Cursor_type;

typedef struct cursor_table_entry {
    unsigned char  *src_bits;
    unsigned char  *mask_bits;
    int             width;
    int             height;
    int             x_offset;   /* pixel x-offset of text baseline */
    int             y_offset;   /* pixel y-offset of text baseline */
} Cursor_table_entry;

typedef struct {
    Xv_opaque	    public_self;	/* back pointer */
    short	    cur_xhot, cur_yhot;	/* offset of mouse position from shape*/
    int		    cur_src_char, cur_mask_char;/* source and mask characters */
    int		    cur_function;	/* relationship of shape to screen */
    Pixrect	   *cur_shape;		/* memory image to use */
    unsigned long   cursor_id;		/* X cursor id		     */
    Cursor_drag_state drag_state;	/* text cursor drag state */
    Cursor_drag_type drag_type;		/* text cursor drag type */
    Xv_singlecolor  fg, bg;		/* fg/bg color of cursor */
    int		    flags;		/* various options */
    Xv_object	    root;		/* root handle		     */
#ifndef OW_I18N
    char	   *string;		/* text cursor string */
#else
    _xv_string_attr_dup_t  string;	/* text cursor string */
#endif
    Cursor_type	    type;		/* pixmap, glyph or text cursor */
} Cursor_info;

/* from cursor.c */
Pkg_private  Xv_opaque       cursor_set_internal();
Pkg_private  int             cursor_create_internal();
Pkg_private  Xv_opaque       cursor_get_internal();
Pkg_private  int             cursor_destroy_internal();

#endif	/* _xview_cursor_impl_h_already_included */
