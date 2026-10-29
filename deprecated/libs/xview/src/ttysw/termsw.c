#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)termsw.c 1.59 93/06/28";
#endif
#endif

/*****************************************************************/
/* termsw.c                        */
/*	
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license. 
 */
/*****************************************************************/

#include <xview_private/i18n_impl.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <xview/sun.h>
#include <xview/frame.h>
#include <xview/tty.h>
#include <xview/ttysw.h>
#include <xview/textsw.h>
#include <xview/termsw.h>
#include <xview/font.h>
#include <xview/defaults.h>
#include <xview_private/term_impl.h>
#include <xview_private/portable.h>
#include <xview/scrollbar.h>
#include <xview_private/charscreen.h>
#include <X11/Xlib.h>
#ifdef OW_I18N
#include <xview_private/txt_impl.h>
#endif

#define	_NOTIFY_MIN_SYMBOLS
#include <xview/notify.h>
#undef	_NOTIFY_MIN_SYMBOLS

#define HELP_INFO(s) XV_HELP_DATA, s,

extern char    *getenv();
extern caddr_t  textsw_checkpoint_undo();
extern Xv_opaque xv_pf_open();
Bool            defaults_get_boolean();
extern Attr_avlist attr_find();

Pkg_private Xv_Window csr_pixwin;
Pkg_private Notify_value ttysw_event();

Pkg_private Menu ttysw_walkmenu();

/*
 * Key data for notice hung off frame
 */
int	tty_notice_key;

static void     termsw_unregister_view();
static void     termsw_register_view();


/*
 * Warning: a termsw is a specialization of window, not ttysw or textsw, so
 * that it has a chance to "fixup" the public object before it gets into
 * ttysw/textsw set/get routines.
 */

Pkg_private int termsw_destroy();


Pkg_private int termsw_view_init();
Pkg_private Xv_opaque termsw_view_set();
Pkg_private Xv_opaque termsw_view_get();
Pkg_private int termsw_view_destroy();
Xv_private  char *xv_font_monospace();


Pkg_private int tty_folio_init();
Pkg_private int tty_view_init();

typedef enum {
    IF_AUTO_SCROLL = 0,
    ALWAYS = 1,
    INSERT_SAME_AS_TEXT = 2
} insert_makes_visible_flags;

static Defaults_pairs insert_makes_visible_pairs[] = {
    "If_auto_scroll", (int) IF_AUTO_SCROLL,
    "Always", (int) ALWAYS,
    "Same_as_for_text", (int) INSERT_SAME_AS_TEXT,
    NULL, (int) INSERT_SAME_AS_TEXT
};

typedef enum {
    DO_NOT_USE_FONT = 0,
    DO_USE_FONT = 1,
    USE_FONT_SAME_AS_TEXT = 2
} control_chars_use_font_flags;

static Defaults_pairs control_chars_use_font_pairs[] = {
    "False", (int) DO_NOT_USE_FONT,
    "True", (int) DO_USE_FONT,
    "Same_as_for_text", (int) USE_FONT_SAME_AS_TEXT,
    NULL, (int) USE_FONT_SAME_AS_TEXT
};

typedef enum {
    DO_NOT_AUTO_INDENT = 0,
    DO_AUTO_INDENT = 1,
    AUTO_INDENT_SAME_AS_TEXT = 2

} auto_indent_flags;

static Defaults_pairs auto_indent_pairs[] = {
    "False", (int) DO_NOT_AUTO_INDENT,
    "True", (int) DO_AUTO_INDENT,
    "Same_as_for_text", (int) AUTO_INDENT_SAME_AS_TEXT,
    NULL, (int) AUTO_INDENT_SAME_AS_TEXT
};



static
termsw_layout(termsw_public, termsw_view_public, op, d1, d2, d3, d4, d5)
    Termsw          termsw_public;
    Xv_Window       termsw_view_public;
    Window_layout_op op;
/* Alpha compatibility, mbuck@debian.org */
#if defined(__alpha)
    unsigned long   d1, d2, d3, d4, d5;
#else
    int             d1, d2, d3, d4, d5;
#endif
{
    Termsw_folio    termsw_folio = TERMSW_PRIVATE(termsw_public);

    switch (op) {
      case WIN_DESTROY:
	termsw_unregister_view(termsw_public, termsw_view_public);
	break;
      case WIN_CREATE:
	if (xv_get(termsw_view_public, XV_IS_SUBTYPE_OF, TERMSW_VIEW)) {
	    /*
	     * termsw_folio->layout_proc(termsw_public, termsw_view_public,
	     * op, d1, d2, d3, d4, d5);
	     */
	    termsw_register_view(termsw_public, termsw_view_public);
	}
      default:
	break;
    }

    if (termsw_folio->layout_proc != NULL)
	return (termsw_folio->layout_proc(termsw_public, termsw_view_public, op,
					  d1, d2, d3, d4, d5));
    else
	return TRUE;

}

/*
 * Termsw: view related procedures
 */



