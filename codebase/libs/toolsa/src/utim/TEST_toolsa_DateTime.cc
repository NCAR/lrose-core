// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*
 * Name: TEST_toolsa_DateTime.cc
 *
 * Purpose:
 *
 *      To test the DATETIME module in the library: libtoolsa.a
 *      This module is documented in the include file <toolsa/DateTime.hh>.
 *
 * Usage:
 *
 *       % TEST_toolsa_DateTime
 *
 * Inputs: 
 *
 *       None
 *
 *
 * Author: Mike Dixon, Oct 2014
 *
 */

/*
 * include files
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <toolsa/DateTime.hh>

/*
 * test individual module subroutines
 */
/* ==================================================================== */

static void print_datetime(date_time_t* dtime)
{
  /* 
   * inputs: Pointer to date_time_t
   *
   * output: prints out the contents of dtime
   *
   */

  printf("datetime: %s\n", utimestr(dtime));
  printf("unix_time: %ld\n", dtime->unix_time);

}

/* ======================================================================= */

static void set_datetime(int year, int month, int day, int hour, int min, int sec, date_time_t *dtime)
{
  /* 
   * set the values of date_time_t
   */

  dtime->year = year;
  dtime->month = month;
  dtime->day = day;
  dtime->hour = hour;
  dtime->min = min;
  dtime->sec = sec;
  uconvert_to_utime(dtime);

}

/* ======================================================================= */

static void set_utime(time_t utime, date_time_t *dtime)
{
  /* 
   * set the values of date_time_t
   */

  dtime->unix_time = utime;
  uconvert_from_utime(dtime);

}

/* ======================================================================== */

/*
 * main program
 */

int main(int argc, char *argv[])
{

  int iret = 0;
  date_time_t dtime;
  
  set_datetime(1993, 3, 31, 13, 50, 46, &dtime);
  print_datetime(&dtime);

  if (dtime.unix_time != 733585846) {
    cerr << "ERROR - TEST_toolsa_DateTime" << endl;
    cerr << "  Incorrect unix time, should be: " << 733585846L << endl;
    iret = 1;
  }

  set_utime(733585846, &dtime);
  print_datetime(&dtime);

  if (dtime.year != 1993 ||
      dtime.month != 3 ||
      dtime.day != 31 ||
      dtime.hour != 13 ||
      dtime.min != 50 ||
      dtime.sec != 46) {
    cerr << "ERROR - TEST_toolsa_DateTime" << endl;
    cerr << "  Incorrect date and time" << endl;
    cerr << "  Should be: 1993/03/31 13:50:46" << endl;
    iret = 1;
  }

  DateTime dtime1;
  dtime1.set(1969, 12, 30, 12, 00, 00);
  dtime1.setSubSec(-0.25);
  double utime1 = dtime1.asDouble();
  
  DateTime dtime2;
  dtime2.set(1970, 01, 02, 12, 00, 00);
  dtime2.setSubSec(0.35);
  double utime2 = dtime2.asDouble();

  double tdiff12 = dtime2 - dtime1;
  double ddiff12 = dtime2.asDouble() - dtime1.asDouble();

  DateTime dtime3 = dtime1 + tdiff12;
  double utime3 = dtime3.asDouble();

  cerr << "dtime1: " << dtime1.asString() << endl;
  cerr << "dtime2: " << dtime2.asString() << endl;
  cerr << "dtime3: " << dtime3.asString() << endl;

  cerr << "utime1: " << setprecision(10) << utime1 << endl;
  cerr << "utime2: " << setprecision(10) << utime2 << endl;
  cerr << "utime3: " << setprecision(10) << utime2 << endl;

  cerr << setprecision(10) << "Time diff 12       (secs): " << tdiff12 << endl;
  cerr << setprecision(10) << "Double time diff12 (secs): " << ddiff12 << endl;
  
  if (tdiff12 != 259200.6) {
    cerr << "ERROR - TEST_toolsa_DateTime" << endl;
    cerr << "  Incorrect tdiff12 : " << setprecision(10) << tdiff12 << endl;
    cerr << "  Should be         : 259200.6" << endl;
    iret = -1;
  }


  if (ddiff12 != 259200.6) {
    cerr << "ERROR - TEST_toolsa_DateTime" << endl;
    cerr << "  Incorrect ddiff12 : " << setprecision(10) << ddiff12 << endl;
    cerr << "  Should be         : 259200.6" << endl;
    iret = -1;
  }

  if (utime2 != utime3) {
    cerr << "ERROR - TEST_toolsa_DateTime" << endl;
    cerr << "  utime2 and utime3 should be the same" << endl;
    iret = -1;
  }

  if (dtime2 != dtime3) {
    cerr << "ERROR - TEST_toolsa_DateTime" << endl;
    cerr << "  dtime2 and dtime3 should be the same" << endl;
    iret = -1;
  }

  if (dtime1 >= dtime2) {
    cerr << "ERROR - TEST_toolsa_DateTime" << endl;
    cerr << "  dtime1 should be less than dtime2" << endl;
    iret = -1;
  }

  if (iret == 0) {
    fprintf(stderr, "SUCCESS - TEST_toolsa_udatetime passed\n");
  } else {
    fprintf(stderr, "FAILURE - TEST_toolsa_udatetime failed\n");
  }

  return iret;

}

/*=============================== END OF FILE =====================================*/
