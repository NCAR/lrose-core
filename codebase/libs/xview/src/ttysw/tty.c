#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)tty.c 20.64 93/06/28";
#endif
#endif

/*****************************************************************/
/* tty.c                           */
/*	
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license. 
 */
/*****************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pixrect/pixrect.h>
#include <pixrect/pixfont.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>
#include <xview/sun.h>
#include <xview/frame.h>
#include <xview/tty.h>
#include <xview/ttysw.h>
#include <xview/textsw.h>
#include <xview/termsw.h>
#include <xview/defaults.h>
#include <xview_private/term_impl.h>
#include <xview/scrollbar.h>
#include <xview_private/charscreen.h>

#ifdef SVR4
#include <sys/suntty.h>
#include <sys/strredir.h>
#endif

#define	_NOTIFY_MIN_SYMBOLS
#include <xview/notify.h>
#undef	_NOTIFY_MIN_SYMBOLS

#include <xview_private/draw_impl.h>

#define HELP_INFO(s) XV_HELP_DATA, s,

extern char    *getenv();
extern caddr_t  textsw_checkpoint_undo();
extern Attr_avlist attr_find();

static int      tty_quit_on_death(), tty_handle_death();

Pkg_private Xv_Window csr_pixwin;
Pkg_private Notify_value ttysw_event();

Pkg_private Menu ttysw_walkmenu();
Pkg_private Ttysw *ttysw_init_internal();

Pkg_private int tty_folio_init();
Pkg_private Xv_opaque ttysw_folio_set();
Pkg_private Xv_opaque ttysw_folio_get();
Pkg_private int ttysw_folio_destroy();

Pkg_private int tty_view_init();
Pkg_private Xv_opaque ttysw_view_set();
Pkg_private Xv_opaque ttysw_view_get();
Pkg_private int ttysw_view_destroy();

static Pixfont* change_font;



/*****************************************************************************/
/* Ttysw init routines for folio and  view	                             */
/*****************************************************************************/
/*ARGSUSED*/
Pkg_private int
tty_folio_init(parent, tty_public, avlist)
    Xv_Window       parent;
    Tty             tty_public;
    Tty_attribute   avlist[];
{
    Ttysw_folio     ttysw;	/* Private object data */
#ifdef OW_I18N
    Xv_private void		tty_text_start();
    Xv_private void		tty_text_done();
    Xv_private void		tty_text_draw();
#endif

    if (!tty_notice_key)  {
	tty_notice_key = xv_unique_key();
    }

    ttysw = (Ttysw_folio) (ttysw_init_folio_internal(tty_public));
    if (!ttysw)
	return (XV_ERROR);

#ifdef OW_I18N
    if ( xv_get(tty_public, WIN_USE_IM) ) {
        /* Set preedit callbacks */
        xv_set(tty_public,
		WIN_IC_PREEDIT_START,
			(XIMProc)tty_text_start,
			(XPointer)tty_public,
		NULL);
 
        xv_set(tty_public, 
		WIN_IC_PREEDIT_DRAW,
			(XIMProc)tty_text_draw,
			(XPointer)tty_public,
		NULL);
 
        xv_set(tty_public,
		WIN_IC_PREEDIT_DONE,
			(XIMProc)tty_text_done,
			(XPointer)tty_public,
		NULL);

	ttysw->start_pecb_struct.client_data = (XPointer)tty_public;
	ttysw->start_pecb_struct.callback = (XIMProc)tty_text_start;

	ttysw->draw_pecb_struct.client_data = (XPointer)tty_public;
	ttysw->draw_pecb_struct.callback = (XIMProc)tty_text_draw;

	ttysw->done_pecb_struct.client_data = (XPointer)tty_public;
	ttysw->done_pecb_struct.callback = (XIMProc)tty_text_done;
    }
#endif
    ttysw->pass_thru_modifiers = (int)xv_get(XV_SERVER_FROM_WINDOW(parent),
						   SERVER_CLEAR_MODIFIERS);
    ttysw->eight_bit_output = (int)defaults_get_boolean("ttysw.eightBitOutput",
						  "Ttysw.EightBitOutput", TRUE);

    ttysw->hdrstate = HS_BEGIN;
    ttysw->ttysw_stringop = ttytlsw_string;
    ttysw->ttysw_escapeop = ttytlsw_escape;
    (void) xv_set(tty_public,
		  WIN_MENU, ttysw_walkmenu(tty_public),
		  NULL);
    ttysw_interpose(ttysw);
    return (XV_OK);
}

