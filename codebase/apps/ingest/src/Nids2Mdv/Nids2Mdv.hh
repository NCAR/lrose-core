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
// Nids2Mdv.hh
//
// Nids2Mdv object
//
// Paddy McCarthy, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// After Mike Dixon
// March 2001
//
///////////////////////////////////////////////////////////////

#ifndef Nids2Mdv_HH
#define Nids2Mdv_HH

#include <string>
#include <Fmq/DsFmq.hh>
#include <dataport/port_types.h>
#include "Args.hh"
#include "Remap.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class Nids2Mdv {
  
public:

  // constructor

  Nids2Mdv (int argc, char **argv);

  // destructor
  
  ~Nids2Mdv();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  DsFmq * _postProcessFmq;
  vector<Remap *> _remapObjects;
  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  int _runRealtime ();
  int _runArchive ();

  int _createRemapObject(const string & inputDir,
                         const string & radarName,
                         const string & outputDir);

};

#endif

