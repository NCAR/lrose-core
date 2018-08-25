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
// Taf2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
///////////////////////////////////////////////////////////////

#include <dsserver/DmapAccess.hh>
#include <didss/DsURL.hh>

#include <string>
#include <vector>
#include <cerrno>
#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <Spdb/DsSpdb.hh>
#include "Taf2Spdb.hh"
#include "Input.hh"
#include "Location.hh"
using namespace std;

// Constructor

Taf2Spdb::Taf2Spdb(int argc, char **argv)

{

  isOK = true;
  _input = NULL;
  _inputPath = NULL;

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

  // input path object

  if (_params.mode == Params::ARCHIVE) {

    if (_args.startTime != 0 && _args.endTime != 0) {
      
      _inputPath = new DsInputPath((char *) _progName.c_str(),
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _params.input_dir_path,
			       _args.startTime,
			       _args.endTime);
      
    } else {
      
      cerr << "ERROR: " << _progName << endl;
      cerr << "In ARCHIVE mode, you must set start and end times." << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;

    }

  } else {

    _inputPath = new DsInputPath((char *) _progName.c_str(),
				 _params.debug >= Params::DEBUG_VERBOSE,
				 _params.input_dir_path,
				 _params.max_realtime_valid_age,
				 PMU_auto_register);
    
  }

  // read in station locations

  if(_params.site_filter) {
    if (_stationLoc.ReadData(_params.st_location_path)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Cannot load locations, file : "
	   << _params.st_location_path << endl;
      isOK = false;
      return;
    }
  }
  
  // input data object

  switch(_params.taf_format) {
  case Params::GTS:
    _input = new Input(_progName, _params);
    break;
  case Params::NNNN:
    _input = new NnnnInput(_progName, _params);
    break;
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

  if (_input) {
    delete _input;
  }
  if (_inputPath) {
    delete _inputPath;
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
    time_t inputFileTime = _inputPath->getDataTime(inputFilePath);
    _processFile(inputFilePath, inputFileTime);
  } // while
  
  return (0);

}

//////////////////////////////////////////////////
// process file

int Taf2Spdb::_processFile (const char *file_path, 
			       time_t input_file_time)
  
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
  }

  DsSpdb spdb;
  spdb.setAppName(_progName);
  string tafStr;
  int ntotal = 0;
  int nstored = 0;
  
  while (_input->readNext(tafStr) == 0) {
    
    PMU_auto_register(procmap_string);
    
    if (tafStr.size() == 0) {
      continue;
    }
    
    ntotal++;

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
    
    // tokenize TAF
    
    vector<string> toks;
    size_t pos = 0;
    bool done = false;
    while (!done) {
      size_t start = tafStr.find_first_not_of(" \n\t\r", pos);
      size_t end = tafStr.find_first_of(" \n\t\r", start);
      if (start == string::npos) {
	done = true;
      } else {
	string tok;
	tok.assign(tafStr, start, end - start);
	toks.push_back(tok);
      }
      pos = end;
    }

    // find station name - this will be the first 4-letter token

    string name = "";
    for (size_t ii = 0; ii < toks.size(); ii++) {
      if (toks[ii].size() == 4) {
	bool all_alpha = true;
	const char *c = toks[ii].c_str();
	for (int i = 0; i < 4; i++) {
	  if (!isalpha(c[i])) {
	    all_alpha = false;
	    break;
	  }
	}
	if (all_alpha) {
	  name = toks[ii];
	  break;
	}
      }
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "===============================" << endl;
      cerr << tafStr << endl;
      for(size_t ii = 0; ii < toks.size(); ii++) {
	cerr << "token " << ii << ": " << toks[ii] << endl;
      }
      cerr << "  name: " << name << endl;
      cerr << "===============================" << endl;
    }

    // check if we want this taf
    
    bool storeIt = true;
    if (_params.site_filter) {
      double lat, lon, alt;
      string type;
      if (_stationLoc.FindPosition(name, lat, lon, alt, type)) {
	storeIt = false;
      }
    }
    
    if (storeIt) {
      nstored++;
      // add to spdb
      string chunk;
      if (_params.store_header) {
	chunk += _input->getHeadStr();
      }
      chunk += tafStr;
      _addTafChunk(chunk, name, input_file_time, spdb);
    } else {
      if (_params.debug) {
	cerr << "Site rejected: " << name << endl;
      }
    }
    
  }
  
  // close file
  
  _input->close();

  // debug print

  if (_params.debug ||
      (_params.debug && nstored > 0)) {
    cerr << "Done with file: " << file_path << endl;
    cerr << "  N total tafs found: " << ntotal << endl;
    cerr << "  N tafs stored: " << nstored << endl;
  }

  // Write

  if (_writeTafs2Spdb(spdb)) {
    return -1;
  } else {
    return 0;
  }
    
}

