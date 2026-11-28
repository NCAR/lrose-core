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
// Input.hh
//
// Input class - handles the input of spdb data and
// does some pre-conditioning on them (bad data set to
// a new bad value, duplicate stations avoided).
//
// Niles Oien
// Frank Hage, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan  1999
//
///////////////////////////////////////////////////////////////


#ifndef Input_HH
#define Input_HH

#include <math.h>
#include <toolsa/port.h>
#include <rapmath/math_macros.h>

//#include <symprod/spdb_products.h>
//#include <symprod/spdb_client.h>

#include <Spdb/DsSpdb.hh>

#include <rapformats/WxObs.hh>
#include <cstdio>
using namespace std;

typedef struct {
  float lat;         /* degrees latitude of station/sensors */
  float lon;         /* degrees longitude of station/sensors */
  float alt;         /* meters altitude of station/sensors */
  float temp;        /* temperature (deg C) */ 
  float dew_point;   /* dew point - (deg C) */
  float relhum;      /* relative humidity percent */
  float windspd;     /* wind speed (m/s)   */
  float winddir;     /* wind direction (deg)   */
  float windgust;    /* max wind gusts (m/sec)   */
  float pres;        /* barometric pressure (mb)  */
  float liquid_accum;/* Liquid accumulation - since being reset mm  */
  float precip_rate; /* precipitation rate - mm/hr */
  float visibility;  /* Visibility distance - km */
  float rvr;         /* Runway Visual range km */
  float ceiling;     /* Weather/ Cloud Ceiling  km */
  float uu;          /* u component of wind */
  float vv;          /* v component of wind */
} interp_report_t;

class Input {

public: 

  // Constructor

  Input();

  // Destructor. Frees up.

  ~Input();

  // clear arrays

  void clear();

  // read the data
  // returns 0 on success, -1 on failure

  int read(char *source_string, // Can be disk location or socket.
           time_t t_start,
           time_t t_end,        // Interval.
           float bad,            // Value to use if data are missing.
           float MaxVis,
           float MaxCeiling);


  // Data
  
  int num_found;
  vector<interp_report_t> reports; // Actual surface reports, no duplicates

  private:

  void _loadReport(const WxObs &obs,
                   interp_report_t &report,
                   float bad,
                   float MaxVis,
                   float MaxCeiling);

};

#endif


