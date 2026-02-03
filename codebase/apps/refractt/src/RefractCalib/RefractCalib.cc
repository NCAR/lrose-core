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
/////////////////////////////////////////////////////////////
// RefractCalib Main
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2026
//
/////////////////////////////////////////////////////////////
//
// RefractCalib:
//    (a) reads radar scan files, in polar coordinates
//    (b) identifies suitable clutter targets
//    (c) computes the mean phase of those targets for a baseline calibration
//    (d) writes the calibration details to a file.
// Typically we use 6 hours of scans for this purpose.
// Ideally the moisture field should be uniform for this procedure to work well.
//
//////////////////////////////////////////////////////////////

#include "RefractCalib.hh"
#include <Refract/RefractInput.hh>
#include <Refract/ParmApp.hh>
#include <Radx/RadxTimeList.hh>
#include <Mdv/MdvxTimeList.hh>
#include <Mdv/DsMdvx.hh>
#include <dsdata/DsUrlTrigger.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/LogStreamInit.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <assert.h>

//---------------------------------------------------------------------

RefractCalib::RefractCalib(int argc, char **argv)

{

  // Initialize the okay flag.

  okay = true;
  _calib = nullptr;
  _startTime = 0;
  _endTime = 0;
  
  // Set the program name.

  _progName = "RefractCalib";
  
  // Display ucopyright message.

  ucopyright(_progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    okay = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    okay = false;
  }

  // check that start and end time is set in archive mode
  
  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      okay = false;
    }
    if (_args.startTime != 0 && _args.endTime != 0) {
      _startTime = _args.startTime;
      _endTime = _args.endTime;
      if (_params.debug) {
        cerr << "Using time limits from command line:" << endl;
        cerr << "  start time: " << DateTime::strm(_startTime) << endl;
        cerr << "  end time: " << DateTime::strm(_endTime) << endl;
      }
    } else {
      _startTime = DateTime::parseDateTime(_params.start_time);
      _endTime = DateTime::parseDateTime(_params.end_time);
      if (_params.debug) {
        cerr << "Using time limits from param file:" << endl;
        cerr << "  start time: " << DateTime::strm(_startTime) << endl;
        cerr << "  end time: " << DateTime::strm(_endTime) << endl;
      }
    }
  }

  // create _calib object

  _calib = new Calib(_params);

  // initialize logging
  LogStreamInit::init(false, false, true, true);
  LOG_STREAM_DISABLE(LogStream::WARNING);
  LOG_STREAM_DISABLE(LogStream::DEBUG);
  LOG_STREAM_DISABLE(LogStream::DEBUG_VERBOSE);
  LOG_STREAM_DISABLE(LogStream::DEBUG_EXTRA);
  if (_params.debug >= Params::DEBUG_EXTRA) {
    LOG_STREAM_ENABLE(LogStream::DEBUG_EXTRA);
    LOG_STREAM_ENABLE(LogStream::DEBUG_VERBOSE);
    LOG_STREAM_ENABLE(LogStream::DEBUG);
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    LOG_STREAM_ENABLE(LogStream::DEBUG_VERBOSE);
    LOG_STREAM_ENABLE(LogStream::DEBUG);
  } else if (_params.debug) {
    LOG_STREAM_ENABLE(LogStream::DEBUG);
    LOG_STREAM_ENABLE(LogStream::WARNING);
  }

}

//---------------------------------------------------------------------
RefractCalib::~RefractCalib()
{

}

//---------------------------------------------------------------------
int RefractCalib::run()
{

  // get vector list of files

  vector<string> fileList;
  
  if (_params.mode == Params::FILELIST) {

    fileList = _args.inputFileList;

  } else {

    // get the files to be processed
    
    RadxTimeList tlist;
    tlist.setDir(_params.input_dir);
    tlist.setModeInterval(_startTime, _endTime);
    if (tlist.compile()) {
      cerr << "ERROR - RadxConvert::_runArchive()" << endl;
      cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
      cerr << "  Start time: " << RadxTime::strm(_startTime) << endl;
      cerr << "  End time: " << RadxTime::strm(_endTime) << endl;
      cerr << tlist.getErrStr() << endl;
      return -1;
    }

    fileList = tlist.getPathList();

  }

  if (fileList.empty()) {
    cerr << "ERROR - RefractCalib" << endl;
    cerr << "  No files found" << endl;
    return -1;
  }
    
  
  // find suitable targets
  // sets input_gate_spacing
  
  double input_gate_spacing;
  if (!_calib->findReliableTargets(fileList,
                                   input_gate_spacing)) {
    return -1;
  }
  
  // perform the calibration step
  
  if (!_calib->calibTargets(fileList,
                            input_gate_spacing)) {
    return -1;
  }
  
  return 0;
  
}

