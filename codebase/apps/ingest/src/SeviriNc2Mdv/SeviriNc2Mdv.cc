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
 *  $Id: SeviriNc2Mdv.cc,v 1.10 2016/03/07 01:23:05 dixon Exp $
 */

# ifndef    lint
static char RCSid[] = "$Id: SeviriNc2Mdv.cc,v 1.10 2016/03/07 01:23:05 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	SeviriNc2Mdv
//
// Author:	G. M. Cunning
//
// Date:	Wed Jul 25 22:10:08 2007
//
// Description: 
//
//

// C++ include files
#include <iostream>
#include <cassert>

// RAL include files
#include <toolsa/os_config.h>
#include <dsdata/DsTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/DsDirListTrigger.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsIntervalTrigger.hh>
#include <dataport/port_types.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>

// Local include files
#include "SeviriNc2Mdv.hh"
#include "SeviriData.hh"
#include "SeviriConverter.hh"
#include "OutputFile.hh"
#include "Args.hh"

using namespace std;

// the singleton itself
SeviriNc2Mdv *SeviriNc2Mdv::_instance = 0;
 
// define any constants
const string SeviriNc2Mdv::_className = "SeviriNc2Mdv";

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

SeviriNc2Mdv::SeviriNc2Mdv() :
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
  
