/*      @(#)tty_impl.h 20.37 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef _xview_private_ttysw_impl_h_already_included
#define _xview_private_ttysw_impl_h_already_included

/*
 * A tty subwindow is a subwindow type that is used to provide a
 * terminal emulation for teletype based programs.
 */

#include <xview_private/portable.h>	/* tty and pty configuration info */

#ifdef	XV_USE_TERMIOS
#include <termios.h>		/* for POSIX-style tty state structure */
#else
#include <sys/ioctl.h>		/* for BSD-style tty state structures */
#endif

#include <xview/tty.h>
#include <xview/sel_svc.h>
#include <xview_private/i18n_impl.h>

#define TTY_PRIVATE(_t)      XV_PRIVATE(Ttysw, Xv_tty, _t)
#define TTY_PUBLIC(_tty_folio)     XV_PUBLIC(_tty_folio)

#define TTY_VIEW_PRIVATE(_t)     	 XV_PRIVATE(Ttysw_view_object, Xv_tty_view, _t)
#define TTY_VIEW_PUBLIC(_tty_view)	 XV_PUBLIC(_tty_view)

#define IS_TTY(_t) \
	(((Xv_base *)(_t))->pkg == TTY)
	
#define IS_TTY_VIEW(_t) \
	(((Xv_base *)(_t))->pkg == TTY_VIEW)
		
/* BUG: This could be made cleaner.  See 
 * TERMSW_FOLIO_FROM_TERMSW_VIEW_HANDLE in termsw_impl.h 
 */
#define TTY_VIEW_HANDLE_FROM_TTY_FOLIO(_tty_folio_private) \
	 ((Ttysw_view_handle)(((Ttysw_folio)_tty_folio_private)->view)) 
 	     
#define TTY_FOLIO_FROM_TTY_VIEW_HANDLE(_tty_view_private) \
	 ((Ttysw_folio)(((Ttysw_view_handle)_tty_view_private)->folio))
	 
#define TTY_FOLIO_FROM_TTY_VIEW(_tty_view_public) 	\
	 ((Ttysw_folio)					\
	  (((Ttysw_view_handle) TTY_VIEW_PRIVATE(_tty_view_public))->folio))

#define TTY_FROM_TTY_VIEW(_tty_view_public) 		\
	((Tty) TTY_PUBLIC(TTY_FOLIO_FROM_TTY_VIEW(_tty_view_public)))

/*
 * These are the data structures internal to the tty subwindow
 * implementation.  They are considered private to the implementation.
 */

struct cbuf {
    CHAR               *cb_rbp;    /* read pointer */
    CHAR               *cb_wbp;    /* write pointer */
    CHAR               *cb_ebp;    /* end of buffer */
    CHAR                cb_buf[2048];
};

struct input_cbuf {
    CHAR               *cb_rbp;    /* read pointer */
    CHAR               *cb_wbp;    /* write pointer */
    CHAR               *cb_ebp;    /* end of buffer */
    CHAR                cb_buf[8192];
};

struct keymaptab {
    int                 kmt_key;
    int                 kmt_output;
    char               *kmt_to;
};

struct textselpos {
    int			tsp_row;
    int			tsp_col;
#ifdef  OW_I18N
    int			tsp_charpos;
#endif
};

struct ttyselection {
    int                 sel_made;  /* a selection has been made */
    int                 sel_null;  /* the selection is null */
    int                 sel_level; /* see below */
    int                 sel_anchor;/* -1 = left, 0 = none, 1 = right */
    struct textselpos   sel_begin; /* beginning of selection */
    struct textselpos   sel_end;   /* end of selection */
    struct timeval      sel_time;  /* time selection was made */
    Seln_rank		sel_rank;  /* type of selection. primary or secondary */
    int			dehilite_op;  /* Operation for taking down selection */
};

