#ifndef lint
#ifdef sccs
static char	sccsid[] = "@(#)txt_once.c 20.131 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Initialization and finalization of text subwindows.
 */

#include <xview_private/primal.h>
#include <xview/textsw.h>
#include <xview_private/txt_impl.h>
#include <fcntl.h>
#include <signal.h>
#include <pixrect/pr_util.h>

#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

#include <pixrect/pixfont.h>
#include <xview/rect.h>
#include <xview/win_struct.h>
#include <xview/win_notify.h>
#include <xview/window.h>
#include <xview/notice.h>
#include <xview/frame.h>
#include <xview/font.h>
#include <xview/openmenu.h>
#include <xview/defaults.h>
#include <xview/cursor.h>
#include <xview/screen.h>
#include <xview_private/win_keymap.h>
#ifdef OW_I18N
#include <xview_private/draw_impl.h>
#endif
#ifdef SVR4
#include <dirent.h>
#else
#include <sys/dir.h>
#endif /* SVR4 */

Pkg_private void textsw_destroy_esh(), textsw_notify_replaced();
Pkg_private Es_status textsw_checkpoint();
Pkg_private long textsw_get_from_defaults();
Pkg_private Es_handle ps_create(),
                es_mem_create(),
                textsw_create_mem_ps(),
                textsw_create_file_ps();
Pkg_private Ei_handle ei_plain_text_create();
Pkg_private Ev_chain ev_create_chain(), ev_destroy_chain_and_views();
Pkg_private Es_index textsw_set_insert();
extern int      gettimeofday();

Pkg_private Ev_handle ev_create_view();
Xv_private Attr_avlist attr_find();
Xv_private int      xv_pf_close();
Xv_private char *xv_font_monospace();

Xv_private void textsw_unregister_view();
Xv_private void textsw_register_view();

Textsw_folio    textsw_head;	/* = 0; implicit for cc -A-R */
extern int termsw_creation_flag;

Pkg_private int             STORE_FILE_POPUP_KEY;
Pkg_private int             SAVE_FILE_POPUP_KEY;
Pkg_private int             LOAD_FILE_POPUP_KEY;
Pkg_private int             FILE_STUFF_POPUP_KEY;
Pkg_private int             SEARCH_POPUP_KEY;
Pkg_private int             MATCH_POPUP_KEY;
Pkg_private int             SEL_LINE_POPUP_KEY;
Pkg_private int             TEXTSW_CURRENT_POPUP_KEY;
Pkg_private int             TEXTSW_MENU_DATA_KEY;

/* BUG ALERT:  Is a "duplicate" cursor ever used? */
/*
 * short dup_cursor_data[] = { #include <images/dup_cursor.pr> };
 * mpr_static(textedit_dup_cursor_pr, 16, 16, 1, dup_cursor_data);
 */


Pkg_private void
textsw_init_again(folio, count)
    register Textsw_folio folio;
    register int    count;
{
    register int    i;
    register int    old_count = folio->again_count;
    register string_t *old_again = folio->again;

    VALIDATE_FOLIO(folio);
    folio->again_first = folio->again_last_plus_one = ES_INFINITY;
    folio->again_insert_length = 0;
    folio->again = (string_t *) ((count)
				 ? calloc(count, sizeof(folio->again[0]))
				 : 0);
    for (i = 0; i < count; i++) {
	folio->again[i] = (i < old_count) ? old_again[i] : null_string;
    }
    for (i = folio->again_count; i < old_count; i++) {
	textsw_free_again(folio, &old_again[i]);
    }
    if (old_again)
	free((char *) old_again);
    folio->again_count = count;
}

Pkg_private void
textsw_init_undo(folio, count)
    register Textsw_folio folio;
    register int    count;
{
    register int    i;
    register int    old_count = folio->undo_count;
    register caddr_t *old_undo = folio->undo;

    VALIDATE_FOLIO(folio);
    folio->undo = (caddr_t *) ((count)
			       ? calloc(count, sizeof(folio->undo[0]))
			       : 0);
    for (i = 0; i < count; i++) {
	folio->undo[i] =
	    (i < old_count) ? old_undo[i] : ES_NULL_UNDO_MARK;
    }
    /*
     * old_undo[ [folio->undo_count..old_count) ] are 32-bit quantities, and
     * thus don't need to be deallocated.
     */

    /*-----------------------------------------------------------
    ... but old_undo itself is a 260 byte quantity that should
    be deallocated to avoid a noticeable memory leak.
    This is a fix for bug 1020222.  -- Mick / inserted jcb 7/20/89
    -----------------------------------------------------------*/
    if(old_undo)
          free((char *)old_undo);

    if (old_count == 0 && folio->undo != NULL )
	folio->undo[0] = es_get(folio->views->esh, ES_UNDO_MARK);
    folio->undo_count = count;
}

static int
textsw_view_chain_notify(folio, attributes)
    register Textsw_folio folio;
    Attr_avlist     attributes;
{
    Pkg_private Textsw_view_handle textsw_view_for_entity_view();
    register Ev_handle e_view;
    register Textsw_view_handle view = 0;
    register Attr_avlist attrs;
    Rect           *from_rect, *rect, *to_rect;

    for (attrs = attributes; *attrs; attrs = attr_next(attrs)) {
	switch ((Ev_notify_action) (*attrs)) {
	    /* BUG ALERT: following need to be fleshed out. */
	  case EV_ACTION_VIEW:
	    e_view = (Ev_handle) attrs[1];
	    view = textsw_view_for_entity_view(folio, e_view);
	    break;
	  case EV_ACTION_EDIT:
	    if (view && (folio->notify_level & TEXTSW_NOTIFY_EDIT)) {
		textsw_notify_replaced((Textsw_opaque) view,
				   (Es_index) attrs[1], (Es_index) attrs[2],
				   (Es_index) attrs[3], (Es_index) attrs[4],
				       (Es_index) attrs[5]);
	    }
	    textsw_checkpoint(folio);
	    break;
	  case EV_ACTION_PAINT:
	    if (view && (folio->notify_level & TEXTSW_NOTIFY_PAINT)) {
		rect = (Rect *) attrs[1];
		textsw_notify(view, TEXTSW_ACTION_PAINTED, rect, NULL);
	    }
	    break;
	  case EV_ACTION_SCROLL:
	    if (view && (folio->notify_level & TEXTSW_NOTIFY_SCROLL)) {
		from_rect = (Rect *) attrs[1];
		to_rect = (Rect *) attrs[2];
		textsw_notify(view,
			      TEXTSW_ACTION_SCROLLED, from_rect, to_rect,
			      NULL);
	    }
	    break;
	  default:
	    LINT_IGNORE(ASSERT(0));
	    break;
	}
    }
}

