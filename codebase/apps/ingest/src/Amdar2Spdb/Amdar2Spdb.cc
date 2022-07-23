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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: Amdar2Spdb.cc,v 1.14 2016/03/07 01:22:59 dixon Exp $
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	Amdar2Spdb
//
// Author:	G. M. Cunning
//
// Date:	Sat Mar 17 13:52 2012
//
// Description: This class creates manages reading, parsing and writing 
//		AMDAR messages.
//
//

// C++ include files
#include <iostream>
#include <cassert>
#include <algorithm>

// System/RAP include files
#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <Spdb/DsSpdb.hh>
#include <rapformats/Amdar.hh>

// Local include files
#include "Amdar2Spdb.hh"
#include "Decoder.hh"
#include "TextDecoder.hh"
#include "BufrDecoder.hh"
#include "Input.hh"
#include "Params.hh"
#include "Args.hh"

using namespace std;

// the singleton itself
Amdar2Spdb *Amdar2Spdb::_instance = 0;
 
// define any constants
const string Amdar2Spdb::_className = "Amdar2Spdb";

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

Amdar2Spdb::Amdar2Spdb() :
  _isOK(true),
  _progName(""),
  _errStr(""),
  _args(0),
  _params(0)

{
  // Make sure the singleton wasn't already created.
  assert(_instance == 0);
 
  // Set the singleton instance pointer
  _instance = this; 
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
Amdar2Spdb::~Amdar2Spdb()
{
  // unregister process
  PMU_auto_unregister();

  // Free contained objects
  delete _params;
  delete _args;
  delete _inputPath;
  delete _input;
  delete _textDecoder;
  delete _bufrDecoder;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Amdar2Spdb::instance
//
// Description:	this method creates instance of Amdar2Spdb object
//
// Returns:	returns pointer to self
//
// Notes:	this method implements the singleton pattern
//
//

Amdar2Spdb*
Amdar2Spdb::instance(int argc, char **argv)
{
  if (_instance == 0) {
    new Amdar2Spdb();
 
   if(_instance->_isOK) {
      _instance->_initialize(argc, argv);
    }
  }
  return(_instance);
}

Amdar2Spdb*
Amdar2Spdb::instance()
{
  assert(_instance != 0);
  
  return(_instance);
}


/////////////////////////////////////////////////////////////////////////
// run
//
int 
Amdar2Spdb::run()
{

  const string methodName = _className + string( "::run" );

  // register with procmap
  PMU_auto_register("Running");

  if(static_cast<int>(_params->debug) != static_cast<int>(Params::DEBUG_OFF)) {
    cerr << methodName << ": Running" << endl;
  } 

  char* inPath = NULL;
  while ((inPath = _inputPath->next(true)) != 0) {
    string inputFilePath(inPath);
    _fileTime = _inputPath->getDataTime(inputFilePath);

    if (_params->mode == Params::REALTIME) {
      _refTime.set(time(0)); // now
    } else {
      _refTime.set(_fileTime);
    }

    string procmapString = "";
    Path path(inputFilePath);
    procmapString = "Processing file <" + path.getFile() + ">";  
    PMU_auto_register(procmapString.c_str());

    _processFile(inputFilePath);

  } // while

  return 0;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// _initialize
//
void 
Amdar2Spdb::_initialize(int argc, char **argv)
{
  const string methodName = _className + string( "::_initialize" );

  // set programe name
  Path pathParts(argv[0]);
  _progName = pathParts.getBase();
  ucopyright(const_cast<char*>(_progName.c_str()));
 
  // get command line args
  _args= new Args(argc, argv, _progName);

  // get TDRP params
  _params = new Params();
  char *paramsPath = const_cast<char*>(string("unknown").c_str());
  if(_params->loadFromArgs(argc, argv, _args->override.list,
			  &paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    _isOK = false;
  } // endif -- _params->loadFromArgs(argc, argv, _args.override.list, ...

  // file input object
  bool debugDs= false;
  if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_VERBOSE)) {
    debugDs= true;
  }

  _inputPath = 0;  
  if (_params->mode == Params::FILELIST) {
    
    // FILELIST mode
    
    _inputPath = new DsInputPath(_progName, debugDs, _args->inputFileList);
    
  } else if (_params->mode == Params::ARCHIVE) {
    
    // archive mode - start time to end time
    
    _inputPath = new DsInputPath(_progName, debugDs, _params->input_dir,
				 DateTime::parseDateTime(_params->start_time),
				 DateTime::parseDateTime(_params->end_time));
    
  } else if (_params->mode == Params::REALTIME) {
    
    // realtime mode - no latest_data_info file
    
    _inputPath = new DsInputPath(_progName, debugDs, _params->input_dir,
				 _params->max_realtime_valid_age,
				 PMU_auto_register,
				 _params->latest_data_info_avail,
				 true);
    
    if (_params->strict_subdir_check) {
      _inputPath->setStrictDirScan(true);
    }
    
    if (_params->file_name_check) {
      _inputPath->setSubString(_params->file_match_string);
    }
    
  }
  
  //
  // expect both types of message formats in the GTS stream
  // examination of the GTS bulletin header will detemine the message format
  //
  _textDecoder = new TextDecoder(_params);
  _bufrDecoder = new BufrDecoder(_params);

  _input = new Input(_params);
  _input->setSOH(_params->start_of_header[0]);
  _input->setEOH(_params->end_of_header[0]);

  // set the WMO and bulletin type IDs for the input formats
  for(int i = 0; i < _params->input_formats_n; i++) {
    if(_params->_input_formats[i] == Params::FM42) {
      _input->addWmoId(_textDecoder->getWmoId());
      _input->addTypeId(_textDecoder->getTypeId());
    }
    else if(_params->_input_formats[i] == Params::FM94) {
      _input->addWmoId(_bufrDecoder->getWmoId());
      _input->addTypeId(_bufrDecoder->getTypeId());
    }
  }

  // init process mapper registration
  if (_params->mode == Params::REALTIME) {
    PMU_auto_init((char *) _progName.c_str(),
		  _params->instance,
		  PROCMAP_REGISTER_INTERVAL);
    PMU_auto_register("In Amdar2Spdb constructor");
  }

}


/////////////////////////////////////////////////////////////////////////
// _processFile
//
int 
Amdar2Spdb::_processFile(const string& file_path)  
{

  const string methodName = _className + string( "::_processFile" );

  // open file
  if(_input->open(file_path)) {
    cerr << "ERROR - " << methodName << endl;
    return -1;
  }
  

  if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_NORM)) {
    cerr << "Processing file: " << file_path << endl;
    cerr << "  File time: " << DateTime::str(_fileTime) << endl << endl;
  }
  
  _textDecoder->setFileTime(_fileTime);
  _bufrDecoder->setFileTime(_fileTime);

  vector<Amdar*> amdars;
  vector<DsSpdb*> asciiSpdb;
  vector<DsSpdb*> xmlSpdb;
  string amdarStr;
  int nTotal = 0;
  int nDecoded = 0;
  Decoder *decoder = 0;

  for(int i = 0; i < _params->ascii_output_urls_n; i++) {
    asciiSpdb.push_back(new DsSpdb);
  }

  for(int i = 0; i < _params->xml_output_urls_n; i++) {
    xmlSpdb.push_back(new DsSpdb);
  }

  while(_input->readNext(amdarStr) == 0) {
    
    PMU_auto_register("reading next message");
    
    if(amdarStr.empty() == true) {
      continue;
    }

    nTotal++;

    if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_EXTRA)) {
      cerr << "====== Count is " << nTotal << " =====" << endl;
      cerr << "========== Start AMDAR ===========" << endl << amdarStr 
	   << endl << "========== End AMDAR ===========" << endl;
    }
  
    if(_textDecoder->checkFormat(_input->getWmoHeader(), amdarStr) == 
       Decoder::MSG_TYPE_ASCII) {
      decoder = _textDecoder; 
      if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_VERBOSE)) {
	cerr << "Using ASCII decoder" << endl;
      }      
    } else if(_bufrDecoder->checkFormat(_input->getWmoHeader(), amdarStr) == 
	      Decoder::MSG_TYPE_BUFR) {
      decoder = _bufrDecoder; 
      if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_VERBOSE)) {
	cerr << "Using BUFR decoder" << endl;
      }
    } else {
	cerr << "***************************" << endl;
        cerr << "WARNING -- " << methodName<< endl;
        cerr << "Unknown message type" << endl;
	if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_NORM)) {
	  cerr << "WMO header: " << _input->getWmoHeader()<< endl;
	  cerr << "Contents:" << endl << amdarStr;
	  if (amdarStr[amdarStr.size()-1] != '\n') {
	    cerr << endl;
	  }
	}
	cerr << "***************************" << endl << endl;
	continue;
    }
      
    if(decoder->process(amdarStr, amdars) == 0) {
      nDecoded += amdars.size();
      _addPut(amdars, asciiSpdb, xmlSpdb);
    } else {
      if (static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_NORM)) {
        cerr << "WARNING -- " << methodName<< endl;
        cerr << "Cannot decode AMDAR:" << endl;
        cerr << amdarStr;
        if (amdarStr[amdarStr.size()-1] != '\n') {
          cerr << endl;
        }
        cerr << "***************************" << endl;
      }
    }

    // clean up 
    for(size_t i = 0; i < amdars.size(); i++) {
      delete amdars[i];
      amdars[i] = 0;
    }
    amdars.clear();

  } // while

  // close file
  
  _input->close();

  // no need to write
  if(nDecoded < 1) {
    return 0;
  }

  if((static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_NORM)) ||
      (_params->print_decode_problems_to_stderr)) {
    cerr << "---------------------------------------" << endl;
    cerr << "Done with file: " << file_path << endl;
    cerr << "  N AMDAR messages found: " << nTotal << endl;
    cerr << "  N AMDAR bulletins decoded: " << nDecoded << endl;
    cerr << "---------------------------------------" << endl;
  }
  
  // Write out
  
  int iret = 0;

  if(_params->store_ascii_format) {
    
    for(int i = 0; i < _params->ascii_output_urls_n; i++) {
      if(asciiSpdb[i]->put(_params->_ascii_output_urls[i],
		       SPDB_ASCII_ID, SPDB_ASCII_LABEL)) {
	cerr << "ERROR - " << methodName << endl;
	cerr << asciiSpdb[i]->getErrStr() << endl;
	iret = -1;
      } 
      else {
	if (static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_NORM)) {
	  cerr << "Wrote ASCII AMDAR data to URL: "
	       << _params->_ascii_output_urls[i] << endl;
	}
      }
    }

  }

  if (_params->store_xml_format) {

    for(int i = 0; i < _params->xml_output_urls_n; i++) {

      if (xmlSpdb[i]->put(_params->_xml_output_urls[i],
		      SPDB_AMDAR_ID, SPDB_AMDAR_LABEL)) {
	cerr << "ERROR - " << endl;
	cerr << xmlSpdb[i]->getErrStr() << methodName << endl;
	iret = -1;
      } else {
	if (static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_NORM)) {
	  cerr << "Wrote decoded XML AMDAR data to URL: "
	       << _params->_xml_output_urls[i] << endl;
	}
      }
    }

  }

  // clean up
  for(size_t i = 0; i < amdars.size(); i++) {
    delete amdars[i];
    amdars[i] = NULL;
  }
  amdars.clear();

  for(size_t i = 0; i < asciiSpdb.size(); i++) {
    delete asciiSpdb[i];
    asciiSpdb[i] = NULL;
  }
  asciiSpdb.clear();

  for(size_t i = 0; i < xmlSpdb.size(); i++) {
    delete xmlSpdb[i];
    xmlSpdb[i] = NULL;
  }
  xmlSpdb.clear();

  return iret;
}

