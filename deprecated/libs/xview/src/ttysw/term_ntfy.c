#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)term_ntfy.c 20.60 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Notifier related routines for the termsw.
 */
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#if defined(SVR4) || defined(__linux) || defined(__APPLE__)
#include <unistd.h>
#endif

#include <pixrect/pixrect.h>
#include <pixrect/pixfont.h>
#include <xview_private/i18n_impl.h>
#include <xview/frame.h>
#include <xview/notice.h>
#include <xview/notify.h>
#include <xview/rect.h>
#include <xview/rectlist.h>
#include <xview/win_input.h>
#include <xview/win_notify.h>
#include <xview/ttysw.h>
#include <xview/termsw.h>
#include <xview/window.h>
#include <xview_private/tty_impl.h>
#include <xview_private/term_impl.h>
#include <xview_private/ultrix_cpt.h>

#define PTY_OFFSET	(int) &(((Ttysw_folio)0)->ttysw_pty)

extern Textsw   textsw_first();
extern Textsw   textsw_next();
extern int      textsw_default_notify();
extern char    *textsw_checkpoint_undo();
extern Textsw_index textsw_insert();
extern Textsw_index textsw_erase_i18n();
extern void     textsw_display();

#ifdef DEBUG
#define ERROR_RETURN(val)	abort();	/* val */
Pkg_private void ttysw_print_debug_string();
#else
#define ERROR_RETURN(val)	return(val);
#endif				/* DEBUG */

/* performance: global cache of getdtablesize() */
extern int      dtablesize_cache;

#if defined(SVR4) || defined(__linux) || defined(__APPLE__)
#define GETDTABLESIZE() \
(dtablesize_cache?dtablesize_cache:(dtablesize_cache=(int)sysconf(_SC_OPEN_MAX)))
#else
#define GETDTABLESIZE() \
        (dtablesize_cache?dtablesize_cache:(dtablesize_cache=getdtablesize()))
#endif

Notify_value    ttysw_text_destroy();	/* Destroy func for termsw */
Notify_value    ttysw_text_event();	/* Event func for termsw */

/* shorthand - Duplicate of what's in ttysw_main.c */

#define	iwbp	ttysw->ttysw_ibuf.cb_wbp
#define	iebp	ttysw->ttysw_ibuf.cb_ebp
#define	ibuf	ttysw->ttysw_ibuf.cb_buf

#if defined(__APPLE__) && !defined(XTABS)
#define XTABS 0x00000c00
#endif

/* #ifdef TERMSW */
static          Textsw_index
find_and_remove_mark(textsw, mark)
    Textsw          textsw;
    Textsw_mark     mark;
{
    Textsw_index    result;

    result = textsw_find_mark_i18n(textsw, mark);
    if (result != TEXTSW_INFINITY)
	textsw_remove_mark(textsw, mark);
    return (result);
}

Pkg_private void
ttysw_move_mark(textsw, mark, to, flags)
    Textsw          textsw;
    Textsw_mark    *mark;
    Textsw_index    to;
    int             flags;
{
    textsw_remove_mark(textsw, *mark);
    *mark = textsw_add_mark_i18n(textsw, to, (unsigned) flags);
}

