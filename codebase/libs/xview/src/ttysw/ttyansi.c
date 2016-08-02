#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)ttyansi.c 20.43 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifdef OW_I18N
#include <xview/xv_i18n.h>
#endif
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <xview/win_struct.h>
#include <xview/ttysw.h>
#include <xview/termsw.h>
#include <xview/textsw.h>
#include <xview_private/tty_impl.h>
#include <xview_private/txt_impl.h>	/* BUG ALERT: Modularity violation */
#include <xview_private/term_impl.h>
#include <xview_private/ev.h>
#include <xview_private/ev_impl.h>
#undef CTRL
#include <xview_private/ttyansi.h>

#include <xview/sel_attrs.h>

/* char           *strncpy(); */
char           *textsw_checkpoint_undo();
Textsw_index    textsw_replace_i18n(), textsw_erase_i18n();

/*
 * jcb	-- remove continual cursor repaint in shelltool windows also known to
 * tty_main.c
 */
int             do_cursor_draw = TRUE;
int             tty_new_cursor_row, tty_new_cursor_col;

#ifdef DEBUG
#define ERROR_RETURN(val)	abort();	/* val */
#else
#define ERROR_RETURN(val)	return(val);
#endif				/* DEBUG */

#ifndef __linux
#define notcontrol(c)	(((c&0177) >= ' ') && (c != '\177'))
#else
#define notcontrol(c)	((c >= ' ') && (c != '\177'))
#endif

/* Logical state of window */
int             curscol;	/* cursor column */
int             cursrow;	/* cursor row */
extern int      cursor;
/* 0 -> NOCURSOR, 1 -> UNDERCURSOR, 2 -> BLOCKCURSOR */

/* extern  int scroll_disabled_from_menu; */
static int      state;		/* ALPHA, SKIPPING, etc, possibly w/ |ESC */
static int      saved_state;
static int      prefix;		/* prefix to arg */
static int      scrlins = 1;	/* How many lines to scroll when you have to */
static int      fillfunc;	/* 0 -> reverse video */
static CHAR     strtype;        /* type of ansi string sequence */

/* dimensions of window */
int             ttysw_top;	/* top row of window (0 for now) */
int             ttysw_bottom;	/* bottom row of window */
int             ttysw_left;	/* left column of window (0 for now) */
int             ttysw_right;	/* right column of window */

#ifdef OW_I18N
/* implement scroll region per Japanese users' requests */
#define SCROLL(scroll_bottom, bottom)  \
    ((scroll_bottom) ? scroll_bottom : bottom)
int scroll_bottom = 0; /* to implement scroll region change */
int pre_edit_rows_scrolled; /* updated in ansi_lf, used in ttysw callbacks */
#endif

static int
  send_input_to_textsw(Textsw textsw, register CHAR  *buf,
                       register long buf_len, Textsw_index end_transcript);
static int
  ansi_lf(Ttysw_view_handle ttysw_view, CHAR *addr, register int len);

static int
  ansi_char(Ttysw_view_handle ttysw_view, register CHAR  *addr, int olen);

/*
 * Interpret a string of characters of length <len>.  Stash and restore the
 * cursor indicator.
 * 
 * Note that characters with the high bit set will not be recognized. This is
 * good, for it reserves them for ASCII-8 X3.64 implementation. It just means
 * all sources of chars which might come here must mask parity if necessary.
 * 
 */

static CHAR    *
from_pty_to_textsw(textsw, cp, buf)
    register Textsw textsw;
    register CHAR  *buf;
    register CHAR  *cp;
{
    int             status = 0;
    register Textsw_index insert, cmd_start;
    register Termsw_folio termsw =
    TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));
    Ttysw_view_handle ttysw_view = TTY_VIEW_PRIVATE_FROM_ANY_PUBLIC(textsw);
    if (cp == buf) {
	return (buf);
    }
    *cp = (CHAR)'\0';
    /* Set up - remove marks, save positions, etc. */
    if (termsw->append_only_log) {
	/* Remove read_only_mark to allow insert */
	textsw_remove_mark(textsw, termsw->read_only_mark);
    }
    /* BUG ALERT Calling textsw routines directly */
    /* Save start of user command */
    if (termsw->cmd_started) {
	if ((cmd_start = textsw_find_mark_i18n(textsw, termsw->user_mark)) ==
	    TEXTSW_INFINITY)
	    ERROR_RETURN(0);
	textsw_remove_mark(textsw, termsw->user_mark);
	termsw->user_mark =
	    textsw_add_mark_i18n(textsw, cmd_start + 1,
				 TEXTSW_MARK_MOVE_AT_INSERT);
    } else
        cmd_start = (Textsw_index) xv_get(textsw, TEXTSW_LENGTH_I18N);

    /* Translate and edit in the pty input */
    ttysw_doing_pty_insert(textsw, termsw, TRUE);

    status =
	send_input_to_textsw(textsw, buf, (long) (cp - buf), cmd_start);

    ttysw_doing_pty_insert(textsw, termsw, FALSE);

    /* Restore user_mark, if cmd_started */
    if (termsw->cmd_started) {
	insert = textsw_find_mark_i18n(textsw, termsw->user_mark);
	textsw_remove_mark(textsw, termsw->user_mark);
	if (insert == TEXTSW_INFINITY)
	    insert = cmd_start;
	else
	    insert--;
	termsw->user_mark =
	    textsw_add_mark_i18n(textsw, insert, TEXTSW_MARK_DEFAULTS);
	if (termsw->append_only_log) {
	    termsw->read_only_mark =
		textsw_add_mark_i18n(textsw,
			 termsw->cooked_echo ? insert : TEXTSW_INFINITY - 1,
				TEXTSW_MARK_READ_ONLY);
	}
    } else {
	termsw->next_undo_point =
	    (caddr_t) textsw_checkpoint_undo(textsw,
					     (caddr_t) TEXTSW_INFINITY);
	if (termsw->append_only_log) {
            insert = (Textsw_index) xv_get(textsw, TEXTSW_LENGTH_I18N);
	    termsw->read_only_mark =
		textsw_add_mark_i18n(textsw,
			 termsw->cooked_echo ? insert : TEXTSW_INFINITY - 1,
				TEXTSW_MARK_READ_ONLY);
	}
    }

    if (status)
	return (NULL);
    else
	return (buf);
}

/*
 * A version of textsw_replace_bytes that allows you to trivially check the
 * error code.
 *
 * Returns 1 if the replacement leaves buffer size and insertion point
 * unchanged; 0 otherwise.
 */
