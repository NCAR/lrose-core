#ifndef lint
#ifdef sccs
static  char sccsid[] = "@(#)tty_gtty.c 20.29 93/06/28 Copyr 1983 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ttysw parameter retrieval mechanism to get original tty settings to pty.
 */

#include <sys/types.h>
#include <stdio.h>
#include <xview_private/portable.h>	/* for tty_mode_t and XV* defines */
#include <xview_private/tty_impl.h>

#undef CTRL
#define CTRL(c) (c & 037)

/*
 * Default settings to use if they can't actually be obtained from a
 * descriptor relevant to the application.
 *
 * XXX:	Is this still necessary in the SVR4-style pty world?  Probably so...
 *
 * XXX:	These settings shouldn't be used unless absolutely necessary, since
 *	they're almost certain to get out of sync with the kernel's defaults
 *	(which is what they're intended to be).
 */
#ifdef	XV_USE_TERMIOS

#if (defined(__linux) || defined(__APPLE__)) && !defined(CINTR)
/* The following values have been obtained from /usr/include/linux/tty.h.
 * They represent the default tty modes on linux.
	intr=^C		quit=^|		erase=del	kill=^U
	eof=^D		vtime=\0	vmin=\1		sxtc=\0
	start=^Q	stop=^S		susp=^Z		eol=\0
	reprint=^R	discard=^U	werase=^W	lnext=^V
	eol2=\0
*/
/* This belongs in sys/termios.h, not here. */
#define CINTR 003
#define CQUIT 034
#define CERASE 0177
#define CKILL 025
#define CEOF 004
#define CTIME 0
#define CMIN 1
#define CSWTC 0
#define CSTART 021
#define CSTOP 023
#define CSUSP 032
#define CEOL 0
#define CREPRINT 022
#define CDISCARD 017
#define CWERASE 027
#define CLNEXT 026
#define CEOL2 0
#endif /* __linux && !CINTR */

#if defined(__linux) || defined(__APPLE__)
/* martin.buck@bigfoot.com */

#ifndef CSWTC
#define CSWTC 0
#endif
#ifndef CEOL2
#define CEOL2 0
#endif

static struct termios   default_modes = {
	ICRNL|IXON,	  		  	/* input modes */
	OPOST|ONLCR,			    	/* output modes */
	B38400|CS8|CREAD,   		 	/* control modes */
	ISIG|ICANON|ECHO|ECHOCTL|ECHOKE, 	/* local modes */

#if 1
/* martin.buck@bigfoot.com */
        0,					/* line discipline */
#endif
	CINTR, CQUIT, CERASE, CKILL,
	CEOF, CTIME, CMIN, CSWTC,
	CSTART, CSTOP, CSUSP, CEOL,
	CREPRINT, CDISCARD, CWERASE, CLNEXT,
	CEOL2
};
	
#else /* __linux */
static struct termios	default_modes = {
	BRKINT|ICRNL|IXON|IGNPAR|IMAXBEL,	    	/* input modes */
	OPOST|ONLCR,				    	/* output modes */
	B9600|(B9600 << IBSHIFT)|CS8|HUPCL|CREAD,    	/* control modes */
	ISIG|ICANON|ECHO|IEXTEN|ECHOE|ECHOK|ECHOCTL|ECHOKE, /* local modes */
						       /* control characters */
	CINTR,		/* VINTR */
	CQUIT,		/* VQUIT */
	CERASE,		/* VERASE */
	CKILL,		/* VKILL */
	CEOT,		/* VEOF */
	CEOL,		/* VEOL */
	CEOL2,		/* VEOL2 */
	CNUL,		/* VSWTCH */
	CSTART,		/* VSTART */
	CSTOP,		/* VSTOP */
	CSUSP,		/* VSUSP */
	CDSUSP,		/* VDSUSP */
	CRPRNT,		/* VRPRNT */
	CFLUSH,		/* VDISCARD */
	CWERASE,	/* VWERASE */
	CLNEXT,		/* VLNEXT */
};
#endif /* __linux */

#else	/* XV_USE_TERMIOS */

static struct sgttyb default_mode = {
    13, 13, '\177', CTRL('U'),		/* ispeed ospeed erase kill */
    EVENP | ODDP | CRMOD | ECHO		/* flags */
};
static struct tchars default_tchars = {
    CTRL('C'), CTRL('\\'), CTRL('Q'),	/* intr quit start */
    CTRL('S'), CTRL('D'), '\033'	/* stop eof brkc */
};
static struct ltchars default_ltchars = {
    CTRL('Z'), CTRL('Y'), CTRL('R'),	/* susp dsusp rprnt */
    CTRL('O'), CTRL('W'), CTRL('V')	/* flush werase lnext */
};

