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

  ClumpDualThresh();
  
  // destructor
  
  virtual ~ClumpDualThresh();

  // set parameters
  // Primary and secondary thresholds are usually DBZ
  // but can be other quantities.
  // Clump volume is in km3

  void setDebug(bool val) { _debug = val; }
  void setPrimaryThreshold(double val) { _primaryThreshold = val; }
  void setSecondaryThreshold(double val) { _secondaryThreshold = val; }
  void setMinFractionAllParts(double val) { _minFractionAllParts = val; }
  void setMinFractionEachPart(double val) { _minFractionEachPart = val; }
  void setMinAreaEachPart(double val) { _minAreaEachPart = val; }
  void setMinClumpVolume(double val) { _minClumpVolume = val; }
  void setMaxClumpVolume(double val) { _maxClumpVolume = val; }

  // set the input data and grid details

  void setInputData(PjgGridGeom &inputGeom, const fl32 *inputData);

  // Compute sub-clumps using the dual threshold.
  // Returns number of sub-clumps.
  
  int compute(const ClumpGrid &clump_grid);

  // write out MDV file for debugging
  // int writeOutputMdv();

  // sub clumps to be returned to calling class

  const ClumpGrid *subClumps() { return _subClumps; }

  // get grids for debug output

  const PjgGridGeom &getGridGeom() const { return _inputGeom; }
  const fl32* getCompFileGrid() const { return _compFileGrid; }
  const ui08* getAllFileGrid() const { return _allFileGrid; }
  const ui08* getValidFileGrid() const { return _validFileGrid; }
  const ui08* getGrownFileGrid() const { return _grownFileGrid; }

protected:
  
private:

  bool _debug;

  // input data

  PjgGridGeom _inputGeom;
  const fl32 *_inputData;
  int _minValidLayer;

  size_t _nxInput;
  size_t _nyInput;
  fl32 _missing;

  // parameters

  double _primaryThreshold;
  double _secondaryThreshold;
  double _minFractionAllParts;
  double _minFractionEachPart;
  double _minAreaEachPart;
  double _minClumpVolume;
  double _maxClumpVolume;

  // clumping

  GridClumping _clumping;
  size_t _nSubClumps;
  size_t _nSubClumpsAlloc;
  ClumpGrid *_subClumps;
  GridClumping **_subClumping;

  // grids

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



