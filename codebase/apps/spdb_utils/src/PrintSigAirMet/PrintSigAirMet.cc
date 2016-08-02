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
// PrintSigAirMet.cc
//
// PrintSigAirMet object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2003
//
///////////////////////////////////////////////////////////////

#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/ushmem.h>
#include <toolsa/pjg.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include "PrintSigAirMet.hh"
using namespace std;

// Constructor

PrintSigAirMet::PrintSigAirMet(int argc, char **argv)
  
{
  
  isOK = true;
  _coordShmem = NULL;

  // set programe name

  _progName = "PrintSigAirMet";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = const_cast<char*>(string("unknown").c_str());
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // Condition search longitudes

  if (_params.bounding_box.min_lon > 180) {
    _params.bounding_box.min_lon -= 360;
  }
  if (_params.bounding_box.max_lon > 180) {
    _params.bounding_box.max_lon -= 360;
  }

  // Attach to Cidd's shared memory segment
  
  if (_params.follow_cidd) {
    if ((_coordShmem =
	 (coord_export_t *) ushm_create(_params.coord_shmem_key,
					sizeof(coord_export_t),
					0666)) == NULL) {
      cerr << "ERROR: " << _progName << endl;
      cerr <<
	"  Cannot create/attach Cidd's coord export shmem segment." << endl;
      isOK = false;
    }
  }
  
  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

// destructor

PrintSigAirMet::~PrintSigAirMet()

{

  // free up

  _clearReports();

  // detach from shared memory

  if (_coordShmem != NULL) {
    ushm_detach(_coordShmem);
  }
  
  // unregister process
  
  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int PrintSigAirMet::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  int iret = 0;
  
  if (_params.follow_cidd) {
    
    while (true) {
      if (_follow_cidd()) {
	iret = -1;
      }
      umsleep(_params.sleep_msecs);
     }

  } else {

    if (_run_once()) {
      iret = -1;
    }

  }

  return iret;

}
  
//////////////////////////////////////////////////
// Run Once

int PrintSigAirMet::_run_once()
{

  DateTime retrieveTime(_params.valid_time.year,
			_params.valid_time.month,
			_params.valid_time.day,
			_params.valid_time.hour,
			_params.valid_time.min,
			_params.valid_time.sec);


  // Print banner

  if (_params.print_banner) {
    char tmpStr[128];

    if (_args.headerTextSet) {
      cout << _args.headerText << endl;
    }

    if (_args.doValid) {
      sprintf(tmpStr, "%d/%02d/%02d %02d:%02d:%02d", _params.valid_time.year, _params.valid_time.month, _params.valid_time.day, _params.valid_time.hour, _params.valid_time.min, _params.valid_time.sec);
      cout << "Retrieving reports valid at: " << tmpStr << " Z" << endl;
    } else {
      fprintf(stdout, "Retrieving reports from: %s to %s\n",
	      utimstr(_args.startTime), utimstr(_args.endTime));
    }
  }

  // Retrieve and print reports

  if (!_retrieve(retrieveTime.utime(), false, 0,
                 _args.dataType2, _args.doValid,
                 _args.startTime, _args.endTime)) {
    _printNoDataMsg();
    return(0);
  }

  if (_params.apply_bounding_box) {
    _sortByBoundingBox(_params.bounding_box.min_lat,
		       _params.bounding_box.min_lon,
		       _params.bounding_box.max_lat,
		       _params.bounding_box.max_lon);

  } else if (_args.weatherTypeSet) {
    _sortByWeatherType(_args.weatherType);

  } else if (_args.dataTypes.size() > 0) {
    _sortByDataType(_args.dataTypes, _args.dataType2, retrieveTime.utime(), _args.doValid, _args.startTime, _args.endTime);

  } else {
    _sortByDistance(_params.closest_point.lat,
		    _params.closest_point.lon);
  }

  if (_reports.size() == 0) {
    _printNoDataMsg();
  } else {
    _print();
  }

  return 0;

}

//////////////////////////////////////////////////
// Follow CIDD

int PrintSigAirMet::_follow_cidd()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Make sure the display is ready before using the shared memory

  while (!_coordShmem->shmem_ready && !_params.no_wait_for_shmem) {
    PMU_auto_register("Waiting for shmem_ready flag");
    umsleep(_params.sleep_msecs);
  }
  
  // Store the latest click number at startup
  
  // int prev_seq_num = _coordShmem->pointer_seq_num;
  int prev_seq_num = -1;
  time_t last_display_time = -1;
  
  // Now, operate forever
  
  while (true) {
    
    // Register with the process mapper
    
    PMU_auto_register("Checking for user input");
    
    // Check for new clicks
    
    time_t curr_time = time(NULL);
    
    if (_coordShmem->pointer_seq_num != prev_seq_num ||
	(_params.auto_click_interval > 0 &&
	 abs(curr_time - last_display_time) > _params.auto_click_interval)) {
      
      if (_coordShmem->pointer_seq_num != prev_seq_num) {
	
	if (_params.mouse_button == 0 ||
	    _coordShmem->button == _params.mouse_button) {
	  
	  time_t retrieveTime;
	  if(_params.use_cidd_time) {
	    retrieveTime =
              _coordShmem->time_cent + _params.cidd_time_adjustment;
	  } else {
	    retrieveTime = curr_time;
	  }
	  
	  if (_params.debug) {
	    fprintf(stderr,
		    "Click - lat = %g, lon = %g, mouse button = %d\n",
		    _coordShmem->pointer_lat,
		    _coordShmem->pointer_lon,
		    (int) _coordShmem->button);
	    if(_params.use_cidd_time) {
	      fprintf(stderr, "Following CIDD time: %s\n",
		      utimstr(retrieveTime));
	    } else {
	      fprintf(stderr, "Using current time: %s\n",
		      utimstr(retrieveTime));
	    }
	  }
	  
	  if (!_retrieve(retrieveTime, false, 0, _args.dataType2, _args.doValid, _args.startTime, _args.endTime)) {
	    _printNoDataMsg();
	  } else {
	    _sortByDistance(_coordShmem->pointer_lat,
			    _coordShmem->pointer_lon);
	    if (_reports.size() == 0) {
	      _printNoDataMsg();
	    } else {
	      _print();
	    }
	  }

	  last_display_time = curr_time;
	  
	} else {
	  
	  if (_params.debug) {
	    fprintf(stderr, "   Not processing clicks for mouse button %d\n",
		    (int) _coordShmem->button);
	  }

	}
      
	prev_seq_num = _coordShmem->pointer_seq_num;
	
      }
      
    } // if (_coordShmem->pointer_seq_num != prev_seq_num ... 
    
    umsleep(_params.sleep_msecs);
    
  } // while(forever)
  
  return 0;

}

