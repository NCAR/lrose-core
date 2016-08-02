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
////////////////////////////////////////////////////////////////
// TitanServer.cc
//
// Track entry object for TitanServer
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2001
//
////////////////////////////////////////////////////////////////

// #define DEBUG_PRINT

#include <titan/TitanServer.hh>
#include <titan/TitanStormFile.hh>
#include <titan/TitanTrackFile.hh>
#include <dataport/bigend.h>
#include <toolsa/TaStr.hh>
#include <toolsa/ReadDir.hh>
#include <toolsa/DateTime.hh>
#include <didss/LdataInfo.hh>
using namespace std;
 
////////////////////////////////////////////////////////////
// Constructor

TitanServer::TitanServer()

{

  clearRead();

}

////////////////////////////////////////////////////////////
// destructor

TitanServer::~TitanServer()

{

  clearArrays();

}

////////////////////////////////////////////////////////////
// clear arrays

void TitanServer::clearArrays()

{

  for (size_t ii = 0; ii < _currentEntries.size(); ii++) {
    delete _currentEntries[ii];
  }
  _currentEntries.clear();

  for (size_t ii = 0; ii < _complexTracks.size(); ii++) {
    delete _complexTracks[ii];
  }
  _complexTracks.clear();

}

////////////////////////////////////////////////////////////
// Clear the read flags, set defaults

void TitanServer::clearRead()

{

  _readTimeMode = TITAN_SERVER_READ_LATEST;
  _trackSet = TITAN_SERVER_ALL_AT_TIME;
  _readTimeMargin = 3600;

  _readLprops = false;
  _readDbzHist = false;
  _readRuns = false;
  _readProjRuns = false;

}

///////////////////////
// set up read request


void TitanServer::setReadLatestTime()

{
  _readTimeMode = TITAN_SERVER_READ_LATEST_TIME;
}

void TitanServer::setReadLatest()

{
  _readTimeMode = TITAN_SERVER_READ_LATEST;
}

void TitanServer::setReadClosest(time_t request_time,
				 int margin)

{
  _readTimeMode = TITAN_SERVER_READ_CLOSEST;
  _requestTime = request_time;
  _readTimeMargin = margin;
}

void TitanServer::setReadInterval(time_t start_time,
				  time_t end_time)

{
  _readTimeMode = TITAN_SERVER_READ_INTERVAL;
  _requestTime = end_time;
  _startTime = start_time;
  _endTime = end_time;
}

void TitanServer::setReadFirstBefore(time_t request_time,
				     int margin)

{
  _readTimeMode = TITAN_SERVER_READ_FIRST_BEFORE;
  _requestTime = request_time;
  _readTimeMargin = margin;
}

void TitanServer::setReadNext(time_t request_time,
			      int margin)
  
{
  _readTimeMode = TITAN_SERVER_READ_NEXT_SCAN;
  _requestTime = request_time;
  _readTimeMargin = margin;
}

void TitanServer::setReadPrev(time_t request_time,
			      int margin)
  
{
  _readTimeMode = TITAN_SERVER_READ_PREV_SCAN;
  _requestTime = request_time;
  _readTimeMargin = margin;
}

void TitanServer::setReadAllAtTime()

{
  _trackSet = TITAN_SERVER_ALL_AT_TIME;
}

void TitanServer::setReadAllInFile()

{
  _trackSet = TITAN_SERVER_ALL_IN_FILE;
}

void TitanServer::setReadSingleTrack(int complex_num)

{
  _trackSet = TITAN_SERVER_SINGLE_TRACK;
  _requestComplexNum = complex_num;
}

void TitanServer::setReadCurrentEntries()

{
  _trackSet = TITAN_SERVER_CURRENT_ENTRIES;
}

////////////////////////////////////////////////////////////
// Print read request

void TitanServer::printReadRequest(ostream &out)