static int
textsw_read_defaults(textsw, defaults)
    register Textsw_folio textsw;
    register Attr_avlist defaults;
{
    char           *def_str;	/* Strings owned by defaults. */
    register Attr_attribute attr;
    Xv_opaque       font = XV_ZERO;
    char	   *name;
    Xv_opaque       textsw_public = TEXTSW_PUBLIC(textsw);
    int             is_client_pane = xv_get(textsw_public,
					    WIN_IS_CLIENT_PANE);

    def_str = defaults_get_string("keyboard.deleteChar",
				  "Keyboard.DeleteChar", "\177");	/* ??? Keymapping
									 * strategy? */
    textsw->edit_bk_char = def_str[0];
    def_str = defaults_get_string("keyboard.deleteWord",
				  "Keyboard.DeleteWord", "\027");	/* ??? Keymapping
									 * strategy? */
    textsw->edit_bk_word = def_str[0];
    def_str = defaults_get_string("keyboard.deleteLine",
				  "Keyboard.DeleteLine", "\025");	/* ??? Keymapping
									 * strategy? */
    textsw->edit_bk_line = def_str[0];
#ifdef OW_I18N
    textsw->need_im = (xv_get(XV_SERVER_FROM_WINDOW(textsw_public), XV_IM) &&
		       xv_get(textsw_public, WIN_USE_IM)) ? TRUE : FALSE;

    /*  Drawing pre-edit text requires doing lots of replace.
     *  This causes the memory buffer to run out very fast,
     *  So if the user do not set this value, we will default it
     *  to TEXTSW_INFINITY.
     */
    if (textsw->need_im)
	textsw->es_mem_maximum =
	    defaults_get_integer_check("text.maxDocumentSize",
	       "Text.MaxDocumentSize", TEXTSW_INFINITY, 0, TEXTSW_INFINITY + 1);
    else
#endif /* OW_I18N */
    textsw->es_mem_maximum =
	defaults_get_integer_check("text.maxDocumentSize",
		     "Text.MaxDocumentSize", 20000, 0, TEXTSW_INFINITY + 1);
    textsw->drag_threshold =
        defaults_get_integer("openWindows.dragThreshold",
                    "OpenWindows.DragThreshold", 5);
#ifndef lint
    if (textsw_get_from_defaults(TEXTSW_ADJUST_IS_PENDING_DELETE))
	textsw->state |= TXTSW_ADJUST_IS_PD;
    else
	textsw->state &= ~TXTSW_ADJUST_IS_PD;
    if (textsw_get_from_defaults(TEXTSW_AUTO_INDENT))
	textsw->state |= TXTSW_AUTO_INDENT;
    else
	textsw->state &= ~TXTSW_AUTO_INDENT;
    if (textsw_get_from_defaults(TEXTSW_BLINK_CARET))
	textsw->caret_state |= TXTSW_CARET_FLASHING;
    else
	textsw->caret_state &= ~TXTSW_CARET_FLASHING;
    if (textsw_get_from_defaults(TEXTSW_CONFIRM_OVERWRITE))
	textsw->state |= TXTSW_CONFIRM_OVERWRITE;
    else
	textsw->state &= ~TXTSW_CONFIRM_OVERWRITE;
    if (textsw_get_from_defaults(TEXTSW_STORE_CHANGES_FILE))
	textsw->state |= TXTSW_STORE_CHANGES_FILE;
    else
	textsw->state &= ~TXTSW_STORE_CHANGES_FILE;
    if (textsw_get_from_defaults(TEXTSW_AGAIN_RECORDING))
        textsw->state &= ~TXTSW_NO_AGAIN_RECORDING;
    else
        textsw->state |= TXTSW_NO_AGAIN_RECORDING;
#endif
    if (defaults_get_boolean("text.retained", "Text.Retained", False))
	textsw->state |= TXTSW_RETAINED;
    else
	textsw->state &= ~TXTSW_RETAINED;

#ifndef lint
    textsw->multi_click_space =
	textsw_get_from_defaults(TEXTSW_MULTI_CLICK_SPACE);
    textsw->multi_click_timeout =
	textsw_get_from_defaults(TEXTSW_MULTI_CLICK_TIMEOUT);
    textsw->insert_makes_visible =
	(Textsw_enum) textsw_get_from_defaults(TEXTSW_INSERT_MAKES_VISIBLE);
#endif

    /*
     * The following go through the standard textsw_set mechanism
     * (eventually) because they rely on all of the side-effects that
     * accompany textsw_set calls.
     */
#ifndef lint
    *defaults++ = attr = TEXTSW_AGAIN_LIMIT;
    *defaults++ = textsw_get_from_defaults(attr);
    *defaults++ = attr = TEXTSW_HISTORY_LIMIT;
    *defaults++ = textsw_get_from_defaults(attr);
    *defaults++ = attr = TEXTSW_AUTO_SCROLL_BY;
    *defaults++ = textsw_get_from_defaults(attr);
    *defaults++ = attr = TEXTSW_LOWER_CONTEXT;
    *defaults++ = textsw_get_from_defaults(attr);
    *defaults++ = attr = TEXTSW_UPPER_CONTEXT;
    *defaults++ = textsw_get_from_defaults(attr);

#ifdef OW_I18N

    defaults_set_locale(NULL, XV_LC_BASIC_LOCALE);
    name = xv_font_monospace();
    defaults_set_locale(NULL, NULL);

    if (name && ((int)strlen(name) > 0)) {
	font = (Xv_opaque) xv_pf_open(name);
    } else
	font = (Xv_opaque) 0;

    if (!font) {
	Xv_opaque       parent_font;
	int             scale, size;

	parent_font = (Xv_opaque) xv_get(textsw_public, WIN_FONT);
	scale = (int) xv_get(parent_font, FONT_SCALE);

	if (scale > 0) {
	    font = (Xv_opaque) xv_find(textsw_public, FONT,
			FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
			FONT_SCALE, scale,
			NULL);
	} else {
	    size = (int) xv_get(parent_font, FONT_SIZE);
	    font = (Xv_opaque) xv_find(textsw_public, FONT,
			FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
			FONT_SIZE, (size > 0) ? size : FONT_SIZE_DEFAULT,
			NULL);
	}
 
	if (!font)
	    font = (Xv_opaque) xv_find(NULL, FONT, NULL);
    }
    if (font) {
	attr = XV_FONT;
	*defaults++ = attr;
	*defaults++ = font;
    }
#else /* OW_I18N */

    name = xv_font_monospace();
    if (name && ((int)strlen(name) > 0)) {
	font = (Xv_opaque) xv_pf_open(name);
    } else
	font = (Xv_opaque) 0;

    if (is_client_pane) {
	Xv_opaque       parent_font;
	int             scale, size;

	if (!font) {
	    parent_font = xv_get(textsw_public, WIN_FONT);
	    scale = xv_get(parent_font, FONT_SCALE);
	    if (scale > 0) {
		font = (Xv_opaque) xv_find(textsw_public, FONT,
				FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
		/* FONT_FAMILY,        FONT_FAMILY_SCREEN, */
		       FONT_SCALE, (scale > 0) ? scale : FONT_SCALE_DEFAULT,
					   NULL);
	    } else {
		size = xv_get(parent_font, FONT_SIZE);
		font = (Xv_opaque) xv_find(textsw_public, FONT,
				FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
		/* FONT_FAMILY,        FONT_FAMILY_SCREEN, */
			   FONT_SIZE, (size > 0) ? size : FONT_SIZE_DEFAULT,
					   NULL);
	    }
	    if (font) {
		attr = XV_FONT;
		*defaults++ = attr;
		*defaults++ = font;
	    }
	}
    } else {
	if (!font) {
	    Xv_opaque       parent_font = xv_get(textsw_public, WIN_FONT);
	    int             scale = xv_get(parent_font, FONT_SCALE);

	    if (scale > 0) {
	        font = (Xv_opaque) xv_find(textsw_public, FONT,
				FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
	    /* FONT_FAMILY,        FONT_FAMILY_SCREEN, */
		       FONT_SCALE, (scale > 0) ? scale : FONT_SCALE_DEFAULT,
				       NULL);
	    } else {
	        int             size = xv_get(parent_font, FONT_SIZE);
	        font = (Xv_opaque) xv_find(textsw_public, FONT,
				FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
	    /* FONT_FAMILY,        FONT_FAMILY_SCREEN, */
		       FONT_SIZE, (size > 0) ? size : FONT_SIZE_DEFAULT, NULL);
	    }
	}
	if (font) {
	    attr = XV_FONT;
	    *defaults++ = attr;
	    *defaults++ = font;
	}
    }
    if ((!font) && is_client_pane) {
	attr = XV_FONT;
	font = textsw_get_from_defaults(attr);
	if (font) {
	    attr = XV_FONT;
	    *defaults++ = attr;
	    *defaults++ = font;
	}
    }
#endif /* OW_I18N */

    *defaults++ = attr = TEXTSW_LINE_BREAK_ACTION;
    *defaults++ = textsw_get_from_defaults(attr);
    *defaults++ = attr = TEXTSW_LEFT_MARGIN;
    *defaults++ = textsw_get_from_defaults(attr);
    *defaults++ = attr = TEXTSW_RIGHT_MARGIN;
    *defaults++ = textsw_get_from_defaults(attr);
    *defaults++ = attr = TEXTSW_TAB_WIDTH;
    *defaults++ = textsw_get_from_defaults(attr);
    *defaults++ = attr = TEXTSW_CONTROL_CHARS_USE_FONT;
    *defaults++ = textsw_get_from_defaults(attr);
#endif
    *defaults = 0;
}

