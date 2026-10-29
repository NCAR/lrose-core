#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_list.c 1.142 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <ctype.h>
#include <string.h>
#include <X11/X.h>
#include <xview_private/panel_impl.h>
#include <xview/cursor.h>
#include <xview/defaults.h>
#include <xview_private/p_lst_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/font.h>
#include <xview/scrollbar.h>
#include <xview/server.h>
#include <xview/screen.h>
#include <xview/openmenu.h>
#include <xview/win_notify.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif /* SVR4 */

#define PANEL_LIST_PRIVATE(item)        \
        XV_PRIVATE(Panel_list_info, Xv_panel_list, item)
#define PANEL_LIST_PUBLIC(item)         XV_PUBLIC(item)
#define PANEL_LIST_FROM_ITEM(ip)	PANEL_LIST_PRIVATE(ITEM_PUBLIC(ip))
#define ITEM_FROM_PANEL_LIST(dp)	ITEM_PRIVATE(PANEL_LIST_PUBLIC(dp))
#define LEVEL_DOT_HEIGHT	6
#define PANEL_LIST_ROW_GAP	5
#define PANEL_LIST_COL_GAP	4	/* space between glyph and string */
#define ROW_MARGIN	9 /* space between vertical borders and box */
#define LIST_BOX_BORDER_WIDTH 1
#define SHOW_ROW TRUE
#define HIDE_ROW FALSE
#define REPAINT_LIST TRUE
#define DO_NOT_REPAINT_LIST FALSE
#define RETURN '\r'
#define TAB '\t'


#ifdef OW_I18N
Xv_public struct pr_size  xv_pf_textwidth_wc();
extern	  wchar_t	 _xv_null_string_wc[];
#else
Xv_public struct pr_size  xv_pf_textwidth();
#endif /* OW_I18N */

Xv_private void	    screen_adjust_gc_color();
Xv_private void	    win_set_no_focus();

/* XView functions */
Pkg_private int panel_list_init();
Pkg_private Xv_opaque panel_list_set_avlist();
Pkg_private Xv_opaque panel_list_get_attr();
Pkg_private int panel_list_destroy();
Pkg_private void panel_list_row_inactive_set();
Pkg_private int panel_list_row_inactive_get();

/* Panel Item Operations */
static void         panel_list_handle_event();
static void	    panel_list_paint();
static void	    panel_list_resize();
static void	    panel_list_remove();
static void	    panel_list_layout();
static void	    show_focus_win();
static void	    hide_focus_win();

/* Local functions */
static void	    accept_change();
static void	    accept_insert();
static Panel_setting change_done();
static Xv_opaque    change_proc();
static int	    check_for_duplicate();
static Xv_opaque    clear_all_choices();
static void	    compute_dimensions();
static Xv_opaque    delete_proc();
static Xv_opaque    enter_edit_mode();
static Xv_opaque    enter_read_mode();
static Row_info	   *find_or_create_nth_row();
static Row_info	   *create_next_row();
static Row_info	   *gimme_the_next_row();
static int	    fit_list_box_to_rows();
static int	    get_row_rect();
static void	    handle_menu_event();
static Xv_opaque    insert_proc();
static Xv_opaque    locate_next_choice();
static void	    make_row_visible();
static Row_info	   *next_row();
static void	    paint_list_box();
static void	    paint_list_box_border();
static void	    paint_row();
static void	    paint_title_box();
static void	    panel_list_create_displayarea();
static void         panel_list_delete_row();
static Row_info	   *panel_list_insert_row();
static int	    row_visible();
static void	    set_current_row();
static void	    set_edit_row();
static void	    set_row_display_str_length();
static void	    set_row_font();
static void	    set_row_glyph();
static void	    set_row_mask_glyph();
static void	    show_feedback();
static void	    list_menu_done_proc();
static int	    is_dbl_click();

static Panel_ops ops = {
    panel_list_handle_event,		/* handle_event() */
    NULL,				/* begin_preview() */
    NULL,				/* update_preview() */
    NULL,				/* cancel_preview() */
    NULL,				/* accept_preview() */
    NULL,				/* accept_menu() */
    NULL,				/* accept_key() */
    panel_default_clear_item,		/* clear() */
    panel_list_paint,			/* paint() */
    panel_list_resize,			/* resize() */
    panel_list_remove,			/* remove() */
    NULL,				/* restore() */
    panel_list_layout,			/* layout() */
    show_focus_win,			/* accept_kbd_focus() */
    hide_focus_win,			/* yield_kbd_focus() */
    NULL				/* extension: reserved for future use */
};


static Defaults_pairs sb_placement_pairs[] = {
	"Right", FALSE,
	"right", FALSE,
	"Left", TRUE,
	"left", TRUE,
	NULL, FALSE,
};


typedef enum {
    INSERT_BEFORE,
    INSERT_AFTER
} Insert_pos_t;



/* ========================================================================= */

/* -------------------- XView Functions  -------------------- */
/*ARGSUSED*/
Pkg_private int
panel_list_init(parent, panel_list_public, avlist)
    Xv_Window       parent;
    Panel_item      panel_list_public;
    Attr_avlist     avlist;
{
    Panel_list_info *dp;
    Xv_panel_list  *panel_list = (Xv_panel_list *) panel_list_public;
    Item_info      *ip = ITEM_PRIVATE(panel_list_public);
    int			chrht;

    ip->item_type = PANEL_LIST_ITEM;
    ip->ops = ops;
    panel_set_bold_label_font(ip);
    dp = xv_alloc(Panel_list_info);

    /* link to object */
    panel_list->private_data = (Xv_opaque) dp;
    dp->public_self = panel_list_public;

    /* Initialize the object */
    dp->nlevels = 1;
    dp->parent_panel = parent;
    dp->choose_one = TRUE; /* exclusive scrolling list */
    dp->edit_op = OP_NONE;
    dp->font = ip->panel->std_font;
#ifdef OW_I18N
    dp->font_set = ip->panel->std_fontset_id;
    dp->stored_length_wc = FALSE;
#else
    dp->font_struct = (XFontStruct *) xv_get(dp->font, FONT_INFO);
#endif /* OW_I18N */
    dp->insert_delete_enabled = TRUE;
    dp->insert_duplicate = TRUE;
    dp->left_hand_sb = defaults_get_enum("OpenWindows.ScrollbarPlacement",
	"OpenWindows.ScrollbarPlacement", sb_placement_pairs);
    dp->list_box.r_left = ip->panel->item_x;
    dp->list_box.r_top = ip->panel->item_y;
    chrht = xv_get(dp->font, FONT_DEFAULT_CHAR_HEIGHT);
    dp->row_height = chrht + PANEL_LIST_ROW_GAP;

    if (ip->panel->status.mouseless) {
	ip->flags |= WANTS_KEY;

	/* A Scrolling List is, by default, a First-Class (primary)
	 * focus client.
	 */
	xv_set(panel_list_public,
	       PANEL_PAINT, PANEL_NONE,
	       XV_FOCUS_RANK, XV_FOCUS_PRIMARY,
	       NULL);

	/* The panel now contains (at least one) First-Class (primary)
	 * focus client
	 */
	xv_set(parent, XV_FOCUS_RANK, XV_FOCUS_PRIMARY, NULL);
    }

    /* Create the text field used in editing mode */
    dp->text_item = xv_create(dp->parent_panel, PANEL_TEXT,
	PANEL_ITEM_OWNER, PANEL_LIST_PUBLIC(dp),
    	PANEL_NOTIFY_STRING,	"\r",   /* RETURN only */
	PANEL_VALUE_DISPLAY_LENGTH, 1, /* some artificial value that doesn't
					* affect the width of the panel
					* (see shrink_to_fit) */
	XV_KEY_DATA, PANEL_LIST, dp,
	XV_SHOW, FALSE,
	NULL);

    return XV_OK;
}


