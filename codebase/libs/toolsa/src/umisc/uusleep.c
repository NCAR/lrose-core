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
/*************************************************************************
 * uusleep.c - utilities library
 *
 * sleeps in microseconds
 *
 * local implementation of the usleep function, since the DEC does not
 * seem to support usleep
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * December 1990
 *
 * modified from code by Gerry Wiener
 *
 ***************************************************************************/

#ifndef SUNOS5_ETG

#include <toolsa/uusleep.h>
#include <sys/time.h>
#include <string.h>
/************************
 * sleep in micro-seconds
 */

void uusleep(unsigned int usecs)

{

  struct timeval sleep_time;
  fd_set read_value;

  sleep_time.tv_sec = usecs / 1000000;
  sleep_time.tv_usec = usecs % 1000000;

  memset ((void *)  &read_value,
          (int) 0, (size_t)  (sizeof(fd_set)));

  select(30, FD_SET_P &read_value, FD_SET_P 0,
	 FD_SET_P 0, &sleep_time);

}

/*************************
 * sleep in milli-seconds
 */

void umsleep(unsigned int msecs)

{
  uusleep(msecs * 1000);

}

#endif

