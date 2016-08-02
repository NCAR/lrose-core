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
// Morphology.cc
//
// Morphology class.
//
// Creates morphology grids and writes out MDV files for debugging.
// The fields in the files are (0) reflectivity margin field,
// (1) edm field, (2) morphology field and (3) eroded field.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "Morphology.hh"
#include "InputMdv.hh"
#include "OutputMdv.hh"
#include "GridClump.hh"
#include "Clumping.hh"
#include <toolsa/umisc.h>
using namespace std;

//////////////
// constructor
//

Morphology::Morphology(const string &prog_name, const Params &params,
		       const InputMdv &input_mdv) :
  Worker(prog_name, params),
  _inputMdv(input_mdv)
  
{
  
  // working grids

  _reflMarginWorkGrid = NULL;
  _edmWorkGrid = NULL;
  _morphWorkGrid = NULL;
  _erodedWorkGrid = NULL;
  _compWorkGrid = NULL;
  _nBytesWorkGridAlloc = 0;

  // file grids

  _reflMarginFileGrid = NULL;
  _edmFileGrid = NULL;
  _morphFileGrid = NULL;
  _erodedFileGrid = NULL;
  _compFileGrid = NULL;
  _nBytesFileGridAlloc = 0;

  // morphology clumping

  _morphClumping = new Clumping(_progName);

  // grid mask

  _gridMask = NULL;
  _nBytesGridMaskAlloc = 0;
  _maskClumping = new Clumping(_progName);

  // sub-clumps

  _nSubClumps = 0;
  _subClumps = NULL;
  
}

/////////////
// destructor
//

Morphology::~Morphology()

{

  if (_morphClumping) {
    delete (_morphClumping);
  }

  if (_maskClumping) {
    delete (_maskClumping);
  }

  if (_reflMarginFileGrid) {
    ufree(_reflMarginFileGrid);
  }
  if (_edmFileGrid) {
    ufree(_edmFileGrid);
  }
  if (_morphFileGrid) {
    ufree(_morphFileGrid);
  }
  if (_erodedFileGrid) {
    ufree(_erodedFileGrid);
  }
  if (_compFileGrid) {
    ufree(_compFileGrid);
  }
  if (_reflMarginWorkGrid) {
    ufree(_reflMarginWorkGrid);
  }
  if (_edmWorkGrid) {
    ufree(_edmWorkGrid);
  }
  if (_morphWorkGrid) {
    ufree(_morphWorkGrid);
  }
  if (_erodedWorkGrid) {
    ufree(_erodedWorkGrid);
  }
  if (_compWorkGrid) {
    ufree(_compWorkGrid);
  }
  if (_gridMask) {
    ufree(_gridMask);
  }
  if (_subClumps) {
    delete[](_subClumps);
  }
}

/////////////////////
// prepare()
//
// Prepare based on input MDV file
//

void Morphology::prepare()
  
{

  _nxInput = _inputMdv.grid.nx;
  _nyInput = _inputMdv.grid.ny;

  if (_params.create_morphology_files) {
    _initFileGrids();
  }

}

////////////////////////////////////////////
// compute()
//
// Compute the morphology, set the sub-clumps
//
// Returns the number of subclumps
//

int Morphology::compute(const GridClump &grid_clump)

{

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
  _nBytesWorkGrid = _nxWork * _nyWork;

  // prepare eroded grid

  _erodeProjArea(grid_clump);

  // load masked grid, excluding points in locations which
  // were eroded away
  
  ui08 *maskedGrid;
  if (_inputMdv.nPlanes > 1) {
    _initGridMask();
    maskedGrid = _loadGridMask(grid_clump);
  } else {
    // just use eroded grid
    maskedGrid = _erodedWorkGrid;
  }

  // clump the masked grid

  _nSubClumps =
    _maskClumping->performClumping(_nxWork, _nyWork,
				   _inputMdv.nPlanes,
				   maskedGrid,
				   _params.min_grid_overlap, 1);
  
  // create the grid clumps
  
  _createSubClumps(grid_clump);
  
  // return number of sub clumps

  return (_nSubClumps);

}
 
 
////////////////////
// writeOutputMdv()
//

int Morphology::writeOutputMdv()
  
