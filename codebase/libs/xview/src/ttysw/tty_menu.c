#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)tty_menu.c 20.68 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ttysw menu initialization and call-back procedures
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/signal.h>

#include <stdio.h>
#include <ctype.h>

#include <pixrect/pixrect.h>
#include <pixrect/pixfont.h>
#include <xview_private/i18n_impl.h>
#include <xview/win_input.h>
#include <xview/frame.h>
#include <xview/scrollbar.h>
#include <xview/ttysw.h>
#include <xview/termsw.h>
#include <xview/textsw.h>
#include <xview/notice.h>
#include <xview/openmenu.h>
#include <xview/selection.h>
#include <xview/sel_svc.h>
#include <xview/server.h>
#include <xview_private/tty_impl.h>
#include <xview_private/term_impl.h>
#ifdef OW_I18N
#include <xview_private/txt_impl.h>
#endif
#include <xview/svrimage.h>

#define HELP_INFO(s) XV_HELP_DATA, s,

extern int      textsw_file_do_menu_action();

#define EDITABLE		0
#define READ_ONLY		1
#define ENABLE_SCROLL		2
#define DISABLE_SCROLL		3



/* shorthand */
#define	iwbp	ttysw->ttysw_ibuf.cb_wbp
#define	irbp	ttysw->ttysw_ibuf.cb_rbp

/* ttysw walking menu definitions */

static Menu_item ttysw_menu_page_state();
Pkg_private void ttysw_show_walkmenu();

static void     ttysw_enable_scrolling();
static void     ttysw_disable_scrolling();
static void     ttysw_menu_page();
static void     ttysw_menu_copy();
static void     ttysw_menu_paste();
/* static */ int ttysw_enable_editor();
/* static */ int ttysw_disable_editor();
/* static */ int ttysw_mode_action();
/* static */ void
	fit_termsw_panel_and_textsw(); /* BUG ALERT: No XView prefix */



/* termsw walking menu definitions */

int             ITEM_DATA_KEY;

/* ttysw walking menu utilities */


Pkg_private Menu
ttysw_walkmenu(ttysw_folio_public)
    Tty             ttysw_folio_public;
{				/* This create a ttysw menu */
    Menu            ttysw_menu;
    Menu_item       page_mode_item, enable_scroll_item, copy_item, paste_item;

    ttysw_menu = xv_create(XV_SERVER_FROM_WINDOW(ttysw_folio_public), MENU,
			   HELP_INFO("ttysw:menu")
			   NULL);

    page_mode_item = xv_create(XV_ZERO,
			       MENUITEM,
			       MENU_STRING, 
			       XV_MSG("Disable Page Mode"),
			       MENU_ACTION, ttysw_menu_page,
			       MENU_GEN_PROC, ttysw_menu_page_state,
			       MENU_CLIENT_DATA, ttysw_folio_public,
			       HELP_INFO("ttysw:mdsbpage")
			       NULL);


    copy_item = xv_create(XV_ZERO,
			  MENUITEM,
			  MENU_STRING, 
			  XV_MSG("Copy"),
			  MENU_ACTION, ttysw_menu_copy,
			  MENU_CLIENT_DATA, ttysw_folio_public,
			  HELP_INFO("ttysw:mcopy")
			  NULL);

    paste_item = xv_create(XV_ZERO,
			   MENUITEM,
			   MENU_STRING, 
			   XV_MSG("Paste"),
			   MENU_ACTION, ttysw_menu_paste,
			   MENU_CLIENT_DATA, ttysw_folio_public,
			   HELP_INFO("ttysw:mpaste")
			   NULL);



    (void) xv_set(ttysw_menu,
		  MENU_TITLE_ITEM, XV_MSG("Term Pane"),
		  MENU_APPEND_ITEM, page_mode_item,
		  MENU_APPEND_ITEM, copy_item,
		  MENU_APPEND_ITEM, paste_item,
		  NULL);


    if (IS_TERMSW(ttysw_folio_public)) {
	enable_scroll_item = xv_create(XV_ZERO,
				       MENUITEM,
				       MENU_STRING, 
				       XV_MSG("Enable Scrolling"),
				       MENU_ACTION, ttysw_enable_scrolling,
				       MENU_CLIENT_DATA, ttysw_folio_public,
				       HELP_INFO("ttysw:menscroll")
				       NULL);
	(void) xv_set(ttysw_menu,
		      MENU_APPEND_ITEM, enable_scroll_item, NULL);

    }
    return (ttysw_menu);
}


