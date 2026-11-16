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
//
// SpdbSim.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2026
//
///////////////////////////////////////////////////////////////
//
// SpdbSim is intended to simulate Spdb-type data in a realtime
// scenario.
//
// SpdbSim loops through a set of specified times in Spdb data,
// modifies the times to wall-clock time, and writes the
// modified chunks to an spdb output dir.
//
///////////////////////////////////////////////////////////////

#ifndef SPDB_SIM_HH
#define SPDB_SIM_HH

#include "Args.hh"
#include "Params.hh"

#include <Spdb/DsSpdb.hh>
#include <csignal>
#include <string>
#include <vector>

class SpdbSim {
public:
  SpdbSim(int argc, char **argv);
  ~SpdbSim();

  int Run();
  void requestStop() { _stopRequested = 1; }

  int OK;

private:
  struct ReplayChunk {
    Spdb::chunk_t chunk;
  };

  std::string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  volatile sig_atomic_t _stopRequested;
  bool _procmapRegistered;
  // chunk_t owns no payload memory.  Keep the reader alive for the full replay
  // because every ReplayChunk below points into its returned chunk storage.
  DsSpdb _input;
  std::vector<ReplayChunk> _chunks;
  int _productId;
  std::string _productLabel;
  time_t _archiveStart;
  time_t _archiveEnd;

  int _validateParams() const;
  int _readAndPreflight();
  int _preflightChunk(const Spdb::chunk_t &chunk) const;
  int _preflightCycle(time_t dataCycleStart) const;
  int _runCycles();
  int _runCycle(time_t dataCycleStart, double deadlineCycleStart);
  int _publishGroup(size_t begin, size_t end, time_t dataCycleStart);
  int _waitUntil(double deadline, const std::string &activity);
  time_t _mappedValid(const Spdb::chunk_t &chunk,
                      time_t dataCycleStart) const;
  Spdb::compression_t _outputCompression(
      const Spdb::chunk_t &chunk) const;

  static bool _chunkLess(const ReplayChunk &lhs, const ReplayChunk &rhs);
  static bool _fitsSpdbTime(int64_t value);
  static double _monotonicNow();
};

#endif