Pkg_private     Notify_value
ttysw_text_event(textsw, event, arg, type)
    register Textsw_view textsw;/* This is really a public termsw view */
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    int             insert = TEXTSW_INFINITY;
    int             length = TEXTSW_INFINITY;
    int             cmd_start;
    int             did_map = 0;
    Notify_value    nv = NOTIFY_IGNORED;
    register        Termsw_folio
                    termsw = TERMSW_FOLIO_FROM_TERMSW_VIEW(textsw);
    register        Ttysw_folio
                    ttysw = TTY_FROM_TERMSW(termsw->public_self);
    register        Ttysw_view_handle
                    ttysw_view = TTY_VIEW_PRIVATE_FROM_ANY_VIEW(textsw);
    /* XXX: Why should cooked_echo make a difference here? */
    register int    action = (termsw->cooked_echo)
			? event_action(event) : event_id(event);
    register int    down_event = event_is_down(event);

    if (!down_event && (action >= ASCII_FIRST) && (action <= ASCII_LAST))
	return (NOTIFY_DONE);

    if (ttysw->pass_thru_modifiers) {
	if (((ttysw->pass_thru_modifiers & 0x200) &&
	     (event_meta_is_down(event))) ||
	    ((ttysw->pass_thru_modifiers & 0x800) &&
	     (event_alt_is_down(event)))) {
		event_set_action(event, ACTION_NULL_EVENT);
		action = event_id(event);
	}
    }

    if (event_is_iso(event) && down_event && event_meta_is_down(event) &&
	event->action == ACTION_NULL_EVENT && ttysw->eight_bit_output) {
        event->ie_code |= 0x80;
        action |= 0x80;
    }



    if ((action == ACTION_MENU) && down_event) {
	/*
	 * The view that should be affected by the menu is the one that the
	 * menu appears over.  This must be reflected in the value of the
	 * MENU_CLIENT_DATA.  Relying on the view that the ttysw last
	 * remembered when it got a KBD_USE is wrong in click-to-type.
	 */
	ttysw_show_walkmenu(textsw, event);
	return (NOTIFY_DONE);
    }

    /*
     * Force the pty into or out of remote mode, as appropriate.
     *
     * XXX:  If the event routine is entered while in the middle of a
     * transition between acting as a ttysw and acting as a termsw, we could
     * potentially end up with the wrong remote mode setting.  If this occurs,
     * it can cause a ttysw to exit unexpectedly and a termsw to echo input
     * inappropriately.  I've seen such problems occur before, suspect they
     * still exist, but have been unable to reproduce them.  If they turn up
     * again, this is the place to start looking.
     */
    if (ttysw->pending_remote != ttysw->remote) {
#if (!defined(__linux) && !defined(__APPLE__)) || defined(TIOCREMOTE)
	if (ioctl(ttysw->ttysw_pty, TIOCREMOTE, &ttysw->pending_remote) < 0)
	    perror("ioctl: TIOCREMOTE");
	else
#endif
	    ttysw->remote = ttysw->pending_remote;
    }

    if (!ttysw_getopt((caddr_t) ttysw, TTYOPT_TEXT)) {
	/*
	 * Currently acting as a tty; hand the event off to its notification
	 * routine.
	 */
	if (textsw == TTY_VIEW_PUBLIC(ttysw_view)) {
	    extern Notify_value ttysw_event();
	    nv = ttysw_event(TTY_VIEW_PUBLIC(ttysw_view), event, arg, type);
	} else {
	    nv = notify_next_event_func((Notify_client) (textsw),
					(Notify_event) event, arg, type);
	}
	return (nv);
    }
    if (termsw->cooked_echo && down_event) {
	insert = (int) xv_get(textsw, TEXTSW_INSERTION_POINT_I18N);
	length = (int) xv_get(textsw, TEXTSW_LENGTH_I18N);
	if (termsw->cmd_started) {
	    /*
	     * Process pending literal next insertion at end of buffer.
	     */
#ifndef __linux
	    if (termsw->literal_next && action <= ASCII_LAST &&
#else
	    if (termsw->literal_next && action <= ISO_LAST &&
#endif
		    insert == length) {
		CHAR	input_char = (CHAR) action;

		/* Following call changes termsw state by side effect. */
		textsw_replace_i18n(textsw, length - 1, length,
		    &input_char, 1);
		termsw->literal_next = FALSE;
		return NOTIFY_DONE;
	    }
	} else if (insert == length)
	    (void) textsw_checkpoint_again(textsw);
    }
    /* ^U after prompt, before newline should only erase back to prompt. */
    if (termsw->cooked_echo
	&& event_id(event) == (short) termsw->erase_line
	&& down_event
	&& !event_shift_is_down(event)
	&& termsw->cmd_started != 0
	&& ((insert = (int) xv_get(textsw, TEXTSW_INSERTION_POINT_I18N))) >
	(cmd_start = (int) textsw_find_mark_i18n(textsw, termsw->user_mark))) {
	int             pattern_start = cmd_start;
	int             pattern_end = cmd_start;
	CHAR            newline = (CHAR)'\n';

	if (textsw_find_i18n(textsw, (long *) &pattern_start,
		(long *) &pattern_end, &newline, 1, 0) == -1
	 || (pattern_start <= cmd_start || pattern_start >= (insert - 1))) {
	    (void) textsw_erase_i18n(textsw,
			   (Textsw_index) cmd_start, (Textsw_index) insert);
	    return NOTIFY_DONE;
	}
    }
    if (!termsw->cooked_echo
	&& (action == '\r' || action == '\n')) {


        if (!tty_iscanon(ttysw)) /*1030878*/
            xv_set( textsw, TEXTSW_DIFFERENTIATE_CR_LF, 1, 0 );

	/* Implement "go to end of file" ourselves. */
	/* First let textsw do it to get at normalize_internal. */
	nv = notify_next_event_func((Notify_client) (textsw),
				    (Notify_client) (event), arg, type);

        if (!tty_iscanon(ttysw)) /*1030878*/
            xv_set( textsw, TEXTSW_DIFFERENTIATE_CR_LF, 0, 0 );

	/*
	 * Now fix it up. Only necessary when !append_only_log because
	 * otherwise the read-only mark at INFINITY-1 gets text to implement
	 * this function for us.
	 */
	if (!termsw->append_only_log)
	    (void) xv_set(textsw, TEXTSW_INSERTION_POINT_I18N,
			  textsw_find_mark_i18n(textsw, termsw->pty_mark), NULL);
    } else if (!termsw->cooked_echo
	       && action <= ASCII_LAST
	       && (iscntrl((char) action) || (char) action == '\177')
	       && (insert = (int) xv_get(textsw, TEXTSW_INSERTION_POINT_I18N))
	       == textsw_find_mark_i18n(textsw, termsw->pty_mark)) {
	/* In !cooked_echo, ensure textsw doesn't gobble up control chars */
	char	input_char = (char) action;

	/* 
	 * If interrupt character, force insertion point to
	 * end of file.
	 *
	 * XXX:	This seems wrong.  For one thing, if we're not in canonical
	 *	mode, the interrupt character has no special meaning.  For
	 *	another, the other characters that generate keyboard signals
	 *	should get the same treatment.  For a third, whatever tests we
	 *	make should also apply to the cooked_echo case.
	 */
#if !defined(__linux) && !defined(__APPLE__)
	if (action == tty_getintrc(ttysw)) {
	        (void) xv_set(textsw, TEXTSW_INSERTION_POINT_I18N,
			  TEXTSW_INFINITY, NULL);
	}
#endif
	(void) textsw_insert(textsw, &input_char, (long) 1);
	nv = NOTIFY_DONE;
    } else if (termsw->cooked_echo && down_event) {
	if (action == tty_getstopc(ttysw)) {
	    /* implement flow control characters as page mode */
	    (void) ttysw_freeze(ttysw_view, 1);
	} else if (action == tty_getstartc(ttysw)) {
	    (void) ttysw_freeze(ttysw_view, 0);
	    ttysw_reset_conditions(ttysw_view);
	} else if (action != tty_geteofc(ttysw)) {
	    /* Nice normal event */
	    nv = notify_next_event_func((Notify_client) (textsw),
					(Notify_event) (event), arg, type);
	}
    } else if (!termsw->cooked_echo && down_event &&
#if !defined(__linux) && !defined(__APPLE__)
		action >= ASCII_FIRST && action <= ASCII_LAST) {
#else
		action >= ASCII_FIRST && action <= ISO_LAST) {
#endif
	/*
	 * Pass the event through to the pty, storing it in the ttysw input
	 * buffer until the notifier triggers code to feed it into the pty
	 * proper.
	 */
	CHAR	input_char = (CHAR) action;

	(void) ttysw_copy_to_input_buffer(ttysw, &input_char, 1);
	ttysw_reset_conditions(ttysw_view);
	return NOTIFY_DONE;
    } else {
	/* Nice normal event */
	nv = notify_next_event_func((Notify_client) (textsw),
				    (Notify_event) (event), arg, type);
    }
    /* jcb 5/29/90 -- changed back to original state */
    if ( /* down_event ||*/ nv == NOTIFY_IGNORED) { /* jcb 5/8/90 */
	if ((action > ISO_LAST) &&
	    (action < LOC_MOVE || action > WIN_UNUSED_11)) {
	    did_map = (ttysw_domap(ttysw, event) == TTY_DONE);
	    nv = did_map ? NOTIFY_DONE : NOTIFY_IGNORED;
	}
    }
    /* the following switch probably belongs in a state transition table */
    switch (event_id(event)) {
      case WIN_REPAINT:
	ttysw_sigwinch(ttysw);
	nv = NOTIFY_DONE;
	break;
      case WIN_RESIZE:
	ttysw_resize(ttysw_view);
	nv = NOTIFY_DONE;
	break;
      case KBD_USE:
	if (TTY_IS_TERMSW(ttysw)) {
	    /*
	     * Update public_self as it tracks the text view that is
	     * transformed into a "pure" ttysw.
	     */
	    ttysw->current_view_public = (Tty_view) textsw;
	}
	/*
	 * Update pty's idea of tty size in case a descendant process looks
	 * at the TERMCAP.  (ttysw_be_ttysw() is too late, because vi, etc.
	 * will have already read the size of the terminal.)
	 */
	(void) xv_tty_new_size(ttysw, textsw_screen_column_count(TERMSW_PUBLIC(termsw)),
			   textsw_screen_line_count(TERMSW_PUBLIC(termsw)));
	break;
      case KBD_DONE:
      case LOC_MOVE:
      case LOC_WINENTER:
      case LOC_WINEXIT:
	break;
      default:
#ifdef DEBUG
	if (action <= ASCII_LAST) {
	    int             ctrl_state = event->ie_shiftmask & CTRLMASK;
	    int             shift_state = event->ie_shiftmask & SHIFTMASK;
	    char            ie_code = action;
	}
#endif	/* DEBUG */

       /*
	* Even if cooked echo is off, we still have to handle
	* keyboard signals.
	*/
#if !defined(__linux) && !defined(__APPLE__)
	if (!termsw->cooked_echo && !tty_issig(ttysw))
	    break;
#else
/* Linux: respect noncanonical mode, do not send interrupts */
        if (!tty_issig(ttysw) || !tty_iscanon(ttysw))
	    break;
#endif

	/* Only look at the down event for control key */
	if (event_is_up(event))
	    break;

	/* Only send interrupts when characters are actually typed. */
	if (action == tty_getintrc(ttysw)) {
	    ttysw_sendsig(ttysw, textsw, SIGINT);
	} else if (action == tty_getquitc(ttysw)) {
	    ttysw_sendsig(ttysw, textsw, SIGQUIT);
	} else if (action == tty_getsuspc(ttysw)
		   || action == tty_getdsuspc(ttysw)) {
	    ttysw_sendsig(ttysw, textsw, SIGTSTP);
	} 

	if (!termsw->cooked_echo)
		break;

	if (action == tty_geteofc(ttysw)) {
	    if (insert == TEXTSW_INFINITY)
		insert = (int) xv_get(textsw, TEXTSW_INSERTION_POINT_I18N);
	    if (length == TEXTSW_INFINITY)
                length = (int) xv_get(textsw, TEXTSW_LENGTH_I18N);
	    if (length == insert) {
		/*
		 * The EOF was entered at the end of the buffer.  Handle it
		 * like a newline or carriage return, transferring the
		 * accumulated command into the pty buffer for transmission
		 * through the slave to the client.
		 */
		if (termsw->cmd_started
		 && length > textsw_find_mark_i18n(textsw, termsw->user_mark)) {
		    if (ttysw_scan_for_completed_commands(ttysw_view, -1, 0))
			nv = NOTIFY_IGNORED;
		} else {
		    termsw->cmd_started = 0;
		}
		/* Remember to send eot. */
		termsw->pty_eot = iwbp - ibuf;
		ttysw_reset_conditions(ttysw_view);
	    } else {		/* length != insert */
		nv = notify_next_event_func((Notify_client) (textsw),
					(Notify_client) (event), arg, type);
	    }
	}
    }				/* switch */
    return (nv);
}

