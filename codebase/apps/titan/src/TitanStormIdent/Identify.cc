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
#include "GridClump.hh"
#include <titan/TitanSpdb.hh>
#include <Spdb/DsSpdb.hh>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/pmu.h>

//////////////
// constructor
//

Identify::Identify(const string &prog_name, const Params &params,
		   const InputMdv &input_mdv, TitanStormFile &storm_file,
		   const ThresholdManager &thresholds):
  Worker(prog_name, params),
  _inputMdv(input_mdv),
  _thresholds(thresholds),
  _sfile(storm_file),
  _clumping(prog_name),
  _time(0),
  _lead(0),
  _hasLead(false)

{
  _props = NULL;
  _props = new Props(_progName, _params, _inputMdv, _thresholds, _sfile);
}

/////////////
// destructor
//

Identify::~Identify()

{
  if (_props) {
    delete (_props);
  }
}

//////////////////////////////////////////////////////
// run()
//
// Perform storm identification for given scan number
//
// Returns 0 on success, -1 on failure.
int Identify::run(int scan_num, time_t scan_time)
{
  _time = scan_time;
  _lead= 0;
  _hasLead = false;
  return _run(scan_num);
}

int Identify::run(int scan_num, time_t scan_gen_time, int scan_lead_time)

{
  _time = scan_gen_time;
  _lead= scan_lead_time;
  _hasLead = true;
  return _run(scan_num);
}  

int Identify::_run(int scan_num)
{

  PMU_auto_register("Identifying storms");
    
  // initialize

  _nStorms = 0;
  const titan_grid_t &grid = _inputMdv.grid;
  int nBytesPlane = grid.nx * grid.ny;
  int nplanes = _inputMdv.nPlanes;

  // perform clumping

  fl32 *startDbz = _inputMdv.dbzVol + _inputMdv.minValidLayer * nBytesPlane;
  _nClumps = _clumping.performClumping(grid.nx, grid.ny, nplanes,
                                       startDbz,
                                       _params.min_grid_overlap,
				       _thresholds.get_low_threshold());
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "Number of clumps  =  %d\n", _nClumps);
  }

  // process the clumps found

  if (_processClumps(scan_num)) {
    return (-1);
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
  // initialize the computation module for storm props
  
  _props->init();

  // loop through the clumps - index starts at 1
  
  Clump_order *clump = _clumping.clumps + 1;
  
  for (int iclump = 0; iclump < _nClumps; iclump++, clump++) {
    
    GridClump gridClump(clump, _inputMdv.grid, 0, 0);
    
    if (_processThisClump(gridClump)) {
      return (-1);
    }
      
    // }

  } // iclump
  

  // produce output and write it out
  return _output(scan_num);
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

  if (_params.debug >= Params::DEBUG_VERBOSE) {
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
    
    // success - write the storm props to storm file (not)

    _nStorms++;

  }

  return (0);

}



//
// produce and write out output
//

