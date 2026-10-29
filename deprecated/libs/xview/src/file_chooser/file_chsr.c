#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)file_chsr.c 1.60 93/06/28";
#endif
#endif
 

/*
 *	(c) Copyright 1992, 1993 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE file
 *	for terms of the license.
 */


#include <stdio.h>
#include <unistd.h>
#include <xview/xview.h>
#include <xview/defaults.h>
#include <xview/panel.h>
#include <xview/file_list.h>
#include <xview/hist.h>
#include <xview/path.h>
#include <xview_private/fchsr_impl.h>
#include <xview_private/i18n_impl.h>


/* private data key for internal objects */
static Attr_attribute	FC_KEY = 0;


/* default PANEL_EVENT_PROC for goto field. */
static void		(* default_goto_event_handler)();


static void		fc_end_create();
static void		fc_create_ui();
static Notify_value	fc_event_interposer();
static void		fc_goto_event_proc();
static void		fc_open_notify();
static void		fc_save_notify();
static void		fc_cancel_notify();
static int		fc_cd_func();
static void		fc_hist_notify();
static Panel_setting	fc_goto_notify();
static int		fc_list_notify();
static int		fc_do_open();
static int		fc_do_save();
static File_list_op	fc_filter_func();
static int		fc_compare_func();
static History_list	fc_default_history();
static void		fc_update_dimming();
static void		fc_goto_btn_event();
static void		fc_document_txt_event();
static void		fc_item_inactive();
static int		fc_confirm_overwrite();


/* not really public, not really private, but useful none the less... */
extern char *		xv_getlogindir();


/*----------------------------------------------------------------------------*/


/*
 * xv_create() method
 */
Pkg_private int
file_chooser_init( owner, public, avlist)
     Xv_opaque owner;
     File_chooser_public *public;
     Attr_avlist avlist;
{
    Fc_private *	private = xv_alloc( Fc_private );
    Attr_avlist 	attrs;
    char *		label_str;



    public->private_data = (Xv_opaque) private;
    private->public_self = (Xv_opaque) public;


    private->type = FILE_CHOOSER_OPEN; /* got to default to something... */
    private->compare_func = FILE_CHOOSER_DEFAULT_COMPARE;
    private->filter_mask = FC_MATCHED_FILES_MASK;


    /*
     * Initialize Keep State.  The Keep State is used by xv_set()
     * to hold redundant values until the UI objects that would
     * normally use them are created.  Because the FRAME_CMD doesn't
     * create the FRAME_CMD_PANEL until XV_END_CREATE, we cannot
     * create our UI components before then.
     */
    private->state = xv_alloc( struct keep_state );
    private->state->directory = (char *) getcwd(NULL, MAXPATHLEN);
    private->state->auto_update = TRUE;


    if ( !FC_KEY )
	FC_KEY = xv_unique_key();


    /*
     * Parse Create-Only Attributes
     */
    for (attrs=avlist; *attrs; attrs=attr_next(attrs)) {
	switch ( (int) attrs[0] ) {
	case FILE_CHOOSER_TYPE:
	    ATTR_CONSUME(attrs[0]);
	    private->type = (File_chooser_type) attrs[1];
	    break;

	default:
	    break;
	}
    } /* create-only attrs */




    /*
     * Set up type-specific defaults.
     */
    switch ( private->type ) {
    case FILE_CHOOSER_OPEN:
	label_str = XV_MSG("Open");
	break;

    case FILE_CHOOSER_SAVE:
	label_str = XV_MSG("Save");
	private->state->doc_name = xv_strcpy(NULL, XV_MSG("Untitled1"));
	break;

    case FILE_CHOOSER_SAVEAS:
	label_str = XV_MSG("Save As");
	break;
    }


    /* Modified defaults */
    xv_set((Xv_opaque) public,
	   XV_LABEL,			label_str,
	   FRAME_SHOW_RESIZE_CORNER,	TRUE,
	   FRAME_SHOW_FOOTER,		TRUE,
	   NULL);

    return XV_OK;
} /* file_chooser_init() */




/*
 * xv_set() method
 */
