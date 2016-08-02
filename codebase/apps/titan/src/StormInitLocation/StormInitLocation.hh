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
// StormInitLocation.h
//
// StormInitLocation object
//
// A Person, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2001
//
///////////////////////////////////////////////////////////////
//
// StormInitLocation produces SPDB output from TITAN binary files.
//
///////////////////////////////////////////////////////////////

#ifndef StormInitLocation_H
#define StormInitLocation_H

#include "Params.hh"

#include <vector>

#include <titan/DsTitan.hh> 
using namespace std;

class StormInitLocation {
  
public:
  
  // constructor
  
  StormInitLocation (Params *TDRP_params);
  
  // destructor
  
  ~StormInitLocation();

  // run 

  int Run(time_t start, time_t end, bool archiveMode);
  
protected:
  
private:

  Params *_params;
  int  _entryInIntVector(vector< int >& V, int entry);
  void _processSimpleTrack(int simpleTrackNum);
  DsTitan _titan;
  vector<int> _alreadyProcessed;
  time_t _finishTime, _start, _end;
  int isCloseToRadar(double lat, double lon);
  double _area;
  int _numWrittenThisRun;
  void _writeDummy();

};

#endif
