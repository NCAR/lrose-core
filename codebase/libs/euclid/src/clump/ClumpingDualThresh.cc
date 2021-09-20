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
// ClumpingDualThresh.cc
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

#include <toolsa/umisc.h>
#include <euclid/ClumpingDualThresh.hh>
#include <vector>
#include <iostream>
using namespace std;

///////////////////////////////////
// constructor - recursion level 0
//

ClumpingDualThresh::ClumpingDualThresh() :
        _debug(false),
        _inputData(NULL),
        _minValidLayer(0),
        _primaryThreshold(35.0),
        _secondaryThreshold(45.0),
        _minFractionAllParts(0.5),
        _minFractionEachPart(0.05),
        _minAreaEachPart(20.0),
        _minClumpVolume(30.0),
        _maxClumpVolume(1.0e9),
        _clumping()

{

  _missing = -9999.0;

  _compWorkGrid = NULL;
  _allWorkGrid = NULL;
  _validWorkGrid = NULL;
  _grownWorkGrid = NULL;
  _nPtsWorkGridAlloc = 0;

  _compFileGrid = NULL;
  _allFileGrid = NULL;
  _validFileGrid = NULL;
  _grownFileGrid = NULL;
  _nPtsFileGridAlloc = 0;

  _cIndex = NULL;
  _cIndexAlloc = 0;

  // sub-clumps

  _nSubClumps = 0;
  _nSubClumpsAlloc = 0;
  _subClumps = NULL;
  _subClumping = NULL;
  
  // grid mask

  _gridMask = NULL;
  _nPtsGridMaskAlloc = 0;

}

/////////////
// destructor
//

ClumpingDualThresh::~ClumpingDualThresh()

{

  if (_compWorkGrid) {
    ufree(_compWorkGrid);
  }

  if (_allWorkGrid) {
    ufree(_allWorkGrid);
  }

  if (_validWorkGrid) {
    ufree(_validWorkGrid);
  }

  if (_grownWorkGrid) {
    ufree(_grownWorkGrid);
  }

  if (_compFileGrid) {
    ufree(_compFileGrid);
  }

  if (_allFileGrid) {
    ufree(_allFileGrid);
  }

  if (_validFileGrid) {
    ufree(_validFileGrid);
  }

  if (_grownFileGrid) {
    ufree(_grownFileGrid);
  }

  if (_cIndex) {
    ufree(_cIndex);
  }

  if (_subClumps) {
    delete[](_subClumps);
  }

  if (_subClumping) {
    for (size_t i = 0; i < _nSubClumpsAlloc; i++) {
      delete(_subClumping[i]);
    }
    ufree(_subClumping);
  }

  if (_gridMask) {
    ufree(_gridMask);
  }

}

////////////////////////////////////////////
// set the input data and grid details

void ClumpingDualThresh::setInputData(PjgGridGeom &inputGeom,
                                      const fl32 *inputData)

{

  _inputGeom = inputGeom;
  _inputData = inputData;

  _nxInput = _inputGeom.nx();
  _nyInput = _inputGeom.ny();
  _initFileGrids();
  
}


////////////////////////////////////////////
// compute()
//
// Compute clumps based on dual threshold, set the sub-clumps
//
// Returns number of sub-clumps.
//

int ClumpingDualThresh::compute(const ClumpGrid &clump_grid)