Pkg_private Xv_opaque
file_chooser_set( public, avlist )
     File_chooser public;
     Attr_avlist avlist;
{
    Fc_private *private = FC_PRIVATE(public);
    Attr_avlist attrs;
    int do_update = FALSE;


    for (attrs=avlist; *attrs; attrs=attr_next(attrs)) {
	switch ( (int) attrs[0] ) {
	case FILE_CHOOSER_UPDATE:
	    ATTR_CONSUME(attrs[0]);
	    if ( private->ui.list )
		xv_set(private->ui.list, FILE_LIST_UPDATE, NULL);
	    break;

	case FILE_CHOOSER_DOC_NAME:
	    ATTR_CONSUME(attrs[0]);

	    if ( private->type == FILE_CHOOSER_OPEN )
		break;

	    if ( private->state )
		private->state->doc_name 
		    = xv_strcpy( private->state->doc_name, 
				   (char *)attrs[1] 
				   );
	    else {
		xv_set( private->ui.document_txt,
		       PANEL_VALUE, attrs[1], 
		       NULL);
		if ( !private->save_to_dir )
		    fc_item_inactive( private->ui.save_btn, no_string((char *)attrs[1]) );
	    }
	    break;

#ifdef OW_I18N
	case FILE_CHOOSER_DOC_NAME_WCS:
	    ATTR_CONSUME(attrs[0]);

	    if ( private->type == FILE_CHOOSER_OPEN )
		break;

	    if ( private->state ) {
		xv_free_ref( private->state->doc_name );
		private->state->doc_name = _xv_wcstombsdup( (wchar_t *) attrs[1] );
	    } else 
		xv_set( private->ui.document_txt,
		       PANEL_VALUE_WCS, attrs[1], 
		       NULL);
		if ( !private->save_to_dir )
		    fc_item_inactive( private->ui.save_btn, no_string((char *)attrs[1]) );
	    break;
#endif /* OW_I18N */




#ifdef OW_I18N
	case FILE_CHOOSER_DIRECTORY_WCS:
#endif
	case FILE_CHOOSER_DIRECTORY:

#ifdef OW_I18N
	    if ( attrs[0] == FILE_CHOOSER_DIRECTORY_WCS ) {
		if ( private->state ) {
		    xv_free_ref( private->state->directory );
		    private->state->directory 
			= _xv_wcstombsdup( (wchar_t *) attrs[1] );
		} else {
		    xv_set(private->ui.list,
			   FILE_LIST_DIRECTORY_WCS,	attrs[1],
			   NULL);
		}
	    } else {		/* Multibyte */
#endif
	    if ( private->state ) {
		private->state->directory 
		    = xv_strcpy(private->state->directory,
				   (char *)attrs[1]
				   );
	    } else {
		xv_set(private->ui.list,
		       FILE_LIST_DIRECTORY,	attrs[1],
		       NULL);
	    }

#ifdef OW_I18N
	    }
#endif

	    /* 
	     * make sure current folder gets null'd as well, 
	     * because the cd-func won't get activated.
	     */
	    if ( !private->state && !attrs[1] )
		xv_set(private->ui.folder_txt,
		       PANEL_VALUE,		"",
		       NULL);

	    ATTR_CONSUME(attrs[0]);
	    break;	/* case FILE_CHOOSER_DIRECTORY(_WCS) */



	case FILE_CHOOSER_CHILD:
	case FILE_CHOOSER_TYPE:
	    ATTR_CONSUME(attrs[0]);
	    xv_error(public,
		     ERROR_PKG,		FILE_CHOOSER,
		     ERROR_CANNOT_SET,	attrs[0],
		     NULL);
	    break;


	case FILE_CHOOSER_NOTIFY_FUNC:
	    ATTR_CONSUME(attrs[0]);
	    private->notify_func = (int (*)()) attrs[1];
	    break;


#ifdef OW_I18N
	case FILE_CHOOSER_NOTIFY_FUNC_WCS:
	    ATTR_CONSUME(attrs[0]);
	    private->notify_func_wcs = (int (*)()) attrs[1];
	    break;
#endif /* OW_I18N */


	case FILE_CHOOSER_AUTO_UPDATE:
	    if ( private->state )
		private->state->auto_update = (unsigned) attrs[1];
	    else
		xv_set(private->ui.list, 
		       FILE_LIST_AUTO_UPDATE,  attrs[1],
		       NULL);
	    break;


	case FILE_CHOOSER_SHOW_DOT_FILES:
	    ATTR_CONSUME(attrs[0]);

	    if ( private->state )
		private->state->hidden_files = (unsigned) ((attrs[1]) ? TRUE : FALSE);
	    else
		xv_set(private->ui.list, 
		       FILE_LIST_SHOW_DOT_FILES, attrs[1],
		       NULL);
	    break;


	case FILE_CHOOSER_ABBREV_VIEW:
	    ATTR_CONSUME(attrs[0]);

	    if ( private->state )
		private->state->abbrev_view = (unsigned) ((attrs[1]) ? TRUE : FALSE);
	    else
		xv_set(private->ui.list,
		       FILE_LIST_ABBREV_VIEW, attrs[1],
		       NULL);
	    break;

#ifdef OW_I18N
	case FILE_CHOOSER_APP_DIR_WCS:
#endif
	case FILE_CHOOSER_APP_DIR: {
	    int fixed_count;

	    if ( !private->hist_list )
		private->hist_list 
		    = fc_default_history( private, 
					 XV_SERVER_FROM_WINDOW(public) 
					 );

	    fixed_count = (int) xv_get(private->hist_list, HISTORY_FIXED_COUNT);

	    /* add a blank between User and App Spaces */
	    if ( fixed_count == private->user_entries )
		xv_set( private->hist_list,
		       HISTORY_ADD_FIXED_ENTRY,	NULL, NULL,
		       NULL);

	    /*
	     * LAF Note:  The Open Look File Choosing Specification states
	     * that the application is allowed no more than 5 entries.  This
	     * may be easily gotten around by setting them directly on the 
	     * History List.
	     */
	    if ( fixed_count <= (FC_APP_SPACE_SIZE + private->user_entries) )
#ifdef OW_I18N
		xv_set( private->hist_list,
		       (attrs[0] == FILE_CHOOSER_APP_DIR_WCS) 
		       	? HISTORY_ADD_FIXED_ENTRY_WCS
		       	: HISTORY_ADD_FIXED_ENTRY,
		       		attrs[1], attrs[2],
		       NULL);
#else
		xv_set( private->hist_list,
		       HISTORY_ADD_FIXED_ENTRY,	attrs[1], attrs[2],
		       NULL);
#endif
	    /* else ignore */

	    ATTR_CONSUME(attrs[0]);
	    break;
	}


	case FILE_CHOOSER_FILTER_STRING:
	    ATTR_CONSUME(attrs[0]);

	    if ( private->state ) {
		private->state->filter_string 
		    = xv_strcpy(private->state->filter_string,
				   (char *)attrs[1]
				   );
	    } else {
		xv_set(private->ui.list,
		       FILE_LIST_FILTER_STRING,	attrs[1],
		       NULL);
	    }
	    break;


#ifdef OW_I18N
	case FILE_CHOOSER_FILTER_STRING_WCS:
	    ATTR_CONSUME(attrs[0]);

	    if ( private->state ) {
		xv_free_ref( private->state->filter_string );
		private->state->filter_string 
		    = _xv_wcstombsdup( (wchar_t *) attrs[1] );
	    } else {
		xv_set(private->ui.list,
		       FILE_LIST_FILTER_STRING_WCS,	attrs[1],
		       NULL);
	    }
	    break;
#endif /* OW_I18N */


	case FILE_CHOOSER_MATCH_GLYPH:
	    ATTR_CONSUME(attrs[0]);

	    if ( private->state )
		private->state->match_glyph = (Server_image) attrs[1];
	    else
		xv_set( private->ui.list,
		       FILE_LIST_MATCH_GLYPH,	attrs[1],
		       NULL);
	    break;


	case FILE_CHOOSER_MATCH_GLYPH_MASK:
	    ATTR_CONSUME(attrs[0]);

	    if ( private->state )
		private->state->match_glyph_mask = (Server_image) attrs[1];
	    else
		xv_set( private->ui.list,
		       FILE_LIST_MATCH_GLYPH_MASK, attrs[1],
		       NULL);
	    break;


	case FILE_CHOOSER_CD_FUNC:
	    ATTR_CONSUME(attrs[0]);
	    private->cd_func = (int (*)()) attrs[1];

	    if ( !private->state ) {
		if ( (int) xv_get(private->ui.list, FILE_LIST_AUTO_UPDATE) )
		    do_update = TRUE;
	    }
	    break;


	case FILE_CHOOSER_FILTER_MASK:
	    ATTR_CONSUME(attrs[0]);
	    private->filter_mask = (unsigned short) attrs[1];

	    if ( !private->state ) {
		if ( (int) xv_get(private->ui.list, FILE_LIST_AUTO_UPDATE) )
		    do_update = TRUE;
	    }
	    break;


	case FILE_CHOOSER_FILTER_FUNC:
	    ATTR_CONSUME(attrs[0]);
	    private->filter_func = (File_chooser_op (*)()) attrs[1];

	    if ( !private->state ) {
		if ( (int) xv_get(private->ui.list, FILE_LIST_AUTO_UPDATE) )
		    do_update = TRUE;
	    }
	    break;


	case FILE_CHOOSER_COMPARE_FUNC:
	    ATTR_CONSUME(attrs[0]);
	    private->compare_func = (int (*)()) attrs[1];

	    if ( !private->state ) {
		if ( (int) xv_get(private->ui.list, FILE_LIST_AUTO_UPDATE) )
		    do_update = TRUE;
	    }
	    break;


	case FILE_CHOOSER_SAVE_TO_DIR:
	    ATTR_CONSUME(attrs[0]);

	    if ( private->type == FILE_CHOOSER_OPEN )
		break;		/* xv_error ? */

	    private->save_to_dir = (unsigned) ((attrs[1]) ? TRUE : FALSE);

	    /*
	     * LAF Note:  if this flag is TRUE, Save typein becomes inactive
	     * and the Save button should not gray out.  if the flag is FALSE,
	     * the Save typein is normal and the Save button becomes inactive
	     * if the typein becomes empty.  default is FALSE.
	     */
	    if ( private->state )
		break;
	    
	    if ( private->save_to_dir == TRUE ) {
		xv_set( private->ui.document_txt,
		       PANEL_VALUE,	"",
		       PANEL_INACTIVE,	TRUE,
		       NULL);
		fc_item_inactive( private->ui.save_btn, FALSE );
	    } else {
		char *val = (char *) xv_get(private->ui.document_txt, PANEL_VALUE);

		fc_item_inactive(private->ui.document_txt, FALSE);
		fc_item_inactive(private->ui.save_btn, no_string(val));
	    }
	    break;


	case FILE_CHOOSER_EXTEN_HEIGHT: {
	    /* 1.5 rows extra white space shown in spec */
	    int white_space = (int)(private->row_height * 1.5);
	    int width;
	    int height;

	    /*
	     * "Common Look" forces spec conformance to new heights.
	     * this uglyness was added to follow the spec exactly, while
	     * not breaking deskset.  in order to get the layout code to
	     * do this exactingly, augment the current and min sizes.
	     */

	    /* Remove the extra space used by extension area */
	    if ( ((int) attrs[1] == 0) && (private->exten_height != 0) ) {
		xv_get(public, FRAME_MIN_SIZE, &width, &height);
		xv_set(public, FRAME_MIN_SIZE, width, height - white_space, NULL);

		height = xv_get(public, XV_HEIGHT);
		xv_set(public, XV_HEIGHT, height - white_space, NULL);
	    } 
	    	/* Add space needed for extension area */
	    else  if ( ((int) attrs[1] > 0) && (private->exten_height == 0) ) {
		xv_get(public, FRAME_MIN_SIZE, &width, &height);
		xv_set(public, FRAME_MIN_SIZE, width, height + white_space, NULL);

		height = xv_get(public, XV_HEIGHT);
		xv_set(public, XV_HEIGHT, height + white_space, NULL);
	    }

	    private->exten_height = (int) attrs[1];
	    ATTR_CONSUME(attrs[0]);
	    break;
	}


	case FILE_CHOOSER_EXTEN_FUNC:
	    ATTR_CONSUME(attrs[0]);
	    private->exten_func = (int (*)()) attrs[1];
	    break;


#ifdef OW_I18N
	case FILE_CHOOSER_CUSTOMIZE_OPEN_WCS:
#endif
	case FILE_CHOOSER_CUSTOMIZE_OPEN:

	    if ( private->type != FILE_CHOOSER_OPEN ) {
		xv_error(public,
			 ERROR_PKG,	FILE_CHOOSER,
			 ERROR_STRING,	XV_MSG("Only valid for Open dialog"),
			 NULL);
		break;
	    }

	    if ( !private->state ) {
		xv_error(public,
			 ERROR_CREATE_ONLY,	attrs[0],
			 NULL);
		break;
	    }
#ifdef OW_I18N
	    if ( attrs[0] == FILE_CHOOSER_CUSTOMIZE_OPEN_WCS ) {
		xv_free_ref( private->state->custom_name );
		private->state->custom_name 
		    = _xv_wcstombsdup( (wchar_t *) attrs[1] );

		xv_free_ref( private->state->custom_string );
		private->state->custom_string
		    = _xv_wcstombsdup( (wchar_t *) attrs[2] );
	    } else {
#endif
	    private->state->custom_name 
		= xv_strcpy(private->state->custom_name,
			       (char *) attrs[1]
			       );

	    private->state->custom_string
		= xv_strcpy(private->state->custom_string,
			       (char *) attrs[2]
			       );
#ifdef OW_I18N
	}
#endif
	    private->custom = (File_chooser_op) attrs[3];
	    ATTR_CONSUME(attrs[0]);
	    break;


#ifdef OW_I18N
	case FILE_CHOOSER_WCHAR_NOTIFY:
	    ATTR_CONSUME(attrs[0]);
	    private->wchar_notify = (int) attrs[1];
	    break;
#endif /* OW_I18N */


	case FILE_CHOOSER_HISTORY_LIST:
	    ATTR_CONSUME(attrs[0]);
	    private->hist_list = (History_list) attrs[1];

	    /* note:  hist-menu package does ref counting */
	    if ( private->ui.hist_menu )
		xv_set(private->ui.hist_menu, 
		       HISTORY_MENU_HISTORY_LIST, private->hist_list, 
		       NULL);
	    break;


	case FILE_CHOOSER_ASSUME_DEFAULT_SIZE:	/* private */
	    /* CHEAPO:  doesn't take exten area into account */
	    xv_set(public,
		   XV_WIDTH,	private->default_width,
		   XV_HEIGHT,	private->default_height,
		   NULL);
	    break;


	case FILE_CHOOSER_NO_CONFIRM:	/* private */
	    private->no_confirm = (unsigned) ((attrs[1]) ? TRUE : FALSE);
	    break;



	    /*
	     * WARNING:  the docs should read that it is the client's
	     * responsiblity to make sure that they don't go under the
	     * min size.  the XV_WIDTH/XV_HEIGH checking is really a token
	     * jesture, because there are many ways in which they could
	     * modify the frame size aside from this (such as XV_RECT, 
	     * and frame_set_height).
	     */
	case XV_WIDTH:
	    if ( private->state ) {
		ATTR_CONSUME(attrs[0]);
		break;
	    }

	    /*
	     * FRAME_MIN_SIZE only works for interactive
	     * sizing, not programatic...
	     */
	    if ( (int) attrs[1] < private->min_width ) {
		Attr_attribute fc_attrs[3];
		fc_attrs[0] = XV_WIDTH;
		fc_attrs[1] = (Attr_attribute) private->min_width;
		fc_attrs[2] = (Attr_attribute) NULL;
		xv_super_set_avlist(public, FILE_CHOOSER, (Attr_avlist) fc_attrs);
		ATTR_CONSUME(attrs[0]);
	    }
	    break;

	case XV_HEIGHT:
	    if ( private->state ) {
		ATTR_CONSUME(attrs[0]);
		break;
	    }

	    /*
	     * FRAME_MIN_SIZE only works for interactive
	     * sizing, not programatic...
	     */
	    if ( (int) attrs[1] < private->min_height ) {
		Attr_attribute fc_attrs[3];
		fc_attrs[0] = XV_HEIGHT;
		fc_attrs[1] = (Attr_attribute) private->min_height;
		fc_attrs[2] = (Attr_attribute) NULL;
		xv_super_set_avlist(public, FILE_CHOOSER, (Attr_avlist) fc_attrs);
		ATTR_CONSUME(attrs[0]);
	    }
	    break;

	case FRAME_MIN_SIZE: {
	    Attr_attribute fc_attrs[4];

	    if ( private->state ) {
		ATTR_CONSUME(attrs[0]);
		break;
	    }

	    /*
	     * Don't want the min size lowered from the calculated
	     * values.  they may increase it, expecially if they are
	     * using the extension api.
	     */
	    if ( ((int) attrs[1] <= private->min_width)
		&& ((int) attrs[2] <= private->min_height)
		) {
		ATTR_CONSUME(attrs[0]);
		break;
	    }

	    if ( (int) attrs[1] > private->min_width )
	        private->min_width = (int) attrs[1];
	    if ( (int) attrs[2] > private->min_height )
	        private->min_height = (int) attrs[2];

	    fc_attrs[0] = FRAME_MIN_SIZE;
	    fc_attrs[1] = (Attr_attribute) private->min_width;
	    fc_attrs[2] = (Attr_attribute) private->min_height;
	    fc_attrs[3] = (Attr_attribute) NULL;
	    xv_super_set_avlist(public, FILE_CHOOSER, (Attr_avlist) fc_attrs);
	    ATTR_CONSUME(attrs[0]);
	    break;
	}


	case XV_END_CREATE:
	    fc_end_create( private );
	    break;


	default:
	    xv_check_bad_attr(FILE_CHOOSER, attrs[0]);
	    break;
	} /* switch() */
    } /* for() */


    /*
     * An attribute was set that maps to the File List, but
     * File Chooser is intercepting.  Must deal with FILE_LIST
     * AUTO_UPDATE manually.
     */
    if ( do_update ) 
	xv_set(private->ui.list, FILE_LIST_UPDATE, NULL);

    return XV_OK;
} /* file_chooser_set() */




