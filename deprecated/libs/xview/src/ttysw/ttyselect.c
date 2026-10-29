#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)ttyselect.c 20.47 93/06/29";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#ifndef XVIEW_USE_INSECURE_TMPFILES
/* martin.buck@bigfoot.com */
#include <dirent.h>
#endif

#include <pixrect/pixrect.h>
#include <pixrect/pixfont.h>
#include <xview_private/i18n_impl.h>
#include <xview/defaults.h>
#include <xview/rect.h>
#include <xview/rectlist.h>
#include <xview/pixwin.h>
#include <xview/win_input.h>
#include <xview/selection.h>
#include <xview/sel_svc.h>
#include <xview/sel_attrs.h>
#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview/ttysw.h>
#include <xview_private/tty_impl.h>
#include <xview_private/ttyansi.h>
#include <xview_private/charimage.h>
#include <xview_private/charscreen.h>

/*
 * global which can be used to make a shell tool which doesn't talk to the
 * service
 */
int             ttysel_use_seln_service = 1;

/* global & private procedures	 */
void            ttyhiliteselection();

static void     tvsub(),
                ttycountchars(),
#ifdef  OW_I18N
                ttycountbytes(),
#endif
                ttyenumerateselection(),
                ttyhiliteline(),
                ttysel_empty(),
                ttysel_resolve(),
                ttysel_read(),
                ttysel_write(),
                ttyputline(),
                ttysel_function(),
                ttysel_end_request();
	     void ttysortextents();
/* static */ void ttysel_cancel();
/* static */ void ttysel_get_selection();
/* static */ void ttyhiliteselection(); /* BUG ALERT: No XView prefix */
/* static */ void ttygetselection();	/* BUG ALERT: No XView prefix */

static int      ttysel_insel(),
                ttysel_eq();

static Seln_result ttysel_copy_in(),
                ttysel_copy_out(),
#ifdef  OW_I18N
                ttysel_copy_out_wchar(),
#endif
                ttysel_reply();

static struct ttyselection *
                ttysel_from_rank();

struct ttysel_context {
    unsigned        continued;
    struct ttysubwindow *ttysw;
    unsigned        bytes_left;
#ifdef  OW_I18N
    Bool            request_is_byte;
    Bool            request_is_ascii;
#endif
};

#define	SEL_NULLPOS	-1

#define	TSE_NULL_EXTENT	{SEL_NULLPOS, SEL_NULLPOS}

static struct textselpos tse_null_extent = TSE_NULL_EXTENT;
static struct ttyselection ttysw_nullttysel = {
    0, 0, SEL_CHAR, 0, TSE_NULL_EXTENT, TSE_NULL_EXTENT, {0, 0}
};

static Ttysw_folio ttysel_ttysw;/* stash for ttysel_read   */
static struct ttyselection *ttysel_ttysel;	/* stash for ttysel_write  */

static struct timeval maxinterval = {0, 400000};	/* XXX - for now */

#ifdef XVIEW_USE_INSECURE_TMPFILES
/* martin.buck@bigfoot.com */
static char    *ttysel_filename = "/tmp/ttyselection";
#else
static char    ttysel_filename[MAXNAMLEN];
#endif

static
  ttysel_resynch(register struct ttysubwindow *ttysw,
                 register Seln_function_buffer *buffer);

/* static */ int
ttysw_is_seln_nonzero(ttysw, rank)
    register struct ttysubwindow *ttysw;
    Seln_rank       rank;
{
    Seln_holder     holder;
    Seln_request   *req_buf;
    char          **argv;
    int             bytesize = 0;

    if (!ttysw_getopt((caddr_t) ttysw, TTYOPT_SELSVC)) {
	return 0;
    }
    holder = seln_inquire(rank);
    if (holder.state != SELN_NONE) {
	req_buf = seln_ask(&holder, SELN_REQ_BYTESIZE, 0, NULL);
	argv = (char **) req_buf->data;
	if (*argv++ == (char *) SELN_REQ_BYTESIZE) {
	    bytesize = (int) *argv;
	}
    }
    return bytesize;
}


/*
 * init_client:
 */
Pkg_private void
ttysel_init_client(ttysw)
    register Ttysw_folio ttysw;
{
    if (!ttysw_getopt((caddr_t) ttysw, TTYOPT_SELSVC)) {
	return;
    }
    ttysw->ttysw_caret = ttysw_nullttysel;
    ttysw->ttysw_primary = ttysw_nullttysel;
    ttysw->ttysw_secondary = ttysw_nullttysel;
    ttysw->ttysw_shelf = ttysw_nullttysel;
    ttysw->ttysw_seln_client =
	seln_create(ttysel_function, ttysel_reply, (char *) ttysw);
    if (ttysw->ttysw_seln_client == (char *) NULL) {
	(void) ttysw_setopt(TTY_VIEW_HANDLE_FROM_TTY_FOLIO(ttysw), TTYOPT_SELSVC, FALSE);
    }
}

Pkg_private void
ttysel_destroy(ttysw)
    register struct ttysubwindow *ttysw;
{
    if (!ttysw_getopt((caddr_t) ttysw, TTYOPT_SELSVC)) {
	return;
    }
    if (ttysw->ttysw_seln_client != (char *) NULL) {
	seln_destroy(ttysw->ttysw_seln_client);
	ttysw->ttysw_seln_client = (char *) NULL;
    }
}

/*
 * Make sure we have desired selection.
 */
Pkg_private void
ttysel_acquire(ttysw, sel_desired)
    register struct ttysubwindow *ttysw;
    register Seln_rank sel_desired;
{
    register Seln_rank sel_received;
    register struct ttyselection *ttysel;

    if (!ttysw_getopt((caddr_t) ttysw, TTYOPT_SELSVC)) {
	return;
    }
    ttysel = ttysel_from_rank(ttysw, sel_desired);
    if (!ttysel->sel_made) {
	if (sel_desired == SELN_CARET) {
	    Seln_holder     holder;

	    holder = seln_inquire(SELN_UNSPECIFIED);
	    if (holder.rank != SELN_PRIMARY)
		return;
	}
	sel_received = seln_acquire(ttysw->ttysw_seln_client, sel_desired);
	if (sel_received == sel_desired) {
	    ttysel->sel_made = TRUE;
	    ttysel_empty(ttysel);
	} else {
	    (void) seln_done(ttysw->ttysw_seln_client, sel_received);
	}
    }
}

/*
 * Return rank of global selection state.
 */
/* NOT USED */
Seln_rank
ttysel_mode(ttysw)
    struct ttysubwindow *ttysw;
{
    Seln_holder     sel_holder;

    if (!ttysw_getopt((caddr_t) ttysw, TTYOPT_SELSVC)) {
	return SELN_PRIMARY;
    } else {
	sel_holder = seln_inquire(SELN_UNSPECIFIED);
	return sel_holder.rank;
    }
}

/*
 * Make a new selection. If multi is set, check for multi-click.
 */
Pkg_private void
ttysel_make(ttysw, event, multi)
    register struct ttysubwindow *ttysw;
    register struct inputevent *event;
    int             multi;
{
    register Seln_rank sel_received;
    register struct ttyselection *ttysel;
    struct textselpos tspb, tspe;
    struct timeval  td;

    if (!ttysw_getopt((caddr_t) ttysw, TTYOPT_SELSVC)) {
	sel_received = SELN_PRIMARY;
    } else {
	sel_received =
	    seln_acquire(ttysw->ttysw_seln_client, SELN_UNSPECIFIED);
    }
    if (sel_received == SELN_PRIMARY) {
	ttysel = &ttysw->ttysw_primary;
	if (ttysw_getopt((caddr_t) ttysw, TTYOPT_SELSVC)) {
	    ttysel_acquire(ttysw, SELN_CARET);

	}
	if (ttysw->ttysw_secondary.sel_made) {
	    (void) ttysel_cancel(ttysw, SELN_SECONDARY);	/* insurance  */
	}
    } else if (sel_received == SELN_SECONDARY) {
	ttysel = &ttysw->ttysw_secondary;
    } else {
	return;
    }
    ttysel_resolve(&tspb, &tspe, SEL_CHAR, event);
    if (multi && ttysel->sel_made) {
	tvsub(&td, &event->ie_time, &ttysel->sel_time);
	if (ttysel_insel(ttysel, &tspe) && timercmp(&td, &maxinterval, <)) {
	    ttysel_adjust(ttysw, event, TRUE, TRUE);
	    return;
	}
    }
    if (ttysel->sel_made)
	ttysel_deselect(ttysel, sel_received);
    ttysel->sel_made = TRUE;
    ttysel->sel_begin = tspb;
    ttysel->sel_end = tspe;
    ttysel->sel_time = event->ie_time;
    ttysel->sel_level = SEL_CHAR;
    ttysel->sel_anchor = 0;
    ttysel->sel_null = FALSE;
    ttyhiliteselection(ttysel, sel_received);
}

