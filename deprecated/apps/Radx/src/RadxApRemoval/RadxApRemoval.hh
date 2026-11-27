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
////////////////////////////////////////////////////////////////////
// RadxApRemoval.hh
//
// RadxApRemoval object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2014
//
////////////////////////////////////////////////////////////////////
//
// Reads a CfRadial volume, filters AP, writes out filtered volume
//
////////////////////////////////////////////////////////////////////

#ifndef RadxApRemoval_HH
#define RadxApRemoval_HH

#include "Args.hh"
#include "Params.hh"
#include <string>

#include <toolsa/MsgLog.hh>

#include "Feature.hh"
#include "FilterBeamInfo.hh"
#include "InterestFunction.hh"
#include "Params.hh"
#include "RadarTilt.hh"
#include "TerrainMask.hh"

#include "GTFilter.hh"
#include "LTFilter.hh"

class RadxVol;
class RadxFile;
using namespace std;

class RadxApRemoval {
  
public:

  // constructor
  
  RadxApRemoval(int argc, char **argv);

  // destructor
  
  ~RadxApRemoval();

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
  vector<string> _readPaths;

  int _runFilelist();
  int _runArchive();
  int _runRealtimeWithLdata();
  int _runRealtimeNoLdata();
  int _processFile(const string &filePath);
  void _setupRead(RadxFile &file);
  void _convertFields(RadxVol &vol);
  void _convertAllFields(RadxVol &vol);
  void _setupWrite(RadxFile &file);
  int _writeVol(RadxVol &vol, const string &readPath);
  int _writeVolMdv(RadxVol &vol, const string &readPath);


};

#endif
