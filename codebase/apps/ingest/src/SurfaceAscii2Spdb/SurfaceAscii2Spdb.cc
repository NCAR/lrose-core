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
// SurfaceAscii2Spdb.cc
//
// SurfaceAscii2Spdb object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////
//
// SurfaceAscii2Spdb reads automated weather station surface
// observations, converts them to station_report_t format
// and writes them to an SPDB data base
//
///////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaStr.hh>
#include <didss/DsInputPath.hh>
#include <physics/thermo.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/WxObs.hh>
#include "SurfaceAscii2Spdb.hh"
using namespace std;

// Constructor

SurfaceAscii2Spdb::SurfaceAscii2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "SurfaceAscii2Spdb";
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

  if (_params.mode == Params::REALTIME) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  return;

}

// destructor

SurfaceAscii2Spdb::~SurfaceAscii2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SurfaceAscii2Spdb::Run ()
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
    
    if (_params.strict_subdir_check) {
      input->setStrictDirScan(true);
    }
    
    if (_params.file_name_check) {
      input->setSubString(_params.file_match_string);
    }
    
  }
  
  // loop through available files
  
  char *inputPath;
  int iret = 0;
  while ((inputPath = input->next()) != NULL) {

    if (_processFile(inputPath)) {
      cerr << "WARNING - SurfaceAscii2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
      iret = -1;
    }
    
  } // while
    
  return iret;

}

////////////////////
// process file

int SurfaceAscii2Spdb::_processFile(const char *file_path)
  
