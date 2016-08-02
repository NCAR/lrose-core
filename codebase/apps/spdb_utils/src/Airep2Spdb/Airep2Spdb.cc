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
/*********************************************************************
 * Airep2Spdb.cc
 *
 * Program to read in Airep data and store in SPDB
 *
 * Mike Dixon, RAP, NCAR, Boulder CO
 *
 * Dec 2003
 *
 *********************************************************************/

#include <cstdio>
#include <cerrno>
#include <toolsa/os_config.h>

#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <didss/DsInputPath.hh>
#include <rapformats/station_reports.h>
#include <Spdb/Product_defines.hh>
#include "Airep2Spdb.hh"

using namespace std;

/////////////////////////////////////////////////////
// Constructor

Airep2Spdb::Airep2Spdb(int argc, char **argv)

{

  isOK = true;
  _inputPath = NULL;
  _inAirep = false;
  _decodedAirep = false;
  MEM_zero(_airepTime);

  // set programe name
  
  _progName = "Airep2Spdb";
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
				   _params.input_dir,
				   _args.startTime,
				   _args.endTime);
      
    } else {
      
      cerr << "ERROR: " << _progName << endl;
      cerr << "In ARCHIVE mode, you must set start and end times." << endl;
      _args.usage(_progName, cerr);
      isOK = false;

    }

  } else {

    _inputPath = new DsInputPath((char *) _progName.c_str(),
				 _params.debug >= Params::DEBUG_VERBOSE,
				 _params.input_dir,
				 _params.max_realtime_valid_age,
				 PMU_auto_register);
    
  }
  
  // initialize process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // initialize get buffer

  _fgets_save((char *) "");
  
  // read in station locations

  if (_stationLoc.ReadData(_params.st_location_path)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot load locations, file : "
	 << _params.st_location_path << endl;
    isOK = false;
  }
  
  // initialize process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
}

///////////////////////////////
// Destructor

Airep2Spdb::~Airep2Spdb()
{

  // unregister process

  PMU_auto_unregister();

  // Free contained objects

  if (_inputPath) {
    delete _inputPath;
  }

}

/////////////////////////////////////
// Run()

int Airep2Spdb::Run()

{

  int iret = 0;

  char *input_filename;
  while((input_filename = _inputPath->next()) != NULL) {

    _inputFileTime.unix_time = _inputPath->getDataTime(input_filename); 
    if(_inputFileTime.unix_time < 0) {
      cerr << "ERROR - Airep2Spdb::Run" << endl;
      cerr << "  Input file name: " << input_filename << endl;
      cerr << "  File name must be hhmmss.ext" << endl;
      continue;
    }

    uconvert_from_utime(&_inputFileTime);
    _airepTime = _inputFileTime;
    
    // Process the input file

    int airep_count = 0;
    
    // Register with the process mapper

    char procmap_string[BUFSIZ];
    Path path(input_filename);
    sprintf(procmap_string, "File <%s>", path.getFile().c_str());
    PMU_auto_register(procmap_string);
      
    if (_params.debug) {
      cerr << "New data in file: " << input_filename << endl;
    }

    // Open the input file.

    FILE *input_file;
    if((input_file = fopen(input_filename, "r")) == NULL) {
      cerr << "ERROR - Airep2Spdb::Run" << endl;
      cerr << "  Cannot open Input file name: " << input_filename << endl;
      perror(input_filename);
      continue;
    }

    // create SPDB objects
    
    DsSpdb spdbDecoded;
    DsSpdb spdbAscii;
    spdbDecoded.setAppName(_progName);
    spdbAscii.setAppName(_progName);

    while (_read(input_file) == 0) {

      PMU_auto_register(procmap_string);

      // accumulate the chunks
      
      if (_params.write_decoded_spdb) {
	_addDecodedSpdb(spdbDecoded);
      }
      
      if (_params.write_ascii_spdb) {
	_addAsciiSpdb(spdbAscii);
      }

      // clean out extra string if decoded

      if (_decodedAirep) {
	_extraStr = "";
      }
      
      // Increment the airep count
      
      airep_count++;

    } /// while (_read(input_file) ...
      
    // Write
    
    if (_writeDecodedSpdb(spdbDecoded)) {
      iret = -1;
    }
    
    if (_writeAsciiSpdb(spdbAscii)) {
      iret = -1;
    }
    
    if (_params.debug) {
      cerr << "-------->> Number of aireps found: " << airep_count << endl;
    }
      
    // Close the input file.

    fclose(input_file);
\
  } // while((input_filename = _inputPath->next() ...
    
  return iret;

}

