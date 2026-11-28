/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <bzlib.h>

#include <string.h>

#include "ulog.h"
#include "mkdirs_open.h"

extern int getbuf(int unit, char *buf, int nbyte);

struct packet {
  short junk1[6];
  unsigned short size;
  unsigned char id, type;
  unsigned short seq, gen_date;
  unsigned int gen_time;
  unsigned short num_seg, seg;
  unsigned int coll_time;
  unsigned short coll_date, range, angle, radial, rad_status, elev_angle;
  unsigned short elev_num;
  short first_refl, first_dopp;
  unsigned short refl_size, dopp_size;
  unsigned short num_refl_gate, num_dopp_gate, sector;
  float gain;
  unsigned short refl_ptr, vel_ptr, spec_ptr, dopp_res, pattern;
  short junk2[4];
  unsigned short refl_ptr_rda, vel_ptr_rda, spec_ptr_rda, nyquist, atten;
  short thresh;
  short junk3[17];
  unsigned char data[2304];
  float dbz[460];
};

static char *compression_type = "BZIP2";
/*
   nexradII outfile
*/
static void
usage(
	char *av0 /*  id string */
)
{
  (void)fprintf(stderr,
		"Usage: %s [options] [filename]\t\nOptions:\n", av0);
  (void)fprintf(stderr,
		"\t-v		Verbose, tell me about each product\n");
  (void)fprintf(stderr,
		"\t-l logfile	Default logs to syslogd\n");
  (void)fprintf(stderr,
		"\t-C type	Compression BZIP2)\n");
  (void)fprintf(stderr,
		"\t-f           Filter out type 2 radials with status 28\n");
	exit(1);
}
/*
 * called upon receipt of signals
 */
static void
signal_handler(int sig)
{
#ifdef SVR3SIGNALS
	/* 
	 * Some systems reset handler to SIG_DFL upon entry to handler.
	 * In that case, we reregister our handler.
	 */
    (void) signal(sig, signal_handler);
#endif
    switch(sig) {
      case SIGHUP :
	udebug("SIGHUP") ;
	return ;
      case SIGINT :
	unotice("Interrupt") ;
	exit(0) ;
      case SIGTERM :
	udebug("SIGTERM") ;
	exit(0) ;
      case SIGUSR1 :
	udebug("SIGUSR1") ;
	return ;
      case SIGUSR2 :
	if (toggleulogpri(LOG_INFO))
	  unotice("Going verbose") ;
	else
	  unotice("Going silent") ;
	return ;
      case SIGPIPE :
	unotice("SIGPIPE") ;
	exit(0) ;
    }
    udebug("signal_handler: unhandled signal: %d", sig) ;
}


static void
set_sigactions()
{
#ifndef NO_POSIXSIGNALS
    struct sigaction sigact ;
    
    sigact.sa_handler = signal_handler;
    sigemptyset(&sigact.sa_mask) ;
    sigact.sa_flags = 0 ;
#ifdef SA_RESTART	/* SVR4, 4.3+ BSD */
    /* usually, restart system calls */
    sigact.sa_flags |= SA_RESTART ;
#endif
    
    (void) sigaction(SIGHUP, &sigact, NULL) ;
    (void) sigaction(SIGINT, &sigact, NULL) ;
    (void) sigaction(SIGTERM, &sigact, NULL) ;
    (void) sigaction(SIGUSR1, &sigact, NULL) ;
    (void) sigaction(SIGUSR2, &sigact, NULL) ;
    (void) sigaction(SIGPIPE, &sigact, NULL) ;
#else
    (void) signal(SIGHUP, signal_handler) ;
    (void) signal(SIGINT, signal_handler) ;
    (void) signal(SIGTERM, signal_handler) ;
    (void) signal(SIGUSR1, signal_handler) ;
    (void) signal(SIGUSR2, signal_handler) ;
    (void) signal(SIGPIPE, signal_handler) ;
#endif
}

