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
// EolMobileKml2Spdb.cc
//
// EolMobileKml2Spdb object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2018
//
///////////////////////////////////////////////////////////////
//
// EolMobileKml2Spdb reads KML files containing location information
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
#include <didss/DsInputPath.hh>
#include "EolMobileKml2Spdb.hh"

using namespace std;

// Constructor

EolMobileKml2Spdb::EolMobileKml2Spdb(int argc, char **argv) {

  isOK = true;
  
  // set programe name

  _progName = "EolMobileKml2Spdb";
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

EolMobileKml2Spdb::~EolMobileKml2Spdb() {

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int EolMobileKml2Spdb::Run() {

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
      cerr << "WARNING - EolMobileKml2Spdb::Run" << endl;
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

int EolMobileKml2Spdb::_processFile(const char *inputPath) 
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
    cerr << "ERROR - EolMobileKml2Spdb::_processFile()" << endl;
    cerr << "  cannot open input file: " << inputPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // get file size

  if (inFile.fstat()) {
    int errNum = errno;
    cerr << "ERROR - EolMobileKml2Spdb::_processFile()" << endl;
    cerr << "  cannot stat input file: " << inputPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }
  struct stat fileStat = inFile.getStat(); 
  
  // read contents into buffer
  
  TaArray<char> kml_;
  char *kml = kml_.alloc(fileStat.st_size + 1);
  if (inFile.fread(kml, 1, fileStat.st_size) != fileStat.st_size) {
    int errNum = errno;
    cerr << "ERROR - EolMobileKml2Spdb::_processFile()" << endl;
    cerr << "  cannot read input file: " << inputPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }
  inFile.fclose();
  
  if (_params.debug) {
    cerr << "Successful read for file: " << inputPath << endl;
    cerr << "===========================================" << endl;
    cerr << kml << endl;
    cerr << "===========================================" << endl;
  }

  // create output object
  
  DsSpdb spdb;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    spdb.setDebug();
  }
  
  // decode KML

  if (_params.input_format == Params::MOBILE_ASSETS) {
    if (_decodeMobileAssets(kml, spdb)) {
      cerr << "ERROR - EolMobileKml2Spdb::_processFile()" << endl;
      cerr << "  Cannot decode KML" << endl;
      return -1;
    }
  } else {
    if (_decodeFlightTrack(kml, spdb)) {
      cerr << "ERROR - EolMobileKml2Spdb::_processFile()" << endl;
      cerr << "  Cannot decode KML" << endl;
      return -1;
    }
  }
  
  // write the output

  if (_doPut(spdb)) {
    return -1;
  }

  return 0;
  
}

///////////////////////////////////////
// decode the KML for mobile assets

int EolMobileKml2Spdb::_decodeMobileAssets(const char *kml, DsSpdb &spdb)
{
  
  // get the placemark buffers
  
  vector<string> placeMarks;
  if (TaXml::readStringArray(kml, "Placemark", placeMarks)) {
    cerr << "ERROR - EolMobileKml2Spdb::decodeMobileAssets" << endl;
    cerr << "  Cannot find Placemark tags" << endl;
    return -1;
  }

  // loop through the placemarks
  
  int nSuccess = 0;
  for (size_t ii = 0; ii < placeMarks.size(); ii++) {
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "=========== Placemark =============" << endl;
      cerr << placeMarks[ii] << endl;
      cerr << "===================================" << endl;
    }
    
    // get the ID
    
    string nameStr;
    if (TaXml::readString(placeMarks[ii], "name", nameStr)) {
      cerr << "WARNING - EolMobileKml2Spdb::decodeMobileAssets" << endl;
      cerr << "  Cannot find name in Placemark:" << endl;
      cerr << placeMarks[ii] << endl;
      continue;
    }
    string imeiStr(nameStr.substr(6));

    // convert to ID str

    string idStr;
    for (int jj = 0; jj < _params.id_lookups_n; jj++) {
      Params::id_lookup_t lookup = _params._id_lookups[jj];
      string imeiStr = lookup.imei_str;
      if (nameStr.find(imeiStr) != string::npos) {
        idStr = lookup.id_str;
        break;
      }
    }
    if (idStr.size() == 0) {
      cerr << "WARNING - EolMobileKml2Spdb::decodeMobileAssets" << endl;
      cerr << "  Cannot find lookup for name: " << nameStr << endl;
      continue;
    }

    // get the time
    
    string timeStr;
    if (TaXml::readString(placeMarks[ii], "when", timeStr)) {
      cerr << "WARNING - EolMobileKml2Spdb::decodeMobileAssets" << endl;
      cerr << "  Cannot find time in Placemark:" << endl;
      cerr << placeMarks[ii] << endl;
      continue;
    }
    DateTime validTime(timeStr);
    if (validTime.utime() == DateTime::NEVER) {
      cerr << "WARNING - EolMobileKml2Spdb::decodeMobileAssets" << endl;
      cerr << "  Cannot decode time string:" << timeStr << endl;
      continue;
    }

    // get coordinates

    string coordStr;
    if (TaXml::readString(placeMarks[ii], "coordinates", coordStr)) {
      cerr << "WARNING - EolMobileKml2Spdb::decodeMobileAssets" << endl;
      cerr << "  Cannot find coords in Placemark:" << endl;
      cerr << placeMarks[ii] << endl;
      continue;
    }
    double lat, lon, alt;
    if (sscanf(coordStr.c_str(), "%lg,%lg,%lg", &lon, &lat, &alt) != 3) {
      cerr << "WARNING - EolMobileKml2Spdb::decodeMobileAssets" << endl;
      cerr << "  Cannot decode coords string:" << coordStr << endl;
      continue;
    }
    
    // get heading - optional

    double heading = 0.0;
    string headingStr;
    if (TaXml::readString(placeMarks[ii], "heading", headingStr) == 0) {
      if (sscanf(headingStr.c_str(), "%lg", &heading) != 1) {
        heading = 0.0;
      }
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "==>> time: " << validTime.asString(0) << endl;
      cerr << "==>> lat, lon, alt, heading: "
           << lat << ", " << lon << ", " 
           << alt << ", " << heading << endl;
    }
    
    // trim the idStr to 4 chars
    
    if (idStr.size() > 4) {
      idStr = idStr.substr(idStr.size() - 4, 4);
    }
    
    // create the ac posn

    ac_posn_wmod_t posn;
    MEM_zero(posn);
    memcpy(posn.callsign, idStr.c_str(), AC_POSN_N_CALLSIGN);
    memcpy(posn.text, imeiStr.c_str(), AC_POSN_WMOD_N_TEXT);
    posn.lat = lat;
    posn.lon = lon;
    posn.alt = alt;
    posn.headingDeg = heading;
    posn.tas = AC_POSN_MISSING_FLOAT;
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

    // keep count of success
    
    nSuccess++;

  } //  ii

  if (nSuccess > 0) {
    return 0;
  } else {
    return -1;
  }

}

///////////////////////////////////////
// decode the KML for flight tracks

int EolMobileKml2Spdb::_decodeFlightTrack(const char *kml, DsSpdb &spdb)
{
  cerr << "ERROR - EolMobileKml2Spdb::_decodeFlightTrack()" << endl;
  cerr << "  FLIGHT_TRACK format not yet supported" << endl;
  return -1;
}

  
////////////////////////////////
// do put to SPDB

int EolMobileKml2Spdb::_doPut(DsSpdb &spdb) {

  int iret = 0;
  
  if (spdb.put(_params.output_spdb_url,
               SPDB_AC_POSN_WMOD_ID,
               SPDB_AC_POSN_WMOD_LABEL)) {
    cerr << "ERROR - EolMobileKml2Spdb::_doPut" << endl;
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

