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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define TIMEOUT 600

static int timeout_flag=0;

static void alarm_handler()
{
  /* unotice("SIGALRM: no data, returning EOF"); */
    timeout_flag++;
}

int getbuf(int unit, char *buf, int nbyte)

{
    static char initialized = 0;
    char *p = buf;
    int n = 0, status, i;

    if (initialized) {
        status  = 0;
    } else {
        struct sigaction        newact;
        struct sigaction        oldact;

        newact.sa_handler       = alarm_handler;
#       ifdef SA_INTERRUPT
            newact.sa_flags     = SA_INTERRUPT; /* for POSIX.1 semantics */
#       else
            newact.sa_flags     = 0;
#       endif
        (void) sigemptyset(&newact.sa_mask);

        if (-1 == sigaction(SIGALRM, &newact, &oldact)) {
	  /* serror("getbuf: can't install SIGALRM handler"); */
	    exit(2);
        } else {
            if (SIG_DFL != oldact.sa_handler && SIG_IGN != oldact.sa_handler) {
	      /* uerror("getbuf: SIGALRM handler already installed"); */
		exit(2);
            } else {
                initialized     = 1;
                status          = 0;
            }
        }
    }
    if(timeout_flag) return(0);
    while (n < nbyte && !timeout_flag) {

	alarm(TIMEOUT);
	if ((i=read(unit, p, nbyte-n)) > 0) {
	    n += i;
	    p += i;
	}
	else if (i == 0) {
	    break;
	}
	else {
	    fprintf(stderr, "Read error - getbuf\n");
	    break;
	}
    }
    alarm(0);
    return(n);
}