static          Notify_value
ttysw_cr(tty_public, tty)
    Tty             tty_public;
    int             tty;
{
    int             nfds = 0;
    fd_set          wfds;
    static struct timeval timeout = {0, 0};
    int             maxfds = GETDTABLESIZE();
    Ttysw_folio     ttysw_folio = TTY_PRIVATE_FROM_ANY_PUBLIC(tty_public);

    /*
     * GROSS HACK:
     * 
     * There is a race condition such that between the notifier's select()
     * call and our write, the app may write to the tty, causing our write to
     * block.  The tty cannot be flushed because we don't get to read the pty
     * because our write is blocked.  This GROSS HACK doesn't eliminate the
     * race condition; it merely narrows the window, making it less likely to
     * occur.  We don't do an fcntl(tty, FN_NODELAY) because that affects the
     * file, not merely the file descriptor, and we don't want to change what
     * the application thinks it sees.
     * 
     * The right solution is either to invent an ioctl that will allow us to
     * set the tty driver's notion of the cursor position, or to avoid using
     * the tty driver altogether.
     */
    FD_ZERO(&wfds);
    FD_SET(tty, &wfds);
    if ((nfds = select(maxfds, NULL, &wfds, NULL, &timeout)) < 0) {
	perror(XV_MSG("ttysw_cr: select"));
	return (NOTIFY_IGNORED);
    }
    if (nfds == 0 || !FD_ISSET(tty, &wfds)) {
	return (NOTIFY_IGNORED);
    }
    if (write(tty, "\r", 1) < 0) {
	fprintf(stderr, "for tty%x, tty fd %d, ",
		tty_public, ttysw_folio->ttysw_tty);
	perror(XV_MSG("TTYSW tty write failure"));
    }
    (void) notify_set_output_func((Notify_client) (tty_public),
				  NOTIFY_FUNC_NULL, tty);
    return (NOTIFY_DONE);
}