Pkg_private void
ttysw_show_walkmenu(anysw_view_public, event)
    Tty_view        anysw_view_public;
    Event          *event;
{
    register Menu   menu;


    if (IS_TTY_VIEW(anysw_view_public)) {
	menu = (Menu) xv_get(TTY_FROM_TTY_VIEW(anysw_view_public), WIN_MENU);
    } else {
	Ttysw_folio     ttysw = TTY_PRIVATE_FROM_TERMSW_VIEW(anysw_view_public);
	Termsw_folio    termsw = TERMSW_FOLIO_FROM_TERMSW_VIEW(anysw_view_public);

	if (ttysw_getopt(ttysw, TTYOPT_TEXT)) {
	    ttysw->current_view_public = anysw_view_public;
	    menu = termsw->text_menu;
	    xv_set(menu, XV_KEY_DATA, TEXTSW_MENU_DATA_KEY,
		   anysw_view_public, NULL);
	} else if (ttysw->current_view_public == anysw_view_public)
	    menu = termsw->tty_menu;
	else {
	    menu = termsw->text_menu;
	    xv_set(menu, XV_KEY_DATA, TEXTSW_MENU_DATA_KEY,
		   anysw_view_public, NULL);
	}
    }

    if (!menu)
	return;

    /* insure that there are no caret render race conditions */
    termsw_menu_set();
    xv_set(menu, MENU_DONE_PROC, termsw_menu_clr, NULL);

    menu_show(menu, anysw_view_public, event, NULL);
}


/*
 * Menu item gen procs
 */
static          Menu_item
ttysw_menu_page_state(mi, op)
    Menu_item       mi;
    Menu_generate   op;
{
    Tty             ttysw_public;
    Ttysw_folio     ttysw;

    if (op == MENU_DISPLAY_DONE)
	return mi;

    /* Looks like we are trying to gte the value of MENU_CLIENT_DATA; for
     * this we have to use xv_get, not menu_get. [vmh - 7/19/90]
     */
    ttysw_public = (Tty) xv_get(mi, MENU_CLIENT_DATA);
    /* ttysw_public = (Tty) menu_get(mi, MENU_CLIENT_DATA); */
    ttysw = TTY_PRIVATE_FROM_ANY_PUBLIC(ttysw_public);


    if (ttysw->ttysw_flags & TTYSW_FL_FROZEN)
	(void) menu_set(mi, MENU_STRING, XV_MSG("Continue"),
			HELP_INFO("ttysw:mcont")
			NULL);
    else if (ttysw_getopt((caddr_t) ttysw, TTYOPT_PAGEMODE))
	(void) menu_set(mi, MENU_STRING, 
		XV_MSG("Disable Page Mode"),
			HELP_INFO("ttysw:mdsbpage")
			NULL);
    else
	(void) menu_set(mi, MENU_STRING, 
		XV_MSG("Enable Page Mode "),
			HELP_INFO("ttysw:menbpage")
			NULL);
    return mi;
}



/*
 * Callout functions
 */





/* ARGSUSED */
static void
ttysw_menu_page(menu, mi)
    Menu            menu;
    Menu_item       mi;
{
    /* Looks like we are trying to gte the value of MENU_CLIENT_DATA; for
     * this we have to use xv_get, not menu_get. [vmh - 7/19/90]
     */
    Tty             ttysw_public = (Tty) xv_get(mi, MENU_CLIENT_DATA);
    /* Tty             ttysw_public = (Tty) menu_get(mi, MENU_CLIENT_DATA);*/
    Ttysw_folio     ttysw = TTY_PRIVATE_FROM_ANY_PUBLIC(ttysw_public);


    if (ttysw->ttysw_flags & TTYSW_FL_FROZEN)
	(void) ttysw_freeze(ttysw->view, 0);
    else
	(void) ttysw_setopt(TTY_VIEW_HANDLE_FROM_TTY_FOLIO(ttysw), TTYOPT_PAGEMODE,
			    !ttysw_getopt((caddr_t) ttysw, TTYOPT_PAGEMODE));
}

/* ARGSUSED */


static void
ttysw_menu_copy(menu, mi)
    Menu            menu;
    Menu_item       mi;
{
    /* Looks like we are trying to gte the value of MENU_CLIENT_DATA; for
     * this we have to use xv_get, not menu_get. [vmh - 7/19/90]
     */
     Tty             ttysw_public = (Tty) xv_get(mi, MENU_CLIENT_DATA);
    /*Tty             ttysw_public = (Tty) menu_get(mi, MENU_CLIENT_DATA);*/
    Ttysw_folio     ttysw = TTY_PRIVATE_FROM_ANY_PUBLIC(ttysw_public);
    Xv_Notice	tty_notice;


    if (!ttysw_do_copy(ttysw)) {
	Frame           frame = xv_get(ttysw_public, WIN_FRAME);
	tty_notice = xv_get(frame, XV_KEY_DATA, tty_notice_key, NULL);

	if (!tty_notice)  {
    	    tty_notice = xv_create(frame, NOTICE,
			  NOTICE_LOCK_SCREEN, FALSE,
			  NOTICE_BLOCK_THREAD, TRUE,
		          NOTICE_BUTTON_YES, 
			  XV_MSG("Continue"),
		          NOTICE_MESSAGE_STRINGS,
		      XV_MSG("Please make a primary selection first."),
		              0,
			  XV_SHOW, TRUE,
		          NULL);

	    xv_set(frame, 
		XV_KEY_DATA, tty_notice_key, tty_notice, 
		NULL);

        }
	else  {
    	    xv_set(tty_notice, 
			NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
		        NOTICE_BUTTON_YES, 
			XV_MSG("Continue"),
		        NOTICE_MESSAGE_STRINGS,
		      XV_MSG("Please make a primary selection first."),
		        0,
			XV_SHOW, TRUE, 
			NULL);
	}
    }
}