{
  
  // set info str

  char info[256];
  
  sprintf(info,
	  "\nMorphology data, dbz threshold %g, "
	  "erosion threshold %g, refl_divisor %g", 
	  _params.low_dbz_threshold,
	  _params.morphology_erosion_threshold,
	  _params.morphology_refl_divisor);

  // create output MDV object
  
  OutputMdv out(_progName, _params,
		_inputMdv, info,
		"TITAN morphology",
		_params.morphology_url);
  
  // Add the fields

  out.addField("Reflectivity margin", "ReflMargin", "count", "none",
	       1.0, 0.0, _reflMarginFileGrid);
  
  out.addField("Euclidean distance measure", "EDM", "km", "none",
	       1.0, 0.0, _edmFileGrid);
  
  out.addField("Morphology measure", "Morphology", "count", "none",
	       1.0, 0.0, _morphFileGrid);
  
  out.addField("Eroded grid", "Eroded", "count", "none",
	       1.0, 0.0, _erodedFileGrid);
  
  out.addField("Composite reflectivity", "CompDbz", "dBZ", "dBZ",
	       _inputMdv.dbzScale, _inputMdv.dbzBias, _compFileGrid);
  
  // write out file

  if (out.writeVol()) {
    cerr << "ERROR - Morphology::writeOutputMdv" << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////
// _erodeProjArea()
//
// Erode projected area based on morphology.
// Load up eroded grid.
//

void Morphology::_erodeProjArea(const GridClump &grid_clump)

{

  // compute km per grid

  double km_per_grid = grid_clump.kmPerGridUnit;

  // initialize the work grids

  _initWorkGrids();

  // compute the composite reflectivity grid - this is the
  // max reflectivity at any height

  _fillCompDbz(grid_clump);
      
  // compute projected area and the morphology threshold
  // The area-based threshold is the square root of the projected
  // area / 2.0. This is the max erosion threshold. The actual
  // threshold used is the min of this and the parameter threshold.

  double projected_area = _nComp * grid_clump.dAreaAtCentroid;
  double area_based_threshold = sqrt(projected_area) / 2.0;
  double morph_erosion_threshold =
    MIN(_params.morphology_erosion_threshold, area_based_threshold);

  // get the intervals in comp grid

  _morphClumping->findIntervals(_nxWork, _nyWork, _compWorkGrid, 1);
  
  // compute the edm
  
  _morphClumping->edm2D(_nxWork, _nyWork, _edmWorkGrid);

  // compute derived grids
  
  int refl_divisor = (int)
    (_params.morphology_refl_divisor * km_per_grid + 0.5);
  
  ui08 *edm = _edmWorkGrid;
  ui08 *comp = _compWorkGrid;
  ui08 *refl_margin = _reflMarginWorkGrid;
  ui08 *morph = _morphWorkGrid;
  ui08 low_dbz_byte = _inputMdv.lowDbzByte;

  for (int i = 0; i < _nBytesWorkGrid;
       i++, comp++, edm++, morph++, refl_margin++) {
    if (*edm > 100) {
      *edm = 100;
    };
    if (*edm) {
      *refl_margin = (*comp - low_dbz_byte) / refl_divisor;
      *morph = *edm + *refl_margin;
    }
  } // i

  // erode the grid
  
  memcpy(_erodedWorkGrid, _morphWorkGrid, _nBytesWorkGrid);
    
  int erosion_threshold = (int)
    (morph_erosion_threshold / km_per_grid + 0.5);

  _morphClumping->erode(_nxWork, _nyWork, _erodedWorkGrid, erosion_threshold);

  // update the grids
  
  if (_params.create_morphology_files) {
    _updateFileGrids(grid_clump);
  }

}

///////////////////
// _fillCompDbz()
//
// Fill the composite dbz array from 3D array.
//

void Morphology::_fillCompDbz(const GridClump &grid_clump)

{

  // compute the composite reflectivity grid - this is the
  // max reflectivity at any height

  int minValidLayer = _inputMdv.minValidLayer;
  int nBytesPlane = _nxInput * _nyInput;

  _nComp = 0;
  for (int intv = 0; intv < grid_clump.nIntervals; intv++) {

    const Interval &intvl = grid_clump.intervals[intv];

    int iplane = intvl.plane + minValidLayer;
    int iy = intvl.row_in_plane + grid_clump.startIy;
    int ix = intvl.begin + grid_clump.startIx;

    ui08 *dbz =
      _inputMdv.dbzVol + (iplane * nBytesPlane) + (iy * _nxInput) + ix;
    
    ui08* comp =
      _compWorkGrid + (intvl.row_in_plane + 1) * _nxWork + intvl.begin + 1;

    for (int i = 0; i < intvl.len; i++, dbz++, comp++) {
      if (!*comp) {
	_nComp++;
      }
      if (*dbz > *comp) {
   	*comp = *dbz;
      }
    } // i

  } // intvl

}

/////////////////////
// _initWorkGrids()

void Morphology::_initWorkGrids()
  
{
  
  if (_nBytesWorkGridAlloc < _nBytesWorkGrid) {
    
    // Work grids
    
    _reflMarginWorkGrid =
      (ui08 *) urealloc (_reflMarginWorkGrid, _nBytesWorkGrid);
    _edmWorkGrid =
      (ui08 *) urealloc (_edmWorkGrid, _nBytesWorkGrid);
    _morphWorkGrid =
      (ui08 *) urealloc (_morphWorkGrid, _nBytesWorkGrid);
    _erodedWorkGrid =
      (ui08 *) urealloc (_erodedWorkGrid, _nBytesWorkGrid);
    _compWorkGrid =
      (ui08 *) urealloc (_compWorkGrid, _nBytesWorkGrid);

    _nBytesWorkGridAlloc = _nBytesWorkGrid;
    
  }

  memset(_reflMarginWorkGrid, 0, _nBytesWorkGrid);
  memset(_edmWorkGrid, 0, _nBytesWorkGrid);
  memset(_morphWorkGrid, 0, _nBytesWorkGrid);
  memset(_erodedWorkGrid, 0, _nBytesWorkGrid);
  memset(_compWorkGrid, 0, _nBytesWorkGrid);

}

/////////////////////
// _initFileGrids()
//
// Allocate grids for MDV output file
//

void Morphology::_initFileGrids()
  
{

  int nbytes = _nxInput * _nyInput * sizeof(ui08);
  
  // adjust grid allocation as necessary
  
  if (_nBytesFileGridAlloc < nbytes) {

    _reflMarginFileGrid =
      (ui08 *) urealloc (_reflMarginFileGrid, nbytes);
    _edmFileGrid =
      (ui08 *) urealloc (_edmFileGrid, nbytes);
    _morphFileGrid =
      (ui08 *) urealloc (_morphFileGrid, nbytes);
    _erodedFileGrid =
      (ui08 *) urealloc (_erodedFileGrid, nbytes);
    _compFileGrid =
      (ui08 *) urealloc (_compFileGrid, nbytes);

    _nBytesFileGridAlloc = nbytes;

  }

  // zero out grids
  
  memset(_reflMarginFileGrid, 0, nbytes);
  memset(_edmFileGrid, 0, nbytes);
  memset(_morphFileGrid, 0, nbytes);
  memset(_erodedFileGrid, 0, nbytes);
  memset(_compFileGrid, 0, nbytes);

}

////////////////////
// _initGridMask()

void Morphology::_initGridMask()

{

  int nVol = _nxWork * _nyWork * _inputMdv.nPlanes;

  if (nVol > _nBytesGridMaskAlloc) {
    _gridMask = (ui08 *) urealloc(_gridMask, nVol);
    _nBytesGridMaskAlloc = nVol;
  }

  memset(_gridMask, 0, nVol);

}

/////////////////////
// _loadGridMask()
//
// load up the masked grid
//
// Mask out those points in the original clump which have
// been eroded and are no longer in _erodedGrid.
//
// returns ptr to grid mask

ui08 *Morphology::_loadGridMask(const GridClump &grid_clump)

{

  // take account of the fact that the working grids
  // have one extra row and col around the outside

  for (int intv = 0; intv < grid_clump.nIntervals; intv++) {

    const Interval &intvl = grid_clump.intervals[intv];

    int iy = intvl.row_in_plane + 1;
    int plane_offset = iy * _nxWork + intvl.begin + 1;
    int vol_offset = intvl.plane * _nBytesWorkGrid + plane_offset;
    ui08 *eroded = _erodedWorkGrid + plane_offset;
    ui08 *masked = _gridMask + vol_offset;

    for (int ix = intvl.begin; ix <= intvl.end; ix++, eroded++, masked++) {
      if (*eroded) {
	*masked = 1;
      }
    } // ix
    
  } // intv

  return (_gridMask);

}

////////////////////
// _createSubClumps()

void Morphology::_createSubClumps(const GridClump &grid_clump)
  
{
  
  if (_subClumps) {
    delete[](_subClumps);
  }
  
  _subClumps = new GridClump[_nSubClumps];

  // copy over - clumps start at position 1 (rather than 0) 
  // in the array

  for (int i = 0; i < _nSubClumps; i++) {
    _subClumps[i].init(_maskClumping->clumps + i + 1,
		       _inputMdv.grid,
		       grid_clump.startIx - 1,
		       grid_clump.startIy - 1);
  }

}

//////////////////////
// _updateFileGrids()
//
// Update the grids which will be used in the output file
//

void Morphology::_updateFileGrids(const GridClump &grid_clump)
  
{
  
  // take account of the fact that the working grids
  // have one extra row and col around the outside

  ui08 *edm = _edmWorkGrid + _nxWork + 1;
  ui08 *refl_margin = _reflMarginWorkGrid + _nxWork + 1;
  ui08 *morph = _morphWorkGrid + _nxWork + 1;
  ui08 *eroded = _erodedWorkGrid + _nxWork + 1;
  ui08 *comp = _compWorkGrid + _nxWork + 1;

  int offset =
    grid_clump.startIy * _nxInput + grid_clump.startIx;
  int missed = _nxInput - (_nxWork - 2);
  
  for (int iy = 1; iy < _nyWork - 1;
       iy++, offset += missed,
	 edm += 2, refl_margin += 2, morph += 2, eroded += 2, comp += 2) {
    
    for (int ix = 1; ix < _nxWork - 1;
	 ix++, offset++, edm++, refl_margin++, morph++, eroded++, comp++) {

      if (*edm) {
	_edmFileGrid[offset] = *edm;
	_reflMarginFileGrid[offset] = *refl_margin;
	_morphFileGrid[offset] = *morph;
	_erodedFileGrid[offset] = *eroded;
	_compFileGrid[offset] = *comp;
      }
      
    } // ix
    
  } // iy

}

