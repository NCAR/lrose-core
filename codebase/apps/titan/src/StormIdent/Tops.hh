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
// Tops.hh
//
// Tops class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef Tops_HH
#define Tops_HH

#include <euclid/clump.h>
#include <dataport/port_types.h>

#include "Params.hh"
using namespace std;
class InputMdv;
class Clumping;

////////////////////////////////
// Tops

class Tops {
  
public:

  enum {
    TOPS_FIELD,
    SHALLOW_FIELD,
    SHALLOW_EDM_FIELD,
    SHALLOW_MASK_FIELD,
    N_TOPS_FIELDS
  } tops_fields;
  
  // constructor

  Tops(char *prog_name, Params *params);

  // destructor
  
  virtual ~Tops();

  // Mask out areas in grid with low tops
  
  void maskLowTops(InputMdv *input_mdv);

  int OK;

protected:
  
private:

  char *_progName;
  Params *_params;

  Clumping *_clumping;

  ui08 *_topsGrid;
  ui08 *_shallowGrid;
  ui08 *_shallowEdmGrid;
  ui08 *_shallowMaskGrid;

  int _nBytesGridAlloc;

  void _allocGrids(InputMdv *input_mdv);
  int _writeOutputMdv(InputMdv *input_mdv);

};

#endif


