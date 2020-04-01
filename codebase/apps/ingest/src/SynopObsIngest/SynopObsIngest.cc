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
////////////////////////////////////////////////////////////////////////
// SynopObsIngest.cc
//
// SynopObsIngest object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2011
//
///////////////////////////////////////////////////////////////
//
// SynopObsIngest reads NOAA synop obs from ASCII files, decodes
// them and writes them out to SPDB.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>
#include <toolsa/Path.hh>
#include <physics/thermo.h>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>
#include <rapformats/station_reports.h>
#include "SynopObsIngest.hh"
using namespace std;

// Constructor

SynopObsIngest::SynopObsIngest(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "SynopObsIngest";
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
    return;
  }

  // set up output objects

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _spdbDecoded.setDebug();
    _spdbAscii.setDebug();
  }
  if (_params.decoded_compression == Params::COMPRESSION_GZIP) {
    _spdbDecoded.setChunkCompressOnPut(Spdb::COMPRESSION_GZIP);
  } else if (_params.decoded_compression == Params::COMPRESSION_BZIP2) {
    _spdbDecoded.setChunkCompressOnPut(Spdb::COMPRESSION_BZIP2);
  }

  // file input object
  
  if (_params.mode == Params::FILELIST) {
    
    // FILELIST mode
    
    _input = new DsInputPath(_progName,
                             _params.debug >= Params::DEBUG_VERBOSE,
                             _args.inputFileList);
    
  } else if (_params.mode == Params::ARCHIVE) {
    
    // archive mode - start time to end time
    
    _input = new DsInputPath(_progName,
                             _params.debug >= Params::DEBUG_VERBOSE,
                             _params.input_dir,
                             _args.startTime, _args.endTime);
    
  } else if (_params.mode == Params::REALTIME) {
    
    // realtime mode - no latest_data_info file
    
    _input = new DsInputPath(_progName,
                             _params.debug >= Params::DEBUG_VERBOSE,
                             _params.input_dir,
                             _params.max_realtime_valid_age,
                             PMU_auto_register,
                             _params.latest_data_info_avail,
                             true);
    
    if (_params.strict_subdir_check) {
      _input->setStrictDirScan(true);
    }
    
    if (_params.file_name_check) {
      _input->setSubString(_params.file_match_string);
    }
    
  }
  
  // load up the station locations

  if (_loadLocations(_params.station_location_path)) {
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

SynopObsIngest::~SynopObsIngest()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  if (_input) {
    delete _input;
  }

}

//////////////////////////////////////////////////
// Run

int SynopObsIngest::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // loop through available files
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    if (DsInputPath::getDataTime(inputPath, _fileTime)) {
      if (_params.mode == Params::REALTIME) {
        _fileTime = time(NULL); // now
      } else {
        cerr << "WARNING - SynopObsIngest::Run" << endl;
        cerr << "  Cannot decode time from file path: " << inputPath << endl;
        continue;
      }
    }
    
    if (_processFile(inputPath)) {
      cerr << "WARNING - SynopObsIngest::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }
    
  } // while

  return 0;

}

////////////////////
// process the file

int SynopObsIngest::_processFile(const char *filePath)
  
{

  if (_params.debug) {
    cerr << "Processing file: " << filePath << endl;
    cerr << "  File time: " << DateTime::strm(_fileTime) << endl;
  }

  // registration

  char procmapString[BUFSIZ];
  Path path(filePath);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);

  // clear SPDB

  _spdbDecoded.clearPutChunks();
  _spdbAscii.clearPutChunks();

  // Open the file

  FILE *fp;
  if((fp = fopen(filePath, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SynopObsIngest::_processFile" << endl;
    cerr << "  Cannot open obs file: "
	 << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read through the file, identifying message blocks

  char line[BUFSIZ];
  string message;
  while(fgets(line, BUFSIZ, fp) != NULL) {

    if (strchr(line, 0x01) != NULL ||
        strchr(line, 0x03) != NULL) {

      // end of message
      if (message.find("AAXX") != string::npos) {
        _processMessage(message);
      }

      message.clear();
      
    } else {

      message += line;

    }
    
  }

  // close the file

  fclose(fp);

  // write the output

  if (_doPut()) {
    return -1;
  } else {
    return 0;
  }
  
}
   
////////////////////
// process a message

int SynopObsIngest::_processMessage(const string &message)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << endl;
    cerr << "Processing OBS message: " << endl;
    cerr << "========================================" << endl;
    cerr << message << endl;
    cerr << "========================================" << endl;
  }

  // tokenize the message
  
  vector<string> toks;
  TaStr::tokenize(message, " \n\r\t", toks);

  if (_params.debug >= Params::DEBUG_EXTRA) {
    for (size_t ii = 0; ii < toks.size(); ii++) {
      cerr << " tok[" << ii << "]: " << toks[ii] << endl;
    }
  }

  // find BBOX

  size_t aaxxPos = 0;
  for (size_t ii = 0; ii < toks.size(); ii++) {
    if (toks[ii] == "AAXX") {
      aaxxPos = ii;
    }
  }
  if (aaxxPos == 0) {
    // not a valid message
    cerr << "ERROR - SynopObsIngest::_processMessage" << endl;
    cerr << "  Cannot find AAXX token, message: " << endl;
    cerr << message << endl;
    return -1;
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "AAXX pos: " << aaxxPos << endl;
  }

  // compute date and time from header
  // will be overwritten later from within message block,
  // if time is available in message

  _computeTimeFromHeader(toks, aaxxPos);

  // break up into blocks

  vector<vector<string> > blocks;
  vector<string> block;
  for (size_t ii = aaxxPos + 1; ii < toks.size(); ii++) {
    block.push_back(toks[ii]);
    if (toks[ii].find("=") != string::npos) {
      blocks.push_back(block);
      block.clear();
    }
  }

  for (size_t jj = 0; jj < blocks.size(); jj++) {
    vector<string> &thisBlock = blocks[jj];
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Block num: " << jj << endl;
      for (size_t ii = 0; ii < thisBlock.size(); ii++) {
        cerr << "    " << thisBlock[ii] << endl;
      }
    }
    _handleBlock(thisBlock);
  }

  return 0;
  
}

