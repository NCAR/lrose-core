#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_menu.c 20.91 93/06/29";
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
#include <xview/textsw.h>
#include <xview_private/txt_impl.h>
#include <xview_private/ev_impl.h>
#include <errno.h>
#include <pixrect/pr_util.h>
#include <xview/file_chsr.h>

#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

#include <pixrect/pixfont.h>
#include <xview/defaults.h>
#include <xview/win_input.h>
#include <xview/win_struct.h>
#include <xview/fullscreen.h>
#include <xview/notice.h>
#include <xview/frame.h>
#include <xview/openmenu.h>
#include <xview/window.h>
#include <xview/cursor.h>
#include <xview/screen.h>
#include <xview/server.h>
#include <xview/svrimage.h>

#define HELP_INFO(s) XV_HELP_DATA, s,

Pkg_private Es_index ev_index_for_line();
static void textsw_done_menu();

#define			MAX_SIZE_MENU_STRING	30

typedef enum {
    TEXTSWMENU,
    TERMSWMENU
}               Menu_type;

typedef struct local_menu_object {
    int             refcount;	/* refcount for textsw_menu */
    Menu            menu;	/* Let default to NULL */
    Menu           *sub_menus;	/* Array of the sub menu handles */
    Menu_item      *menu_items /* [TEXTSW_MENU_LAST_CMD] */ ;
}               Local_menu_object;

int TXT_MENU_REFCOUNT_KEY, TXT_MENU_KEY, TXT_SUB_MENU_KEY;
int TXT_MENU_ITEMS_KEY, TXT_FILE_MENU_KEY, TXT_SET_DEF_KEY;
static Menu    *textsw_file_menu;
static short    set_def  = FALSE;

Pkg_private int      textsw_has_been_modified();
pkg_private void textsw_get_extend_to_edge();
pkg_private void textsw_set_extend_to_edge();
Pkg_private Textsw_view_handle textsw_create_view();
int             textsw_file_do_menu_action();

Pkg_private Menu     textsw_menu_init();
Pkg_private void     textsw_do_menu();
Pkg_private Menu     textsw_get_unique_menu();
Pkg_private Menu_item textsw_extras_gen_proc();
Pkg_private void     textsw_do_save();

int             STORE_FILE_POPUP_KEY;
int             SAVE_FILE_POPUP_KEY;
int             LOAD_FILE_POPUP_KEY;
int             FILE_STUFF_POPUP_KEY;
int             SEARCH_POPUP_KEY;
int             MATCH_POPUP_KEY;
int             SEL_LINE_POPUP_KEY;
int             EXTRASMENU_FILENAME_KEY;
int             TEXTSW_MENU_DATA_KEY;
int             TEXTSW_HANDLE_KEY;
int             TEXTSW_CURRENT_POPUP_KEY;
int             FC_PARENT_KEY;
int             FC_EXTEN_ITEM_KEY;

/* Menu strings for File sub menu */
#define	SAVE_FILE	"Save "
#define	STORE_NEW_FILE	"Save as..."
#define	LOAD_FILE	"Open..."
#define	INCLUDE_FILE	"Include..."
#define	EMPTY_DOC	"Empty Document"

/* Menu strings for Edit sub menu */
#define	AGAIN_STR	"Again"
#define	UNDO_STR	"Undo"
#define	COPY_STR	"Copy"
#define	PASTE_STR	"Paste"
#define	CUT_STR		"Cut"

/* Menu strings for View sub menu */
#define	SEL_LINE_AT_NUM		"Select Line at Number..."
#define	WHAT_LINE_NUM		"What Line Number?"
#define	SHOW_CARET_AT_TOP	"Show Caret at Top"
#define	CHANGE_LINE_WRAP	"Change Line Wrap"

/* Menu strings for Find sub menu */
#define	FIND_REPLACE		"Find and Replace..."
#define	FIND_SELECTION		"Find Selection"
#define	FIND_MARKED_TEXT	"Find Marked Text..."
#define	REPLACE_FIELD		"Replace |>field<| "

static Defaults_pairs line_break_pairs[] = {
    "TEXTSW_CLIP", (int) TEXTSW_CLIP,
    "TEXTSW_WRAP_AT_CHAR", (int) TEXTSW_WRAP_AT_CHAR,
    "TEXTSW_WRAP_AT_WORD", (int) TEXTSW_WRAP_AT_WORD,
    NULL, (int) TEXTSW_WRAP_AT_WORD
};

static int textsw_edit_do_menu_action();
static int textsw_view_do_menu_action();
static int textsw_find_do_menu_action();


