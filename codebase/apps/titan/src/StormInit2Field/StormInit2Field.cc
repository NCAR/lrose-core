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
///////////////////////////////////////////////////////////////
// StormInit2Field.cc
//
// StormInit2Field object
//
// RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
// Reads SPDB GenPts from StormInitLocation, writes gridded MDV
// data.
//

#include "StormInit2Field.hh"
#include "LordOfTheMdv.hh"
#include "BdryMgr.hh"

#include <toolsa/file_io.h>
#include <string>
#include <toolsa/umisc.h>
#include <titan/DsTitan.hh>
#include <cerrno>
#include <rapformats/bdry.h>
#include <rapformats/bdry_extrap.h>
#include <toolsa/toolsa_macros.h>

#include <rapformats/GenPt.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/pmu.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>

#include <cstdio>
using namespace std;

// Constructor

StormInit2Field::StormInit2Field(Params *TDRP_params)
  
{
  //
  // Make copy of the input parameters.
  //
  _params = TDRP_params;
  
  return;

}

// destructor. That stuff which dreams are made of.

StormInit2Field::~StormInit2Field(){

}

//////////////////////////////////////////////////
// Run

int StormInit2Field::Run(time_t start, time_t end)
{

  if (_params->debug){
    cerr << "Processing data from " << utimstr(start);
    cerr << " to " << utimstr(end) << endl;
  }

  //
  // Read the SPDB database of storm init locations.
  //
  DsSpdb stormPoints;
  if (stormPoints.getInterval(_params->spdbUrl, start, end)){
    cerr << "getInterval failed." << endl; // Unlikely.
    return -1;
  }
  if (_params->debug){
    cerr << stormPoints.getNChunks() << " points found." << endl;
  }

  //
  // Loop through the locations.
  //
  LordOfTheMdv *L = new LordOfTheMdv (_params, start, end);
  //
  for (int point=0; point < stormPoints.getNChunks(); point++){

    //
    // Dissassemble the data into a GenPt struct.
    //
    GenPt G;
    if (0 != G.disassemble(stormPoints.getChunks()[point].data,
			   stormPoints.getChunks()[point].len)){
      cerr << "GenPt dissassembly failed for point " << point << endl;
      return -1;
    }


    //
    // Get stuff from the GenPt.
    //
    //
    double lat = G.getLat();
    double lon = G.getLon();
    time_t spdbTime = G.getTime();

    if (
	(lat < -89.9) &&
	(fabs(lon) < 0.01)
	) {
      //
      // This is one of the dummy points written out
      // in realtime mode to show that StormInitLocation
      // is operative. Ignore it.
      //
      continue;
    }
    
    //
    // Get the values that were stored with the GenPt.
    //
    double badVal = -999.0;

    double maxArea = badVal;
    int fn = G.getFieldNum("area_max");
    if (fn != -1){
      maxArea = G.get1DVal(fn);
    }

    double maxDbz = badVal;
    fn = G.getFieldNum("dbz_max");
    if (fn != -1){
      maxDbz = G.get1DVal(fn);
    }

    double Duration = badVal;
    fn = G.getFieldNum("duration");
    if (fn != -1){
      Duration= G.get1DVal(fn);
    }


    //
    // Apply thresholds, if requested.
    //
    if (_params->applyAreaThresholds){
      if (
	  ( maxArea > _params->areaThresholds.max ) ||
	  ( maxArea < _params->areaThresholds.min )
	  ){
	continue;
      }
    }
    //
    if (_params->applyDurationThresholds){
      if (
	  ( Duration > _params->durationThresholds.max ) ||
	  ( Duration < _params->durationThresholds.min )
	  ){
	continue;
      }
    }
    
    if (_params->boundaryProximityRequired){
      if (!(StormInit2Field::_isCloseToBoundary(lat,lon,end))){
	continue;
      }
    }

    if (_params->debug){
      cerr << "Adding point " << point << " at time " << utimstr( spdbTime ) << endl;
      cerr << "  maxDBZ: " << maxDbz << endl;
    }


    L->addGaussian(lat, lon, Duration, maxArea);


  } // End of loop through all GenPts.

  //
  // Destroy the Mdv object - which writes out the grid.
  //
  delete L;

  return 0;

}

///////////////////////////////////////////////////////////
//
// Routine to determine if we are close to a boundary.
//
int StormInit2Field::_isCloseToBoundary(double lat, double lon, time_t time){

  int numBdry;

  BdryMgr B(_params, 
	    time - _params->boundaryLookback, 
	    time, lat, lon, &numBdry);
  
  if (numBdry == 0) return 0;

  for (int i=0; i < numBdry; i++){
    B.getBdryN( i );
    if (B.Dist <= _params->boundaryProximity){
      return 1;
    }
  }
  
  return 0;

}
