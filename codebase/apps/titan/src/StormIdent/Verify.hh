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
// Verify.hh
//
// Verify class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef Verify_HH
#define Verify_HH

#include <euclid/clump.h>
#include <dataport/port_types.h>

#include "Params.hh"
using namespace std;
class InputMdv;
class GridClump;

////////////////////////////////
// Verify

class Verify {
  
public:

  enum {
    STORM_IDENT_ALL_STORMS_FIELD,
    STORM_IDENT_VALID_STORMS_FIELD,
    N_VERIFY_FIELDS
  } verify_fields;
  
  // constructor

  Verify(char *prog_name, Params *params);

  // destructor
  
  virtual ~Verify();

  // prepareGrids()
  
  void prepareGrids(InputMdv *input_mdv);

  // update the all storms grid with the given clump
  
  void updateAllStormsGrid(Clump_order *clump);

  // update the valid storms grid with the given clump
  
  void updateValidStormsGrid(GridClump *grid_clump);

  // write out MDV file

  int writeOutputMdv();

  int OK;

protected:
  
private:

  char *_progName;
  Params *_params;

  InputMdv *_inputMdv;

  ui08 *_allStormsGrid;
  ui08 *_validStormsGrid;

  int _nBytesPlaneAlloc;

};

#endif


