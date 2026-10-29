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
// MM5Itfa.hh
//
// MM5Itfa object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#ifndef MM5Itfa_H
#define MM5Itfa_H

#include <mm5/ItfaIndices.hh>
#include <toolsa/umisc.h>
#include <tdrp/tdrp.h>
#include <toolsa/ldata_info.h>
#include "InputPath.hh"

#include "Args.hh"
#include "Params.hh"
using namespace std;

class MM5Data;

#define MISSING_DOUBLE -9999.9

class MM5Itfa {
  
public:

  // constructor

  MM5Itfa (int argc, char **argv);

  // destructor
  
  ~MM5Itfa();

  // run 

  int Run();

  // data members

  int OK;

protected:
  
private:

  char *_progName;
  char *_paramsPath;
  Args *_args;
  Params *_params;

  LDATA_handle_t _ldata;
  
  MM5Data *_inData;

  int _checkParams();

  int _run(InputPath &input_path);

  int _processForecast(MM5Data &inData,
		       time_t gen_time,
		       int lead_time,
		       time_t forecast_time);
  
  void _computeItfa(MM5Data &inData,
		    ItfaIndices &itfa);
  
  void _trimTurbField(ItfaIndices &itfa);

  void write_index_field(const MM5Data &inData,
			 fl32***, char*, int *);

  void write_index_field(const MM5Data &inData,
			 fl32**, char*, int *);
//  fl32 diff_quotient(fl32,fl32,fl32,fl32); 
  float diff_quotient(float,float,float,float);

};

#endif
