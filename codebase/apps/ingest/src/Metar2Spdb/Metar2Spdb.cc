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
// Metar2Spdb.cc
//
// Metar2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2001
//
///////////////////////////////////////////////////////////////
//
// Metar2Spdb reads ASCII files containing METAR data, converts the
// data to station_report_t, and writes to SPDB.
//
///////////////////////////////////////////////////////////////////////

#include <cerrno>
#include<fstream>
#include<sstream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <didss/DsInputPath.hh>
#include "Metar2Spdb.hh"

using namespace std;

// Constructor

Metar2Spdb::Metar2Spdb(int argc, char **argv) {

  isOK = true;

  // set programe name

  _progName = "Metar2Spdb";
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

  // load up the station locations

  if (_loadLocations(_params.st_location_path)) {
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

Metar2Spdb::~Metar2Spdb() {

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Metar2Spdb::Run() {

  // register with procmap

  PMU_auto_register("Run");

  // file input object

  DsInputPath *input = NULL; // Set to NULL to get around compiler warnings.

  if (_params.mode == Params::FILELIST) {

    // FILELIST mode

    input = new DsInputPath(_progName,
                            _params.debug >= Params::DEBUG_VERBOSE,
                            _args.inputFileList);

  } else if (_params.mode == Params::ARCHIVE) {

    // archive mode - start time to end time

    input = new DsInputPath(_progName,
                            _params.debug >= Params::DEBUG_VERBOSE,
                            _params.input_dir,
                            _args.startTime, _args.endTime);

  } else if (_params.mode == Params::REALTIME) {

    // realtime mode - no latest_data_info file

    input = new DsInputPath(_progName,
                            _params.debug >= Params::DEBUG_VERBOSE,
                            _params.input_dir,
                            _params.max_realtime_valid_age,
                            PMU_auto_register,
                            _params.latest_data_info_avail,
                            true);

    if (_params.strict_subdir_check) {
      input->setStrictDirScan(true);
    }

    if (_params.file_name_check) {
      input->setSubString(_params.file_match_string);
    }

  }

  // loop through available files

  char *inputPath;
  while ((inputPath = input->next()) != NULL) {

    time_t inputTime;
    input->getDataTime(inputPath, inputTime);

    if (_processFile(inputPath, inputTime)) {
      cerr << "WARNING - Metar2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }

  } // while

  delete input;

  return 0;

}

////////////////////
// process the file

int Metar2Spdb::_processFile(const char *file_path,
                             time_t file_time) {

  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
    cerr << "  File time: " << utimstr(file_time) << endl;
  }

  // registration

  char procmapString[BUFSIZ];
  Path path(file_path);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);

  // create output objects

  DsSpdb spdbDecoded;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    spdbDecoded.setDebug();
  }
  if (_params.decoded_compression == Params::COMPRESSION_GZIP) {
    spdbDecoded.setChunkCompressOnPut(Spdb::COMPRESSION_GZIP);
  } else if (_params.decoded_compression == Params::COMPRESSION_BZIP2) {
    spdbDecoded.setChunkCompressOnPut(Spdb::COMPRESSION_BZIP2);
  }

  DsSpdb spdbAscii;

  // Open the file

  ifstream fp(file_path);

  if( ! fp.is_open() ){
    cerr << "ERROR - Metar2Spdb::_processFile\n";
    cerr << "  Cannot open metar file: " << file_path << endl;
    return -1;
  }

  //
  // Concatenate the lines in the file until we reach a blank line,
  // or until the line added contains an '='.
  // This means that we have reached the end of a metar message, so
  // create a metar object and store this information.  Then add this
  // metar object to our list of metars.
  //

  // we have to handle METAR's in two forms
  // (a) A lines starts with METAR and is followed by the data record
  // (b) A line with only the word METAR in it signifies the beginning of a 
  //     block of metars, which is terminated by a blank line or ^C.
  // We also have to deal with SPECIs, in the same way as METARs.

  string metarMessage = "";
  string reportType = "";
  bool inBlock = false;
  int blockHour = -1;
  int blockMin = -1;
  int blockDate = -1;

  char line[BUFSIZ];
  time_t curr_valid_time = -1;

  int iret = 0;

  while(!fp.eof()){

    fp.getline(line,BUFSIZ);

    // if start of SA Block, start block metar

    if (_startOfSABlock(line, blockHour, blockMin, blockDate)) {
      metarMessage = "";
      inBlock = true;
      continue;
    }

    // if METAR or SPECI is present, start new metar

    char *startChar = line;

    istringstream iss(line);
    string firstWord;
    iss >> firstWord;

    if (firstWord == "METAR" || firstWord == "SPECI") {

      // Assune type of METAR
      reportType = "METAR";

      if (firstWord ==  "SPECI") {
        reportType = "SPECI";
      }

      metarMessage = "";
      inBlock = true;

      if (strlen(line) < 10) {
        // only METAR or SPECI on line, must be start of block
        continue;
      }

      //
      // Jump ahead of METAR or SPECI prefix, if it exists
      //

      if (strlen(line) > 7 && !strncmp(line, " METAR ", 7)) {
        startChar = line + 7;
      } else if (strlen(line) > 6 && !strncmp(line, "METAR ", 6)) {
        startChar = line + 6;
      } else if (strlen(line) > 7 && !strncmp(line, " SPECI ", 7)) {
        startChar = line + 7;
        reportType = "SPECI";
      } else if (strlen(line) > 6 && !strncmp(line, "SPECI ", 6)) {
        startChar = line + 6;
        reportType = "SPECI";
      } else {
        startChar = line;
      }

    }

    // if we are not in a METAR block, continue

    if (!inBlock) {
      continue;
    }

    // add line to metar string if not blank

    if (!STRequal(startChar, "\n")) {
      metarMessage += startChar;
    }

    // check if complete

    bool complete = false;
    if (STRequal(startChar, "\n") || strchr(startChar, '=')) {
      complete = true;
    }

    // check for end of block - blank line or Ctrl-C

    if (strstr(startChar, "NNNN") || strchr(startChar, 0x03)) {
      inBlock = false;
      blockHour = -1;
      blockMin = -1;
    }

    if (complete) {

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << endl;
        cerr << "Processing metar message: " << metarMessage << endl;
        cerr << endl;
    }

      MemBuf buf;
      time_t valid_time;
      string stationName;

      if (_decodeMetar(file_path, file_time, blockHour, blockMin, blockDate,
                       metarMessage, reportType, stationName, buf, valid_time) == 0) {

        if (_params.force_put_when_valid_time_changes) {

          if (curr_valid_time != valid_time) {
            if (curr_valid_time > 0) {
              if (_doPut(spdbDecoded, spdbAscii)) {
                iret = -1;
              }
            }
            curr_valid_time = valid_time;
          }

        } // if (_params.force_put_when_valid_time_changes)


        // add chunks to spdb objects

        int stationId = Spdb::hash4CharsToInt32(stationName.c_str());

        if (_params.write_decoded_metars) {
          spdbDecoded.addPutChunk(stationId,
                                  valid_time,
                                  valid_time + _params.expire_seconds,
                                  buf.getLen(), buf.getPtr());
        }

        if (_params.write_ascii_metars) {

          if (_params.dress_raw_metar_text) {
            metarMessage = reportType + " " + metarMessage;
          }

          spdbAscii.addPutChunk(stationId,
                                valid_time,
                                valid_time + _params.expire_seconds,
                                metarMessage.size() + 1,
                                metarMessage.c_str());
        }
      } /* endif - _decodeMetar(...) == 0) */

      metarMessage = "";
    }

  } // while (fgets ...

  fp.close();

  // write the output

  if (_doPut(spdbDecoded, spdbAscii)) {
    iret = -1;
  }

  return iret;

}

