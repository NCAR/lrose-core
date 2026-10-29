/*	@(#)term_impl.h 20.32 93/06/28 SMI	*/

/****************************************************************************/
/*	
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license. 
 */
/****************************************************************************/

#ifndef _xview_private_termsw_impl_h_already_included
#define _xview_private_termsw_impl_h_already_included

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview/pkg_public.h>
#include <xview/openmenu.h>
#include <xview/termsw.h>
#include <xview/panel.h>
#include <xview_private/tty_impl.h>
#ifdef	XV_USE_TTCOMPAT
#include <sys/stream.h>
#include <sys/ttold.h>
#include <sys/ttcompat.h>
#endif

/*			Implementation Overview.
 *
 *	A command subwindow is a misnomer, for the same reason that text
 * subwindow is a misnomer.  In both cases, the "subwindow" is actually
 * one or more subwindows, all of which view the same text.  Just as the
 * text views share a common folio, which represents the text being
 * manipulated and all of the associated state, so the Termsw views also
 * share a common folio.  The Termsw_folio represents additional Termsw
 * package-specific state associated with the underlying text.
 *	Every Termsw view is also a Ttysw, in that each view responds to
 * TTY attributes.  In addition, at most one of the Termsw views may actually
 * be a full-fledged Ttysw including using Ttysw repaint routines, etc. if
 * a program such as vi is being run.  In actuality, there is a single Ttysw
 * associated with all of the Termsw views, and the Ttysw's idea of its
 * display surface, size, etc., is dynamically changed to the appropriate
 * Termsw view.
 *	Through trickery in termsw_init/set_internal/get_internal/destroy,
 * and taking advantage of the conformal types of the Textsw and Ttysw
 * structures, a Termsw is simultaneously conformal with both Textsw and Ttysw.
 * Thus, a Termsw can be used as an argument to both Textsw or Ttysw routines.
 * The catch is that direct calls from external agents (e.g., the Notifier),
 * do not pass through the code in the Termsw package that massages the
 * Termsw to appear as a "vanilla" Ttysw instead of a "vanilla" Textsw and thus
 * such functions must be written to expect a Termsw in addition to a Ttysw.
 */

#define TERMSW_PRIVATE(_termsw_folio_public)	XV_PRIVATE(Termsw_folio_object, Xv_termsw, _termsw_folio_public)
#define TERMSW_PUBLIC(_termsw_folio_private)   	XV_PUBLIC(_termsw_folio_private)

#define TERMSW_VIEW_PRIVATE(_termsw_view_public)	XV_PRIVATE(Termsw_view_object, Xv_termsw_view, _termsw_view_public)
#define TERMSW_VIEW_PUBLIC(_termsw_view_private)  XV_PUBLIC(_termsw_view_private)

#define TERMSW_VIEW_FROM_TERMSW_FOLIO(_termsw_folio) \
	((_termsw_folio)->first_view)

#define TERMSW_VIEW_PRIVATE_FROM_TEXTSW(_abs_public)	\
		(IS_TERMSW(_abs_public) ?		\
		TERMSW_VIEW_FROM_TERMSW_FOLIO(TERMSW_PRIVATE(_abs_public)) : \
		TERMSW_VIEW_PRIVATE(_abs_public))

#define TEXTSW_PRIVATE_FROM_TERMSW(_termsw_public)	\
		(((Xv_termsw *)(_termsw_public))->private_text)
	
#define TERMSW_VIEW_PRIVATE_FROM_TTY_PRIVATE(_private_abs)		\
		(IS_TERMSW(XV_PUBLIC(_private_abs)) ?			\
	          TERMSW_VIEW_FROM_TERMSW_FOLIO(			\
	            TERMSW_PRIVATE(TTY_PUBLIC(_private_abs))) :		\
	            TERMSW_VIEW_PRIVATE(TTY_VIEW_PUBLIC(_private_abs)))
	            
#define TERMSW_PRIVATE_FROM_TTY_PRIVATE(_private_abs)			\
    		(IS_TERMSW(XV_PUBLIC(_private_abs)) ?			\
	            TERMSW_PRIVATE(TTY_PUBLIC(_private_abs)) :		\
	            TERMSW_PRIVATE(TTY_PUBLIC(((Ttysw_view_handle)_private_abs)->folio)))	


#define TEXTSW_FROM_TTY(ttysw)	(Textsw)(TTY_PUBLIC(ttysw))

