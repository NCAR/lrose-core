#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)hlp_rodata.c 1.4 93/06/28";
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

#include <xview/xview.h>
#include <xview/svrimage.h>

const unsigned short    mglass_data[] = {
#include <images/mglass.icon>
};


const unsigned short    mglass_mask_data[] = {
#include <images/mglass_mask.icon>
};

