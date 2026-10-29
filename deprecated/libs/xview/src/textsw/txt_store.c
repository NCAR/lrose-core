#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_store.c 1.31 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Text store popup frame creation and support.
 */

#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <xview_private/ev_impl.h>
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

#ifdef SVR4
#include <unistd.h>
#include <string.h>
#endif /* SVR4 */
 
#define		MAX_DISPLAY_LENGTH	50
#define   	MAX_STR_LENGTH		1024

#define HELP_INFO(s) XV_HELP_DATA, s,

typedef enum {
    FILE_CMD_ITEM = 0,
    DIR_STRING_ITEM = 1,
    FILE_STRING_ITEM = 2,
}               File_panel_item_enum;

Pkg_private Panel_item store_panel_items[];

Pkg_private Textsw_view_handle text_view_frm_p_itm();
Pkg_private Xv_Window frame_from_panel_item();

static int
do_store_proc(folio, ie)
    Textsw_folio    folio;
    Event          *ie;
{
    CHAR           *dir_str, *file_str;
    register int    locx, locy;
    Frame           popup_frame;
    Frame           frame;
    Xv_Notice	    text_notice;
    char            curr_dir[MAX_STR_LENGTH];
#ifdef OW_I18N
    CHAR            curr_dir_ws[MAX_STR_LENGTH];
#endif

#ifdef OW_I18N
    dir_str = (CHAR *) xv_get(store_panel_items[(int)DIR_STRING_ITEM],
			      PANEL_VALUE_WCS);
    file_str = (CHAR *) xv_get(store_panel_items[(int)FILE_STRING_ITEM],
			       PANEL_VALUE_WCS);
#else
    dir_str = (char *) xv_get(store_panel_items[(int)DIR_STRING_ITEM],
			      PANEL_VALUE);
    file_str = (char *) xv_get(store_panel_items[(int)FILE_STRING_ITEM],
			       PANEL_VALUE);
#endif /* OW_I18N */

    if AN_ERROR
	(ie == 0)
	    locx = locy = 0;
    else {
	locx = ie->ie_locx;
	locy = ie->ie_locy;
    }
#ifdef OW_I18N
    if (textsw_expand_filename(folio, dir_str, MAX_STR_LENGTH, locx, locy)) {
	/* error handled inside routine */
	return TRUE;
    }
    if (textsw_expand_filename(folio, file_str, MAX_STR_LENGTH, locx, locy)) {
	/* error handled inside routine */
	return TRUE;
    }
#else /* OW_I18N */
    if (textsw_expand_filename(folio, dir_str, locx, locy)) {
	/* error handled inside routine */
	return TRUE;
    }
    if (textsw_expand_filename(folio, file_str, locx, locy)) {
	/* error handled inside routine */
	return TRUE;
    }
#endif /* OW_I18N */

    /* if "cd" is not disabled and the "cd" dir is not the current dir */
    (void) getcwd(curr_dir, MAX_STR_LENGTH);
#ifdef OW_I18N
    (void) mbstowcs(curr_dir_ws, curr_dir, MAX_STR_LENGTH);
    if (STRCMP(curr_dir_ws, dir_str) != 0) {	/* } for match */
#else
    if (strcmp(curr_dir, dir_str) != 0) {
#endif /* OW_I18N */
	if (!(folio->state & TXTSW_NO_CD)) {
	    if (textsw_change_directory(folio, dir_str, FALSE, locx, locy) != 0) {
		return TRUE;
	    }
	} else {
	    frame = FRAME_FROM_FOLIO_OR_VIEW(folio);
            text_notice = (Xv_Notice)xv_get(frame, 
			    XV_KEY_DATA, text_notice_key, NULL);
            if (!text_notice)  {
                text_notice = xv_create(frame, NOTICE,
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("Cannot change directory.\n\
Change Directory Has Been Disabled."),
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
		    XV_MSG("Cannot change directory.\n\
Change Directory Has Been Disabled."),
                    0,
                    NOTICE_BUTTON_YES, XV_MSG("Continue"),
                    XV_SHOW, TRUE, 
                    NULL);
            }
	    return TRUE;
	}
    }
    if ((int)STRLEN(file_str) > 0) {
#ifdef OW_I18N
	if (textsw_store_file_wcs(FOLIO_REP_TO_ABS(folio), file_str, locx, locy) == 0) {
#else
	if (textsw_store_file(FOLIO_REP_TO_ABS(folio), file_str, locx, locy) == 0) {
#endif /* OW_I18N */
	    popup_frame =
		frame_from_panel_item(store_panel_items[(int) FILE_CMD_ITEM]);
	    (void) xv_set(popup_frame, XV_SHOW, FALSE, NULL);
	    return FALSE;
	}
	return TRUE;
    }
    frame = FRAME_FROM_FOLIO_OR_VIEW(folio);
    text_notice = (Xv_Notice)xv_get(frame, 
                                XV_KEY_DATA, text_notice_key, 
				NULL);
    if (!text_notice)  {
        text_notice = xv_create(frame, NOTICE,
                            NOTICE_LOCK_SCREEN, FALSE,
			    NOTICE_BLOCK_THREAD, TRUE,
                            NOTICE_MESSAGE_STRINGS,
			    XV_MSG("No file name was specified.\n\
Specify a file name to store as new file."),
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
		    XV_MSG("No file name was specified.\n\
Specify a file name to store as new file."),
                    0,
                    NOTICE_BUTTON_YES, XV_MSG("Continue"),
                    XV_SHOW, TRUE, 
                    NULL);
    }

    /* if we mad it here then there was an error or notice */
    return TRUE;
}

static          Panel_setting
store_cmd_proc(item, event)
    Panel_item      item;
    Event          *event;
{
    Textsw_view_handle view = text_view_frm_p_itm(item);
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    int             error;

    if (item == store_panel_items[(int) FILE_CMD_ITEM]) {
	error = do_store_proc(folio, event);
	if (error) {
	    xv_set(item, PANEL_NOTIFY_STATUS, XV_ERROR, NULL);
	}
	return PANEL_NONE;
    }
    return PANEL_NEXT;
}

static          Panel_setting
store_cmd_proc_accel(item, event)
    Panel_item      item;
    Event          *event;
{
    Textsw_view_handle view = text_view_frm_p_itm(item);
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    int             error;

    if (item == store_panel_items[(int) FILE_STRING_ITEM]) {
	error = do_store_proc(folio, event);
	if (error) {
	    xv_set(item, PANEL_NOTIFY_STATUS, XV_ERROR, NULL);
	}
	return PANEL_NONE;
    }
    return PANEL_NEXT;
}

/* This creates all of the panel_items */
static void
create_store_items(panel, view)
    Panel           panel;
    Textsw_view_handle view;
{

    CHAR            store_string[MAX_STR_LENGTH];
    char            current_dir_store_string[MAX_STR_LENGTH];
    Es_index        dummy;


    store_string[0] = XV_ZERO;
    (void) textsw_get_selection(view, &dummy, &dummy, store_string, MAX_STR_LENGTH);
    (void) getcwd(current_dir_store_string, MAX_STR_LENGTH);
    store_panel_items[(int) DIR_STRING_ITEM] = panel_create_item(panel, PANEL_TEXT,
						 PANEL_LABEL_X, ATTR_COL(0),
						 PANEL_LABEL_Y, ATTR_ROW(0),
			     PANEL_VALUE_DISPLAY_LENGTH, MAX_DISPLAY_LENGTH,
				  PANEL_VALUE_STORED_LENGTH, MAX_STR_LENGTH,
				      PANEL_VALUE, current_dir_store_string,
					   PANEL_LABEL_STRING, 
					   XV_MSG("Directory:"),
					       HELP_INFO("textsw:dirstring")
								 NULL);
    store_panel_items[(int) FILE_STRING_ITEM] = panel_create_item(panel, PANEL_TEXT,
						 PANEL_LABEL_X, ATTR_COL(5),
						 PANEL_LABEL_Y, ATTR_ROW(1),
			     PANEL_VALUE_DISPLAY_LENGTH, MAX_DISPLAY_LENGTH,
				  PANEL_VALUE_STORED_LENGTH, MAX_STR_LENGTH,
#ifdef OW_I18N
						  PANEL_VALUE_WCS, store_string,
#else
						  PANEL_VALUE, store_string,
#endif /* OW_I18N */
						PANEL_LABEL_STRING, 
						XV_MSG("File:"),
					PANEL_NOTIFY_LEVEL, PANEL_SPECIFIED,
						PANEL_NOTIFY_STRING, "\n\r",
				    PANEL_NOTIFY_PROC, store_cmd_proc_accel,
					     HELP_INFO("textsw:storestring")
								  NULL);
    xv_set(panel, PANEL_CARET_ITEM,
	   store_panel_items[(int) FILE_STRING_ITEM], NULL);

    store_panel_items[(int) FILE_CMD_ITEM] = panel_create_item(panel,
							       PANEL_BUTTON,
						PANEL_LABEL_X, ATTR_COL(26),
						 PANEL_LABEL_Y, ATTR_ROW(2),
					     PANEL_LABEL_STRING, 
				    XV_MSG("Store as New File"),
					  PANEL_NOTIFY_PROC, store_cmd_proc,
					       HELP_INFO("textsw:storefile")
							       NULL);
    (void) xv_set(panel,
		  PANEL_DEFAULT_ITEM, store_panel_items[(int) FILE_CMD_ITEM],
		  NULL);
}

Pkg_private          Panel
textsw_create_store_panel(frame, view)
    Frame           frame;
    Textsw_view_handle view;
{
    Panel           panel;

    panel = (Panel) xv_get(frame, FRAME_CMD_PANEL,
			   HELP_INFO("textsw:storepanel")
			   NULL);
    (void) create_store_items(panel, view);

    return (panel);
}
