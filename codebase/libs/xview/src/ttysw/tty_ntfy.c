#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)tty_ntfy.c 20.45 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Notifier related routines for the ttysw.
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <pixrect/pixrect.h>
#include <pixrect/pixfont.h>

#include <xview_private/portable.h>

#include <xview_private/i18n_impl.h>
#include <xview/notify.h>
#include <xview/rect.h>
#include <xview/rectlist.h>
#include <xview/win_input.h>
#include <xview/win_notify.h>
#include <xview/defaults.h>
#include <xview/ttysw.h>
#include <xview/notice.h>
#include <xview/frame.h>
#include <xview/termsw.h>
#include <xview/window.h>
#include <xview_private/tty_impl.h>
#include <xview_private/term_impl.h>
#include <xview_private/ultrix_cpt.h>

#if defined(__linux) && defined(__GLIBC__)
/* martin.buck@bigfoot.com */
#if defined(__linux)
#include <sys/ioctl.h>
#else
#include <ioctls.h>
#endif
#endif

#define PTY_OFFSET	(int) &(((Ttysw_folio)0)->ttysw_pty)

extern void     textsw_display();

/* #else */
#include <xview_private/charimage.h>
#include <xview_private/charscreen.h>
#undef length
#define ITIMER_NULL   ((struct itimerval *)0)

#define	TTYSW_USEC_DELAY 100000
/* Duplicate of what's in ttysw_tio.c */

Notify_value    ttysw_itimer_expired();
static Notify_value ttysw_pty_output_pending();
Notify_value    ttysw_pty_input_pending();
static Notify_value ttysw_prioritizer();
Notify_func     ttysw_cached_pri;	/* Default prioritizer */

extern Notify_value ttysw_text_destroy();	/* Destroy func for termsw */
extern Notify_value ttysw_text_event();	/* Event func for termsw */

/* static */ void cim_resize();

/*
 * These three procedures are no longer needed because the pty driver bug
 * that causes the ttysw to lock up is fixed. void add_pty_timer(); void
 * tysw_remove_pty_timer(); Notify_value
 * d();
 */

/* Accelerator to avoid excessive notifier activity */
int             ttysw_waiting_for_pty_input;
/* Accelerator to avoid excessive notifier activity */
static          ttysw_waiting_for_pty_output;

/* shorthand - Duplicate of what's in ttysw_main.c */

#define	iwbp	ttysw->ttysw_ibuf.cb_wbp
#define	irbp	ttysw->ttysw_ibuf.cb_rbp
#define	iebp	ttysw->ttysw_ibuf.cb_ebp
#define	ibuf	ttysw->ttysw_ibuf.cb_buf
#define	owbp	ttysw->ttysw_obuf.cb_wbp
#define	orbp	ttysw->ttysw_obuf.cb_rbp

Pkg_private void
ttysw_interpose(ttysw_folio)
    Ttysw_folio     ttysw_folio;
{
    Tty             ttysw_folio_public = TTY_PUBLIC(ttysw_folio);

    (void) notify_set_input_func((Notify_client) ttysw_folio_public,
			   ttysw_pty_input_pending, ttysw_folio->ttysw_pty);
    ttysw_waiting_for_pty_input = 1;
    ttysw_cached_pri = notify_set_prioritizer_func(
		     (Notify_client) ttysw_folio_public, ttysw_prioritizer);
}


Pkg_private void
ttysw_interpose_on_textsw(textsw)
    Textsw_view     textsw;	/* This is really a termsw view public */
{
    (void) notify_interpose_event_func(
		     (Notify_client) textsw, ttysw_text_event, NOTIFY_SAFE);
    (void) notify_interpose_event_func(
		(Notify_client) textsw, ttysw_text_event, NOTIFY_IMMEDIATE);
    /* Since we also post NOTIFY_IMMEDIATE, we need to set up an immediate */
    /* event handler. 							   */
    (void) notify_interpose_event_func(
		(Notify_client) textsw, ttysw_text_event, NOTIFY_IMMEDIATE);
#ifdef SUNVIEW1
    (void) notify_interpose_destroy_func(
				(Notify_client) textsw, ttysw_text_destroy);
#endif
}