{

  out << "TitanServer read request" << endl;
  out << "------------------------" << endl;

  switch (_readTimeMode) {
  case TITAN_SERVER_READ_LATEST_TIME:
    out << "  Read time mode: TITAN_SERVER_READ_LATEST_TIME" << endl;
    break;
  case TITAN_SERVER_READ_LATEST:
    out << "  Read time mode: TITAN_SERVER_READ_LATEST" << endl;
    break;
  case TITAN_SERVER_READ_CLOSEST:
    out << "  Read time mode: TITAN_SERVER_READ_CLOSEST" << endl;
    out << "  Request time: " << utimstr(_requestTime) << endl;
    out << "  Read time margin: " << _readTimeMargin << endl;
    break;
  case TITAN_SERVER_READ_INTERVAL:
    out << "  Read time mode: TITAN_SERVER_READ_INTERVAL" << endl;
    out << "  Start time: " << utimstr(_startTime) << endl;
    out << "  End   time: " << utimstr(_endTime) << endl;
    break;
  case TITAN_SERVER_READ_FIRST_BEFORE:
    out << "  Read time mode: TITAN_SERVER_READ_FIRST_BEFORE" << endl;
    out << "  Request time: " << utimstr(_requestTime) << endl;
    out << "  Read time margin: " << _readTimeMargin << endl;
    break;
  case TITAN_SERVER_READ_NEXT_SCAN:
    out << "  Read time mode: TITAN_SERVER_READ_NEXT_SCAN" << endl;
    out << "  Request time: " << utimstr(_requestTime) << endl;
    out << "  Read time margin: " << _readTimeMargin << endl;
    break;
  case TITAN_SERVER_READ_PREV_SCAN:
    out << "  Read time mode: TITAN_SERVER_READ_PREV_SCAN" << endl;
    out << "  Request time: " << utimstr(_requestTime) << endl;
    out << "  Read time margin: " << _readTimeMargin << endl;
    break;
  } // switch (_readTimeMode)

  switch (_trackSet) {
  case TITAN_SERVER_ALL_AT_TIME:
    out << "  Track set: TITAN_SERVER_ALL_AT_TIME" << endl;
    break;
  case TITAN_SERVER_ALL_IN_FILE:
    out << "  Track set: TITAN_SERVER_ALL_IN_FILE" << endl;
    break;
  case TITAN_SERVER_SINGLE_TRACK:
    out << "  Track set: TITAN_SERVER_SINGLE_TRACK" << endl;
    out << "    Requested complex track num: " << _requestComplexNum << endl;
    break;
  case TITAN_SERVER_CURRENT_ENTRIES:
    out << "  Track set: TITAN_SERVER_CURRENT_ENTRIES" << endl;
    break;
  } // switch (_trackSet)
  
  if (_readLprops) {
    out << "  Read layer props: true" << endl;
  } else {
    out << "  Read layer props: false" << endl;
  }

  if (_readDbzHist) {
    out << "  Read dbz hist: true" << endl;
  } else {
    out << "  Read dbz hist: false" << endl;
  }

  if (_readRuns) {
    out << "  Read storm runs: true" << endl;
  } else {
    out << "  Read storm runs: false" << endl;
  }

  if (_readProjRuns) {
    out << "  Read projected-area runs: true" << endl;
  } else {
    out << "  Read projected-area runs: false" << endl;
  }
  
}

////////////////////////////////////////////////////////////
// Print

void TitanServer::print(FILE *out)

{

  if (_complexTracks.size() > 0) {

    fprintf(out, "TitanServer - complex tracks\n");
    fprintf(out, "============================\n\n");
    
    fprintf(out, "N complex tracks: %d\n", (int) _complexTracks.size());
    
    for (size_t ii = 0; ii < _complexTracks.size(); ii++) {
      _complexTracks[ii]->print(out, _stormFileParams, _trackFileParams);
      
    }

  }
  
  if (_currentEntries.size() > 0) {

    fprintf(out, "TitanServer - current entries\n");
    fprintf(out, "=============================\n\n");
    
    fprintf(out, "N current entries: %d\n", (int) _currentEntries.size());
    
    for (size_t ii = 0; ii < _currentEntries.size(); ii++) {
      _currentEntries[ii]->print(out, ii, _stormFileParams, _trackFileParams);
    }

  }
  
}

////////////////////////////////////////////////////////////
// PrintXML

void TitanServer::printXML(FILE *out)

