#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ntfyperror.c 20.19 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ntfy_perror.c - Notify_perror implementation.
 */

#include <stdio.h>
#include <xview_private/i18n_impl.h>
#include <xview/xv_error.h>
#include <xview_private/ntfy.h>

extern void
notify_perror(str)
    char           *str;
{
    register char  *msg;
    char            dummy[128];

    switch (notify_errno) {
      case NOTIFY_OK:
	msg = XV_MSG("Success");
	break;
      case NOTIFY_UNKNOWN_CLIENT:
	msg = XV_MSG("Unknown client");
	break;
      case NOTIFY_NO_CONDITION:
	msg = XV_MSG("No condition for client");
	break;
      case NOTIFY_BAD_ITIMER:
	msg = XV_MSG("Unknown interval timer type");
	break;
      case NOTIFY_BAD_SIGNAL:
	msg = XV_MSG("Bad signal number");
	break;
      case NOTIFY_NOT_STARTED:
	msg = XV_MSG("Notifier not started");
	break;
      case NOTIFY_DESTROY_VETOED:
	msg = XV_MSG("Destroy vetoed");
	break;
      case NOTIFY_INTERNAL_ERROR:
	msg = XV_MSG("Notifier internal error");
	break;
      case NOTIFY_SRCH:
	msg = XV_MSG("No such process");
	break;
      case NOTIFY_BADF:
	msg = XV_MSG("Bad file number");
	break;
      case NOTIFY_NOMEM:
	msg = XV_MSG("Not enough memory");
	break;
      case NOTIFY_INVAL:
	msg = XV_MSG("Invalid argument");
	break;
      case NOTIFY_FUNC_LIMIT:
	msg = XV_MSG("Too many interposition functions");
	break;
      default:
	msg = XV_MSG("Unknown notifier error");
    }
    (void) sprintf(dummy, "%s: %s", str, msg);
    xv_error(XV_ZERO,
	     ERROR_STRING, dummy,
	     NULL);
}
