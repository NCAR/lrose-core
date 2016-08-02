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
// StormInstance.hh
//
// Storm bject at a give time
//
// Oct 2007
//
/////////////////////////////////////////////////////////////

#ifndef XMLCASE_HH
#define XMLCASE_HH

#include <iostream>
#include <string>
#include <vector>
#include <tdrp/tdrp.h>
#include <Spdb/Symprod.hh>
using namespace std;

class StormInstance
{
  
public:

  // constructor

  StormInstance();

  // destructor

  virtual ~StormInstance();

  // public data

  int stormId;
  time_t time;
  string relative_time;
  double lat;
  double lon;
  double speedKmh;
  double dirnDegT;

  bool nowcastSet;
  int ageSecs;
  double topKm;
  double volumeKm3;
  double areaKm2;
  double maxDbz;
  double htMaxDbzKm;
  double vil;
  int stormIntensity;
  double hailProb;
  double hailMass;
  double hailMassAloft;

  bool ellipseSet;
  double ellipseMinorAxisKm;
  double ellipseMajorAxisKm;
  double ellipseOrientationDegT;
  
  bool polygonSet;
  vector<Symprod::wpt_t> polygonPts;
  
  vector<int> parentIds;
  vector<int> childIds;

private:

};

#endif