static int
termsw_view_init_internal(parent, termsw_view_public, avlist)
    Xv_Window       parent;
    Termsw_view     termsw_view_public;
    Textsw_attribute avlist[];
{
    Xv_termsw_view *termsw_view_object =
    (Xv_termsw_view *) termsw_view_public;
    Termsw          termsw_public = TERMSW_FROM_TERMSW_VIEW(termsw_view_public);
    Xv_termsw      *termsw_folio_object = (Xv_termsw *) termsw_public;

    /* Make the folio of termsw into a ttysw folio */
    termsw_folio_object->parent_data.private_data = termsw_folio_object->private_tty;

    /* Initialized to ttysw */
    if (tty_view_init(parent, termsw_view_public, avlist) == XV_ERROR) {
	goto Error_Return;
    }
    /*
     * Turning this off to prevent the application from waking each time the
     * cursor passes over the window. Might have to be turned on when
     * follow_cursor is implemented.
     * (void)win_getinputmask(termsw_view_public, &im, 0);
     * win_setinputcodebit(&im, LOC_WINENTER); win_setinputcodebit(&im,
     * LOC_WINEXIT); (void)win_setinputmask(termsw_view_public, &im, 0, 0);
     */

    termsw_view_object->private_tty = (Xv_opaque) TTY_VIEW_PRIVATE(termsw_view_public);


    /* BUG:  This is a work around until WIN_REMOVE_EVENT_PROC is ready */
    notify_remove_event_func(termsw_view_public, (Notify_func) ttysw_event,
			     NOTIFY_SAFE);
    notify_remove_event_func(termsw_view_public, (Notify_func) ttysw_event,
			     NOTIFY_IMMEDIATE);
    /*
     * Restore the object as a textsw view
     */
    termsw_folio_object->parent_data.private_data =
	termsw_folio_object->private_text;
    termsw_view_object->parent_data.private_data =
	termsw_view_object->private_text;

    ttysw_interpose_on_textsw(termsw_view_public);

    return (XV_OK);

Error_Return:
    return (XV_ERROR);
}


