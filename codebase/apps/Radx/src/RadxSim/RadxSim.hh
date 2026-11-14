// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2026                                         
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
//
// RadxSim.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2026
//
///////////////////////////////////////////////////////////////
//
// RadxSim is intended to simulate Radx-type files in a realtime
// scenario.
//
// RadxSim loops through a set of polar CfRadial files,
// modifies the ray times to wall-clock time, and writes the
// modified files to an output dir.
//
///////////////////////////////////////////////////////////////

#ifndef RADX_SIM_HH
#define RADX_SIM_HH

#include "Args.hh"
#include "Params.hh"

#include <csignal>
#include <string>
#include <vector>

class RadxVol;

class RadxSim {
public:
  RadxSim(int argc, char **argv);
  ~RadxSim();

  int Run();
  void requestStop() { _stopRequested = 1; }

  int OK;

private:
  struct InputFile {
    std::string path;
    double startTime;
    double endTime;
  };

  std::string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  volatile sig_atomic_t _stopRequested;
  bool _procmapRegistered;
  std::vector<InputFile> _inputs;
  double _archiveStart;
  double _archiveEnd;
  unsigned long long _publicationSequence;

  int _validateParams() const;
  int _resolveInputs();
  int _collectCandidatePaths(std::vector<std::string> &paths) const;
  int _inspectInput(const std::string &path, InputFile &input) const;
  int _runCycles();
  int _runCycle(double dataCycleStart, double monotonicCycleStart);
  int _readVolume(const std::string &path, RadxVol &vol) const;
  int _retimeVolume(RadxVol &vol, double dataCycleStart) const;
  int _waitUntil(double deadline, const std::string &activity);
  int _publishVolume(RadxVol &vol);

  static double _wallClockNow();
  static double _monotonicNow();
  static std::string _canonicalPath(const std::string &path);
  static bool _inputLess(const InputFile &lhs, const InputFile &rhs);
};

#endif