/*
 * Move the current selection.
 */
/* NOT USED */
void
ttysel_move(ttysw, event)
    register struct ttysubwindow *ttysw;
    register struct inputevent *event;
{
    register Seln_rank rank;
    register struct ttyselection *ttysel;
    struct textselpos tspb, tspe;

    if (ttysw->ttysw_secondary.sel_made) {
	rank = SELN_SECONDARY;
	ttysel = &ttysw->ttysw_secondary;
    } else if (ttysw->ttysw_primary.sel_made) {
	rank = SELN_PRIMARY;
	ttysel = &ttysw->ttysw_primary;
    } else {
	return;
    }
    ttysel_resolve(&tspb, &tspe, ttysel->sel_level, event);
    ttyhiliteselection(ttysel, rank);
    ttysel->sel_begin = tspb;
    ttysel->sel_end = tspe;
    ttysel->sel_time = event->ie_time;
    ttysel->sel_anchor = 0;
    ttysel->sel_null = FALSE;
    ttyhiliteselection(ttysel, rank);
}

/*
 * Adjust the current selection according to the event. If multi is set,
 * check for multi-click.
 */
Pkg_private void
ttysel_adjust(ttysw, event, multi, ok_to_extend)
    register struct ttysubwindow *ttysw;
    struct inputevent *event;
    int             multi;
    int             ok_to_extend;
{
    register struct textselpos *tb;
    register struct textselpos *te;
    Seln_rank       rank;
    int             count;
    int             extend = 0;
    struct textselpos tspc, tspb, tspe, tt;
    struct ttyselection *ttysel;
    struct timeval  td;

    if (ttysw->ttysw_secondary.sel_made) {
	rank = SELN_SECONDARY;
	ttysel = &ttysw->ttysw_secondary;
    } else if (ttysw->ttysw_primary.sel_made) {
	rank = SELN_PRIMARY;
	ttysel = &ttysw->ttysw_primary;
    } else {
	return;
    }
    tb = &ttysel->sel_begin;
    te = &ttysel->sel_end;
    if (!ttysel->sel_made || ttysel->sel_null)
	return;
    ttysel_resolve(&tspb, &tspc, SEL_CHAR, event);
    if (multi) {
	tvsub(&td, &event->ie_time, &ttysel->sel_time);
	if (ttysel_insel(ttysel, &tspc) && timercmp(&td, &maxinterval, <) &&
	    ok_to_extend) {
	    extend = 1;
	    if (++ttysel->sel_level > SEL_MAX) {
		ttysel->sel_level = SEL_CHAR;
		extend = 0;
	    }
	}
	ttysel->sel_time = event->ie_time;
	ttysel->sel_anchor = 0;
    }
    ttysel_resolve(&tspb, &tspe, ttysel->sel_level, event);
    /*
     * If inside current selection, pull in closest end.
     */
    if (!extend && ttysel_insel(ttysel, &tspc)) {
	int             left_end, right_end;

	if (ttysel->sel_anchor == 0) {
	    /* count chars to left */
	    count = 0;
	    tt = *te;
	    *te = tspc;
	    ttyenumerateselection(ttysel, ttycountchars, (char *) (&count));
	    *te = tt;
	    left_end = count;
	    /* count chars to right */
	    count = 0;
	    tt = *tb;
	    *tb = tspc;
	    ttyenumerateselection(ttysel, ttycountchars, (char *) (&count));
	    *tb = tt;
	    right_end = count;
	    if (right_end <= left_end)
		ttysel->sel_anchor = -1;
	    else
		ttysel->sel_anchor = 1;
	}
	if (ttysel->sel_anchor == -1) {
	    if (!ttysel_eq(te, &tspe)) {
		/* pull in right end */
		tt = *tb;
		*tb = tspe;
		tb->tsp_col++;
		if (rank == SELN_PRIMARY)
		    ttyhiliteselection(ttysel, rank);
		else {
		    ttysel->dehilite_op = TRUE;
		    ttyhiliteselection(ttysel, rank);
		    ttysel->dehilite_op = FALSE;
		}
		*tb = tt;
		*te = tspe;
	    }
	} else {
	    if (!ttysel_eq(tb, &tspb)) {
		/* pull in left end */
		tt = *te;
		*te = tspb;
		te->tsp_col--;
		if (rank == SELN_PRIMARY)
		    ttyhiliteselection(ttysel, rank);
		else {
		    ttysel->dehilite_op = TRUE;
		    ttyhiliteselection(ttysel, rank);
		    ttysel->dehilite_op = FALSE;
		}
		*te = tt;
		*tb = tspb;
	    }
	}
    } else {
	/*
	 * Determine which end to extend. Both ends may extend if selection
	 * level has increased.
	 */
	int             newanchor = 0;

	if (tspe.tsp_row > te->tsp_row ||
	    tspe.tsp_row == te->tsp_row && tspe.tsp_col > te->tsp_col) {
	    if (ttysel->sel_anchor == 1) {
	        /* selection is crossing over anchor point.
	        *  pull in left end before extending right.
	        */
	        if (tb->tsp_col != te->tsp_col) {
		    tt = *te;
		    te->tsp_col--;
		    if (rank == SELN_PRIMARY)
		        ttyhiliteselection(ttysel, rank);
		    else {
		        ttysel->dehilite_op = TRUE;
		        ttyhiliteselection(ttysel, rank);
		        ttysel->dehilite_op = FALSE;
		    }
		    *te = tt;
		    *tb = *te;
	        }
		ttysel->sel_anchor = -1;
	    } else if (ttysel->sel_anchor == 0)
		newanchor = -1;
	    /* extend right end */
	    tt = *tb;
	    *tb = *te;
	    tb->tsp_col++;	/* check for overflow? */
	    *te = tspe;
	    ttyhiliteselection(ttysel, rank);
	    *tb = tt;
	}
	if (tspb.tsp_row < tb->tsp_row ||
	    tspb.tsp_row == tb->tsp_row && tspb.tsp_col < tb->tsp_col) {
	    if (ttysel->sel_anchor == -1) {
	        /* selection is crossing over anchor point.
	        *  pull in right end before extending left.
	        */
	        if (tb->tsp_col != te->tsp_col) {
		    tt = *tb;
		    tb->tsp_col++;
		    if (rank == SELN_PRIMARY)
		        ttyhiliteselection(ttysel, rank);
		    else {
		        ttysel->dehilite_op = TRUE;
		        ttyhiliteselection(ttysel, rank);
		        ttysel->dehilite_op = FALSE;
		    }
		    *tb = tt;
		    *te = *tb;
	          }
		ttysel->sel_anchor = 1;
	    } else if (ttysel->sel_anchor == 0)
		if (newanchor == 0)
		    newanchor = 1;
		else
		    newanchor = 0;
	    /* extend left end */
	    tt = *te;
	    *te = *tb;
	    te->tsp_col--;	/* check for underflow? */
	    *tb = tspb;
	    ttyhiliteselection(ttysel, rank);
	    *te = tt;
	}
	if (ttysel->sel_anchor == 0)
	    ttysel->sel_anchor = newanchor;
    }
}