static int
termsw_folio_init_internal(parent, termsw_folio, avlist)
    Xv_Window       parent;
    register        Termsw_folio
                    termsw_folio;
    Textsw_attribute avlist[];
{
    Termsw          termsw_public = TERMSW_PUBLIC(termsw_folio);
    Xv_termsw      *termsw_object = (Xv_termsw *) termsw_public;
    int             fd;
    char           *tmpfile_name = (char *) malloc(30);
    Textsw_status   status;
    int             is_client_pane;
#ifdef OW_I18N
    Xv_opaque       font = NULL;
#else
    Xv_opaque       font;
#endif
    char           *def_str;
    char           *termcap;
    static char    *cmd_termcap =
    "TERMCAP=sun-cmd:te=\\E[>4h:ti=\\E[>4l:tc=sun:";
    static char    *cmd_term = "TERM=sun-cmd";
    int             on = 1;
    Xv_opaque       defaults_array[10];
    register        Attr_avlist
                    defaults;
    register int    temp;
    Ttysw_folio     ttysw_folio;
    char            *font_name = NULL;

    Xv_opaque       parent_font;
    int             scale, size;
#ifdef OW_I18N
    Textsw_folio    txt_folio;
#endif

    /* Generate a new temporary file name and open the file up. */
    (void) strcpy(tmpfile_name, "/tmp/tty.txt.XXXXXX");
    (void) mkstemp(tmpfile_name);
    if ((fd = open(tmpfile_name, O_CREAT | O_RDWR | O_EXCL, 0600)) < 0) {
	return (XV_ERROR);
    }
    (void) close(fd);

    is_client_pane = (int) xv_get((Xv_object) termsw_public, WIN_IS_CLIENT_PANE);
#ifdef OW_I18N
    defaults_set_locale(NULL, XV_LC_BASIC_LOCALE);
    font_name = xv_font_monospace();
    defaults_set_locale(NULL, NULL);

    if (font_name)
        font = xv_pf_open(font_name);
    else
	font = (Xv_opaque) 0;

    /*
     * if name is present, it has already been handled during the
     * creation of the "Window" superclass in window_init.
     */
    if (font == NULL) {
        parent_font = (Xv_opaque) xv_get(termsw_public, WIN_FONT);
        scale = (int) xv_get(parent_font, FONT_SCALE);
        if (scale > 0)
            font = (Xv_opaque) xv_find(termsw_public, FONT,
                                FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
                                FONT_SCALE, scale,
                                NULL);
        else {
            size = (int) xv_get(parent_font, FONT_SIZE);
            font = (Xv_opaque) xv_find(termsw_public, FONT,
                                FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
                                FONT_SIZE, size,
                                NULL);
        }
    }
 
    if (font == NULL)
        font = (Xv_opaque) xv_get(termsw_public, WIN_FONT);
 
#else
    font_name = xv_font_monospace();

    if (font_name && (strlen(font_name) != 0)) {
	font = xv_pf_open(font_name);
    } else
	font = (Xv_opaque) 0;

    if (is_client_pane) {
	if (!font) {
	    parent_font = (Xv_opaque) xv_get(termsw_public, WIN_FONT);
	    scale = (int) xv_get(parent_font, FONT_SCALE);
	    if (scale > 0) {
		font = (Xv_opaque) xv_find(termsw_public, FONT,
				FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
		/* FONT_FAMILY,        FONT_FAMILY_SCREEN, */
		       FONT_SCALE, (scale > 0) ? scale : FONT_SCALE_DEFAULT,
					   NULL);
	    } else {
		size = (int) xv_get(parent_font, FONT_SIZE);
		font = (Xv_opaque) xv_find(termsw_public, FONT,
				FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
		/* FONT_FAMILY,        FONT_FAMILY_SCREEN,  */
			   FONT_SIZE, (size > 0) ? size : FONT_SIZE_DEFAULT,
					   NULL);
	    }
	}
    } else {
	if (!font) {
	    parent_font = (Xv_opaque) xv_get(termsw_public, WIN_FONT);
	    scale = (int) xv_get(parent_font, FONT_SCALE);

	    if (scale > 0) {
	        font = (Xv_opaque) xv_find(termsw_public, FONT,
				FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
	    			/* FONT_FAMILY,        FONT_FAMILY_SCREEN, */
		       		FONT_SCALE, (scale > 0) ? scale : FONT_SCALE_DEFAULT,
				NULL);
	    } else {
	        int             size = (int) xv_get(parent_font, FONT_SIZE);
	        font = (Xv_opaque) xv_find(termsw_public, FONT,
				FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
	    			/* FONT_FAMILY,        FONT_FAMILY_SCREEN, */
			   	FONT_SIZE, (size > 0) ? size : FONT_SIZE_DEFAULT,
				NULL);
	    }
	}
    }
    if (!font)
	font = (Xv_opaque) xv_get(termsw_public, WIN_FONT);
#endif

    (void) xv_set(termsw_public,
		  WIN_FONT, font,
		  TEXTSW_STATUS, &status,
		  TEXTSW_DISABLE_LOAD, TRUE,
		  TEXTSW_DISABLE_CD, TRUE,
		  TEXTSW_ES_CREATE_PROC, ts_create,
		  TEXTSW_NO_RESET_TO_SCRATCH, TRUE,
		  TEXTSW_IGNORE_LIMIT, TEXTSW_INFINITY,
		  TEXTSW_NOTIFY_LEVEL,
		  TEXTSW_NOTIFY_STANDARD | TEXTSW_NOTIFY_EDIT |
		  TEXTSW_NOTIFY_DESTROY_VIEW | TEXTSW_NOTIFY_SPLIT_VIEW,
		  HELP_INFO("ttysw:termsw")
		  NULL);
    if (status != TEXTSW_STATUS_OKAY) {
	goto Error_Return;
    }
    /* BUG ALERT textsw attr */
    termsw_folio->erase_line = (char) xv_get(termsw_public, TEXTSW_EDIT_BACK_LINE);
    termsw_folio->erase_word = (char) xv_get(termsw_public, TEXTSW_EDIT_BACK_WORD);
    termsw_folio->erase_char = (char) xv_get(termsw_public, TEXTSW_EDIT_BACK_CHAR);
    termsw_folio->pty_eot = -1;
    termsw_folio->ttysw_resized = FALSE;

    /* Initialized to ttysw */
    if (tty_folio_init(parent, termsw_public, avlist) == XV_ERROR) {
	goto Error_Return;
    }
    termsw_folio->tty_menu = (Menu) xv_get(termsw_public, WIN_MENU);


    ttysw_folio = TTY_PRIVATE(termsw_public);
    ttysw_folio->ttysw_opt |= 1 << TTYOPT_TEXT;
    ttysw_folio->ttysw_flags |= TTYSW_FL_IS_TERMSW;
    termsw_object->private_tty = (Xv_opaque) ttysw_folio;

    /*
     * Set TERM and TERMCAP environment variables.
     *
     * XXX: Use terminfo here?
     */
    putenv(cmd_term);
    termcap = getenv("TERMCAP");
    if (!termcap || *termcap != '/')
	putenv(cmd_termcap);

#   ifdef XV_USE_SVR4_PTYS
    /*
     * We'll discover the tty modes as soon as we get the first input through
     * the master side of the pty.  In the meantime, initialize cooked_echo to
     * an arbitrary value.
     */
    termsw_folio->cooked_echo = TRUE;
#   else /* XV_USE_SVR4_PTYS */
    /* Find out what the intra-line editing, etc. chars are. */
    fd = (int) xv_get(termsw_public, TTY_TTY_FD);
#   ifdef XV_USE_TERMIOS
    (void) tcgetattr(fd, &ttysw_folio->termios);
#   else /* XV_USE_TERMIOS */
    (void) ioctl(fd, TIOCGETP, &ttysw_folio->sgttyb);
    (void) ioctl(fd, TIOCGETC, &ttysw_folio->tchars);
    (void) ioctl(fd, TIOCGLTC, &ttysw_folio->ltchars);
#   endif /* XV_USE_TERMIOS */
    termsw_folio->cooked_echo =
	tty_iscanon(ttysw_folio) && tty_isecho(ttysw_folio);
#   endif /* XV_USE_SVR4_PTYS */

    /* Set the PTY to operate as a "remote terminal". */
    fd = (int) xv_get(termsw_public, TTY_PTY_FD);
#if !defined(__linux) || defined(TIOCREMOTE)
    (void) ioctl(fd, TIOCREMOTE, &on);
#endif
    ttysw_folio->remote = ttysw_folio->pending_remote = on;

    /*
     * Restore the object as a textsw
     */

    termsw_object->parent_data.private_data = termsw_object->private_text;

#ifdef OW_I18N
    /*
     * Restore the preedit callbacks of textsw
     */

    txt_folio= (Textsw_folio)TEXTSW_PRIVATE_FROM_TERMSW(termsw_public);

    xv_set(termsw_public,
		WIN_IC_PREEDIT_START,
		(XIMProc)txt_folio->start_pecb_struct.callback,
		(XPointer)txt_folio->start_pecb_struct.client_data,
		NULL);

    xv_set(termsw_public,
		WIN_IC_PREEDIT_DRAW,
		(XIMProc)txt_folio->draw_pecb_struct.callback,
		(XPointer)txt_folio->draw_pecb_struct.client_data,
		NULL);

    xv_set(termsw_public,
		WIN_IC_PREEDIT_DONE,
		(XIMProc)txt_folio->done_pecb_struct.callback,
		(XPointer)txt_folio->done_pecb_struct.client_data,
		NULL);
#endif

    /*
     * Build attribute list for textsw from /Tty defaults.
     */
    defaults = defaults_array;
    def_str = defaults_get_string("text.autoIndent", "Text.AutoIndent",
				  "False");
    switch (temp = defaults_lookup(def_str, auto_indent_pairs)) {
      case DO_AUTO_INDENT:
      case DO_NOT_AUTO_INDENT:
	*defaults++ = (Xv_opaque) TEXTSW_AUTO_INDENT;
	*defaults++ = (Xv_opaque) (temp == (int) DO_AUTO_INDENT);
	break;
	/* default: do nothing */
    }
    def_str = defaults_get_string("text.displayControlChars",
			    "Text.DisplayControlChars", "Same_as_for_text");
    switch (temp = defaults_lookup(def_str, control_chars_use_font_pairs)) {
      case DO_USE_FONT:
      case DO_NOT_USE_FONT:
	*defaults++ = (Xv_opaque) TEXTSW_CONTROL_CHARS_USE_FONT;
	*defaults++ = (Xv_opaque) (temp == (int) DO_USE_FONT);
	break;
	/* default: do nothing */
    }
    def_str = defaults_get_string("text.insertMakesCaretVisible",
			     "Text.InsertMakesCaretVisible", (char *) NULL);
    switch (temp = defaults_lookup(def_str, insert_makes_visible_pairs)) {
      case IF_AUTO_SCROLL:
      case ALWAYS:
	*defaults++ = (Xv_opaque) TEXTSW_INSERT_MAKES_VISIBLE;
	*defaults++ = (Xv_opaque) ((temp == (int) IF_AUTO_SCROLL)
				   ? TEXTSW_IF_AUTO_SCROLL : TEXTSW_ALWAYS);
	break;
	/* default: do nothing */
    }
    *defaults++ = 0;
    /*
     * Point the textsw at the temporary file.  The TEXTSW_CLIENT_DATA must
     * be the tty private data during the load so that the tty entity stream
     * will be provided the appropriate client data during the textsw's
     * outward call to ts_create. Note: reset TEXTSW_CLIENT_DATA in separate
     * call because the file load happens AFTER all the attributes are
     * processed.
     */
    termsw_folio->layout_proc = (int (*) ()) xv_get(termsw_public,
						    WIN_LAYOUT_PROC);
    (void) xv_set(termsw_public,
		  ATTR_LIST, defaults_array,
		  TEXTSW_CLIENT_DATA, termsw_object->private_tty,
		  TEXTSW_STATUS, &status,
		  OPENWIN_VIEW_ATTRS, TEXTSW_FILE, tmpfile_name, 0,
		  TEXTSW_TEMP_FILENAME, tmpfile_name,
		  TEXTSW_NOTIFY_PROC, ttysw_textsw_changed,
		  WIN_LAYOUT_PROC, termsw_layout,
		  NULL);
    /*
     * (void)xv_set(termsw_public, TEXTSW_CLIENT_DATA, 0, 0);
     * 
     */
     /* jcb */ (void) xv_set(termsw_public, OPENWIN_AUTO_CLEAR, FALSE,
			     WIN_BIT_GRAVITY, ForgetGravity,
			     NULL);

    if (status != TEXTSW_STATUS_OKAY) {
	goto Error_Return;
    }
    /*
     * Finish set up of fields that track state of textsw i/o for ttysw. Only
     * AFTER they are correct, interpose on textsw i/o.
     */
    termsw_folio->cmd_started = termsw_folio->pty_owes_newline = 0;
    termsw_folio->append_only_log =
	(int) defaults_get_boolean("term.enableEdit", "Term.EnableEdit",
				   (Bool) TRUE);

    /*
     * Must come *after* setting append_only_log to get string correct in the
     * append_only_log toggle item, and after textsw_menu has been restored.
     */
    ttysw_set_menu(termsw_public);
    xv_set(termsw_public, WIN_MENU, termsw_folio->text_menu, NULL);
    return (XV_OK);

Error_Return:
    return (XV_ERROR);
}