//////////////////////////////
// check for start of SA block
//
// Typical blocks:
//
// SAAW31 KWBC 171000 RRD (GTS)
// ^BSAXX99 EBBR 172100 (AFTN)

bool Metar2Spdb::_startOfSABlock(const char *line,
                                 int &blockHour,
                                 int &blockMin,
                                 int &blockDate) {

  // check for SA weather group - not start of SA group

  if (strstr(line, " SA ")) {
    return false;
  }

  // check for SA characters close to start of line

  const char *sa = strstr(line, "SA");
  if (sa == NULL) {
    return false;
  }

  // SA should not be preceded by an alpha-numeric

  if (sa != line) {
    if (isalpha(sa[-1])) {
      return false;
    }
    if (sa - line > 4) {
      return false;
    }
  }

  // SA should not be followed by a space

  if (sa[2] == ' ') {
    return false;
  }

  if (strlen(sa) < 6) {
    return false;
  }

  if (strlen(sa) > 128) {
    return false;
  }

  // check for "SA* stid ddhhmm"

  char sagroup[128];
  char stid[128];
  char timestr[128];

  if (sscanf(sa, "%s %s %s", sagroup, stid, timestr) != 3) {
    return false;
  }

  if (strlen(sagroup) < 3) {
    return false;
  }

  if (strlen(stid) != 4) {
    return false;
  }

  int day, hour, min;
  if (sscanf(timestr, "%2d%2d%2d", &day, &hour, &min) != 3) {
    return false;
  }
  blockHour = hour;
  blockMin = min;
  blockDate = day;
  return true;

}

