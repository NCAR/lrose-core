#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_popup.c 1.54 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Text subwindow menu creation and support.
 */

#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <xview_private/ev_impl.h>
#include <xview_private/txt_18impl.h>
#include <sys/time.h>
#include <signal.h>
#include <xview/notice.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/openmenu.h>
#include <xview/wmgr.h>
#include <xview/pixwin.h>
#include <xview/win_struct.h>
#include <xview/win_screen.h>
#include <xview/file_chsr.h>

#ifdef SVR4
#include <unistd.h>
#endif /* SVR4 */
 
#define   	MAX_STR_LENGTH		1024
/* This is for select line number */
#define		MAX_SEL_LINE_PANEL_ITEMS  2
/* This is for load, store and include file */
#define		MAX_FILE_PANEL_ITEMS	  3
/* This is for find and replace */
#define		MAX_SEARCH_PANEL_ITEMS	  10
/* This is for find marked text */
#define		MAX_MATCH_PANEL_ITEMS	  6

/* for select line number */
typedef enum {
    SEL_LINE_ITEM = 0,
    SEL_LINE_NUMBER_ITEM = 1,
}               Sel_line_panel_item_enum;

/* for load, store and include file */
typedef enum {
    FILE_CMD_ITEM = 0,
    DIR_STRING_ITEM = 1,
    FILE_STRING_ITEM = 2,
}               File_panel_item_enum;

/* for find and replace */
typedef enum {
    FIND_ITEM = 0,
    FIND_STRING_ITEM = 1,
    FIND_DONE_ITEM = 2,
    REPLACE_ITEM = 3,
    REPLACE_STRING_ITEM = 4,
    WRAP_ITEM = 5,
    FIND_THEN_REPLACE_ITEM = 6,
    REPLACE_THEN_FIND_ITEM = 7,
    REPLACE_ALL_ITEM = 8,
    SEARCH_BLINK_PARENT_ITEM = 9
} Search_panel_item_enum;

/* This is for find marked text */
typedef enum {
    CHOICE_ITEM = 0,
    FIND_PAIR_ITEM = 1,
    FIND_PAIR_CHOICE_ITEM = 2,
    INSERT_ITEM = 3,
    REMOVE_ITEM = 4
} Match_panel_item_enum;

Pkg_private int      STORE_FILE_POPUP_KEY;
Pkg_private int      SAVE_FILE_POPUP_KEY;
Pkg_private int      LOAD_FILE_POPUP_KEY;
Pkg_private int      FILE_STUFF_POPUP_KEY;
Pkg_private int      SEARCH_POPUP_KEY;
Pkg_private int      MATCH_POPUP_KEY;
Pkg_private int      SEL_LINE_POPUP_KEY;
Pkg_private int      TEXTSW_CURRENT_POPUP_KEY;
Pkg_private int      FC_EXTEN_ITEM_KEY;
Pkg_private int      FC_PARENT_KEY;

static int      TEXTSW_POPUP_KEY;  /* key data holding one of the above
				      key names */

Panel_item      store_panel_items[MAX_FILE_PANEL_ITEMS];
Panel_item      load_panel_items[MAX_FILE_PANEL_ITEMS];
Panel_item      include_panel_items[MAX_FILE_PANEL_ITEMS];
Panel_item      search_panel_items[MAX_SEARCH_PANEL_ITEMS];
Panel_item      match_panel_items[MAX_MATCH_PANEL_ITEMS];
Panel_item      sel_line_panel_items[MAX_SEL_LINE_PANEL_ITEMS];
Pkg_private     open_cmd_proc();
Pkg_private     include_cmd_proc();
Pkg_private     save_cmd_proc();

static void
  add_exten_item( File_chooser fc );

static void
  show_dot_files_proc( Panel_choice_item item, int value, Event *event );

static int
  fc_exten_func( File_chooser fc, Rect *frame_rect, Rect *exten_rect,
                 int left_edge, int right_edge, int max_height );

