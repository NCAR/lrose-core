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
/**************************************************************************
 * DAEMON.C   Subroutines for use with Unix Daemon processes
 *
 * F. Hage    NCAR/RAP 1991
 * Keyword: Make process a unix daemon, 
 */

#include <stdio.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>


/**************************************************************************
 * DAEMONIZE: Transform calling process into a daemon by disassociating it
 *        from the controlling terminal.
 */

void
daemonize()
{
    /* fork a child process and make the parent exit */
    switch(fork()) {
        case -1: /* error in fork */
            perror("Daemonize, fork");
            exit(-1);
        break;

        case 0: /* child */
            /* nop */
        break;

        default:  /* is Parent; fork returns pid of child */
            exit(0);
        break;
    }

    /* Disassociate process from controlling terminal */

#if defined(AIX) || defined(DECOSF1) || defined(HPUX) || \
    defined(SUNOS4)
    if(setpgrp(0,0) == -1) {
#else
    if(setsid() == -1) {
#endif
        perror("Daemonize, setgrp");
        exit(-1);
    }
}