/* selection levels */
#define	SEL_CHAR	0
#define	SEL_WORD	1
#define	SEL_LINE	2
#define	SEL_PARA	3
#define	SEL_MAX		3

extern struct ttyselection	null_ttyselection;

enum ttysw_hdrstate { HS_BEGIN, HS_HEADER, HS_ICON, HS_ICONFILE, HS_FLUSH };

typedef struct ttysubwindow {
    Tty			public_self;		/* Back pointer to the object*/
    struct ttysw_view_object			/* View window */
    			*view;			/* (Pure tty has only one view) */
    Tty_view		current_view_public;	/* This keep trace of the view become ttysw */			
    unsigned		ttysw_flags;
    /* common */
    int                 ttysw_opt;		/* option mask; see ttysw.h */
    struct input_cbuf   ttysw_ibuf;		/* input buffer */
    struct cbuf         ttysw_obuf;		/* output buffer */
    /* pty and subprocess */
    int                 ttysw_pty;	/* master (pty) file descriptor */
    int                 ttysw_tty;	/* slave (tty) file descriptor */
    char		tty_name[20];	/* slave (tty) file name */
    int                 ttysw_ttyslot;		/* ttyslot in utmp for tty */
    /* saved tty mode information: see access functions below */
#   ifdef XV_USE_TERMIOS
    struct termios	termios;
#   else /* XV_USE_TERMIOS */
    struct sgttyb	sgttyb;
    struct tchars	tchars;
    struct ltchars	ltchars;
#   endif /* XV_USE_TERMIOS */
    /*
     * The next two fields record the current pty remote mode state and the
     * remote mode state to which it should be set before processing the next
     * input character.
     */
    int			remote;
    int			pending_remote;
    /* page mode */
    int                 ttysw_lpp;		/* page mode: lines per page */
    /* subprocess */
    int                 ttysw_pidchild;		/* pid of the child */
    /* Caps Lock */
    int                 ttysw_capslocked;
#define TTYSW_CAPSLOCKED	0x01	/* capslocked on mask bit */
#define TTYSW_CAPSSAWESC	0x02	/* saw escape while caps locked */
    /* stuff from old ttytlsw */
    enum ttysw_hdrstate	hdrstate;		/* string trying to load */
    CHAR		*nameptr;               /* namebuf ptr */
    CHAR		namebuf[256];           /* accumulates esc string */
    /* selection */
    int                 ttysw_butdown;		/* which button is down */
    struct ttyselection	ttysw_caret;
    struct ttyselection	ttysw_primary;
    struct ttyselection	ttysw_secondary;
    struct ttyselection	ttysw_shelf;
    caddr_t             ttysw_seln_client;
    /* replaceable ops (return TTY_OK or TTY_DONE) */
    int                 (*ttysw_escapeop) ();	/* handle escape sequences */
    int                 (*ttysw_stringop) ();	/* handle accumulated string */
    int                 (*ttysw_eventop) ();	/* handle input event */
    /* kbd translation */
    struct keymaptab    ttysw_kmt[3 * 16 + 2];	/* Key map list */
    struct keymaptab   *ttysw_kmtp;		/* next empty ttysw_kmt slot */
    int		  	(*layout_proc)(); /* interposed window layout proc */
#ifdef  OW_I18N
    int                 im_first_col;
    int                 im_first_row;
    int                 im_len;
    wchar_t             *im_store;
    XIMFeedback         *im_attr;
    Bool                preedit_state;
    XIC                 ic;
    int			implicit_commit;

    XIMCallback     	start_pecb_struct;
    XIMCallback     	draw_pecb_struct;
    XIMCallback     	done_pecb_struct;
#ifdef FULL_R5
    XIMStyle	xim_style;
#endif /* FULL_R5 */    
#endif
    int			pass_thru_modifiers;  /* Modifiers we don't interpret */
    int			eight_bit_output; /* Print eight bit characters? */
}   Ttysw;

typedef Ttysw		*Ttysw_folio;

