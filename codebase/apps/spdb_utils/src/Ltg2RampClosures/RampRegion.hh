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
/**
 * @file RampRegion.hh
 * @brief RampRegion
 * @class RampRegion
 * @brief RampRegion 
 * @note
 * @todo
 */

#ifndef RAMP_REGION_HH
#define RAMP_REGION_HH

#include <cstdio>
#include <algorithm>
#include <vector>
#include <cmath>
#include <iostream>
#include <euclid/Pjg.hh>
#include <rapformats/ltg.h>
using namespace std;
#include "RampClosure.hh"

class RampRegion {

public:

  /**
   *  Constructor
   */
  RampRegion(const char* name, const float lat, const float lon, 
	     const float  radius, const int closureTime, 
	     const string outdir, const Pjg &proj);

  /**
   *  Destructor
   */
  ~RampRegion();
  
  /**
   *
   */
  void process( LTG_extended_t &strike);

  /**
   *
   */
  void process( LTG_strike_t &strike);
  
  /**
   * 
   */ 
  void printData();
  
  void computeClosures();

  void setIntervalStart(const time_t startTime) { pIntervalStart = startTime;}

  void setIntervalEnd(const time_t endTime) { pIntervalEnd = endTime;}
  
protected:
   
private:
 
  /**
   * Latitude of verification region center.
   */
  double pLat;

  /**
   * Longitude of verification region center.
   */
  double pLon;

  /**
   * Name of verification region
   */
  string pName;
 
  /**
   * Radius of verification region in kilometers
   */
  double pRadKm;

  int pClosureTime;

  Pjg pProj;
  
  vector <time_t > pLtgTimeSeries;

  vector < RampClosure *>  pClosures;

  string pOutdir;

  time_t pIntervalStart;

  time_t pIntervalEnd;
  
  bool pInRegion(float &strikeLat, float &strikeLon);

  void pWriteData();
 
};

#endif