#define needctrlmask(c)		((0<=(c) && (c)<=31) || (128<=(c) && (c)<=159))
#define needshiftmask(c)	((64<=(c) && (c)<=95) || (192<=(c) && (c)<=223))
#define needmetamask(c)		(128<=(c) && (c)<=255)

static int
textsw_layout(textsw, child, op, d1, d2, d3, d4, d5)
    Textsw          textsw;
    Xv_Window       child;
    Window_layout_op op;
/* Alpha compatibility, mbuck@debian.org */
#if defined(__alpha)
    unsigned long   d1, d2, d3, d4, d5;
#else
    int             d1, d2, d3, d4, d5;
#endif
{
    Textsw_folio    folio = TEXTSW_PRIVATE(textsw);

    switch (op) {
      case WIN_DESTROY:
	/* if (xv_get(child, XV_IS_SUBTYPE_OF, TEXTSW_VIEW)) */
	textsw_unregister_view(textsw, child);
	break;
      case WIN_CREATE:
	if (xv_get(child, XV_IS_SUBTYPE_OF, TEXTSW_VIEW)) {
	    textsw_register_view(textsw, child);
	}
      default:
	break;
    }

    if (folio->layout_proc != NULL)
	return (folio->layout_proc(textsw, child, op,
				   d1, d2, d3, d4, d5));
    else
	return TRUE;

}

Pkg_private void
textsw_set_cursor(textsw, cursor_type)
    Textsw          textsw;
    int             cursor_type;
{
    Textsw_folio    folio = FOLIO_ABS_TO_REP(textsw);

    if (folio->first_view->cursor_type != cursor_type) {
	Textsw_view_handle view;
	Xv_object       screen = xv_get(textsw, XV_SCREEN);
	Xv_object       server = xv_get(screen, SCREEN_SERVER);
	Xv_Cursor       cursor = (Xv_Cursor) xv_get(server, XV_KEY_DATA,
						    cursor_type);

	/*
	 * BUG: Should print out error message
	 */
	if (cursor == XV_ZERO)
	    return;
	FORALL_TEXT_VIEWS(folio, view) {
	    xv_set(VIEW_REP_TO_ABS(view), WIN_CURSOR, cursor, NULL);
	    view->cursor_type = cursor_type;
	}
    }
}