int Identify::_output(int scan_num)
{
  // load up scan structure
  
  storm_file_scan_header_t &scan_hdr = _sfile._scan;
  
  scan_hdr.nbytes_char = TITAN_N_GRID_LABELS * TITAN_GRID_UNITS_LEN;
  scan_hdr.scan_num = scan_num;
  scan_hdr.nstorms = _nStorms;
  scan_hdr.time = _inputMdv.mdvx.getMasterHeader().time_centroid;
  scan_hdr.min_z = _props->minValidZ;
  scan_hdr.delta_z = _inputMdv.grid.dz;
  scan_hdr.grid = _inputMdv.grid;

  // for now height of freezing is a static user parameter
  // eventually it may be derived dynamically from a sounding

  scan_hdr.ht_of_freezing = _params.ht_of_freezing;
  
  // update the file header
  
  if (scan_num == 0) {
    _sfile._header.start_time = scan_hdr.time;
  }
  
  _sfile._header.end_time = scan_hdr.time;
  
  // fill in number of scans, tracks and current file size
  
  _sfile._header.n_scans = scan_num + 1;
  
  // printout

  if (_params.debug) {
    fprintf(stderr, "========== identification ========\n");
    fprintf(stderr, "Scan number %d\n", scan_num);
    fprintf(stderr, "  nstorms : %d\n", _nStorms);
    fprintf(stderr, "  Time: %s\n", utimstr(_time));
    if (_hasLead) {
      fprintf(stderr, "  lead: %d\n", _lead);
      if (_lead == 3600)
      {
	printf("HERE\n");
      }
    }
  }
    
  // load up scan header
  tstorm_spdb_header_t tstorm_header;
  int n_entries = _sfile._scan.nstorms;
  TitanSpdb::loadHeader(tstorm_header, _sfile.params(), _sfile._scan.grid,
			_sfile._header.end_time, n_entries);

  
  // allocate buffer
  
  int buffer_len = tstorm_spdb_buffer_len(&tstorm_header);
  TaArray<ui08> buffer_;
  ui08 *output_buffer = buffer_.alloc(buffer_len);

  // copy header into buffer, put into BE ordering
  
  memcpy(output_buffer, &tstorm_header, sizeof(tstorm_spdb_header_t));
  tstorm_spdb_header_to_BE((tstorm_spdb_header_t *) output_buffer);
  
  // initialize projection

  titan_grid_comps_t grid_comps;
  TITAN_init_proj(&_sfile.scan().grid, &grid_comps);

  // make a fake track_file_entry_t,

  track_file_entry_t scan_entry;

  // set simple id number to the clump index

  scan_entry.simple_track_num = 0;

  // set  set complex id number either
  // to lead time (forecasts) or to the clump index (flat)

  if (_hasLead) {
    scan_entry.complex_track_num = _lead;
    scan_entry.forecast_valid = 1;
  } else {
    scan_entry.complex_track_num = 0;
    scan_entry.forecast_valid = 0;
  }

  // make a fake track_file_forecast_props_t
  // make sure values that are written out are set to something

  track_file_forecast_props_t fprops;
  fprops.smoothed_speed = 0;
  fprops.smoothed_direction = 0;
  fprops.proj_area = 0;
  fprops.mass = 0;


  // loop through the entries

  tstorm_spdb_entry_t *tstorm_entry =
    (tstorm_spdb_entry_t *) (output_buffer + sizeof(tstorm_spdb_header_t));

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  Nentries in scan: " << n_entries << endl;
  }
  
  
  for (int ientry = 0; ientry < n_entries; ientry++, scan_entry.simple_track_num++,
	 tstorm_entry++) {

    if (_hasLead) {
      scan_entry.complex_track_num = _lead;
    } else {
      scan_entry.complex_track_num = scan_entry.simple_track_num;
    }

    const storm_file_global_props_t *gprops = _sfile.gprops() + ientry;
    
    // load up entry struct
    
    TitanSpdb::loadEntry(tstorm_header, scan_entry,
			 *gprops, fprops, grid_comps, *tstorm_entry);
    
    //  set entry to BE ordering
    
    tstorm_spdb_entry_to_BE(tstorm_entry);

  } // ientry
  
  // write buffer to SPDB

  DsSpdb spdb;
  int id2;
  time_t validTime;
  if (_hasLead)  {
    spdb.setLeadTimeStorage(Spdb::LEAD_TIME_IN_DATA_TYPE2);
    id2 = _lead;
    validTime = _time + _lead;

    // NOTE: it appears putModeOver works in all cases because the
    // valid time AND the data_type2 and data_type are used to give
    // the put its position in the data base.  For our use, validTime
    // and id2 should be unique per put.  validTime alone is not unique

  } else {
    id2 = SPDB_TSTORMS_ID;
    validTime = _time;
  }

  if (spdb.put(_params.storm_data_dir,
	       SPDB_TSTORMS_ID,
	       SPDB_TSTORMS_LABEL,
	       SPDB_TSTORMS_PROD_TYPE,
	       validTime, validTime,
	       buffer_len, output_buffer,
	       id2)) {
    cerr << "ERROR - _processScan" << endl;
    cerr << "  " << spdb.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "  Wrote SPDB data to URL: " << _params.storm_data_dir << endl;
  }
  
  return (0);
  
}