/*
 * Clear out the current selection.
 */
/* static */ void
ttysel_cancel(ttysw, rank)
    register struct ttysubwindow *ttysw;
    Seln_rank       rank;
{
    struct ttyselection *ttysel;

    switch (rank) {
      case SELN_CARET:
	ttysel = &ttysw->ttysw_caret;
	break;
      case SELN_PRIMARY:
	ttysel = &ttysw->ttysw_primary;
	break;
      case SELN_SECONDARY:
	ttysel = &ttysw->ttysw_secondary;
	break;
      case SELN_SHELF:
	ttysel = &ttysw->ttysw_shelf;
	break;
      default:
	return;
    }
    if (!ttysel->sel_made)
	return;
    ttysel_deselect(ttysel, rank);
    ttysel->sel_made = FALSE;
    if (!ttysw_getopt((caddr_t) ttysw, TTYOPT_SELSVC))
	(void) seln_done(ttysw->ttysw_seln_client, rank);
}

/* XXX - compatibility kludge */
/* BUG ALERT: No XView prefix */
Pkg_private void
ttynullselection(ttysw)
    struct ttysubwindow *ttysw;
{
    (void) ttysel_cancel(ttysw, SELN_PRIMARY);
}

/*
 * Remove a selection from the screen
 */
Pkg_private void
ttysel_deselect(ttysel, rank)
    register struct ttyselection *ttysel;
    Seln_rank       rank;
{
    if (!ttysel->sel_made)
	return;
    ttysel->dehilite_op = TRUE;
    ttyhiliteselection(ttysel, rank);
    ttysel->dehilite_op = FALSE;
    if (!ttysel->sel_null)
	ttysel_empty(ttysel);
}

/*
 * Hilite a selection. Enumerate all the lines of the selection; hilite each
 * one as appropriate.
 */
/* BUG ALERT: No XView prefix */
/* static */ void
ttyhiliteselection(ttysel, rank)
    register struct ttyselection *ttysel;
    Seln_rank       rank;
{
    struct pr_size  offsets;

    if (!ttysel->sel_made || ttysel->sel_null) {
	return;
    }
    ttysel->sel_rank = rank;
    offsets.x = 0;
    offsets.y = chrheight;

    ttyenumerateselection(ttysel, ttyhiliteline, (char *) (&offsets));
}

/*
 * Set local selection as global one.
 */
/* BUG ALERT: No XView prefix */
Pkg_private void
ttysetselection(ttysw)
    register struct ttysubwindow *ttysw;
{
    int             count;
    struct selection selection;
    struct ttyselection *ttysel;

    if (ttysw->ttysw_secondary.sel_made ||
	!ttysw->ttysw_primary.sel_made)
	return;
    ttysel = &ttysw->ttysw_primary;
    ttysel_ttysel = ttysel;	/* stash for ttysel_write	 */
    selection = selnull;
    selection.sel_type = SELTYPE_CHAR;
    count = 0;
    ttyenumerateselection(ttysel, ttycountchars, (char *) (&count));
    selection.sel_items = count;
    selection.sel_itembytes = 1;
    selection.sel_pubflags = SEL_PRIMARY;
    selection.sel_privdata = 0;
    (void) selection_set(&selection, (int (*) ()) (ttysel_write),
	       (int (*) ()) 0, (int) window_get(TTY_PUBLIC(ttysw), WIN_FD));
}

/*
 * Read global selection into input stream.
 */
/* static */ void
ttygetselection(ttysw)
    Ttysw_folio     ttysw;
{
    ttysel_ttysw = ttysw;	/* stash for ttysel_read	 */
    (void) selection_get((int (*) ()) (ttysel_read),
			 (int) window_get(TTY_PUBLIC(ttysw), WIN_FD));
}


/* internal (static) routines	 */

/*
 * convert from a selection rank to a ttysel struct
 */
static struct ttyselection *
ttysel_from_rank(ttysw, rank)
    struct ttysubwindow *ttysw;
    Seln_rank       rank;
{
    switch (rank) {
      case SELN_CARET:
	return &ttysw->ttysw_caret;
      case SELN_PRIMARY:
	return &ttysw->ttysw_primary;
      case SELN_SECONDARY:
	return &ttysw->ttysw_secondary;
      case SELN_SHELF:
	return &ttysw->ttysw_shelf;
      default:
	break;
    }
    return (struct ttyselection *) NULL;
}

/*
 * Make a selection be empty
 */
static void
ttysel_empty(ttysel)
    register struct ttyselection *ttysel;
{
    ttysel->sel_null = TRUE;
    ttysel->sel_level = SEL_CHAR;
    ttysel->sel_begin = tse_null_extent;
    ttysel->sel_end = tse_null_extent;
}

/*
 * Is the specified position within the current selection?
 */
static int
ttysel_insel(ttysel, tsp)
    struct ttyselection *ttysel;
    register struct textselpos *tsp;
{
    register struct textselpos *tb = &ttysel->sel_begin;
    register struct textselpos *te = &ttysel->sel_end;

    if (tsp->tsp_row < tb->tsp_row || tsp->tsp_row > te->tsp_row)
	return (0);
    if (tb->tsp_row == te->tsp_row)
	return (tsp->tsp_col >= tb->tsp_col &&
		tsp->tsp_col <= te->tsp_col);
    if (tsp->tsp_row == tb->tsp_row)
	return (tsp->tsp_col >= tb->tsp_col);
    if (tsp->tsp_row == te->tsp_row)
	return (tsp->tsp_col <= te->tsp_col);
    return (1);
}

static int
ttysel_eq(t1, t2)
    register struct textselpos *t1, *t2;
{

    return (t1->tsp_row == t2->tsp_row && t1->tsp_col == t2->tsp_col);
}

static void
tvsub(tdiff, t1, t0)
    register struct timeval *tdiff, *t1, *t0;
{

    tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
    tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
    if (tdiff->tv_usec < 0)
	tdiff->tv_sec--, tdiff->tv_usec += 1000000;
}

static void
ttyenumerateselection(ttysel, proc, data)
    struct ttyselection *ttysel;
    register void   (*proc) ();
    register char  *data;
{
    struct textselpos *xbegin, *xend;
    register struct textselpos *begin, *end;
    register int    row;

    if (!ttysel->sel_made || ttysel->sel_null)
	return;
    /*
     * Sort extents
     */
    ttysortextents(ttysel, &xbegin, &xend);
    begin = xbegin;
    end = xend;
    /*
     * Process a line at a time
     */
    for (row = begin->tsp_row; row <= end->tsp_row; row++) {
	if (row == begin->tsp_row && row == end->tsp_row) {
	    /*
	     * Partial line hilite in middle
	     */
	    proc(begin->tsp_col, end->tsp_col, row, data, ttysel);
	} else if (row == begin->tsp_row) {
	    /*
	     * Partial line hilite from beginning
	     */
#ifdef OW_I18N
            proc(begin->tsp_col, LINE_LENGTH(image[row]), row, data, ttysel);
#else
	    proc(begin->tsp_col, LINE_LENGTH(image[row]), row, data, ttysel);
#endif
	} else if (row == end->tsp_row) {
	    /*
	     * Partial line hilite not to end
	     */
	    proc(0, end->tsp_col, row, data, ttysel);
	} else {
	    /*
	     * Full line hilite
	     */
#ifdef OW_I18N
            proc(0, LINE_LENGTH(image[row]), row, data, ttysel);
#else
	    proc(0, LINE_LENGTH(image[row]), row, data, ttysel);
#endif
	}
    }
}