/*
 * xv_get() method
 */
Pkg_private Xv_opaque
file_chooser_get ( public, status, attr, args )
     File_chooser_public	*public;
     int 	            	*status;
     Attr_attribute  	   	attr;
     va_list     	  	args;
{
    Fc_private *private = FC_PRIVATE(public);

    switch ( (int) attr ) {
    case FILE_CHOOSER_DIRECTORY:
	return xv_get(private->ui.list, FILE_LIST_DIRECTORY);

#ifdef OW_I18N
    case FILE_CHOOSER_DIRECTORY_WCS:
	return xv_get(private->ui.list, FILE_LIST_DIRECTORY_WCS);
#endif

    case FILE_CHOOSER_TYPE:
	return (Xv_opaque) private->type;

    case FILE_CHOOSER_CHILD: {
	File_chooser_child child = va_arg(args, int);

	switch( child ) {
	case FILE_CHOOSER_GOTO_MESSAGE_CHILD:
	    return (Xv_opaque) private->ui.goto_msg;
	case FILE_CHOOSER_GOTO_BUTTON_CHILD:
	    return (Xv_opaque) private->ui.goto_btn;
	case FILE_CHOOSER_GOTO_PATH_CHILD:
	    return (Xv_opaque) private->ui.goto_txt;
	case FILE_CHOOSER_HISTORY_MENU_CHILD:
	    return (Xv_opaque) private->ui.hist_menu;
	case FILE_CHOOSER_CURRENT_FOLDER_CHILD:
	    return (Xv_opaque) private->ui.folder_txt;
	case FILE_CHOOSER_SELECT_MESSAGE_CHILD:
	    return (Xv_opaque) private->ui.select_msg;
	case FILE_CHOOSER_FILE_LIST_CHILD:
	    return (Xv_opaque) private->ui.list;
	case FILE_CHOOSER_DOCUMENT_NAME_CHILD:
	    return (Xv_opaque) private->ui.document_txt;
	case FILE_CHOOSER_OPEN_BUTTON_CHILD:
	    return (Xv_opaque) private->ui.open_btn;
	case FILE_CHOOSER_SAVE_BUTTON_CHILD:
	    return (Xv_opaque) private->ui.save_btn;
	case FILE_CHOOSER_CANCEL_BUTTON_CHILD:
	    return (Xv_opaque) private->ui.cancel_btn;
	case FILE_CHOOSER_CUSTOM_BUTTON_CHILD:
	    return (Xv_opaque) private->ui.custom_btn;
	}
	break;
    }

    case FILE_CHOOSER_HISTORY_LIST:
	return (Xv_opaque) private->hist_list;

    case FILE_CHOOSER_AUTO_UPDATE:
	return xv_get(private->ui.list, FILE_LIST_AUTO_UPDATE);

    case FILE_CHOOSER_DOC_NAME:
	if ( private->type == FILE_CHOOSER_OPEN )
	    return XV_NULL;
	else
	    return xv_get(private->ui.document_txt, PANEL_VALUE);

#ifdef OW_I18N
    case FILE_CHOOSER_DOC_NAME_WCS:
	if ( private->type == FILE_CHOOSER_OPEN )
	    return XV_NULL;
	else
	    return xv_get(private->ui.document_txt, PANEL_VALUE_WCS);
#endif /* OW_I18N */

    case FILE_CHOOSER_FILTER_STRING:
	return xv_get(private->ui.list, FILE_LIST_FILTER_STRING);

    case FILE_CHOOSER_NOTIFY_FUNC:
	return (Xv_opaque) private->notify_func;

#ifdef OW_I18N
    case FILE_CHOOSER_FILTER_STRING_WCS:
	return xv_get(private->ui.list, FILE_LIST_FILTER_STRING_WCS);

    case FILE_CHOOSER_NOTIFY_FUNC_WCS:
	return (Xv_opaque) private->notify_func_wcs;
#endif

    case FILE_CHOOSER_SHOW_DOT_FILES:
	return xv_get(private->ui.list, FILE_LIST_SHOW_DOT_FILES);

    case FILE_CHOOSER_ABBREV_VIEW:
	return xv_get(private->ui.list, FILE_LIST_ABBREV_VIEW);

    case FILE_CHOOSER_MATCH_GLYPH:
	return xv_get(private->ui.list, FILE_LIST_MATCH_GLYPH);

    case FILE_CHOOSER_MATCH_GLYPH_MASK:
	return xv_get(private->ui.list, FILE_LIST_MATCH_GLYPH_MASK);

    case FILE_CHOOSER_CD_FUNC:
	return (Xv_opaque) private->cd_func;

    case FILE_CHOOSER_FILTER_MASK:
	return private->filter_mask;

    case FILE_CHOOSER_FILTER_FUNC:
	return (Xv_opaque) private->filter_func;

    case FILE_CHOOSER_COMPARE_FUNC:
	return (Xv_opaque) private->compare_func;

    case FILE_CHOOSER_SAVE_TO_DIR:
	return (Xv_opaque) private->save_to_dir;

    case FILE_CHOOSER_EXTEN_HEIGHT:
	return (Xv_opaque) private->exten_height;

    case FILE_CHOOSER_EXTEN_FUNC:
	return (Xv_opaque) private->exten_func;

#ifdef OW_I18N
    case FILE_CHOOSER_WCHAR_NOTIFY:
	return (Xv_opaque) private->wchar_notify;
#endif

    case FILE_CHOOSER_NO_CONFIRM: /* private */
	return (Xv_opaque) private->no_confirm;

    default:
	*status = xv_check_bad_attr(FILE_CHOOSER, attr);
	return (Xv_opaque)XV_OK;
    } /* switch */

} /* file_chooser_get() */



/*
 * xv_destroy() method
 */
Pkg_private int
file_chooser_destroy( public, status )
     File_chooser_public *public;
     Destroy_status status;
{
    Fc_private *private = FC_PRIVATE(public);

    if (status != DESTROY_CLEANUP)
	return XV_OK;

    /* will deal with history list */
    xv_destroy( private->ui.hist_menu );

    xv_free ( private );

    return XV_OK;
} /* file_chooser_destroy() */


/******************************************************************************/



/*
 * Do XV_END_CREATE stuff
 */