typedef struct ttysw_view_object {
    Tty_view		public_self;
    Ttysw_folio		folio;
} Ttysw_view_object;

typedef Ttysw_view_object* 	Ttysw_view_handle;

/* Values for ttysw_flags */
#define TTYSW_FL_FROZEN			0x1
#define TTYSW_FL_IS_TERMSW		0x2
#define TTYSW_FL_IN_PRIORITIZER		0x4

/*
 * Functions, macros, and typedefs for abstracting away differences between
 * termios and old BSD-style tty mode representations.
 */
/*
 * Access functions for tty characteristics.
 */
#ifdef	XV_USE_TERMIOS
#define	tty_gettabs(t)		((t)->termios.c_oflag & XTABS)
#if (!defined(__linux) && !defined(__APPLE__)) || defined(VDSUSP)
#define	tty_getdsuspc(t)	((int) ((t)->termios.c_cc[VDSUSP]))
#else
#define	tty_getdsuspc(t)	((int) -1)
#endif
#define	tty_geteofc(t)		((int) ((t)->termios.c_cc[VEOF]))
#define	tty_geteolc(t)		((int) ((t)->termios.c_cc[VEOL]))
#define	tty_geteol2c(t)		((int) ((t)->termios.c_cc[VEOL2]))
#define	tty_getintrc(t)		((int) ((t)->termios.c_cc[VINTR]))
#define	tty_getlnextc(t)	((int) ((t)->termios.c_cc[VLNEXT]))
#define	tty_getquitc(t)		((int) ((t)->termios.c_cc[VQUIT]))
#define	tty_getrprntc(t)	((int) ((t)->termios.c_cc[VREPRINT]))
#define	tty_getstartc(t)	((int) ((t)->termios.c_cc[VSTART]))
#define	tty_getstopc(t)		((int) ((t)->termios.c_cc[VSTOP]))
#define	tty_getsuspc(t)		((int) ((t)->termios.c_cc[VSUSP]))
#else	/* XV_USE_TERMIOS */
#define	tty_gettabs(t)		((t)->sgttyb.sg_flags & XTABS)
#define	tty_getdsuspc(t)	((int) ((t)->ltchars.t_dsuspc))
#define	tty_geteofc(t)		((int) ((t)->tchars.t_eofc))
#define	tty_geteolc(t)		((int) ((t)->tchars.t_brkc))
#define	tty_geteol2c(t)		((int) ((t)->tchars.t_brkc))
#define	tty_getintrc(t)		((int) ((t)->tchars.t_intrc))
#define	tty_getlnextc(t)	((int) ((t)->ltchars.t_lnextc))
#define	tty_getquitc(t)		((int) ((t)->tchars.t_quitc))
#define	tty_getrprntc(t)	((int) ((t)->ltchars.t_rprntc))
#define	tty_getstartc(t)	((int) ((t)->tchars.t_startc))
#define	tty_getstopc(t)		((int) ((t)->tchars.t_stopc))
#define	tty_getsuspc(t)		((int) ((t)->ltchars.t_suspc))
#endif	/* XV_USE_TERMIOS */
/*
 * Predicates for tty characteristics.
 */
#ifdef	XV_USE_TERMIOS
#define	tty_iscanon(t)		(((t)->termios.c_lflag & ICANON) != 0)
#define	tty_isecho(t)		(((t)->termios.c_lflag & ECHO  ) != 0)
#define tty_issig(t)		(((t)->termios.c_lflag & ISIG  ) != 0)
#else	/* XV_USE_TERMIOS */
#define	tty_iscanon(t)		(((t)->sgttyb.sg_flags & (RAW|CBREAK)) == 0)
#define	tty_isecho(t)		(((t)->sgttyb.sg_flags & ECHO) != 0)
#define tty_issig(t)		(((t)->sgttyb.sg_flags & RAW) == 0)
#endif	/* XV_USE_TERMIOS */
/*
 * Capture fd's current tty modes and store them in *mode.
 */
