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
 // Taf2Spdb.cc
 //
 // Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
 //
 // Oct 2009
 //
///////////////////////////////////////////////////////////////
//
// Taf2Spdb reads TAF data from text files
// and stores them in SPDB format.
//
 ///////////////////////////////////////////////////////////////

#include <dsserver/DmapAccess.hh>
#include <didss/DsURL.hh>

#include <string>
#include <vector>
#include <cerrno>
#include <toolsa/toolsa_macros.h>
#include <toolsa/globals.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <Spdb/DsSpdb.hh>
#include "Taf2Spdb.hh"

// Constructor

Taf2Spdb::Taf2Spdb(int argc, char **argv)
  
{
  
  isOK = true;
  _inputPath = NULL;
  _input = NULL;

  // set programe name

  _progName = "Taf2Spdb";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params

  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
                           &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end times." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }
  }

  // input path object

  if (_params.mode == Params::REALTIME) {

    _inputPath = new DsInputPath((char *) _progName.c_str(),
                                 _params.debug >= Params::DEBUG_VERBOSE,
                                 _params.input_dir_path,
                                 _params.max_realtime_valid_age,
                                 PMU_auto_register);

  } else if (_params.mode == Params::ARCHIVE) {

    _inputPath = new DsInputPath((char *) _progName.c_str(),
                                 _params.debug >= Params::DEBUG_VERBOSE,
                                 _params.input_dir_path,
                                 _args.startTime,
                                 _args.endTime);

  } else  if (_params.mode == Params::FILELIST) {

    _inputPath = new DsInputPath((char *) _progName.c_str(),
                                 _params.debug >= Params::DEBUG_VERBOSE,
                                 _args.inputFileList);

  }

  // set up input data object
  
  switch(_params.taf_format) {
    case Params::AFTN:
    case Params::GTS:
      _input = new Input(_progName, _params);
      break;
    case Params::CAF:
      _input = new CafInput(_progName, _params);
      break;
    case Params::CWB_CAF:
      _input = new CwbCafInput(_progName, _params);
      break;
    default:
      _input = new Input(_progName, _params);
  }
  
  // read in station locations

  if (_stationLoc.ReadData(_params.station_location_path)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot load station locations, file : "
         << _params.station_location_path << endl;
    isOK = false;
    return;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

Taf2Spdb::~Taf2Spdb()

{

  // unregister process

  PMU_auto_unregister();

  // Free contained objects

  if (_inputPath) {
    delete _inputPath;
  }
  if (_input) {
    _input->close();
    delete _input;
  }

}

//////////////////////////////////////////////////
// Run

int Taf2Spdb::Run ()
{

  // register with procmap

  PMU_auto_register("Run");

  if (_params.mode == Params::ARCHIVE) {
    _inputPath->reset();
  }

  char *inputFilePath;
  while ((inputFilePath = _inputPath->next()) != NULL) {

    // compute file time

    _fileTime = _inputPath->getDataTime(inputFilePath);

    // in realtime mode, ref time is now
    // otherwise, ref time is file time

    if (_params.mode == Params::REALTIME) {
      _refTime.set(time(NULL)); // now
    } else {
      _refTime.set(_fileTime);
    }

    _processFile(inputFilePath);

  } // while

  return (0);

}

//////////////////////////////////////////////////
// process file

int Taf2Spdb::_processFile (const char *file_path)

{

  char procmap_string[256];
  Path path(file_path);
  sprintf(procmap_string, "Processing file <%s>", path.getFile().c_str());
  PMU_auto_register(procmap_string);

  // open file

  if (_input->open(file_path)) {
    cerr << "ERROR - Taf2Spdb::_processFile" << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
    cerr << "  File time: " << DateTime::str(_fileTime) << endl;
  }
  
  DsSpdb asciiSpdb;
  DsSpdb xmlSpdb;
  string tafStr;
  int nTotal = 0;
  int nDecoded = 0;

  // If we want to preserve the cancelled messages in the ASCII
  // database, set the put mode to "add" so that the cancel
  // message doesn't overwrite the original message.  I tried
  // to just change the mode when writing the cancel message,
  // but I was getting weird behavior.  I had to add the "Unique"
  // because we were getting the cancel message multiple times.

  if (_params.ascii_cancel_msg_handling == Params::CANCEL_MSG_ADDED_WITH_ORIGINAL_VALID_TIME)
    asciiSpdb.setPutMode(Spdb::putModeAddUnique);

  while (_input->readNext(tafStr) == 0) {

    PMU_auto_register(procmap_string);
    
    if (tafStr.size() == 0) {
      continue;
    }
    nTotal++;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "========== Got TAF ===========" << endl;
      cerr << tafStr << endl;
    }
  
    if (_decodeMessage(tafStr, asciiSpdb, xmlSpdb) == 0) {
      nDecoded++;
      _addPut(tafStr, asciiSpdb, xmlSpdb);
    } else {
      if (_params.debug) {
        cerr << "WARNING - Taf2Spdb" << endl;
        cerr << "Cannot decode TAF:" << endl;
        cerr << tafStr;
        if (tafStr[tafStr.size()-1] != '\n') {
          cerr << endl;
        }
        cerr << "***************************" << endl;
      }
    }
    
  } // while

  // close file
  
  _input->close();

  // debug print

  if ((_params.debug >= Params::DEBUG_VERBOSE) ||
      (_params.print_decode_problems_to_stderr)) {
    cerr << "---------------------------------------" << endl;
    cerr << "Done with file: " << file_path << endl;
    cerr << "  N TAFs found: " << nTotal << endl;
    cerr << "  N TAFs decoded: " << nDecoded << endl;
    cerr << "---------------------------------------" << endl;
  }
  
  // Write out
  
  int iret = 0;

  if (_params.store_ascii_format) {
    if (asciiSpdb.put(_params.ascii_output_url,
                      SPDB_ASCII_ID, SPDB_ASCII_LABEL)) {
      cerr << "ERROR - Taf2Spdb::_processFile" << endl;
      cerr << asciiSpdb.getErrStr() << endl;
      iret = -1;
    } else {
      if (_params.debug) {
        cerr << "Wrote ASCII TAF data to URL: "
             << _params.ascii_output_url << endl;
      }
    }
  }

  if (_params.store_xml_format) {
    if (xmlSpdb.put(_params.xml_output_url,
                    SPDB_TAF_ID, SPDB_TAF_LABEL)) {
      cerr << "ERROR - Taf2Spdb::_processFile" << endl;
      cerr << xmlSpdb.getErrStr() << endl;
      iret = -1;
    } else {
      if (_params.debug) {
        cerr << "Wrote decoded XML TAF data to URL: "
             << _params.xml_output_url << endl;
      }
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// decode message
//
// Returns 0 if decoded, -1 if not

int Taf2Spdb::_decodeMessage(string tafStr,
                             DsSpdb &asciiSpdb,
                             DsSpdb &xmlSpdb)
  
{

  // clear out TAF object

  _taf.clear();
  _taf.setText(tafStr);

  // add "TAF" to the taf string if not already there
  
  if (!strstr(tafStr.c_str(), "TAF")) {
    string tmp = tafStr;
    tafStr = "TAF ";
    tafStr += tmp;
  }
  
  // remove '=' if there
  
  size_t equals = tafStr.find_first_of('=', 0);
  if (equals != string::npos) {
    tafStr.resize(equals);
  }
    
  // tokenize message
  
  _tokenize(tafStr, " \n\t\r", _toks);
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Handling message ----------->>" << endl;
    cerr << "  " << tafStr << endl;
    cerr << "  ntoks: " << _toks.size() << endl;
  }
  
  // if no toks, continue
  
  if (_toks.size() == 0) {
    return -1;
  }

  // set up used array, initialize to false
  
  _used.clear();
  for (size_t ii = 0; ii < _toks.size(); ii++) {
    _used.push_back(false);
  }
  
  // initialize token numbers

  _tafTokNum = -1;
  _nameTokNum = -1;
  _issueTimeTokNum = -1;
  _validTimeTokNum = -1;
  _firstTokMainData = -1;
  _lastTokMainData = (int) _toks.size() - 1;

  // find the TAF token

  if (_findTafToken()) {
    if (_params.debug) {
      cerr << "WARNING - TAF token not found" << endl;
    }
    return -1;
  }

  // set amended or corrected flag

  _setAmendedCorrected();
  
  // find station name - this will be the first 4-letter token

  if (_setStationName()) {
    if (_params.debug) {
      cerr << "WARNING - Station name not found" << endl;
    }
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->>  name: " << _name << endl;
  }

  // check if this taf has an entry in the stations list
  
  double lat, lon, alt;
  string type;
  if (_stationLoc.FindPosition(_name, lat, lon, alt, type)) {
    if (_params.debug) {
      cerr << "WARNING - Cannot find station name in station location file" << endl;
      cerr << "  TAF station name: " << _name << endl;
      cerr << "  location file: " << _params.station_location_path << endl;
    }
    return -1;
  }
  _taf.setLatitude(lat);
  _taf.setLongitude(lon);
  _taf.setElevationM(alt);
  
  // should we check acceptedStations?
  
  if (!_acceptStation()) {
    return -1;
  }

  // set the issue time
  
  if (_setIssueTime()) {
    if (_params.debug) {
      cerr << "WARNING - Issue time not found" << endl;
    }
  }
  if (_issueTimeTokNum >= (int) _toks.size() - 1) {
    if (_params.debug) {
      cerr << "WARNING - TAF too short" << endl;
    }
    return -1;
  }
  
  // is this a nil TAF?

  if (_toks[_issueTimeTokNum+1] == "NIL") {
    _taf.setNil();
    return 0;
  }
  
  // set forecast valid and expire times
  
  if (_setValidExpireTimes()) {
    if (_params.debug) {
      cerr << "WARNING - Forecast time limits not found" << endl;
    }
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===============================" << endl;
    cerr << "Processing TAF: " << endl;
    cerr << tafStr << endl;
    cerr << "  Name: " << _name << endl;
    cerr << "  Issue  time: " << DateTime::str(_taf.getIssueTime()) << endl;
    cerr << "  Valid  time: " << DateTime::str(_taf.getValidTime()) << endl;
    cerr << "  Expire time: " << DateTime::str(_taf.getExpireTime()) << endl;
    cerr << "===============================" << endl;
  }

  // is this a cancellation message
  
  if (_validTimeTokNum < (int) _toks.size() - 1) {
    if (_toks[_validTimeTokNum+1] == "CNL") {
      _taf.setCancelled();
      return 0;
    }
  }

  // add main time period to start of periods vector

  _firstTokMainData = _validTimeTokNum + 1;
  _periodLimits.clear();
  {
    // add main period at start of period list
    PeriodLimits mainLimits;
    _periodLimits.push_back(mainLimits);
  }

  // set up the limits on time periods
  
  _setPeriodLimits();

  // set main period times
  
  PeriodLimits &mainLimits = _periodLimits[0];
  mainLimits.period.startTime = _taf.getValidTime();
  mainLimits.period.endTime = _taf.getExpireTime();
  for (int ii = 0; ii < (int) _periodLimits.size(); ii++) {
    const PeriodLimits &limits = _periodLimits[ii];
    if (limits.period.pType == Taf::PERIOD_FROM ||
        limits.period.pType == Taf::PERIOD_BECMG) {
      // end time superceded by start time of first FROM period
      mainLimits.period.endTime = limits.period.startTime;
      break;
    }
  }
  mainLimits.startTok = 0;
  mainLimits.startData = _firstTokMainData;
  mainLimits.endData = _lastTokMainData;

  // set text for each period

  _setPeriodText();

  // print limits in debug mode

  _printPeriodLimits();

  // set data fields for each period

  for (int ii = 0; ii < (int) _periodLimits.size(); ii++) {

    PeriodLimits &limits = _periodLimits[ii];
    Taf::ForecastPeriod &period = limits.period;
    
    if (_setFields(limits.startData, limits.endData, period)) {
      return -1;
    }

    // copy data to later periods as applicable - only for
    // MAIN, FROM and BECMG types
    // not for TEMPO and PROB types, these are local to the time limits

    if (period.pType != Taf::PERIOD_TEMPO && 
        period.pType != Taf::PERIOD_PROB) {
      for (int jj = ii + 1; jj < (int) _periodLimits.size(); jj++) {
        Taf::ForecastPeriod &later = _periodLimits[jj].period;
        later.windDirnDegT = period.windDirnDegT;
        later.windDirnVrb = period.windDirnVrb;
        later.windSpeedKmh = period.windSpeedKmh;
        later.windGustKmh = period.windGustKmh;
        later.visKm = period.visKm;
        later.isCavok = period.isCavok;
        later.ceilingKm = period.ceilingKm;
        later.layers = period.layers;
      }
    }
        
  } // ii

  // add periods to TAF

  for (int ii = 0; ii < (int) _periodLimits.size(); ii++) {
    PeriodLimits &limits = _periodLimits[ii];
    Taf::ForecastPeriod &period = limits.period;
    _taf.addForecastPeriod(period);
  }

  // Print a break at the start of each TAF
  
  if (_params.print_decode_problems_to_stderr) {
    cerr << "--------------------------------------" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// add put chunks to SPDB objects

void Taf2Spdb::_addPut(string tafStr,
                       DsSpdb &asciiSpdb,
                       DsSpdb &xmlSpdb)
  
{

  // ASCII output?
  
  if (_params.store_ascii_format) {
    
    string asciiChunk;
    if (_params.store_header) {
      asciiChunk += _input->getHeadStr();
    }
    asciiChunk += tafStr;

    if (_taf.isCancelled())
    {
      // If this is a cancel message, handle it as indicated
      // in the parameter file

      switch (_params.ascii_cancel_msg_handling)
      {
      case Params::CANCEL_MSG_OVERWRITES_ORIGINAL_MSG :
      case Params::CANCEL_MSG_ADDED_WITH_ORIGINAL_VALID_TIME :
      {
	// The original TAF and the new cancel message have the same
	// valid time.  If we are using CANCEL_MSG_OVERWRITES_ORIGINAL_MSG
	// then we are using the default put mode, putModeOver, so the
	// cancel message will overwrite the original message.  If we
	// are using CANCEL_MSG_ADDED_WITH_ORIGINAL_VALID_TIME, we set
	// the put mode to putModeAdd earlier so the original message
	// will be retained in the database.

	asciiSpdb.addPutChunk(Spdb::hash4CharsToInt32(_name.c_str()),
			      _taf.getValidTime(),
			      _taf.getExpireTime(),
			      asciiChunk.size() + 1,
			      asciiChunk.c_str());
	if (_params.debug >= Params::DEBUG_EXTRA) {
	  cerr << "==============================" << endl;
	  if (_params.ascii_cancel_msg_handling == Params::CANCEL_MSG_OVERWRITES_ORIGINAL_MSG)
	    cerr << "Adding ASCII TAF CNL chunk, replacing original TAF: " << endl;
	  else
	    cerr << "Adding ASCII TAF CNL chunk, leaving original TAF in database: " << endl;
	  cerr << "  Valid time: " << DateTime::str(_taf.getValidTime()) << endl;
	  cerr << tafStr << endl;
	  cerr << "==============================" << endl;
	}
	break;
      }
      case Params::CANCEL_MSG_ADDED_WITH_ISSUE_TIME_AS_VALID_TIME :
      {
	// We use the issue time as the valid time for the cancel message
	// so both messages will still be in the database.

	asciiSpdb.addPutChunk(Spdb::hash4CharsToInt32(_name.c_str()),
			      _taf.getIssueTime(),
			      _taf.getExpireTime(),
			      asciiChunk.size() + 1,
			      asciiChunk.c_str());
	if (_params.debug >= Params::DEBUG_EXTRA) {
	  cerr << "==============================" << endl;
	  cerr << "Adding ASCII TAF CNL chunk using issue time as valid time: " << endl;
	  cerr << "  Valid time: " << DateTime::str(_taf.getValidTime()) << endl;
	  cerr << tafStr << endl;
	  cerr << "==============================" << endl;
	}
	break;
      }
      } // endswitch - _params.ascii_cancel_msg_handling
    }
    else
    {
      // For any other message, just add the message to the 
      // ASCII database

      asciiSpdb.addPutChunk(Spdb::hash4CharsToInt32(_name.c_str()),
			    _taf.getValidTime(),
			    _taf.getExpireTime(),
			    asciiChunk.size() + 1,
			    asciiChunk.c_str());
      if (_params.debug >= Params::DEBUG_EXTRA) {
	cerr << "==============================" << endl;
	cerr << "Adding ASCII TAF chunk: " << endl;
	cerr << "  Valid time: " << DateTime::str(_taf.getValidTime()) << endl;
	cerr << tafStr << endl;
	cerr << "==============================" << endl;
      }

    }

  } // if (_params.store_ascii_format) {
  
  // XML output?
  
  if (_params.store_xml_format) {

    // if the TAF has been cancelled in this message, overwrite the original TAF
    // adding the cancelled flag
    
    if (_taf.isCancelled()) {
      _overwriteCancelled();
      return;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "===========================" << endl;
      cerr << "Adding decoded TAF chunk: " << endl;
      cerr << "  Valid time: " << DateTime::str(_taf.getValidTime()) << endl;
      _taf.print(cerr);
      cerr << "===========================" << endl;
    }

    
    si32 data_type = Spdb::hash4CharsToInt32(_name.c_str());
    si32 data_type2 = 0;
    
    _taf.assemble();
    xmlSpdb.addPutChunk(data_type,
                        _taf.getValidTime(),
                        _taf.getExpireTime(),
                        _taf.getBufLen(),
                        _taf.getBufPtr(),
                        data_type2);

  }

}

/////////////////////////////////////////////////////////////////
// should we accept the station?
// returns true or false

bool Taf2Spdb::_acceptStation()
  
{

  // should we check acceptedStations?
  
  bool storeIt = true;
  if (_params.useAcceptedStationsList) {
    bool accept = false;
    for (int ii = 0; ii < _params.acceptedStations_n; ii++) {
      if (_name == _params._acceptedStations[ii]) {
        accept = true;
        break;
      }
    }
    if (!accept) {
      if (_params.debug) {
        cerr << endl;
        cerr << "Rejecting station: " << _name << endl;
        cerr << "  Not in acceptedStations list" << endl;
      }
      storeIt = false;
    }
  }
  
  // should we check rejectedStations?
  
  if (_params.useRejectedStationsList) {
    bool reject = false;
    for (int ii = 0; ii < _params.rejectedStations_n; ii++) {
      if (_name == _params._rejectedStations[ii]) {
        reject = true;
        break;
      }
    }
    if (reject) {
      if (_params.debug) {
        cerr << endl;
        cerr << "Rejecting station: " << _name << endl;
        cerr << "  Station name is in rejectedStations list" << endl;
      }
      storeIt = false;
    }
  }

  return storeIt;

}

/////////////////////////////////////////////////////////////////
// find the TAF token, set the last tok used
// returns 0 on success, -1 on failure

int Taf2Spdb::_findTafToken()
  
{

  for (int ii = 0; ii < (int) _toks.size(); ii++) {
    
    if (_toks[ii] == "TAF") {
      _tafTokNum = ii;
      _used[ii] = true;
      return 0;
    }
    
  }

  return -1;

}
  
/////////////////////////////////////////////////////////////////
// set amended or corrected

void Taf2Spdb::_setAmendedCorrected()
  
{

  for (int ii = _tafTokNum + 1; ii < (int) _toks.size(); ii++) {

    if (_toks[ii] == "AMD") {
      _taf.setAmended(true);
      _used[ii] = true;
      // _lastTokUsed = ii;
      return;
    }

    if (_toks[ii] == "COR") {
      _taf.setCorrected(true);
      _used[ii] = true;
      // _lastTokUsed = ii;
      return;
    }

  }

}
  
/////////////////////////////////////////////////////////////////
// set station name
// returns 0 on success, -1 on failure

int Taf2Spdb::_setStationName()
  
{

  // name will be close to start
  // we use the first available 4-character all-alpha token

  _name.clear();
  _nameTokNum = -1;
  for (int ii = 0; ii < (int) _toks.size(); ii++) {

    if (_used[ii]) {
      continue;
    }

    if (_toks[ii].size() == 4) {
      // name must be all alpha, no numerics
      const char *c = _toks[ii].c_str();
      bool allAlpha = true;
      for (int jj = 0; jj < 4; jj++) {
        if (!isalpha(c[jj])) {
          allAlpha = false;
        }
      } // jj
      if (allAlpha) {
        _nameTokNum = ii;
        _name = _toks[ii];
        _used[ii] = true;
        _taf.setStationId(_name);
        return 0;
      }
    } // if (_toks[1].size() == 4)

  } // ii

  return -1;

}

/////////////////////////////////////////////////////////////////
// increment the day, adjusting for change in month
// relative to reference time.
//
// This is done when the expire hour is less than the valid hour.

int Taf2Spdb::_incrementDay(int day)
  
{
  int refYear = _refTime.getYear();
  int refMonth = _refTime.getMonth();

  DateTime gtime(refYear, refMonth, day, 0, 0, 0);

  gtime += 86400; // one day 

  return gtime.getDay();
}
  
/////////////////////////////////////////////////////////////////
// compute time, adjusting for change in month
// relative to reference time.
//
// This is done because TAFS do not contain year or month information.

time_t Taf2Spdb::_computeTime(int day, int hour, int min)
  
{
  
  int refYear = _refTime.getYear();
  int refMonth = _refTime.getMonth();
  int refDay = _refTime.getDay();
  
  int dayDiff = abs(day - refDay);
  if (dayDiff < 10) {
    // no change in month
    DateTime gtime(refYear, refMonth, day, hour, min, 0);
    return gtime.utime();
  }

  // there has been a change in month
  
  if (day < refDay) {

    // move ahead by one month
    
    if (refMonth == 12) {
      DateTime gtime(refYear + 1, 1, day, hour, min, 0);
      return gtime.utime();
    } else {
      DateTime gtime(refYear, refMonth + 1, day, hour, min, 0);
      return gtime.utime();
    }

  } else {

    // move back by one month
    
    if (refMonth == 1) {
      DateTime gtime(refYear - 1, 12, day, hour, min, 0);
      return gtime.utime();
    } else {
      DateTime gtime(refYear, refMonth - 1, day, hour, min, 0);
      return gtime.utime();
    }

  }

}

/////////////////////////////////////////////////////////////////
// set issue time
// returns 0 on success, -1 on failure

int Taf2Spdb::_setIssueTime()
  
{
  
  _taf.setIssueTime(_fileTime);
  for (size_t ii = _nameTokNum + 1; ii < _toks.size(); ii++) {

    if (_used[ii]) {
      continue;
    }
    
    if (_toks[ii].size() != 7 || _toks[ii][6] != 'Z') {
      continue;
    }
    int day, hour, minute;
    if ((sscanf(_toks[ii].c_str(), "%2d%2d%2d",
		&day, &hour, &minute)) != 3) {
      continue;
    }

    _taf.setIssueTime(_computeTime(day, hour, minute));
    _issueTimeTokNum = ii;
    _used[ii] = true;
    return 0;
    
  } // ii

  // If we get here, we didn't find the issue time.  If we want to process the TAF
  // anyway, we need to set the issue time token number to the name token number so
  // the rest of the tokens will be processed correctly.  In this case, we want to use
  // the WMO header time for the issue time.

  if (_params.process_tafs_with_missing_issue_times)
  {
    _taf.setIssueTime(_getWmoHeaderTime().utime());
    _issueTimeTokNum = _nameTokNum;

    if (_params.debug >= Params::DEBUG_EXTRA)
      cerr << "Issue time not found, setting to " << DateTime::str(_taf.getIssueTime()) << endl;
  }
  
  return -1;

}
  
/////////////////////////////////////////////////////////////////
// get the WMO header time from the WMO header line
// returns 0 on success, -1 on failure

DateTime Taf2Spdb::_getWmoHeaderTime()
  
{
  const string &wmo_hdr_str = _input->getHeadStr();

  // tokenize the header string

  vector<string> toks;
  TaArray<char> lineCopy_;
  char *lineCopy = lineCopy_.alloc(wmo_hdr_str.length() + 1);
  strcpy(lineCopy, wmo_hdr_str.c_str());
  char *tok = strtok(lineCopy, " ");
  while (tok) {
    toks.push_back(tok);
    tok = strtok(NULL, " ");
  }

  // The header time is the 3rd token

  int day, hour, minute;
  if ((sscanf(toks[2].c_str(), "%2d%2d%2d",
	      &day, &hour, &minute)) != 3) {
    if (_params.debug >= Params::DEBUG_NORM)
    {
      cerr << "WARNING - Taf2Spdb" << endl;
      cerr << "Cannot decode WMO header time string: " << toks[2].c_str() << endl;
      cerr << "Using file time" << endl;
    }
    
    return _fileTime;
  }

  return _computeTime(day, hour, minute);
}


/////////////////////////////////////////////////////////////////
// set forecast valid and expire times
// returns 0 on success, -1 on failure

int Taf2Spdb::_setValidExpireTimes()
  
{

  int validTimeTokNum = _issueTimeTokNum + 1;
  if (validTimeTokNum >= (int) _toks.size()) {
    return -1;
  }
  
  string fTimesStr = _toks[validTimeTokNum];

  size_t slash = fTimesStr.find_first_of("/");
  if (slash == string::npos) {
    
    // no slash, expect ddhhhh

    // ensure no alpha characters

    for (int ii = 0; ii < (int) fTimesStr.size(); ii++) {
      if (isalpha(fTimesStr[ii])) {
        return -1;
      }
    }

    int day, hour1, hour2;
    if ((sscanf(fTimesStr.c_str(), "%2d%2d%2d",
		&day, &hour1, &hour2)) != 3) {
      return -1;
    }

    // Work out case where expire hour is less than the valid hour.
    // This means the expire time occurs on the following day. Use
    // issue time as a starting point to construct a more well defined 
    // valid and expire times to pass on to _computeTime

    int day1, day2;
    day1 = day;
    if (hour2 <= hour1) {
      day2 = _incrementDay(day);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "INCREMENTING DAY: " << day1 << "  " << day2 << endl;
      }	
    } else {
      day2 = day;
    }
    

    _taf.setValidTime(_computeTime(day1, hour1, 0));
    _taf.setExpireTime(_computeTime(day2, hour2, 0));

  } else {

    // with slash, expect ddhh/ddhh

    time_t validTime, expireTime;
    if (_setStartEndTimes(fTimesStr, validTime, expireTime)) {
      return -1;
    }
    _taf.setValidTime(validTime);
    _taf.setExpireTime(expireTime);
    
  }

  _validTimeTokNum = validTimeTokNum;
  _used[validTimeTokNum] = true;

  return 0;

}
  
/////////////////////////////////////////////////////////////////
// set start end times, separated by slash
//   ddhh/ddhh
// returns 0 on success, -1 on failure

int Taf2Spdb::_setStartEndTimes(const string &tok, time_t &startTime, time_t &endTime)
  
{

  // with slash, expect ddhh/ddhh
    
  int day1, day2, hour1, hour2;
  if ((sscanf(tok.c_str(), "%2d%2d/%2d%2d",
              &day1, &hour1, &day2, &hour2)) != 4) {
    return -1;
  }

  startTime = _computeTime(day1, hour1, 0);
  endTime = _computeTime(day2, hour2, 0);

  return 0;

}
  
/////////////////////////////////////////////////////////////////
// set from time: FMddhhmm
// returns 0 on success, -1 on failure

int Taf2Spdb::_setFromTime(const string &tok, time_t &fromTime)
  
{

  // expect FMddhhmm
    
  int day, hour, min;
  if ((sscanf(tok.c_str(), "FM%2d%2d%2d",
              &day, &hour, &min)) != 3) {
    return -1;
  }
  fromTime = _computeTime(day, hour, min);

  return 0;

}
  
/////////////////////////////////////////////////////////////////
// set the period limits

void Taf2Spdb::_setPeriodLimits()
  
{

  for (int ii = 1; ii < (int) _toks.size(); ii++) {

    if (_used[ii]) {
      continue;
    }

    const string thisTok = _toks[ii];
    string nextTok;
    if (ii < (int) _toks.size() - 1) {
      nextTok = _toks[ii+1];
    }
    string probTok = _toks[ii-1];
    
    PeriodLimits limits;

    if (thisTok == "TEMPO" || thisTok == "INTER") {
      
      time_t startTime, endTime;
      if (_setStartEndTimes(nextTok, startTime, endTime) == 0) {
        limits.period.pType = Taf::PERIOD_TEMPO;
        limits.period.startTime = startTime;
        limits.period.endTime = endTime;
        limits.startTok = ii;
        limits.startData = ii + 2;
        int prob;
        if (sscanf(probTok.c_str(), "PROB%d", &prob) == 1) {
          limits.period.probPercent = prob;
          limits.startTok = ii - 1;
        }
      }
      
      _used[ii] = true;
      _used[ii+1] = true;
      ii += 2;

    } else if (thisTok == "BECMG") {
      
      time_t startTime, endTime;
      if (_setStartEndTimes(nextTok, startTime, endTime) == 0) {
        limits.period.pType = Taf::PERIOD_BECMG;
        limits.period.startTime = startTime;
        limits.period.endTime = endTime;
        limits.startTok = ii;
        limits.startData = ii + 2;
        int prob;
        if (sscanf(probTok.c_str(), "PROB%d", &prob) == 1) {
          limits.period.probPercent = prob;
          limits.startTok = ii - 1;
        }
      }
      
      _used[ii] = true;
      _used[ii+1] = true;
      ii += 2;

    } else if (thisTok.find("FM") == 0) {
      
      time_t fromTime;
      if (_setFromTime(thisTok, fromTime) == 0) {
        limits.period.pType = Taf::PERIOD_FROM;
        limits.period.startTime = fromTime;
        limits.period.endTime = _taf.getExpireTime();
        limits.startTok = ii;
        limits.startData = ii + 1;
        int prob;
        if (sscanf(probTok.c_str(), "PROB%d", &prob) == 1) {
          limits.period.probPercent = prob;
          limits.startTok = ii - 1;
        }
        _used[ii] = true;
        ii++;
      }
      
    } else if (thisTok.find("PROB") == 0) {

      if (nextTok == "TEMPO" || nextTok == "BECMG" ||
          nextTok.find("FM") == 0) {
        // deal with next token
        continue;
      }

      int prob;
      if (sscanf(thisTok.c_str(), "PROB%d", &prob) == 1) {
        time_t startTime, endTime;
        if (_setStartEndTimes(nextTok, startTime, endTime) == 0) {
          limits.period.pType = Taf::PERIOD_PROB;
          limits.period.startTime = startTime;
          limits.period.endTime = endTime;
          limits.startTok = ii;
          limits.startData = ii + 2;
          limits.period.probPercent = prob;
        }
      }
      _used[ii] = true;
      _used[ii+1] = true;
      ii += 2;

    }

    if (limits.period.pType != Taf::PERIOD_MAIN) {
      _periodLimits.push_back(limits);
    }
    
  } // ii

  // set end tok and end time for each period, not including the main period

  for (int ii = 1; ii < (int) _periodLimits.size(); ii++) {
    PeriodLimits &limits = _periodLimits[ii];
    if (ii == (int) _periodLimits.size() - 1) {
      // last period
      if (limits.period.pType == Taf::PERIOD_FROM) {
        limits.period.endTime = _taf.getExpireTime();
      }
      limits.endData = (int) _toks.size() - 1;
    } else {
      PeriodLimits &next = _periodLimits[ii+1];
      if (limits.period.pType == Taf::PERIOD_FROM) {
        limits.period.endTime = next.period.startTime;
      }
      limits.endData = next.startTok - 1;
    }
  }

  // if periods found, set the end token for the main period
  
  if (_periodLimits.size() > 1) {
    _lastTokMainData = _periodLimits[1].startTok - 1;
  }

}
  
/////////////////////////////////////////////////////////////////
// set the period text
//
// Concatenate the tokens for each period

void Taf2Spdb::_setPeriodText()
  
{

  for (int ii = 0; ii < (int) _periodLimits.size(); ii++) {
    PeriodLimits &limits = _periodLimits[ii];
    string text;
    for (int jj = limits.startTok; jj <= limits.endData; jj++) {
      text += _toks[jj];
      if (jj != limits.endData) {
        text += " ";
      }
    } // jj
    limits.period.text = text;
  } // ii

}
  
/////////////////////////////////////////////////////////////////
// print the period limits

void Taf2Spdb::_printPeriodLimits()
  
{

  // print in verbose debug mode

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "================= period limits ===================" << endl;
    for (int ii = 0; ii < (int) _periodLimits.size(); ii++) {
      const PeriodLimits &limits = _periodLimits[ii];
      cerr << "Period num: " << ii << endl;
      cerr << "  pType: " << Taf::periodType2Str(limits.period.pType) << endl;
      cerr << "  startTok: " << limits.startTok << endl;
      cerr << "  startData: " << limits.startData << endl;     
      cerr << "  endData: " << limits.endData << endl;
      cerr << "  startTime: " << DateTime::strm(limits.period.startTime) << endl;     
      cerr << "  endTime: " << DateTime::strm(limits.period.endTime) << endl;
      cerr << "  probPercent: " << limits.period.probPercent << endl;
      cerr << "  data text: ";
      for (int jj = limits.startData; jj <= limits.endData; jj++) {
        cerr << _toks[jj] << " ";
      }
      cerr << endl;
    } // ii
    cerr << "===================================================" << endl;
  }
  
}
  
/////////////////////////////////////////////////////////////////
// set data fields for specified forecast period
// returns 0 on success, -1 on failure

int Taf2Spdb::_setFields(int startTokNum,
                         int endTokNum,
                         Taf::ForecastPeriod &period)
  
{
  
  // set wind - required
  
  if (_setWind(startTokNum, endTokNum, period)) {
    if (period.pType == Taf::PERIOD_MAIN) {
      if (_params.debug) {
        cerr << "WARNING - Taf2Spdb::_setFields" << endl;
        cerr << "  Cannot set wind" << endl;
      }
      return -1;
    }
  }

  // check for CAVOK

  if (_checkCavok(startTokNum, endTokNum, period)) {
    // no other fields needed
    period.visKm = 9.999;
    return 0;
  }

  // set visibility
  
  if (_setVis(startTokNum, endTokNum, period)) {
    if (period.pType == Taf::PERIOD_MAIN) {
      if (_params.debug) {
        cerr << "WARNING - Taf2Spdb::_setFields" << endl;
        cerr << "  Cannot set vis" << endl;
      }
      return -1;
    }
  }

  // set clouds
  
  if (_setClouds(startTokNum, endTokNum, period)) {
    if (period.pType == Taf::PERIOD_MAIN) {
      if (_params.debug) {
        cerr << "WARNING - Taf2Spdb::_setFields" << endl;
        cerr << "  Cannot set clouds" << endl;
      }
      return -1;
    }
  }

  // set weather

  _setWx(startTokNum, endTokNum, period);

  // set max and min temperatures

  _setTemps(startTokNum, endTokNum, period);
  
  return 0;

}

/////////////////////////////////////////////////////////////////
// set wind speed and direction, for specified forecast period
// returns 0 on success, -1 on failure

int Taf2Spdb::_setWind(int startTokNum,
                       int endTokNum,
                       Taf::ForecastPeriod &period)
  
{
  
  // initialize
  
  double dirn = Taf::missing;
  double vrb = false;
  double speed = Taf::missing;
  double gust = Taf::missing;

  //multiply by this to convert to KMH
  double convFactorKMH = 1;  
  // find units token, decide if units are kmh or knots

  int unitsTokNum = -1;
  for (int ii = startTokNum; ii <= endTokNum; ii++) {
    if (_used[ii]) {
      continue;
    }
    const string &tok = _toks[ii];
    if (tok.find("KMH") != string::npos) {
      unitsTokNum = ii;
      _used[ii] = true;
      break;
    } else if (tok.find("KT") != string::npos) {
      unitsTokNum = ii;
      _used[ii] = true;
      convFactorKMH = KM_PER_NM;
      break;
    } else if (tok.find("MPS") != string::npos) {
      unitsTokNum = ii;
      _used[ii] = true;
      convFactorKMH = KMH_PER_MPS;
    }
  }
  if (unitsTokNum < 0) {
    return -1;
  }
  
  // find wind tok - sometimes the units are in the following token
  // so we have to step back
  
  int windTokNum = unitsTokNum;
  string windStr = _toks[windTokNum];
  {
    int idirn = 0;
    if ((windStr.find("VRB") != 0) &&
        (sscanf(windStr.c_str(), "%3d", &idirn) != 1)) {
      // go back by one token
      windTokNum--;
      windStr = _toks[windTokNum];
    }
  }

  // check for variable, get direction

  if (windStr.find("VRB") == 0) {
    vrb = true;
  } else {
    int idirn = 0;
    if (sscanf(windStr.c_str(), "%3d", &idirn) != 1) {
      return -1;
    }
    dirn = (double) idirn;
  }
  
  // strip off first 3 characters

  string speedStr(windStr, 3);

  // speed is in first 2 or 3 characters

  int ispeed = 0;
  if (sscanf(speedStr.c_str(), "%3d", &ispeed) != 1) {
    if (sscanf(speedStr.c_str(), "%2d", &ispeed) != 1) {
      return -1;
    }
  }
  speed = (double) ispeed * convFactorKMH;

  // gust position

  size_t gpos = speedStr.find("G");
  if (gpos == string::npos) {
    gpos = speedStr.find("P");
  }
  if (gpos != string::npos) {
    string gustStr(speedStr, gpos + 1);
    int igust = 0;
    bool gustFound = false;
    if (sscanf(gustStr.c_str(), "%3d", &igust) == 1) {
      gustFound = true;
    } else if (sscanf(gustStr.c_str(), "%2d", &igust) == 1) {
      gustFound = true;
    }
    if (gustFound) {
      gust = (double) igust * convFactorKMH;
    }
  }

  period.windDirnDegT = dirn;
  period.windDirnVrb = vrb;
  period.windSpeedKmh = speed;
  period.windGustKmh = gust;

  _used[windTokNum] = true;

  return 0;

}
  
/////////////////////////////////////////////////////////////////
// check if this period is CAVOK
// returns true or false

bool Taf2Spdb::_checkCavok(int startTokNum,
                           int endTokNum,
                           Taf::ForecastPeriod &period)
  
{
  period.isCavok = false;
  for (int ii = startTokNum; ii <= endTokNum; ii++) {
    if (_used[ii]) {
      continue;
    }
    const string &tok = _toks[ii];
    if (tok.find("CAVOK") != string::npos) {
      period.isCavok = true;
      _used[ii] = true;
      return true;
    }
  }
  return false;
}

/////////////////////////////////////////////////////////////////
// set visibility in KM
// returns 0 on success, -1 on failure

int Taf2Spdb::_setVis(int startTokNum,
                      int endTokNum,
                      Taf::ForecastPeriod &period)
  
{
  
  // loop through toks, looking for visibility
  
  for (int ii = startTokNum; ii <= endTokNum; ii++) {
    
    if (_used[ii]) {
      continue;
    }

    const string &tok = _toks[ii];

    // check for unlimited in statute miles
    
    if (tok.find("P6SM") != string::npos) {
      period.visKm = 9.999;
      _used[ii] = true;
      return 0;
    }
    
    // units in statute miles?
    
    bool unitsKm = true;
    if (tok.find("SM") != string::npos) {
      unitsKm = false;
    }
    
    if (unitsKm) {
      
      // name must be all numeric, no alpha
      
      bool alphaFound = false;
      for (int jj = 0; jj < (int) tok.size(); jj++) {
        if (isalpha(tok[jj])) {
          alphaFound = true;
          break;
        }
      }
      if (alphaFound) {
        continue;
      }
      
      // read vis in km
      
      int ivis = 0;
      if (sscanf(tok.c_str(), "%d", &ivis) != 1) {
        continue;
      }
      
      period.visKm = (double) ivis / 1000.0;
      _used[ii] = true;
      return 0;

    } else {
      
      // read vis in statute miles
      
      int ivis = 0;
      if (sscanf(tok.c_str(), "%d", &ivis) != 1) {
        if (sscanf(tok.c_str(), "P%d", &ivis) != 1) {
          continue;
        }
      }
      
      period.visKm = ((double) ivis * KM_PER_MI);
      _used[ii] = true;
      return 0;

    }
    
  } // ii
  
  // failure

  return -1;
  
}
  
/////////////////////////////////////////////////////////////////
// set cloud layers, CB, ceiling
// returns 0 on success, -1 on failure

int Taf2Spdb::_setClouds(int startTokNum,
                         int endTokNum,
                         Taf::ForecastPeriod &period)
  
{
  
  // loop through toks, looking for clouds

  vector<Taf::CloudLayer> layers;
  
  for (int ii = startTokNum; ii <= endTokNum; ii++) {
    
    if (_used[ii]) {
      continue;
    }

    const string &tok = _toks[ii];

    // check for 'Sky Clear' and 'No Significant Clouds'
    
    if (tok.find("SKC") != string::npos ||
        tok.find("NSC") != string::npos) {
      period.ceilingKm = 10000 * FEET_TO_KM;
      period.layers.clear();
      _used[ii] = true;
      return 0;
    }
    
    // check for vertical visibility
    
    if (tok.find("VV") != string::npos) {
      int vvis = 0;
      if (sscanf(tok.c_str(), "VV%d", &vvis) == 1) {
        period.ceilingKm = vvis * 100 * FEET_TO_KM;
        period.layers.clear();
        _used[ii] = true;
        return 0;
      }
    }

    // check for cloud layers
    
    if (tok.find("FEW") != string::npos) {
      int level = 0;
      if (sscanf(tok.c_str(), "FEW%d", &level) == 1) {
        Taf::CloudLayer layer;
        layer.heightKm = level * 100 * FEET_TO_KM;
        layer.cloudCover = 0.25;
        if (tok.find("CB") != string::npos) {
          layer.cbPresent = true;
        }
        layers.push_back(layer);
        _used[ii] = true;
      }
    }

    if (tok.find("SCT") != string::npos) {
      int level = 0;
      if (sscanf(tok.c_str(), "SCT%d", &level) == 1) {
        Taf::CloudLayer layer;
        layer.heightKm = level * 100 * FEET_TO_KM;
        layer.cloudCover = 0.5;
        if (tok.find("CB") != string::npos) {
          layer.cbPresent = true;
        }
        layers.push_back(layer);
        _used[ii] = true;
      }
    }

    if (tok.find("BKN") != string::npos) {
      int level = 0;
      if (sscanf(tok.c_str(), "BKN%d", &level) == 1) {
        Taf::CloudLayer layer;
        layer.heightKm = level * 100 * FEET_TO_KM;
        layer.cloudCover = 0.75;
        if (tok.find("CB") != string::npos) {
          layer.cbPresent = true;
        }
        layers.push_back(layer);
        _used[ii] = true;
      }
    }

    if (tok.find("OVC") != string::npos) {
      int level = 0;
      if (sscanf(tok.c_str(), "OVC%d", &level) == 1) {
        Taf::CloudLayer layer;
        layer.heightKm = level * 100 * FEET_TO_KM;
        layer.cloudCover = 1.0;
        if (tok.find("CB") != string::npos) {
          layer.cbPresent = true;
        }
        layers.push_back(layer);
        _used[ii] = true;
      }
    }

  } // ii
  
  if (layers.size() == 0) {
    return -1;
  }

  // compute ceiling from layers

  period.ceilingKm = 10000 * FEET_TO_KM;
  for (int ii = 0; ii < (int) layers.size(); ii++) {
    const Taf::CloudLayer &layer = layers[ii];
    if (layer.cloudCover > 0.6) { // BKN and OVC
      double ceilingKm = layer.heightKm;
      if (ceilingKm < period.ceilingKm) {
        period.ceilingKm = ceilingKm;
      }
    }
  } // ii
  period.layers = layers;

  return 0;
  
}
  
/////////////////////////////////////////////////////////////////
// set weather types

void Taf2Spdb::_setWx(int startTokNum,
                      int endTokNum,
                      Taf::ForecastPeriod &period)
  
{

  // check is CBs are present in the cloud layers

  bool cbPresent = false;
  for (int ii = 0; ii < (int) period.layers.size(); ii++) {
    if (period.layers[ii].cbPresent) {
      cbPresent = true;
    }
  }

  // loop through toks, looking for wx
  
  vector<string> wx;
  if (cbPresent) {
    wx.push_back("CB");
  }
  
  for (int ii = startTokNum; ii <= endTokNum; ii++) {
    
    if (_used[ii]) {
      continue;
    }

    const string &tok = _toks[ii];

    for (int jj = 0; jj < Taf::nWxTypes; jj++) {
      if (tok.find(Taf::wxTypes[jj]) != string::npos) {
        wx.push_back(tok);
        _used[ii] = true;
        break;
      }
    }

  }

  period.wx = wx;

}
  
/////////////////////////////////////////////////////////////////
// set temperatures if available

void Taf2Spdb::_setTemps(int startTokNum,
                         int endTokNum,
                         Taf::ForecastPeriod &period)
  
{
  
  // loop through toks, looking for visibility
  
  for (int ii = startTokNum; ii <= endTokNum; ii++) {
    
    if (_used[ii]) {
      continue;
    }
    
    const string &tok = _toks[ii];

    // check for max temp
    
    if (tok.find("TX") == 0) {
      int itemp, day, hour;
      if (sscanf(tok.c_str(), "TX%2d/%2d%2d", &itemp, &day, &hour) == 3) {
        time_t mtime = _computeTime(day, hour, 0);
        period.maxTempC = (double) itemp;
        period.maxTempTime = mtime;
        _used[ii] = true;
        continue;
      }
      if (sscanf(tok.c_str(), "TXM%2d/%2d%2d", &itemp, &day, &hour) == 3) {
        time_t mtime = _computeTime(day, hour, 0);
        period.maxTempC = (double) itemp * -1.0;
        period.maxTempTime = mtime;
        _used[ii] = true;
        continue;
      }
    }
      
    // check for min temp
    
    if (tok.find("TN") == 0) {
      int itemp, day, hour;
      if (sscanf(tok.c_str(), "TN%2d/%2d%2d", &itemp, &day, &hour) == 3) {
        time_t mtime = _computeTime(day, hour, 0);
        period.minTempC = (double) itemp;
        period.minTempTime = mtime;
        _used[ii] = true;
        continue;
      }
      if (sscanf(tok.c_str(), "TNM%2d/%2d%2d", &itemp, &day, &hour) == 3) {
        time_t mtime = _computeTime(day, hour, 0);
        period.minTempC = (double) itemp * -1.0;
        period.minTempTime = mtime;
        _used[ii] = true;
        continue;
      }
    }
      
  } // ii
  
}
  
////////////////////////////////////////////////
// over-write cancelled TAF

void Taf2Spdb::_overwriteCancelled()
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _overwriteCancelled()" << endl;
  }
  
  // get the data type

  si32 dataType = Spdb::hash4CharsToInt32(_taf.getStationId().c_str());
  
  // get the entry to cancel
  
  DsSpdb in;
  if(in.getExact(_params.cancel_input_url,
                 _taf.getValidTime(),
                 dataType)) {
    if (_params.debug) {
      cerr << "ERROR - Taf2Spdb::_overwriteCancelled" << endl;
      cerr << "  Cannot retrieve TAF to cancel: " << endl;
      cerr << "    Station: " << _taf.getStationId() << endl;
      cerr << "    Valid time: " << DateTime::strm(_taf.getValidTime()) << endl;
    }
    return;
  }
  
  // get chunk vector
  
  const vector<Spdb::chunk_t> &chunks = in.getChunks();
  if (chunks.size() < 1) {
    if (_params.debug) {
      cerr << "ERROR - Taf2Spdb::_overwriteCancelled" << endl;
      cerr << "  Cannot find TAF to cancel: " << endl;
      cerr << "    Station: " << _taf.getStationId() << endl;
      cerr << "    Valid time: " << DateTime::strm(_taf.getValidTime()) << endl;
      return;
    }
  }
  
  for (int ii = chunks.size() - 1; ii >= 0; ii--) {
    
    Taf candidate;
    candidate.disassemble(chunks[ii].data, chunks[ii].len);
    
    if (candidate.isCancelled()) {
      // already cancelled
      if (_params.debug) {
        cerr << "-->> Previously cancelled" << endl;
      }
      continue;
    }

    time_t cancelTime = _taf.getIssueTime();
    candidate.setCancelled(true);
    candidate.setCancelTime(cancelTime);
    
    if (_params.debug) {
      cerr << "Cancelling TAF: " << endl;
      candidate.print(cerr);
    }
    
    // add to spdb objects as appropriate
    
    candidate.assemble();
    
    DsSpdb out;
    out.addPutChunk(chunks[ii].data_type,
		    chunks[ii].valid_time,
		    chunks[ii].expire_time,
		    candidate.getBufLen(),
		    candidate.getBufPtr(),
		    chunks[ii].data_type2);
    if (out.put(_params.xml_output_url,
		SPDB_TAF_ID, SPDB_TAF_LABEL)) {
      cerr << "ERROR - Taf2Spdb::_overwriteCancelled" << endl;
      cerr << out.getErrStr() << endl;
    }

  } // ii

}

//////////////////////////////////////////////
// tokenize a string into a vector of strings

void Taf2Spdb::_tokenize(const string &str,
                         const string &spacer,
                         vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  while (true) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      return;
    } else if (end == string::npos) {
      string tok;
      tok.assign(str, start, string::npos);
      toks.push_back(tok);
      return;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}
