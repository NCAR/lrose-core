/*  @(#)p_lst_impl.h 1.50 93/06/28 SMI */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#ifndef __panel_list_impl_h
#define __panel_list_impl_h

#include <xview/panel.h>
#include <xview/font.h>
#include <xview/openmenu.h>
#include <xview/scrollbar.h>

typedef struct panel_list_struct		Panel_list_info;
typedef struct panel_list_row_struct		Row_info;

typedef enum {
    OP_NONE,
    OP_CHANGE,
    OP_INSERT,
    OP_DELETE
} Edit_op;

#define PRIMARY_CHOICE	0
#define SHELF_CHOICE	1
#define PANEL_LIST_DEFAULT_ROW	5

/*** Note: Update PANEL_LIST_SORT code when changing this structure. ***/
struct panel_list_row_struct {
	Xv_opaque	client_data;	/* Client data with each row */
	int		display_str_len; /* length of displayed string */
	Xv_Font		font;		/* NULL => use WIN_FONT */
	Pixrect		*glyph;
	Pixrect		*mask_glyph;
	int		row;            /* Row number */
#ifdef OW_I18N
	_xv_string_attr_dup_t
			string;
#else
	char		*string;
#endif
	int		string_y;
	Xv_opaque	exten_data;	/* client data for extensions */
	struct timeval	click_time; 	/* double-click detection */

	struct {
	  unsigned edit_selected : 1;	/* selected in edit mode */
#ifndef OW_I18N
	  unsigned free_string : 1;	/* free malloc'ed string */
#endif
	  unsigned selected : 1;        /* selected in read mode (= current) */
	  unsigned show : 1;		/* row is to be painted */
	  unsigned row_inactive : 1;	/* row is inactive */
	} f;	/* flags */

	/* Chaining */
	struct panel_list_row_struct *next;
	struct panel_list_row_struct *prev;
};
 
struct panel_list_struct {
	Panel_item	public_self;
        Panel		parent_panel;   /* Panel we're in */
	Rect		list_box;	/* Box enclosing list of rows */
	Scrollbar	list_sb;	/* Scrollbar for list_box */
	Menu		edit_menu;	/* Row panel edit mode menu */
	Edit_op		edit_op;	/* current edit operation, if any */
	Row_info	*focus_row;	/* Location Cursor is on this row */
	int		focus_win_x;	/* x position of focus window */
	int		focus_win_y;	/* y position of focus window */
	Xv_Font		font;		/* font of parent panel */
#ifdef OW_I18N
        XFontSet        font_set;       /* font set of parent panel */
#else
	XFontStruct     *font_struct;	/* font structure of parent panel */
#endif
	Menu		read_menu;	/* Row panel read mode menu */
	int		sb_active;	/* all events go to Scrollbar */
	Rect		sb_rect;	/* Scrollbar window rectangle */
	Panel_item	text_item;	/* Text item used during editing */
	Row_info	*text_item_row;	/* Text item is editing this row */
	int		text_item_view_start; /* first row in view when text
					       * item was made visible. */
#ifdef OW_I18N
	_xv_string_attr_dup_t
			title;
#else
	CHAR		*title;
#endif
	int		title_display_str_len;
	Rect		title_rect;

	/* control */
	unsigned choose_none: 1;	/* no current row is okay */
	unsigned choose_one : 1;    /* TRUE: exclusive, FALSE: non-exclusive */
	unsigned edit_mode : 1;		/* TRUE: read-write, FALSE: read */
	unsigned focus_win_shown : 1; /* state of XV_SHOW for focus window */
	unsigned initialized : 1;	/* set of XV_END_CREATE has occurred */
	unsigned insert_delete_enabled;	/* OK to insert or delete rows */
        unsigned insert_duplicate : 1;/* OK to insert duplicate strings */
	unsigned left_hand_sb: 1; /* list_box Scrollbar is on left hand side */
	unsigned read_only : 1;		/* TRUE: read, FALSE: read/write */
	unsigned setting_current_row:1;
	unsigned destroy_edit_menu:1; /* flags to check if we need to */
	unsigned destroy_read_menu:1; /* destroy the edit/read menu */
#ifdef OW_I18N         
        unsigned stored_length_wc:1;  /* TRUE: use PANEL_VALUE_STORED_LENGTH_WCS,
        				 FALSE: use PANEL_VALUE_STORED_LENGTH */
#endif

	unsigned do_dbl_click : 1;	/* deliver dbl click ops */
	
	/* sizes and positions */
	unsigned short	nlevels;	/* Number of levels */
	unsigned short	height;		/* Height of the scroll list */
	int	width;		/* -1 = extend width to edge of panel
				 * 0 = fit width to widest row
				 * other = list box width */
	unsigned short	nrows;		/* Number of rows */
	unsigned short	rows_displayed;	/* Number of rows displayed */
	unsigned short	row_height;  /* Height of each row. 0 => font height */
	unsigned short  string_x;	/* left margin of each row's string */

	/* Current data */
	Row_info	*rows;
	Row_info	*current_row;	/* last row selected */
	Row_info	*last_edit_row; /* last row selected for editing */
	Row_info	*last_click_row;	/* last row click'd in */
};

#ifdef OW_I18N
#define	STRING	string.pswcs.value
#define	TITLE	title.pswcs.value
#define	PANEL_VALUE_I18N	PANEL_VALUE_WCS
#define	PANEL_STRSAVE		panel_strsave_wc
#define	XV_PF_TEXTWIDTH		xv_pf_textwidth_wc
#define	NULL_STRING		_xv_null_string_wc
#else
#define	STRING	string
#define	TITLE	title
#define	PANEL_VALUE_I18N	PANEL_VALUE
#define	PANEL_STRSAVE		panel_strsave
#define	XV_PF_TEXTWIDTH		xv_pf_textwidth
#define	NULL_STRING		""
#endif

#endif /* __panel_list_impl_h */