int termsw_creation_flag;

Pkg_private int
termsw_folio_init(parent, termsw_public, avlist)
    Xv_Window       parent;
    Termsw          termsw_public;
    Textsw_attribute avlist[];
{
    Xv_termsw      *termsw_object = (Xv_termsw *) termsw_public;
    Termsw_folio    termsw_folio;


    if (!tty_notice_key)  {
	tty_notice_key = xv_unique_key();
    }

    termsw_folio = xv_alloc(Termsw_folio_object);
    if (!termsw_folio)
	return (XV_ERROR);

    /* link to object; termsw is a textsw at this moment */
    termsw_object->private_data = (Xv_opaque) termsw_folio;
    termsw_folio->public_self = termsw_public;

    termsw_object->private_tty = (Xv_opaque) 0;

    /* Initialized to textsw */
    termsw_creation_flag = TRUE;
    if (xv_textsw_pkg.init(parent, termsw_public, avlist) == XV_ERROR) {
        termsw_creation_flag = FALSE;
	return (XV_ERROR);
    }
    termsw_creation_flag = FALSE;
    termsw_object->private_text = termsw_object->parent_data.private_data;

    if (termsw_folio_init_internal(parent, termsw_folio, avlist) != XV_OK) {
	free((char *) termsw_folio);
	return (XV_ERROR);
    }
    return (XV_OK);
}