/* VARARGS0 */
static void
textsw_new_menu(folio)
    Textsw_folio    folio;
{
    register Menu  *sub_menu;
    register Menu_item *menu_items;
    Menu            undo_cmds, break_mode, find_sel_cmds, select_field_cmds, top_menu;
    Textsw          textsw = FOLIO_REP_TO_ABS(folio);
    Frame           frame = xv_get(textsw, WIN_FRAME);
    Menu_item       break_mode_item, undo_cmds_item, find_sel_cmds_item,
                    select_field_cmds_item;
    int             index;
    Pkg_private char *textsw_get_extras_filename();
    Pkg_private int textsw_build_extras_menu_items();
    char           *filename;
    char           *def_str;
    int             line_break;
    Xv_Server	    server;

    if (!STORE_FILE_POPUP_KEY) {
	STORE_FILE_POPUP_KEY = xv_unique_key();
	SAVE_FILE_POPUP_KEY = xv_unique_key();
	LOAD_FILE_POPUP_KEY = xv_unique_key();
	FILE_STUFF_POPUP_KEY = xv_unique_key();
	SEARCH_POPUP_KEY = xv_unique_key();
	MATCH_POPUP_KEY = xv_unique_key();
	SEL_LINE_POPUP_KEY = xv_unique_key();
	EXTRASMENU_FILENAME_KEY = xv_unique_key();
	TEXTSW_MENU_DATA_KEY = xv_unique_key();
	TEXTSW_HANDLE_KEY = xv_unique_key();
        TXT_MENU_REFCOUNT_KEY = xv_unique_key();
	TXT_MENU_KEY = xv_unique_key();
	TXT_SUB_MENU_KEY = xv_unique_key();
	TXT_MENU_ITEMS_KEY = xv_unique_key();
	TXT_FILE_MENU_KEY = xv_unique_key();
	TXT_SET_DEF_KEY = xv_unique_key();
	TEXTSW_CURRENT_POPUP_KEY = xv_unique_key();
	FC_PARENT_KEY = xv_unique_key();
	FC_EXTEN_ITEM_KEY = xv_unique_key();
    }

    menu_items = (Menu_item *) malloc((unsigned) TEXTSW_MENU_LAST_CMD *
				      sizeof(Menu_item));
    sub_menu = (Menu *) malloc(((unsigned) TXTSW_EXTRAS_SUB_MENU + 1) * (sizeof(Menu)));

    server = XV_SERVER_FROM_WINDOW(textsw);
    break_mode = xv_create(server, MENU_CHOICE_MENU,
			   HELP_INFO("textsw:mbreakmode")
			   NULL);
    menu_items[(int) TEXTSW_MENU_WRAP_LINES_AT_WORD] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG("Wrap at Word"),
		  MENU_VALUE, TEXTSW_MENU_WRAP_LINES_AT_WORD,
		  HELP_INFO("textsw:mwrapwords")
		  NULL);
    menu_items[(int) TEXTSW_MENU_WRAP_LINES_AT_CHAR] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG("Wrap at Character"),
		  MENU_VALUE, TEXTSW_MENU_WRAP_LINES_AT_CHAR,
		  HELP_INFO("textsw:mwrapchars")
		  NULL);
    menu_items[(int) TEXTSW_MENU_CLIP_LINES] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG("Wrap at Character"),
		  MENU_STRING, XV_MSG("Clip Lines"),
		  MENU_VALUE, TEXTSW_MENU_CLIP_LINES,
		  HELP_INFO("textsw:mcliplines")
		  NULL);
    def_str = defaults_get_string("text.lineBreak", "Text.LineBreak", (char *) 0);
    if (def_str == NULL || def_str[0] == '\0' ||
	(line_break = (int) defaults_lookup(def_str, line_break_pairs)) == TEXTSW_WRAP_AT_WORD)
	xv_set(break_mode,
	 MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_WRAP_LINES_AT_WORD],
	 MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_WRAP_LINES_AT_CHAR],
	       MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_CLIP_LINES],
	       NULL);
    else if (TEXTSW_WRAP_AT_CHAR == line_break)
	xv_set(break_mode,
	 MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_WRAP_LINES_AT_CHAR],
	 MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_WRAP_LINES_AT_WORD],
	       MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_CLIP_LINES],
	       NULL);
    else
	xv_set(break_mode,
	       MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_CLIP_LINES],
	 MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_WRAP_LINES_AT_CHAR],
	 MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_WRAP_LINES_AT_WORD],
	       NULL);

    undo_cmds = xv_create(server, MENU,
			  HELP_INFO("textsw:mundocmds")
			  NULL);
    menu_items[(int) TEXTSW_MENU_UNDO] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG("Undo Last Edit"),
		  MENU_VALUE, TEXTSW_MENU_UNDO,
		  HELP_INFO("textsw:mundolast")
		  NULL);

    /*
     * Set accelerator for menu item
     */
    xv_set(menu_items[(int) TEXTSW_MENU_UNDO], 
		MENU_ACCELERATOR, "coreset Undo", NULL);

    menu_items[(int) TEXTSW_MENU_UNDO_ALL] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG("Undo All Edits"),
		  MENU_VALUE, TEXTSW_MENU_UNDO_ALL,
		  HELP_INFO("textsw:mundoall")
		  NULL);
    xv_set(undo_cmds,
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_UNDO],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_UNDO_ALL],
	   NULL);
    xv_set(undo_cmds, XV_KEY_DATA, TEXTSW_MENU_DATA_KEY, textsw, NULL);

    select_field_cmds = xv_create(server, MENU,
				  HELP_INFO("textsw:mselfieldcmds")
				  NULL);
    menu_items[(int) TEXTSW_MENU_SEL_ENCLOSE_FIELD] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG("Expand"),
		  MENU_VALUE, TEXTSW_MENU_SEL_ENCLOSE_FIELD,
		  HELP_INFO("textsw:mselexpand")
		  NULL);
    menu_items[(int) TEXTSW_MENU_SEL_NEXT_FIELD] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG("Next"),
		  MENU_VALUE, TEXTSW_MENU_SEL_NEXT_FIELD,
		  HELP_INFO("textsw:mselnext")
		  NULL);
    menu_items[(int) TEXTSW_MENU_SEL_PREV_FIELD] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG("Previous"),
		  MENU_VALUE, TEXTSW_MENU_SEL_PREV_FIELD,
		  HELP_INFO("textsw:mselprevious")
		  NULL);
    xv_set(select_field_cmds,
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_SEL_ENCLOSE_FIELD],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_SEL_NEXT_FIELD],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_SEL_PREV_FIELD],
	   NULL);


    find_sel_cmds = xv_create(server, MENU,
			      HELP_INFO("textsw:mfindselcmds")
			      NULL);
    menu_items[(int) TEXTSW_MENU_FIND] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG("Forward"),
		  MENU_VALUE, TEXTSW_MENU_FIND,
		  HELP_INFO("textsw:mfindforward")
		  NULL);

    /*
     * Set accelerator for menu item
     */
    xv_set(menu_items[(int) TEXTSW_MENU_FIND],
		MENU_ACCELERATOR, "coreset Find", NULL);

    menu_items[(int) TEXTSW_MENU_FIND_BACKWARD] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG("Backward"),
		  MENU_VALUE, TEXTSW_MENU_FIND_BACKWARD,
		  HELP_INFO("textsw:mfindbackward")
		  NULL);
    xv_set(find_sel_cmds,
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_FIND],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_FIND_BACKWARD],
	   NULL);

    menu_items[(int) TEXTSW_MENU_LOAD] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG(LOAD_FILE),
		  MENU_VALUE, TEXTSW_MENU_LOAD,
		  HELP_INFO("textsw:mloadfile")
		  NULL);

    /*
     * Set accelerator for menu item
     */
    xv_set(menu_items[(int) TEXTSW_MENU_LOAD],
		MENU_ACCELERATOR, "coreset Open", NULL);

    menu_items[(int) TEXTSW_MENU_SAVE] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG(SAVE_FILE),
		  MENU_VALUE, TEXTSW_MENU_SAVE,
		  HELP_INFO("textsw:msavefile")
		  NULL);

    menu_items[(int) TEXTSW_MENU_STORE] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG(STORE_NEW_FILE),
		  MENU_VALUE, TEXTSW_MENU_STORE,
    /* MENU_LINE_AFTER_ITEM,	MENU_HORIZONTAL_LINE, */
		  HELP_INFO("textsw:mstorefile")
		  NULL);

    /*
     * Set accelerator for menu item.
     * Meta+S will accelerate "Save"
     *
     * Meta+S is an already existing SunView key binding for 
     * "Save As".
     * We override it here with a different action even though
     * this may cause compatibility problems because this
     * is required by the spec.
     */
    xv_set(menu_items[(int) TEXTSW_MENU_SAVE],
		MENU_ACCELERATOR, "coreset Save", NULL);


    menu_items[(int) TEXTSW_MENU_FILE_STUFF] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG(INCLUDE_FILE),
		  MENU_VALUE, TEXTSW_MENU_FILE_STUFF,
		  HELP_INFO("textsw:mincludefile")
		  NULL);
    menu_items[(int) TEXTSW_MENU_RESET] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG(EMPTY_DOC),
		  MENU_VALUE, TEXTSW_MENU_RESET,
		  HELP_INFO("textsw:memptydoc")
		  NULL);

    sub_menu[(int) TXTSW_FILE_SUB_MENU] = xv_create(server, MENU,
					       HELP_INFO("textsw:mfilecmds")
						    NULL);

    xv_set(sub_menu[(int) TXTSW_FILE_SUB_MENU],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_LOAD],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_SAVE],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_STORE],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_FILE_STUFF],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_RESET],
	   NULL);

    menu_items[(int) TEXTSW_MENU_AGAIN] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG(AGAIN_STR),
		  MENU_VALUE, TEXTSW_MENU_AGAIN,
		  HELP_INFO("textsw:meditagain")
		  NULL);
    undo_cmds_item = xv_create(XV_ZERO,
			       MENUITEM,
			       MENU_STRING, XV_MSG(UNDO_STR),
			       MENU_PULLRIGHT, undo_cmds,
    /* MENU_LINE_AFTER_ITEM,	 MENU_HORIZONTAL_LINE, */
			       HELP_INFO("textsw:meditundo")
			       NULL);
    menu_items[(int) TEXTSW_MENU_COPY] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING,  XV_MSG(COPY_STR),
		  MENU_VALUE, TEXTSW_MENU_COPY,
		  HELP_INFO("textsw:meditcopy")
		  NULL);
    menu_items[(int) TEXTSW_MENU_PASTE] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG(PASTE_STR),
		  MENU_VALUE, TEXTSW_MENU_PASTE,
		  HELP_INFO("textsw:meditpaste")
		  NULL);
    menu_items[(int) TEXTSW_MENU_CUT] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG(CUT_STR),
		  MENU_VALUE, TEXTSW_MENU_CUT,
    /* MENU_LINE_AFTER_ITEM,	MENU_HORIZONTAL_LINE, */
		  HELP_INFO("textsw:meditcut")
		  NULL);

    /*
     * Set accelerator for menu items
     */
    xv_set(menu_items[(int) TEXTSW_MENU_COPY], 
		MENU_ACCELERATOR, "coreset Copy", NULL);
    xv_set(menu_items[(int) TEXTSW_MENU_PASTE], 
		MENU_ACCELERATOR, "coreset Paste", NULL);
    xv_set(menu_items[(int) TEXTSW_MENU_CUT], 
		MENU_ACCELERATOR, "coreset Cut", NULL);

    sub_menu[(int) TXTSW_EDIT_SUB_MENU] = xv_create(server, MENU,
					       HELP_INFO("textsw:meditcmds")
						    NULL);
    xv_set(sub_menu[(int) TXTSW_EDIT_SUB_MENU],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_AGAIN],
	   MENU_APPEND_ITEM, undo_cmds_item,
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_COPY],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_PASTE],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_CUT],
	   NULL);


    menu_items[(int) TEXTSW_MENU_NORMALIZE_LINE] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG(SEL_LINE_AT_NUM),
		  MENU_VALUE, TEXTSW_MENU_NORMALIZE_LINE,
		  HELP_INFO("textsw:mselectline")
		  NULL);
    menu_items[(int) TEXTSW_MENU_COUNT_TO_LINE] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG(WHAT_LINE_NUM),
		  MENU_VALUE, TEXTSW_MENU_COUNT_TO_LINE,
    /* MENU_LINE_AFTER_ITEM,	MENU_HORIZONTAL_LINE, */
		  HELP_INFO("textsw:mwhatline")
		  NULL);
    menu_items[(int) TEXTSW_MENU_NORMALIZE_INSERTION] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG(SHOW_CARET_AT_TOP),
		  MENU_VALUE, TEXTSW_MENU_NORMALIZE_INSERTION,
		  HELP_INFO("textsw:mshowcaret")
		  NULL);
    break_mode_item = xv_create(XV_ZERO,
				MENUITEM,
				MENU_STRING, 
				XV_MSG(CHANGE_LINE_WRAP),
				MENU_PULLRIGHT, break_mode,
				HELP_INFO("textsw:mchangelinewrap")
				NULL);

    sub_menu[(int) TXTSW_VIEW_SUB_MENU] = xv_create(server, MENU,
					    HELP_INFO("textsw:mdisplaycmds")
						    NULL);
    xv_set(sub_menu[(int) TXTSW_VIEW_SUB_MENU],
	   MENU_CLIENT_DATA, textsw,
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_NORMALIZE_LINE],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_COUNT_TO_LINE],
	MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_NORMALIZE_INSERTION],
	   MENU_APPEND_ITEM, break_mode_item,
	   NULL);

    menu_items[(int) TEXTSW_MENU_FIND_AND_REPLACE] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG(FIND_REPLACE),
		  MENU_VALUE, TEXTSW_MENU_FIND_AND_REPLACE,
    /* MENU_LINE_AFTER_ITEM,	MENU_HORIZONTAL_LINE, */
		  HELP_INFO("textsw:mfindreplace")
		  NULL);
    find_sel_cmds_item = xv_create(XV_ZERO,
				   MENUITEM,
				   MENU_STRING, 
				   XV_MSG(FIND_SELECTION),
				   MENU_PULLRIGHT, find_sel_cmds,
				   HELP_INFO("textsw:mfindselcmds")
				   NULL);
    menu_items[(int) TEXTSW_MENU_SEL_MARK_TEXT] =
	xv_create(XV_ZERO,
		  MENUITEM,
		  MENU_STRING, XV_MSG(FIND_MARKED_TEXT),
		  MENU_VALUE, TEXTSW_MENU_SEL_MARK_TEXT,
		  HELP_INFO("textsw:mfindtext")
		  NULL);
    select_field_cmds_item = xv_create(XV_ZERO,
				       MENUITEM,
				       MENU_STRING, 
				       XV_MSG(REPLACE_FIELD),
				       MENU_PULLRIGHT, select_field_cmds,
				       HELP_INFO("textsw:mselfieldcmds")
				       NULL);

    sub_menu[(int) TXTSW_FIND_SUB_MENU] = xv_create(server, MENU,
					       HELP_INFO("textsw:mfindcmds")
						    NULL);
    xv_set(sub_menu[(int) TXTSW_FIND_SUB_MENU],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_FIND_AND_REPLACE],
	   MENU_APPEND_ITEM, find_sel_cmds_item,
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_SEL_MARK_TEXT],
	   MENU_APPEND_ITEM, select_field_cmds_item,
	   NULL);

    sub_menu[(int) TXTSW_EXTRAS_SUB_MENU] = xv_create(server, MENU,
					      HELP_INFO("textsw:extrasmenu")
						      NULL);


    top_menu = xv_create(server, MENU,
			 MENU_TITLE_ITEM, XV_MSG("Text Pane"),
			 HELP_INFO("textsw:mtopmenu")
			 NULL);
    menu_items[(int) TEXTSW_MENU_FILE_CMDS] = xv_create(XV_ZERO,
							MENUITEM,
							MENU_STRING, 
						XV_MSG("File"),
			MENU_PULLRIGHT, sub_menu[(int) TXTSW_FILE_SUB_MENU],
					       HELP_INFO("textsw:mfilecmds")
							NULL);


    menu_items[(int) TEXTSW_MENU_VIEW_CMDS] = xv_create(XV_ZERO,
							MENUITEM,
							MENU_STRING, 
						XV_MSG("View"),
			MENU_PULLRIGHT, sub_menu[(int) TXTSW_VIEW_SUB_MENU],
					    HELP_INFO("textsw:mdisplaycmds")
							NULL);
    menu_items[(int) TEXTSW_MENU_EDIT_CMDS] = xv_create(XV_ZERO,
							MENUITEM,
							MENU_STRING, 
						XV_MSG("Edit"),
			MENU_PULLRIGHT, sub_menu[(int) TXTSW_EDIT_SUB_MENU],
					       HELP_INFO("textsw:meditcmds")
							NULL);
    menu_items[(int) TEXTSW_MENU_FIND_CMDS] = xv_create(XV_ZERO,
							MENUITEM,
							MENU_STRING, 
						XV_MSG("Find"),
			MENU_PULLRIGHT, sub_menu[(int) TXTSW_FIND_SUB_MENU],
					       HELP_INFO("textsw:mfindcmds")
							NULL);
    menu_items[(int) TEXTSW_MENU_EXTRAS_CMDS] = xv_create(XV_ZERO,
							  MENUITEM,
				      MENU_GEN_PROC, textsw_extras_gen_proc,
		      MENU_PULLRIGHT, sub_menu[(int) TXTSW_EXTRAS_SUB_MENU],
						      MENU_STRING, 
					 XV_MSG("Extras"),
						   MENU_CLIENT_DATA, textsw,
						 HELP_INFO("textsw:mextras")
							  NULL);

    textsw_file_menu = (Menu *)menu_items[(int) TEXTSW_MENU_FILE_CMDS];

    filename = textsw_get_extras_filename(menu_items[(int) TEXTSW_MENU_EXTRAS_CMDS]);
    (void) textsw_build_extras_menu_items(textsw, filename, sub_menu[(int) TXTSW_EXTRAS_SUB_MENU]);

    xv_set(top_menu,
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_FILE_CMDS],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_VIEW_CMDS],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_EDIT_CMDS],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_FIND_CMDS],
	   MENU_APPEND_ITEM, menu_items[(int) TEXTSW_MENU_EXTRAS_CMDS],
	   NULL);

    for (index = (int) TEXTSW_MENU_LOAD;
	 index <= (int) TEXTSW_MENU_RESET;
	 index++) {
	if (menu_items[index]) {
	    menu_set(menu_items[index],
		     MENU_ACTION, textsw_file_do_menu_action,
		     NULL);
	}
    }
    for (index = (int) TEXTSW_MENU_AGAIN;
	 index <= (int) TEXTSW_MENU_CUT;
	 index++) {
	if (menu_items[index]) {
	    menu_set(menu_items[index],
		     MENU_ACTION, textsw_edit_do_menu_action,
		     NULL);
	}
    }
    for (index = (int) TEXTSW_MENU_NORMALIZE_LINE;
	 index <= (int) TEXTSW_MENU_CLIP_LINES;
	 index++) {
	if (menu_items[index]) {
	    menu_set(menu_items[index],
		     MENU_ACTION, textsw_view_do_menu_action,
		     NULL);
	}
    }
    for (index = (int) TEXTSW_MENU_FIND_AND_REPLACE;
	 index <= (int) TEXTSW_MENU_SEL_PREV_FIELD;
	 index++) {
	if (menu_items[index]) {
	    menu_set(menu_items[index],
		     MENU_ACTION, textsw_find_do_menu_action,
		     NULL);
	}
    }


    /*
     * This is a fix/hack for menu accelerators.
     * The menu action procs depend on TEXTSW_MENU_DATA_KEY
     * to get the textsw view. The textsw view is set on the
     * menu in the event proc for the textsw view.
     * Since we don't have that info when accelerators are
     * used, we have to set it here. A new key to store the
     * textsw is used because at this time in this function,
     * the views do not exist yet.
     *
     * Note:
     * Sharing of the textsw menus may break menu accelerators.
     */
    xv_set(sub_menu[(int) TXTSW_FILE_SUB_MENU], 
		XV_KEY_DATA, TEXTSW_HANDLE_KEY, textsw, NULL);
    xv_set(sub_menu[(int) TXTSW_EDIT_SUB_MENU], 
		XV_KEY_DATA, TEXTSW_HANDLE_KEY, textsw, NULL);
    xv_set(undo_cmds,
		XV_KEY_DATA, TEXTSW_HANDLE_KEY, textsw, NULL);
    xv_set(find_sel_cmds,
		XV_KEY_DATA, TEXTSW_HANDLE_KEY, textsw, NULL);

    xv_set(sub_menu[(int) TXTSW_EDIT_SUB_MENU], MENU_GEN_PIN_WINDOW, frame, 
	XV_MSG("Edit"),
	   NULL);

    folio->sub_menu_table = sub_menu;
    folio->menu_table = menu_items;

    xv_set(top_menu, MENU_DONE_PROC, textsw_done_menu, NULL);
    folio->menu = top_menu;
}


