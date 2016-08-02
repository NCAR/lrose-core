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
// QpeVerify.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2015
//
///////////////////////////////////////////////////////////////
//
// QpeVerify reads precip accumulation measurements from SPDB,
// and compares these with radar-derived QPE values stored
// in gridded files.
//
///////////////////////////////////////////////////////////////////////

#ifndef QpeVerify_H
#define QpeVerify_H

#include <string>
#include <dataport/port_types.h>
#include <Spdb/DsSpdb.hh>
#include <Mdv/DsMdvx.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;
class WxObs;
class DsSpdb;
class MdvxField;
class MdvxProj;

////////////////////////
// This class

class QpeVerify {
  
public:

  // constructor

  QpeVerify (int argc, char **argv);

  // destructor

  ~QpeVerify();

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

  // regression

  double _nCorr;
  double _sumAccum;
  double _sumAccum2;
  double _sumQpe;
  double _sumQpe2;
  double _sumAccumQpe;

  int _runRealtime();
  int _runArchive();
  int _run(time_t startTime, time_t endTime);

  int _processTime(time_t validTime,
                   vector<const Spdb::chunk_t *> &validChunks);

  int _compareWithQpe(string stationId,
                      time_t obsTime,
                      double lat,
                      double lon,
                      double accumDepth,
                      double accumSecs,
                      MdvxField *qpeField,
                      const MdvxProj &proj);
  
};

#endif