/*
 * termsw_g/set_internal and termsw_destroy These routines must be careful to
 * guarantee that the value of termsw_object->parent_data.private_data is
 * preserved over a call to them. [In particular, violating this rule makes
 * the value of the field wrong after the initial call to tty_init.]  Since
 * they need to switch between the textsw's and ttysw's private data in this
 * field during the outward calls on the respective g/set procedures, this
 * requires saving and restoring the value.
 */
typedef         Xv_opaque(*opaque_fnp) ();






Pkg_private     Xv_opaque
termsw_folio_set(termsw_folio_public, avlist)
    Termsw_view     termsw_folio_public;
    Attr_avlist     avlist;
{
    register Xv_termsw *termsw_object = (Xv_termsw *) termsw_folio_public;
    register opaque_fnp fnp;
    Xv_opaque       error_code;
    register Xv_opaque save = termsw_object->parent_data.private_data;


    register Attr_avlist attrs;
    char           *buf;
    int            *buf_used;
    int             buf_len;

    /* First do termsw set */
    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch ((int) attrs[0]) {
	  case TTY_INPUT:{
		Ttysw_view_handle ttysw_view = TTY_VIEW_PRIVATE_FROM_TERMSW
		(termsw_folio_public);
		Termsw_folio    termsw_folio = TERMSW_PRIVATE(termsw_folio_public);
		Ttysw_folio     ttysw = TTY_FOLIO_FROM_TTY_VIEW_HANDLE(ttysw_view);

		if (ttysw_getopt((caddr_t) ttysw, TTYOPT_TEXT) &&
		    termsw_folio->cooked_echo) {
		    buf = (char *) attrs[1];
		    buf_len = (int) attrs[2];
		    buf_used = (int *) attrs[3];

		    *buf_used = ttysw_cooked_echo_cmd(ttysw_view, buf, buf_len);
		    ATTR_CONSUME(*attrs);
		}
	    }			/* else let tty's set do the work */
	    break;
	  case TERMSW_MODE:{
		Ttysw_view_handle ttysw_view = TTY_VIEW_PRIVATE_FROM_ANY_PUBLIC
		(termsw_folio_public);
		Ttysw_folio     ttysw = TTY_FOLIO_FROM_TTY_VIEW_HANDLE(ttysw_view);

		ttysw_setopt((caddr_t) ttysw, TTYOPT_TEXT, ((Termsw_mode) attrs[1] == TERMSW_MODE_TYPE));
		ATTR_CONSUME(*attrs);
	    }
	    break;

	  default:
	    (void) xv_check_bad_attr(TERMSW, attrs[0]);
	    break;
	}
    }

    /* Next do textsw set */
    fnp = (opaque_fnp) xv_textsw_pkg.set;
    if (termsw_object->private_text) {
	termsw_object->parent_data.private_data = termsw_object->private_text;
    }				/* else must be in text init and field is
				 * already correct */
    error_code = (fnp) (termsw_folio_public, avlist);
    if (error_code != (Xv_opaque) XV_OK)
	goto Return;

    /* Next do ttysw set */
    if (termsw_object->private_tty) {
	fnp = (opaque_fnp) xv_tty_pkg.set;
	termsw_object->parent_data.private_data = termsw_object->private_tty;
	error_code = (fnp) (termsw_folio_public, avlist);
	if (error_code != (Xv_opaque) XV_OK)
	    goto Return;
    }
Return:
    termsw_object->parent_data.private_data = save;
    return (error_code);
}



Pkg_private     Xv_opaque
termsw_folio_get(termsw_folio_public, status, attr, args)
    Termsw_view     termsw_folio_public;
    int            *status;
    Termsw_attribute attr;
    va_list         args;
{
    register Xv_termsw *termsw_object = (Xv_termsw *) termsw_folio_public;
    register opaque_fnp fnp;
    register Xv_opaque result;
    register Xv_opaque save = termsw_object->parent_data.private_data;


    switch (attr) {
      case OPENWIN_VIEW_CLASS:
	return ((Xv_opaque) TERMSW_VIEW);
      case XV_IS_SUBTYPE_OF:{
	    Xv_pkg         *pkg = va_arg(args, Xv_pkg *);

	    /* Termsw is a textsw or textsw view */
	    if (pkg == TEXTSW)
		return ((Xv_opaque) TRUE);
	}
	break;
      default:
	break;
    }

    /* First do textsw get */
    fnp = (opaque_fnp) xv_textsw_pkg.get;
    if (termsw_object->private_text) {
	termsw_object->parent_data.private_data = termsw_object->private_text;
    }				/* else must be in text init and field is
				 * already correct */
    result = (fnp) (termsw_folio_public, status, attr, args);
    if (*status == XV_OK)
	goto Return;

    /* Next do ttysw get */
    if (termsw_object->private_tty) {
	*status = XV_OK;
	fnp = (opaque_fnp) xv_tty_pkg.get;
	termsw_object->parent_data.private_data = termsw_object->private_tty;
	/* BUG ALERT: should do equivalent of va_start/end(args) around call. */
	result = (fnp) (termsw_folio_public, status, attr, args);
	if (*status == XV_OK)
	    goto Return;
    }
    /* Finally, do termsw get */
    *status = XV_ERROR;
    result = 0;

Return:
    termsw_object->parent_data.private_data = save;
    return (result);
}