/*ARGSUSED*/
Pkg_private int
tty_view_init(parent, tty_view_public, avlist)
    Xv_Window       parent;	/* Tty public folio */
    Tty_view        tty_view_public;
    Tty_attribute   avlist[];
{
    Ttysw_view_handle ttysw_view;	/* Private object data */

    if (!tty_notice_key)  {
	tty_notice_key = xv_unique_key();
    }

    /*
     * BUG ALERT!  Re-arrange code to pass this pixwin into the appropriate
     * layer instead of just smashing it set from here!
     */
    csr_pixwin = tty_view_public;


    ttysw_view = (Ttysw_view_handle) (ttysw_init_view_internal(parent, tty_view_public));

    if (!ttysw_view)
	return (XV_ERROR);


    /* ttysw_walkmenu() can only be called after public self linked to */
    (void) xv_set(tty_view_public,
		  WIN_NOTIFY_SAFE_EVENT_PROC, ttysw_event,
		  WIN_NOTIFY_IMMEDIATE_EVENT_PROC, ttysw_event,
		  NULL);

    /* ttysw_interpose(ttysw_view); */

    /* Draw cursor on the screen and retained portion */
#ifdef OW_I18N
#ifdef FULL_R5
    if (IS_TTY_VIEW(tty_view_public))
#endif 
#endif
        (void) ttysw_drawCursor(0, 0);
    return (XV_OK);
}




/***************************************************************************
ttysw_set_internal
*****************************************************************************/
static          Xv_opaque
#ifdef OW_I18N
ttysw_set_internal(tty_public, avlist, is_folio)
    int			is_folio;
