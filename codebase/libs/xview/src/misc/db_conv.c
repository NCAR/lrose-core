#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)db_conv.c 50.14 93/06/28";
#endif
#endif

#include <xview_private/i18n_impl.h>
#include <xview/xview.h>

/*
 *      (c) Copyright 1989 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE
 *      file for terms of the license.
 */

#ifdef OW_I18N
Pkg_private int
db_cvt_string_to_wcs(from_value, to_value)
    char	*from_value;
    Xv_opaque	*to_value;
{
    static wchar_t *cvtbufptr = NULL;
    static int	buflen = 0;
    int		newlen, status;

    if (buflen < (newlen=strlen(from_value)+1)) {
        if (cvtbufptr != NULL) {
	    xv_free (cvtbufptr);
	}
	cvtbufptr = (wchar_t *)xv_calloc(1, newlen*sizeof(wchar_t));
	buflen = newlen;
    }

    status = mbstowcs(cvtbufptr, from_value, newlen);
    *to_value = (Xv_opaque) cvtbufptr;
    return ((status == -1) || (status > newlen-1) ? XV_ERROR : XV_OK);
}
#endif /* OW_I18N */

Pkg_private int
db_cvt_string_to_long(from_value, to_value)
    char        *from_value;
    Xv_opaque   *to_value;
{
    char  	*ptr; 

    *to_value = (Xv_opaque)strtol(from_value, &ptr, 10);
    return(((ptr == from_value) || (*ptr != '\0')) ? XV_ERROR : XV_OK);
}

Pkg_private int
db_cvt_string_to_int(from_value, to_value)
    char        *from_value;
    Xv_opaque   *to_value;
{
    char  	*ptr; 
    int		tmp;

    tmp = (int)strtol(from_value, &ptr, 10);
    *to_value = (Xv_opaque)tmp;
    return(((ptr == from_value) || (*ptr != '\0')) ? XV_ERROR : XV_OK);
}


Pkg_private int
db_cvt_string_to_bool(from_value, to_value)
    char        *from_value;
    Xv_opaque   *to_value;
{
#define DB_BOOL_VALUES	16
    static char *db_bool_table[DB_BOOL_VALUES] = {
	"true","false",
        "yes", "no",
        "on", "off",
        "enabled",  "disabled",
        "set", "reset",
	"set", "cleared",
        "activated", "deactivated",
        "1", "0",
    };
    register int	i;
    register char   	chr1, chr2;
    register char	*symbol1, *symbol2;

    for (i = 0; i < DB_BOOL_VALUES; i++) {
	symbol1 = *(db_bool_table + i);
	symbol2 = from_value;
	while ((chr1 = *symbol1++) != '\0') {
	    chr2 = *symbol2++;
	    if (('A' <= chr2) && (chr2 <= 'Z'))
		chr2 += 'a' - 'A';
	    if (chr1 != chr2)
		break;
  	}
	if (chr1 == XV_ZERO) {
	    *to_value = (i % 2) ? False : True;
	    return(XV_OK);
	}
    }
    return(XV_ERROR);
}


Pkg_private int
db_cvt_string_to_char(from_value, to_value)
    char        *from_value;
    Xv_opaque   *to_value;
{
    *to_value = from_value[0];
    return(XV_OK);
}