Pkg_private          Menu
textsw_menu_init(folio)
    Textsw_folio    folio;
{
    int ref;
	Textsw textsw = FOLIO_REP_TO_ABS(folio);
	Xv_Screen screen =  (Xv_Screen) xv_get(xv_get(textsw, WIN_FRAME), XV_SCREEN );
							
/* BUG: All this caching code was removed as a quick fix. It needs to be really
   fixed.

	if (xv_get(screen, XV_KEY_DATA, TXT_MENU_KEY) != 0) {
		folio->menu = (Menu) xv_get(screen, XV_KEY_DATA, TXT_MENU_KEY);
		folio->menu_table = (Menu_item *) xv_get(screen, XV_KEY_DATA, TXT_MENU_ITEMS_KEY );
		folio->sub_menu_table = (Menu *) xv_get(screen, XV_KEY_DATA, TXT_SUB_MENU_KEY);
	} else {

*/
                (void) textsw_new_menu(folio);

	    xv_set(screen, XV_KEY_DATA, TXT_MENU_KEY, folio->menu, NULL);
		xv_set(screen, XV_KEY_DATA, TXT_MENU_ITEMS_KEY, folio->menu_table, NULL);
		xv_set(screen, XV_KEY_DATA, TXT_SUB_MENU_KEY, folio->sub_menu_table, NULL);
		xv_set(screen, XV_KEY_DATA, TXT_MENU_REFCOUNT_KEY, 0, NULL);
/*
	}
*/
    ref = (int) xv_get(screen, XV_KEY_DATA, TXT_SUB_MENU_KEY);
    ref++;
    xv_set(screen, XV_KEY_DATA, TXT_MENU_REFCOUNT_KEY, ref, NULL);


    return (folio->menu);
}

