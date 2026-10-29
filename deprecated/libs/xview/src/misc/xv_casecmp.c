#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_casecmp.c 1.4 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1992 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *	file for terms of the license.
 */

/*
 * The strcasecmp() function is neither defined by POSIX or SVID,
 * so we provide this for portablity.  Note that a real implementation
 * would optimize performance with a lookup table rather than calling
 * tolower(), but hey, this is a GUI toolkit, not libc; so I ain't goin'
 * to that much trouble.
 */

#include <stdio.h>
#include <ctype.h>
#include <xview_private/i18n_impl.h>

int
xv_strcasecmp( str1, str2 )
     char *str1;
     char *str2;
{
    char low1, low2;

    if ( str1 == str2 )
	return 0;

    while ( (low1 = tolower(*str1)) == (low2 = tolower(*str2)) ) {
	if ( !low1 )
	    return 0;
	str1++; str2++;
    }

    return low1 - low2;
}

int
xv_strncasecmp( str1, str2, n)
     char *str1;
     char *str2;
     int   n;
{
    char low1, low2;

    if ( str1 == str2 )
	return 0;

    n++;

    while ( (--n != 0) && ((low1 = tolower(*str1)) == (low2 = tolower(*str2))) ) {
	if ( !low1 )
	    return 0;
	str1++; str2++;
    }

    return ( (n == 0) ? 0 : (low1 - low2) );
}

#ifdef OW_I18N

int
xv_wscasecmp( str1, str2 )
     CHAR *str1;
     CHAR *str2;
{
    CHAR low1, low2;

    if ( str1 == str2 )
	return 0;

    while ( (low1 = towlower(*str1)) == (low2 = towlower(*str2)) ) {
	if ( !low1 )
	    return 0;
	str1++; str2++;
    }

    return low1 - low2;
}

int
xv_wsncasecmp( str1, str2, n)
     CHAR *str1;
     CHAR *str2;
     int   n;
{
    CHAR low1, low2;

    if ( str1 == str2 )
	return 0;

    n++;

    while ( (--n != 0) && ((low1 = towlower(*str1)) == (low2 = towlower(*str2))) ) {
	if ( !low1 )
	    return 0;
	str1++; str2++;
    }

    return ( (n == 0) ? 0 : (low1 - low2) );
}

#endif /* OW_I18N */