Pkg_private int
ttysw_destroy(ttysw_folio_public, status)
    Tty             ttysw_folio_public;
    Destroy_status  status;
{
    register Ttysw_folio ttysw_folio_private = TTY_PRIVATE_FROM_ANY_PUBLIC(ttysw_folio_public);

    if ((status != DESTROY_CHECKING) && (status != DESTROY_SAVE_YOURSELF)) {
	/*
	 * Pty timer is no longer needed because the pty driver bug that
	 * causes the ttysw to lock up is fixed.
	 * ttysw_remove_pty_timer(ttysw);

	(void) notify_set_itimer_func((Notify_client) (ttysw_folio_public),
				      ttysw_itimer_expired,
			  ITIMER_REAL, (struct itimerval *) 0, ITIMER_NULL);
	 */

#ifdef SunOS3
	/* Sending both signal is to cover all bases  */
	ttysw_sendsig(ttysw, (Textsw) 0, SIGTERM);
	ttysw_sendsig(ttysw, (Textsw) 0, SIGHUP);
#endif

	ttysw_done(ttysw_folio_private);
	(void) notify_remove((Notify_client)ttysw_folio_public);
	(void) notify_remove((Notify_client)ttysw_folio_private);
	return (XV_OK);
    }
    return (XV_OK);
}

Pkg_private void
ttysw_sigwinch(ttysw)
    Ttysw_folio     ttysw;
{
    int             pgrp;
    int             sig = SIGWINCH;

    /* if no child, then just return. */
    if (ttysw->ttysw_pidchild==TEXTSW_INFINITY) {
	return;
    }
    /*
     * 2.0 tty based programs relied on getting SIGWINCHes at times other
     * then when the size changed.  Thus, for compatibility, we also do that
     * here.  However, I wish that I could get away with only sending
     * SIGWINCHes on resize.
     */
    /* Notify process group that terminal has changed. */
#ifdef __linux
    /* Under Linux, we can use this ioctl only on the master pty,
     * otherwise we'll get ENOTTY. It seems to return the right process
     * group nevertheless.
     */
    if (ioctl(ttysw->ttysw_pty, TIOCGPGRP, &pgrp) == -1) {
#else
    if (ioctl(ttysw->ttysw_tty, TIOCGPGRP, &pgrp) == -1) {
#endif
	perror(XV_MSG("ttysw_sigwinch, can't get tty process group"));
	return;
    }
    /*
     * Only killpg when pgrp is not tool's.  This is the case of haven't
     * completed ttysw_fork yet (or even tried to do it yet).
     */
#ifndef __linux
    if (getpgrp(0) != pgrp)
#else
    if (getpgrp() != pgrp)
#endif
	/*
	 * killpg could return -1 with errno == ESRCH but this is OK.
	 */
#ifndef sun
	(void) killpg(pgrp, SIGWINCH);
#else				/* only SunOS supports the TIOCSIGNAL ioctl */
	ioctl(ttysw->ttysw_pty, TIOCSIGNAL, &sig);
#endif
    return;
}

Pkg_private void
ttysw_sendsig(ttysw, textsw, sig)
    Ttysw_folio     ttysw;
    Textsw          textsw;
    int             sig;
{
    int             control_pg;

    /* if no child, then just return. */
    if (ttysw->ttysw_pidchild==TEXTSW_INFINITY) {
	return;
    }
    /* Send the signal to the process group of the controlling tty */
#ifdef __linux
    /* See the comment in ttysw_sigwinch */
    if (ioctl(ttysw->ttysw_pty, TIOCGPGRP, &control_pg) >= 0) {
#else
    if (ioctl(ttysw->ttysw_tty, TIOCGPGRP, &control_pg) >= 0) {
#endif
	/*
	 * Flush our buffers of completed and partial commands. Be sure to do
	 * this BEFORE killpg, or we'll flush the prompt coming back from the
	 * shell after the process dies.
	 */
	(void) ttysw_flush_input((caddr_t) ttysw);

	if (textsw) {
	    Termsw_folio    termsw =
	    TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TEXTSW(textsw));

	    ttysw_move_mark(textsw, &termsw->pty_mark,
			    (Textsw_index) xv_get(textsw, TEXTSW_LENGTH_I18N),
			    TEXTSW_MARK_DEFAULTS);
	}
	if (TTY_IS_TERMSW(ttysw)) {
	    Termsw_folio    termsw =
	    TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TTY_PRIVATE(ttysw));

	    termsw->cmd_started = 0;
	    termsw->pty_owes_newline = 0;
	}
#	if defined(XV_USE_SVR4_PTYS) || defined(sun)
	(void) ioctl(ttysw->ttysw_pty, TIOCSIGNAL, &sig);;
#	else
	(void) killpg(control_pg, sig);
#	endif
    } else
	perror(XV_MSG("ioctl"));
}

