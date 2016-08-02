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
////////////////////////////////////////////////////////////////////////
// Tstorms2Spdb.cc
//
// Tstorms2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2002
//
///////////////////////////////////////////////////////////////
//
// Tstorms2Symprod reads native TITAN data files, converts the
// data into rapformats/tstorm_spdb.h style structs and writes
// the data out to SPDB.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <toolsa/ucopyright.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <didss/RapDataDir.hh>
#include <titan/TitanSpdb.hh>
#include <Spdb/DsSpdb.hh>
#include "Tstorms2Spdb.hh"
using namespace std;

// Constructor

Tstorms2Spdb::Tstorms2Spdb(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "Tstorms2Spdb";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = const_cast<char*>(string("unknown").c_str());
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // check args in ARCHIVE mode
  
  if (_params.mode == Params::ARCHIVE) {
    if (_args.inputFileList.size() == 0) {
      if ((_args.startTime == 0 || _args.endTime == 0)) {
	cerr << "ERROR: " << _progName << endl;
	cerr << "In ARCHIVE mode, you must specify a file list" << endl
	     << "  or start and end times." << endl;
	isOK = FALSE;
	return;
      }
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // set up input object

  if (_params.mode == Params::ARCHIVE) {
    if (_args.inputFileList.size() > 0) {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _args.inputFileList);
      _input->setSearchExt("th5");
    } else if (_args.startTime != 0 && _args.endTime != 0) {
      string inDir;
      RapDataDir.fillPath(_params.input_dir, inDir);
      if (_params.debug) {
	cerr << "Input dir: " << inDir << endl;
      }
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       inDir,
			       _args.startTime,
			       _args.endTime);
      _input->setSearchExt("th5");
    }
  } else {
    string inDir;
    RapDataDir.fillPath(_params.input_dir, inDir);
    if (_params.debug) {
      cerr << "Input dir: " << inDir << endl;
    }
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     inDir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register);
  }

  return;

}

// destructor

Tstorms2Spdb::~Tstorms2Spdb()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Tstorms2Spdb::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  if (_params.mode == Params::ARCHIVE) {
    _input->reset();
  }

  char *inputFilePath;
  while ((inputFilePath = _input->next()) != NULL) {
  
    if (_params.debug) {
      cerr << "Processing input file: " << inputFilePath << endl;
    }

    _processTrackFile(inputFilePath);
    
  }

  return 0;

}

//////////////////////////////////////////////////
// process track file

int Tstorms2Spdb::_processTrackFile (const char *input_file_path)

{

  TitanTrackFile tFile;
  TitanStormFile sFile;

  // open files

  if (_openFiles(input_file_path, tFile, sFile)) {
    return -1;
  }

  // load up scan times

  vector<time_t> scanTimes;

  if (_loadScanTimes(sFile, scanTimes)) {
    return -1;
  }

  if (_params.mode == Params::REALTIME) {

    // REALTIME mode - find the scan which matches the latest data info
    // and only process that scan

    time_t valid_time = _input->getLdataInfo().getLatestTime();

    for (size_t iscan = 0; iscan < scanTimes.size(); iscan++) {
      if (scanTimes[iscan] == valid_time) {
	time_t expire_time;
	if (iscan == 0) {
	  expire_time = valid_time;
	} else {
	  expire_time = valid_time + (valid_time - scanTimes[iscan - 1]);
	}
	_processScan(sFile, tFile, iscan, valid_time, expire_time);
	break;
      }
    }

  } else {

    // ARCHIVE mode - process all scans
    
    for (size_t iscan = 0; iscan < scanTimes.size(); iscan++) {
      time_t valid_time = scanTimes[iscan];
      time_t expire_time;
      if (scanTimes.size() == 1) {
	expire_time = valid_time;
      } else {
	if (iscan == scanTimes.size() - 1) {
	  expire_time = valid_time + (valid_time - scanTimes[iscan - 1]);
	} else {
	  expire_time = scanTimes[iscan + 1];
	}
      }
      _processScan(sFile, tFile, iscan, valid_time, expire_time);
    }

  }


  return 0;

}

//////////////////////////////////////////////////
// open track and storm files

int Tstorms2Spdb::_openFiles(const char *input_file_path,
			     TitanTrackFile &tFile,
			     TitanStormFile &sFile)

