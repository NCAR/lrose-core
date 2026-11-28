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
// MM52Mdv.hh
//
// MM52Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#ifndef MM52Mdv_H
#define MM52Mdv_H

#include <toolsa/umisc.h>
#include <tdrp/tdrp.h>
#include <toolsa/ldata_info.h>

#include "Args.hh"
#include "Params.hh"
using namespace std;

class InputFile;
class OutputFile;
class FlightLevel;
class ModelGrid;

#define MISSING_DOUBLE -9999.9

class MM52Mdv {
  
public:

  // constructor

  MM52Mdv (int argc, char **argv);

  // destructor
  
  ~MM52Mdv();

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
  
  FlightLevel *_fLevel;
  InputFile *_inFile;
  OutputFile *_outFile;
  ModelGrid *_mGrid;

  int _runRealtime();

  int _runArchive();

  void _processTime(time_t forecast_time);

  void _interp3dField(char *field_name,
		      fl32 ***field_data,
		      ui08 **planes,
		      int planeOffset,
		      double scale,
		      double bias);

  void _interp2dField(char *field_name,
		      fl32 **field_data,
		      ui08 **planes,
		      int planeOffset,
		      double scale,
		      double bias);
  
};

#endif