Pkg_private          Menu
textsw_get_unique_menu(folio)
    Textsw_folio    folio;
{
    int ref;
	Textsw textsw = FOLIO_REP_TO_ABS(folio);
	Xv_Screen screen = (Xv_Screen) xv_get(xv_get(textsw, WIN_FRAME),
	XV_SCREEN);
			 
    if (folio->menu == (Menu) xv_get(screen, XV_KEY_DATA, TXT_MENU_KEY)) {    /* refcount == 1 ==> textsw is the only referencer */
        ref = (int) xv_get(screen, XV_KEY_DATA, TXT_SUB_MENU_KEY);
        if (ref == 1) {
            xv_set(screen, XV_KEY_DATA, TXT_MENU_KEY, 0, NULL);
            xv_set(screen, XV_KEY_DATA, TXT_MENU_ITEMS_KEY, 0, NULL);
            xv_set(screen, XV_KEY_DATA, TXT_MENU_REFCOUNT_KEY, 0, NULL);
        } else {
            (void) textsw_new_menu(folio);
            xv_set(screen, XV_KEY_DATA, TXT_MENU_KEY, folio->menu, NULL);
            ref--;
            xv_set(screen, XV_KEY_DATA, TXT_MENU_REFCOUNT_KEY, ref, NULL);
        }
    }
    return (folio->menu);
}

Pkg_private void
textsw_do_menu(view, ie)
    Textsw_view_handle view;
    Event          *ie;
{
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    Textsw_view     textsw_view = VIEW_REP_TO_ABS(view);

    /* freeze caret so don't invalidate menu's bitmap under data */
    textsw_freeze_caret(folio);

    xv_set(folio->menu, XV_KEY_DATA, TEXTSW_MENU_DATA_KEY,
	   textsw_view, NULL);

    menu_show(folio->menu, textsw_view, ie, NULL);

}