{

  if (_debug) {
    fprintf(stderr, "Computing using secondary threshold\n");
  }

  // check size
  
  if (clump_grid.clumpSize < _minClumpVolume ||
      clump_grid.clumpSize > _maxClumpVolume) {
     cerr << "222222222222 clump_grid.clumpSize, "
          << "_minClumpVolume, _maxClumpVolume: "
          << clump_grid.clumpSize << ", "
          << _minClumpVolume << ", "
          << _maxClumpVolume << endl;
      
    return (0);
  }
    
  // work grids have a spare row and col around them to make the
  // computations more efficient. This strategy means we do not
  // have to check for edge conditions

  _nxWork = clump_grid.nX + 2;
  _nyWork = clump_grid.nY + 2;
  _nPtsWorkGrid = _nxWork * _nyWork;

  // initialize the working grids

  _initWorkGrids();

  // fill out the composite reflectivity grid

  _fillComposite(clump_grid);

  // clump composite grid at the dual threshold
  
  int nSecondary =
    _clumping.performClumping(_nxWork, _nyWork, 1,
			      _compWorkGrid, 1,
			      _secondaryThreshold);
  
  // load up all clumps grid for debugging

  for (int i = 0; i < nSecondary; i++) {
    const Clump_order *clump =  _clumping.getClumps() + i;
    for (int j = 0; j < clump->size; j++) {
      Interval *intv = clump->ptr[j];
      int offset = intv->row_in_plane * _nxWork + intv->begin;
      memset(_allWorkGrid + offset, i + 1, intv->len);
    } // j
  } // i
  _updateFileComp(clump_grid);

  // loop through the clumps, determining if they are large
  // enough to be sub-clumps

  double sizeOuter = (double) _nComp;
  double sumSize = 0.0;
  int nLargeEnough = 0;

  vector<int> valid;

  for (int i = 0; i < nSecondary; i++) {
    const Clump_order *clump =  _clumping.getClumps() + i;
    ClumpGrid gridClump;
    gridClump.init(clump,
                   clump_grid.gridGeom,
		   clump_grid.startIx, clump_grid.startIy);
    double thisSize = gridClump.nPoints;
    sumSize += thisSize;
    double fractionThisPart = thisSize / sizeOuter;
    if (fractionThisPart >= _minFractionEachPart &&
	gridClump.clumpSize >= _minAreaEachPart) {
      nLargeEnough++;
      valid.push_back(TRUE);
    } else {
      valid.push_back(FALSE);
    }
  }
  double fractionAllParts = sumSize / sizeOuter;
  
  cerr << "333333 sumSize, sizeOuter, fractionAllParts, _minFractionAllParts, nLargeEnough: "
       << sumSize << ", "
       << sizeOuter << ", "
       << fractionAllParts << ", "
       << _minFractionAllParts << ", "
       << nLargeEnough << endl;
    
  if (fractionAllParts < _minFractionAllParts ||
      nLargeEnough < 2) {

    // the parts are not large or numerous enough to make this
    // procedure valid, so  just use the input clump
    
    _nSubClumps = 1;
    _allocSubClumps();
    _subClumps[0] = clump_grid;
    return (1);

  }

  // allocate the sub clump memory

  _nSubClumps = nLargeEnough;
  _allocSubClumps();

  // for each valid clump, copy the clump ids to the valid grid

  if (_debug) {
    fprintf(stderr,
	    "nSecondary, fractionAllParts, nLargeEnough: %d, %g, %d\n",
	    nSecondary, fractionAllParts, nLargeEnough);
  }

  int n = 0;
  for (int i = 0; i < nSecondary; i++) {
    if (valid[i]) {
      n++;
      const Clump_order *clump =  _clumping.getClumps() + i;
      for (int j = 0; j < clump->size; j++) {
	Interval *intv = clump->ptr[j];
	int offset = intv->row_in_plane * _nxWork + intv->begin;
	memset(_validWorkGrid + offset, n, intv->len);
      } // j
    } // if (valid[i])
  } // i

  // load up array index array for points in the composite clump

  _loadCompIndex();

  // 'grow' the secondary threshold clumps out to the original
  // clump boundary - where they meet each other they stop growing

  _growSubAreas();

  // update the file grids for debugging

  _updateFileGrids(clump_grid);

  // compute each sub clump

  for (size_t i = 0; i < _nSubClumps; i++) {
    _computeSubClump(clump_grid, i+1);
  }
  
  cerr << "1111111111111111111 nSubClumps: " << _nSubClumps << endl;
  return (_nSubClumps);

}

///////////////////
// _fillCompDbz()
//
// Fill the composite dbz array from 3D array.
//

void ClumpingDualThresh::_fillComposite(const ClumpGrid &clump_grid)