{

  fprintf(out, "<n_complex_tracks> %d </n_complex_tracks>\n",
          (int) _complexTracks.size());
  if (_complexTracks.size() > 0) {

    for (size_t ii = 0; ii < _complexTracks.size(); ii++) {
      _complexTracks[ii]->printXML(out, _stormFileParams, _trackFileParams);
      
    }
  }
  
  if (_currentEntries.size() > 0) {

    fprintf(out, "<num_track_entries> %d </num_track_entries>\n",
            (int) _currentEntries.size());
    for (size_t ii = 0; ii < _currentEntries.size(); ii++) {
      _currentEntries[ii]->printXML(out, ii, _stormFileParams, _trackFileParams);
    }
  }
  
}

/////////////////////////////////////////////////////
// perform the read
//
// Returns 0 on success, -1 on failure.
//
// On failure, use getErrStr() to get error string.

int TitanServer::read(const string &dir)

{

  _clearErrStr();
  clearArrays();
  _errStr = "ERROR - TitanServer::read\n";
  TaStr::AddStr(_errStr, "Dir: ", dir);

  _dirInUse = dir;

  // get latest time is special case

  if (_readTimeMode == TITAN_SERVER_READ_LATEST_TIME) {
    return _readLatestTime();
  }

  // find the file and scan number for the current request
  
  if (_readTimeMode == TITAN_SERVER_READ_LATEST) {
    if (_findLatestScan()) {
      return -1;
    }
  } else {
    if (_findScan()) {
      return -1;
    }
  }

  // initialize data time limits

  _dataStartTime = _timeInUse;
  _dataEndTime = _timeInUse;

  // compute file paths
  
  DateTime dtime(_idayInUse * SECS_IN_DAY);
  char stormPath[MAX_PATH_LEN];
  char trackPath[MAX_PATH_LEN];
  sprintf(stormPath, "%s%s%.4d%.2d%.2d.%s",
	  _dirInUse.c_str(), PATH_DELIM,
	  dtime.getYear(), dtime.getMonth(), dtime.getDay(),
	  STORM_HEADER_FILE_EXT);
  sprintf(trackPath, "%s%s%.4d%.2d%.2d.%s",
	  _dirInUse.c_str(), PATH_DELIM,
	  dtime.getYear(), dtime.getMonth(), dtime.getDay(),
	  TRACK_HEADER_FILE_EXT);
  _stormPathInUse = stormPath;
  _trackPathInUse = trackPath;
  
  // open the files and lock
  
  TitanStormFile sfile;
  if (sfile.OpenFiles("r", stormPath)) {
    TaStr::AddStr(_errStr, "Cannot open storm file: ", stormPath);
    _errStr += sfile.getErrStr();
    return -1;
  }
  if (sfile.LockHeaderFile("r")) {
    _errStr += sfile.getErrStr();
    return -1;
  }
  _dataStartTime = sfile.header().start_time;
  _dataEndTime = sfile.header().end_time;

  
  TitanTrackFile tfile;
  if (tfile.OpenFiles("r", trackPath)) {
    TaStr::AddStr(_errStr, "Cannot open track file: ", trackPath);
    _errStr += tfile.getErrStr();
    return -1;
  }
  if (tfile.LockHeaderFile("r")) {
    _errStr += tfile.getErrStr();
    return -1;
  }

  // read the headers

  if (sfile.ReadHeader()) {
    _errStr += sfile.getErrStr();
    return -1;
  }
  _stormFileParams = sfile.params();
  if (tfile.ReadHeader()) {
    _errStr += tfile.getErrStr();
    return -1;
  }
  _trackFileParams = tfile.params();

  if (_trackSet == TITAN_SERVER_CURRENT_ENTRIES) {
    return _readCurrentEntries(sfile, tfile);
  } else {
    return _readTracks(sfile, tfile);
  }

}

/////////////////////////////////////////////////////
// read latest time
//
// Returns 0 on success, -1 on failure.
//
// On failure, use getErrStr() to get error string.

int TitanServer::_readLatestTime()

{

  LdataInfo ldata(_dirInUse);
  if (ldata.readForced()) {
    TaStr::AddStr(_errStr, "  Cannot read latest data file, dir: ",
		  _dirInUse);
    return -1;
  }
  _timeInUse = ldata.getLatestValidTime();
  return 0;

}

