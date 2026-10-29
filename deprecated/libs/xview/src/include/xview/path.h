/*      @(#)path.h 1.6 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *	file for terms of the license.
 */
 

#ifndef xview_path_pkg_DEFINED
#define xview_path_pkg_DEFINED

#include <sys/stat.h>

extern Xv_pkg		path_pkg;
#define PATH_NAME	&path_pkg

typedef Xv_opaque Path_name;

#define PATH_ATTR(type, ordinal)	ATTR(ATTR_PKG_PATH, type, ordinal)

typedef enum {
    __PATH_IS_DIRECTORY	= PATH_ATTR(ATTR_BOOLEAN,	1), /* CGS */
    __PATH_USE_FRAME	= PATH_ATTR(ATTR_BOOLEAN,	2), /* CGS */
    __PATH_RELATIVE_TO	= PATH_ATTR(ATTR_STRING,	3), /* CGS */
    __PATH_LAST_VALIDATED= PATH_ATTR(ATTR_STRING,	4), /* -G- */

#ifdef OW_I18N
    __PATH_RELATIVE_TO_WCS	= PATH_ATTR(ATTR_WSTRING,	6), /* CGS */
    __PATH_LAST_VALIDATED_WCS	= PATH_ATTR(ATTR_WSTRING,	7), /* -G- */
#endif /* OW_I18N */    


    /***************************************************************
    	Private Attributes
    ****************************************************************/
    __PATH_IS_NEW_FILE	= PATH_ATTR(ATTR_BOOLEAN,	5), /* CGS */
} Path_attr;

#define PATH_IS_DIRECTORY ((Attr_attribute) __PATH_IS_DIRECTORY)
#define PATH_USE_FRAME ((Attr_attribute) __PATH_USE_FRAME)
#define PATH_RELATIVE_TO ((Attr_attribute) __PATH_RELATIVE_TO)
#define PATH_LAST_VALIDATED ((Attr_attribute) __PATH_LAST_VALIDATED)

#ifdef OW_I18N
#define PATH_RELATIVE_TO_WCS ((Attr_attribute) __PATH_RELATIVE_TO_WCS)
#define PATH_LAST_VALIDATED_WCS ((Attr_attribute) __PATH_LAST_VALIDATED_WCS)
#endif /* OW_I18N */    

#define PATH_IS_NEW_FILE ((Attr_attribute) __PATH_IS_NEW_FILE)

typedef struct {
    Xv_panel_text	parent_data;
    Xv_opaque		private_data;
} Path_public;


#endif	/* ~xview_path_pkg_DEFINED */