static void
fc_end_create( private )
     Fc_private *private;
{
    Attr_attribute fc_attrs[8];
    Xv_font font;
    
    
    /*
     * Create UI at End-Create time so that Cmd Panel
     * is properly initialized.
     */
    private->ui.panel = xv_get(FC_PUBLIC(private), FRAME_CMD_PANEL);
    fc_create_ui( private );
    
    
    /*
     * Base row height on what was calculated for the
     * Scrolling List.  This more acurately adheres to the
     * OL Spec than the font height.
     */
    private->row_height 
	= (int) xv_get(private->ui.list, PANEL_LIST_ROW_HEIGHT);
    
    /* base the column width for layout on the font width */
    font = xv_get(FC_PUBLIC(private), XV_FONT);
    private->col_width 
#ifdef OW_I18N
	/*
	 * HIGH WEIRDNESS:  somewhere between S493-Alpha4.3 and
	 * Beta1.0, FONT_DEFAULT_CHAR_WIDTH started returning
	 * strange values!  FONT_COLUMN_WIDTH still seems to return
	 * the right stuff, but i'm not sure if a domestic build
	 * will still work.  Since i don't see any Font package changes
	 * in this timeframe, i suspect that the Xv_server is behaving 
	 * differently.
	 */
	= (int) xv_get(font, FONT_COLUMN_WIDTH);
#else
    = (int) xv_get(font, FONT_DEFAULT_CHAR_WIDTH);
#endif
    
    
    
    /*
     * Set the default and min size of the frame.
     */
    file_chooser_calc_min_size( private, 
			       &private->min_width, 
			       &private->min_height 
			       );
    file_chooser_calc_default_size( private, 
				   private->min_width, 
				   private->min_height,
				   &private->default_width, 
				   &private->default_height 
				   );
    
    /* kinda reminds you of Xt programming .... */
    fc_attrs[0] = XV_WIDTH;
    fc_attrs[1] = (Attr_attribute) private->default_width;
    fc_attrs[2] = XV_HEIGHT;
    fc_attrs[3] = (Attr_attribute) private->default_height;
    fc_attrs[4] = FRAME_MIN_SIZE;
    fc_attrs[5] = (Attr_attribute) private->min_width;
    fc_attrs[6] = (Attr_attribute) private->min_height;
    fc_attrs[7] = (Attr_attribute) NULL;
    xv_super_set_avlist(FC_PUBLIC(private), FILE_CHOOSER, (Attr_avlist) fc_attrs);
    
    /* look for Resize events to update the layout */
    (void) notify_interpose_event_func( (Notify_client) FC_PUBLIC(private), 
				       fc_event_interposer,
				       NOTIFY_SAFE
				       );
    
    /* 
     * Need to note when chooser first becomes visible.  see
     * comments in fc_event_interposer().
     */
    xv_set(FC_PUBLIC(private), 
	   WIN_CONSUME_EVENTS, WIN_VISIBILITY_NOTIFY, NULL,
	   NULL);
    
    /* Destroy the Keep State */
    xv_free_ref( private->state->directory );
    xv_free_ref( private->state->doc_name );
    xv_free_ref( private->state->filter_string );
    xv_free_ref( private->state->custom_name );
    xv_free_ref( private->state->custom_string );
    xv_free_ref( private->state );
} /* fc_end_create() */




/*
 * Look for events on the Frame itself.
 */
static Notify_value
fc_event_interposer( client, event, arg, type )
     Notify_client client;
     Notify_event event;
     Notify_arg arg;
     Notify_event_type type;
{
    Fc_private *private = FC_PRIVATE(client);
    Notify_value status;


    /*
     * Send Resize event on to Panel, so that the user doesn't
     * see a prolonged flash of white Frame background while File
     * Chooser calculates it's layout.
     */
    status = notify_next_event_func(client, event, arg, type);


    /*
     * Prevent the Exten Func from being called before the
     * chooser becomes mapped.  Frame seems to get 4 resizes
     * on creation...
     */
    if ( event_action((Event *)event) == WIN_VISIBILITY_NOTIFY ) {
	xv_set((Xv_opaque)client, WIN_IGNORE_EVENT, WIN_VISIBILITY_NOTIFY, NULL);
	private->been_mapped = TRUE;
    }

    if ( (private->been_mapped && event_action((Event *)event) == WIN_RESIZE)
	|| (event_action((Event *)event) == WIN_VISIBILITY_NOTIFY) 
	) {
	Rect *rect = (Rect *)xv_get(client, XV_RECT);

	if ( rect_sizes_differ(&(private->rect), rect) ) {
	    private->rect = (* rect);
	    file_chooser_position_objects( private );
	} else {
	    private->rect = (* rect);
	}
    }

    return status;
} /* fc_event_interposer() */





/*
 * Gross uglieness so that the File Chooser will know if the
 * DefaultAction (Meta-Return) on the GoTo Field resulted in
 * a valid file selection.
 */
static void
fc_goto_event_proc( path_name, event )
     Path_name path_name;
     Event *event;
{
    Fc_private *private = (Fc_private *)xv_get(path_name, XV_KEY_DATA, FC_KEY);
    Pkg_private Panel_setting xv_path_name_notify_proc();

    if (event_action(event) == ACTION_DEFAULT_ACTION
	&& event_meta_is_down(event)
	&& event_is_down(event)
	) {
	(void) xv_path_name_notify_proc(path_name, event);

	/*
	 * PANEL_NOTIFY_STATUS is overloaded in this instance to
	 * signify if the new path name was successfully validated
	 * or not by the notify proc.
	 */
	if ( (int) xv_get(path_name, PANEL_NOTIFY_STATUS) == XV_ERROR )
	    private->default_action_failed = TRUE;
    }

    if ( default_goto_event_handler )
	(* default_goto_event_handler)(path_name, event);
} /* fc_goto_interposer() */






/*
 * Find or create default History List.  History List's are find'able only
 * by name.  XView defines a name for the file chooser's default history list
 * and always uses that.  Also, on creation, add the default entries defined
 * by the Spec and from the Workspace.
 */
static History_list
fc_default_history( private, server )
     Fc_private *private;
     Xv_server server;
{
#define USER_DIR_DELIMITER	"\n"
    extern char *	xv_strtok();
    History_list 	hl;

    hl = xv_find( server, HISTORY_LIST,
		 XV_NAME,		FC_HISTORY_NAME,
		 XV_AUTO_CREATE,	FALSE,
		 NULL );

    if ( !hl ) {
	char *user_dir;
	int recent_max 
	    = defaults_get_integer_check( "openWindows.gotoMenu.recentCount",
					 "OpenWindows.GotoMenu.RecentCount",
					 FC_RECENT_SPACE_DEFAULT_SIZE,
					 0, FC_RECENT_SPACE_MAX
					 );
	char *user_entries
	    = defaults_get_string( "openWindows.gotoMenu.userDirs",
				  "OpenWindows.GotoMenu.UserDirs",
				  NULL
				  );

	hl = xv_create( server, HISTORY_LIST,
		       XV_NAME,				FC_HISTORY_NAME,
		       HISTORY_ADD_FIXED_ENTRY,		XV_MSG("Home"), xv_getlogindir(),
		       HISTORY_DUPLICATE_LABELS,	FALSE,
		       HISTORY_DUPLICATE_VALUES,	FALSE,
		       HISTORY_ROLLING_MAXIMUM,		recent_max,
		       NULL);
	private->user_entries = 1; 	/* Home */


	/*
	 * Parse out the users entries and put them in the list.  The list
	 * of directories is delimited by a \n character.  BUG:  this seems
	 * to do wierd things when done via the -xrm option.
	 */
	user_dir = xv_strtok( user_entries, USER_DIR_DELIMITER );
	while ( user_dir ) {
	    char *real_path = xv_expand_path( user_dir );
	    if ( !xv_isdir( real_path ) ) {
		char buf[MAXPATHLEN+1];
		(void) sprintf(buf, 
		           XV_MSG("Unable to access OpenWindows.GotoMenu.UserDirs entry:\n\"%s\".\n"),
			   user_dir
			   );
		xv_error(FC_PUBLIC(private),
			 ERROR_PKG,	FILE_CHOOSER,
			 ERROR_STRING,	buf,
			 NULL);
 	    } else {
		xv_set(hl, HISTORY_ADD_FIXED_ENTRY, user_dir, real_path, NULL);
		private->user_entries++;
	    }
	    xv_free_ref( real_path );
	    user_dir = xv_strtok( NULL, USER_DIR_DELIMITER );
	}

    }

    return hl;
} /* fc_default_history() */





/*
 * Create the User Interface objects for the File Chooser.
 * Layout occurs later when a WIN_RESIZE is received.
 */
