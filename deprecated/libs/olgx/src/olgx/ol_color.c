/*#ident "@(#)ol_color.c	1.9 93/06/28 SMI" */

/*
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

/*
 * Referernce: Hearn/Baker, "Computer Graphics", Prentice Hall, Section 14-4.
 */

#include <olgx_private/olgx_impl.h>


/*
 * Private Routine  return the max of three x,y,z in x
 */

static int
max3(x, y, z)
    register int    x, y, z;
{
    if (y > x)
	x = y;
    if (z > x)
	x = z;
    return x;
}

/*
 * Private Routine  return the min of three variables x,y,z in x
 */

static int
min3(x, y, z)
    register int    x, y, z;
{
    if (y < x)
	x = y;
    if (z < x)
	x = z;
    return x;
}

void
hsv_to_rgb(hsv, rgb)
    HSV            *hsv;
    RGB            *rgb;
{
    int             h = hsv->h;
    int             s = hsv->s;
    int             v = hsv->v;
    int             r, g, b;
    int             i, f;
    int             p, q, t;

    s = (s * MAXRGB) / MAXSV;
    v = (v * MAXRGB) / MAXSV;
    if (h == 360)
	h = 0;

    if (s == 0) {

	h = 0;
	r = g = b = v;
    }
    i = h / 60;
    f = h % 60;
    p = v * (MAXRGB - s) / MAXRGB;
    q = v * (MAXRGB - s * f / 60) / MAXRGB;
    t = v * (MAXRGB - s * (60 - f) / 60) / MAXRGB;

    switch (i) {

      case 0:
	r = v, g = t, b = p;
	break;

      case 1:
	r = q, g = v, b = p;
	break;

      case 2:
	r = p, g = v, b = t;
	break;

      case 3:
	r = p, g = q, b = v;
	break;

      case 4:
	r = t, g = p, b = v;
	break;

      case 5:
	r = v, g = p, b = q;
	break;
    }

    rgb->r = r;
    rgb->g = g;
    rgb->b = b;

}

void
rgb_to_hsv(rgb, hsv)
    RGB            *rgb;
    HSV            *hsv;
{
    int             r = rgb->r;
    int             g = rgb->g;
    int             b = rgb->b;
    register int    maxv = max3(r, g, b);
    register int    minv = min3(r, g, b);
    int             h;
    int             s;
    int             v;

    v = maxv;

    if (maxv)
	s = (maxv - minv) * MAXRGB / maxv;
    else
	s = 0;

    if (s == 0)
	h = 0;
    else {
	int             rc;
	int             gc;
	int             bc;
	int             hex;

	rc = (maxv - r) * MAXRGB / (maxv - minv);
	gc = (maxv - g) * MAXRGB / (maxv - minv);
	bc = (maxv - b) * MAXRGB / (maxv - minv);

	if (r == maxv) {
	    h = bc - gc, hex = 0;
	} else if (g == maxv) {
	    h = rc - bc, hex = 2;
	} else if (b == maxv) {
	    h = gc - rc, hex = 4;
	}
	h = hex * 60 + (h * 60 / MAXRGB);
	if (h < 0)
	    h += 360;
    }

    hsv->h = h;
    hsv->s = (s * MAXSV) / MAXRGB;
    hsv->v = (v * MAXSV) / MAXRGB;
}

void
rgb_to_xcolor(r, x)
    RGB            *r;
    XColor         *x;
{
    x->red = (unsigned short) r->r << 8;
    x->green = (unsigned short) r->g << 8;
    x->blue = (unsigned short) r->b << 8;
    x->flags = DoRed | DoGreen | DoBlue;
}

/*
 * Load an XColor with an HSV.
 */
void
hsv_to_xcolor(h, x)
    HSV            *h;
    XColor         *x;
{
    RGB             r;
    hsv_to_rgb(h, &r);
    rgb_to_xcolor(&r, x);
}

/*
 * Load an XColor with an HSV.
 */
void
xcolor_to_hsv(x, h)
    XColor         *x;
    HSV            *h;
{
    RGB             r;

    r.r = (int) x->red >> 8;
    r.g = (int) x->green >> 8;
    r.b = (int) x->blue >> 8;
    rgb_to_hsv(&r, h);

}

void
olgx_hsv_to_3D(bg1, bg2, bg3, white)
    HSV            *bg1;
    XColor         *bg2, *bg3, *white;
{
    HSV             hsv;
    int             h = bg1->h;
    int             s = bg1->s;
    int             v = bg1->v;

    v = (v * VMUL) / 10;
    if (v > MAXSV) {
	s /= SDIV;
	v = MAXSV;
    }
    if (v < VMIN)
	v = VMIN;

    hsv.h = h;
    hsv.s = s;
    hsv.v = v;
    hsv_to_xcolor(&hsv, white);

    hsv.h = bg1->h;
    hsv.s = bg1->s;
    hsv.v = (bg1->v * 9) / 10;	/* 90% */
    hsv_to_xcolor(&hsv, bg2);

    hsv.h = bg1->h;
    hsv.s = bg1->s;
    hsv.v = bg1->v >> 1;	/* 50% */
    hsv_to_xcolor(&hsv, bg3);

}


/* ARGSUSED */

void
olgx_calculate_3Dcolors(fg, bg1, bg2, bg3, white)
    XColor         *fg;
    XColor         *bg1, *bg2, *bg3, *white;
{
    HSV             base;

    xcolor_to_hsv(bg1, &base);
    olgx_hsv_to_3D(&base, bg2, bg3, white);

}