/*ARGSUSED*/
static void
ttysw_menu_paste(menu, mi)
    Menu            menu;
    Menu_item       mi;
{
    /* Looks like we are trying to gte the value of MENU_CLIENT_DATA; for
     * this we have to use xv_get, not menu_get. [vmh - 7/19/90] 
     */ 
     Tty             ttysw_public = (Tty) xv_get(mi, MENU_CLIENT_DATA);
    /*Tty             ttysw_public = (Tty) menu_get(mi, MENU_CLIENT_DATA);*/
    Ttysw_folio     ttysw = TTY_PRIVATE_FROM_ANY_PUBLIC(ttysw_public);
    Xv_Notice	tty_notice;

#ifdef OW_I18N
    ttysw_implicit_commit(ttysw, 1);
#endif
    if (!ttysw_do_paste(ttysw)) {
	Frame           frame = xv_get(ttysw_public, WIN_FRAME);
	tty_notice = xv_get(frame, XV_KEY_DATA, tty_notice_key, NULL);
	if (!tty_notice)  {
    	    tty_notice = xv_create(frame, NOTICE,
			  NOTICE_LOCK_SCREEN, FALSE,
			  NOTICE_BLOCK_THREAD, TRUE,
		          NOTICE_BUTTON_YES, 
			  XV_MSG("Continue"),
		          NOTICE_MESSAGE_STRINGS,
		      XV_MSG("Please Copy text onto clipboard first."),
		              0,
			  XV_SHOW, TRUE,
		          NULL);

	    xv_set(frame, 
		XV_KEY_DATA, tty_notice_key, tty_notice, 
		NULL);

        }
	else  {
    	    xv_set(tty_notice, 
			NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
		        NOTICE_BUTTON_YES, 
			XV_MSG("Continue"),
			NOTICE_MESSAGE_STRINGS,
		      XV_MSG("Please Copy text onto clipboard first."),
			0,
			XV_SHOW, TRUE, 
			NULL);
	}

    }
}

