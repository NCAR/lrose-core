#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_attr.c 20.127 93/04/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Attribute set/get routines for text subwindows.
 */

#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <xview_private/txt_18impl.h>
#ifdef SVR4
#include <dirent.h>
#include <string.h>
#else
#include <sys/dir.h>
#endif /* SVR4 */
#include <pixrect/pixfont.h>
#include <xview/window.h>
#include <xview/openmenu.h>
#include <xview/defaults.h>
#include <xview_private/ev_impl.h>
#include <xview_private/draw_impl.h>

Pkg_private int ei_plain_text_line_height();
Pkg_private void     ev_line_info();
Pkg_private Es_handle es_file_create(), es_mem_create();
Pkg_private Es_handle textsw_create_ps();
Pkg_private void textsw_display_view_margins();
Pkg_private void textsw_init_again(), textsw_init_undo();
Pkg_private Es_status textsw_load_file_internal();
Pkg_private Textsw_index textsw_position_for_physical_line();
Pkg_private Textsw_index textsw_replace();
Pkg_private Es_index textsw_get_contents();
Pkg_private void  textsw_view_cms_change();
#ifdef OW_I18N
Pkg_private Es_index textsw_get_contents_wcs();
#endif /* OW_I18N */
pkg_private void textsw_resize();
Xv_private char *xv_font_monospace();

Xv_private PIXFONT *xv_pf_open();
Xv_private int      xv_pf_close();

#ifndef CTRL
#ifndef __STDC__
#define CTRL(c)		('c' & 037)
#else /* __STDC__ */
#define CTRL(c)		(c & 037)
#endif /* __STDC__ */
#endif
#define	DEL		0x7f

#define SET_BOOL_FLAG(flags, to_test, flag)			\
	if ((unsigned)(to_test) != 0) (flags) |= (flag);	\
	else (flags) &= ~(flag)

#define BOOL_FLAG_VALUE(flags, flag)				\
	((flags & flag) ? TRUE : FALSE)


