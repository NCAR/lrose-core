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
// Storms2Xml2.cc
//
// Storms2Xml2 object
//
// The One, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Major changes for revised XML file format (rjp 19 May 2006)
//
///////////////////////////////////////////////////////////////
//
// Storms2Xml2 produces XML output from TITAN server.
//
///////////////////////////////////////////////////////////////

#include "Storms2Xml2.hh"

#include <cerrno>
#include <vector>
#include <string>

#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/pjg_flat.h>
#include <dsserver/DsLdataInfo.hh>
#include <titan/DsTitan.hh>
#include <titan/TitanComplexTrack.hh>
#include <titan/TitanSimpleTrack.hh>
#include <titan/TitanTrackEntry.hh>


// Constructor

Storms2Xml2::Storms2Xml2(int argc, char **argv)
  
{
  
  OK = true;

  // set programe name

  _progName = "Storms2Xml2";
  ucopyright((char *) _progName.c_str());

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = false;
  }

  // check that start and end time is set in archive mode
  
  if (_params.triggerMode == Params::TRIGGER_ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      OK = false;
    }
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.Instance,
		PROCMAP_REGISTER_INTERVAL);

  // Make sure the output directory exists.
  
  if (ta_makedir_recurse(_params.outDir)){
    cerr << "Failed to create output directory : ";
    cerr << _params.outDir << " - exiting." << endl;
    exit(-1);
  }
  
  return;

}

// destructor

Storms2Xml2::~Storms2Xml2()

{

}

//////////////////////////////////////////////////
// Run

int Storms2Xml2::Run()
{

  int iret=0;
  
  if (_params.triggerMode == Params::TRIGGER_ARCHIVE) {
    
    // Archive mode.
    
    time_t archiveTriggerTime;
    archiveTriggerTime = _args.startTime;
    
    while( archiveTriggerTime <= _args.endTime) {

      iret = _run(archiveTriggerTime - _params.lookBack, 
		  archiveTriggerTime);
      
      archiveTriggerTime += _params.timeStep;

    } // while
    
    return (iret);
    
  } else if (_params.triggerMode == Params::TRIGGER_INTERVAL) {
    
    // Realtime mode.
    
    while (true) {
      
      time_t end = time(NULL);
      time_t start = end - _params.lookBack;
      iret = _run(start, end);
      
      if (iret == 0){
	for (int i = 0; i < _params.timeStep; i++){
	  umsleep(1000);
	  PMU_auto_register("Zzzz ...");
	}
      }

    } // while

    return iret;

  } else {

    // trigger from _latest_data_info
    
    LdataInfo ldata(_params.titanDir);
    
    while(true) {
      
      if (ldata.read(_params.maxValidAge) == 0) {
	
	time_t dataTime = ldata.getLatestTime();
	iret = _run(dataTime - _params.lookBack, 
		    dataTime);
	
      } else {
	
	PMU_auto_register("Zzzzzz ...");
	umsleep(1000);
	
      }
      
    } // while
    
    return iret;

  }
    
  return 0;

}

//////////////////////////////////////////////////
// run on time interval

