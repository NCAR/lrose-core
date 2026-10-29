/*      @(#)fchsr_impl.h 1.22 93/06/28 SMI      */

/*
 *	(c) Copyright 1992, 1993 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE file
 *	for terms of the license.
 */


#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/notice.h>
#include <xview_private/xv_path_util.h>
#include <xview/hist.h>
#include <xview/path.h>
#include <xview/file_list.h>
#include <xview/file_chsr.h>



typedef struct {
    Panel		panel;
    History_menu	hist_menu;
    Panel_message_item	goto_msg;
    Panel_button_item	goto_btn;
    Path_name		goto_txt;
    Panel_text_item	folder_txt;
    Panel_message_item	select_msg;
    File_list		list;

    Panel_text_item	document_txt; 	/* not use in Open */
    Panel_button_item	open_btn;	/* "Open Folder" in Save */
    Panel_button_item	cancel_btn;
    Panel_button_item	save_btn; 	/* not used in Open */
    Panel_button_item	custom_btn;	/* used by Custom Dialog attr */

    Xv_notice		notice;		/* re-used by error checks */
} Fc_ui;



/*
 * Need to keep state until UI objects are created.
 * Note:  the Frame_cmd package doesn't create the
 * Cmd Panel until XV_END_CREATE, so any "redundant"
 * attributes set at create time need to be saved
 * until the items they apply to are created.
 */
struct keep_state {
    char *		directory;		/* FILE_CHOOSER_DIRECTORY */
    char *		filter_string;		/* FILE_CHOOSER_FILTER_STRING */
    char *		doc_name;		/* FILE_CHOOSER_DOC_NAME */
    char *		custom_name;		/* FILE_CHOOSER_CUSTOM_DIALOG */
    char *		custom_string;		/* FILE_CHOOSER_CUSTOM_DIALOG */
    Server_image	match_glyph;		/* FILE_CHOOSER_MATCH_GLYPH */
    Server_image	match_glyph_mask;	/* FILE_CHOOSER_MATCH_GLYPH_MASK */
    unsigned		hidden_files : 1;	/* FILE_CHOOSER_SHOW_HIDDEN_FILES */
    unsigned		abbrev_view : 1;	/* FILE_CHOOSER_ABBREV_VIEW */
    unsigned		auto_update : 1;	/* FILE_CHOOSER_AUTO_UPDATE */
};





typedef struct {
    Xv_opaque		public_self;
    File_chooser_type	type;
    Fc_ui		ui;
    History_list	hist_list;
    int			(* notify_func)();
    int			(* cd_func)();
    File_chooser_op	(* filter_func)();
    int			(* compare_func)();
    struct keep_state *	state;		/* use until UI created */
    Rect		rect;		/* size of dialog, used by layout */
    int			col_width; 	/* from FONT_DEFAULT_CHAR_WIDTH */
    int			row_height;	/* from FOND_DEFAULT_CHAR_HEIGHT */
    int			user_entries;	/* number of user entries in goto menu */
    unsigned short	filter_mask;	/* mask for filter_func */
    int			default_width;  /* default width as calculated */
    int			default_height; /* default height as calculated */
    int			min_width;	/* min width as calculated */
    int			min_height;	/* min height as calculated */
    unsigned		goto_select:1; 	/* SELECT down on Go To button */
    unsigned		save_to_dir:1;	/* allow saving empty file name */
    unsigned		been_mapped:1;	/* chooser has been mapped */
    unsigned		no_confirm:1;	/* turn off save confirmation */
    unsigned		default_action_failed:1; /* flag DefaultAction */


    /* PANEL_EVENT_PROC for document typein */
    void		(* document_default_event)(); 

    /*
     * Define Extension Items and space.
     */
    File_chooser_op	custom;		/* "customized" dialog */
    int			exten_height;
    int			(* exten_func)();

#ifdef OW_I18N
    int			wchar_notify; /* use wide char for notify procs */
    int			(* notify_func_wcs)();
#endif
} Fc_private;



/*
 * Name File Chooser uses to find it's default History_list
 */
#define FC_HISTORY_NAME		"XView GoTo History"


/* The Open Look File Choosing Spec defines these numbers */
#define FC_APP_SPACE_SIZE		5 	/* number of app dirs in goto menu */
#define FC_RECENT_SPACE_DEFAULT_SIZE	8 	/* size of recent space in goto menu */
#define FC_RECENT_SPACE_MAX		15	/* max size of goto recent space */


#define FC_PUBLIC(item)	XV_PUBLIC(item)
#define FC_PRIVATE(item) \
    	XV_PRIVATE(Fc_private, File_chooser_public, item)



Pkg_private void	file_chooser_position_objects();
Pkg_private void	file_chooser_calc_min_size();
Pkg_private void	file_chooser_calc_default_size();