static     void
textsw_done_menu(menu, result)
    Menu            menu;
    Xv_opaque	    result;
{
    Textsw_view textsw_view = (Textsw_view) xv_get(menu,
	XV_KEY_DATA, TEXTSW_MENU_DATA_KEY);
    Textsw_view_handle view = VIEW_ABS_TO_REP(textsw_view);

    textsw_thaw_caret(FOLIO_FOR_VIEW(view));
    textsw_stablize(FOLIO_FOR_VIEW(view));
}

Pkg_private     Textsw_view
textsw_from_menu(menu)
    Menu            menu;
{
    Textsw_view     textsw_view = XV_ZERO;
    Menu            temp_menu;
    Menu_item       temp_item;

    while (menu) {
	temp_item = xv_get(menu, MENU_PARENT);
	if (temp_item) {
	    temp_menu = xv_get(temp_item, MENU_PARENT);

	    /* if there is no menu parent, use menu's view */
	    if (temp_menu == 0)
		textsw_view = (Textsw_view) xv_get(menu, XV_KEY_DATA,
			TEXTSW_MENU_DATA_KEY);
	    menu = temp_menu;

	} else {
	    textsw_view = (Textsw_view) xv_get(menu, XV_KEY_DATA,
					       TEXTSW_MENU_DATA_KEY);
	    break;
	}
    }
    return (textsw_view);
}

Pkg_private	int
textsw_file_do_menu_action(cmd_menu, cmd_item)
    Menu            cmd_menu;
    Menu_item       cmd_item;
{
    Textsw          abstract;
    Textsw_view     textsw_view = textsw_from_menu(cmd_menu);
    register Textsw_view_handle view;
    register Textsw_folio textsw;
    Textsw_menu_cmd cmd = (Textsw_menu_cmd)
    menu_get(cmd_item, MENU_VALUE, 0);
    register Event *ie = (Event *)
    menu_get(cmd_menu, MENU_FIRST_EVENT, 0);
    register int    locx, locy;
    Xv_Notice	    text_notice;

    if AN_ERROR(textsw_view == 0)  {
	if (event_action(ie) == ACTION_ACCELERATOR)  {
            abstract = xv_get(cmd_menu, XV_KEY_DATA, TEXTSW_HANDLE_KEY);
            textsw = TEXTSW_PRIVATE(abstract);
	    textsw_view = (Textsw_view)xv_get(abstract, OPENWIN_NTH_VIEW, NULL);
            view = VIEW_ABS_TO_REP(textsw_view);
	}
	else  {
            return -1;
	}
    }
    else  {
        view = VIEW_ABS_TO_REP(textsw_view);
        textsw = FOLIO_FOR_VIEW(view);
        abstract = TEXTSW_PUBLIC(textsw);
    }

    if AN_ERROR
	(ie == 0) {
	locx = locy = 0;
    } else {
	locx = ie->ie_locx;
	locy = ie->ie_locy;
    }

    switch (cmd) {

      case TEXTSW_MENU_RESET:
	textsw_empty_document(abstract, ie);
	(void) xv_set( cmd_menu, MENU_DEFAULT, 1, 0 );
	break;

      case TEXTSW_MENU_LOAD:{
	    Frame           base_frame = (Frame) xv_get(abstract, WIN_FRAME);
	    Frame           popup = (Frame) xv_get(base_frame, XV_KEY_DATA,
						   LOAD_FILE_POPUP_KEY);

	    if (textsw->state & TXTSW_NO_LOAD) {
		Frame	frame;

		frame = FRAME_FROM_FOLIO_OR_VIEW(view);
                text_notice = (Xv_Notice)xv_get(frame, 
			XV_KEY_DATA, text_notice_key, NULL);
                if (!text_notice)  {
                    text_notice = xv_create(frame, NOTICE,
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("Illegal Operation.\n\
Load File Has Been Disabled."),
                        0,
                        NOTICE_BUTTON_YES, XV_MSG("Continue"),
                        XV_SHOW, TRUE,
                        NULL);

                    xv_set(frame, 
                        XV_KEY_DATA, text_notice_key, text_notice,
                        NULL);
                }
                else  {
                    xv_set(text_notice, 
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("Illegal Operation.\n\
Load File Has Been Disabled."),
                        0,
                        NOTICE_BUTTON_YES, XV_MSG("Continue"),
                        XV_SHOW, TRUE, 
                        NULL);
                }

		break;
	    }
	    if (popup) {
		(void) textsw_set_dir_str((int) TEXTSW_MENU_LOAD);
		(void) textsw_get_and_set_selection(popup, view, (int) TEXTSW_MENU_LOAD);
	    } else {
		(void) textsw_create_popup_frame(view, (int) TEXTSW_MENU_LOAD);
	    }
	    break;
	}

      case TEXTSW_MENU_SAVE: {
            textsw_do_save(abstract, textsw, view);
	    break;
     }

      case TEXTSW_MENU_STORE:{
	    Frame           base_frame = (Frame) xv_get(abstract, WIN_FRAME);
	    Frame           popup = (Frame) xv_get(base_frame, XV_KEY_DATA,
						   STORE_FILE_POPUP_KEY);
	    if (popup) {
		(void) textsw_set_dir_str((int) TEXTSW_MENU_STORE);
		(void) textsw_get_and_set_selection(popup, view, (int) TEXTSW_MENU_STORE);
	    } else {
		(void) textsw_create_popup_frame(view, (int) TEXTSW_MENU_STORE);
	    }
	    break;
	}

      case TEXTSW_MENU_FILE_STUFF:{
	    Frame           base_frame = (Frame) xv_get(abstract, WIN_FRAME);
	    Frame           popup = (Frame) xv_get(base_frame, XV_KEY_DATA,
						   FILE_STUFF_POPUP_KEY);
	    if (popup) {
		(void) textsw_set_dir_str((int) TEXTSW_MENU_FILE_STUFF);
		(void) textsw_get_and_set_selection(popup, view,
					      (int) TEXTSW_MENU_FILE_STUFF);
	    } else {
		(void) textsw_create_popup_frame(view, (int) TEXTSW_MENU_FILE_STUFF);
	    }
	    break;
	}

      default:
	break;
    }
}