int Storms2Xml2::_run(time_t start, time_t end)
{
  
  // Initialize a few things.
  
  int numTimesOutput = 0;
  int lastStormID = -1;

  if (_params.debug){
    cerr << endl;
    cerr << "Running from " << utimstr(start) << " to " << utimstr(end) << endl;
  }
 
  // set up the DsTitan server object
  
  DsTitan myTitan;
  myTitan.clearArrays();  myTitan.clearRead();
  if (_params.debugTitan) {
    myTitan.setDebug(true);
  }

  // read radar name from params file 
  char *radarName; 
  radarName = _params.radar_name;

  // get current time rjp 23/5/2006
  /*
    time_t currentTime = time(NULL);
    date_time_t curTime;
    curTime.unix_time = currentTime;
    uconvert_from_utime(&curTime);
  */

  //
  // Set startTime, endTime
  //
  date_time_t startTime, endTime;
  startTime.unix_time = start;
  uconvert_from_utime(&startTime);
  endTime.unix_time = end;
  uconvert_from_utime(&endTime);

  //
  // Determine time increment
  //
  time_t margin = (end-start)/2;
  time_t seekTime = end - margin;

  myTitan.setReadClosest(seekTime, margin);
  myTitan.setReadAllInFile();

  if (_params.debugTitan) {
    myTitan.printReadRequest(cerr);
  }

  //
  // Open the output XML file and add header. Return on failure.
  //
  if ( _openOutput( end )){
    return -1;
  }

  //
  // Open <nowcast-data> node
  //
  fprintf(outFile,"<nowcast-data>\n");

  //
  // Print <time-layout> node to XML file.  
  //
  fprintf(outFile,"   <time-layout time-coordinate=\"UTC\">\n");
  fprintf(outFile,"      <start-valid-time>%d-%02d-%02dT%02d:%02d:%02d</start-valid-time>\n",
	  startTime.year, startTime.month, startTime.day, startTime.hour, startTime.min, startTime.sec);
  fprintf(outFile,"      <end-valid-time>%d-%02d-%02dT%02d:%02d:%02d</end-valid-time>\n",
	  endTime.year, endTime.month, endTime.day, endTime.hour, endTime.min, endTime.sec);
  fprintf(outFile,"   </time-layout>\n");

  //
  // See if we are reading from a directory or a URL.
  //
  char *inputSource;
  if (_params.triggerMode == Params::TRIGGER_LDATAINFO){
    inputSource = _params.titanDir;
  } else {
    inputSource = _params.titanUrl;
  }
  //
  // In either case, pass that off to the read object.
  //
  PMU_auto_register("Reading data....");
  if (myTitan.read( inputSource )) {
    cerr << "ERROR - Storms2Xml2::Run()" << endl;
    cerr << myTitan.getErrStr() << endl;
    //
    // This may just mean there are no storms - hardly a fatal
    // error condition.
    //
    //
    // Close <nowcast-data> node and then close outFile. 
    //
    fprintf(outFile,"</nowcast-data>\n");
    _closeOutput();
    return 0;
  }

  if (_params.debug) {
    cout << "dirInUse: " << myTitan.getDirInUse() << endl;
    cout << "timeInUse: "
	 << DateTime::str(myTitan.getTimeInUse()) << endl;
    cout << "scanInUse: " << myTitan.getScanInUse() << endl;
    // commented out rjp 23/11/05
    // cout << "idayInUse: "
    //	 << DateTime::str(myTitan.getIdayInUse() * SECS_IN_DAY) << endl;
    
    cout << "stormPathInUse: " << myTitan.getStormPathInUse() << endl;
    cout << "trackPathInUse: " << myTitan.getTrackPathInUse() << endl;
    cout << endl;

    if (_params.debugTitan){
      cout << "*********************************************" << endl;
      myTitan.print(stdout);
      cout << "*********************************************" << endl;
    }
  }

  //
  // Get storm parameters
  //
  const storm_file_params_t &storm_params = myTitan.storm_params();

  double LowDBZThresh = storm_params.low_dbz_threshold;
  double SizeThreshold = storm_params.min_storm_size;
  double MinRadarTops = storm_params.min_radar_tops;

  //
  // Get the vector of complex tracks.
  //
  const vector<TitanComplexTrack *> &ComplexTracks = myTitan.complex_tracks();

  int numComplex = ComplexTracks.size();
  if (_params.debug) {
    cerr << numComplex << " complex tracks found." << endl;
  }

  //
  // Loop through the complex tracks. For those that satisfy the area
  // criteria, look up the start points. Need a longevity too.
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
    //  if (_params.debug) {
    //   cerr << "Complex track " << ct;
    //   cerr << " has " << numSimple << " simple tracks." << endl;
    //  }
    //
    // Loop through the simple tracks.
    //
    for (int st = 0; st < numSimple; st++){
      PMU_auto_register("Processing simple tracks....");

      const TitanSimpleTrack &SimpleTrack = *SimpleTracks[st];

      bool newSimpleTrack = true;

      //
      // Get the simple track parameters.
      //
      const simple_track_params_t &SimpleParams =  SimpleTrack.simple_params();

      //
      // Get the vector of track entries.
      //
      const vector<TitanTrackEntry *> &TrackEntries = SimpleTrack.entries();
      int numEntries = TrackEntries.size();
      //
      //
      // The following was found to be a bit long winded.
      //
      // if (_params.debug) {
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

	if ((trackEntryTime >= start) && (trackEntryTime <= end )) {

	  const long Duration = SimpleParams.history_in_secs;
	  int simpleTrackNum = SimpleParams.simple_track_num;

	  const storm_file_scan_header_t &Scan = TrackEntry.scan();

	  // get lat, long for radar from track file. (Not actually used)
	  double latOrg = Scan.grid.proj_origin_lat;
	  double lonOrg = Scan.grid.proj_origin_lon;

	  const storm_file_global_props_t &Gp = TrackEntry.gprops();
	  double X = Gp.proj_area_centroid_x;
	  double Y = Gp.proj_area_centroid_y;

	  double Major = Gp. proj_area_major_radius;
	  double Minor = Gp. proj_area_minor_radius;
	  double Rotation = Gp.proj_area_orientation;

	  double MaxDbz = Gp.dbz_max;
	  double HtMaxDbz = Gp.ht_of_dbz_max;
	  double Volume = Gp.volume;
	  double Top = Gp.top;	 
	  double ProjArea = Gp.proj_area;
	  double MeanArea = Gp.area_mean;
	  double Vil = Gp.vil_from_maxz;
	  double FOKRCategory = Gp.add_on.hail_metrics.FOKRcategory;
	  double WaldvogelProb = Gp.add_on.hail_metrics.waldvogelProbability;
	  double vertIntHailMass = Gp.add_on.hail_metrics.vihm;
	  double HailMassAloft = Gp.add_on.hail_metrics.hailMassAloft;

	  double Speed = trackFileEntry.dval_dt.smoothed_speed;
	  double Direction = trackFileEntry.dval_dt.smoothed_direction;

	  double latCent, lonCent;

	  if (Scan.grid.proj_type == TITAN_PROJ_LATLON){
	    latCent = Y; lonCent = X;
	  } else {
	    PJGLatLonPlusDxDy(latOrg, lonOrg, X, Y, &latCent, &lonCent);
	  }

	  //
	  // Get trackEntryTime.  
	  //
	  date_time_t Tim;
	  Tim.unix_time = trackEntryTime;
	  uconvert_from_utime( &Tim );

	  lastStormID = simpleTrackNum;

	  //
	  // Print debug information.
	  //
	  if (_params.debug) {
	    cerr << "Entry " << te;
	    cerr << ", simple track " << st;
	    cerr << ", complex track " << ct;
	    cerr << ", Lon " << lonCent;
	    cerr << ", Lat " << latCent;
	    cerr << ", Duration " << Duration;
	    cerr << ", ProjArea " << ProjArea;
	    cerr << ", Time " << utimstr(trackEntryTime) << endl;
	  }

	  //
	  // Open storm event node in XML file.
	  //
	  if (newSimpleTrack) {
	    //
	    //close last event if required
	    if (numTimesOutput > 0) {
 	      fprintf(outFile,"   </event>\n"); 
	      numTimesOutput = 0;
	    }
 	    fprintf(outFile,"   <event ID=\"%d\">\n", simpleTrackNum);
            newSimpleTrack = false;
	  }

	  //
	  // Open <case> node in XML file 
	  //
	  fprintf(outFile,"      <case>\n");
	  fprintf(outFile,"         <time time-coordinate=\"UTC\">%04d-%02d-%02dT%02d:%02d:%02d</time>\n",
                  Tim.year, Tim.month, Tim.day, Tim.hour, Tim.min, Tim.sec);

	  //
	  // Print storm <location> node.
	  //
	  fprintf(outFile,"         <location>\n");
          fprintf(outFile,"            <area>\n");
          fprintf(outFile,"               <ellipse>\n");
	  fprintf(outFile,"                  <moving-point type=\"centroid\">\n");
	  fprintf(outFile,"                     <latitude>%8.4f</latitude>\n", latCent);
	  fprintf(outFile,"                     <longitude>%8.4f</longitude>\n", lonCent);
	  fprintf(outFile,"                     <polar-motion>\n");
	  fprintf(outFile,"                        <speed units=\"km h-1\">%g</speed>\n", Speed);
	  fprintf(outFile,"                        <direction_to units=\"degrees true\">%g</direction_to>\n", Direction);
	  fprintf(outFile,"                     </polar-motion>\n");
	  fprintf(outFile,"                  </moving-point>\n");
	  fprintf(outFile,"                  <major_axis units=\"km\">%g</major_axis>\n", Major);
	  fprintf(outFile,"                  <minor_axis units=\"km\">%g</minor_axis>\n", Minor);
	  fprintf(outFile,"                  <orientation units=\"degrees true\">%g</orientation>\n", Rotation);
          fprintf(outFile,"               </ellipse>\n");
	  fprintf(outFile,"            </area>\n");
	  fprintf(outFile,"         </location>\n");

	  //
	  // Print storm metrics <nowcast-parameters>
	  //
	  fprintf(outFile,"         <nowcast-parameters>\n");

          //
          // Add parent/child information to XML file if there is any.
          //
          // Parents.
          //
          const int numParents = SimpleParams.nparents;
          const int *parents = SimpleParams.parent;
          for (int i=0; i < numParents; i++){
	    fprintf(outFile,"            <ID_parent units=\"none\">%d</ID_parent>\n", parents[i]);
          }
          //
          // Children.
          //
          const int numChildren = SimpleParams.nchildren;
          const int *children = SimpleParams.child;
          for (int i=0; i < numChildren; i++){
	    fprintf(outFile,"            <ID_child units=\"none\">%d</ID_child>\n", children[i]);
          }

          fprintf(outFile,"            <reflectivity_threshold units=\"dbz\">%g</reflectivity_threshold>\n", LowDBZThresh);
          fprintf(outFile,"            <volume_threshold units=\"km3\">%g</volume_threshold>\n", SizeThreshold) ;
          fprintf(outFile,"            <height_threshold units=\"km\">%g</height_threshold>\n", MinRadarTops) ;
	  fprintf(outFile,"            <cell_volume units=\"km3\">%g</cell_volume>\n",Volume);
	  fprintf(outFile,"            <cell_top units=\"km\">%g</cell_top>\n",Top);
	  fprintf(outFile,"            <projected_area units=\"km2\">%g</projected_area>\n", ProjArea);
	  fprintf(outFile,"            <mean_area units=\"km2\">%g</mean_area>\n", MeanArea);
	  fprintf(outFile,"            <max_dbz units=\"dbz\">%g</max_dbz>\n", MaxDbz);
	  fprintf(outFile,"            <height_max_dbz units=\"km\">%g</height_max_dbz>\n",HtMaxDbz);
	  fprintf(outFile,"            <VIL units=\"kg m-2\">%g</VIL>\n",Vil);
	  fprintf(outFile,"            <storm_intensity units=\"category\">%g</storm_intensity>\n",FOKRCategory);
	  fprintf(outFile,"            <hail_probability units=\"percent\">\n");
	  fprintf(outFile,"               <value threshold_value=\"0\" threshold_units=\"mm\">%g</value>\n",WaldvogelProb*100);
	  fprintf(outFile,"            </hail_probability>\n");
	  fprintf(outFile,"            <hail_mass units=\"kg\">%g</hail_mass>\n",vertIntHailMass);
	  fprintf(outFile,"            <hail_mass_aloft units=\"kg\">%g</hail_mass_aloft>\n",HailMassAloft);
	  fprintf(outFile,"         </nowcast-parameters>\n");

	  // Close <case> node 
          fprintf(outFile,"      </case>\n");

	  numTimesOutput++;

	}    // trackEntryTime
      }    // te - track entry
    }    // st - simple tracks
  }    // ct - complex tracks

  if (numTimesOutput > 0){
    //
    // Close <event> node for last storm event.
    //
    fprintf(outFile,"   </event>\n");
    //
    // Close <nowcast-data> node
    //
    //    fprintf(outFile,"</nowcast-data>\n");

    if (_params.debug){
      cerr << numTimesOutput << " time entries for storm ID " << lastStormID;
      cerr << " in file " << outAbsFilePath << endl;
    }
  }

  //
  // Close <nowcast-data> node
  //
  fprintf(outFile,"</nowcast-data>\n");

  _closeOutput();
  return 0;

}