{

  // compute the composite reflectivity grid - this is the
  // max reflectivity at any height

  int nPtsPlane = _nxInput * _nyInput;

  for (int intv = 0; intv < clump_grid.nIntervals; intv++) {

    const Interval &intvl = clump_grid.intervals[intv];

    int iplane = intvl.plane + _minValidLayer;
    int iy = intvl.row_in_plane + clump_grid.startIy;
    int ix = intvl.begin + clump_grid.startIx;
    
    const fl32 *dbz =
      _inputData + (iplane * nPtsPlane) + (iy * _nxInput) + ix;

    fl32* comp =
      _compWorkGrid + (intvl.row_in_plane + 1) * _nxWork + intvl.begin + 1;

    for (int i = 0; i < intvl.len; i++, dbz++, comp++) {
      if (*dbz > *comp) {
   	*comp = *dbz;
      }
    } // i

  } // intvl

  // compute number of non-missing points in composite

  _nComp = 0;
  fl32 *comp = _compWorkGrid;
  for (size_t iy = 0; iy < _nyWork; iy++) {
    for (size_t ix = 0; ix < _nxWork; ix++, comp++) {
      if (*comp != _missing) {
        _nComp++;
      }
    } // ix
  } // iy
    
}

///////////////////
// _initWorkGrids()
//
// Allocate and initialize work grids
//

void ClumpingDualThresh::_initWorkGrids()
  
{
  
  if (_nPtsWorkGridAlloc < _nPtsWorkGrid) {
    _compWorkGrid = (fl32 *) urealloc(_compWorkGrid, _nPtsWorkGrid * sizeof(fl32));
    _allWorkGrid = (ui08 *) urealloc(_allWorkGrid, _nPtsWorkGrid);
    _validWorkGrid = (ui08 *) urealloc(_validWorkGrid, _nPtsWorkGrid);
    _grownWorkGrid = (ui08 *) urealloc(_grownWorkGrid, _nPtsWorkGrid);
    _nPtsWorkGridAlloc = _nPtsWorkGrid;
  }

  for (size_t ii = 0; ii < _nPtsWorkGrid; ii++) {
    _compWorkGrid[ii] = _missing;
  }

  memset(_allWorkGrid, 0, _nPtsWorkGrid);
  memset(_validWorkGrid, 0, _nPtsWorkGrid);
  memset(_grownWorkGrid, 0, _nPtsWorkGrid);

}

/////////////////////
// _initFileGrids()
//
// Allocate and initialize grids for MDV output file
//

void ClumpingDualThresh::_initFileGrids()
  
{

  size_t npts = _nxInput * _nyInput * sizeof(ui08);
  
  // adjust grid allocation as necessary
  
  if (_nPtsFileGridAlloc < npts) {

    _compFileGrid = (fl32 *) urealloc (_compFileGrid, npts * sizeof(fl32));
    _allFileGrid = (ui08 *) urealloc (_allFileGrid, npts);
    _validFileGrid = (ui08 *) urealloc (_validFileGrid, npts);
    _grownFileGrid = (ui08 *) urealloc (_grownFileGrid, npts);

    _nPtsFileGridAlloc = npts;

  }

  // initialize grids
  
  for (size_t ii = 0; ii < npts; ii++) {
    _compFileGrid[ii] = _missing;
  }
  memset(_allFileGrid, 0, npts);
  memset(_validFileGrid, 0, npts);
  memset(_grownFileGrid, 0, npts);

}

//////////////////////
// _updateFileComp()
//
// Update the composite grid in the output file
//

void ClumpingDualThresh::_updateFileComp(const ClumpGrid &clump_grid)
  
{
  
  ui08 *all = _allWorkGrid + _nxWork + 1;
  fl32 *comp = _compWorkGrid + _nxWork + 1;
  
  int offset =
    clump_grid.startIy * _nxInput + clump_grid.startIx;
  int missed = _nxInput - (_nxWork - 2);
  
  for (size_t iy = 1; iy < _nyWork - 1;
       iy++, offset += missed, comp += 2, all += 2) {
    
    for (size_t ix = 1; ix < _nxWork - 1; ix++, offset++, comp++, all++) {
      
      if (*comp != _missing) {
	_compFileGrid[offset] = *comp;
      }

      if (*all) {
	_allFileGrid[offset] = *all;
      }

    } // ix
    
  } // iy

}