static int
textsw_edit_do_menu_action(cmd_menu, cmd_item)
    Menu            cmd_menu;
    Menu_item       cmd_item;
{
    Textsw          abstract;
    Textsw_view     textsw_view = textsw_from_menu(cmd_menu);
    register Textsw_view_handle view;
    register Textsw_folio textsw;
    Textsw_menu_cmd cmd = (Textsw_menu_cmd)
    menu_get(cmd_item, MENU_VALUE, 0);
    register Event *ie = (Event *)
    menu_get(cmd_menu, MENU_FIRST_EVENT, 0);
    register int    locx, locy;
    Xv_Notice	    text_notice;
    int		    menu_pinned = FALSE;
    Frame	    frame;
    Frame	    menu_cmd_frame = 
			(Frame)xv_get(cmd_menu, MENU_PIN_WINDOW);

    if AN_ERROR(textsw_view == 0)  {
	if (event_action(ie) == ACTION_ACCELERATOR)  {
            abstract = xv_get(cmd_menu, XV_KEY_DATA, TEXTSW_HANDLE_KEY);
            textsw = TEXTSW_PRIVATE(abstract);
	    textsw_view = (Textsw_view)xv_get(abstract, OPENWIN_NTH_VIEW, NULL);
            view = VIEW_ABS_TO_REP(textsw_view);
	}
	else  {
            return -1;
	}
    }
    else  {
        view = VIEW_ABS_TO_REP(textsw_view);
        textsw = FOLIO_FOR_VIEW(view);
        abstract = TEXTSW_PUBLIC(textsw);
    }

    if (menu_cmd_frame && (xv_get(menu_cmd_frame, XV_SHOW)))  {
	menu_pinned = TRUE;
    }

    if AN_ERROR
	(ie == 0) {
	locx = locy = 0;
    } else {
	locx = ie->ie_locx;
	locy = ie->ie_locy;
    }

    switch (cmd) {

      case TEXTSW_MENU_AGAIN:
	textsw_again(view, locx, locy);
	break;

      case TEXTSW_MENU_UNDO:
	if (textsw_has_been_modified(abstract))
	    textsw_undo(textsw);
	break;

      case TEXTSW_MENU_UNDO_ALL:
	if (textsw_has_been_modified(abstract)) {
	    int             result;

	    frame = (Frame)xv_get(abstract, WIN_FRAME);
            text_notice = (Xv_Notice)xv_get(frame, 
                                XV_KEY_DATA, text_notice_key, 
				NULL);
            if (!text_notice)  {
                text_notice = xv_create(frame, NOTICE,
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("Undo All Edits will discard unsaved edits.\n\
Please confirm."),
                        0,
                        NOTICE_BUTTON_YES, 
			    XV_MSG("Confirm, discard edits"),
                        NOTICE_BUTTON_NO, XV_MSG("Cancel"),
			/*
                        NOTICE_NO_BEEPING, 1,
			*/
		        NOTICE_STATUS, &result,
                        XV_SHOW, TRUE,
                        NULL);

                xv_set(frame, 
                        XV_KEY_DATA, text_notice_key, text_notice,
                        NULL);
            }
            else  {
                xv_set(text_notice, 
		    NOTICE_LOCK_SCREEN, FALSE,
		    NOTICE_BLOCK_THREAD, TRUE,
                    NOTICE_MESSAGE_STRINGS,
		    XV_MSG("Undo All Edits will discard unsaved edits.\n\
Please confirm."),
                    0,
                    NOTICE_BUTTON_YES, 
			XV_MSG("Confirm, discard edits"),
                    NOTICE_BUTTON_NO, XV_MSG("Cancel"),
		    /*
                    NOTICE_NO_BEEPING, 1,
		    */
		    NOTICE_STATUS, &result,
		    XV_SHOW, TRUE,
                    NULL);
            }

	    if (result == NOTICE_YES)
		textsw_reset_2(abstract, locx, locy, TRUE, TRUE);
	}
	break;

      case TEXTSW_MENU_CUT:{
	    Es_index        first, last_plus_one;

	    (void) ev_get_selection(textsw->views,
				    &first, &last_plus_one, EV_SEL_PRIMARY);
	    if (first < last_plus_one)	/* Local primary selection */
		textsw_function_delete(view);
	    else {
		frame = (Frame)xv_get(VIEW_REP_TO_ABS(view), WIN_FRAME);

                text_notice = (Xv_Notice)xv_get(frame, 
			        XV_KEY_DATA, text_notice_key, NULL);
                if (!text_notice)  {
                    text_notice = xv_create(frame, NOTICE,
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("Please make a primary selection in this textsw first.\n\
Press \"Continue\" to proceed."),
                        0,
                        NOTICE_BUTTON_YES, XV_MSG("Continue"),
                        XV_SHOW, TRUE,
			NOTICE_BUSY_FRAMES,
				menu_pinned ? menu_cmd_frame : XV_ZERO,
			NULL,
                        NULL);

                    xv_set(frame, 
                        XV_KEY_DATA, text_notice_key, text_notice,
                        NULL);
                }
                else  {
                    xv_set(text_notice, 
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("Please make a primary selection in this textsw first.\n\
Press \"Continue\" to proceed."),
                        0,
                        NOTICE_BUTTON_YES, XV_MSG("Continue"),
                        XV_SHOW, TRUE, 
			NOTICE_BUSY_FRAMES,
				menu_pinned ? menu_cmd_frame : XV_ZERO,
			NULL,
                        NULL);
                }
	    }
	    break;
	}
      case TEXTSW_MENU_COPY:{
	    if (textsw_is_seln_nonzero(textsw, EV_SEL_PRIMARY))
		textsw_put(view);
	    else {
                frame = (Frame)xv_get(VIEW_REP_TO_ABS(view), WIN_FRAME);
                text_notice = (Xv_Notice)xv_get(frame, 
			        XV_KEY_DATA, text_notice_key, NULL);
                if (!text_notice)  {
                    text_notice = xv_create(frame, NOTICE,
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("Please make a primary selection first.\n\
Press \"Continue\" to proceed."),
                        0,
                        NOTICE_BUTTON_YES, XV_MSG("Continue"),
                        XV_SHOW, TRUE,
			NOTICE_BUSY_FRAMES,
				menu_pinned ? menu_cmd_frame : XV_ZERO,
			NULL,
			NULL,
                        NULL);

                    xv_set(frame, 
                        XV_KEY_DATA, text_notice_key, text_notice,
                        NULL);
                }
                else  {
                    xv_set(text_notice, 
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("Please make a primary selection first.\n\
Press \"Continue\" to proceed."),
                        0,
                        NOTICE_BUTTON_YES, XV_MSG("Continue"),
                        XV_SHOW, TRUE, 
			NOTICE_BUSY_FRAMES,
				menu_pinned ? menu_cmd_frame : XV_ZERO,
			NULL,
                        NULL);
                }
	    }
	    break;
	}
      case TEXTSW_MENU_PASTE:{
	    textsw_function_get(view);
	    break;
	}
      default:
	break;
    }
}