static void
fc_create_ui( private )
     Fc_private *private;
{
    Xv_Server server = XV_SERVER_FROM_WINDOW(FC_PUBLIC(private));
    char *	default_help;
    Xv_opaque	default_item;
    Menu	menu;


    /*
     * if nobody passed in a History_list, find or create
     * the default one.
     */
    if ( !private->hist_list )
	private->hist_list = fc_default_history( private, server );


    private->ui.hist_menu 
	= xv_create(server, HISTORY_MENU, 
		    HISTORY_MENU_HISTORY_LIST,	private->hist_list,
		    HISTORY_NOTIFY_PROC,	fc_hist_notify,
		    XV_KEY_DATA,		FC_KEY, private,
		    NULL);

    menu = xv_get(private->ui.hist_menu, HISTORY_MENU_OBJECT);
    xv_set(menu, XV_HELP_DATA, "xview:file_chooser_goto_menu", NULL);

    private->ui.goto_msg
	= xv_create(private->ui.panel, PANEL_MESSAGE,
		    XV_HELP_DATA, "xview:file_chooser",
		    PANEL_LABEL_STRING,	
		    	XV_MSG("Type in the path to the folder and press Return."),
		    NULL);
    private->ui.goto_btn 
	= xv_create(private->ui.panel, PANEL_BUTTON,
		    XV_HELP_DATA, 		"xview:file_chooser_goto_menu",
		    PANEL_LABEL_STRING,		XV_MSG("Go To:"),
		    PANEL_ITEM_MENU,		menu,
		    PANEL_VALUE_STORED_LENGTH, 	MAXPATHLEN+1,
		    PANEL_EVENT_PROC,		fc_goto_btn_event,
		    XV_KEY_DATA,		FC_KEY, private,
		    NULL);

    private->ui.goto_txt 
	= xv_create(private->ui.panel, PATH_NAME,
		    XV_HELP_DATA, 	"xview:file_chooser_goto_path",
		    PATH_USE_FRAME,	TRUE,
		    PANEL_NOTIFY_PROC,	fc_goto_notify,
		    XV_KEY_DATA,	FC_KEY, private,
		    NULL);
    /*
     * Since the client can't supply a goto field, it is safe to
     * ASSume that they all have the same default event handler.
     */
    if ( !default_goto_event_handler )
	default_goto_event_handler 
		= (void (*)()) xv_get(private->ui.goto_txt, PANEL_EVENT_PROC);
    xv_set(private->ui.goto_txt, PANEL_EVENT_PROC, fc_goto_event_proc, NULL);

    private->ui.folder_txt 
	= xv_create(private->ui.panel, PANEL_TEXT,
		    XV_HELP_DATA, 		"xview:file_chooser_current_folder",
		    PANEL_READ_ONLY,		TRUE,
		    PANEL_LABEL_STRING,		XV_MSG("Current Folder:"),
		    PANEL_VALUE,		(private->state->directory)
		    					? private->state->directory
		    					: "",
		    PANEL_VALUE_UNDERLINED,	FALSE,
		    PANEL_VALUE_STORED_LENGTH, MAXPATHLEN+1,
		    PANEL_LAYOUT,		PANEL_VERTICAL,
		    NULL);


    if ( private->custom )
	default_help = private->state->custom_string;
    else if ( private->type == FILE_CHOOSER_OPEN )
	default_help = XV_MSG("Select a file or folder and click Open.");
    else
	default_help = XV_MSG("Select a file or folder and click Open Folder.");
	
    private->ui.select_msg 
	= xv_create(private->ui.panel, PANEL_MESSAGE,
		    XV_HELP_DATA, 	"xview:file_chooser",
		    PANEL_LABEL_STRING,	default_help,
		    NULL);

    private->ui.list 
	= xv_create(private->ui.panel, FILE_LIST,
		    XV_HELP_DATA, 		"xview:file_chooser_file_list",
		    FILE_LIST_DIRECTORY,	private->state->directory,
		    FILE_LIST_FILTER_STRING,	private->state->filter_string,
		    FILE_LIST_MATCH_GLYPH,	private->state->match_glyph,
		    FILE_LIST_MATCH_GLYPH_MASK,	private->state->match_glyph_mask,
		    FILE_LIST_FILTER_MASK,	FL_ALL_MASK, /* see fc_filter_func() */
		    FILE_LIST_ABBREV_VIEW,	private->state->abbrev_view,
		    FILE_LIST_SHOW_DOT_FILES,	private->state->hidden_files,
		    FILE_LIST_AUTO_UPDATE,	private->state->auto_update,
		    FILE_LIST_COMPARE_FUNC,	fc_compare_func,
		    FILE_LIST_USE_FRAME,	TRUE,
		    FILE_LIST_CHANGE_DIR_FUNC,	fc_cd_func,
		    PANEL_NOTIFY_PROC,		fc_list_notify,
		    FILE_LIST_FILTER_FUNC,	fc_filter_func,
		    XV_KEY_DATA,		FC_KEY, private,
		    NULL);

    if ( private->type != FILE_CHOOSER_OPEN ) {
	private->ui.document_txt 
	    = xv_create(private->ui.panel, PANEL_TEXT,
			XV_HELP_DATA, 	"xview:file_chooser_document_name",
			PANEL_LABEL_STRING,
				(private->type == FILE_CHOOSER_SAVE)
					? XV_MSG("Save:") 
					: XV_MSG("Save As:"),
			PANEL_VALUE_STORED_LENGTH, MAXPATHLEN+1,
			PANEL_VALUE,		private->state->doc_name,
			XV_KEY_DATA,		FC_KEY, private,
			NULL);

	private->document_default_event 
	    = (void (*)()) xv_get( private->ui.document_txt, PANEL_EVENT_PROC);
	xv_set(private->ui.document_txt,
	       PANEL_EVENT_PROC, fc_document_txt_event, 
	       NULL);
    }

    private->ui.open_btn 
	= xv_create(private->ui.panel, PANEL_BUTTON,
		    XV_HELP_DATA, 	"xview:file_chooser_open_button",
		    PANEL_LABEL_STRING,
		    	((private->type == FILE_CHOOSER_OPEN) && !private->custom)
		    		? XV_MSG("Open")
		    		: XV_MSG("Open Folder"),
		    PANEL_NOTIFY_PROC,	fc_open_notify,
		    XV_KEY_DATA,	FC_KEY, private,
		    NULL);

    private->ui.cancel_btn 
	= xv_create(private->ui.panel, PANEL_BUTTON,
		    XV_HELP_DATA, 	"xview:file_chooser_cancel_button",
		    PANEL_LABEL_STRING,	XV_MSG("Cancel"),
		    PANEL_NOTIFY_PROC,	fc_cancel_notify,
		    XV_KEY_DATA,	FC_KEY, private,
		    NULL);

    if ( private->type != FILE_CHOOSER_OPEN )
	private->ui.save_btn 
	    = xv_create(private->ui.panel, PANEL_BUTTON,
			XV_HELP_DATA, 		"xview:file_chooser_save_button",
			PANEL_LABEL_STRING,	XV_MSG("Save"),
			PANEL_NOTIFY_PROC,	fc_save_notify,
			PANEL_INACTIVE,		no_string( private->state->doc_name ),
			XV_KEY_DATA,		FC_KEY, private,
			NULL);

    /*
     * BUG ALERT:  how to handle help key for custom button?
     */
    if ( private->custom )
	private->ui.custom_btn
	    = xv_create(private->ui.panel, PANEL_BUTTON,
			XV_HELP_DATA, 		"xview:file_chooser",
			PANEL_LABEL_STRING,	private->state->custom_name,
			PANEL_NOTIFY_PROC,	fc_open_notify,
			XV_KEY_DATA,		FC_KEY, private,
			NULL);


    /* set default button */
    if ( private->custom )
	default_item = private->ui.custom_btn;
    else if ( private->type == FILE_CHOOSER_OPEN )
	default_item = private->ui.open_btn;
    else
	default_item = private->ui.save_btn;


    xv_set(private->ui.panel,
	   XV_HELP_DATA, 	"xview:file_chooser",
	   PANEL_CARET_ITEM,	(private->type == FILE_CHOOSER_OPEN)
	   				? private->ui.goto_txt
	   				: private->ui.document_txt,
	   PANEL_DEFAULT_ITEM,	default_item,
	   PANEL_BORDER,	TRUE,
	   NULL);


    /* make sure dimming is current. */
    if ( private->custom )
	fc_update_dimming( private, 0 );

} /* fc_create_ui() */





/*
 * Open button was pressed.  kind of ASSumes that PANEL_CHOOSE_NONE
 * is FALSE on the Scrolling list (i.e. it goes by the PANEL_LIST_
 * FIRST_SELECTED).
 */
static void
fc_open_notify( item, event )
     Panel_button_item item;
     Event *event;
{
    Fc_private *private = (Fc_private *)xv_get(item, XV_KEY_DATA, FC_KEY);
    int row = (int) xv_get(private->ui.list, PANEL_LIST_FIRST_SELECTED);
    char *dir = (char *) xv_get(private->ui.list, FILE_LIST_DIRECTORY);
    char *file = (char *) xv_get(private->ui.list, PANEL_LIST_STRING, row);
    Xv_opaque client_data = xv_get(private->ui.list, PANEL_LIST_CLIENT_DATA, row);


    /* FILE_LIST_DIRECTORY can be NULL */
    if ( no_string(dir) )
	return;


    /*
     * DefaultAction occured in GoTo Field, but was not
     * sucessfully validated (see fc_goto_event_proc).
     */
    if ( private->default_action_failed ) {
	private->default_action_failed = FALSE;
	return;
    }


    /* if open failed, don't dismiss popup */
    if ( fc_do_open( private, row, dir, file, client_data ) != XV_OK )
	xv_set(item, PANEL_NOTIFY_STATUS, XV_ERROR, NULL);
} /* fc_open_notify() */






/*
 * Save Button was pressed.  kind of ASSumes PANEL_CHOOSE_NONE
 * is FALSE on the Scrolling List. (i.e. it goes by the PANEL_LIST_
 * FIRST_SELECTED).
 */
static void
fc_save_notify( item, event )
     Panel_button_item item;
     Event *event;
{
    Fc_private *private = (Fc_private *)xv_get(item, XV_KEY_DATA, FC_KEY);
    char *dir = (char *) xv_get(private->ui.list, FILE_LIST_DIRECTORY);
    char *file = (char *) xv_get(private->ui.document_txt, PANEL_VALUE);
    char *path = (char *)NULL;

    /*
     * if the user put full path into Save typein, make sure
     * that the expanded version of the path goes into the Save
     * callback.
     */
    if ( !no_string(file) )
	path = xv_expand_path( file );
    if ( fc_do_save( private, dir, path ) != XV_OK )
	xv_set(item, PANEL_NOTIFY_STATUS, XV_ERROR, NULL);
    xv_free_ref( path );
} /* fc_save_notify */





/*
 * Cancel Button was pressed.
 */
static void
fc_cancel_notify( item, event )
     Panel_button_item item;
     Event *event;
{
    Fc_private *private = (Fc_private *)xv_get(item, XV_KEY_DATA, FC_KEY);

    /*
     * All Cancel really does is dismiss the popup.  Note:  Panel_button_item
     * will automatically set XV_SHOW to FALSE.
     */
    xv_set(FC_PUBLIC(private), FRAME_CMD_PIN_STATE, FRAME_CMD_PIN_OUT, NULL);
} /* fc_cancel_notify */