////////////////////////////////////////////
// compute date and time from message header

int SynopObsIngest::_computeTimeFromHeader(const vector<string> &toks,
                                          size_t aaxxPos)

{

  // move back through tokens before AAXX
  
  for (int ii = (int) aaxxPos - 1; ii >= 0; ii--) {
    if (_computeTime(toks[ii]) == 0) {
      return 0;
    }
  }

  return -1;

}

///////////////////////////////////
// compute date and time from token

int SynopObsIngest::_computeTime(const string &tok)

{
  
  if (tok.size() != 6) {
    return -1;
  }

  // get obs time from token

  int day, hour, min;
  if (sscanf(tok.c_str(), "%2d%2d%2d", &day, &hour, &min) != 3) {
    return -1;
  }

  // compute obs time from file name

  DateTime fileTime(_fileTime);
  DateTime obsTime(fileTime.getYear(), fileTime.getMonth(), day, hour, min, 0);

  // check that we have the correct month

  int timeDiff = obsTime.utime() - fileTime.utime();
  if (timeDiff > 86400) {
    // one month ahead
    if (obsTime.getMonth() == 1) {
      obsTime.setMonth(12);
      obsTime.setYear(obsTime.getYear() - 1);
    } else {
      obsTime.setMonth(obsTime.getMonth() - 1);
    }
  } else if (timeDiff < -86400) {
    // one month behind
    if (obsTime.getMonth() == 12) {
      obsTime.setMonth(1);
      obsTime.setYear(obsTime.getYear() + 1);
    } else {
      obsTime.setMonth(obsTime.getMonth() + 1);
    }
  }

  _obsTime = obsTime.utime();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Obs time: " << DateTime::strm(_obsTime) << endl;
  }

  return 0;

}

//////////////////////////////
// init variables in a report
  
void SynopObsIngest::_initReportVars()
  
{

  _stationName.clear();
  _longName.clear();
  _stationId = -9999;

  _latitude = -9999;
  _longitude = -9999;

  _precipOmitted = true;
  _wxIncluded = true;
  _cloudBaseMeters = true;
  _visMeters = true;

  _octas = -9999;
  _windDirn = -9999;
  _windVariable = false;
  _windSpeedMps = -9999;
  _windSpeedCode = 0;
  _windSpeedIsMetric = true;
  _windSpeedMult = 1.0;
  _windSpeedNotAvailable = false;

  _tempC = -9999;
  _dewPtC = -9999;
  _pressureMslHpa = -9999;
  _pressureStationHpa = -9999;

  _pressureTendCode = -9999;
  _pressureTendSign = -9999;
  _pressureTendHpa = -9999;

  _precipAccumMm = -9999;
  _accumPeriodSecs = -9999;
  
  _presentWx = 0;

  _sstC = -9999;

}

///////////////////////
// handle a block
  
int SynopObsIngest::_handleBlock(const vector<string> &toks)
  
{

  // initialize

  _initReportVars();

  // decode the obs block
  
  if (_decodeObsBlock(toks)) {
    return -1;
  }

  // check bounding box

  if (_params.check_bounding_box) {
    if (_latitude < _params.bounding_box.min_lat ||
	_latitude > _params.bounding_box.max_lat ||
	_longitude < _params.bounding_box.min_lon ||
	_longitude > _params.bounding_box.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << endl;
	cerr << "Rejecting station: " << _stationName << endl;
	cerr << "  Station position no within bounding box" << endl;
      }
      return 0;
    }
  }

  // encode the report

  if (_params.output_report_type == Params::XML_ONLY) {
    _loadWxObs();
  } else {
    _loadReport();
  }

  // add chunk to spdb objects

  if (_params.write_decoded_obs) {
    _spdbDecoded.addPutChunk(_stationId,
                             _obsTime,
                             _obsTime + _params.expire_seconds,
                             _buf.getLen(), _buf.getPtr());
  }
  
  if (_params.write_ascii_obs) {
    _spdbAscii.addPutChunk(_stationId,
                           _obsTime,
                           _obsTime + _params.expire_seconds,
                           _obsStr.size() + 1,
                           _obsStr.c_str());
  }
  return 0;

}

///////////////////////
// decode the obs block
  
