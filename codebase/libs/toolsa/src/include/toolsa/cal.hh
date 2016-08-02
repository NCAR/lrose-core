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

// Calandar Functions
// Randy Bullock (c) 2003 NCAR/RAP

#ifndef  __VERIF_CALENDAR_H__
#define  __VERIF_CALENDAR_H__

using namespace std;

////////////////////////////////////////////////////////////////////////


static const int mjd_ut0 = 40587;   //  mjd of Jan 1, 1970

static const int standard_time_zone = 7;   //  mountain standard time


////////////////////////////////////////////////////////////////////////


typedef long long unixtime;


////////////////////////////////////////////////////////////////////////


extern const char *day_name[];

extern const char *short_day_name[];

extern const char *month_name[];

extern const char *short_month_name[];


////////////////////////////////////////////////////////////////////////

   //
   //  32 bit stuff
   //

extern int mdyhms_to_unix(int, int, int, int, int, int);

extern void unix_to_mdyhms(int, int &, int &, int &, int &, int &, int &);


extern int is_dst(int);

extern int doy_to_unix(int, int, int, int, int);


////////////////////////////////////////////////////////////////////////


   //
   //  64 bit stuff
   //

extern unixtime mdyhms_to_unix_64(int, int, int, int, int, int);

extern void unix_to_mdyhms_64(unixtime, int &, int &, int &, int &, int &, int &);


extern int is_dst(int, int, int, int, int, int);

extern int is_dst(unixtime);


////////////////////////////////////////////////////////////////////////


   //
   //  other stuff
   //

extern int day_of_week(int, int, int);

extern int day_dif(int, int, int, int, int, int);

extern int date_to_mjd(int, int, int);

extern void mjd_to_date(int, int &, int &, int &);

extern void easter(int &, int &, int);

extern int is_leap_year(int);


////////////////////////////////////////////////////////////////////////


#endif  //  __VERIF_CALENDAR_H__


////////////////////////////////////////////////////////////////////////


