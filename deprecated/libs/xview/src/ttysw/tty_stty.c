#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)tty_stty.c 20.20 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ttysw parameter setting mechanism using given tty settings.
 */

#include <sys/types.h>
#include <stdio.h>
#include <xview_private/portable.h>	/* for XV* defines and termios */
#include <xview_private/tty_impl.h>

#undef CTRL
#define CTRL(c) (c & 037)

#ifdef	XV_USE_TERMIOS

/*
 * Determine ttyfd tty settings and cache in environment.
 */
Pkg_private int
ttysw_saveparms(ttyfd)
    int             ttyfd;
{
    struct termios	termios;

    /*
     * Get attributes.
     */
    if (tcgetattr(ttyfd, &termios) < 0)
	return (-1);
    /*
     * Write environment variable
     */  
    (void) we_setptyparms(&termios);
    return (0);
}

#else	/* XV_USE_TERMIOS */

/*
 * Determine ttyfd tty settings and cache in environment.
 */
Pkg_private int
ttysw_saveparms(ttyfd)
    int             ttyfd;
{
    int             ldisc, localmodes;
    struct sgttyb   mode;
    struct tchars   tchars;
    struct ltchars  ltchars;

    /*
     * Get line discipline.
     */  
    if ( ioctl(ttyfd, TIOCGETD, &ldisc) == -1)
        return(-1);
    /*
     * Get tty parameters
     */  
    if ( ioctl(ttyfd, TIOCGETP, &mode) == -1)
        return(-1);
    /*
     * Get local modes
     */  
    if ( ioctl(ttyfd, TIOCLGET, &localmodes) == -1)
        return(-1);
    /*
     * Get terminal characters
     */  
    /*
     * Added hack to check for sane tty settings, for the case 
     * where there is no controlling tty.  If the settings
     * aren't what I think they should be, returns -1 so default
     * tty settings are used.  [sjoe 7/2/91]
     */
    if (( ioctl(ttyfd, TIOCGETC, &tchars) == -1) ||
		((tchars.t_intrc != CTRL('C')) && (tchars.t_quitc !=
		CTRL('\\')) && (tchars.t_eofc != CTRL('D'))))
        return(-1);
    /*
     * Get local special characters
     */  
    if (ioctl(ttyfd, TIOCGLTC, &ltchars) == -1)  
        return(-1);
    /*
     * Write environment variable
     */  
    (void) we_setptyparms(ldisc, localmodes, &mode, &tchars, &ltchars);
    return(0);

}

#endif	/* XV_USE_TERMIOS */

#ifdef	XV_USE_TERMIOS

#define	WE_TTYPARMSLEN	150		/* XXX:	long enough? */

/*
 * Save tty settings in environment.
 */
/* BUG ALERT: No XView prefix */
Pkg_private void
we_setptyparms(tp)
    register struct termios	*tp;
{
    static char            str[WE_TTYPARMSLEN];

    str[0] = '\0';
    /*
     * %c cannot be used to write the character valued fields because they
     * often have a value of \0.
     */
    strcpy( str, WE_TTYPARMS_E );
#ifndef __linux
    (void) sprintf(str + strlen( str ),
		"%ld,%ld,%ld,%ld,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd",
		tp->c_iflag, tp->c_oflag, tp->c_cflag, tp->c_lflag,
		tp->c_cc[0],  tp->c_cc[1],  tp->c_cc[2],  tp->c_cc[3],
		tp->c_cc[4],  tp->c_cc[5],  tp->c_cc[6],  tp->c_cc[7],
		tp->c_cc[8],  tp->c_cc[9],  tp->c_cc[10], tp->c_cc[11],
		tp->c_cc[12], tp->c_cc[13], tp->c_cc[14], tp->c_cc[15]);
#else /* __linux */
    (void) sprintf(str + strlen( str ),
		"%ld,%ld,%ld,%ld,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd",
		tp->c_iflag, tp->c_oflag, tp->c_cflag, tp->c_lflag,
		tp->c_cc[0],  tp->c_cc[1],  tp->c_cc[2],  tp->c_cc[3],
		tp->c_cc[4],  tp->c_cc[5],  tp->c_cc[6],  tp->c_cc[7],
		tp->c_cc[8],  tp->c_cc[9],  tp->c_cc[10], tp->c_cc[11],
		tp->c_cc[12], tp->c_cc[13], tp->c_cc[14], tp->c_cc[15],
		tp->c_cc[16]);
#endif
    (void) putenv(str);
}

#else	/* XV_USE_TERMIOS */

#define	WE_TTYPARMSLEN	150

/*
 * Save tty settings in environment.
 */
/* BUG ALERT: No XView prefix */
Pkg_private void
we_setptyparms(ldisc, localmodes, mode, tchars, ltchars)
    int             ldisc, localmodes;
    struct sgttyb  *mode;
    struct tchars  *tchars;
    struct ltchars *ltchars;
{
    static char            str[WE_TTYPARMSLEN];

    str[0] = '\0';
    /*
     * %c cannot be used to write the character valued fields because they
     * often have a value of \0.
     */
    strcpy( str, WE_TTYPARMS_E );
    (void) sprintf(str + strlen( str ),
              "%ld,%ld,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		   ldisc, localmodes,
	      mode->sg_ispeed, mode->sg_ospeed, mode->sg_erase, mode->sg_kill,
		   mode->sg_flags,
		   tchars->t_intrc, tchars->t_quitc, tchars->t_startc,
		   tchars->t_stopc, tchars->t_eofc, tchars->t_brkc,
		   ltchars->t_suspc, ltchars->t_dsuspc, ltchars->t_rprntc,
		   ltchars->t_flushc, ltchars->t_werasc, ltchars->t_lnextc);
    (void) putenv(str);
}

#endif	/* XV_USE_TERMIOS */