int SynopObsIngest::_decodeObsBlock(const vector<string> &toks)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "--------------------------------------------" << endl;
    cerr << "Decoding obs block:" << endl;
    for (size_t ii = 0; ii < toks.size(); ii++) {
      cerr << "  tok[" << ii << "]: " << toks[ii] << endl;
    }
    cerr << "--------------------------------------------" << endl;
  }

  if (toks.size() < 4) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - not enough tokens for valid message" << endl;
    }
    return -1;
  }

  // time group is first token
  // overwrite time from header if available in message

  size_t pos = 0;
  _decodeObsTime(toks[pos]);
  pos++;
  
  // station name is second token
  
  _stationName = toks[pos];
  if (sscanf(_stationName.c_str(), "%d", &_stationId) != 1) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - cannot decode station name into ID" << endl;
      cerr << "  name: " << _stationName << endl;
    }
    return -1;
  }
  pos++;
  
  // get station location
       
  map< int, StationLoc, less<int> >::iterator iloc;
  iloc = _locations.find(_stationId);
  if (iloc == _locations.end()) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - cannot find station in list" << endl;
      cerr << "  name: " << _stationName << endl;
      cerr << "  id: " << _stationId << endl;
    }
    return -1;
  }
  StationLoc &stationLoc = iloc->second;

  _longName = stationLoc.getCountry();
  _longName += ":";
  _longName += stationLoc.getLongName();
  _latitude = stationLoc.getLatitude();
  _longitude = stationLoc.getLongitude();
  _latitude = stationLoc.getLatitude();
  _heightM = stationLoc.getHeightM();

  // ceiling, vis, flags - mandatory

  if (toks.size() > pos) {
    const string &tok = toks[pos];
    if (_decodeObsVis(tok) == 0) {
      pos++;
    } else {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - bad ceiling/vis token: " << tok << endl;
      }
      return -1;
    }
  }

  // wind speed and dirn - mandatory

  if (toks.size() > pos) {
    const string &tok = toks[pos];
    if (_decodeObsWind(tok) == 0) {
      pos++;
    } else {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - bad wind token: " << tok << endl;
      }
      return -1;
    }
  }

  // high wind speed
  
  if (toks.size() > pos) {
    const string &tok = toks[pos];
    if (tok[0] == '0' && tok[1] == '0') {
      _decodeObsHighWindSpeed(tok);
      pos++;
    }
  }

  // temperature

  if (toks.size() > pos) {
    const string &tok = toks[pos];
    if (tok[0] == '1') {
      _decodeObsTemp(tok);
      pos++;
    }
  }

  // dewpt

  if (toks.size() > pos) {
    const string &tok = toks[pos];
    if (tok[0] == '2') {
      _decodeObsDewPt(tok);
      pos++;
    }
  }

  // station pressure

  if (toks.size() > pos) {
    const string &tok = toks[pos];
    if (tok[0] == '3') {
      _decodeObsPressureStation(tok);
      pos++;
    }
  }

  // MSL pressure

  if (toks.size() > pos) {
    const string &tok = toks[pos];
    if (tok[0] == '4') {
      _decodeObsPressureMsl(tok);
      pos++;
    }
  }

  // surface pressure tendency

  if (toks.size() > pos) {
    const string &tok = toks[pos];
    if (tok[0] == '5') {
      _decodeObsPressureTendency(tok);
      pos++;
    }
  }

  // precip amount

  if (toks.size() > pos) {
    const string &tok = toks[pos];
    if (tok[0] == '6') {
      _decodeObsPrecip(tok);
      pos++;
    }
  }

  // present weather

  if (toks.size() > pos) {
    const string &tok = toks[pos];
    if (tok[0] == '7') {
      _decodeObsWx(tok);
      pos++;
    }
  }

  // find start of oceanographic data

  for (size_t ii = pos; ii < toks.size(); ii++) {
    string first = toks[ii].substr(0, 3);
    if (first == "222") {
      pos = ii+1;
      break;
    }
  }

  // sea surface temp

  if (toks.size() > pos) {
    const string &tok = toks[pos];
    if (tok[0] == '0') {
      _decodeObsSst(tok);
      pos++;
    }
  }

  // create obs string

  char hdr[4096];
  sprintf(hdr,
          "AAXX synop report:\n"
          "  station, longName: %s, %s\n"
          "  time: %s\n"
          "  lon, lat, htM: %g, %g, %g\n"
          "  pressureHpa MSL, station, tendency: %g, %g, %g\n"
          "  octas, cloudHtM, visM: %d, %g, %g\n"
          "  tempC, dewptC: %g, %g\n"
          "  wind dirn, speedMps, speedKnots: %g, %g, %g\n"
          "  sstC: %g\n",
          _stationName.c_str(),
          _longName.c_str(),
          DateTime::strm(_obsTime).c_str(),
          _longitude, _latitude, _heightM,
          _pressureMslHpa, _pressureStationHpa, _pressureTendHpa,
          _octas, _cloudBaseMeters, _visMeters,
          _tempC, _dewPtC,
          _windDirn, _windSpeedMps, _windSpeedMps * MS_TO_KNOTS,
          _sstC);

  _obsStr = hdr;
  _obsStr += "  ";
  for (size_t ii = 0; ii < toks.size(); ii++) {
    _obsStr += toks[ii];
    if (ii < toks.size() - 1) {
      if ((ii + 1) % 6 == 0) {
        _obsStr += "\n  ";
      } else {
        _obsStr += " ";
      }
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "ObsStr: " << endl;
    cerr << _obsStr << endl;
  }

  return 0;

}