/* ARGSUSED */
/* BUG ALERT: Why was this marked Xv_public in V3.0?  Should be Pkg_private. */
Xv_public       Notify_value
ttysw_event(ttysw_view_public, event, arg, type)
    Tty_view        ttysw_view_public;
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    Ttysw_folio     ttysw_folio_private = TTY_PRIVATE_FROM_ANY_VIEW(ttysw_view_public);

    if ((*(ttysw_folio_private)->ttysw_eventop) (ttysw_view_public, event) == TTY_DONE)
#ifdef OW_I18N
	/*
	 * window pkg needs those two events to set/unset IC focus.
	 */
	if ( event_action(event) == KBD_USE || event_action(event) == KBD_DONE )
	    return notify_next_event_func(ttysw_view_public,
						(Notify_event)event, arg, type);
	else
            return (NOTIFY_DONE);
#else
	return (NOTIFY_DONE);
#endif
    else
	return (NOTIFY_IGNORED);

}

Pkg_private void
ttysw_display(ttysw, ie)
    Ttysw_folio     ttysw;
    Event	    *ie;
{
    if (ttysw_getopt((caddr_t) ttysw, TTYOPT_TEXT)) {
	textsw_display(TEXTSW_FROM_TTY(ttysw));
    } else {
	(void) ttysw_prepair(event_xevent(ie));
	/* primary selection is repainted in ttysw_prepair. */
	if (ttysw->ttysw_secondary.sel_made)
		ttyhiliteselection(&ttysw->ttysw_secondary, SELN_SECONDARY);
    }
}

static Notify_value
ttysw_pty_output_pending(tty_public, pty)
    Tty             tty_public;
    int             pty;
{
    (void) ttysw_pty_output(TTY_PRIVATE_FROM_ANY_PUBLIC(tty_public), pty);
    return (NOTIFY_DONE);
}

Pkg_private Notify_value
ttysw_pty_input_pending(tty_public, pty)
    Tty             tty_public;
    int             pty;
{
    (void) ttysw_pty_input(TTY_PRIVATE_FROM_ANY_PUBLIC(tty_public), pty);
    return (NOTIFY_DONE);
}

/* ARGSUSED */
Pkg_private Notify_value
ttysw_itimer_expired(tty_public, which)
    Tty             tty_public;
    int             which;
{
    (void) ttysw_handle_itimer(TTY_PRIVATE_FROM_ANY_PUBLIC(tty_public));
    return (NOTIFY_DONE);
}

/*
 * Conditionally set conditions
 */