///////////////////
// decode the metar

int Metar2Spdb::_decodeMetar(const char *file_path,
                             time_t file_time,
                             int blockHour,
                             int blockMin,
                             int blockDate,
                             const string &metarMessage,
                             const string &reportType,
                             string &stationName,
                             MemBuf &buf,
                             time_t &valid_time) {

  // Ignore blank messages - need at least 4 chars for station name

  if (metarMessage.size() < 4) {
    return -1;
  }

  // check if in the main list

  stationName = metarMessage.substr(0, 4);
  map<string, StationLoc, less<string> >::iterator iloc;
  iloc = _locations.find(stationName);
  if (iloc == _locations.end()) {
    if(_params.debug >= Params::DEBUG_VERBOSE){
      cerr << "Station " << stationName << " not fond in list...skipping.\n";
    }
    return -1;
  }
  StationLoc &stationLoc = iloc->second;

  // should we check acceptedStations?

  if (_params.useAcceptedStationsList) {
    bool accept = false;
    for (int ii = 0; ii < _params.acceptedStations_n; ii++) {
      if (stationName == _params._acceptedStations[ii]) {
        accept = true;
        break;
      }
    }
    if (!accept) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << endl;
        cerr << "Rejecting station: " << stationName << endl;
        cerr << "  Not in acceptedStations list" << endl;
      }
      return -1;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << endl;
      cerr << "Conditionally accepting station: " << stationName << endl;
      cerr << "  Is in acceptedStations list" << endl;
    }
  }

  // should we check rejectedStations?

  if (_params.useRejectedStationsList) {
    bool reject = false;
    for (int ii = 0; ii < _params.rejectedStations_n; ii++) {
      if (stationName == _params._rejectedStations[ii]) {
        reject = true;
        break;
      }
    }
    if (reject) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << endl;
        cerr << "Rejecting station: " << stationName << endl;
        cerr << "  Station name is in rejectedStations list" << endl;
      }
      return -1;
    }
  }

  // should we check bounding box?

  if (_params.checkBoundingBox) {
    if (stationLoc.lat < _params.boundingBox.min_lat ||
        stationLoc.lat > _params.boundingBox.max_lat ||
        stationLoc.lon < _params.boundingBox.min_lon ||
        stationLoc.lon > _params.boundingBox.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << endl;
        cerr << "Rejecting station: " << stationName << endl;
        cerr << "  Station position not within bounding box" << endl;
      }
      return -1;
    }
  }

  ////////////////////////////////////////////////////////////////
  // Decode the METAR
  // this uses strtok() which corrupts the message, so make a copy

  TaArray<char> messageCopy_;
  char *messageCopy = messageCopy_.alloc(metarMessage.size() + 1);
  strcpy(messageCopy, metarMessage.c_str());

  Decoded_METAR dcdMetar;
  if (DcdMETAR(messageCopy, &dcdMetar, true) != 0) {
    return -1;
  }

  ////////////////////////////////////////////////////////////////
  // set valid time

  int ob_hour, ob_minute, ob_date;
  if (dcdMetar.ob_hour != MAXINT && dcdMetar.ob_minute != MAXINT) {
    ob_hour = dcdMetar.ob_hour;
    ob_minute = dcdMetar.ob_minute;
  } else {
    ob_hour = blockHour;
    ob_minute = blockMin;
  }

  if (dcdMetar.ob_date != MAXINT)
    ob_date = dcdMetar.ob_date;
  else
    ob_date = blockDate;

  if (ob_hour == -1 || ob_minute == -1) {
    // time not set
    if (_params.mode == Params::REALTIME && _params.guess_time_if_missing) {
      // set time to current hour
      time_t now = time(NULL);
      time_t now_hour = (now / 3600) * 3600;
      valid_time = now_hour;
    } else {
      // time missing - no good
      return -1;
    }
  } else {
    // Set time - hours and minutes are provided by the decoded
    // metar struct; seconds are zeroed out
    date_time_t tvalid;
    tvalid.unix_time = file_time;
    uconvert_from_utime(&tvalid);
    if (_params.use_metar_date)
      tvalid.day = ob_date;
    tvalid.hour = ob_hour;
    tvalid.min = ob_minute;
    tvalid.sec = 0;
    uconvert_to_utime(&tvalid);
    valid_time = tvalid.unix_time;
    // correct for time in the future if hour and minute is from
    // previous day
    if (_params.mode == Params::REALTIME && !_params.use_metar_date) {
      time_t now = time(NULL);
      if (valid_time > (now + 43200)) {
        valid_time -= 86400;
      }
    }
  }

  //check if the metar should be rejected because it is from the future or past
  if (_params.use_metar_date) {
    time_t now = time(NULL);
    if ((_params.valid_past_margin > 0) &&
        ((now - valid_time) > _params.valid_past_margin)) {
      cerr << "Rejecting METAR from the past:" << metarMessage
           << "\nvalid time: " << valid_time
           << " now: " << now << " valid_past_margin: "
           << _params.valid_past_margin << endl;

      return -1;
    }
    if ((_params.valid_future_margin > 0) &&
        ((valid_time - now) > _params.valid_future_margin)) {
      cerr << "Rejecting METAR from the future:" << metarMessage
           << "\nvalid time: " << valid_time
           << " now: " << now << " valid_future_margin: "
           << _params.valid_future_margin << endl;

      return -1;

    }
  }


  switch (_params.output_report_type) {

    case Params::METAR_REPORT:
    case Params::METAR_WITH_REMARKS_REPORT:
    case Params::STATION_REPORT:
    case Params::PRESSURE_STATION_REPORT: {
      if (_loadReport(dcdMetar, valid_time,
                      stationLoc.lat, stationLoc.lon, stationLoc.alt,
                      buf) != 0) {
        return -1;
      }
      break;
    }

    case Params::REPORT_PLUS_METAR_XML:
    case Params::REPORT_PLUS_FULL_XML:
    case Params::XML_ONLY: {
      if (_loadWxObs(metarMessage, reportType, stationName, dcdMetar, valid_time,
                     stationLoc.lat, stationLoc.lon, stationLoc.alt,
                     buf) != 0) {
        return -1;
      }
      break;
    }

    default: {
      return -1;
    }

  } // switch

  return 0;

}