////////////////////////////////////////////
// get the data
//
// Returns true on success, false on failure

bool PrintSigAirMet::_retrieve(time_t retrieveTime, bool doDataTypes,
			       int dataType, int dataType2, bool doValid,
			       time_t startTime, time_t endTime)
  
{

  int success;

  // should we check write time?
  
  if (_params.use_cidd_time && _coordShmem &&
      _coordShmem->checkWriteTimeOnRead) {
    _spdb.setCheckWriteTimeOnGet(_coordShmem->latestValidWriteTime);
  } else {
    _spdb.clearCheckWriteTimeOnGet();
  }
  
  // Request the data from the spdb database

  if (doValid) {
    if (doDataTypes && dataType2 == -1) {
      success=_spdb.getValid(_params.input_url, retrieveTime, dataType);
    } else if (doDataTypes && dataType2 != -1) {
      success=_spdb.getValid(_params.input_url, retrieveTime, dataType, dataType2);
    } else {
      success=_spdb.getValid(_params.input_url, retrieveTime);
    }
  } else {
    if (doDataTypes && dataType2 == -1) {
      success=_spdb.getInterval(_params.input_url, startTime, endTime, dataType);
    } else if (doDataTypes && dataType2 != -1) {
      success=_spdb.getInterval(_params.input_url, startTime, endTime, dataType, dataType2);
    } else {
      success=_spdb.getInterval(_params.input_url, startTime, endTime);
    }
  }

  if (success == -1) {
    if (_params.log_spdb_errors) {
      MsgLog msgLog;
      msgLog.setApplication(_progName, _params.instance);
      msgLog.setSuffix("log");
      msgLog.setAppendMode();
      msgLog.setDayMode();
      msgLog.setOutputDir(_params.errors_log_dir);
      msgLog.postMsg("ERROR - PrintSigAirMet::_retrieve");
      msgLog.postMsg("  Retrieving data.");
      msgLog.postMsg("%s", _spdb.getErrStr().c_str());
    } else {
      cerr << "ERROR - PrintSigAirMet::_retrieve" << endl;
      cerr << "  Retrieving data." << endl;
      cerr << "  " << _spdb.getErrStr() << endl;
    }
    return false;
  }
  
  if (_params.debug) {
    cerr << "Retrieved " << _spdb.getNChunks() << " chunks from "
	 << _params.input_url << endl;
    cerr << "valid_time: " << DateTime::str(retrieveTime) << endl;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    Spdb::chunk_ref_t *refs = _spdb.getChunkRefs();
    for (int i = 0; i < _spdb.getNChunks(); i++) {
      cerr << "station, id: "
	   << Spdb::dehashInt32To4Chars(refs[i].data_type)
	   << ", " << refs[i].data_type2 << endl;
    }
  }

  if (_spdb.getNChunks() > 0) {
    return true;
  } else {
    return false;
  }

}