static Notify_value
textsw_popup_destroy_func(client, status)
     Notify_client client;
     Destroy_status status;
{
    Frame popup_frame = client;
    Frame textsw_frame;
    Textsw textsw;
    int popup_key_name;

    popup_key_name = xv_get(popup_frame, XV_KEY_DATA, TEXTSW_POPUP_KEY);
    textsw = xv_get(popup_frame,XV_KEY_DATA, TEXTSW_CURRENT_POPUP_KEY);
    
    if (textsw) {
	textsw_frame = xv_get(textsw,WIN_FRAME);
	xv_set(textsw_frame,XV_KEY_DATA, popup_key_name, 0, NULL);
    }
    return notify_next_destroy_func(client,status);
}

Pkg_private void
textsw_create_popup_frame(view, popup_type)
    Textsw_view_handle view;
    int             popup_type;

{
    Frame           frame_parent = xv_get(VIEW_REP_TO_ABS(view), WIN_FRAME);
    Frame           popup_frame, base_frame;
    Panel           panel = XV_ZERO;
    char           *label;
#ifdef OW_I18N
    int		    win_use_im = ((popup_type != TEXTSW_MENU_SEL_MARK_TEXT) &&
				  (popup_type != TEXTSW_MENU_NORMALIZE_LINE));
#endif

    base_frame = (xv_get(frame_parent, XV_IS_SUBTYPE_OF, FRAME_BASE) ?
		  frame_parent : xv_get(frame_parent, WIN_OWNER));

    if (!TEXTSW_POPUP_KEY) {
	TEXTSW_POPUP_KEY = xv_unique_key();
    }
    switch (popup_type) {
      case TEXTSW_MENU_STORE:
        popup_frame = (Frame) xv_create(base_frame, FILE_CHOOSER,
#ifdef OW_I18N
                                    WIN_USE_IM, win_use_im,
#endif
                                    FILE_CHOOSER_TYPE,FILE_CHOOSER_SAVEAS,
#ifdef OW_I18N
                                    FILE_CHOOSER_NOTIFY_FUNC_WCS, save_cmd_proc,
#else
                                    FILE_CHOOSER_NOTIFY_FUNC, save_cmd_proc,
#endif
                                    FRAME_SHOW_LABEL, TRUE,
                                    WIN_CLIENT_DATA, view,
                                    NULL);
	xv_set(frame_parent, XV_KEY_DATA, STORE_FILE_POPUP_KEY, popup_frame, 
	       NULL);
        add_exten_item(popup_frame); /* add the dotfile choice item */
	xv_set(popup_frame, 
	       XV_KEY_DATA, TEXTSW_POPUP_KEY, STORE_FILE_POPUP_KEY,
	       NULL);
	label = XV_MSG("Text:Save As");
	break;
      case TEXTSW_MENU_LOAD:
        popup_frame = (Frame) xv_create(base_frame, FILE_CHOOSER,
#ifdef OW_I18N
                                    WIN_USE_IM, win_use_im,
#endif
                                    FILE_CHOOSER_TYPE,FILE_CHOOSER_OPEN,
#ifdef OW_I18N
                                    FILE_CHOOSER_NOTIFY_FUNC_WCS, open_cmd_proc,
#else
                                    FILE_CHOOSER_NOTIFY_FUNC, open_cmd_proc,
#endif
                                    FRAME_SHOW_LABEL, TRUE,
                                    WIN_CLIENT_DATA, view,
                                    NULL);
	xv_set(frame_parent, XV_KEY_DATA, LOAD_FILE_POPUP_KEY, popup_frame,
	       NULL);
        add_exten_item(popup_frame); /* add the dotfile choice item */
	xv_set(popup_frame, 
	       XV_KEY_DATA, TEXTSW_POPUP_KEY, LOAD_FILE_POPUP_KEY,
	       NULL);
	label = XV_MSG("Text:Open");
	break;
      case TEXTSW_MENU_SAVE:
        popup_frame = (Frame) xv_create(base_frame, FILE_CHOOSER,
#ifdef OW_I18N
                                    WIN_USE_IM, win_use_im,
#endif
                                    FILE_CHOOSER_TYPE,FILE_CHOOSER_SAVE,
#ifdef OW_I18N
                                    FILE_CHOOSER_NOTIFY_FUNC_WCS, save_cmd_proc,
#else
                                    FILE_CHOOSER_NOTIFY_FUNC, save_cmd_proc,
#endif
                                    FRAME_SHOW_LABEL, TRUE,
                                    WIN_CLIENT_DATA, view,
                                    NULL);
	xv_set(frame_parent, XV_KEY_DATA, SAVE_FILE_POPUP_KEY, popup_frame,
	       NULL);
        add_exten_item(popup_frame); /* add the dotfile choice item */
	xv_set(popup_frame, 
	       XV_KEY_DATA, TEXTSW_POPUP_KEY, SAVE_FILE_POPUP_KEY,
	       NULL);

	label = XV_MSG("Text:Save");
	break;

      case TEXTSW_MENU_FILE_STUFF:
        popup_frame = (Frame) xv_create(base_frame, FILE_CHOOSER_OPEN_DIALOG,
#ifdef OW_I18N
                                    WIN_USE_IM, win_use_im,
#endif
                                    FRAME_SHOW_LABEL, TRUE,
                                    FILE_CHOOSER_CUSTOMIZE_OPEN, XV_MSG("Include"), 
                                                 XV_MSG("Click Select to Include"),
                                                  FILE_CHOOSER_SELECT_FILES,
#ifdef OW_I18N
                                    FILE_CHOOSER_NOTIFY_FUNC_WCS, include_cmd_proc,
#else
                                    FILE_CHOOSER_NOTIFY_FUNC, include_cmd_proc,
#endif
                                    WIN_CLIENT_DATA, view,
                                    NULL);

	xv_set(frame_parent, XV_KEY_DATA, FILE_STUFF_POPUP_KEY, popup_frame,
	       NULL);
        add_exten_item(popup_frame); /* add the dotfile choice item */
	xv_set(popup_frame, 
	       XV_KEY_DATA, TEXTSW_POPUP_KEY, FILE_STUFF_POPUP_KEY,
	       NULL);
	label = XV_MSG("Text:Include");
	break;

      case TEXTSW_MENU_FIND_AND_REPLACE:
        popup_frame = (Frame) xv_create(base_frame, FRAME_CMD,
#ifdef OW_I18N
                                    WIN_USE_IM, win_use_im,
#endif
                                    FRAME_SHOW_LABEL, TRUE,
                                    WIN_CLIENT_DATA, view,
                                    WIN_FRONT,
                                    NULL);
	xv_set(frame_parent, XV_KEY_DATA, SEARCH_POPUP_KEY, popup_frame,
	       NULL);
	xv_set(popup_frame, 
	       XV_KEY_DATA, TEXTSW_POPUP_KEY, SEARCH_POPUP_KEY,
	       NULL);
	panel = (Panel) textsw_create_search_panel(popup_frame, view);
	label = XV_MSG("Text:Find and Replace");
	break;

      case TEXTSW_MENU_SEL_MARK_TEXT:
        popup_frame = (Frame) xv_create(base_frame, FRAME_CMD,
#ifdef OW_I18N
                                    WIN_USE_IM, win_use_im,
#endif
                                    FRAME_SHOW_LABEL, TRUE,
                                    WIN_CLIENT_DATA, view,
                                    WIN_FRONT,
                                    NULL);
	xv_set(frame_parent, XV_KEY_DATA, MATCH_POPUP_KEY, popup_frame,
	       NULL);
	xv_set(popup_frame, 
	       XV_KEY_DATA, TEXTSW_POPUP_KEY, MATCH_POPUP_KEY,
	       NULL);
	panel = (Panel) textsw_create_match_panel(popup_frame, view);
	label = XV_MSG("Text:Find Marked Text");
	break;

      case TEXTSW_MENU_NORMALIZE_LINE:
        popup_frame = (Frame) xv_create(base_frame, FRAME_CMD,
#ifdef OW_I18N
                                    WIN_USE_IM, win_use_im,
#endif
                                    FRAME_SHOW_LABEL, TRUE,
                                    WIN_CLIENT_DATA, view,
                                    WIN_FRONT,
                                    NULL);
	xv_set(frame_parent, XV_KEY_DATA, SEL_LINE_POPUP_KEY, popup_frame,
	       NULL);
	xv_set(popup_frame, 
	       XV_KEY_DATA, TEXTSW_POPUP_KEY, SEL_LINE_POPUP_KEY,
	       NULL);
	panel = (Panel) textsw_create_sel_line_panel(popup_frame, view);
	label = XV_MSG("Text:Line Number");
	break;
    }



    if (panel) {
           window_fit(panel);
           window_fit(popup_frame);
    }
    (void) xv_set(popup_frame, 
                  FRAME_LABEL, label,
		  XV_KEY_DATA, TEXTSW_CURRENT_POPUP_KEY, 
		  FOLIO_REP_TO_ABS(FOLIO_FOR_VIEW(view)),
		  XV_SHOW, TRUE, NULL);
    notify_interpose_destroy_func(popup_frame,textsw_popup_destroy_func);
}