/* termsw walking menu definitions */
Pkg_private void
ttysw_set_menu(termsw_public)
    Termsw          termsw_public;
{
    Menu            history_menu, scroll_menu, editor_menu, mode_menu;
    Menu_item       edit_item, find_item, extras_item, history_item, mode_item,
                    scroll_item, editor_item;
    Menu_item       editiable_item, readonly_item, enable_editor_item, disable_editor_item,
                    enable_scroll_item, disable_scroll_item, store_item, clear_item;
    Xv_Server	    server;
    Termsw_folio    termsw_folio = TERMSW_PRIVATE(termsw_public);
    Textsw          textsw = termsw_public;


    server = XV_SERVER_FROM_WINDOW(termsw_public);
    termsw_folio->text_menu = xv_create(server, MENU,
					HELP_INFO("ttysw:mterms")
					NULL);

    /* History sub menu */
    history_menu = xv_create(server, MENU,
			     MENU_CLIENT_DATA, textsw,
			     HELP_INFO("ttysw:mhistory")
			     NULL);
    mode_menu = xv_create(server, MENU_CHOICE_MENU,
			  HELP_INFO("ttysw:mmode")
			  NULL);

    editiable_item = xv_create(XV_ZERO,
			       MENUITEM,
			       MENU_STRING, 
			       XV_MSG("Editable"),
			       MENU_VALUE, EDITABLE,
			       MENU_ACTION, ttysw_mode_action,
			       MENU_CLIENT_DATA, textsw,
			       HELP_INFO("ttysw:mmode")
			       NULL);
    readonly_item = xv_create(XV_ZERO,
			      MENUITEM,
			      MENU_STRING, 
			      XV_MSG("Read Only"),
			      MENU_VALUE, READ_ONLY,
			      MENU_ACTION, ttysw_mode_action,
			      MENU_CLIENT_DATA, textsw,
			      HELP_INFO("ttysw:mmode")
			      NULL);

    xv_set(mode_menu,
	   MENU_APPEND_ITEM, editiable_item,
	   MENU_APPEND_ITEM, readonly_item,
	   MENU_DEFAULT_ITEM, readonly_item,
	   NULL);

    mode_item = (Menu_item) xv_create(XV_ZERO,
				      MENUITEM,
				      MENU_STRING, 
				      XV_MSG("Mode"),
				      MENU_PULLRIGHT, mode_menu,
				      HELP_INFO("ttysw:mmode")
				      NULL);

    store_item = xv_create(XV_ZERO,
			   MENUITEM,
			   MENU_STRING, 
			   XV_MSG("Store log as new file "),
			   MENU_ACTION, textsw_file_do_menu_action,
			   MENU_VALUE, TEXTSW_MENU_STORE,
			   MENU_CLIENT_DATA, textsw,
			   HELP_INFO("textsw:mstorelog")
			   NULL);
    clear_item = xv_create(XV_ZERO,
			   MENUITEM,
			   MENU_STRING, 
			   XV_MSG("Clear log"),
			   MENU_ACTION, textsw_file_do_menu_action,
			   MENU_VALUE, TEXTSW_MENU_RESET,
			   MENU_CLIENT_DATA, textsw,
			   HELP_INFO("textsw:mclearlog")
			   NULL);

    xv_set(history_menu,
	   MENU_APPEND_ITEM, mode_item,
	   MENU_APPEND_ITEM, store_item,
	   MENU_APPEND_ITEM, clear_item,
	   NULL);

    history_item = (Menu_item) xv_create(XV_ZERO,
					 MENUITEM,
					 MENU_STRING, 
					 XV_MSG("History"),
					 MENU_PULLRIGHT, history_menu,
					 HELP_INFO("ttysw:mhistory")
					 NULL);

    edit_item = (Menu) xv_create(XV_ZERO,
				 MENUITEM,
				 MENU_STRING, XV_MSG("Edit"),
		 MENU_PULLRIGHT, xv_get(termsw_public, TEXTSW_SUBMENU_EDIT),
				 HELP_INFO("ttysw:medit")
				 NULL);

    find_item = (Menu) xv_create(XV_ZERO,
				 MENUITEM,
				 MENU_STRING, XV_MSG("Find"),
		 MENU_PULLRIGHT, xv_get(termsw_public, TEXTSW_SUBMENU_FIND),
				 HELP_INFO("ttysw:mfind")
				 NULL);

    extras_item = (Menu) xv_create(XV_ZERO,
				   MENUITEM,
				   MENU_STRING, 
				   XV_MSG("Extras"),
	      MENU_PULLRIGHT, xv_get(termsw_public, TEXTSW_EXTRAS_CMD_MENU),
				   HELP_INFO("ttysw:mcommands")
				   NULL);

    /* Editor sub menu */
    editor_menu = xv_create(server, MENU_CHOICE_MENU,
			    HELP_INFO("textsw:meditor")
			    NULL);

    enable_editor_item = xv_create(XV_ZERO,
				   MENUITEM,
				   MENU_STRING, 
				   XV_MSG("Enable"),
				   MENU_ACTION, ttysw_enable_editor,
				   MENU_CLIENT_DATA, textsw,
				   HELP_INFO("textsw:meneditor")
				   NULL);
    disable_editor_item = xv_create(XV_ZERO,
				    MENUITEM,
				    MENU_STRING, 
				    XV_MSG("Disable"),
				    MENU_CLIENT_DATA, textsw,
				    MENU_ACTION, ttysw_disable_editor,
				    HELP_INFO("textsw:mdiseditor")
				    NULL);

    xv_set(editor_menu,
	   MENU_APPEND_ITEM, enable_editor_item,
	   MENU_APPEND_ITEM, disable_editor_item,
	   MENU_DEFAULT_ITEM, enable_editor_item,
	   NULL);
    editor_item = (Menu) xv_create(XV_ZERO,
				   MENUITEM,
				   MENU_STRING, 
				   XV_MSG("File Editor"),
				   MENU_PULLRIGHT, editor_menu,
				   HELP_INFO("ttysw:meditor")
				   NULL);

    /* Scrolling sub menu */
    scroll_menu = xv_create(server, MENU_CHOICE_MENU,
			    HELP_INFO("textsw:mscroll")
			    NULL);

    enable_scroll_item = xv_create(XV_ZERO,
				   MENUITEM,
				   MENU_STRING, 
				   XV_MSG("Enable Scrolling"),
				   MENU_VALUE, ENABLE_SCROLL,
				   MENU_CLIENT_DATA, textsw,
				   HELP_INFO("textsw:menscroll")
				   NULL);
    disable_scroll_item = xv_create(XV_ZERO,
				    MENUITEM,
				    MENU_STRING, 
				    XV_MSG("Disable Scrolling"),
				    MENU_VALUE, DISABLE_SCROLL,
				    MENU_ACTION, ttysw_disable_scrolling,
				    MENU_CLIENT_DATA, textsw,
				    HELP_INFO("textsw:mdisscroll")
				    NULL);

    xv_set(scroll_menu,
	   MENU_APPEND_ITEM, enable_scroll_item,
	   MENU_APPEND_ITEM, disable_scroll_item,
	   HELP_INFO("textsw:mscroll")
	   NULL);
    scroll_item = (Menu) xv_create(XV_ZERO,
				   MENUITEM,
				   MENU_STRING, 
				   XV_MSG("Scrolling"),
				   MENU_PULLRIGHT, scroll_menu,
				   HELP_INFO("ttysw:mscroll")
				   NULL);

    (void) xv_set(termsw_folio->text_menu,
		  MENU_TITLE_ITEM, XV_MSG("Term Pane"),
		  MENU_APPEND_ITEM, history_item,
		  MENU_APPEND_ITEM, edit_item,
		  MENU_APPEND_ITEM, find_item,
		  MENU_APPEND_ITEM, extras_item,
		  MENU_APPEND_ITEM, editor_item,
		  MENU_APPEND_ITEM, scroll_item,
		  NULL);
}

