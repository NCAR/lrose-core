#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)cim_change.c 20.19 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Character image manipulation (except size change) routines.
 */

#include <string.h>
#include <xview_private/i18n_impl.h>
#include <sys/types.h>
#include <pixrect/pixrect.h>
#include <xview_private/ttyansi.h>
#include <xview_private/charimage.h>
#include <xview_private/charscreen.h>
#include <xview_private/tty_impl.h>

char            boldify;

/* extern char    *strcpy(); */

/* static */ void ttysw_roll();
static void reverse();
/* static */ void ttysw_swap();

#define JF

Pkg_private void
ttysw_vpos(row, col)
    int             row, col;
{
    register CHAR  *line = image[row];
    register char  *bold = screenmode[row];
    register int    i;

#ifdef OW_I18N
    while ((int)LINE_LENGTH(line) <= col) {
        bold[LINE_LENGTH(line)] = MODE_CLEAR;
#else
    while ((int)LINE_LENGTH(line) <= col) {
	bold[LINE_LENGTH(line)] = MODE_CLEAR;
#endif
	i = LINE_LENGTH(line);
	line[-1]++;
	line[i] = (CHAR)' ';
    }
#ifdef OW_I18N
    setlinelength(line, ((int)LINE_LENGTH(line)));
#else
    setlinelength(line, ((int)LINE_LENGTH(line)));
#endif
}

Pkg_private void
ttysw_bold_mode()
{
    boldify |= MODE_BOLD;
}

/* NOT USED */
ttysw_nobold_mode()
{
    boldify &= ~MODE_BOLD;
}

Pkg_private void
ttysw_underscore_mode()
{
    boldify |= MODE_UNDERSCORE;
}

/* NOT USED */
ttysw_nounderscore_mode()
{
    boldify &= ~MODE_UNDERSCORE;
}

Pkg_private void
ttysw_inverse_mode()
{
    boldify |= MODE_INVERT;
}

/* NOT USED */
ttysw_noinverse_mode()
{
    boldify &= ~MODE_INVERT;
}

Pkg_private void
ttysw_clear_mode()
{
    boldify = MODE_CLEAR;
}

Pkg_private void
ttysw_writePartialLine(s, curscolStart)
    CHAR           *s;
    register int    curscolStart;
{
    register CHAR  *sTmp;
    register CHAR  *line = image[cursrow];
    register char  *bold = screenmode[cursrow];
    register int    curscolTmp = curscolStart;
#ifdef  OW_I18N
    int    c_sizefactor;
#endif

    /*
     * Fix line length if start is past end of line length. This shouldn't
     * happen but does.
     */
#ifdef OW_I18N
    if ((int)LINE_LENGTH(line) < curscolStart)
#else
    if ((int)LINE_LENGTH(line) < curscolStart)
#endif
	(void) ttysw_vpos(cursrow, curscolStart);
    /*
     * Stick characters in line.
     */
    for (sTmp = s; *sTmp != '\0'; sTmp++) {
	line[curscolTmp] = *sTmp;
	bold[curscolTmp] = boldify;
#ifdef  OW_I18N
        c_sizefactor = tty_character_size( *sTmp );
        while( --c_sizefactor > 0 ) {
                curscolTmp++;
                line[curscolTmp] = TTY_NON_WCHAR;
                bold[curscolTmp] = boldify;
        }
#endif
	curscolTmp++;
    }
    /*
     * Set new line length.
     */
#ifdef OW_I18N
    if ((int)LINE_LENGTH(line) < curscolTmp)
#else
    if ((int)LINE_LENGTH(line) < curscolTmp)
#endif
	setlinelength(line, curscolTmp);
    /*
     * if (sTmp>(s+3)) printf("%d\n",sTmp-s);
     */
    /* Note: curscolTmp should equal curscol here */
    /*
     * if (curscolTmp!=curscol) printf("csurscolTmp=%d, curscol=%d\n",
     * curscolTmp,curscol);
     */
    (void) ttysw_pstring(s, boldify, curscolStart, cursrow, PIX_SRC);
}

#ifdef JF
Pkg_private void
ttysw_cim_scroll(n)
    register int    n;
{
    register int    new;

#ifdef DEBUG_LINES
    printf(" ttysw_cim_scroll(%d)	\n", n);
#endif
    if (n > 0) {		/* text moves UP screen	 */
	(void) delete_lines(ttysw_top, n);
    } else {			/* (n<0)	text moves DOWN	screen	 */
	new = ttysw_bottom + n;
	(void) ttysw_roll(ttysw_top, new, ttysw_bottom);
	(void) ttysw_pcopyscreen(ttysw_top, ttysw_top - n, new);
	(void) ttysw_cim_clear(ttysw_top, ttysw_top - n);
    }
}

#else
Pkg_private void
ttysw_cim_scroll(toy, fromy)
    int             fromy, toy;
{

    if (toy < fromy)		/* scrolling up */
	(void) ttysw_roll(toy, ttysw_bottom, fromy);
    else
	ttysw_swapregions(fromy, toy, ttysw_bottom - toy);
    if (fromy > toy) {
	(void) ttysw_pcopyscreen(fromy, toy, ttysw_bottom - fromy);
	(void) ttysw_cim_clear(ttysw_bottom - (fromy - toy), ttysw_bottom);
	/* move text up */
    } else {
	(void) ttysw_pcopyscreen(fromy, toy, ttysw_bottom - toy);
	(void) ttysw_cim_clear(fromy, ttysw_bottom - (toy - fromy));	/* down */
    }
}

#endif

Pkg_private void
ttysw_insert_lines(where, n)
    register int    where, n;
{
    register int    new = where + n;

#ifdef DEBUG_LINES
    printf(" ttysw_insert_lines(%d,%d) ttysw_bottom=%d	\n", where, n, ttysw_bottom);
#endif
    if (new > ttysw_bottom)
	new = ttysw_bottom;
    (void) ttysw_roll(where, new, ttysw_bottom);
    (void) ttysw_pcopyscreen(where, new, ttysw_bottom - new);
    (void) ttysw_cim_clear(where, new);
}

/* BUG ALERT:  Externally visible procedure without a valid XView prefix. */
Pkg_private void
delete_lines(where, n)
    register int    where, n;
{
    register int    new = where + n;

#ifdef DEBUG_LINES
    printf(" delete_lines(%d,%d)	\n", where, n);
#endif
    if (new > ttysw_bottom) {
	n -= new - ttysw_bottom;
	new = ttysw_bottom;
    }
    (void) ttysw_roll(where, ttysw_bottom - n, ttysw_bottom);
    (void) ttysw_pcopyscreen(new, where, ttysw_bottom - new);
    (void) ttysw_cim_clear(ttysw_bottom - n, ttysw_bottom);
}

/* static */ void
ttysw_roll(first, mid, last)
    int             first, last, mid;
{

    /* printf("first=%d, mid=%d, last=%d\n", first, mid, last); */
    reverse(first, last);
    reverse(first, mid);
    reverse(mid, last);
}

static void
reverse(a, b)
    int             a, b;
{

    b--;
    while (a < b)
	(void) ttysw_swap(a++, b--);
}

/* static */ void
ttysw_swapregions(a, b, n)
    int             a, b, n;
{

    while (n--)
	(void) ttysw_swap(a++, b++);
}

/* static */ void
ttysw_swap(a, b)
    int             a, b;
{
    CHAR           *tmpline = image[a];
    char           *tmpbold = screenmode[a];

    image[a] = image[b];
    image[b] = tmpline;
    screenmode[a] = screenmode[b];
    screenmode[b] = tmpbold;
}

Pkg_private void
ttysw_cim_clear(a, b)
    int             a, b;
{
    register int    i;

    for (i = a; i < b; i++)
	setlinelength(image[i], 0);
    (void) ttysw_pclearscreen(a, b);
    if (a == ttysw_top && b == ttysw_bottom) {
	if (delaypainting)
	    (void) ttysw_pdisplayscreen(1);
	else
	    delaypainting = 1;
    }
}

Pkg_private void
ttysw_deleteChar(fromcol, tocol, row)
    int             fromcol, tocol, row;
{
    CHAR           *line = image[row];
    char           *bold = screenmode[row];
#ifdef OW_I18N
#ifndef SVR4
    int             len = LINE_LENGTH(line);
#else
    int             len = (int)LINE_LENGTH(line);
#endif /* ~SVR4 */
#else
#ifndef SVR4
    int             len = LINE_LENGTH(line);
#else
    int             len = (int)LINE_LENGTH(line);
#endif /* ~SVR4 */
#endif /* OW_I18N */

    if (fromcol >= tocol)
	return;

#ifdef  OW_I18N
/*
 *      Just in case , caller should take care that deletion occurs
 *      character by character instead of column by column
 */
    if( line[fromcol] == TTY_NON_WCHAR ) {
        while( fromcol > 0 && line[fromcol] == TTY_NON_WCHAR )
                fromcol--;
    }

    if( line[tocol] == TTY_NON_WCHAR ) {
        while( tocol < len - 1 && line[tocol] == TTY_NON_WCHAR )
                tocol++;
    }
#endif

    if (tocol < len) {
	/*
	 * There's a fragment left at the end
	 */
	int             gap = tocol - fromcol;
	{
            register CHAR  *a = line + fromcol;
            register CHAR  *b = line + tocol;
	    register char  *am = bold + fromcol;
	    register char  *bm = bold + tocol;
	    while (*a++ = *b++)
		*am++ = *bm++;
	}
	setlinelength(line, len - gap);
	(void) ttysw_pcopyline(fromcol, tocol, len - tocol, row);
	(void) ttysw_pclearline(len - gap, len, row);
    } else if (fromcol < len) {
	setlinelength(line, fromcol);
	(void) ttysw_pclearline(fromcol, len, row);
    }
}

Pkg_private void
ttysw_insertChar(fromcol, tocol, row)
    int             fromcol;
    register int    tocol;
    int             row;
{
    register CHAR  *line = image[row];
    register char  *bold = screenmode[row];
#ifdef OW_I18N
    int             len = LINE_LENGTH(line);
#else
    int             len = LINE_LENGTH(line);
#endif
    register int    i;
    int             delta, newlen, slug, rightextent;

#ifdef  OW_I18N
/*
 *      Just in case , caller should take care that deletion occurs
 *      character by character instead of column by column
 */
    if( line[fromcol] == TTY_NON_WCHAR ) {
        while( fromcol > 0 && line[fromcol] == TTY_NON_WCHAR )
                fromcol--;
    }
 
    if( line[tocol] == TTY_NON_WCHAR ) {
        while( tocol < len - 1 && line[tocol] == TTY_NON_WCHAR )
                tocol++;
    }
#endif

    if (fromcol >= tocol || fromcol >= len)
	return;
    delta = tocol - fromcol;
    newlen = len + delta;
    if (newlen > ttysw_right)
	newlen = ttysw_right;
    if (tocol > ttysw_right)
	tocol = ttysw_right;
    for (i = newlen; i >= tocol; i--) {
	line[i] = line[i - delta];
	bold[i] = bold[i - delta];
    }
    for (i = fromcol; i < tocol; i++) {
	line[i] = ' ';
	bold[i] = MODE_CLEAR;
    }
    setlinelength(line, newlen);
    rightextent = len + (tocol - fromcol);
    slug = len - fromcol;
    if (rightextent > ttysw_right)
	slug -= rightextent - ttysw_right;
    (void) ttysw_pcopyline(tocol, fromcol, slug, row);
    (void) ttysw_pclearline(fromcol, tocol, row);
}

#ifdef OW_I18N
Pkg_private void
tty_column_wchar_type( xChar , yChar , cwidth , offset )
    int         xChar;
    int         yChar;
    int         *cwidth;        /* character width (RETURN) */
    int         *offset;        /* offset of charcter (RETURN) */
{
    CHAR               *line = image[yChar];
    register CHAR       c = line[xChar];

    *offset = 0;
    if( c == TTY_NON_WCHAR ) {
        while( c == TTY_NON_WCHAR ) {
                c = line[--xChar];
                (*offset) ++;
        }
    }    

    *cwidth = tty_character_size( c );

}

Pkg_private int
tty_get_nchars( colstart , colend , row )
    int                 colstart;
    register int        colend;
    int                 row;
{
    CHAR        *line = image[row];
    register    int     nchar = 0;
    int         i;

    if( colend == TTY_LINE_INF_INDEX )   /* up to end of line */
        colend = LINE_LENGTH( line ) - 1 ;

    for( i = colstart; i<= colend ; i++ ) {
        if( line[i] == TTY_NON_WCHAR )
                continue;
        nchar++;
    }

    return nchar;

}

#endif