/*
 * Unless and until a ttysw/termsw object goes through the initialization code
 * in termsw_folio_init_internal, it cannot become a termsw.  The
 * TTY_IS_TERMSW macro records whether or not the object has termsw-related
 * state and therefore can switch over to act as a termsw.
 */
#define TTY_IS_TERMSW(ttysw)	(ttysw->ttysw_flags & TTYSW_FL_IS_TERMSW)

#define TERMSW_FOLIO_FROM_TERMSW_VIEW_HANDLE(_termsw_view_private) \
	 ((_termsw_view_private)->folio)
	 
#define TERMSW_FOLIO_FROM_TERMSW_VIEW(_termsw_view_public) 	\
	    TERMSW_FOLIO_FROM_TERMSW_VIEW_HANDLE(			\
	           TERMSW_VIEW_PRIVATE(_termsw_view_public))	

#define TERMSW_FROM_TERMSW_VIEW(_termsw_view_public) 		\
	(TERMSW_PUBLIC(TERMSW_FOLIO_FROM_TERMSW_VIEW(_termsw_view_public)))


/* In general, the tty private data is available by applying TTY_PRIVATE to
 * the public object.  However, for direct calls from the Notifier, the
 * Textsw, the Ttysw client, etc., the tty data must be accessed either from
 * the public objedt or directly from the termsw private data, depending on
 * the value of the public object's package.
 */
#define TTY_PRIVATE_TERMSW(_termsw_public)	\
		((Ttysw_folio)((Xv_termsw *)(_termsw_public))->private_tty)

#define TTY_VIEW_PRIVATE_FROM_TERMSW_VIEW(_termsw_view_public)		\
		((Ttysw_view_handle)					\
		   ((Xv_termsw_view *)(_termsw_view_public))->private_tty)						
#define TTY_PRIVATE_FROM_TERMSW_VIEW(_termsw_view_public)		\
		(TTY_FOLIO_FROM_TTY_VIEW_HANDLE(		\
		     TTY_VIEW_PRIVATE_FROM_TERMSW_VIEW(_termsw_view_public)))

#define TTY_VIEW_PRIVATE_FROM_ANY_VIEW(_abs_view)		\
		(IS_TTY_VIEW(_abs_view) 			\
		    ? TTY_VIEW_PRIVATE(_abs_view)		 \
		    : TTY_VIEW_PRIVATE_FROM_TERMSW_VIEW(_abs_view) )
		
#define TTY_FROM_TERMSW(_termsw_public)	TTY_PRIVATE_TERMSW(_termsw_public)


#define IS_TERMSW(_t) \
	(((Xv_base *)(_t))->pkg == TERMSW)
	
#define IS_TERMSW_VIEW(_t) \
	(((Xv_base *)(_t))->pkg == TERMSW_VIEW)


#define TTY_PRIVATE_FROM_ANY_FOLIO(_abs)		\
		(IS_TTY(_abs) ? TTY_PRIVATE(_abs)	\
		  : TTY_PRIVATE_TERMSW(_abs) )
		  
#define TTY_PRIVATE_FROM_ANY_VIEW(_abs)				\
		(IS_TTY_VIEW(_abs) ? 				\
		    TTY_FOLIO_FROM_TTY_VIEW(_abs) \
		     : TTY_PRIVATE_FROM_TERMSW_VIEW(_abs) )
		  		  
#define	TTY_PRIVATE_FROM_ANY_PUBLIC(_abs)			\
		((IS_TTY(_abs) ||  IS_TERMSW(_abs)) ? 		\
		       TTY_PRIVATE_FROM_ANY_FOLIO(_abs) : 	\
		        TTY_PRIVATE_FROM_ANY_VIEW(_abs) )

#define TTY_VIEW_PRIVATE_FROM_TTY(_ttysw_folio_public)		\
		((Ttysw_view_handle)((Ttysw_folio) TTY_PRIVATE(_ttysw_folio_public))->view)

#define TTY_VIEW_PRIVATE_FROM_TERMSW(_termsw_folio_public)		\
		(TTY_VIEW_HANDLE_FROM_TTY_FOLIO(TTY_PRIVATE_TERMSW(_termsw_folio_public)))