static          Textsw_status
set_first(view, error_msg, filename, reset_mode, first, first_line, all_views)
    register Textsw_view_handle view;
    char           *error_msg;
    CHAR           *filename;
    int             reset_mode;
    Es_index        first;
    int             first_line;
    int             all_views;
{
    char            msg_buf[MAXNAMLEN + 100];
    char           *msg;
    Es_status       load_status;
    Textsw_status   result = TEXTSW_STATUS_OKAY;
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
#ifdef OW_I18N
    Pkg_private void     textsw_normalize_view_wc();
#else
    Pkg_private void     textsw_normalize_view();
#endif

    msg = (error_msg) ? error_msg : msg_buf;
#ifdef OW_I18N
    if (filename && (STRLEN(filename) > 0)) {	/* } for match */
#else
    if (filename) {
#endif
	CHAR            scratch_name[MAXNAMLEN];
	Es_handle       new_esh;
#ifdef OW_I18N
	char           *filename_mb;
#endif
	/* Ensure no caret turds will leave behind */
	textsw_take_down_caret(folio);
	load_status =
	    textsw_load_file_internal(
		     folio, filename, scratch_name, &new_esh, ES_CANNOT_SET,
				      1);
	if (load_status == ES_SUCCESS) {
#ifdef OW_I18N
	    SET_CONTENTS_UPDATED(folio, TRUE);
	    if ((int) es_get((Es_handle)
				es_get(new_esh, ES_PS_ORIGINAL), ES_SKIPPED))
		textsw_invalid_data_notice(view, filename, 1);
#endif /* OW_I18N */
	    if (first_line > -1) {
		first = textsw_position_for_physical_line(
				     VIEW_REP_TO_ABS(view), first_line + 1);
	    }
	    if (reset_mode != TEXTSW_LRSS_CURRENT) {
		(void) ev_set(view->e_view,
			      EV_FOR_ALL_VIEWS,
			      EV_DISPLAY_LEVEL, EV_DISPLAY_NONE,
			      EV_DISPLAY_START, first,
			      EV_DISPLAY_LEVEL, EV_DISPLAY,
			      NULL);
	    }
#ifdef OW_I18N
	    filename_mb = _xv_wcstombsdup(filename);
	    textsw_notify(view,
			  TEXTSW_ACTION_LOADED_FILE, filename_mb,
			  TEXTSW_ACTION_LOADED_FILE_WCS, filename, NULL);
	    if (filename_mb)
		free(filename_mb);
#else
	    textsw_notify(view,
			  TEXTSW_ACTION_LOADED_FILE, filename, NULL);
#endif
	} else {
	    textsw_format_load_error(msg, load_status,
				     filename, scratch_name);
	    if (error_msg == NULL)
		textsw_post_error((Textsw_opaque) folio, 0, 0, msg, NULL);
	    result = TEXTSW_STATUS_OTHER_ERROR;
	}
    } else {
	if (first_line > -1) {
	    first = textsw_position_for_physical_line(
				     VIEW_REP_TO_ABS(view), first_line + 1);
	}
	if (first != ES_CANNOT_SET) {
	    if (all_views) {
		Textsw_view_handle view_ptr;
		for (view_ptr = folio->first_view; view_ptr; view_ptr = view_ptr->next) {
		    textsw_normalize_internal(view_ptr, first, first, 0, 0,
					      TXTSW_NI_DEFAULT);
		};

	    } else
#ifdef OW_I18N
		textsw_normalize_view_wc(VIEW_REP_TO_ABS(view), first);
#else
		textsw_normalize_view(VIEW_REP_TO_ABS(view), first);
#endif
	} else {
	    result = TEXTSW_STATUS_OTHER_ERROR;
	}
    }
    return (result);
}


Pkg_private void
textsw_set_null_view_avlist(folio, attrs)
    register Textsw_folio folio;
    Attr_avlist     attrs;
{
    Attr_attribute  avarray[ATTR_STANDARD_SIZE];
    Attr_attribute *view_attrs = avarray;

    /*
     * consume the view attrs from attrs, consume the non-view attrs from
     * view_attrs.
     */
    (void) attr_copy_avlist(view_attrs, attrs);
    for (; *view_attrs; view_attrs = attr_next(view_attrs),
	 attrs = attr_next(attrs)) {

	switch (*view_attrs) {
	  case TEXTSW_AUTO_SCROLL_BY:
	  case TEXTSW_CONTENTS:
	  case TEXTSW_FILE_CONTENTS:
	  case TEXTSW_FILE:
	  case TEXTSW_FIRST:
#ifdef OW_I18N
	  case TEXTSW_CONTENTS_WCS:
	  case TEXTSW_FILE_CONTENTS_WCS:
	  case TEXTSW_FILE_WCS:
	  case TEXTSW_INSERT_FROM_FILE_WCS:
	  case TEXTSW_FIRST_WC:
#endif /* OW_I18N */
	  case TEXTSW_FIRST_LINE:
	  case TEXTSW_INSERT_FROM_FILE:
	  case TEXTSW_LINE_BREAK_ACTION:
	  case TEXTSW_LOWER_CONTEXT:
	  case TEXTSW_NO_REPAINT_TIL_EVENT:
	  case TEXTSW_RESET_TO_CONTENTS:
	  case TEXTSW_UPPER_CONTEXT:
	  case XV_LEFT_MARGIN:
	  case XV_RIGHT_MARGIN:
	    /*
	     * these are all view attrs, so consume from attrs.
	     */
	    ATTR_CONSUME(*attrs);
	    break;

	  case TEXTSW_STATUS:
	    /*
	     * this applies to both the view and the folio, so leave it
	     * alone.
	     */
	    break;

	  default:
	    /*
	     * must be a non-view attr, so consume from view_attrs.
	     */
	    ATTR_CONSUME(*(Attr_avlist) view_attrs);
	    break;
	}
    }
    /* now apply the view attrs to the view */
    xv_set(FOLIO_REP_TO_ABS(folio),
	   OPENWIN_VIEW_ATTRS, ATTR_LIST, avarray, 0,
	   NULL);

    /* now attrs has no view attrs left uncomsumed in it */
}

#ifdef OW_I18N
#define	IC_ACTIVE_TRUE		1
#define	IC_ACTIVE_FALSE		2
#endif

Pkg_private     Textsw_status
textsw_set_internal(textsw, view, attrs, is_folio)
    register Textsw_folio textsw;
/* folio private handle */
    Textsw_view_handle view;
    Attr_attribute *attrs;
    int             is_folio;
/*
 * TRUE= we're setting attr's on the textsw_folio FALSE= we're setting attr's
 * on the textsw_view
 */
{
    Textsw          textsw_public = TEXTSW_PUBLIC(textsw);
    Textsw_status   status, status_dummy;
    Textsw_status  *status_ptr = &status_dummy;
    Textsw_view_handle next;
    char            error_dummy[256];
    char           *error_msg = error_dummy;
#ifdef OW_I18N
    CHAR            file[MAXPATHLEN];
    CHAR            name_wc[MAXPATHLEN];
#ifdef FULL_R5
    XVaNestedList   va_nested_list;
#endif /* FULL_R5 */
#else /* OW_I18N */
    char           *file = NULL;
    /* Added because it is used on both sides of OW_I18N ifdef. */
    /* When OW_I18n is not defined, it is an unused parameter */
    /* of textsw_set_internal_tier2. */
    char            *name_wc = NULL;
#endif /* OW_I18N */
    int             reset_mode = -1;
    register int    temp;
    int             all_views = 0;
    int             display_views = 0;
    int             update_scrollbar = 0;
    int             read_only_changed = 0;
    int             read_only_start;
    int		    row_height;
    int             set_read_only_esh = 0;
    int             str_length = 0;
    Es_handle       ps_esh, scratch_esh, mem_esh;
    Es_status       es_status;
    Es_index        tmp;
    extern	    Textsw_status textsw_set_internal_tier2();
/*
 * NOTE: This line of code  is no longer used
 * Remove it after a suitable grace period
    register int    consume_attrs = 0;
 */

#ifdef OW_I18N
    int		    ic_active_changed = 0;
    extern CHAR     _xv_null_string_wc[];

    file[0] = NULL;
#endif
    /*
     * This code will not handle view attr if view is null.
     */

    status = TEXTSW_STATUS_OKAY;
    for (; *attrs; attrs = attr_next(attrs)) {

	switch (*attrs) {
	  case XV_END_CREATE:
	    if (view) {
		row_height = ei_plain_text_line_height(textsw->views->eih);
		if (is_folio) {
		    if (defaults_get_boolean("text.enableScrollbar",
					     "Text.EnableScrollbar", True)) {
			/* Create a scrollbar as a child of the textsw. */
			view->scrollbar = xv_create(textsw_public,
			    SCROLLBAR, NULL);
			textsw_setup_scrollbar(view->scrollbar);
			view->state &= ~TXTSW_SCROLLBAR_DISABLED;
		    } else {
			view->scrollbar = XV_ZERO;
			view->state |= TXTSW_SCROLLBAR_DISABLED;
		    }
		    /*
		     * Set WIN_ROW_HEIGHT to the height of a line so that
		     * a subsequent xv_set of WIN_ROWS will work.
		     */
		    xv_set(textsw_public,
			   WIN_ROW_HEIGHT, row_height,
			   NULL);
		} else {
		    xv_set(VIEW_PUBLIC(view),
			   WIN_ROW_HEIGHT, row_height,
			   NULL);
		}

		/* don't want things to autoclear for us! jcb 5/15/90 */
		(void) xv_set(textsw_public, OPENWIN_AUTO_CLEAR, FALSE, NULL);

#ifdef OW_I18N
		if (textsw->need_im) {
		    Textsw_view		view_public = VIEW_PUBLIC(view);
                    Xv_Drawable_info    *info;
 
                    DRAWABLE_INFO_MACRO(view_public, info);
 
		    /*
		     * create IC, even read only case. Because the mode may be
		     * changed to Edit from Read-only mode later on.
		     */
		    textsw->ic = (XIC)xv_get(textsw_public, WIN_IC);

		    if (textsw->ic) {
			xv_set(view_public, WIN_IC, textsw->ic, NULL);
#ifdef FULL_R5		
		        XGetICValues(textsw->ic, XNInputStyle, &textsw->xim_style, NULL);
                        window_set_ic_focus_win(view_public, textsw->ic, xv_xid(info));
#endif /* FULL_R5 */					
		    }
		    if (!textsw->ic || TXTSW_IS_READ_ONLY(textsw) ||
			    xv_get(textsw_public, WIN_IC_ACTIVE) == FALSE)
			xv_set(view_public, WIN_IC_ACTIVE, FALSE, NULL);
		}
		else
		    xv_set(VIEW_PUBLIC(view), WIN_IC_ACTIVE, FALSE, NULL);
#endif /* OW_I18N */

		textsw_resize(view);
	    }
	    break;
	  case TEXTSW_AGAIN_LIMIT:
	    /* Internally we want one more again slot that client does.  */
	    temp = (int) (attrs[1]);
	    temp = (temp > 0) ? temp + 1 : 0;
	    textsw_init_again(textsw, temp);
	    break;
	  case TEXTSW_AGAIN_RECORDING:
	    SET_BOOL_FLAG(textsw->state, !attrs[1],
			  TXTSW_NO_AGAIN_RECORDING);
	    break;
	  case TEXTSW_AUTO_INDENT:
	    SET_BOOL_FLAG(textsw->state, attrs[1], TXTSW_AUTO_INDENT);
	    break;
	  case TEXTSW_AUTO_SCROLL_BY:
	    (void) ev_set(view->e_view,
			  EV_CHAIN_AUTO_SCROLL_BY, (int) (attrs[1]),
			  NULL);
	    break;
	  case TEXTSW_CLIENT_DATA:
	    textsw->client_data = attrs[1];
	    break;
	  case TEXTSW_CONTROL_CHARS_USE_FONT:
	    (void) ei_set(textsw->views->eih,
			  EI_CONTROL_CHARS_USE_FONT, attrs[1], NULL);
	    break;
	  case TEXTSW_DISABLE_CD:
	    SET_BOOL_FLAG(textsw->state, attrs[1], TXTSW_NO_CD);
	    break;
	  case TEXTSW_DISABLE_LOAD:
	    SET_BOOL_FLAG(textsw->state, attrs[1], TXTSW_NO_LOAD);
	    break;
	  case TEXTSW_ES_CREATE_PROC:
	    textsw->es_create = (Es_handle(*) ()) attrs[1];
	    break;
	  case TEXTSW_FILE:
#ifndef OW_I18N
	    file = (char *) (attrs[1]);
#else /* OW_I18N */
	    if ((char *)(attrs[1]) == (char *)NULL)
		file[0]  = NULL;
	    else
		(void) mbstowcs(file, (char *) (attrs[1]), MAXPATHLEN);
#endif /* OW_I18N */
	    break;

	  case TEXTSW_FILE_CONTENTS:
	    textsw_flush_caches(view, TFC_PD_SEL);
	    ps_esh = ES_NULL;
            str_length = (attrs[1] ? strlen((char *)attrs[1]) : 0);
#ifdef OW_I18N
            if (str_length > 0)
		(void) mbstowcs(name_wc, (char *) (attrs[1]), MAXPATHLEN);
	    goto Do_create_file;

	case TEXTSW_FILE_CONTENTS_WCS:
	    textsw_flush_caches(view, TFC_PD_SEL);
	    ps_esh = ES_NULL;
            str_length = (attrs[1] ? STRLEN((CHAR *)attrs[1]) : 0);
            if (str_length > 0)
		STRCPY(name_wc, (CHAR *) (attrs[1]));

  Do_create_file:
	    textsw_implicit_commit(textsw);
	    if (STRLEN(file) == 0) {		/* } for match */
		if (str_length > 0) {		/* } for match */
		    scratch_esh = es_file_create(name_wc, 0, &es_status);
#else /* OW_I18 */
	    if (!file) {
		if (str_length > 0) {
		    scratch_esh = es_file_create(attrs[1], 0, &es_status);
#endif /* OW_I18N */
		    /* Ensure no caret turds will leave behind */
		    textsw_take_down_caret(textsw);
		} else {
		    scratch_esh = textsw->views->esh;
		}

		if (scratch_esh) {
#ifdef OW_I18N
		    SET_CONTENTS_UPDATED(textsw, TRUE);
		    if ((int) es_get(scratch_esh, ES_SKIPPED))
			textsw_invalid_data_notice(view, name_wc, 1);
		    mem_esh = es_mem_create(es_get_length(scratch_esh) + 1,
					    _xv_null_string_wc);
#else /* OW_I18N */
		    mem_esh = es_mem_create(es_get_length(scratch_esh) + 1, "");
#endif /* OW_I18N */
		    if (mem_esh) {
			if (es_copy(scratch_esh, mem_esh, FALSE) != ES_SUCCESS) {
			    es_destroy(mem_esh);
			    mem_esh = ES_NULL;
			}
		    }
		    if (str_length > 0) {
			es_destroy(scratch_esh);
		    }
		    if (mem_esh) {
			scratch_esh = es_mem_create(textsw->es_mem_maximum,
#ifdef OW_I18N
						    _xv_null_string_wc);
#else
						    "");
#endif
			if (scratch_esh) {
			    ps_esh = textsw_create_ps(textsw, mem_esh, scratch_esh,
						      &es_status);
			} else {
			    es_destroy(mem_esh);
			}
		    }
		}
	    }
	    if (ps_esh) {
		textsw_replace_esh(textsw, ps_esh);

		if (str_length > 0) {
		    Ev_handle       ev_next;
		    Ev_impl_line_seq seq;

		    EV_SET_INSERT(textsw->views, es_get_length(ps_esh), tmp);

		    FORALLVIEWS(textsw->views, ev_next) {
			seq = (Ev_impl_line_seq) ev_next->line_table.seq;
			seq[0].damaged = 0;	/* Set damage bit in line
						 * table to force redisplay  */
			ev_update_view_display(ev_next);
		    }
		    display_views = 2;	/* TEXTSW_FIRST will set it to 0 to
					 * avoid repaint */
		    all_views = TRUE;	/* For TEXTSW_FIRST, or
					 * TEXTSW_FIRST_LINE */
		    textsw_invert_caret(textsw);
		}
	    } else {
		*status_ptr = TEXTSW_STATUS_OTHER_ERROR;
	    }
	    break;	        

	  case TEXTSW_FIRST:
#ifdef OW_I18N
	    *status_ptr = set_first(view, error_msg, file, reset_mode,
			textsw_wcpos_from_mbpos(textsw, (Es_index) (attrs[1])),
			-1, all_views);
	    goto MakeSure;
	  case TEXTSW_FIRST_WC:
#endif /* OW_I18N */
	    *status_ptr = set_first(view, error_msg, file, reset_mode,
				    (Es_index) (attrs[1]), -1,
				    all_views);
	    goto MakeSure;
	  case TEXTSW_FIRST_LINE:
	    *status_ptr = set_first(view, error_msg, file, reset_mode,
				    ES_CANNOT_SET, (int) (attrs[1]),
				    all_views);
    MakeSure:
	    /*
	     * Make sure the scrollbar get updated, when this attribute is
	     * called with TEXT_FIRST.
	     */
#ifdef OW_I18N
	    if ((STRLEN(file) > 0) && !update_scrollbar)
		update_scrollbar = 2;
	    file[0] = NULL;
#else
	    if (file && !update_scrollbar)
		update_scrollbar = 2;
	    file = NULL;
#endif /* OW_I18N */
	    display_views = 0;
	    break;
	  case TEXTSW_CHECKPOINT_FREQUENCY:
	    textsw->checkpoint_frequency = (int) (attrs[1]);
	    break;
	  case TEXTSW_HISTORY_LIMIT:
	    /* Internally we want one more again slot that client does.  */
	    temp = (int) (attrs[1]);
	    temp = (temp > 0) ? temp + 1 : 0;
	    textsw_init_undo(textsw, temp);
	    break;
	  case TEXTSW_IGNORE_LIMIT:
	    textsw->ignore_limit = (unsigned) (attrs[1]);
	    break;
	  case TEXTSW_INSERTION_POINT:
#ifdef OW_I18N
	    textsw_implicit_commit(textsw);
	    (void) textsw_set_insert(textsw,
			textsw_wcpos_from_mbpos(textsw, (Es_index) (attrs[1])));
	    break;
	  case TEXTSW_INSERTION_POINT_WC:
	    textsw_implicit_commit(textsw);
#endif /* OW_I18N */
	    (void) textsw_set_insert(textsw, (Es_index) (attrs[1]));
	    break;
	  case TEXTSW_LINE_BREAK_ACTION:{
		Ev_right_break  ev_break_action;
		switch ((Textsw_enum) attrs[1]) {
		  case TEXTSW_CLIP:
		    ev_break_action = EV_CLIP;
		    goto TLBA_Tail;
		  case TEXTSW_WRAP_AT_CHAR:
		    ev_break_action = EV_WRAP_AT_CHAR;
		    goto TLBA_Tail;
		  case TEXTSW_WRAP_AT_WORD:
		    ev_break_action = EV_WRAP_AT_WORD;
	    TLBA_Tail:
		    (void) ev_set(view->e_view,
				  (all_views) ?
				  EV_FOR_ALL_VIEWS : EV_END_ALL_VIEWS,
				  EV_RIGHT_BREAK, ev_break_action,
				  NULL);
		    display_views = (all_views) ? 2 : 1;
		    break;
		  default:
		    *status_ptr = TEXTSW_STATUS_BAD_ATTR_VALUE;
		    break;
		}
		break;
	    }
	  case TEXTSW_LOWER_CONTEXT:
	    (void) ev_set(view->e_view,
			  EV_CHAIN_LOWER_CONTEXT, (int) (attrs[1]),
			  NULL);
	    break;
	  case TEXTSW_MEMORY_MAXIMUM:
	    textsw->es_mem_maximum = (unsigned) (attrs[1]);
	    if (textsw->es_mem_maximum == 0) {
		textsw->es_mem_maximum = TEXTSW_INFINITY;
	    } else if (textsw->es_mem_maximum < 128)
		textsw->es_mem_maximum = 128;
	    break;
	  case TEXTSW_NO_RESET_TO_SCRATCH:
	    SET_BOOL_FLAG(textsw->state, attrs[1],
			  TXTSW_NO_RESET_TO_SCRATCH);
	    break;
	  case TEXTSW_NOTIFY_LEVEL:
	    textsw->notify_level = (int) (attrs[1]);
	    break;
	  case TEXTSW_NOTIFY_PROC:
	    textsw->notify = (int (*) ()) attrs[1];
	    if (textsw->notify_level == 0)
		textsw->notify_level = TEXTSW_NOTIFY_STANDARD;
	    break;
	  case TEXTSW_READ_ONLY:
	    read_only_start = TXTSW_IS_READ_ONLY(textsw);
#ifdef OW_I18N
	    /* to Read-only mode from Edit mode */
	    if (!read_only_start && attrs[1] == TRUE)
		textsw_implicit_commit(textsw);
#endif
	    SET_BOOL_FLAG(textsw->state, attrs[1], TXTSW_READ_ONLY_ESH);
	    set_read_only_esh = (textsw->state & TXTSW_READ_ONLY_ESH);
	    read_only_changed = (read_only_start != TXTSW_IS_READ_ONLY(textsw));
	    break;
	  case TEXTSW_STATUS:
	    status_ptr = (Textsw_status *) attrs[1];
	    *status_ptr = TEXTSW_STATUS_OKAY;
	    break;
	  case TEXTSW_TAB_WIDTH:
	    (void) ei_set(textsw->views->eih, EI_TAB_WIDTH, attrs[1], NULL);
	    break;
	  case TEXTSW_TEMP_FILENAME:
	    if (textsw->temp_filename)
		free(textsw->temp_filename);
	    textsw->temp_filename = STRDUP((char *) attrs[1]);
	    break;
	  case TEXTSW_UPPER_CONTEXT:
	    (void) ev_set(view->e_view,
			  EV_CHAIN_UPPER_CONTEXT, (int) (attrs[1]),
			  NULL);
	    break;
	  case TEXTSW_WRAPAROUND_SIZE:
	    es_set(textsw->views->esh,
		   ES_PS_SCRATCH_MAX_LEN, attrs[1],
		   NULL);
	    break;
	  case TEXTSW_ACCELERATE_MENUS:
	    /*
	     * Do only if the current state is different
	     */
	    if (textsw->accel_menus != attrs[1])  {
		/*
		 * Do only if the menu has been created
		 */
	        if (textsw->menu)  {
		    /*
		     * Get frame handle
		     */
		    Frame	frame = 
			(Frame)xv_get(textsw_public, WIN_FRAME);

		    /*
		     * Do only if frame handle != NULL
		     */
		    if (frame)  {
			/*
			 * Add/Delete menu to accelerate
			 */
		        if (attrs[1])  {
		            xv_set(frame, FRAME_MENU_ADD, 
					textsw->menu, NULL);
		        }
		        else  {
		            xv_set(frame, FRAME_MENU_DELETE, 
					textsw->menu, NULL);
		        }

			/*
			 * Update state
			 */
		        textsw->accel_menus = attrs[1];
		    }
	        }
	    }
	    break;
#ifdef XV_DEBUG
	  case TEXTSW_MALLOC_DEBUG_LEVEL:
	    malloc_debug((int) attrs[1]);
	    break;
#endif
/*
 * NOTE: This code no longer used.
 * Remove after suitable grace period
	  case TEXTSW_CONSUME_ATTRS:
	    consume_attrs = (attrs[1] ? 1 : 0);
	    break;
 */

	    /* Super-class attributes that we monitor. */
	  case WIN_FONT:
	    if (attrs[1]) {
		Ev_handle       ev_next;
		if (textsw->state & TXTSW_INITIALIZED) {
		    /* BUG ALERT!  Is this needed any longer? */
		    if (textsw->state & TXTSW_OPENED_FONT) {
			PIXFONT        *old_font;
			old_font = (PIXFONT *)
			    ei_get(textsw->views->eih, EI_FONT);

			if (old_font == (PIXFONT *) attrs[1])
			    break;

			xv_pf_close(old_font);
			textsw->state &= ~TXTSW_OPENED_FONT;
		    }
		    (void) ei_set(textsw->views->eih,
				  EI_FONT, attrs[1], NULL);
#ifdef FULL_R5
#ifdef OW_I18N
        	if (textsw->ic && (textsw->xim_style & (XIMPreeditArea | XIMPreeditPosition | XIMPreeditNothing))) {  
    		        va_nested_list = XVaCreateNestedList(NULL, 
					      	 XNLineSpace, (int)ei_get(textsw->views->eih, EI_LINE_SPACE), 
						 NULL);

        		XSetICValues(textsw->ic, XNPreeditAttributes,
        					 va_nested_list,
        	     		     NULL);
        	     	XFree(va_nested_list);	     
    		    }
#endif /* OW_I18N */	
#endif /* FULL_R5 */			    		  
		}
		/* Adjust the views to account for the font change */
		FORALLVIEWS(textsw->views, ev_next) {
		    (void) ev_set(ev_next,
				  EV_CLIP_RECT, &ev_next->rect,
				  EV_RECT, &ev_next->rect, NULL);
		}
	    }
	    break;
	  case WIN_MENU:
	    textsw->menu = attrs[1];
	    break;

	    /* Super-class attributes that we override. */
	  case XV_LEFT_MARGIN:
	    *attrs = (Textsw_attribute) ATTR_NOP(*attrs);
	    (void) ev_set(view->e_view,
			  (all_views) ?
			  EV_FOR_ALL_VIEWS : EV_END_ALL_VIEWS,
			  EV_LEFT_MARGIN, (int) (attrs[1]),
			  NULL);
	    display_views = (all_views) ? 2 : 1;
	    break;
	  case XV_RIGHT_MARGIN:
	    *attrs = (Textsw_attribute) ATTR_NOP(*attrs);
	    (void) ev_set(view->e_view,
			  (all_views) ?
			  EV_FOR_ALL_VIEWS : EV_END_ALL_VIEWS,
			  EV_RIGHT_MARGIN, (int) (attrs[1]),
			  NULL);
	    display_views = (all_views) ? 2 : 1;
	    break;

	  case WIN_REMOVE_CARET:
	    textsw_hide_caret(textsw);
	    break;

	  case WIN_SET_FOCUS:{
		Xv_Drawable_info *view_info;
		Xv_Window	    view_public;
		int		    view_nbr;
		if (!is_folio)
		    break;
		/* Set the focus to the first Openwin view */
		*attrs = (Textsw_attribute) ATTR_NOP(*attrs);
		status = TEXTSW_STATUS_OTHER_ERROR;
		for (view_nbr = 0;;view_nbr++) {
		    view_public = xv_get(textsw_public,
					 OPENWIN_NTH_VIEW, view_nbr);
		    if (!view_public)
			break;
		    DRAWABLE_INFO_MACRO(view_public, view_info);
		    if (!xv_no_focus(view_info) &&
			win_getinputcodebit((Inputmask *) xv_get(view_public,
						WIN_INPUT_MASK), KBD_USE)) {
			win_set_kbd_focus(view_public, xv_xid(view_info));
			status = TEXTSW_STATUS_OKAY;
			break;
		    }
		}
		break;
	    }

#ifdef OW_I18N
	  case WIN_IC_ACTIVE:
	    if (is_folio && textsw->need_im) {
		ic_active_changed =
			(attrs[1] == TRUE) ? IC_ACTIVE_TRUE : IC_ACTIVE_FALSE;
		if (attrs[1] == FALSE && textsw->ic &&
			(int)xv_get(textsw_public, WIN_IC_CONVERSION)) {
		    textsw_implicit_commit(textsw);
		    xv_set(textsw_public, WIN_IC_CONVERSION, FALSE, NULL);
		}

		if (TXTSW_IS_READ_ONLY(textsw) && attrs[1] == TRUE)
		    break;
		FORALL_TEXT_VIEWS(textsw, next) {
		    xv_set(VIEW_PUBLIC(next), WIN_IC_ACTIVE, attrs[1], NULL);
		}
	    }
	    break;
#endif /* OW_I18N */
	  case TEXTSW_COALESCE_WITH: /* Get only */
	  case TEXTSW_BLINK_CARET:
	    /*
	     * jcb 4/29/89	SET_BOOL_FLAG(textsw->caret_state, attrs[1],
	     * TXTSW_CARET_FLASHING);
	     */
             break;

	  case TEXTSW_FOR_ALL_VIEWS:
	  case TEXTSW_END_ALL_VIEWS:
	  case TEXTSW_ADJUST_IS_PENDING_DELETE:
	  case TEXTSW_BROWSING:
	  case TEXTSW_CONFIRM_OVERWRITE:
	  case TEXTSW_CONTENTS:
	  case TEXTSW_DESTROY_ALL_VIEWS:
          case TEXTSW_DIFFERENTIATE_CR_LF: /*1030878*/
	  case TEXTSW_STORE_CHANGES_FILE:
	  case TEXTSW_EDIT_BACK_CHAR:
	  case TEXTSW_EDIT_BACK_WORD:
	  case TEXTSW_EDIT_BACK_LINE:
	  case TEXTSW_ERROR_MSG:
	  case TEXTSW_INSERT_FROM_FILE:
#ifdef OW_I18N
	  case TEXTSW_FILE_WCS:
	  case TEXTSW_CONTENTS_WCS:
	  case TEXTSW_INSERT_FROM_FILE_WCS:
#endif /* OW_I18N */
	  case TEXTSW_INSERT_MAKES_VISIBLE:
	  case TEXTSW_MULTI_CLICK_SPACE:
	  case TEXTSW_MULTI_CLICK_TIMEOUT:
	  case TEXTSW_NO_REPAINT_TIL_EVENT:
	  case TEXTSW_RESET_MODE:
	  case TEXTSW_RESET_TO_CONTENTS:
	  case TEXTSW_TAB_WIDTHS:
	  case TEXTSW_UPDATE_SCROLLBAR:
	  case WIN_CMS_CHANGE:
		textsw_set_internal_tier2 (textsw, view, attrs, is_folio,
		   status_ptr, &error_msg, file, name_wc, &reset_mode,
		   &all_views, &update_scrollbar, &read_only_changed);
		break;

	  default:
	    (void) xv_check_bad_attr(&xv_textsw_pkg,
				     (Attr_attribute) attrs[0]);
	    break;
	}
    }

#ifdef OW_I18N
    if (STRLEN(file) > 0) {	/* } for match */
	textsw_implicit_commit(textsw);
#else
    if (file) {
#endif
	*status_ptr = set_first(view, error_msg, file, reset_mode, ES_CANNOT_SET, -1, all_views);
	/*
	 * This is for resetting the TXTSW_READ_ONLY_ESH flag that got
	 * cleared in textsw_replace_esh
	 */
	if (set_read_only_esh)
	    textsw->state |= TXTSW_READ_ONLY_ESH;

	display_views = 0;
    }
    if (display_views && (textsw->state & TXTSW_DISPLAYED)) {
	FORALL_TEXT_VIEWS(textsw, next) {
	    if ((display_views == 1) && (next != view))
		continue;
	    textsw_display_view_margins(next, RECT_NULL);
	    ev_display_view(next->e_view);
	}
	update_scrollbar = display_views;
    }
    if (update_scrollbar) {
	textsw_update_scrollbars(textsw,
		   (update_scrollbar == 2) ? (Textsw_view_handle) 0 : view);
    }
    if (read_only_changed) {
	if (TXTSW_IS_READ_ONLY(textsw))
	    textsw_hide_caret(textsw);
	else
	    textsw_show_caret(textsw);
#ifdef OW_I18N
	if (textsw->need_im) {
	    if ((ic_active_changed == IC_ACTIVE_TRUE) ||
	        (!ic_active_changed && xv_get(textsw_public, WIN_IC_ACTIVE))) {

		FORALL_TEXT_VIEWS(textsw, next) {
		    xv_set(VIEW_PUBLIC(next), WIN_IC_ACTIVE,
			   (TXTSW_IS_READ_ONLY(textsw) ? FALSE : TRUE),
			   NULL);
		}
	    }
	}
#endif /* OW_I18N */
    }
    return (status);
}

Pkg_private     Textsw_status
textsw_set_internal_tier2(textsw, view, attrs, is_folio, status_ptr,
			  error_msg_addr, file, name_wc, reset_mode,
			  all_views, update_scrollbar, read_only_changed)
    register Textsw_folio textsw;
/* folio private handle */
    Textsw_view_handle view;
    Attr_attribute *attrs;
    int             is_folio;
    Textsw_status  *status_ptr;
    char	  **error_msg_addr;
    CHAR	    *file, *name_wc;
    int     *reset_mode, *all_views, *update_scrollbar, *read_only_changed;
/*
 * TRUE= we're setting attr's on the textsw_folio FALSE= we're setting attr's
 * on the textsw_view
 */
{
    Textsw          textsw_public = TEXTSW_PUBLIC(textsw);
    register int    temp;
    int             read_only_start;
#ifndef LEFT_HAND_SIDE_CAST
    int            *int_ptr;
#endif /* LEFT_HAND_SIDE_CAST */

    switch (*attrs) {
          case TEXTSW_FOR_ALL_VIEWS:
            *all_views = TRUE;
            break;
          case TEXTSW_END_ALL_VIEWS:
            *all_views = FALSE;
            break;

          case TEXTSW_ADJUST_IS_PENDING_DELETE:
            SET_BOOL_FLAG(textsw->state, attrs[1], TXTSW_ADJUST_IS_PD);
            break;
          case TEXTSW_BROWSING:
            read_only_start = TXTSW_IS_READ_ONLY(textsw);
#ifdef OW_I18N
            /* to Read-only mode from Edit mode */
            if (!read_only_start && attrs[1] == TRUE)
                textsw_implicit_commit(textsw);
#endif
            SET_BOOL_FLAG(textsw->state, attrs[1], TXTSW_READ_ONLY_SW);
            *read_only_changed = (read_only_start!=TXTSW_IS_READ_ONLY(textsw));            break;

          case TEXTSW_CONFIRM_OVERWRITE:
            SET_BOOL_FLAG(textsw->state, attrs[1],
                          TXTSW_CONFIRM_OVERWRITE);
            break;
          case TEXTSW_CONTENTS:
            temp = (textsw->state & TXTSW_NO_AGAIN_RECORDING);
            if (!(textsw->state & TXTSW_INITIALIZED))
                textsw->state |= TXTSW_NO_AGAIN_RECORDING;
#ifdef OW_I18N
            (void) textsw_replace_bytes(VIEW_REP_TO_ABS(view), 0,
#else
            (void) textsw_replace(VIEW_REP_TO_ABS(view), 0,
#endif
                   TEXTSW_INFINITY, (char *)attrs[1], strlen((char *)attrs[1]));            if (!(textsw->state & TXTSW_INITIALIZED))
                SET_BOOL_FLAG(textsw->state, temp,
                              TXTSW_NO_AGAIN_RECORDING);
            break;
#ifdef OW_I18N
          case TEXTSW_CONTENTS_WCS:
            temp = (textsw->state & TXTSW_NO_AGAIN_RECORDING);
            if (!(textsw->state & TXTSW_INITIALIZED))
                textsw->state |= TXTSW_NO_AGAIN_RECORDING;
            textsw_implicit_commit(textsw);
            (void) textsw_replace(VIEW_REP_TO_ABS(view), 0,
                   TEXTSW_INFINITY, (CHAR *)attrs[1], STRLEN((CHAR *)attrs[1]));            if (!(textsw->state & TXTSW_INITIALIZED))
                SET_BOOL_FLAG(textsw->state, temp,
                              TXTSW_NO_AGAIN_RECORDING);
            break;
#endif /* OW_I18N */

          case TEXTSW_DESTROY_ALL_VIEWS:
            SET_BOOL_FLAG(textsw->state, attrs[1],
                          TXTSW_DESTROY_ALL_VIEWS);
            break;
          case TEXTSW_DIFFERENTIATE_CR_LF: /*1030878*/
            SET_BOOL_FLAG(textsw->state, attrs[1], TXTSW_DIFF_CR_LF);
            break;
          case TEXTSW_STORE_CHANGES_FILE:
            SET_BOOL_FLAG(textsw->state, attrs[1],
                          TXTSW_STORE_CHANGES_FILE);
            break;
          case TEXTSW_EDIT_BACK_CHAR:
            textsw->edit_bk_char = (char) (attrs[1]);
            break;
          case TEXTSW_EDIT_BACK_WORD:
            textsw->edit_bk_word = (char) (attrs[1]);
            break;
          case TEXTSW_EDIT_BACK_LINE:
            textsw->edit_bk_line = (char) (attrs[1]);
            break;
          case TEXTSW_ERROR_MSG:
            *error_msg_addr = (char *) (attrs[1]);
            (*error_msg_addr)[0] = '\0';
            break;
#ifdef OW_I18N
          case TEXTSW_FILE_WCS:
            if ((CHAR *)(attrs[1]) == (CHAR *)NULL)
                file[0]  = NULL;
            else
                (void) STRCPY(file, (CHAR *) (attrs[1]));
            break;
#endif /* OW_I18N */

          case TEXTSW_INSERT_FROM_FILE:{
#ifdef OW_I18N
                pkg_private Textsw_status textsw_get_from_file();

                (void) mbstowcs(name_wc, (char *) (attrs[1]), MAXPATHLEN);
                *status_ptr = textsw_get_from_file(view, name_wc, TRUE);
                if (*status_ptr == TEXTSW_STATUS_OKAY)
                    *update_scrollbar = 2;
                break;
            };
          case TEXTSW_INSERT_FROM_FILE_WCS:{
#endif /* OW_I18N */
                pkg_private Textsw_status textsw_get_from_file();

                *status_ptr = textsw_get_from_file(view, (CHAR *) (attrs[1]),
                                             TRUE);
                if (*status_ptr == TEXTSW_STATUS_OKAY)
                    *update_scrollbar = 2;
                break;
            };
          case TEXTSW_INSERT_MAKES_VISIBLE:
            switch ((Textsw_enum) attrs[1]) {
              case TEXTSW_ALWAYS:
              case TEXTSW_IF_AUTO_SCROLL:
                textsw->insert_makes_visible = (Textsw_enum) attrs[1];
                break;
              default:
                *status_ptr = TEXTSW_STATUS_BAD_ATTR_VALUE;
                break;
            }
            break;

          case TEXTSW_MULTI_CLICK_SPACE:
            if ((int) (attrs[1]) != -1)
                textsw->multi_click_space = (int) (attrs[1]);
            break;
          case TEXTSW_MULTI_CLICK_TIMEOUT:
            if ((int) (attrs[1]) != -1)
                textsw->multi_click_timeout = (int) (attrs[1]);
            break;
          case TEXTSW_NO_REPAINT_TIL_EVENT:
            ev_set(view->e_view, EV_NO_REPAINT_TIL_EVENT, (int) (attrs[1]),
                   NULL);
            break;
           case TEXTSW_RESET_MODE:
            *reset_mode = (int) (attrs[1]);
            break;
          case TEXTSW_RESET_TO_CONTENTS:
            (void) textsw_reset_2(VIEW_REP_TO_ABS(view), 0, 0, TRUE, FALSE);
            break;
           case TEXTSW_TAB_WIDTHS:
#ifdef LEFT_HAND_SIDE_CAST
            /* XXX cheat here */
            *(int *) attrs = (int) EI_TAB_WIDTHS;
#else
            int_ptr = (int *) attrs;
            *int_ptr = (int) EI_TAB_WIDTHS;
#endif /* LEFT_HAND_SIDE_CAST */
            ei_plain_text_set(textsw->views->eih, attrs);
            /* (void) ei_set(textsw->views->eih, EI_TAB_WIDTHS, attrs[1], 0); */            break;
          case TEXTSW_UPDATE_SCROLLBAR:
            *update_scrollbar = (all_views) ? 2 : 1;
            break;

          case WIN_CMS_CHANGE:
            if (is_folio) {
                Xv_Window       textsw_public = TEXTSW_PUBLIC(textsw);
                Xv_Window       view_public;
                Textsw_view_handle view_next;
                Xv_Drawable_info *info;
                Cms             cms;

                DRAWABLE_INFO_MACRO(textsw_public, info);
                cms = xv_cms(info);
                FORALL_TEXT_VIEWS(textsw, view_next) {
                    view_public = WINDOW_FROM_VIEW(view_next);
                    window_set_cms(view_public, cms, xv_cms_bg(info), xv_cms_fg(info));
                }
            } else {
                textsw_view_cms_change(textsw, view);
            }
            break;
    }
}


static Defaults_pairs insert_makes_visible_pairs[] = {
    "If_auto_scroll", (int) TEXTSW_IF_AUTO_SCROLL,
    "Always", (int) TEXTSW_ALWAYS,
    NULL, (int) TEXTSW_IF_AUTO_SCROLL
};


static Defaults_pairs line_break_pairs[] = {
    "Clip", (int) TEXTSW_CLIP,
    "Wrap_char", (int) TEXTSW_WRAP_AT_CHAR,
    "Wrap_word", (int) TEXTSW_WRAP_AT_WORD,
    NULL, (int) TEXTSW_WRAP_AT_WORD
};


Pkg_private	void
textsw_view_cms_change(textsw, view)
    register Textsw_folio textsw;
    Textsw_view_handle view;
{
    ev_set(view->e_view, EV_NO_REPAINT_TIL_EVENT, FALSE, NULL);
    textsw_repaint(view);
    /* if caret was up and we took it down, put it back */
    if ((textsw->caret_state & TXTSW_CARET_ON)
	&& (textsw->caret_state & TXTSW_CARET_ON) == 0) {
	textsw_remove_timer(textsw);
	textsw_timer_expired(textsw, 0);
    }
}

Pkg_private     long
textsw_get_from_defaults(attribute)
    register Textsw_attribute attribute;
{
    char           *def_str;	/* Points to strings owned by defaults. */

    switch (attribute) {
      case TEXTSW_ADJUST_IS_PENDING_DELETE:
	return ((long) True);
      case TEXTSW_AGAIN_LIMIT:
	return ((long)
		defaults_get_integer_check("text.againLimit",
					   "Text.AgainLimit", 1, 0, 500));
      case TEXTSW_AGAIN_RECORDING:
        return ((long)
                defaults_get_boolean("text.againRecording",
                                     "Text.againRecording", True));
      case TEXTSW_AUTO_INDENT:
	return ((long)
		defaults_get_boolean("text.autoIndent",
				     "Text.AutoIndent", False));
      case TEXTSW_AUTO_SCROLL_BY:
	return ((long)
		defaults_get_integer_check("text.autoScrollBy",
					   "Text.AutoScrollBy", 1, 0, 100));
      case TEXTSW_BLINK_CARET:
#ifdef notdef
	/* BUG: always return FALSE for alpha4 performance */
	return ((long)
		defaults_get_boolean("text.blinkCaret",
				     "Text.BlinkCaret", True));
#endif
	return (long) FALSE;
      case TEXTSW_CHECKPOINT_FREQUENCY:
	/* Not generally settable via defaults */
	return ((long) 0);
      case TEXTSW_CONFIRM_OVERWRITE:
	return ((long)
		defaults_get_boolean("text.confirmOverwrite",
				     "Text.ConfirmOverwrite", True));
      case TEXTSW_CONTROL_CHARS_USE_FONT:
	return ((long)
		defaults_get_boolean("text.displayControlChars",
				     "Text.DisplayControlChars", False));
      case TEXTSW_EDIT_BACK_CHAR:
	return ((long)
		defaults_get_character("keyboard.deleteChar",
				       "Keyboard.DeleteChar", DEL));		/* ??? Keymapping  strategy? */
      case TEXTSW_EDIT_BACK_WORD:
	return ((long)
		defaults_get_character("keyboard.deleteWord",
#ifndef __STDC__
				       "Keyboard.DeleteWord", CTRL(W)));	/* ??? Keymapping strategy? */
#else /* __STDC__ */
				       "Keyboard.DeleteWord", CTRL('W')));	/* ??? Keymapping strategy? */
#endif /* __STDC__ */
      case TEXTSW_EDIT_BACK_LINE:
	return ((long)
		defaults_get_character("keyboard.deleteLine",
#ifndef __STDC__
				       "Keyboard.DeleteLine", CTRL(U)));	/* ??? Keymapping strategy? */
#else /* __STDC__ */
				       "Keyboard.DeleteLine", CTRL('U')));	/* ??? Keymapping strategy? */
#endif /* __STDC__ */
      case WIN_FONT:{
	    PIXFONT        *font;

	    /* Text.d may have "" rather than NULL, so check for this case.  */
#ifdef OW_I18N

	    defaults_set_locale(NULL, XV_LC_BASIC_LOCALE);
	    def_str = xv_font_monospace();
	    defaults_set_locale(NULL, NULL);
#else
	    def_str = xv_font_monospace();
#endif /* OW_I18N */
      	    font = (def_str && ((int)strlen(def_str) > 0))
                    ? xv_pf_open(def_str) : 0;
	    return ((long) font);
	}
      case TEXTSW_HISTORY_LIMIT:
	return ((long)
	      defaults_get_integer_check("text.undoLimit", "Text.UndoLimit",
					 50, 0, 500));
      case TEXTSW_INSERT_MAKES_VISIBLE:
	def_str = defaults_get_string("text.insertMakesCaretVisible",
				"Text.InsertMakesCaretVisible", (char *) 0);
	if (def_str && ((int)strlen(def_str) > 0)) {
	    return ((long)
		    defaults_lookup(def_str,
				    insert_makes_visible_pairs));
	} else
	    return (long) TEXTSW_IF_AUTO_SCROLL;
      case TEXTSW_LINE_BREAK_ACTION:
	def_str = defaults_get_string("text.lineBreak",
				      "Text.LineBreak", (char *) 0);
	if (def_str && ((int)strlen(def_str) > 0)) {
	    return ((long)
		    defaults_lookup(def_str, line_break_pairs));
	} else
	    return (long) TEXTSW_WRAP_AT_WORD;
      case TEXTSW_LOWER_CONTEXT:
	return ((long)
		defaults_get_integer_check("text.margin.bottom",
			       "Text.Margin.Bottom", 0, EV_NO_CONTEXT, 50));
	/* ??? Implement Text.EnableScrolling */
      case TEXTSW_MULTI_CLICK_SPACE:
	return ((long)
		defaults_get_integer_check("mouse.multiclick.space",
				      "Mouse.Multiclick.Space", 4, 0, 500));	/* ??? OL-compliant? */
      case TEXTSW_MULTI_CLICK_TIMEOUT:
	return ((long) (100 *
		 defaults_get_integer_check("openWindows.multiClickTimeout",
			       "OpenWindows.MultiClickTimeout", 4, 2, 10)));
      case TEXTSW_STORE_CHANGES_FILE:
	return ((long)
		defaults_get_boolean("text.storeChangesFile",
				     "Text.StoreChangesFile", True));
      case TEXTSW_UPPER_CONTEXT:
	return ((long)
		defaults_get_integer_check("text.margin.top",
				  "Text.Margin.Top", 2, EV_NO_CONTEXT, 50));
      case XV_LEFT_MARGIN:
	return ((long)
		defaults_get_integer_check("text.margin.left",
					   "Text.Margin.Left", 8, 0, 2000));
      case XV_RIGHT_MARGIN:
	return ((long)
		defaults_get_integer_check("text.margin.right",
					   "Text.Margin.Right", 0, 0, 2000));
      case TEXTSW_TAB_WIDTH:
	return ((long)
		defaults_get_integer_check("text.tabWidth",
					   "Text.TabWidth", 8, 0, 50));
      default:
	return ((long) 0);
    }
}

