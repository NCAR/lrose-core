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
// DualThresh.cc
//
// DualThresh class
//
// This class performs the second stage in
// multiple threshold identification.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "DualThresh.hh"
#include "InputMdv.hh"
#include "GridClump.hh"
#include "OutputMdv.hh"

#include <toolsa/umisc.h>
#include <vector>
using namespace std;

///////////////////////////////////
// constructor - recursion level 0
//

DualThresh::DualThresh(const string &prog_name, const Params &params,
		       const InputMdv &input_mdv) :
  Worker(prog_name, params),
  _inputMdv(input_mdv),
  _inputGrid(input_mdv.grid),
  _clumping(prog_name)
  

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

  _primaryDbzThreshold = _params.low_dbz_threshold;
  _secondaryDbzThreshold = _params.dual_threshold.dbz_threshold;
  _minFractionAllParts = _params.dual_threshold.min_fraction_all_parts;
  _minFractionEachPart = _params.dual_threshold.min_fraction_each_part;
  _minAreaEachPart = _params.dual_threshold.min_area_each_part;

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

DualThresh::~DualThresh()

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
    for (int i = 0; i < _nSubClumpsAlloc; i++) {
      delete(_subClumping[i]);
    }
    ufree(_subClumping);
  }

  if (_gridMask) {
    ufree(_gridMask);
  }

}

/////////////////////
// prepare()
//
// Prepare based on input MDV file
//

void DualThresh::prepare()
  
{

  _nxInput = _inputGrid.nx;
  _nyInput = _inputGrid.ny;

  if (_params.create_dual_threshold_files) {
    _initFileGrids();
  }

}

////////////////////////////////////////////
// compute()
//
// Compute clumps based on dual threshold, set the sub-clumps
//
// Returns number of sub-clumps.
//

int DualThresh::compute(const GridClump &grid_clump)

