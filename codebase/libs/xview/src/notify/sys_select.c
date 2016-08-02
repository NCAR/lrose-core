#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)sys_select.c 20.17 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Sys_select.c - Real system call to select.
 */

#ifndef SVR4
#if !defined(__linux) && !defined(__APPLE__)
#include <syscall.h>
#else
#include "linux_select.h"
#endif
#else /* SVR4 */
#include <values.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/poll.h>
#include <sys/select.h>
#endif /* SVR4 */
#include <xview_private/ntfy.h>	/* For ntfy_assert */
#include <errno.h>		/* For debugging */
#include <stdio.h>		/* For debugging */

extern          int errno; /* __APPLE__ port */

#ifndef NULL
#define NULL	0
#endif 

pkg_private int
#ifndef SVR4
notify_select(nfds, readfds, writefds, exceptfds, tv)
#else /* SVR4 */
notify_select(nfds, in0, out0, ex0, tv)
#endif /* SVR4 */
#ifndef SVR4
#if !defined(__linux) && !defined(__APPLE__)
    int             nfds, *readfds, *writefds, *exceptfds;
#else
    int             nfds;
    fd_set *readfds, *writefds, *exceptfds;
#endif /* __linux */
#else /* SVR4 */
    int nfds;
    fd_set *in0, *out0, *ex0;
#endif /* SVR4 */
    struct timeval *tv;
{

/* RedHat does not seem to find the fds_bits in fd_set - so define it */
#if defined(__linux)
#define fds_bits __fds_bits
#endif

#ifndef SVR4
#if !defined(__linux) && !defined(__APPLE__)
    nfds = syscall(SYS_select, nfds, readfds, writefds, exceptfds, tv);
    ntfy_assert(!(nfds == 0 && tv == (struct timeval *) 0 &&
		  *readfds == 0 && *writefds == 0 && *exceptfds == 0), 39
		/* SYS_select returned when no stimuli */);
#else
    nfds = linux_select(nfds, readfds, writefds, exceptfds, tv);
    ntfy_assert(!(nfds == 0 && tv == (struct timeval *) 0 &&
		readfds->fds_bits[0] == 0 && writefds->fds_bits[0] == 0 &&
		exceptfds->fds_bits[0] == 0), 39
		/* SYS_select returned when no stimuli */);
#endif
    return (nfds);
#else /* SVR4 */
    /* register declarations ordered by expected frequency of use */
    register long *in, *out, *ex;
    register u_long m;	/* bit mask */
    register int j;		/* loop counter */
    register u_long b;	/* bits to test */
    register int n, rv, ms;
    struct pollfd pfd[FD_SETSIZE];
    register struct pollfd *p = pfd;
    int lastj = -1;
    /* "zero" is read-only, it could go in the text segment */
    static fd_set zero = { 0 };

    /*
     * If any input args are null, point them at the null array.
     */
    if (in0 == NULL)
	    in0 = &zero;
    if (out0 == NULL)
	    out0 = &zero;
    if (ex0 == NULL)
	    ex0 = &zero;

