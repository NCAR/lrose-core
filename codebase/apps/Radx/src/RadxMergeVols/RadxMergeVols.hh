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
// RadxMergeVols.hh
//
// RadxMergeVols object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2016
//
///////////////////////////////////////////////////////////////
//
// Merges volumes from multiple CfRadial files into a single file
//
////////////////////////////////////////////////////////////////

#ifndef RadxMergeVols_H
#define RadxMergeVols_H

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <Radx/RadxVol.hh>
class RadxFile;
using namespace std;

class RadxMergeVols {
  
public:

  // constructor
  
  RadxMergeVols (int argc, char **argv);

  // destructor
  
  ~RadxMergeVols();

  // run 

  int Run();

  // data members

  int OK;

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  
  int _serialStartIndex;
  int _serialThisIndex;
  RadxVol _mergedVol;

  int _runFilelist();
  int _runArchive();
  int _runRealtimeWithLdata();
  int _runRealtimeNoLdata();
  void _setupRead(RadxFile &file);
  void _setupWrite(RadxFile &file);
  int _writeVol(RadxVol &vol);
  int _processFile(const string &primaryPath);
  int _processFileParallel(const string &primaryPath);
  int _processFileSerial(const string &serialPath);
  int _checkGeom(const RadxVol &primaryVol, const RadxVol &secondaryVol);
  int _mergeVol(RadxVol &primaryVol, const RadxVol &secondaryVol);
  
};

#endif