int main(int argc, char *argv[], char *envp[])
{
    char *block = (char *)malloc(8192), *oblock = (char *)malloc(262144);
    unsigned isize = 8192, osize=262144, olength;
    int length, go;
    int fd;
    int logmask = (LOG_MASK(LOG_ERR)) | (LOG_MASK(LOG_NOTICE));
  
    int logfd;
    char *logfname = "";
    extern int optind;
    extern int opterr;
    extern char *optarg;
    int ch;
    int bzip2 = 1;
    int filter = 0;

    opterr = 1;
    while ((ch = getopt(argc, argv, "fvxl:C:")) != EOF)
	switch (ch) {
	case 'v':
	    logmask |= LOG_MASK(LOG_INFO);
	    break;
	case 'x':
	    logmask |= LOG_MASK(LOG_DEBUG);
	    break;
	case 'l':
	    if(optarg[0] == '-' && optarg[1] != 0)
	    {
		fprintf(stderr, "logfile \"%s\" ??\n",
			optarg);
		usage(argv[0]);
	    }
	    /* else */
	    logfname = optarg;
	    break;
        case 'C':
	    if (strcmp(optarg, "BZIP2")) {
		fprintf(stderr, "Compression type must be BZIP2\n");
		exit(1);
	    }
	    else {
		compression_type = optarg;
		bzip2 = (strcmp(compression_type, "BZIP2")==0);
	    }
	    break;
	case 'f':
	    filter = 1;
	    break;
	case '?':
	    usage(argv[0]);
	    break;
	}
    setulogmask(logmask);
    /*
     * initialize logger
     */
    if(logfname == NULL || !(*logfname == '-' && logfname[1] == 0))
	(void) fclose(stderr);
    logfd = openulog(ubasename(argv[0]),
		     (LOG_CONS|LOG_PID), LOG_LDM, logfname);
    if (argc - optind <= 0 ) {
	fd = 1;
    }
    else {
	if ((fd=mkdirs_open(argv[optind],O_WRONLY | O_CREAT, 0664)) == -1) {
	    uerror("Cannot open %s", argv[1]);
	    return 1;
	}
	lseek(fd, 0, SEEK_END);
    }
    /*
     * set up signal handlers
     */
    set_sigactions();
    go = 1;
    while (go) {
	int i; 

	if ((i=getbuf(0, (char *)&length, 4)) != 4) {
	    /* if (i > 0) uerror("Short block length"); */
	    return -1;
	}
	if ( (strncmp((char *)&length, "ARCH", 4)==0) ||
	     (strncmp((char *)&length, "AR2V", 4)==0)
            ) {
	    memcpy(block, &length, 4);
	    if (getbuf(0, block+4, 20) != 20) {
		uerror("Missing header");
		return -1;
	    }
	    lseek(fd, 0, SEEK_SET);
	    write(fd, block, 24);
	    continue;
	}
	length = ntohl(length);
	if(length < 0) {
	    uinfo("EOF");
	    length = -length;
	    go = 0;
	}
	if (length > isize) {
	    isize = length;
	   /* uinfo ("Expanding input buffer to %d", isize); */
	    if ((block = (char *)realloc(block, isize)) == NULL) {
		uerror("Cannot allocate input buffer");
		return -1;
	    }
	}
	if (getbuf(0, block, length) != length) {
	    uerror("Short block read");
	    return -1;
	}
	if (length > 10) {
	    int error;

	tryagain:
	    olength = osize;
	    if (bzip2 == 1)
#ifdef BZ_CONFIG_ERROR
		error = BZ2_bzBuffToBuffDecompress(oblock, &olength,
#else
		error = bzBuffToBuffDecompress(oblock, &olength,
#endif
					       block, length, 0, 0);
	    if (error) {
		if (error == BZ_OUTBUFF_FULL) {
		    osize += 262144;
		    /* uinfo ("Expanding output buffer to %d", osize); */
		    if ((oblock=(char*) realloc(oblock, osize)) == NULL) {
			uerror("Cannot allocate output buffer");
		    }
		    goto tryagain;
		}
		uerror("decompress error - %d\n", error);
		exit(1);
	    }
	    if (filter) {
	        int i;

		for (i=0; i < olength; i += 2432) {
		    struct packet *packet=(struct packet *) (oblock+i);

		    if (packet->type != 2 || packet->rad_status != 28)
			write(fd, oblock+i, 2432);
		}
	    }
	    else write(fd, oblock, olength);
	}
    }
    close(fd);
    return 0;
}