Pkg_private     Textsw_view_handle
textsw_view_init_internal(view, status)
    register Textsw_view_handle view;
    Textsw_status  *status;
{
    Textsw_view     view_public = VIEW_PUBLIC(view);
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    PIXFONT        *font = (PIXFONT *) ei_get(folio->views->eih, EI_FONT);
    Xv_opaque       public_textsw = TEXTSW_PUBLIC(folio);
    Xv_object       screen = xv_get(public_textsw, XV_SCREEN);

    *status = TEXTSW_STATUS_OTHER_ERROR;

    (void) xv_set(view_public,
		  WIN_RETAINED, xv_get(screen, SCREEN_RETAIN_WINDOWS),
		  OPENWIN_AUTO_CLEAR, FALSE,
		  WIN_BIT_GRAVITY, ForgetGravity,
		  XV_FONT, font,
		  WIN_X_PAINT_WINDOW, TRUE,
		  NULL);

    view->cursor_type = CURSOR_BASIC_PTR;

    textsw_set_base_mask(view_public);

    view->e_view = ev_create_view(folio->views, view_public, &view->rect);

    if (view->e_view == EV_NULL)
	goto Error_Return;
    (void) ev_set(view->e_view, EV_NO_REPAINT_TIL_EVENT, FALSE, NULL);

    if (!folio->first_view) {
	textsw_register_view(FOLIO_REP_TO_ABS(folio), view_public);
    }
    return (view);

Error_Return:
    free((char *) folio);
    free((char *) view);
    return (0);

}



#define	TXTSW_NEED_SELN_CLIENT	(Seln_client)1