#ifdef	XV_USE_TERMIOS
#define	tty_mode	termios		/* Ttysw field alias (ugh!) */
typedef struct termios	tty_mode_t;
#else	/* XV_USE_TERMIOS */
#define	tty_mode	sgttyb		/* Ttysw field alias (ugh!) */
typedef struct sgttyb	tty_mode_t;
#endif	/* XV_USE_TERMIOS */

/*
 * Determine where to store tty characteristics in the environment.  To avoid
 * possible misinterpretation, we use different locations depending on whether
 * or not XV_USE_TERMIOS is set.
 */
#ifdef	XV_USE_TERMIOS
#define	WE_TTYPARMS	"WINDOW_TERMIOS"
#define	WE_TTYPARMS_E	"WINDOW_TERMIOS="
#else	/* XV_USE_TERMIOS */
#define	WE_TTYPARMS	"WINDOW_TTYPARMS"
#define	WE_TTYPARMS_E	"WINDOW_TTYPARMS="
#endif	/* XV_USE_TERMIOS */


#define TTYSW_NULL      ((Ttysw *)0)

/*
 * Possible return codes from replaceable ops. 
 */
#define	TTY_OK		(0)	   /* args should be handled as normal */
#define	TTY_DONE	(1)	   /* args have been fully handled */

#define	ttysw_handleevent(ttysw, ie) \
	(*(ttysw)->ttysw_eventop)(TTY_PUBLIC(ttysw), (ie))
#define	ttysw_handleescape(_ttysw_view, c, ac, av) \
	(*(ttysw)->ttysw_escapeop)(TTY_VIEW_PUBLIC(_ttysw_view), (c), (ac), (av))
#define	ttysw_handlestring(ttysw, strtype, c) \
	(*(ttysw)->ttysw_stringop)(TTY_PUBLIC(ttysw), (strtype), (c))


/*** XView private routines ***/
Xv_private void
	tty_background(),
	tty_copyarea(),
	tty_newtext(),
	tty_clear_clip_rectangles();

#define MAX_LINES 128
typedef struct {
	int	caret_line_exposed:1;
	int	caret_line;
	int	leftmost;
	char	line_exposed[MAX_LINES];
} Tty_exposed_lines;
Xv_private Tty_exposed_lines *tty_calc_exposed_lines();
Xv_private int ttysw_view_obscured;



/*** Package private routines ***/