static void
ttysw_reset_column(ttysw)
    Ttysw_folio     ttysw;
{
    Tty             tty_public = TTY_PUBLIC(ttysw);

    /* BUG ALERT accessing field of termsw */
    if ((tty_gettabs(ttysw))
	&& notify_get_output_func((Notify_client) (tty_public),
				  ttysw->ttysw_tty) != ttysw_cr) {
	if (notify_set_output_func((Notify_client) (tty_public),
			  ttysw_cr, ttysw->ttysw_tty) == NOTIFY_FUNC_NULL) {
	    fprintf(stderr,
		    XV_MSG("cannot set output func on ttysw %x, tty fd %d\n"),
		    ttysw, ttysw->ttysw_tty);
	}
    }
}

static void
ttysw_post_error(public_folio_or_view, msg1, msg2)
    Xv_opaque       public_folio_or_view;	/* Public handle */
    char           *msg1, *msg2;
{
    Frame	    frame;
    Xv_Notice	tty_notice;
    char            buf[1000];
    int             size_to_use = sizeof(buf);

    buf[0] = '\0';
    (void) strncat(buf, msg1, size_to_use);
    if (msg2) {
	int             len = strlen(buf);
	if (len < size_to_use) {
	    (void) strncat(buf, msg2, size_to_use - len);
	}
    }
    frame = (Frame)xv_get(public_folio_or_view, WIN_FRAME),
    tty_notice = xv_get(frame, XV_KEY_DATA, tty_notice_key, NULL);
    if (!tty_notice)  {
        tty_notice = xv_create(frame, NOTICE,
                        NOTICE_LOCK_SCREEN, FALSE,
			NOTICE_BLOCK_THREAD, TRUE,
                        NOTICE_BUTTON_YES, 
				XV_MSG("Continue"),
                        NOTICE_MESSAGE_STRINGS,
			buf,
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
                NOTICE_BUTTON_YES, XV_MSG("Continue"),
                NOTICE_MESSAGE_STRINGS,
                buf,
                0,
                XV_SHOW, TRUE, 
                NULL);
    }
}


