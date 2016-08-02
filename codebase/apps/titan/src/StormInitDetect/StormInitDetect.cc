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
// StormInitDetect.cc
//
// StormInitDetect object
//
// RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
///////////////////////////////////////////////////////////////
//
// StormInitDetect produces SPDB output from TITAN server.
//
///////////////////////////////////////////////////////////////

#include "StormInitDetect.hh"

#include <vector>
#include <string>
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pjg_flat.h>
#include <titan/DsTitan.hh>
#include <cerrno>
#include <titan/TitanComplexTrack.hh>
#include <titan/TitanSimpleTrack.hh>
#include <titan/TitanTrackEntry.hh>
#include <rapformats/GenPt.hh>
#include<Spdb/DsSpdb.hh>
#include <toolsa/pmu.h>

#include <toolsa/MsgLog.hh>
#include <Fmq/DsRadarQueue.hh>
#include <Fmq/NowcastQueue.hh>



using namespace std;

// Constructor

StormInitDetect::StormInitDetect(Params *TDRP_params)
  
{
  //
  // Make copy of the input parameters.
  //
  _params = TDRP_params;
  //
  // Set up the FMQ, if desired.
  //
  if (_params->sendStormInitFmq){
   if ( _nowcastQueue.init( _params->stormInitFmq,
			    "StormInitDetect", 
			    _params->Instance,
			    _params->debug ) != 0 ) {
      cerr << "Failed to initialize FMQ " << _params->stormInitFmq << endl;
      exit(-1);
    }
  }

  return;

}

// destructor

StormInitDetect::~StormInitDetect()

{

}

//////////////////////////////////////////////////
// Run

