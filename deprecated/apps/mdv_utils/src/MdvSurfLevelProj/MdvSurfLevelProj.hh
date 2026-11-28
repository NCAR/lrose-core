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
// MdvSurfLevelProj.hh
//
// MdvSurfLevelProj object
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2004
//
///////////////////////////////////////////////////////////////

#ifndef MdvSurfLevelProj_H
#define MdvSurfLevelProj_H

#include <string>
#include "Args.hh"
#include "Params.hh"
#include <Mdv/DsMdvxInput.hh>
using namespace std;

////////////////////////
// This class

class MdvSurfLevelProj {
  
public:

  // constructor

  MdvSurfLevelProj (int argc, char **argv);

  // destructor
  
  ~MdvSurfLevelProj();

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
  DsMdvxInput _input;

  int _doRead(DsMdvx &inMdvx);
  int _doProjection(DsMdvx &inMdvx, DsMdvx &outMdvx);
  void createNewField(DsMdvx &outMdvx, Mdvx::field_header_t& input3Dfhdr, float *data);
  void MdvSurfLevelProj::setNewMasterHeader (const DsMdvx &inMdvx, DsMdvx &outMdvx);
  
  static const float  MdvSurfLevelProj::MISSING_DATA;
  static const float  MdvSurfLevelProj::BAD_DATA;


};

#endif