/*ARGSUSED*/
static void
panel_button_proc(item, event)
    Panel_item     *item;
    Event          *event;
{
    Textsw textsw = (Textsw)xv_get((Xv_opaque)item, XV_KEY_DATA, ITEM_DATA_KEY);
    Menu            menu = (Menu) xv_get((Xv_opaque)item, PANEL_ITEM_MENU);
    Panel           p_menu = (Panel) xv_get(menu, MENU_PIN_WINDOW);
    Menu_item       menu_item;
    Menu            pullr_menu;
    int             num_items, i;

    xv_set(menu, XV_KEY_DATA, TEXTSW_MENU_DATA_KEY,
	   textsw, NULL);
    if (p_menu) {
	num_items = (int) xv_get(menu, MENU_NITEMS);
	for (i = 1; i <= num_items; i++) {
	    menu_item = (Menu_item) xv_get(menu, MENU_NTH_ITEM, i);
	    if (menu_item) {
		pullr_menu = (Menu) xv_get(menu_item, MENU_PULLRIGHT);
		if (pullr_menu)
		    xv_set(pullr_menu, XV_KEY_DATA, TEXTSW_MENU_DATA_KEY,
			   textsw, NULL);
	    }
	}
    }
}

static void
create_textedit_panel_item(panel, textsw)
    Panel           panel;
    Textsw          textsw;
{
    Panel_item      file_panel_item, edit_panel_item, display_panel_item, find_panel_item;

    if (!ITEM_DATA_KEY)
		ITEM_DATA_KEY = xv_unique_key();

    file_panel_item = xv_create(panel, PANEL_BUTTON,
				PANEL_LABEL_STRING, 
				XV_MSG("File"),
				PANEL_NOTIFY_PROC, panel_button_proc,
		PANEL_ITEM_MENU, (Menu) xv_get(textsw, TEXTSW_SUBMENU_FILE),
				NULL);
    display_panel_item = xv_create(panel, PANEL_BUTTON,
				   PANEL_LABEL_STRING, 
				   XV_MSG("View"),
				   PANEL_NOTIFY_PROC, panel_button_proc,
		PANEL_ITEM_MENU, (Menu) xv_get(textsw, TEXTSW_SUBMENU_VIEW),
				   NULL);

    edit_panel_item = xv_create(panel, PANEL_BUTTON,
				PANEL_LABEL_STRING, 
				XV_MSG("Edit"),
				PANEL_NOTIFY_PROC, panel_button_proc,
		PANEL_ITEM_MENU, (Menu) xv_get(textsw, TEXTSW_SUBMENU_EDIT),
				NULL);


    find_panel_item = xv_create(panel, PANEL_BUTTON,
				PANEL_LABEL_STRING, 
				XV_MSG("Find"),
				PANEL_NOTIFY_PROC, panel_button_proc,
		PANEL_ITEM_MENU, (Menu) xv_get(textsw, TEXTSW_SUBMENU_FIND),
				NULL);

    xv_set(file_panel_item, XV_KEY_DATA, ITEM_DATA_KEY,
	   textsw, NULL);
    xv_set(display_panel_item, XV_KEY_DATA, ITEM_DATA_KEY,
	   textsw, NULL);
    xv_set(edit_panel_item, XV_KEY_DATA, ITEM_DATA_KEY,
	   textsw, NULL);
    xv_set(find_panel_item, XV_KEY_DATA, ITEM_DATA_KEY,
	   textsw, NULL);

    window_fit_height(panel);

}



/* BUG ALERT: No XView prefix */
/* static */ void
fit_termsw_panel_and_textsw(frame, termsw_folio)
    Frame           frame;
    register Termsw_folio termsw_folio;

