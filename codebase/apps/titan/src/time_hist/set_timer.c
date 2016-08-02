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
 * set_timer.c
 *
 * Set up the timer for the frequency of display updates
 *
 * RAP, NCAR, Boulder CO
 *
 * Jan 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "time_hist.h"
#include <sys/time.h>
#include <signal.h>

void set_timer(void)

{

  struct itimerval t_alarm;
  double base_timer_interval;
  double secs_fraction;
  double secs_whole;

  /*
   * set alarm interval for checking clients
   */

  base_timer_interval = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				      "base_timer_interval",
				      BASE_TIMER_INTERVAL);

  if (base_timer_interval == 0.0)
    base_timer_interval = 1.0;

  secs_whole = floor(base_timer_interval);
  secs_fraction = base_timer_interval - secs_whole;

  t_alarm.it_interval.tv_sec = (int) (secs_whole + 0.5);
  t_alarm.it_interval.tv_usec =
    (int) (secs_fraction * 1000000.0 + 0.5);

  t_alarm.it_value.tv_sec = t_alarm.it_interval.tv_sec;
  t_alarm.it_value.tv_usec = t_alarm.it_interval.tv_usec;

  /*
   * register respond_to_timer function to respond to alarm signals
   */

  PORTsignal(SIGALRM, (void(*)()) respond_to_timer);
  
  /*
   * set the interval timer going
   */

  if (setitimer(ITIMER_REAL, &t_alarm, (struct itimerval *) 0) != 0) {

    fprintf(stderr, "ERROR - %s:set_timer.\n", Glob->prog_name);
    perror("Cannot start interval timer");
    tidy_and_exit(1);

  }

}