static int
local_replace_bytes(textsw, pty_insert, last_plus_one, buf, buf_len)
    Textsw          textsw;
    Textsw_index    pty_insert;
    Textsw_index    last_plus_one;
    register CHAR  *buf;
    register long   buf_len;
{
    int             delta = 0;
    int             status = 0;
    Textsw_mark     tmp_mark;

    tmp_mark = textsw_add_mark_i18n(textsw, pty_insert,
				    TEXTSW_MARK_MOVE_AT_INSERT);
    delta = textsw_replace_i18n(textsw, pty_insert, last_plus_one,
				buf, buf_len);
    if (!delta && (textsw_find_mark_i18n(textsw, tmp_mark) == pty_insert)) {
	status = 1;
    }
    textsw_remove_mark(textsw, tmp_mark);
    return (status);
}

/*
 * Caller must be inserting text from pty and is responsible for unsetting
 * the user_mark and read_only_mark BEFORE calling, and AFTER call for
 * resetting them.
 */
static int
  send_input_to_textsw(Textsw textsw, register CHAR  *buf,
                       register long buf_len, Textsw_index end_transcript)
{
    Termsw_folio    termsw =
    TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));
    Textsw_index    pty_insert = textsw_find_mark_i18n(textsw,
						       termsw->pty_mark);
    Textsw_index    insert = (Textsw_index) xv_get(textsw,
						   TEXTSW_INSERTION_POINT_I18N);
    Textsw_index    last_plus_one;
    Textsw_index    add_newline = 0;
    Textsw_index    expanded_size;
#define BUFSIZE 200
    CHAR            expand_buf[BUFSIZE];
    Textsw_mark     owe_newline_mark;
    int             status = 0;
#ifdef  OW_I18N
    static wchar_t          wchar_newlin[2] = { (wchar_t)'\n', (wchar_t)'\0' };
#endif

    textsw_remove_mark(textsw, termsw->pty_mark);
    last_plus_one = end_transcript;
    if (termsw->pty_owes_newline)
	last_plus_one--;
    if (buf_len < (last_plus_one - pty_insert))
	last_plus_one = pty_insert + buf_len;
    /* replace from pty_insert to last_plus_one with buf */
    if (termsw->pty_owes_newline) {
	/* try to pay the newline back */
	if (buf[buf_len - 1] == '\n' && last_plus_one == end_transcript) {
	    termsw->pty_owes_newline = 0;
	    if (--buf_len == (long) 0) {
		return (status);
	    }
	}
    } else {
	if ((termsw->cmd_started != 0) && (buf[buf_len - 1] != '\n')) {
	    add_newline = 1;
	    owe_newline_mark = textsw_add_mark_i18n(textsw,
						    end_transcript,
						    TEXTSW_MARK_MOVE_AT_INSERT);
	}
    }
    /* in case of tabs or control chars, expand chars to be replaced */
    expanded_size = last_plus_one - pty_insert;
    switch (textsw_expand(
		     textsw, pty_insert, last_plus_one, expand_buf, BUFSIZE,
			  (int *) (&expanded_size))) {
      case TEXTSW_EXPAND_OK:
	break;
      case TEXTSW_EXPAND_FULL_BUF:
      case TEXTSW_EXPAND_OTHER_ERROR:
      default:
	expanded_size = last_plus_one - pty_insert;
	break;
    }
    if (expanded_size > buf_len) {
        (void) STRNCPY(buf + buf_len, expand_buf + buf_len,
		       (int) (expanded_size - buf_len));
	buf_len = expanded_size;
    }
    if ((status = local_replace_bytes(textsw, pty_insert, last_plus_one,
				      buf, buf_len))) {
	add_newline = 0;
	buf_len = 0;
    }
    termsw->pty_mark = textsw_add_mark_i18n(textsw,
					    pty_insert + buf_len,
					    TEXTSW_MARK_DEFAULTS);
    if (add_newline != 0) {
	add_newline = textsw_find_mark_i18n(textsw, owe_newline_mark);
	textsw_remove_mark(textsw, owe_newline_mark);
	termsw->pty_owes_newline =
            textsw_replace_i18n(textsw, add_newline, add_newline,
#ifdef OW_I18N
                                 wchar_newlin, (long int) 1);
#else
				 "\n", (long int) 1);
#endif
	if (!termsw->pty_owes_newline) {
	    status = 1;
	}
	add_newline = 1;
    }
    if (status)
	return (status);
    /*
     * BUG ALERT! If !append_only_log, and caret is in text that is being
     * replaced, you lose.
     */
    if (termsw->cooked_echo && insert >= end_transcript) {
	/* if text before insertion point grew, move insertion point */
	if (buf_len + add_newline > last_plus_one - pty_insert) {
	    insert += buf_len + add_newline -
		(int) (last_plus_one - pty_insert);
	    (void) xv_set(textsw, TEXTSW_INSERTION_POINT_I18N, insert, NULL);
	}
    } else if (!termsw->cooked_echo && insert == pty_insert) {
	insert += buf_len;
	(void) xv_set(textsw, TEXTSW_INSERTION_POINT_I18N, insert, NULL);
    }
    return (status);
#undef BUFSIZE
}

Pkg_private void
ttysw_ansiinit(ttysw)
    struct ttysubwindow *ttysw;
{
#ifdef SUNVIEW1
    char            windowname[WIN_NAMESIZE];
    /*
     * Need to we_setmywindow in case tty processes want to find out which
     * window running in.
     */
    (void) win_fdtoname(ttysw->ttysw_wfd, windowname);
    (void) we_setmywindow(windowname);
    /*
     * Setup gfx window environment value for gfx processes. Can be reset if
     * a more appropriate window is available.
     */
    (void) we_setgfxwindow(windowname);
#endif

    ttysw->ttysw_stringop = ttysw_ansi_string;
    ttysw->ttysw_escapeop = ttysw_ansi_escape;
}

/* ARGSUSED */
Pkg_private int
ttysw_ansi_string(data, type, c)
    caddr_t         data;
    char            type, c;
{
    return (TTY_OK);
}

/* NOT USED */
void
ttysw_save_state()
{
    saved_state = state;
    state = S_ALPHA;
}

/* NOT USED */
void
ttysw_restore_state()
{
    state = saved_state;
}