Pkg_private int
ttysw_scan_for_completed_commands(ttysw_view, start_from, maybe_partial)
    Ttysw_view_handle ttysw_view;
    int             start_from;
    int             maybe_partial;
{
    Ttysw_folio     ttysw = TTY_FOLIO_FROM_TTY_VIEW_HANDLE(ttysw_view);
    register Textsw textsw = TEXTSW_FROM_TTY(ttysw);
    register Termsw_folio termsw =
        TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));
    register CHAR  *cp;
    int             length = (int) xv_get(textsw, TEXTSW_LENGTH_I18N);
    int             cmd_length;

    /*
     * Determine starting position for the scan.
     */
    if (start_from == -1) {
	/*
	 * Use the mark as the starting position.
	 */
	start_from = textsw_find_mark_i18n(textsw, termsw->user_mark);
	if (start_from == TEXTSW_INFINITY)
	    ERROR_RETURN(1);
	if (start_from == length) {
	    /* Nothing to do -- nothing new in the buffer. */
	    return (0);
	}
    }

    cmd_length = length - start_from;

    /*
     * Check for buffer overflow.
     */
    if ((iwbp + cmd_length) >= iebp) {
	ttysw_post_error(textsw,
		  XV_MSG("Pty cmd buffer overflow: last cmd ignored."), 
		  (char *) 0);
	return (0);
    }

    /* Copy these commands into the buffer for pty */
    (void) xv_get(textsw, TEXTSW_CONTENTS_I18N, start_from, iwbp, cmd_length);
    if (maybe_partial) {
	/*
	 * Discard partial commands.
	 *
	 * XXX: Should we worry about EOFs here?
	 */
	for (cp = iwbp + cmd_length - 1; cp >= iwbp; --cp) {
	    switch (*cp) {
	      case '\n':
	      case '\r':
		goto Done;
	      default:
		if (*cp == tty_geteolc(ttysw) || *cp == tty_geteol2c(ttysw))
		    goto Done;
		cmd_length--;
		break;
	    }
	}
    }

