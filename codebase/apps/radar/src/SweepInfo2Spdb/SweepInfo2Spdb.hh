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
// SweepInfo2Spdb.hh
//
// SweepInfo2Spdb object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2006
//
///////////////////////////////////////////////////////////////
//
// SweepInfo2Spdb reads sweep data from a catalog file and
// writes the info to an SPDB data base.
//
///////////////////////////////////////////////////////////////////////

#ifndef SweepInfo2Spdb_H
#define SweepInfo2Spdb_H

#include <string>
#include <rapformats/DsRadarSweep.hh>
#include "Args.hh"
#include "Params.hh"
class DsSpdb;
using namespace std;

////////////////////////
// This class

class SweepInfo2Spdb {
  
public:

  // constructor

  SweepInfo2Spdb (int argc, char **argv);
  
  // destructor
  
  ~SweepInfo2Spdb();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  int _currentVolNum;
  int _sweepNumStartCurrentVol;

  // UDP mode
    
  bool _sweepInProgress;
  sweep_info_packet_t _currentInfo;
  sweep_info_packet_t _latestInfo;
  DsRadarSweep _currentSweep;

  // FILE mode

  long int _currentPos;
  string _currentPath;

  // shared memory

  sweep_info_packet_t *_sweepShmem;

  // functions

  int _runArchive();
  int _runRealtime();
  int _runUdp();

  int _processFile(const char *file_path,
                   long int startPos,
                   long int &endPos);

  int _processUdpInfo(const sweep_info_packet_t &info);
  int _handleNewSweep(const sweep_info_packet_t &newInfo);

  int _doPut(DsSpdb &spdb);

  void _tokenize(const string &str,
                 const string &spacer,
                 vector<string> &toks);

  int _saveStateToFile();

  int _readStateFromFile();

  int _findLatestCatPath(string &latestPath, int &fileLen);

  void _printInfo(const sweep_info_packet_t &info,
                  ostream &out);

};

#endif