#else
ttysw_set_internal(tty_public, avlist)
#endif
    Tty             tty_public;
    Attr_attribute  avlist[];
{
    Ttysw_folio     ttysw = TTY_PRIVATE_FROM_ANY_PUBLIC(tty_public);
    register Attr_avlist attrs;
    static int      quit_tool;
    int             pid = -1, bold_style = -1, argv_set = 0;
    char          **argv = 0;
    int             do_fork = FALSE;
    char           *buf;
    int            *buf_used;
    int             buf_len;
    Xv_Drawable_info *info;
#ifdef OW_I18N
    Tty		ttysw_pub;
#endif

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (attrs[0]) {

	  case TTY_ARGV:
	    do_fork = TRUE;
	    argv_set = 1;
	    argv = (char **) attrs[1];
	    break;

	  case TTY_CONSOLE:
	    if (attrs[1]) {
#ifdef sun			/* Vaxen do not support the TIOCCONS ioctl */
#ifdef SVR4
                int consfd;

                if ((consfd=open("/dev/console", O_RDONLY)) == -1)
                    xv_error( tty_public,
                              ERROR_STRING, "open of /dev/console failed",
                              ERROR_LAYER, ERROR_SYSTEM,
                              ERROR_PKG, TTY,
                              NULL );

                else if ((ioctl(consfd, SRIOCSREDIR, ttysw->ttysw_tty)) == -1)
                    xv_error( tty_public,
                              ERROR_STRING,
      "ioctl SRIOCSREDIR returned -1, attempt to make tty the console failed",
                              ERROR_LAYER, ERROR_SYSTEM,
                              ERROR_PKG, TTY,
                              NULL );

#else
		if ((ioctl(ttysw->ttysw_tty, TIOCCONS, 0)) == -1)
                    xv_error( tty_public,
                              ERROR_STRING,
      "ioctl TIOCCONS returned -1, attempt to make tty the console failed",
                              ERROR_LAYER, ERROR_SYSTEM,
                              ERROR_PKG, TTY,
                              NULL );
#endif
#endif
	    };
	    break;

	  case TTY_INPUT:
	    buf = (char *) attrs[1];
	    buf_len = (int) attrs[2];
	    buf_used = (int *) attrs[3];
	    *buf_used = ttysw_input_it(ttysw, buf, buf_len);
	    break;

	  case TTY_PAGE_MODE:
	    (void) ttysw_setopt(TTY_VIEW_HANDLE_FROM_TTY_FOLIO(ttysw), TTYOPT_PAGEMODE, (int)
				(attrs[1]));
	    break;

	  case TTY_QUIT_ON_CHILD_DEATH:
	    quit_tool = (int) attrs[1];
	    break;

	  case TTY_BOLDSTYLE:
	    (void) ttysw_setboldstyle((int) attrs[1]);
	    break;

	  case TTY_BOLDSTYLE_NAME:
	    bold_style = ttysw_lookup_boldstyle((char *) attrs[1]);
	    if (bold_style == -1)
		(void) ttysw_print_bold_options();
	    else
		(void) ttysw_setboldstyle(bold_style);
	    break;

	  case TTY_INVERSE_MODE:
	    (void) ttysw_set_inverse_mode((int) attrs[1]);
	    break;

	  case TTY_PID:
	    do_fork = TRUE;
	    /* TEXTSW_INFINITY ==> no child process, 0 ==> we want one */
	    /* BUG ALERT: need validity check on (int)attrs[1]. */
	    ttysw->ttysw_pidchild = (int) attrs[1];
	    break;

	  case TTY_UNDERLINE_MODE:
	    (void) ttysw_set_underline_mode((int) attrs[1]);
	    break;

	  case WIN_FONT:
	    {

                if (attrs[1] && csr_pixwin) {
		    /*
		     * Cursor for the original font has been drawn, so take
		     * down
		     */
		    ttysw_removeCursor();
		    (void) xv_new_tty_chr_font(attrs[1]);
		    /* after changing font size, cursor needs to be re-drawn */
		    (void) ttysw_drawCursor(0, 0);
		}
                else if( attrs[1] )
                    change_font = (Pixfont *)attrs[1];
		break;
	    }

	  case WIN_SET_FOCUS: {
		Tty_view win;
		ATTR_CONSUME(avlist[0]);

		win = TTY_VIEW_PUBLIC(TTY_VIEW_HANDLE_FROM_TTY_FOLIO(ttysw));
		DRAWABLE_INFO_MACRO(win, info);
		if (win_getinputcodebit(
			    (Inputmask *) xv_get(win, WIN_INPUT_MASK),
			    KBD_USE)) {
			win_set_kbd_focus(win, xv_xid(info));
			return (XV_OK);
		}
		return (XV_ERROR);
	  }

#ifdef OW_I18N
          case WIN_IC_ACTIVE:
	    ttysw_pub = TTY_PUBLIC(ttysw);
            if ( is_folio && (int)xv_get(ttysw_pub, WIN_USE_IM) ) {
		Ttysw_view_handle       view;
		Tty_view                view_public;

		view = TTY_VIEW_HANDLE_FROM_TTY_FOLIO(ttysw);
		view_public = TTY_VIEW_PUBLIC(view);
                xv_set(view_public, WIN_IC_ACTIVE, attrs[1], NULL);
            }
            break;
#endif

	  case XV_END_CREATE:
	    /*
	     * xv_create(0, TTY, 0) should fork a default shell, but
	     * xv_create(0, TTY, TTY_ARGV, TTY_ARGV_DO_NOT_FORK, 0) should
	     * not fork anything (ttysw_pidchild will == TEXTSW_INFINITY >
	     * 0).
	     */
	    if (!do_fork && ttysw->ttysw_pidchild <= 0)
		do_fork = TRUE;
	    if (ttysw->view)
		ttysw_resize(ttysw->view);

            if( change_font )
            {
                ttysw_removeCursor();
                (void) xv_new_tty_chr_font(change_font);
                /* after changing font size, cursor needs to be re-drawn */
                (void) ttysw_drawCursor(0, 0);
                change_font = NULL;
            }
#ifdef OW_I18N
	   ttysw->ic = NULL;
	   ttysw_pub = TTY_PUBLIC(ttysw);

	   if( xv_get(ttysw_pub, WIN_USE_IM)){
		ttysw->ic = (XIC)xv_get(ttysw_pub, WIN_IC);
#ifdef FULL_R5		
		if (ttysw->ic)
		    XGetICValues(ttysw->ic, XNInputStyle, &ttysw->xim_style, NULL);
#endif /* FULL_R5 */		
		
	   }

	   if ( TTY_IS_TERMSW(ttysw) )
		break;

	   if ( ttysw->ic ) {
                Ttysw_view_handle       view;
                Tty_view                view_public;

                view = TTY_VIEW_HANDLE_FROM_TTY_FOLIO(ttysw);
                view_public = TTY_VIEW_PUBLIC(view);

                xv_set(view_public, WIN_IC, ttysw->ic, NULL);

		if ( xv_get(ttysw_pub, WIN_IC_ACTIVE) == FALSE )
		    xv_set(view_public, WIN_IC_ACTIVE, FALSE, NULL);
            }
#endif
            break;

	  default:
	    (void) xv_check_bad_attr(TTY, attrs[0]);
	    break;
	}
    }

    /*
     * WARNING. For certain sequences of calls, the following code loses
     * track of the process id of the current child, and could be tricked
     * into having multiple children executing at once.
     */
    if ((int) argv == TTY_ARGV_DO_NOT_FORK) {
	ttysw->ttysw_pidchild = TEXTSW_INFINITY;
    } else {
	if (argv_set && ttysw->ttysw_pidchild == TEXTSW_INFINITY) {
	    ttysw->ttysw_pidchild = 0;
	}
	if (ttysw->ttysw_pidchild <= 0 && do_fork) {
	    pid = ttysw_fork_it((char *) (ttysw), argv ? argv : (char **) &argv,
				0);
	    if (pid > 0) {
		(void) notify_set_wait3_func((Notify_client) ttysw,
				(Notify_func) (quit_tool ? tty_quit_on_death
					       : tty_handle_death),
					     pid);
	    }
	}
    }

    return (XV_OK);
}