/*
 * File List Change-dir callback
 */
static int
fc_cd_func( list, path, sbuf, op )
     File_list list;
     char *path;
     struct stat *sbuf;
     File_list_op op;
{
    Fc_private *private = (Fc_private *)xv_get(list, XV_KEY_DATA, FC_KEY);



    /*
     * "cd" occured, add new path to History List and
     * update Current Folder field.  Also, make path the
     * relative path for the Goto typein
     */
    if ( op == FILE_LIST_AFTER_CD ) {
	if ( private->hist_list )
	    xv_set(private->hist_list, 
		   HISTORY_ADD_ROLLING_ENTRY,	path, path,
		   NULL);
	
	xv_set(private->ui.folder_txt,
	       PANEL_VALUE, path,
	       NULL);
	
	xv_set(private->ui.goto_txt,
	       PATH_RELATIVE_TO,	path,
	       NULL);
    }

#ifdef OW_I18N
    if ( private->cd_func && private->wchar_notify ) {
	wchar_t *wpath = _xv_mbstowcsdup( path );
	int status = (* private->cd_func)(FC_PUBLIC(private), 
					  wpath, sbuf, 
					  (File_chooser_op) op
					  );
	xv_free( wpath );
	return status;
    } else
#endif
    if ( private->cd_func )
	return (* private->cd_func)(FC_PUBLIC(private), 
				    path, sbuf, 
				    (File_chooser_op) op
				    );
    return XV_OK;
} /* fc_cd_func() */




/*
 * PANEL_EVENT_PROC on Go To Button.
 *
 * LAF Note:  As with FileManager, the Go To button is supposed
 * to goto the contents of the typein, if there is a path typed
 * typed in, when ACTION_SELECT is pressed on the Go To Button.
 * This is not standard OL, as the default for a Menu Button is
 * usually the value of the default item on the menu.
 */
static void
fc_goto_btn_event( item, event )
     Panel_button_item	item;
     Event *		event;
{
    Fc_private *private = (Fc_private *)xv_get(item, XV_KEY_DATA, FC_KEY);

    /*
     * Set a flag that denotes that the History Menu callback
     * will be envoked by way of an ACTION_SELECT rather than
     * by ACTION_MENU.
     */
    if ( event_action(event) == ACTION_SELECT )
	private->goto_select = event_is_up(event);

    /* ASSume the default handler for PANEL_BUTTON */
    panel_default_handle_event(item, event);
} /* fc_goto_btn_event() */




/*
 * History Menu callback
 */
static void
fc_hist_notify( hm, label, value )
     History_menu hm;
     char *label;
     char *value;
{
    Fc_private *private = (Fc_private *)xv_get(hm, XV_KEY_DATA, FC_KEY);
    char *path = (char *) xv_get(private->ui.goto_txt, PANEL_VALUE);


    /*
     * LAF Note:  This check is done to make the File Chooser's
     * Go To button opperate like the File Manager's Go To button.
     * i.e. if you press SELECT on it and there is a path in the
     * Go To typein, than go there; else go to the default menu
     * item.
     */
    if ( private->goto_select && !no_string(path) ) {
	Event event;
	void (*event_proc)() 
	    = (void (*)()) xv_get(private->ui.goto_txt, PANEL_EVENT_PROC);

	/*
	 * Make the PATH_NAME object think the user pressed
	 * <Return> on it.  Cheezy though this may look, it's
	 * the best way to get the desired behavior without
	 * running into validation problems.
	 */
	event_init( &event );
	event_set_action(&event, '\r');
	if ( event_proc )
	    (* event_proc)( private->ui.goto_txt, &event );
	private->goto_select = FALSE;
    } else {
	
	/*
	 * Selection made off of Goto Menu, change to
	 * the new directory.  Note that the rest of the
	 * dialog will get updated from the cd-func.
	 */
	xv_set(private->ui.list,
	       FILE_LIST_DIRECTORY,	value,
	       NULL);

	/* blank out typein as does FileManager */
	xv_set(private->ui.goto_txt, PANEL_VALUE, "", NULL);
    }
} /* fc_hist_notify() */






/*
 * Goto Typein callback.  If value is directory, go there.
 * if value is file, go there and hilight file.
 */
static Panel_setting
fc_goto_notify( item, event, sbuf )
     Path_name item;
     Event *event;
     struct stat *sbuf;
{
    Fc_private *private = (Fc_private *)xv_get(item, XV_KEY_DATA, FC_KEY);
    char *path = (char *) xv_get(item, PATH_LAST_VALIDATED);
    char *dir;
    char *file;
    int row;


    if ( no_string(path) || !sbuf ) {
	/* should this print an error message? */
	return panel_text_notify(item, event);
    }


    /*
     * If the value is a directory, then goto the directory.  The
     * rest of the dialog will be updated in the CD-func.
     */
    if ( S_ISDIR(sbuf->st_mode) ) {
	xv_set(private->ui.list,
	       FILE_LIST_DIRECTORY, path,
	       NULL);

	/* blank out typein as does FileManager */
	xv_set( private->ui.goto_txt, PANEL_VALUE, "", NULL );
	return panel_text_notify(item, event);
    }



    /*
     * Separate path into dir and file name.
     */
    dir = xv_dirpart( path );
    file = xv_basepart( path );

    /* display directory */
    if ( !strequal(dir, (char *)xv_get(private->ui.list, FILE_LIST_DIRECTORY)) )
	xv_set(private->ui.list,
	       FILE_LIST_DIRECTORY,	dir,
	       PANEL_PAINT, PANEL_NONE,
	       NULL);


    /*
     * In a Save or Save As dialog, the files are all inactive,
     * and can't be selected.  This is not considered an error.
     */
    if ( private->type != FILE_CHOOSER_OPEN ) {
	xv_set( private->ui.goto_txt, PANEL_VALUE, "", NULL );
	xv_error_sprintf(FC_PUBLIC(private),
			 TRUE, 
			 XV_MSG("Type the name of the file in the Save field.")
			 );
	return panel_text_notify(item, event);
    }


    /* 
     * See if file is in list.
     */
    row = xv_get(private->ui.list, PANEL_LIST_NROWS) - 1;
    while ( row >= 0 ) {
	char *str = (char *) xv_get(private->ui.list, PANEL_LIST_STRING, row);
	if ( strequal(file, str) )
	    break;
	row--;
    }

    /*
     * if file is in list, select it.  else
     * notify user of error.
     */
    if ( row >= 0 ) {
	xv_set(private->ui.list, PANEL_LIST_SELECT, row, TRUE, NULL);
	fc_update_dimming( private, row );

	/* blank out typein as does FileManager */
	xv_set( private->ui.goto_txt, PANEL_VALUE, "", NULL );
    } else {
	/* shouldn't happen since we've already stat'd it! */
	xv_error_sprintf(FC_PUBLIC(private), TRUE, XV_MSG("%s does not exist!"), file);
    }

    xv_free_ref( dir );
    xv_free_ref( file );

    return panel_text_notify(item, event);
} /* fc_goto_notify() */




/*
 * PANEL_NOTIFY_PROC for File List.
 */
static int
fc_list_notify( item, dir, file, client_data, op, event, row )
     File_list item;
     char *dir;
     char *file;
     Xv_opaque client_data;
     Panel_list_op op;
     Event *event;
     int row;
{
    Fc_private *private = (Fc_private *)xv_get(item, XV_KEY_DATA, FC_KEY);
    int status;

    switch( op ) {
    case PANEL_LIST_OP_DBL_CLICK:
	if ( private->type == FILE_CHOOSER_OPEN )
	    status = fc_do_open( private, row, dir, file, client_data );
#ifndef DBL_CLICK_FOR_SAVE
	else
	    status = XV_ERROR;	/* shouldn't happen */
#else
	/*
	 * New OL directive in the final draft of the OLFCS says
	 * that double-click is *not* an accelerator for saving a
	 * document; and, in fact, add documents should be gray'ed
	 * out in a Save or Save As dialog.
	 */
	else {
	   File_list_row_type type;

	    type = (File_list_row_type) xv_get(private->ui.list, FILE_LIST_ROW_TYPE, row);
	    if ( type != FILE_LIST_FILE_TYPE ) {
		xv_error_sprintf(FC_PUBLIC(private), TRUE, XV_MSG("Selection is not a document!"));
		status = XV_OK;
	    } else
		status = fc_do_save( private, dir, file );
	}
#endif /* DBL_CLICK_FOR_SAVE */

	/* if operation was successful, dismiss the dialog */
	if ( status == XV_OK )
	    xv_set( FC_PUBLIC(private),
		   XV_SHOW,		FALSE,
		   NULL );
	break;

    case PANEL_LIST_OP_SELECT:
	fc_update_dimming( private, row );
	break;

    case PANEL_LIST_OP_DESELECT:
    case PANEL_LIST_OP_VALIDATE:
    case PANEL_LIST_OP_DELETE:
	break;
    }

    return XV_OK;
} /* fc_list_notify() */





/*
 * Selected string changed.  Make sure "Open Folder"
 * button is dimmed/undimmed accordingly.
 */
static void
fc_update_dimming( private, row )
     Fc_private *private;
     int row;
{
    File_list_row_type type;


    if ( (private->type == FILE_CHOOSER_OPEN) 
	&& ( (private->custom == FILE_CHOOSER_SELECT_ALL)
	    || (private->custom == XV_ZERO)
	    )
	)
	return;


    type = (File_list_row_type) xv_get(private->ui.list, 
				       FILE_LIST_ROW_TYPE, 
				       row
				       );

    if ( private->custom )
	fc_item_inactive( private->ui.custom_btn, 
			 !(type == FILE_LIST_FILE_TYPE)
			 );

    fc_item_inactive( private->ui.open_btn, 
		     (type == FILE_LIST_FILE_TYPE) 
		     );
} /* fc_update_dimming() */




