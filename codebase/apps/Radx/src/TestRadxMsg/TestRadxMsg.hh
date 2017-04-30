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
// TestRadxMsg.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2017
//
///////////////////////////////////////////////////////////////
//
// TestRadxMsg test the serialization and deserialization of Radx
// objects, using RadxMsg.
//
////////////////////////////////////////////////////////////////

#ifndef TestRadxMsg_HH
#define TestRadxMsg_HH

#include <string>
#include <vector>
#include "Args.hh"
#include "Params.hh"
#include <Radx/RadxVol.hh>
#include <Radx/RadxFile.hh>
using namespace std;

class DsInputPath;

////////////////////////
// This class

class TestRadxMsg {
  
public:

  // constructor

  TestRadxMsg (int argc, char **argv);

  // destructor
  
  ~TestRadxMsg();

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
  DsInputPath *_input;

  RadxVol _vol;
  Radx::DataType_t _dataType;
  int _dataByteWidth;
  vector<string> _readPaths;
  
  int _processFile(const string filePath);
  void _setupRead(RadxFile &file);
  int _testRadxField();
  int _testRadxRay();
  int _testRadxVol();

};

#endif