Pkg_private     Xv_opaque
ttysw_folio_set(ttysw_folio_public, avlist)
    Tty             ttysw_folio_public;
    Tty_attribute   avlist[];
{
#ifdef OW_I18N
    return (ttysw_set_internal(ttysw_folio_public, avlist, 1));
#else
    return (ttysw_set_internal(ttysw_folio_public, avlist));
#endif

}

Pkg_private     Xv_opaque
ttysw_view_set(ttysw_view_public, avlist)
    Tty_view        ttysw_view_public;
    Tty_attribute   avlist[];
{
#ifdef OW_I18N
    return (ttysw_set_internal(ttysw_view_public, avlist, 0));
#else
    return (ttysw_set_internal(ttysw_view_public, avlist));
#endif

}




/*****************************************************************************/
/* ttysw_get_internal        				                     */
/*****************************************************************************/
/*ARGSUSED */
static          Xv_opaque
ttysw_get_internal(tty_public, status, attr, args)
    Tty             tty_public;
    int            *status;
    Tty_attribute   attr;
    va_list         args;
{
    Ttysw_folio     ttysw = TTY_PRIVATE_FROM_ANY_PUBLIC(tty_public);

    switch (attr) {
      case OPENWIN_VIEW_CLASS:
	return ((Xv_opaque) TTY_VIEW);

      case TTY_PAGE_MODE:
	return (Xv_opaque) ttysw_getopt((char *) (ttysw), TTYOPT_PAGEMODE);

      case TTY_QUIT_ON_CHILD_DEATH:
	return (Xv_opaque) 0;

      case TTY_PID:
	return (Xv_opaque) ttysw->ttysw_pidchild;

      case TTY_PTY_FD:
	return (Xv_opaque) ttysw->ttysw_pty;

      case TTY_TTY_FD:
	return (Xv_opaque) ttysw->ttysw_tty;

      case WIN_TYPE:		/* SunView1.X compatibility */
	return (Xv_opaque) TTY_TYPE;

      default:
	if (xv_check_bad_attr(TTY, attr) == XV_ERROR) {
	    *status = XV_ERROR;
	}
	return ((Xv_opaque) 0);
    }
}