Pkg_private int
termsw_folio_destroy(termsw_folio_public, status)
    Termsw_view     termsw_folio_public;
    Destroy_status  status;
{
    register        Xv_termsw
    *               termsw_object = (Xv_termsw *) termsw_folio_public;
    register        Xv_opaque
                    save = termsw_object->parent_data.private_data;

    int             result = XV_OK;

    switch (status) {
      case DESTROY_CHECKING:
	termsw_object->parent_data.private_data = termsw_object->private_tty;
	result = xv_tty_pkg.destroy(termsw_folio_public, status);
	if (result != XV_OK)
	    break;
	termsw_object->parent_data.private_data = termsw_object->private_text;
	result = xv_textsw_pkg.destroy(termsw_folio_public, status);
	break;
      case DESTROY_CLEANUP:{
	    Termsw_folio    folio = TERMSW_PRIVATE(termsw_folio_public);
	    /* Reset layout proc, and loop thru each view to destroy them */
	    Termsw_view_handle temp_view = folio->first_view, next;

	    (void) xv_set(termsw_folio_public,
			  WIN_LAYOUT_PROC, folio->layout_proc, NULL);

	    while (temp_view) {
		next = temp_view->next;
		(void)notify_post_destroy(TERMSW_VIEW_PUBLIC(temp_view),
					  status, NOTIFY_IMMEDIATE);
		temp_view = next;
	    }
	    termsw_object->parent_data.private_data = termsw_object->private_tty;
	    result = xv_tty_pkg.destroy(termsw_folio_public, status);
	    if (result != XV_OK)
		break;		/* BUG ALERT!  May have storage leak here. */
	    termsw_object->private_tty = XV_NULL;

	    termsw_object->parent_data.private_data = termsw_object->private_text;
	    result = xv_textsw_pkg.destroy(termsw_folio_public, status);
	    if (result != XV_OK)
		break;
	    /* BUG ALERT!  May have storage leak here. */
	    xv_free(folio);
	    termsw_object->private_text = XV_NULL;
	    break;
	}
      case DESTROY_PROCESS_DEATH:
      default:
	break;
    }
    termsw_object->parent_data.private_data = save;
    return (result);
}


/*
 * Termsw: view related procedures
 */
Pkg_private int
termsw_view_init(parent, termsw_view_public, avlist)
    Xv_Window       parent;
    Termsw_view     termsw_view_public;
    Termsw_attribute avlist[];
{
    Termsw_view_handle view;
    Xv_termsw_view *view_object = (Xv_termsw_view *)
    termsw_view_public;

    if (!tty_notice_key)  {
	tty_notice_key = xv_unique_key();
    }

    view = xv_alloc(Termsw_view_object);
    if (!view)
	return (XV_ERROR);

    /* link to object */
    view_object->private_data = (Xv_opaque) view;
    view->public_self = termsw_view_public;
    view->folio = TERMSW_PRIVATE(parent);


    /* Initialized to textsw_view */
    if (xv_textsw_view_pkg.init(parent, termsw_view_public, avlist) == XV_ERROR) {
	return (XV_ERROR);
    }
    view_object->private_text = view_object->parent_data.private_data;
    /* Might not want to call textsw_register_view() here */
    textsw_register_view(parent, termsw_view_public);

    if (termsw_view_init_internal(parent, termsw_view_public, avlist) !=
	XV_OK) {
	free((char *) view);
	return (XV_ERROR);
    }
    return (XV_OK);

}