/////////////////////////////////////////////////
// load up buffer from station_report_t struct
//
// Returns 0 on success, -1 on failure

int Metar2Spdb::_loadReport(const Decoded_METAR &dcdMetar,
                            time_t valid_time,
                            double lat,
                            double lon,
                            double alt,
                            MemBuf &buf) {

  station_report_t report;
  memset(&report, 0, sizeof(station_report_t));

  if (_params.output_report_type == Params::METAR_REPORT) {
    if (decoded_metar_to_report(&dcdMetar, &report,
                                valid_time, lat, lon, alt)) {
      return -1;
    }
  } else if (_params.output_report_type == Params::STATION_REPORT) {
    if (decoded_metar_to_station_report(&dcdMetar, &report,
                                        valid_time, lat, lon, alt)) {
      return -1;
    }
  } else if (_params.output_report_type == Params::PRESSURE_STATION_REPORT) {
    if (decoded_metar_to_pressure_station_report(&dcdMetar, &report,
                                                 valid_time,
                                                 lat, lon, alt)) {
      return -1;
    }
  } else if (_params.output_report_type == Params::METAR_WITH_REMARKS_REPORT) {
    if (decoded_metar_to_report_with_remarks(&dcdMetar, &report,
                                             valid_time,
                                             lat, lon, alt)) {
      return -1;
    }

  } else {
    return -1;
  }


  if (_params.debug >= Params::DEBUG_VERBOSE) {
    print_station_report(stderr, "", &report);
  }
  station_report_to_be(&report);
  buf.add(&report, sizeof(report));

  return 0;

}