{
    Rect            rect, panel_rect, textsw_rect;
    int             termsw_height;

    rect = *((Rect *) xv_get(TERMSW_PUBLIC(termsw_folio), WIN_RECT));

    termsw_height = (rect.r_height / 3);
    xv_set(TERMSW_PUBLIC(termsw_folio), XV_HEIGHT, termsw_height, NULL);

    panel_rect = *((Rect *) xv_get(termsw_folio->textedit_panel, WIN_RECT));
    panel_rect.r_left = rect.r_left;
    panel_rect.r_top = rect.r_top + termsw_height + 1;
    panel_rect.r_width = rect.r_width;

    xv_set(termsw_folio->textedit_panel, WIN_RECT, &panel_rect,
	   XV_SHOW, TRUE, NULL);

    textsw_rect.r_left = panel_rect.r_left;
    textsw_rect.r_top = panel_rect.r_top + panel_rect.r_height + 1;
    textsw_rect.r_width = panel_rect.r_width;
    if ((textsw_rect.r_height =
                rect.r_height - (panel_rect.r_top + panel_rect.r_height)) <= 0)
        textsw_rect.r_height = 1;


    xv_set(termsw_folio->textedit, WIN_RECT, &textsw_rect,
	   XV_SHOW, TRUE, NULL);

    window_fit(frame);

}

/*ARGSUSED*/
/* static */ int
ttysw_enable_editor(cmd_menu, cmd_item)
    Menu            cmd_menu;
    Menu_item       cmd_item;
{
    Textsw      textsw = (Textsw) (xv_get(cmd_item, MENU_CLIENT_DATA));
    /*Textsw    textsw = (Textsw) (menu_get(cmd_item, MENU_CLIENT_DATA));*/
    Frame           frame = (Frame) xv_get(textsw, WIN_FRAME);
    register        Termsw_folio
                    termsw_folio = TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));
    Xv_opaque       my_font = xv_get(textsw, WIN_FONT);
    Xv_Notice	tty_notice;


    if (termsw_folio->first_view->next) {
	tty_notice = xv_get(frame, XV_KEY_DATA, tty_notice_key, NULL);
	if (!tty_notice)  {
    	    tty_notice = xv_create(frame, NOTICE,
			  NOTICE_LOCK_SCREEN, FALSE,
			  NOTICE_BLOCK_THREAD, TRUE,
		          NOTICE_BUTTON_YES, 
			  XV_MSG("Continue"),
		          NOTICE_MESSAGE_STRINGS,
			  XV_MSG("Please destroy all split views before enabling File Editor.\n\
Press \"Continue\" to proceed."),
		              0,
			  XV_SHOW, TRUE,
		          NULL);

	    xv_set(frame, 
		XV_KEY_DATA, tty_notice_key, tty_notice, 
		NULL);

        }
	else  {
    	    xv_set(tty_notice, 
			NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
		        NOTICE_BUTTON_YES, 
			XV_MSG("Continue"),
			NOTICE_MESSAGE_STRINGS,
			XV_MSG("Please destroy all split views before enabling File Editor.\n\
Press \"Continue\" to proceed."),
			0,
			XV_SHOW, TRUE, 
			NULL);
	}
	return 0;
    }
    if (!termsw_folio->textedit) {
	termsw_folio->textedit_panel = xv_create(frame, PANEL,
				     WIN_BELOW, TERMSW_PUBLIC(termsw_folio),
					     PANEL_LAYOUT, PANEL_HORIZONTAL,
						 XV_SHOW, FALSE,
				    XV_WIDTH, (int) xv_get(frame, XV_WIDTH),
						 NULL);

	termsw_folio->textedit = xv_create(frame, TEXTSW,
					   WIN_FONT, my_font,
				    WIN_BELOW, termsw_folio->textedit_panel,
					   XV_SHOW, FALSE,
					   NULL);

	(void) create_textedit_panel_item(termsw_folio->textedit_panel, termsw_folio->textedit);

    }
    if ((int) xv_get(termsw_folio->textedit, XV_SHOW)) {
	tty_notice = xv_get(frame, XV_KEY_DATA, tty_notice_key, NULL);
	if (!tty_notice)  {
    	    tty_notice = xv_create(frame, NOTICE,
			  NOTICE_LOCK_SCREEN, FALSE,
			  NOTICE_BLOCK_THREAD, TRUE,
		          NOTICE_BUTTON_YES, 
			  XV_MSG("Continue"),
		          NOTICE_MESSAGE_STRINGS,
			  XV_MSG("Textedit is already created.\n\
Press \"Continue\" to proceed."),
		              0,
			  XV_SHOW, TRUE,
		          NULL);

	    xv_set(frame, 
		XV_KEY_DATA, tty_notice_key, tty_notice, 
		NULL);

        }
	else  {
    	    xv_set(tty_notice, 
			NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
		        NOTICE_BUTTON_YES, 
			XV_MSG("Continue"),
			NOTICE_MESSAGE_STRINGS,
			XV_MSG("Textedit is already created.\n\
Press \"Continue\" to proceed."),
			0,
			XV_SHOW, TRUE, 
			NULL);
	}
	return 0;
    }
    (void) fit_termsw_panel_and_textsw(frame, termsw_folio);

    /* Change default to "Disable editor" */
    xv_set(cmd_item, MENU_SELECTED, FALSE, NULL);
    xv_set(cmd_menu, MENU_DEFAULT, 2, NULL);
}


