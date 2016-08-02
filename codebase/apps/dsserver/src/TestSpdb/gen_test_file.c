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
/***********************************************************************
 * gen_test_file.c
 *
 * Generate a test file for load_spdb
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * July 1996
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <toolsa/umisc.h>

#define TIME_STEP 1800
#define NTIMES 100
#define START_YEAR 1999
#define START_MONTH 6
#define START_DAY 20
#define START_HOUR 12
#define START_MIN 0
#define START_SEC 0

int main(int argc, char **argv)

{

  int i;
  date_time_t ut;

  /*
   * initialize
   */

  ut.year = START_YEAR;
  ut.month = START_MONTH;
  ut.day = START_DAY;
  ut.hour = START_HOUR;
  ut.min = START_MIN;
  ut.sec = START_SEC;
  uconvert_to_utime(&ut);

  for (i = 0; i < NTIMES; i++, ut.unix_time += TIME_STEP) {

    uconvert_from_utime(&ut);

    fprintf(stdout, "%.4d %.2d %.2d %.2d %.2d %.2d %s%d\n",
	    ut.year, ut.month, ut.day,
	    ut.hour, ut.min, ut.sec,
	    "Test_data_string_", i);

  } /* i */

  return (0);

}