/////////////////////////////////////////////////
// load up buffer from WxObs object
//
// Returns 0 on success, -1 on failure

int Metar2Spdb::_loadWxObs(const string &metarText,
                           const string &reportType,
                           const string &stationName,
                           const Decoded_METAR &dcdMetar,
                           time_t valid_time,
                           double lat,
                           double lon,
                           double alt,
                           MemBuf &buf) {

  WxObs obs;
  if (obs.setFromDecodedMetar(metarText, stationName, dcdMetar, valid_time,
                              lat, lon, alt)) {
    return -1;
  }

  // Dress the raw text with the report type and ending character (=)

  if (_params.dress_raw_metar_text) {
    obs.dressRawMetarText(reportType);
  }

  if (_params.output_report_type == Params::REPORT_PLUS_METAR_XML) {
    obs.assembleAsReport(REPORT_PLUS_METAR_XML);
  } else if (_params.output_report_type == Params::REPORT_PLUS_FULL_XML) {
    obs.assembleAsReport(REPORT_PLUS_FULL_XML);
  } else if (_params.output_report_type == Params::XML_ONLY) {
    obs.assembleAsXml();
  } else {
    return -1;
  }

  buf.add(obs.getBufPtr(), obs.getBufLen());

  return 0;

}

////////////////////////////////
// load up the station locations

