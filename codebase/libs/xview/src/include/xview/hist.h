/*      @(#)hist.h 1.5 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *	file for terms of the license.
 */

#ifndef xview_history_pkg_DEFINED
#define xview_history_pkg_DEFINED


extern Xv_pkg		history_menu_pkg;
extern Xv_pkg		history_list_pkg;
#define HISTORY_MENU	&history_menu_pkg
#define HISTORY_LIST	&history_list_pkg


typedef Xv_opaque	History_list;
typedef Xv_opaque	History_menu;



#define HIST_ATTR(type, ordinal)	ATTR(ATTR_PKG_HIST, type, ordinal)

typedef enum {
    /*
     * Public History_menu attributes
     */
    __HISTORY_MENU_OBJECT	= HIST_ATTR(ATTR_NO_VALUE,	1), /* -G- */
    __HISTORY_NOTIFY_PROC	= HIST_ATTR(ATTR_FUNCTION_PTR,	2), /* CGS */
    __HISTORY_MENU_HISTORY_LIST	= HIST_ATTR(ATTR_OPAQUE,	3), /* CGS */


    /*
     * Public History_list Attributes
     */
    __HISTORY_ADD_FIXED_ENTRY	= HIST_ATTR(ATTR_OPAQUE_PAIR,	4), /* C-S */
    __HISTORY_ADD_ROLLING_ENTRY	= HIST_ATTR(ATTR_OPAQUE_PAIR,	5), /* C-S */
    __HISTORY_ROLLING_MAXIMUM	= HIST_ATTR(ATTR_INT,		6), /* CGS */
    __HISTORY_DUPLICATE_LABELS	= HIST_ATTR(ATTR_BOOLEAN,	7), /* CGS */
    __HISTORY_DUPLICATE_VALUES	= HIST_ATTR(ATTR_BOOLEAN,	8), /* CGS */
    __HISTORY_FIXED_COUNT	= HIST_ATTR(ATTR_INT,		9), /* -G- */
    __HISTORY_ROLLING_COUNT	= HIST_ATTR(ATTR_INT,		10), /* -G- */
    __HISTORY_VALUE		= HIST_ATTR(ATTR_INT_PAIR,	11), /* -G- */
    __HISTORY_LABEL		= HIST_ATTR(ATTR_INT_PAIR,	12), /* -G- */
    __HISTORY_INACTIVE		= HIST_ATTR(ATTR_INT_TRIPLE,	13), /* -GS */

#ifdef OW_I18N
    /*
     * Wide-Char Interface
     */
    __HISTORY_ADD_FIXED_ENTRY_WCS 	= HIST_ATTR(ATTR_OPAQUE_PAIR,	16), /* C-S */
    __HISTORY_ADD_ROLLING_ENTRY_WCS	= HIST_ATTR(ATTR_OPAQUE_PAIR,	17), /* C-S */
    __HISTORY_VALUE_WCS			= HIST_ATTR(ATTR_INT_PAIR,	18), /* -G- */
    __HISTORY_LABEL_WCS			= HIST_ATTR(ATTR_INT_PAIR,	19), /* -G- */
    __HISTORY_NOTIFY_PROC_WCS		= HIST_ATTR(ATTR_FUNCTION_PTR,	20), /* CGS */
#endif  /* OW_I18N */


    /*
     * Private History_list Attributes, used by History_menu package.
     */
    __HISTORY_POPULATE_MENU	  = HIST_ATTR(ATTR_OPAQUE,	14),
    __HISTORY_VALUE_FROM_MENUITEM = HIST_ATTR(ATTR_OPAQUE,	15),
} History_attr;

#define HISTORY_MENU_OBJECT ((Attr_attribute) __HISTORY_MENU_OBJECT)
#define HISTORY_NOTIFY_PROC ((Attr_attribute) __HISTORY_NOTIFY_PROC)
#define HISTORY_MENU_HISTORY_LIST ((Attr_attribute) __HISTORY_MENU_HISTORY_LIST)
#define HISTORY_ADD_FIXED_ENTRY ((Attr_attribute) __HISTORY_ADD_FIXED_ENTRY)
#define HISTORY_ADD_ROLLING_ENTRY ((Attr_attribute) __HISTORY_ADD_ROLLING_ENTRY)
#define HISTORY_ROLLING_MAXIMUM ((Attr_attribute) __HISTORY_ROLLING_MAXIMUM)
#define HISTORY_DUPLICATE_LABELS ((Attr_attribute) __HISTORY_DUPLICATE_LABELS)
#define HISTORY_DUPLICATE_VALUES ((Attr_attribute) __HISTORY_DUPLICATE_VALUES)
#define HISTORY_FIXED_COUNT ((Attr_attribute) __HISTORY_FIXED_COUNT)
#define HISTORY_ROLLING_COUNT ((Attr_attribute) __HISTORY_ROLLING_COUNT)
#define HISTORY_VALUE ((Attr_attribute) __HISTORY_VALUE)
#define HISTORY_LABEL ((Attr_attribute) __HISTORY_LABEL)
#define HISTORY_INACTIVE ((Attr_attribute) __HISTORY_INACTIVE)

#ifdef OW_I18N

#define HISTORY_ADD_FIXED_ENTRY_WCS ((Attr_attribute) __HISTORY_ADD_FIXED_ENTRY_WCS)
#define HISTORY_ADD_ROLLING_ENTRY_WCS ((Attr_attribute) __HISTORY_ADD_ROLLING_ENTRY_WCS)
#define HISTORY_VALUE_WCS ((Attr_attribute) __HISTORY_VALUE_WCS)
#define HISTORY_LABEL_WCS ((Attr_attribute) __HISTORY_LABEL_WCS)
#define HISTORY_NOTIFY_PROC_WCS ((Attr_attribute) __HISTORY_NOTIFY_PROC_WCS)

#endif  /* OW_I18N */

#define HISTORY_POPULATE_MENU ((Attr_attribute) __HISTORY_POPULATE_MENU)
#define HISTORY_VALUE_FROM_MENUITEM ((Attr_attribute) __HISTORY_VALUE_FROM_MENUITEM)


/*
 * Used with HISTORY_VALUE, HISTORY_LABEL
 * and HISTORY_INACTIVE attributes.
 */
typedef enum {
    HISTORY_FIXED,
    HISTORY_ROLLING
} History_space;



typedef struct {
    Xv_generic_struct	parent_data;
    Xv_opaque		private_data;
} History_menu_public;

typedef struct {
    Xv_generic_struct	parent_data;
    Xv_opaque		private_data;
} History_list_public;


#endif	/* ~xview_history_pkg_DEFINED */
