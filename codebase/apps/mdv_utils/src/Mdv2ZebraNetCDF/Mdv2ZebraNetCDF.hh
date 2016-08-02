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
// Mdv2ZebraNetCDF.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2011
//
///////////////////////////////////////////////////////////////
//
// Mdv2ZebraNetCDF converts MDV files to NetCDF format suitable
// for use in Zebra
//
///////////////////////////////////////////////////////////////

#ifndef Mdv2ZebraNetCDF_HH
#define Mdv2ZebraNetCDF_HH

#include "Args.hh"
#include "Params.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
using namespace std;

class DsInputPath;

class Mdv2ZebraNetCDF {
  
public:

  // constructor

  Mdv2ZebraNetCDF (int argc, char **argv);

  // destructor
  
  ~Mdv2ZebraNetCDF();

  // run 

  int Run();

  // data members

  bool isOK;
  
protected:
  
private:
  
  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  DsInputPath *_input;

  bool _isLatLon;

  int _processFile(const char *file_path);
  void _setupRead(DsMdvx &mdvx);
  int _remapFields(DsMdvx &mdvx);
  int _writeXYNetCDF(const DsMdvx mdvx);
  int _writeLatLonNetCDF(const DsMdvx mdvx);

};

#endif
