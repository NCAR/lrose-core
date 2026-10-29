#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndis_d_pri.c 20.16 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndis_d_pri.c - Default prioritizer for dispatcher.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndis.h>
#include <signal.h>
#ifdef __linux
#include <sys/param.h> /* for howmany(), NBBY */
#ifndef NBBY
#define NBBY 8
#endif
#endif

typedef enum notify_error (*Notify_error_func) ();

static void     ndis_send_ascending_sig();
static void     ndis_send_ascending_fd();

pkg_private     Notify_value
ndis_default_prioritizer(nclient, nfd, ibits_ptr, obits_ptr, ebits_ptr,
	 nsig, sigbits_ptr, auto_sigbits_ptr, event_count_ptr, events, args)
    register Notify_client nclient;
    fd_set         *ibits_ptr, *obits_ptr, *ebits_ptr;
    int             nsig, *event_count_ptr;
    register sigset_t   *sigbits_ptr, *auto_sigbits_ptr;
    register int   nfd;
    Notify_event   *events;
    Notify_arg     *args;
{
    register int    i;

    if (!sigisempty( auto_sigbits_ptr )) {
	/* Send itimers */
	if (sigismember( auto_sigbits_ptr, SIGALRM )) {
	    (void) notify_itimer(nclient, ITIMER_REAL);
	    sigdelset( auto_sigbits_ptr, SIGALRM );
	}
	if (sigismember( auto_sigbits_ptr, SIGVTALRM )) {
	    (void) notify_itimer(nclient, ITIMER_VIRTUAL);
	    sigdelset( auto_sigbits_ptr, SIGVTALRM );
	}
	/* Send child process change */
	if (sigismember( auto_sigbits_ptr, SIGCHLD )) {
	    (void) notify_wait3(nclient);
	    sigdelset( auto_sigbits_ptr, SIGCHLD );
	}
    }
    if (!sigisempty(sigbits_ptr))
	/* Send signals (by ascending signal numbers) */
	ndis_send_ascending_sig(nclient, nsig, sigbits_ptr,
				notify_signal);
    if (ntfy_fd_anyset(ebits_ptr))
	/* Send exception fd activity (by ascending fd numbers) */
	ndis_send_ascending_fd(nclient, nfd, ebits_ptr,
			       notify_exception);
    /* Send client event (in order received) */
    for (i = 0; i < *event_count_ptr; i++)
	(void) notify_event(nclient, *(events + i), *(args + i));
    *event_count_ptr = 0;
    if (ntfy_fd_anyset(obits_ptr))
	/* Send output fd activity (by ascending fd numbers) */
	ndis_send_ascending_fd(nclient, nfd, obits_ptr, notify_output);
    if (ntfy_fd_anyset(ibits_ptr))
	/* Send input fd activity (by ascending fd numbers) */
	ndis_send_ascending_fd(nclient, nfd, ibits_ptr, notify_input);

    if (!sigisempty( auto_sigbits_ptr )) {
	/* Send destroy checking */
	if (sigismember( auto_sigbits_ptr, SIGTSTP )) {
	    if ((notify_destroy(nclient, DESTROY_CHECKING) ==
		 NOTIFY_DESTROY_VETOED) &&
		((sigismember( auto_sigbits_ptr, SIGTERM )) ||
		 (sigismember( auto_sigbits_ptr, SIGKILL )))) {
		/* Remove DESTROY_CLEANUP from dispatch list. */
		notify_flush_pending(nclient);
		/* Prevent DESTROY_CLEANUP in this call */
	        sigdelset( auto_sigbits_ptr, SIGTERM );
	        sigdelset( auto_sigbits_ptr, SIGKILL );
	    }
	    sigdelset( auto_sigbits_ptr, SIGTSTP );
	}
	/* Send destroy (only one of them) */
	if (sigismember( auto_sigbits_ptr, SIGTERM )) {
	    (void) notify_destroy(nclient, DESTROY_CLEANUP);
	    sigdelset( auto_sigbits_ptr, SIGTERM );
	} else if (sigismember( auto_sigbits_ptr, SIGKILL )) {
	    (void) notify_destroy(nclient, DESTROY_PROCESS_DEATH);
	    sigdelset( auto_sigbits_ptr, SIGKILL );
	} else if (sigismember( auto_sigbits_ptr, SIGUSR1 )) {
	    (void) notify_destroy(nclient, DESTROY_SAVE_YOURSELF);
	    sigdelset( auto_sigbits_ptr, SIGUSR1 );
	}
    }
    return (NOTIFY_DONE);
}

/* RedHat does not seem to find the fds_bits in fd_set - so define it */
#if defined(__linux)
#define fds_bits __fds_bits
#endif

static void
ndis_send_ascending_fd(nclient, nbits, bits_ptr, func)
    Notify_client   nclient;
    register int    nbits;
    fd_set         *bits_ptr;
    Notify_error_func func;
{
    register        fd, i, byteNum;
    unsigned long   byte;

    /* Send fd (by ascending numbers) */
    for (i = 0; i < howmany(nbits, NFDBITS); i++)
	if (bits_ptr->fds_bits[i])
	    /* For each fd_mask set in bits_ptr, mask off all but  */
	    /* one byte and see if anything is set.                */
	    for (byte = 0xff, byteNum = 0; byte != 0L;
		 byte <<= NBBY, byteNum++)
		if (bits_ptr->fds_bits[i] & byte)
		    /* If a byte is set, find out which bit is set.*/
		    for (fd = byteNum * NBBY + i * NFDBITS;
			 fd < byteNum * NBBY + i * NFDBITS + NBBY; fd++)
			if (FD_ISSET(fd, bits_ptr)) {
			    (void) func(nclient, fd);
			    FD_CLR(fd, bits_ptr);
			}
}

static void
ndis_send_ascending_sig(nclient, nbits, bits_ptr, func)
    Notify_client   nclient;
    register int    nbits;
    register sigset_t    *bits_ptr;
    Notify_error_func func;
{
    register        sig;

    /* Send func (by ascending numbers) */
    for (sig = 1; sig < nbits; sig++) {
	if (sigismember( bits_ptr, sig )) {
	    (void) func(nclient, sig);
	    sigdelset( bits_ptr, sig );
	}
    }
}