static int
erase_chars(textsw, pty_insert, end_span)
    Textsw          textsw;
    Textsw_index    pty_insert;
    Textsw_index    end_span;
{
    int             status = 0;
    register        Termsw_folio
                    termsw = TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));

    if (pty_insert < 0)
	pty_insert = 0;
    if (end_span <= pty_insert)
	return (status);
    if (termsw->append_only_log) {
	/* Remove read_only_mark to allow insert */
	textsw_remove_mark(textsw, termsw->read_only_mark);
    }
    ttysw_doing_pty_insert(textsw, termsw, TRUE);

    status = textsw_erase_i18n(textsw, pty_insert, end_span) ? 0 : 1;

    ttysw_doing_pty_insert(textsw, termsw, FALSE);
    if (termsw->append_only_log) {
	int             cmd_start;
	if (termsw->cmd_started)
	    cmd_start = textsw_find_mark_i18n(textsw, termsw->user_mark);
	else
            cmd_start = (int) xv_get(textsw, TEXTSW_LENGTH_I18N);
	termsw->read_only_mark =
	    textsw_add_mark_i18n(textsw,
			    (Textsw_index) (termsw->cooked_echo ?
					    cmd_start : TEXTSW_INFINITY - 1),
			    TEXTSW_MARK_READ_ONLY);
    }
    return (status);
}

static int
replace_chars(textsw, start_span, end_span, buf, buflen)
    Textsw          textsw;
    Textsw_index    start_span;
    Textsw_index    end_span;
    CHAR           *buf;
    long int        buflen;
{
    int             status = 0;
    register        Termsw_folio
                    termsw = TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));

    if (start_span < 0)
	start_span = 0;
    if (end_span < start_span)
	end_span = start_span;
    if (termsw->append_only_log) {
	/* Remove read_only_mark to allow insert */
	textsw_remove_mark(textsw, termsw->read_only_mark);
    }
    ttysw_doing_pty_insert(textsw, termsw, TRUE);

    status = local_replace_bytes(textsw, start_span, end_span, buf, buflen);

    ttysw_doing_pty_insert(textsw, termsw, FALSE);
    if (termsw->append_only_log) {
	int             cmd_start;
	if (termsw->cmd_started)
	    cmd_start = textsw_find_mark_i18n(textsw, termsw->user_mark);
	else
            cmd_start = (int) xv_get(textsw, TEXTSW_LENGTH_I18N);
	termsw->read_only_mark =
	    textsw_add_mark_i18n(textsw,
			    (Textsw_index) (termsw->cooked_echo ?
					    cmd_start : TEXTSW_INFINITY - 1),
			    TEXTSW_MARK_READ_ONLY);
    }
    return (status);
}

static void
adjust_insertion_point(textsw, pty_index, new_pty_index)
    Textsw          textsw;
    int             pty_index, new_pty_index;
{
    register        Termsw_folio
                    termsw = TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));

    /* in ![cooked, echo], pty_mark = insert */
    if (!termsw->cooked_echo &&
	(int) xv_get(textsw, TEXTSW_INSERTION_POINT_I18N) == pty_index) {
	if (termsw->append_only_log) {
	    /* Remove read_only_mark to allow insert */
	    textsw_remove_mark(textsw, termsw->read_only_mark);
	}
	(void) xv_set(textsw, TEXTSW_INSERTION_POINT_I18N, new_pty_index, NULL);
	if (termsw->append_only_log) {
	    termsw->read_only_mark =
		textsw_add_mark_i18n(textsw, TEXTSW_INFINITY - 1,
				     TEXTSW_MARK_READ_ONLY);
	}
    }
}

static int
do_backspace(textsw, addr)
    Textsw          textsw;
    CHAR            *addr;
{
    Textsw_index    pty_index;
    Textsw_index    pty_end;
    Textsw_index    textsw_start_of_display_line();
    int             increment = 0;
    Textsw_index    expanded_size = 1;
#define BUFSIZE 10
    CHAR            buf[BUFSIZE];
    register        Termsw_folio
                    termsw = TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));
#ifdef  OW_I18N
    CHAR            ctr_h[3];
    ctr_h[0]    = (CHAR)' ';
    ctr_h[1]    = (CHAR)'^H';
    ctr_h[2]    = (CHAR)0;
#endif

    pty_end = termsw->cmd_started ?
	textsw_find_mark_i18n(textsw, termsw->user_mark) :
        (int) xv_get(textsw, TEXTSW_LENGTH_I18N);
    pty_index = textsw_find_mark_i18n(textsw, termsw->pty_mark);
    if (pty_index > textsw_start_of_display_line(textsw, pty_index)) {
	switch (textsw_expand(
			      textsw, pty_index - 1, pty_index, buf, BUFSIZE,
			      (int *) (&expanded_size))) {
	  case TEXTSW_EXPAND_OK:
	    break;
	  case TEXTSW_EXPAND_FULL_BUF:
	  case TEXTSW_EXPAND_OTHER_ERROR:
	  default:
	    buf[0] = ' ';
	    expanded_size = 1;
	    break;
	}
	textsw_remove_mark(textsw, termsw->pty_mark);
	if (expanded_size != 1) {
	    if (replace_chars(textsw, pty_index - 1, pty_index,
			      buf, expanded_size)) {
		increment = -1;
	    }
	    pty_index += expanded_size - 1;
	    pty_end += expanded_size - 1;
	}
	termsw->pty_mark = textsw_add_mark_i18n(textsw, pty_index - 1,
						TEXTSW_MARK_DEFAULTS);
	if (increment < 0)
	    return (increment);
	adjust_insertion_point(textsw, (int) pty_index, (int) pty_index - 1);
	/*
	 * if at the end of transcript, interpret ' ' as delete a character.
	 */
#ifdef  OW_I18N
        if (pty_end == pty_index && STRNCMP(addr + 1, ctr_h , 2) == 0) {
#else
	if (pty_end == pty_index && strncmp(addr + 1, " ", 2) == 0) {
#endif
	    if (erase_chars(textsw, pty_index - 1, pty_index)) {
		increment = -1;
	    } else {
		increment = 2;
	    }
	}
    }
    return (increment);
#undef BUFSIZE
}

