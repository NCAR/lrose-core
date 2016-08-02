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
// BdryMgr.hh
//
// BdryMgr object
//
//  RAP, NCAR P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2002
//
///////////////////////////////////////////////////////////////
//
// StormInitBdry produces SPDB output from TITAN binary files.
//
///////////////////////////////////////////////////////////////

#ifndef BdryMgr_H
#define BdryMgr_H

#include <Spdb/DsSpdb.hh>
#include <rapformats/bdry.h>

#include "Params.hh"
using namespace std;


class BdryMgr {
  
public:

  //
  // Struct for the vital boundary statistics. Also
  // holds stuff from the storm init point.
  //
  typedef struct {
    //
    // Boundary stuff.
    //
    double Dist; 
    double Angle;
    double Vel;
    double BdryLat;
    double BdryLon;
    double Umean;
    double Vmean;
    int ExtrapSecs;
    time_t bdryTime;
    int sequenceNum;
    int groupID;
    //
    // Storm init point stuff.
    //
    double InitLat;
    double InitLon;
    double dbz_max;
    double area_max;
    double duration;
    time_t initTime;
    double c_track_num;
    double s_track_num;
  } bdryStats_t;
  
  // constructor. Copies the TDRP params,
  // the lat and the lon of the init point,
  // and returns the number of boudaries found
  // between start and end.
  //
  BdryMgr (Params *TDRP_params,
	   time_t start, 
	   time_t end,
	   double lat,
	   double lon,
	   int *numBdry);
  
  // destructor
  //  
  ~BdryMgr();


  void getBdryN(int bdryNum, BdryMgr::bdryStats_t *bdry);
  
protected:
  
private:

  Params *_params;
  double _lat, _lon;
  
  Spdb::chunk_ref_t *_bdryProductHdrs;
  ui08 *_bdryProductBuffer;
  DsSpdb _Input;

};

#endif
