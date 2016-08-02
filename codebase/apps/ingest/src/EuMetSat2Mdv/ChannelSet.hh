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
// ChannelSet.hh
//
// Container for set of channels.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
/////////////////////////////////////////////////////////////

#ifndef ChannelSet_HH
#define ChannelSet_HH

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "Channel.hh"
#include "Params.hh"

using namespace std;

class ChannelSet {
  
public:
  
  static const int nChannels = 12;

  // constructor
  
  ChannelSet(const string &prog_name,
             const Params &params);
  
  // Destructor
  
  virtual ~ChannelSet();
  
  // did object constuct OK?

  bool OK() { return _OK; }

  // get a channel based on the channel number
  // returns NULL on failure

  Channel *getChannel(int num) const;

  // get a channel based on the name
  // returns NULL on failure

  const Channel *getChannel(const string &name) const;

  /////////////////////////////////////////
  // Update calibration
  // Vectors must have length of nChannels

  int updateCalibration(const vector<double> &slope,
                        const vector<double> &offset);

  // Read new calib file
  // returns 0 on success, -1 on failure
  
  int readNewCalibFile(const char *calibPath);

private:

  typedef map<string, Channel*, less<string> > ChannelMap;
  typedef pair<string, Channel* > ChannelMapEntry;

  static const char* _name[nChannels];
  static const double _waveLength[nChannels];
  static const double _waveNumber[nChannels];
  static const double _defaultCalSlope[nChannels];
  static const double _defaultCalOffset[nChannels];
  static const double _tempCorrectionScale[nChannels];
  static const double _tempCorrectionBias[nChannels];
  static const bool _isPassive[nChannels];
  static const double _solarFactor[nChannels];

  bool _OK;

  const string &_progName;
  const Params &_params;

  vector<Channel *> _channels;
  ChannelMap _map;

  time_t _lastCalibFileTime;
  
};

#endif