static
get_end_of_line(textsw)
    Textsw          textsw;
{
    int             pty_index;
    int             pty_end;
    int             pattern_start;
    int             pattern_end;
    CHAR            newline = (CHAR)'\n';
    Termsw_folio    termsw = TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));

    pty_end = termsw->cmd_started ?
	textsw_find_mark_i18n(textsw, termsw->user_mark) :
        (int) xv_get(textsw, TEXTSW_LENGTH_I18N);
    pty_index = textsw_find_mark_i18n(textsw, termsw->pty_mark);
    pattern_start = pty_index;
    if (pty_index == pty_end - termsw->pty_owes_newline
        || textsw_find_i18n(textsw, (long *) (&pattern_start),
			     (long *) (&pattern_end), &newline, 1, 0) == -1
	|| pattern_start >= pty_end - (int) termsw->pty_owes_newline
	|| pattern_start < pty_index) {
	pattern_start = pty_end - (int) termsw->pty_owes_newline;
    }
    return (pattern_start);
}

/*
 * By definition, the pty_mark is on the last line of the transcript.
 * Therefore, must insert a newline at pty_end, plus enough spaces to line up
 * with old column.
 */
static int
do_linefeed(textsw)
    Textsw          textsw;
{
    int             pty_index;
    int             pty_end;
    Textsw_index    line_start;
    Textsw_index    textsw_start_of_display_line();
    CHAR            newline = (CHAR)'\n';
#define BUFSIZE 2048
    CHAR            buf[2048];
    CHAR            *cp = buf;
    int             column;
    int             i;
    Termsw_folio    termsw = TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));

    pty_end = termsw->cmd_started ?
	textsw_find_mark_i18n(textsw, termsw->user_mark) :
        (int) xv_get(textsw, TEXTSW_LENGTH_I18N);
    pty_index = textsw_find_mark_i18n(textsw, termsw->pty_mark);
    line_start = textsw_start_of_display_line(textsw, pty_index);
    column = MIN(BUFSIZE - 3, (pty_index - line_start));

    textsw_remove_mark(textsw, termsw->pty_mark);
    termsw->pty_mark = textsw_add_mark_i18n(textsw,
			(Textsw_index) (pty_end - termsw->pty_owes_newline),
				       TEXTSW_MARK_DEFAULTS);
    adjust_insertion_point(textsw,
			   pty_index, pty_end - termsw->pty_owes_newline);

    *cp++ = newline;
    for (i = 0; i < column; i++) {
        *cp++ = (CHAR)' ';
    }
    return (from_pty_to_textsw(textsw, cp, buf) ? 0 : 1);
#undef BUFSIZE
}

/*
 * This is a static instead of a return code, for backward compatibility
 * reasons.
 */
static int      handle_escape_status = 0;

Xv_public int
ttysw_output(ttysw_public, addr, len0)
    Tty             ttysw_public;
    char           *addr;
    int             len0;
{
    Ttysw_folio     ttysw_folio = TTY_PRIVATE_FROM_ANY_PUBLIC(ttysw_public);
#ifdef OW_I18N
    char            *mbp;
    CHAR            *addr_wc, *wcp;
    int             char_count0 = 0;
    int             char_out;
    int		    i, j;

    addr_wc = (CHAR *)malloc((len0 + 1) * sizeof (CHAR));
    if ( !addr_wc ) {
	perror(XV_MSG("TTYSW:ttysw_output: out of memory"));
	return;
    }

    mbp = addr;
    wcp = addr_wc;

    for ( i = 0; i < len0; ) {
	if ( *mbp == '\0' ) {
	    *wcp = (CHAR)'\0';
	    j = 1;
	} else {
	    if ( ( j = mbtowc(wcp, mbp, MB_CUR_MAX) ) < 0 ) {
		mbp++;
		i++;
		continue;
	    }
	}
	mbp += j;
	i += j;
	wcp++;
	char_count0++;
    }

    char_out = ttysw_output_it(ttysw_folio->view, addr_wc, char_count0);
    if ( addr_wc )
	free(addr_wc);
    return (char_out);
#else
    return (ttysw_output_it(ttysw_folio->view, addr, len0));
#endif
}

#ifdef OW_I18N
Xv_public int
ttysw_output_wcs(ttysw_public, addr, len0)
    Tty                 ttysw_public;
    CHAR                *addr;
    int                 len0;
{
    Ttysw_folio     ttysw_folio = TTY_PRIVATE_FROM_ANY_PUBLIC(ttysw_public);
    return (ttysw_output_it(ttysw_folio->view, addr, len0));
}
#endif

