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
#include<fstream>
#include<sstream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/TaFile.hh>
#include <toolsa/TaArray.hh>
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
      cerr << "WARNING - EolMobileKml2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }

  } // while

  delete input;

  return 0;

}

////////////////////
// process the file

int EolMobileKml2Spdb::_processFile(const char *inputPath) 
{

  int iret = 0;

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
    cerr << "ERROR - cannot open input file: " << inputPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // get file size

  if (inFile.fstat()) {
    int errNum = errno;
    cerr << "ERROR - cannot stat input file: " << inputPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }
  struct stat fileStat = inFile.getStat(); 
  
  // read contents into buffer
  
  TaArray<char> kml_;
  char *kml = kml_.alloc(fileStat.st_size + 1);
  if (inFile.fread(kml, 1, fileStat.st_size) != fileStat.st_size) {
    int errNum = errno;
    cerr << "ERROR - cannot read input file: " << inputPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }
  inFile.fclose();
  
  if (_params.debug) {
    cerr << "Read in file: " << inputPath << endl;
    cerr << "===========================================" << endl;
    cerr << kml << endl;
    cerr << "===========================================" << endl;
  }
  
  // create output object
  
  DsSpdb spdb;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    spdb.setDebug();
  }
  
  // write the output

  // if (_doPut(spdb)) {
  //   iret = -1;
  // }

  return iret;

}

////////////////////////////////
// do put to SPDB

int EolMobileKml2Spdb::_doPut(DsSpdb &spdb) {

  int iret = 0;
  
  if (spdb.put(_params.output_spdb_url,
               SPDB_AC_POSN_ID,
               SPDB_AC_POSN_LABEL)) {
    cerr << "ERROR - EolMobileKml2Spdb::_doPut" << endl;
    cerr << "  Cannot put data to: "
         << _params.output_spdb_url << endl;
    cerr << "  " << spdb.getErrStr() << endl;
    iret = -1;
  }

  spdb.clearPutChunks();

  return iret;

}