//
// Routine to open the output XML file and write header.
//
int Storms2Xml2::_openOutput(time_t when){

  //
  // If we are time stamping, set up the files that way.
  //
  if (_params.timestampFiles){

    outTime = when;
    date_time_t fileTime;
    fileTime.unix_time = outTime;
    uconvert_from_utime( &fileTime );

    // compute subdir path

    char outSubDir[MAX_PATH_LEN];
    sprintf(outSubDir,"%s/%.4d%.2d%.2d",
	    _params.outDir,
	    fileTime.year, fileTime.month, fileTime.day);

    // Make sure the output directory exists.
    
    if (ta_makedir_recurse(outSubDir)){
      int errNum = errno;
      cerr << "ERROR - Failed to create output directory : ";
      cerr << outSubDir << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }

    // compute rel file name and abs file path
    
    sprintf(outRelFileName,"%.4d%.2d%.2d/%.4d%.2d%.2d_%.2d%.2d%.2d_%s",
	    fileTime.year, fileTime.month, fileTime.day,
	    fileTime.year, fileTime.month, fileTime.day,
	    fileTime.hour, fileTime.min, fileTime.sec,
	    _params.outName);

    sprintf(outAbsFilePath,"%s/%s",
	    _params.outDir, outRelFileName);
    
  } else {
    //
    // We are not timestamping. In this case remove the
    // existing file before opening it to ensure that it
    // updates across a cross mount.
    //
    sprintf(outAbsFilePath,"%s/%s",
	    _params.outDir, _params.outName);
  }

  unlink(outAbsFilePath);

  outFile = fopen(outAbsFilePath,"w");

  if (outFile == NULL){
    cerr << "Failed to create " << outAbsFilePath << endl;
    return -1;
  }

  // read radar name from params file 
  char *radarName; 
  radarName = _params.radar_name;
  
  // Read radar lat, long, alt from params file 
  double _radarLat=0., _radarLon=0., _radarAlt=0.;
  _radarLat = _params.radar_location.latitude;
  _radarLon = _params.radar_location.longitude;
  _radarAlt = _params.radar_location.altitude;
  
  //
  //Print header lines for XML file  
  //
  fprintf(outFile,"<?xml version=\"1.0\" ?>\n");
  fprintf(outFile,"<wxml\n");
  fprintf(outFile,"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"1.0\"\n");
  fprintf(outFile,"xsi:schemaLocation=\"http://www.bom.gov.au/bmrc/wefor/projects/b08fdp/WxML  http://www.bom.gov.au/bmrc/wefor/projects/b08fdp/WxML/wxml.1.01.xsd\">\n");
  fprintf(outFile,"\n");

  //
  //Print <head> section to XML file 
  //
  time_t currentTime = time(NULL);
  date_time_t curTime;
  curTime.unix_time = currentTime;
  uconvert_from_utime(&curTime);

  fprintf(outFile,"<head>\n");
  fprintf(outFile,"   <product concise-name=\"track\" operational-mode=\"experimental\">\n");
  fprintf(outFile,"      <system>TITAN</system>\n");
  fprintf(outFile,"      <title>TITAN Thunderstorm Track</title>\n");
  fprintf(outFile,"      <description>B08FDB TITAN</description>\n");
  fprintf(outFile,"      <category>analysis</category>\n");
  fprintf(outFile,"      <creation-date refresh-frequency=\"PT10M\">%04d-%02d-%02dT%02d:%02d:%02d</creation-date>\n",
	  curTime.year, curTime.month, curTime.day,
	  curTime.hour, curTime.min, curTime.sec);
  fprintf(outFile,"   </product>\n");
  fprintf(outFile,"   <data-source>\n");
  fprintf(outFile,"      <radar name=\"%s\" type=\"radar\" latitude=\"%8.4f\" longitude=\"%8.4f\" />\n",
	  radarName,_radarLat,_radarLon);
  fprintf(outFile,"   </data-source>\n");
  fprintf(outFile,"   <product-source>\n");
  fprintf(outFile,"      <more-information>\n");
  fprintf(outFile,"            http://www.bom.gov.au/bmrc/wefor/projects/b08fdp/info.html\n");
  fprintf(outFile,"      </more-information>\n");
  fprintf(outFile,"      <production-center>Bureau of Meteorology\n");
  fprintf(outFile,"         <sub-center>Research Centre</sub-center>\n");
  fprintf(outFile,"      </production-center>\n");
  fprintf(outFile,"      <disclaimer>http://www.bom.gov.au/other/disclaimer.shtml</disclaimer>\n");
  fprintf(outFile,"      <credit>http://www.bom.gov.au</credit>\n");
  fprintf(outFile,"      <credit-logo>http://www.bom.gov.au/images/bom_logo.gif</credit-logo>\n");
  fprintf(outFile,"   </product-source>\n");
  fprintf(outFile,"</head>\n");

  return 0;

}


//
// Small routine to close the output file.
//
void Storms2Xml2::_closeOutput(){

  //
  // Add close tag to output XML file
  //
  fprintf(outFile,"</wxml>\n");

  fclose(outFile);

  // write ldata_info

  DsLdataInfo ldata(_params.outDir);
  ldata.setWriter("Storms2Xml2");
  ldata.setDataFileExt("xml");
  ldata.setDataType("xml");
  ldata.setRelDataPath(outRelFileName);
  ldata.write(outTime);
  
  if (_params.runScriptWhenDone){

    sleep(1); // Let the file system relax.

    char comBuffer[1024];
    sprintf(comBuffer,"%s %s", _params.scriptToRun,outAbsFilePath);

    if (_params.debug){
      cerr << "Executing command : " << comBuffer << endl;
    }

    int i = system( comBuffer );

    if (_params.debug){
      cerr << "Command returned value : " << i << endl;
    }

  }

}

