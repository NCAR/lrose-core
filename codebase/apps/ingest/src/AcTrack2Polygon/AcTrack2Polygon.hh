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
// AcTrack2Polygon.hh
//
// AcTrack2Polygon object
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2006
//
///////////////////////////////////////////////////////////////
//
// AcTrack2Polygon reads aircraft position points, creates a convex hull
// around the track, and write a GenPoly SPDB object.
//
///////////////////////////////////////////////////////////////////////

#ifndef AcTrack2Polygon_H
#define AcTrack2Polygon_H

#include <string>
#include <math.h>
#include <euclid/geometry.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/ac_posn.h>
#include <rapformats/GenPoly.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsTrigger.hh>
#include <dsdata/TriggerInfo.hh>
#include "Args.hh"
#include "Params.hh"

using namespace std;

////////////////////////
// This class

class AcTrack2Polygon {
  
public:

  // constructor

  AcTrack2Polygon (int argc, char **argv);

  // destructor
  
  ~AcTrack2Polygon();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  vector < ac_posn_wmod_t* > _acPosVec;
  GenPoly genPoly;

  void clear();
  int _processData(time_t input_time, const string file_path); 
  int _readSpdb(time_t start_time, time_t end_time);
  int _createConvexHull(time_t dataTime);
  int _writeGenPoly();
  int _expandTrack();

};

#endif

