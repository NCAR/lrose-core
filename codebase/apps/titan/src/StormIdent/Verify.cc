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
// Verify.cc
//
// Verify class.
//
// Creates verification grids and writes out MDV files for debugging.
// The fields in the files are (0) location of all storms and (1)
// location of only valid storms, i.e. those which have the required
// properties to be considered a storm.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "Verify.hh"
#include "InputMdv.hh"
#include "OutputMdv.hh"
#include "GridClump.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
using namespace std;

//////////////
// constructor
//

Verify::Verify(char *prog_name, Params *params)

{
  
  OK = TRUE;
  _progName = STRdup(prog_name);
  _params = params;

  _nBytesPlaneAlloc = 0;

  _allStormsGrid = NULL;
  _validStormsGrid = NULL;

}

/////////////
// destructor
//

Verify::~Verify()

{

  STRfree(_progName);

  if (_allStormsGrid) {
    ufree(_allStormsGrid);
  }
  if (_validStormsGrid) {
    ufree(_validStormsGrid);
  }

}

/////////////////
// prepareGrids()

void Verify::prepareGrids(InputMdv *input_mdv)
  
{
  
  _inputMdv = input_mdv;

  int nbytesPlane =
    _inputMdv->handle.grid.nx * _inputMdv->handle.grid.ny * sizeof(ui08);
  
  // adjust grid allocation as necessary
  
  if (nbytesPlane > _nBytesPlaneAlloc) {
    
    _allStormsGrid = (ui08 *) urealloc ((char *) _allStormsGrid,
				   (ui32) nbytesPlane);
    _validStormsGrid = (ui08 *) urealloc ((char *) _validStormsGrid,
				      (ui32) nbytesPlane);
    _nBytesPlaneAlloc = nbytesPlane;

  } //if (nbytesPlane < _nBytesPlaneAlloc)
    
  // zero out grids

  memset(_allStormsGrid, 0, _nBytesPlaneAlloc);
  memset(_validStormsGrid, 0, _nBytesPlaneAlloc);

}

//////////////////////////////////////////////////
// updateAllStormsGrid()
//
// update the all storms grid with the given clump

void Verify::updateAllStormsGrid(Clump_order *clump)
  
{
  
  ui08 *allGrid = _allStormsGrid;
  ui08 *compGrid = _inputMdv->compDbz;

  for (int intv = 0; intv < clump->size; intv++) {
    
    Interval *intvl = clump->ptr[intv];
    int offset = (intvl->row_in_plane * _inputMdv->handle.grid.nx +
		  intvl->begin);
    memcpy(allGrid + offset, compGrid + offset, intvl->len);
    
  } // intv

}

//////////////////////////////////////////////////
// updateValidStormsGrid()
//
// update the valid storms grid with the given clump

void Verify::updateValidStormsGrid(GridClump *grid_clump)

{

  ui08 *validGrid = _validStormsGrid;
  ui08 *compGrid = _inputMdv->compDbz;

  Interval *intvl = grid_clump->intervals;
  for (int intv = 0; intv < grid_clump->nIntervals; intv++, intvl++) {
    
    int iy = intvl->row_in_plane + grid_clump->startIy;
    int ix = intvl->begin + grid_clump->startIx;
    int offset = iy * grid_clump->grid.nx + ix;
    memcpy(validGrid + offset, compGrid + offset, intvl->len);
    
  } // intv

}

////////////////////
// writeOutputMdv()
//

int Verify::writeOutputMdv()
  
{
  
  // create output MDV object
  
  char note[256];
  
  sprintf(note, "\nVerification data, dbz threshold %g", 
	  _params->low_dbz_threshold);
  
  OutputMdv *out = new OutputMdv(_progName, _params,
				 _inputMdv, N_VERIFY_FIELDS, note,
				 _params->verify_dir);
  
  if (!out) {
    return (-1);
  }
    
  // fill in output file field info
  
  out->fillFieldHeader(STORM_IDENT_ALL_STORMS_FIELD,
		       "All storms verification",
		       "All storms", "dBZ", "dBZ");
  out->fillFieldHeader(STORM_IDENT_VALID_STORMS_FIELD,
		       "Valid storms verification",
		       "Valid storms", "dBZ", "dBZ");
  
  // load field data
  
  double scale = _inputMdv->handle.fld_hdrs[_params->dbz_field].scale;
  double bias = _inputMdv->handle.fld_hdrs[_params->dbz_field].bias;

  out->loadFieldData(STORM_IDENT_ALL_STORMS_FIELD,
		     _allStormsGrid, scale, bias);
  out->loadFieldData(STORM_IDENT_VALID_STORMS_FIELD,
		     _validStormsGrid, scale, bias);

  // write out file

  out->writeVol();

  // clean up

  delete (out);
  
  return(0);

}