/*
 * Change the items PANEL_INACTIVE flag, first checking
 * to see if the state changed in order to save the extra
 * repaint.
 */
static void
fc_item_inactive( item, state )
     Panel_item item;
     int	state;
{
    int current_state = (int) xv_get(item, PANEL_INACTIVE);

    if ( current_state != state )
	xv_set(item, PANEL_INACTIVE, state, NULL);
} /* fc_item_inactive() */





/*
 * PANEL_EVENT_PROC for Document Name Typein
 *
 * LAF Note:  Save button is supposed to gray-out if document
 * field is empty.
 */
static void
fc_document_txt_event( item, event )
     Panel_text_item	item;
     Event *		event;
{
    Fc_private *private = (Fc_private *)xv_get(item, XV_KEY_DATA, FC_KEY);
    char *value;

    if ( private->document_default_event )
	(* private->document_default_event)( item, event );

    value = (char *)xv_get(item, PANEL_VALUE);

    /*
     * BUG ALERT:  Even an inactive textfield seems to get
     * some events.  Why! Why! Why!
     */
    if ( !private->save_to_dir )
	fc_item_inactive(private->ui.save_btn, no_string(value));
} /* fc_document_txt_event() */





/*
 * Perform the "Open" operation.  Called if Open button
 * picked, or double-click in Open dialog.
 *
 * Note:  return of XV_OK means to dismiss the popup.  we
 * only want to dismiss the dialog when a file was saved
 * successfully.
 */
static int
fc_do_open( private, row, dir, file, client_data )
     Fc_private *private;
     int row;
     char *dir;
     char *file;
     Xv_opaque client_data;
{
    File_list_row_type row_type 
	= (File_list_row_type) xv_get(private->ui.list, FILE_LIST_ROW_TYPE, row);
    char *path;
    int status = XV_ERROR;


    /*
     * Pressed "Open" on a folder, load the directory.
     * Note:  File List is unsophisticated about path expansion.
     * it can, however, deal with a relative name by appending it
     * to it's current path, or ".." by truncating it's current
     * path.
     */
    if ( private->custom != FILE_CHOOSER_SELECT_ALL ) {
	if ( row_type == FILE_LIST_DIR_TYPE ) {
	    xv_set(private->ui.list, FILE_LIST_DIRECTORY, file, NULL);
	    return XV_ERROR;
	} else if ( row_type == FILE_LIST_DOTDOT_TYPE ) {
	    xv_set(private->ui.list, FILE_LIST_DIRECTORY, "..", NULL);
	    return XV_ERROR;
	}
    }
    
    /*
     * In Open Folder, attempt to opening a file is an error.
     */
    if ( private->type != FILE_CHOOSER_OPEN )
	return XV_ERROR;


    /* ASSume success */
#ifdef OW_I18N
    if ( !private->notify_func_wcs && !private->notify_func )
	return XV_OK;
#else
    if ( !private->notify_func )
	return XV_OK;
#endif /* OW_I18N */


    path = xv_dircat( dir, file );


    /*
     * Make sure path is readable by user.
     */
    if ( !private->no_confirm && (xv_access(path, R_OK) == -1) ) {
	if ( !private->ui.notice )
	    private->ui.notice = xv_create(FC_PUBLIC(private), NOTICE, NULL);
	xv_set(private->ui.notice,
	       NOTICE_BUTTON_NO,	XV_MSG("Cancel"),
	       NOTICE_MESSAGE_STRINGS,
	       		XV_MSG("You cannot open the file:"),
	       		"\n",
	       		path,
	       		"\n",
	       		XV_MSG("because you do not have permission to read it."),
	       		XV_MSG("Only the owner of the file can change permissions."),
	       		NULL,
	       XV_SHOW,	TRUE,
	       NULL);
	xv_free_ref( path );
	return XV_ERROR;
    }

#ifdef OW_I18N
    if ( private->notify_func_wcs ) {
	wchar_t *wpath = _xv_mbstowcsdup( path );
	wchar_t *wfile = _xv_mbstowcsdup( file );

	status = (* private->notify_func_wcs)( FC_PUBLIC(private), wpath, wfile, client_data );
	xv_free( wpath );
	xv_free( wfile );
    } else if ( private->notify_func )
#endif
    status = (* private->notify_func)( FC_PUBLIC(private), path, file, client_data );

    xv_free_ref( path );

    return status;
} /* fc_do_open() */





/*
 * Notify user to save the given path.  Called if Save button
 * picked or double-click in Save dialog.
 *
 * Note:  return of XV_OK means to dismiss the popup.  we
 * only want to dismiss the dialog when a file was saved
 * successfully.
 */
static int
fc_do_save( private, dir, file )
     Fc_private *private;
     char *dir;
     char *file;
{
    char *path;
    int status = XV_ERROR;
    int canceled = NOTICE_NO;
    struct stat *exists = (struct stat *)NULL;
    struct stat sbuf;
    char *access_dir = (char *)NULL;
    int have_file_name = !no_string(file);


    /* ASSume success */
#ifdef OW_I18N
    if ( !private->notify_func && !private->notify_func_wcs )
	return XV_OK;
#else
    if ( !private->notify_func )
	return XV_OK;
#endif




    /*
     * Make sure the user has write permission on 
     * the directory.
     */
    if ( !have_file_name || is_relative(file) )
	access_dir = xv_strcpy(NULL, dir);
    else if ( dir )
	access_dir = xv_dirpart( file );
    else
	return XV_OK;

    if ( xv_access(access_dir, W_OK) == -1 ) {
	char *file_part = xv_basepart( access_dir );
	xv_error_sprintf( FC_PUBLIC(private), 
			 TRUE, 
			 XV_MSG("You do not have permission to write to folder \"%s\""),
			 file_part
			 );
	xv_free_ref( file_part );
	xv_free_ref( access_dir );
	return XV_ERROR;
    }
    xv_free_ref( access_dir );





    if ( !have_file_name && !private->save_to_dir ) {
	xv_error_sprintf( FC_PUBLIC(private), 
			 TRUE, 
			 XV_MSG("Please enter a name in the Save: field.")
			 );
	return XV_ERROR;
    }





    /*
     * Decide what path name to send to the callback
     */
    if ( private->save_to_dir )
	/* ignore the field contents */
	path = xv_strcpy(NULL, dir);
    else if ( is_relative(file) )
	/* std case, name in save typein */
	path = xv_dircat( dir, file );
    else
	/* absolute path name in save typein */
	path = xv_strcpy(NULL, file);




    /*
     * if file exists, allow user to overwrite.
     */
    if ( !private->no_confirm
	&& (xv_stat(path, &sbuf) == 0) 
	&& !private->save_to_dir 
	) {
	exists = &sbuf;
	canceled = fc_confirm_overwrite( private, path, file, &sbuf );
    }



    /*
     * Canceled will be NOTICE_YES if the user picked "Cancel"
     * from the notice displayed if the file already exists.
     */
    if ( canceled == NOTICE_NO ) {
#ifdef OW_I18N
	if ( private->notify_func_wcs ) {
	    wchar_t *wpath = _xv_mbstowcsdup( path );
	    status = (* private->notify_func_wcs)( FC_PUBLIC(private), wpath, exists );
	    xv_free( wpath );
	} else if ( private->notify_func )
#endif
	status = (* private->notify_func)( FC_PUBLIC(private), path, exists );
    }

    xv_free_ref( path );

    return status;
} /* fc_do_save() */




/*
 * Confirm that the user can overwrite an existing file.
 */
static int
fc_confirm_overwrite( private, path, file, sbuf )
     Fc_private *private;
     char *path;
     char *file;
     struct stat *sbuf;
{
    Xv_notice notice;
    int canceled = NOTICE_NO;
    char buf[128];
    char buf2[128];
    uid_t uid = geteuid();	/* don't cache, might change over lifetime of app */
    gid_t gid = getegid();	/* don't cache, might change over lifetime of app */



    if ( !private->ui.notice )
	private->ui.notice = xv_create(FC_PUBLIC(private), NOTICE, NULL);



    /*
     * Some snapperhead wants to overwrite a directory name
     * with a file name...
     */
    if ( S_ISDIR(sbuf->st_mode) ) {
	xv_set(private->ui.notice,
	       NOTICE_STATUS,		&canceled,
	       NOTICE_BUTTON_YES,	XV_MSG("Cancel"),
	       NOTICE_MESSAGE_STRINGS,
	       		XV_MSG("A folder already exists with this name:"),
			"\n",
			file,
			"\n",
			XV_MSG("You cannot overwrite a folder name with a"),
			XV_MSG("file name.  Please choose a different name."),
			NULL,
	       NULL);
    }



    /*
     * Check to see if user can write to file.
     */
    else if ( (sbuf->st_uid == uid && sbuf->st_mode & S_IWUSR) 		/* write user */
	     || (sbuf->st_gid == gid && sbuf->st_mode & S_IWGRP)	/* write group */
	     || (sbuf->st_mode & S_IWOTH) 				/* write anyone */
	) {
	xv_set(private->ui.notice,
	       NOTICE_BUTTON_YES,	XV_MSG("Cancel"),
	       NOTICE_BUTTON_NO,	XV_MSG("Overwrite Existing File"),
	       NOTICE_STATUS,		&canceled,
	       NULL);

	if ( is_absolute(file) ) {
	    xv_set( private->ui.notice,
		   NOTICE_MESSAGE_STRINGS,
		   	XV_MSG("This file already exists:"),
			file,
		   	"\n",
		   	XV_MSG("Do you want to overwrite the existing file?"),
		   	NULL,
		   NULL);
	} else {
	    (void) sprintf(buf,
			   XV_MSG("The file \"%s\" already exists."),
			   file 
			   );
	    (void) sprintf(buf2, 
			   XV_MSG("Do you want to overwrite the existing file \"%s\"?"),
			   file 
			   );
	    xv_set( private->ui.notice, 
		   NOTICE_MESSAGE_STRINGS,
		   	buf,
		   	buf2,
		   	NULL,
		   NULL);
	}
    } /* can write to file */




