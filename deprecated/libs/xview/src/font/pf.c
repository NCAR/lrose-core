#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)pf.c 20.17 93/06/28 SMI";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Pixfont open/close: xv_pf_open(), xv_pf_open_private(), xv_pf_close()
 * 
 * Variant version for XView: based on standard pixrect version from SunOS 4.0
 */

#include <sys/types.h>
#include <pixrect/pixrect.h>
#include <pixrect/pixfont.h>
#include <xview/server.h>

/* public functions */

extern Pixfont *xv_pf_open();

Xv_public Pixfont        *
xv_pf_default()
{
    return (xv_pf_open((char *) 0));
}

Xv_public Pixfont        *
xv_pf_open(fontname)
    char           *fontname;
{
    if (!xv_default_server)
	xv_create(0, SERVER, NULL);
    return ((Pixfont *) xv_get(xv_default_server,
			       SERVER_FONT_WITH_NAME, fontname));
}

Xv_public Pixfont        *
xv_pf_open_private(fontname)
    char           *fontname;
{
    return 0;
}

Xv_public int
xv_pf_close(pf)
    Pixfont        *pf;
{
    return 0;
}


/* implementation */

#ifdef SUNVIEW1
static Pixfont *
xv_pf_load_vfont(fontname)
    char           *fontname;
{
    FILE           *fontf = 0;
    struct header   hd;
    struct dispatch disp[NUM_DISPATCH];

    register Pixfont *pf = 0;
    register struct dispatch *d;
    register struct pixchar *pc;


    if ((fontf = fopen(fontname, "r")) == 0 ||
	fread((char *) &hd, sizeof hd, 1, fontf) != 1 ||
	fread((char *) disp, sizeof disp, 1, fontf) != 1 ||
	hd.magic != VFONT_MAGIC)
	goto bad;

    /*
     * Allocate font header and set default sizes. The default width of the
     * font is taken to be the width of a lower-case a, if there is one. The
     * default interline spacing is taken to be 3/2 the height of an
     * upper-case A above the baseline.
     */
    if ((pf = (Pixfont *) xv_calloc(1, sizeof *pf)) == 0)
	goto bad;

    if (disp['a'].nbytes && disp['a'].width > 0 &&
	disp['A'].nbytes && disp['A'].up > 0) {
	pf->pf_defaultsize.x = disp['a'].width;
	pf->pf_defaultsize.y = disp['A'].up * 3 >> 1;
    } else {
	pf->pf_defaultsize.x = hd.maxx;
	pf->pf_defaultsize.y = hd.maxy;
    }

    if (pf->pf_defaultsize.x >= 1024 || pf->pf_defaultsize.x <= 0 ||
	pf->pf_defaultsize.y >= 1024 || pf->pf_defaultsize.y <= 0)
	goto bad;

    /*
     * Create memory pixrects for characters of font.
     */
    for (d = disp, pc = pf->pf_char; d < &disp[NUM_DISPATCH]; d++, pc++) {
	if (d->nbytes) {
	    int             w, h, bytes, pad;
	    register char  *image;

	    w = d->left + d->right;
	    h = d->up + d->down;

	    if ((pc->pc_pr = mem_create(w, h, 1)) == 0)
		goto bad;

	    bytes = (w + 7) >> 3;

	    pad = mpr_d(pc->pc_pr)->md_linebytes - bytes;
	    image = (char *) mpr_d(pc->pc_pr)->md_image;

	    if (fseek(fontf,
		      (long) (sizeof hd + sizeof disp + d->addr), 0) < 0)
		goto bad;

	    while (h--) {
		register int    i;

		for (i = bytes; i; i--)
		    *image++ = getc(fontf);
		image += pad;
	    }

	    if (feof(fontf) || ferror(fontf))
		goto bad;

	    pc->pc_home.x = -d->left;
	    pc->pc_home.y = -d->up;
	    pc->pc_adv.x = d->width;
	    pc->pc_adv.y = 0;
	}
    }

    /*
     * Done.
     */
exit:
    if (fontf)
	(void) fclose(fontf);
    return (pf);

bad:
    if (pf) {
	xv_pf_free_font(pf);
	pf = 0;
    }
    goto exit;
}

#endif
