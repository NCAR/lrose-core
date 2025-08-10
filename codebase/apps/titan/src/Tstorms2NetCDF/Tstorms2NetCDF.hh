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
// Tstorms2NetCDF.hh
//
// Tstorms2NetCDF object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2025
//
///////////////////////////////////////////////////////////////
//
// Tstorms2NetCDF reads native TITAN binary data files,
// converts the data into NetCDF format,
// and writes the data out in NetCDF files.
//
////////////////////////////////////////////////////////////////

#ifndef Tstorms2NetCDF_H
#define Tstorms2NetCDF_H

#include <string>
#include "Args.hh"
#include "Params.hh"
#include <didss/DsInputPath.hh>
#include <titan/TitanFile.hh>

////////////////////////
// This class

class Tstorms2NetCDF {
  
public:

  // constructor

  Tstorms2NetCDF (int argc, char **argv);

  // destructor
  
  ~Tstorms2NetCDF();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  std::string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  DsInputPath *_input;
  string _inputPath;
  
  TitanFile _inFile;
  TitanFile _outFile;
  
  vector<time_t> _scanTimes;

  int _processInputFile();
  int _loadScanTimes();
  int _processScan(int scan_num,
                   time_t scan_time);
  
};

#endif

