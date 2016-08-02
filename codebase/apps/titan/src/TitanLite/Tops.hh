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

#include "Worker.hh"
#include "Clumping.hh"
using namespace std;
class InputMdv;

////////////////////////////////
// Tops

class Tops : public Worker {
  
public:

  // constructor

  Tops(const string &prog_name, const Params &params,
       const InputMdv &input_mdv);

  // destructor
  
  virtual ~Tops();

  // Mask out areas in grid with low tops
  
  void maskLowTops();

protected:
  
private:

  const InputMdv &_inputMdv;
  Clumping _clumping;

  ui08 *_topsGrid;
  ui08 *_shallowGrid;
  ui08 *_shallowEdmGrid;
  ui08 *_shallowMaskGrid;

  int _nBytesGridAlloc;

  void _allocGrids();
  int _writeOutputMdv();

};

#endif


