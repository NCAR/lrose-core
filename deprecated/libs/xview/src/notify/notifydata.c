#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)notifydata.c 20.12 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * notifydata.c: fix for shared libraries in SunOS4.0.  Code was isolated
 * from ndet_itimer.c
 */

#include <xview_private/ntfy.h>

struct itimerval NOTIFY_NO_ITIMER = {{0, 0}, {0, 0}};
struct itimerval NOTIFY_POLLING_ITIMER = {{0, 1}, {0, 1}};