int StormInitDetect::Run(time_t scanTime )
{
  //
  // Keep a count of the number of points written this run.
  // If this is 0 and we are in realtime mode, we generate dummy
  // output so that it is clear that the program is still running.
  //
  _numWrittenThisRun = 0;

  //
  if ((_params->debug) || (_params->printTimes)){
    cerr << "Running on scan at " << utimstr(scanTime) << endl;
  }

  // set up the DsTitan server object

  _titan.clearArrays();  _titan.clearRead();
  //
  // At the time of writing this is mode dependant. I'm puzzled by this
  // but I have discussed it with Mike Dixon (who is also puzzled) and
  // it works.
  //

  if (_params->triggerMode == Params::TRIGGER_ARCHIVE){
    _titan.setReadClosest(scanTime, 0); // Search margin of 0 since time already pinpointed.
  } else {
    _titan.setReadLatest();
  }

  _titan.setReadAllAtTime();


  // do the read

  PMU_auto_register("Reading data....");
  if (_titan.read(_params->titanUrl)) {
    if (_params->debug){
      cerr << "ERROR - StormInitDetect::Run()" << endl;
      cerr << _titan.getErrStr() << endl;
    }
    //
    // This may just mean there are no storms - hardly a fatal
    // error condition.
    //
    if (_params->triggerMode != Params::TRIGGER_ARCHIVE)
      _writeDummy(scanTime);
    //
    // Fire an FMQ if we have to.
    //
    if (_params->sendStormInitFmq){
      if (_nowcastQueue.fireTrigger( "StormInitDetect", 
				     scanTime,
				     1, 1 )){
	cerr << "Failed to fire nowcast trigger!" << endl;
	exit(-1);
      }
    }
    //
    return 0;
  }

  PMU_auto_register("Read data....");

  if (_params->triggerMode == Params::TRIGGER_LDATAINFO){
    //
    // Make sure the scan time is correct.
    //
    scanTime = _titan.getTimeInUse();
  }
  //
  // Set up the vector of simple track numbers that meet our
  // threshold of significant storms. Also set up a vector to push
  // back the finish times.
  //

  vector< int > sigTrackNums;
  vector< time_t > finishTimes;
  vector< double > area;

  //
  // Get the complex tracks.
  //
  const vector<TitanComplexTrack *> &ComplexTracks = _titan.complex_tracks();

  int numComplex = ComplexTracks.size();

  if (_params->debug){
    cerr << numComplex << " complex tracks found." << endl;
  }

  //
  // Loop through the complex tracks. For those that satisfy the area
  // critera, look up the start points. Need a longevity too.
  //
  for (int ct=0; ct < numComplex; ct++){
    //
    PMU_auto_register("Processing complex tracks....");
    //
    //
    const TitanComplexTrack &ComplexTrack = *ComplexTracks[ct];
    //
    // Get the complex track parameters
    //
    // const complex_track_params_t &ComplexParams =
    // ComplexTrack.complex_params();
    //
    // And the vector of tracks.
    //
    const vector<TitanSimpleTrack *> &SimpleTracks =
      ComplexTrack.simple_tracks();
    const int numSimple = SimpleTracks.size();
    //
    // Loop through the simple tracks.
    //
    if (_params->debug){
      cerr << numSimple << " simple tracks found for complex track " << ct+1 << " of " << numComplex  << endl;
    }

    for (int st = 0; st < numSimple; st++){
      //
      PMU_auto_register("Processing simple tracks....");
      //
      const TitanSimpleTrack &SimpleTrack = *SimpleTracks[st];
      //
      // Get the simple track parameters.
      //
      const simple_track_params_t &SimpleParams =  SimpleTrack.simple_params();
      //
      // And the vector of track entries.
      //
      const vector<TitanTrackEntry *> &TrackEntries = SimpleTrack.entries();
      int numEntries = TrackEntries.size();
      //
      //
      // Loop through the track entries.
      //
      if (_params->debug){
	cerr << numEntries << " entries found for simple track " << st+1;
	cerr << " of " << numSimple << endl;
      }
      //
      for (int te=0; te < numEntries; te++){
	PMU_auto_register("Processing track entries....");
	const TitanTrackEntry &TrackEntry = *TrackEntries[te];
	//
	//
	const track_file_entry_t T = TrackEntry.entry();
	//

	const storm_file_scan_header_t &Scan = TrackEntry.scan();

	if (Scan.time == scanTime){
	  //
	  // Only process if this is the first time it has popped up - no history.
	  //
	  if ( T.duration_in_secs == 0 ){
	    //
	    //
	    const int numParents = SimpleParams.nparents;
	    //
	    int simpleTrackNum = SimpleParams.simple_track_num;
	    //
	    int complexTrackNum = SimpleParams.complex_track_num;
	    //
	    // Skip it if we are using the complex track number list and this is not
	    // on that list.
	    //
	    if (_params->useComplexNumList){
	      bool found = false;
	      for (int il=0; il < _params->complexTrackNumList_n; il++){
		if (complexTrackNum == _params->_complexTrackNumList[il]){
		  found = true;
		  break;
		}
	      }
	      if (!(found)) continue;
	    }
	    //
	    if (_params->debug){
	      cerr << "Track " << complexTrackNum << " " << simpleTrackNum;
	      cerr << " has " << numParents << " parents." << endl;
	      const int *parents = SimpleParams.parent;
	      for (int ip=0; ip < numParents; ip++){
		cerr << "Parent " << ip+1 << " is number " << parents[ip] << endl;
	      }
	    }
	    //
	    if (numParents == 0){
	      //
	      // A simple track with no parents - must be first detect.
	      // Save this storm out.
	    
	      //
	      const vector<TitanTrackEntry *> &TrackEntries = SimpleTrack.entries();
	      const TitanTrackEntry &TrackEntry = *TrackEntries[0];
	    
	      //
	      // Get the lat/lon of the init.
	      //  
	      double latOrg = Scan.grid.proj_origin_lat;
	      double lonOrg = Scan.grid.proj_origin_lon;
	    
	
	    
	      const storm_file_global_props_t &Gp = TrackEntry.gprops();
	      double X = Gp.proj_area_centroid_x;
	      double Y = Gp.proj_area_centroid_y;
	  
	      double latCent, lonCent;
	    
	      if (Scan.grid.proj_type == TITAN_PROJ_LATLON){
		latCent = Y; lonCent = X;
	      } else {
		PJGLatLonPlusDxDy(latOrg, lonOrg, X, Y, &latCent, &lonCent);
	      }
	      //
	      // There are some checks to do to see if we want
	      // to save this point.
	      //
	      bool OKtoSave = true;
	      //
	      //
	      // See if the lat/lon are near a radar, if requested,
	      // before saving out SPDB data.
	      //
	      if (_params->apply_closeness_criteria){
		if (!(isCloseToRadar(latCent, lonCent))){
		  OKtoSave = false;
		  if (_params->debug){
		    cerr << "Point failed closeness to radar criteria." << endl;
		  }
		}
	      }
	      //
	      // Apply a bounding box, if required.
	      //
	      if (_params->apply_bounding_box){
		if (
		    (latCent > _params->bounding_box.max_lat) &&
		    (latCent < _params->bounding_box.min_lat) &&
		    (lonCent > _params->bounding_box.max_lon) &&
		    (lonCent < _params->bounding_box.min_lon)
		    ){
		  OKtoSave = false;
		  if (_params->debug){
		    cerr << "Point failed bounding box criteria." << endl;
		  }
		}
	      }

	      //
	      // If we are OK to save, do so.
	      //
	      if ( OKtoSave ){
		//
		//
		if (_params->debug){
		  cerr << "Parentless track " << simpleTrackNum << " starts at ";
		  cerr << latCent << " " << lonCent;
		  cerr << " at time " << utimstr(Scan.time) << endl;
		}

		//
		// Save out in a GenPt struct.
		//
		GenPt G;
		G.setName("Titan init point");
		G.setId( 0 );
		G.setTime( Scan.time );
		G.setLat( latCent );
		G.setLon( lonCent );
		G.setNLevels( 1 );
	      
	      
		G.addVal( simpleTrackNum );
		G.addFieldInfo( "simpleTrackNumber", "None");
	      
		G.addVal( complexTrackNum );
		G.addFieldInfo( "complexTrackNumber", "None");
		//
	
		
		if (!(G.check())){
		  cerr << "GenPt check failed." << endl;
		  cerr << G.getErrStr() << endl;
		  exit(-1);
		}
		
		if( G.assemble() != 0 ) {
		  cerr << "Failed dismally to assemble GenPt." << endl;
		  exit(-1);
		}

		DsSpdb spdbMgr;
		//
		// Write the point to the database
		//
		if (spdbMgr.put( _params->outUrl, 
				 SPDB_GENERIC_POINT_ID,
				 SPDB_GENERIC_POINT_LABEL,
				 simpleTrackNum,
				 Scan.time, 
				 Scan.time + _params->maxValidTime,
				 G.getBufLen(), G.getBufPtr(),
				 0 )){
		
		  cerr << "Failed to write to SPDB." << endl;
		  exit(-1);
		}
		_numWrittenThisRun++;

	      }
	    }	
	  }
	}
      }
    }
  }
  //
  // If no valid data is being output, then
  // write a placeholder so that sysView knows that output
  // is being produced.
  //
  if (
      (_params->triggerMode != Params::TRIGGER_ARCHIVE) &&
      (_numWrittenThisRun == 0)
      ){
    _writeDummy(scanTime);
  }

  //
  // Fire off an FMQ, if requested.
  //
  if (_params->sendStormInitFmq){
    if (_nowcastQueue.fireTrigger( "StormInitDetect", 
				   scanTime,
				   1, 1 )){
      cerr << "Failed to fire nowcast trigger!" << endl;
      exit(-1);
    }
  }

  for (int i=0; i < _params->sleepAfterRun; i++){
    sleep(1);
    if ((i % 15) == 0){
      PMU_auto_register("Sleeping after run...");
    }
  }


  return 0;

}



