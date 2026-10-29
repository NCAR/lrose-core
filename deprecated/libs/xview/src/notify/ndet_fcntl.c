#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndet_fcntl.c 20.13 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_fcntl.c - Notifier's version of fcntl.  Used to detect async mode of
 * fds managing.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <signal.h>
#include <fcntl.h>
#ifdef SVR4
#include <sys/file.h>
#endif /* SVR4 */
#if defined(__linux) || defined(__APPLE__)
#include <stdarg.h>
#endif

int
#if !defined(__linux) && !defined(__APPLE__)
#ifdef SVR4
xv_fcntl(fd, cmd, arg)
#else
fcntl(fd, cmd, arg)
#endif SVR4
    int             fd, cmd, arg;
{
    fd_set          bit;
    int             res;
#else /* __linux */
/* fcntl() is declared using variable args in linux */
fcntl(int fd, int cmd, ...) {
    fd_set          bit;
    int             res;
    va_list         args;
    int             arg;

    va_start(args, cmd);
    arg = va_arg(args, int);
    va_end(args);
#endif /* __linux */

    /* Set fd bit */
    FD_ZERO(&bit);
    FD_SET(fd, &bit);
    /* If call fails then ignore transition */
    if ((res = notify_fcntl(fd, cmd, arg)) == -1)
	return (res);
    /*
     * Update non-blocking read and async data ready flags if setting or
     * querying.  Doing it on querying double checks information.
     */
    /*
     * System V has its own distinct style of no-delay i/o requested by FNBIO
     * in addition to the 4.2 FNDELAY.  This code segment recognizes both and
     * sets the ndet_fndelay_mask.  At this time (87/2/9) it has not been
     * verified that this is sufficient--perhaps more discrimination is
     * necessary.  The #ifdef is needed because the FNBIO symbol has not yet
     * arrived in our include files from argon.  The #ifdef can go away when
     * we get the new include files.
     */
    if (cmd == F_SETFL || cmd == F_GETFL) {
	/* For F_GETFL, res contains flags */
	if (cmd == F_GETFL)
	    arg = res;
	NTFY_BEGIN_CRITICAL;
#if (defined(__linux) || defined(__APPLE__)) && !defined(FNDELAY)
	if (arg & O_NONBLOCK)
	    FD_SET(fd, &ndet_fndelay_mask);
#else
	if (arg & FNDELAY)
	    FD_SET(fd, &ndet_fndelay_mask);
#endif
#ifdef FNBIO
	else if (arg & FNBIO)
	    FD_SET(fd, &ndet_fndelay_mask);
#endif
	else
	    FD_CLR(fd, &ndet_fndelay_mask);
#if (!defined(__linux) && !defined(__APPLE__)) || defined(FASYNC)
	if (arg & FASYNC)
	    FD_SET(fd, &ndet_fndelay_mask);
	else
	    FD_CLR(fd, &ndet_fndelay_mask);
#endif
	/* Make sure that are catching async related signals now */
	if (ntfy_fd_anyset(&ndet_fasync_mask)) {
	    ndet_enable_sig(SIGIO);
#ifndef __linux
	    ndet_enable_sig(SIGURG);
#endif
	}
	/*
	 * Setting NDET_FD_CHANGE will "fix up" signals being caught to be
	 * the minimum required next time around the notification loop.
	 */
	ndet_flags |= NDET_FD_CHANGE;
	NTFY_END_CRITICAL;
    }
    return (res);
}
