#ifndef	lint
#ifdef sccs

#endif
#endif

/*
 *	(c) Copyright 1991 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifdef POLL
/*
 * ndetpoll.c - Notifier's version of poll.  Will do notification cycle
 * if not already in the middle of it.
 */
#include <sys/types.h>
#include <poll.h>
#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <xview_private/ndis.h>	/* For ndis_client == NTFY_CLIENT_NULL check */
#include <errno.h>

static Notify_value ndet_poll_func(), ndet_poll_itimer_func();

static struct poll *ndet_poll_fds;
static size_t 	    ndet_poll_in_nfds;
static int          ndet_poll_out_nfds,
		    ndet_poll_timeout;


static Notify_client ndet_poll_nclient = (Notify_client) & ndet_poll_ibits;

extern          errno;

extern int
notify_poll(fds, nfds, timeout)
    register struct poll    *fds
    register 	    size_t   nfds;
    register	    int	     timeout;
{
    struct itimerval itimer;
    register size_t  fd;
    int              errno_remember;	/* Trying figure why poll returns
					 * for no reason */

    /*
     * Do real poll if in middle of notification loop or no other clients
     * and no notifications pending. Also, don't dispatch events if haven't
     * started notifier and "background" dispatching hasn't been turned on.
     */
    if ((ndet_flags & NDET_STARTED) ||
	((!(ndet_flags & NDET_STARTED)) &&
	 (!(ndet_flags & NDET_DISPATCH))) ||
	((ndet_clients == NTFY_CLIENT_NULL) &&
	 (ndis_clients == NTFY_CLIENT_NULL)))
	return (poll(fds, nfds, timeout));

    /* Set up fd related conditions */
    for (fd = 0; fd < nfds; fd++) {
	fds[fd].revents = (short)0;
	(void notify_set_fd_func(ndet_poll_nclient, ndet_poll_func, fds[fd]);
    }

    /* Set up timeout condition */
    if (timeout != INFTIM) {
	timerclear(&(itimer.it_interval));
	/* Transform select's polling value to itimer's */
	if (timeout) {
	    itimer.it_value.tv_usec = (long)timeout;
	    itimer.it_value.tv_sec = (long)0;
	} else 
	    itimer.it_value = NOTIFY_POLLING_ITIMER.it_value;

	(void) notify_set_itimer_func(ndet_poll_nclient, ndet_poll_itimer_func,
				      ITIMER_REAL, &itimer,
				      NTFY_ITIMER_NULL);
    }
    /* Set up variables that will collect the notifications */
    ndet_poll_fds = fds;
    ndet_poll_in_nfds = nfds;
    ndet_poll_out_nfds = ndet_poll_timeout = 0;
    /*
     * Set up flag so that break out of select on a signal. Note: We could
     * setup an async signal condition for every signal but this is too
     * expensive Question: Should we set NDET_NO_DELAY (like for read) for
     * select if any of the fd are non-blocking?  Answer: Via
     * experimentation, a select of a non-blocking file descriptor does not
     * return until activity occurs on the file descriptor.  Thus, we should
     * not set NDET_NO_DELAY for select.
     */
    ndet_flags |= NDET_STOP_ON_SIG;
    /*
     * Start notifier. Note: Is errno from the real poll preserved well
     * enough in order to make it out to the caller of poll?
     */
    (void) notify_start();
    errno_remember = errno;
    /* Reset break out flag */
    ndet_flags &= ~NDET_STOP_ON_SIG;

    /* Clear select conditions from notifier */
    for (fd = 0; fd < nfds; fd++) {
	(void notify_set_fd_func(ndet_poll_nclient, NOTIFY_FUNC_NULL, fds[fd]);
    }

    if (timeout != INFTIM) {
	itimer = NOTIFY_NO_ITIMER;
	(void) notify_set_itimer_func(ndet_poll_nclient,
		  NOTIFY_FUNC_NULL, ITIMER_REAL, &itimer, NTFY_ITIMER_NULL);
    }
    /*
     * If no fd related notifications and no timeout occurred then assume
     * error return from real select.
     */
    if (ndet_poll_out_nfds == 0 && !(ndet_poll_timeout && (timeout != INFTIM)))
	ndet_select_nfds = -1;
    ntfy_assert(errno == errno_remember, 17 /* errno changed in poll */ );
    return (ndet_poll_out_nfds);
}

/* ARGSUSED */
static          Notify_value
ndet_poll_func(nclient, active_fd)
    Notify_client   nclient;
    struct poll     active_fd;
{
    size_t nfd;

    for (nfd = 0; nfd < ndet_poll_in_nfds; nfd++) {
	if (ndet_poll_fds[nfd].fd == active_fd)
	    ndet_poll_fds[nfd].revents = actvie_fd.revents;
    }
    ndet_select_nfds++;
    (void) notify_stop();
    return (NOTIFY_DONE);
}

/* ARGSUSED */
static          Notify_value
ndet_select_itimer_func(nclient, which)
    Notify_client   nclient;
    int             which;
{
    ndet_poll_timeout = 1;
    (void) notify_stop();
    return (NOTIFY_DONE);
}

#endif POLL