///////////////////////////////////////////////////////
// sort by distance from a point

void PrintSigAirMet::_sortByDistance(double search_lat,
				     double search_lon)
  
{
  
  // decode sigmets

  const vector<Spdb::chunk_t> &chunks = _spdb.getChunks();
  vector<SigAirMet *> all;
  for (size_t ii = 0; ii < chunks.size(); ii++) {
    SigAirMet *sa = new SigAirMet();
    if (sa->disassemble(chunks[ii].data, chunks[ii].len) == 0 &&
	sa->centroidSet() &&
	(_params.print_cancelled_reports || !sa->getCancelFlag())) {
      all.push_back(sa);
    } else {
      delete sa;
    }
  }
  
  // sort sigmets in map by distance from search point

  distmap_t distMap;
  for (int ii = (int) all.size() - 1; ii >= 0; ii--){
    SigAirMet *sa = all[ii];
    double dist, dirn;
    PJGLatLon2RTheta(sa->getCentroidLat(), sa->getCentroidLon(),
		     search_lat, search_lon,
		     &dist, &dirn);
    distpair_t pp(dist, sa);
    distMap.insert(pp);
  } // ii
  
  // load up closest nmax_print reports, free others
  
  _clearReports();
  distmap_t::iterator ii;
  int count = 0;
  for (ii = distMap.begin(); ii != distMap.end(); ii++, count++) {
    SigAirMet *sa = (*ii).second;
    if (_params.nmax_items_print < 0 ||
	count < _params.nmax_items_print) {
      _reports.push_back(sa);
    } else {
      delete sa;
    }
  }
      
}

///////////////////////////////////////////////////////
// sort by bounding box

void PrintSigAirMet::_sortByBoundingBox(double min_lat,
					double min_lon,
					double max_lat,
					double max_lon)
  
{

  string funcname="_sortByBoundingBox";

  // decode sigmets
  
  const vector<Spdb::chunk_t> &chunks = _spdb.getChunks();
  vector<SigAirMet *> all;
  for (size_t ii = 0; ii < chunks.size(); ii++) {
    SigAirMet *sa = new SigAirMet();
    if (sa->disassemble(chunks[ii].data, chunks[ii].len) == 0 &&
	sa->centroidSet() &&
	(_params.print_cancelled_reports || !sa->getCancelFlag())) {
      all.push_back(sa);
    } else {
      delete sa;
    }
  }
  
  // condition longitudes

  bool straddlesDateLine = false;
  double lon2 = max_lon;
  if (min_lon > max_lon) {
    lon2 += 360.0;
    straddlesDateLine = true;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << funcname.c_str() << ": straddlesDateLine: " << straddlesDateLine << endl;
    cerr << "   adjusted max_lon lon2: " << lon2 << endl;
    cerr << "   min_lat: " << min_lat << ", min_lon: " << min_lon << endl;
    cerr << "   max_lat: " << max_lat << ", max_lon: " << max_lon << endl;
  }

  // include only those within bounding box

  _clearReports();
  for (size_t ii = 0; ii < all.size(); ii++) {
    SigAirMet *sa = all[ii];
    double lat = sa->getCentroidLat();
    double lon = sa->getCentroidLon();
    bool withinBox = false;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << funcname.c_str() << ": " << sa->getSource() << "/" << sa->getId() << ": centroid lat: " << lat << ", lon: " << lon << endl;
    }

    if (straddlesDateLine) {
      if (lon < max_lon) {
	lon += 360.0;
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << funcname.c_str() << ": Adjusting lon, new lon: " << lon << endl;
	}
      }
      if (lat >= min_lat && lat <= max_lat &&
	  lon >= min_lon && lon <= lon2) {
	withinBox = true;
      }
    } else {
      if (lat >= min_lat && lat <= max_lat &&
	  lon >= min_lon && lon <= max_lon) {
	withinBox = true;
      }
    }
    if (withinBox) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << " ... is within bounding box" << endl;
      }
      _reports.push_back(sa);
    } else {
      delete sa;
    }
  }

}


