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
// DualThresh.hh
//
// DualThresh class
// 
// This class performs multiple stage threshold identification.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef DualThresh_HH
#define DualThresh_HH

#include "Worker.hh"
#include "Clumping.hh"

class InputMdv;
class GridClump;

#include <dataport/port_types.h>
#include <rapformats/titan_grid.h>
using namespace std;

////////////////////////////////
// DualThresh

class DualThresh : public Worker {
  
public:

  // constructor

  DualThresh(const string &prog_name, const Params &params,
	     const InputMdv &input_mdv);

  // destructor
  
  virtual ~DualThresh();

  // Prepare based on input MDV file

  void prepare();

  // Compute sub-clumps using the dual threshold.
  // Returns number of sub-clumps.
  
  int compute(const GridClump &grid_clump);

  // write out MDV file for debugging

  int writeOutputMdv();

  // sub clumps to be returned to calling class

  const GridClump *subClumps() { return _subClumps; }

protected:
  
private:

  const InputMdv &_inputMdv;
  const titan_grid_t &_inputGrid;
  Clumping _clumping;

  int _nxInput;
  int _nyInput;
  fl32 _missing;

  double _primaryDbzThreshold;
  double _secondaryDbzThreshold;
  double _minFractionAllParts;
  double _minFractionEachPart;
  double _minAreaEachPart;

  int _nSubClumps;
  int _nSubClumpsAlloc;
  GridClump *_subClumps;
  Clumping **_subClumping;
  
  int _nComp;

  int _nxWork, _nyWork;
  int _nPtsWorkGrid;

  fl32* _compWorkGrid;
  ui08* _allWorkGrid;
  ui08* _validWorkGrid;
  ui08* _grownWorkGrid;
  int _nPtsWorkGridAlloc;

  fl32* _compFileGrid;
  ui08* _allFileGrid;
  ui08* _validFileGrid;
  ui08* _grownFileGrid;
  int _nPtsFileGridAlloc;

  int *_cIndex;
  int _cIndexAlloc;

  ui08 *_gridMask;
  int _nPtsGridMaskAlloc;

  void _initWorkGrids();
  void _initFileGrids();
  void _fillCompDbz(const GridClump &grid_clump);
  void _updateFileGrids(const GridClump &grid_clump);
  void _updateFileComp(const GridClump &grid_clump);
  void _loadCompIndex();
  void _growSubAreas();
  void _allocSubClumps();
  void _computeSubClump(const GridClump &grid_clump, int clump_id);
  void _initGridMask();
  void _loadGridMask(const GridClump &grid_clump, int clump_id);

};

#endif