Pkg_private
textsw_get_and_set_selection(popup_frame, view, popup_type)
    Frame           popup_frame;
    Textsw_view_handle view;
    int             popup_type;
{
    Es_index        dummy;
    CHAR            show_str[MAX_STR_LENGTH];

#ifdef OW_I18N
#define PANEL_SET_VALUE		panel_set_value_wcs
#else
#define PANEL_SET_VALUE		panel_set_value
#endif

    show_str[0] = XV_ZERO;
    xv_set(popup_frame, 
	   XV_KEY_DATA, TEXTSW_CURRENT_POPUP_KEY, 
	   FOLIO_REP_TO_ABS(FOLIO_FOR_VIEW(view)),
	   NULL);
    (void) textsw_get_selection(view, &dummy, &dummy, show_str, MAX_STR_LENGTH);

    switch (popup_type) {
      case TEXTSW_MENU_STORE:
      case TEXTSW_MENU_LOAD:
      case TEXTSW_MENU_FILE_STUFF:
           xv_set(popup_frame, FILE_CHOOSER_UPDATE,NULL);
	break;
      case TEXTSW_MENU_FIND_AND_REPLACE:
	(void) PANEL_SET_VALUE(search_panel_items[(int) FIND_STRING_ITEM],
			       show_str);
	break;
      case TEXTSW_MENU_NORMALIZE_LINE:
	(void) PANEL_SET_VALUE(sel_line_panel_items[(int) SEL_LINE_NUMBER_ITEM],
			       show_str);
	break;

    }

    (void) xv_set(popup_frame, XV_SHOW, TRUE,
		  WIN_CLIENT_DATA, view,
		  NULL);

#undef PANEL_SET_VALUE
}

