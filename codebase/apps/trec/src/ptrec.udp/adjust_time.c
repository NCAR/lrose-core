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
/****************************************************************
 * adjust_time()
 */

#include "trec.h"

void adjust_time(ll_params_t *beam, long time_correction)

{

  date_time_t dtime;
  
  if (beam->year < 1900) {
    if (beam->year > 70) {
      beam->year += 1900;
    } else {
      beam->year += 2000;
    }
  }

  if (time_correction == 0) {
    return;
  }

  dtime.year = beam->year;
  dtime.month = beam->month;
  dtime.day = beam->day;
  dtime.hour = beam->hour;
  dtime.min = beam->min;
  dtime.sec = beam->sec;

  uconvert_to_utime(&dtime);

  dtime.unix_time += time_correction;

  uconvert_from_utime(&dtime);
    
  beam->year = dtime.year;
  beam->month = dtime.month;
  beam->day = dtime.day;
  beam->hour = dtime.hour;
  beam->min = dtime.min;
  beam->sec = dtime.sec;

  return;

}
