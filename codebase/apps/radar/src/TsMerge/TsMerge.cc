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
// TsMerge.cc
//
// TsMerge object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2007
//
///////////////////////////////////////////////////////////////
//
// TsMerge reads raw time-series data from 2 sets of files,
// one for clutter and one for weather. It merges them on a
// beam-by-beam basis. The intention is to provide a merged
// data set for which the clutter and weather components are known.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <iomanip>
#include <toolsa/os_config.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include "TsMerge.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

TsMerge::TsMerge(int argc, char **argv)

{

  _clutInput = NULL;
  _wxInput = NULL;
  _clutOutput = NULL;
  _wxOutput = NULL;
  _mergedOutput = NULL;
  isOK = true;
  
  // set programe name

  _progName = "TsMerge";
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
  
  // check that file lists are set
  
  if (_args.clutFileList.size() == 0) {
    cerr << "ERROR: TsMerge::TsMerge." << endl;
    cerr << "  You must use -clut to specify clutter time series files."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
    return;
  }

  if (_args.wxFileList.size() == 0) {
    cerr << "ERROR: TsMerge::TsMerge." << endl;
    cerr << "  You must use -wx to specify weather time series files."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
    return;
  }
  
    
  // initialize the data input path objects
  
  _clutInput = new Input("clutter", _params, _args.clutFileList);
  _wxInput = new Input("weather", _params, _args.wxFileList);
  _clutOutput = new Output("clutter", _params);
  _wxOutput = new Output("weather", _params);
  _mergedOutput = new Output("merged", _params);

}

//////////////////////////////////////////////////////////////////
// destructor

TsMerge::~TsMerge()

{

  if (_clutInput) {
    delete _clutInput;
  }
  
  if (_wxInput) {
    delete _wxInput;
  }

  if (_clutOutput) {
    delete _clutOutput;
  }
  
  if (_wxOutput) {
    delete _wxOutput;
  }

  if (_mergedOutput) {
    delete _mergedOutput;
  }

}

//////////////////////////////////////////////////
// Run