static void
my_write_string(start, end, row)
    int             start, end, row;
{
    extern char     boldify;
    CHAR           *str = image[row];
    CHAR            temp_char = (CHAR)'\0';
#ifdef OW_I18N
    int             cwidth;
    int             offset;

    tty_column_wchar_type( start, row, &cwidth, &offset );
    start -= offset;
    tty_column_wchar_type( end, row, &cwidth, &offset );
    end += (cwidth - offset - 1);
#endif

    if ((end + 1) < (int)STRLEN(str)) { /* This is a very dirty trick for
                                         * speed */
	temp_char = str[end + 1];
	str[end + 1] = (CHAR)'\0';
        (void) ttysw_pclearline(start, STRLEN(str), row);
    } else
        (void) ttysw_pclearline(start, STRLEN(str) + 1, row);

    (void) ttysw_pstring((str + start), boldify, start, row, PIX_SRC);

    if (temp_char != '\0')
	str[end + 1] = temp_char;

}


static void
ttyhiliteline(start, finish, row, offsets, ttysel)
    int             start, finish, row;
    struct pr_size *offsets;
    struct ttyselection *ttysel;
{
    struct rect     r;

    rect_construct(&r, col_to_x(start),
		   row_to_y(row) + offsets->x,
		   col_to_x(finish + 1) - col_to_x(start),
		   offsets->y);
    if (r.r_width == 0)
	return;
    if (ttysel->dehilite_op)
	(void) my_write_string(start, finish, row);
    else {
	if (ttysel->sel_rank == SELN_SECONDARY)
	    (void) my_write_string(start, finish, row);

	(void) ttysw_pselectionhilite(&r, ttysel->sel_rank);
    }

}

/*
 *	this little chunk of code is used to load and check against 
 *	the known delimiters that are available in the ISO world. 
 *	the user has a choice of setting the delimiters that are
 *	used depending on the locale. there is another lovely chunk
 *	of this kind of code over in ei_text.c [jcb 5/17/90]
 */
#define	DELIMITERS	 " \t,.:;?!\'\"`*/-+=(){}[]<>\\|~@#$%^&"

static	short		delim_init	= FALSE;
static	char		delim_table[256];

static	void
init_delim_table()
{
    char	*delims;
    char	delim_chars[256];

    /* changing the logic to use the delimiters that are in the array rather than
       those characters that are simply isalnum(). this is so the delimiters can
       be expanded to include those which are in the ISO latin specification from
       the user defaults.
     */

    /* get the string from the defaults if one exists. */
    delims	= (char*)defaults_get_string( "text.delimiterChars",
				       "Text.DelimiterChars", DELIMITERS );

    /* print the string into an array to parse the potential octal/special characters */
    sprintf( delim_chars, delims );

    /* mark off the delimiters specified */
    for( delims = delim_chars; *delims; delims++ ) {
/*	    printf("%c(%o)", (isprint(*delims) ? *delims : ' '), (int)*delims ); */
	    delim_table[(int)*delims]	= TRUE;
    }
/*    printf("\n"); */

    delim_init	= TRUE;
}


#ifdef OW_I18N
static void
ttysel_resolve(tb, te, level, event)
    register struct textselpos *tb, *te;
    int             level;
    struct inputevent *event;
{
    register CHAR  *line;
    int             cwidth;
    int             offset;
 
    tb->tsp_row = y_to_row(event->ie_locy);
    if (tb->tsp_row >= ttysw_bottom)
        tb->tsp_row = MAX(0, ttysw_bottom - 1);
    else
    if( tb->tsp_row < 0 )
        tb->tsp_row = 0;
 
    line = image[tb->tsp_row];
    tb->tsp_col = x_to_col(event->ie_locx);
 
    if (tb->tsp_col > (int)LINE_LENGTH(line))
        tb->tsp_col = LINE_LENGTH(line);
           
    *te = *tb;
    switch (level) {
      case SEL_CHAR:
            tty_column_wchar_type( tb->tsp_col, tb->tsp_row, &cwidth, &offset );
            tb->tsp_col -= offset;
            tty_column_wchar_type( te->tsp_col, te->tsp_row, &cwidth, &offset );
            te->tsp_col +=( cwidth - offset -1 );
        break;
      case SEL_WORD:{
            register int     chr,col;
            CHAR             wchr;
            register unsigned char match_mode;
               /*
                *    It is no use if we start at second or latter column of a
                *    character. So we adjust the starting position to
                *    get a correct match_mode.
                */
            tty_column_wchar_type( te->tsp_col, te->tsp_row, &cwidth, &offset );
            te->tsp_col -= offset;
#ifdef  DELIM_TABLE_USE
        /*
         *      SUNDAE;  This compile switch is used if you want to
         *      use delim_table which is created ininit_delim_table
         *      to distinguish a word. When this switch is off,
         *      you use wchar_type() to distinguish a word.
         */
            if( delim_init == FALSE )
                    init_delim_table();
            match_mode  = delim_table[line[te->tsp_col]];
#else
            match_mode  = (unsigned char)wchar_type( &line[te->tsp_col] );
#endif

            for (col = te->tsp_col; col < (int)LINE_LENGTH(line); col++) {
#ifdef  DELIM_TABLE_USE
                chr = (int)line[col];
                if( (CHAR)chr == TTY_NON_WCHAR )
                        continue;
                if ( delim_table[chr] != match_mode )
                        break;
#else
                wchr = line[col];
                if( wchr == TTY_NON_WCHAR )
                        continue;
                if ( wchar_type(&wchr) != match_mode )
                        break;
#endif
            }
            /*
             *  Here, col surely points to the 1st column of a character.
             *  So we can just step one column backwards to get to the
             *  word boundary.
             */
            te->tsp_col = MAX(col - 1, tb->tsp_col);
            for (col = tb->tsp_col; col >= 0; col--) {
#ifdef  DELIM_TABLE_USE
                chr = (int)line[col];
                if( (CHAR)chr == TTY_NON_WCHAR )
                        continue;
                if ( delim_table[chr] != match_mode )
                        break;
#else
                wchr = line[col];
                if( wchr == TTY_NON_WCHAR )
                        continue;
                if ( wchar_type(&wchr) != match_mode )
                        break;
#endif
            }
            /*
             *  We can be sure that current position is the first column
             *  of a character. So offset must be zero.
             */
            tty_column_wchar_type( col, tb->tsp_row, &cwidth, &offset );
            tb->tsp_col = MIN(col + cwidth, te->tsp_col);
            break;
        }
      case SEL_LINE:
        tb->tsp_col = 0;
        te->tsp_col = LINE_LENGTH(line) - 1;
        break;
      case SEL_PARA:{
            register int    row;
 
            for (row = tb->tsp_row; row >= ttysw_top; row--)
                if (LINE_LENGTH(image[row]) == 0)
                    break;
            tb->tsp_row = MIN(tb->tsp_row, row + 1);
            tb->tsp_col = 0;
            for (row = te->tsp_row; row < ttysw_bottom; row++)
                if (LINE_LENGTH(image[row]) == 0)
                    break;
            te->tsp_row = MAX(te->tsp_row, row - 1);
            te->tsp_col = LINE_LENGTH(image[te->tsp_row]) - 1;
            break;
        }
    }    
}
#else	/* OW_I18N */
static void
ttysel_resolve(tb, te, level, event)
    register struct textselpos *tb, *te;
    int             level;
    struct inputevent *event;
{
    register char  *line;

    tb->tsp_row = y_to_row(event->ie_locy);
    if (tb->tsp_row >= ttysw_bottom)
	tb->tsp_row = MAX(0, ttysw_bottom - 1);
    else
    if( tb->tsp_row < 0 )
        tb->tsp_row = 0;

    line = image[tb->tsp_row];
    tb->tsp_col = x_to_col(event->ie_locx);

    if (tb->tsp_col > (int)LINE_LENGTH(line))
	tb->tsp_col = LINE_LENGTH(line);