/////////////////////////////////////////////////////
// read whole tracks
//
// Returns 0 on success, -1 on failure.

int TitanServer::_readTracks(TitanStormFile &sfile,
			     TitanTrackFile &tfile)

{

  // compile the track set for the request

  if (_compileTrackSet(tfile)) {
    return -1;
  }

  // read through complex tracks in set

  for (size_t icomplex = 0; icomplex < _trackSetNums.size(); icomplex++) {

    int complexNum = _trackSetNums[icomplex];

    // read in complex params

    if (tfile.ReadComplexParams(complexNum, true)) {
      _errStr += tfile.getErrStr();
      return -1;
    }

    // create new complex track, set complex params

    TitanComplexTrack *complexTrack = new TitanComplexTrack();
    _complexTracks.push_back(complexTrack);
    complexTrack->_complex_params = tfile.complex_params();

    // read in simple tracks for this complex track

    for (int isimple = 0;
	 isimple < complexTrack->_complex_params.n_simple_tracks; isimple++) {
      
      int simpleNum = tfile.simples_per_complex()[complexNum][isimple];

      // read simple params, go to start of track
      
      if (tfile.RewindSimple(simpleNum)) {
	_errStr += tfile.getErrStr();
	return -1;
      }
      
      // create new simple track, set simple params
      
      TitanSimpleTrack *simpleTrack = new TitanSimpleTrack();
      complexTrack->_simple_tracks.push_back(simpleTrack);
      simpleTrack->_simple_params = tfile.simple_params();

      // read in track entries

      for (int ientry = 0;
	   ientry < simpleTrack->_simple_params.duration_in_scans;
	   ientry++) {

	// read in next entry

	if (tfile.ReadEntry()) {
	  _errStr += tfile.getErrStr();
	  return -1;
	}

	time_t entryTime = tfile.entry().time;
	_dataStartTime = MIN(_dataStartTime, entryTime);
	_dataEndTime = MAX(_dataEndTime, entryTime);

	// create a new track entry, set header

	TitanTrackEntry *entry = new TitanTrackEntry;
	simpleTrack->_entries.push_back(entry);
	entry->_entry = tfile.entry();

	// read in storm file scan header and global props
	
	if (sfile.ReadScan(entry->_entry.scan_num, entry->_entry.storm_num)) {
	  _errStr += sfile.getErrStr();
	  return -1;
	}
	entry->_scan = sfile.scan();
	entry->_gprops = sfile.gprops()[entry->_entry.storm_num];

	// read in other props
	
	if (_readLprops || _readDbzHist || _readRuns || _readProjRuns) {
	  if (sfile.ReadProps(entry->_entry.storm_num)) {
	    _errStr += sfile.getErrStr();
	    return -1;
	  }
	}

	if (_readLprops) {
	  for (int i = 0; i < entry->_gprops.n_layers; i++) {
	    entry->_lprops.push_back(sfile.lprops()[i]);
	  }
	} else {
	  entry->_gprops.n_layers = 0;
	}

	if (_readDbzHist) {
	  for (int i = 0; i < entry->_gprops.n_dbz_intervals; i++) {
	    entry->_hist.push_back(sfile.hist()[i]);
	  }
	} else {
	  entry->_gprops.n_dbz_intervals = 0;
	}
      
	if (_readRuns) {
	  for (int i = 0; i < entry->_gprops.n_runs; i++) {
	    entry->_runs.push_back(sfile.runs()[i]);
	  }
	} else {
	  entry->_gprops.n_runs = 0;
	}
      
	if (_readProjRuns) {
	  for (int i = 0; i < entry->_gprops.n_proj_runs; i++) {
	    entry->_proj_runs.push_back(sfile.proj_runs()[i]);
	  }
	} else {
	  entry->_gprops.n_proj_runs = 0;
	}
      
      } // ientry

    } // isimple
    
  } // icomplex
  
  return 0;

}

/////////////////////////////////////////////////////
// read current entries
//
// Returns 0 on success, -1 on failure.

int TitanServer::_readCurrentEntries(TitanStormFile &sfile,
				     TitanTrackFile &tfile)

