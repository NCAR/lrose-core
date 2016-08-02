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
//
// GenPoly2Mdv.hh
//
//
// GenPoly2Mdv produces MDV grids from GenPoly objects.
//
///////////////////////////////////////////////////////////////

#ifndef GenPoly2Mdv_H
#define GenPoly2Mdv_H

#include <toolsa/file_io.h>
#include <string>
#include <vector>
#include <toolsa/umisc.h>
#include <cerrno>
#include <rapformats/GenPoly.hh>
#include <rapmath/math_macros.h>
#include <Fmq/NowcastQueue.hh>
#include <rapformats/GenPt.hh>
#include <Spdb/DsSpdb.hh>
#include <didss/LdataInfo.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <cstdio>

#include "Params.hh"
#include "Args.hh"
#include "MdvMgr.hh"
#include <vector>

#include <titan/DsTitan.hh> 
using namespace std;

class GenPoly2Mdv {
  
public:
  
  GenPoly2Mdv (int argc, char **argv);
  
  ~GenPoly2Mdv();

  int Run();

  bool isOK;

protected:
  
private:

  int  _run(time_t begin_time, time_t end_time);

  string _progName;
  char  *_paramsPath;
  Args   _args;
  Params _params;

};

#endif