///////////////////////////////////
// decode the obs time

int SynopObsIngest::_decodeObsTime(const string &tok)

{
  
  if (tok.size() != 5) {
    return -1;
  }

  // get obs time from token

  int day, hour;
  if (sscanf(tok.c_str(), "%2d%2d", &day, &hour) != 2) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - cannot decode day/hour" << endl;
      cerr << "  tok: " << tok << endl;
    }
    return -1;
  }

  // compute obs time

  DateTime fileTime(_fileTime);
  DateTime obsTime(fileTime.getYear(), fileTime.getMonth(), day, hour, 0, 0);

  // check that we have the correct month

  int timeDiff = obsTime.utime() - fileTime.utime();
  if (timeDiff > 86400) {
    // one month ahead
    if (obsTime.getMonth() == 1) {
      obsTime.setMonth(12);
      obsTime.setYear(obsTime.getYear() - 1);
    } else {
      obsTime.setMonth(obsTime.getMonth() - 1);
    }
  } else if (timeDiff < -86400) {
    // one month behind
    if (obsTime.getMonth() == 12) {
      obsTime.setMonth(1);
      obsTime.setYear(obsTime.getYear() + 1);
    } else {
      obsTime.setMonth(obsTime.getMonth() + 1);
    }
  }

  _obsTime = obsTime.utime();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  Obs time: " << DateTime::strm(_obsTime) << endl;
  }

  // set wind speed units

  if (tok[4] == '/') {

    _windSpeedNotAvailable = true;

  } else {
    int speedFlag = 0;
    int iflag;
    if (sscanf(tok.c_str() + 4, "%1d", &iflag) == 1) {
      speedFlag = iflag;
    }
    
    if (speedFlag < 2) {
      _windSpeedIsMetric = true;
      _windSpeedMult = 1.0;
    } else {
      _windSpeedIsMetric = false;
      _windSpeedMult = KNOTS_TO_MS;
    }

  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  Wind speed not available: "
         << string(_windSpeedNotAvailable?"Y":"N") << endl;
    cerr << "  Wind speed metric: "
         << string(_windSpeedIsMetric?"Y":"N") << endl;
  }

  return 0;

}

///////////////////////////////////
// decode the ceiling and vis

int SynopObsIngest::_decodeObsVis(const string &tok)

{
  
  if (tok.size() != 5) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - cannot decode vis token" << endl;
      cerr << "  tok: " << tok << endl;
    }
    return -1;
  }

  int iR = 9;
  int ival;
  if (sscanf(tok.c_str(), "%1d", &ival) == 1) {
    iR = ival;
  }
  if (iR > 4) {
    return -1;
  }
  if (iR == 4) {
    _precipOmitted = true;
  } else {
    _precipOmitted = false;
  }
  
  int iX = 9;
  if (sscanf(tok.c_str() + 1, "%1d", &ival) == 1) {
    iX = ival;
  }
  if (iX < 1 || iX > 7) {
    return -1;
  }
  if (iX == 1) {
    _wxIncluded = true;
  } else {
    _wxIncluded = false;
  }

  int iH = 9;
  if (sscanf(tok.c_str() + 2, "%1d", &ival) == 1) {
    iH = ival;
    switch (iH) {
      case 0: _cloudBaseMeters = 50; break;
      case 1: _cloudBaseMeters = 100; break;
      case 2: _cloudBaseMeters = 200; break;
      case 3: _cloudBaseMeters = 300; break;
      case 4: _cloudBaseMeters = 600; break;
      case 5: _cloudBaseMeters = 1000; break;
      case 6: _cloudBaseMeters = 1500; break;
      case 7: _cloudBaseMeters = 2000; break;
      case 8: _cloudBaseMeters = 2550; break;
      default: _cloudBaseMeters = 9999;
    }
  } else {
    _cloudBaseMeters = -9999;
  }
    
  int iVV = 99;
  if (sscanf(tok.c_str() + 3, "%2d", &ival) == 1) {
    iVV = ival;
    if (iVV <= 50) {
      _visMeters = iVV * 100.0;
    } else if (iVV <= 80) {
      _visMeters = (iVV - 50.0) * 1000.0;
    } else if (iVV <= 89) {
      _visMeters = ((iVV - 80) * 5.0 + 30.0) * 1000.0;
    } else {
      switch (iVV) {
        case 90: _visMeters = 25; break;
        case 91: _visMeters = 50; break;
        case 92: _visMeters = 200; break;
        case 93: _visMeters = 500; break;
        case 94: _visMeters = 1000; break;
        case 95: _visMeters = 2000; break;
        case 96: _visMeters = 4000; break;
        case 97: _visMeters = 10000; break;
        case 98: _visMeters = 20000; break;
        default: _visMeters = 50000;
      }
    }
  } else {
    _visMeters = -9999;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  precipOmitted: " << _precipOmitted << endl;
    cerr << "  wxIncluded: " << _wxIncluded << endl;
    cerr << "  cloudBaseMeters: " << _cloudBaseMeters << endl;
    cerr << "  visMeters: " << _visMeters << endl;
  }

  return 0;

}
  