Pkg_private     Xv_opaque
termsw_view_set(termsw_view_public, avlist)
    Termsw_view     termsw_view_public;
    Attr_avlist     avlist;
{
    register Xv_termsw_view *termsw_object = (Xv_termsw_view *) termsw_view_public;
    register opaque_fnp fnp;
    Xv_opaque       error_code;
    register Xv_opaque save = termsw_object->parent_data.private_data;
    register Attr_avlist attrs;
    char           *buf;
    int            *buf_used;
    int             buf_len;

    /* First do termsw set */
    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch ((int) attrs[0]) {
	  case TTY_INPUT:{
		Ttysw_view_handle ttysw_view =
			TTY_VIEW_PRIVATE_FROM_TERMSW_VIEW(termsw_view_public);
		Termsw_folio    termsw_folio =
			TERMSW_FOLIO_FROM_TERMSW_VIEW(termsw_view_public);
		Ttysw_folio     ttysw =
			TTY_FOLIO_FROM_TTY_VIEW_HANDLE(ttysw_view);

		if (ttysw_getopt((caddr_t) ttysw, TTYOPT_TEXT) &&
		    termsw_folio->cooked_echo) {
		    buf = (char *) attrs[1];
		    buf_len = (int) attrs[2];
		    buf_used = (int *) attrs[3];

		    *buf_used = ttysw_cooked_echo_cmd(ttysw_view, buf, buf_len);
		    ATTR_CONSUME(*attrs);
		}
	    }			/* else let tty's set do the work */
	    break;

	  default:
	    (void) xv_check_bad_attr(TERMSW_VIEW, attrs[0]);
	    break;
	}
    }

    /* Next do textsw set */
    fnp = (opaque_fnp) xv_textsw_view_pkg.set;

    if (termsw_object->private_text) {
	termsw_object->parent_data.private_data = termsw_object->private_text;
    }				/* else must be in text init and field is
				 * already correct */
    error_code = (fnp) (termsw_view_public, avlist);
    if (error_code != (Xv_opaque) XV_OK)
	goto Return;

    /* Next do ttysw set */
    if (termsw_object->private_tty) {
	fnp = (opaque_fnp) xv_tty_view_pkg.set;
	termsw_object->parent_data.private_data = termsw_object->private_tty;
	error_code = (fnp) (termsw_view_public, avlist);
	if (error_code != (Xv_opaque) XV_OK)
	    goto Return;
    }
Return:
    termsw_object->parent_data.private_data = save;
    return (error_code);
}



Pkg_private     Xv_opaque
termsw_view_get(termsw_view_public, status, attr, args)
    Termsw_view     termsw_view_public;
    int            *status;
    Attr_attribute  attr;
    va_list         args;
{
    register Xv_termsw_view *termsw_object = (Xv_termsw_view *) termsw_view_public;
    register opaque_fnp fnp;
    register Xv_opaque result;
    register Xv_opaque save = termsw_object->parent_data.private_data;


    if ((attr == XV_IS_SUBTYPE_OF) && (va_arg(args, Xv_pkg *) == TEXTSW_VIEW))
	return ((Xv_opaque) TRUE);

    switch (attr) {
      case OPENWIN_VIEW_CLASS:
	return ((Xv_opaque) TERMSW_VIEW);
      case XV_IS_SUBTYPE_OF:{
	    Xv_pkg         *pkg = va_arg(args, Xv_pkg *);

	    /* Termsw is a textsw or textsw view */
	    if (pkg == TEXTSW)
		return ((Xv_opaque) TRUE);
	}
	break;
      default:
	break;
    }

    /* First do textsw get */
    fnp = (opaque_fnp) xv_textsw_view_pkg.get;
    if (termsw_object->private_text) {
	termsw_object->parent_data.private_data = termsw_object->private_text;
    }				/* else must be in text init and field is
				 * already correct */
    result = (fnp) (termsw_view_public, status, attr, args);
    if (*status == XV_OK)
	goto Return;

    /* Next do ttysw get */
    if (termsw_object->private_tty) {
	*status = XV_OK;
	fnp = (opaque_fnp) xv_tty_view_pkg.get;
	termsw_object->parent_data.private_data = termsw_object->private_tty;
	/* BUG ALERT: should do equivalent of va_start/end(args) around call. */
	result = (fnp) (termsw_view_public, status, attr, args);
	if (*status == XV_OK)
	    goto Return;
    }
    /* Finally, do termsw get */
    *status = XV_ERROR;
    result = 0;

Return:
    termsw_object->parent_data.private_data = save;
    return (result);
}



Pkg_private int
termsw_view_destroy(termsw_view_public, status)
    Termsw_view     termsw_view_public;
    Destroy_status  status;
{
    register        Xv_termsw_view
    *               termsw_view_object = (Xv_termsw_view *) termsw_view_public;
    register        Xv_opaque
                    save = termsw_view_object->parent_data.private_data;
    Termsw_view_handle view = TERMSW_VIEW_PRIVATE(termsw_view_public);
    int             result = XV_OK;

    switch (status) {
      case DESTROY_SAVE_YOURSELF:
      case DESTROY_PROCESS_DEATH:
	break;
      case DESTROY_CHECKING:
	termsw_view_object->parent_data.private_data =
	    termsw_view_object->private_tty;
	result = xv_tty_view_pkg.destroy(termsw_view_public, status);
	if (result != XV_OK)
	    break;
	termsw_view_object->parent_data.private_data =
	    termsw_view_object->private_text;
	result = xv_textsw_view_pkg.destroy(termsw_view_public, status);
	break;
      case DESTROY_CLEANUP:
      default:
	termsw_view_object->parent_data.private_data =
	    termsw_view_object->private_tty;
	result = xv_tty_view_pkg.destroy(termsw_view_public, status);
	if (result != XV_OK)
	    break;		/* BUG ALERT!  May have storage leak here. */
	termsw_view_object->private_tty = XV_NULL;
	termsw_view_object->parent_data.private_data =
	    termsw_view_object->private_text;

	result = xv_textsw_view_pkg.destroy(termsw_view_public, status);
	if (result != XV_OK)
	    break;		/* BUG ALERT!  May have storage leak here. */
	termsw_view_object->private_text = XV_NULL;
	xv_free(view);
	break;
    }
    termsw_view_object->parent_data.private_data = save;
    return (result);
}