{

  int iret = 0;

  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
  }

  // registration
  
  char procmapString[BUFSIZ];
  Path path(file_path);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);
  
  // create spdb output object
  
  DsSpdb out;

  // read in file, loading up chunks in output object spdb
  
  FILE *in;
  if ((in = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SurfaceAscii2Spdb::_processAwsFile" << endl;
    cerr << "  Cannot open file: " << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  int nObs = 0;
  while (!feof(in)) {

    char line[1024];

    if (fgets(line, 1024, in) == NULL) {
      break;
    }

    if (line[0] == '#') {
      continue;
    }

    // tokenize the line
    
    vector<string> toks;
    TaStr::tokenize(line, _params.input_delimiter, toks);
    WxObs obs;
    si32 stationId = 0;
    if (_fillObs(toks, obs, stationId)) {
      cerr << "WARNING - bad line, cannot decode: " << line;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read line: " << line;
      cerr << "Decoded to the following: " << endl;
      obs.print(cerr);
    }
    
    if (_params.output_report_type == Params::STATION_REPORT) {
      obs.assembleAsReport(STATION_REPORT);
    } else if (_params.output_report_type == Params::XML) {
      obs.assembleAsXml();
    } else {
      fclose(in);
      return -1;
    }
    
    // int stationId = Spdb::hash4CharsToInt32(stid + 1);
    time_t validTime = obs.getObservationTime();
    out.addPutChunk(stationId,
                    validTime,
                    validTime + _params.expire_seconds,
                    obs.getBufLen(), obs.getBufPtr());
    
    nObs++;
    
  } // while

  // close file

  fclose(in);

  // check for sanity

  if (nObs < 0) {
    cerr << "ERROR - SurfaceAscii2Spdb::_processAwsFile" << endl;
    cerr << "  No valid obs in file: " << file_path << endl;
    return -1;
  }

  // put the data
  
  if (out.put(_params.output_url,
	      SPDB_STATION_REPORT_ID,
	      SPDB_STATION_REPORT_LABEL)) {
    cerr << "ERROR - SurfaceAscii2Spdb::_processAwsFile" << endl;
    cerr << "  Cannot put station data to: "
	 << _params.output_url << endl;
    cerr << "  " << out.getErrStr() << endl;
    iret = -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote " << nObs << " reports to url: "
	 << _params.output_url << endl;
  }

  if (_params.debug) {
    cerr << "  Done with file: " << file_path << endl;
  }

  return iret;
   
}

///////////////////////////////
// fill out obs object

int SurfaceAscii2Spdb::_fillObs(const vector<string> &toks, WxObs &obs, si32 &stationId)
  
{

  int iret = 0;

  if (toks.size() < 12) {
    cerr << "ERROR - line too short, only n tokens: " << toks.size() << endl;
    return -1;
  }

  int year = _decodeInt(toks[0]);
  int month = _decodeInt(toks[1]);
  int day = _decodeInt(toks[2]);
  int hour = _decodeInt(toks[3]);
  int min = _decodeInt(toks[4]);
  int sec = _decodeInt(toks[5]);
  DateTime dtime(year, month, day, hour, min, sec);
  obs.setObservationTime(dtime.utime());
  
  int id = _decodeInt(toks[6]);
  if (id > 0) {
    obs.setStationId(toks[6]);
    stationId = id;
  } else {
    if (toks[7].find("unknown") == string::npos) {
      obs.setStationId(toks[7]);
      stationId = Spdb::hash4CharsToInt32(toks[7].c_str());
    }
  }
  
  obs.setLatitude(_decodeDouble(toks[8]));
  obs.setLongitude(_decodeDouble(toks[9]));
  obs.setElevationM(_decodeDouble(toks[10]));

  double temp = _decodeDouble(toks[11]);
  if (temp > -9990) {
    obs.addTempC(temp);
  }

  if (toks.size() < 14) {
    return 0;
  }
  double dewp = _decodeDouble(toks[12]);
  double rh = _decodeDouble(toks[13]);

  if (dewp > -9990) {
    obs.addDewpointC(dewp);
  } else if (rh > -9990 && temp > -9990) {
    dewp = PHYrhdp(temp, rh);
    obs.addDewpointC(dewp);
  }

  if (rh > -9990) {
    obs.addRhPercent(rh);
  } else if (dewp > -9990 && temp > -9990) {
    rh = PHYrelh(temp, dewp);
    obs.addRhPercent(rh);
  }
  
  if (toks.size() < 15) {
    return 0;
  }
  double pressure = _decodeDouble(toks[14]);
  if (pressure > -9990) {
    obs.addPressureMb(pressure);
  }

  if (toks.size() < 16) {
    return 0;
  }
  double windspeed = _decodeDouble(toks[15]);
  if (windspeed > -9990) {
    obs.addWindSpeedMps(windspeed);
  }

  if (toks.size() < 17) {
    return 0;
  }
  double winddirn = _decodeDouble(toks[16]);
  if (winddirn > -9990) {
    obs.addWindDirnDegT(winddirn);
  }

  if (toks.size() < 18) {
    return 0;
  }
  double windgust = _decodeDouble(toks[17]);
  if (windgust > -9990) {
    obs.addWindGustMps(windgust);
  }

  if (toks.size() < 19) {
    return 0;
  }
  double precipaccum = _decodeDouble(toks[18]);
  if (precipaccum > -9990) {
    obs.addPrecipLiquidMm(precipaccum, 3600);
  }

  if (toks.size() < 20) {
    return 0;
  }
  double preciprate = _decodeDouble(toks[19]);
  if (preciprate > -9990) {
    obs.setPrecipRateMmPerHr(preciprate);
  }

  if (toks.size() < 21) {
    return 0;
  }
  double vis = _decodeDouble(toks[20]);
  if (vis > -9990) {
    obs.addVisibilityKm(vis);
  }

  if (toks.size() < 22) {
    return 0;
  }
  double rvr = _decodeDouble(toks[21]);
  if (rvr > -9990) {
    obs.addRvrKm(rvr);
  }

  if (toks.size() < 23) {
    return 0;
  }
  double ceiling = _decodeDouble(toks[22]);
  if (ceiling > -9990) {
    obs.addCeilingKm(ceiling);
  }

  return iret;
   
}

////////////////////////
// decode token values

int SurfaceAscii2Spdb::_decodeInt(const string &tok)

{
  int val;
  if (sscanf(tok.c_str(), "%d", &val) != 1) {
    return -9999;
  }
  return val;
}

double SurfaceAscii2Spdb::_decodeDouble(const string &tok)
{
  
  double val;
  if (sscanf(tok.c_str(), "%lg", &val) != 1) {
    return -9999.0;
  }
  return val;

}
