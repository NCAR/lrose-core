#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_i18n.c 1.15 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#ifdef OW_I18N

#include <xview/xview.h>
#include <xview_private/i18n_impl.h>

Xv_private_data wchar_t		_xv_null_string_wc[] = { '\0' };


/*
 * *** Pseudo Static Wide Character String support routines ***
 */

/*
 * _xv_pswcs_wcsdup: Copy/dup wcs to pswcs.
 */
Xv_private void
_xv_pswcs_wcsdup(pswcs, new)
    _xv_pswcs_t	*pswcs;
    wchar_t	*new;
{
    int		len;

    if (new == NULL) {
	pswcs->value = NULL;
	return;
    }

    if ((len = wslen(new) + 1) > pswcs->length) {
	if (pswcs->storage != NULL)
	    xv_free(pswcs->storage);
	pswcs->storage = xv_alloc_n(wchar_t, len);
	pswcs->length = len;
    }
    wscpy(pswcs->storage, new);
    pswcs->value = pswcs->storage;
}


/*
 * _xv_pswcs_mbsdup: Convert mbs to wcs, and copy/dup it to pswcs.
 */
Xv_private void
_xv_pswcs_mbsdup(pswcs, new)
    _xv_pswcs_t	*pswcs;
    char	*new;
{
    int		len;


    if (new == NULL) {
	pswcs->value = NULL;
	return;
    }

    if ((len = strlen(new) + 1) > pswcs->length) {
	if (pswcs->storage != NULL)
	    xv_free(pswcs->storage);
	pswcs->storage = xv_alloc_n(wchar_t, len);
	pswcs->length = len;
    }
    mbstowcs(pswcs->storage, new, len);
    pswcs->value = pswcs->storage;
}


/*
 * *** Pseudo Static Multi Byte character String support routines ***
 */

/*
 * _xv_psmbs_wcsdup: Convert wcs to mbs, and copy/dup its to psmbs.
 */
Xv_private void
_xv_psmbs_wcsdup(psmbs, new)
    _xv_psmbs_t	*psmbs;
    wchar_t	*new;
{
    int		len;

    if (new == NULL) {
	psmbs->value = NULL;
	return;
    }

    if ((len = (wslen(new) + 1) * MB_CUR_MAX) > psmbs->length) {
	if (psmbs->storage != NULL)
	    xv_free(psmbs->storage);
	psmbs->storage = xv_alloc_n(char, len);
	psmbs->length = len;
    }
    wcstombs(psmbs->storage, new, len);
    psmbs->value = psmbs->storage;
}


/*
 * _xv_psmbs_mbsdup: Copy/dup mbs to psmbs.
 */
Xv_private void
_xv_psmbs_mbsdup(psmbs, new)
    _xv_psmbs_t	*psmbs;
    char	*new;
{
    int		len;


    if (new == NULL) {
	psmbs->value = NULL;
	return;
    }

    if ((len = strlen(new) + 1) > psmbs->length) {
	if (psmbs->storage != NULL)
	    xv_free(psmbs->storage);
	psmbs->storage = xv_alloc_n(char, len);
	psmbs->length = len;
    }
    strcpy(psmbs->storage, new);
    psmbs->value = psmbs->storage;
}


/*
 * Following routines are convinience routine to handle string
 * attribute which should not make duplicate copy of the contents (for
 * binary/source compat with none Asian XView apps).
 */

/*
 * _xv_use_pswcs_value_nodup: Make pswcs.value to be ready to use.
 */
Xv_private void
_xv_use_pswcs_value_nodup(xsan)
    _xv_string_attr_nodup_t	*xsan;
{
    switch(xsan->flag) {
    case XSAN_NOT_SET:
	xsan->pswcs.value = NULL;
	break;

    case XSAN_SET_BY_MBS:
	_xv_pswcs_mbsdup(&(xsan->pswcs), xsan->psmbs.value);
	break;

    case XSAN_SET_BY_WCS:
	break;
    }
}


/*
 * _xv_use_psmbs_value_nodup: Make psmbs.value to be ready to use.
 */
Xv_private void
_xv_use_psmbs_value_nodup(xsan)
    _xv_string_attr_nodup_t	*xsan;
{
    switch(xsan->flag) {
    case XSAN_NOT_SET:
	xsan->psmbs.value = NULL;
	break;

    case XSAN_SET_BY_MBS:
	break;

    case XSAN_SET_BY_WCS:
	_xv_psmbs_wcsdup(&(xsan->psmbs), xsan->pswcs.value);
	break;
    }
}


/*
 * _xv_set_mbs_attr_nodup: xv_set support for mbs attr.
 */