static void
termsw_register_view(termsw_public, termsw_view_public)
    Termsw          termsw_public;
    Xv_Window       termsw_view_public;
{
    Termsw_folio    termsw_folio = TERMSW_PRIVATE(termsw_public);
    Termsw_view_handle view = TERMSW_VIEW_PRIVATE(termsw_view_public);
    Termsw_view_handle temp_view = termsw_folio->first_view;

    while (temp_view) {
	if (temp_view == view)
	    return;		/* This view is already registered */
	else
	    temp_view = temp_view->next;
    }

    if (termsw_folio->first_view) {
	/* Chain up the views */
	view->next = termsw_folio->first_view;
	termsw_folio->first_view = view;
    } else {
	int             length;
	int             ttymargin = 0;

	termsw_folio->first_view = view;
	ttymargin += (int) xv_get(termsw_public, TEXTSW_LEFT_MARGIN);
	ttymargin += (int) xv_get(termsw_public, TEXTSW_RIGHT_MARGIN);

	/* Misc other setup */
	(void) ttysw_setleftmargin(ttymargin);	/* BUG need attr */

	termsw_folio->next_undo_point = textsw_checkpoint_undo(termsw_public,
						 (caddr_t) TEXTSW_INFINITY);



	/*
	 * Finish set up of fields that track state of textsw i/o for ttysw.
	 * Only AFTER they are correct, interpose on textsw i/o.
	 */
        length = (int) xv_get(termsw_view_public, TEXTSW_LENGTH_I18N);
	termsw_folio->user_mark =
	    textsw_add_mark_i18n(termsw_view_public,
				 length, TEXTSW_MARK_DEFAULTS);
	termsw_folio->pty_mark =
	    textsw_add_mark_i18n(termsw_view_public,
				 length, TEXTSW_MARK_DEFAULTS);
	if (termsw_folio->append_only_log) {
	    /*
	     * Note that read_only_mark is not TEXTSW_MOVE_AT_INSERT. Thus,
	     * as soon as it quits being moved by pty inserts, it will equal
	     * the user_mark.
	     */
	    termsw_folio->read_only_mark =
		textsw_add_mark_i18n(termsw_view_public,
		   termsw_folio->cooked_echo ? length : TEXTSW_INFINITY - 1,
				TEXTSW_MARK_READ_ONLY);
	}
    }

    termsw_folio->view_count++;
}


static void
termsw_unlink_view(folio, view)
    register Termsw_folio folio;
    register Termsw_view_handle view;
{
    Termsw_view_handle temp_view = folio->first_view;

    /*
     * view is freed at this point so don't reach into its data structure.
     */

    /* Unlink view from view chain */
    if (view == folio->first_view) {
	folio->first_view = folio->first_view->next;
	(folio->view_count)--;
    } else {
	while (temp_view) {
	    if (temp_view->next == view) {
		temp_view->next = view->next;
		(folio->view_count)--;
		break;
	    } else
		temp_view = temp_view->next;
	}
    }
}

static void
termsw_unregister_view(termsw_public, termsw_view_public)
    Termsw          termsw_public;
    Xv_Window       termsw_view_public;
{
    Termsw_folio    termsw_folio = TERMSW_PRIVATE(termsw_public);
    Termsw_view_handle view = TERMSW_VIEW_PRIVATE(termsw_view_public);


    /*
     * view is freed at this point so don't reach into its data structure.
     */
    if (view)
	termsw_unlink_view(termsw_folio, view);

    /*
     * This is to update the ttysw folio's view to the first view, so it will
     * not reference a destroyed view.
     * 
     */
    if (termsw_folio->first_view) {
	Ttysw_folio     ttysw_folio;

	termsw_view_public = TERMSW_VIEW_PUBLIC(termsw_folio->first_view);
	ttysw_folio = TTY_PRIVATE_FROM_TERMSW_VIEW(termsw_view_public);
	ttysw_folio->view = TTY_VIEW_PRIVATE_FROM_ANY_VIEW(termsw_view_public);
    }
}

/*
 * The following four routines have been created to insure that there is no
 * asynchronous update of the caret during periods of popup menu operation.
 * There is a global flag that is set/cleared during menu rendering and
 * handling logic. This flag is checked in the itimer interrupt routine
 * before the caret is to be rendered.
 * 
 * Note that this code finds it's way here because it is shared between ttysw
 * and txt logic. This method is used because it is simple enough to work.
 */

static short    menu_currently_active = FALSE;


Pkg_private void
termsw_menu_set()
{
    menu_currently_active = TRUE;

    /* printf("setting menu state active\n"); */
}

Pkg_private void
termsw_menu_clr()
{
    menu_currently_active = FALSE;

    /* printf("setting menu state INactive\n"); */
}

/*
 * more poor hacks to prevent the cursor from leaving turds
 */

static short    caret_cleared = FALSE;

Pkg_private void
termsw_caret_cleared()
{
    caret_cleared = TRUE;
}

/* NOT USED */
termsw_caret_rendered()
{
    caret_cleared = FALSE;
}

/*
 * this only returns correct value the next time
 */
/* NOT USED */
termsw_caret_invalid()
{
    short           ret = caret_cleared;

    caret_cleared = FALSE;

    return ret;
}