///////////////////////////////////
// decode the wind etc

int SynopObsIngest::_decodeObsWind(const string &tok)

{
  
  if (tok.size() != 5) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - cannot decode wind token" << endl;
      cerr << "  tok: " << tok << endl;
    }
    return -1;
  }

  int ival;
  if (sscanf(tok.c_str(), "%1d", &ival) == 1) {
    _octas = ival;
  }

  _windSpeedCode = 0;
  if (sscanf(tok.c_str() + 3, "%2d", &ival) == 1) {
    _windSpeedCode = ival;
    _windSpeedMps = (double) _windSpeedCode * _windSpeedMult;
  }
  
  int iDir = 99;
  if (sscanf(tok.c_str() + 1, "%2d", &ival) == 1) {
    iDir = ival;
    if (iDir == 0) {
      _windVariable = false;
      _windDirn = 0.0;
      _windSpeedMps = 0.0;
    } else if (iDir == 99) {
      _windVariable = true;
      _windDirn = 0.0;
    } else if (iDir > 36) {
      return -1;
    } else {
      _windVariable = false;
      _windDirn = (double) iDir * 10.0;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  octas: " << _octas << endl;
    cerr << "  windDirn: " << _windDirn << endl;
    cerr << "  windVariable: " << string(_windVariable? "Y":"N") << endl;
    cerr << "  windSpeedMps: " << _windSpeedMps << endl;
    cerr << "  windSpeedKts: " << _windSpeedMps * MS_TO_KNOTS << endl;
  }

  return 0;

}

///////////////////////////////////
// decode high wind speed

int SynopObsIngest::_decodeObsHighWindSpeed(const string &tok)

{

  if (tok.size() != 5) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - cannot decode high wind speed token" << endl;
      cerr << "  tok: " << tok << endl;
    }
    return -1;
  }

  int iSpeed = 0;
  if (sscanf(tok.c_str(), "00%3d", &iSpeed) != 1) {
    return 0;
  }
  if (iSpeed == 0) {
    return 0;
  }

  // only applies if normal wind speed is over 98

  if (_windSpeedCode < 98) {
    return 0;
  } 

  _windSpeedMps = (double) iSpeed * _windSpeedMps;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  HIGH windSpeedMps: " << _windSpeedMps << endl;
  }

  return 0;

}

///////////////////////////////////
// decode the temperature

int SynopObsIngest::_decodeObsTemp(const string &tok)

{
  
  if (tok.size() != 5) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - cannot decode temp token" << endl;
      cerr << "  tok: " << tok << endl;
    }
    return -1;
  }

  int isign, itemp;
  if (sscanf(tok.c_str(), "1%1d%3d", &isign, &itemp) != 2) {
    return 0;
  }
  
  _tempC = (double) itemp / 10.0;
  if (isign != 0) {
    _tempC *= -1.0;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  tempC: " << _tempC << endl;
  }

  return 0;

}
  
///////////////////////////////////
// decode the dew point

int SynopObsIngest::_decodeObsDewPt(const string &tok)

{
  
  if (tok.size() != 5) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - cannot decode dewPt token" << endl;
      cerr << "  tok: " << tok << endl;
    }
    return -1;
  }

  int isign, ival;
  if (sscanf(tok.c_str(), "2%1d%3d", &isign, &ival) != 2) {
    return 0;
  }
  
  if (isign == 9) {
    // data is RH
    _rh = ival;
    _dewPtC = PHYrhdp(_tempC, _rh);
  } else {
    // data is dewpt
    _dewPtC = (double) ival / 10.0;
    if (isign != 0) {
      _dewPtC *= -1.0;
    }
    _rh = PHYrelh(_tempC, _dewPtC);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  dewPtC: " << _dewPtC << endl;
    cerr << "  RH: " << _rh << endl;
  }

  return 0;

}
  
///////////////////////////////////
// decode the station pressure

int SynopObsIngest::_decodeObsPressureStation(const string &tok)

{
  
  if (tok.size() != 5) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - cannot decode station pressure token" << endl;
      cerr << "  tok: " << tok << endl;
    }
    return -1;
  }

  int iPress;
  if (sscanf(tok.c_str(), "3%4d", &iPress) != 1) {
    return 0;
  }

  if (iPress < 5000) {
    _pressureStationHpa = (double) iPress / 10.0 + 1000.0;
  } else {
    _pressureStationHpa = (double) iPress / 10.0;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  pressureStationHpa: " << _pressureStationHpa << endl;
  }

  return 0;

}
  
///////////////////////////////////
// decode the MSL pressure

int SynopObsIngest::_decodeObsPressureMsl(const string &tok)

{
  
  if (tok.size() != 5) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - cannot decode MSL pressure token" << endl;
      cerr << "  tok: " << tok << endl;
    }
    return -1;
  }

  int iPress;
  if (sscanf(tok.c_str(), "4%4d", &iPress) != 1) {
    return 0;
  }

  if (iPress < 5000) {
    _pressureMslHpa = (double) iPress / 10.0 + 1000.0;
  } else {
    _pressureMslHpa = (double) iPress / 10.0;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  pressureMslHpa: " << _pressureMslHpa << endl;
  }

  return 0;

}
  
