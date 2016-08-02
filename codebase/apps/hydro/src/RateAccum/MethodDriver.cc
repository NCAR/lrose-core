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
// MethodDriver.cc
//
// MethodDriver object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#include "MethodDriver.hh"
#include "AccumData.hh"
#include <didss/DsInputPath.hh>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <algorithm>
using namespace std;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
// Abstract base class
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

MethodDriver::MethodDriver(const string &prog_name,
			   const Args &args, const Params &params) :
        _progName(prog_name), _args(args), _params(params), _accum(NULL)

{
  _accum = new AccumData(prog_name, params);
}

//////////////
// destructor

MethodDriver::~MethodDriver()

{
  if (_accum) {
    delete _accum;
  }
}

/////////////////////////////////////////////////////
// Load up file list, compute times and durations

void MethodDriver::_loadPathList(time_t startTime,
                                 time_t endTime)
{

  PMU_auto_register("MethodDriver::_loadPathList");

  _accumStartTime = startTime;
  _accumEndTime = endTime;
  
  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "MethodDriver::loadPathList - period: "
         << DateTime::strm(_accumStartTime) << " to "
         << DateTime::strm(_accumEndTime) << endl;
  }
  
  // get input file list
  
  DsInputPath inputFiles(_progName, _params.debug,
			 _params.input_dir,
			 _accumStartTime, _accumEndTime);

  // load up time vector

  _fileTimes.clear();
  _filePaths.clear();
  _fileDurations.clear();

  time_t prevTime = -1;
  int count = 0;
  
  char *inputPath;
  while ((inputPath = inputFiles.next()) != NULL) {
    time_t fileTime = DsInputPath::getDataTime(inputPath);
    _filePaths.push_back(inputPath);
    _fileTimes.push_back(fileTime);
    if (fileTime < prevTime) {
      cerr << "WARNING - file times out of order" << endl;
      cerr << "  File index: " << count << endl;
      cerr << "  Prev time: " << DateTime::strm(prevTime);
      cerr << "  File time: " << DateTime::strm(fileTime);
    }
    count++;
  }

  // only 1 file?
  
  if (_filePaths.size() < 2) {
    _fileDurations.push_back(_params.max_vol_duration);
    return;
  }
  
  // compute median duration
  
  vector<double> durations;
  for (size_t ii = 1; ii < _fileTimes.size(); ii++) {
    double fileDuration = (double) _fileTimes[ii] - (double) _fileTimes[ii-1];
    durations.push_back(fileDuration);
  }
  sort(durations.begin(), durations.end());
  double medianDuration = durations[durations.size() / 2];
  
  // compute durations
  
  for (size_t ii = 0; ii < _fileTimes.size(); ii++) {
    double fileDuration = medianDuration;
    if (ii != 0) {
      fileDuration = (double) _fileTimes[ii] - (double) _fileTimes[ii-1];
      if (fileDuration <= 0 ||
          fileDuration > _params.max_vol_duration) {
        fileDuration = medianDuration;
      }
    }
    _fileDurations.push_back(fileDuration);
  }

}

  
    