Pkg_private void
ttysw_reset_conditions(ttysw_view)
    Ttysw_view_handle ttysw_view;
{
    register Ttysw_folio ttysw = TTY_FOLIO_FROM_TTY_VIEW_HANDLE(ttysw_view);
    static struct itimerval ttysw_itimerval = {{0, 0}, {0, TTYSW_USEC_DELAY}};
    register int    pty = ttysw->ttysw_pty;
    Termsw_folio    termsw;

    /* Send program output to terminal emulator */
    (void) ttysw_consume_output(ttysw_view);
    /* Toggle between window input and pty output being done */
    termsw = TERMSW_FOLIO_FOR_VIEW(TERMSW_VIEW_PRIVATE_FROM_TTY_PRIVATE(ttysw));
    if ((iwbp > irbp && ttysw_pty_output_ok(ttysw)) ||
	    (ttysw_getopt((caddr_t) ttysw, TTYOPT_TEXT) && termsw != NULL &&
	    termsw->pty_eot > -1)) {
	if (!ttysw_waiting_for_pty_output) {
	    /* Wait for output to complete on pty */
	    (void) notify_set_output_func((Notify_client) (TTY_PUBLIC(ttysw)),
					  ttysw_pty_output_pending, pty);
	    ttysw_waiting_for_pty_output = 1;
	    /*
	     * Pty timer is no longer needed because the pty driver bug that
	     * causes the ttysw to lock up is fixed.
	     * (void)ttysw_add_pty_timer(ttysw, &pty_itimerval);
	     */
	}
    } else {
	if (ttysw_waiting_for_pty_output) {
	    /* Don't wait for output to complete on pty any more */
	    (void) notify_set_output_func((Notify_client) (TTY_PUBLIC(ttysw)),
					  NOTIFY_FUNC_NULL, pty);
	    ttysw_waiting_for_pty_output = 0;
	}
    }
    /* Set pty input pending */
    if (owbp == orbp) {
	if (!ttysw_waiting_for_pty_input) {
	    (void) notify_set_input_func((Notify_client) (TTY_PUBLIC(ttysw)),
					 ttysw_pty_input_pending, pty);
	    ttysw_waiting_for_pty_input = 1;
	}
    } else {
	if (ttysw_waiting_for_pty_input) {
	    (void) notify_set_input_func((Notify_client) (TTY_PUBLIC(ttysw)),
					 NOTIFY_FUNC_NULL, pty);
	    ttysw_waiting_for_pty_input = 0;
	}
    }
    /*
     * Try to optimize displaying by waiting for image to be completely
     * filled after being cleared (vi(^F ^B) page) before painting.
     */
    if (!ttysw_getopt((caddr_t) ttysw, TTYOPT_TEXT) && delaypainting)
	(void) notify_set_itimer_func((Notify_client) (TTY_PUBLIC(ttysw)),
				      ttysw_itimer_expired,
				ITIMER_REAL, &ttysw_itimerval, ITIMER_NULL);
#ifdef DEBUG
   printf("ttysw_reset_conditions() waiting_for_pty_output=%d, waiting_for_pty_input=%d\n",
	ttysw_waiting_for_pty_output, ttysw_waiting_for_pty_input);
#endif
}



static          Notify_value
ttysw_prioritizer(tty_public, nfd, ibits_ptr, obits_ptr, ebits_ptr,
	 nsig, sigbits_ptr, auto_sigbits_ptr, event_count_ptr, events, args)
    Tty             tty_public;
    fd_set         *ibits_ptr, *obits_ptr, *ebits_ptr;
    int             nsig, *sigbits_ptr, *event_count_ptr;
    register int   *auto_sigbits_ptr, nfd;
    Notify_event   *events;
    Notify_arg     *args;
