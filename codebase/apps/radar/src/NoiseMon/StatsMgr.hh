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
// StatsMgr.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2022
//
///////////////////////////////////////////////////////////////

#ifndef StatsMgr_H
#define StatsMgr_H

#include <string>
#include <vector>
#include <map>
#include <cstdio>

#include <Radx/RadxTime.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxPlatform.hh>
#include <euclid/SunPosn.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

////////////////////////
// This class

class StatsMgr {
  
public:

  // constructor

  StatsMgr(const string &prog_name,
           const Args &args,
	   const Params &params);

  // destructor
  
  virtual ~StatsMgr();

  // run 

  virtual int run() = 0;

  // process a ray
  
  void processRay(const RadxPlatform &radar,
                  RadxRay *ray);

  // check and compute when ready
  
  void checkCompute(const RadxTime &mtime);

  // compute methods
  
  void clearStats();
  int computeStats();
  
  // write methods
  
  int writeStats();
  void printStats(FILE *out);
  int writeStatsToSpdb();

  // missing value
  
  static const double missingVal;

protected:
  
  string _progName;
  const Args &_args;
  const Params &_params;

  // map of field ids to names
  
  map<int, string> _fieldNameMap;

  // moments data
  
  int _nGates;

  // sun location

  RadxTime _sunTime;
  SunPosn _sunPosn;
  double _elSun, _azSun;

  // analysis

  RadxTime _thisStartTime;
  RadxTime _nextStartTime;
  
  // sums and means

  double _countCoPol;
  double _countCrossPol;
  double _sumDbmhc;
  double _sumDbmvc;
  double _sumDbmhx;
  double _sumDbmvx;
  double _sumHtKm;

  double _meanDbmhc;
  double _meanDbmvc;
  double _meanDbmhx;
  double _meanDbmvx;
  double _meanNoiseZdr;
  double _meanHtKm;
  
  double _computeStrongEchoDbzSum(RadxRay *ray);
    
private:

};

#endif
