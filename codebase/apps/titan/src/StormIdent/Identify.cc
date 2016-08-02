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
// Identify.cc
//
// Identify class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "Identify.hh"
#include "InputMdv.hh"
#include "StormFile.hh"
#include "Props.hh"
#include "Clumping.hh"
#include "Verify.hh"
#include "Morphology.hh"
#include "GridClump.hh"
#include "DualThresh.hh"

#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
using namespace std;

//////////////
// constructor
//

Identify::Identify(char *prog_name, Params *params,
		   InputMdv *input_mdv,
		   StormFile *storm_file)

{

  OK = TRUE;

  _progName = STRdup(prog_name);
  _params = params;
  _inputMdv = input_mdv;
  _stormFile = storm_file;

  _clumping = new Clumping(_progName);

  _verify = NULL;
  if (_params->create_verification_files) {
    _verify = new Verify(_progName, _params);
  }

  _props = NULL;
  _props = new Props(_progName, _params, _inputMdv, _stormFile, _verify);

  // dual threshold takes precedence over morphology

  _morph = NULL;
  _dualT = NULL;
  if (_params->use_dual_threshold) {
    _dualT = new DualThresh(_progName, _params);
  } else if (_params->check_morphology) {
    _morph = new Morphology(_progName, _params);
  }
  
}

/////////////
// destructor
//

Identify::~Identify()

{

  STRfree(_progName);
  if (_clumping) {
    delete (_clumping);
  }
  if (_verify) {
    delete (_verify);
  }
  if (_props) {
    delete (_props);
  }
  if (_dualT) {
    delete (_dualT);
  }
  if (_morph) {
    delete (_morph);
  }

}

//////////////////////////////////////////////////////
// perform()
//
// Perform storm identification for given scan number
//
// Returns 0 on success, -1 on failure.

int Identify::perform(int scan_num)

{

  PMU_auto_register("Identifying storms");
    
  // initialize

  _nStorms = 0;
  mdv_grid_t *grid = &_inputMdv->handle.grid;
  int nplanes = _inputMdv->nPlanes;

  // prepare verification and morphology objects as required

  if (_verify) {
    _verify->prepareGrids(_inputMdv);
  }

  if (_dualT) {
    _dualT->prepare(_inputMdv);
  }

  if (_morph) {
    _morph->prepare(_inputMdv);
  }

  // perform clumping
  
  _nClumps = _clumping->performClumping(grid->nx, grid->ny, nplanes,
					_inputMdv->dbzVol,
					_params->min_grid_overlap,
					_inputMdv->lowDbzByte);
  
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "Number of clumps  =  %d\n", _nClumps);
  }

  // update the verification grid

  if (_verify) {
    Clump_order *clump = _clumping->clumps + 1;
    for (int i = 0; i < _clumping->nClumps; i++, clump++) {
      _verify->updateAllStormsGrid(clump);
    }
  }
  
  // lock the header file

  if (_stormFile->lockHeaderFile()) {
    return (-1);
  }

  // process the clumps found

  if (_processClumps(scan_num)) {
    return (-1);
  }
  
  // unlock the header file

  if (_stormFile->unlockHeaderFile()) {
    return (-1);
  }

  if (_verify) {
    _verify->writeOutputMdv();
  }
  
  if (_dualT && _params->create_dual_threshold_files) {
    _dualT->writeOutputMdv();
  }

  if (_morph && _params->create_morphology_files) {
    _morph->writeOutputMdv();
  }
  
  return (0);

}

////////////////////
// _processClumps()
//
//
// Checks storm props
// If necessary splits storms.
// Passes clumps to props_compute for computations of storm properties.
//
// Returns 0 on success, -1 on failure
//

int Identify::_processClumps(int scan_num)
     