    /*
     * For each fd, if any bits are set convert them into
     * the appropriate pollfd struct.
     */
    in = (long *)in0->fds_bits;
    out = (long *)out0->fds_bits;
    ex = (long *)ex0->fds_bits;
    for (n = 0; n < nfds; n += NFDBITS) {
	    b = (u_long)(*in | *out | *ex);
	    for (j = 0, m = 1; b != 0; j++, b >>= 1, m <<= 1) {
		    if (b & 1) {
			    p->fd = n + j;
			    if (p->fd >= nfds)
				    goto done;
			    p->events = 0;
			    if (*in & m)
				    p->events |= POLLIN;
			    if (*out & m)
				    p->events |= POLLOUT;
			    if (*ex & m)
				    p->events |= POLLRDBAND;
			    p++;
		    }
	    }
	    in++;
	    out++;
	    ex++;
    }
done:
    /*
     * Convert timeval to a number of millseconds.
     * Test for zero cases to avoid doing arithmetic.
     * XXX - this is very complex, is it worth it?
     */
    if (tv == NULL) {
	    ms = -1;
    } else if (tv->tv_sec == 0) {
	    if (tv->tv_usec == 0) {
		    ms = 0;
	    } else if (tv->tv_usec < 0 || tv->tv_usec > 1000000) {
		    errno = EINVAL;
		    return (-1);
	    } else {
		    /*
		     * lint complains about losing accuracy,
		     * but I know otherwise.  Unfortunately,
		     * I can't make lint shut up about this.
		     */
		    ms = (int)(tv->tv_usec / 1000);
	    }
    } else if (tv->tv_sec > (MAXINT) / 1000) {
	    if (tv->tv_sec > 100000000) {
		    errno = EINVAL;
		    return (-1);
	    } else {
		    ms = MAXINT;
	    }
    } else if (tv->tv_sec > 0) {
	    /*
	     * lint complains about losing accuracy,
	     * but I know otherwise.  Unfortunately,
	     * I can't make lint shut up about this.
	     */
	    ms = (int)((tv->tv_sec * 1000) + (tv->tv_usec / 1000));
    } else {	/* tv_sec < 0 */
	    errno = EINVAL;
	    return (-1);
    }

    /*
     * Now do the poll.
     */
    n = p - pfd;		/* number of pollfd's */
    rv = syscall (SYS_poll, pfd, (u_long)n, ms);
    if (rv < 0)		/* let 0 drop through so fs_set's get 0'ed */
	    return (rv);

    /*
     * Convert results of poll back into bits
     * in the argument arrays.
     *
     * We assume POLLIN, POLLOUT, and POLLRDBAND will only be set
     * on return from poll if they were set on input, thus we don't
     * worry about accidentally setting the corresponding bits in the
     * zero array if the input bit masks were null.
     */
    for (p = pfd; n-- > 0; p++) {
	    j = p->fd / NFDBITS;
	    /* have we moved into another word of the bit mask yet? */
	    if (j != lastj) {
		    /* clear all output bits to start with */
		    in = (long *)&in0->fds_bits[j];
		    out = (long *)&out0->fds_bits[j];
		    ex = (long *)&ex0->fds_bits[j];
		    /*
		     * In case we made "zero" read-only (e.g., with
		     * cc -R), avoid actually storing into it.
		     */
		    if (in0 != &zero)
			    *in = 0;
		    if (out0 != &zero)
			    *out = 0;
		    if (ex0 != &zero)
			    *ex = 0;
		    lastj = j;
	    }
	    if (p->revents) {
		    /*
		     * select will return EBADF immediately if any fd's
		     * are bad.  poll will complete the poll on the
		     * rest of the fd's and include the error indication
		     * in the returned bits.  This is a rare case so we
		     * accept this difference and return the error after
		     * doing more work than select would've done.
		     */
		    if (p->revents & POLLNVAL) {
			    errno = EBADF;
			    return (-1);
		    }

		    m = 1 << (p->fd % NFDBITS);
		    if (p->revents & POLLIN)
			    *in |= m;
		    if (p->revents & POLLOUT)
			    *out |= m;
		    if (p->revents & POLLRDBAND)
			    *ex |= m;
		    /*
		     * Only set this bit on return if we asked about
		     * input conditions.
		     */
		    if ((p->revents & (POLLHUP|POLLERR)) &&
			(p->events & POLLIN))
			    *in |= m;
		    /*
		     * Only set this bit on return if we asked about
		     * output conditions.
		     */
		    if ((p->revents & (POLLHUP|POLLERR)) &&
			(p->events & POLLOUT))
			    *out |= m;
	    }
    }
    ntfy_assert(!(nfds == 0 && tv == (struct timeval *) 0), 40
	    /* select returned when no stimuli */);
    return (rv);
#endif /* SVR4 */
}