#endif	/* XV_USE_TERMIOS */

#ifdef	XV_USE_TERMIOS

/*
 * Retrieve tty settings from environment and set ttyfd to them.
 * (Why do we get tty settings from the environment?  Why do we
 * set the environment and then immediately unset it?)
 */
Pkg_private int
ttysw_restoreparms(ttyfd)
    int	ttyfd;
{
    struct termios	termios;
    int			retrying = 0;
    int			fd = 2;

    /*
     * Read environment variable
     */
    while (we_getptyparms(&termios) == -1) {
	if (retrying++)
	    return (1);
	/*
	 * Try to get the tty parameters from stderr (2). Using stdin (0)
	 * fails when being started in the background because csh redirects
	 * stdin from the tty to /dev/null.
	 */
	if (!isatty(fd)) {
	    fd = open("/dev/console", 2);
	}
        if ((fd <= 0) || (ttysw_saveparms(fd) == -1)) {
	    termios = default_modes;
	    we_setptyparms(&termios);
	}
	if (fd != 2)
	    (void) close(fd);
    }
    /* Force the termios flags to be reasonable. Pass through any
     * inherited control characters, except EOF and EOL.
     * If the erase character is NULL, force the entire termios structure
     * to be the default value. 
     */
    if( termios.c_cc[VERASE] )  {
        termios.c_iflag = default_modes.c_iflag;
        termios.c_oflag = default_modes.c_oflag;
        termios.c_cflag = default_modes.c_cflag;
        termios.c_lflag = default_modes.c_lflag;
        termios.c_cc[VEOF] = default_modes.c_cc[VEOF];
        termios.c_cc[VEOL] = default_modes.c_cc[VEOL];
    }
    else
        termios = default_modes;
    /*
     * XXX: Should we use something other than TCSANOW?
     */
    (void) tcsetattr(ttyfd, TCSANOW, &termios);
    return (0);
}

#else	/* XV_USE_TERMIOS */

/*
 * Retrieve tty settings from environment and set ttyfd to them.
 */
Pkg_private int
ttysw_restoreparms(ttyfd)
    int             ttyfd;
{
    int             ldisc, localmodes, retrying = 0;
    int             fd = 2;
    struct sgttyb   mode;
    struct tchars   tchars;
    struct ltchars  ltchars;

    /*
     * Read environment variable
     */
    while (we_getptyparms(
		     &ldisc, &localmodes, &mode, &tchars, &ltchars) == -1) {
	if (retrying++)
	    return (1);
	/*
	 * Try to get the tty parameters from stderr (2). Using stdin (0)
	 * fails when being started in the background because csh redirects
	 * stdin from the tty to /dev/null.
	 */
	if (!isatty(fd)) {
	    fd = open("/dev/console", 2);
	}
        if ((fd <= 0) || (ttysw_saveparms(fd) == -1)) {
	    ldisc = NTTYDISC;
	    localmodes =
		LPENDIN | LCRTBS | LCRTERA |
		LCRTKIL | LCTLECH | LDECCTQ;
	    mode = default_mode;
	    tchars = default_tchars;
	    ltchars = default_ltchars;
	   /*
	    * break out of the loop because I can't see any
	    * reason to go back and set/unset the environment
	    * variable.
	    */
	    break;
	}
	if (fd != 2) {
	    (void) close(fd);
	}
    }
#ifndef SVR4
    /*
     * Set line discipline.
     */
    (void) ioctl(ttyfd, TIOCSETD, &ldisc);
#endif
    /*
     * Set tty parameters
     */
    (void) ioctl(ttyfd, TIOCSETP, &mode);
    /*
     * Set local modes
     */
    (void) ioctl(ttyfd, TIOCLSET, &localmodes);
    /*
     * Set terminal characters
     */
    (void) ioctl(ttyfd, TIOCSETC, &tchars);
    /*
     * Set local special characters
     */
    (void) ioctl(ttyfd, TIOCSLTC, &ltchars);
    return (0);
}

#endif	/* XV_USE_TERMIOS */

#ifdef	XV_USE_TERMIOS

#define	WE_TTYPARMSLEN	120		/* XXX:	long enough? */

/*
 * Get tty settings from environment.
 */
