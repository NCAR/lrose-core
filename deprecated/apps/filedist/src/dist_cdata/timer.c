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
/*********************************************************************
 * timer.c
 * 
 * Timer routines
 *
 * Frank Hage Dec 1995 NCAR, Research Applications Program
 */

#include "dist_cdata.h"
 
/**********************************************************************
 * TIMER_FUNC: This routine is called only if the file transfer times out
 * - Print an error msg and die. This is only called by the child process
 *
 */

void timer_func(int sig)
{
  fprintf(stderr,
	  "Data transfer timed out to host %s, file: %s\n",
	  gd.cur_host, gd.cur_file);
  _exit(-1);
}
 
/**********************************************************************
 * SET_TIMER:  Start up the interval timer
 */
 
void set_timer(int seconds)

{
  struct  itimerval   timer;
  
  signal(SIGALRM,timer_func);  /* Set routine to call on each alarm */
  
  /* set up interval timer interval */
  timer.it_value.tv_sec = seconds;
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
  
  setitimer(ITIMER_REAL, &timer, NULL); /*  */

}