Pkg_private     Textsw_folio
textsw_init_internal(folio, status, default_notify_proc, attrs)
    Textsw_folio    folio;
    Textsw_status  *status;
    int             (*default_notify_proc) ();
    Textsw_attribute *attrs;
{
    register Textsw textsw = TEXTSW_PUBLIC(folio);
    Attr_attribute  defaults_array[ATTR_STANDARD_SIZE];
    Attr_avlist     defaults;
    Es_handle       ps_esh;
    Ei_handle       plain_text_eih;
    char	   *name = 0;
    CHAR	    scratch_name[MAXNAMLEN];
    Es_status       es_status;
    Frame	    frame;
    Xv_Notice	    text_notice;
    extern void     textsw_split_init_proc();
#ifdef OW_I18N
    CHAR	    name_wc[MAXNAMLEN];
#ifdef FULL_R5
    XVaNestedList   va_nested_list;
#endif /* FULL_R5 */	
    
    name_wc[0] = NULL;
#endif    
    folio->magic = TEXTSW_MAGIC;

    if ((plain_text_eih = ei_plain_text_create()) == 0)
	goto Error_Return;

    defaults = defaults_array;
    /*
     * The following go through the standard textsw_set mechanism
     * (eventually) because they rely on all of the side-effects that
     * accompany textsw_set calls.
     */
    *defaults++ = TEXTSW_NOTIFY_PROC;
    *defaults++ = (Attr_attribute) default_notify_proc;
    *defaults++ = TEXTSW_INSERTION_POINT;
    *defaults++ = 0;

    *defaults = 0;
    textsw_read_defaults(folio, defaults);
    /*
     * Special case the initial attributes that must be handled as part of
     * the initial set up.  Optimizing out creating a memory entity_stream
     * and then replacing it with a file causes most of the following
     * complications.
     */
    defaults = attr_find(defaults_array, XV_FONT);
    if (*defaults) {
	(void) ei_set(plain_text_eih, EI_FONT, defaults[1], NULL);
	ATTR_CONSUME(*defaults);
    } else {
	(void) ei_set(plain_text_eih,
		      EI_FONT, xv_get(textsw, XV_FONT),
		      NULL);
    }
    folio->state |= TXTSW_OPENED_FONT;
#ifdef FULL_R5
#ifdef OW_I18N
    if (folio->ic && (folio->xim_style & (XIMPreeditArea | XIMPreeditPosition | XIMPreeditNothing))) {  
        va_nested_list = XVaCreateNestedList(NULL, 
					     XNLineSpace, (int)ei_get(plain_text_eih, EI_LINE_SPACE), 
						 NULL);
        XSetICValues(folio->ic, XNPreeditAttributes,va_nested_list, 
        	     NULL);
        XFree(va_nested_list);	     
    }
#endif /* OW_I18N */	
#endif /* FULL_R5 */	
    /*
     * Look for client provided entity_stream creation proc, and client
     * provided data, which must be passed to the creation proc.
     */
    defaults = attr_find(attrs, TEXTSW_ES_CREATE_PROC);
    if (*defaults) {
	ATTR_CONSUME(*defaults);
	folio->es_create = (Es_handle(*) ()) defaults[1];
    } else
	folio->es_create = ps_create;
    defaults = attr_find(attrs, TEXTSW_CLIENT_DATA);
    if (*defaults) {
	ATTR_CONSUME(*defaults);
	folio->client_data = defaults[1];
    }

    if (termsw_creation_flag)
	folio->es_mem_maximum = 130;
    else {
        defaults = attr_find(attrs, TEXTSW_MEMORY_MAXIMUM);
        if (*defaults) {
	    folio->es_mem_maximum = (unsigned) defaults[1];
        }
        if (folio->es_mem_maximum == 0) {
	    folio->es_mem_maximum = TEXTSW_INFINITY;
        } else if (folio->es_mem_maximum < 128)
	    folio->es_mem_maximum = 128;
    }

    defaults = attr_find(attrs, TEXTSW_FILE);
    if (*defaults) {
	ATTR_CONSUME(*defaults);
	name = (char *) defaults[1];
#ifdef OW_I18N
	if (name)
	    (void) mbstowcs(name_wc, name, MAXNAMLEN);
#endif
    }

#ifdef OW_I18N
    defaults = attr_find(attrs, TEXTSW_FILE_WCS);
    if (*defaults) {
	char	name_mb[MAXNAMLEN];

	ATTR_CONSUME(*defaults);
	STRCPY(name_wc, (CHAR  *) defaults[1]);
	(void) wcstombs(name_mb, name_wc, MAXNAMLEN);
	name = name_mb;
    }
    if (name_wc[0] != NULL) {		/* } for match */
	ps_esh = textsw_create_file_ps(folio, name_wc,
				       scratch_name, &es_status);
#else /* OW_I18N */
    if (name) {
	ps_esh = textsw_create_file_ps(folio, name,
				       scratch_name, &es_status);
#endif /* OW_I18N */
	if (es_status != ES_SUCCESS) {
	    frame = (Frame)xv_get((Xv_opaque)textsw, WIN_FRAME);
            text_notice = (Xv_Notice)xv_get(frame, 
                                XV_KEY_DATA, text_notice_key, 
				NULL);
            if (!text_notice)  {
                text_notice = xv_create(frame, NOTICE,
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
                        XV_MSG("Can't load specified file:"),
                        name,
                        XV_MSG("Starting with empty buffer."),
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
                        XV_MSG("Can't load specified file:"),
                        name,
                        XV_MSG("Starting with empty buffer."),
                        0,
                        NOTICE_BUTTON_YES, XV_MSG("Continue"),
                        XV_SHOW, TRUE, 
                        NULL);
            }

	    *status = TEXTSW_STATUS_CANNOT_OPEN_INPUT;
	}
    } else {
	Attr_avlist     attr = (Attr_avlist) attrs;
	int             have_file_contents;
	char           *initial_greeting;
#ifdef OW_I18N
	CHAR           *initial_greeting_ws;
	int             free_string = 0;
	extern CHAR    _xv_null_string_wc[];
#endif

	attr = attr_find(attrs, TEXTSW_FILE_CONTENTS);
	have_file_contents = (*attr != 0);
	/*
	 * Always look for TEXTSW_CONTENTS in defaults_array so that it is
	 * freed, even if it is not used, to avoid storage leak. Similarly,
	 * always consume TEXTSW_CONTENTS from attrs.
	 */
	defaults = attr_find(defaults_array, TEXTSW_CONTENTS);
	attr = attr_find(attrs, TEXTSW_CONTENTS);
	initial_greeting =
	    (have_file_contents) ? ""
	    : ((*attr) ? (char *) attr[1]
	       : ((*defaults) ? (char *) defaults[1]
		  : ""));
#ifdef OW_I18N
	if ((initial_greeting) && (unsigned) strlen(initial_greeting) >  0) {
	    initial_greeting_ws = _xv_mbstowcsdup(initial_greeting);
	    free_string = TRUE;
	} else {
	    attr = attr_find(attrs, TEXTSW_FILE_CONTENTS_WCS);
	    if (!have_file_contents)
		have_file_contents = (*attr != 0);
	/*
	 * Always look for TEXTSW_CONTENTS_WCS in defaults_array so that it is
	 * freed, even if it is not used, to avoid storage leak. Similarly,
	 * always consume TEXTSW_CONTENTS_WCS from attrs.
	 */
	    defaults = attr_find(defaults_array, TEXTSW_CONTENTS_WCS);
	    attr = attr_find(attrs, TEXTSW_CONTENTS_WCS);
	    initial_greeting_ws =
		(have_file_contents) ? _xv_null_string_wc
		: ((*attr) ? (CHAR  *) attr[1]
		   : ((*defaults) ? (CHAR  *) defaults[1]
		      : _xv_null_string_wc));
	}
	if (!initial_greeting_ws) {
	    initial_greeting_ws = _xv_null_string_wc;
	}
	
	ps_esh = es_mem_create((unsigned) STRLEN(initial_greeting_ws),
			       initial_greeting_ws);
	if (free_string)
	    free((char *)initial_greeting_ws);		       
#else /* OW_I18N */
	ps_esh = es_mem_create((unsigned) strlen(initial_greeting),
			       initial_greeting);
#endif /* OW_I18N */
	ps_esh = textsw_create_mem_ps(folio, ps_esh);
	if (*defaults) {
	    ATTR_CONSUME(*defaults);
	    free((char *)defaults[1]);
	}
	if (*attr) {
	    ATTR_CONSUME(*attr);
	}
    }

    if (ps_esh == ES_NULL)
	goto Error_Return;
    /*
     * Make the view chain and the initial view(s).
     */
    folio->views = ev_create_chain(ps_esh, plain_text_eih);
    (void) ev_set((Ev_handle) 0, folio->views,
		  EV_CHAIN_DATA, folio,
		  EV_CHAIN_NOTIFY_PROC, textsw_view_chain_notify,
		  EV_CHAIN_NOTIFY_LEVEL, EV_NOTIFY_ALL,
		  NULL);

    /*
     * Set the default, and then the client's, attributes.
     */
    if (!folio->first_view) {
	(void) textsw_set_null_view_avlist(folio, defaults_array);
	(void) xv_set_avlist(textsw, defaults_array);

	(void) textsw_set_null_view_avlist(folio, attrs);
    }
    folio->layout_proc = (int (*) ()) xv_get(textsw, WIN_LAYOUT_PROC);

    (void) xv_set_avlist(textsw, (Attr_avlist)attrs);
    /* This xv_set call should be combined with the xv_set_avlist call.
     * BUG ALERT:  This code assumes that the default "text.enableScrollbar"
     * is always TRUE, since that is what OPEN LOOK requires.
     * This default should be eliminated in "txt_attr.c".
     */
    (void) xv_set(textsw,
		  WIN_LAYOUT_PROC, textsw_layout,
		  OPENWIN_ADJUST_FOR_VERTICAL_SCROLLBAR, TRUE,
		  XV_FOCUS_RANK, XV_FOCUS_PRIMARY,
		  NULL);

    /*
     * Make last_point/_adjust/_ie_time close (but not too close) to current
     * time to avoid overflow in tests for multi-click.
     */
    (void) gettimeofday(&folio->last_point, (struct timezone *) 0);
    folio->last_point.tv_sec -= 1000;
    folio->last_adjust = folio->last_point;
    folio->last_ie_time = folio->last_point;
    /*
     * Final touchups.
     */
    folio->trash = ES_NULL;
    folio->to_insert_next_free = folio->to_insert;
    folio->to_insert_counter = 0;
    folio->span_level = EI_SPAN_POINT;
    SET_TEXTSW_TIMER(&folio->timer);
    EV_INIT_MARK(folio->save_insert);
    folio->owed_by_filter = 0;
    /*
     * Get the user filters in the ~/.textswrc file. Note that their
     * description is read only once per process, and shared among all of the
     * folios in each process.
     */
    if (textsw_head) {
	folio->key_maps = textsw_head->key_maps;
    } else
	(void) textsw_parse_rc(folio);
    /*
     * Initialize selection service data. Note that actual hookup will only
     * be attempted when necessary.
     */
    folio->selection_client = TXTSW_NEED_SELN_CLIENT;
    timerclear(&folio->selection_died);
    folio->selection_func.function = SELN_FN_ERROR;
    folio->selection_holder = (Seln_holder *) 0;
    *status = TEXTSW_STATUS_OKAY;
    folio->state |= TXTSW_INITIALIZED;
    folio->temp_filename = NULL;
    (void) textsw_menu_init(folio);

    /*
     * Link this folio in.
     */
    if (textsw_head)
	folio->next = textsw_head;
    textsw_head = folio;
    xv_set(textsw, OPENWIN_SPLIT, OPENWIN_SPLIT_INIT_PROC, textsw_split_init_proc, 0, NULL);

    /* set delete replaces clipboard flag based on resource
       Text.DeleteReplacesClipboard */

    if (defaults_get_boolean("text.deleteReplacesClipboard",
			     "Text.DeleteReplacesClipboard",False)) {
	folio->state |= TXTSW_DELETE_REPLACES_CLIPBOARD;
    }
#ifdef OW_I18N
    folio->ic = NULL;
    EV_INIT_MARK(folio->preedit_start);
    folio->blocking_newline = FALSE;
    folio->locale_is_ale = (int) ei_get(plain_text_eih, EI_LOCALE_IS_ALE);
    (void) getwidth(&folio->euc_width);
    folio->euc_width._eucw2++;	/* increment one for SS2 */
    folio->euc_width._eucw3++;	/* increment one for SS3 */
    textsw_init_convpos(folio);
    /*
     * Textsw should have IC even if TEXTSW_READ_ONLY is True.
     * Because read_only mode may be changed to edit mode later on
     * by setting TEXTSW_READ_ONLY to TRUE, or by loading another file.
     */
    if (folio->need_im) {
	Xv_private void		textsw_pre_edit_start();
	Xv_private void		textsw_pre_edit_draw();
	Xv_private void		textsw_pre_edit_done();

	/* Set preedit callbacks */
	xv_set(textsw,
	       WIN_IC_PREEDIT_START,
			(XIMProc)textsw_pre_edit_start, (XPointer)textsw,
	       WIN_IC_PREEDIT_DRAW,
			(XIMProc)textsw_pre_edit_draw, (XPointer)textsw,
	       WIN_IC_PREEDIT_DONE,
			(XIMProc)textsw_pre_edit_done, (XPointer)textsw,
	       NULL);

	folio->start_pecb_struct.callback = (XIMProc)textsw_pre_edit_start;
	folio->start_pecb_struct.client_data = (XPointer)textsw;

	folio->draw_pecb_struct.callback = (XIMProc)textsw_pre_edit_draw;
	folio->draw_pecb_struct.client_data = (XPointer)textsw;

	folio->done_pecb_struct.callback = (XIMProc)textsw_pre_edit_done;
	folio->done_pecb_struct.client_data = (XPointer)textsw;
    }
#endif /* OW_I18N */

    return (folio);

Error_Return:
    free((char *) folio);
    return (0);


}



Pkg_private void
textsw_setup_scrollbar(sb)
    Scrollbar       sb;
{

    Pkg_private int textsw_compute_scroll();
    if (sb)
	(void) xv_set(sb,
		      SCROLLBAR_PIXELS_PER_UNIT, 1,
		      SCROLLBAR_OBJECT_LENGTH, 0,
		      SCROLLBAR_VIEW_START, 0,
		      SCROLLBAR_VIEW_LENGTH, 0,
		      SCROLLBAR_COMPUTE_SCROLL_PROC,
		      textsw_compute_scroll,
		      SCROLLBAR_SPLITTABLE, TRUE,
		      SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
		      WIN_COLLAPSE_MOTION_EVENTS, TRUE,
		      NULL);

}

static void
textsw_destroy_popup(key_data_name, textsw, parent_frame)
     Textsw textsw;
     int key_data_name;
     Frame parent_frame;
{
    Frame           popup_frame = xv_get(parent_frame,
					 XV_KEY_DATA, key_data_name);
    if (popup_frame && 
	(textsw==xv_get(popup_frame,
			XV_KEY_DATA, TEXTSW_CURRENT_POPUP_KEY))) {
	xv_set(parent_frame, XV_KEY_DATA, key_data_name, 0,
	       NULL);
	xv_destroy(popup_frame);
    }
}

static void
textsw_folio_cleanup(folio)
    register Textsw_folio folio;
{
    Key_map_handle  this_key, next_key;
    Textsw textsw = FOLIO_REP_TO_ABS(folio);
    Frame parent_frame = xv_get(textsw,WIN_FRAME);

    textsw_init_again(folio, 0);/* Flush AGAIN info */
    /*
     * Clean up of AGAIN info requires valid esh in case of piece frees.
     * textsw_destroy_esh may try to give Shelf to Seln. Svc., so need to
     * keep textsw->selection_client around.
     */
    textsw_destroy_esh(folio, folio->views->esh);

    /* destroy any popups which are accessing this textsw */

    textsw_destroy_popup(STORE_FILE_POPUP_KEY,textsw,parent_frame);
    textsw_destroy_popup(SAVE_FILE_POPUP_KEY,textsw,parent_frame);
    textsw_destroy_popup(LOAD_FILE_POPUP_KEY,textsw,parent_frame);
    textsw_destroy_popup(FILE_STUFF_POPUP_KEY,textsw,parent_frame);
    textsw_destroy_popup(SEARCH_POPUP_KEY,textsw,parent_frame);
    textsw_destroy_popup(MATCH_POPUP_KEY,textsw,parent_frame);
    textsw_destroy_popup(SEL_LINE_POPUP_KEY,textsw,parent_frame);

    /*
     * exit() snaps TCP connection to Seln. Svc., so it will know we are gone
     * when the process finishes dying.
     */
    if (folio->selection_client &&
	(folio->selection_client != TXTSW_NEED_SELN_CLIENT)) {
	seln_destroy(folio->selection_client);
	folio->selection_client = 0;
    }
    if (folio->state & TXTSW_OPENED_FONT) {
	PIXFONT        *font = (PIXFONT *)
	ei_get(folio->views->eih, EI_FONT);
	xv_pf_close(font);
    }
    folio->views->eih = ei_destroy(folio->views->eih);
    (void) ev_destroy_chain_and_views(folio->views);
#ifdef OW_I18N
    textsw_destroy_convpos(folio);
#endif
    folio->caret_state &= ~TXTSW_CARET_ON;
    textsw_remove_timer(folio);
    /*
     * Unlink the textsw from the chain.
     */
    if (folio == textsw_head) {
	textsw_head = folio->next;
	if (folio->next == 0) {
	    /*
	     * Last textsw in process, so free key_maps.
	     */

	    for (this_key = folio->key_maps; this_key;
		 this_key = next_key) {
		next_key = this_key->next;
		free((char *) this_key);
	    }
	}
    } else {
	Textsw_folio    temp;
	for (temp = textsw_head; temp; temp = temp->next) {
	    if (folio == temp->next) {
		temp->next = folio->next;
		break;
	    }
	}
    }
    /*
    *  The following code should reduce the memory leakage
    *  in a textsw create/destroy cycle from 8k to 1k.
    */
    if(folio->menu) menu_destroy(folio->menu);
    if(folio->menu_table) free(folio->menu_table);
    if(folio->undo) free(folio->undo);
 
    free((char *) folio);
}

static void
textsw_unlink_view(folio, view)
    register Textsw_folio folio;
    register Textsw_view_handle view;
{
    Textsw_view_handle temp_view;

    /* Unlink view from view chain */
    if (view == folio->first_view)
	folio->first_view = folio->first_view->next;
    else {
	FORALL_TEXT_VIEWS(folio, temp_view) {
	    if (temp_view->next == view) {
		temp_view->next = view->next;
		break;
	    }
	}
    }
}

static void
textsw_view_cleanup(view)
    register Textsw_view_handle view;
{
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);

    view->state |= TXTSW_VIEW_DYING;
    /* Warn client that view is dying BEFORE killing it. */
    if (folio->notify_level & TEXTSW_NOTIFY_DESTROY_VIEW)
	textsw_notify(view, TEXTSW_ACTION_DESTROY_VIEW, NULL);

    /* This is for the panel menu of textedit */
    if ((!(folio->state & TXTSW_DESTROY_ALL_VIEWS)) &&
	(folio->first_view != NULL)) {
	int             i;
	Textsw          textsw = VIEW_REP_TO_ABS(folio->first_view);

	for (i = (int) TXTSW_FILE_SUB_MENU;
	     i <= (int) TXTSW_FIND_SUB_MENU; i++) {
	    if (folio->sub_menu_table[i])
		menu_set(folio->sub_menu_table[i],
			 MENU_CLIENT_DATA, textsw,
			 NULL);
	}
	for (i = (int) TEXTSW_MENU_FILE_CMDS;
	     i < (int) TEXTSW_MENU_LAST_CMD; i++) {
	    if (folio->menu_table[i])
		menu_set(folio->menu_table[i],
			 MENU_CLIENT_DATA, textsw,
			 NULL);
	}
    }
    /* Remove from the list. */
    textsw_unlink_view(folio, view);
    /* Destroy all of the view's auxillary objects and any back links */
    ev_destroy(view->e_view);
    free((char *) view);
}


