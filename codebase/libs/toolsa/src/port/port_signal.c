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
/* port_signal.c
   portability covers */

#if defined(IRIX4) || defined(IRIX5)
#define __EXTENSIONS__  /* needed to compile sigaction */
#endif
#if defined(SUNOS5) || defined(SUNOS5_ETG) || defined(IRIX5) || defined(__linux)
#define _POSIX_SOURCE   /* needed to compile sigaction */
#endif

#include <signal.h>
#include <stdlib.h>

#include <toolsa/globals.h>
#include <toolsa/port.h>


PORTsigfunc PORTsignal(int signo, PORTsigfunc func)
{
    struct sigaction	act, oact;
    
    act.sa_handler = func;
    sigemptyset( &act.sa_mask);
    act.sa_flags = 0;

    if( signo == SIGALRM) 
	{
#ifdef SA_INTERRUPT
	act.sa_flags |= SA_INTERRUPT;	/* SunOS */
#endif
    	} 
    else 
	{
#ifdef SA_RESTART
	act.sa_flags |= SA_RESTART;	/* SVR4 4.3+BSD */
#endif
    	}
    
    if(sigaction( signo, &act, &oact) < 0)
	return ((PORTsigfunc) (SIG_ERR));
    
    return ((PORTsigfunc) (oact.sa_handler));
}




