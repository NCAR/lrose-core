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
/**********************************************************************
 * uvalid_datetime: returns 1 if a valid date and time is passed,
 *                 0 otherwise.
 *
 * utility routine
 *
 * Mike Dixon CADSWES CU July 1990
 *
 **********************************************************************/

#include <toolsa/umisc.h>

int uvalid_datetime(date_time_t *datetime)
{
  date_time_t tmptime;
  int return_val;

  tmptime = *datetime;

  uconvert_to_utime(&tmptime);
  uconvert_from_utime(&tmptime);

  return_val = 1;

  if (tmptime.year != datetime->year ||
      tmptime.month != datetime->month ||
      tmptime.day != datetime->day)
    return_val = 0;

  if (datetime->hour < 0 || datetime->hour > 23)
    return_val = 0;

  if (datetime->min < 0 || datetime->min > 59)
    return_val = 0;

  if (datetime->sec < 0 || datetime->sec > 59)
    return_val = 0;

  return (return_val);

}