Pkg_private     Xv_opaque
ttysw_folio_get(ttysw_folio_public, status, attr, args)
    Tty             ttysw_folio_public;
    int            *status;
    Tty_attribute   attr;
    va_list         args;
{
    return (ttysw_get_internal(ttysw_folio_public, status, attr, args));

}

Pkg_private     Xv_opaque
ttysw_view_get(ttysw_view_public, status, attr, args)
    Tty_view        ttysw_view_public;
    int            *status;
    Tty_attribute   attr;
    va_list         args;
{
    return (ttysw_get_internal(ttysw_view_public, status, attr, args));

}


/* ARGSUSED */
static
tty_quit_on_death(client, pid, status, rusage)
    caddr_t         client;
    int             pid;
#ifndef SVR4
    union wait     *status;
#else
    int     *status;
#endif
    struct rusage  *rusage;
{
    Ttysw_folio     ttysw = (Ttysw_folio) client;
    Tty             tty_public = TTY_PUBLIC(ttysw);
    Xv_object       frame;

    if (!(WIFSTOPPED(*status))) {
	if (WTERMSIG(*status) || WEXITSTATUS(*status) || WCOREDUMP(*status)) {
	    if (TTY_IS_TERMSW(ttysw))  {
	        (void)fprintf(stderr, 
		    XV_MSG("A command window has exited because its child exited.\n"));
	    }
	    else  {
	        (void)fprintf(stderr, 
		    XV_MSG("A tty window has exited because its child exited.\n"));
	    }

	    (void) fprintf(stderr, 
		XV_MSG("Its child's process id was %d and it"), pid);
	    if (WTERMSIG(*status)) {
		(void) fprintf(stderr, 
			XV_MSG(" died due to signal %d"),
			       WTERMSIG(*status));
	    } else if (WEXITSTATUS(*status)) {
		(void) fprintf(stderr, 
			XV_MSG(" exited with return code %d"),
			       WEXITSTATUS(*status));
	    }
	    if (WCOREDUMP(*status)) {
		(void) fprintf(stderr, 
			XV_MSG(" and left a core dump.\n"));
	    } else {
		(void) fprintf(stderr, ".\n");
	    }
	}
	frame = xv_get(tty_public, WIN_FRAME);
	(void) xv_set(frame, FRAME_NO_CONFIRM, TRUE, NULL);
	xv_destroy(frame);

    }
}

/* ARGSUSED */
static
tty_handle_death(tty_folio_private, pid, status, rusage)
    Ttysw_folio     tty_folio_private;
    int             pid;
#ifndef SVR4
    union wait     *status;
#else
    int     *status;
#endif
    struct rusage  *rusage;
{
    if (!(WIFSTOPPED(*status))) {
	tty_folio_private->ttysw_pidchild = 0;
    }
}


Pkg_private int
ttysw_view_destroy(ttysw_view_public, status)
    Tty_view        ttysw_view_public;
    Destroy_status  status;
{
    Ttysw_view_handle ttysw_view_private =
    TTY_VIEW_PRIVATE_FROM_ANY_VIEW(ttysw_view_public);


    if ((status != DESTROY_CHECKING) && (status != DESTROY_SAVE_YOURSELF)) {
	csr_pixwin = (Xv_Window)NULL;
	free((char *) ttysw_view_private);
    }
    return (XV_OK);
}

Pkg_private int
ttysw_folio_destroy(ttysw_folio_public, status)
    Ttysw_folio     ttysw_folio_public;
    Destroy_status  status;
{
    return (ttysw_destroy(ttysw_folio_public, status));
}