Pkg_private int
ttysw_output_it(ttysw_view, addr, len0)
    Ttysw_view_handle ttysw_view;
    register CHAR  *addr;
    int             len0;
{
    Ttysw_folio     ttysw = TTY_FOLIO_FROM_TTY_VIEW_HANDLE(ttysw_view);
    static int      av[10];	/* args in ESCBRKT sequences.	 */
    /* -1 => defaulted		 */
    static int      ac;		/* number of args in av		 */
#define BUFSIZE 8192
    Textsw          textsw;
    Ev_chain	    views;
    Ev_handle	    e_view;
    Ev_pd_handle    private;
    Termsw_folio    termsw;
    CHAR            buf[BUFSIZE];
    CHAR            *cp = buf;
    register int    len = 0;
    int             upper_context;
#ifdef OW_I18N
    /* implement save and restore cursor per Japanese users' requests */
    static int      saved_row, saved_col; /* \E7 and \E8 */
#endif

    addr[len0] = '\0';
    if (TTY_IS_TERMSW(ttysw)) {
	textsw = (Textsw) TTY_PUBLIC(ttysw);
	views = (VIEW_ABS_TO_REP(textsw)->e_view)->view_chain;
	termsw = TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));
	if (!ttysw_getopt((caddr_t) ttysw, TTYOPT_TEXT) &&
	    do_cursor_draw /* jcb */ ) {
	    (void) ttysw_removeCursor();
	}
    } else if (do_cursor_draw) {/* jcb */
	(void) ttysw_removeCursor();
    }
    for (; len < len0 && !(ttysw->ttysw_flags & TTYSW_FL_FROZEN);
	 len++, addr++) {
	if (state & S_ESC) {
	    switch (*addr) {
	      case NUL:
	      case DEL:
		/* all ignored */
		continue;
	      case '[':	/* Begin X3.64 escape code sequence */
		ac = 0;
		prefix = 0;
		av[0] = -1;
		state = S_ESCBRKT;
		continue;

	      case 'P':	/* ANSI Device Control String */
	      case ']':	/* ANSI Operating System Command */
	      case '^':	/* ANSI Privacy Message */
	      case '_':	/* ANSI Application Program Command */
		state = S_STRING;
		strtype = *addr;
		continue;

	      case '?':
		/* simulate DEL char for systems that can't xmit it. */
		*addr = DEL;
		state &= ~S_ESC;
		break;

#ifdef OW_I18N
              case '7': /* \E7 is save cursor */
                        saved_row = cursrow;
                        saved_col = curscol;
                        state &= ~S_ESC;
                        continue;

              case '8': /* \E8 is restore cursor */
                        ttysw_pos(saved_col, saved_row);
                        state &= ~S_ESC;
                        continue;
#endif

	      case '\\':	/* ANSI string terminator */
		if (state == (S_STRING | S_ESC)) {
		    ttysw_handlestring(ttysw, strtype, 0);
		    state = S_ALPHA;
		    continue;
		}
		/* FALL THROUGH */

	      default:
		state &= ~S_ESC;
		continue;
	    }
	}
	switch (state) {
	  case S_ESCBRKT:
	    if (prefix == 0 && *addr >= '<' && *addr <= '?')
		prefix = *addr;
	    else if (*addr >= '0' && *addr <= '9') {
		if (av[ac] == -1)
		    av[ac] = 0;
		av[ac] = ((short) av[ac]) * 10 + *addr - '0';
		/* short for inline muls */
	    } else if (*addr == ';') {
		av[ac] |= prefix << 24;
		ac++;
		av[ac] = -1;
		prefix = 0;
	    } else {
		/* XXX - should only terminate on valid end char */
		av[ac] |= prefix << 24;
		ac++;
		switch (ttysw_handleescape(ttysw_view, *addr, ac, av)) {
		  case TTY_OK:
		    state = S_SKIPPING;
		  case TTY_DONE:
		    state = S_ALPHA;
		  default:{
		    }
		}
		if (handle_escape_status) {
		    handle_escape_status = 0;
		    (void) ttysw_setopt((Xv_opaque) ttysw_view, TTYOPT_TEXT, 0);
		    return (0);
		}
		ac = 0;
		prefix = 0;
	    }
	    break;

	  case S_SKIPPING:
	    /* Waiting for char from cols 4-7 to end esc string */
	    if (*addr < '@')
		break;
	    state = S_ALPHA;
	    break;

	  case S_STRING:
	    if (notcontrol(*addr))
		ttysw_handlestring(ttysw, strtype, *addr);
	    else if (*addr == CTRL('['))
		state |= S_ESC;
	    break;

	  case S_ALPHA:
	  default:
	    if (ttysw_getopt((caddr_t) ttysw, TTYOPT_TEXT)) {
		state = S_ALPHA;
		switch (*addr) {
		  case CTRL('['):	/* Escape */
		    state |= S_ESC;
		    /* spit out what we have so far */
		    cp = from_pty_to_textsw(textsw, cp, buf);
    		    if (TTY_IS_TERMSW(ttysw)) {
			FORALLVIEWS(views,e_view){
			    private = EV_PRIVATE(e_view);
			    private->state &= ~EV_VS_BUFFERED_OUTPUT;
			}
		    }
		    if (!cp) {
			return (0);
		    }
		    break;
		  case CTRL('G'):{
			extern Xv_Window csr_pixwin;
			Xv_Window       tmp_pixwin = csr_pixwin;
			csr_pixwin = textsw;
			ttysw_blinkscreen();
			csr_pixwin = tmp_pixwin;
			break;
		    }
		  case NUL:	/* ignored */
		  case DEL:	/* ignored */
		    break;
		  case '\f':{	/* formfeed */
			Textsw          view, textsw_first(), textsw_next();
			int             pty_mark_shows;
			int             pty_index =
			textsw_find_mark_i18n(textsw, termsw->pty_mark);
			*cp++ = '\n';
			for (view = textsw_first(textsw);
			     view;
			     view = textsw_next(view)) {
			    /*
			     * If pty_mark is showing, or if
			     * TEXTSW_INSERT_MAKES_VISIBLE == TEXTSW_ALWAYS
			     */
			    pty_mark_shows = !textsw_does_index_not_show(view,
					       (long) pty_index, (int *) 0);
			    if (pty_mark_shows
				|| (Textsw_enum) xv_get(view,
						TEXTSW_INSERT_MAKES_VISIBLE)
				== TEXTSW_ALWAYS /* != NEVER ??? */ ) {
				/* spit out what we have so far */
				cp = from_pty_to_textsw(view, cp, buf);
    				if (TTY_IS_TERMSW(ttysw)) {
				    FORALLVIEWS(views,e_view){
			    		private = EV_PRIVATE(e_view);
			    		private->state &= ~EV_VS_BUFFERED_OUTPUT;
				    }
				}
				if (!cp) {
				    return (0);
				}
				pty_index =
				    textsw_find_mark_i18n(textsw, termsw->pty_mark);
				/*
				 * we set the upper context to 0 for the
				 * clear
				 */
				/*
				 * command, then set it back to original
				 * value
				 */
				upper_context =
				    (int) xv_get(view, TEXTSW_UPPER_CONTEXT);
				xv_set(view, TEXTSW_UPPER_CONTEXT, 0, NULL);
				(void) xv_set(view,
					      TEXTSW_FIRST_I18N, pty_index, NULL);
				xv_set(view, TEXTSW_UPPER_CONTEXT,
				       upper_context, NULL);
			    }
			}
#ifdef OW_I18N
                        if (cp >= &buf[sizeof(buf) / sizeof(CHAR) - 1]) {
#else
			if (cp >= &buf[sizeof(buf) - 1]) {
#endif
			    /* spit out what we have so far and */
			    /* set buffered output flag         */
			    cp = from_pty_to_textsw(textsw, cp, buf);
    			    if (TTY_IS_TERMSW(ttysw)) {
				FORALLVIEWS(views,e_view){
			    	    private = EV_PRIVATE(e_view);
			    	    private->state |= EV_VS_BUFFERED_OUTPUT;
				}
			    }
			    if (!cp) {
				return (0);
			    }
			}
			break;
		    }
		  case '\b':{	/* backspace */
			register int    increment;

			/* preprocess buf */
			if (cp > buf && *(cp - 1) != '\t' && *(cp - 1) != '\n') {
			    while (*addr == '\b' && notcontrol(*(addr + 1))
				   && *(addr + 1) != ' ') {
				*(cp - 1) = *(++addr);
				addr++;
				len += 2;
			    }
			}
			if (*addr != '\b') {
			    addr--;
			    len--;
			    break;
			}
			/* back up pty mark */
			cp = from_pty_to_textsw(textsw, cp, buf);
    			if (TTY_IS_TERMSW(ttysw)) {
			    FORALLVIEWS(views,e_view){
		    		private = EV_PRIVATE(e_view);
		    		private->state &= ~EV_VS_BUFFERED_OUTPUT;
			    }
			}
			if (!cp) {
			    return (0);
			}
			if ((increment = do_backspace(textsw, addr)) > 0) {
			    addr += increment;
			    len += increment;
			} else if (increment < 0) {
			    (void) ttysw_setopt((Xv_opaque) ttysw_view, TTYOPT_TEXT, 0);
			    return (0);
			}
			break;
		    }
		  case '\r':{
			int             pty_index;
			Textsw_index    line_start;
			Textsw_index    textsw_start_of_display_line();

			switch (*(addr + 1)) {
			  case '\r':
			    /*
			     * compress multiple returns.
			     */
			    break;
			  case '\n':{
				/*
				 * if we're at the end, increment to the
				 * newline and goto print_char, else process
				 * return normally.
				 */
				pty_index =
				    textsw_find_mark_i18n(textsw, termsw->pty_mark);
				if ((cp - buf) >=
				    (get_end_of_line(textsw) - pty_index)) {
				    addr++;
				    len++;
				    goto print_char;
				}
			    }
			  default:{

				/* spit out what we have so far */
				cp = from_pty_to_textsw(textsw, cp, buf);
    				if (TTY_IS_TERMSW(ttysw)) {
				    FORALLVIEWS(views,e_view){
			    		private = EV_PRIVATE(e_view);
			    		private->state &= ~EV_VS_BUFFERED_OUTPUT;
				    }
				}
				if (!cp) {
				    return (0);
				}
				pty_index =
				    textsw_find_mark_i18n(textsw, termsw->pty_mark);
				line_start =
				    textsw_start_of_display_line(textsw, pty_index);
				textsw_remove_mark(textsw, termsw->pty_mark);
				termsw->pty_mark = textsw_add_mark_i18n(textsw,
								 line_start,
						      TEXTSW_MARK_DEFAULTS);
				adjust_insertion_point(
				       textsw, pty_index, (int) line_start);
			    }
			}	/* else textsw displays \n as \r\n */
			break;
		    }
		  case '\n':{	/* linefeed */
			cp = from_pty_to_textsw(textsw, cp, buf);
    			if (TTY_IS_TERMSW(ttysw)) {
			    FORALLVIEWS(views,e_view){
				private = EV_PRIVATE(e_view);
			    	private->state &= ~EV_VS_BUFFERED_OUTPUT;
			    }
			}
			if (!cp || do_linefeed(textsw)) {
			    return (0);
			}
			break;
		    }
		  case CTRL('K'):	/* explicitly NOT HANDLED       */
		  case '\t':	/* let textsw handle tab        */
	    print_char:
		  default:
		    if (!(notcontrol(*addr)) && *addr != '\t' && *addr != '\n')
			break;
		    if (ttysw->ttysw_flags & TTYSW_FL_FROZEN)
			break;
		    while ((notcontrol(*addr) || *addr == '\t' || *addr == '\n')
			   && len < len0) {
			*cp++ = *addr++;
			len++;
#ifdef OW_I18N
                        if (cp == &buf[sizeof(buf) / sizeof(CHAR) - 1]) {
#else
			if (cp == &buf[sizeof(buf) - 1]) {
#endif
			    /* spit out what we have so far and */
			    /* set insert point flag            */
			    cp = from_pty_to_textsw(textsw, cp, buf);
    			    if (TTY_IS_TERMSW(ttysw)) {
				FORALLVIEWS(views,e_view){
				    private = EV_PRIVATE(e_view);
				    private->state |= EV_VS_BUFFERED_OUTPUT;
				}
			    }
			    if (cp) {
				if (ttysw->ttysw_flags & TTYSW_FL_FROZEN) {
				    break;	/* out of while */
				}
			    } else {
				return (0);
			    }
			}
		    }
		    len--;
		    addr--;
		    break;
		}		/* switch (*addr) */
	    } else {		/* if (! TTYOPT_TEXT) */
		switch (*addr) {
		  case CTRL('G'):
		    (void) ttysw_blinkscreen();
		    break;
		  case '\b':
		    ttysw_pos(curscol - 1, cursrow);
		    break;
		  case '\t':
		    ttysw_pos((curscol & -8) + 8, cursrow);
		    break;
		  case '\n':	/* linefeed */
		    if (ansi_lf(ttysw_view, addr, (len0 - len) - 1) == 0)
			goto ret;
		    break;
		  case CTRL('K'):
		    ttysw_pos(curscol, cursrow - 1);	/* 4014 */
		    break;
		  case '\f':
		    if ((ttysw->ttysw_opt & (1 << TTYOPT_PAGEMODE)) &&
			ttysw->ttysw_lpp > 1) {
			if (ttysw_freeze(ttysw_view, 1))
			    goto ret;
		    }
		    ttysw_clear(ttysw);
		  case '\r':
		    /* ttysw_pos(0,cursrow); */
		    curscol = 0;
		    break;
		  case CTRL('['):
		    state |= S_ESC;
		    break;
		  case DEL:	/* ignored */
		    break;

		  default:
		    if (notcontrol(*addr)) {
			int             n;

			n = ansi_char(ttysw_view, addr, (len0 - len));
			addr += n;
			len += n;
		    }
		}
	    }			/* if (TTYOPT_TEXT) */
	}			/* switch (state) */
    }				/* for (; *addr; addr++) */
ret:
    if (ttysw_getopt((caddr_t) ttysw, TTYOPT_TEXT)) {
	cp = from_pty_to_textsw(textsw, cp, buf);
        if (TTY_IS_TERMSW(ttysw)) {
	    FORALLVIEWS(views,e_view){
		private = EV_PRIVATE(e_view);
		private->state &= ~EV_VS_BUFFERED_OUTPUT;
	    }
	}
	if (!cp) {
	    return (0);
	}
    } else {
	if (do_cursor_draw)	/* jcb */
	    (void) ttysw_drawCursor(cursrow, curscol);
	else {
	    tty_new_cursor_row = cursrow;
	    tty_new_cursor_col = curscol;
	}
    }
    return (len);
}


Pkg_private void
ttysw_lighten_cursor()
{
    (void) ttysw_removeCursor();
    cursor |= LIGHTCURSOR;
    (void) ttysw_restoreCursor();
}

Pkg_private void
ttysw_restore_cursor()
{
    (void) ttysw_removeCursor();
    cursor &= ~LIGHTCURSOR;
    (void) ttysw_restoreCursor();
}

static int
  ansi_lf(Ttysw_view_handle ttysw_view, CHAR *addr, register int len)
{
    register Ttysw_folio ttysw = TTY_FOLIO_FROM_TTY_VIEW_HANDLE(ttysw_view);
    register int    lfs = scrlins;
    extern int      delaypainting;

#ifdef OW_I18N
    if (ttysw->ttysw_lpp >= (SCROLL(scroll_bottom, ttysw_bottom))) {
#else
    if (ttysw->ttysw_lpp >= ttysw_bottom) {
#endif
	if (ttysw_freeze(ttysw_view, 1))
	    return (0);
    }
#ifdef OW_I18N
    if (cursrow < (SCROLL(scroll_bottom, ttysw_bottom) - 1)) {
#else
    if (cursrow < ttysw_bottom - 1) {
#endif
	/* ttysw_pos(curscol, cursrow+1); */
	cursrow++;
	if (ttysw->ttysw_opt & (1 << TTYOPT_PAGEMODE))
	    ttysw->ttysw_lpp++;
	if (!scrlins)		/* ...clear line */
	    (void) ttysw_deleteChar(ttysw_left, ttysw_right, cursrow);
    } else {
	if (delaypainting)
	    (void) ttysw_pdisplayscreen(1);
	if (!scrlins) {		/* Just wrap to top of screen and clr line */
	    ttysw_pos(curscol, 0);
	    (void) ttysw_deleteChar(ttysw_left, ttysw_right, cursrow);
	} else {
	    if (lfs == 1) {
		/* Find pending LF's and do them all now */
                register CHAR  *cp;
		register int    left_end;

		for (cp = addr + 1, left_end = len; left_end--; cp++) {
                    if (*cp == (CHAR)'\n')
			lfs++;
                    else if (*cp == (CHAR)'\r' || *cp >= (CHAR)' ')
			continue;
                    else if (*cp > (CHAR)'\n')
			break;
		}
	    }
#ifdef OW_I18N
            if (lfs + ttysw->ttysw_lpp >
                SCROLL(scroll_bottom, ttysw_bottom))
                    lfs = SCROLL(scroll_bottom, ttysw_bottom)
                        - ttysw->ttysw_lpp;
#else
	    if (lfs + ttysw->ttysw_lpp > ttysw_bottom)
		lfs = ttysw_bottom - ttysw->ttysw_lpp;
#endif
	    (void) ttysw_cim_scroll(lfs);
	    if (ttysw->ttysw_opt & (1 << TTYOPT_PAGEMODE))
		ttysw->ttysw_lpp++;
	    if (lfs != 1)	/* avoid upsetting <dcok> for nothing */
		ttysw_pos(curscol, cursrow + 1 - lfs);
	}
    }
    return (lfs);
}

static int
  ansi_char(Ttysw_view_handle ttysw_view, register CHAR  *addr, int olen)
{
    register int    len = olen;
    CHAR            buf[300];
    register CHAR   *cp = &buf[0];
    int             curscolstart = curscol;
#ifdef  OW_I18N
    int             colwidth; /* column width of a char */
#endif

    for (;;) {
	*cp++ = *addr;
#ifdef  OW_I18N
        colwidth = tty_character_size( *addr );
#endif
	/* Update cursor position.  Inline for speed. */
#ifdef  OW_I18N
        if (curscol < ttysw_right - colwidth)
            curscol += colwidth;
#else
	if (curscol < ttysw_right - 1)
	    curscol++;
#endif
	else {
	    /* Wrap to col 1 then pretend LF seen */
#ifdef OW_I18N
	    if ( curscol + colwidth > ttysw_right ) {
		*cp--;
		addr--;
		len++;
	    }
#endif
            *cp = (CHAR)'\0';
	    (void) ttysw_writePartialLine(buf, curscolstart);
	    curscol = 0;
	    (void) ansi_lf(ttysw_view, addr, len);
	    return (olen - len);
	}
	if (len > 0) {
	    if (notcontrol(*(addr + 1))
#ifdef OW_I18N
                && cp < &buf[sizeof(buf) / sizeof(CHAR) - 1]) {
#else
		&& cp < &buf[sizeof(buf) - 1]) {
#endif
		len--;
		addr++;
		continue;
	    } else
		break;		/* out of for loop */
	} else
	    break;		/* out of for loop */
    }
    *cp = '\0';
    (void) ttysw_writePartialLine(buf, curscolstart);
    return (olen - len);
}

Pkg_private int
ttysw_ansi_escape(ttysw_view_public, c, ac, av)
    Tty_view        ttysw_view_public;
    register CHAR   c;
    register int    ac, *av;
{

    Ttysw_folio     ttysw = TTY_PRIVATE_FROM_ANY_PUBLIC(ttysw_view_public);
    Ttysw_view_handle ttysw_view = TTY_VIEW_PRIVATE_FROM_ANY_PUBLIC(ttysw_view_public);
    register int    av0, i, found = TRUE;
    Textsw          textsw;
    register Termsw_folio termsw = NULL;
    static          reset_as_termsw;

    if (IS_TERMSW_VIEW(ttysw_view_public))
	termsw = TERMSW_FOLIO_FROM_TERMSW_VIEW(ttysw_view_public);
    else if (IS_TERMSW(ttysw_view_public))
	termsw = TERMSW_PRIVATE(ttysw_view_public);

    if ((av0 = av[0]) <= 0)
	av0 = 1;
    if (ttysw_getopt((caddr_t) ttysw, TTYOPT_TEXT)) {
	found = FALSE;
    } else
	switch (c) {
	  case '@':
	    (void) ttysw_insertChar(curscol, curscol + av0, cursrow);
	    break;
	  case 'A':
	    ttysw_pos(curscol, cursrow - av0);
	    break;
	  case 'B':
	    ttysw_pos(curscol, cursrow + av0);
	    break;
	  case 'C':
	    ttysw_pos(curscol + av0, cursrow);
	    break;
	  case 'D':
	    ttysw_pos(curscol - av0, cursrow);
	    break;
	  case 'E':
	    ttysw_pos(ttysw_left, cursrow + av0);
	    break;
	  case 'f':
	  case 'H':
	    if (av[1] <= 0)
		av[1] = 1;
	    ttysw_pos(av[1] - 1, av0 - 1);
	    av[1] = 1;
	    break;
	  case 'L':
	    (void) ttysw_insert_lines(cursrow, av0);
	    break;
	  case 'M':
	    (void) delete_lines(cursrow, av0);
	    break;
	  case 'P':
	    (void) ttysw_deleteChar(curscol, curscol + av0, cursrow);
	    break;
	  case 'm':
	    for (i = 0; i < ac; i++) {
		switch (av[i]) {
		  case 0:
		    ttysw_clear_mode();
		    break;
		  case 1:
		    ttysw_bold_mode();
		    break;
		  case 4:
		    ttysw_underscore_mode();
		    break;
		  case 7:
		    ttysw_inverse_mode();
		    break;
		  case 2:
		  case 3:
		  case 5:
		  case 6:
		  case 8:
		  case 9:{
			int             ttysw_getboldstyle();
			int             boldstyle = ttysw_getboldstyle();

			if (boldstyle & TTYSW_BOLD_NONE)
			    ttysw_inverse_mode();
			else
			    ttysw_bold_mode();
			break;
		    }
		  default:
		    ttysw_clear_mode();
		    break;
		}
	    }
	    break;
	  case 'p':
	    if (!fillfunc) {
		(void) ttysw_screencomp();
		fillfunc = 1 - fillfunc;
	    }
	    break;
	  case 'q':
	    if (fillfunc) {
		(void) ttysw_screencomp();
		fillfunc = 1 - fillfunc;
	    }
	    break;
	  case 'r':
#ifdef OW_I18N
            ttysw_cim_scroll(SCROLL(scroll_bottom, ttysw_bottom) -av[1]);
            scroll_bottom = av[1];
            ttysw_pos(curscol, scroll_bottom - 1);
#else
	    scrlins = av0;
#endif
	    break;
	  case 's':
	    scrlins = 1;
	    (void) ttysw_clear_mode();
	    break;
	  default:
	    found = FALSE;
	    break;
	}
    if (!found)
	switch (c) {
	  case 'J':
	    if (ttysw_getopt((caddr_t) ttysw, TTYOPT_TEXT)) {
		textsw = TTY_PUBLIC(ttysw);
		if (termsw) {
		    if (erase_chars(textsw,
				textsw_find_mark_i18n(textsw, termsw->pty_mark),
				termsw->cmd_started ?
				textsw_find_mark_i18n(textsw, termsw->user_mark)
				- (Textsw_index) termsw->pty_owes_newline :
                          (Textsw_index) xv_get(textsw, TEXTSW_LENGTH_I18N))) {
		        handle_escape_status = 1;
		    }
		}
	    } else {
		(void) delete_lines(cursrow + 1, ttysw_bottom - (cursrow + 1));
		(void) ttysw_deleteChar(curscol, ttysw_right, cursrow);
	    }
	    break;
	  case 'K':		/* clear to end of line */
	    if (ttysw_getopt((caddr_t) ttysw, TTYOPT_TEXT)) {
		textsw = TTY_PUBLIC(ttysw);
		if (termsw) {
		    if (erase_chars(textsw,
				textsw_find_mark_i18n(textsw, termsw->pty_mark),
				(Textsw_index) get_end_of_line(textsw))) {
		        handle_escape_status = 1;
		    }
		}
	    } else {
		(void) ttysw_deleteChar(curscol, ttysw_right, cursrow);
	    }
	    break;
	  case 'h':{		/* set mode */
		int             turn_on;

		for (i = 0; i < ac; i++) {
		    if (av[i] > 0 &&
			(av[i] & 0xff000000) == ('>' << 24)) {

			if (termsw && (av[i] & 0x00ffffff) == TTYOPT_TEXT) {
			    termsw->ok_to_enable_scroll = TRUE;
			    turn_on = reset_as_termsw;
			    reset_as_termsw = FALSE;
			} else
			    turn_on = TRUE;
			(void) ttysw_setopt(ttysw_view,
					    av[i] & 0x00ffffff, turn_on);
		    }
		}
		break;
	    }

	  case 'k':		/* report mode */
	    for (i = 0; i < ac; i++)
		if (av[i] > 0 &&
		    (av[i] & 0xff000000) == ('>' << 24)) {
		    char            buf[16];

		    (void) sprintf(buf, "\33[>%d%c",
				   av[i] & 0x00ffffff,
				   ttysw_getopt((caddr_t) ttysw,
					   av[i] & 0x00ffffff) ? 'h' : 'l');
		    (void) ttysw_input_it(ttysw,
					  buf, strlen(buf));
		}
	    break;

	  case 'l':		/* reset mode */
	    for (i = 0; i < ac; i++)
		if (av[i] > 0 &&
		    (av[i] & 0xff000000) == ('>' << 24)) {
		    /* This is invoked by vi, so don't disable scroll */
		    if (termsw && ((av[i] & 0x00ffffff) == TTYOPT_TEXT) &&
			(!ttysw_getopt(ttysw, TTYOPT_TEXT))) {
			/*
			 * Don't allow the menu to disable the scroll,
			 * because we are in vi
			 */
			termsw->ok_to_enable_scroll = FALSE;
			break;	/* It is already a ttysw */
		    }
		    (void) ttysw_setopt(ttysw_view,
					av[i] & 0x00ffffff, 0);

		    if (termsw && (av[i] & 0x00ffffff) == TTYOPT_TEXT) {
			/*
			 * If vi is invoke from a termsw, then return to
			 * termsw when exit
			 */
			reset_as_termsw = TRUE;
			/*
			 * Don't allow the menu to disable the scroll,
			 * because we are in vi
			 */
			if (termsw)
			    termsw->ok_to_enable_scroll = FALSE;
		    }
		}
	    break;

	  default:		/* X3.64 says ignore if we don't know */
	    return (TTY_OK);
	}
    return (TTY_DONE);
}

Pkg_private void
ttysw_pos(col, row)
    register int    col, row;
{

    if (col >= ttysw_right)
	col = ttysw_right - 1;
    if (col < ttysw_left)
	col = ttysw_left;
    if (row >= ttysw_bottom)
	row = ttysw_bottom - 1;
    if (row < ttysw_top)
	row = ttysw_top;
    cursrow = row;
    curscol = col;
    (void) ttysw_vpos(row, col);
}

/* ARGSUSED */
Pkg_private void
ttysw_clear(ttysw)
    Ttysw          *ttysw;
{
    /* jcb	-- insure that the caret is marked cleared if needed */
    if (TTY_IS_TERMSW(ttysw))
	termsw_caret_cleared();

    ttysw_pos(ttysw_left, ttysw_top);
    (void) ttysw_cim_clear(ttysw_top, ttysw_bottom);
}