Pkg_private     Xv_opaque
panel_list_set_avlist(panel_list_public, avlist)
    Panel_item      panel_list_public;
    Attr_avlist     avlist;
{
    register Panel_list_info *dp = PANEL_LIST_PRIVATE(panel_list_public);
    register int i;
    int		    forward;
    int		    insert_glyphs;
    Item_info      *ip = ITEM_PRIVATE(panel_list_public);
    int             max_glyph_width;
    Row_info	   *next;	/* next node in list */
    Row_info       *node;	/* current node in list */
    Xv_opaque	   *obj_ptr;	/* array of strings or glyphs */
    Attr_avlist     orig_avlist = avlist;
    Row_info	   *prev;	/* previous node in list */
    int             reset_rows = FALSE;
    Xv_opaque	    result;
    Row_info	    row_tmp;
    Xv_Server	    server;
    struct pr_size  size;
    int		    sort_count;
    int		    which_row;
    int             xv_end_create = FALSE;
#ifdef OW_I18N
    _xv_pswcs_t	    pswcs = {0, NULL, NULL};
#endif /* OW_I18N */

    /* Call generic item set code to handle layout attributes */
    if (*avlist != XV_END_CREATE) {
	/* Prevent panel_redisplay_item from being called in item_set_avlist.
	 * Otherwise, the ip->painted_rect will be cleared prematurely, and
	 * we'll be unable to clear out the old painted rect (especially
	 * important in case the item rect is being moved).
	 */
	ip->panel->no_redisplay_item = TRUE;
	result = xv_super_set_avlist(panel_list_public, &xv_panel_list_pkg,
				     avlist);
	ip->panel->no_redisplay_item = FALSE;
    	if (result != XV_OK)
	    return result;
    }

    for ( ; *avlist; avlist = attr_next(avlist)) {
	switch ((int) avlist[0]) {
	  case PANEL_LIST_ROW_HEIGHT:
	    if (!dp->initialized)
		dp->row_height = (unsigned short) avlist[1];
	    break;
	  case PANEL_LIST_INSERT_DUPLICATE:
	    dp->insert_duplicate = avlist[1] ? TRUE : FALSE;
	    break;
	  default:
	    break;
	}
    }

    /*
     * Process the attributes that rely on the already processed attributes.
     */
    avlist = orig_avlist;
    for (avlist = orig_avlist; *avlist; avlist = attr_next(avlist)) {
	switch ((int) avlist[0]) {


	/*
	 *  Leave this as the first case in the switch!!!
	 *
	 *  This attribute was added for performance only.  rather
	 *  than traversing the linked list multiple times for each
	 *  of these attributes, get the node once and re-use it.
	 *  see the Panel_list_row_values struct in panel.h for
	 *  definition of the struct.
	 */
#ifdef OW_I18N
	  case PANEL_LIST_ROW_VALUES:
	  case PANEL_LIST_ROW_VALUES_WCS: {
	      Attr_attribute	which_attr = avlist[0];
#else
	  case PANEL_LIST_ROW_VALUES: {
#endif /* OW_I18N */
	      Panel_list_row_values *entry;
	      int count;
	      int ii;

	      /*
	       * Note:  this attr is defined as ATTR_OPAQUE_TRIPLE,
	       * it takes the row number, followed by the address of
	       * an array of structs, followed by the number of structs.
	       */
	      ATTR_CONSUME(avlist[0]);
	      which_row = (int) avlist[1];
#ifdef OW_I18N
	      /*
	       * DEPEND_ON_OFFSET_ALIGMENT: Most of the CPU will not
	       * make different offset for Panel_list_row_values and
	       * Panel_list_row_values_wcs, so here in code assume
	       * that is the case.
	       */
#endif
	      entry = (Panel_list_row_values *) avlist[2];
	      count = (int) avlist[3];
	      node = (Row_info *)NULL;


	      for( ii=0; ii<count; ++ii ) {

		  if ( !node )
		      node = find_or_create_nth_row(dp, which_row, TRUE);
		  else
     		      node = gimme_the_next_row(dp, node);

		  
		  /*
		   * do PANEL_LIST_STRING stuff
		   */
#ifdef OW_I18N
	          if (which_attr == PANEL_LIST_ROW_VALUES)
		      _xv_pswcs_mbsdup(&pswcs, entry->string);
		  else
		      pswcs.value = (wchar_t *) entry->string;

		  if (!dp->insert_duplicate &&
		      check_for_duplicate(dp, pswcs.value))
		      break;  /* don't insert a duplicate string */

                  _xv_set_wcs_attr_dup(&node->string, pswcs.value);
		  if (pswcs.value)
		      node->display_str_len = 0;
#else /* OW_I18N */
		  if (!dp->insert_duplicate &&
		      check_for_duplicate(dp, entry->string))
		      break;  /* don't insert a duplicate string */

		  if (node->f.free_string)
		      xv_free(node->string);

		  if (entry->string) {
		      node->string = panel_strsave(entry->string);
		      node->display_str_len = 0;  /* force recomputation */
		      node->f.free_string = TRUE;
		  } else {

		      node->string = (char *)NULL;
		      node->f.free_string = FALSE;
		  }
#endif /* OW_I18N */
		  
		  /*
		   * Do PANEL_LIST_GLYPH stuff
		   */
		  if (entry->glyph)
		      set_row_glyph(dp, node, (Pixrect *) entry->glyph);
		  else
		      node->glyph = NULL;
		  
		  /*
		   * Do PANEL_LIST_MASK_GLYPH stuff
		   */
		  if (entry->mask_glyph)
		      set_row_mask_glyph(dp, node, (Pixrect *) entry->mask_glyph);
		  else
		      node->mask_glyph = NULL;


		  /*
		   * do PANEL_LIST_FONT stuff
		   */
		  set_row_font(dp, node, entry->font);
		  
		  
		  /*
		   * do PANEL_LIST_CLIENT_DATA stuff
		   */
		  node->client_data = entry->client_data;
		  node->exten_data = entry->extension_data;

		  /*
		   * do PANEL_LIST_INACTIVE stuff
		   */
		  node->f.row_inactive = panel_list_row_inactive_get(entry);
		  if ( node->f.row_inactive )
		      node->f.selected = FALSE;
		  
		  /*
		   * do PANEL_LIST_SELECTED stuff
		   */
		  if (!node->f.row_inactive) {
		      if (node->f.selected != entry->selected) {
			  set_current_row(dp, node, NULL);

		      if (ip->panel->status.painted 
			  && !hidden(ip)
			  && entry->selected
			  ) {
			  dp->focus_row = node;
			  make_row_visible(dp, node->row);
		      }
		      }
		  }
		  entry++;
	      } /* for(ii<count) */
	      break;
	  }

	  case PANEL_CHOOSE_NONE:
		dp->choose_none = avlist[1] ? TRUE : FALSE;
		break;
	  case PANEL_CHOOSE_ONE:
		dp->choose_one = avlist[1] ? TRUE : FALSE;
		break;
	  case PANEL_LIST_WIDTH:
		/* -1 = extend width to edge of panel
		 *  0 = fit list box around rows
		 * other = width of list box
		 */
		if ((int) avlist[1] <= 0)
		    dp->width = (int) avlist[1];
		else {
		    dp->width = MAX((int) avlist[1], 2*LIST_BOX_BORDER_WIDTH +
				    2*ROW_MARGIN + PANEL_LIST_COL_GAP);
		}
	        break;
	  case PANEL_LIST_DISPLAY_ROWS:
	        dp->rows_displayed = (int) avlist[1];
	        reset_rows = TRUE;
	        break;

	  case PANEL_LIST_FONTS:
		node = dp->rows;
		for (i = 1; node && avlist[i]; i++) {
		    set_row_font(dp, node, (Xv_Font) avlist[i]);
		    node = node->next;
		}
		break;

	  case PANEL_LIST_FONT:
		node = find_or_create_nth_row(dp, (int) avlist[1], FALSE);
		if (node)
		    set_row_font(dp, node, (Xv_Font) avlist[2]);
		break;

	  case PANEL_ITEM_COLOR:
		if (dp->list_sb)
		    xv_set(dp->list_sb,
			   WIN_FOREGROUND_COLOR, ip->color_index,
			   NULL);
		break;

	  case PANEL_ITEM_MENU:
		if (dp->edit_mode)  {
		    if (dp->edit_menu != (Menu)avlist[1])  {
		        if (dp->destroy_edit_menu && dp->edit_menu)  {
		            xv_destroy(dp->edit_menu);
		        }
		        dp->edit_menu = (Menu) avlist[1];
			if ( dp->edit_menu )
			    xv_set(dp->edit_menu, XV_KEY_DATA, PANEL_LIST, dp, NULL);
		        dp->destroy_edit_menu = FALSE;
		    }
		}
		else  {
		    if (dp->read_menu != (Menu)avlist[1])  {
		        if (dp->destroy_read_menu && dp->read_menu)  {
		            xv_destroy(dp->read_menu);
		        }
		        dp->read_menu = (Menu) avlist[1];
			if ( dp->read_menu )
			    xv_set(dp->read_menu, XV_KEY_DATA, PANEL_LIST, dp, NULL);
		        dp->destroy_read_menu = FALSE;
		    }
		}
	        ATTR_CONSUME(*((Attr_avlist) & avlist[0]));
	        break;

	  case PANEL_LIST_MODE:
		if (!dp->initialized || dp->read_only)
		    break;
		if ((Panel_list_mode) avlist[1] == PANEL_LIST_READ &&
		    dp->edit_mode)
		    enter_read_mode(dp->read_menu, NULL);
		else if ((Panel_list_mode) avlist[1] == PANEL_LIST_EDIT &&
		    !dp->edit_mode)
		    enter_edit_mode(dp->edit_menu, NULL);
		break;
		
	  case PANEL_LIST_SELECT:
		which_row = (int) avlist[1];
		node = find_or_create_nth_row(dp, which_row, FALSE);
		if (node && !node->f.row_inactive) {
		    if (node->f.selected != (int) avlist[2])
			set_current_row(dp, node, NULL);
		    if (ip->panel->status.painted && !hidden(ip)) {
			dp->focus_row = node;
			make_row_visible(dp, node->row);
		    }
		}
		break;

#ifdef OW_I18N
	  case PANEL_LIST_STRINGS:
	  case PANEL_LIST_STRINGS_WCS:
		node = dp->rows;
		for (i = 0; avlist[i+1]; i++) {
		    if ((int) avlist[0] == PANEL_LIST_STRINGS)
			_xv_pswcs_mbsdup(&pswcs, (char *) avlist[i+1]);
		    else
			pswcs.value = (wchar_t *) avlist[i+1];
		    if (!dp->insert_duplicate &&
			check_for_duplicate(dp, pswcs.value))
			continue;  /* don't insert a duplicate string */
		    node = next_row(dp, node, i);
		    _xv_set_wcs_attr_dup(&node->string, pswcs.value);
		    node->display_str_len = 0;  /* force recomputation */
		}
		break;

	  case PANEL_LIST_STRING:
		_xv_pswcs_mbsdup(&pswcs, (char *) avlist[2]);
		goto pls;

	  case PANEL_LIST_STRING_WCS:
		pswcs.value = (wchar_t *) avlist[2];
pls:
		if (!dp->insert_duplicate &&
		    check_for_duplicate(dp, pswcs.value))
		    break;  /* don't insert a duplicate string */
		which_row = (int) avlist[1];
		node = find_or_create_nth_row(dp, which_row, TRUE);
		if (avlist[2])
		    node->display_str_len = 0;  /* force recomputation */
		_xv_set_wcs_attr_dup(&node->string, pswcs.value);
		break;

#else /* OW_I18N */

	  case PANEL_LIST_STRINGS:
		node = dp->rows;
		for (i = 0; avlist[i+1]; i++) {
		    if (!dp->insert_duplicate &&
			check_for_duplicate(dp, (char *) avlist[i+1]))
			continue;  /* don't insert a duplicate string */
		    node = next_row(dp, node, i);
		    if (node->f.free_string)
			free(node->string);
		    node->string = panel_strsave((char *) avlist[i+1]);
		    node->display_str_len = 0;  /* force recomputation */
		    node->f.free_string = TRUE;
		}
    		if (dp->rows_displayed == 0) {
		    if (dp->nrows < PANEL_LIST_DEFAULT_ROW)
	                dp->rows_displayed = dp->nrows;
	            else
	                dp->rows_displayed = PANEL_LIST_DEFAULT_ROW;
                }
		reset_rows = 1;
		break;

	  case PANEL_LIST_STRING:
		if (!dp->insert_duplicate &&
		    check_for_duplicate(dp, (char *) avlist[2]))
		    break;  /* don't insert a duplicate string */
		which_row = (int) avlist[1];
		node = find_or_create_nth_row(dp, which_row,
					     TRUE);
		if (node->f.free_string)
		    free(node->string);
		if (avlist[2]) {
		    node->string = panel_strsave((char *) avlist[2]);
		    node->display_str_len = 0;  /* force recomputation */
		    node->f.free_string = TRUE;
		} else {
		    node->string = (char *) avlist[2];
		    node->f.free_string = FALSE;
		}
		break;
#endif /* OW_I18N */

	  case PANEL_LIST_GLYPHS:
		i = 0;
		node = dp->rows;
		while (avlist[i+1]) {
		    node = next_row(dp, node, i++);
		    set_row_glyph(dp, node, (Pixrect *) avlist[i]);
		}
		break;

	  case PANEL_LIST_GLYPH:
		which_row = (int) avlist[1];
		node = find_or_create_nth_row(dp,
					     which_row, TRUE);
		if (avlist[2])
		    set_row_glyph(dp, node, (Pixrect *) avlist[2]);
		else
		    node->glyph = NULL;
		break;

	  case PANEL_LIST_MASK_GLYPHS:
		i = 0;
		node = dp->rows;
		while (avlist[i+1]) {
		    node = next_row(dp, node, i++);
		    set_row_mask_glyph(dp, node, (Pixrect *) avlist[i]);
		}
		break;

	  case PANEL_LIST_MASK_GLYPH:
		which_row = (int) avlist[1];
		node = find_or_create_nth_row(dp,
					     which_row, TRUE);
		if (avlist[2])
		    set_row_mask_glyph(dp, node, (Pixrect *) avlist[2]);
		else
		    node->mask_glyph = NULL;
		break;

#ifdef OW_I18N
	  case PANEL_LIST_TITLE:
		_xv_set_mbs_attr_dup(&dp->title, (char *) avlist[1]);
		break;

	  case PANEL_LIST_TITLE_WCS:
		_xv_set_wcs_attr_dup(&dp->title, (wchar_t *) avlist[1]);
		break;

#else /* OW_I18N */
	  case PANEL_LIST_TITLE:
		if (dp->title)
		    xv_free(dp->title);
		if (avlist[1])
		    dp->title = panel_strsave((char *) avlist[1]);
		else
		    dp->title = NULL;
		break;
#endif /* OW_I18N */

	  case PANEL_LIST_CLIENT_DATAS:
	  	i=1;
		node = dp->rows;
		while (avlist[i]) {
		    if (node) {
			node->client_data = (Xv_opaque) avlist[i];
		    }
		    node = node->next;
		    i++;
		}
		break;

	  case PANEL_LIST_CLIENT_DATA:
		which_row = (int) avlist[1];
		node = find_or_create_nth_row(dp, which_row, FALSE);
		if (node)
		    node->client_data = (Xv_opaque) avlist[2];
		break;

	  case PANEL_LIST_INSERT:
		if (!dp->insert_delete_enabled) {
		    xv_error(panel_list_public,
			ERROR_STRING,
		            XV_MSG("PANEL_LIST insertions are currently disabled"),
		        ERROR_PKG, PANEL,
			NULL);
		    break;
		}
		which_row = (int) avlist[1];
		panel_list_insert_row(dp, which_row, SHOW_ROW,
				      DO_NOT_REPAINT_LIST);
		break;

	  case PANEL_LIST_INSERT_GLYPHS:
	  case PANEL_LIST_INSERT_STRINGS:
#ifdef OW_I18N
	  case PANEL_LIST_INSERT_STRINGS_WCS:
#endif
		if (!dp->insert_delete_enabled) {
		    xv_error(panel_list_public,
			ERROR_STRING,
		            XV_MSG("PANEL_LIST insertions are currently disabled"),
		        ERROR_PKG, PANEL,
			NULL);
		    break;
		}
		which_row = (int) avlist[1];
		/* Find node in list to insert new nodes before */
		prev = NULL;
		for (next = dp->rows; next && next->row != which_row;
		     next = next->next)
		    prev = next;
		if (prev)
		    which_row = prev->row + 1; /* in case we're appending */
		else
		    which_row = 0;
		/* Insert new rows */
		insert_glyphs = avlist[0] == PANEL_LIST_INSERT_GLYPHS;
		node = NULL;
		for (obj_ptr = (Xv_opaque *) avlist[2]; *obj_ptr; obj_ptr++) {
#ifdef OW_I18N
		    if (!insert_glyphs) {
			if ((int) avlist[0] == PANEL_LIST_INSERT_STRINGS)
			    _xv_pswcs_mbsdup(&pswcs, (char *) *obj_ptr);
			else
			    pswcs.value = (wchar_t *) *obj_ptr;
			if (!dp->insert_duplicate
			 && check_for_duplicate(dp, pswcs.value))
			    continue;
		    }
#else
		    if (!insert_glyphs && !dp->insert_duplicate &&
			check_for_duplicate(dp, (char *) *obj_ptr))
			continue;  /* don't insert duplicate string */
#endif /* OW_I18N */
		    node = xv_alloc(Row_info);
		    if (prev)
			prev->next = node;
		    else {
			dp->rows = node;
			if (!dp->focus_row)
			    dp->focus_row = node;
		    }
		    node->prev = prev;
		    node->row = which_row++;
		    node->f.show = TRUE;
		    node->string_y = LIST_BOX_BORDER_WIDTH + ROW_MARGIN +
			node->row*dp->row_height;
		    if (insert_glyphs)
			set_row_glyph(dp, node, (Pixrect *) *obj_ptr);
		    else {
#ifdef OW_I18N
			_xv_set_wcs_attr_dup(&node->string, pswcs.value);
#else
			if (node->f.free_string)
			    free(node->string);
			node->string = panel_strsave((char *) *obj_ptr);
			node->f.free_string = TRUE;
#endif /* OW_I18N */
		    }
		    prev = node;
		}
		if (!node)
		    break;   /* no new nodes were added */
		if (next) {
		    /* Point the last new node to the following node */
		    node->next = next;
		    /* Update the nodes that follow the new nodes */
		    next->prev = node;
		    for (node = next; node; node = node->next) {
			node->row = which_row++;
			node->string_y = LIST_BOX_BORDER_WIDTH + ROW_MARGIN +
			    node->row*dp->row_height;
		    }
		}
		/* Update Scrolling List and it's scrollbar */
		dp->nrows = which_row;
		if (dp->list_sb)
		    xv_set(dp->list_sb,
			   SCROLLBAR_OBJECT_LENGTH, dp->nrows,
			   NULL);
		break;

	  case PANEL_LIST_DELETE:
		if (!dp->insert_delete_enabled) {
		    xv_error(panel_list_public,
			ERROR_STRING,
		            XV_MSG("PANEL_LIST deletions are currently disabled"),
		        ERROR_PKG, PANEL,
			NULL);
		    break;
		}
		which_row = (int) avlist[1];
		node = dp->rows;

		while (node && (node->row != which_row))
		    node = node->next;
		if (node)
		    panel_list_delete_row(dp, node, DO_NOT_REPAINT_LIST);
		break;

	  case PANEL_LIST_DELETE_ROWS:
		if (!dp->insert_delete_enabled) {
		    xv_error(panel_list_public,
			ERROR_STRING,
		            XV_MSG("PANEL_LIST deletions are currently disabled"),
		        ERROR_PKG, PANEL,
			NULL);
		    break;
		}
		which_row = (int) avlist[1];
		node = dp->rows;

		/* Find the first row to delete */
		while (node && (node->row != which_row))
		    node = node->next;
		if (!node)
		    break;
		prev = node->prev;
		/* Delete the requested number of rows */
		for (i = 1; node && i <= (int) avlist[2]; i++) {
		    next = node->next;
#ifdef OW_I18N
		    _xv_free_ps_string_attr_dup(&node->string);
#else
		    if (node->f.free_string)
			xv_free(node->string);
#endif
		    if (dp->focus_row == node)
			dp->focus_row = NULL;
		    if (dp->current_row == node)
			dp->current_row = NULL;
		    xv_free(node);
		    node = next;
		}
		if (prev)
		    prev->next = next;
		else
		    dp->rows = next;
		if (!dp->focus_row)
		    dp->focus_row = next;
		/* Update the nodes that follow the deleted nodes */
		if (next) {
		    next->prev = prev;
		    for (node = next; node; node = node->next) {
			node->row = which_row++;
			node->string_y = LIST_BOX_BORDER_WIDTH + ROW_MARGIN +
			    node->row*dp->row_height;
		    }
		}
		/* Update Scrolling List and it's scrollbar */
		dp->nrows = which_row;
		if (dp->list_sb)
		    xv_set(dp->list_sb,
			   SCROLLBAR_OBJECT_LENGTH, dp->nrows,
			   NULL);
		break;

	  case PANEL_LIST_DELETE_SELECTED_ROWS:
	    if (!created(ip) || !dp->insert_delete_enabled) {
		xv_error(panel_list_public,
		    ERROR_STRING,
			XV_MSG("PANEL_LIST deletions are currently disabled"),
		    ERROR_PKG, PANEL,
		    NULL);
		break;	/* not valid on xv_create or in notify proc */
	    }
	    for (node = dp->rows; node; node = next) {
		next = node->next;
		if (node->f.selected)
		    panel_list_delete_row(dp, node, DO_NOT_REPAINT_LIST);
	    }
	    break;

	  case PANEL_INACTIVE:
	    if (avlist[1] && ip->panel->kbd_focus_item == ip) {
		if (dp->initialized)
		    panel_list_paint(panel_list_public);
		ip->panel->kbd_focus_item =
		    panel_next_kbd_focus(ip->panel, TRUE);
		panel_accept_kbd_focus(ip->panel);
	    }
	    if (dp->list_sb)
		xv_set(dp->list_sb, SCROLLBAR_INACTIVE, avlist[1], NULL);
	    break;

	  case PANEL_LIST_INACTIVE:
		which_row = (int) avlist[1];
		node = find_or_create_nth_row(dp, which_row, FALSE);
		if (node) {
		    node->f.row_inactive = (unsigned int) avlist[2];
		    if ( node->f.row_inactive )
			node->f.selected = FALSE;
		}
		break;

	  case PANEL_LIST_DELETE_INACTIVE_ROWS:
	    if (!created(ip) || !dp->insert_delete_enabled) {
		xv_error(panel_list_public,
		    ERROR_STRING,
			XV_MSG("PANEL_LIST deletions are currently disabled"),
		    ERROR_PKG, PANEL,
		    NULL);
		break;	/* not valid on xv_create or in notify proc */
	    }
	    for (node = dp->rows; node; node = next) {
		next = node->next;
		if (node->f.row_inactive)
		    panel_list_delete_row(dp, node, DO_NOT_REPAINT_LIST);
	    }
	    break;

	  case PANEL_LIST_DO_DBL_CLICK:
	    dp->do_dbl_click = (int)avlist[1];
	    break;

	  case PANEL_LIST_EXTENSION_DATAS:
	  	i=1;
		node = dp->rows;
		while (avlist[i]) {
		    if (node) {
			node->exten_data = (Xv_opaque) avlist[i];
		    }
		    node = node->next;
		    i++;
		}
		break;

	  case PANEL_LIST_EXTENSION_DATA:
		which_row = (int) avlist[1];
		node = find_or_create_nth_row(dp, which_row, FALSE);
		if (node)
		    node->exten_data = (Xv_opaque) avlist[2];
		break;

	  case PANEL_READ_ONLY:
	    dp->read_only = avlist[1] ? TRUE : FALSE;
	    break;

	  case PANEL_LIST_SORT:
	    if (!created(ip))
		break;	/* not valid on xv_create */
	    forward = (Panel_setting) avlist[1] == PANEL_FORWARD;
	    do {
		sort_count = 0;
		for (node = dp->rows; node && node->next; node = node->next) {
		    if (!node->STRING)
			continue;
		    next = node->next;
#ifdef OW_I18N
		    if (!next->STRING ||
		         (forward ? (wscoll(node->string.pswcs.value,
					    next->string.pswcs.value) > 0)
				  : (wscoll(node->string.pswcs.value,
					    next->string.pswcs.value) < 0)))
#else
		    if (!next->string ||
			(forward ? (strcmp(node->string, next->string) > 0)
				 : (strcmp(node->string, next->string) < 0)))
#endif /* OW_I18N */
		    {
			sort_count++;
			/* Swap contents of nodes */
			row_tmp = *node;
			node->client_data = next->client_data;
			node->display_str_len = next->display_str_len;
			node->font = next->font;
			node->glyph = next->glyph;
			node->string = next->string;
			node->f = next->f;
			next->client_data = row_tmp.client_data;
			next->display_str_len = row_tmp.display_str_len;
			next->font = row_tmp.font;
			next->glyph = row_tmp.glyph;
			next->string = row_tmp.string;
			next->f = row_tmp.f;

			if ( node == dp->current_row )
			    dp->current_row = next;
			else if ( next == dp->current_row )
			    dp->current_row = node;
		    }
		}
	    } while (sort_count);
	    break;

	  case PANEL_VALUE_STORED_LENGTH:
#ifdef OW_I18N
	    dp->stored_length_wc = FALSE;
#endif
	    xv_set(dp->text_item,
		   PANEL_VALUE_STORED_LENGTH, avlist[1],
		   NULL);
	    break;

#ifdef OW_I18N
	  case PANEL_VALUE_STORED_LENGTH_WCS:
	    dp->stored_length_wc = TRUE;
	    xv_set(dp->text_item,
		   PANEL_VALUE_STORED_LENGTH_WCS, avlist[1],
		   NULL);
	    break;
#endif

	  case XV_SHOW:
	    if (dp->text_item_row)
		xv_set(dp->text_item, XV_SHOW, avlist[1], NULL);
	    if (dp->list_sb)
		xv_set(dp->list_sb, XV_SHOW, avlist[1], NULL);
	    break;

	  case XV_END_CREATE:
	    panel_list_create_displayarea(dp);
	    dp->initialized = TRUE;
	    xv_end_create = TRUE;
	    break;

	  default:
	    break;
	}
    }

    /* Non-exclusive lists can have no current row */
    if (!dp->choose_one)
	dp->choose_none = TRUE;

	/* HELP!  Optimize Me!  shouldn't do 2 list traversals! */
    if (!dp->choose_none && dp->rows && !dp->setting_current_row) {
        /* If no row is selected, then select the first row */
        for (node = dp->rows; node; node = node->next)
            if (node->f.selected && !node->f.row_inactive)
            	break;
        if (!node) {
	    for (node=dp->rows; node; node = node->next)
		if ( !node->f.row_inactive ) {
		    dp->current_row = node;
		    node->f.selected = TRUE;
		    break;
		}
        }
    }

    if (reset_rows && dp->initialized) {
	dp->list_box.r_height = 2*LIST_BOX_BORDER_WIDTH +
	    2*ROW_MARGIN + dp->rows_displayed*dp->row_height;
	xv_set(dp->list_sb,
	    SCROLLBAR_VIEW_LENGTH, dp->rows_displayed,
	    XV_HEIGHT, dp->list_box.r_height,
	    NULL);
	dp->sb_rect.r_height = dp->list_box.r_height;
    }

    /* Horizontally align strings in row panel */
    max_glyph_width = 0;
    for (node = dp->rows; node; node = node->next)
	if (node->glyph) {
	    if (node->glyph->pr_width > max_glyph_width)
		max_glyph_width = node->glyph->pr_width;
	}
    dp->string_x = LIST_BOX_BORDER_WIDTH + ROW_MARGIN + PANEL_LIST_COL_GAP;
    if (max_glyph_width) {
	dp->string_x += max_glyph_width;
    }

    if (dp->initialized) {
	switch (ip->layout) {
	  case PANEL_HORIZONTAL:
	    ip->value_rect.r_left = rect_right(&ip->label_rect) + 1 +
		(ip->label_rect.r_width ? LABEL_X_GAP : 0);
	    ip->value_rect.r_top = ip->label_rect.r_top;
	    break;
	  case PANEL_VERTICAL:
	    ip->value_rect.r_left = ip->label_rect.r_left;
	    ip->value_rect.r_top = rect_bottom(&ip->label_rect) + 1 +
		(ip->label_rect.r_height ? LABEL_Y_GAP : 0);
	    break;
	}
	if (dp->TITLE) {
	    dp->title_rect.r_height = LEVEL_DOT_HEIGHT +
		(int) xv_get(ip->panel->bold_font, FONT_DEFAULT_CHAR_HEIGHT) +
		PANEL_LIST_ROW_GAP;
	    size = XV_PF_TEXTWIDTH(STRLEN(dp->TITLE),
				   ip->panel->bold_font,
				   dp->TITLE);
	    dp->title_rect.r_width = LIST_BOX_BORDER_WIDTH*2 +
		ROW_MARGIN*2 + size.x;
	} else
	    rect_construct(&dp->title_rect, 0, 0, 0, 0);
	dp->title_rect.r_left = ip->value_rect.r_left;
	dp->title_rect.r_top = ip->value_rect.r_top;
	dp->list_box.r_left = dp->title_rect.r_left;
	dp->list_box.r_top = dp->title_rect.r_top + dp->title_rect.r_height;
	if (dp->left_hand_sb) {
	    dp->title_rect.r_left += dp->sb_rect.r_width;
	    dp->list_box.r_left += dp->sb_rect.r_width;
	}
	compute_dimensions(ip, dp);
	if (xv_end_create) {
	    /*
	     * Note: item_set_avlist sets ip->rect to the enclosing rect of
	     * ip->label_rect and ip->value_rect. However, on XV_END_CREATE,
	     * item_set_avlist gets called before panel_list_set_avlist.  So, if
	     * we're processing the XV_END_CREATE, we must also set ip->rect.
	     */
	    ip->rect = panel_enclosing_rect(&ip->label_rect, &ip->value_rect);
	    panel_check_item_layout(ip);

	    /* Create Row Panel menus, if not user defined.  */
	    if (!dp->read_menu) {
		server = XV_SERVER_FROM_WINDOW(PANEL_PUBLIC(ip->panel));
		/*
		 * Set destroy flag
		 */
		dp->destroy_read_menu = TRUE;
		dp->read_menu = xv_create(server, MENU,
		    XV_KEY_DATA, PANEL_LIST, dp,
		    MENU_TITLE_ITEM, XV_MSG("Scrolling List"),
		    XV_KEY_DATA, MENU_DONE_PROC, NULL,
		    MENU_ITEM,
			MENU_STRING, dp->choose_one ?
			    XV_MSG("Locate Choice") : 
			    XV_MSG("Locate Next Choice"),
			MENU_ACTION,	locate_next_choice,
			0,
		    NULL);
		if (!dp->choose_one)
		    xv_set(dp->read_menu,
			MENU_APPEND_ITEM,
			    xv_create(XV_ZERO, MENUITEM,
				MENU_STRING, 
				    XV_MSG("Clear All Choices"),
				MENU_ACTION, clear_all_choices,
				MENU_RELEASE,
				NULL),
			NULL);
		if (!dp->read_only) {
		    xv_set(dp->read_menu,
			MENU_APPEND_ITEM,
			    xv_create(XV_ZERO, MENUITEM,
				MENU_STRING, 
				XV_MSG("Edit List"),
				MENU_ACTION, enter_edit_mode,
				MENU_RELEASE,
				NULL),
		        NULL);
		    if (!dp->edit_menu) {
			/*
			 * Set destroy flag
			 */
			dp->destroy_edit_menu = TRUE;
			dp->edit_menu = xv_create(server, MENU,
			    XV_KEY_DATA, PANEL_LIST, dp,
			    MENU_TITLE_ITEM, XV_MSG("Scrolling List"),
			    XV_KEY_DATA, MENU_DONE_PROC, NULL,
			    MENU_ITEM,
				MENU_STRING, 
				XV_MSG("Change"),
				MENU_ACTION, change_proc,
				0,
			    MENU_ITEM,
				MENU_STRING, XV_MSG("Insert"),
				MENU_PULLRIGHT,
				    xv_create(server, MENU,
					XV_KEY_DATA, PANEL_LIST, dp,
					MENU_ITEM,
					    MENU_STRING, 
					    XV_MSG("Before"),
					    MENU_ACTION, insert_proc,
					    XV_KEY_DATA, PANEL_INSERT,
						INSERT_BEFORE,
					    0,
					MENU_ITEM,
					    MENU_STRING, 
					    XV_MSG("After"),
					    MENU_ACTION, insert_proc,
					    XV_KEY_DATA, PANEL_INSERT,
						INSERT_AFTER,
					    0,
					NULL),
				0,
			    MENU_ITEM,
				MENU_STRING, XV_MSG("Delete"),
				MENU_ACTION, delete_proc,
				0,
			    MENU_ITEM,
				MENU_STRING,	"",
				MENU_FEEDBACK, FALSE,
				0,
			    MENU_ITEM,
				MENU_STRING, 
				    XV_MSG("End Editing"),
				MENU_ACTION, enter_read_mode,
				0,
			    NULL);
		    }   /* if (!dp->edit_menu) */
		}   /* if (!dp->read_only) */
	    }   /* if (!dp->read_menu) */
	    if (ip->panel->status.painted) {
		if (dp->TITLE)
		    paint_title_box(dp);
		paint_list_box(dp);
	    }
	}   /* if (xv_end_create) */
	else {
	    xv_set(dp->list_sb,
		   SCROLLBAR_OBJECT_LENGTH, dp->nrows,
		   NULL);
	}
    }   /* if (dp->initialized) */

#ifdef OW_I18N
    if (pswcs.storage != NULL)
	xv_free(pswcs.storage);
#endif
    return XV_OK;
}


Pkg_private     Xv_opaque
panel_list_get_attr(panel_list_public, status, which_attr, valist)
    Panel_item      panel_list_public;
    int            *status;
    Attr_attribute  which_attr;
    va_list         valist;
{
    Panel_list_info *dp = PANEL_LIST_PRIVATE(panel_list_public);
    Row_info	   *node;
    int		    row;

    switch (which_attr) {

      case PANEL_CHOOSE_NONE:
	return ((Xv_opaque) dp->choose_none);

      case PANEL_CHOOSE_ONE:
	return ((Xv_opaque) dp->choose_one);

      case PANEL_ITEM_NTH_WINDOW:
/* Alpha compatibility, mbuck@debian.org */
#if 1
	if (va_arg(valist, int) == 0)
#else
	if (*(int *) valist == 0)
#endif
	    return (Xv_opaque) dp->list_sb;
	else
	    return (Xv_opaque) NULL;

      case PANEL_ITEM_NWINDOWS:
	return (Xv_opaque) 1;

      case PANEL_LIST_INSERT_DUPLICATE:
	return ((Xv_opaque) dp->insert_duplicate ? TRUE : FALSE);

      case PANEL_LIST_MODE:
	return ((Xv_opaque) dp->edit_mode ? PANEL_LIST_EDIT : PANEL_LIST_READ);

      case PANEL_LIST_WIDTH:
	return ((Xv_opaque) dp->width ? dp->width : dp->list_box.r_width);

      case PANEL_LIST_DISPLAY_ROWS:
	return ((Xv_opaque) dp->rows_displayed);

      case PANEL_LIST_ROW_HEIGHT:
	return (Xv_opaque) dp->row_height;

      case PANEL_LIST_SCROLLBAR:
	return (Xv_opaque) dp->list_sb;

      case PANEL_LIST_SELECTED:
/* Alpha compatibility, mbuck@debian.org */
#if 1
	row = va_arg(valist, int);
#else
	row = *(int *) valist;
#endif
	node = find_or_create_nth_row(dp, row, FALSE);
	return (node ? (Xv_opaque) node->f.selected : (Xv_opaque) XV_ERROR);

      case PANEL_LIST_FIRST_SELECTED:
	for (node=dp->rows; node; node = node->next) {
	    if (node->f.selected)
		return (node->row);
	}
	return -1;	/* no rows selected */
	
      case PANEL_LIST_NEXT_SELECTED:
/* Alpha compatibility, mbuck@debian.org */
#if 1
	row = va_arg(valist, int);
#else
	row = *(int *) valist;
#endif
	node = find_or_create_nth_row(dp, row, FALSE);
	if (!node)
	    return -1;	/* specified row doesn't exist */
	while (node=node->next) {
	    if (node->f.selected)
		return (node->row);
	}
	return -1;	/* no subsequent row selected */

      case PANEL_LIST_CLIENT_DATA:
/* Alpha compatibility, mbuck@debian.org */
#if 1
	row = va_arg(valist, int);
#else
	row = *(int *) valist;
#endif
	node = find_or_create_nth_row(dp, row, FALSE);
	return (node ? (Xv_opaque) node->client_data : (Xv_opaque) XV_ERROR);

      case PANEL_LIST_STRING:
#ifdef OW_I18N
      case PANEL_LIST_STRING_WCS:
#endif
/* Alpha compatibility, mbuck@debian.org */
#if 1
	row = va_arg(valist, int);
#else
	row = *(int *) valist;
#endif
	node = find_or_create_nth_row(dp, row, FALSE);
#ifdef OW_I18N
	if (node != NULL)
	    if (which_attr == PANEL_LIST_STRING_WCS)
		return (Xv_opaque) _xv_get_wcs_attr_dup(&node->string);
	    else
	        return (Xv_opaque) _xv_get_mbs_attr_dup(&node->string);
	else
	    return (Xv_opaque) XV_ERROR;
#else
	return (node ? (Xv_opaque) node->string : (Xv_opaque) XV_ERROR);
#endif /* OW_I18N */

      case PANEL_LIST_GLYPH:
/* Alpha compatibility, mbuck@debian.org */
#if 1
	row = va_arg(valist, int);
#else
	row = *(int *) valist;
#endif
	node = find_or_create_nth_row(dp, row, FALSE);
	return (node ? (Xv_opaque) node->glyph : (Xv_opaque) XV_ERROR);

#ifdef OW_I18N
      case PANEL_LIST_TITLE:
	return (Xv_opaque) _xv_get_mbs_attr_dup(&dp->title);

      case PANEL_LIST_TITLE_WCS:
	return (Xv_opaque) _xv_get_wcs_attr_dup(&dp->title);
#else
      case PANEL_LIST_TITLE:
	return ((Xv_opaque) dp->title);
#endif /* OW_I18N */

      case PANEL_LIST_FONT:
/* Alpha compatibility, mbuck@debian.org */
#if 1
	row = va_arg(valist, int);
#else
	row = *(int *) valist;
#endif
	node = find_or_create_nth_row(dp, row, FALSE);
	return (node ? (Xv_opaque) node->font : (Xv_opaque) XV_ERROR);

      case PANEL_LIST_NROWS:
	return ((Xv_opaque) dp->nrows);

      case PANEL_ITEM_MENU:
	return ((Xv_opaque) dp->edit_mode ? dp->edit_menu :
	    dp->read_menu);

      case PANEL_READ_ONLY:
	return ((Xv_opaque) dp->read_only);

      case PANEL_VALUE_STORED_LENGTH:
	return (xv_get(dp->text_item, PANEL_VALUE_STORED_LENGTH));

#ifdef OW_I18N
      case PANEL_VALUE_STORED_LENGTH_WCS:
	return (xv_get(dp->text_item, PANEL_VALUE_STORED_LENGTH_WCS));
#endif /* OW_I18N */


    case PANEL_LIST_INACTIVE:
/* Alpha compatibility, mbuck@debian.org */
#if 1
	row = va_arg(valist, int);
#else
	row = *(int *) valist;
#endif
	node = find_or_create_nth_row(dp, row, FALSE);
	return (node ? (Xv_opaque) node->f.row_inactive : (Xv_opaque) XV_ERROR);

    case PANEL_LIST_DO_DBL_CLICK:
	return (Xv_opaque) dp->do_dbl_click;

#ifdef OW_I18N
    case PANEL_LIST_ROW_VALUES:
    case PANEL_LIST_ROW_VALUES_WCS: {
#else
    case PANEL_LIST_ROW_VALUES: {
#endif
	Panel_list_row_values *entry;
	int count;
	int ii;

	row = va_arg(valist, int);
#ifdef OW_I18N
	/*
	 * DEPEND_ON_OFFSET_ALIGMENT: Most of the CPU will not make
	 * different offset for Panel_list_row_values and
	 * Panel_list_row_values_wcs, so here in code assume that is
	 * the case.
	 */
#endif
	entry = va_arg(valist, Panel_list_row_values *);
	count = va_arg(valist, int);
	node = find_or_create_nth_row(dp, row, FALSE);

	for(ii=0; ii<count; ++ii) {
	    if ( !node )
		return ii;
#ifdef OW_I18N
	    if (which_attr == PANEL_LIST_ROW_VALUES_WCS)
		entry->string = (char *) _xv_get_wcs_attr_dup(&node->string);
	    else
		entry->string = _xv_get_mbs_attr_dup(&node->string);
#else
	    entry->string = node->string;
#endif /* OW_I18N */
	    entry->glyph = (Server_image) node->glyph;
	    entry->mask_glyph = (Server_image) node->mask_glyph;
	    entry->font = node->font;
	    entry->client_data = node->client_data;
	    entry->extension_data = node->exten_data;
	    panel_list_row_inactive_set(entry, node->f.row_inactive);
	    entry->selected = node->f.selected;
	    entry++;
	    node = node->next;
	}
	return (Xv_opaque) count;
    }

    case PANEL_LIST_EXTENSION_DATA:
/* Alpha compatibility, mbuck@debian.org */
#if 1
	row = va_arg(valist, int);
#else
	row = *(int *) valist;
#endif
	node = find_or_create_nth_row(dp, row, FALSE);
	return (node ? (Xv_opaque) node->exten_data : (Xv_opaque) XV_ERROR);

      default:
	*status = XV_ERROR;
	return ((Xv_opaque) 0);
    }
}


Pkg_private int
panel_list_destroy(item_public, status)
    Panel_item      item_public;
    Destroy_status  status;
{
    Panel_list_info *dp = PANEL_LIST_PRIVATE(item_public);
    Row_info       *row = dp->rows;
    Row_info       *next;

    if ((status == DESTROY_CHECKING) || (status == DESTROY_SAVE_YOURSELF)) {
	return XV_OK;
    }
    panel_list_remove(item_public);
    while (row) {
	next = row->next;
#ifdef OW_I18N
	_xv_free_ps_string_attr_dup(&row->string);
#else
	if (row->f.free_string)
	    xv_free(row->string);
#endif /* OW_I18N */
	xv_free(row);
	row = next;
    }
#ifdef OW_I18N
    _xv_free_ps_string_attr_dup(&dp->title);
#else
    if (dp->title)
	xv_free(dp->title);
#endif
    xv_destroy(dp->text_item);
    /*
     * Check destroy flag before destroying read/edit menu
     * This is to prevent destroying a menu supplied by
     * PANEL_ITEM_MENU
     */
    if (dp->destroy_read_menu && dp->read_menu)  {
        xv_destroy(dp->read_menu);
    }
    if (dp->destroy_edit_menu && dp->read_menu)  {
        xv_destroy(dp->edit_menu);
    }
    xv_destroy(dp->list_sb);
    xv_free(dp);

    return XV_OK;
}



/* --------------------  Panel Item Operations  -------------------- */
static void
panel_list_handle_event(item_public, event)
    Panel_item      item_public;
    Event          *event;
{
    Panel_list_info *dp = PANEL_LIST_PRIVATE(item_public);
    int		    i;
    Item_info	   *ip = ITEM_PRIVATE(item_public);
    int		    navigation_cmd;
    Panel_info	   *panel = PANEL_PRIVATE(dp->parent_panel);
    Rect	    rect;
    Row_info	   *row;
    int		    row_nbr;
    struct timeval  wait;
    int		    y_offset;
#ifdef OW_I18N
    wchar_t	    wc;
    char	    mb;
#endif /* OW_I18N */

    if (event_action(event) != SCROLLBAR_REQUEST &&
	(dp->sb_active ||
	 (event_is_button(event) && event_is_down(event) &&
	  rect_includespoint(&dp->sb_rect, event_x(event), event_y(event))))) {
	event_window(event) = dp->list_sb;
	event_x(event) -= dp->sb_rect.r_left;
	event_y(event) -= dp->sb_rect.r_top;
	if (event_is_button(event)) {
	    dp->sb_active = event_is_down(event);
	    panel->status.current_item_active = event_is_down(event);
	}
	win_post_event(dp->list_sb, event, NOTIFY_IMMEDIATE);
	return;
    }

    if (inactive(ip) || event_action(event) == PANEL_EVENT_CANCEL)
	return;

    if  (event_action(event) == SCROLLBAR_REQUEST) {
	/* Scroll request received from scrollbar */
	panel_clear_rect(panel, dp->list_box);
	paint_list_box(dp);
	return;
    } else if (event_is_iso(event)) {
        if (event_is_up(event))
            return;
	/* Move the Location Cursor to the row starting with the character
	 * typed that is after the current focus row, if any.
	 * Match is case-insensitive.
	 */
	if (dp->focus_row && dp->focus_row->next) {
#ifdef OW_I18N
	    if (event_is_string(event)) {
		if (mbtowc(&wc, event_string(event), MB_CUR_MAX) <= 0)
		    goto beep;
	    } else {
		mb = event_action(event);
		if (mbtowc(&wc, &mb, 1) <= 0)
		    goto beep;
	    }
#endif
	    for (row = dp->focus_row->next;
		 row != dp->focus_row && row->STRING;) {
#ifdef OW_I18N
		 if (row->STRING[0] == wc ||
		     (iswlower(wc) &&
				 row->STRING[0] == towupper(wc)) ||
		     (iswupper(wc) &&
				 row->STRING[0] == towlower(wc))) {
#else
		if (row->string[0] == event_action(event) ||
		    (event_action(event) >= 'a' &&
		     event_action(event) <= 'z' &&
		     row->string[0] == event_action(event) - 0x20) ||
		    (event_action(event) >= 'A' &&
		     event_action(event) <= 'Z' &&
		     row->string[0] == event_action(event) + 0x20)
		   ) {
#endif
		     /* Character match: move Location Cursor to row */
		     dp->focus_row = row;
		     show_focus_win(item_public);
		     return;
		 }
		row = row->next;
		if (!row)
		    /* Wrap to first row */
		    row = dp->rows;
	    }
	}
#ifdef OW_I18N
beep:
#endif
	/* No character match: beep user */
	wait.tv_sec = 0;
	wait.tv_usec = 0;
	win_bell(event_window(event), wait, 0);
	return;
    } else if (!event_is_button(event) && event_action(event) != LOC_DRAG) {
	if (event_action(event) != ACTION_MENU &&
	    (!dp->focus_row ||
	     (event_action(event) != ACTION_ADJUST && event_is_up(event))))
	    /* Note: we need to pass ADJUST-up through in case a
	     * panel_user_error is in effect.
	     */
	    return;
	navigation_cmd = TRUE;
	row = NULL;
	switch (event_action(event)) {
	  case ACTION_UP:
	    row = dp->focus_row->prev;
	    break;
	  case ACTION_DOWN:
	    row = dp->focus_row->next;
	    break;
	  case ACTION_JUMP_UP:
	    for (i=1, row=dp->focus_row; (unsigned)i<=dp->rows_displayed && row->prev;
		 i++)
		row = row->prev;
	    break;
	  case ACTION_JUMP_DOWN:
	    for (i=1, row=dp->focus_row; (unsigned)i<=dp->rows_displayed && row->next;
		 i++)
		row = row->next;
	    break;
	  case ACTION_PANE_UP:
	    row_nbr = (int) xv_get(dp->list_sb, SCROLLBAR_VIEW_START);
	    row = find_or_create_nth_row(dp, row_nbr, FALSE);
	    break;
	  case ACTION_PANE_DOWN:
	    row_nbr = MIN((unsigned) xv_get(dp->list_sb, SCROLLBAR_VIEW_START) +
		dp->rows_displayed - 1, dp->nrows - 1);
	    row = find_or_create_nth_row(dp, row_nbr, FALSE);
	    break;
	  case ACTION_DATA_START:
	    row = dp->rows;
	    break;
	  case ACTION_DATA_END:
	    for (row = dp->focus_row; row->next;)
		row = row->next;
	    break;
	  case ACTION_SELECT:
	  case ACTION_ADJUST:
	  case ACTION_MENU:
	    navigation_cmd = FALSE;
	    row = dp->focus_row;
	    event_set_x(event, dp->list_box.r_left);
	    if (row && get_row_rect(dp, row, &rect)) {
		event_set_y(event, rect.r_top +
		    (rect.r_height - FRAME_FOCUS_RIGHT_HEIGHT)/2);
	    } else
		event_set_y(event, dp->list_box.r_top);
	    break;
	}
	if (navigation_cmd) {
	    if (row) {
		/* Move Location Cursor to requested row */
		dp->focus_row = row;
		show_focus_win(item_public);
	    }
	    return;
	}
    } else {
	/* -- Mouse button event -- */
	/* Find row */
	y_offset = event_y(event) - dp->list_box.r_top -
	    LIST_BOX_BORDER_WIDTH - ROW_MARGIN;
	if (y_offset < 0)
	    /* event above first row => first row */
	    y_offset = 0;
	if ((unsigned)y_offset >= dp->rows_displayed*dp->row_height)
	    /* event below last row => last row */
	    y_offset = dp->rows_displayed*dp->row_height - 1;
	row_nbr = (unsigned) xv_get(dp->list_sb, SCROLLBAR_VIEW_START) +
	    (unsigned)y_offset / dp->row_height;
	for (row=dp->rows; row; row=row->next)
	    if (row->row == row_nbr)
		break;
	if (event_action(event) != ACTION_MENU) {
	    if (!row) {
		/* Non-menu event not over a row: ignore all but SELECT-down */
		if (event_is_up(event) || event_action(event) != ACTION_SELECT)
		    return;
	    } else if (!row->f.show) {
		/* Non-menu event is over text item in edit mode */
		panel_handle_event(dp->text_item, event);
		return;
	    }
	}
    }

    switch (event_action(event)) {

      /* WARNING: set_current_row calls show_feedback, which calls the
       * notify proc.  The notify proc may delete the event row.  This
       * means that 'row' is invalid after the call to set_current_row.
       */
      case ACTION_SELECT:
	if (event_is_down(event)) {
	    if (row) {
		dp->focus_row = row;
		if (dp->edit_mode)
		    set_edit_row(dp, row, FALSE, event);
		else
		    set_current_row(dp, row, event);
	    }
	    if (event_is_button(event) &&
		wants_key(ip) && !hidden(ip) && !inactive(ip)) {
		if (panel->kbd_focus_item != ip ) {
		    /* Move the keyboard focus to the Scrolling List */
		    if (panel->status.has_input_focus)
			panel_set_kbd_focus(panel, ip);
		    else {
			panel->kbd_focus_item = ip;
			panel->status.focus_item_set = TRUE;
		    }
		} else {
		    /* Move the Location Cursor to the new focus row */
		    show_focus_win(item_public);
		}
	    }
	} else if (dp->edit_mode)
	    /* Possible end of SELECT-drag */
	    dp->last_edit_row = NULL;
	break;

      case ACTION_ADJUST:
	if (dp->edit_mode) {
	    if (event_is_down(event))
		set_edit_row(dp, row, TRUE, event);
	    else
		/* Possible end of ADJUST-drag */
		dp->last_edit_row = NULL;
	} else {
	    /* Display the question mark while ADJUST is down */
	    panel_user_error(ip, event);
	}
	break;

      case ACTION_MENU:
	if (event_is_down(event))
	    handle_menu_event(dp, event);
	break;

      case LOC_DRAG:
	if (action_select_is_down(event)) {
	    if (dp->edit_mode) {
		if (dp->last_edit_row != row) {
		    set_edit_row(dp, row, TRUE, event);
		}
	    } else if (dp->current_row != row)
		set_current_row(dp, row, event);
	} else if (action_adjust_is_down(event) && dp->edit_mode) {
	    if (dp->last_edit_row != row) {
		set_edit_row(dp, row, TRUE, event);
	    }
	}
        dp->focus_row = row;
	show_focus_win(item_public);
	break;
    }
}


static void
panel_list_paint(item_public)
    Panel_item	    item_public;
{
    Panel_list_info *dp = PANEL_LIST_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);

    /* Paint the label */
    panel_paint_image(ip->panel, &ip->label, &ip->label_rect, inactive(ip),
		      ip->color_index);

    /* Paint the value (i.e., the Scrolling List) */
    if (dp->initialized) {
	/* Note: paint_list_box is called first because it calls
	 * paint_list_box_border.  paint_list_box_border paints the
	 * border for both the list box and the title box.  If the
	 * PANEL_LIST is inactive, paint_list_box will gray out the
	 * rows and the border around the rows.  Next, paint_title_box
	 * paints its title and then, if inactive, grays out the title
	 * and its portion of the border.
	 *   Reversing the order of these calls would leave the border
	 * around the title not grayed out.
	 *   Sorry if this is a bit confusing, but titles were added
	 * after the original design.
	 */
	paint_list_box(dp);  /* including border for title */
	if (dp->TITLE)
	    paint_title_box(dp);
    }
}


static void
panel_list_resize(item_public)
    Panel_item	    item_public;
{
    Panel_list_info *dp = PANEL_LIST_PRIVATE(item_public);
    Item_info	   *ip = ITEM_PRIVATE(item_public);
    int		    old_width;

    if (dp->width < 0) {
	/* Clear current panel list */
	panel_default_clear_item(item_public);
	/* Extend-to-edge: recompute list box dimensions */
	old_width = ip->rect.r_width;
	compute_dimensions(ip, dp);
	ip->rect = panel_enclosing_rect(&ip->label_rect, &ip->value_rect);
	if (ip->rect.r_width <= old_width) {
	    /* Width has not grown, so a resize may not occur.  (It depends
	     * on whether the height has increased and what's obscured.)
	     * We need to manually repaint the Scrolling List.
	     */
	    panel_redisplay_item(ip, PANEL_NO_CLEAR);
	}
    }
}


static void
panel_list_remove(item_public)
    Panel_item	    item_public;
{
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Panel_info	   *panel = ip->panel;

    /*
     * Only reassign the keyboard focus to another item if the panel isn't
     * being destroyed.
     */
    if (!panel->status.destroying && panel->kbd_focus_item == ip) {
	hide_focus_win(item_public);
	if (panel->primary_focus_item == ip)
	    panel->primary_focus_item = NULL;
	panel->kbd_focus_item = panel_next_kbd_focus(panel, TRUE);
	panel_accept_kbd_focus(panel);
    }

    return;
}


static void
panel_list_layout(item_public, deltas)
    Panel_item	    item_public;
    Rect           *deltas;
{
    Panel_list_info *dp = PANEL_LIST_PRIVATE(item_public);
    Item_info      *ip = ITEM_PRIVATE(item_public);

    if (!created(ip))
	return;
    dp->title_rect.r_left += deltas->r_left;
    dp->title_rect.r_top += deltas->r_top;
    dp->list_box.r_left += deltas->r_left;
    dp->list_box.r_top += deltas->r_top;
    if (dp->list_sb) {
	dp->sb_rect.r_left += deltas->r_left;
	dp->sb_rect.r_top += deltas->r_top;
	xv_set(dp->list_sb,
	       XV_X, dp->sb_rect.r_left,
	       XV_Y, dp->sb_rect.r_top,
	       NULL);
    }
    xv_set(dp->text_item,
	   XV_X, xv_get(dp->text_item, XV_X) + deltas->r_left,
	   XV_Y, xv_get(dp->text_item, XV_Y) + deltas->r_top,
	   NULL);
}


static void
show_focus_win(item_public)
    Panel_item	    item_public;
{
    Panel_list_info *dp = PANEL_LIST_PRIVATE(item_public);
    Frame	    frame;
    Item_info      *ip = ITEM_PRIVATE(item_public);
    Rect	    rect;
    int		    x;
    int		    y;

    frame = xv_get(PANEL_PUBLIC(ip->panel), WIN_FRAME);
    xv_set(frame, FRAME_FOCUS_DIRECTION, FRAME_FOCUS_RIGHT, NULL);
    x = dp->list_box.r_left;
    if (dp->focus_row) {
	make_row_visible(dp, dp->focus_row->row);
	(void) get_row_rect(dp, dp->focus_row, &rect);
	y = rect.r_top +
	    (rect.r_height - FRAME_FOCUS_RIGHT_HEIGHT)/2;
    } else
	y = dp->list_box.r_top;
    if (!dp->focus_win_shown ||
	x != dp->focus_win_x ||
	y != dp->focus_win_y) {
	dp->focus_win_x = x;
	dp->focus_win_y = y;
	panel_show_focus_win(item_public, frame,
			     dp->focus_win_x, dp->focus_win_y);
	dp->focus_win_shown = TRUE;
    }
}


static void
hide_focus_win(item_public)
    Panel_item	    item_public;
{
    Panel_list_info *dp = PANEL_LIST_PRIVATE(item_public);
    Xv_Window	    focus_win;
    Frame	    frame;
    Item_info      *ip = ITEM_PRIVATE(item_public);

    if (!dp->focus_win_shown)
	return;
    frame = xv_get(PANEL_PUBLIC(ip->panel), WIN_FRAME);
    focus_win = xv_get(frame, FRAME_FOCUS_WIN);
    xv_set(focus_win, XV_SHOW, FALSE, NULL);
    dp->focus_win_shown = FALSE;
}



/* --------------------  Local Routines  -------------------- */

static void
accept_change(text_item, edit_row)
    Panel_item	text_item;
    Row_info	*edit_row;
{
    Panel_list_info *dp;

    dp = (Panel_list_info *) xv_get(text_item, XV_KEY_DATA, PANEL_LIST);
    panel_set_kbd_focus(PANEL_PRIVATE(dp->parent_panel),
	ITEM_FROM_PANEL_LIST(dp));
    xv_set(text_item, XV_SHOW, FALSE, NULL);
    xv_set(dp->list_sb, SCROLLBAR_INACTIVE, FALSE, NULL);
    dp->text_item_row = NULL;   /* no row being edited */
#ifdef OW_I18N
    _xv_set_wcs_attr_dup(&edit_row->string,
			 (wchar_t *) xv_get(text_item, PANEL_VALUE_WCS));
#else
    if (edit_row->f.free_string)
	free(edit_row->string);
    edit_row->string = panel_strsave((char *) xv_get(text_item, PANEL_VALUE));
    edit_row->f.free_string = TRUE;
#endif /* OW_I18N */
    edit_row->f.show = TRUE;
    set_row_display_str_length(dp, edit_row);
    paint_row(dp, edit_row);
}


static void
accept_insert(dp, row)
    Panel_list_info *dp;
    Row_info	    *row;
{
    PANEL_PRIVATE(dp->parent_panel)->kbd_focus_item = NULL;
    xv_set(dp->text_item, XV_SHOW, FALSE, NULL);
    xv_set(dp->list_sb, SCROLLBAR_INACTIVE, FALSE, NULL);
    dp->text_item_row = NULL;   /* no row being edited */
#ifdef OW_I18N
    _xv_set_wcs_attr_dup(&row->string,
			 (wchar_t *) xv_get(dp->text_item, PANEL_VALUE_WCS));
#else
    if (row->f.free_string)
	free(row->string);
    row->string = panel_strsave((char *) xv_get(dp->text_item, PANEL_VALUE));
    row->f.free_string = TRUE;
#endif /* OW_I18N */
    row->f.show = TRUE;
    set_row_display_str_length(dp, row);
    paint_row(dp, row);
}


static Panel_setting
change_done(text_item, event)
    Panel_item	text_item;
    Event	*event;
{
    int		  (*notify_proc)();
    Panel_list_info *dp;
    int		    result;
#ifdef OW_I18N
    void	   *string;
    Item_info	   *panel_list_ip;
#else
    char	   *string;
#endif /* OW_I18N */
    
    /* Validate entry */
    dp = (Panel_list_info *) xv_get(text_item, XV_KEY_DATA, PANEL_LIST);
    string = (CHAR *) xv_get(text_item, PANEL_VALUE_I18N);
    if (!dp->insert_duplicate  /* no duplicates allowed */
	&& STRCMP(dp->text_item_row->STRING, string) /* row has been changed */
	&& check_for_duplicate(dp, string)) /* it matches another row */
	return (PANEL_NONE);   /* change not accepted */
#ifdef OW_I18N
    panel_list_ip = ITEM_PRIVATE(dp->public_self);
    if (wchar_notify(panel_list_ip))
	notify_proc = (int (*)()) xv_get(text_item, XV_KEY_DATA, PANEL_NOTIFY_PROC_WCS);
    else {
        string = (char *) xv_get(text_item, PANEL_VALUE);
	notify_proc = (int (*)()) xv_get(text_item, XV_KEY_DATA, PANEL_NOTIFY_PROC);
    }
#else
    notify_proc = (int (*)()) xv_get(text_item, XV_KEY_DATA, PANEL_NOTIFY_PROC);
#endif
    if (notify_proc) {
	dp->insert_delete_enabled = FALSE;
	result = (*notify_proc) (dp->public_self, string,
				 dp->text_item_row->client_data,
				 PANEL_LIST_OP_VALIDATE, event,
				 dp->text_item_row->row);
	dp->insert_delete_enabled = TRUE;
	if (result == XV_ERROR)
	    return(PANEL_NONE);
    }

    /* Entry validated.  Update edit row. */
    accept_change(text_item, dp->text_item_row);

    dp->edit_op = OP_NONE;
    return(PANEL_NONE);
}


static Xv_opaque
change_proc(menu, menu_item)	/*ARGSUSED*/
    Menu menu;
    Menu_item menu_item;
{
    register Panel_list_info *dp;
    Row_info	*edit_row = NULL;
    int		first_row_in_view;
    int		item_y;
    Xv_font	font;

    dp = (Panel_list_info *) xv_get(menu, XV_KEY_DATA, PANEL_LIST);
    for (edit_row=dp->rows; edit_row; edit_row=edit_row->next) {
    	if (edit_row->f.edit_selected)
	    break;
    }
    if (!edit_row)
    	return(XV_OK);	/* shouldn't occur, but let's be safe */

    edit_row->f.show = FALSE;
    paint_row(dp, edit_row);	/* clear glyph and string */

    /* Overlay text item over blank row */
    first_row_in_view = (int) xv_get(dp->list_sb, SCROLLBAR_VIEW_START);
    xv_set(dp->list_sb, SCROLLBAR_INACTIVE, TRUE, NULL);
    item_y = dp->list_box.r_top + edit_row->string_y -
	first_row_in_view * dp->row_height;
    font = (edit_row->font) ? edit_row->font : dp->font;
    xv_set(dp->text_item,
    	PANEL_ITEM_Y,
	   item_y + ((int)(dp->row_height - panel_fonthome(font)) / 2) - 1,
    	PANEL_NOTIFY_PROC,	change_done,
    	PANEL_VALUE_I18N,	edit_row->STRING,
	PANEL_TEXT_SELECT_LINE,
	XV_SHOW,	TRUE,
	NULL);
    dp->text_item_row = edit_row;

    /* Warp the pointer to the bottom right corner of the text item */
    xv_set(dp->parent_panel,
	   WIN_MOUSE_XY,
	        rect_right(&dp->list_box) - LIST_BOX_BORDER_WIDTH -
		   ROW_MARGIN,
		item_y + dp->row_height - 1,
	    NULL);

    /* Transfer keyboard focus to the text item */
    panel_set_kbd_focus(PANEL_PRIVATE(dp->parent_panel),
	ITEM_PRIVATE(dp->text_item));
    
    dp->edit_op = OP_CHANGE;
    return(XV_OK);
}


static int
check_for_duplicate(dp, string)
    Panel_list_info *dp;
    CHAR	   *string;
{
    Row_info	   *row;

    for (row=dp->rows; row; row=row->next)
	if (row->STRING && STRCMP(row->STRING, string) == 0)
	    return TRUE;  /* found duplicate string */
    return FALSE;   /* no duplicate string */
}


static Xv_opaque
clear_all_choices(menu, menu_item)	/*ARGSUSED*/
    Menu menu;
    Menu_item menu_item;
{
    register Panel_list_info *dp;
    Event	*event = (Event *) xv_get(menu, MENU_LAST_EVENT);
    Row_info	*row;
    
    dp = (Panel_list_info *) xv_get(menu, XV_KEY_DATA, PANEL_LIST);
    dp->insert_delete_enabled = FALSE;
    for (row=dp->rows; row; row=row->next) {
	if (row->f.selected) {
	    row->f.selected = FALSE;
	    show_feedback(dp, row, event);
	}
    }
    dp->insert_delete_enabled = TRUE;
    return(XV_OK);
}


static void
compute_dimensions(ip, dp)
    Item_info	   *ip;
    Panel_list_info *dp;
{
    int		    ext_width;	/* extended width */
#ifdef OW_I18N
    XFontSet	    font_set;
#else
    XFontStruct	   *font_struct;
#endif /* OW_I18N */
    int		    list_box_width_changed;
    int		    max_string_width;
    Row_info       *node;	/* current node in list */
    Rect	   *view_rect;

    if (dp->width < 0) {
	/* Extend list box width to edge of panel */
	view_rect = panel_viewable_rect(ip->panel,
					ip->panel->paint_window->pw);
	ext_width = view_rect->r_left + view_rect->r_width -
	    dp->list_box.r_left - (dp->left_hand_sb ? 0 :
	    dp->sb_rect.r_width);
    } else if (dp->width == 0) {
	/* Fit to rows: Make sure list box is at least as wide as title */
	ext_width = dp->title_rect.r_width;
    } else
	/* An explicit width was specified */
	ext_width = 0;
    list_box_width_changed = fit_list_box_to_rows(dp);
    if (ext_width > dp->list_box.r_width) {
	dp->list_box.r_width = ext_width;
	list_box_width_changed = TRUE;
    }
    dp->title_rect.r_width = dp->list_box.r_width;

    /*
     * Calculate the string y-coordinate and displayed string length
     * of each row.
     */
    for (node=dp->rows; node; node=node->next) {
	node->string_y = LIST_BOX_BORDER_WIDTH + ROW_MARGIN +
	    node->row*dp->row_height;
	if (!node->display_str_len || list_box_width_changed)
	    set_row_display_str_length(dp, node);
    }
    /* Calculate the displayed string length of the title */
    if (dp->TITLE) {
#ifdef OW_I18N
	font_set = ip->panel->bold_fontset_id;
#else
	font_struct = (XFontStruct *) xv_get(ip->panel->bold_font,
					     FONT_INFO);
#endif /* OW_I18N */
	dp->title_display_str_len = STRLEN(dp->TITLE);
	max_string_width = dp->title_rect.r_width -
	    2*LIST_BOX_BORDER_WIDTH - 2*ROW_MARGIN;
#ifdef OW_I18N
	while (XwcTextEscapement(font_set, dp->TITLE, dp->title_display_str_len)
	       > max_string_width)
#else
	while (XTextWidth(font_struct, dp->title, dp->title_display_str_len)
	       > max_string_width)
#endif /* OW_I18N */
	{
	    dp->title_display_str_len--;
	}
    }

    ip->value_rect.r_width = dp->list_box.r_width + dp->sb_rect.r_width;
    ip->value_rect.r_height = dp->title_rect.r_height +
	dp->list_box.r_height;
    if (dp->left_hand_sb)
	dp->sb_rect.r_left = dp->list_box.r_left - dp->sb_rect.r_width;
    else
	dp->sb_rect.r_left = rect_right(&dp->list_box) + 1;
    dp->sb_rect.r_top = dp->list_box.r_top;
    xv_set(dp->list_sb,
	XV_X, dp->sb_rect.r_left,
	XV_Y, dp->sb_rect.r_top,
	NULL);
}


static Xv_opaque
delete_proc(menu, menu_item)	/*ARGSUSED*/
    Menu menu;
    Menu_item menu_item;
{
    register Panel_list_info *dp;
    int		(*notify_proc)();
    int		row_deleted;
    Item_info	*panel_list_ip;
    Row_info	*edit_row;   /* Row being edited with a text field */
    Row_info	*row;
    Event	*event;
#ifdef OW_I18N
    void	*string;
#endif
    
    dp = (Panel_list_info *) xv_get(menu, XV_KEY_DATA, PANEL_LIST);
    panel_list_ip = ITEM_PRIVATE(dp->public_self);
#ifdef OW_I18N
    if (wchar_notify(panel_list_ip))
        notify_proc = panel_list_ip->notify_wc;
    else
#endif
    notify_proc = panel_list_ip->notify;
    if (notify_proc)
	event = (Event *) xv_get(menu, MENU_LAST_EVENT);
    edit_row = dp->text_item_row;
    do {
	row_deleted = FALSE;
	for (row=dp->rows; row; row=row->next)
	    if (row->f.edit_selected && row != edit_row) {
		if (notify_proc) {
		    dp->insert_delete_enabled = FALSE;
#ifdef OW_I18N
		    if (wchar_notify(panel_list_ip))
			string = row->STRING;
		    else
			string = _xv_get_mbs_attr_dup(&row->string);
		    (*notify_proc) (dp->public_self,
				    string, row->client_data,
#else
		    (*notify_proc) (dp->public_self,
				    row->string, row->client_data,
#endif
				    PANEL_LIST_OP_DELETE,
				    event, row->row);
		    dp->insert_delete_enabled = TRUE;
		}
		panel_list_delete_row(dp, row, REPAINT_LIST);
		row_deleted = TRUE;
		break;
	    }
    } while (row_deleted);
    return(XV_OK);
}


static Xv_opaque
enter_edit_mode(menu, menu_item)	/*ARGSUSED*/
    Menu menu;
    Menu_item menu_item;	/* not used */
{
    register Panel_list_info *dp;
    int		    display_length;
    Item_info	   *panel_list_ip;
    Row_info	   *row;
    int		    stored_length;
    int		    text_width, chrwth;
    
    dp = (Panel_list_info *) xv_get(menu, XV_KEY_DATA, PANEL_LIST);
    dp->edit_mode = TRUE;

    /*
     *** Set the attributes of the text field for editing ***
     */
    /* Determine the panel text field's display length, in characters. */
    text_width = dp->list_box.r_width - LIST_BOX_BORDER_WIDTH -
	ROW_MARGIN - dp->string_x;
#ifdef OW_I18N
    /*  Characters have different widths.  
     *  Display_length is actually described
     *  in how many columns, and not how many characters.
     *  Text_width is in pixels. So in order to compare
     *  display_length and stored_length they both have
     *  to be converted to pixels.  I am assuming one
     *  byte still occupies one column, so if the user
     *  specified PANEL_VALUE_STORED_LENGTH, the maximum
     *  storage in pixels would be (n bytes * default column width).
     *  This is a reasonable assumption in EUC world,
     *  but would be completely broken for Unicode.
     */
    {
	int	col_width;     /* column width */
	int	stored_width;  /* maximum pixels for stored_length */
	col_width = xv_get(dp->font, FONT_COLUMN_WIDTH);
	chrwth = xv_get(dp->font, FONT_DEFAULT_CHAR_WIDTH);
	display_length = text_width / col_width;
	if (dp->stored_length_wc == FALSE) {
	    stored_length = (int) xv_get(dp->text_item, PANEL_VALUE_STORED_LENGTH);
	    stored_width = stored_length * col_width;
	}
	else {
	    stored_length = (int) xv_get(dp->text_item, PANEL_VALUE_STORED_LENGTH_WCS);
	    stored_width = stored_length * chrwth;
	}
	/*  Need to limit the display length to be
	 *  less than or equal to the stored length.
	 *  So calculate everything in pixels.
	 */
	if (text_width > stored_width)
	    display_length = stored_width / col_width;
    } 

#else
    chrwth = xv_get(dp->font, FONT_DEFAULT_CHAR_WIDTH);
    display_length = text_width / chrwth;
    stored_length = (int) xv_get(dp->text_item, PANEL_VALUE_STORED_LENGTH);
    if (display_length > stored_length)
	display_length = stored_length;
#endif /* OW_I18N */
    
    /* Modify the text field */
    panel_list_ip = ITEM_PRIVATE(dp->public_self);
    xv_set(dp->text_item,
	PANEL_ITEM_COLOR, panel_list_ip->color_index,
    	PANEL_ITEM_X,	dp->list_box.r_left + dp->string_x,
    	PANEL_VALUE_DISPLAY_LENGTH,	display_length,
    	XV_KEY_DATA, PANEL_NOTIFY_PROC, panel_list_ip->notify,
    	NULL);
    dp->text_item_row = NULL;   /* no row being edited */

    /* Repaint boxes around selected rows */
    for (row=dp->rows; row; row=row->next)
	if (row->f.selected)
	    paint_row(dp, row);

    return(XV_OK);
}


static Xv_opaque
enter_read_mode(menu, menu_item)	/*ARGSUSED*/
    Menu menu;
    Menu_item menu_item;	/* not used */
{
    register Panel_list_info *dp;
    Item_info	*panel_list_ip;
    int		(*notify_proc)();
    Event	*event = (Event *) xv_get(menu, MENU_LAST_EVENT);
    Event	 null_event;
    int		 repaint_visible_rows = FALSE;
    Row_info	*row;
#ifdef OW_I18N
    void	*string;
#else
    CHAR	*string;
#endif
    int		 validated;

    dp = (Panel_list_info *) xv_get(menu, XV_KEY_DATA, PANEL_LIST);

    row = dp->text_item_row;
    if (row) {
	panel_list_ip = ITEM_FROM_PANEL_LIST(dp);
	if (STRLEN((CHAR *)xv_get(dp->text_item, PANEL_VALUE_I18N)) == 0)
	{
	    panel_set_kbd_focus(PANEL_PRIVATE(dp->parent_panel),
		panel_list_ip);
	    xv_set(dp->text_item, XV_SHOW, FALSE, NULL);
	    xv_set(dp->list_sb, SCROLLBAR_INACTIVE, FALSE, NULL);
	    panel_list_delete_row(dp, row, REPAINT_LIST);
	} else {
#ifdef OW_I18N
	    if (wchar_notify(panel_list_ip))
		notify_proc = panel_list_ip->notify_wc;
	    else
#else
	    notify_proc = panel_list_ip->notify;
#endif
	    string = (CHAR *) xv_get(dp->text_item, PANEL_VALUE_I18N);
	    if (!dp->insert_duplicate && check_for_duplicate(dp, string))
		validated = FALSE;
	    else {
		if (!event) {
		    event_init(&null_event);
		    event = &null_event;
		}
		dp->insert_delete_enabled = FALSE;
#ifdef OW_I18N
		if (! wchar_notify(panel_list_ip))
		    string = (char *) xv_get(dp->text_item, PANEL_VALUE);
#endif
	    	validated = (!notify_proc ||
			     (*notify_proc) (dp->public_self,
					     string, row->row,
					     PANEL_LIST_OP_VALIDATE,
					     event, row->row)
			     == XV_OK);
		dp->insert_delete_enabled = TRUE;
	    }
	    if (validated) {
		/* Text was validated */
		if (dp->edit_op == OP_CHANGE)
		    accept_change(dp->text_item, row);
		else {
		    accept_insert(dp, row);
		    panel_set_kbd_focus(PANEL_PRIVATE(dp->parent_panel),
			panel_list_ip);
		}
	    } else {
		/* Text was not validated */
		panel_set_kbd_focus(PANEL_PRIVATE(dp->parent_panel),
		    panel_list_ip);
		xv_set(dp->text_item, XV_SHOW, FALSE, NULL);
		xv_set(dp->list_sb, SCROLLBAR_INACTIVE, FALSE, NULL);
		if (!row->STRING || STRLEN(row->STRING) == 0)
		{
		    panel_list_delete_row(dp, row, DO_NOT_REPAINT_LIST);
		    repaint_visible_rows = TRUE;
		} else {
		    row->f.show = TRUE;
		    paint_row(dp, row);
		}
	    }
	}
    }
    dp->edit_op = OP_NONE;
    dp->text_item_row = NULL;   /* no row being edited */
    dp->edit_mode = FALSE;

    if (!dp->current_row && !dp->choose_none && dp->rows) {
        /* Last selected row was deleted: If no row is selected,
	 * then select the first row.
	 */
        for (row = dp->rows; row; row = row->next)
            if (row->f.selected && !row->f.row_inactive)
            	break;
        if (!row) {
	    for (row = dp->rows; row; row = row->next)
		if ( !row->f.row_inactive ) {
		    dp->current_row = row;
		    row->f.selected = TRUE;
		    break;
		}
        }
    }


    /* Remove highlighting and repaint boxes around selected rows */
    for (row=dp->rows; row; row=row->next) {
    	if (repaint_visible_rows || row->f.edit_selected || row->f.selected) {
    	    row->f.edit_selected = FALSE;
	    paint_row(dp, row);
    	}
    }

    return(XV_OK);
}


/*
 * Find/Create the row
 */
static Row_info *
find_or_create_nth_row(dp, which_row, create)
     Panel_list_info	*dp;
     int             	which_row;
     int             	create;
{
    Row_info       *prev = NULL;
    Row_info       *node = dp->rows;


    while (node && (node->row != which_row)) {
	prev = node;
	node = node->next;
    }

    if (!node && create)
	node = create_next_row(dp, prev);

    return (node);
}


/*
 * Like find_or_create_nth_row(), for cases where the current
 * row handle is known.  This saves traversing non-contiguous
 * memory when it isn't necessary.
 */
static Row_info *
gimme_the_next_row( dp, prev )
     Panel_list_info *dp;
     Row_info *prev;
{
    if ( prev->next )
	return prev->next;
    return create_next_row(dp, prev);
}


/*
 * Create the row
 */
static Row_info *
create_next_row(dp, prev)
     Panel_list_info	*dp;
     Row_info		*prev;
{
    Row_info *node;

    node = xv_alloc(Row_info);
    if (prev) {
	node->row = prev->row + 1;
	prev->next = node;
    } else {
	dp->rows = node;
	node->row = 0;
	if (!dp->focus_row)
	    dp->focus_row = node;
    }
    node->next = NULL;
    node->prev = prev;
    /*
     * It's possible that prev->row + 1 is not equal to which_row ie. the
     * row is created at the end of the list
     */
#ifdef OW_I18N
    /*
     * xv_alloc calls calloc, therefor all fields are zeroed already.
     */
#else
    node->string = NULL;
    node->f.free_string = FALSE;
#endif /* OW_I18N */
    node->glyph = 0;
    node->f.show = TRUE;
    node->f.row_inactive = FALSE;
    dp->nrows++;
    return (node);
}




/*
 * fit the scrolling list
 */
static int	/* returns TRUE if dp->list_box.r_width changed */
fit_list_box_to_rows(dp)
    Panel_list_info *dp;
{
    int		    max_str_size;
    int		    new_width;
    Row_info       *row;
    struct pr_size  str_size;	/* width & height of row string */
    int		    width_changed;

    if (dp->width <= 0) {
	max_str_size = 0;
	for (row=dp->rows; row; row=row->next) {
	    if (row->STRING) {
		str_size = XV_PF_TEXTWIDTH(STRLEN(row->STRING), dp->font,
		    row->STRING);
		max_str_size = MAX(max_str_size, str_size.x);
	    }
	}
	new_width = dp->string_x + max_str_size + ROW_MARGIN +
	    LIST_BOX_BORDER_WIDTH;
    } else
	new_width = dp->width;
    width_changed = new_width != dp->list_box.r_width;
    dp->list_box.r_width = new_width;
    return (width_changed);
}


/* get_row_rect:  Get the rect for the specified row
 * 	returns: TRUE= row in view; FALSE= row not in view
 */
static int
get_row_rect(dp, row, rect)
    Panel_list_info *dp;
    Row_info	   *row;
    Rect	   *rect;	/* Output parameter */
{
    int		    first_row_in_view;   /* row number */
    int		    view_start;  /* in pixels */

    /* Insure that the row is completely visible within the list box.  */
    first_row_in_view = (int) xv_get(dp->list_sb, SCROLLBAR_VIEW_START);
    if (row->row < first_row_in_view ||
	row->row >= (unsigned)first_row_in_view + dp->rows_displayed)
	return FALSE;   /* row not within list box view */

    /* Get row rect. */
    view_start = dp->row_height * first_row_in_view;
    rect->r_top = row->string_y - view_start;
    rect->r_top += dp->list_box.r_top;
    rect->r_left = dp->list_box.r_left + LIST_BOX_BORDER_WIDTH +
	ROW_MARGIN;
    rect->r_width = dp->list_box.r_width - 2*LIST_BOX_BORDER_WIDTH -
	2*ROW_MARGIN;
    rect->r_height = dp->row_height;
    if (rect_bottom(rect) > rect_bottom(&dp->list_box))
	rect->r_height = dp->list_box.r_height - rect->r_top;
    return TRUE;   /* row within list box view */
}


static void
handle_menu_event(dp, event)
    Panel_list_info	*dp;
    Event		*event;
{
    int		    edit_cnt;
    Menu	    menu;
    Panel_item	    change_item;
    Panel_item	    delete_item;
    Panel_item	    insert_item;
    Item_info	   *ip = ITEM_FROM_PANEL_LIST(dp);
    Row_info	    *edit_row;
    Row_info	    *row;

    if (dp->edit_mode) {
	menu = dp->edit_menu;
	if (menu) {
	    /* Note: The client can change the edit menu */
	    change_item = xv_find(menu, MENUITEM,
		    XV_AUTO_CREATE, FALSE,
		    MENU_STRING, XV_MSG("Change"),
		    NULL);
	    delete_item = xv_find(menu, MENUITEM,
		    XV_AUTO_CREATE, FALSE,
		    MENU_STRING, XV_MSG("Delete"),
		    NULL);
	    insert_item = xv_find(menu, MENUITEM,
		    XV_AUTO_CREATE, FALSE,
		    MENU_STRING, XV_MSG("Insert"),
		    NULL);
	    edit_row = dp->text_item_row;
	    if (change_item || delete_item) {
		edit_cnt = 0;
		for (row=dp->rows; row; row=row->next)
		    if (row->f.edit_selected && row != edit_row)
			edit_cnt++;
		if (change_item)
		    xv_set(change_item,
			MENU_INACTIVE, edit_row || edit_cnt != 1,
			NULL);
		if (delete_item)
		    xv_set(delete_item,
			MENU_INACTIVE, edit_cnt == 0,
			NULL);
	    }
	    if (insert_item)
		xv_set(insert_item,
		    MENU_INACTIVE, edit_row ? TRUE : FALSE,
		    NULL);
	}
    } else
	menu = dp->read_menu;
    if (menu) {
	xv_set ( menu, 
		MENU_COLOR,	ip->color_index, 
		XV_KEY_DATA,	PANEL_FIRST_ITEM, ip,
		XV_KEY_DATA,	MENU_DONE_PROC, xv_get(menu, MENU_DONE_PROC),
		MENU_DONE_PROC,	list_menu_done_proc,
		NULL );
	ip->panel->status.current_item_active = TRUE;
	menu_show(menu, event_window(event), event, NULL);
    }
}


static Panel_setting
insert_done(text_item, event)
    Panel_item	text_item;
    Event	*event;
{
    Panel_list_info *dp;
    Row_info	   *insert_row;
    int		    item_y;
    int		    first_row_in_view;
    int		  (*notify_proc)();
    int		    result;
    Xv_font	    font;
#ifdef OW_I18N
    void	   *string;
    Item_info	   *panel_list_ip;
#else
    CHAR	   *string;
#endif /* OW_I18N */
    
    dp = (Panel_list_info *) xv_get(text_item, XV_KEY_DATA, PANEL_LIST);
    insert_row = dp->text_item_row;

    /* If value is empty, then exit insert mode. */
    if (STRLEN((CHAR *) xv_get(text_item, PANEL_VALUE_I18N)) == 0)
    {
	panel_set_kbd_focus(PANEL_PRIVATE(dp->parent_panel),
	    ITEM_FROM_PANEL_LIST(dp));
	xv_set(text_item, XV_SHOW, FALSE, NULL);
	xv_set(dp->list_sb, SCROLLBAR_INACTIVE, FALSE, NULL);
	dp->text_item_row = NULL;   /* no row being edited */
	panel_list_delete_row(dp, insert_row, REPAINT_LIST);
	dp->edit_op = OP_NONE;
	return(PANEL_NONE);
    }

    /* Validate entry */
    string = (CHAR *) xv_get(text_item, PANEL_VALUE_I18N);
    if (!dp->insert_duplicate && check_for_duplicate(dp, string))
	return (PANEL_NONE);
#ifdef OW_I18N
    panel_list_ip = ITEM_PRIVATE(dp->public_self);
    if (wchar_notify(panel_list_ip))
	notify_proc = (int (*)()) 
		xv_get(text_item, XV_KEY_DATA, PANEL_NOTIFY_PROC_WCS);
    else
#endif
    notify_proc = (int (*)()) xv_get(text_item, XV_KEY_DATA, PANEL_NOTIFY_PROC);
    if (notify_proc) {
	dp->insert_delete_enabled = FALSE;
#ifdef OW_I18N
	if (! wchar_notify(panel_list_ip))
	    string = (char *) xv_get(text_item, PANEL_VALUE);
#endif
	result = (*notify_proc) (dp->public_self, string,
	    (Xv_opaque) insert_row->row, /* pass row # as client data */
	    PANEL_LIST_OP_VALIDATE, event, insert_row->row);
	dp->insert_delete_enabled = TRUE;
	if (result == XV_ERROR)
	    return(PANEL_NONE);
    }

    /* Entry validated.  Insert into Scrolling List */
    accept_insert(dp, insert_row);

    /* Create a new row before the next row.  If no next row, then append. */
    insert_row = panel_list_insert_row(dp,
	insert_row->next ? insert_row->next->row : -1, HIDE_ROW, REPAINT_LIST);

    /* Continue insert mode. insert_row now points to empty
     * row just inserted.
     *
     * If new row is out of the view window, then scroll new row
     * to top of view window.
     */
    dp->focus_row = insert_row;
    make_row_visible(dp, insert_row->row);

    /* Overlay text item over blank new row */
    first_row_in_view = (int) xv_get(dp->list_sb, SCROLLBAR_VIEW_START);
    xv_set(dp->list_sb, SCROLLBAR_INACTIVE, TRUE, NULL);
    item_y = dp->list_box.r_top + insert_row->string_y -
	first_row_in_view * dp->row_height;
    font = (insert_row->font) ? insert_row->font : dp->font;
    xv_set(dp->text_item,
    	   PANEL_ITEM_Y,
	   	item_y + ((int)(dp->row_height - panel_fonthome(font)) / 2) - 1,
    	   PANEL_NOTIFY_PROC,	insert_done,
	   PANEL_VALUE_I18N,	NULL_STRING,
	   XV_SHOW,	TRUE,
	   NULL);
    dp->text_item_row = insert_row;
    
    /* Warp the pointer to the bottom right corner of the text item */
    xv_set(dp->parent_panel,
	   WIN_MOUSE_XY,
	        rect_right(&dp->list_box) - LIST_BOX_BORDER_WIDTH -
		   ROW_MARGIN,
		item_y + dp->row_height - 1,
	    NULL);

    /* Transfer keyboard focus to the text item */
    panel_set_kbd_focus(PANEL_PRIVATE(dp->parent_panel),
	ITEM_PRIVATE(dp->text_item));
    
    return(PANEL_NONE);
}


static Xv_opaque
insert_proc(menu, menu_item)
    Menu menu;
    Menu_item menu_item;
{
    register Panel_list_info *dp;
    int			first_row_in_view;
    int			item_y;
    int			which_row;
    Insert_pos_t	insert_position;
    Row_info		*insert_row = NULL;
    Xv_font		font;
    
    dp = (Panel_list_info *) xv_get(menu, XV_KEY_DATA, PANEL_LIST);
    insert_position = (Insert_pos_t) xv_get(menu_item,
	XV_KEY_DATA, PANEL_INSERT);

    /* Find the first row selected for editing */
    for (insert_row=dp->rows; insert_row;
	 insert_row=insert_row->next) {
	if (insert_row->f.edit_selected)
	    break;
    }
    if (!insert_row)
	insert_row = dp->rows;

    /* Determine where to insert new row */
    if (insert_position == INSERT_AFTER) {
	if (insert_row)
	    insert_row = insert_row->next;
	if (insert_row)
	    which_row = insert_row->row;
	else
	    which_row = -1;   /* append to end of list */
    } else {
	if (insert_row)
	    which_row = insert_row->row;
	else
	    which_row = 0;   /* insert at beginning of list */
    }

    /* Create insert row */
    insert_row = panel_list_insert_row(dp, which_row, HIDE_ROW,
				       REPAINT_LIST);

    /* If new row is out of the view window, then scroll new row
     * to the top of view window.
     */
    dp->focus_row = insert_row;
    make_row_visible(dp, insert_row->row);
    
    /* Overlay text item over blank new row */
    first_row_in_view = (int) xv_get(dp->list_sb, SCROLLBAR_VIEW_START);
    xv_set(dp->list_sb, SCROLLBAR_INACTIVE, TRUE, NULL);
    item_y = dp->list_box.r_top + insert_row->string_y -
	first_row_in_view * dp->row_height;
    font = (insert_row->font) ? insert_row->font : dp->font;
    xv_set(dp->text_item,
    	PANEL_ITEM_Y,
	   item_y + ((int)(dp->row_height - panel_fonthome(font)) / 2) - 1,
    	PANEL_NOTIFY_PROC,	insert_done,
	PANEL_VALUE_I18N,	NULL_STRING,
	XV_SHOW,	TRUE,
	NULL);
    dp->text_item_row = insert_row;

    /* Warp the pointer to the bottom right corner of the text item */
    xv_set(dp->parent_panel,
	   WIN_MOUSE_XY,
	        rect_right(&dp->list_box) - LIST_BOX_BORDER_WIDTH -
		   ROW_MARGIN,
		item_y + dp->row_height - 1,
	    NULL);

    /* Transfer keyboard focus to the text item */
    panel_set_kbd_focus(PANEL_PRIVATE(dp->parent_panel),
	ITEM_PRIVATE(dp->text_item));
    
    dp->edit_op = OP_INSERT;
    return(XV_OK);
}


static Xv_opaque
locate_next_choice(menu, menu_item)	/*ARGSUSED*/
    Menu menu;
    Menu_item menu_item;
{
    register Panel_list_info *dp;
    Row_info *first_row;
    Row_info *row;
    int view_end;
    int view_start;  /* in pixels */
    
    dp = (Panel_list_info *) xv_get(menu, XV_KEY_DATA, PANEL_LIST);

    if (!dp->rows)
	return(XV_OK);  /* no rows */

    /* Determine the range of the view */
    view_start = dp->row_height *
	(int) xv_get(dp->list_sb, SCROLLBAR_VIEW_START);
    view_end = view_start +
	dp->rows_displayed * dp->row_height - 1;

    /* Find the first row beyond the view (if any) */
    for (first_row=dp->rows; first_row; first_row=first_row->next) {
	if (first_row->string_y >= view_end)
	    break;
    }
    if (!first_row)
	first_row = dp->rows;

    /* Find the next selected row (if any) */
    row = first_row;
    do {
	if (row->f.selected)
	    break;
	row = row->next;
	if (!row)
	    row = dp->rows;
    } while (row!=first_row);

    /* Scroll the display to this row */
    dp->focus_row = row;
    make_row_visible(dp, row->row);
    return(XV_OK);
}


static void
make_row_visible(dp, desired_row_nbr)
    Panel_list_info *dp;
    int desired_row_nbr;
{
    /* If desired row is out of the view window, then scroll desired row
     * to the top of the view window.
     */
    if (!row_visible(dp, desired_row_nbr))  {
	desired_row_nbr = MIN((unsigned)desired_row_nbr, dp->nrows - dp->rows_displayed);
	xv_set(dp->list_sb, SCROLLBAR_VIEW_START, desired_row_nbr, NULL);
    }
}


static Row_info *
next_row(dp, row, n)
    Panel_list_info *dp;
    Row_info       *row;
    int             n;
{
    Row_info       *node = NULL;

    if (!row) {
	dp->rows = row = xv_alloc(Row_info);
	dp->nrows = 1;
	row->prev = NULL;
	if (!dp->focus_row)
	    dp->focus_row = row;
    } else {
	if (n == 0) {		/* If it is the first row, return the row
				 * itself */
	    return (row);
	} else if (row->next)	/* Already created */
	    return (row->next);
	else {
	    node = xv_alloc(Row_info);
	    node->prev = row;
	    row->next = node;
	    row = node;
	    dp->nrows++;
	}
    }
    row->next = NULL;
    row->f.selected = FALSE;
    row->f.show = TRUE;
    row->row = n;
#ifdef OW_I18N
    /*
     * xv_alloc uses calloc therefor all fields are zeroed alreday.
     */
#else
    row->string = NULL;
    row->f.free_string = FALSE;
#endif /* OW_I18N */
    row->glyph = NULL;
    return (row);
}


static void
paint_list_box(dp)
    Panel_list_info *dp;
{
    Xv_Drawable_info *info;
    Item_info	   *ip = ITEM_FROM_PANEL_LIST(dp);
    Row_info	   *row;
    Xv_Window	    pw;
    Xv_Screen      screen;
    GC             *gc_list;

    /* Paint list box border.
     *   Dashed = Scrolling List does not have keyboard focus
     *   Solid = Scrolling List has keyboard focus
     */
    paint_list_box_border(dp);

    /* Paint (visible) rows */
    for (row=dp->rows; row; row=row->next)
	paint_row(dp, row);

    if (ip->panel->status.has_input_focus && ip->panel->kbd_focus_item == ip) {
	if (!dp->focus_row || row_visible(dp, dp->focus_row->row))
	    /* update position of Location Cursor */
	    show_focus_win(ITEM_PUBLIC(ip));
	else
	    hide_focus_win(ITEM_PUBLIC(ip));
    }
    
    if (inactive(ip)) {
	PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	    DRAWABLE_INFO_MACRO(pw, info);
	    screen = xv_screen(info);
  	    gc_list = (GC *)xv_get(screen,  SCREEN_OLGC_LIST, pw);
	    screen_adjust_gc_color(pw, SCREEN_INACTIVE_GC);
	    XFillRectangle(xv_display(info), xv_xid(info),
			   gc_list[SCREEN_INACTIVE_GC],
			   dp->list_box.r_left, dp->list_box.r_top,
			   dp->list_box.r_width, dp->list_box.r_height);
	PANEL_END_EACH_PAINT_WINDOW
    }
}


static void
paint_list_box_border(dp)   /* paints border around list_box + title_rect */
    Panel_list_info *dp;
{
    Display	   *display;
    GC		    gc;
    int		    gc_mask;
    XGCValues	    gc_values;
    Xv_Drawable_info *info;
    Item_info	   *ip = ITEM_FROM_PANEL_LIST(dp);
    GC             *gc_list;
    Xv_window	    pw;
    Rect	    rect;
    Xv_Screen       screen;
    Drawable	    xid;

    /* Paint list box border */
    rect = dp->list_box;
    if (dp->TITLE) {
	rect.r_top = dp->title_rect.r_top;
	rect.r_height += dp->title_rect.r_height;
    }
    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
        screen = xv_screen(info);
        gc_list = (GC *)xv_get(screen, SCREEN_OLGC_LIST, pw);
	display = xv_display(info);
	xid = xv_xid(info);
	if (ip->panel->status.three_d) {

	    /*
	     * Note:  chiseled separator line requires 2
	     * OLGX calls -- one invoked and one normal.
	     */
	    olgx_draw_box(ip->panel->ginfo, xid,
			  rect.r_left, rect.r_top,
			  rect.r_width, rect.r_height,
			  OLGX_INVOKED, FALSE);
	    olgx_draw_box(ip->panel->ginfo, xid,
			  rect.r_left+1, rect.r_top+1,
			  rect.r_width-2, rect.r_height-2,
			  OLGX_NORMAL, FALSE);
	} else {
	    if (ip->color_index >= 0) {
		gc = gc_list[SCREEN_NONSTD_GC];
		XSetForeground(display, gc_list[SCREEN_NONSTD_GC],
		    xv_get(xv_cms(info), CMS_PIXEL, ip->color_index));
		gc_values.line_style = LineSolid;
		gc_mask = GCLineStyle;
		XChangeGC(display, gc, gc_mask, &gc_values);
	    } else {
		gc = gc_list[SCREEN_SET_GC];
	    }
	    XDrawRectangle(display, xid, gc,
			   rect.r_left, rect.r_top,
			   rect.r_width - 1, rect.r_height - 1);
	}
    PANEL_END_EACH_PAINT_WINDOW
}


static void
paint_row(dp, row)
    Panel_list_info *dp;
    Row_info *row;
{
    Display	   *display;
    int		    fg_pixval;	/* foreground pixel value */
    Xv_Font	    font;
    int		    gc_mask;
    XGCValues	    gc_values;
    Xv_Drawable_info *info;
    Item_info	   *ip = ITEM_FROM_PANEL_LIST(dp);
    GC		    gc;
    Xv_window	    pw;
    Rect	    row_rect;
    int		    save_black;
    Rect	    string_rect;
    Item_info	   *text_item_private;
    Drawable	    xid;
    Xv_Screen      screen;
    GC             *gc_list;
#ifdef OW_I18N
    XFontSet	    font_set;
#endif /* OW_I18N */

    if (!get_row_rect(dp, row, &row_rect))
	return;
    if (row->STRING)
    {
	string_rect.r_left = dp->list_box.r_left + dp->string_x;
	string_rect.r_top = row_rect.r_top;
	string_rect.r_width = row_rect.r_width - dp->string_x +
	    LIST_BOX_BORDER_WIDTH + ROW_MARGIN;
	string_rect.r_height = row_rect.r_height;
    } else
	string_rect = row_rect;

    /* Clear row */
    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
	display = xv_display(info);
	xid = xv_xid(info);
	XClearArea(display, xid, row_rect.r_left, row_rect.r_top,
		   row_rect.r_width, row_rect.r_height, False);
	if (!row->f.show && row == dp->text_item_row) {
	    text_item_private = ITEM_PRIVATE(dp->text_item);
	    (*text_item_private->ops.panel_op_paint) (dp->text_item);
	}
    PANEL_END_EACH_PAINT_WINDOW

    if (!row->f.show)
	return;

    PANEL_EACH_PAINT_WINDOW(ip->panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
	screen = xv_screen(info);
        gc_list = (GC *)xv_get(screen, SCREEN_OLGC_LIST, pw);
	display = xv_display(info);
	xid = xv_xid(info);
	if (ip->color_index >= 0)
	    fg_pixval = xv_get(xv_cms(info), CMS_PIXEL, ip->color_index);

	/* If 3D, read mode, and row is selected, then draw a recessed box */
	if (ip->panel->status.three_d && !dp->edit_mode && row->f.selected) {
	    if (ip->color_index >= 0) {
		save_black = olgx_get_single_color(ip->panel->ginfo,
						   OLGX_BLACK);
		olgx_set_single_color(ip->panel->ginfo, OLGX_BLACK, fg_pixval,
				      OLGX_SPECIAL);
	    }
	    olgx_draw_box(ip->panel->ginfo, xid,
		row_rect.r_left, row_rect.r_top,
		row_rect.r_width, row_rect.r_height,
		OLGX_INVOKED, TRUE);
	    if (ip->color_index >= 0)
		olgx_set_single_color(ip->panel->ginfo, OLGX_BLACK, save_black,
				      OLGX_SPECIAL);
	}

	/* Paint text */
	if (row->STRING)
	{
	    /* I have no idea if this is the proper solution, but it fixes */
	    /* the bug that causes PANEL_LISTS to be drawn with the wrong */
	    /* foreground-color sometimes. */
	    /* martin-2.buck@student.uni-ulm.de */
#if 1
	    XSetForeground(display, gc_list[SCREEN_TEXT_GC],
			   (ip->color_index >= 0) ? fg_pixval : xv_fg(info));
#else
	    if (ip->color_index >= 0) {
		XSetForeground(display, gc_list[SCREEN_TEXT_GC],
		    fg_pixval);
	    }
#endif
	    if (row->font) {
		font = row->font;
#ifndef OW_I18N
		XSetFont(display, gc_list[SCREEN_TEXT_GC],
			 xv_get(font, XV_XID));
#endif /* OW_I18N */
	    } else
		font = dp->font;
#ifdef OW_I18N
	    font_set = (XFontSet)xv_get(font, FONT_SET_ID); 
	    XwcDrawString(display, xid, font_set, gc_list[SCREEN_TEXT_GC],
			  string_rect.r_left,
			  string_rect.r_top 
			  	+ ((string_rect.r_height + panel_fonthome(font)) / 2) 
			  	- 1,
			  row->STRING, 
			  row->display_str_len
			  );
#else
	    XDrawString(display, xid, gc_list[SCREEN_TEXT_GC],
			string_rect.r_left,
			string_rect.r_top 
				+ ((string_rect.r_height + panel_fonthome(font)) / 2) 
			  	- 1,
			row->string, 
			row->display_str_len
			);
	    if (row->font) {
		/* Set the text gc back to the standard font */
		XSetFont(display, gc_list[SCREEN_TEXT_GC],
			 ip->panel->std_font_xid);
	    }
#endif /* OW_I18N */
	    if (ip->color_index >= 0) {
		XSetForeground(display, gc_list[SCREEN_TEXT_GC],
			       xv_fg(info));
	    }
	}

	if (dp->edit_mode && row->f.edit_selected) {
	    /* Invert text */
	    panel_pw_invert(pw, &row_rect, ip->color_index);
	}

	/* Paint glyph */
	if (row->glyph)
	    /*
	     * center glyph both vertically and horizontally.  since we can't
	     * break compatibility by increasing the size of the row, move the
	     * glyph over by 1/2 the COL_GAP so there is at least enough room
	     * to show hilighting around it.  the OL File Choosing Spec wants
	     * 10 pixels at the default font...
	     */
	    panel_paint_svrim(pw, row->glyph, 
			      row_rect.r_left + (PANEL_LIST_COL_GAP / 2), 
			      row_rect.r_top + ((row_rect.r_height - row->glyph->pr_height) / 2),
			      ip->color_index, 
			      (dp->edit_mode && row->f.edit_selected) ? (Pixrect *)NULL : row->mask_glyph
			      );

	if (row->f.selected &&
	    (!ip->panel->status.three_d || dp->edit_mode)) {
	    /* Box text */
	    if (ip->color_index < 0) {
		if (!dp->edit_mode)
		    gc = gc_list[SCREEN_SET_GC];
		else
		    gc = gc_list[SCREEN_DIM_GC];
	    } else {
		gc = gc_list[SCREEN_NONSTD_GC];
		XSetForeground(display, gc, fg_pixval);
		if (!dp->edit_mode) {
		    gc_values.line_style = LineSolid;
		    gc_mask = GCLineStyle;
		} else {
		    gc_values.line_style = LineDoubleDash;
		    gc_values.dashes = 1;
		    gc_mask = GCLineStyle | GCDashList;
		}
		XChangeGC(display, gc, gc_mask, &gc_values);
	    }
	    XDrawRectangle(display, xid, gc,
		       row_rect.r_left, row_rect.r_top,
		       row_rect.r_width - 1, row_rect.r_height - 1);
	}

    if ( row->f.row_inactive ) {
	screen_adjust_gc_color(pw, SCREEN_INACTIVE_GC);
	XFillRectangle(xv_display(info), xv_xid(info),
		       gc_list[SCREEN_INACTIVE_GC],
		       row_rect.r_left, row_rect.r_top,
		       row_rect.r_width, row_rect.r_height);
    }

    PANEL_END_EACH_PAINT_WINDOW
}


static void
paint_title_box(dp)
    Panel_list_info *dp;
{
    Xv_Drawable_info *info;
    Item_info	   *ip;
    GC		   *gc_list;
    Panel_info	   *panel;
    Xv_Window	    pw;
    Xv_Screen	    screen;
    CHAR	   *title_str;
    int		    x;
    int		    y;

    ip = ITEM_FROM_PANEL_LIST(dp);
    panel = ip->panel;
    x = dp->title_rect.r_left + LIST_BOX_BORDER_WIDTH + ROW_MARGIN;
    y = dp->title_rect.r_top + LIST_BOX_BORDER_WIDTH + LEVEL_DOT_HEIGHT;
#ifdef OW_I18N
    title_str = (wchar_t *) panel_strsave_wc((wchar_t *)dp->TITLE);
#else
    title_str = (char *) xv_malloc(dp->title_display_str_len + 1);
    strncpy(title_str, (char *) dp->title, dp->title_display_str_len);
#endif /* OW_I18N */
    title_str[dp->title_display_str_len] = 0;
    PANEL_EACH_PAINT_WINDOW(panel, pw)
	DRAWABLE_INFO_MACRO(pw, info);
#ifdef OW_I18N
	panel_paint_text(pw, panel->bold_fontset_id, 
		ip->color_index, x, y + 
		panel_fonthome(panel->bold_font), title_str);
#else
	panel_paint_text(pw, panel->bold_font_xid, ip->color_index,
	    x, y + panel_fonthome(panel->bold_font), title_str);
#endif /* OW_I18N */
	screen = xv_screen(info);
        gc_list = (GC *)xv_get(screen, SCREEN_OLGC_LIST, pw);
        screen_adjust_gc_color(pw, SCREEN_SET_GC); 
	XDrawLine(xv_display(info), xv_xid(info),
	    gc_list[SCREEN_SET_GC],
	    x,
	    rect_bottom(&dp->title_rect),
	    rect_right(&dp->title_rect) - LIST_BOX_BORDER_WIDTH - ROW_MARGIN,
	    rect_bottom(&dp->title_rect));
	if (inactive(ip)) {
	    screen_adjust_gc_color(pw, SCREEN_INACTIVE_GC);
	    XFillRectangle(xv_display(info), xv_xid(info),
			   gc_list[SCREEN_INACTIVE_GC],
			   dp->title_rect.r_left, dp->title_rect.r_top,
			   dp->title_rect.r_width, dp->title_rect.r_height);
	}
    PANEL_END_EACH_PAINT_WINDOW
    xv_free(title_str);
}


static void
panel_list_create_displayarea(dp)
    Panel_list_info *dp;
{
    Item_info      *ip = ITEM_PRIVATE(dp->public_self);
    int             number_rows;
    
    if (dp->rows_displayed == 0) {
	if (dp->nrows < PANEL_LIST_DEFAULT_ROW)
	    dp->rows_displayed = dp->nrows;
	else
	    dp->rows_displayed = PANEL_LIST_DEFAULT_ROW;
    }
    number_rows = dp->rows_displayed;

    /*
     * By this time all the rows have been created. Calculate the size of the
     * list box (depends on the number of rows to be displayed and the size of
     * the fonts).
     */
    dp->list_box.r_height = 2*LIST_BOX_BORDER_WIDTH +
	2*ROW_MARGIN + number_rows*dp->row_height;

    /* Create the Scrolling List Scrollbar
     */
    dp->list_sb = xv_create(ip->panel->paint_window->pw, SCROLLBAR,
    	WIN_INHERIT_COLORS, TRUE, /* inherit both the visual and CMS */
	XV_HEIGHT, dp->list_box.r_height,
	XV_KEY_DATA, PANEL_LIST, ip,
	SCROLLBAR_NOTIFY_CLIENT, ip->panel->paint_window->pw,
	SCROLLBAR_VIEW_LENGTH,	dp->rows_displayed,  /* in rows */
	SCROLLBAR_OBJECT_LENGTH, dp->nrows,  /* in rows */
	SCROLLBAR_PIXELS_PER_UNIT, dp->row_height,
	SCROLLBAR_INACTIVE, inactive(ip),
	XV_SHOW, !hidden(ip),
	XV_KEY_DATA, FRAME_ORPHAN_WINDOW, TRUE,
	NULL);
    if (ip->color_index >= 0)
	xv_set(dp->list_sb,
	       WIN_FOREGROUND_COLOR, ip->color_index,
	       NULL);
    dp->sb_rect = *(Rect *) xv_get(dp->list_sb, XV_RECT);
    if (dp->left_hand_sb)
	dp->list_box.r_left += dp->sb_rect.r_width;

    /*
     * Don't allow the Scrolling List Scrollbar to accept keyboard input focus.
     */
    win_set_no_focus(dp->list_sb, TRUE);
}


static void
panel_list_delete_row(dp, node, repaint)
    Panel_list_info *dp;
    Row_info        *node;
    int		     repaint;
{
    Row_info       *prev = node->prev;

    if (prev) {
	prev->next = node->next;
    } else {
	dp->rows = node->next;
    }

    if (node->next) {
	node->next->prev = prev;
    }

    if (dp->focus_row == node) {
	dp->focus_row = node->next;
	if (!dp->focus_row)
	    dp->focus_row = prev;
    }

    if (node == dp->current_row)
	dp->current_row = NULL;

    /* Adjust the row numbers */
    prev = node;
    node = node->next;
#ifdef OW_I18N
    _xv_free_ps_string_attr_dup(&prev->string);
#else
    if (prev->f.free_string)
	xv_free(prev->string);
#endif /* OW_I18N */
    xv_free(prev);
    while (node) {
	node->row--;
	node->string_y -= dp->row_height;
	node = node->next;
    }
    dp->nrows--;
    if (dp->list_sb)
	xv_set(dp->list_sb,
	       SCROLLBAR_OBJECT_LENGTH, dp->nrows,  /* in rows */
	       NULL);

    if (repaint) {
	/* Erase old rows */
	panel_clear_rect(PANEL_PRIVATE(dp->parent_panel), dp->list_box);
	/* Repaint list box and currently visible rows */
	paint_list_box(dp);
    }
}


static Row_info *
panel_list_insert_row(dp, which_row, show, repaint)
    Panel_list_info *dp;
    int             which_row;
    int		    show;
    int		    repaint;
{
    Row_info       *node = dp->rows;
    Row_info       *prev = NULL;
    Row_info       *row = xv_alloc(Row_info);

    while (node && (node->row != which_row)) {
	prev = node;
	node = node->next;
    }
    row->f.selected = FALSE;
    row->f.show = show;
    row->next = node;
    row->prev = prev;
    row->glyph = NULL;
#ifdef OW_I18N
    /*
     * xv_alloc uses callc, therefor you do not need to zeroing again.
     */
#else
    row->string = NULL;
    row->f.free_string = FALSE;
#endif /* OW_I18N */

    /*
     * It's possible that prev->row + 1 is not equal to which_row ie. the row
     * is created at the end of the list
     */
    if (prev) {
	prev->next = row;
	row->row = prev->row + 1;
    } else {			/* Insert at the begining of the list */
	dp->rows = row;
	row->row = 0;
	if (!dp->focus_row)
	    dp->focus_row = row;
    }
    row->string_y = LIST_BOX_BORDER_WIDTH + ROW_MARGIN +
	row->row*dp->row_height;
    if (node) {
	node->prev = row;
    }
    while (node) {
	node->row++;
	node->string_y += dp->row_height;
	node = node->next;
    }
    dp->nrows++;
    if (dp->list_sb)
	xv_set(dp->list_sb,
	       SCROLLBAR_OBJECT_LENGTH, dp->nrows,  /* in rows */
	       NULL);

    if (repaint)
	paint_list_box(dp);

    return(row);
}


static int
row_visible(dp, desired_row_nbr)   /* returns TRUE or FALSE */
    Panel_list_info *dp;
    int desired_row_nbr;
{
    int			first_row_nbr_in_view;

    first_row_nbr_in_view = (int) xv_get(dp->list_sb, SCROLLBAR_VIEW_START);
    return (desired_row_nbr >= first_row_nbr_in_view &&
	(unsigned)desired_row_nbr < (unsigned)first_row_nbr_in_view + dp->rows_displayed);
}


static void
set_current_row(dp, event_row, event)
    Panel_list_info	*dp;
    Row_info		*event_row;
    Event		*event;	/* NULL => not event generated */
{
    int		    new_state = TRUE;
    int		    toggle = FALSE;
    
    if ( event_row->f.row_inactive )
	return;

    if (dp->choose_one) {
    	if (dp->current_row == event_row) {
    	    if (dp->choose_none)
    	    	toggle = TRUE;
    	} else if (dp->current_row) {
	    dp->setting_current_row = TRUE;
	    dp->current_row->f.selected = FALSE;
    	}
	if (toggle)
	    new_state = event_row->f.selected ? FALSE : TRUE;
	event_row->f.selected = new_state;
	if (dp->setting_current_row) {
	    /* Note: The notify proc is called with the DESELECT operation
	     * after the new row has been selected.  This lets the notify
	     * proc differentiate between toggling off a row or choosing a
	     * new row in a Choggle (choose_one && choose_none).
	     */
	    dp->insert_delete_enabled = FALSE;
    	    show_feedback(dp, dp->current_row, event);
	    dp->insert_delete_enabled = TRUE;
	}
	dp->current_row = event_row;
	show_feedback(dp, dp->current_row, event);
	dp->setting_current_row = FALSE;
    } else {
    	new_state = event_row->f.selected ? FALSE : TRUE;
    	event_row->f.selected = new_state;
	dp->current_row = event_row;
    	show_feedback(dp, dp->current_row, event);
    }
}


static void
set_edit_row(dp, event_row, toggle, event)
    Panel_list_info	*dp;
    Row_info		*event_row;	/* the row the event occurred in */
    int			toggle;	/* TRUE or FALSE */
    Event		*event;
{
    Row_info	*row;
    int		new_state = TRUE;
    
    /* NOTE: This routine depends on no rows being inserted or deleted while in
     * show_feedback.  Since show_feedback doesn't call the notify proc, this
     * is not an issue.
     */
    if (!toggle) {
	event_row->f.edit_selected = TRUE;
	show_feedback(dp, event_row, event);
    	for (row=dp->rows; row; row=row->next) {
    	    if (row != event_row && row->f.edit_selected) {
    	    	row->f.edit_selected = FALSE;
    	        show_feedback(dp, row, event);
    	    }
    	}
    } else {
    	new_state = event_row->f.edit_selected ? FALSE : TRUE;
    	event_row->f.edit_selected = new_state;
    	show_feedback(dp, event_row, event);
    }
    dp->last_edit_row = event_row;
}


static void
set_row_display_str_length(dp, row)
    Panel_list_info *dp;
    Row_info       *row;
{
#ifdef OW_I18N
    XFontSet	    font_set;
#else
    XFontStruct	   *font_struct;
#endif /* OW_I18N */
    int		    max_string_width;

#ifdef OW_I18N
    if (row->font)
	font_set = (XFontSet) xv_get(row->font, FONT_SET_ID);
    else
	font_set = dp->font_set;
    row->display_str_len = row->STRING ? wslen(row->STRING) : 0;
    max_string_width = dp->list_box.r_width - LIST_BOX_BORDER_WIDTH -
	ROW_MARGIN - dp->string_x;
    while (XwcTextEscapement(font_set, row->STRING, row->display_str_len) >
	   max_string_width) {
#else
    if (row->font)
	font_struct = (XFontStruct *) xv_get(row->font, FONT_INFO);
    else
	font_struct = dp->font_struct;
    row->display_str_len = row->string ? strlen(row->string) : 0;
    max_string_width = dp->list_box.r_width - LIST_BOX_BORDER_WIDTH -
	ROW_MARGIN - dp->string_x;
    while (XTextWidth(font_struct, row->string, row->display_str_len) >
	   max_string_width) {
#endif /* OW_I18N */
	row->display_str_len--;
    }
}


static void
set_row_font(dp, row, font)
    Panel_list_info *dp;
    Row_info	   *row;
    Xv_Font	    font;
{
    Xv_Font	    old_font = row->font;

    if (!font)
	row->font = font;
    else if ((unsigned)xv_get(font, FONT_DEFAULT_CHAR_HEIGHT) <= dp->row_height)
	row->font = font;
    else {
	xv_error(font,
		 ERROR_STRING,
		   XV_MSG("Font height exceeds row height; font ignored"),
		 ERROR_PKG, PANEL,
		 NULL);
	row->font = XV_ZERO;
    }
    if (row->font != old_font)
	row->display_str_len = 0;   /* force recomputation */
}


static void
set_row_glyph(dp, row, glyph_pr)
    Panel_list_info *dp;
    Row_info *row;
    Pixrect *glyph_pr;
{
    if ((unsigned)(glyph_pr->pr_height) <= dp->row_height) {
	row->glyph = glyph_pr;
    } else {
	xv_error((Xv_opaque)glyph_pr,
		 ERROR_STRING,
		   XV_MSG("Panel List glyph height exceeds row height; glyph ignored"),
		 ERROR_PKG, PANEL,
		 NULL);
	row->glyph = NULL;
    }
}



static void
set_row_mask_glyph(dp, row, glyph_pr)
    Panel_list_info *dp;
    Row_info *row;
    Pixrect *glyph_pr;
{
    if ( !glyph_pr ) {
	row->mask_glyph = (Pixrect *)NULL;
	return;
    }

    if ( (int)(glyph_pr->pr_depth) != 1 ) {
	xv_error((Xv_opaque)glyph_pr,
		 ERROR_STRING,
		 	XV_MSG("Panel List mask glyph depth not equal 1; mask glyph ignored"),
		 ERROR_PKG, PANEL,
		 NULL);
	row->mask_glyph = NULL;
	return;
    }

    if ((unsigned)(glyph_pr->pr_height) <= dp->row_height) {
	row->mask_glyph = glyph_pr;
    } else {
	xv_error((Xv_opaque)glyph_pr,
		 ERROR_STRING,
		   XV_MSG("Panel List mask glyph height exceeds row height; glyph ignored"),
		 ERROR_PKG, PANEL,
		 NULL);
	row->mask_glyph = NULL;
    }
}


static void
show_feedback(dp, row, event)
    Panel_list_info *dp;
    Row_info *row;
    Event *event;	/* NULL => feedback was not event generated */
{
    Item_info *panel_list_ip = ITEM_PRIVATE(dp->public_self);
    int (*notify_proc)() = panel_list_ip->notify;
    Panel_list_op op;
    int dbl_click = FALSE;

    if (!panel_list_ip->panel->status.painted)
	return;

#ifdef OW_I18N
    if ( wchar_notify(panel_list_ip) )
	notify_proc = panel_list_ip->notify_wc;
#endif


    /*
     * if double-click, we must convert a de-select into a select/dbl-click op.
     */
    if ( notify_proc && event
	&& row->f.selected
	&& dp->do_dbl_click
	&& is_dbl_click(dp, row, event)
	) {
	dbl_click = TRUE;
    } 


    if (!hidden(panel_list_ip))
	paint_row(dp, row);


    if (!dp->edit_mode) {
	if (notify_proc && event) {
	    if ( dbl_click )
		op = PANEL_LIST_OP_DBL_CLICK;
	    else
		op = row->f.selected ? PANEL_LIST_OP_SELECT : PANEL_LIST_OP_DESELECT;

#ifdef OW_I18N
	    if (wchar_notify(panel_list_ip)) {
		(*notify_proc) (dp->public_self, row->STRING, row->client_data,
				op, event, row->row);
	    }
	    else {
		(*notify_proc) (dp->public_self,
				_xv_get_mbs_attr_dup(&row->string),
				row->client_data,
				op, event, row->row);
	    }
#else
	    (*notify_proc) (dp->public_self, row->string, row->client_data,
		op, event, row->row);
#endif /* OW_I18N */
	}
    }
}


static void
list_menu_done_proc(menu, result)
     Menu            menu;
     Xv_opaque       result;
{
    Item_info	*ip;
    void	(*orig_done_proc) ();	/* original menu-done procedure */
    
    ip = (Item_info *) xv_get(menu, XV_KEY_DATA, PANEL_FIRST_ITEM);

    /* Restore original menu done proc. */
    orig_done_proc = (void (*) ()) xv_get(menu, XV_KEY_DATA, MENU_DONE_PROC);
    xv_set ( menu,
	    MENU_DONE_PROC, orig_done_proc,
	    NULL );

    /* Invoke original menu done proc (if any) */
    if (orig_done_proc)
	(orig_done_proc) (menu, result);

    ip->panel->status.current_item_active = FALSE;
}



static int
is_dbl_click(dp, row, event)
    Panel_list_info *dp;
    Row_info *row;
    Event *event;
{
    Item_info *ip = ITEM_FROM_PANEL_LIST(dp);
    int is_multiclick;
    static struct timeval empty_time = {0, 0};


    /* make sure they don't try to pull one over on us */
    if ( (event_action(event) != ACTION_SELECT) || !event_is_down(event) )
	return FALSE;


    /* make sure both click's occured on the same row */
    if ( !dp->last_click_row || (dp->last_click_row != row) ) {
	dp->last_click_row = row;
	dp->last_click_row->click_time = event_time(event);
	return FALSE;
    }
    

    /* weigh timeval's against multiclick-timeout resource */
    is_multiclick = panel_is_multiclick(ip->panel, 
					&dp->last_click_row->click_time,
					&event_time(event)
					);

    if ( is_multiclick )
	dp->last_click_row->click_time = empty_time; 	/* reset timeval */
    else
	dp->last_click_row->click_time = event_time(event);

    return is_multiclick;

}
