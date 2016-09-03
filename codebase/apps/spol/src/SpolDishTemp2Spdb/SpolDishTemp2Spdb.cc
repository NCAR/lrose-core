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
// SpolDishTemp2Spdb.cc
//
// SpolDishTemp2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2016
//
///////////////////////////////////////////////////////////////
//
// SpolDishTemp2Spdb reads dish temp records from ASCII files,
// converts them to station_report_t format and
// writes them to an SPDB data base.
//
///////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaStr.hh>
#include <didss/DsInputPath.hh>
#include "SpolDishTemp2Spdb.hh"
using namespace std;

// Constructor

SpolDishTemp2Spdb::SpolDishTemp2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "SpolDishTemp2Spdb";
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

SpolDishTemp2Spdb::~SpolDishTemp2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SpolDishTemp2Spdb::Run ()
{

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
  while ((inputPath = input->next()) != NULL) {
    if (_processFile(inputPath)) {
      cerr << "WARNING - SpolDishTemp2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }
  } // while

  delete input;
    
  return 0;

}

////////////////////
// process the file

int SpolDishTemp2Spdb::_processFile(const char *file_path)
  
{
  
  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
  }

  // registration

  char procmapString[BUFSIZ];
  Path path(file_path);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);

  // Open the file

  FILE *fp;
  if((fp = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SpolDishTemp2Spdb::_processFile" << endl;
    cerr << "  Cannot open metar file: "
	 << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
   
  char line[BUFSIZ];
  int iret = 0;
  int idIndex = -1;
  int timeIndex = -1;
  int valIndex = -1;

  // read in a line at a time

  while(fgets(line, BUFSIZ, fp) != NULL) {

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << line;
    }

    // tokenize the line
    
    vector<string> toks;
    TaStr::tokenize(line, "\",\n\r", toks);
    if (toks.size() < 4) {
      break;
    }

    // set the index for each field

    bool isLabelLine = false;
    for (size_t ii = 0; ii < toks.size(); ii++) {
      if (toks[ii] == "SensorID") {
        idIndex = ii;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "idIndex: " << idIndex << endl;
        }
        isLabelLine = true;
      }
      if (toks[ii] == "Date") {
        timeIndex = ii;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "timeIndex: " << timeIndex << endl;
        }
        isLabelLine = true;
      }
      if (toks[ii] == "Value") {
        valIndex = ii;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "valIndex: " << valIndex << endl;
        }
        isLabelLine = true;
      }
    }
    if (isLabelLine) {
      continue;
    }

    // decode the tokens

    int stationId;
    if (sscanf(toks[idIndex].c_str(), "%d", &stationId) != 1) {
      cerr << "ERROR - cannot decode ID: " << toks[idIndex] << endl;
      continue;
    }

    double tempValue;
    if (sscanf(toks[valIndex].c_str(), "%lg", &tempValue) != 1) {
      cerr << "ERROR - cannot decode tempValue: " << toks[valIndex] << endl;
      continue;
    }

    int year, month, day, hour, min, sec;
    char ampm[16];
    if (sscanf(toks[timeIndex].c_str(), "%d/%d/%d %d:%d:%d %s",
               &month, &day, &year, &hour, &min, &sec, ampm) != 7) {
      cerr << "ERROR - cannot decode datetime: " << toks[timeIndex] << endl;
      continue;
    }
    if (strcmp(ampm, "PM") == 0) {
      hour += 12.0;
    }
    DateTime dtime(year, month, day, hour, min, sec);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "time, stationId, tempValue: " 
           << dtime.asString() << ", " << stationId << ", " << tempValue << endl;
    }

    // create weather obs

    WxObs obs;
    obs.setStationId("SPOL");
    obs.setLatitude(_params.radar_latitude_deg);
    obs.setLongitude(_params.radar_longitude_deg);
    obs.setElevationM(_params.radar_altitude_meters);
    obs.setObservationTime(dtime.utime());
    obs.addTempC(tempValue);
    obs.assembleAsXml();
    
    cerr <<(char *) obs.getBufPtr() << endl;

    // put to SPDB
    
    DsSpdb spdb;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      spdb.setDebug();
    }
    if (_params.debug) {
      cerr << "Writing to output url: " << _params.output_url << endl;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      obs.print(cerr);
    }

    si32 dataType = Spdb::hash4CharsToInt32("SPOL");

    if (spdb.put(_params.output_url,
                 SPDB_STATION_REPORT_ID,
                 SPDB_STATION_REPORT_LABEL,
                 dataType,
                 dtime.utime(),
                 dtime.utime() + _params.expire_seconds,
                 obs.getBufLen(), obs.getBufPtr())) {
      cerr << "ERROR - SpolDishTemp2Spdb::_doPut" << endl;
      cerr << "  Cannot put temperature data to: "
           << _params.output_url << endl;
      cerr << "  " << spdb.getErrStr() << endl;
      iret = -1;
    }

  } // while (fgets ...
  
  fclose(fp);

  return iret;
   
}