//////////////////////////////////////////////////////////////////
//
// Routine to only accept data close to radars.
//
int StormInitDetect::isCloseToRadar(double lat, double lon){

  //
  // Loop through radar locations from parameter file.
  //
  for (int i=0; i < _params->radar_locations_n; i++){
    //
    // Find the distance (r) to this radar.
    //
    double r,theta;
    PJGLatLon2RTheta(lat, lon,
		     _params->_radar_locations[i].lat,
		     _params->_radar_locations[i].lon,
		     &r, &theta);
    //
    // If we are close to this radar, return 1.
    //

    if (r <= _params->_radar_locations[i].max_dist){
      return 1;
    }

  }
  //
  // The point was not close to any of the radars.
  // Return 0.
  //
  return 0;

}

////////////////////////////////////////////////
//
// Routine to write dummy output so that program is
// seen to be functioning.
//
void StormInitDetect::_writeDummy(time_t scanTime){

  //
  // Save out silly values in a GenPt struct.
  // Downstream processes should check that
  // the duration is positive.
  //
  GenPt G;
  G.setName("Titan init point");
  G.setId( 0 );
  //
  // This is only done in REALTIME mode so use the current time.
  //
  time_t dataTime = scanTime;

  G.setTime( dataTime );
  G.setLat( -90.0 );
  G.setLon( 0.0 );
  G.setNLevels( 1 );

  G.addVal( -100.0 );
  G.addFieldInfo( "simpleTrackNumber", "None");

  G.addVal( -100.0 );
  G.addFieldInfo( "complexTrackNumber", "None");

  //
  // Add the generation time.
  //
  time_t now = time(NULL);
  double saveTime = (double) now;
  G.addVal( saveTime );
  G.addFieldInfo( "saveTime", "unix_time");
		
  if (!(G.check())){
    cerr << "GenPt check failed in writeDummy." << endl;
    cerr << G.getErrStr() << endl;
    exit(-1);
  }
		
  if( G.assemble() != 0 ) {
    cerr << "Failed dismally to assemble dummy GenPt." << endl;
    exit(-1);
  }

  DsSpdb spdbMgr;
  //
  // Write the dummy point to the database
  //
  if (spdbMgr.put( _params->outUrl, 
		   SPDB_GENERIC_POINT_ID,
		   SPDB_GENERIC_POINT_LABEL,
		   -100,
		   dataTime, 
		   dataTime + _params->maxValidTime,
		   G.getBufLen(), G.getBufPtr(),
		   0 )){
		
    cerr << "Failed to write dummy point to SPDB." << endl;
    exit(-1);
  }

  return;

}

