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
 *
 * RfUtilities.c
 *
 * part of the rfutil library - radar file access
 *
 * General utilities for dealing with some of the radar structures.
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1996
 *
 **************************************************************************/

#include <time.h>

#include <titan/file_io.h>

#include <titan/radar.h>

/*************************************************************************
 *
 * RfTimeSet()
 *
 * part of the rfutil library - radar file access
 *
 * Set the given radtim_t structure from a time_t variable.
 *
 **************************************************************************/

void RfRadtimFromUnix(radtim_t *radar_time,
		      time_t unix_time)
{
  struct tm *time_ptr;
  
  time_ptr = gmtime(&unix_time);
  
  radar_time->year = time_ptr->tm_year + 1900;
  radar_time->month = time_ptr->tm_mon + 1;
  radar_time->day = time_ptr->tm_mday;
  radar_time->hour = time_ptr->tm_hour;
  radar_time->min = time_ptr->tm_min;
  radar_time->sec = time_ptr->tm_sec;
  
  return;
}
