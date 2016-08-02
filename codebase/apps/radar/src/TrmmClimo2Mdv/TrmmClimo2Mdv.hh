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
////////////////////////////////////////////////////////
// TrmmClimo2Mdv
//
// TrmmClimo2Mdv object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2011 
//
///////////////////////////////////////////////////////////////
//
// TrmmClimo2Mdv reads netCDF files for the TRMM climatology
// data set, converts these to MDV or CF-netCDF, 
// and writes them out as specified.
//
////////////////////////////////////////////////////////////////

#ifndef TrmmClimo2Mdv_H
#define TrmmClimo2Mdv_H

#include <string>
#include <netcdf.hh>
#include <didss/DsInputPath.hh>
#include <didss/LdataInfo.hh>
#include <Mdv/DsMdvx.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class TrmmClimo2Mdv {
  
public:

  // constructor

  TrmmClimo2Mdv (int argc, char **argv);
  
  // destructor
  
  ~TrmmClimo2Mdv();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const float _missingFloat;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsInputPath *_input;

  NcDim *_xDim;
  NcDim *_yDim;
  NcDim *_tDim;
  NcVar *_xVar;
  NcVar *_yVar;
  NcVar *_tVar;
  NcVar *_dataVar;

  int _processFile(const char *input_path);
  int _writeAsSingleFile();
  int _writeAsMultFiles();
  void _initMasterHeader(DsMdvx &mdvx, time_t dataTime);
  void _addField(DsMdvx &mdvx, int fieldNum, fl32 *data);
  int _writeOutput(DsMdvx &mdvx);

};

#endif

