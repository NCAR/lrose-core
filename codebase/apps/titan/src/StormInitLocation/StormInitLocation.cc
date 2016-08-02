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
// StormInitLocation.cc
//
// StormInitLocation object
//
// The One, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
///////////////////////////////////////////////////////////////
//
// StormInitLocation produces SPDB output from TITAN server.
// Output goes to stdout.
//
///////////////////////////////////////////////////////////////

#include "StormInitLocation.hh"

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
using namespace std;

// Constructor

StormInitLocation::StormInitLocation(Params *TDRP_params)
  
{
  //
  // Make copy of the input parameters.
  //
  _params = TDRP_params;
  
  return;

}

// destructor

StormInitLocation::~StormInitLocation()

{

}

//////////////////////////////////////////////////
// Run

int StormInitLocation::Run(time_t start, time_t end, bool archiveMode )
{
  //
  // Keep a count of the number of points written this run.
  // If this is 0 and we are in realtime mode, we generate dummy
  // output so that it is clear that the program is still running.
  //
  _numWrittenThisRun = 0;

  //
  // Make copies of the start and end times.
  //
  _start = start; _end = end;
  //
  if (_params->debug){
    cerr << "Running from " << utimstr(start);
    cerr << " to " << utimstr(end) << endl;
  }

  // set up the DsTitan server object

  _titan.clearArrays();  _titan.clearRead();

  //  if (_params->debug) {
  //    _titan.setDebug(true);
  //  }


  time_t margin = (end-start)/2;
  time_t seekTime = end - margin;

  _titan.setReadClosest(seekTime, margin);
  _titan.setReadAllInFile();

  //  if (_params->debug) {
  //    _titan.printReadRequest(cerr);
  //  }

  // do the read

  PMU_auto_register("Reading data....");
  if (_titan.read(_params->titanUrl)) {
    cerr << "ERROR - StormInitLocation::Run()" << endl;
    cerr << _titan.getErrStr() << endl;
    //
    // This may just mean there are no storms - hardly a fatal
    // error condition.
    //
    return 0;
  }
  PMU_auto_register("Read data....");

  // print out

  // Decided this was really a bit too verbose ....

  //  if (_params->debug) {
  //    cout << "dirInUse: "
  //	 << _titan.getDirInUse() << endl;
  //    cout << "timeInUse: "
  //	 << DateTime::str(_titan.getTimeInUse()) << endl;
  //    cout << endl;
    
  //    cout << "scanInUse: "
  //	 << _titan.getScanInUse() << endl;
  //    cout << "idayInUse: "
  //	 << DateTime::str(_titan.getIdayInUse() * SECS_IN_DAY) << endl;
    
  //    cout << "stormPathInUse: " << _titan.getStormPathInUse() << endl;
  //    cout << "trackPathInUse: " << _titan.getTrackPathInUse() << endl;
  //    cout << endl;

  //    _titan.print(stdout);

  //  }
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

  //  if (_params->debug) {
  //    cerr << numComplex << " complex tracks found." << endl;
  //  }
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

    //    if (_params->debug) {
    //      cerr << "Complex track " << ct;
    //      cerr << " has " << numSimple << " simple tracks." << endl;
    //    }
    //
    // Loop through the simple tracks.
    //
    for (int st = 0; st < numSimple; st++){
      //
      cerr << "Simple track " << st << " of " << numSimple << endl;
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
      //      if (_params->debug) {
      //	cerr << "  Simple track " << st;
      //	cerr << " of complex track " << ct;
      //	cerr << " has " << numEntries << " track entries." << endl;
      //      }
      //
      // Loop through the track entries.
      //
      for (int te=0; te < numEntries; te++){
	PMU_auto_register("Processing track entries....");
	const TitanTrackEntry &TrackEntry = *TrackEntries[te];
	//
	//
	//
	const storm_file_scan_header_t &Scan = TrackEntry.scan();

	if (_params->debug){
	  cerr << "Scan at time " << utimstr(Scan.time) << endl;
	}

	const track_file_entry_t &trackFileEntry = TrackEntry.entry();


	double latOrg = Scan.grid.proj_origin_lat;
	double lonOrg = Scan.grid.proj_origin_lon;


	const storm_file_global_props_t &Gp = TrackEntry.gprops();
	double X = Gp.proj_area_centroid_x;
	double Y = Gp.proj_area_centroid_y;

	double Area = Gp.area_mean;

	double latCent, lonCent;

	if (Scan.grid.proj_type == TITAN_PROJ_LATLON){
	  latCent = Y; lonCent = X;
	} else {
	  PJGLatLonPlusDxDy(latOrg, lonOrg, X, Y, &latCent, &lonCent);
	}

	//
	// Note that Duration is the time in secs since the start of the
	// earliest branch of this storm - so it may be too long for
	// a particular branch. But if it does not exceed the temporal
	// threshold, then none of its branches will, so it is worth
	// checking. The threshold will have to be checked again, however.
	//
	const long Duration = SimpleParams.history_in_secs;
	const time_t trackEntryTime = trackFileEntry.time;
	//	const int numParents = SimpleParams.nparents;
	//	const int *parents = SimpleParams.parent;
	int simpleTrackNum = SimpleParams.simple_track_num;

	if (
	    (Area >= _params->areaThreshold) &&
	    (Duration >= _params->temporalThreshold)
	    ){

	  //	  if (_params->debug) {
	  //	    cerr << "    Track entry " << te;
	  //	    cerr << "  of simple track " << st;
	  //	    cerr << " of complex track " << ct;
	  //	    cerr << " Lon : " << lonCent;
	  //	    cerr << " Lat : " << latCent;
	  //	    cerr << " Duration : " << Duration;
	  //	    cerr << " Mean area : " << Area << endl;
	  //	    cerr << "Has " << numParents << " parents." << endl;
	  //	    for (int i=0; i < numParents; i++){
	  //	      cerr << "Parent " << i << " simple track number " << parents[i] << endl;
	  //	    }
	  //	  }
	  //
	  // The simple track number has exceeded the threshold and is a significant
	  // storm. push it back onto the vector of significant storm simple track numbers,
	  // if we have not done so already.
	  //
	  if (!( StormInitLocation::_entryInIntVector(sigTrackNums, simpleTrackNum))){
	    sigTrackNums.push_back(simpleTrackNum);
	    finishTimes.push_back( trackEntryTime );
	    area.push_back(Area);
	  }

	}
      }
    }
  }
  //
  // Print out simple track numbers we want to process.
  //
  int vnum = sigTrackNums.size();
  //  if (_params->debug) {
  //    cerr << vnum << " simple tracks to process." << endl;
  //  }

  _alreadyProcessed.clear();
  for (int i=0; i < vnum; i++){
    //
    //   if (_params->debug) {
    //     cerr << "Processing track " << sigTrackNums[i] << endl;
    //    }
    //
    // Set _finishTime and _area before we start processing.
    //
    _finishTime = finishTimes[i];
    _area = area[i];
    StormInitLocation::_processSimpleTrack(sigTrackNums[i]);
    //
    // Clear the record of what we've processed.
    //
    _alreadyProcessed.clear();
  }
  //
  // In REALTIME mode, generate some dummy output
  // so that program is seen to be functional.
  //
  if (
      (!(archiveMode)) &&
      ( _numWrittenThisRun == 0)
      ){
    _writeDummy();
  }

  return 0;

}