////////////////////////////////////
// add decoded struct to spdb object

void Airep2Spdb::_addDecodedSpdb(DsSpdb &spdb)
{

  if (!_decodedAirep) {
    return;
  }

  if (_params.debug) {
    cerr << "Adding decoded AIREP: " << endl;
    pirep_print(stderr, "\t", &_airepInfo);
    cerr << endl;
  }
  
  // copy locally and swap
  
  pirep_t airepInfo = _airepInfo;
  BE_from_pirep(&airepInfo);
  
  // add chunk
  
  spdb.addPutChunk(0,
		   _airepInfo.time,
		   _airepInfo.time + _params.expire_secs,
		   sizeof(pirep_t),
		   &airepInfo);
  
}

/////////////////////////////////
// add ASCII airep to spdb object

void Airep2Spdb::_addAsciiSpdb(DsSpdb &spdb)
{

  if ((_hdrStr.size() > 0) &&
      (_bodyStr.size() > 0 || _extraStr.size() > 0)) {

    string messStr = _hdrStr + _extraStr + _bodyStr;
    spdb.addPutChunk(0,
		     _airepInfo.time,
		     _airepInfo.time + _params.expire_secs,
		     messStr.size() + 1,
		     messStr.c_str());

    if (_params.debug) {
      cerr << "Adding raw AIREP: " << endl << messStr << endl;
    }

  }

}

///////////////////////////////////
// write decoded struct to database

int Airep2Spdb::_writeDecodedSpdb(DsSpdb &spdb)
{
  
  if (spdb.nPutChunks() > 0) {
    if (_params.debug) {
      cerr << "Putting decoded aireps to URL: " << _params.decoded_spdb_url << endl;
    }
    spdb.setPutMode(Spdb::putModeOver);
    if (spdb.put(_params.decoded_spdb_url,
		 SPDB_PIREP_ID, SPDB_PIREP_LABEL)) {
      cerr << "ERROR - Airep2Spdb::_writeDecodedSpdb" << endl;
      cerr << spdb.getErrStr() << endl;
      return -1;
    }
  }

  return 0;

}

////////////////////////////////
// write ASCII airep to database

int Airep2Spdb::_writeAsciiSpdb(DsSpdb &spdb)
{
  
  if (spdb.nPutChunks() > 0) {
    if (_params.debug) {
      cerr << "Putting ascii aireps to URL: " << _params.ascii_spdb_url << endl;
    }
    spdb.setPutMode(Spdb::putModeOver);
    if (spdb.put(_params.ascii_spdb_url,
		 SPDB_ASCII_ID, SPDB_ASCII_LABEL)) {
      cerr << "ERROR - Airep2Spdb::_writeAsciiSpdb" << endl;
      cerr << spdb.getErrStr() << endl;
      return -1;
    }
  }
  
  return 0;

}

///////////////////////////////////////////////////////
// _read(): Read airep data from an input file. 
//
// Returns 0 on success, -1 on failure.
// On success, airep data is put into _airepInfo
// and text is in _airepStr.

int Airep2Spdb::_read(FILE *input_file)
  
