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
////////////////////////////////////////////////////////////
// Mdv2NetCDF.hh
//
// Mdv2NetCDF object
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2007
//
///////////////////////////////////////////////////////////////
//
// Mdv2NetCDF converts Mdv format to CF-1.0 compliant NetCDF files.
//
///////////////////////////////////////////////////////////////////////

#ifndef Mdv2NetCDF_H
#define Mdv2NetCDF_H

#include <string>
#include <cmath>
#include <Ncxx/Nc3File.hh>
#include <Mdv/DsMdvx.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsSpecificFcstLdataTrigger.hh>
#include <dsdata/DsTrigger.hh>
#include <dsdata/TriggerInfo.hh>
#include <Mdv/Mdv2NcfTrans.hh>
#include "Args.hh"
#include "Params.hh"

using namespace std;

////////////////////////
// This class


class Mdv2NetCDF {
  
public:

  // constructor

  Mdv2NetCDF (int argc, char **argv);

  // destructor

  ~Mdv2NetCDF();

  
  int Run();

  // public data member

  bool isOK;

protected:
   
private:

  // data members

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsTrigger *_trigger;
  
  bool _commentWasSet;
  string _comment;
  string _outputDir;

  // methods

  int _setUpTrigger();
  int _processData(time_t inputTime, int leadTime, const string filepath); 
  int _readMdv( time_t requestTime, int leadTime,
                const string filepath, DsMdvx &mdvx);
  string _computeOutputPath(const DsMdvx &mdvx);
  void _writeLdataInfo(const DsMdvx &mdvx, const string &outputPath);
  void _checkForComment(DsMdvx &mdvx);

};

#endif