Done:
    if (cmd_length <= 0)
	return (0);

    iwbp += cmd_length;
    cp = iwbp - 1;
    ttysw_reset_conditions(ttysw_view);
    if (*cp == (CHAR)'\n' || *cp == (CHAR)'\r')
	ttysw_reset_column(ttysw);
    ttysw_move_mark(textsw, &termsw->pty_mark,
		    (Textsw_index) (start_from + cmd_length),
		    TEXTSW_MARK_DEFAULTS);
    if (termsw->cmd_started) {
	if (start_from + cmd_length < length) {
	    ttysw_move_mark(textsw, &termsw->user_mark,
			    (Textsw_index) (start_from + cmd_length),
			    TEXTSW_MARK_DEFAULTS);
	} else
	    termsw->cmd_started = 0;
	if (termsw->append_only_log) {
	    ttysw_move_mark(textsw, &termsw->read_only_mark,
			    (Textsw_index) (start_from + cmd_length),
			    TEXTSW_MARK_READ_ONLY);
	}
    }
    termsw->pty_owes_newline = 0;

    return (0);
}

Pkg_private void
ttysw_doing_pty_insert(textsw, commandsw, toggle)
    register Textsw textsw;
    Termsw_folio    commandsw;
    int        	    toggle;
{
    unsigned        notify_level = (unsigned) window_get(textsw,
						       TEXTSW_NOTIFY_LEVEL);
    commandsw->doing_pty_insert = toggle;
    if (toggle) {
	window_set(textsw,
		   TEXTSW_NOTIFY_LEVEL, notify_level & (~TEXTSW_NOTIFY_EDIT),
		   NULL);
    } else {
	window_set(textsw,
		   TEXTSW_NOTIFY_LEVEL, notify_level | TEXTSW_NOTIFY_EDIT,
		   NULL);
    }
}

Pkg_private int
ttysw_cooked_echo_cmd(ttysw_view, buf, buflen)
    Ttysw_view_handle ttysw_view;
    char           *buf;
    int             buflen;
{
    Ttysw_folio     ttysw = TTY_FOLIO_FROM_TTY_VIEW_HANDLE(ttysw_view);
    register Textsw textsw = TEXTSW_FROM_TTY(ttysw);
    register Termsw_folio termsw =
	TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));
    Textsw_index    insert = (Textsw_index) xv_get(textsw,
						   TEXTSW_INSERTION_POINT_I18N);
    int             length = (Textsw_index) xv_get(textsw, TEXTSW_LENGTH_I18N);
    Textsw_index    insert_at;
    Textsw_mark     insert_mark;

    if (termsw->append_only_log) {
	textsw_remove_mark(textsw, termsw->read_only_mark);
    }
    if (termsw->cmd_started) {
	insert_at = find_and_remove_mark(textsw, termsw->user_mark);
	if (insert_at == TEXTSW_INFINITY)
	    ERROR_RETURN(-1);
	if (insert == insert_at) {
	    insert_mark = TEXTSW_NULL_MARK;
	} else {
	    insert_mark =
		textsw_add_mark_i18n(textsw, insert, TEXTSW_MARK_DEFAULTS);
	}
    } else {
	if (insert == length)
	    (void) textsw_checkpoint_again(textsw);
	termsw->next_undo_point = textsw_checkpoint_undo(textsw,
						 (caddr_t) TEXTSW_INFINITY);
	insert_at = length;
    }
    if (insert != insert_at) {
	(void) xv_set(textsw, TEXTSW_INSERTION_POINT_I18N, insert_at, NULL);
    }
    (void) textsw_checkpoint_undo(textsw, termsw->next_undo_point);
    /* Stop this insertion from triggering the cmd scanner! */
    ttysw_doing_pty_insert(textsw, termsw, TRUE);
    (void) textsw_insert(textsw, buf, (long) buflen);
    ttysw_doing_pty_insert(textsw, termsw, FALSE);
    (void) ttysw_scan_for_completed_commands(ttysw_view, (int) insert_at, TRUE);
    if (termsw->cmd_started) {
	insert_at = (Textsw_index) xv_get(textsw, TEXTSW_INSERTION_POINT_I18N);
	if (insert_at == TEXTSW_INFINITY)
	    ERROR_RETURN(-1);
	termsw->user_mark =
	    textsw_add_mark_i18n(textsw, (Textsw_index) insert_at, TEXTSW_MARK_DEFAULTS);
	if (termsw->append_only_log) {
	    termsw->read_only_mark =
		textsw_add_mark_i18n(textsw,
		      termsw->cooked_echo ? insert_at : TEXTSW_INFINITY - 1,
				TEXTSW_MARK_READ_ONLY);
	}
	if (insert_mark != TEXTSW_NULL_MARK) {
	    insert = find_and_remove_mark(textsw, insert_mark);
	    if (insert == TEXTSW_INFINITY)
		ERROR_RETURN(-1);
	    (void) xv_set(textsw, TEXTSW_INSERTION_POINT_I18N, insert, NULL);
	}
    } else {
	if (insert < length)
	    (void) xv_set(textsw, TEXTSW_INSERTION_POINT_I18N, insert, NULL);
	if (termsw->append_only_log) {
            length = (int) xv_get(textsw, TEXTSW_LENGTH_I18N);
	    termsw->read_only_mark =
		textsw_add_mark_i18n(textsw,
				(Textsw_index) (termsw->cooked_echo ? length : TEXTSW_INFINITY - 1),
				TEXTSW_MARK_READ_ONLY);
	}
    }
    return (0);
}

