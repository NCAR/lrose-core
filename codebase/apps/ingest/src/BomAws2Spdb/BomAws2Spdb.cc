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
// BomAws2Spdb.cc
//
// BomAws2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2007
//
///////////////////////////////////////////////////////////////
//
// BomAws2Spdb reads automated weather station surface
// observations, converts them to station_report_t format
// and writes them to an SPDB data base
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
#include <didss/DsInputPath.hh>
#include "BomAws2Spdb.hh"
#include "ReadObsAxf.hh"
using namespace std;

// Constructor

BomAws2Spdb::BomAws2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "BomAws2Spdb";
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

BomAws2Spdb::~BomAws2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int BomAws2Spdb::Run ()
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
  while ((inputPath = input->next()) != NULL) {
    
    if (_processFile(inputPath)) {
      cerr << "WARNING - BomAws2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }
    
  } // while
    
  return 0;

}

////////////////////
// process the file

int BomAws2Spdb::_processFile(const char *file_path)
  
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

  // create reader object

  readObsAXF reader(_params);

  // read in file, loading up chunks in output object spdb
  
  int nObs = reader.readFile(file_path, out);
  if (nObs < 0) {
    cerr << "ERROR - BomAws2Spdb::_processFile" << endl;
    cerr << "  Cannot read file: " << file_path << endl;
    return -1;
  }

  // put the data

  if (out.put(_params.output_url,
	      SPDB_STATION_REPORT_ID,
	      SPDB_STATION_REPORT_LABEL)) {
    cerr << "ERROR - BomAws2Spdb::_processFile" << endl;
    cerr << "  Cannot put station data to: "
	 << _params.output_url << endl;
    cerr << "  " << out.getErrStr() << endl;
    iret = -1;
  }

  if (_params.debug) {
    cerr << "  Done with file: " << file_path << endl;
  }

  return iret;
   
}

