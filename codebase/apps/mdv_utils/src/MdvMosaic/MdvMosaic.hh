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
// MdvMosaic.hh
//
// MdvMosaic object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
///////////////////////////////////////////////////////////////

#ifndef MdvMosaic_H
#define MdvMosaic_H

#include <toolsa/umisc.h>
#include <tdrp/tdrp.h>
#include <mdv/mdv_grid.h>

#include "Args.hh"
#include "Params.hh"
using namespace std;

#define VGRID_MISSING -9999.999

typedef struct {
  float lat, lon;
} lat_lon_t;

class Trigger;
class InputFile;

class MdvMosaic {
  
public:

  // constructor

  MdvMosaic (int argc, char **argv);

  // destructor
  
  ~MdvMosaic();

  // run 

  int Run();

  // data members

  int OK;
  int Done;

protected:
  
private:

  char *_progName;
  Args *_args;
  MdvMosaic_tdrp_struct *_params;
  mdv_grid_t _outGrid;
  lat_lon_t *_locArray;

  // functions

  void _loadGrid();

  Trigger *_createTrigger();

  int _computeScaleAndBias(int out_field, int nInput, InputFile **inFiles,
			   double *scale_p, double *bias_p);

};

#endif