//////////////////////////////////////////////////////////////////
//
// Small routine to determine if a given integer is already in a vector
// of integers.
//
int StormInitLocation::_entryInIntVector(vector< int >& V, int entry){

  int num = V.size();
  for (int i=0; i < num; i++){
    if ( V[i] == entry) return 1; // Entry is in vector.
  }

  return 0; // Entry is not in vector.

}

//////////////////////////////////////////////////////////////////
//
// Large, ungainly routine to process a given simple track number.
// Recurses on itself if track number has parents.
//
void  StormInitLocation::_processSimpleTrack(int simpleTrackNum){

  PMU_auto_register("Recursing on simple tracks....");

  DsSpdb spdbMgr;

  const vector<TitanComplexTrack *> &ComplexTracks = _titan.complex_tracks();
  int numComplex = ComplexTracks.size();
  for (int ct=0; ct < numComplex; ct++){
    //
    const TitanComplexTrack &ComplexTrack = *ComplexTracks[ct];
    const vector<TitanSimpleTrack *> &SimpleTracks = ComplexTrack.simple_tracks();
    const int numSimple = SimpleTracks.size();
    //
    // Loop through the simple tracks.
    //
    for (int st = 0; st < numSimple; st++){
      //
      const TitanSimpleTrack &SimpleTrack = *SimpleTracks[st];
      const simple_track_params_t &SimpleParams =  SimpleTrack.simple_params();
      //
      // If this is the track we want, process it.
      //
      if (simpleTrackNum == SimpleParams.simple_track_num){
	if (SimpleParams.nparents > 0 ){
	  if (_params->recurseOnParents){
	    //
	    // Recurse on parents. Check that we have not already
	    // processed that track though - save much time.
	    //
	    for (int ip=0; ip < SimpleParams.nparents; ip++){
	      if (!( StormInitLocation::_entryInIntVector(_alreadyProcessed, SimpleParams.parent[ip]))){
		// cerr << "  Track " << simpleTrackNum  << " parental track " << SimpleParams.parent[ip] << endl;
		StormInitLocation::_processSimpleTrack( SimpleParams.parent[ip] );
		_alreadyProcessed.push_back(SimpleParams.parent[ip]);
	      }
	    }
	  }
	} else {
	  //
	  // Track has no parents, should save out data here if the temporal
	  // longevity has been exceeded.
	  //
	  //  cerr << "Track " << simpleTrackNum << " has no parents." << endl;
	  //
	  int complexTrackNum = SimpleParams.complex_track_num;
	  //
	  const vector<TitanTrackEntry *> &TrackEntries = SimpleTrack.entries();
	  const TitanTrackEntry &TrackEntry = *TrackEntries[0];

	  const track_file_entry_t &trackFileEntry = TrackEntry.entry(); 
	  const time_t startTime = trackFileEntry.time;
	  //
	  // Now that we have the star and end times, we can
	  // decide if the duration exceeds the threshold.
	  //
	  long Duration = _finishTime - startTime;
	  double Minutes = Duration/60.0;

	  if (Duration >= _params->temporalThreshold){

	    const storm_file_scan_header_t &Scan = TrackEntry.scan();
	    
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
	      }
	    }
	    //
	    // Check that the time is inside the
	    // temporal window.
	    //
	    if (
		( Scan.time < _start) ||
		( Scan.time > _end)
		){
	      OKtoSave = false;
	    }
	    //
	    // If we are OK to save, do so.
	    //
	    if ( OKtoSave ){
	      //
	      //	      if (_params->debug){
	      //		cerr << "Parentless track " << simpleTrackNum << " starts at ";
	      //		cerr << latCent << " " << lonCent << endl;
	      //	      }

	      //
	      // See if we have any data existing at this time.
	      // If we do, we just update it. If not, we save
	      // what we have.
	      //
	      double maxArea = _area; // _area was set on entry to this track.

	      double oldSaveTime = 0.0; // Set to avoid warnings.
	      bool gotOldSaveTime = false;
	      //
	      DsSpdb Existing;
	      bool priorExistance = false;
	      Existing.getExact(_params->outUrl, 
				Scan.time,
				simpleTrackNum,
				0);
	      if ( Existing.getNChunks() > 0){
		priorExistance = true;
		//
		// There was previous data, see if we can
		// read the area and compare it to the area we have now.
		//
		GenPt OG; // Old GenPt
		if (0==OG.disassemble(Existing.getChunks()[0].data,
				      Existing.getChunks()[0].len)){
		  //
		  // Dissassembly went OK - try to read the area_max field.
		  //
		  int fn = OG.getFieldNum("area_max");
		  if (fn != -1){
		    double existingArea = OG.get1DVal(fn);
		    if (existingArea > maxArea){
		      maxArea = existingArea;
		    }
		  }
		  //
		  // Also get the time saved, if it exists.
		  //
		  fn = OG.getFieldNum("saveTime");
		  if (fn != -1){
		    gotOldSaveTime = true;
		    oldSaveTime = OG.get1DVal(fn);
		  }

		}
	      } 
	      //////////////////////////////////////////////////
	      //
	      // Done with stuffing around with previous area entries to determine
	      // if we have a maximum here.
	      //
	      //////////////////////////////////////////////////

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
	      G.addVal( Gp.dbz_max );
	      G.addFieldInfo( "dbz_max", "dbz");
	      G.addVal( Minutes );
	      G.addFieldInfo( "duration", "minutes");
	      G.addVal( maxArea );
	      G.addFieldInfo( "area_max", "Km square or cubed");
	      G.addVal( simpleTrackNum );
	      G.addFieldInfo( "simpleTrackNumber", "None");
	      G.addVal( complexTrackNum );
	      G.addFieldInfo( "complexTrackNumber", "None");
	      //
	      // Add the generation time.
	      //
	      double saveTime = 0.0;
	      if (!(gotOldSaveTime)){
		time_t now = time(NULL);
		saveTime = (double) now;
	      } else {
		saveTime = oldSaveTime; // Preserve this value.
	      }
	      G.addVal( saveTime );
	      G.addFieldInfo( "saveTime", "unix_time");

	      if (_params->debug){
	
		if (!(priorExistance)){
		  cerr << " Saving data for track [" << simpleTrackNum <<", ";
		  cerr << complexTrackNum << "] datatime : ";
		  cerr << utimstr(Scan.time) << endl;
		  cerr << "   Location (" << latCent << ", ";
		  cerr << lonCent << ") save time : " << utimstr((time_t)saveTime) << endl;
		} else {
		  cerr << " Updating data for track [" << simpleTrackNum <<", ";
		  cerr << complexTrackNum << "]" << endl;
		  cerr << "  max_area= " << maxArea << " Duration= " << Minutes << endl;
		}
		//
		cerr << "   Existed prior : ";
		if (priorExistance){
		  cerr << "  true." << endl << endl;
		} else {
		  cerr << "  false." << endl << endl;
		}
	      }
		
	      if (!(G.check())){
		cerr << G.getErrStr() << endl;
		exit(-1);
	      }
		
	      if( G.assemble() != 0 ) {
		cerr << "Failed dismally to assemble GenPt." << endl;
		exit(-1);
	      }
		
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

//////////////////////////////////////////////////////////////////
//
// Routine to only accept data close to radars.
//
int StormInitLocation::isCloseToRadar(double lat, double lon){

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
// Routine to write dummy output so that prgram is
// seen to be functioning.
//
void StormInitLocation::_writeDummy(){

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
  time_t dataTime = time(NULL);

  G.setTime( dataTime );
  G.setLat( -90.0 );
  G.setLon( 0.0 );
  G.setNLevels( 1 );
  G.addVal( -100.0 );
  G.addFieldInfo( "dbz_max", "dbz");
  G.addVal( -100.0 );
  G.addFieldInfo( "duration", "minutes");
  G.addVal( -100.0 );
  G.addFieldInfo( "area_max", "Km2");
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
		
    cerr << "Failed to write to SPDB." << endl;
    exit(-1);
  }
}