{

  char line[MAX_LINE];

  memset(line, 0, MAX_LINE);
  _decodedAirep = false;

  // read through file

  while (_fgets(line, MAX_LINE, input_file) != NULL) {
    
    if(!_inAirep) {
      
      // check for start of AIREP
      
      if(strncmp(line, "\002AIREP OF ", 10) == 0) {

	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "---------------- start -----------------------" << endl;
	  cerr << line + 1 << endl;
	}
	
	// tokenize the line
	
	_tokenize(line + 10);
	_hdrStr = line + 1;
	_bodyStr = "";
	_extraStr = "";
	
        // intialize

	_callSign = "";
	_flevel = 0;
	
	// find callsign - required
	
	if (_tokens.size() > 0) {
	  _callSign = _tokens[0];
	  _tokens.erase(_tokens.begin());
	} else {
	  continue; 
	}

	// decode flight level if on header line
	
	int flevel;
	if (_findFlevel(flevel) == 0) {
	  _flevel = flevel;
	}
        _inAirep = true;

      } // if(strncmp(line, "\002AIREP OF ", 10)
      
    } else {
      
      // already in AIREP
      
      // check for end of AIREP, i.e. start of next block
      
      if(strncmp(line, "\013\003\001BWX", 6) == 0) {
	_inAirep = false;
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "----------------  end  -----------------------" << endl;
	}
	return 0;
      }
      
      // tokenize and decode AIREP
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << line << endl;
      }

      _tokenize(line);
      _decodeAirep();

      if (_locationFound && _timeFound) {

	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "****** Good airep ******" << endl;
	}

	// good standard AIREP
	
	_bodyStr = line;
	_decodedAirep = true;
	return 0;

      } else if (_timeFound) {

	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "****** Incomplete standard airep with time ******" << endl;
	}

	// incomplete standard AIREP - with time only save ASCII

	_bodyStr = line;
	_decodedAirep = false;
	return 0;

      } else if (_windFound || _tempFound) {

	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "****** Incomplete standard airep, NO time ******" << endl;
	}

	// incomplete standard AIREP without time - ignore
	
      } else {

	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "****** Non-standard text - add to extras ******" << endl;
	}
	
	// non-standard text - store as extra line

	if (_extraStr.size() > 0) {
	  _extraStr.resize(_extraStr.size() - 1);
	  _extraStr += ", ";
	}
	_extraStr += line;
	
      }
      
    } // if (!_inAirep)

  } // while (_fgets(line, MAX_LINE, input_file) != NULL)
  
  return -1;

}

char *Airep2Spdb::_fgets(char *buffer, int len, FILE *input_file)
{
  if(*fgets_buffer != '\0')
  {
    strncpy(buffer, fgets_buffer, len);
    *fgets_buffer = '\0';
    return buffer;
  } else {
    return fgets(buffer, len, input_file);
  }
}

// save line for later use

void Airep2Spdb::_fgets_save(char *buffer)
{
  strcpy(fgets_buffer, buffer);
}

// tokenize line

void Airep2Spdb::_tokenize(const char *line)
{
  _tokens.clear();
  char buf[strlen(line) + 1];
  strcpy(buf, line);
  char *token = strtok(buf, " \n\r\t");
  while (token != NULL) {
    _tokens.push_back(token);
    token = strtok(NULL, " \n\r\t");
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> Tokens: ";
    for (size_t ii = 0; ii < _tokens.size(); ii++) {
      cerr << _tokens[ii] << ", ";
    }
    cerr << endl;
  }
}

// decode airep

int Airep2Spdb::_decodeAirep()

{
  
  pirep_init(&_airepInfo);

  STRncopy(_airepInfo.callsign, _callSign.c_str(),
	   PIREP_CALLSIGN_LEN);
  if (_flevel != 0.0) {
    _airepInfo.alt = _flevel * 100.0;
  }

  _locationFound = false;
  _timeFound = false;
  _windFound = false;
  _tempFound = false;

  if (_tokens.size() < 2) {
    return -1;
  }

  // abeam??
  
  _findAbeam();
  
  // find position - required
  
  if (_findLocation() == 0) {
    _locationFound = true;
  }
  
  // find time - required
  
  if (_findTime() == 0) {
    _timeFound = true;
  }
  
  // find flight level

  int flevel;
  if (_findFlevel(flevel) == 0) {
    _airepInfo.alt = flevel * 100.0;
  }
  
  // find wind
  
  if (_findWind() == 0) {
    _windFound = true;
  }
  
  // find temp
  
  if (_findTemp() == 0) {
    _tempFound = true;
  }
  
  // remaining tokens are the text
  
  _findText();

  return 0;

}
  