Pkg_private int
textsw_folio_destroy(folio_public, status)
    Textsw          folio_public;
    register Destroy_status status;
{
    register Textsw_folio folio = TEXTSW_PRIVATE(folio_public);
    Frame	frame;
    Xv_Notice	text_notice;

    switch (status) {
      case DESTROY_CHECKING:
	if (textsw_has_been_modified(folio_public) &&
	    (folio->ignore_limit != TEXTSW_INFINITY)) {
	    int             result;

	    frame = (Frame)xv_get(folio_public, WIN_FRAME);
            text_notice = (Xv_Notice)xv_get(frame, 
                                XV_KEY_DATA, text_notice_key, 
				NULL);

            if (!text_notice)  {
                text_notice = xv_create(frame, NOTICE,
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("The text has been edited.\n\
\n\
You may discard edits now and quit, or cancel\n\
the request to Quit and go back and either save the\n\
contents or store the contents as a new file."),
                        0,
                        NOTICE_BUTTON_YES, 
			XV_MSG("Cancel, do NOT Quit"),
                        NOTICE_BUTTON, 
			XV_MSG("Discard edits, then Quit"), 123,
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
		    XV_MSG("The text has been edited.\n\
\n\
You may discard edits now and quit, or cancel\n\
the request to Quit and go back and either save the\n\
contents or store the contents as a new file."),
                    0,
                    NOTICE_BUTTON_YES, 
		    XV_MSG("Cancel, do NOT Quit"),
                    NOTICE_BUTTON, 
		    XV_MSG("Discard edits, then Quit"), 123,
		    NOTICE_STATUS, &result,
                    XV_SHOW, TRUE, 
                    NULL);
            }

	    if ((result == ACTION_STOP) || (result == NOTICE_YES)
		|| (result == NOTICE_FAILED)) {
		return (XV_ERROR);
	    } else {
		(void) textsw_reset(folio_public, 0, 0);
		(void) textsw_reset(folio_public, 0, 0);
	    }
	}
	break;
      case DESTROY_CLEANUP:{
	    Textsw_view_handle temp_view = folio->first_view, next;

	    folio->state |= TXTSW_DESTROY_ALL_VIEWS;

	    while (temp_view) {
		next = temp_view->next;
		notify_post_destroy(VIEW_PUBLIC(temp_view), DESTROY_CLEANUP,
				    NOTIFY_IMMEDIATE);
		temp_view = next;
	    }

	    (void) xv_set(folio_public, WIN_LAYOUT_PROC, folio->layout_proc, 
			  NULL);
	    textsw_folio_cleanup(folio);
	    break;
	}
      case DESTROY_PROCESS_DEATH:
	/*
	 * Clean up of AGAIN info requires valid esh in case of piece frees.
	 * textsw_destroy_esh may try to give Shelf to Seln. Svc., so need to
	 * keep textsw->selection_client around.
	 */
	textsw_destroy_esh(folio, folio->views->esh);
	break;

      default:			/* Conservative in face of new cases. */
	break;
    }
    if (status == DESTROY_PROCESS_DEATH || status == DESTROY_CLEANUP) {
	(void) notify_remove((Notify_client)folio_public);
	(void) notify_remove((Notify_client)folio);
    }
    return (XV_OK);
}