int TsMerge::Run ()
{

  // read through clutter data to find starting azimuth
  // this positions the clutter data stream at the correct azimuth

  if (_params.debug) {
    cerr << "Clutter data, searching for start az: "
         << _params.clutter_start_az << endl;
  }
    
  bool found = false;
  // time_t clutStartTime = 0;
  // double clutStartEl = 0;
  // double clutStartAz = 0;
  double clutPrf = 0;

  while (!found) {
    Pulse *pulse = _clutInput->readNextPulse();
    if (pulse == NULL) {
      return -1;
    }
    double thisAz = pulse->getAz();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Clutter data, searching for start az "
           << _params.clutter_start_az << ", this az: "
           << thisAz << endl;
    }
    if (fabs(thisAz - _params.clutter_start_az) < 0.1) {
      found = true;
      // clutStartTime = pulse->getTime();
      // clutStartEl = pulse->getEl();
      // clutStartAz = pulse->getAz();
      clutPrf = pulse->getPrf();
    }
    delete pulse;
  }

  // read through weather data to find starting azimuth
  // this positions the weather data stream at the correct azimuth
  
  if (_params.debug) {
    cerr << "Weather data, searching for start az: "
         << _params.weather_start_az << endl;
  }
    
  // time_t wxStartTime = 0;
  // double wxStartEl = 0;
  // double wxStartAz = 0;
  double wxPrf = 0;

  found = false;
  while (!found) {
    Pulse *pulse = _wxInput->readNextPulse();
    if (pulse == NULL) {
      return -1;
    }
    double thisAz = pulse->getAz();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Weather data, searching for start az "
           << _params.weather_start_az << ", this az: "
           << thisAz << endl;
    }
    if (fabs(thisAz - _params.weather_start_az) < 0.1) {
      found = true;
      // wxStartTime = pulse->getTime();
      // wxStartEl = pulse->getEl();
      // wxStartAz = pulse->getAz();
      wxPrf = pulse->getPrf();
    }
    delete pulse;
  }

  // check the PRF

  if (fabs(clutPrf - wxPrf) > 0.01) {
    cerr << "ERROR - TsMerge" << endl;
    cerr << "  PRFs do not match" << endl;
    cerr << "  Clutter PRF: " << clutPrf << endl;
    cerr << "  Weather PRF: " << wxPrf << endl;
    return -1;
  }

  // compute antenna movement per pulse
  
  double degPerSec = _params.antenna_speed;
  double prt = 1.0 / clutPrf;
  double azPerPulse = degPerSec * prt;

  /////////////////////
  // open output files
  // set clutter time 60 secs before weather, merged 60 secs
  // after weather

  DateTime otime(_params.output_time.year,
                 _params.output_time.month,
                 _params.output_time.day,
                 _params.output_time.hour,
                 _params.output_time.min,
                 _params.output_time.sec);
  
  time_t clutOutputTime = otime.utime();
  time_t wxOutputTime = clutOutputTime + _params.output_time_delta;
  time_t mergedOutputTime = wxOutputTime + _params.output_time_delta;

  if (_clutOutput->openFile(clutOutputTime,
                            _params.output_elevation, 0.0)) {
    return -1;
  }
  if (_wxOutput->openFile(wxOutputTime,
                          _params.output_elevation, 0.0)) {
    return -1;
  }

  if (_mergedOutput->openFile(mergedOutputTime,
                              _params.output_elevation, 0.0)) {
    return -1;
  }

  // write ops info to output files

  if (_clutOutput->writeInfo(*_clutInput->getOpsInfo())) {
    return -1;
  }
  if (_wxOutput->writeInfo(*_wxInput->getOpsInfo())) {
    return -1;
  }
  if (_mergedOutput->writeInfo(*_wxInput->getOpsInfo())) {
    return -1;
  }

  // read in clutter and weather data together

  if (_params.debug) {
    cerr << "Combining clutter data and weather data" << endl;
  }
    
  double azimuth = 0.0;
  double deltaTime = 0.0;
  while (azimuth < _params.sector_width) {
    
    Pulse *clutPulse = _clutInput->readNextPulse();
    if (clutPulse == NULL) {
      cerr << "WARNING - TsMerge" << endl;
      cerr << "  End of clutter data" << endl;
      cerr << "  azimith: " << azimuth;
      return -1;
    }
    
    Pulse *wxPulse = _wxInput->readNextPulse();
    if (wxPulse == NULL) {
      cerr << "WARNING - TsMerge" << endl;
      cerr << "  End of weather data" << endl;
      cerr << "  azimuth: " << azimuth;
      delete clutPulse;
      return -1;
    }
    
    azimuth += azPerPulse;
    deltaTime += prt;
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "clutter az, wx az, azimuth: "
           << clutPulse->getAz() << ", "
           << wxPulse->getAz() << ", "
           << azimuth << endl;
    }
    
    // compute azimuth

    int iAz = (int) ((azimuth / 360.0) * 65535.0);
    int iEl = (int) ((_params.output_elevation / 360.0) * 65535.0);

    // set the azimuths and elevations
    
    clutPulse->iAz = iAz;
    wxPulse->iAz = iAz;
    clutPulse->iEl = iEl;
    wxPulse->iEl = iEl;

    // compute times

    int secsElapsed = (int) deltaTime;
    int msecsElapsed = (int) ((deltaTime - secsElapsed) * 1000);
    
    // scale clutter as requested

    if (_params.clutter_reduction_db > 0) {
      clutPulse->reduceIq(_params.clutter_reduction_db);
    }
    clutPulse->packIq();

    // write pulse headers for weather and clutter
    
    clutPulse->iTimeUTC = clutOutputTime + secsElapsed;
    clutPulse->iMSecUTC = msecsElapsed;
    if (_clutOutput->writePulseHeader(*clutPulse)) {
      return -1;
    }

    wxPulse->iTimeUTC = wxOutputTime + secsElapsed;
    wxPulse->iMSecUTC = msecsElapsed;
    if (_wxOutput->writePulseHeader(*wxPulse)) {
      return -1;
    }

    // write pulse data for weather and clutter

    if (_clutOutput->writeIQ(clutPulse->getPacked(),
                             clutPulse->getNIq() * sizeof(si16))) {
      return -1;
    }
    if (_wxOutput->writeIQ(wxPulse->getPacked(),
                           wxPulse->getNIq() * sizeof(si16))) {
      return -1;
    }
    
    // add clutter and weather powers to create merged result,
    // using the wx pulse to store the data

    wxPulse->addIq(clutPulse->getIq(), clutPulse->getNIq(), 0);

    // pack data for output

    wxPulse->packIq();

    // set merged time

    wxPulse->iTimeUTC = mergedOutputTime + secsElapsed;
    
    // write pulse data for merged data
    
    if (_mergedOutput->writePulseHeader(*wxPulse)) {
      return -1;
    }
    
    // write merged data
    
    if (_mergedOutput->writeIQ(wxPulse->getPacked(),
                               wxPulse->getNIq() * sizeof(si16))) {
      return -1;
    }
    
    delete clutPulse;
    delete wxPulse;

  }

  return 0;

}

