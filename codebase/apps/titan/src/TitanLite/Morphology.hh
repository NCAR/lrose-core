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
// Morphology.hh
//
// Morphology class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef Morphology_HH
#define Morphology_HH

#include <euclid/clump.h>
#include <euclid/boundary.h>
#include <euclid/point.h>
#include <euclid/node.h>
#include <dataport/port_types.h>

#include "Worker.hh"
using namespace std;
class InputMdv;
class GridClump;
class Clumping;

////////////////////////////////
// Morphology

class Morphology : public Worker {
  
public:

  // constructor

  Morphology(const string &prog_name, const Params &params,
	     const InputMdv &input_mdv);

  // destructor
  
  virtual ~Morphology();

  // Prepare based on input MDV file

  void prepare();

  // Compute the morphology, set the sub-clumps
  // Returns the number of sub-clumps
  
  int compute(const GridClump &grid_clump);

  // write out MDV file

  int writeOutputMdv();

  // sub clumps to be returned to calling class

  GridClump *subClumps() { return _subClumps; }

protected:
  
private:

  const InputMdv &_inputMdv;

  ui08 *_reflMarginFileGrid;
  ui08 *_edmFileGrid;
  ui08 *_morphFileGrid;
  ui08 *_erodedFileGrid;
  ui08 *_compFileGrid;
  int _nBytesFileGridAlloc;

  int _nxInput, _nyInput;

  int _nComp;
  int _nxWork, _nyWork;
  int _nBytesWorkGrid;

  ui08 *_reflMarginWorkGrid;
  ui08 *_edmWorkGrid;
  ui08 *_morphWorkGrid;
  ui08 *_erodedWorkGrid;
  ui08 *_compWorkGrid;
  int _nBytesWorkGridAlloc;

  ui08 *_gridMask;
  int _nBytesGridMaskAlloc;

  int _nSubClumps;
  GridClump *_subClumps;

  Clumping *_morphClumping;
  Clumping *_maskClumping;

  void _erodeProjArea(const GridClump &grid_clump);
  void _initWorkGrids();
  void _initFileGrids();
  void _fillCompDbz(const GridClump &grid_clump);
  void _updateFileGrids(const GridClump &grid_clump);
  void _initGridMask();
  ui08 *_loadGridMask(const GridClump &grid_clump);
  void _createSubClumps(const GridClump &grid_clump);

};

#endif