Pkg_private int
textsw_view_destroy(view_public, status)
    Textsw_view     view_public;
    register Destroy_status status;
{

    register Textsw_view_handle view = VIEW_PRIVATE(view_public);
    register Textsw_folio folio = FOLIO_FOR_VIEW(view);
    register Textsw folio_public = TEXTSW_PUBLIC(folio);
    Frame	    frame;
    Xv_Notice	    text_notice;

    switch (status) {
      case DESTROY_CHECKING:
	if (!folio->first_view->next &&
	    textsw_has_been_modified(folio_public) &&
	    (folio->ignore_limit != TEXTSW_INFINITY)) {
	    int             result;

	    frame = (Frame)xv_get(folio_public, WIN_FRAME);
            text_notice = (Xv_Notice)xv_get(frame, 
                                XV_KEY_DATA, text_notice_key, 
				NULL);
            if (!text_notice)  {
                text_notice = xv_create(frame, NOTICE,
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_MESSAGE_STRINGS,
			XV_MSG("The text has been edited.\n\
\n\
You may discard edits now and quit, or cancel\n\
the request to Quit and go back and either save the\n\
contents or store the contents as a new file."),
                        0,
                        NOTICE_BUTTON_YES, 
			XV_MSG("Cancel, do NOT Quit"),
                        NOTICE_BUTTON, 
			XV_MSG("Discard edits, then Quit"), 123,
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
			XV_MSG("The text has been edited.\n\
\n\
You may discard edits now and quit, or cancel\n\
the request to Quit and go back and either save the\n\
contents or store the contents as a new file."),
                        0,
                        NOTICE_BUTTON_YES, 
			XV_MSG("Cancel, do NOT Quit"),
                        NOTICE_BUTTON, 
			XV_MSG("Discard edits, then Quit"), 123,
			NOTICE_STATUS, &result,
                        XV_SHOW, TRUE, 
                        NULL);
            }

	    if ((result == ACTION_STOP) || (result == NOTICE_YES)
		|| (result == NOTICE_FAILED)) {
		return (XV_ERROR);
	    } else {
		(void) textsw_reset(folio_public, 0, 0);
		(void) textsw_reset(folio_public, 0, 0);
	    }
	}
	break;
      case DESTROY_CLEANUP:
#ifdef OW_I18N
	/*
	 * When focus window set in ic is equal to the destroying view window,
	 * set XNFocusWindow with the other view window. And then unsetting of
	 * ic focus and after care of caret have to be done because KBD_DONE
	 * event will not be sent to textsw in this case.
	 */
	if (folio->ic) {
	    Xv_Drawable_info   *info;
	    XID			ic_xid;

	    XGetICValues(folio->ic, XNFocusWindow, &ic_xid, NULL);
	    DRAWABLE_INFO_MACRO(view_public, info);

	    if (xv_xid(info) == ic_xid) {
		Xv_Window	view_win;
		int		view_nbr;
		Xv_object	frame;

		for (view_nbr = 0 ; ; view_nbr++) {
		    view_win = xv_get(folio_public, OPENWIN_NTH_VIEW, view_nbr);
		    if (!view_win)
			break;
		    if (view_public != view_win) {
			DRAWABLE_INFO_MACRO(view_win, info);
			window_set_ic_focus_win(folio_public, folio->ic, xv_xid(info));
			XUnsetICFocus(folio->ic);

			/* after care of caret */
			textsw_hide_caret(folio);
			folio->state &= ~TXTSW_HAS_FOCUS;
			if (frame = xv_get(FOLIO_REP_TO_ABS(folio), WIN_FRAME))
				frame_kbd_done(frame, FOLIO_REP_TO_ABS(folio));
			textsw_stop_blinker(folio);

			folio->focus_view = view_win;
			break;
		    }
		}
	    }
	}
#endif /* OW_I18N */
	/* If menus access the view being destroy, then set
	   TEXTSW_MENU_DATA_KEY to another view. */
	if (view_public== (Textsw_view)xv_get(folio->menu,
					      XV_KEY_DATA,
					      TEXTSW_MENU_DATA_KEY)) {
	    int v;
	    Textsw_view another_view;
	    for (v = 0; ; v++) {
		another_view = xv_get(folio_public, OPENWIN_NTH_VIEW, v);
		if (!another_view)
		    break;
		if (view_public!=another_view) {
		    xv_set(folio->menu,XV_KEY_DATA,TEXTSW_MENU_DATA_KEY,
			   another_view,
			   NULL);
		    break;
		}
	    }
	}
	frame = xv_get(folio_public,WIN_FRAME);
	/* destroy any popups which are accessing this textsw.
	   Actually, I should destroy only those which access this view,
	   but, I don't have the view information, so just destroy it
	   if access this textsw. */

	textsw_destroy_popup(STORE_FILE_POPUP_KEY,folio_public,frame);
	textsw_destroy_popup(SAVE_FILE_POPUP_KEY,folio_public,frame);
	textsw_destroy_popup(LOAD_FILE_POPUP_KEY,folio_public,frame);
	textsw_destroy_popup(FILE_STUFF_POPUP_KEY,folio_public,frame);
	textsw_destroy_popup(SEARCH_POPUP_KEY,folio_public,frame);
	textsw_destroy_popup(MATCH_POPUP_KEY,folio_public,frame);
	textsw_destroy_popup(SEL_LINE_POPUP_KEY,folio_public,frame);
	    
	xv_destroy(view->drop_site);
	textsw_view_cleanup(view);
        VIEW_PRIVATE(view_public)/* = NULL */ ; /* WG Mar '95 */
	break;

      default:			/* Conservative in face of new cases. */
	break;
    }
    return (XV_OK);
}