{

  // read in entries for current scan

  if (tfile.ReadScanEntries(_scanInUse)) {
    _errStr += sfile.getErrStr();
    return -1;
  }

  // read in storm file scan header and global props for current scan
  
  if (sfile.ReadScan(_scanInUse)) {
    _errStr += sfile.getErrStr();
    return -1;
  }

  // loop through the entries

  int nEntries = tfile.scan_index()[_scanInUse].n_entries;
  for (int ientry = 0; ientry < nEntries; ientry++) {
    
    // create a new TitanTrackEntry object, add to the vector

    TitanTrackEntry *entry = new TitanTrackEntry;
    _currentEntries.push_back(entry);

    // set members
    
    entry->_entry = tfile.scan_entries()[ientry];
    entry->_scan = sfile.scan();
    entry->_gprops = sfile.gprops()[entry->_entry.storm_num];

    // read in other props
    
    if (_readLprops || _readDbzHist || _readRuns || _readProjRuns) {
      if (sfile.ReadProps(entry->_entry.storm_num)) {
	_errStr += sfile.getErrStr();
	return -1;
      }
    }

    if (_readLprops) {
      for (int i = 0; i < entry->_gprops.n_layers; i++) {
	entry->_lprops.push_back(sfile.lprops()[i]);
      }
    } else {
      entry->_gprops.n_layers = 0;
    }
    
    if (_readDbzHist) {
      for (int i = 0; i < entry->_gprops.n_dbz_intervals; i++) {
	entry->_hist.push_back(sfile.hist()[i]);
      }
    } else {
      entry->_gprops.n_dbz_intervals = 0;
    }
    
    if (_readRuns) {
      for (int i = 0; i < entry->_gprops.n_runs; i++) {
	entry->_runs.push_back(sfile.runs()[i]);
      }
    } else {
      entry->_gprops.n_runs = 0;
    }
    
    if (_readProjRuns) {
      for (int i = 0; i < entry->_gprops.n_proj_runs; i++) {
	entry->_proj_runs.push_back(sfile.proj_runs()[i]);
      }
    } else {
      entry->_gprops.n_proj_runs = 0;
    }

  } // ientry

  return 0;

}


/////////////////////////////////////////////////////
// find the file path and scan number for the
// time requested
//
// Returns 0 on success, -1 on failure.

int TitanServer::_findScan()