Pkg_private
textsw_set_dir_str(popup_type)
    int             popup_type;
{
    char            curr_dir[MAX_STR_LENGTH];

/*
    (void) getcwd(curr_dir, MAX_STR_LENGTH);
    switch (popup_type) {
      case TEXTSW_MENU_STORE:
	(void) panel_set_value(store_panel_items[(int) DIR_STRING_ITEM],
			       curr_dir);
	break;
      case TEXTSW_MENU_LOAD:
	(void) panel_set_value(load_panel_items[(int) DIR_STRING_ITEM],
			       curr_dir);
	break;
      case TEXTSW_MENU_FILE_STUFF:
	(void) panel_set_value(include_panel_items[(int) DIR_STRING_ITEM],
			       curr_dir);
	break;
    }
*/
}

Pkg_private     Textsw_view_handle
text_view_frm_p_itm(panel_item)
    Panel_item      panel_item;
{
    Panel           panel = panel_get(panel_item, XV_OWNER, NULL);
    Xv_Window       search_frame = xv_get(panel, WIN_FRAME);
    Textsw_view_handle view = (Textsw_view_handle) window_get(search_frame, WIN_CLIENT_DATA, NULL);

    return (view);
}
Pkg_private     Xv_Window
frame_from_panel_item(panel_item)
    Panel_item      panel_item;
{
    Panel           panel = panel_get(panel_item, XV_OWNER, NULL);
    Xv_Window       popup_frame = xv_get(panel, WIN_FRAME);

    return (popup_frame);
}
Pkg_private int
textsw_get_selection(view, first, last_plus_one, selected_str, max_str_len)
    Textsw_view_handle view;
    Es_index       *first, *last_plus_one;
    CHAR           *selected_str;
    int             max_str_len;
{
    /*
     * Return true iff primary selection is in the textsw of current view. If
     * there is a selection in any of the textsw and selected_str is not
     * null, then it will copy it to selected_str.
     */
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    Textsw_selection_object selection;
    CHAR            selection_buf[MAX_STR_LENGTH];
    unsigned        options = EV_SEL_PRIMARY;

    textsw_init_selection_object(
	    folio, &selection, selection_buf, SIZEOF(selection_buf), FALSE);

    selection.type = textsw_func_selection_internal(
			       folio, &selection, EV_SEL_BASE_TYPE(options),
						    TFS_FILL_ALWAYS);

    textsw_clear_secondary_selection(folio, selection.type);

    if ((selection.type & TFS_IS_SELF) &&
	(selection.type & EV_SEL_PRIMARY)) {
	/* If this window owns the primary selection, do nothing. */
    } else {
	selection.first = selection.last_plus_one = ES_CANNOT_SET;
    }


    if ((selection.type & EV_SEL_PRIMARY) &&
	(selection.buf_len > 0) && (selected_str != NULL)) {

	if (selection.buf_len >= max_str_len)
	    selection.buf_len = max_str_len - 1;

	STRNCPY(selected_str, selection.buf, selection.buf_len);
	selected_str[selection.buf_len] = XV_ZERO;
    }
    *first = selection.first;
    *last_plus_one = selection.last_plus_one;

    return ((*first != ES_CANNOT_SET) && (*last_plus_one != ES_CANNOT_SET));
}