/* Called directly from notify_client(), so tty_public may be termsw! */
{
    Ttysw_folio     ttysw = TTY_PRIVATE_FROM_ANY_PUBLIC(tty_public);
    Ttysw_view_handle ttysw_view = TTY_VIEW_HANDLE_FROM_TTY_FOLIO(ttysw);
    register int    pty = ttysw->ttysw_pty;
    register int    i;
    int             count = *event_count_ptr;


    ttysw->ttysw_flags |= TTYSW_FL_IN_PRIORITIZER;
    if (*auto_sigbits_ptr) {
	/* Send itimers */
	if (*auto_sigbits_ptr & SIG_BIT(SIGALRM)) {
	    (void) notify_itimer((Notify_client) (tty_public), ITIMER_REAL);
	    *auto_sigbits_ptr &= ~SIG_BIT(SIGALRM);
	}
    }
    if (FD_ISSET(ttysw->ttysw_tty, obits_ptr)) {
	(void) notify_output((Notify_client) (tty_public), ttysw->ttysw_tty);
	FD_CLR(ttysw->ttysw_tty, obits_ptr);
    }
    /*
     * Post events. This is done in place of calling notify_input with wfd's
     */
    for (i = 0; i < count; i++)
	(void) notify_event((Notify_client) (tty_public), events[i], args[i]);

    if (FD_ISSET(pty, obits_ptr)) {
	(void) notify_output((Notify_client) (tty_public), pty);
	FD_CLR(pty, obits_ptr);
	/*
	 * Pty timer is no longer needed because the
	 * (void)ttysw_remove_pty_timer(ttysw);
	 */
    }
    if (FD_ISSET(pty, ibits_ptr)) {
	/* This is aviod the race condition, created by the timer flush */

	if (IS_TERMSW(TTY_PUBLIC(ttysw))
	    && (ttysw_getopt(ttysw, TTYOPT_TEXT))) {
	    textsw_flush_std_caches(TTY_VIEW_PUBLIC(ttysw_view));
	}
	(void) notify_input((Notify_client) (tty_public), pty);
	FD_CLR(pty, ibits_ptr);
    }
    (void) ttysw_cached_pri(tty_public, nfd, ibits_ptr, obits_ptr, ebits_ptr,
	nsig, sigbits_ptr, auto_sigbits_ptr, event_count_ptr, events, args);
    ttysw_reset_conditions(ttysw_view);
    ttysw->ttysw_flags &= ~TTYSW_FL_IN_PRIORITIZER;

    return (NOTIFY_DONE);
}

Pkg_private void
ttysw_resize(ttysw_view)
    Ttysw_view_handle ttysw_view;
{
    Ttysw_folio     ttysw = TTY_FOLIO_FROM_TTY_VIEW_HANDLE(ttysw_view);
    int             pagemode;

    /*
     * Turn off page mode because send characters through character image
     * manager during resize.
     */
    pagemode = ttysw_getopt((caddr_t) ttysw, TTYOPT_PAGEMODE);
    (void) ttysw_setopt((caddr_t) ttysw, TTYOPT_PAGEMODE, NULL);
    if (ttysw_getopt((caddr_t) ttysw, TTYOPT_TEXT)) {
	(void) xv_tty_new_size(ttysw, textsw_screen_column_count(TTY_PUBLIC(ttysw)),
			       textsw_screen_line_count(TTY_PUBLIC(ttysw)));
    } else {
	/* Have character image update self */
	csr_resize(ttysw_view);
	/* Have screen update any size change parameters */
	cim_resize(ttysw_view);
	if (TTY_IS_TERMSW(ttysw)) {
	    Termsw_private *termsw = TERMSW_VIEW_PRIVATE_FROM_TTY_PRIVATE(ttysw);

	    termsw->folio->ttysw_resized++;
	}
    }
    /* Turn page mode back on */
    (void) ttysw_setopt((caddr_t) ttysw, TTYOPT_PAGEMODE, pagemode);
}

/* BUG ALERT: No XView prefix */
/* static */ void
cim_resize(ttysw_view)
    Ttysw_view_handle ttysw_view;
{
    struct rectlist rl;

    /* Prevent any screen writing by making clipping null */
    rl = rl_null;
    win_set_clip(TTY_VIEW_PUBLIC(ttysw_view), &rl);
    /* Redo character image */
    (void) ttysw_imagerepair(ttysw_view);
    /* Restore normal clipping */
    win_set_clip(TTY_VIEW_PUBLIC(ttysw_view), RECTLIST_NULL);
}

/* BUG ALERT: No XView prefix */
Pkg_private void
csr_resize(ttysw_view)
    Ttysw_view_handle ttysw_view;
{
    Rect           *r_new = (Rect *) xv_get(TTY_VIEW_PUBLIC(ttysw_view), WIN_RECT);
    Ttysw_folio     ttysw = TTY_FOLIO_FROM_TTY_VIEW_HANDLE(ttysw_view);

    /* Update notion of size */
    winwidthp = r_new->r_width;
    winheightp = r_new->r_height;
    /* Don't currently support selections across size changes */
    ttynullselection(ttysw);
}