static int
textsw_view_do_menu_action(cmd_menu, cmd_item)
    Menu            cmd_menu;
    Menu_item       cmd_item;
{
    Pkg_private int      textsw_match_selection_and_normalize();
    Textsw          abstract;
    Textsw_view     textsw_view = textsw_from_menu(cmd_menu);
    register Textsw_view_handle view;
    register Textsw_folio textsw;
    Textsw_menu_cmd cmd = (Textsw_menu_cmd)
    menu_get(cmd_item, MENU_VALUE, 0);
    Es_index        first, last_plus_one;
    Xv_Notice	    text_notice;
    Frame	    frame;

     if AN_ERROR
	(textsw_view == 0)
	    return -1;

    view = VIEW_ABS_TO_REP(textsw_view);
    textsw = FOLIO_FOR_VIEW(view);
    abstract = TEXTSW_PUBLIC(textsw);

    switch (cmd) {

      case TEXTSW_MENU_CLIP_LINES:
	xv_set(FOLIO_REP_TO_ABS(FOLIO_FOR_VIEW(view)),
	       TEXTSW_LINE_BREAK_ACTION, TEXTSW_CLIP,
	       NULL);
	break;

      case TEXTSW_MENU_WRAP_LINES_AT_CHAR:
	xv_set(FOLIO_REP_TO_ABS(FOLIO_FOR_VIEW(view)),
	       TEXTSW_LINE_BREAK_ACTION, TEXTSW_WRAP_AT_CHAR,
	       NULL);
	break;

      case TEXTSW_MENU_WRAP_LINES_AT_WORD:
	xv_set(FOLIO_REP_TO_ABS(FOLIO_FOR_VIEW(view)),
	       TEXTSW_LINE_BREAK_ACTION, TEXTSW_WRAP_AT_WORD,
	       NULL);
	break;

      case TEXTSW_MENU_NORMALIZE_INSERTION:{
	    Es_index        insert;
	    int             upper_context;
	    insert = EV_GET_INSERT(textsw->views);
	    if (insert != ES_INFINITY) {
		upper_context = (int)
		    ev_get(view->e_view, EV_CHAIN_UPPER_CONTEXT);
		textsw_normalize_internal(view, insert, insert, upper_context, 0,
					  TXTSW_NI_DEFAULT);
	    }
	    break;
	}

      case TEXTSW_MENU_NORMALIZE_LINE:{
	    Frame           base_frame = (Frame) xv_get(abstract, WIN_FRAME);
	    Frame           popup = (Frame) xv_get(base_frame, XV_KEY_DATA,
						   SEL_LINE_POPUP_KEY);
	    if (popup) {
		(void) textsw_get_and_set_selection(popup, view,
					  (int) TEXTSW_MENU_NORMALIZE_LINE);
	    } else {
		(void) textsw_create_popup_frame(view,
					  (int) TEXTSW_MENU_NORMALIZE_LINE);
	    }
	    break;
	}

      case TEXTSW_MENU_COUNT_TO_LINE:{
	    char            msg[200];
	    int             count;
	    if (!textsw_is_seln_nonzero(textsw, EV_SEL_PRIMARY)) {
                frame = (Frame)xv_get(VIEW_REP_TO_ABS(view), WIN_FRAME);
                text_notice = (Xv_Notice)xv_get(frame, 
			        XV_KEY_DATA, text_notice_key, NULL);
                if (!text_notice)  {
                    text_notice = xv_create(frame, NOTICE,
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("Please make a primary selection first.\n\
Press \"Continue\" to proceed."),
                        0,
                        NOTICE_BUTTON_YES, XV_MSG("Continue"),
                        XV_SHOW, TRUE,
                        NULL);

                    xv_set(frame, 
                        XV_KEY_DATA, text_notice_key, text_notice,
                        NULL);
                }
                else  {
                    xv_set(text_notice, 
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("Please make a primary selection first.\n\
Press \"Continue\" to proceed."),
                        0,
                        NOTICE_BUTTON_YES, XV_MSG("Continue"),
                        XV_SHOW, TRUE, 
                        NULL);
                }
		break;
	    }
	    ev_get_selection(
		     textsw->views, &first, &last_plus_one, EV_SEL_PRIMARY);
	    if (first >= last_plus_one)
		break;
	    count = ev_newlines_in_esh(textsw->views->esh, 0, first);
	    (void) sprintf(msg, 
		XV_MSG("Selection starts in line %d."), 
		count + 1);

	    frame = (Frame)xv_get(abstract, WIN_FRAME);
            text_notice = (Xv_Notice)xv_get(frame, 
				XV_KEY_DATA, text_notice_key, 
				NULL);
            if (!text_notice)  {
                text_notice = xv_create(frame, NOTICE,
                                NOTICE_LOCK_SCREEN, FALSE,
			        NOTICE_BLOCK_THREAD, TRUE,
                                NOTICE_MESSAGE_STRINGS,
                                    msg,
                                    XV_MSG("Press \"Continue\" to proceed."),
                                0,
                                NOTICE_BUTTON_YES, 
				    XV_MSG("Continue"),
                                XV_SHOW, TRUE,
                                NULL);

                xv_set(frame, 
                    XV_KEY_DATA, text_notice_key, text_notice,
                    NULL);
            }
            else  {
                xv_set(text_notice, 
                    NOTICE_LOCK_SCREEN, FALSE,
		    NOTICE_BLOCK_THREAD, TRUE,
                    NOTICE_MESSAGE_STRINGS,
                        msg,
                        XV_MSG("Press \"Continue\" to proceed."),
                    0,
                    NOTICE_BUTTON_YES, XV_MSG("Continue"),
                    XV_SHOW, TRUE, 
                    NULL);
            }

	    break;
	}

      default:
	break;
    }
}
static int
textsw_find_do_menu_action(cmd_menu, cmd_item)
    Menu            cmd_menu;
    Menu_item       cmd_item;
{
    Pkg_private void     textsw_find_selection_and_normalize();
    Textsw          abstract;
    Textsw_view     textsw_view = textsw_from_menu(cmd_menu);
    register Textsw_view_handle view;
    register Textsw_folio textsw;
    Textsw_menu_cmd cmd = (Textsw_menu_cmd)
    menu_get(cmd_item, MENU_VALUE, 0);
    Event          *ie = (Event *)
    menu_get(cmd_menu, MENU_FIRST_EVENT, 0);
    register int    locx, locy;
    register long unsigned find_options = 0L;
#ifdef OW_I18N
    static CHAR bar_lt[] = { '<', '|',  0 };
    static CHAR bar_gt[] = { '|', '>',  0 };
#endif

    if AN_ERROR(textsw_view == 0)  {
	if (event_action(ie) == ACTION_ACCELERATOR)  {
            abstract = xv_get(cmd_menu, XV_KEY_DATA, TEXTSW_HANDLE_KEY);
            textsw = TEXTSW_PRIVATE(abstract);
	    textsw_view = (Textsw_view)xv_get(abstract, OPENWIN_NTH_VIEW, NULL);
            view = VIEW_ABS_TO_REP(textsw_view);
	}
	else  {
            return -1;
	}
    }
    else  {
        view = VIEW_ABS_TO_REP(textsw_view);
        textsw = FOLIO_FOR_VIEW(view);
        abstract = TEXTSW_PUBLIC(textsw);
    }


    if AN_ERROR
	(ie == 0) {
	locx = locy = 0;
    } else {
	locx = ie->ie_locx;
	locy = ie->ie_locy;
    }

    switch (cmd) {

      case TEXTSW_MENU_FIND_BACKWARD:
	find_options = TFSAN_BACKWARD;
	/* Fall through */
      case TEXTSW_MENU_FIND:
	find_options |= (EV_SEL_PRIMARY | TFSAN_SHELF_ALSO);
	if (textsw_is_seln_nonzero(textsw, find_options))
	    textsw_find_selection_and_normalize(view, locx, locy, find_options);
	else
	    window_bell(WINDOW_FROM_VIEW(view));
	break;

      case TEXTSW_MENU_SEL_ENCLOSE_FIELD:{
	    int             first, last_plus_one;

	    first = last_plus_one = EV_GET_INSERT(textsw->views);
	    (void) textsw_match_field_and_normalize(view, &first, &last_plus_one,
#ifdef OW_I18N
		 	bar_lt, 2, TEXTSW_FIELD_ENCLOSE, FALSE);
#else
		 	"<|", 2, TEXTSW_FIELD_ENCLOSE, FALSE);
#endif
	    break;
	}
      case TEXTSW_MENU_SEL_NEXT_FIELD:
	(void) textsw_match_selection_and_normalize(view,
#ifdef OW_I18N
			bar_gt, TEXTSW_FIELD_FORWARD);
#else
			 "|>", TEXTSW_FIELD_FORWARD);