////////////////////////////////////////
// decode the surface pressure tendency

int SynopObsIngest::_decodeObsPressureTendency(const string &tok)

{
  
  if (tok.size() != 5) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - cannot decode pressure tendency token" << endl;
      cerr << "  tok: " << tok << endl;
    }
    return -1;
  }

  int iCode, iPress;
  if (sscanf(tok.c_str(), "5%1d%3d", &iCode, &iPress) != 2) {
    return 0;
  }

  _pressureTendCode = iCode;
  if (iCode == 4) {
    _pressureTendSign = 0;
  } else if (iCode >= 0 && iCode <= 3) {
    _pressureTendSign = 1;
  } else {
    _pressureTendSign = -1;
  }

  if (_pressureTendSign == 0) {
    _pressureTendHpa = (iPress / 10.0);
  } else {
    _pressureTendHpa = (iPress / 10.0) * _pressureTendSign;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  pressureTendCode: " << _pressureTendCode << endl;
    cerr << "  pressureTendSign: " << _pressureTendSign << endl;
    cerr << "  pressureTendHpa: " << _pressureTendHpa << endl;
  }

  return 0;

}
  
////////////////////////////////////////
// decode the precip accum

int SynopObsIngest::_decodeObsPrecip(const string &tok)

{
  
  if (tok.size() != 5) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - cannot decode precip token" << endl;
      cerr << "  tok: " << tok << endl;
    }
    return -1;
  }

  int iVal, iCode;
  if (sscanf(tok.c_str(), "6%3d%1d", &iVal, &iCode) != 2) {
    return 0;
  }
  
  _precipAccumMm = 0.0;
  if (iVal < 990) {
    _precipAccumMm = iVal;
  } else if (iVal == 990) {
    _precipAccumMm = 0.05;
  } else {
    _precipAccumMm = (iVal - 990.0) / 10.0;
  }

  switch (iCode) {
    case 1: _accumPeriodSecs = 21600; break;
    case 2: _accumPeriodSecs = 43200; break;
    case 3: _accumPeriodSecs = 64800; break;
    case 4: _accumPeriodSecs = 86400; break;
    case 5: _accumPeriodSecs = 3600; break;
    case 6: _accumPeriodSecs = 7200; break;
    case 7: _accumPeriodSecs = 10800; break;
    case 8: _accumPeriodSecs = 32400; break;
    case 9: _accumPeriodSecs = 54000; break;
    default: _accumPeriodSecs = 86400;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  precipAccumMm: " << _precipAccumMm << endl;
    cerr << "  accumPeriodSecs: " << _accumPeriodSecs << endl;
  }

  return 0;

}
  
//////////////////////////////////////
// decode the present and past weather

int SynopObsIngest::_decodeObsWx(const string &tok)

{
  
  if (tok[0] != '7') {
    return 0;
  }

  if (tok.size() != 5) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - cannot decode present and past wx token" << endl;
      cerr << "  tok: " << tok << endl;
    }
    return -1;
  }
  
  _presentWx = 0;

  int iwx = -1;
  int ival;
  if (sscanf(tok.c_str() + 1, "%2d", &ival) == 1) {
    iwx = ival;
  }

  // int pwx1 = -1, pwx2 = -1;
  // int ival1, ival2;
  // if (sscanf(tok.c_str() + 3, "%1d%1d", &ival1, &ival2) == 2) {
  //   pwx1 = ival1;
  //   pwx2 = ival2;
  // }

  switch (iwx) {
    case 99:
      _presentWx = (WT_PTS | WT_GR);
      break;
    case 98:
      _presentWx |= (WT_TS | WT_DS);
      break;
    case 97:
      _presentWx |= (WT_PTS | WT_PRA);
      break;
    case 96:
      _presentWx = (WT_TS | WT_GR);
      break;
    case 95: case 93: case 91:
      _presentWx |= (WT_TS | WT_MRA);
      break;
    case 94: case 92:
      _presentWx |= (WT_TS | WT_PRA);
      break;
    case 90: case 89:
      _presentWx |= WT_GR;
      break;
    case 88: case 87:
      _presentWx |= WT_PE;
      break;
    case 86:
      _presentWx |= WT_PSN;
      break;
    case 85:
      _presentWx |= WT_MSN;
      break;
    case 84:
      _presentWx |= (WT_PRA | WT_PSN);
      break;
    case 82:
      _presentWx |= WT_PRA;
      break;
    case 81:
      _presentWx |= WT_RA;
      break;
    case 80:
      _presentWx |= WT_MRA;
      break;
    case 79:
      _presentWx |= WT_PE;
      break;
    case 78:
      _presentWx |= WT_SN;
      break;
    case 77: case 76:
      _presentWx |= WT_MSN;
      break;
    case 74: 
      _presentWx |= WT_PSN;
      break;
    case 73: case 72:
      _presentWx |= WT_SN;
      break;
    case 71: case 70:
      _presentWx |= WT_MSN;
      break;
    case 69:
      _presentWx |= (WT_PRA | WT_PSN);
      break;
    case 68:
      _presentWx |= (WT_MRA | WT_MSN);
      break;
    case 67:
      _presentWx |= WT_PFZRA;
      break;
    case 66:
      _presentWx |= WT_MFZRA;
      break;
    case 65: case 64:
      _presentWx |= WT_PRA;
      break;
    case 63: case 62:
      _presentWx |= WT_RA;
      break;
    case 61: case 60:
      _presentWx |= WT_MRA;
      break;
    case 59: case 58:
      _presentWx |= WT_DZ;
      break;
    case 57:
      _presentWx |= WT_FZDZ;
      break;
    case 56:
      _presentWx |= WT_MFZDZ;
      break;
    case 55: case 54: case 53: case 52: case 51: case 50:
      _presentWx |= WT_DZ;
      break;
    case 49: case 48: case 47: case 46: case 45:
    case 44: case 43: case 42: case 41: case 40:
      _presentWx |= WT_FG;
      break;
    case 39: case 38: case 37: case 36:
      _presentWx |= WT_BLSN;
      break;
    case 35: case 34: case 33: case 32: case 31: case 30:
      _presentWx |= WT_DS;
      break;
    case 29:
      _presentWx |= WT_TS;
      break;
    case 28:
      _presentWx |= WT_FG;
      break;
    case 27:
      _presentWx |= WT_GR;
      break;
    case 26:
      _presentWx |= WT_SN;
      break;
    case 25:
      _presentWx |= WT_RA;
      break;
    case 24:
      _presentWx |= WT_FZRA;
      break;
    case 23:
      _presentWx |= WT_RA | WT_SN;
      break;
    case 22:
      _presentWx |= WT_SN;
      break;
    case 21:
      _presentWx |= WT_RA;
      break;
    case 20:
      _presentWx |= WT_DZ;
      break;
    case 19:
      _presentWx |= WT_FC;
      break;
    case 18:
      _presentWx |= WT_SQ;
      break;
    case 17:
      _presentWx |= WT_TS;
      break;
    case 16: case 15: case 14:
      _presentWx |= WT_UP;
      break;
    case 13:
      _presentWx |= WT_TS;
      break;
    case 12: case 11:
      _presentWx |= WT_FG;
      break;
    case 10:
      _presentWx |= WT_BR;
      break;
    case 9: case 8: case 6:
      _presentWx |= WT_DS;
      break;
    case 7:
      _presentWx |= WT_BR;
      break;
    case 5: case 4:
      _presentWx |= WT_HZ;
      break;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE && _presentWx != 0) {
    cerr << "  presentWx: " << weather_type2string(_presentWx) << endl;
  }

  return 0;

}
  