{

  char track_file_path[MAX_PATH_LEN];
  STRncopy(track_file_path, input_file_path, MAX_PATH_LEN);
  if (_params.mode == Params::REALTIME) {
    // In Realtime mode the latest data info file has
    // the storm file in it instead of the track file so
    // we need to change the 'sh' to a 'th'.
    char *sh = strstr(track_file_path, "sh");
    if (sh) {
      *sh = 't';
    }
  }

  if (tFile.OpenFiles("r", track_file_path)) {
    cerr << "ERROR - Tstorms2Spdb::_openFiles" << endl;
    cerr << "  " << tFile.getErrStr() << endl;
    return -1;
  }

  Path stormPath(track_file_path);
  stormPath.setFile(tFile.header().storm_header_file_name);

  if (sFile.OpenFiles("r", stormPath.getPath().c_str())) {
    cerr << "ERROR - Tstorms2Spdb::_openFiles" << endl;
    cerr << "  " << sFile.getErrStr() << endl;
    return -1;
  }
  
  // lock files

  if (tFile.LockHeaderFile("r")) {
    cerr << "ERROR - Tstorms2Spdb::_openFiles" << endl;
    cerr << "  " << tFile.getErrStr() << endl;
    return -1;
  }
  if (sFile.LockHeaderFile("r")) {
    cerr << "ERROR - Tstorms2Spdb::_openFiles" << endl;
    cerr << "  " << sFile.getErrStr() << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// load up scan times from storm file

int Tstorms2Spdb::_loadScanTimes(TitanStormFile &sFile,
				 vector<time_t> &scanTimes)

{

  int nScans = sFile.header().n_scans;
  for (int i = 0; i < nScans; i++) {
    // read in scan
    if (sFile.ReadScan(i)) {
      cerr << "ERROR - Tstorms2Spdb::_loadScanTimes" << endl;
      cerr << "  " << sFile.getErrStr() << endl;
      return -1;
    }
    scanTimes.push_back(sFile.scan().time);
  }

  return 0;

}

int Tstorms2Spdb::_processScan(TitanStormFile &sFile,
			       TitanTrackFile &tFile,
			       int scan_num,
			       time_t valid_time,
			       time_t expire_time)

{

  if (_params.debug) {
    cerr << "Processing scan num: " << scan_num << endl;
    cerr << "  Valid time: " << DateTime::str(valid_time) << endl;
    cerr << "  Expire time: " << DateTime::str(expire_time) << endl;
  }

  // read in track file scan entries

  if (tFile.ReadScanEntries(scan_num)) {
    cerr << "ERROR - Tstorms2Spdb::_processScan" << endl;
    cerr << "  " << sFile.getErrStr() << endl;
    return -1;
  }

  // read in storm file scan

  if (sFile.ReadScan(scan_num)) {
    cerr << "ERROR - Tstorms2Spdb::_processScan" << endl;
    cerr << "  " << sFile.getErrStr() << endl;
    return -1;
  }

  // load up scan header

  const storm_file_scan_header_t &scan = sFile.scan();
  int n_entries = tFile.scan_index()[scan_num].n_entries;

  /*
   * load header
   */

  tstorm_spdb_header_t tstorm_header;
  TitanSpdb::loadHeader(tstorm_header, sFile.params(), scan.grid,
			valid_time, n_entries);

  // allocate buffer
  
  int buffer_len = tstorm_spdb_buffer_len(&tstorm_header);
  TaArray<ui08> buffer_;
  ui08 *output_buffer = buffer_.alloc(buffer_len);

  // copy header into buffer, put into BE ordering
  
  memcpy(output_buffer, &tstorm_header, sizeof(tstorm_spdb_header_t));
  tstorm_spdb_header_to_BE((tstorm_spdb_header_t *) output_buffer);
  
  // initialize projection

  titan_grid_comps_t grid_comps;
  TITAN_init_proj(&sFile.scan().grid, &grid_comps);

  // loop through the entries
  
  const track_file_entry_t *scan_entries = tFile.scan_entries();
  tstorm_spdb_entry_t *tstorm_entry =
    (tstorm_spdb_entry_t *) (output_buffer + sizeof(tstorm_spdb_header_t));

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  Nentries in scan: " << n_entries << endl;
  }
  
  for (int ientry = 0; ientry < n_entries;
       ientry++, scan_entries++, tstorm_entry++) {

    const track_file_forecast_props_t *fprops = &scan_entries->dval_dt;
    const storm_file_global_props_t *gprops =
      sFile.gprops() + scan_entries->storm_num;
    
    // load up entry struct
    
    TitanSpdb::loadEntry(tstorm_header, *scan_entries,
			 *gprops, *fprops, grid_comps, *tstorm_entry);
    
    //  set entry to BE ordering
    
    tstorm_spdb_entry_to_BE(tstorm_entry);

  } // ientry
  
  // write buffer to SPDB

  DsSpdb spdb;
  if (spdb.put(_params.output_url,
	       SPDB_TSTORMS_ID,
	       SPDB_TSTORMS_LABEL,
	       SPDB_TSTORMS_PROD_TYPE,
	       valid_time, expire_time,
	       buffer_len, output_buffer,
	       SPDB_TSTORMS_ID)) {
    cerr << "ERROR - _processScan" << endl;
    cerr << "  " << spdb.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "  Wrote SPDB data to URL: " << _params.output_url << endl;
  }
  
  return 0;

}