///////////////////////////////////////////////////////
// sort by weather type

void PrintSigAirMet::_sortByWeatherType(string weather_type)
  
{

  string funcname="_sortByWeatherType";

  // decode sigmets
  
  const vector<Spdb::chunk_t> &chunks = _spdb.getChunks();
  vector<SigAirMet *> all;
  for (size_t ii = 0; ii < chunks.size(); ii++) {
    SigAirMet *sa = new SigAirMet();
    if (sa->disassemble(chunks[ii].data, chunks[ii].len) == 0 &&
	(_params.print_cancelled_reports || !sa->getCancelFlag())) {
      all.push_back(sa);
    } else {
      delete sa;
    }
  }
  
  // include only those that match the weather type

  _clearReports();
  for (size_t ii = 0; ii < all.size(); ii++) {
    SigAirMet *sa = all[ii];
    string WxString = sa->getWx();

    if (_WildCard((char *)weather_type.c_str(), (char *)WxString.c_str(),0)) {

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << funcname.c_str() << ": found match with weather_type" << weather_type.c_str() << ", to sigmet weather string: " << WxString.c_str() << endl;
      }

      _reports.push_back(sa);

    } else {
      delete sa;
    }
  }
}

///////////////////////////////////////////////////////
// sort by data type

void PrintSigAirMet::_sortByDataType(vector <int> dataTypes, int dataType2,
				     time_t retrieveTime, bool doValid,
				     time_t startTime, time_t endTime)
				     
{
  string funcname="_sortByDataType";

  // Request the data from the spdb database

  _clearReports();

  for (size_t ii=0; ii< dataTypes.size(); ii++) {

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << funcname.c_str() <<  ": dataType: " << dataTypes[ii] << ", dataType2: " << dataType2 << endl;
    }

    if (_retrieve(retrieveTime, true, dataTypes[ii], dataType2, doValid, startTime, endTime)) {

      // decode sigmets
  
      const vector<Spdb::chunk_t> &chunks = _spdb.getChunks();
      for (size_t ii = 0; ii < chunks.size(); ii++) {
	SigAirMet *sa = new SigAirMet();
	if (sa->disassemble(chunks[ii].data, chunks[ii].len) == 0 &&
	    (_params.print_cancelled_reports || !sa->getCancelFlag())) {
	  _reports.push_back(sa);
	} else {
	  delete sa;
	}
      }
    } //endif
  } // endfor
}


//////////////////////////////////////
// print no data message

void PrintSigAirMet::_printNoDataMsg()
  
{
  
  // scroll down
  
  for (int i = 0; i < _params.nlines_scroll_before_printing; i++) {
    cout << endl;
  }

  // Print no data message
  
  cout << _params.no_data_message << endl;
  cout << endl;
  cout.flush();
  
}

////////////////////////////////////
// normal print

void PrintSigAirMet::_print()
  