SeviriNc2Mdv::~SeviriNc2Mdv()
{
  // unregister process
  PMU_auto_unregister();

  // Free contained objects
  delete _params;
  delete _args;
  delete _outputFile;
  delete _seviriData;
  delete _seviriConverter;
  delete _seviriPjg;
  delete _dsTrigger;
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
// Method Name:	SeviriNc2Mdv::instance
//
// Description:	this method creates instance of SeviriNc2Mdv object
//
// Returns:	returns pointer to self
//
// Notes:	this method implements the singleton pattern
//
//

SeviriNc2Mdv*
SeviriNc2Mdv::instance(int argc, char **argv)
{
  if (_instance == 0) {
    new SeviriNc2Mdv();

    if (_instance->_isOK) {
      _instance->_initialize(argc, argv);
    }
  }

  return(_instance);
}

SeviriNc2Mdv*
SeviriNc2Mdv::instance()
{
  assert(_instance != 0);
  
  return(_instance);
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriNc2Mdv::run
//
// Description:	runs the object
//
// Returns:	returns 0
//
// Notes:	
//
//

int 
SeviriNc2Mdv::run()
{

  const string methodName = _className + string( "::run" );

  // register with procmap
  PMU_auto_register("Running");

  if (_params->debug_mode != Params::DEBUG_OFF) {
    cerr << "Running:" << endl;
  } // endif -- _params->debug_mode != Params::DEBUG_OFF

  // Do something

  while (!_dsTrigger->endOfData()) {

    TriggerInfo triggerInfo;
    if (_dsTrigger->next(triggerInfo) == 0) {

      cout << methodName << " -- processing " << triggerInfo.getFilePath() << endl;

      string filePath = _uncompressFile(triggerInfo.getFilePath());

      cout << methodName << " -- uncompressed path is " <<  filePath << endl;
      
      _seviriData->readFile(filePath);

      if (_seviriData->process()) {

	if (_params->convert_counts) {
	  _seviriData->convertUnits(_seviriConverter);
	}
	
	_outputFile->addData(_seviriData);

      }
      _seviriData->reset();

      _compressFile(filePath, "gz");
	
    }
    else {

      if (_params->mode != Params::DIR_LIST) {
	cerr << methodName << " -- _dsTrigger->next() failed: " << _dsTrigger->getErrStr() << endl;
      }
    }
      
  } // end while -- !_dsTrigger->endOfData()

  _outputFile->write();

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
//
// Method Name:	SeviriNc2Mdv::_initialize
//
// Description:	Initializes the class.
//
// Returns:	none
//
// Notes:	use isOK method to test success
//
//

void 
SeviriNc2Mdv::_initialize(int argc, char **argv)
{
  const string methodName = _className + "::_initialize";

  // set programe name
  Path pathParts(argv[0]);
  _progName = pathParts.getBase();
  ucopyright(const_cast<char*>(_progName.c_str()));
 
  // get command line args
  _args = new Args();

  _args->parse(argc, argv, _progName);
  if (!_args->isOK) {
    _isOK = false;
    _errStr = "failed to construct Args object.";
    return;
  }

  // get TDRP params
  _params = new Params();

  char *paramsPath = "unknown";
  if (_params->loadFromArgs(argc, argv, _args->override.list,
			  &paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    _isOK = false;
    return;
  } // endif -- _params.loadFromArgs(argc, argv, _args.override.list, ...


  // setup input file gathering
  if (!_setupInput()) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with input setup" << endl;
    _isOK = false;
    return;
  } // endif -- _params.loadFromArgs(argc, argv, _args.override.list, ...

  // display RCSid
  if (_params->debug_mode != Params::DEBUG_VERBOSE) {
    cerr << RCSid << endl;
  } //endif -- _params->debug_mode != Params::DEBUG_VERBOSE

  _seviriData = new SeviriData;
  _seviriData->setParams(_params);

  _seviriConverter = new SeviriConverter;

  _outputFile = new OutputFile;
  _outputFile->init(_params);
 
  _seviriData->setMissing(_outputFile->MISSING_DATA_VALUE);
  _seviriConverter->setMissing(_outputFile->MISSING_DATA_VALUE);

 // init process mapper registration
  PMU_auto_init((char *) _progName.c_str(),
		_params->instance,
		PROCMAP_REGISTER_INTERVAL);


}


/////////////////////////////////////////////////////////////////////////
//
// Method Name: SeviriNc2Mdv::_setupInput
//
// Description:	manage instantiation of the different DsTrigger sub-
//		classes, based on settings in the configuration file
//
// Returns:	if succesfull, return true
//
// Notes:
//
//

bool
SeviriNc2Mdv::_setupInput()
{
  const string methodName = _className + string( "::_setupInput" );

  switch (_params->mode) {
  case Params::REALTIME:
    {
      DsInputDirTrigger *inputDirTrigger = new DsInputDirTrigger();

      inputDirTrigger->init(_params->input_dir, _params->file_substring, false);
      _dsTrigger = inputDirTrigger;

      break;
    }
  case Params::TIME_LIST:
    {
      DsIntervalTrigger *intervalTrigger =  new DsIntervalTrigger();
      _dsTrigger = intervalTrigger;

      cerr << "Use of this triggering method has not been implemented yet." << endl; 
      exit(1);
      break;
    }
  case Params::DIR_LIST:
    {
      DsDirListTrigger *dirListTrigger = new DsDirListTrigger();

      dirListTrigger->init(_args->getInputDirList(), _params->file_substring);
      _dsTrigger = dirListTrigger;

      break;
    }
  case Params::FILE_LIST:
    {
      DsFileListTrigger *fileListTrigger = new DsFileListTrigger();

	fileListTrigger->init(_args->getInputFileList());
	_dsTrigger = fileListTrigger;

      break;
    }
  default:
    cerr << "ERROR: " << methodName << endl;
    cerr << "Invalid mode given for input processing" << endl;
    return false;
  } // endswitch - _params->mode
  

    return true;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriNc2Mdv::_uncompressFile
//
// Description:	this is a wrapper around  ta_file_uncompress that can 
//		use a string object.
//
// Returns:	uncompressed file path is successful. empty string on failure
//
// Notes:
//
//

string 
SeviriNc2Mdv::_uncompressFile(const string& in_path)
{
  const string methodName = _className + string( "::_uncompressFile" );

  int strLen = in_path.size()+1;
  char* modPath = new char[strLen];
  STRcopy(modPath, in_path.c_str(),strLen);

  string outPath;
  if (ta_file_uncompress(modPath) == 1) {
    outPath = modPath;
  }
  else {
    outPath = in_path;
  }
  

  delete [] modPath;

  return outPath;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriNc2Mdv::_compressFile
//
// Description:	this is a wrapper around  ta_file_uncompress that can 
//		use a string object.
//
// Returns:	uncompressed file path is successful. empty string on failure
//
// Notes:
//
//

bool
SeviriNc2Mdv::_compressFile(const string& path, const string& ext)
{
  const string methodName = _className + string( "::_compressFile" );
  cerr << methodName << " is not implemented." << endl;
  return false;
}