    *te = *tb;
    switch (level) {
      case SEL_CHAR:
	break;
      case SEL_WORD:{
	    register int    col, chr;
	    register unsigned char match_mode;

	    if( delim_init == FALSE )
		    init_delim_table();

	    match_mode	= delim_table[line[te->tsp_col]];

	    for (col = te->tsp_col; col < (int)LINE_LENGTH(line); col++) {
		chr = line[col];
		if ( delim_table[chr] != match_mode )
		    break;
	    }
	    te->tsp_col = MAX(col - 1, tb->tsp_col);
	    for (col = tb->tsp_col; col >= 0; col--) {
		chr = line[col];
		if ( delim_table[chr] != match_mode )
		    break;
	    }
	    tb->tsp_col = MIN(col + 1, te->tsp_col);
	    break;
	}
      case SEL_LINE:
	tb->tsp_col = 0;
	te->tsp_col = LINE_LENGTH(line) - 1;
	break;
      case SEL_PARA:{
	    register int    row;

	    for (row = tb->tsp_row; row >= ttysw_top; row--)
		if (LINE_LENGTH(image[row]) == 0)
		    break;
	    tb->tsp_row = MIN(tb->tsp_row, row + 1);
	    tb->tsp_col = 0;
	    for (row = te->tsp_row; row < ttysw_bottom; row++)
		if (LINE_LENGTH(image[row]) == 0)
		    break;
	    te->tsp_row = MAX(te->tsp_row, row - 1);
	    te->tsp_col = LINE_LENGTH(image[te->tsp_row]);
	    break;
	}
    }
}
#endif

Xv_private void
ttysortextents(ttysel, begin, end)
    register struct ttyselection *ttysel;
    struct textselpos **begin, **end;
{

    if (ttysel->sel_begin.tsp_row == ttysel->sel_end.tsp_row) {
	if (ttysel->sel_begin.tsp_col > ttysel->sel_end.tsp_col) {
	    *begin = &ttysel->sel_end;
	    *end = &ttysel->sel_begin;
	} else {
	    *begin = &ttysel->sel_begin;
	    *end = &ttysel->sel_end;
	}
    } else if (ttysel->sel_begin.tsp_row > ttysel->sel_end.tsp_row) {
	*begin = &ttysel->sel_end;
	*end = &ttysel->sel_begin;
    } else {
	*begin = &ttysel->sel_begin;
	*end = &ttysel->sel_end;
    }
}

#ifdef OW_I18N
static void
ttycountchars(start, finish, row, count)
/*
 * Since it does not use the selection rank, it is not include in the
 * argument list
 */
    register int                        row, *count;
    register int                        start, finish;
{
    register int        i;
    register int        char_conut = 0;
    CHAR                *line = image[row];
 
    for( i = start; i<= finish; i++ ) {
        if( line[i] != TTY_NON_WCHAR )
                char_conut++;
    }
    *count += char_conut;
    if (LINE_LENGTH(image[row]) == finish &&
            finish == ttysw_right) {
        *count -= 1;            /* no CR on wrapped lines        */
    }
}
static void
ttycountbytes(start, finish, row, count)
/*
 * Since it does not use the selection rank, it is not include in the
 * argument list
 */
    register int                        row, *count;
    register int                        start, finish;
{
    register int        i;
    register int        byte_count = 0;
    register int        len;
    CHAR                *line = image[row];
    char                dummy[10];

    for( i = start; i<= finish; i++ ) {
        if( line[i] == TTY_NON_WCHAR )
                continue;
        len = wctomb( dummy, line[i] );
        /*
         *       Take care the case of null character
         */
        if( len == 0 )
                len = 1;
        byte_count += len;
    }
    *count += byte_count;
    if (LINE_LENGTH(image[row]) == finish &&
            finish == ttysw_right) {
        *count -= 1;            /* no CR on wrapped lines        */
    }
}
#else /* OW_I18N */
static void
ttycountchars(start, finish, row, count)
/*
 * Since it does not use the selection rank, it is not include in the
 * argument list
 */
    register int    start, finish, row, *count;
{

    *count += finish + 1 - start;
    if (LINE_LENGTH(image[row]) == finish && finish == ttysw_right) {
	*count -= 1;		/* no CR on wrapped lines	 */
    }
}
#endif

/* ARGSUSED */
static void
ttysel_write(sel, file)
    struct selection *sel;
    FILE           *file;
{
    ttyenumerateselection(ttysel_ttysel, ttyputline, (char *) (file));
}

#ifdef lint
#undef putc
#define putc(_char, _file) \
	_file = (FILE *)(_file ? _file : 0)
#endif				/* lint */

#ifdef OW_I18N
static void
ttyputline(start, finish, row, file)
    int             start;
    register int    finish;
    register int    row;
    register FILE  *file;
{
    register int    char_index;

    for (char_index = start; char_index <= finish; char_index++) {
        if (LINE_LENGTH(image[row]) == char_index) {
            /*
             * For full width lines, don't put in CR so can grab command
             * lines that wrap.
             */
            if (char_index != ttysw_right) {
                putc('\n', file);
            }
        } else {
            if( *(image[row] + char_index) != TTY_NON_WCHAR )
                putwc(*(image[row] + char_index), file);
        }
    }
}
#else
static void
ttyputline(start, finish, row, file)
    int             start;
    register int    finish;
    register int    row;
    register FILE  *file;
{
    register int    col;

    finish++;
    for (col = start; col < finish; col++) {
	if (LINE_LENGTH(image[row]) == col) {
	    /*
	     * For full width lines, don't put in CR so can grab command
	     * lines that wrap.
	     */
	    if (col != ttysw_right) {
		putc('\n', file);
	    }
	} else {
	    putc(*(image[row] + col), file);
	}
    }
}
#endif

