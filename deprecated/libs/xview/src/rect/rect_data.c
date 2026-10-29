#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)rect_data.c 20.11 93/06/28 Copyr 1984 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * rect_data.c: fix for shared libraries in SunOS4.0.  Code was isolated from
 * rectlist.c and rect.c
 */

#include <xview/base.h>
#include <xview/rect.h>
#include <xview/rectlist.h>

/*
 * rectlist constants
 */
struct rectlist rl_null = {0, 0, 0, 0, 0, 0, 0, 0};

struct rect     rect_null = {0, 0, 0, 0};