/* ARGSUSED */
static void
ttysw_textsw_changed_handler(textsw, insert_before, length_before,
			     replaced_from, replaced_to, count_inserted)
    Textsw          textsw;
    int             insert_before;
    int             length_before;
    int             replaced_from;
    int             replaced_to;
    int             count_inserted;
{
    CHAR            last_inserted;
#ifdef OW_I18N
    char            last_inserted_mbs[4];
    int             mbs_len;
#endif
    Termsw_view_handle view = TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw);
    Termsw_folio    termsw = TERMSW_FOLIO_FOR_VIEW(view);
    Ttysw_folio     ttysw = TTY_FROM_TERMSW(termsw->public_self);
    Ttysw_view_handle ttysw_view = TTY_VIEW_PRIVATE_FROM_TERMSW_VIEW(TERMSW_VIEW_PUBLIC(view));

    if (insert_before != length_before)
	return;
    if (termsw->cmd_started == 0) {
	if (termsw->cmd_started = (count_inserted > 0)) {
	    (void) textsw_checkpoint_undo(textsw, termsw->next_undo_point);
	    ttysw_move_mark(textsw, &termsw->user_mark,
			    (Textsw_index) length_before,
			    TEXTSW_MARK_DEFAULTS);
	}
    }
    if (!termsw->cmd_started)
	termsw->next_undo_point =
	    (caddr_t) textsw_checkpoint_undo(textsw,
					     (caddr_t) TEXTSW_INFINITY);
    if (count_inserted >= 1) {
	/* Get the last inserted character. */
        (void) xv_get(textsw, TEXTSW_CONTENTS_I18N,
		      replaced_from + count_inserted - 1,
		      &last_inserted, 1);
#ifdef OW_I18N
        mbs_len = wctomb( last_inserted_mbs , last_inserted );
        if ( mbs_len == 1  &&
                /* last_inserted_mbs[0] == ttysw->ltchars.t_rprntc ) {*/
                last_inserted_mbs[0] == tty_getrprntc(ttysw)) {
#else
	if (last_inserted == tty_getrprntc(ttysw)) {
#endif
#ifndef	BUFSIZE
#define	BUFSIZE 1023
#endif	/* BUFSIZE */
	    CHAR            buf[BUFSIZE + 1];
	    CHAR            cr_nl[4];
	    int             buflen = 0;
	    Textsw_index    start_from;
	    Textsw_index    length = (int) xv_get(textsw, TEXTSW_LENGTH_I18N);

	    cr_nl[0] = (CHAR)'\r';
	    cr_nl[1] = (CHAR)'\n';
	    cr_nl[2] = (CHAR)'\0';
	    start_from = textsw_find_mark_i18n(textsw, termsw->user_mark);
	    if (start_from == (length - 1)) {
		*buf = (CHAR)'\0';
	    } else {
                (void) xv_get(textsw, TEXTSW_CONTENTS_I18N, start_from, buf,
			  (buflen = MIN(BUFSIZE, length - 1 - start_from)));
	    }
	    termsw->pty_owes_newline = 0;
	    termsw->cmd_started = 0;
	    ttysw_move_mark(textsw, &termsw->pty_mark, length,
			    TEXTSW_MARK_DEFAULTS);
	    if (termsw->append_only_log) {
		ttysw_move_mark(textsw, &termsw->read_only_mark, length,
				TEXTSW_MARK_READ_ONLY);
	    }
	    ttysw_output_it(ttysw_view, cr_nl, 2);
	    if (buflen > 0)
#ifdef OW_I18N
                ttysw_input_it_wcs(ttysw, buf, buflen);
        } else if ( mbs_len == 1 &&
                /* last_inserted_mbs[0] == ttysw->ltchars.t_lnextc) { */
                last_inserted_mbs[0] == tty_getlnextc(ttysw)) {
            termsw->literal_next = TRUE;
        } else if ( mbs_len == 1 &&
                   /* ( last_inserted_mbs[0] == ttysw->tchars.t_brkc */
                   ( last_inserted_mbs[0] == tty_geteolc(ttysw)
		   || last_inserted_mbs[0] == tty_geteol2c(ttysw)
                   || last_inserted_mbs[0] == (wchar_t)'\n'
                   || last_inserted_mbs[0] == (wchar_t)'\r' )) {
#else
		ttysw_input_it(ttysw, buf, buflen);
	} else if (last_inserted == tty_getlnextc(ttysw)) {
	    termsw->literal_next = TRUE;
	} else if (last_inserted == tty_geteolc(ttysw)
		   || last_inserted == tty_geteol2c(ttysw)
		   || last_inserted == '\n'
		   || last_inserted == '\r') {
#endif
	    (void) ttysw_scan_for_completed_commands(ttysw_view, -1, 0);
	}
    }
}

