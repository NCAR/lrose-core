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
// Storms2Xml.cc
//
// Storms2Xml object
//
// The One, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
///////////////////////////////////////////////////////////////
//
// Storms2Xml produces XML output from TITAN server.
//
///////////////////////////////////////////////////////////////

#include "Storms2Xml.hh"

#include <vector>
#include <string>
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pjg_flat.h>
#include <titan/DsTitan.hh>
#include <errno.h>
#include <titan/TitanComplexTrack.hh>
#include <titan/TitanSimpleTrack.hh>
#include <titan/TitanTrackEntry.hh>
#include <rapformats/GenPt.hh>
#include<Spdb/DsSpdb.hh>
#include <toolsa/pmu.h>

#include <unistd.h> // For the unlink routine.
#include <stdlib.h> // For the 'system' routine.

// Constructor

Storms2Xml::Storms2Xml(Params *TDRP_params)
  
{
  //
  // Make copy of the input parameters.
  //
  _params = TDRP_params;
  
  return;

}

// destructor

Storms2Xml::~Storms2Xml()

{

}

//////////////////////////////////////////////////
// Run

int Storms2Xml::Run(time_t start, time_t end )
{

  // Initialize a few things.

  int numTimesOutput = 0;
  int lastStormID = -1;

  if (_params->debug){
    cerr << "Running from " << utimstr(start) << " to " << utimstr(end) << endl;
  }

  // Open the output file. Return on failure.

  if ( _openOutput( end )){
    return -1;
  }

  // set up the DsTitan server object

  DsTitan myTitan;

  myTitan.clearArrays();  myTitan.clearRead();

  if (_params->debugTitan) {
    myTitan.setDebug(true);
  }


  time_t margin = (end-start)/2;
  time_t seekTime = end - margin;

  myTitan.setReadClosest(seekTime, margin);
  myTitan.setReadAllInFile();

  if (_params->debugTitan) {
    myTitan.printReadRequest(cerr);
  }

  char *inputSource;
  //
  // See if we are reading from a directory or a URL.
  //
  if (_params->triggerMode == Params::TRIGGER_LDATAINFO){
    inputSource = _params->titanDir;
  } else {
    inputSource = _params->titanUrl;
  }
  //
  // In either case, pass that off to the read object.
  //
  PMU_auto_register("Reading data....");
  if (myTitan.read( inputSource )) {
    cerr << "ERROR - Storms2Xml::Run()" << endl;
    cerr << myTitan.getErrStr() << endl;
    //
    // This may just mean there are no storms - hardly a fatal
    // error condition.
    //
    _closeOutput();
    return 0;
  }

  if (_params->debug) {
    cout << "dirInUse: "
	 << myTitan.getDirInUse() << endl;
    cout << "timeInUse: "
	 << DateTime::str(myTitan.getTimeInUse()) << endl;
    cout << endl;
    
    cout << "scanInUse: "
	 << myTitan.getScanInUse() << endl;
    cout << "idayInUse: "
	 << DateTime::str(myTitan.getIdayInUse() * SECS_IN_DAY) << endl;
    
    cout << "stormPathInUse: " << myTitan.getStormPathInUse() << endl;
    cout << "trackPathInUse: " << myTitan.getTrackPathInUse() << endl;
    cout << endl;

    if (_params->debugTitan){
      myTitan.print(stdout);
    }

  }
  //
  // Get the vector of complex tracks.
  //
  const vector<TitanComplexTrack *> &ComplexTracks = myTitan.complex_tracks();

  int numComplex = ComplexTracks.size();
  if (_params->debug) {
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
    //
    // And the vector of tracks.
    //
    const vector<TitanSimpleTrack *> &SimpleTracks =
      ComplexTrack.simple_tracks();
    const int numSimple = SimpleTracks.size();
    //
    //  if (_params->debug) {
    //   cerr << "Complex track " << ct;
    //   cerr << " has " << numSimple << " simple tracks." << endl;
    //  }
    //
    // Loop through the simple tracks.
    //
    for (int st = 0; st < numSimple; st++){
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
      // The following was found to be a bit long winded.
      //
      // if (_params->debug) {
      //  cerr << "  Simple track " << st;
      //  cerr << " of complex track " << ct;
      //  cerr << " has " << numEntries << " track entries." << endl;
      // }
      //
      // Loop through the track entries.
      //
      for (int te=0; te < numEntries; te++){
	PMU_auto_register("Processing track entries....");
	const TitanTrackEntry &TrackEntry = *TrackEntries[te];
	//
	//
	//
	const track_file_entry_t &trackFileEntry = TrackEntry.entry();
	//
	// See if this is within our temporal range.
	//
	const time_t trackEntryTime = trackFileEntry.time;

	if (
	    (trackEntryTime >= start) &&
	    (trackEntryTime <= end  )
	    ){

	  const long Duration = SimpleParams.history_in_secs;
	  int simpleTrackNum = SimpleParams.simple_track_num;
	  const storm_file_scan_header_t &Scan = TrackEntry.scan();
	  double latOrg = Scan.grid.proj_origin_lat;
	  double lonOrg = Scan.grid.proj_origin_lon;


	  const storm_file_global_props_t &Gp = TrackEntry.gprops();
	  double X = Gp.proj_area_centroid_x;
	  double Y = Gp.proj_area_centroid_y;

	  double MaxDbz = Gp.dbz_max;
	  double Major = Gp. proj_area_major_radius;
	  double Minor = Gp. proj_area_minor_radius;
	  double Rotation = Gp.proj_area_orientation;

	  double Area = Gp.area_mean;
	 

	  
 
	  double Speed = trackFileEntry.dval_dt.smoothed_speed;
	  double Direction = trackFileEntry.dval_dt.smoothed_direction;

	  double latCent, lonCent;

	  if (Scan.grid.proj_type == TITAN_PROJ_LATLON){
	    latCent = Y; lonCent = X;
	  } else {
	    PJGLatLonPlusDxDy(latOrg, lonOrg, X, Y, &latCent, &lonCent);
	  }

	  if (lastStormID != simpleTrackNum){

	    if (numTimesOutput > 0){
	      //
	      // Must close last storm.
	      //
	      fprintf(outFile,"\n </STORM>\n");
	      if (_params->debug){
		cerr << numTimesOutput << " time entries for storm ID " << lastStormID;
		cerr << " in file " << outfileName << endl;
	      }
	      numTimesOutput=0;
	    }
	    lastStormID = simpleTrackNum;
	    //
	    // Now open this storm.
	    //

	    fprintf(outFile,"\n\n <STORM ID=\"%d\">\n", simpleTrackNum);
	    //
	    // Put the parent/child information in the XML file, if
	    // there is any.
	    //
	    // Parents.
	    //
	    const int numParents = SimpleParams.nparents;
	    const int *parents = SimpleParams.parent;
	    for (int i=0; i < numParents; i++){
	      fprintf(outFile,"  <PARENT ID=\"%d\" />\n", parents[i]);
	    }
	    //
	    // Children.
	    //
	    const int numChildren = SimpleParams.nchildren;
	    const int *children = SimpleParams.child;
	    for (int i=0; i < numChildren; i++){
	      fprintf(outFile,"  <CHILD ID=\"%d\" />\n", children[i]);
	    }

	  }

	  //
	  // Print some debugging.
	  //
	  if (_params->debug) {
	    cerr << "Entry " << te;
	    cerr << " simple track " << st;
	    cerr << " complex track " << ct;
	    cerr << " Lon " << lonCent;
	    cerr << " Lat " << latCent;
	    cerr << " Duration " << Duration;
	    cerr << " Area " << Area;
	    cerr << " Time " << utimstr(trackEntryTime) << endl;
	  }

	  date_time_t Tim;
	  Tim.unix_time = trackEntryTime;
	  uconvert_from_utime( &Tim );

	  fprintf(outFile,"\n  <OBS TIME=\"%d-%02d-%02dT%02d:%02d:%02d\"\n",
		  Tim.year, Tim.month, Tim.day,
		  Tim.hour, Tim.min, Tim.sec);

	  fprintf(outFile,"   LAT=\"%g\"\n", latCent);
	  fprintf(outFile,"   LON=\"%g\"\n", lonCent);
	  
	  fprintf(outFile,"   SPEED=\"%g\"\n", Speed);
	  fprintf(outFile,"   DIRECTION=\"%g\"\n", Direction);
	  
	  fprintf(outFile,"   MAJOR=\"%g\"\n", Major);
	  fprintf(outFile,"   MINOR=\"%g\"\n", Minor);
	  fprintf(outFile,"   ROTATION=\"%g\"\n", Rotation);


	  fprintf(outFile,"   AREA=\"%g\"\n", Area);
	  fprintf(outFile,"   MAXDBZ=\"%g\"\n", MaxDbz);
	 

	  fprintf(outFile,"  />\n");
	  numTimesOutput++;

	}
      }
    }
  }

  if (numTimesOutput > 0){
    //
    // Must close last storm.
    //
    fprintf(outFile," </STORM>\n");
    if (_params->debug){
      cerr << numTimesOutput << " time entries for storm ID " << lastStormID;
      cerr << " in file " << outfileName << endl;
    }
  }
  
  _closeOutput();
  return 0;

}
//
// Small routine to open the output file.
//
int Storms2Xml::_openOutput(time_t when){

  //
  // If we are time stamping, set up the files that way.
  //
  if (_params->timestampFiles){

    date_time_t fileTime;
    fileTime.unix_time = when;
    uconvert_from_utime( &fileTime );
    sprintf(outfileName,"%s/%d%02d%02d_%02d%02d%02d_%s",
	    _params->outDir,
	    fileTime.year, fileTime.month, fileTime.day,
	    fileTime.hour, fileTime.min, fileTime.sec,
	    _params->outName);
  } else {
    //
    // We are not timestamping. In this case remove the
    // existing file before opening it to ensure that it
    // updates across a cross mount.
    //
    sprintf(outfileName,"%s/%s",
	    _params->outDir, _params->outName);
  }

  unlink(outfileName);

  outFile = fopen(outfileName,"w");

  if (outFile == NULL){
    cerr << "Failed to create " << outfileName << endl;
    return -1;
  }

  fprintf(outFile,"%s","<?xml version=\"1.0\" ?>\n\n");
  fprintf(outFile,"%s\n","<TITAN>\n");

  return 0;

}


//
// Small routine to close the output file.
//
void Storms2Xml::_closeOutput(){

  fprintf(outFile,"%s\n","\n\n</TITAN>\n");
  fclose(outFile);

  if (_params->runScriptWhenDone){

    sleep(1); // Let the file system relax.

    char comBuffer[1024];
    sprintf(comBuffer,"%s %s", _params->scriptToRun,outfileName);

    if (_params->debug){
      cerr << "Executing command : " << comBuffer << endl;
    }

    int i = system( comBuffer );

    if (_params->debug){
      cerr << "Command returned value : " << i << endl;
    }

  }


}
