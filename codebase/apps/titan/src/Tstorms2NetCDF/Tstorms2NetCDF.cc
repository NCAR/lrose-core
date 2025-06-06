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
// Tstorms2NetCDF.cc
//
// Tstorms2NetCDF object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2025
//
///////////////////////////////////////////////////////////////
//
// Tstorms2NetCDF reads native TITAN binary data files,
// converts the data into NetCDF format,
// and writes the data out in NetCDF files.
//
////////////////////////////////////////////////////////////////

#include <iostream>

#include <toolsa/ucopyright.h>
#include <toolsa/pmu.h>
#include <toolsa/pjg.h>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaXml.hh>
#include <didss/RapDataDir.hh>
#include <dsserver/DsLdataInfo.hh>
#include <titan/TitanSpdb.hh>
#include <Spdb/DsSpdb.hh>
#include <rapformats/tstorm_hull_smooth.h>
#include "Tstorms2NetCDF.hh"

using namespace std;


// Constructor

Tstorms2NetCDF::Tstorms2NetCDF(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "Tstorms2NetCDF";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;

    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // check args in ARCHIVE mode
  
  if (_params.input_mode == Params::ARCHIVE) {
    if (_args.inputFileList.size() == 0) {
      if ((_args.startTime == 0 || _args.endTime == 0)) {
	cerr << "ERROR: " << _progName << endl;
	cerr << "In ARCHIVE mode, you must specify a file list" << endl
	     << "  or start and end times." << endl;
	isOK = FALSE;
	return;
      }
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

// destructor

Tstorms2NetCDF::~Tstorms2NetCDF()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Tstorms2NetCDF::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // set up input object
  
  if (_params.input_mode == Params::FILELIST) {
    
    if (_args.inputFileList.size() > 0) {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _args.inputFileList);
      _input->setSearchExt("th5");
    } else {
      cerr << "ERROR - Tstorms2NetCDF" << endl;
      cerr <<
	"  In FILELIST mode, you must specify "
	"files on the command line" << endl;
      return -1;
    }
    
  } else if (_params.input_mode == Params::ARCHIVE) {

    if (_args.startTime != 0 && _args.endTime != 0) {
      string inDir;
      RapDataDir.fillPath(_params.input_dir, inDir);
      if (_params.debug) {
	cerr << "Input dir: " << inDir << endl;
      }
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       inDir,
			       _args.startTime,
			       _args.endTime);
      _input->setSearchExt("th5");
    } else {
      cerr << "ERROR - Tstorms2NetCDF" << endl;
      cerr <<
	"  In ARCHIVE mode, you must specify "
	"start and end times on the command line" << endl;
      return -1;
    }

  } else if (_params.input_mode == Params::REALTIME) {

    // REALTIME mode
    
    string inDir;
    RapDataDir.fillPath(_params.input_dir, inDir);
    if (_params.debug) {
      cerr << "Input dir: " << inDir << endl;
    }
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     inDir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register);
    
  } else {

    cerr << "ERROR - unknown input_mode: " << _params.input_mode << endl;
    return -1;
    
  }
  
  if (_params.input_mode == Params::ARCHIVE) {
    _input->reset();
  }

  char *inputFilePath;
  while ((inputFilePath = _input->next()) != NULL) {

    _inputPath = inputFilePath;
    
    if (_params.debug) {
      cerr << "Processing input path: " << _inputPath << endl;
    }
    
    _processInputPath();
    
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process input data

int Tstorms2NetCDF::_processInputPath()

{
  
  // set input paths by replacing the extensions
  
  Fpath inputPath(_inputPath);
  _stormHeaderPath = inputPath.replace_extension(".sh5");
  _stormDataPath = inputPath.replace_extension(".sd5");
  _trackHeaderPath = inputPath.replace_extension(".th5");
  _trackDataPath = inputPath.replace_extension(".td5");

  // compute output path
  
  string filename = "titan_";
  filename.append(inputPath.filename());
  _ncFilePath = _params.output_dir;
  _ncFilePath.append(filename);
  _ncFilePath = _ncFilePath.replace_extension(".nc");
  
  // open input files based on the provided path

  if (_openInputFiles()) {
    cerr << "ERROR - Tstorms2NetCDF::_processInputPath" << endl;
    cerr << "  Cannot open input files, input_path: " << _inputPath << endl;
    return -1;
  }

  // open netcdf file for writing

  if (_ncFile.openNcFile(_ncFilePath.string(), NcxxFile::FileMode::replace)) {
    cerr << "ERROR - Tstorms2NetCDF::_processInputPath" << endl;
    cerr << "  Cannot open output netcdf file: " << _ncFilePath << endl;
    cerr << "  Error: " << _ncFile.getErrStr() << endl;
    return -1;
  }
  
  // load up scan times
  
  if (_loadScanTimes()) {
    return -1;
  }

  // read storm file header

  if (_sFile.ReadHeader()) {
    cerr << "ERROR - Tstorms2NetCDF::_processInputPath" << endl;
    cerr << "  Cannot read storm file header, input_path: " << _inputPath << endl;
    return -1;
  }

  // write the storm header

  _ncFile.writeStormHeader(_sFile.header());

#ifdef JUNK
  
  if (_params.input_mode == Params::REALTIME) {
    
    // REALTIME mode - find the scan which matches the latest data info
    // and only process that scan

    time_t valid_time = _input->getLdataInfo().getLatestTime();

    for (size_t iscan = 0; iscan < _scanTimes.size(); iscan++) {
      if (_scanTimes[iscan] == valid_time) {
	time_t expire_time;
	if (iscan == 0) {
	  expire_time = valid_time;
	} else {
	  expire_time = valid_time + (valid_time - _scanTimes[iscan - 1]);
	}
	_processTime(valid_time, expire_time);
	break;
      }
    }

  } else {

    // ARCHIVE or FILELIST mode
    
    for (size_t iscan = 0; iscan < _scanTimes.size(); iscan++) {
      time_t valid_time = _scanTimes[iscan];
      time_t expire_time;
      if (_scanTimes.size() == 1) {
	expire_time = valid_time;
      } else {
	if (iscan == _scanTimes.size() - 1) {
	  expire_time = valid_time + (valid_time - _scanTimes[iscan - 1]);
	} else {
	  expire_time = _scanTimes[iscan + 1];
	}
      }
      if (_params.input_mode == Params::ARCHIVE) {
	if (valid_time >= _args.startTime && valid_time <= _args.endTime) {
	  _processTime(valid_time, expire_time);
	}
      } else {
	// FILELIST mode - process all scans
	_processTime(valid_time, expire_time);
      }
    }

  }

#endif

  _closeInputFiles();
  
  return 0;

}

//////////////////////////////////////////////////
// open track and storm files,
// given the trigger path

int Tstorms2NetCDF::_openInputFiles()
  
{

  if (_tFile.OpenFiles("r", _trackHeaderPath.string().c_str())) {
    cerr << "ERROR - Tstorms2NetCDF::_openInput" << endl;
    cerr << "  " << _tFile.getErrStr() << endl;
    return -1;
  }
  
  if (_sFile.OpenFiles("r", _stormHeaderPath.string().c_str())) {
    cerr << "ERROR - Tstorms2NetCDF::_openInput" << endl;
    cerr << "  " << _sFile.getErrStr() << endl;
    return -1;
  }
  
  // lock files

  if (_tFile.LockHeaderFile("r")) {
    cerr << "ERROR - Tstorms2NetCDF::_openInput" << endl;
    cerr << "  " << _tFile.getErrStr() << endl;
    return -1;
  }
  if (_sFile.LockHeaderFile("r")) {
    cerr << "ERROR - Tstorms2NetCDF::_openInput" << endl;
    cerr << "  " << _sFile.getErrStr() << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// close track and storm input files

void Tstorms2NetCDF::_closeInputFiles()

{

  _sFile.CloseFiles();
  _tFile.CloseFiles();

}

//////////////////////////////////////////////////
// load up scan times from storm file

int Tstorms2NetCDF::_loadScanTimes()

{

  _scanTimes.clear();
  int nScans = _sFile.header().n_scans;
  for (int i = 0; i < nScans; i++) {
    // read in scan
    if (_sFile.ReadScan(i)) {
      cerr << "ERROR - Tstorms2NetCDF::_loadScanTimes" << endl;
      cerr << "  " << _sFile.getErrStr() << endl;
      return -1;
    }
    _scanTimes.push_back(_sFile.scan().time);
  }

  return 0;

}

/////////////////////
// process this scan

int Tstorms2NetCDF::_processTime(time_t valid_time,
                                 time_t expire_time)
  
{

  PMU_auto_register("Reading data....");
  
  time_t end_time = valid_time;
  time_t start_time = valid_time;
  
  if (_params.debug) {
    cerr << "Processing: " << endl;
    cerr << "  Valid time: " << DateTime::str(valid_time) << endl;
    cerr << "  Expire time: " << DateTime::str(expire_time) << endl;
    cerr << "  Start time: " << DateTime::str(start_time) << endl;
    cerr << "  End time: " << DateTime::str(end_time) << endl;
  }
  
  // read into the DsTitan object

#ifdef JUNK
  
  
  DsTitan titan;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    titan.setDebug(true);
  }
  titan.setReadClosest(valid_time, 0);
  titan.setReadAllInFile();
  if (titan.read(_params.input_dir)) {
    cerr << "ERROR - Tstorms2NetCDF::_processTime" << endl;
    cerr << titan.getErrStr() << endl;
    return -1;
  }

  // write to NetCDF

  if (_writeNetcdfFile(start_time, end_time)) {
    return -1;
  }

  string xml;
  if (_params.xml_format == Params::TSTORMS_FORMAT) {
    _loadTstormsXml(start_time, end_time,
                    valid_time, expire_time,
                    titan, xml);
  } else {
    _loadWxml(start_time, end_time,
              valid_time, expire_time,
              titan, xml);
  }

  // write to files as needed

  if (_params.write_to_xml_files) {
    _writeXmlFile(valid_time, xml);
  }

#endif
  
  return 0;

}
  
//////////////////////////////////////
// write out netcdf file

int Tstorms2NetCDF::_writeNetcdfFile(time_t start_time,
                                     time_t end_time)

{

  // storm parameters
  
  // const storm_file_params_t &stormParams = titan.storm_params();
  
  return 0;
  
}

  
#ifdef JUNK
  
////////////////////////////
// write XML to ASCII file

int Tstorms2NetCDF::_writeXmlFile(time_t valid_time,
                                  const string &xml)

{

  DateTime vtime(valid_time);

  // compute subdir path
  
  char outSubDir[MAX_PATH_LEN];
  sprintf(outSubDir,"%s/%.4d%.2d%.2d",
	  _params.xml_dir,
	  vtime.getYear(), vtime.getMonth(), vtime.getDay());

  // Make sure the output directory exists.
    
  if (ta_makedir_recurse(outSubDir)){
    int errNum = errno;
    cerr << "ERROR - Tstorms2NetCDF::_writeFile" << endl;
    cerr << "Failed to create output directory: " << outSubDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute rel file name and abs file path
  
  char outRelFileName[MAX_PATH_LEN];
  sprintf(outRelFileName,"%.4d%.2d%.2d/%.4d%.2d%.2d_%.2d%.2d%.2d_%s.xml",
	  vtime.getYear(), vtime.getMonth(), vtime.getDay(),
	  vtime.getYear(), vtime.getMonth(), vtime.getDay(),
	  vtime.getHour(), vtime.getMin(), vtime.getSec(),
	  _params.xml_name);
  
  char outAbsFilePath[MAX_PATH_LEN];
  sprintf(outAbsFilePath,"%s/%s",
	  _params.xml_dir, outRelFileName);

  if (_params.debug) {
    cerr << "Writing file: " << outAbsFilePath << endl;
  }

  // open file

  FILE *outFile = fopen(outAbsFilePath,"w");
  if (outFile == NULL){
    int errNum = errno;
    cerr << "ERROR - Tstorms2NetCDF::_writeFile" << endl;
    cerr << "Failed to open file: " << outAbsFilePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // write xml to file

  if (fwrite(xml.c_str(), 1, xml.size(), outFile) != xml.size()) {
    int errNum = errno;
    cerr << "ERROR - Tstorms2NetCDF::_writeFile" << endl;
    cerr << "Failed to write file: " << outAbsFilePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    fclose(outFile);
    return -1;
  }

  // close

  fclose(outFile);

  // write ldata_info

  DsLdataInfo ldata(_params.xml_dir);
  ldata.setWriter("Tstorms2NetCDF");
  ldata.setDataFileExt("xml");
  ldata.setDataType("xml");
  ldata.setRelDataPath(outRelFileName);
  ldata.write(valid_time);

  return 0;

}

//////////////////////////////////////
// load up XWML-format XML

void Tstorms2NetCDF::_loadTstormsXml(time_t start_time,
                                     time_t end_time,
                                     time_t valid_time,
                                     time_t expire_time,
                                     const DsTitan &titan,
                                     string &xml)
  
{
  
  // Get storm parameters - they apply to all storms
  
  const storm_file_params_t &stormParams = titan.storm_params();
  
  // add header
  
  _addTstormsHeader(xml);

  // add nowcast header

  _addStormDataHeader(xml, valid_time, stormParams);
  
  // Get the vector of complex tracks.
  
  const vector<TitanComplexTrack *> &ComplexTracks = titan.complex_tracks();

  // Loop through the complex tracks. For those that satisfy the area
  // criteria, look up the start points. Need a longevity too.
  
  for (int ct = 0; ct < (int) ComplexTracks.size(); ct++) {
    
    const TitanComplexTrack &ComplexTrack = *ComplexTracks[ct];
    
    // And the vector of simple tracks.

    const vector<TitanSimpleTrack *> &SimpleTracks =
      ComplexTrack.simple_tracks();
    const int numSimple = SimpleTracks.size();
    
    // Loop through the simple tracks.
    
    for (int st = 0; st < numSimple; st++){

      const TitanSimpleTrack &SimpleTrack = *SimpleTracks[st];

      // Get the simple track parameters.

      const simple_track_params_t &SimpleParams =  SimpleTrack.simple_params();

      // Get the vector of track entries.

      const vector<TitanTrackEntry *> &TrackEntries = SimpleTrack.entries();
      int numEntries = TrackEntries.size();

      // check that we have valid cases in the track entries
      
      bool haveValidCases = false;
      for (int te=0; te < numEntries; te++){
	const TitanTrackEntry &TrackEntry = *TrackEntries[te];
	time_t trackEntryTime = TrackEntry.entry().time;
	if ((trackEntryTime >= start_time) && (trackEntryTime <= end_time)) {
	  haveValidCases = true;
	}
      }

      // if there are valid cases, process them

      if (haveValidCases) {
	
	// add start event tag
	
	int simpleTrackNum = SimpleParams.simple_track_num;
	vector<TaXml::attribute> attrs;
	TaXml::addIntAttr("ID", simpleTrackNum, attrs);
	xml += TaXml::writeStartTag("storm", 1, attrs, true);
	
	// Loop through the track entries.
	
	for (int te=0; te < numEntries; te++){

	  const TitanTrackEntry &TrackEntry = *TrackEntries[te];
	  time_t trackEntryTime = TrackEntry.entry().time;
	  
	  bool checkForParents = false;
	  bool checkForChildren = false;
	  if (te == 0) {
	    checkForParents = true;
	  }
	  if (te == numEntries - 1) {
	    checkForChildren = true;
	  }
	  
	  // See if this is within our temporal range.

	  if (trackEntryTime == valid_time) {

	    // entry is current
	    
	    _addTstormsObs(stormParams, SimpleParams,
                           TrackEntry, TRUE,
                           checkForParents, checkForChildren, 
                           xml);
	    
	  } else if ((trackEntryTime >= start_time) &&
		     (trackEntryTime < end_time)) {
	    
	    // entry is within time limits
	    
	    _addTstormsObs(stormParams, SimpleParams,
                           TrackEntry, FALSE,
                           checkForParents, checkForChildren, 
                           xml);
	    
	  }
	  
	  if ((trackEntryTime == valid_time) && 
	      _params.generate_forecasts) {
	    for (int ii = 0; ii < _params.forecast_lead_times_n; ii++) {
	      int leadTime = _params._forecast_lead_times[ii];
	      _addTstormsForecast(leadTime, stormParams, SimpleParams,
                                  TrackEntry, xml);
	    }
	  }
          
	} // te

	// add close event tag

	xml += TaXml::writeEndTag("storm", 1);

      } // if (haveValidCases)
      
    } // st - simple tracks

  } // ct - complex tracks
  
  // add trailers
  
  _addStormDataTrailer(xml);
  _addTstormsTrailer(xml);
  
}

//////////////////////////////////////
// load up XWML-format XML

void Tstorms2NetCDF::_loadWxml(time_t start_time,
                               time_t end_time,
                               time_t valid_time,
                               time_t expire_time,
                               const DsTitan &titan,
                               string &xml)
  
{
  
  // add header
  
  _addWxmlHeader(xml);

  // add nowcast header

  _addNowcastHeader(xml, valid_time, expire_time);
  
  // Get storm parameters - they apply to all storms
  
  const storm_file_params_t &stormParams = titan.storm_params();
  
  // Get the vector of complex tracks.
  
  const vector<TitanComplexTrack *> &ComplexTracks = titan.complex_tracks();

  // Loop through the complex tracks. For those that satisfy the area
  // criteria, look up the start points. Need a longevity too.
  
  for (int ct = 0; ct < (int) ComplexTracks.size(); ct++) {
    
    const TitanComplexTrack &ComplexTrack = *ComplexTracks[ct];
    
    // And the vector of simple tracks.

    const vector<TitanSimpleTrack *> &SimpleTracks =
      ComplexTrack.simple_tracks();
    const int numSimple = SimpleTracks.size();

    // Loop through the simple tracks.
    
    for (int st = 0; st < numSimple; st++){

      const TitanSimpleTrack &SimpleTrack = *SimpleTracks[st];

      // Get the simple track parameters.

      const simple_track_params_t &SimpleParams =  SimpleTrack.simple_params();

      // Get the vector of track entries.

      const vector<TitanTrackEntry *> &TrackEntries = SimpleTrack.entries();
      int numEntries = TrackEntries.size();

      // check that we have valid cases in the track entries
      
      bool haveValidCases = false;
      for (int te=0; te < numEntries; te++){
	const TitanTrackEntry &TrackEntry = *TrackEntries[te];
	time_t trackEntryTime = TrackEntry.entry().time;
	if ((trackEntryTime >= start_time) && (trackEntryTime <= end_time)) {
	  haveValidCases = true;
	}
      }

      // if there are valid cases, process them

      if (haveValidCases) {
	
	// add start event tag
	
	int simpleTrackNum = SimpleParams.simple_track_num;
	vector<TaXml::attribute> attrs;
	TaXml::addIntAttr("ID", simpleTrackNum, attrs);
	xml += TaXml::writeStartTag("event", 1, attrs, true);
	
	// Loop through the track entries.
	
	for (int te=0; te < numEntries; te++){

	  const TitanTrackEntry &TrackEntry = *TrackEntries[te];
	  time_t trackEntryTime = TrackEntry.entry().time;
	  
	  bool checkForParents = false;
	  bool checkForChildren = false;
	  if (te == 0) {
	    checkForParents = true;
	  }
	  if (te == numEntries - 1) {
	    checkForChildren = true;
	  }
	  
	  // See if this is within our temporal range.

	  if (trackEntryTime == valid_time) {

	    // entry is current
	    
	    _addWxmlObs(stormParams, SimpleParams,
                        TrackEntry, TRUE,
                        checkForParents, checkForChildren, 
                        xml);
	    
	  } else if ((trackEntryTime >= start_time) &&
		     (trackEntryTime < end_time)) {
	    
	    // entry is within time limits
	    
	    _addWxmlObs(stormParams, SimpleParams,
                        TrackEntry, FALSE,
                        checkForParents, checkForChildren, 
                        xml);
	    
	  }
	  
	  if ((trackEntryTime == valid_time) && 
	      _params.generate_forecasts) {
	    for (int ii = 0; ii < _params.forecast_lead_times_n; ii++) {
	      int leadTime = _params._forecast_lead_times[ii];
	      _addWxmlForecast(leadTime, stormParams, SimpleParams,
                               TrackEntry, xml);
	    }
	  }

	} // te

	// add close event tag

	xml += TaXml::writeEndTag("event", 1);

      } // if (haveValidCases)
      
    } // st - simple tracks

  } // ct - complex tracks
  
  // initialize projection
  
  //   titan_grid_comps_t grid_comps;
  //   TITAN_init_proj(&sFile.scan().grid, &grid_comps);

  // add trailers
  
  _addNowcastTrailer(xml);
  _addWxmlTrailer(xml);
  
}

////////////////////////////////////
// add observed track entry to Wxml

void Tstorms2NetCDF::_addWxmlObs(const storm_file_params_t &stormParams,
                                 const simple_track_params_t &SimpleParams,
                                 const TitanTrackEntry &TrackEntry,
                                 bool isCurrent,
                                 bool checkForParents,
                                 bool checkForChildren,
                                 string &xml)

{
  
  const storm_file_scan_header_t &Scan = TrackEntry.scan();
  const track_file_entry_t &trackFileEntry = TrackEntry.entry();
  time_t trackEntryTime = trackFileEntry.time;
  
  double LowDBZThresh = stormParams.low_dbz_threshold;
  double SizeThreshold = stormParams.min_storm_size;
  double MinRadarTops = stormParams.min_radar_tops;
  int Age = SimpleParams.history_in_secs;

  // get lat, long for radar from track file.
  
  double latOrg = Scan.grid.proj_origin_lat;
  double lonOrg = Scan.grid.proj_origin_lon;
  
  const storm_file_global_props_t &Gp = TrackEntry.gprops();
  double X = Gp.proj_area_centroid_x;
  double Y = Gp.proj_area_centroid_y;
  
  double MaxDbz = Gp.dbz_max;
  double HtMaxDbz = Gp.ht_of_dbz_max;
  double Volume = Gp.volume;
  double Prec_flux = Gp.precip_flux;
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
	
  // Print debug information.

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Adding entry";
    cerr << ", Lon " << lonCent;
    cerr << ", Lat " << latCent;
    cerr << ", Age " << Age;
    cerr << ", ProjArea " << ProjArea;
    cerr << ", Time " << utimstr(trackEntryTime) << endl;
  }
	
  // Open storm event node in XML file.
	
  vector<TaXml::attribute> attrs;

  // Open <case> node in XML file

  if (isCurrent) {
    TaXml::setStringAttr("description", "current", attrs);
  } else {
    TaXml::setStringAttr("description", "history", attrs);
  }
  xml += TaXml::writeStartTag("case", 2, attrs, true);

  TaXml::setStringAttr("time-coordinate", "UTC", attrs);
  xml += TaXml::writeTime("time", 3, attrs, trackEntryTime);
	
  // Print storm <location> node.

  xml += TaXml::writeStartTag("location", 3);
  xml += TaXml::writeStartTag("area", 4);

  if (_params.include_ellipse) {
    _addEllipse(TrackEntry, latCent, lonCent, Speed, Direction,
		0, 1.0, xml);
  }
  
  if (_params.include_polygon) {
    _addPolygon(stormParams, TrackEntry,
		latCent, lonCent,
		Speed, Direction, 0, FALSE, xml);
  }

  xml += TaXml::writeEndTag("area", 4);
  xml += TaXml::writeEndTag("location", 3);

  // Print storm metrics <nowcast-parameters>

  xml += TaXml::writeStartTag("nowcast-parameters", 3);
	
  // Add parent/child information to XML file if there is any.
  // Parents.

  const int numParents = SimpleParams.nparents;
  if (checkForParents && numParents > 0) {
    const int *parents = SimpleParams.parent;
    string parentId;
    for (int i=0; i < numParents; i++){
      char id[128];
      sprintf(id, "%d", parents[i]);
      parentId += id;
      if (i != numParents - 1) {
	parentId += ",";
      }
    }
    TaXml::setStringAttr("units", "none", attrs);
    xml += TaXml::writeString("ID_parent", 4, attrs, parentId);
  }

  // Children.

  const int numChildren = SimpleParams.nchildren;
  if (checkForChildren && numChildren > 0) {
    const int *children = SimpleParams.child;
    string childId;
    for (int i=0; i < numChildren; i++){
      char id[128];
      sprintf(id, "%d", children[i]);
      childId += id;
      if (i != numChildren - 1) {
	childId += ",";
      }
    }
    TaXml::setStringAttr("units", "none", attrs);
    xml += TaXml::writeString("ID_child", 4, attrs, childId);
  }

  // other attributes

  TaXml::setStringAttr("units", "seconds", attrs);
  xml += TaXml::writeInt("age", 4, attrs, Age);

  TaXml::setStringAttr("units", "dbz", attrs);
  xml += TaXml::writeDouble("reflectivity_threshold", 4, attrs, LowDBZThresh);

  TaXml::setStringAttr("units", "km3", attrs);
  xml += TaXml::writeDouble("volume_threshold", 4, attrs, SizeThreshold);

  TaXml::setStringAttr("units", "km", attrs);
  xml += TaXml::writeDouble("height_threshold", 4, attrs, MinRadarTops);

  TaXml::setStringAttr("units", "km3", attrs);
  xml += TaXml::writeDouble("cell_volume", 4, attrs, Volume);

  if (_params.include_prec_flux_xml) {
    TaXml::setStringAttr("units", "m3/s", attrs);
    xml += TaXml::writeDouble("precipitation_flux", 4, attrs, Prec_flux);
  }

  TaXml::setStringAttr("units", "km", attrs);
  xml += TaXml::writeDouble("cell_top", 4, attrs, Top);

  TaXml::setStringAttr("units", "km2", attrs);
  xml += TaXml::writeDouble("projected_area", 4, attrs, ProjArea);
  xml += TaXml::writeDouble("mean_area", 4, attrs, MeanArea);

  TaXml::setStringAttr("units", "dbz", attrs);
  xml += TaXml::writeDouble("max_dbz", 4, attrs, MaxDbz);

  TaXml::setStringAttr("units", "km", attrs);
  xml += TaXml::writeDouble("height_max_dbz", 4, attrs, HtMaxDbz);

  TaXml::setStringAttr("units", "kg m-2", attrs);
  xml += TaXml::writeDouble("VIL", 4, attrs, Vil);

  TaXml::setStringAttr("units", "category", attrs);
  xml += TaXml::writeDouble("storm_intensity", 4, attrs, FOKRCategory);

  TaXml::setStringAttr("units", "percent", attrs);
  xml += TaXml::writeStartTag("hail_probability", 4, attrs, true);

  TaXml::setStringAttr("threshold_value", "0", attrs);
  TaXml::addStringAttr("threshold_units", "mm", attrs);
  xml += TaXml::writeDouble("value", 5, attrs, WaldvogelProb*100);
  xml += TaXml::writeEndTag("hail_probability", 4);
	
  TaXml::setStringAttr("units", "kg", attrs);
  xml += TaXml::writeDouble("hail_mass", 4, attrs, vertIntHailMass);
  xml += TaXml::writeDouble("hail_mass_aloft", 4, attrs, HailMassAloft);

  xml += TaXml::writeEndTag("nowcast-parameters", 3);
  xml += TaXml::writeEndTag("case", 2);

}

//////////////////////////////////
// add forecast to Wxml

void Tstorms2NetCDF::_addWxmlForecast(int leadTime,
                                      const storm_file_params_t &stormParams,
                                      const simple_track_params_t &SimpleParams,
                                      const TitanTrackEntry &TrackEntry,
                                      string &xml)

{
  
  const storm_file_scan_header_t &Scan = TrackEntry.scan();
  const track_file_entry_t &trackFileEntry = TrackEntry.entry();
  time_t trackEntryTime = trackFileEntry.time;
  time_t forecastTime = trackEntryTime + leadTime;
  
  double LowDBZThresh = stormParams.low_dbz_threshold;
  double SizeThreshold = stormParams.min_storm_size;
  double MinRadarTops = stormParams.min_radar_tops;
  int Age = SimpleParams.history_in_secs;

  // get lat, long for radar from track file.
  
  double latOrg = Scan.grid.proj_origin_lat;
  double lonOrg = Scan.grid.proj_origin_lon;
  
  const storm_file_global_props_t &Gp = TrackEntry.gprops();
  double X = Gp.proj_area_centroid_x;
  double Y = Gp.proj_area_centroid_y;
  
  double MaxDbz = Gp.dbz_max;
  double HtMaxDbz = Gp.ht_of_dbz_max;
  double Volume = Gp.volume;
  double Prec_flux = Gp.precip_flux;
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
  double DareaDt = trackFileEntry.dval_dt.proj_area;
  
  // current location

  double latCent, lonCent;
  if (Scan.grid.proj_type == TITAN_PROJ_LATLON){
    latCent = Y; lonCent = X;
  } else {
    PJGLatLonPlusDxDy(latOrg, lonOrg, X, Y, &latCent, &lonCent);
  }

  // position - from forecast location if applicable
  
  double latPos = latCent;
  double lonPos = lonCent;
  if (leadTime > 0) {
    double distKm = Speed * (leadTime / 3600.0);
    PJGLatLonPlusRTheta(latCent, lonCent, distKm, Direction,
			&latPos, &lonPos);
  }
  
  // forecast area

  double forecastArea = ProjArea + DareaDt * leadTime / 3600.0;
  if (forecastArea < 1.0) {
    forecastArea = 1.0;
  }
  
  double linealGrowth = 1.0;
  if(_params.forecast_growth_and_decay) {
    linealGrowth = sqrt(forecastArea / ProjArea);
  }
  
  // Print debug information.
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Adding forecast";
    cerr << ", Lon " << lonPos;
    cerr << ", Lat " << latPos;
    cerr << ", ProjArea " << ProjArea;
    cerr << ", Time " << utimstr(forecastTime) << endl;
  }
	
  // Open storm event node in XML file.
	
  vector<TaXml::attribute> attrs;

  // Open <case> node in XML file

  TaXml::setStringAttr("description", "forecast", attrs);
  xml += TaXml::writeStartTag("case", 2, attrs, true);

  TaXml::setStringAttr("time-coordinate", "UTC", attrs);
  xml += TaXml::writeTime("time", 3, attrs, forecastTime);
	
  // Print storm <location> node.

  xml += TaXml::writeStartTag("location", 3);
  xml += TaXml::writeStartTag("area", 4);

  if (_params.include_ellipse) {
    _addEllipse(TrackEntry, latPos, lonPos,
		Speed, Direction,
		leadTime, linealGrowth,
		xml);
  }

  if (_params.include_polygon) {
    _addPolygon(stormParams, TrackEntry,
		latPos, lonPos,
		Speed, Direction,
		leadTime, _params.forecast_growth_and_decay,
		xml);
  }

  xml += TaXml::writeEndTag("area", 4);
  xml += TaXml::writeEndTag("location", 3);

  // Print storm metrics <nowcast-parameters>

  xml += TaXml::writeStartTag("nowcast-parameters", 3);
	
  TaXml::setStringAttr("units", "seconds", attrs);
  xml += TaXml::writeInt("age", 4, attrs, Age);

  TaXml::setStringAttr("units", "dbz", attrs);
  xml += TaXml::writeDouble("reflectivity_threshold", 4, attrs, LowDBZThresh);

  TaXml::setStringAttr("units", "km3", attrs);
  xml += TaXml::writeDouble("volume_threshold", 4, attrs, SizeThreshold);

  TaXml::setStringAttr("units", "km", attrs);
  xml += TaXml::writeDouble("height_threshold", 4, attrs, MinRadarTops);

  TaXml::setStringAttr("units", "km3", attrs);
  xml += TaXml::writeDouble("cell_volume", 4, attrs, Volume);

  if (_params.include_prec_flux_xml) {
    TaXml::setStringAttr("units", "m3/s", attrs);
    xml += TaXml::writeDouble("precipitation_flux", 4, attrs, Prec_flux);
  }

  TaXml::setStringAttr("units", "km", attrs);
  xml += TaXml::writeDouble("cell_top", 4, attrs, Top);

  TaXml::setStringAttr("units", "km2", attrs);
  xml += TaXml::writeDouble("projected_area", 4, attrs, ProjArea);
  xml += TaXml::writeDouble("mean_area", 4, attrs, MeanArea);

  TaXml::setStringAttr("units", "dbz", attrs);
  xml += TaXml::writeDouble("max_dbz", 4, attrs, MaxDbz);

  TaXml::setStringAttr("units", "km", attrs);
  xml += TaXml::writeDouble("height_max_dbz", 4, attrs, HtMaxDbz);

  TaXml::setStringAttr("units", "kg m-2", attrs);
  xml += TaXml::writeDouble("VIL", 4, attrs, Vil);

  TaXml::setStringAttr("units", "category", attrs);
  xml += TaXml::writeDouble("storm_intensity", 4, attrs, FOKRCategory);

  TaXml::setStringAttr("units", "percent", attrs);
  xml += TaXml::writeStartTag("hail_probability", 4, attrs, true);

  TaXml::setStringAttr("threshold_value", "0", attrs);
  TaXml::addStringAttr("threshold_units", "mm", attrs);
  xml += TaXml::writeDouble("value", 5, attrs, WaldvogelProb*100);
  xml += TaXml::writeEndTag("hail_probability", 4);
	
  TaXml::setStringAttr("units", "kg", attrs);
  xml += TaXml::writeDouble("hail_mass", 4, attrs, vertIntHailMass);
  xml += TaXml::writeDouble("hail_mass_aloft", 4, attrs, HailMassAloft);

  xml += TaXml::writeEndTag("nowcast-parameters", 3);
  xml += TaXml::writeEndTag("case", 2);

}

///////////////////
// add ellipse data

void Tstorms2NetCDF::_addEllipse(const TitanTrackEntry &TrackEntry,
                                 double latPos, double lonPos,
                                 double Speed, double Direction,
                                 int leadTime, double linealGrowth,
                                 string &xml)
  
{

  // size and orientation
  
  const titan_grid_t &grid = TrackEntry.scan().grid;
  const storm_file_global_props_t &Gp = TrackEntry.gprops();
  double MajorRad = Gp.proj_area_major_radius * linealGrowth;
  double MinorRad = Gp.proj_area_minor_radius * linealGrowth;
  double Orientation = Gp.proj_area_orientation;

  // compute ellipse radii in km

  double MajorRadKm = MajorRad;
  double MinorRadKm = MinorRad;
  
  if (grid.proj_type == TITAN_PROJ_LATLON) {

    double centroidLat = Gp.proj_area_centroid_y;
    double xScaleKm = KM_PER_DEG_AT_EQ * cos(centroidLat * DEG_TO_RAD);
    double yScaleKm = KM_PER_DEG_AT_EQ;

    double OrientationRad = Orientation * DEG_TO_RAD;
    double cosOrient = cos(OrientationRad);
    double sinOrient = sin(OrientationRad);

    double majorXKm = MajorRad * sinOrient * xScaleKm;
    double majorYKm = MajorRad * cosOrient * yScaleKm;
    MajorRadKm = sqrt(majorXKm * majorXKm + majorYKm * majorYKm);
    
    double minorXKm = MinorRad * cosOrient * xScaleKm;
    double minorYKm = MinorRad * sinOrient * yScaleKm;
    MinorRadKm = sqrt(minorXKm * minorXKm + minorYKm * minorYKm);
    
  }
	
  vector<TaXml::attribute> attrs;
  
  xml += TaXml::writeStartTag("ellipse", 5);

  _addMovingPoint(latPos, lonPos, Speed, Direction, xml);
  
  TaXml::setStringAttr("units", "km", attrs);

  // Ellipse axis lengths - radius, not diameter

  xml += TaXml::writeDouble("major_axis", 6, attrs, MajorRadKm);
  xml += TaXml::writeDouble("minor_axis", 6, attrs, MinorRadKm);
  
  TaXml::setStringAttr("units", "degrees true", attrs);
  xml += TaXml::writeDouble("orientation", 6, attrs, Orientation);
  
  xml += TaXml::writeEndTag("ellipse", 5);

}

///////////////////
// add polygon data

void Tstorms2NetCDF::_addPolygon(const storm_file_params_t &sParams,
                                 const TitanTrackEntry &TrackEntry,
                                 double latPos, double lonPos,
                                 double Speed, double Direction,
                                 int leadTime, bool grow,
                                 string &xml)
  
{

  // polygon points section

  const storm_file_scan_header_t &Scan = TrackEntry.scan();
  const track_file_entry_t &trackFileEntry = TrackEntry.entry();
  const storm_file_global_props_t &Gp = TrackEntry.gprops();
  
  // initialize projection

  titan_grid_comps_t grid_comps;
  TITAN_init_proj(&Scan.grid, &grid_comps);

  // load up spdb header and entry
  
  tstorm_spdb_header_t spdbHeader;
  tstorm_spdb_entry_t spdbEntry;
  TitanSpdb::loadHeader(spdbHeader, sParams, Scan.grid,
			trackFileEntry.time, 1);
  TitanSpdb::loadEntry(spdbHeader, trackFileEntry,
		       Gp, trackFileEntry.dval_dt, grid_comps, spdbEntry);
  
  
	
  tstorm_polygon_t polygon;
  int npts = N_POLY_SIDES;
  
  // load up polygon, 0 lead time (current pos)
  
  if (_params.smooth_polygon) {
    
    tstorm_growth_hull_smooth(&spdbHeader, &spdbEntry,
			      _params.polygon_smooth_inner_mult,
			      _params.polygon_smooth_outer_mult,
			      &polygon,
			      &npts,
			      leadTime, FALSE, grow);
    
  } else {
    
    tstorm_spdb_load_growth_polygon(&spdbHeader, &spdbEntry,
				    &polygon, leadTime, grow);
    
  }

  vector<double> lats;
  vector<double> lons;
  
  for (int ii = 0; ii < npts; ii++) {
    lats.push_back(polygon.pts[ii].lat);
    lons.push_back(polygon.pts[ii].lon);
  }

  // check for closure
  
  double dlat = polygon.pts[0].lat - polygon.pts[npts-1].lat;
  double dlon = polygon.pts[0].lon - polygon.pts[npts-1].lon;
  if (fabs(dlat) > 0.0001 || fabs(dlon) > 0.0001) {
    // duplicate the start point
    lats.push_back(polygon.pts[0].lat);
    lons.push_back(polygon.pts[0].lon);
  }

  // start tag

  vector<TaXml::attribute> attrs;
  TaXml::setIntAttr("npoints", (int) lats.size(), attrs);
  xml += TaXml::writeStartTag("polygon", 5, attrs, true);

  // moving point section

  _addMovingPoint(latPos, lonPos, Speed, Direction, xml);

  // points

  for (int ii = 0; ii < (int) lats.size(); ii++) {
    TaXml::setDoubleAttr("latitude", lats[ii], attrs, "%.4f");
    TaXml::addDoubleAttr("longitude", lons[ii], attrs, "%.4f");
    xml += TaXml::writeTagClosed("point", 6, attrs);
  }
  
  // end tag

  xml += TaXml::writeEndTag("polygon", 5);

}

///////////////////////////
// add moving-point section

void Tstorms2NetCDF::_addMovingPoint(double latPos, double lonPos,
                                     double Speed, double Direction,
                                     string &xml)
  
{

  vector<TaXml::attribute> attrs;
  TaXml::setStringAttr("type", "centroid", attrs);

  xml += TaXml::writeStartTag("moving-point", 6, attrs, true);
  xml += TaXml::writeDouble("latitude", 7, latPos, "%.4f");
  xml += TaXml::writeDouble("longitude", 7, lonPos, "%.4f");
  
  xml += TaXml::writeStartTag("polar_motion", 7);
  
  TaXml::setStringAttr("units", "km h-1", attrs);
  xml += TaXml::writeDouble("speed", 8, attrs, Speed);
  
  TaXml::setStringAttr("units", "degrees true", attrs);
  xml += TaXml::writeDouble("direction_to", 8, attrs, Direction);
  
  xml += TaXml::writeEndTag("polar_motion", 7);
  xml += TaXml::writeEndTag("moving-point", 6);
  
}

//////////////////
// add wxml header

void Tstorms2NetCDF::_addWxmlHeader(string &xml)
  
{
  
  // preamble

  xml += TaXml::writeString("<?xml version=\"1.0\" ?>\n");
  
  // opening <wxml> tag

  vector<TaXml::attribute> attrs;
  TaXml::addStringAttr("xmlns:xsi", _params.schema_instance, attrs);
  TaXml::addStringAttr("xmlns", _params.namespace_location, attrs);
  TaXml::addStringAttr("version", _params.version, attrs);
  TaXml::addStringAttr("xsi:schemaLocation",
		       _params.schema_location, attrs);
  
  xml += TaXml::writeStartTag("wxml", 0, attrs, true);
  
  // <head> section

  time_t currentTime = time(NULL);
  xml += TaXml::writeStartTag("head", 0);

  // product
 
  TaXml::setStringAttr("concise-name", "track", attrs);
  TaXml::addStringAttr("operational-mode",
		       _params.operational_mode, attrs);

  xml += TaXml::writeStartTag("product", 1, attrs, true);
  xml += TaXml::writeString("system", 2, "TITAN");
  xml += TaXml::writeString("title", 2, "TITAN Thunderstorm Track");
  xml += TaXml::writeString("description", 2, _params.product_description);
  xml += TaXml::writeString("category", 2, "analysis");
  
  TaXml::setStringAttr("refresh-frequency",
		       _params.refresh_frequency, attrs);
  xml += TaXml::writeTime("creation-date", 2, attrs, currentTime);
  
  xml += TaXml::writeEndTag("product", 1);

  // data source - radar

  xml += TaXml::writeStartTag("data-source", 1);
  TaXml::setStringAttr("name", _params.radar_name, attrs);
  TaXml::addStringAttr("type", "radar", attrs);
  TaXml::addDoubleAttr("latitude", _params.radar_latitude_deg, attrs, "%.4f");
  TaXml::addDoubleAttr("longitude", _params.radar_longitude_deg, attrs, "%.4f");
  xml += TaXml::writeTagClosed("radar", 2, attrs);
  xml += TaXml::writeEndTag("data-source", 1);
  
  // product source
  
  xml += TaXml::writeStartTag("product-source", 1);
  xml += TaXml::writeString("more-information", 2,
			    _params.more_information);
  xml += TaXml::writeString("production-center", 2,
			    _params.production_center);
  xml += TaXml::writeString("disclaimer", 2,
			    _params.disclaimer);
  xml += TaXml::writeString("credit", 2, _params.credit);
  xml += TaXml::writeString("credit-logo", 2,
			    _params.credit_logo);
  xml += TaXml::writeEndTag("product-source", 1);
  
  xml += TaXml::writeEndTag("head", 0);
  
}

//////////////////
// add wxml trailer

void Tstorms2NetCDF::_addWxmlTrailer(string &xml)

{

  xml += TaXml::writeEndTag("wxml", 0);

}

/////////////////////
// add nowcast header

void Tstorms2NetCDF::_addNowcastHeader(string &xml,
                                       time_t valid_time,
                                       time_t expire_time)
  
{

  xml += TaXml::writeStartTag("nowcast-data", 0);

  xml += TaXml::writeTime("observation-time", 1, valid_time);

  vector<TaXml::attribute> attrs;
  TaXml::setStringAttr("time-coordinate", "UTC", attrs);
  xml += TaXml::writeStartTag("time-layout", 1, attrs, true);
  xml += TaXml::writeTime("start-valid-time", 2, valid_time);
  xml += TaXml::writeTime("end-valid-time", 2, expire_time);
  xml += TaXml::writeEndTag("time-layout", 1);
  
}

//////////////////////
// add nowcast trailer

void Tstorms2NetCDF::_addNowcastTrailer(string &xml)

{
  
  xml += TaXml::writeEndTag("nowcast-data", 0);

}

//////////////////////
// add tstorms header

void Tstorms2NetCDF::_addTstormsHeader(string &xml)
  
{
  
  // preamble
  
  xml += TaXml::writeString("<?xml version=\"1.0\" ?>\n");
  
  // opening <tstorms> tag
  
  vector<TaXml::attribute> attrs;
  TaXml::addStringAttr("xmlns:xsi", _params.schema_instance, attrs);
  // TaXml::addStringAttr("xmlns", _params.namespace_location, attrs);
  TaXml::addStringAttr("version", _params.version, attrs);
  TaXml::addStringAttr("xsi:schemaLocation",
		       _params.schema_location, attrs);
  
  xml += TaXml::writeStartTag("tstorms", 0, attrs, true);
  
  // <head> section

  time_t currentTime = time(NULL);
  xml += TaXml::writeStartTag("head", 0);

  // product
 
  xml += TaXml::writeString("system", 2, "TITAN");
  xml += TaXml::writeString("title", 2, "TITAN Thunderstorm Tracks");
  xml += TaXml::writeString("product-description", 2,
                            _params.product_description);
  
  xml += TaXml::writeString("time-coordinate", 2, "UTC");
  xml += TaXml::writeTime("write-time", 2, currentTime);
  
  xml += TaXml::writeEndTag("head", 0);
  
}

///////////////////////////
// add tstorms trailer

void Tstorms2NetCDF::_addTstormsTrailer(string &xml)

{

  xml += TaXml::writeEndTag("tstorms", 0);

}

/////////////////////
// add StormData header

void Tstorms2NetCDF::_addStormDataHeader(string &xml,
                                         time_t valid_time,
                                         const storm_file_params_t &stormParams)
 
{
  
  xml += TaXml::writeStartTag("storm-data", 0);
  xml += TaXml::writeTime("observation-time", 1, valid_time);
  xml += TaXml::writeDouble("dbz-threshold", 1,
                            stormParams.low_dbz_threshold);
  
}

//////////////////////
// add StormData trailer

void Tstorms2NetCDF::_addStormDataTrailer(string &xml)

{
  
  xml += TaXml::writeEndTag("storm-data", 0);

}

///////////////////////////////////////////
// add observed track entry to tstorms xml

void Tstorms2NetCDF::_addTstormsObs(const storm_file_params_t &stormParams,
                                    const simple_track_params_t &SimpleParams,
                                    const TitanTrackEntry &TrackEntry,
                                    bool isCurrent,
                                    bool checkForParents,
                                    bool checkForChildren,
                                    string &xml)

{
  
  const storm_file_scan_header_t &Scan = TrackEntry.scan();
  const track_file_entry_t &trackFileEntry = TrackEntry.entry();
  time_t trackEntryTime = trackFileEntry.time;
  
  int Age = SimpleParams.history_in_secs;

  // get lat, long for radar from track file.
  
  double latOrg = Scan.grid.proj_origin_lat;
  double lonOrg = Scan.grid.proj_origin_lon;
  
  const storm_file_global_props_t &Gp = TrackEntry.gprops();
  double X = Gp.proj_area_centroid_x;
  double Y = Gp.proj_area_centroid_y;
  
  double MaxDbz = Gp.dbz_max;
  double Top = Gp.top;	 
  double Speed = trackFileEntry.dval_dt.smoothed_speed;
  double Direction = trackFileEntry.dval_dt.smoothed_direction;
	
  double latCent, lonCent;
  if (Scan.grid.proj_type == TITAN_PROJ_LATLON){
    latCent = Y; lonCent = X;
  } else {
    PJGLatLonPlusDxDy(latOrg, lonOrg, X, Y, &latCent, &lonCent);
  }
	
  // Print debug information.

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Adding entry";
    cerr << ", Lon " << lonCent;
    cerr << ", Lat " << latCent;
    cerr << ", Age " << Age;
    cerr << ", Time " << utimstr(trackEntryTime) << endl;
  }
	
  // Open storm event node in XML file.
	
  vector<TaXml::attribute> attrs;

  // Open <case> node in XML file
  
  if (isCurrent) {
    TaXml::setStringAttr("relative-time", "current", attrs);
  } else {
    TaXml::setStringAttr("relative-time", "history", attrs);
  }
  xml += TaXml::writeStartTag("instance", 2, attrs, true);
  
  xml += TaXml::writeTime("time", 3, trackEntryTime);
	
  // Print storm <location> node.

  xml += TaXml::writeStartTag("location", 3);
  xml += TaXml::writeStartTag("area", 4);

  if (_params.include_ellipse) {
    _addEllipse(TrackEntry, latCent, lonCent, Speed, Direction,
		0, 1.0, xml);
  }
  
  if (_params.include_polygon) {
    _addPolygon(stormParams, TrackEntry,
		latCent, lonCent,
		Speed, Direction, 0, FALSE, xml);
  }

  xml += TaXml::writeEndTag("area", 4);
  xml += TaXml::writeEndTag("location", 3);

  // Print storm metrics <nowcast-parameters>

  xml += TaXml::writeStartTag("storm-parameters", 3);
	
  // Add parent/child information to XML file if there is any.
  // Parents.

  const int numParents = SimpleParams.nparents;
  if (checkForParents && numParents > 0) {
    const int *parents = SimpleParams.parent;
    string parentId;
    for (int i=0; i < numParents; i++){
      char id[128];
      sprintf(id, "%d", parents[i]);
      parentId += id;
      if (i != numParents - 1) {
	parentId += ",";
      }
    }
    TaXml::setStringAttr("units", "none", attrs);
    xml += TaXml::writeString("ID_parent", 4, attrs, parentId);
  }

  // Children.

  const int numChildren = SimpleParams.nchildren;
  if (checkForChildren && numChildren > 0) {
    const int *children = SimpleParams.child;
    string childId;
    for (int i=0; i < numChildren; i++){
      char id[128];
      sprintf(id, "%d", children[i]);
      childId += id;
      if (i != numChildren - 1) {
	childId += ",";
      }
    }
    TaXml::setStringAttr("units", "none", attrs);
    xml += TaXml::writeString("ID_child", 4, attrs, childId);
  }

  // other attributes

  TaXml::setStringAttr("units", "seconds", attrs);
  xml += TaXml::writeInt("age", 4, attrs, Age);

  TaXml::setStringAttr("units", "km", attrs);
  xml += TaXml::writeDouble("cell_top", 4, attrs, Top);

  TaXml::setStringAttr("units", "dbz", attrs);
  xml += TaXml::writeDouble("max_dbz", 4, attrs, MaxDbz);

  xml += TaXml::writeEndTag("storm-parameters", 3);
  xml += TaXml::writeEndTag("instance", 2);

}

//////////////////////////////////
// add forecast to tstorms xml

void Tstorms2NetCDF::_addTstormsForecast(int leadTime,
                                         const storm_file_params_t &stormParams,
                                         const simple_track_params_t &SimpleParams,
                                         const TitanTrackEntry &TrackEntry,
                                         string &xml)
  
{
  
  const storm_file_scan_header_t &Scan = TrackEntry.scan();
  const track_file_entry_t &trackFileEntry = TrackEntry.entry();
  time_t trackEntryTime = trackFileEntry.time;
  time_t forecastTime = trackEntryTime + leadTime;
  
  int Age = SimpleParams.history_in_secs;

  // get lat, long for radar from track file.
  
  double latOrg = Scan.grid.proj_origin_lat;
  double lonOrg = Scan.grid.proj_origin_lon;
  
  const storm_file_global_props_t &Gp = TrackEntry.gprops();
  double X = Gp.proj_area_centroid_x;
  double Y = Gp.proj_area_centroid_y;
  
  double MaxDbz = Gp.dbz_max;
  double Top = Gp.top;	 
  double ProjArea = Gp.proj_area;
  double Speed = trackFileEntry.dval_dt.smoothed_speed;
  double Direction = trackFileEntry.dval_dt.smoothed_direction;
  double DareaDt = trackFileEntry.dval_dt.proj_area;
  
  // current location

  double latCent, lonCent;
  if (Scan.grid.proj_type == TITAN_PROJ_LATLON){
    latCent = Y; lonCent = X;
  } else {
    PJGLatLonPlusDxDy(latOrg, lonOrg, X, Y, &latCent, &lonCent);
  }

  // position - from forecast location if applicable
  
  double latPos = latCent;
  double lonPos = lonCent;
  if (leadTime > 0) {
    double distKm = Speed * (leadTime / 3600.0);
    PJGLatLonPlusRTheta(latCent, lonCent, distKm, Direction,
			&latPos, &lonPos);
  }
  
  // forecast area

  double forecastArea = ProjArea + DareaDt * leadTime / 3600.0;
  if (forecastArea < 1.0) {
    forecastArea = 1.0;
  }
  
  double linealGrowth = 1.0;
  if(_params.forecast_growth_and_decay) {
    linealGrowth = sqrt(forecastArea / ProjArea);
  }
  
  // Print debug information.
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Adding forecast";
    cerr << ", Lon " << lonPos;
    cerr << ", Lat " << latPos;
    cerr << ", ProjArea " << ProjArea;
    cerr << ", Time " << utimstr(forecastTime) << endl;
  }
	
  // Open storm event node in XML file.
	
  vector<TaXml::attribute> attrs;

  // Open <case> node in XML file

  TaXml::setStringAttr("relative-time", "forecast", attrs);
  xml += TaXml::writeStartTag("instance", 2, attrs, true);

  xml += TaXml::writeTime("time", 3, forecastTime);
	
  // Print storm <location> node.

  xml += TaXml::writeStartTag("location", 3);
  xml += TaXml::writeStartTag("area", 4);

  if (_params.include_ellipse) {
    _addEllipse(TrackEntry, latPos, lonPos,
		Speed, Direction,
		leadTime, linealGrowth,
		xml);
  }

  if (_params.include_polygon) {
    _addPolygon(stormParams, TrackEntry,
		latPos, lonPos,
		Speed, Direction,
		leadTime, _params.forecast_growth_and_decay,
		xml);
  }

  xml += TaXml::writeEndTag("area", 4);
  xml += TaXml::writeEndTag("location", 3);

  // Print storm metrics <nowcast-parameters>

  xml += TaXml::writeStartTag("storm-parameters", 3);
  
  TaXml::setStringAttr("units", "seconds", attrs);
  xml += TaXml::writeInt("age", 4, attrs, Age);
  
  TaXml::setStringAttr("units", "km", attrs);
  xml += TaXml::writeDouble("cell_top", 4, attrs, Top);
  
  TaXml::setStringAttr("units", "dbz", attrs);
  xml += TaXml::writeDouble("max_dbz", 4, attrs, MaxDbz);
  
  xml += TaXml::writeEndTag("storm-parameters", 3);
  xml += TaXml::writeEndTag("instance", 2);

}

// store XML data read in from a file

int Tstorms2NetCDF::_storeXmlFromFile()

{

  // read in xml from file

  FILE *xml;
  if ((xml = fopen(_args.xmlFilePath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Tstorms2NetCDF::_storeXmlFromFile" << endl;
    cerr << "  Cannot open xml input file: " << _args.xmlFilePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  string xmlBuf;
  char line[1024];
  
  while (fgets(line, 1024, xml) != NULL) {
    xmlBuf += line;
  }
  
  fclose(xml);

  // write to Spdb
 
  if (_writeToSpdb(_args.xmlValidTime, _args.xmlValidTime, xmlBuf)) {
    cerr << "ERROR - Tstorms2NetCDF::_storeXmlFromFile" << endl;
    cerr << "  Cannot store XML to SPDB" << endl;
    return -1;
  }
  
  return 0;

}

#endif
