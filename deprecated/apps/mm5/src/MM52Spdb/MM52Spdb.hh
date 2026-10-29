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
// MM52Spdb.hh
//
// MM52Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2000
//
///////////////////////////////////////////////////////////////

#ifndef MM52Spdb_H
#define MM52Spdb_H

#include <toolsa/umisc.h>
#include <tdrp/tdrp.h>
#include <string>

#include "Args.hh"
#include "Params.hh"
#include <mm5/MM5Grid.hh>
#include <rapformats/ComboPt.hh>
using namespace std;

class MM5Data;
class DsInputPath;

class MM52Spdb {
  
public:

  // constructor

  MM52Spdb (int argc, char **argv);

  // destructor
  
  ~MM52Spdb();

  // run 

  int Run();

  // data members

  int OK;

protected:
  
private:

  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  DsInputPath *_input;

  int _run();
  
  int _processForecast(MM5Data &inData,
		       time_t gen_time,
		       int lead_time,
		       time_t forecast_time);

  void _process3d(const MM5Data &inData,
		  const MM5Grid &mGrid,
		  int isig,
		  GenPt &pt);

  void _process2d(const MM5Data &inData,
		  const MM5Grid &mGrid,
		  GenPt &pt);
  
  void _initMulti(GenPt &pt,
		  const MM5Data &inData,
		  const char *name,
		  time_t valid_time,
		  double lat, double lon);
  
  void _initSingle(GenPt &pt,
		   const MM5Data &inData,
		   const char *name,
		   time_t valid_time,
		   double lat, double lon);
  
};

#endif