Pkg_private void
	csr_resize(),		/* BUG ALERT: No XView prefix */
	delete_lines(),		/* BUG ALERT: No XView prefix */
	termsw_caret_cleared(),
	termsw_menu_set(),
	termsw_menu_clr(),
	ttynullselection(),	/* BUG ALERT: No XView prefix */
	ttysetselection(),	/* BUG ALERT: No XView prefix */
	ttysel_acquire(),
	ttysel_adjust(),
	ttysel_deselect(),
	ttysel_destroy(),
	ttysel_getselection(),
	ttysel_init_client(),
	ttysel_make(),
	ttysel_nullselection(),
	ttysel_setselection(),
	ttysw_ansiinit(),
	ttysw_blinkscreen(),
	ttysw_bold_mode(),
	ttysw_cim_clear(),
	ttysw_cim_scroll(),
	ttysw_clear(),
	ttysw_clear_mode(),
	ttysw_consume_output(),
	ttysw_deleteChar(),
	ttysw_display(),
	ttysw_display_capslock(),
	ttysw_doing_pty_insert(),
	ttysw_drawCursor(),
	ttysw_flush_input(),
	ttysw_getp(),
	ttysw_handle_itimer(),
	ttysw_imagerepair(),
	ttysw_implicit_commit(),
	ttysw_insertChar(),
	ttysw_insert_lines(),
	ttysw_interpose(),
	ttysw_interpose_on_textsw(),
	ttysw_inverse_mode(),
	ttysw_lighten_cursor(),
	ttysw_move_mark(),
	ttysw_pclearline(),
	ttysw_pclearscreen(),
	ttysw_pcopyline(),
	ttysw_pcopyscreen(),
	ttysw_pdisplayscreen(),
	ttysw_pos(),
	ttysw_prepair(),
	ttysw_pselectionhilite(),
	ttysw_pstring(),
	ttysw_pty_input(),
	ttysw_readrc(),
	ttysw_removeCursor(),
	ttysw_reset_conditions(),
	ttysw_resize(),
	ttysw_restore_cursor(),
	ttysw_restoreCursor(),	/* BUG ALERT: unnecessary routine */
	ttysw_saveCursor(),	/* BUG ALERT: unnecessary routine */
	ttysw_screencomp(),	/* BUG ALERT: unnecessary routine */
	ttysw_sendsig(),
	ttysw_set_inverse_mode(),
	ttysw_set_menu(),
	ttysw_set_underline_mode(),
	ttysw_setleftmargin(),
	ttysw_setopt(),
	ttysw_show_walkmenu(),
	ttysw_sigwinch(),
	ttysw_textsw_changed(),
	ttysw_underscore_mode(),
	ttysw_vpos(),
	ttysw_writePartialLine(),
	we_setptyparms(),	/* BUG ALERT: No XView prefix */
	xv_new_tty_chr_font(),
	xv_tty_free_image_and_mode(),
	xv_tty_imagealloc(),
	xv_tty_new_size();

#ifdef OW_I18N
Pkg_private void
	tty_column_wchar_type();
#endif

Pkg_private int
	tty_folio_init(),
	tty_getmode(),
	tty_view_init(),
	ttysw_ansi_escape(),
	ttysw_ansi_string(),
	ttysw_be_termsw(),
	ttysw_be_ttysw(),
	ttysw_cooked_echo_mode(),
	ttysw_destroy(),
	ttysw_do_copy(),
	ttysw_do_paste(),
	ttysw_domap(),
	ttysw_fork_it(),
	ttysw_freeze(),
	ttysw_getboldstyle(),
	ttysw_getopt(),
	ttysw_input_it(),
	ttysw_lookup_boldstyle(),
	ttysw_mapsetim(),
	ttysw_output_it(),
	ttysw_print_bold_options(),
	ttysw_pty_output(),
	ttysw_pty_output_ok(),
	ttysw_restoreparms(),
	ttysw_saveparms(),
	ttysw_scan_for_completed_commands(),
	ttysw_setboldstyle(),
	ttytlsw_escape(),	/* BUG ALERT: No XView prefix */
	ttytlsw_string(),	/* BUG ALERT: No XView prefix */
	wininit(),		/* BUG ALERT: No XView prefix */
	xv_tty_imageinit();

#ifdef OW_I18N
Pkg_private int
	tty_character_size(),
	tty_get_nchars(),
	ttysw_input_it_wcs();
#endif

Pkg_private Notify_value
	ttysw_itimer_expired(),
	ttysw_pty_input_pending(),
	ttysw_text_event();

Pkg_private Xv_opaque
	ts_create(),		/* BUG ALERT: No XView prefix */
	ttysw_init_folio_internal(),
	ttysw_init_view_internal(),
	ttysw_walkmenu();


#ifdef	cplus
/*
 * C Library routines specifically related to private ttysw subwindow
 * functions.  ttysw_output and ttysw_input return the number of characters
 * accepted/processed (usually equal to len). 
 */
int 
ttysw_output(Tty ttysw_public, char *addr, int len);

/* Interpret string in terminal emulator. */
int 
ttysw_input(Tty ttysw_public, char *addr, int len);

/* Add string to the input queue. */
#endif /*	cplus */

#endif /* _xview_private_ttysw_impl_h_already_included */