    /*
     * File is read-only.  does user own it?
     */
    else if ( (sbuf->st_uid == uid) && (sbuf->st_mode & S_IRUSR) ) {
	xv_set(private->ui.notice,
	       NOTICE_BUTTON_YES,	XV_MSG("Cancel"),
	       NOTICE_BUTTON_NO,	XV_MSG("Overwrite Existing File"),
	       NOTICE_STATUS,		&canceled,
	       NULL);

	if ( is_absolute(file) ) {
	    xv_set( private->ui.notice,
		   NOTICE_MESSAGE_STRINGS,
		   	XV_MSG("This file already exists and is read-only:"),
			file,
		   	"\n",
		   	XV_MSG("Do you want to overwrite the existing file?"),
		   	NULL,
		   NULL);
	} else {
	    (void) sprintf(buf,
			   XV_MSG("The file \"%s\" already exists and is read-only."),
			   file 
			   );
	    (void) sprintf(buf2, 
			   XV_MSG("Do you want to overwrite the file \"%s\"?"),
			   file 
			   );
	    xv_set( private->ui.notice, 
		   NOTICE_MESSAGE_STRINGS,
		   	buf,
		   	buf2,
		   	NULL,
		   NULL);
	}
    } /* owned by user */



    /*
     * Cannot overwrite file.
     */
    else {
	xv_set(private->ui.notice,
	       NOTICE_BUTTON_YES,	XV_MSG("Cancel"),
	       NOTICE_STATUS,		&canceled,
	       NOTICE_MESSAGE_STRINGS,
	       		XV_MSG("You cannot save to the file:"),
			"\n",
			path,
			"\n",
			XV_MSG("because you do not have permission to write to"),
			XV_MSG("the file.  Only the owner can change permissions."),
			NULL,
	       NULL);
    } /* can't overwrite file */

    
    xv_set(private->ui.notice, XV_SHOW, TRUE, NULL);

    return canceled;
} /* fc_confirm_overwrite() */





/*
 * FILE_LIST_FILTER_FUNC turns around and calls client's filter
 * func with modified parameters.  Since the File List enforces
 * little policy as to what changes the filter func can make to
 * the row values, it's up to the FILE_CHOOSER to make sure that
 * the client doesn't stray too far from Open Look guidelines.
 * Besides, they always have the File List...
 */
static File_list_op
fc_filter_func( path, row )
     char *path;
     File_list_row *row;
{
    Fc_private *private = (Fc_private *)xv_get(row->file_list, XV_KEY_DATA, FC_KEY);
    Xv_opaque client_data = XV_NULL;
    Server_image glyph = XV_NULL;
    Server_image mask_glyph = XV_NULL;
    File_chooser_op status;
    int notify_client = FALSE;


    /*
     * This is cop'ed from the File List code.  The problem is that,
     * for a Save or Save As dialog, i have to automatically set inactive
     * all documents.  To do this, i get called back for each entry, then
     * monitor the mask myself.  Perhaps when i have time, i should find
     * a way around this, at least for the Open dialog.
     */
    if ( is_dotdot(path) ) {
	if ( private->filter_mask & FC_DOTDOT_MASK )
	    notify_client = TRUE;
    } else 
	if ( (S_ISDIR(row->stats.st_mode) 
	      && ( ( (private->filter_mask & FC_MATCHED_DIRS_MASK) 
		    && row->matched == FILE_LIST_MATCHED
		    )
		  || ((private->filter_mask & FC_NOT_MATCHED_DIRS_MASK) 
		      && row->matched == FILE_LIST_NOT_MATCHED
		      )
		  )
	      ) || 
	    ( !S_ISDIR(row->stats.st_mode)
	       && ( ((private->filter_mask & FC_MATCHED_FILES_MASK) 
		     && row->matched == FILE_LIST_MATCHED 
		     )
		   || ((private->filter_mask & FC_NOT_MATCHED_FILES_MASK) 
		       && row->matched == FILE_LIST_NOT_MATCHED
		       )
		   )
	     )
	    )
	notify_client = TRUE;
    
    if ( notify_client && private->filter_func ) {
#ifdef OW_I18N
	if ( private->wchar_notify ) {
	    wchar_t *wpath = _xv_mbstowcsdup( path );
	    status = (* private->filter_func) ( FC_PUBLIC(private),
					       wpath,
					       row->stats,
					       (File_chooser_op) row->matched,
					       &glyph,
					       &client_data,
					       &mask_glyph
					       );
	    xv_free( wpath );
	} else
#endif
	status = (* private->filter_func) ( FC_PUBLIC(private),
					   path,
					   row->stats,
					   (File_chooser_op) row->matched,
					   &glyph,
					   &client_data,
					   &mask_glyph
					   );

	/*
	 * Note:  this is here because, while the FILE_LIST
	 * assumes you can override this flag in the filter func,
 	 * the FILE_CHOOSER does not give access.  thus, allow it
	 * to override it via the return value.
	 *
	 * this may still get changed below for the Save/As case...
	 */
	if ( status == FILE_CHOOSER_ACCEPT )
	    row->vals.inactive = FALSE;

	if ( glyph ) {
	    row->vals.glyph = glyph;
	    row->vals.mask_glyph = mask_glyph;
	}
	row->vals.client_data = client_data;
    } else {
	/*
	 * if it got inactiveated because of the regex, ignore it.  Note:
	 * directories may match, but are not set inactive by default.  Hence,
	 * FILE_CHOOSER_ABBREV_VIEW works as expected.
	 */
	status = (row->vals.inactive) ? FILE_LIST_IGNORE : FILE_LIST_ACCEPT;
    }


#ifndef DBL_CLICK_FOR_SAVE
    /*
     * See comment in fc_list_notify(), OL wants all documents
     * in a Save or Save As dialog grayed out.  I still allow a
     * filter func in case the client wants to add glyphs to the
     * grayed out documents.  Note that doing this will prevent
     * double click event from happening on any documents.
     *
     * because of this, FILE_CHOOSER_ABBREV_VIEW attribute should
     * not be set on a Save or Save As dialog...
     */
    if ( (private->type != FILE_CHOOSER_OPEN) 
	&& !S_ISDIR(row->stats.st_mode) 
	) {
	row->vals.inactive = TRUE;
	if ( !(notify_client && private->filter_func) )
	    status = FILE_LIST_IGNORE;
    }
#endif

    return (File_list_op) status;
} /* fc_filter_func() */





/*
 * FILE_LIST_COMPARE_FUNC.  As above, the File List enforces
 * next to no policy on this, so the File Chooser tries to see
 * that Open Look isn't thrown too far out the window.  If they
 * don't like this, they can always access the File List directly.
 */
static int
fc_compare_func( row1, row2 )
     File_list_row *row1;
     File_list_row *row2;
{
    Fc_private *private 
	= (Fc_private *) xv_get(row1->file_list, XV_KEY_DATA, FC_KEY);
    int status;
    File_chooser_row fc_row1;
    File_chooser_row fc_row2;



    /*
     * Note from 'man qsort': "The relative order in the output of
     * two items that compare as equal is unpredictable."  In this
     * case it appears to be the order in which they were read in,
     * so this seems reasonable.
     */
    if ( !private->compare_func )
	return 0;


#ifdef OW_I18N
    if ( private->wchar_notify ) {
	fc_row1.file = (char *)_xv_mbstowcsdup( row1->vals.string );
	fc_row2.file = (char *)_xv_mbstowcsdup( row2->vals.string );
    } else {
#endif
	fc_row1.file = row1->vals.string;
	fc_row2.file = row2->vals.string;
#ifdef OW_I18N
    }
#endif

    fc_row1.stats = &(row1->stats);
    fc_row1.matched = (File_chooser_op) row1->matched;
    fc_row1.xfrm = row1->xfrm;

    fc_row2.stats = &(row2->stats);
    fc_row2.matched = (File_chooser_op) row2->matched;
    fc_row2.xfrm = row2->xfrm;

    status = (* private->compare_func)( &fc_row1, &fc_row2 );

#ifdef OW_I18N
    if ( private->wchar_notify ) {
	xv_free( fc_row1.file );
	xv_free( fc_row2.file );
    }
#endif

    return status;
} /* fc_compare_func() */




/*
 * Built in sort comparisons.
 */
Xv_public int
fchsr_no_case_ascend_compare( row1, row2 )
     File_chooser_row *row1;
     File_chooser_row *row2;
{
    return xv_strcasecmp(row1->xfrm, row2->xfrm);
}

Xv_public int
fchsr_no_case_descend_compare( row1, row2 )
     File_chooser_row *row1;
     File_chooser_row *row2;
{
    return xv_strcasecmp(row2->xfrm, row1->xfrm);
}

Xv_public int
fchsr_case_ascend_compare( row1, row2 )
     File_chooser_row *row1;
     File_chooser_row *row2;
{
    return strcmp(row1->xfrm, row2->xfrm);
}

Xv_public int
fchsr_case_descend_compare( row1, row2 )
     File_chooser_row *row1;
     File_chooser_row *row2;
{
    return strcmp(row2->xfrm, row1->xfrm);
}

/*----------------------------------------------------------------------------*/
