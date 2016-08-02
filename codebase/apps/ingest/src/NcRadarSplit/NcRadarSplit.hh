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
// NcRadarSplit.hh
//
// NcRadarSplit object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2001
//
///////////////////////////////////////////////////////////////

#ifndef NcRadarSplit_H
#define NcRadarSplit_H

#include <string>
#include <vector>
#include "Args.hh"
#include "Params.hh"
#include <didss/DsInputPath.hh>
#include <netcdf.hh>
using namespace std;

typedef struct {
  int startBeam;
  int endBeam;
  int tiltNum;
  int volNum;
} ppi_t;

////////////////////////
// This class

class NcRadarSplit {
  
public:

  // constructor

  NcRadarSplit (int argc, char **argv);

  // destructor
  
  ~NcRadarSplit();

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

  int _prevTiltNum;
  int _volNum;

  vector<ppi_t> _ppis;

  int _processFile(const char *input_path);
  int _checkFile(NcFile &ncf);

  void _findPpis(NcFile &ncf);
  void _printFile(NcFile &ncf);
  void _printAtt(NcAtt *att);
  void _printVarVals(NcVar *var);

  int _getTiltNum(double elevation);
  int _doSplit(NcFile &ncf);

};

#endif