{

  if (_params.debug >= Params::DEBUG_EXTRA) {
    fprintf(stderr, "Computing using secondary threshold\n");
  }

  // check size
  
  if (grid_clump.stormSize < _params.min_storm_size ||
      grid_clump.stormSize > _params.max_storm_size) {
    return (0);
  }
    
  // work grids have a spare row and col around them to make the
  // computations more efficient. This strategy means we do not
  // have to check for edge conditions

  _nxWork = grid_clump.nX + 2;
  _nyWork = grid_clump.nY + 2;
  _nPtsWorkGrid = _nxWork * _nyWork;

  // initialize the working grids

  _initWorkGrids();

  // fill out the composite reflectivity grid

  _fillCompDbz(grid_clump);

  // clump composite grid at the dual threshold
  
  int nSecondary =
    _clumping.performClumping(_nxWork, _nyWork, 1,
			      _compWorkGrid, 1,
			      _params.dual_threshold.dbz_threshold);
  
  // load up all clumps grid for debugging

  if (_params.create_dual_threshold_files) {
    for (int i = 0; i < nSecondary; i++) {
      Clump_order *clump =  _clumping.clumps + i + 1;
      for (int j = 0; j < clump->size; j++) {
	Interval *intv = clump->ptr[j];
	int offset = intv->row_in_plane * _nxWork + intv->begin;
	memset(_allWorkGrid + offset, i + 1, intv->len);
      } // j
    } // i
  }
  _updateFileComp(grid_clump);

  // loop through the clumps, determining if they are large
  // enough to be sub-clumps

  double sizeOuter = (double) _nComp;
  double sumSize = 0.0;
  int nLargeEnough = 0;

  vector<int> valid;

  for (int i = 0; i < nSecondary; i++) {
    Clump_order *clump =  _clumping.clumps + i + 1;
    GridClump gridClump;
    gridClump.init(clump, grid_clump.grid,
		   grid_clump.startIx, grid_clump.startIy);
    double thisSize = gridClump.nPoints;
    sumSize += thisSize;
    double fractionThisPart = thisSize / sizeOuter;
    if (fractionThisPart >= _minFractionEachPart &&
	gridClump.stormSize >= _minAreaEachPart) {
      nLargeEnough++;
      valid.push_back(TRUE);
    } else {
      valid.push_back(FALSE);
    }
  }
  double fractionAllParts = sumSize / sizeOuter;
  
  if (fractionAllParts < _minFractionAllParts ||
      nLargeEnough < 2) {

    // the parts are not large or numerous enough to make this
    // procedure valid, so  just use the input clump
    
    _nSubClumps = 1;
    _allocSubClumps();
    _subClumps[0] = grid_clump;
    return (1);

  }

  // allocate the sub clump memory

  _nSubClumps = nLargeEnough;
  _allocSubClumps();

  // for each valid clump, copy the clump ids to the valid grid

  if (_params.debug >= Params::DEBUG_EXTRA) {
    fprintf(stderr,
	    "nSecondary, fractionAllParts, nLargeEnough: %d, %g, %d\n",
	    nSecondary, fractionAllParts, nLargeEnough);
  }

  int n = 0;
  for (int i = 0; i < nSecondary; i++) {
    if (valid[i]) {
      n++;
      Clump_order *clump =  _clumping.clumps + i + 1;
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

  _updateFileGrids(grid_clump);

  // compute each sub clump

  for (int i = 0; i < _nSubClumps; i++) {
    _computeSubClump(grid_clump, i+1);
  }
  
  return (_nSubClumps);

}

////////////////////
// writeOutputMdv()
//

int DualThresh::writeOutputMdv()
  
{
  
  // set info str

  char info[256];
  
  sprintf(info,
	  "\n"
	  "  Dual threshold analysis: \n"
 	  "    dbz_threshold: %g\n"
 	  "    min_fraction_all_parts: %g\n"
 	  "    min_fraction_each_part: %g\n",
	  _params.dual_threshold.dbz_threshold,
	  _params.dual_threshold.min_fraction_all_parts,
	  _params.dual_threshold.min_fraction_each_part);

  // create output MDV object
  
  OutputMdv out(_progName, _params,
		_inputMdv, info,
		"TITAN dual thresholds",
		_params.dual_threshold_url);
  
  // Add the fields

  out.addField("Composite reflectivity", "CompDbz", "dBZ", "dBZ", _compFileGrid);
  out.addField("All clumps", "All", "count", "none", 1.0, 0.0, _allFileGrid);
  out.addField("Valid clumps", "Valid", "count", "none", 1.0, 0.0, _validFileGrid);
  out.addField("Grown clumps", "Grown", "count", "none", 1.0, 0.0, _grownFileGrid);

  // write out file

  if (out.writeVol()) {
    cerr << "ERROR - DualThresh::writeOutputMdv" << endl;
    return -1;
  }

  return 0;

}

///////////////////
// _fillCompDbz()
//
// Fill the composite dbz array from 3D array.
//

void DualThresh::_fillCompDbz(const GridClump &grid_clump)

{

  // compute the composite reflectivity grid - this is the
  // max reflectivity at any height

  int minValidLayer = _inputMdv.minValidLayer;
  int nPtsPlane = _nxInput * _nyInput;

  for (int intv = 0; intv < grid_clump.nIntervals; intv++) {

    const Interval &intvl = grid_clump.intervals[intv];

    int iplane = intvl.plane + minValidLayer;
    int iy = intvl.row_in_plane + grid_clump.startIy;
    int ix = intvl.begin + grid_clump.startIx;
    
    fl32 *dbz =
      _inputMdv.dbzVol + (iplane * nPtsPlane) + (iy * _nxInput) + ix;

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
  for (int iy = 0; iy < _nyWork; iy++) {
    for (int ix = 0; ix < _nxWork; ix++, comp++) {
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

void DualThresh::_initWorkGrids()
  
{
  
  if (_nPtsWorkGridAlloc < _nPtsWorkGrid) {
    _compWorkGrid = (fl32 *) urealloc(_compWorkGrid, _nPtsWorkGrid * sizeof(fl32));
    _allWorkGrid = (ui08 *) urealloc(_allWorkGrid, _nPtsWorkGrid);
    _validWorkGrid = (ui08 *) urealloc(_validWorkGrid, _nPtsWorkGrid);
    _grownWorkGrid = (ui08 *) urealloc(_grownWorkGrid, _nPtsWorkGrid);
    _nPtsWorkGridAlloc = _nPtsWorkGrid;
  }

  for (int ii = 0; ii < _nPtsWorkGrid; ii++) {
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

void DualThresh::_initFileGrids()
  
{

  int npts = _nxInput * _nyInput * sizeof(ui08);
  
  // adjust grid allocation as necessary
  
  if (_nPtsFileGridAlloc < npts) {

    _compFileGrid = (fl32 *) urealloc (_compFileGrid, npts * sizeof(fl32));
    _allFileGrid = (ui08 *) urealloc (_allFileGrid, npts);
    _validFileGrid = (ui08 *) urealloc (_validFileGrid, npts);
    _grownFileGrid = (ui08 *) urealloc (_grownFileGrid, npts);

    _nPtsFileGridAlloc = npts;

  }

  // initialize grids
  
  for (int ii = 0; ii < npts; ii++) {
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

void DualThresh::_updateFileComp(const GridClump &grid_clump)
  
{
  
  if (!_params.create_dual_threshold_files) {
    return;
  }

  ui08 *all = _allWorkGrid + _nxWork + 1;
  fl32 *comp = _compWorkGrid + _nxWork + 1;
  
  int offset =
    grid_clump.startIy * _nxInput + grid_clump.startIx;
  int missed = _nxInput - (_nxWork - 2);
  
  for (int iy = 1; iy < _nyWork - 1;
       iy++, offset += missed, comp += 2, all += 2) {
    
    for (int ix = 1; ix < _nxWork - 1; ix++, offset++, comp++, all++) {
      
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

void DualThresh::_updateFileGrids(const GridClump &grid_clump)
  
{
  
  if (!_params.create_dual_threshold_files) {
    return;
  }

  ui08 *valid = _validWorkGrid + _nxWork + 1;
  ui08 *grown = _grownWorkGrid + _nxWork + 1;
  
  int offset =
    grid_clump.startIy * _nxInput + grid_clump.startIx;
  int missed = _nxInput - (_nxWork - 2);
  
  for (int iy = 1; iy < _nyWork - 1;
       iy++, offset += missed, valid += 2, grown += 2) {
    
    for (int ix = 1; ix < _nxWork - 1;
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

void DualThresh::_loadCompIndex()

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
  
  for (int iy = 0; iy < _nyWork; iy++) {
    for (int ix = 0; ix < _nxWork; ix++, index++, comp++) {
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
  for (int i = 0; i < _nComp; i++) {
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

void DualThresh::_growSubAreas()

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

    for (int i = 0; i < _nComp; i++) {

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

void DualThresh::_allocSubClumps()
  
{
  
  if (_subClumps) {
    delete[](_subClumps);
  }
  _subClumps = new GridClump[_nSubClumps];

  if (_nSubClumps > _nSubClumpsAlloc) {
    _subClumping = (Clumping **)
      urealloc(_subClumping, _nSubClumps * sizeof(Clumping *));
    for (int i = _nSubClumpsAlloc; i < _nSubClumps; i++) {
      _subClumping[i] = new Clumping(_progName);
    }
    _nSubClumpsAlloc = _nSubClumps;
  }

}

/////////////////////
// _computeSubClump()

void DualThresh::_computeSubClump(const GridClump &grid_clump, int clump_id)
  
{

  // load up the masked grid with those parts of the input clump
  // which have the given id the the grown grid

  _initGridMask();
  _loadGridMask(grid_clump, clump_id);
  
  // clump the masked grid
  
  int nclumps =
    _subClumping[clump_id-1]->performClumping(_nxWork, _nyWork,
					      _inputMdv.nPlanes,
					      _gridMask, 1, 1);

  int clumpNum = 0;
  if (nclumps == 1) {
    clumpNum = 1;
  } else {
    int maxPts = 0;
    for (int i = 1; i <= nclumps; i++) {
      if (_subClumping[clump_id-1]->clumps[i].pts > maxPts) {
	clumpNum = i;
	maxPts = _subClumping[clump_id-1]->clumps[i].pts;
      }
    }
  }

  // set up the grid clump object

  _subClumps[clump_id-1].init(_subClumping[clump_id-1]->clumps + clumpNum,
			      _inputGrid,
			      grid_clump.startIx - 1,
			      grid_clump.startIy - 1);

}

////////////////////
// _initGridMask()

void DualThresh::_initGridMask()

{

  int nVol = _nxWork * _nyWork * _inputMdv.nPlanes;

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

void DualThresh::_loadGridMask(const GridClump &grid_clump, int clump_id)
  
{

  // take account of the fact that the working grids
  // have one extra row and col around the outside
  
  for (int intv = 0; intv < grid_clump.nIntervals; intv++) {

    const Interval &intvl = grid_clump.intervals[intv];

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