#endif
	break;

      case TEXTSW_MENU_SEL_PREV_FIELD:
	(void) textsw_match_selection_and_normalize(view,
#ifdef OW_I18N
			bar_lt, TEXTSW_FIELD_BACKWARD);
#else
			"<|", TEXTSW_FIELD_BACKWARD);
#endif
	break;

      case TEXTSW_MENU_SEL_MARK_TEXT:{
	    Frame           base_frame = (Frame) xv_get(abstract, WIN_FRAME);
	    Frame           popup = (Frame) xv_get(base_frame, XV_KEY_DATA,
						   MATCH_POPUP_KEY);
	    if (popup) {
		(void) xv_set(popup, XV_SHOW, TRUE,
			      WIN_CLIENT_DATA, view, NULL);
	    } else {
		(void) textsw_create_popup_frame(view,
					   (int) TEXTSW_MENU_SEL_MARK_TEXT);
	    }
	    break;
	}

      case TEXTSW_MENU_FIND_AND_REPLACE:{
	    Frame           base_frame = (Frame) xv_get(abstract, WIN_FRAME);
	    Frame           popup = (Frame) xv_get(base_frame, XV_KEY_DATA,
						   SEARCH_POPUP_KEY);
	    if (popup) {
		(void) textsw_get_and_set_selection(popup, view,
					(int) TEXTSW_MENU_FIND_AND_REPLACE);
	    } else {
		(void) textsw_create_popup_frame(view,
					(int) TEXTSW_MENU_FIND_AND_REPLACE);
	    }
	    break;
	}

      default:
	break;
    }
}

Pkg_private void
textsw_set_extend_to_edge(view, height, width)
    Textsw_view_handle view;
    int             height, width;

{
    if (view) {
	if (height == WIN_EXTEND_TO_EDGE)
	    xv_set(VIEW_REP_TO_ABS(view),
		   WIN_DESIRED_HEIGHT, WIN_EXTEND_TO_EDGE,
		   NULL);
	if (width == WIN_EXTEND_TO_EDGE)
	    xv_set(VIEW_REP_TO_ABS(view),
		   WIN_DESIRED_WIDTH, WIN_EXTEND_TO_EDGE,
		   NULL);
    }
}

Pkg_private void
textsw_get_extend_to_edge(view, height, width)
    Textsw_view_handle view;
    int            *height, *width;

{
    *height = 0;
    *width = 0;

    if (view) {
	*height = (int) xv_get(VIEW_REP_TO_ABS(view), WIN_DESIRED_HEIGHT);
	*width = (int) xv_get(VIEW_REP_TO_ABS(view), WIN_DESIRED_WIDTH);
    }
}

/*
 * called after a file is loaded, this sets the menu default to save file
 */

Pkg_private void
textsw_set_file_menu_default_to_savefile()
{ 
  if( textsw_file_menu != NULL )
	(void) xv_set( (Xv_opaque)textsw_file_menu, MENU_DEFAULT, 2, 0 );
  else
         set_def = TRUE;
}

Pkg_private void
textsw_do_save(abstract, textsw, view)
    Textsw		abstract;
    Textsw_folio	textsw;
    Textsw_view_handle	view;
{
    Frame           base_frame = (Frame) xv_get(abstract, WIN_FRAME);
    Frame           popup      = (Frame) xv_get(base_frame, XV_KEY_DATA,
						    SAVE_FILE_POPUP_KEY);
    CHAR           *name; 
    Xv_Notice	    text_notice;

    if (textsw_has_been_modified(abstract)) {
        Es_handle       original;
        Frame           frame;

        original = (Es_handle) es_get(textsw->views->esh, ES_PS_ORIGINAL);
        if ((TXTSW_IS_READ_ONLY(textsw)) || (original == ES_NULL) ||
            ((Es_enum) es_get(original, ES_TYPE) != ES_TYPE_FILE)) {

            if ((Es_enum) es_get(original, ES_TYPE) != ES_TYPE_FILE) {
                goto final;
            }
                        

            frame = FRAME_FROM_FOLIO_OR_VIEW(view);
            text_notice = (Xv_Notice)xv_get(frame, 
                                XV_KEY_DATA, text_notice_key, 
                                NULL);

            if (!text_notice)  {
                text_notice = xv_create(frame, NOTICE,
                                    NOTICE_LOCK_SCREEN, FALSE,
                                    NOTICE_BLOCK_THREAD, TRUE,
                                    NOTICE_MESSAGE_STRINGS,
                                        XV_MSG("Unable to Save Current File."),
                                    0,
                                    NOTICE_BUTTON_YES, XV_MSG("Continue"),
                                    XV_SHOW, TRUE,
                                    NULL);
                xv_set(frame, 
                    XV_KEY_DATA, text_notice_key, text_notice,
                    NULL);
            }
            else  {
                xv_set(text_notice, 
                        NOTICE_LOCK_SCREEN, FALSE,
                        NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
                            XV_MSG("Unable to Save Current File."),
                        0,
                        NOTICE_BUTTON_YES, XV_MSG("Continue"),
                        XV_SHOW, TRUE, 
                        NULL);
            }
            return;		/* jcb */
        }
    } else {
        Frame           base_frame = FRAME_FROM_FOLIO_OR_VIEW(view);

        text_notice = (Xv_Notice)xv_get(base_frame, 
                            XV_KEY_DATA, text_notice_key, 
                            NULL);

        if (!text_notice)  {
            text_notice = xv_create(base_frame, NOTICE,
                        NOTICE_LOCK_SCREEN, FALSE,
                        NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
                            XV_MSG("File has not been modified.\n\
Save File operation ignored."),
                        0,
                        NOTICE_BUTTON_YES, XV_MSG("Continue"),
                        XV_SHOW, TRUE,
                        NULL);

            xv_set(base_frame, 
                XV_KEY_DATA, text_notice_key, text_notice,
                NULL);
        }
        else  {
            xv_set(text_notice, 
                NOTICE_LOCK_SCREEN, FALSE,
                NOTICE_BLOCK_THREAD, TRUE,
                NOTICE_MESSAGE_STRINGS,
                    XV_MSG("File has not been modified.\n\
Save File operation ignored."),
                0,
                NOTICE_BUTTON_YES, XV_MSG("Continue"),
                XV_SHOW, TRUE, 
                NULL);
        }
        return;
    }

    if (textsw_file_name(FOLIO_FOR_VIEW(view), &name) == 0)  {
	int	confirm_state_changed = 0;

	if (textsw->state & TXTSW_CONFIRM_OVERWRITE) {
	    textsw->state &= ~TXTSW_CONFIRM_OVERWRITE;
	    confirm_state_changed = 1;
	}
#ifdef OW_I18N
        textsw_store_file_wcs(VIEW_REP_TO_ABS(view),name,0,0);
#else
        textsw_store_file(VIEW_REP_TO_ABS(view),name,0,0);
#endif
	if (confirm_state_changed)
	    textsw->state |= TXTSW_CONFIRM_OVERWRITE;
        return;
    }
                   
final: 

    popup = (Frame) xv_get(base_frame, XV_KEY_DATA, SAVE_FILE_POPUP_KEY);
    if (popup){
        (void) textsw_get_and_set_selection(popup, view, (int) TEXTSW_MENU_SAVE);
    }
    else {
        (void) textsw_create_popup_frame(view, (int) TEXTSW_MENU_SAVE);
    }
}
