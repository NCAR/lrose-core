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

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <rdi/misc.h>

#ifdef AIX
#include <sys/select.h>
#endif 

void rdi_usleep(unsigned int us)

{
    struct timeval timeout;
    fd_set fd;

    timeout.tv_sec = us / 1000000;
    timeout.tv_usec = us % 1000000;

    select (0, &fd, &fd, &fd, &timeout);

}

void rdi_msleep (int ms)
{
    struct timeval timeout;
    fd_set fds;

    timeout.tv_sec = ms / 1000;
    timeout.tv_usec = 1000 * (ms % 1000);

    select (0, &fds, &fds, &fds, &timeout);


}