{

  // read in storm file header
  
  if (RfReadStormHeader(&_stormFile->handle, "Identify::_processClumps")) {
    return(-1);
  }
  
  // Make sure there's memory allocated for the scan
  
  RfAllocStormScan(&_stormFile->handle, 1, "Identify::_processClumps");
  
  // initialize the computation module for storm props
  
  _props->init();

  // loop through the clumps - index starts at 1
  
  Clump_order *clump = _clumping->clumps + 1;
  mdv_grid_t *grid = &_inputMdv->handle.grid;
  
  for (int iclump = 0; iclump < _nClumps; iclump++, clump++) {
    
    GridClump *gridClump = new GridClump(clump, grid, 0, 0);
    
    // dual threshold takes precedence over morphology

    if (_params->use_dual_threshold) {
      
      int n_sub_clumps = _dualT->compute(gridClump);

      if (n_sub_clumps == 1) {

	if (_processThisClump(gridClump)) {
	  return (-1);
	}

      } else {

	for (int i = 0; i < n_sub_clumps; i++) {
	  if (_processThisClump(&_dualT->subClumps[i])) {
	    return (-1);
	  }
	}

      }

    } else if (_params->check_morphology) {
      
      int n_sub_clumps = _morph->compute(gridClump);
      for (int i = 0; i < n_sub_clumps; i++) {
	if (_processThisClump(&_morph->subClumps()[i])) {
	  return (-1);
	}
      }

    } else {
      
      if (_processThisClump(gridClump)) {
	return (-1);
      }
      
    }

    delete (gridClump);

  } // iclump
  
  // load up scan structure
  
  storm_file_scan_header_t *scan_hdr = _stormFile->handle.scan;
  
  scan_hdr->nbytes_char = MDV_N_GRID_LABELS * MDV_GRID_UNITS_LEN;
  scan_hdr->scan_num = scan_num;
  scan_hdr->nstorms = _nStorms;

  scan_hdr->time = _inputMdv->handle.master_hdr.time_centroid;
  
  scan_hdr->min_z = _props->minValidZ;
  scan_hdr->delta_z = _inputMdv->handle.grid.dz;
  memcpy(&scan_hdr->grid, &_inputMdv->handle.grid, sizeof(titan_grid_t));

  // read in storm file header

  if (RfReadStormHeader(&_stormFile->handle, "Identify::_processClumps")) {
    return(-1);
  }
  
  // write the scan header and global props
  
  if (RfWriteStormScan(&_stormFile->handle, scan_num,
		       "Identify::_processClumps")) {
    return(-1);
  }
  
  // update the file header
  
  if (scan_num == 0) {
    _stormFile->handle.header->start_time = scan_hdr->time;
  }
  
  _stormFile->handle.header->end_time = scan_hdr->time;
  
  // fill in number of scans, tracks and current file size
  
  _stormFile->handle.header->n_scans = scan_num + 1;
  
  // rewrite header
  
  if (RfWriteStormHeader(&_stormFile->handle, "Identify::_processClumps")) {
    return(-1);
  }
  
  if (RfFlushStormFiles(&_stormFile->handle, "Identify::_processClumps")) {
    return(-1);
  }
  
  // printout
  
  fprintf(stdout, "scan number %d\n", scan_num);
  fprintf(stdout, "nstorms : %d\n", _nStorms);
  fprintf(stdout, "Time: %s\n",
	  utimstr(_stormFile->handle.header->end_time));
  
  return (0);
  
}

/////////////////////////////////
// _processThisClump()
//

int Identify::_processThisClump(GridClump *grid_clump)

{

  // check size

  if (grid_clump->stormSize < _params->min_storm_size ||
      grid_clump->stormSize > _params->max_storm_size) {
    return(0);
  }

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr,
	    "Clump: size, startIx, startIy, nx, ny, offsetx, offsety: "
	    "%g, %d, %d, %d, %d, %g, %g\n",
	    grid_clump->stormSize,
	    grid_clump->startIx, grid_clump->startIy,
	    grid_clump->nX, grid_clump->nY,
	    grid_clump->offsetX, grid_clump->offsetY);
  }

  RfAllocStormScan(&_stormFile->handle, _nStorms + 1,
		   "Identify::_processThisClump");
  
  if (_props->compute(grid_clump, _nStorms) == 0) {
    
    // success - write the storm props to storm file
    
    if (RfWriteStormProps(&_stormFile->handle, _nStorms,
			  "Identify::_processThisClump")) {
      return (-1);
    }
    
    _nStorms++;

  }

  return (0);

}