//////////////////////
// _updateFileGrids()
//
// Update the ident grids in the output file
//

void ClumpingDualThresh::_updateFileGrids(const ClumpGrid &clump_grid)
  
{
  
  ui08 *valid = _validWorkGrid + _nxWork + 1;
  ui08 *grown = _grownWorkGrid + _nxWork + 1;
  
  int offset =
    clump_grid.startIy * _nxInput + clump_grid.startIx;
  int missed = _nxInput - (_nxWork - 2);
  
  for (size_t iy = 1; iy < _nyWork - 1;
       iy++, offset += missed, valid += 2, grown += 2) {
    
    for (size_t ix = 1; ix < _nxWork - 1;
	 ix++, offset++, valid++, grown++) {
      
      if (*valid) {
	_validFileGrid[offset] = *valid;
      }

      if (*grown) {
	_grownFileGrid[offset] = *grown;
      }

    } // ix
    
  } // iy

}

////////////////////////////////////
// _loadCompIndex()
//
// Loads up array of indices for points in the composite.
//

void ClumpingDualThresh::_loadCompIndex()

{

  // check memory allocation

  if (_nComp > _cIndexAlloc) {
    _cIndex = (int *) urealloc (_cIndex, _nComp * sizeof(int));
    _cIndexAlloc = _nComp;
  }

  // load up array index array for points in the composite clump
  
  fl32 *comp = _compWorkGrid;
  int index = 0;
  int count = 0;
  
  for (size_t iy = 0; iy < _nyWork; iy++) {
    for (size_t ix = 0; ix < _nxWork; ix++, index++, comp++) {
      if (*comp != _missing) {
	_cIndex[count] = index;
	count++;
      }
    } // ix
  } // iy

  // mix up the indices in a pseudo-random way

#ifndef RAND_MAX
#define RAND_MAX 2000000000
#endif

  srand(1);
  for (size_t i = 0; i < _nComp; i++) {
    int ix1 = (int) (((double) rand() / RAND_MAX) * _nComp);
    int ix2 = (int) (((double) rand() / RAND_MAX) * _nComp);
    int tmp = _cIndex[ix1];
    _cIndex[ix1] = _cIndex[ix2];
    _cIndex[ix2] = tmp;
  }

}

////////////////////////////////////
// _growSubAreas()
//
// Grow the sub areas back to the original clump boundary.
// Where the growing areas meet, they stop. The areas
// grow in the direction of the highest dBZ value from each edge
// point.
//

void ClumpingDualThresh::_growSubAreas()

{

  // copy over the valid grid into the grown grid

  memcpy(_grownWorkGrid, _validWorkGrid, _nPtsWorkGrid);
  
  // repeatedly loop through the clump, expanding the points
  // as they are found

  int changed = TRUE;
  int count = 0;
  
  while (changed) {

    changed = FALSE;
    count++;

    for (size_t i = 0; i < _nComp; i++) {

      int centerIndex = _cIndex[i];
      int compVal = 0;
      int id = -1;

      if (centerIndex >= 0 && _grownWorkGrid[centerIndex] == 0) {

	// point below

	int belowIndex = centerIndex - _nxWork;
	if (_grownWorkGrid[belowIndex]) {
	  if (_compWorkGrid[belowIndex] > compVal) {
	    compVal = _compWorkGrid[belowIndex];
	    id = _grownWorkGrid[belowIndex];
	  }
	}

	// point left

	int leftIndex = centerIndex - 1;
	if (_grownWorkGrid[leftIndex]) {
	  if (_compWorkGrid[leftIndex] > compVal) {
	    compVal = _compWorkGrid[leftIndex];
	    id = _grownWorkGrid[leftIndex];
	  }
	}

	// point right

	int rightIndex = centerIndex + 1;
	if (_grownWorkGrid[rightIndex]) {
	  if (_compWorkGrid[rightIndex] > compVal) {
	    compVal = _compWorkGrid[rightIndex];
	    id = _grownWorkGrid[rightIndex];
	  }
	}

	// point above

	int aboveIndex = centerIndex + _nxWork;
	if (_grownWorkGrid[aboveIndex]) {
	  if (_compWorkGrid[aboveIndex] > compVal) {
	    compVal = _compWorkGrid[aboveIndex];
	    id = _grownWorkGrid[aboveIndex];
	  }
	}

	if (id > 0) {
	  _grownWorkGrid[centerIndex] = id;
	  _cIndex[i] = -1;
	  changed = TRUE;
	}
	
      } // if (_grownWorkGrid[index] == 0) 
      
    } // i

  } // while (changed)

}