/* Caller turns varargs into va_list that has already been va_start'd */
static          Xv_opaque
textsw_get_internal(folio, view, status, attribute, args)
    register Textsw_folio folio;
    Textsw_view_handle view;
    int            *status;	/* initialized by caller */
    Textsw_attribute attribute;
    va_list         args;
{

    /* If view is not created yet, return zero for this attrs */
    switch (attribute) {
      case TEXTSW_INSERTION_POINT:
      case TEXTSW_LENGTH:
      case TEXTSW_FIRST:
      case TEXTSW_FIRST_LINE:
      case XV_LEFT_MARGIN:
      case XV_RIGHT_MARGIN:
      case WIN_VERTICAL_SCROLLBAR:
      case TEXTSW_CONTENTS:
#ifdef OW_I18N
      case TEXTSW_CONTENTS_WCS:
      case TEXTSW_CONTENTS_NO_COMMIT:
      case TEXTSW_CONTENTS_WCS_NO_COMMIT:
      case TEXTSW_LENGTH_WC:
      case TEXTSW_INSERTION_POINT_WC:
      case TEXTSW_FIRST_WC:
#endif /* OW_I18N */
      case TEXTSW_EDIT_COUNT:
	if (!view) {
	    *status = XV_ERROR;
	    return ((Xv_opaque) 0);
	}
	break;
    }

    /*
     * Note that ev_get(chain, EV_CHAIN_xxx) casts chain to be view in order
     * to keep lint happy.
     */
    switch (attribute) {
      case TEXTSW_LENGTH:
#ifdef OW_I18N
	textsw_flush_caches(view, TFC_STD);
	return ((Xv_opaque) textsw_get_mb_length(folio));
      case TEXTSW_LENGTH_WC:
#endif /* OW_I18N */
	textsw_flush_caches(view, TFC_STD);
	return ((Xv_opaque) es_get_length(folio->views->esh));
      case TEXTSW_INSERTION_POINT:
#ifdef OW_I18N
	textsw_flush_caches(view, TFC_STD);
	return ((Xv_opaque) textsw_mbpos_from_wcpos(folio,
					EV_GET_INSERT(folio->views)));
      case TEXTSW_INSERTION_POINT_WC:
#endif
	textsw_flush_caches(view, TFC_STD);
	return ((Xv_opaque) EV_GET_INSERT(folio->views));
      case TEXTSW_LOWER_CONTEXT:
	return ((Xv_opaque) ev_get((Ev_handle) (view->e_view),
	    EV_CHAIN_LOWER_CONTEXT));
      case OPENWIN_VIEW_CLASS:
	return ((Xv_opaque) TEXTSW_VIEW);
      case TEXTSW_ACCELERATE_MENUS:
	return ((Xv_opaque)folio->accel_menus);
      case TEXTSW_ADJUST_IS_PENDING_DELETE:
	return ((Xv_opaque)
		BOOL_FLAG_VALUE(folio->state, TXTSW_ADJUST_IS_PD));
      case TEXTSW_AGAIN_LIMIT:
	return ((Xv_opaque)
		((folio->again_count) ? (folio->again_count - 1) : 0));
      case TEXTSW_AGAIN_RECORDING:
	return ((Xv_opaque)
		! BOOL_FLAG_VALUE(folio->state, TXTSW_NO_AGAIN_RECORDING));
      case TEXTSW_AUTO_INDENT:
	return ((Xv_opaque)
		BOOL_FLAG_VALUE(folio->state, TXTSW_AUTO_INDENT));
      case TEXTSW_AUTO_SCROLL_BY:
	return ((Xv_opaque) ev_get((Ev_handle) (view->e_view),
		       EV_CHAIN_AUTO_SCROLL_BY));
      case TEXTSW_BLINK_CARET:
	return ((Xv_opaque)
		BOOL_FLAG_VALUE(folio->caret_state, TXTSW_CARET_FLASHING));
      case TEXTSW_BROWSING:
	return ((Xv_opaque)
		BOOL_FLAG_VALUE(folio->state, TXTSW_READ_ONLY_SW));
      case TEXTSW_CLIENT_DATA:
	return (folio->client_data);
      case TEXTSW_COALESCE_WITH:
	return ((Xv_opaque) folio->coalesce_with);
      case TEXTSW_CONFIRM_OVERWRITE:
	return ((Xv_opaque)
		BOOL_FLAG_VALUE(folio->state, TXTSW_CONFIRM_OVERWRITE));
      case TEXTSW_CONTENTS:{
#ifdef OW_I18N
	    textsw_implicit_commit(folio);
      }
      case TEXTSW_CONTENTS_NO_COMMIT:{
      /* This is used by only mailtool for autosave.*/
#endif
	    /* pos, buf and buf_len are */
	    Es_index        pos = va_arg(args, Es_index);
	    /* temporaries for TEXTSW_CONTENTS */
	    char           *buf = va_arg(args, caddr_t);
	    int             buf_len = va_arg(args, int);

	    return ((Xv_opaque)
		    textsw_get_contents(folio, pos, buf, buf_len));
	}
#ifdef OW_I18N
      case TEXTSW_CONTENTS_WCS:
	    textsw_implicit_commit(folio);
      case TEXTSW_CONTENTS_WCS_NO_COMMIT: { /* This is used by only Termsw. */
	    /* OW_I18N: pos is character based */
	    Es_index        pos = va_arg(args, Es_index);
	    /* temporaries for TEXTSW_CONTENTS */
	    CHAR           *buf = va_arg(args, CHAR *);
	    /* OW_I18N: buf_len is character based */
	    int             buf_len = va_arg(args, int);
	    return ((Xv_opaque)
		    textsw_get_contents_wcs(folio, pos, buf, buf_len));
	}
#endif	
      case TEXTSW_CONTROL_CHARS_USE_FONT:
	return ((Xv_opaque)
		ei_get(folio->views->eih, EI_CONTROL_CHARS_USE_FONT));
      case TEXTSW_DESTROY_ALL_VIEWS:
	return ((Xv_opaque)
		BOOL_FLAG_VALUE(folio->state, TXTSW_DESTROY_ALL_VIEWS));
      case TEXTSW_DISABLE_CD:
	return ((Xv_opaque)
		BOOL_FLAG_VALUE(folio->state, TXTSW_NO_CD));
      case TEXTSW_DISABLE_LOAD:
	return ((Xv_opaque)
		BOOL_FLAG_VALUE(folio->state, TXTSW_NO_LOAD));
      case TEXTSW_DIFFERENTIATE_CR_LF: /*1030878*/
        return ((Xv_opaque)
                BOOL_FLAG_VALUE(folio->state, TXTSW_DIFF_CR_LF));
      case TEXTSW_EDIT_BACK_CHAR:
	return ((Xv_opaque) folio->edit_bk_char);
      case TEXTSW_EDIT_BACK_WORD:
	return ((Xv_opaque) folio->edit_bk_word);
      case TEXTSW_EDIT_BACK_LINE:
	return ((Xv_opaque) folio->edit_bk_line);
      case TEXTSW_EDIT_COUNT:
	return ((Xv_opaque) ev_get((Ev_handle) (view->e_view),
		       EV_CHAIN_EDIT_NUMBER));
      case TEXTSW_WRAPAROUND_SIZE:
	return ((Xv_opaque) es_get(folio->views->esh, ES_PS_SCRATCH_MAX_LEN));
      case TEXTSW_ES_CREATE_PROC:
	return ((Xv_opaque) folio->es_create);

#ifdef OW_I18N
      case TEXTSW_FILE_WCS:{ 	/* } for match */
#else
      case TEXTSW_FILE:{
#endif
	    CHAR           *name;
	    if (textsw_file_name(folio, &name))
		return ((Xv_opaque) 0);
	    else
		return ((Xv_opaque) name);
	}

#ifdef OW_I18N
      case TEXTSW_FILE:{
	    CHAR           *name;
	    char           name_mb[MAXPATHLEN];
	    if (textsw_file_name(folio, &name))
		return ((Xv_opaque) 0);
	    else {
		(void) wcstombs(name_mb, name, MAXPATHLEN);
	    	return ((Xv_opaque) name_mb);
	    }
	}
#endif
      case TEXTSW_SUBMENU_FILE:{
	    if ((!folio->menu) || (!folio->sub_menu_table))
		return ((Xv_opaque) NULL);
	    else
		return ((Xv_opaque) folio->sub_menu_table[(int) TXTSW_FILE_SUB_MENU]);
	}
      case TEXTSW_SUBMENU_EDIT:{
	    if ((!folio->menu) || (!folio->sub_menu_table))
		return ((Xv_opaque) NULL);
	    else
		return ((Xv_opaque) folio->sub_menu_table[(int) TXTSW_EDIT_SUB_MENU]);
	}
      case TEXTSW_SUBMENU_VIEW:{
	    if ((!folio->menu) || (!folio->sub_menu_table))
		return ((Xv_opaque) NULL);
	    else
		return ((Xv_opaque) folio->sub_menu_table[(int) TXTSW_VIEW_SUB_MENU]);
	}
      case TEXTSW_SUBMENU_FIND:{
	    if ((!folio->menu) || (!folio->sub_menu_table))
		return ((Xv_opaque) NULL);
	    else
		return ((Xv_opaque) folio->sub_menu_table[(int) TXTSW_FIND_SUB_MENU]);
	}
      case TEXTSW_EXTRAS_CMD_MENU:{
	    if ((!folio->menu) || (!folio->sub_menu_table))
		return ((Xv_opaque) NULL);
	    else
		return ((Xv_opaque) folio->sub_menu_table[(int) TXTSW_EXTRAS_SUB_MENU]);
	}
      case TEXTSW_FIRST:
#ifdef OW_I18N
	return ((Xv_opaque) textsw_mbpos_from_wcpos(folio,
		ft_position_for_index(view->e_view->line_table, 0)));
      case TEXTSW_FIRST_WC:
#endif
	return ((Xv_opaque)
		ft_position_for_index(view->e_view->line_table, 0));
      case TEXTSW_FIRST_LINE:{
	    int             top, bottom;

	    ev_line_info(view->e_view, &top, &bottom);
	    return ((Xv_opaque) top - 1);
	}
      case TEXTSW_HISTORY_LIMIT:
	return ((Xv_opaque)
		((folio->undo_count) ? (folio->undo_count - 1) : 0));
      case TEXTSW_IGNORE_LIMIT:
	return ((Xv_opaque) folio->ignore_limit);
      case TEXTSW_INSERT_MAKES_VISIBLE:
	return ((Xv_opaque) folio->insert_makes_visible);
      case TEXTSW_MODIFIED:
	return ((Xv_opaque) textsw_has_been_modified(FOLIO_REP_TO_ABS(folio)));
      case TEXTSW_MEMORY_MAXIMUM:
	return ((Xv_opaque) folio->es_mem_maximum);
      case TEXTSW_MULTI_CLICK_SPACE:
	return ((Xv_opaque) folio->multi_click_space);
      case TEXTSW_MULTI_CLICK_TIMEOUT:
	return ((Xv_opaque) folio->multi_click_timeout);
      case TEXTSW_NO_RESET_TO_SCRATCH:
	return ((Xv_opaque)
		BOOL_FLAG_VALUE(folio->state, TXTSW_NO_RESET_TO_SCRATCH));
      case TEXTSW_NOTIFY_LEVEL:
	return ((Xv_opaque) folio->notify_level);
      case TEXTSW_NOTIFY_PROC:
	return ((Xv_opaque) folio->notify);
      case TEXTSW_READ_ONLY:
	return ((Xv_opaque)
		BOOL_FLAG_VALUE(folio->state, TXTSW_READ_ONLY_ESH));
      case TEXTSW_TAB_WIDTH:
	return ((Xv_opaque) ei_get(folio->views->eih, EI_TAB_WIDTH));
      case TEXTSW_TEMP_FILENAME:
	return ((Xv_opaque) folio->temp_filename);
      case TEXTSW_UPPER_CONTEXT:
	return ((Xv_opaque) ev_get((Ev_handle) (view->e_view),
	    EV_CHAIN_UPPER_CONTEXT));
      case TEXTSW_LINE_BREAK_ACTION:
	return ((Xv_opaque) ev_get((Ev_handle) (view->e_view), EV_RIGHT_BREAK));

	/* Super-class attributes that we override. */
      case WIN_MENU:{
	    Pkg_private Menu     textsw_get_unique_menu();

	    return ((Xv_opaque) textsw_get_unique_menu(folio));
	}
      case XV_LEFT_MARGIN:
	return ((Xv_opaque) ev_get(view->e_view, EV_LEFT_MARGIN));
      case XV_RIGHT_MARGIN:
	return ((Xv_opaque) ev_get(view->e_view, EV_RIGHT_MARGIN));
      case WIN_VERTICAL_SCROLLBAR:
	return ((Xv_opaque) SCROLLBAR_FOR_VIEW(view));
      case WIN_TYPE:		/* SunView1.X compatibility */
	return ((Xv_opaque) TEXTSW_TYPE);
      default:
	if (xv_check_bad_attr(&xv_textsw_pkg, attribute) == XV_ERROR) {
	    *status = XV_ERROR;
	}
	return ((Xv_opaque) 0);
    }
}


/* Caller turns varargs into va_list that has already been va_start'd */
Pkg_private     Xv_opaque
textsw_get(abstract, status, attribute, args)
    Textsw          abstract;
    int            *status;	/* initialized by caller */
    Textsw_attribute attribute;
    va_list         args;
{
    Textsw_folio    folio = FOLIO_ABS_TO_REP(abstract);

    return ((Xv_opaque) textsw_get_internal(folio, VIEW_FROM_FOLIO_OR_VIEW(folio),
					    status, attribute, args));

}

/* Caller turns varargs into va_list that has already been va_start'd */
Pkg_private     Xv_opaque
textsw_view_get(view_public, status, attribute, args)
    Textsw_view     view_public;
    int            *status;	/* initialized by caller */
    Textsw_attribute attribute;
    va_list         args;
{
    Textsw_view_handle view = VIEW_PRIVATE(view_public);
    
    return ((Xv_opaque) textsw_get_internal(FOLIO_FROM_VIEW(view), view,
					    status, attribute, args));

}

/* VARARGS1 */
Pkg_private     Xv_opaque
textsw_set(abstract, avlist)
    Textsw          abstract;
    Textsw_attribute avlist[];
{
    Textsw_folio    folio = FOLIO_ABS_TO_REP(abstract);

    return ((Xv_opaque)
	    textsw_set_internal(folio, VIEW_FROM_FOLIO_OR_VIEW(folio),
				avlist, TRUE));
}

/* VARARGS1 */
Pkg_private     Xv_opaque
textsw_view_set(view_public, avlist)
    Textsw_view     view_public;
    Textsw_attribute avlist[];
{
    /*
     * Performance enhancment:  Don't use VIEW_ABS_TO_REP because it calls
     * xv_get(), and we know view_public is a public view handle.
     */

    Textsw_view_handle view = VIEW_PRIVATE(view_public);

    return ((Xv_opaque)
	    textsw_set_internal(FOLIO_FROM_VIEW(view), view, avlist, FALSE));
}

Pkg_private void
#ifdef ANSI_FUNC_PROTO
textsw_notify(Textsw_view_handle view, ...)
#else
textsw_notify(view, va_alist)
    Textsw_view_handle view;
va_dcl
#endif
{
    register Textsw_folio folio;
    int             doing_event;
    AVLIST_DECL;
    va_list         args;

    VA_START(args, view);
    view = VIEW_FROM_FOLIO_OR_VIEW(view);
    MAKE_AVLIST( args, avlist );
    va_end(args);
    folio = FOLIO_FOR_VIEW(view);
    doing_event = (folio->state & TXTSW_DOING_EVENT);
    folio->state &= ~TXTSW_DOING_EVENT;
    folio->notify(VIEW_REP_TO_ABS(view), avlist);
    if (doing_event)
	folio->state |= TXTSW_DOING_EVENT;
}

Pkg_private void
textsw_notify_replaced(folio_or_view, insert_before, length_before,
		       replaced_from, replaced_to, count_inserted)
    Textsw_opaque   folio_or_view;
    Es_index        insert_before;
    Es_index        length_before;
    Es_index        replaced_from;
    Es_index        replaced_to;
    Es_index        count_inserted;
{
    Textsw_view_handle view = VIEW_FROM_FOLIO_OR_VIEW(folio_or_view);
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    int             in_notify_proc =
    folio->state & TXTSW_IN_NOTIFY_PROC;

    folio->state |= TXTSW_IN_NOTIFY_PROC;
    textsw_notify(view, TEXTSW_ACTION_REPLACED,
		  insert_before, length_before,
		  replaced_from, replaced_to, count_inserted, NULL);
    if (!in_notify_proc)
	folio->state &= ~TXTSW_IN_NOTIFY_PROC;
}

Pkg_private     Es_index
#ifdef OW_I18N
textsw_get_contents_wcs(textsw, position, buffer, buffer_length)
#else
textsw_get_contents(textsw, position, buffer, buffer_length)
#endif
    register Textsw_folio textsw;
    Es_index        position;
    CHAR           *buffer;
    register int    buffer_length;

{
    Es_index        next_read_at;
    int             read;

    es_set_position(textsw->views->esh, position);
    next_read_at = es_read(textsw->views->esh, buffer_length, buffer,
			   &read);
    if AN_ERROR
	(read != buffer_length) {
#ifdef OW_I18N
	XV_BZERO(buffer + read, ((buffer_length - read) * sizeof(CHAR)));
#else
	XV_BZERO(buffer + read, buffer_length - read);
#endif
	}
    return (next_read_at);
}

#ifdef OW_I18N

Pkg_private     Es_index
textsw_get_contents(textsw, position, buffer, buffer_length)
    register Textsw_folio textsw;
    Es_index        position;
    char           *buffer;
    register int    buffer_length;

{
    Es_index        next_read_at, wc_position;
    int             read;
    CHAR	   *buf_wcs = MALLOC(buffer_length + 1);

    wc_position = textsw_wcpos_from_mbpos(textsw, position);

    /* Make sure whether position is over the end of contents or not */
    textsw_flush_caches(VIEW_FROM_FOLIO_OR_VIEW(textsw), TFC_STD);
    if (wc_position == TEXTSW_INFINITY ||
	wc_position >= es_get_length(textsw->views->esh)) {
	next_read_at = (Es_index) textsw_get_mb_length(textsw);
	read = 0;
    }
    else {
	es_set_position(textsw->views->esh, wc_position);
	(void) es_read(textsw->views->esh, buffer_length, buf_wcs, &read);
	buf_wcs[read] = NULL;
	read = wcstombs(buffer, buf_wcs, buffer_length);
	next_read_at = (position < 0 ? 0 : position) + read;
    }

    free((char *)buf_wcs);
    if AN_ERROR
	(read != buffer_length) {
	XV_BZERO(buffer + read, buffer_length - read);
	}
    return (next_read_at);
}
#endif /* OW_I18N */