{

  vector<_tserver_scan_t> scanList;
  int rday = _requestTime / SECS_IN_DAY;

  // load up vector of scan information for two previous days
  
  for (int iday = rday - 2; iday < rday; iday++) {
    _loadScanList(iday, scanList);
  }

  // if no scans yet, go back for up to a year until we get some

  if (scanList.size() == 0) {
    for (int iday = rday - 3; iday > rday - 365; iday--) {
      _loadScanList(iday, scanList);
      if (scanList.size() > 0) {
	break;
      }
    }
  }

  // load up scan information for current day and next day

  for (int iday = rday; iday <= rday + 1; iday++) {
    _loadScanList(iday, scanList);
  }

  // look through the list, finding the first scan before the
  // request time
  
  int scanNum = -1;
  if (scanList.size() > 0) {
    for (int ii = (int) scanList.size() - 1; ii >= 0; ii--) {
      if (scanList[ii].time <= _requestTime) {
	scanNum = ii;
	break;
      }
    } // ii
  }
    
#ifdef DEBUG_PRINT
  for (size_t ii = 0; ii < scanList.size(); ii++) {
    cerr << "iday, scan_num, time: "
  	 << scanList[ii].iday << ", "
  	 << scanList[ii].num << ", "
  	 << DateTime::str(scanList[ii].time) << endl;
  }
  
  cerr << "scanNum found: " << scanNum << endl;
#endif

  if (scanNum >= 0) {

    switch (_readTimeMode) {
      
    case TITAN_SERVER_READ_CLOSEST:
      {
	if (scanNum == (int) scanList.size() - 1) {
	  //
	  // Added code to cope with this case.
	  // Niles Oien August 2003.
	  //
	  // Calculate the backwards time difference only,
	  // it's all we need here. If it meets the read margin,
	  // then we're OK.
	  //
	  int diffBack = _requestTime - scanList[scanNum].time;
	  if (diffBack <= _readTimeMargin) {
	    _scanInUse = scanList[scanNum].num;
	    _timeInUse = scanList[scanNum].time;
	    _idayInUse = scanList[scanNum].iday;
	    return 0;
	  }
	}

	if (scanNum < (int) scanList.size() - 1) {
	  int diffBack = _requestTime - scanList[scanNum].time;
	  int diffFwd = scanList[scanNum + 1].time - _requestTime;
	  if (diffBack <= diffFwd) {
	    if (diffBack <= _readTimeMargin) {
	      _scanInUse = scanList[scanNum].num;
	      _timeInUse = scanList[scanNum].time;
	      _idayInUse = scanList[scanNum].iday;
	      return 0;
	    }
	  } else {
	    if (diffFwd <= _readTimeMargin) {
	      _scanInUse = scanList[scanNum + 1].num;
	      _timeInUse = scanList[scanNum + 1].time;
	      _idayInUse = scanList[scanNum + 1].iday;
	      return 0;
	    }
	  } // if (diffBack <= diffFwd)
	} // if (scanNum < scanList.size() - 1) 
      }
    break;

    case TITAN_SERVER_READ_FIRST_BEFORE:
      {
	int diffBack = _requestTime - scanList[scanNum].time;
	if (diffBack <= _readTimeMargin) {
	  _scanInUse = scanList[scanNum].num;
	  _timeInUse = scanList[scanNum].time;
	  _idayInUse = scanList[scanNum].iday;
	  return 0;
	}
      }
    break;

    case TITAN_SERVER_READ_NEXT_SCAN:
      {
	int diffBack = _requestTime - scanList[scanNum].time;
	if (diffBack <= _readTimeMargin) {
	  if (scanNum <  (int) scanList.size() - 1) {
	    _scanInUse = scanList[scanNum + 1].num;
	    _timeInUse = scanList[scanNum + 1].time;
	    _idayInUse = scanList[scanNum + 1].iday;
	  } else {
	    _scanInUse = scanList[scanNum].num;
	    _timeInUse = scanList[scanNum].time;
	    _idayInUse = scanList[scanNum].iday;
	  }
	  return 0;
	}
      }
      break;

    case TITAN_SERVER_READ_PREV_SCAN:
      {
	int diffBack = _requestTime - scanList[scanNum].time;
	if (diffBack <= _readTimeMargin) {
	  if (scanNum > 0) {
	    _scanInUse = scanList[scanNum - 1].num;
	    _timeInUse = scanList[scanNum - 1].time;
	    _idayInUse = scanList[scanNum - 1].iday;
	  } else {
	    _scanInUse = scanList[scanNum].num;
	    _timeInUse = scanList[scanNum].time;
	    _idayInUse = scanList[scanNum].iday;
	  }
	  return 0;
	}
      }
      break;

    case TITAN_SERVER_READ_INTERVAL:
      {
	_scanInUse = scanList[scanNum].num;
	_timeInUse = scanList[scanNum].time;
	_idayInUse = scanList[scanNum].iday;
	return 0;
      }
      break;

    default: {}

    } // switch

  } // if (scanNum >= 0)
  
  // failed to find suitable scan
  
  TaStr::AddStr(_errStr, "Cannot find suitable data scan for time: ",
		DateTime::str(_requestTime));
  
  switch (_readTimeMode) {
  case TITAN_SERVER_READ_LATEST:
    TaStr::AddStr(_errStr, "Time search mode: ",
		  "TITAN_SERVER_READ_LATEST");
    break;
  case TITAN_SERVER_READ_CLOSEST:
    TaStr::AddStr(_errStr, "Time search mode: ",
		  "TITAN_SERVER_READ_CLOSEST");
    break;
  case TITAN_SERVER_READ_FIRST_BEFORE:
    TaStr::AddStr(_errStr, "Time search mode: ",
		  "TITAN_SERVER_READ_FIRST_BEFORE");
    break;
  case TITAN_SERVER_READ_NEXT_SCAN:
    TaStr::AddStr(_errStr, "Time search mode: ",
		  "TITAN_SERVER_READ_NEXT_SCAN");
    break;
  case TITAN_SERVER_READ_PREV_SCAN:
    TaStr::AddStr(_errStr, "Time search mode: ",
		  "TITAN_SERVER_READ_PREV_SCAN");
    break;
  default: {}
  } // switch
  
  TaStr::AddInt(_errStr, "Search margin: ", _readTimeMargin);
  
  return -1;
  
}

