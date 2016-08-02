#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)mem.c 1.22 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/*
 * Memory pixrect creation/destruction, SunXView version. This module is used
 * as a replacement for the libpixrect library. It is a descendent of
 * "/usr/src/usr.lib/libpixrect/mem/mem.c".
 */

#include <sys/types.h>
#include <pixrect/pixrect.h>
#include <pixrect/pr_impl_make.h>

#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

#include <xview/pkg.h>
#include <xview_private/portable.h>

#ifdef i386
extern struct pixrectops mem_ops;
#endif /* i386 */
static int      xv_pr_error();
int             xv_mem_destroy();

/*
 * Default patterns for pw_line and pw_polyline.
 */
short           pw_tex_dotted[] = {1, 5, 1, 5, 1, 5, 1, 5, 0};
short           pw_tex_dashed[] = {7, 7, 7, 7, 0};
short           pw_tex_dashdot[] = {7, 3, 1, 3, 7, 3, 1, 3, 0};
short           pw_tex_dashdotdotted[] = {9, 3, 1, 3, 1, 3, 1, 3, 0};
short           pw_tex_longdashed[] = {13, 7, 0};

/* ------------------------------------------------------------------------- */


/*
 * Create a memory pixrect that points to an existing (non-pixrect) image.
 */
Xv_private Pixrect *
xv_mem_point(w, h, depth, image)
    int             w, h, depth;
    short          *image;
{
    register Pixrect *pr;
    register struct mpr_data *md;

    if (!(pr = (Pixrect *)
	  xv_malloc(sizeof(Pixrect) + sizeof(struct mpr_data))))
	return pr;

    md = (struct mpr_data *) ((caddr_t) pr + sizeof(Pixrect));

    pr->pr_ops = &mem_ops;
    pr->pr_size.x = w;
    pr->pr_size.y = h;
    pr->pr_depth = depth;
    pr->pr_data = (caddr_t) md;

    md->md_linebytes = mpr_linebytes(w, depth);
    md->md_offset.x = 0;
    md->md_offset.y = 0;
    md->md_primary = 0;
    md->md_flags = 0;
    md->md_image = image;

    return pr;
}

/*
 * Create a memory pixrect, allocate space, and clear it.
 */
Xv_private Pixrect *
xv_mem_create(w, h, depth)
    int             w, h, depth;
{
    register Pixrect *pr;
    register struct mpr_data *md;

    if ((pr = xv_mem_point(w, h, depth, (short *) 0)) == 0)
	return pr;

    md = mpr_d(pr);

    /*
     * If compiled for a 32-bit machine, pad linebytes to a multiple of 4
     * bytes.
     */
#ifndef mc68010
    if (md->md_linebytes & 2 && md->md_linebytes > 2)
	md->md_linebytes += 2;
#endif				/* mc68010 */

	/*
	 * Removed SVR4 port by ISC
	 * The port ifdef'd out a check for malloc'ing 0 bytes.
	 */
	/* allocate space for the pixrect itself, but only if necessary */
	if (h && md->md_linebytes) {
		if (!(md->md_image =
			(short *) xv_malloc((unsigned) (h *= md->md_linebytes)))) {
				(void) pr_destroy(pr);
				return 0;
    			}
		XV_BZERO((char *) md->md_image, h);
	}
	else {
		md->md_image = (short *) 0;
	}
    md->md_primary = 1;

    return pr;
}

/*
 * Destroy a memory pixrect
 */
Xv_private int
xv_mem_destroy(pr)
    Pixrect        *pr;
{
    if (pr) {
	register struct mpr_data *md;

	if ((md = mpr_d(pr)) &&
	    md->md_primary && md->md_image)
	    free((char *) md->md_image);

	free((char *) pr);
    }
    return 0;
}