////////////////////
// _allocSubClumps()

void ClumpingDualThresh::_allocSubClumps()
  
{
  
  if (_subClumps) {
    delete[](_subClumps);
  }
  _subClumps = new ClumpGrid[_nSubClumps];

  if (_nSubClumps > _nSubClumpsAlloc) {
    _subClumping = (ClumpingMgr **)
      urealloc(_subClumping, _nSubClumps * sizeof(ClumpingMgr *));
    for (size_t i = _nSubClumpsAlloc; i < _nSubClumps; i++) {
      _subClumping[i] = new ClumpingMgr();
    }
    _nSubClumpsAlloc = _nSubClumps;
  }

}

/////////////////////
// _computeSubClump()

void ClumpingDualThresh::_computeSubClump(const ClumpGrid &clump_grid, int clump_id)
  
{

  // load up the masked grid with those parts of the input clump
  // which have the given id the the grown grid

  _initGridMask();
  _loadGridMask(clump_grid, clump_id);
  
  // clump the masked grid
  
  int nclumps =
    _subClumping[clump_id-1]->performClumping(_nxWork, _nyWork, _inputGeom.nz(),
					      _gridMask, 1, 1);

  int clumpNum = 0;
  if (nclumps > 1) {
    // find clump with max number of pts
    int maxPts = 0;
    for (int i = 0; i < nclumps; i++) {
      int nPts = _subClumping[clump_id-1]->getClumps()[i].pts;
      if (nPts > maxPts) {
	clumpNum = i;
	maxPts = nPts;
      }
    }
  }

  // set up the grid clump object

  _subClumps[clump_id-1].init(_subClumping[clump_id-1]->getClumps() + clumpNum,
			      _inputGeom,
			      clump_grid.startIx - 1,
			      clump_grid.startIy - 1);

}

////////////////////
// _initGridMask()

void ClumpingDualThresh::_initGridMask()

{

  size_t nVol = _nxWork * _nyWork * _inputGeom.nz();

  if (nVol > _nPtsGridMaskAlloc) {
    _gridMask = (ui08 *) urealloc(_gridMask, nVol);
    _nPtsGridMaskAlloc = nVol;
  }

  memset(_gridMask, 0, nVol);

}

//////////////////////////////////////////////////////
// _loadGridMask()
//
// load up the masked grid for a given subclump id
//
// Mask out those points in the original clump which
// do not have the given clump_id in the grown grid.
//

void ClumpingDualThresh::_loadGridMask(const ClumpGrid &clump_grid, int clump_id)
  
{

  // take account of the fact that the working grids
  // have one extra row and col around the outside
  
  for (int intv = 0; intv < clump_grid.nIntervals; intv++) {

    const Interval &intvl = clump_grid.intervals[intv];

    int iy = intvl.row_in_plane + 1;
    int plane_offset = iy * _nxWork + intvl.begin + 1;
    int vol_offset = intvl.plane * _nPtsWorkGrid + plane_offset;
    ui08 *grown = _grownWorkGrid + plane_offset;
    ui08 *masked = _gridMask + vol_offset;

    for (int ix = intvl.begin; ix <= intvl.end; ix++, grown++, masked++) {
      if (*grown == clump_id) {
	*masked = 1;
      }
    } // ix
    
  } // intv
  
}

