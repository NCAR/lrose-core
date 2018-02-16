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
#include "Props.hh"
#include "Verify.hh"
#include "GridClump.hh"
#include "DualThresh.hh"
#include "Sounding.hh"

#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
using namespace std;

//////////////
// constructor
//

Identify::Identify(const string &prog_name, const Params &params,
		   const InputMdv &input_mdv, TitanStormFile &storm_file):
  Worker(prog_name, params),
  _inputMdv(input_mdv),
  _sfile(storm_file),
  _clumping(prog_name)

{

  _props = NULL;
  _verify = NULL;
  _dualT = NULL;
  
  if (_params.create_verification_files) {
    _verify = new Verify(_progName, _params, _inputMdv);
  }

  _props = new Props(_progName, _params, _inputMdv, _sfile, _verify);

  // dual threshold takes precedence over morphology

  if (_params.use_dual_threshold) {
    _dualT = new DualThresh(_progName, _params, _inputMdv);
  }
  
}

/////////////
// destructor
//

Identify::~Identify()

{

  if (_verify) {
    delete (_verify);
  }
  if (_props) {
    delete (_props);
  }
  if (_dualT) {
    delete (_dualT);
  }

}

//////////////////////////////////////////////////////
// run()
//
// Perform storm identification for given scan number
//
// Returns 0 on success, -1 on failure.

int Identify::run(int scan_num)

{

  PMU_auto_register("Identifying storms");
    
  // initialize

  _nStorms = 0;
  const titan_grid_t &grid = _inputMdv.grid;
  int nBytesPlane = grid.nx * grid.ny;
  int nplanes = _inputMdv.nPlanes;

  // prepare verification and morphology objects as required

  if (_verify) {
    _verify->prepareGrids();
  }

  if (_dualT) {
    _dualT->prepare();
  }

  // perform clumping

  fl32 *startDbz = _inputMdv.dbzVol + _inputMdv.minValidLayer * nBytesPlane;
  _nClumps = _clumping.performClumping(grid.nx, grid.ny, nplanes,
                                       startDbz,
                                       _params.min_grid_overlap,
                                       _params.low_dbz_threshold);
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    fprintf(stderr, "Number of clumps  =  %d\n", _nClumps);
  }

  // update the verification grid

  if (_verify) {
    Clump_order *clump = _clumping.clumps + 1;
    for (int i = 0; i < _clumping.nClumps; i++, clump++) {
      _verify->updateAllStormsGrid(*clump);
    }
  }
  
  // lock the header file

  if (_sfile.LockHeaderFile("w")) {
    cerr << "ERROR - " << _progName << "Identify::run" << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }

  // process the clumps found

  if (_processClumps(scan_num)) {
    return (-1);
  }
  
  // unlock the header file

  if (_sfile.UnlockHeaderFile()) {
    cerr << "ERROR - " << _progName << "Identify::run" << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }

  if (_verify) {
    _verify->writeOutputMdv();
  }
  
  if (_dualT && _params.create_dual_threshold_files) {
    _dualT->writeOutputMdv();
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

  if (_sfile.ReadHeader()) {
    cerr << "ERROR - " << _progName << "Identify::_processClumps" << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }

  // initialize the computation module for storm props
  
  _props->init();

  // loop through the clumps - index starts at 1
  
  Clump_order *clump = _clumping.clumps + 1;
  
  for (int iclump = 0; iclump < _nClumps; iclump++, clump++) {
    
    GridClump gridClump(clump, _inputMdv.grid, 0, 0);
    
    // dual threshold takes precedence over morphology

    if (_params.use_dual_threshold) {
      
      int n_sub_clumps = _dualT->compute(gridClump);

      if (n_sub_clumps == 1) {
	
	if (_processThisClump(gridClump)) {
	  return (-1);
	}

      } else {

	for (int i = 0; i < n_sub_clumps; i++) {
	  if (_processThisClump(_dualT->subClumps()[i])) {
	    return (-1);
	  }
	}

      }

    } else {
      
      if (_processThisClump(gridClump)) {
	return (-1);
      }
      
    }

  } // iclump
  
  // load up scan structure
  
  storm_file_scan_header_t &scan_hdr = _sfile._scan;
  
  scan_hdr.nbytes_char = TITAN_N_GRID_LABELS * TITAN_GRID_UNITS_LEN;
  scan_hdr.scan_num = scan_num;
  scan_hdr.nstorms = _nStorms;
  scan_hdr.time = _inputMdv.mdvx.getMasterHeader().time_centroid;
  scan_hdr.min_z = _props->getMinValidZ();
  scan_hdr.delta_z = _inputMdv.grid.dz;
  scan_hdr.grid = _inputMdv.grid;

  // for now height of freezing is a static user parameter
  // eventually it may be derived dynamically from a sounding

  Sounding &sndg = Sounding::inst();
  scan_hdr.ht_of_freezing = sndg.getProfile().getFreezingLevel();
  
  // read in storm file header

  if (_sfile.ReadHeader()) {
    cerr << "ERROR - " << _progName << "Identify::_processClumps" << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }

  // write the scan header and global props

  if (_sfile.WriteScan(scan_num)) {
    cerr << "ERROR - " << _progName << "Identify::_processClumps" << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }

  // update the file header
  
  if (scan_num == 0) {
    _sfile._header.start_time = scan_hdr.time;
  }
  
  _sfile._header.end_time = scan_hdr.time;
  
  // fill in number of scans, tracks and current file size
  
  _sfile._header.n_scans = scan_num + 1;
  
  // rewrite header
  
  if (_sfile.WriteHeader()) {
    cerr << "ERROR - " << _progName << "Identify::_processClumps" << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }
  _sfile.FlushFiles();

  // printout

  if (_params.debug) {
    fprintf(stderr, "========== identification ========\n");
    fprintf(stderr, "Scan number %d\n", scan_num);
    fprintf(stderr, "  nstorms : %d\n", _nStorms);
    fprintf(stderr, "  Time: %s\n",
	    utimstr(_sfile._header.end_time));
  }
    
  return (0);
  
}

/////////////////////////////////
// _processThisClump()
//

int Identify::_processThisClump(const GridClump &grid_clump)

{

  // check size

  if (grid_clump.stormSize < _params.min_storm_size ||
      grid_clump.stormSize > _params.max_storm_size) {
    return(0);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    fprintf(stderr,
	    "Clump: size, startIx, startIy, nx, ny, offsetx, offsety: "
	    "%g, %d, %d, %d, %d, %g, %g\n",
	    grid_clump.stormSize,
	    grid_clump.startIx, grid_clump.startIy,
	    grid_clump.nX, grid_clump.nY,
	    grid_clump.offsetX, grid_clump.offsetY);
  }
  
  _sfile.AllocGprops(_nStorms + 1);

  if (_props->compute(grid_clump, _nStorms) == 0) {
    
    // success - write the storm props to storm file

    if (_sfile.WriteProps(_nStorms)) {
      cerr << "ERROR - " << _progName << "Identify::_processThisClump" << endl;
      cerr << _sfile.getErrStr() << endl;
      return(-1);
    }

    _nStorms++;

  }

  return (0);

}