Pkg_private void
textsw_set_pop_up_location(frame_parent, pop_up_frame)
    Frame           frame_parent, pop_up_frame;
{
#define MY_OFFSET		4

    Rect            base_rect, pop_up_rect, screen_rect;
    short           new_x, new_y;
    int             pop_up_fd = (int) window_get(pop_up_frame, WIN_FD);
    int             max_cover_area;

    screen_rect = *((Rect *) window_get(frame_parent, WIN_SCREEN_RECT));
    base_rect = *((Rect *) window_get(frame_parent, WIN_RECT));

    win_getrect(pop_up_fd, &pop_up_rect);

    new_x = pop_up_rect.r_left;
    new_y = pop_up_rect.r_top;
    max_cover_area = base_rect.r_width / 3;

    if ((base_rect.r_top - (pop_up_rect.r_height + MY_OFFSET)) >= 0)
	new_y = base_rect.r_top - (pop_up_rect.r_height + MY_OFFSET);
    else if ((base_rect.r_left - pop_up_rect.r_width + MY_OFFSET) >= 0)
	new_x = base_rect.r_left - (pop_up_rect.r_width + MY_OFFSET);
    else if ((base_rect.r_left + base_rect.r_width + pop_up_rect.r_width + MY_OFFSET) <=
	     screen_rect.r_width)
	new_x = base_rect.r_left + base_rect.r_width;
    else if ((pop_up_rect.r_width + MY_OFFSET - base_rect.r_left) <= max_cover_area)
	new_x = 0;
    else if ((base_rect.r_left + base_rect.r_width - max_cover_area) <= (screen_rect.r_width - (pop_up_rect.r_width + MY_OFFSET)))
	new_x = screen_rect.r_width - (pop_up_rect.r_width + MY_OFFSET);
    if (new_y < 0)
	new_y = 0;

    pop_up_rect.r_left = new_x;
    pop_up_rect.r_top = new_y;


    win_setrect(pop_up_fd, &pop_up_rect);
#undef MY_OFFSET
}