Xv_private void
_xv_set_mbs_attr_nodup(xsan, new)
    _xv_string_attr_nodup_t	*xsan;
    char			*new;
{
    xsan->flag = XSAN_SET_BY_MBS;
    xsan->psmbs.value = new;
}


/*
 * _xv_set_wcs_attr_nodup: xv_set support for wcs attr.
 */
Xv_private void
_xv_set_wcs_attr_nodup(xsan, new)
    _xv_string_attr_nodup_t	*xsan;
    wchar_t			*new;
{
    xsan->flag = XSAN_SET_BY_WCS;
    xsan->pswcs.value = new;
}


/*
 * _xv_get_mbs_attr_nodup: xv_get support for mbs attr.
 */
Xv_private char *
_xv_get_mbs_attr_nodup(xsan)
    _xv_string_attr_nodup_t	*xsan;
{
    _xv_use_psmbs_value_nodup(xsan);
    return xsan->psmbs.value;
}


/*
 * _xv_get_wcs_attr_nodup: xv_get support for wcs attr.
 */
Xv_private wchar_t *
_xv_get_wcs_attr_nodup(xsan)
    _xv_string_attr_nodup_t	*xsan;
{
    _xv_use_pswcs_value_nodup(xsan);
    return xsan->pswcs.value;
}


/*
 * _xv_free_string_attr_nodup: freeup the value originally specified
 * by xv_get calls.
 */
Xv_private void
_xv_free_string_attr_nodup(xsan)
    _xv_string_attr_nodup_t	*xsan;
{
    switch(xsan->flag) {
    case XSAN_NOT_SET:
	break;

    case XSAN_SET_BY_MBS:
	if (xsan->psmbs.value != NULL) {
	    /*
	     * Should be using "free()", not xv_free, because this is
	     * not allocated by XView but user's application.
	     */
	    free(xsan->psmbs.value);
	}
	break;

    case XSAN_SET_BY_WCS:
	if (xsan->pswcs.value != NULL) {
	    /*
	     * Should be using "free()", not xv_free, because this is
	     * not allocated by XView but user's application.
	     */
	    free(xsan->pswcs.value);
	}
	break;
    }
}


/*
 * _xv_free_ps_string_attr_nodup: Freeup space which allocated by
 * either pswcs or psmbs.
 */
Xv_private void
_xv_free_ps_string_attr_nodup(xsan)
    _xv_string_attr_nodup_t	*xsan;
{
    if (xsan->psmbs.storage != NULL) {
	xv_free(xsan->psmbs.storage);
	xsan->psmbs.storage = NULL;
	xsan->psmbs.length = 0;
    }
    if (xsan->pswcs.storage != NULL) {
	xv_free(xsan->pswcs.storage);
	xsan->pswcs.storage = NULL;
	xsan->pswcs.length = 0;
    }
}


/*
 * Followings for XView string attributes which does copy/dup.
 */

/*
 * _xv_set_mbs_attr_dup: xv_set support for mbs attr.
 */
Xv_private void
_xv_set_mbs_attr_dup(xsad, new)
    _xv_string_attr_dup_t	*xsad;
    char			*new;
{
    xsad->psmbs.value = NULL;
    _xv_pswcs_mbsdup(&xsad->pswcs, new);
}


/*
 * _xv_set_wcs_attr_dup: xv_set support for wcs attr.
 */
Xv_private void
_xv_set_wcs_attr_dup(xsad, new)
    _xv_string_attr_dup_t	*xsad;
    wchar_t			*new;
{
    xsad->psmbs.value = NULL;
    _xv_pswcs_wcsdup(&xsad->pswcs, new);
}


/*
 * _xv_get_mbs_attr_dup: xv_get support for mbs attr.
 */
Xv_private char *
_xv_get_mbs_attr_dup(xsad)
    _xv_string_attr_dup_t	*xsad;
{
    if (xsad->psmbs.value == NULL)
	_xv_psmbs_wcsdup(&xsad->psmbs, xsad->pswcs.value);

    return xsad->psmbs.value;
}


/*
 * _xv_get_wcs_attr_dup: xv_get support for wcs attr.
 */
Xv_private wchar_t *
_xv_get_wcs_attr_dup(xsad)
    _xv_string_attr_dup_t	*xsad;
{
    return xsad->pswcs.value;
}


/*
 * _xv_free_ps_string_attr_dup: Free up space allocated by either
 * pswcs or psmbs.
 */
