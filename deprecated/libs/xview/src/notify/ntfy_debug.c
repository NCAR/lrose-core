#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ntfy_debug.c 20.20 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ntfy_debug.c - Debugging routines enabled by NTFY_DEBUG in ntfy.h
 */

#include <stdio.h>
#include <xview_private/i18n_impl.h>
#include <xview/xv_error.h>
#include <xview_private/ntfy.h>

pkg_private_data int ntfy_errno_no_print;
pkg_private_data int ntfy_warning_print;

pkg_private_data int ntfy_errno_abort;
pkg_private_data int ntfy_errno_abort_init;

pkg_private void
ntfy_set_errno_debug(error)
    Notify_error    error;
{
    notify_errno = error;
    /* dixon - suppressing annoying error message */
    /* if ((!ntfy_errno_no_print) && error != NOTIFY_OK) 
       notify_perror("Notifier error"); */
    if (!ntfy_errno_abort_init) {
	extern char    *getenv();
	char           *str = getenv("Notify_error_ABORT");

	if (str && (str[0] == 'y' || str[0] == 'Y'))
	    ntfy_errno_abort = 1;
	else
	    ntfy_errno_abort = 0;
    }
    if (ntfy_errno_abort == 1 && error != NOTIFY_OK)
	abort();
}

pkg_private void
ntfy_set_warning_debug(error)
    Notify_error    error;
{
    notify_errno = error;
    if (ntfy_warning_print && error != NOTIFY_OK)
	notify_perror("Notifier warning");
}

pkg_private void
ntfy_assert_debug(code)
    int		    code;
{
    char	   *error_string;

    error_string = xv_malloc(strlen("Notifier internal error (code #999)") + 1);
    sprintf(error_string, "Notifier internal error (code #%d)", code);
    xv_error(XV_ZERO,
	     ERROR_STRING, error_string,
	     NULL);
    free(error_string);
}

pkg_private void
ntfy_fatal_error(msg)
    char           *msg;
{
    char	   *error_string;

    error_string = xv_malloc(strlen(msg) + strlen(XV_MSG("Notifier fatal error: "))
			  + 2);
    strcpy(error_string, XV_MSG("Notifier fatal error: "));
    strcat(error_string, msg);
    xv_error(XV_ZERO,
	     ERROR_STRING, error_string,
	     NULL);
    free(error_string);
}
