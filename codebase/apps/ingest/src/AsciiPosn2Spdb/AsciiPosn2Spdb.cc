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
// AsciiPosn2Spdb.cc
//
// AsciiPosn2Spdb object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2018
//
///////////////////////////////////////////////////////////////
//
// AsciiPosn2Spdb reads ascii files containing location information
// for mobile assets, and and writes to SPDB as ac_posn data.
//
///////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaFile.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaStr.hh>
#include <didss/DsInputPath.hh>
#include "AsciiPosn2Spdb.hh"

using namespace std;

// Constructor

AsciiPosn2Spdb::AsciiPosn2Spdb(int argc, char **argv) {

  isOK = true;
  
  // set programe name

  _progName = "AsciiPosn2Spdb";
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

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

AsciiPosn2Spdb::~AsciiPosn2Spdb() {

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int AsciiPosn2Spdb::Run() {

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
    if (_params.file_name_check) {
      input->setSubString(_params.file_match_string);
    }
  }

  // loop through available files

  char *inputPath;
  int iret = 0;
  while ((inputPath = input->next()) != NULL) {

    if (_processFile(inputPath)) {
      cerr << "WARNING - AsciiPosn2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
      iret = -1;
    }

  } // while

  // clean up

  delete input;

  // done

  return iret;

}

////////////////////
// process the file

int AsciiPosn2Spdb::_processFile(const char *inputPath) 
{

  if (_params.debug) {
    cerr << "Processing file: " << inputPath << endl;
  }
  
  // procmap registration
  
  char procmapString[BUFSIZ];
  Path path(inputPath);
  snprintf(procmapString, BUFSIZ,
           "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);

  // Open the file
  
  TaFile inFile;
  if (inFile.fopen(inputPath, "r") == NULL) {
    int errNum = errno;
    cerr << "ERROR - AsciiPosn2Spdb::_processFile()" << endl;
    cerr << "  cannot open input file: " << inputPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // create output object
  
  DsSpdb spdb;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    spdb.setDebug();
  }

  // read file lines

  FILE *ifile = inFile.getFILE();
  char line[BUFSIZ];
  int nGood = 0;
  while (!feof(ifile)) {
    if (fgets(line, BUFSIZ, ifile) == NULL) {
      break;
    }
    if (_decodeGpsLoggerLine(line, spdb) == 0) {
      nGood++;
    }
  }
  inFile.fclose();
  
  if (_params.debug && nGood > 0) {
    cerr << "Successful read for gps logger file: " << inputPath << endl;
  }

  // write the output if good

  if (nGood > 0) {
    if (_doPut(spdb)) {
      return -1;
    }
  }

  return 0;
  
}

///////////////////////////////////////
// decode the GPS logger CSV

int AsciiPosn2Spdb::_decodeGpsLoggerLine(const char *line,
                                         DsSpdb &spdb)
{

  // is this the header line?

  if (strstr(line, "time") != NULL) {
    // header line
    return -1;
  }

  // tokenize the line
  
  vector<string> toks;
  TaStr::tokenizeAllowEmpty(line, ',', toks);
  if (toks.size() < 7) {
    cerr << "WARNING - bad line: " << line;
    return -1;
  }
  
  // get the time
  
  string timeStr(toks[0]);
  DateTime validTime(timeStr);
  if (validTime.utime() == DateTime::NEVER) {
    cerr << "WARNING - AsciiPosn2Spdb::decodeGpsLoggerLine" << endl;
    cerr << "  Cannot decode time string:" << timeStr << endl;
    return -1;
  }

  // get the lat/lon
  
  double lat = 0;
  if (sscanf(toks[1].c_str(), "%lg", &lat) != 1) {
    cerr << "WARNING - AsciiPosn2Spdb::decodeMobileAssets" << endl;
    cerr << "  Cannot decode latitude:" << toks[1] << endl;
    return -1;
  }
  double lon = 0;
  if (sscanf(toks[2].c_str(), "%lg", &lon) != 1) {
    cerr << "WARNING - AsciiPosn2Spdb::decodeMobileAssets" << endl;
    cerr << "  Cannot decode longitude:" << toks[2] << endl;
    return -1;
  }
  double altM = 0;
  if (sscanf(toks[3].c_str(), "%lg", &altM) != 1) {
    cerr << "WARNING - AsciiPosn2Spdb::decodeMobileAssets" << endl;
    cerr << "  Cannot decode altitude:" << toks[3] << endl;
    return -1;
  }

  double heading = 0;
  if (sscanf(toks[5].c_str(), "%lg", &heading) != 1) {
    heading = AC_POSN_MISSING_FLOAT;
  }
  double speed = 0;
  if (sscanf(toks[6].c_str(), "%lg", &speed) != 1) {
    speed = AC_POSN_MISSING_FLOAT;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> time: " << validTime.asString(0) << endl;
    cerr << "==>> lat, lon, alt, heading, speed: "
         << lat << ", " << lon << ", " 
         << altM << ", " << heading << ", " << speed << endl;
  }
    
  // trim the idStr to 4 chars

  string idStr(_params.platform_id_string);
  if (idStr.size() > 4) {
    idStr = idStr.substr(idStr.size() - 4, 4);
  }
    
  // create the posn
  
  ac_posn_wmod_t posn;
  MEM_zero(posn);
  memcpy(posn.callsign, idStr.c_str(), AC_POSN_N_CALLSIGN);
  posn.lat = lat;
  posn.lon = lon;
  posn.alt = altM;
  posn.headingDeg = heading;
  posn.tas = speed;
  posn.gs = AC_POSN_MISSING_FLOAT;
  posn.temp = AC_POSN_MISSING_FLOAT;
  posn.dew_pt = AC_POSN_MISSING_FLOAT;
  posn.lw = AC_POSN_MISSING_FLOAT;
  posn.fssp = AC_POSN_MISSING_FLOAT;
  posn.rosemount = AC_POSN_MISSING_FLOAT;
    
  // add chunk
  // dataType is set to hashed callsign
  // dataType2 is set to SPDB_AC_POSN_WMOD_ID
  
  si32 dataType = Spdb::hash4CharsToInt32(posn.callsign);
  BE_from_ac_posn_wmod(&posn);
  
  spdb.addPutChunk(dataType,
                   validTime.utime(),
                   validTime.utime() + _params.expire_seconds,
                   sizeof(ac_posn_wmod_t),
                   &posn,
                   SPDB_AC_POSN_WMOD_ID);
  
  return 0;

}

////////////////////////////////
// do put to SPDB

int AsciiPosn2Spdb::_doPut(DsSpdb &spdb) {

  int iret = 0;
  
  if (spdb.put(_params.output_spdb_url,
               SPDB_AC_POSN_WMOD_ID,
               SPDB_AC_POSN_WMOD_LABEL)) {
    cerr << "ERROR - AsciiPosn2Spdb::_doPut" << endl;
    cerr << "  Cannot put data to: "
         << _params.output_spdb_url << endl;
    cerr << "  " << spdb.getErrStr() << endl;
    iret = -1;
  }
  spdb.clearPutChunks();

  if (_params.debug) {
    cerr << "SUCCESS - put data to: " << _params.output_spdb_url << endl;
    cerr << "  n obs: " << spdb.getNChunks() << endl;
  }

  return iret;

}

