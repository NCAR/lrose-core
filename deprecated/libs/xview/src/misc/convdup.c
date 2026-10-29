#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)convdup.c 1.19 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#ifdef OW_I18N
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <xview_private/i18n_impl.h>
#include <xview/pkg.h>
#include <xview/xv_error.h>

static wchar_t * tbl;

static wchar_t * 
initTable(void)
{
    int i;
    char ch;

    tbl = xv_malloc(256*sizeof(wchar_t));    
    tbl[0] = 0;
    for(i=1;i<256;i++) {
        ch = i;
        mbtowc(tbl+i, &ch, 1);
    }
    return(tbl);
}

Xv_private size_t  
_xv_mbstowcs(wcs, mbs, n)
    register wchar_t 	* 	wcs;
    register unsigned char *    mbs;
    unsigned int n;
{
/*    if (multibyte) */
        return (mbstowcs(wcs, (char *)mbs, n));

/*    else {
        register int ch = 0;
        register wchar_t * cnvt = tbl;
        if (!cnvt) cnvt = initTable();        
        while(n--) {
            ch++;
            *wcs = cnvt[*mbs++];
            if (*wcs == 0) break;
            wcs++;
        }
        *wcs = 0;
        return(ch);
    }
*/
}

Xv_private wchar_t *
_xv_mbstowcsdup(mbs)
     char	*mbs;
{
	int	 n;
	wchar_t	*wcs;

	if (mbs == NULL)
	    return NULL;

	n = strlen(mbs) + 1;
	wcs = xv_alloc_n(wchar_t, n);
	mbstowcs(wcs, mbs, n);

	return wcs;
}


Xv_private char *
_xv_wcstombsdup(wcs)
    wchar_t	*wcs;
{
	int	 n;
	char	*mbs;

	if (wcs == NULL)
		return NULL;

	n = (wslen(wcs) * MB_CUR_MAX) + 1;
	mbs = xv_malloc (n);
	wcstombs(mbs, wcs, n);

	return mbs;
}
#endif /* OW_I18N */