// decode flight level, erasing the flight level token if successful

int Airep2Spdb::_findFlevel(int &flevel)
{
  
  vector<string>::iterator ii;
  for (ii = _tokens.begin(); ii != _tokens.end(); ii++) {
    const string &token = *ii;
    if (sscanf(token.c_str(), "F%d", &flevel) == 1) {
      _tokens.erase(ii);
      return 0;
    }
    if (sscanf(token.c_str(), "FL%d", &flevel) == 1) {
      _tokens.erase(ii);
      return 0;
    }
  } // ii

  return -1;

}

// find abeam indication, erasing the abeam token if successful

int Airep2Spdb::_findAbeam()
{
  
  vector<string>::iterator ii;
  for (ii = _tokens.begin(); ii != _tokens.end(); ii++) {
    const string token = *ii;
    if (token == "ABM" ||
	token == "ABEAM") {
      _tokens.erase(ii);
      return 0;
    }
  } // ii

  return -1;

}


// find location, erasing the location token

int Airep2Spdb::_findLocation()
{

  if (_tokens.size() < 1) {
    return -1;
  }

  const string locStr = _tokens[0];
  _tokens.erase(_tokens.begin());

  double lat, lon, alt;
  string type;
  if (_stationLoc.FindPosition(locStr, lat, lon, alt, type) == 0) {
    _airepInfo.lat = lat;
    _airepInfo.lon = lon;
    return 0;
  }

  return -1;

}

// find time, erasing the time token if successful

int Airep2Spdb::_findTime()
{

  if (_tokens.size() < 1) {
    return -1;
  }

  const string timeStr = _tokens[0];
  int hour, min;
  if (sscanf(timeStr.c_str(), "%2d%2d", &hour, &min) != 2) {
    return -1;
  }

  _airepTime.hour = hour;
  _airepTime.min = min;
  uconvert_to_utime(&_airepTime);
  _airepInfo.time = _airepTime.unix_time;

  _tokens.erase(_tokens.begin());
  return 0;

}

// find wind, erasing the wind token if successful

int Airep2Spdb::_findWind()
{
  
  vector<string>::iterator ii;
  for (ii = _tokens.begin(); ii != _tokens.end(); ii++) {
    const string token = *ii;
    int dirn, speed;
    if (sscanf(token.c_str(), "%d/%d", &dirn, &speed) == 2) {
      _airepInfo.wind_dirn = dirn;
      _airepInfo.wind_speed = speed;
      _tokens.erase(ii);
      return 0;
    }
  } // ii

  return -1;

}


// find temp, erasing the temp token if successful

int Airep2Spdb::_findTemp()
{

  vector<string>::iterator ii;
  for (ii = _tokens.begin(); ii != _tokens.end(); ii++) {
    const string token = *ii;
    int temp;
    if (sscanf(token.c_str(), "M%d", &temp) == 1) {
      _airepInfo.temp = temp * -1.0;
      _tokens.erase(ii);
      return 0;
    }
    if (sscanf(token.c_str(), "MS%d", &temp) == 1) {
      _airepInfo.temp = temp * -1.0;
      _tokens.erase(ii);
      return 0;
    }
    if (sscanf(token.c_str(), "%2d", &temp) == 1) {
      if (temp < 50) {
	_airepInfo.temp = temp;
	_tokens.erase(ii);
	return 0;
      }
    }
  } // ii

  return -1;

}

// find text

void Airep2Spdb::_findText()
{
  
  string textStr;
  vector<string>::iterator ii;
  for (ii = _tokens.begin(); ii != _tokens.end(); ii++) {
    textStr += *ii;
    if (ii != _tokens.end() - 1) {
      textStr += " ";
    }
  } // ii
  if (_extraStr.size() > 0) {
    if (textStr.size() > 0) {
      textStr += ", ";
    }
    textStr += _extraStr;
  }
  
  STRconcat(_airepInfo.text, textStr.c_str(), PIREP_TEXT_LEN);
  
}
