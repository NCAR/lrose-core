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
/////////////////////////////////////////////////////////////
// SunPosn.hh
//
// Command line object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
/////////////////////////////////////////////////////////////

#ifndef SUNPOSN_HH
#define SUNPOSN_HH

#include <string>
#include <iostream>
using namespace std;

class SunPosn {
  
public:
  
  SunPosn();
  ~SunPosn();

  // set the lat/lon/alt for which sun position is computed
  // lat/lon in degrees, alt_m in meters

  void setLocation(double lat, double lon, double alt_m);

  // compute sun position
  
  void computePosn(double now, double &el, double &az);

  // compute sun position using NOVA routines

  void computePosnNova(double now, double &el, double &az);

  // original method from Reinhart

  void computePosnOld(double now, double &el, double &az);

  // original method from Meeus

  void computePosnMeeus(double now, double &el, double &az);

protected:
  
private:
  
  typedef struct {
    int year, month, day, hour, min, sec;
    time_t unix_time;
  } date_time_t;
  
  double _prevTime;
  double _lat, _lon, _alt_m;
  double _el, _az;
    
  long _julian_date(int year, int month, int day);
  void _calendar_date(long jdate, int &year, int &month, int &day);
  time_t _compute_unix_time(int year, int month, int day, int hour,
                            int min, int sec);
  void _compute_date_time(time_t unix_time,
                          int &year, int &month, int &day,
                          int &hour, int &min, int &sec);
  
};

#endif