int Metar2Spdb::_loadLocations(const char *station_location_path) {

  FILE *fp;
  char line[BUFSIZ];
  char station[128];
  double lat, lon, alt;

  string stationId;

  if ((fp = fopen(station_location_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Metar2Spdb::loadLocations" << endl;
    cerr << "  Cannot open station location file: "
         << station_location_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  int count = 0;

  while (fgets(line, BUFSIZ, fp) != NULL) {

    // If the line is not a comment, process it

    if (line[0] == '#' || line[0] == '!' || strlen(line) < 4) {
      continue;
    }

    // Read in the line - try different formats

    if (sscanf(line, "%4s, %lg, %lg, %lg",
               station, &lat, &lon, &alt) != 4) {
      if (sscanf(line, "%4s,%lg,%lg,%lg",
                 station, &lat, &lon, &alt) != 4) {
        if (sscanf(line, "%3s,%lg,%lg,%lg",
                   station, &lat, &lon, &alt) != 4) {
          if (_readThompsonLocation(line, station, lat, lon, alt)) {
            if (_params.debug >= Params::DEBUG_VERBOSE) {
              cerr << "WARNING - Metar2Spdb::loadLocations" << endl;
              cerr << "  Cannot read line from station location file: "
                   << station_location_path << endl;
              cerr << "  Line: " << line << endl;
            }
            continue;
          }
        }
      }
    }

    count++;

    // Convert station to a string

    stationId = station;

    // Convert altitude to meters

    if (alt == -999.0 ||
        alt == 999.0) {
      alt = STATION_NAN;
    } else {
      switch (_params.altUnits) {
        case Params::ALT_FEET :
          alt *= M_PER_FT;
          break;
        case Params::ALT_METERS :
          // do nothing
          break;
      }
    }

    // Create new metar location and add it to the map

    pair<string, StationLoc> pr;
    pr.first = stationId;
    pr.second.set(stationId, lat, lon, alt);
    _locations.insert(_locations.begin(), pr);

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

////////////////////////////////////////////////
// decode a line from Grep Thompson's type file

#ifdef EXAMPLE_LINES
AK ADAK NAS         PADK  ADK   70454  51 53N  176 39W    4   X     T          7 US
CO DENVER (DIA)     KDEN  DEN   72565  39 51N  104 39W 1640   X     U     A    0 US
#endif

int Metar2Spdb::_readThompsonLocation(const char *line,
                                      char *station,
                                      double &lat,
                                      double &lon,
                                      double &alt) {

  if (strlen(line) < 60) {
    return -1;
  }

  // station name

  strncpy(station, line + 20, 4);

  // latitude

  char latStr[16];
  strncpy(latStr, line + 39, 5);
  int latDeg, latMin;
  if (sscanf(latStr, "%d %d", &latDeg, &latMin) != 2) {
    return -1;
  }
  lat = (double) latDeg + (double) latMin / 60.0;
  if (line[44] == 'S') {
    lat *= -1.0;
  }

  // longitude

  char lonStr[16];
  strncpy(lonStr, line + 47, 6);
  int lonDeg, lonMin;
  if (sscanf(lonStr, "%d %d", &lonDeg, &lonMin) != 2) {
    return -1;
  }
  lon = (double) lonDeg + (double) lonMin / 60.0;
  if (line[53] == 'W') {
    lon *= -1.0;
  }

  // altitude

  char altStr[16];
  strncpy(altStr, line + 55, 4);
  int altM;
  if (sscanf(altStr, "%d", &altM) != 1) {
    return -1;
  }
  alt = (double) altM;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=============================================" << endl;
    cerr << "Reading data from Greg Thompson station file" << endl;
    cerr << "  Line: " << line << endl;
    cerr << "  station, lat, lon, alt: " << station << ", " << lat << ", " << lon << ", " << alt << endl;
    cerr << "=============================================" << endl;
  }

  return 0;

}



////////////////////////////////
// do put to SPDB

int Metar2Spdb::_doPut(DsSpdb &spdbDecoded, DsSpdb &spdbAscii) {

  int iret = 0;

  if (_params.write_decoded_metars) {
    if (spdbDecoded.put(_params.decoded_output_url,
                        SPDB_STATION_REPORT_ID,
                        SPDB_STATION_REPORT_LABEL)) {
      cerr << "ERROR - Metar2Spdb::_doPut" << endl;
      cerr << "  Cannot put decoded metars to: "
           << _params.decoded_output_url << endl;
      cerr << "  " << spdbDecoded.getErrStr() << endl;
      iret = -1;
    }
    spdbDecoded.clearPutChunks();
  }

  if (_params.write_ascii_metars) {
    if (spdbAscii.put(_params.ascii_output_url,
                      SPDB_ASCII_ID,
                      SPDB_ASCII_LABEL)) {
      cerr << "ERROR - Metar2Spdb::_doPut" << endl;
      cerr << "  Cannot put ascii metars to: "
           << _params.ascii_output_url << endl;
      cerr << "  " << spdbAscii.getErrStr() << endl;
      iret = -1;
    }
    spdbAscii.clearPutChunks();
  }

  return iret;

}