////////////////////////////////////////
// decode the sea surface temp

int SynopObsIngest::_decodeObsSst(const string &tok)

{
  
  if (tok.size() != 5) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - cannot decode sst token" << endl;
      cerr << "  tok: " << tok << endl;
    }
    return -1;
  }

  int iCode, iTemp;
  if (sscanf(tok.c_str(), "0%1d%3d", &iCode, &iTemp) != 2) {
    return 0;
  }

  double sstSign = 1.0;
  if (iCode % 2 != 0) {
    sstSign = -1.0;
  }

  _sstC = (iTemp / 10.0) * sstSign;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  sstC: " << _sstC << endl;
  }

  return 0;

}
  
/////////////////////////////////////////////////
// load up buffer from station_report_t struct
//
// Returns 0 on success, -1 on failure

int SynopObsIngest::_loadReport()

{

  _buf.clear();
  station_report_t report;
  memset(&report, 0, sizeof(station_report_t));

  report.msg_id = STATION_REPORT;
  report.time = (ui32) _obsTime;
  report.weather_type = _presentWx;
  report.lat = _latitude;
  report.lon = _longitude;
  report.alt = _heightM;
  report.temp = _tempC;
  report.dew_point = _dewPtC;
  report.relhum = _rh;
  report.windspd = _windSpeedMps * KNOTS_TO_MS;
  report.winddir = _windDirn;
  report.windgust = -9999;
  report.pres = _pressureMslHpa;
  report.liquid_accum = _precipAccumMm;
  report.visibility = _visMeters / 1000.0;
  report.ceiling = _cloudBaseMeters / 1000.0;
  report.shared.pressure_station.stn_pres = _pressureStationHpa;
  report.shared.station.Spare1 = _pressureTendHpa;
  report.shared.station.Spare2 = _sstC;

  STRncopy(report.station_label, _longName.c_str(), ST_LABEL_SIZE);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    print_station_report(stderr, "", &report);
  }

  station_report_to_be(&report);
  _buf.add(&report, sizeof(report));
  
  return 0;

}

/////////////////////////////////////////////////
// load up buffer from WxObs object
//
// Returns 0 on success, -1 on failure

int SynopObsIngest::_loadWxObs()