/////////////////////////////////////////////////////////
// look for requested time in the file for the given day
//
// Sets scan_found true if suitable time found.
//
// Returns 0 on success, -1 on error

void TitanServer::_loadScanList(int iday,
				vector<_tserver_scan_t> &scanList)
  
{

  // compute track file path
  
  DateTime dtime(iday * SECS_IN_DAY);
  char trackPath[MAX_PATH_LEN];
  sprintf(trackPath, "%s%s%.4d%.2d%.2d.%s",
	  _dirInUse.c_str(), PATH_DELIM,
	  dtime.getYear(), dtime.getMonth(), dtime.getDay(),
	  TRACK_HEADER_FILE_EXT);
  
  // open the track file and lock
  
  TitanTrackFile tfile;
  if (tfile.OpenFiles("r", trackPath)) {
    // no file available
    return;
  }
  if (tfile.LockHeaderFile("r")) {
    TaStr::AddStr(_errStr, "ERROR - ", "TitanServer::_loadScanList");
    _errStr += tfile.getErrStr();
    tfile.CloseFiles();
    return;
  }
  
  // read in the scan index
  
  if (tfile.ReadScanIndex()) {
    TaStr::AddStr(_errStr, "ERROR - ", "TitanServer::_loadScanList");
    _errStr += tfile.getErrStr();
    tfile.CloseFiles();
    return;
  }

  // add to scan list

  time_t latestTime = 0;
  if (scanList.size() > 0) {
    latestTime = scanList[scanList.size() - 1].time;
  }

  for (int iscan = 0; iscan < tfile.header().n_scans; iscan++) {
    const track_file_scan_index_t &sindex = tfile.scan_index()[iscan];
    if (sindex.utime > latestTime) {
      _tserver_scan_t scan;
      scan.iday = iday;
      scan.num = iscan;
      scan.time = sindex.utime;
      scanList.push_back(scan);
      latestTime = sindex.utime;
    }
  } // iscan

  // close track file

  tfile.CloseFiles();

}

/////////////////////////////////////////////////////
// find the file path and scan number for the
// latest scan
//
// Returns 0 on success, -1 on failure.

int TitanServer::_findLatestScan()

{

  // try to read latest data info. If that fails get the
  // last time in the files

  time_t latestTime;
  LdataInfo ldata;
  if (ldata.read() == 0) {
    latestTime = ldata.getLatestTime();
  } else {
    if (_findLastDay(latestTime)) {
      _errStr += "TitanServer::_findLatestScan()\n";
      TaStr::AddStr(_errStr, "Cannot find last day in dir: ", _dirInUse);
      return -1;
    }
  }

  // compute file path

  DateTime dtime(latestTime);
  char stormPath[MAX_PATH_LEN];
  sprintf(stormPath, "%s%s%.4d%.2d%.2d.%s",
	  _dirInUse.c_str(), PATH_DELIM,
	  dtime.getYear(), dtime.getMonth(), dtime.getDay(),
	  STORM_HEADER_FILE_EXT);

  // open the storm file and lock
  
  TitanStormFile sfile;
  if (sfile.OpenFiles("r", stormPath)) {
    _errStr += "TitanServer::_findLatestScan()\n";
    _errStr += sfile.getErrStr();
    return -1;
  }
  if (sfile.LockHeaderFile("r")) {
    _errStr += "TitanServer::_findLatestScan()\n";
    _errStr += sfile.getErrStr();
    sfile.CloseFiles();
    return -1;
  }
  
  // read in the header
  
  if (sfile.ReadHeader()) {
    _errStr += "TitanServer::_findLatestScan()\n";
    _errStr += sfile.getErrStr();
    sfile.CloseFiles();
    return -1;
  }
  
  // read the last scan
  
  if (sfile.ReadScan(sfile.header().n_scans - 1)) {
    _errStr += "TitanServer::_findLatestScan()\n";
    _errStr += sfile.getErrStr();
    sfile.CloseFiles();
    return -1;
  }
  const storm_file_scan_header_t &scanHdr = sfile.scan();
  _scanInUse = scanHdr.scan_num;
  _timeInUse = scanHdr.time;
  _idayInUse = latestTime / SECS_IN_DAY;

  sfile.CloseFiles();

  return 0;

}

