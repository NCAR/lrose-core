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
//////////////////////////////////////////////////////////
// StormInstance.cc
//
// Storm bject at a give time
//
// Oct 2007
//
//////////////////////////////////////////////////////////

#include "StormInstance.hh"
#include <iostream>
using namespace std;

//////////////////////
// Constructor

StormInstance::StormInstance()
{

  // initialize

  stormId = -9999;
  time = 0;
  relative_time = "not-set";
  lat = -9999;
  lon = -9999;
  speedKmh = -9999;
  dirnDegT = -9999;

  nowcastSet = false;
  ageSecs = -9999;
  topKm = -9999;
  volumeKm3 = -9999;
  areaKm2 = -9999;
  maxDbz = -9999;
  htMaxDbzKm = -9999;
  vil = -9999;
  stormIntensity = -9999;
  hailProb = -9999;
  hailMass = -9999;
  hailMassAloft = -9999;

  ellipseSet = false;
  ellipseMinorAxisKm = -9999;
  ellipseMajorAxisKm = -9999;
  ellipseOrientationDegT = -9999;
  
  polygonSet = false;

}

//////////////////////
// Destructor

StormInstance::~StormInstance()
{

}

