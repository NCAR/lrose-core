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
// StormInitBdry.cc
//
// StormInitBdry object
//
// The One, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
///////////////////////////////////////////////////////////////
//
// StormInitBdry extracts data from gridded MDV fields
// at the times and locations of storm initiation. The time/space
// data are saved as an SPDB database of GenPts by the StormInitLocation
// program.
//
///////////////////////////////////////////////////////////////

#include "StormInitBdry.hh"

#include <toolsa/file_io.h>
#include <string>
#include <toolsa/umisc.h>
#include <titan/DsTitan.hh>
#include <cerrno>
#include <unistd.h>  

#include <rapformats/GenPt.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/pmu.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>

#include <cstdio>
using namespace std;

// Constructor

StormInitBdry::StormInitBdry(Params *TDRP_params)
  
{
  //
  // Make copy of the input parameters.
  //
  _params = TDRP_params;
  
  return;

}

// destructor. That stuff which dreams are made of.

StormInitBdry::~StormInitBdry(){

}

//////////////////////////////////////////////////
// Run

int StormInitBdry::Run(time_t start, time_t end)
{

  //
  // Set up the run start time string, used in the construction
  // of the output file names.
  //
  date_time_t runStartTime;
  runStartTime.unix_time = start;

  uconvert_from_utime( &runStartTime );

  sprintf(_runStartTimeString,"%d%02d%02d_%02d%02d%02d",
	  runStartTime.year, runStartTime.month, runStartTime.day,
	  runStartTime.hour, runStartTime.min,   runStartTime.sec);
  //
  // While we're here, delete the results of any prior runs.
  //
  char outFileName[MAX_PATH_LEN];
  //
  for (int j=0; j < _params->Limits_n; j++){
    sprintf(outFileName,"%s/%s_%s.dat",
	    _params->outDir,
	    _params->_Limits[j].Name,
	    _runStartTimeString);
    unlink( outFileName );
  }
  sprintf(outFileName,"%s/%s_%s.dat",
 	  _params->outDir,
	  _params->staticBdryLimit.Name,
	  _runStartTimeString);
  unlink( outFileName );


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
    exit(-1);
  }
  if (_params->debug){
    cerr << stormPoints.getNChunks() << " points found." << endl;
  }

  //
  // Loop through the locations.
  //
  for (int point=0; point < stormPoints.getNChunks(); point++){

    //
    // Dissassemble the data into a GenPt struct.
    //
    GenPt G;
    if (0 != G.disassemble(stormPoints.getChunks()[point].data,
			   stormPoints.getChunks()[point].len)){
      cerr << "GenPt dissassembly failed for point " << point << endl;
      exit(-1);
    }

    //
    // Get stuff from the GenPt and put it into a bdryStats_t
    // structure. This does not entirely fill out the structure, however, we
    // need the boundary to do that.
    //
    BdryMgr::bdryStats_t bdry;
    bdry.initTime = G.getTime();
    bdry.InitLat = G.getLat();
    bdry.InitLon = G.getLon();

    //
    // Get the values that were stored with the GenPt.
    //
    bdry.area_max = _params->badVal;
    int fn = G.getFieldNum("area_max");
    if (fn != -1){
      bdry.area_max = G.get1DVal(fn);
    }

    bdry.dbz_max = _params->badVal;
    fn = G.getFieldNum("dbz_max");
    if (fn != -1){
      bdry.dbz_max = G.get1DVal(fn);
    }

    bdry.duration = _params->badVal;
    fn = G.getFieldNum("duration");
    if (fn != -1){
      bdry.duration = G.get1DVal(fn);
      if (bdry.duration < 0){
	//
	// This is a dummy output from StormInitLocation in
	// realtime mode so that StormInitLocation can be seen to be
	// working. Ignore it.
	//
	continue;
      }
    }

    bdry.s_track_num = _params->badVal;
    fn = G.getFieldNum("simpleTrackNumber");
    if (fn != -1){
      bdry.s_track_num = G.get1DVal(fn);
    }

    bdry.c_track_num = _params->badVal;
    fn = G.getFieldNum("complexTrackNumber");
    if (fn != -1){
      bdry.c_track_num = G.get1DVal(fn);
    }


    int numBdryFound;
    BdryMgr *BdMgr = new BdryMgr(_params,
				 bdry.initTime - _params->secsPriorToInit,
				 bdry.initTime + _params->secsPostInit,
				 bdry.InitLat, bdry.InitLon, &numBdryFound);
    //
    // Loop through the boundaries for this point.
    //
    for (int ib=0; ib < numBdryFound; ib++){
      //
      // Get the stats on this boundary.
      //
      BdMgr->getBdryN( ib, &bdry );
      //
      // See if it is a stationary boundary.
      //
      if (bdry.Vel < _params->minBdryVel){
	//
	// Stationary boundary, only apply the stationary limit.
	//
	_applyLimit(bdry, _params->staticBdryLimit, false);
      } else {
	//
	// Moving boundary, loop through all the limts and
	// produce output if they are exceeded.
	//
	for (int j=0; j < _params->Limits_n; j++){
	  _applyLimit(bdry, _params->_Limits[j], true);
	}
      }

    }
    delete BdMgr;



  } // End of loop through all GenPts.


  return 0;

}

