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
// ClumpingDualThresh.hh
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

#ifndef ClumpingDualThresh_HH
#define ClumpingDualThresh_HH

#include <dataport/port_types.h>
#include <rapformats/titan_grid.h>
#include <euclid/PjgGridGeom.hh>
class ClumpingMgr;
class ClumpProps;
using namespace std;

////////////////////////////////
// ClumpingDualThresh

class ClumpingDualThresh {
  
public:

  // constructor

  ClumpingDualThresh();
  
  // destructor
  
  virtual ~ClumpingDualThresh();

  // set parameters
  // Primary and secondary thresholds are usually DBZ
  // but can be other quantities.
  // Clump volume is in km3

  void setDebug(bool val) { _debug = val; }
  void setPrimaryThreshold(double val) { _primaryThreshold = val; }
  void setSecondaryThreshold(double val) { _secondaryThreshold = val; }
  void setMinFractionAllParts(double val) { _minFractionAllParts = val; }
  void setMinFractionEachPart(double val) { _minFractionEachPart = val; }
  void setMinSizeEachPart(double val) { _minSizeEachPart = val; }
  void setMinClumpVolume(double val) { _minClumpVolume = val; }
  void setMaxClumpVolume(double val) { _maxClumpVolume = val; }

  // set the input data and grid details

  void setInputData(PjgGridGeom &inputGeom, const fl32 *inputData);

  // Compute sub-clumps using the dual threshold.
  // Returns number of sub-clumps.
  
  int compute(const ClumpProps &primaryClump);

  // write out MDV file for debugging
  // int writeOutputMdv();

  // sub clumps to be returned to calling class

  const ClumpProps *subClumps() { return _subClumps; }

  // get grids for debug output

  const PjgGridGeom &getGridGeom() const { return _inputGeom; }
  const fl32* getDbzCompOutputGrid() const {
    return _dbzCompOutputGrid;
  }
  const ui08* getLargeClumpsOutputGrid() const {
    return _largeClumpsOutputGrid;
  }
  const ui08* getAllSubclumpsOutputGrid() const {
    return _allSubclumpsOutputGrid;
  }
  const ui08* getValidSubclumpsOutputGrid() const {
    return _validSubclumpsOutputGrid;
  }
  const ui08* getGrownSubclumpsOutputGrid() const {
    return _grownSubclumpsOutputGrid;
  }

protected:
  
private:

  bool _debug;

  // input data

  PjgGridGeom _inputGeom;
  PjgGridGeom _workGeom;
  const fl32 *_inputData;

  size_t _nxInput;
  size_t _nyInput;
  fl32 _missing;

  // parameters

  double _primaryThreshold;
  double _secondaryThreshold;
  double _minFractionAllParts;
  double _minFractionEachPart;
  double _minSizeEachPart;
  double _minClumpVolume;
  double _maxClumpVolume;

  // clumping

  ClumpingMgr *_clumping;
  size_t _nSubClumps;
  size_t _nSubClumpsAlloc;
  ClumpProps *_subClumps;
  ClumpingMgr **_subClumping;

  // grids

  size_t _nComp;

  size_t _nxWork, _nyWork;
  size_t _nPtsWorkGrid;

  fl32* _compWorkGrid;
  ui08* _allWorkGrid;
  ui08* _validWorkGrid;
  ui08* _grownWorkGrid;
  size_t _nPtsWorkGridAlloc;

  fl32* _dbzCompOutputGrid;
  ui08* _largeClumpsOutputGrid;
  ui08* _allSubclumpsOutputGrid;
  ui08* _validSubclumpsOutputGrid;
  ui08* _grownSubclumpsOutputGrid;
  size_t _nPtsOutputGridAlloc;

  int *_cIndex;
  size_t _cIndexAlloc;

  ui08 *_gridMask;
  size_t _nPtsGridMaskAlloc;

  void _initWorkGrids(const ClumpProps &primaryClump);
  void _initOutputGrids();
  void _fillComposite(const ClumpProps &primaryClump);
  void _updateClumpGrids(const ClumpProps &primaryClump);
  void _updateSubclumpGrids(const ClumpProps &primaryClump);
  void _loadCompIndex();
  void _growSubAreas();
  void _allocSubClumps();
  void _computeSubClump(const ClumpProps &primaryClump, int clump_id);
  void _initGridMask();
  void _loadGridMask(const ClumpProps &cprops, int clump_id);

};

#endif