/* BUG ALERT: No XView prefix */
/* static */ int
we_getptyparms(tp)
    struct termios	*tp;
{
    char	str[WE_TTYPARMSLEN];
#ifndef __linux
    short	temps[16];
#else
    short	temps[17];
#endif

    if (_we_setstrfromenvironment(WE_TTYPARMS, str))
	return (-1);
    else {
	register int i;

#ifndef __linux
	if (sscanf(str,
		"%ld,%ld,%ld,%ld,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd",
		&tp->c_iflag, &tp->c_oflag, &tp->c_cflag, &tp->c_lflag,
		&temps[0],  &temps[1],  &temps[2],  &temps[3],
		&temps[4],  &temps[5],  &temps[6],  &temps[7],
		&temps[8],  &temps[9],  &temps[10], &temps[11],
		&temps[12], &temps[13], &temps[14], &temps[15]) != 
	    20)
	    return (-1);
	for (i = 0; i <= VLNEXT; i++)
	    tp->c_cc[i] = temps[i];
	/*
	 * Always clear
	 */
	(void) putenv(WE_TTYPARMS_E);
	return (0);
    }
#else /* __linux */
	if (sscanf(str,
		"%ld,%ld,%ld,%ld,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd",
		&tp->c_iflag, &tp->c_oflag, &tp->c_cflag, &tp->c_lflag,
		&temps[0],  &temps[1],  &temps[2],  &temps[3],
		&temps[4],  &temps[5],  &temps[6],  &temps[7],
		&temps[8],  &temps[9],  &temps[10], &temps[11],
		&temps[12], &temps[13], &temps[14], &temps[15],
		&temps[16]) != 21)
	    return (-1);
	for (i = 0; i <= VEOL2; i++)
	    tp->c_cc[i] = temps[i];
	/*
	 * Always clear
	 */
	(void) putenv(WE_TTYPARMS_E);
	return (0);
    }
#endif /* __linux */
}

#else	/* XV_USE_TERMIOS */

#define	WE_TTYPARMSLEN	120

/*
 * Get tty settings from environment.
 */
/* BUG ALERT: No XView prefix */
/* static */ int
we_getptyparms(ldisc, localmodes, mode, tchars, ltchars)
    int            *ldisc, *localmodes;
    struct sgttyb  *mode;
    struct tchars  *tchars;
    struct ltchars *ltchars;
{
    char            str[WE_TTYPARMSLEN];
    short           temps[16];	/* Needed for sscanf as there is no %hhd */

    if (_we_setstrfromenvironment(WE_TTYPARMS, str))
	return (-1);
    else {
	if (sscanf(str,
		   "%ld,%ld,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd",
		   ldisc, localmodes, &temps[0], &temps[1], &temps[2],
		&temps[3], &mode->sg_flags, &temps[4], &temps[5], &temps[6],
		   &temps[7], &temps[8], &temps[9], &temps[10], &temps[11],
		   &temps[12], &temps[13], &temps[14], &temps[15])
	    != 19)
	    return (-1);
	mode->sg_ispeed = temps[0];
	mode->sg_ospeed = temps[1];
	mode->sg_erase = temps[2];
	mode->sg_kill = temps[3];
	tchars->t_intrc = temps[4];
	tchars->t_quitc = temps[5];
	tchars->t_startc = temps[6];
	tchars->t_stopc = temps[7];
	tchars->t_eofc = temps[8];
	tchars->t_brkc = temps[9];
	ltchars->t_suspc = temps[10];
	ltchars->t_dsuspc = temps[11];
	ltchars->t_rprntc = temps[12];
	ltchars->t_flushc = temps[13];
	ltchars->t_werasc = temps[14];
	ltchars->t_lnextc = temps[15];
	/*
	 * Always clear
	 */
	(void) putenv(WE_TTYPARMS_E);
	return (0);
    }
}

#endif	/* XV_USE_TERMIOS */

/*
 * Capture fd's current tty modes and store them in *mode.  Return 0 on
 * success and -1 on failure.
 */
Pkg_private int
tty_getmode(fd, mode)
    int		fd;
    tty_mode_t	*mode;
{
    /*
     * This change should be used in generic code too. So, the macro
     * should be XV_USE_TERMIOS.
     */
#ifdef XV_USE_TERMIOS
    return (tcgetattr(fd, mode));
#else
    return (ioctl(fd, TIOCGETP, (struct sgttyb *)mode));
#endif
}