/* ARGSUSED */
/* static */ int
ttysw_disable_editor(cmd_menu, cmd_item)
    Menu            cmd_menu;
    Menu_item       cmd_item;
{
    Textsw      textsw = (Textsw) (xv_get(cmd_item, MENU_CLIENT_DATA));
    /*Textsw  textsw = (Textsw) (menu_get(cmd_item, MENU_CLIENT_DATA)); */
    Frame           frame = (Frame) xv_get(textsw, WIN_FRAME);
    register        Termsw_folio
                    termsw_folio = TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));
    Event           ie;
    extern int      win_getmouseposition();
    extern int      textsw_empty_document();
    Rect            rect;
    Xv_Notice	tty_notice;

    if ((!termsw_folio->textedit) ||
	(!(int) xv_get(termsw_folio->textedit, XV_SHOW))) {
	tty_notice = xv_get(frame, XV_KEY_DATA, tty_notice_key, NULL);
	if (!tty_notice)  {
    	    tty_notice = xv_create(frame, NOTICE,
			  NOTICE_LOCK_SCREEN, FALSE,
			  NOTICE_BLOCK_THREAD, TRUE,
		          NOTICE_BUTTON_YES, 
			  XV_MSG("Continue"),
		          NOTICE_MESSAGE_STRINGS,
			  XV_MSG("No textedit is enabled yet.\n\
Press \"Continue\" to proceed."),
		              0,
			  XV_SHOW, TRUE,
		          NULL);

	    xv_set(frame, 
		XV_KEY_DATA, tty_notice_key, tty_notice, 
		NULL);

        }
	else  {
    	    xv_set(tty_notice, 
			NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
		        NOTICE_BUTTON_YES, 
			XV_MSG("Continue"),
			NOTICE_MESSAGE_STRINGS,
			XV_MSG("No textedit is enabled yet.\n\
Press \"Continue\" to proceed."),
			0,
			XV_SHOW, TRUE, 
			NULL);
	}
	return 0;
    }
    (void) win_getmouseposition(termsw_folio->textedit, &ie.ie_locx, &ie.ie_locy);
    if (textsw_empty_document(termsw_folio->textedit, &ie) == XV_ERROR)
	return 0;

    /* Change default to "Enable editor" */
    xv_set(cmd_item, MENU_SELECTED, FALSE, NULL);
    xv_set(cmd_menu, MENU_DEFAULT, 1, NULL);

    rect = *((Rect *) xv_get(termsw_folio->textedit, WIN_RECT));


    xv_set(termsw_folio->textedit, XV_SHOW, FALSE, NULL);
    xv_set(termsw_folio->textedit_panel, XV_SHOW, FALSE, NULL);

    xv_set(TERMSW_PUBLIC(termsw_folio),
	   XV_HEIGHT, rect.r_top + rect.r_height - 1,
	   XV_WIDTH, rect.r_width, NULL);
    window_fit(frame);


}

/* ARGSUSED */
/* static */ int
ttysw_mode_action(cmd_menu, cmd_item)
    Menu            cmd_menu;
    Menu_item       cmd_item;
{
    Textsw      textsw = (Textsw) (xv_get(cmd_item, MENU_CLIENT_DATA));
    /*Textsw    textsw = (Textsw) (menu_get(cmd_item, MENU_CLIENT_DATA));*/
    register        Termsw_folio
                    termsw = TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));
    int         value = (int) (xv_get(cmd_item, MENU_VALUE, NULL));
    /*int       value = (int) menu_get(cmd_item, MENU_VALUE, 0);*/

    Textsw_index    tmp_index, insert;

    if ((value == READ_ONLY) && !termsw->append_only_log) {
	tmp_index = (int) textsw_find_mark_i18n(textsw, termsw->pty_mark);
	insert = (Textsw_index) xv_get(textsw, TEXTSW_INSERTION_POINT_I18N);
	if (insert != tmp_index) {
	    (void) xv_set(textsw, TEXTSW_INSERTION_POINT_I18N, tmp_index, NULL);
	}
	termsw->read_only_mark =
	    textsw_add_mark_i18n(textsw,
		      termsw->cooked_echo ? tmp_index : TEXTSW_INFINITY - 1,
			    TEXTSW_MARK_READ_ONLY);
	termsw->append_only_log = TRUE;
    } else if ((value == EDITABLE) && termsw->append_only_log) {
	textsw_remove_mark(textsw, termsw->read_only_mark);
	termsw->append_only_log = FALSE;
    }
}