{
  const double EPSILON = 10E-6;
  // scroll down

  for (int i = 0; i < _params.nlines_scroll_before_printing; i++) {
    cout << endl;
  }

  // print delimiter

  if (_params.delimiter_line_between_groups) {
    for (int i = 0; i < _params.max_line_length; i++) {
      cout << "=";
    }
    cout << endl;
  }

  // print out sigmets

  for (size_t ii = 0; ii < _reports.size(); ii++) {

    int jj = (int) ii;
    if (_params.reverse) {
      jj = _reports.size() - 1 - ii;
    }

    SigAirMet *sa = _reports[jj];

    if (_params.print_source_id) {

      if (sa->getGroup() == SIGMET_GROUP) {
	cout << "SIGMET: ";
      } else {
	cout << "AIRMET: ";
      }
      cout << sa->getSource() << "/" << sa->getId() << endl;
    }

    if (_params.print_start_time) {
      cout << "  Start time:    " << DateTime::str(sa->getStartTime()) << endl;
    }

    if (_params.print_end_time) {
      cout << "  Expire time:   " << DateTime::str(sa->getEndTime()) << endl;
    }

    if (sa->getCancelFlag()) {
      cout << "  ** CANCELLED **" << endl;
    }

    if (_params.print_weather_type) {
      cout << "  Weather type:  " << sa->getWx() << endl;
    }

    // Flight levels have -1 for the bottom if have a top but no bottom
    // and -1 for the top if have a bottom but no top

    if ((_params.print_flight_levels) &&
	(sa->flightLevelsSet())) {
      char tmpStr[128];
      if (sa->getBottomFlightLevel() > 0 &&
	  sa->getTopFlightLevel() < 0) {
	sprintf(tmpStr, "Above FL %.3d",
		(int) (sa->getBottomFlightLevel() + 0.5));
      }
      else if (sa->getBottomFlightLevel() < 0 &&
	       sa->getTopFlightLevel() > 0) {
	sprintf(tmpStr, "Below FL %.3d",
		(int) (sa->getTopFlightLevel() + 0.5));
      } else if (sa->getBottomFlightLevel() < EPSILON){
        sprintf(tmpStr, "SFC - FL%.3d",
                (int) (sa->getTopFlightLevel() + 0.5));
      } else {
	sprintf(tmpStr, "FL %.3d - FL%.3d",
		(int) (sa->getBottomFlightLevel() + 0.5),
		(int) (sa->getTopFlightLevel() + 0.5));
      }
      cout << "  Flight levels: " << tmpStr << endl;
    }
    
    if ((_params.print_fir) && (sa->firSet())) {
      string firName=sa->getFir();
      if (firName.find("UNKNOWN", 0) == string::npos) {
	cout << "  FIR: " << sa->getFir() << endl;
      }
    }

    if ((_params.print_if_polygon_is_fir) && sa->polygonIsFirBdry()) {
      cout << "  Polygon is FIR boundary. See text for actual bdry." << endl;
    }
    
    if ((_params.print_movement) &&
	(sa->getMovementSpeed() != 0)) {
      char tmpStr[128];
      if (sa->getMovementSpeed() > 0) {
        double dirn = sa->getMovementDirn();
        if (dirn < 0) {
          dirn += 360.0;
        }
        int idirn = ((int) dirn / 5) * 5;
        double kts = sa->getMovementSpeed() / KM_PER_NM;
        int ikts = (int) (kts + 0.5);
	sprintf(tmpStr, "%.3d deg at %.3d kts", idirn, ikts);
        cout << "  Movement:      " << tmpStr << endl;
      }
    }

    if (_params.print_source_id) {
      cout << "  Text:" << endl;
    }

    if (!_params.wrap_long_lines) {
      cout << "  " << sa->getText() << endl;
    } else {
      vector<string> toks;
      _tokenize(sa->getText(), " ", toks);
      int len = 0;
      for (size_t kk = 0; kk < toks.size(); kk++) {
	if (len == 0) {
	  cout << "    " << toks[kk];
	  len += toks[kk].size();
	} else if (len + (int) toks[kk].size() <=
		   _params.max_line_length - 4) {
	  cout << " " << toks[kk];
	  len += 1 + toks[kk].size();
	} else {
	  cout << endl;
	  len = 0;
	  kk--;
	}
      }
    }
    cout << endl;

    // print delimiter
    
    if (_params.delimiter_line_after_each_item) {
      for (int i = 0; i < _params.max_line_length; i++) {
	cout << "-";
      }
      cout << endl;
    }
    
    if (_params.newline_between_items) {
      cout << endl;
    }

    cout.flush();

  } // ii

}