Pkg_private void
ttysw_textsw_changed(textsw, attributes)
    Textsw          textsw;
    Attr_avlist     attributes;
{
    register Attr_avlist attrs;
    int             do_default = 0;
    register Termsw_view_handle view = TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw);
    register Termsw_folio termsw = TERMSW_FOLIO_FOR_VIEW(view);
    register Ttysw_folio ttysw = TTY_FROM_TERMSW(termsw->public_self);

    for (attrs = attributes; *attrs; attrs = attr_next(attrs)) {
	switch ((Textsw_action) (*attrs)) {
	  case TEXTSW_ACTION_CAPS_LOCK:
	    ttysw->ttysw_capslocked = (attrs[1]) ? TTYSW_CAPSLOCKED : 0;
	    ttysw_display_capslock(ttysw);
	    break;
	  case TEXTSW_ACTION_REPLACED:
	    if (!termsw->doing_pty_insert)
		ttysw_textsw_changed_handler(textsw,
			     (int) attrs[1], (int) attrs[2], (int) attrs[3],
					     (int) attrs[4], (int) attrs[5]);
	    break;
	  case TEXTSW_ACTION_LOADED_FILE:{
		Textsw_index    insert;
		Textsw_index    length;

		insert =
		    (Textsw_index) xv_get(textsw, TEXTSW_INSERTION_POINT_I18N);
                length = (Textsw_index) xv_get(textsw, TEXTSW_LENGTH_I18N);
		if (length == insert + 1) {
		    (void) xv_set(textsw,
				  TEXTSW_INSERTION_POINT_I18N, length, NULL);
		    ttysw_reset_column(ttysw);
		} else if (length == 0) {
		    ttysw_reset_column(ttysw);
		}
		if (length < textsw_find_mark_i18n(textsw, termsw->pty_mark)) {
		    ttysw_move_mark(textsw, &termsw->pty_mark, length,
				    TEXTSW_MARK_DEFAULTS);
		}
		if (termsw->append_only_log) {
		    ttysw_move_mark(textsw, &termsw->read_only_mark,
				    length, TEXTSW_MARK_READ_ONLY);
		}
		termsw->cmd_started = FALSE;
		termsw->pty_owes_newline = 0;
	    }
	  default:
	    do_default = TRUE;
	    break;
	}
    }
    if (do_default) {
	(void) textsw_default_notify(textsw, attributes);
    }
}