/*ARGSUSED*/
static void
ttysw_enable_scrolling(menu, mi)
    Menu            menu;
    Menu_item       mi;
/*
 * This routine should only be invoked from the item added to the ttysw menu
 * when sw is created as a termsw.  It relies on the menu argument being the
 * handle for the ttysw menu.
 */
{
    /* The textsw handle is really a termsw handle */
    Textsw          textsw = (Textsw) (xv_get(mi, MENU_CLIENT_DATA));
    /*Textsw        textsw = (Textsw) (menu_get(mi, MENU_CLIENT_DATA));*/
    Ttysw_view_handle ttysw_view_handle = TTY_VIEW_PRIVATE_FROM_ANY_PUBLIC(textsw);
    Termsw_folio    termsw_folio = TERMSW_PRIVATE((Termsw) textsw);
    Xv_Notice	tty_notice;

#ifdef OW_I18N
    Ttysw_folio     ttysw_folio = TTY_PRIVATE_FROM_ANY_PUBLIC(textsw);
#endif

    if (termsw_folio->ok_to_enable_scroll) {
#ifdef OW_I18N
	ttysw_implicit_commit(ttysw_folio, 0);
#endif
	ttysw_setopt(ttysw_view_handle, TTYOPT_TEXT, 1);
    } else {
	Frame           frame = xv_get(textsw, WIN_FRAME);

	tty_notice = xv_get(frame, XV_KEY_DATA, tty_notice_key, NULL);

	if (!tty_notice)  {
    	    tty_notice = xv_create(frame, NOTICE,
			  NOTICE_LOCK_SCREEN, FALSE,
			  NOTICE_BLOCK_THREAD, TRUE,
		          NOTICE_BUTTON_YES, 
			  XV_MSG("Continue"),
		          NOTICE_MESSAGE_STRINGS,
		          XV_MSG("Cannot enable scrolling while this application is running."),
		              0,
			  XV_SHOW, TRUE,
		          NULL);

	    xv_set(frame, 
		XV_KEY_DATA, tty_notice_key, tty_notice, 
		NULL);

        }
	else  {
    	    xv_set(tty_notice, 
			NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
		        NOTICE_BUTTON_YES, 
			XV_MSG("Continue"),
		        NOTICE_MESSAGE_STRINGS,
		        XV_MSG("Cannot enable scrolling while this application is running."),
		        0,
			XV_SHOW, TRUE, 
			NULL);
	}
    }
}

/* ARGSUSED */
static void
ttysw_disable_scrolling(cmd_menu, cmd_item)
    Menu            cmd_menu;
    Menu_item       cmd_item;
{
    /* The textsw handle is really a termsw handle */
    Textsw      textsw = (Textsw) (xv_get(cmd_item, MENU_CLIENT_DATA));
    /*Textsw    textsw = (Textsw) (menu_get(cmd_item, MENU_CLIENT_DATA));*/
    Ttysw_view_handle ttysw_view = TTY_VIEW_PRIVATE_FROM_ANY_PUBLIC(textsw);
    Ttysw_folio     ttysw_folio = TTY_FOLIO_FROM_TTY_VIEW_HANDLE(ttysw_view);
    Xv_Notice	tty_notice;

    if (ttysw_getopt(ttysw_folio, TTYOPT_TEXT)) {
#ifdef OW_I18N
	textsw_implicit_commit(TEXTSW_PRIVATE(textsw));
#endif
	ttysw_setopt(ttysw_view, TTYOPT_TEXT, NULL);
    } else {
	Frame           frame = xv_get(textsw, WIN_FRAME);

	tty_notice = xv_get(frame, XV_KEY_DATA, tty_notice_key, NULL);

	if (!tty_notice)  {
    	    tty_notice = xv_create(frame, NOTICE,
			  NOTICE_LOCK_SCREEN, FALSE,
			  NOTICE_BLOCK_THREAD, TRUE,
		          NOTICE_BUTTON_YES, 
			  XV_MSG("Continue"),
		          NOTICE_MESSAGE_STRINGS,
		    XV_MSG("Only one termsw view can turn into a ttysw at a time."),
		              0,
			  XV_SHOW, TRUE,
		          NULL);

	    xv_set(frame, 
		XV_KEY_DATA, tty_notice_key, tty_notice, 
		NULL);

        }
	else  {
    	    xv_set(tty_notice, 
			NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
		        NOTICE_BUTTON_YES, 
			XV_MSG("Continue"),
		        NOTICE_MESSAGE_STRINGS,
		    XV_MSG("Only one termsw view can turn into a ttysw at a time."),
		        0,
			XV_SHOW, TRUE, 
			NULL);
	}
    }
    xv_set(cmd_menu, MENU_DEFAULT, 1, NULL);
}