Xv_private void
_xv_free_ps_string_attr_dup(xsad)
    _xv_string_attr_dup_t	*xsad;
{
    if (xsad->psmbs.storage != NULL) {
	xv_free(xsad->psmbs.storage);
	xsad->psmbs.storage = NULL;
	xsad->psmbs.length = 0;
    }

    if (xsad->pswcs.storage != NULL) {
	xv_free(xsad->pswcs.storage);
	xsad->pswcs.storage = NULL;
	xsad->pswcs.length = 0;
    }
}


/*
 * _xv_XwcTextListToTextProperty: Error handling wrapper for the
 * XwcTextListToTextProperty() Xlib function.
 */
Xv_private int
_xv_XwcTextListToTextProperty(object, pkg, dpy, list, count, style, text_prop)
    Xv_object			  object;
    Xv_pkg			 *pkg;
    Display			 *dpy;
    wchar_t			**list;
    int				  count;
    XICCEncodingStyle		  style;
    XTextProperty		 *text_prop;
{
    int		 state;
    Bool	 alloc_msg;
    char	*errmsg;
    char	*msg;


    state = XwcTextListToTextProperty(dpy, list, count, style, text_prop);
    switch (state) {
	case XNoMemory:
	    errmsg = XV_MSG("Not enough memory to convert the text list");
	    alloc_msg = False;
	    goto err;

	case XLocaleNotSupported:
	    /*
	     * We should not get this error (since we call
	     * XSupportedLocale in server pkg), however...
	     */
	    msg = XV_MSG("The locale (%10.10s) does not supports text list conversion");
	    errmsg = (char *) xv_malloc(strlen(msg) + 11);
	    (void) sprintf(errmsg, msg, setlocale(LC_CTYPE, NULL));
	    alloc_msg = True;
	    goto err;

	default:
	    if (state == 0)
		break;
	    msg = XV_MSG("%d character(s) could not converted to the text property");
	    /* 19 holds even for long long + 1 for sign + 1 for null */
	    errmsg = (char *) xv_malloc(strlen(msg) + 19 + 2);
	    (void) sprintf(errmsg, msg, state);
	    alloc_msg = True;
err:
	    if (pkg == NULL) {
	        xv_error(object,
			 ERROR_STRING,	errmsg,
			 NULL);
	    } else {
	        xv_error(object,
			 ERROR_PKG,	pkg,
			 ERROR_STRING,	errmsg,
			 NULL);
	    }
	    if (alloc_msg == True)
		xv_free(errmsg);
	    break;
    }

    return state;
}


/*
 * _xv_XwcTextPropertyToTextList: Error handling wrapper for the
 * XwcTextPropertyToTextList() Xlib function.
 */
Xv_private int
_xv_XwcTextPropertyToTextList(object, pkg, dpy, text_prop, list, count)
    Xv_object			   object;
    Xv_pkg			  *pkg;
    Display			  *dpy;
    XTextProperty		  *text_prop;
    wchar_t			***list;
    int				  *count;
{
    int		 state;
    Bool	 alloc_msg;
    char	*errmsg;
    char	*msg;


    state = XwcTextPropertyToTextList(dpy, text_prop, list, count);
    switch (state) {
	case XNoMemory:
	    errmsg = XV_MSG("Not enough memory to convert the text property");
	    alloc_msg = False;
	    goto err;

	case XLocaleNotSupported:
	    /*
	     * We should not get this error (since we call
	     * XSupportedLocale in server pkg), however...
	     */
	    msg = XV_MSG("The locale (%10.10s) does not supports text list conversion");
	    errmsg = xv_malloc(strlen(msg) + 11);
	    (void) sprintf(errmsg, msg, setlocale(LC_CTYPE, NULL));
	    alloc_msg = True;
	    goto err;

	case XConverterNotFound:
	    msg = XV_MSG("Converter could not find to convert from text property (atom#%d)");
	    /* 19 holds even for long long + 1 for sign + 1 for null */
	    errmsg = (char *) xv_malloc(strlen(msg) + 19 + 2);
	    (void) sprintf(errmsg, msg, text_prop->encoding);
	    goto err;

	default:
	    if (state == 0)
		    break;
	    msg = XV_MSG("%d character(s) could not converted from text property");
	    /* 19 holds even for long long + 1 for sign + 1 for null */
	    errmsg = (char *) xv_malloc(strlen(msg) + 19 + 2);
	    (void) sprintf(errmsg, msg, state);
	    alloc_msg = True;
err:
	    if (pkg == NULL) {
	        xv_error(object,
			 ERROR_STRING,	errmsg,
			 NULL);
	    } else {
	        xv_error(object,
			 ERROR_PKG,	pkg,
			 ERROR_STRING,	errmsg,
			 NULL);
	    }
	    if (alloc_msg == True)
		xv_free(errmsg);
	    break;
				
    }

    return state;
}
#endif /* OW_I18N */
