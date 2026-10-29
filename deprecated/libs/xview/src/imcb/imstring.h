/*      @(#)imstring.h 50.2 93/06/28 SMI      */

/*
 *      (c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *      file for terms of the license.
 */


/*** !!!!!!!!!!  for test ****/
typedef	struct _InputContext {
	Window		win;
	Xv_Window	xv_win;
	XrmDatabase	db;
}  InputContext;

typedef	enum {
	IMPlain		= 0,
	IMPrimary	= 1,
	IMSecondary	= 2,
}  IMTextAttrType;

typedef	struct	_IMString {
	int		len;
	IMTextAttrType	*attr;
	Bool		encoding_is_wchar;
	union {
		char	*text;
		wchar_t	*text_wc;
	} Text_data;
}  IMString;