/////////////////////////////////////////////////////
// find the last day in the files
//
// Returns 0 on success, -1 on failure.

int TitanServer::_findLastDay(time_t &last_day)

{

  ReadDir rdir;
  if (rdir.open(_dirInUse.c_str())) {
    _errStr += "TitanServer::_findLastTime()\n";
    TaStr::AddStr(_errStr, "Cannot open dir: ", _dirInUse);
    return -1;
  }

  // Loop thru directory looking for subdir names which represent dates
  // Set last day

  time_t lastDay = -1;
  struct dirent *dp;
  for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
    // exclude dir entries and files beginning with '.'
    if (dp->d_name[0] == '.') {
      continue;
    }
    // check that subdir name is in the correct format
    int year, month, day;
    char ext[64];
    if (sscanf(dp->d_name, "%4d%2d%2d.%s", &year, &month, &day, ext) != 4) {
      continue;
    }
    if (year < 1900 || year > 9999 || month < 1 || month > 12 ||
	day < 1 || day > 31) {
      continue;
    }
    // check extension
    if (strcmp(ext, STORM_HEADER_FILE_EXT)) {
      continue;
    }
    // accept
    DateTime dtime(year, month, day);
    if (dtime.utime() > lastDay) {
      lastDay = dtime.utime();
    }
  } // for (dp ...
  rdir.close();
  
  if (lastDay == -1) {
    _errStr += "TitanServer::_findLastTime()\n";
    TaStr::AddStr(_errStr, "No data files in dir: ", _dirInUse);
    return -1;
  }

  last_day = lastDay;
  return 0;

}

/////////////////////////////////////////////////////
// compile the track set for the request
//
// Returns 0 on success, -1 on failure.

int TitanServer::_compileTrackSet(TitanTrackFile &tfile)

{

  _trackSetNums.clear();

  if (tfile.ReadUtime()) {
    _errStr += "ERROR - TitanServer::_compileTrackSet\n";
    _errStr += tfile.getErrStr();
    return -1;
  }

  switch (_trackSet) {

  case TITAN_SERVER_ALL_AT_TIME:
    {
      time_t startTimeInUse = _timeInUse;
      if (_readTimeMode == TITAN_SERVER_READ_INTERVAL) {
	int interval = _endTime - _startTime;
	startTimeInUse = _timeInUse - interval;
      }
#ifdef DEBUG_PRINT
      cerr << "startTimeInUse: " << utimstr(startTimeInUse) << endl;
      cerr << "_timeInUse: " << utimstr(_timeInUse) << endl;
#endif
      for (int icomplex = 0; icomplex < tfile.header().n_complex_tracks;
	   icomplex++) {
	int complexNum = tfile.complex_track_nums()[icomplex];
	const track_utime_t &utime = tfile.track_utime()[complexNum];
	if (utime.start_complex <= _timeInUse &&
	    utime.end_complex >= startTimeInUse) {
#ifdef DEBUG_PRINT
	  cerr << "  Including complex num, start, end: "
	       << complexNum << " "
	       << utimstr(utime.start_complex) << " "
	       << utimstr(utime.end_complex) << endl;
#endif
	  _trackSetNums.push_back(complexNum);
	}
      } // icomplex
    }
    break;

  case TITAN_SERVER_ALL_IN_FILE:
    {
      for (int icomplex = 0; icomplex < tfile.header().n_complex_tracks;
	   icomplex++) {
	int complexNum = tfile.complex_track_nums()[icomplex];
	_trackSetNums.push_back(complexNum);
      }
    }
    break;

  case TITAN_SERVER_SINGLE_TRACK:
    {
      _trackSetNums.push_back(_requestComplexNum);
    }
    break;
    
  default: {}
    
  } // switch (_trackSet)

#ifdef DEBUG_PRINT
  cerr << "Track set:";
  for (size_t ii = 0; ii < _trackSetNums.size(); ii++) {
    cerr << " " << _trackSetNums[ii];
  }
  cerr << endl;
#endif
  
  return 0;

}

