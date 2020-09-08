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
// RayLoc.hh
//
// Ray location object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2017
//
///////////////////////////////////////////////////////////////

#ifndef RayLoc_HH
#define RayLoc_HH

class RadxRay;
class Beam;

class RayLoc {

public:

  int startIndex;
  int midIndex;
  int endIndex;
  bool active;
  const RadxRay *ray;
  Beam *beam;
  
  RayLoc() {
    startIndex = -1;
    midIndex = -1;
    endIndex = -1;
    active = false;
    ray = NULL;
    beam = NULL;
  }

  void clear() {
    startIndex = -1;
    midIndex = -1;
    endIndex = -1;
    active = false;
    ray = NULL;
    beam = NULL;
  }

  // ray locator
  // we use a locator with data every 1/10th degree
  // around the 360 degree circle
  
  static const int RAY_LOC_RES = 10;
  static const int RAY_LOC_N = 3600;

};

#endif