static void
ttysel_read(sel, file)
    struct selection *sel;
    register FILE  *file;
{
    register int    buf;
    CHAR            c;

    if (sel->sel_type != SELTYPE_CHAR || sel->sel_itembytes != 1)
	return;
#ifdef OW_I18N
    while ((buf = getwc(file)) != EOF) {
#else
    while ((buf = getc(file)) != EOF) {
#endif
	c = buf;		/* goddamn possibly-signed characters!  */
#ifdef  OW_I18N
        (void) ttysw_input_it_wcs(ttysel_ttysw, &c, 1);
#else
	(void) ttysw_input_it(ttysel_ttysw, &c, 1);
#endif
    }
    ttysw_reset_conditions(TTY_VIEW_HANDLE_FROM_TTY_FOLIO(ttysel_ttysw));
}

/*
 * Do_function: respond to notification that a function key went up
 */
static void
ttysel_function(ttysw, buffer)
    register struct ttysubwindow *ttysw;
    register Seln_function_buffer *buffer;
{
    Seln_response   response;
    Seln_holder    *holder;

    response = seln_figure_response(buffer, &holder);
    switch (response) {
      case SELN_IGNORE:
	return;
      case SELN_DELETE:
      case SELN_FIND:
	break;
      case SELN_REQUEST:
	if (ttysw->ttysw_seln_client == (char *) NULL) {
	    ttygetselection(ttysw);
	} else {
	    ttysel_get_selection(ttysw, holder);
	}
	if (holder->rank == SELN_SECONDARY) {
	    ttysel_end_request(ttysw, holder, SELN_SECONDARY);
	}
	break;
      case SELN_SHELVE:{
	    FILE           *held_file;
	    struct ttyselection *ttysel;

	    ttysel = ttysel_from_rank(ttysw, SELN_PRIMARY);
	    if (!ttysel->sel_made) {
		return;
	    }
#ifndef XVIEW_USE_INSECURE_TMPFILES
	    /* martin.buck@bigfoot.com */
	    if (!ttysel_filename[0]) {
		sprintf(ttysel_filename, "%s/.ttyselection", xv_getlogindir());
	    }
#endif
	    if ((held_file = fopen(ttysel_filename, "w")) == (FILE *) NULL) {
		return;
	    }
	    (void) fchmod(fileno(held_file), 00666);	/* Allowing world to
							 * read and write */
	    ttyenumerateselection(ttysel, ttyputline, (char *) (held_file));
	    (void) fclose(held_file);
	    (void) seln_hold_file(SELN_SHELF, ttysel_filename);
	    break;
	}
      default:
	xv_error((Xv_opaque)ttysw,
		 ERROR_STRING,
		   XV_MSG("ttysw didn't recognize function to perform on selection"),
		 ERROR_PKG, TTY,
		 NULL);
	break;
    }
    ttysel_resynch(ttysw, buffer);
    if (buffer->addressee_rank == SELN_SECONDARY) {
	(void) ttysel_cancel(ttysw, buffer->addressee_rank);
    }
}

/* static */ void
ttysel_get_selection(ttysw, holder)
    Ttysw          *ttysw;
    Seln_holder    *holder;
{
    struct ttysel_context context;
#ifdef  OW_I18N
    Seln_result     query_result;
#endif

    context.continued = FALSE;
    context.ttysw = ttysw;
#ifdef  OW_I18N
    query_result =
        seln_query(holder, ttysel_copy_in, (char *) (&context),
                        SELN_REQ_CHARSIZE, 0,
                        NULL);
    if( query_result != SELN_SUCCESS ) {
#endif
    (void) seln_query(holder, ttysel_copy_in, (char *) (&context),
		      SELN_REQ_BYTESIZE, 0,
		      SELN_REQ_CONTENTS_ASCII, 0,
		      NULL);
#ifdef  OW_I18N
    }
    else {
        (void) seln_query(holder, ttysel_copy_in, (char *) (&context),
                      SELN_REQ_CHARSIZE, 0,
                      SELN_REQ_CONTENTS_WCS, 0,
                      NULL);
    }
#endif
}

Pkg_private int
ttysw_do_copy(ttysw)
    register Ttysw_folio ttysw;
{
    Seln_holder     holder;
    Seln_function_buffer buffer;
    int             ok;

    if (ok = ttysw_is_seln_nonzero(ttysw, SELN_PRIMARY)) {
	holder = seln_inquire(SELN_PRIMARY);
	(void) seln_inform(ttysw->ttysw_seln_client, SELN_FN_PUT, TRUE);
	buffer = seln_inform(ttysw->ttysw_seln_client, SELN_FN_PUT, FALSE);
	if (buffer.function != SELN_FN_ERROR &&
	    ttysw->ttysw_seln_client != NULL) {
	    ttysel_function(ttysw, &buffer);
	}
    }
    return (ok);
}

Pkg_private int
ttysw_do_paste(ttysw)
    register Ttysw_folio ttysw;
{
    Seln_holder     holder;

    holder = seln_inquire(SELN_SHELF);
    ttysel_get_selection(ttysw, &holder);
    return 1;
}




/*
 * Do the action for "Put, then Get" menu item.
 */
/* NOT USED */
void
ttysw_do_put_get(ttysw)
    register Ttysw_folio ttysw;
{
    Seln_holder     holder;
    Seln_function_buffer buffer;

    /*
     * if there is a nonzero length primary selection, do put_then_get: copy
     * the primary selection to the tty cursor, then fake Put down, up. NOTE:
     * can't Put, then Get from shelf because of **race** else do get_only:
     * copy the shelf to the tty cursor.
     */
    if (ttysw_is_seln_nonzero(ttysw, SELN_PRIMARY)) {
	holder = seln_inquire(SELN_PRIMARY);
	ttysel_get_selection(ttysw, &holder);
	(void) seln_inform(ttysw->ttysw_seln_client, SELN_FN_PUT, TRUE);
	buffer = seln_inform(ttysw->ttysw_seln_client, SELN_FN_PUT, FALSE);
	if (buffer.function != SELN_FN_ERROR &&
	    ttysw->ttysw_seln_client != NULL) {
	    ttysel_function(ttysw, &buffer);
	}
    } else if (ttysw_is_seln_nonzero(ttysw, SELN_SHELF)) {
	holder = seln_inquire(SELN_SHELF);
	ttysel_get_selection(ttysw, &holder);
    }
}



/*
 * Reply:  respond to a request sent from another process
 */
static          Seln_result
ttysel_reply(request, context, buffer_length)
    Seln_attribute  request;
    register
    Seln_replier_data *context;
    int             buffer_length;
{
    unsigned        count;
    struct ttysubwindow *ttysw;
    struct ttyselection *ttysel;

    ttysw = (struct ttysubwindow *) context->client_data;
    ttysel = ttysel_from_rank(ttysw, context->rank);
#ifndef OW_I18N		/* ??? */
    if (!ttysel->sel_made) {
	return SELN_DIDNT_HAVE;
    }
#endif
    switch (request) {
      case SELN_REQ_BYTESIZE:
        if (buffer_length < sizeof(CHAR **)) {
	    return SELN_FAILED;
	}
	count = 0;
	if (!ttysel->sel_null) {
#ifdef  OW_I18N
            ttyenumerateselection(ttysel, ttycountbytes, (char *) (&count));
#else
	    ttyenumerateselection(ttysel, ttycountchars, (char *) (&count));
#endif
	}
	*context->response_pointer++ = (char *) count;
	return SELN_SUCCESS;
#ifdef  OW_I18N
      case SELN_REQ_CHARSIZE:
        if (buffer_length < sizeof(CHAR **)) {
            return SELN_FAILED;
        }
        count = 0;
        if (!ttysel->sel_null) {
            ttyenumerateselection(ttysel, ttycountchars, (char *) (&count));
        }
        *context->response_pointer++ = (char *) count;
        return SELN_SUCCESS;
      case SELN_REQ_CONTENTS_WCS:
        if (ttysel->sel_null) {
            *context->response_pointer++ = (char *) 0;
                return SELN_SUCCESS;
        }
        return ttysel_copy_out_wchar(ttysel, context, buffer_length);
#endif
      case SELN_REQ_CONTENTS_ASCII:
	if (ttysel->sel_null) {
	    *context->response_pointer++ = (char *) 0;
	    return SELN_SUCCESS;
	}
	return ttysel_copy_out(ttysel, context, buffer_length);
      case SELN_REQ_YIELD:
	if (buffer_length < sizeof(char **)) {
	    return SELN_FAILED;
	}
	if (!ttysel->sel_made) {
	    *context->response_pointer++ = (char *) SELN_DIDNT_HAVE;
	} else {
	    (void) ttysel_cancel(ttysw, context->rank);
	    *context->response_pointer++ = (char *) SELN_SUCCESS;
	}
	return SELN_SUCCESS;
#ifdef  OW_I18N
      case SELN_REQ_END_REQUEST:
        return SELN_FAILED;
#endif
      default:
	return SELN_UNRECOGNIZED;
    }
}

/*
 * Send_deselect: tell another process to deselect
 */
static void
ttysel_end_request(ttysw, addressee, rank)
    struct ttysubwindow *ttysw;
    Seln_holder    *addressee;
    Seln_rank       rank;
{
    Seln_request    buffer;

    if (seln_holder_same_client(addressee, (char *) ttysw)) {
	(void) ttysel_cancel(ttysw, rank);
	return;
    }
    seln_init_request(&buffer, addressee, SELN_REQ_COMMIT_PENDING_DELETE,
		      SELN_REQ_YIELD, 0, NULL);
    (void) seln_request(addressee, &buffer);
}

#ifdef  OW_I18N
static          Seln_result
ttysel_copy_out(ttysel, context, max_length)
    register struct ttyselection *ttysel;
    register Seln_replier_data *context;
    register int    max_length;
{
    register int    i, curr_col, curr_row, row_end,char_len;
    register CHAR   *src;
    register char   *dest_mbs;
    char            tmp_mbs[10];
    int             start_col, end_col, last_row , row_col_len;
    int             cwidth, offset;

    if (context->context != (char *) NULL) {
        ttysel = (struct ttyselection *) context->context;
    }

    start_col = ttysel->sel_begin.tsp_col;
    end_col = ttysel->sel_end.tsp_col;
    last_row = ttysel->sel_end.tsp_row;

    dest_mbs = (char *) context->response_pointer;

    curr_col = start_col;
    curr_row = ttysel->sel_begin.tsp_row;

    while (curr_row < last_row) {
        /*
         *      We cannot foresee required buf length. So we need to keep
         *      track of each charactsr.
         */
        row_end = (int)LINE_LENGTH(image[curr_row]);
        src = image[curr_row] + curr_col;
        for( i = curr_col; i < row_end ; i++ ) {
                if( *src == TTY_NON_WCHAR ){
                        src++;
                        continue;
                }
                char_len = wctomb( tmp_mbs , *src++ );
                if( char_len == 0 )
                        char_len = 1;
                if( ( max_length -= char_len ) < 0 ) {
                        row_col_len = i - curr_col;
                        goto continue_reply;
                }
                XV_BCOPY( tmp_mbs, dest_mbs, char_len );
                dest_mbs += char_len;
        }
        row_col_len = LINE_LENGTH( image[curr_row] ) - curr_col;
        if (row_col_len + curr_col != ttysw_right) {
            *dest_mbs++ = '\n';
            max_length--;
        }
        curr_col = 0;
        curr_row += 1;
    }
    row_end = end_col + 1;
    src = image[curr_row] + curr_col;
    for( i = curr_col; i < row_end ; i++ ) {
        if( *src == TTY_NON_WCHAR ){
                src++;
                continue;
        }
        char_len = wctomb( tmp_mbs , *src++ );
        if( char_len == 0 )
                char_len = 1;
        if( ( max_length -= char_len ) < 0 ) {
                row_col_len = i - curr_col;
                goto continue_reply;
        }
        XV_BCOPY( tmp_mbs, dest_mbs, char_len );
        dest_mbs += char_len;
    }
    if (end_col == LINE_LENGTH(image[curr_row]) && end_col < ttysw_right) {
        dest_mbs[-1] = '\n';
        *dest_mbs = '\0';
        max_length--;
    }

    /* round up to word boundary  */
    while ((unsigned) dest_mbs % 4 != 0) {
        *dest_mbs++ = '\0';
    }

    context->response_pointer = (char **) dest_mbs;
    *context->response_pointer++ = 0;
    if (context->context != (char *) NULL) {
        free(context->context);
    }

    return SELN_SUCCESS;

continue_reply:
    {
        register struct ttyselection *ttysel2;

        if (context->context == (char *) NULL) {
            ttysel2 =
                (struct ttyselection *)
                malloc(sizeof(struct ttyselection));
            if (ttysel2 == (struct ttyselection *) NULL) {
                xv_error((Xv_opaque)ttysel,
                         ERROR_LAYER, ERROR_SYSTEM,
                         ERROR_STRING,
                         XV_MSG("failed for selection copy-out"),
                         ERROR_PKG, TTY,
                         NULL);

                return SELN_FAILED;
            }
            *ttysel2 = *ttysel;
        } else {
            ttysel2 = (struct ttyselection *) context->context;
        }
        ttysel2->sel_begin.tsp_row = curr_row;
        ttysel2->sel_begin.tsp_col = curr_col + row_col_len;
        ttysel2->sel_end.tsp_row = last_row;
        ttysel2->sel_end.tsp_col = end_col;
        context->context = (char *) ttysel2;
        context->response_pointer = (char **) dest_mbs;
        return SELN_CONTINUED;
    }
}

static          Seln_result
ttysel_copy_out_wchar(ttysel, context, max_length)
    register struct ttyselection *ttysel;
    register Seln_replier_data *context;
    register int    max_length;
{
    register int    curr_row, index, row_len;
    register CHAR  *dest, *src;
    register int    row_col_len;
    int             last_row;
    int             curr_col,start_col,end_col;

    if (context->context != (char *) NULL) {
        ttysel = (struct ttyselection *) context->context;
    }
    start_col  =ttysel->sel_begin.tsp_col;
    last_row = ttysel->sel_end.tsp_row;
    end_col = ttysel->sel_end.tsp_col;
    dest = (CHAR *) context->response_pointer;

    curr_col = start_col;
    curr_row = ttysel->sel_begin.tsp_row;
    while (curr_row < last_row) {
        row_len = MIN( tty_get_nchars(curr_col, TTY_LINE_INF_INDEX, curr_row) ,
                                      max_length/sizeof(CHAR));
        row_col_len = 0;
        index = row_len;
        src = image[curr_row] + curr_col;
        while (index) {
            row_col_len ++;
            if( *src == TTY_NON_WCHAR ) {
                src++;
                continue;
            }
            *dest++ = *src++;
            index--;
        }
        if ((max_length -= row_len * sizeof(CHAR)) <= 0) {
            goto continue_reply;
        }
        if (row_col_len + curr_col != ttysw_right) {
            *dest++ = (CHAR)'\n';
            max_length -= sizeof(CHAR);
        }
        curr_col = 0;
        curr_row += 1;
    }
    row_len = MIN( tty_get_nchars(curr_col, end_col, curr_row ),
                max_length/sizeof(CHAR));
    index = row_len;
    row_col_len = 0;
    src = image[curr_row] + curr_col;
    while (index) {
        row_col_len++;
        if( *src == TTY_NON_WCHAR ) {
                src++;
                continue;
        }
        *dest++ = *src++;
        index--;
    }
    if ((max_length -= row_len*sizeof(CHAR)) <= 0) {
        goto continue_reply;
    }
    if (end_col == LINE_LENGTH(image[curr_row]) && end_col < ttysw_right) {
        dest[-1] = (CHAR)'\n';
        *dest = (CHAR)'\0';
        max_length -= sizeof(CHAR);
    }
    /* round up to word boundary  */
    while ((unsigned) dest % 4 != 0) {
        *dest++ = (CHAR)'\0';
    }

    context->response_pointer = (char **) dest;
    *context->response_pointer++ = 0;
    if (context->context != (char *) NULL) {
        free(context->context);
    }
    return SELN_SUCCESS;

continue_reply:
    {
        register struct ttyselection *ttysel2;

        if (context->context == (char *) NULL) {
            ttysel2 =
                (struct ttyselection *)
                malloc(sizeof(struct ttyselection));
            if (ttysel2 == (struct ttyselection *) NULL) {
                xv_error((Xv_opaque)ttysel,
                         ERROR_LAYER, ERROR_SYSTEM,
                         ERROR_STRING,
                         "failed for selection copy-out",
                         ERROR_PKG, TTY,
                         NULL);
                return SELN_FAILED;
            }
            *ttysel2 = *ttysel;
        } else {
            ttysel2 = (struct ttyselection *) context->context;
        }
        ttysel2->sel_begin.tsp_row = curr_row;
        ttysel2->sel_begin.tsp_col = curr_col + row_col_len;
        ttysel2->sel_end.tsp_row = last_row;
        ttysel2->sel_end.tsp_col = end_col;
        context->context = (char *) ttysel2;
        context->response_pointer = (char **) dest;
        return SELN_CONTINUED;
    }
}

#else   /*OW_I18N*/
static          Seln_result
ttysel_copy_out(ttysel, context, max_length)
    register struct ttyselection *ttysel;
    register Seln_replier_data *context;
    register int    max_length;
{
    register int    curr_col, curr_row, index, row_len;
    register char  *dest, *src;
    int             start_col, end_col, last_row;

    if (context->context != (char *) NULL) {
	ttysel = (struct ttyselection *) context->context;
    }
    start_col = ttysel->sel_begin.tsp_col;
    end_col = ttysel->sel_end.tsp_col;
    last_row = ttysel->sel_end.tsp_row;
    dest = (char *) context->response_pointer;

    curr_col = start_col;
    curr_row = ttysel->sel_begin.tsp_row;
    while (curr_row < last_row) {
	row_len = MIN((int)LINE_LENGTH(image[curr_row]) - curr_col, max_length);
	index = row_len;
	src = image[curr_row] + curr_col;
	while (index--) {
	    *dest++ = *src++;
	}
	if ((max_length -= row_len) <= 0) {
	    goto continue_reply;
	}
	if (row_len + curr_col != ttysw_right) {
	    *dest++ = '\n';
	    max_length--;
	}
	curr_col = 0;
	curr_row += 1;
    }
    row_len = MIN(end_col + 1 - curr_col, max_length);
    index = row_len;
    src = image[curr_row] + curr_col;
    while (index--) {
	*dest++ = *src++;
    }
    if ((max_length -= row_len) <= 0) {
	goto continue_reply;
    }
    if (end_col == LINE_LENGTH(image[curr_row]) && end_col < ttysw_right) {
	dest[-1] = '\n';
	*dest = '\0';
	max_length--;
    }
    /* round up to word boundary  */
    while ((unsigned) dest % 4 != 0) {
	*dest++ = '\0';
    }

    context->response_pointer = (char **) dest;
    *context->response_pointer++ = 0;
    if (context->context != (char *) NULL) {
	free(context->context);
    }
    return SELN_SUCCESS;

continue_reply:
    {
	register struct ttyselection *ttysel2;

	if (context->context == (char *) NULL) {
	    ttysel2 =
		(struct ttyselection *)
		malloc(sizeof(struct ttyselection));
	    if (ttysel2 == (struct ttyselection *) NULL) {
		xv_error((Xv_opaque)ttysel,
			 ERROR_LAYER, ERROR_SYSTEM,
			 ERROR_STRING, 
			 XV_MSG("failed for selection copy-out"),
			 ERROR_PKG, TTY,
			 NULL);
		return SELN_FAILED;
	    }
	    *ttysel2 = *ttysel;
	} else {
	    ttysel2 = (struct ttyselection *) context->context;
	}
	ttysel2->sel_begin.tsp_row = curr_row;
	ttysel2->sel_begin.tsp_col = curr_col + row_len;
	ttysel2->sel_end.tsp_row = last_row;
	ttysel2->sel_end.tsp_col = end_col;
	context->context = (char *) ttysel2;
	context->response_pointer = (char **) dest;
	return SELN_CONTINUED;
    }
}
#endif

#ifdef  OW_I18N
static          Seln_result
ttysel_copy_in(buffer)
    register Seln_request *buffer;
{
    register Attr_attribute *response_ptr;
    register struct ttysel_context *context;
    Ttysw_folio     ttysw;
    register int    current_size;
    register Attr_attribute *attr;
    int             true_len;   
    int             result=SELN_FAILED;

    if (buffer == (Seln_request *) NULL) {
        return SELN_UNRECOGNIZED;
    }
    context = (struct ttysel_context *) buffer->requester.context;
    ttysw = context->ttysw;
    response_ptr = (Attr_attribute *) buffer->data;
    if (!context->continued) {
        for (attr = (Attr_attribute *)response_ptr; *attr;
                attr = attr_next(attr)) {
           switch ((Seln_attribute) (*response_ptr++)) {
                case SELN_REQ_BYTESIZE:
                   context->bytes_left = (int) *response_ptr++;
                   current_size = MIN(context->bytes_left,
                        buffer->buf_size - 3 * sizeof(Seln_attribute));
                   context->request_is_byte = TRUE;
		   result = SELN_SUCCESS;
                   break;
                case SELN_REQ_CHARSIZE:
                   context->bytes_left = (int) *response_ptr++;
                   current_size = MIN(context->bytes_left,
                        (buffer->buf_size - 3 * sizeof(Seln_attribute))/sizeof(CHAR));
                   context->request_is_byte = FALSE;
		   result = SELN_SUCCESS;
                   break;
                case SELN_REQ_CONTENTS_ASCII:
                   context->request_is_ascii = TRUE;
		   result = SELN_SUCCESS;
                   break;
                case SELN_REQ_CONTENTS_WCS:
                   context->request_is_ascii = FALSE;
		   result = SELN_SUCCESS;
                   break;
                default:
		   /* result is initialized to SELN_FAILED, so don't have
		      to set it */
		   break;
            }
        }
	/* if all the attributes failed, then result will be
	   SELN_FAILED, so return. If at least one of the attributes
	   succeeded, then we can continue on. */
	if (result==SELN_FAILED) {
	    return result;
	}
    }
    else
        current_size = MIN(context->bytes_left, buffer->buf_size/sizeof(CHAR));

    /* Now the response_ptr either points to NULL, or points to value
       part of attribute SELN_REQ_CONTENTS_ASCII or SELN_REQ_CONTENTS_WCS */
   /*
    *  Ttysw is confused if null characters are padded at the
    *  end of the selected string. So we need to pass the exact length
    *  of the string to ttwsw.
    */
    if( context->request_is_ascii ){
       true_len = strlen( (char *)response_ptr );
       (void) ttysw_input_it(ttysw, (char *) response_ptr, true_len);
    }
    else {
        true_len = wslen( (CHAR *) response_ptr );
       (void) ttysw_input_it_wcs(ttysw, (CHAR *) response_ptr, true_len);
    }
    ttysw_reset_conditions(TTY_VIEW_HANDLE_FROM_TTY_FOLIO(ttysw));
    if (buffer->status == SELN_CONTINUED) {
        context->continued = TRUE;
        context->bytes_left -= current_size;
    }
    return SELN_SUCCESS;
}

#else  /* OW_I18N */

static          Seln_result
ttysel_copy_in(buffer)
    register Seln_request *buffer;
{
    register Attr_attribute *response_ptr;
    register struct ttysel_context *context;
    Ttysw_folio     ttysw;
    register int    current_size;

    if (buffer == (Seln_request *) NULL) {
	return SELN_UNRECOGNIZED;
    }
    context = (struct ttysel_context *) buffer->requester.context;
    ttysw = context->ttysw;
    response_ptr = (Attr_attribute *) buffer->data;
    if (!context->continued) {
	if (*response_ptr++ != SELN_REQ_BYTESIZE) {
	    return SELN_FAILED;
	}
	context->bytes_left = (int) *response_ptr++;
       /*
        * The next field of response_ptr is the string itself, so we
        * have to subtract 4 bytes (the size of the current field) to
        * get the actual length of the string.
        */
        current_size = MIN(context->bytes_left,
                           strlen((char *) response_ptr) - 4);
	if (*response_ptr++ != SELN_REQ_CONTENTS_ASCII) {
	    return SELN_FAILED;
	}
    } else {
	current_size = MIN(context->bytes_left, buffer->buf_size);
    }
    (void) ttysw_input_it(ttysw, (char *) response_ptr, current_size);
    ttysw_reset_conditions(TTY_VIEW_HANDLE_FROM_TTY_FOLIO(ttysw));
    if (buffer->status == SELN_CONTINUED) {
	context->continued = TRUE;
	context->bytes_left -= current_size;
    }
    return SELN_SUCCESS;
}
#endif  /* OW_I18N */

static
  ttysel_resynch(register struct ttysubwindow *ttysw,
                 register Seln_function_buffer *buffer)
{
    if (ttysw->ttysw_caret.sel_made &&
	!seln_holder_same_client(&buffer->caret, (char *) ttysw)) {
	ttysel_deselect(&ttysw->ttysw_caret, SELN_CARET);
	ttysw->ttysw_caret.sel_made = FALSE;
    }
    if (ttysw->ttysw_primary.sel_made &&
	!seln_holder_same_client(&buffer->primary, (char *) ttysw)) {
	ttysel_deselect(&ttysw->ttysw_primary, SELN_PRIMARY);
	ttysw->ttysw_primary.sel_made = FALSE;
    }
    if (ttysw->ttysw_secondary.sel_made &&
	!seln_holder_same_client(&buffer->secondary, (char *) ttysw)) {
	ttysel_deselect(&ttysw->ttysw_secondary, SELN_SECONDARY);
	ttysw->ttysw_secondary.sel_made = FALSE;
    }
    if (ttysw->ttysw_shelf.sel_made &&
	!seln_holder_same_client(&buffer->shelf, (char *) ttysw)) {
	ttysel_deselect(&ttysw->ttysw_shelf, SELN_SHELF);
	ttysw->ttysw_shelf.sel_made = FALSE;
    }
}