Xv_private void
textsw_register_view(textsw, view_public)
    Textsw          textsw;
    Xv_Window       view_public;
{
    Textsw_folio    folio = TEXTSW_PRIVATE(textsw);
    Textsw_view_handle view = VIEW_PRIVATE(view_public);
    CHAR           *name;
    Textsw_view_handle temp_view = NULL;

    FORALL_TEXT_VIEWS(folio, temp_view) {
	if (temp_view == view)
	    return;		/* This view is already registered */
    }

    if (textsw_file_name(folio, &name))
	textsw_notify(view, TEXTSW_ACTION_USING_MEMORY, NULL);
#ifdef OW_I18N
    else {
	char            *name_mb = _xv_wcstombsdup(name);

	textsw_notify(view, TEXTSW_ACTION_LOADED_FILE, name_mb,
			    TEXTSW_ACTION_LOADED_FILE_WCS, name, NULL);
	if (name_mb)
	    free(name_mb);
    }
#else
    else
	textsw_notify(view, TEXTSW_ACTION_LOADED_FILE, name, NULL);
#endif /* OW_I18N */

    if (!folio->first_view) {
	folio->first_view = view;
	view->state |= TXTSW_IS_FIRST_VIEW;
    } else {

	(void) ev_set(view->e_view,
		      EV_SAME_AS, folio->first_view->e_view, NULL);

	/* set up the chain for the view */
	view->next = folio->first_view;
	folio->first_view = view;
    }

}

Xv_private void
textsw_unregister_view(textsw, view_public)
    Textsw          textsw;
    Xv_Window       view_public;
{
    Textsw_folio    folio = TEXTSW_PRIVATE(textsw);
    Textsw_view_handle view = VIEW_PRIVATE(view_public);

    if (view)
	textsw_unlink_view(folio, view);
}
