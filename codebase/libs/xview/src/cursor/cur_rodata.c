#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)cur_rodata.c 1.5 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1989 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE
 *      file for terms of the license.
 */

/* This file is compiled with the -R flag.
 * This ensures that  all data in this file is moved to the text segment
 * of the shared library. So,this data is strictly read-only.
 */

#include <xview_private/curs_impl.h>
#define static const
#include <bitmaps/txtmv>
#include <bitmaps/txtmvmask>
#include <bitmaps/txtmvmore>
#include <bitmaps/txtdup>
#include <bitmaps/txtdupmask>
#include <bitmaps/txtdupmore>
#include <bitmaps/txtmvok>
#include <bitmaps/txtmvokmask>
#include <bitmaps/txtmvokmore>
#include <bitmaps/txtdupok>
#include <bitmaps/txtdupokmask>
#include <bitmaps/txtdupokmore>