////////////////////////////////////////////////////
//
//
//
void StormInitBdry::_applyLimit(BdryMgr::bdryStats_t bdry, 
			       Params::Limit_t Limit, 
			       bool applyAngleCheck){
  

  //
  // Check on the distance and the time.
  //
  if (
      (bdry.Dist < Limit.min_dist) ||
      (bdry.Dist > Limit.max_dist) ||
      (bdry.ExtrapSecs < Limit.minExtrapSecs) ||
      (bdry.ExtrapSecs > Limit.maxExtrapSecs)
      ){
    return;
  }

  //
  // Check on the angle, if requested.
  //
  if (applyAngleCheck){
    if (
	(bdry.Angle < Limit.min_angle) ||
	(bdry.Angle > Limit.max_angle)
	){
      return;
    }
  }
  //
  if (_params->debug){
    cerr << "Success for limit " << Limit.Name << endl;
  }
  //
  // Construe the output file name.
  //
  char outFileName[MAX_PATH_LEN];
  
  sprintf(outFileName,"%s/%s_%s.dat",
	  _params->outDir,
	  Limit.Name,
	  _runStartTimeString);

  FILE *fp = fopen(outFileName,"a");
  if (fp == NULL){
    cerr << "Failed to open " << outFileName << " append." << endl;
    exit(-1);
  }


  //
  // Print the timing and positional stuff.
  //
  date_time_t initTime;
  initTime.unix_time = bdry.initTime;
  uconvert_from_utime( &initTime );

  fprintf(fp,
	  "%ld\t%d\t%d\t%d\t%d\t%d\t%d\t",
	  (long) initTime.unix_time,
	  initTime.year, initTime.month, initTime.day,
	  initTime.hour, initTime.min, initTime.sec);

  int deltaTime = (int)(bdry.bdryTime - bdry.initTime);

  //
  // Some more stuff, mostly boundary related.
  //
  fprintf(fp,
	  "%d\t%d\t%g\t%g\t%g\t%g\t%d\t%d\t%g\t%g\t",
	  deltaTime, bdry.ExtrapSecs, bdry.InitLat, bdry.InitLon,
	  bdry.BdryLat, bdry.BdryLon, bdry.sequenceNum, bdry.groupID,
	  bdry.c_track_num, bdry.s_track_num);
  //
  // Some storm init stuff.
  //
  fprintf(fp,
	  "%g\t%g\t%g\t",
	  bdry.dbz_max, bdry.area_max, bdry.duration);
  //
  // The relation between the two.
  //
  fprintf(fp,
	  "%g\t%g\t%g\t%g",
	  bdry.Dist, bdry.Vel, bdry.Umean, bdry.Vmean);

  if (applyAngleCheck){
    //
    // Include the angle in the output. This will be the case for all
    // but static boundaries.
    //
    fprintf(fp,"\t%g",
	    bdry.Angle);
  }

  fprintf(fp,"\n");

  fclose(fp);
	  


  return;

}
