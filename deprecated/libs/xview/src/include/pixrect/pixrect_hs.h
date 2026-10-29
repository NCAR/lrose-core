/*	@(#)pixrect_hs.h 1.23 90/04/23 SMI	*/

/*
 * Copyright 1986, 1987 by Sun Microsystems, Inc.
 */

/*
 * Include this header file to get all pixrect related header files.
 */

#ifndef	pixrect_hs_DEFINED
#define	pixrect_hs_DEFINED

#include <sys/types.h>
#include <pixrect/pixrect.h>

#ifndef NEWPIXMEM
#include <pixrect/pr_dblbuf.h>
#include <pixrect/pr_line.h>
#include <pixrect/pr_planegroups.h>
#endif /* NEWPIXMEM */

#include <pixrect/pr_util.h>

#ifndef NEWPIXMEM
#include <pixrect/traprop.h>

#include <pixrect/bw2var.h>
#include <pixrect/cg2var.h>
#include <pixrect/gp1var.h>
#include <pixrect/cg4var.h>
#include <pixrect/cg8var.h>
#include <pixrect/cg9var.h>
#include <pixrect/cg12_var.h>
#endif /* NEWPIXMEM */

#include <pixrect/memvar.h>

#ifndef NEWPIXMEM
#include <pixrect/mem32_var.h>

#include <pixrect/pixfont.h>
#endif /* NEWPIXMEM */

#include <rasterfile.h>
#include <pixrect/pr_io.h>

#endif /*	pixrect_hs_DEFINED */