{

  _buf.clear();
  WxObs obs;

  obs.setStationId(_stationName);
  obs.setLongName(_longName);
  obs.setObservationTime(_obsTime);
  obs.setLatitude(_latitude);
  obs.setLongitude(_longitude);
  obs.setElevationM(_heightM);

  if (_tempC > -9990) obs.setTempC(_tempC);
  if (_sstC > -9990) obs.setSeaSurfaceTempC(_sstC);
  if (_dewPtC > -9990) obs.setDewpointC(_dewPtC);
  if (_rh > -9990) obs.setRhPercent(_rh);
  if (_windDirn > -9990) obs.setWindDirnDegT(_windDirn);
  if (_windSpeedMps > -9990) obs.setWindSpeedMps(_windSpeedMps);
  if (_visMeters > -9990) obs.setVisibilityKm(_visMeters / 1000.0);
  if (_cloudBaseMeters > -9990) obs.setCeilingKm(_cloudBaseMeters / 1000.0);
  if (_pressureMslHpa > -9990) obs.setSeaLevelPressureMb(_pressureMslHpa);
  if (_pressureStationHpa > -9990) obs.setPressureMb(_pressureStationHpa);
  if (_pressureTendHpa > -9990) {
    obs.setPressureTendencyMb(_pressureTendHpa, 10800);
  }
  if (_precipAccumMm > -9990) {
    obs.setPrecipLiquidMm(_precipAccumMm, _accumPeriodSecs);
  }
  if (_octas > -9990) {
    obs.setSkyObscuration(_octas / 8.0, _cloudBaseMeters / 1000.0);
  }
  obs.setMetarWx(weather_type2string(_presentWx));

  if (_params.output_report_type == Params::REPORT_PLUS_FULL_XML) {
    obs.assembleAsReport(REPORT_PLUS_FULL_XML);
  } else if (_params.output_report_type == Params::XML_ONLY) {
    obs.assembleAsXml();
  } else {
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    obs.print(cerr);
    if (_params.debug >= Params::DEBUG_EXTRA) {
      obs.printAsXml(cerr);
    }
  }

  _buf.add(obs.getBufPtr(), obs.getBufLen());
  
  return 0;

}

////////////////////////////////
// do put to SPDB

int SynopObsIngest::_doPut()
  
{

  int iret = 0;
  
  if (_params.write_decoded_obs) {
    if (_spdbDecoded.put(_params.decoded_output_url,
                         SPDB_STATION_REPORT_ID,
                         SPDB_STATION_REPORT_LABEL)) {
      cerr << "ERROR - SynopObsIngest::_doPut" << endl;
      cerr << "  Cannot put decoded obs to: "
           << _params.decoded_output_url << endl;
      cerr << "  " << _spdbDecoded.getErrStr() << endl;
      iret = -1;
    }
    _spdbDecoded.clearPutChunks();
  }
  
  if (_params.write_ascii_obs) {
    if (_spdbAscii.put(_params.ascii_output_url,
                       SPDB_ASCII_ID,
                       SPDB_ASCII_LABEL)) {
      cerr << "ERROR - SynopObsIngest::_doPut" << endl;
      cerr << "  Cannot put ascii obs to: "
           << _params.ascii_output_url << endl;
      cerr << "  " << _spdbAscii.getErrStr() << endl;
      iret = -1;
    }
    _spdbAscii.clearPutChunks();
  }

  return iret;

}

////////////////////////////////
// load up the station locations

int SynopObsIngest::_loadLocations(const char* station_location_path)
  
{

  FILE *fp;
  if((fp = fopen(station_location_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Metar2Spdb::loadLocations" << endl;
    cerr << "  Cannot open station location file: "
	 << station_location_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
   
  // Read in a line

  int count = 0;
  char line[BUFSIZ];
  while( fgets(line, BUFSIZ, fp) != NULL ) {

    // If the line is not a comment, process it
    
    if( line[0] == '#' ) {
      continue;
    }

    // add in missing icao identifier if needed

    if (line[0] == ' ') {
      for (int ii = 0; ii < 4; ii++) {
        line[ii] = '-';
      }
    }
    
    // replace spaces with underscores in long name
    
    bool nonSpaceFound = false;
    for (int ii = 46; ii >= 12; ii--) {
      if (line[ii] == ' ') {
        if (nonSpaceFound) {
          line[ii] = '_';
        }
      } else {
        nonSpaceFound = true;
      }
    }

    // decode the line

    char icaoName[8];
    int stationId;
    char longName[40];
    char countryCode[4];
    int ilon, ilat, iht;
    
    if (sscanf(line, "%4s %6d %35s %2s %5d %6d %5d", 
               icaoName, &stationId, longName, countryCode,
               &ilat, &ilon, &iht) != 7) {
      if (_params.debug) {
        cerr << "WARNING - Metar2Spdb::loadLocations" << endl;
        cerr << "  Cannot read line from station location file: "
             << station_location_path << endl;
        cerr << "  Line: " << line << endl;
      }
      continue;
    }
    count++;

    // create station name string

    char stationName[8];
    sprintf(stationName, "%d", stationId);
    string stationIdStr = stationName;

    // compute lat, lon, alt

    double lat = ilat / 100.0;
    double lon = ilon / 100.0;
    double htM = iht;
  
    // Create new location and add it to the map
    
    pair<int, StationLoc> pr;
    pr.first = stationId;
    pr.second.set(icaoName, stationIdStr, longName, countryCode,
                  stationId, lat, lon, htM);
    
    _locations.insert(_locations.begin(), pr);

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Line: " << line;
      pr.second.print(cerr);
    }

  }

  fclose(fp);

  if (count == 0) {
    cerr << "ERROR - Metar2Spdb::loadLocations" << endl;
    cerr << "  No suitable locations in file: : "
	 << station_location_path << endl;
    return -1;
  }

  return 0;

}