static void
  add_exten_item( File_chooser fc )
{
    Panel panel;
    int item_width;
    int item_height;
    int frame_width;
    int frame_height;
    Panel_item item;
    
    panel = xv_get(fc, FRAME_CMD_PANEL);
    
    item = xv_create(panel, PANEL_CHOICE,
                      PANEL_LABEL_STRING,       XV_MSG("Hidden Files:"),
                      PANEL_CHOICE_STRINGS,     XV_MSG("Hide"), XV_MSG("Show"), NULL,
                      PANEL_NOTIFY_PROC,        show_dot_files_proc,
                      XV_KEY_DATA,              FC_PARENT_KEY, fc,
		      XV_HELP_DATA,		"textsw:hiddenfileschoice",
                      NULL);
    
    item_width = (int) xv_get(item, XV_WIDTH);
    item_height = (int) xv_get(item, XV_HEIGHT);
 
    
    /*
     * Adjust Frame default size to make room for
     * the extension item.
     */  
    frame_width = (int) xv_get(fc, XV_WIDTH);
    frame_height = (int) xv_get(fc, XV_HEIGHT);
    xv_set(fc,
           XV_WIDTH,  MAX(frame_width, (item_width + xv_cols(panel, 4))),
           XV_HEIGHT, frame_height + item_height,
           NULL);


    /*
     * Adjust Frame Min Size.  provide for at least 2
     * columns on either side of the extension item.
     */  
    xv_get(fc, FRAME_MIN_SIZE, &frame_width, &frame_height);
    xv_set(fc,
           FRAME_MIN_SIZE,
                MAX( frame_width, (item_width + xv_cols(panel, 4))),
                frame_height + item_height,
           NULL);


    /* Tell File Chooser to reserve layout space for it */
    xv_set(fc,
           FILE_CHOOSER_EXTEN_HEIGHT,   item_height,
           FILE_CHOOSER_EXTEN_FUNC,     fc_exten_func,
           XV_KEY_DATA,                 FC_EXTEN_ITEM_KEY, item,
           NULL);
}

static void
  show_dot_files_proc( Panel_choice_item item, int value, Event *event )
{
    File_chooser fc = xv_get(item, XV_KEY_DATA, FC_PARENT_KEY);

    xv_set(fc, FILE_CHOOSER_SHOW_DOT_FILES, value, NULL);
}

/*
 * FILE_CHOOSER_EXTEN_FUNC, layout extension items within the
 * given extension rect.
 */
static int
  fc_exten_func( File_chooser fc, Rect *frame_rect, Rect *exten_rect,
                 int left_edge, int right_edge, int max_height )
{
    Panel_item item = (Panel_item) xv_get(fc, XV_KEY_DATA, FC_EXTEN_ITEM_KEY);
    int item_width;

    item_width = (int) xv_get(item, XV_WIDTH);

    /*
     * show item centered in frame.
     */  
    xv_set(item,
           XV_X,        (frame_rect->r_width - item_width) / 2,
           XV_Y,        exten_rect->r_top,
           PANEL_PAINT, PANEL_NONE,
           NULL);

    return -1;
}

