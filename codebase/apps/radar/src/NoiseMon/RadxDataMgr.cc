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
// RadxDataMgr.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2022
//
///////////////////////////////////////////////////////////////
//
// Data manager for Radx moments data
//
////////////////////////////////////////////////////////////////

#include "RadxDataMgr.hh"
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxRay.hh>

using namespace std;

const double RadxDataMgr::_missingDouble = -9999.0;
const double RadxDataMgr::_missingTest = -9998.0;

// Constructor

RadxDataMgr::RadxDataMgr(const string &prog_name,
                         const Args &args,
                         const Params &params) :
        StatsMgr(prog_name, args, params)
  
{
  
}

// destructor

RadxDataMgr::~RadxDataMgr()

{

}

//////////////////////////////////////////////////
// Run

int RadxDataMgr::run ()
{

  if (_params.debug) {
    cerr << "Running NoiseMon in RADX_MOMENTS_INPUT mode" << endl;
  }

  // check if start and end times are set

  bool startTimeSet = true;
  time_t startTime = RadxTime::parseDateTime(_params.start_time);
  if (startTime == RadxTime::NEVER || startTime < 1) {
    startTimeSet = false;
  }

  bool endTimeSet = true;
  time_t endTime = RadxTime::parseDateTime(_params.end_time);
  if (endTime == RadxTime::NEVER || endTime < 1) {
    endTimeSet = false;
  }
  
  vector<string> paths = _args.inputFileList;
  if (paths.size() == 0) {

    if (startTimeSet && endTimeSet) {
      
      if (_params.debug) {
        cerr << "  Input dir: " << _params.input_dir << endl;
        cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
        cerr << "  End time: " << RadxTime::strm(endTime) << endl;
      }
      
      // get the files to be processed
      
      RadxTimeList tlist;
      tlist.setDir(_params.input_dir);
      tlist.setModeInterval(startTime, endTime);
      if (tlist.compile()) {
        cerr << "ERROR - NoiseMon::RadxDataMgr::run()" << endl;
        cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
        cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
        cerr << "  End time: " << RadxTime::strm(endTime) << endl;
        cerr << tlist.getErrStr() << endl;
        return -1;
      }
      
      paths = tlist.getPathList();
      
      if (paths.size() < 1) {
        cerr << "ERROR - NoiseMon::RadxDataMgr::run()" << endl;
        cerr << "  No files found, dir: " << _params.input_dir << endl;
        return -1;
      }
    
    } // if (startTimeSet && endTimeSet)

  }

  // loop through the input file list
  
  int iret = 0;
  for (size_t ipath = 0; ipath < paths.size(); ipath++) {
    if (_processFile(paths[ipath])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int RadxDataMgr::_processFile(const string &filePath)
{
  
  PMU_auto_register(((string) "Processing file: " + filePath).c_str());

  // ensure memory is freed up
  
  _readVol.clear();

  if (_params.debug) {
    cerr << "INFO - RadxDataMgr::_processFile" << endl;
    cerr << "  Input file path: " << filePath << endl;
    cerr << "  Reading in file ..." << endl;
  }
  
  // read in file
  
  if (_readFile(filePath)) {
    return -1;
  }

  // check we have at least 2 rays

  if (_readVol.getNRays() < 2) {
    cerr << "ERROR - RadxDataMgr::_processFile" << endl;
    cerr << "  Too few rays: " << _readVol.getNRays() << endl;
    return -1;
  }

  // loop through the rays, processing them

  const vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    processRay(_readVol.getPlatform(), rays[ii]);
  }

  // free up

  _readVol.clear();

  return 0;

}

//////////////////////////////////////////////////
// Read in a RADX file
// Returns 0 on success, -1 on failure

int RadxDataMgr::_readFile(const string &filePath)
{

  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  if (inFile.readFromPath(filePath, _readVol)) {
    cerr << "ERROR - RadxDataMgr::_readFile" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }

  // convert data to floats

  _readVol.convertToFl32();

  return 0;

}

//////////////////////////////////////////////////
// set up read

void RadxDataMgr::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }

  for (int ii = 0; ii < _params.input_fields_n; ii++) {
    if (string(_params._input_fields[ii].moments_name) != "missing") {
      file.addReadField(_params._input_fields[ii].moments_name);
    }
  } // ii 

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.printReadRequest(cerr);
  }
  
}