#define TTY_VIEW_PRIVATE_FROM_ANY_FOLIO(_abs)		\
		(IS_TTY(_abs) ? TTY_VIEW_PRIVATE_FROM_TTY(_abs)	\
		  : TTY_VIEW_PRIVATE_FROM_TERMSW(_abs) )
		  
#define	TTY_VIEW_PRIVATE_FROM_ANY_PUBLIC(_abs)			\
		((IS_TTY(_abs) ||  IS_TERMSW(_abs)) ? 		\
		       TTY_VIEW_PRIVATE_FROM_ANY_FOLIO(_abs) : 	\
		        TTY_VIEW_PRIVATE_FROM_ANY_VIEW(_abs) )

		        
		        
#define TEXTSW_MODE	1
#define TTYSW_MODE	2


/* Termsw's folio data structure */
typedef struct {
    Termsw		public_self;
    struct termsw_view_object	
    			*first_view;
    Menu		text_menu;
    Menu		tty_menu;
    Textsw_mark		user_mark;
    Textsw_mark		pty_mark;
    Textsw_mark		read_only_mark;   /* Valid iff append_only_log */
    caddr_t		next_undo_point;
    unsigned char	view_count;
    char		erase_line;
    char		erase_word;
    char		erase_char;
    int			history_limit;    /* save while in !cooked_echo*/
    int			pty_eot;	  /* # of remaining chars from pty */
		/* Various state booleans */
    unsigned		append_only_log		: 1;
    unsigned		cmd_started		: 1;
    unsigned		cooked_echo		: 1;
    unsigned		doing_pty_insert	: 1;
    unsigned		pty_owes_newline	: 1;
    unsigned		ttysw_resized		: 1;
    unsigned		literal_next		: 1;
    unsigned		ok_to_enable_scroll	: 1;
    int			(*layout_proc)(); /* interposed window layout proc */
    /* For Textedit */
    Textsw		textedit;
    Panel		textedit_panel;

#ifdef OW_I18N
    XIC                 ic;             /* This IC is created by textsw */
#endif

} Termsw_folio_object;
typedef Termsw_folio_object	*Termsw_folio;


/* Termsw's view data structure */
typedef struct termsw_view_object {
    Termsw_view		public_self;	/* Back pointer to the object*/
    Termsw_folio		folio;
    struct termsw_view_object	*next;	
} Termsw_view_object;
typedef Termsw_view_object	*Termsw_view_handle;

#define	Termsw_private		Termsw_view_object


#define TERMSW_FOLIO_FOR_VIEW(_termsw_view_handle)   	\
			TERMSW_FOLIO_FROM_TERMSW_VIEW_HANDLE(_termsw_view_handle)

Pkg_private int		tty_notice_key;

/* to obviate #ifdefs elsewhere... */
#ifdef	OW_I18N
#define	TEXTSW_LENGTH_I18N		TEXTSW_LENGTH_WC
/* This attribute dosen't do implicit commit. */
#define	TEXTSW_CONTENTS_I18N		TEXTSW_CONTENTS_WCS_NO_COMMIT
#define	textsw_replace_i18n		textsw_replace_wcs
#define	textsw_find_i18n		textsw_find_wcs
#define	TEXTSW_FIRST_I18N		TEXTSW_FIRST_WC
#define	TEXTSW_INSERTION_POINT_I18N	TEXTSW_INSERTION_POINT_WC
#define	textsw_find_mark_i18n		textsw_find_mark_wc
#define	textsw_add_mark_i18n		textsw_add_mark_wc
#define	textsw_erase_i18n		textsw_erase_wcs
#define	textsw_delete_i18n		textsw_delete_wcs
#else /* OW_I18N */
#define	TEXTSW_LENGTH_I18N		TEXTSW_LENGTH
#define	TEXTSW_CONTENTS_I18N		TEXTSW_CONTENTS
#define	textsw_replace_i18n		textsw_replace_bytes
#define	textsw_find_i18n		textsw_find_bytes
#define	TEXTSW_FIRST_I18N		TEXTSW_FIRST
#define	TEXTSW_INSERTION_POINT_I18N	TEXTSW_INSERTION_POINT
#define	textsw_find_mark_i18n		textsw_find_mark
#define	textsw_add_mark_i18n		textsw_add_mark
#define	textsw_erase_i18n		textsw_erase
#define	textsw_delete_i18n		textsw_delete
#endif /* OW_I18N */

#endif /* _xview_private_termsw_impl_h_already_included */
