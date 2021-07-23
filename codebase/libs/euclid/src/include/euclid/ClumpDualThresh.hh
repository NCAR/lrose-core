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
// ClumpDualThresh.hh
//
// This class performs the second stage in
// multiple threshold identification.
//
// Copied over from apps/Titan/DualThresh.cc, and modified for lib use.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef ClumpDualThresh_HH
#define ClumpDualThresh_HH

#include <dataport/port_types.h>
#include <rapformats/titan_grid.h>
#include <euclid/GridClumping.hh>
#include <euclid/ClumpGrid.hh>
#include <euclid/PjgGridGeom.hh>
using namespace std;

////////////////////////////////
// ClumpDualThresh

class ClumpDualThresh {
  
public:

  // constructor

  ClumpDualThresh(bool debug,
                  PjgGridGeom &inputGeom,
                  const fl32 *inputVol,
                  int minValidLayer,
                  double primaryThreshold,
                  double secondaryThreshold,
                  double minFractionAllParts,
                  double minFractionEachPart,
                  double minAreaEachPart,
                  double minStormSize,
                  double maxStormSize);
  
  // destructor
  
  virtual ~ClumpDualThresh();
  
  // Compute sub-clumps using the dual threshold.
  // Returns number of sub-clumps.
  
  int compute(const ClumpGrid &clump_grid);

  // write out MDV file for debugging
  // int writeOutputMdv();

  // sub clumps to be returned to calling class

  const ClumpGrid *subClumps() { return _subClumps; }

protected:
  
private:

  bool _debug;

  GridClumping _clumping;
  PjgGridGeom _inputGeom;

  const fl32 *_inputVol;

  size_t _nxInput;
  size_t _nyInput;
  fl32 _missing;

  int _minValidLayer;
  double _primaryThreshold;
  double _secondaryThreshold;
  double _minFractionAllParts;
  double _minFractionEachPart;
  double _minAreaEachPart;

  double _minStormSize;
  double _maxStormSize;

  size_t _nSubClumps;
  size_t _nSubClumpsAlloc;
  ClumpGrid *_subClumps;
  GridClumping **_subClumping;
  
  size_t _nComp;

  size_t _nxWork, _nyWork;
  size_t _nPtsWorkGrid;

  fl32* _compWorkGrid;
  ui08* _allWorkGrid;
  ui08* _validWorkGrid;
  ui08* _grownWorkGrid;
  size_t _nPtsWorkGridAlloc;

  fl32* _compFileGrid;
  ui08* _allFileGrid;
  ui08* _validFileGrid;
  ui08* _grownFileGrid;
  size_t _nPtsFileGridAlloc;

  int *_cIndex;
  size_t _cIndexAlloc;

  ui08 *_gridMask;
  size_t _nPtsGridMaskAlloc;

  void _initWorkGrids();
  void _initFileGrids();
  void _fillComposite(const ClumpGrid &clump_grid);
  void _updateFileGrids(const ClumpGrid &clump_grid);
  void _updateFileComp(const ClumpGrid &clump_grid);
  void _loadCompIndex();
  void _growSubAreas();
  void _allocSubClumps();
  void _computeSubClump(const ClumpGrid &clump_grid, int clump_id);
  void _initGridMask();
  void _loadGridMask(const ClumpGrid &clump_grid, int clump_id);

};

#endif