//////////////////////////////////////////////
// tokenize a string into a vector of strings

void PrintSigAirMet::_tokenize(const string &str,
			       const string &spacer,
			       vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  bool done = false;
  while (!done) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      done = true;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}

///////////////////////////////////////
// clear reports

void PrintSigAirMet::_clearReports()

{
  for (size_t ii = 0; ii < _reports.size(); ii++) {
    delete _reports[ii];
  }
  _reports.clear();
}


//////////////////////////////////////////////////////////////////////
// Wildcard checker.

int PrintSigAirMet::_WildCard(char *WildSpec, char *String, int debug){
  //
  // WildCard returns zero if a wild card specification is not
  // matched, 1 if there is a match.
  //
  // You pass in :
  //
  // * WildSpec - the wild carded specification, ie.
  //              "*.dat" or "20*.mdv"
  //
  // * String - the string to test against the wildcard,
  //            ie. "Station.dat" or "200402.mdv"
  //
  //
  // * debug - if non-zero, then messages are printed to stderr
  //           explaining the string parsing as it happens.
  //


  int istart = 0;
  int iend = 0;

  char *searchString = String;

  do {
    //
    // Position istart at first non-wildcard.
    //

    char buffer[1024];
    do{
      //
      // If it is a wildcard, skip it.
      //
      if (WildSpec[istart] == '*'){
	istart++;
      }

      //
      // If we have reached the end of the string, return OK.
      //
      if (WildSpec[istart] == char(0)){
	return 1;
      }

      //
      // If it is not a wildcard, copy it into the buffer.
      //
      if (
	  (WildSpec[istart] != '*') &&
	  (WildSpec[istart] != char(0))
	  ){
	//
	// Find the end of this item.
	//
	iend = istart;
	do {
	  iend++;

	} while (
		 (WildSpec[iend] != '*') &&
		 (WildSpec[iend] != char(0))
		 );

	for (int i=istart; i < iend; i++){
	  buffer[i-istart] = WildSpec[i];
	  buffer[i-istart+1] = char(0); 
	}

	if (debug) fprintf(stderr,"ITEM : %s : ",buffer);

	int MustMatchExact = 0;
	if (WildSpec[iend] == char(0)){
	   MustMatchExact = 1;
	}


	if (MustMatchExact){
	  if (debug) fprintf(stderr, "MUST MATCH EXACT : ");

	  if (strlen(searchString) <  strlen(buffer)){
	    //
	    // What we are looking for cannot be here - string too short.
	    //
	    if (debug) fprintf(stderr, "IT CANNOT.\n");
	    return 0;
	  }

	  char *p = searchString + strlen(searchString) - strlen(buffer);
	  if (debug) fprintf(stderr,"%s cf. %s : ",
		  buffer, p);
	  if (!(strcmp(buffer,p))){
	    //
	    // It is an exact match - at the end of the wild card.
	    // We have a match.
	    //
	    if (debug) fprintf(stderr,"IT DOES.\n");
	    return 1;
	    //
	  } else {
	    //
	    // No match - a failure.
	    //
	    if (debug) fprintf(stderr,"IT DOES NOT.\n");
	    return 0;
	  }
	}

	if (istart == 0){
	  if (debug) fprintf(stderr, "MUST BE THE START OF %s: ", searchString);
	  if (!(strncmp(buffer, searchString,strlen(buffer)))){
	    if (debug) fprintf(stderr,"IT IS.\n");
	    searchString = searchString + strlen(buffer);
	  } else {
	    if (debug) fprintf(stderr,"IT IS NOT.\n");
	    return 0;
	  }
	}



	if (istart != 0){
	  if (debug) fprintf(stderr, "MUST BE PRESENT IN %s: ", searchString);
	  char *p = strstr(searchString, buffer);
	  if (p == NULL){
	    if (debug) fprintf(stderr,"IT IS NOT.\n");
	    return 0;
	  } else {
	    if (debug) fprintf(stderr,"IT IS.\n");
	    searchString = p + strlen(buffer);
	  }
	}


	//
	// If that was the last item, return success.
	//
	if (WildSpec[iend] == char(0)){
	  return 1;
	}
	
	istart = iend;
      }

    } while (1);

  } while (1);

}


