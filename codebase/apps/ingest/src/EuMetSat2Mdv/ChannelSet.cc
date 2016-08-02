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
//////////////////////////////////////////////////////////
// ChannelSet.cc
//
// Container for set of channels.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
//////////////////////////////////////////////////////////

#include "ChannelSet.hh"

#include <toolsa/file_io.h>
#include <cerrno>
#include <cstring>

using namespace std;

// standard channel names

const char* ChannelSet::_name[ChannelSet::nChannels] =
  { "VIS006", "VIS008", "IR_016", "IR_039", "WV_062", "WV_073",
    "IR_087", "IR_097", "IR_108", "IR_120", "IR_134", "HRV"};

const double ChannelSet::_waveLength[ChannelSet::nChannels] =
  { 0.635e-6, 0.810e-6, 1.640e-6, 3.920e-6, 6.250e-6, 7.350e-6,
    8.700e-6, 9.660e-6, 10.80e-6, 12.00e-6, 13.40e-6, 0.750e-6 }; 

const double ChannelSet::_waveNumber[ChannelSet::nChannels] =
  { 15748.031, 12345.679, 6097.561, 2569.094, 1598.566, 1362.142,
    1149.083, 1034.345, 930.659, 839.661, 752.381, 13333.33 };

const double ChannelSet::_defaultCalSlope[ChannelSet::nChannels] =
  { 0.023128, 0.029727, 0.023622, 0.003659, 0.008318, 0.038622,
    0.126744, 0.103961, 0.205034, 0.222311, 0.157607, 0.031999 };

const double ChannelSet::_defaultCalOffset[ChannelSet::nChannels] =
  { -1.179533, -1.516057, -1.204717, -0.186592, -0.424224, -1.969721,
    -6.463923, -5.302023, -10.456757, -11.337882, -8.037953, -1.631964 };

const double ChannelSet::_tempCorrectionScale[ChannelSet::nChannels] =
  { 1.0, 1.0, 1.0, 0.9959, 0.9963, 0.9991,
    0.9996, 0.9999, 0.9983, 0.9988, 0.9981, 1.0 };

const double ChannelSet::_tempCorrectionBias[ChannelSet::nChannels] =
  { 0.0, 0.0, 0.0, 3.471, 2.219, 0.485,
    0.181, 0.060, 0.627, 0.397, 0.576, 0.0 };

const bool ChannelSet::_isPassive[ChannelSet::nChannels] =
  { false, false, false, true, true, true,
    true, true, true, true, true, false };

const double ChannelSet::_solarFactor[ChannelSet::nChannels] =
  { 20.76, 23.24, 19.85, 1.0, 1.0, 1.0,
    1.0, 1.0, 1.0, 1.0, 1.0, 25.11 };

//////////////
// Constructor

ChannelSet::ChannelSet(const string &prog_name,
                       const Params &params) :
        _progName(prog_name),
        _params(params)
  
{

  _OK = true;
  _lastCalibFileTime = 0;
  
  // construct channel objects, add to vector and map

  for (int ii = 0; ii < nChannels; ii++) {
    Channel *channel =
      new Channel(ii, _name[ii], _waveLength[ii], _waveNumber[ii],
                  _defaultCalSlope[ii], _defaultCalOffset[ii],
		  _tempCorrectionScale[ii], _tempCorrectionBias[ii],
                  _isPassive[ii], _solarFactor[ii]);
    _channels.push_back(channel);
    ChannelMapEntry entry(_name[ii], channel);
    _map.insert(entry);
  } // ii

}

/////////////
// Destructor

ChannelSet::~ChannelSet()

{

  for (int ii = 0; ii < (int) _channels.size(); ii++) {
    delete _channels[ii];
  }

}

/////////////////////////////////////////
// get a channel based on the id
// returns NULL on failure

Channel *ChannelSet::getChannel(int id) const
  
{
  
  if (id > nChannels - 1) {
    return NULL;
  } else {
    return _channels[id];
  }

}

/////////////////////////////////////////
// get a channel based on the name
// returns NULL on failure

const Channel *ChannelSet::getChannel(const string &name) const

{

  ChannelMap::const_iterator it = _map.find(name);
  if (it == _map.end()) {
    return NULL;
  } else {
    return it->second;
  }

}

/////////////////////////////////////////
// Update calibration
//
// Vectors must have length of nChannels

int ChannelSet::updateCalibration(const vector<double> &slope,
                                  const vector<double> &offset)

{
  
  if ((int) slope.size() != nChannels || (int) offset.size() != nChannels) {
    cerr << "ERROR - ChannelSet::updateCalibration" << endl;
    cerr << "  slope and offset vectors must be of length: "
         << nChannels << endl;
    cerr << "  slope vector length: " << slope.size() << endl;
    cerr << "  offset vector length: " << offset.size() << endl;
    return -1;
  }

  for (int ii = 0; ii < nChannels; ii++) {
    _channels[ii]->setCalSlope(slope[ii]);
    _channels[ii]->setCalOffset(offset[ii]);
  }

  return 0;

}

/////////////////////////////////////////
// Read new calib file
//
// returns 0 on success, -1 on failure

int ChannelSet::readNewCalibFile(const char *calibPath)

{

  // check file mod time, decide if we need to read it or not

  struct stat fileStat;
  if (ta_stat(calibPath, &fileStat)) {
    int errNum = errno;
    cerr << "ERROR - ChannelSet::readNewCalibFile" << endl;
    cerr << "  Cannot stat file: " << calibPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (fileStat.st_mtime <= _lastCalibFileTime) {
    // no new cal, do not read in
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "INFO - ChannelSet::readNewCalibFile" << endl;
      cerr << "  Cal file unchanged: " << calibPath << endl;
    }
    return 0;
  }
  _lastCalibFileTime = fileStat.st_mtime;

  // open cal file

  FILE *cal;
  if ((cal = fopen(calibPath, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ChannelSet::readNewCalibFile" << endl;
    cerr << "  Cannot open file: " << calibPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read in each of nChannels lines, one per channel

  for (int ii = 0; ii < nChannels; ii++) {

    // read line

    char line[256];
    if (fgets(line, 256, cal) == NULL) {
      int errNum = errno;
      cerr << "ERROR - ChannelSet::readNewCalibFile" << endl;
      cerr << "  Cannot read file: " << calibPath << endl;
      cerr << "  " << strerror(errNum) << endl;
      fclose(cal);
      return -1;
    }

    // decode id, slope and offset

    int id;
    double slope, offset;
    if (sscanf(line, "%d %lg %lg", &id, &slope, &offset) != 3) {
      cerr << "ERROR - ChannelSet::readNewCalibFile" << endl;
      cerr << "  Cannot decode file: " << calibPath << endl;
      cerr << "  line: " << line << endl;
      fclose(cal);
      return -1;
    }

    // check id

    if (id != ii + 1) {
      cerr << "ERROR - ChannelSet::readNewCalibFile" << endl;
      cerr << "  Cannot decode file: " << calibPath << endl;
      cerr << "  Channel ID incorrect, line: " << line << endl;
      fclose(cal);
      return -1;
    }

    // set slope and offset

    _channels[ii]->setCalSlope(slope);
    _channels[ii]->setCalOffset(offset);

  } // ii

  fclose(cal);
  
  return 0;

}