/////////////////////////////////
// add TAF chunk to spdb object

void Taf2Spdb::_addTafChunk(const string &tafStr,
			    const string &name,
			    time_t file_time,
			    DsSpdb &spdb)
  
{
  
  if (tafStr.size() == 0) {
    if (_params.debug) {
      cerr << "WARNING - Taf2Spdb::_addTafChunk" << endl;
      cerr << "  Zero length TAF" << endl;
      cerr << "  Valid time: " << DateTime::str(file_time) << endl;
      cerr << "  Name: " << name << endl;
    }
    return;
  }

  // tokenize the string

  vector<string> toks;
  _tokenize(tafStr, " \n\r", toks);

  // look through the tokens for a time string, set the valid time

  time_t valid_time = file_time;
  for (size_t ii = 0; ii < toks.size(); ii++) {

    if (toks[ii].size() != 7 ||	toks[ii][6] != 'Z') {
      continue;
    }
    int day, hour, minute;
    if ((sscanf(toks[ii].c_str(), "%2d%2d%2d",
		&day, &hour, &minute)) != 3) {
      continue;
    }

    date_time_t gtime;
    gtime.unix_time = file_time;
    uconvert_from_utime(&gtime);
    gtime.hour = hour;
    gtime.min = minute;
    gtime.sec = 0;
    uconvert_to_utime(&gtime);
    valid_time = gtime.unix_time;
    
    // correct for time in the future if hour and minute is from
    // previous day
    
    if (_params.mode == Params::REALTIME) {
      time_t now = time(NULL);
      if (valid_time > (now + 43200)) {
	valid_time -= 86400;
      }
    } else {
      if (valid_time > (file_time + 43200)) {
	valid_time -= 86400;
      }
    }
    break;

  } // ii
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "***************************" << endl;
    cerr << "Adding TAF chunk: " << endl;
    cerr << "  Valid time: " << DateTime::str(valid_time) << endl;
    cerr << "  URL: " << _params.output_url << endl;
    cerr << tafStr;
    cerr << "***************************" << endl;
  }

  spdb.addPutChunk(Spdb::hash4CharsToInt32(name.c_str()),
		   valid_time,
		   valid_time + _params.expire_secs,
		   tafStr.size() + 1,
		   tafStr.c_str());
  
}

////////////////////////////////
// write ASCII airep to database

int Taf2Spdb::_writeTafs2Spdb(DsSpdb &spdb)
{
  
  if (spdb.nPutChunks() > 0) {
    
    if (_params.debug) {
      cerr << "Putting tafs to URL: " << _params.output_url << endl;
    }
    spdb.setPutMode(Spdb::putModeOver);
    if (spdb.put(_params.output_url,
		 SPDB_ASCII_ID, SPDB_ASCII_LABEL)) {
      cerr << "ERROR - Taf2Spdb::_writeTafs2Spdb" << endl;
      cerr << spdb.getErrStr() << endl;
      return -1;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////
// tokenize a string into a vector of strings

void Taf2Spdb::_tokenize(const string &str,
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

