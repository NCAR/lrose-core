/*      @(#)hist_impl.h 1.7 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *	file for terms of the license.
 */

#include <xview/openmenu.h>
#include <xview_private/xv_path_util.h>
#include <xview/hist.h>




/********* History List definitions **********/

#define DEFAULT_ROLL_MAX	15

struct history_list {
    Xv_opaque		public_self;
    Xv_server		server;
    int			duplicate_labels;	/* allow dup labels */
    int			duplicate_values;	/* allow dup values */
    Menu_item		blank;

    /*
     * Static Space is a simple linked list
     */
    int			fixed_count;
    struct hist_entry *	fixed_first;
    struct hist_entry *	fixed_last;


    /*
     * define Rolling Space as a stack 
     * that rolls off after max entries
     */
    int			roll_count;		/* actual num entries */
    int			roll_max; 		/* max num entries */
    struct hist_entry *	roll_first;		/* first node in list */
    struct hist_entry *	roll_last; 		/* last node in list */

    struct history_list	*next;			/* keep list for xv_find */
};

typedef struct history_list	History_list_private;


struct hist_entry {
    Menu_item		mi;	
    char *		label;	/* string displayed */
    char *		value;	/* string interpreted */
    struct hist_entry *	next;
    struct hist_entry *	prev;
#ifdef OW_I18N
    wchar_t *		label_wcs;
    wchar_t *		value_wcs;
#endif	/* OW_I18N */
};


#define HIST_LIST_PUBLIC(item)	XV_PUBLIC(item)
#define HIST_LIST_PRIVATE(item)	XV_PRIVATE(History_list_private, History_list_public, item)



/********* History Menu definitions **********/

typedef struct {
    Xv_opaque		public_self;
    History_list	list;
    Menu		menu;
    void		(* notify_proc)();
#ifdef OW_I18N
    void		(* notify_proc_wcs)();
#endif
} History_menu_private;


#define HIST_MENU_PUBLIC(item)	XV_PUBLIC(item)
#define HIST_MENU_PRIVATE(item)	XV_PRIVATE(History_menu_private, History_menu_public, item)