/////////////////////////////////////////////////////////////////////////
// _addPut
//
void 
Amdar2Spdb::_addPut(const vector<Amdar*>& amdars, vector<DsSpdb*>& ascii_spdb, 
		    vector<DsSpdb*>& xml_spdb)
{

  // ASCII output?

  time_t testTime;
  if (_params->mode == Params::REALTIME) {
    testTime = time(0);
  }
  else {
    testTime = _fileTime;
  }

  for(size_t i = 0; i < amdars.size(); i++) {
  
    // check that issue time is not significantly in past
      time_t deltaT = testTime - amdars[i]->getIssueTime();
      if((deltaT > _params->max_bulletin_valid_age) || (deltaT < 0)) {
	if (static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_EXTRA)) {
	  cerr << "==============================" << endl;
	  cerr << "Skipping ASCII AMDAR: " << endl;
	  cerr << "  Issue time: " << DateTime::str(amdars[i]->getIssueTime()) << endl;
	  cerr << "  greater than maximum for a bulleting " << endl;
	  cerr << "  " << amdars[i]->getText() << endl;
	  cerr << "==============================" << endl;
	}
	continue;
      }

    si32 dataType = Spdb::hash4CharsToInt32(amdars[i]->getAircraftId().c_str());
    si32 dataType2 = Spdb::hash4CharsToInt32(amdars[i]->getFlightPhase().c_str());
    time_t validTime = amdars[i]->getIssueTime();
    time_t expireTime =  validTime + _params->expire_seconds;

    if (_params->store_ascii_format) {
    
      string asciiChunk;
      if (_params->store_wmo_header) {
	asciiChunk += _input->getWmoHeader();
	asciiChunk += "\n";
      }

      asciiChunk += amdars[i]->getText();
      for(int j = 0; j < _params->ascii_output_urls_n; j++) {
	ascii_spdb[j]->addPutChunk(dataType, validTime, expireTime,
				  asciiChunk.size() + 1, asciiChunk.c_str(),
				  dataType2);
      }

      if (static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_VERBOSE)) {
	cerr << "==============================" << endl;
	cerr << "Adding ASCII AMDAR chunk: " << endl;
	cerr << "  Issue time: " << DateTime::str(validTime) << endl;
	amdars[i]->print(cerr);
	cerr << "==============================" << endl;
      }

    } // if (_params->store_ascii_format) {
  
    // XML output?
  
    if (_params->store_xml_format) {
      amdars[i]->assemble();
      for(int j = 0; j < _params->xml_output_urls_n; j++) {
	xml_spdb[j]->addPutChunk(dataType, validTime, expireTime,
				amdars[i]->getBufLen(), amdars[i]->getBufPtr(),
				dataType2);
      }

      if (static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_VERBOSE)) {
	cerr << "===========================" << endl;
	cerr << "Adding decoded AMDAR chunk: " << endl;
	cerr << "  Issue time: " << DateTime::str(validTime) << endl;
	amdars[i]->print(cerr);
	cerr << "===========================" << endl;
      }

    }

  }
}
