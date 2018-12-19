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
///////////////////////////////////////////////////////////////
// ClutterMap.cc
//
// ClutterMap object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2004
//
///////////////////////////////////////////////////////////////
//
// ClutterMap computes clutter auto-correlation statistics
//
////////////////////////////////////////////////////////////////

#include "ClutterMap.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

using namespace std;

// Constructor

ClutterMap::ClutterMap(int argc, char **argv)

{

  isOK = TRUE;
  _trigger = NULL;
  _nTimes = 0;
  _nPointsGrid = 0;
  _nn = NULL;
  _dbzThresh = NULL;
  _sumDbz = NULL;
  _meanDbz = NULL;
  _frac = NULL;
   
  // set programe name
  
  _progName = "ClutterMap";
  ucopyright((char *) _progName.c_str());
  
  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // check params
  
  // check start and end in ARCHIVE mode

  if ((_params.mode == Params::ARCHIVE) &&
      (_args.startTime == 0 || _args.endTime == 0)) {
    cerr << "ERROR - ClutterMap" << endl;
    cerr << "  In ARCHIVE mode start and end times must be specified." << endl;
    isOK = false;
    return;
  }

  // create the trigger

  if (_params.mode == Params::REALTIME) {
    _trigger = new RealtimeTrigger(_progName, _params);
  } else if (_params.mode == Params::ARCHIVE) {
    _trigger = new ArchiveTrigger(_progName, _params,
				  _args.startTime,  _args.endTime);
  } else {
    _trigger = new IntervalTrigger(_progName, _params,
				   _args.startTime,  _args.endTime);
  }

  // initialize process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
}

// destructor

ClutterMap::~ClutterMap()

{

  if (_trigger) {
    delete _trigger;
  }
  _freeArrays();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ClutterMap::Run ()
{

  PMU_auto_register("Start of Run");
  
  // loop through times
  
  while (_trigger->next(_dataStartTime, _dataEndTime) == 0) {
    
    PMU_auto_register("Run - got trigger");
    
    if (_params.debug) {
      cerr << "----> Triggering" << endl;
      cerr << "------> Data start time: " << utimstr(_dataStartTime) << endl;
      cerr << "------> Data end   time: " << utimstr(_dataEndTime) << endl;
    }
    
    // respond to the trigger
    
    if (_respondToTrigger()) {
      cerr << "ERROR - ClutterMap::Run" << endl;
      cerr << "  Computation failed" << endl;
      cerr << "  Data start time: " << utimstr(_dataStartTime) << endl;
      cerr << "  Data end   time: " << utimstr(_dataEndTime) << endl;
    }
    
  } // while
  
  return 0;
  
}

/////////////////////////////////////////////////////
// Respond to the trigger

int ClutterMap::_respondToTrigger()
{
  
  PMU_auto_register("_respondToTrigger");
  
  // get list of input files in the running time period
  
  time_t periodStart = _dataStartTime;
  time_t periodEnd = _dataEndTime;
  
  if (_params.debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Computing stats for period %s to %s\n",
	    utimstr(periodStart), utimstr(periodEnd));
  }
  
  DsMdvx mdvx;
  mdvx.setTimeListModeValid(_params.input_url, periodStart, periodEnd);
  if (mdvx.compileTimeList()) {
    cerr << "ERROR - ClutterMap::_respondToTrigger()" << endl;
    cerr << "  Cannot compile time list" << endl;
    cerr << "  " << mdvx.getErrStr() << endl;
    return -1;
  }
  vector<time_t> times(mdvx.getTimeList());
  if (times.size() < 1) {
    cerr << "ERROR - ClutterMap::_respondToTrigger()" << endl;
    cerr << "  Too few data times found: " << times.size() << endl;
    return -1;
  }

  // initialize from first time
  
  if (_initArrays(times[0]) != 0) {
    return -1;
  }

  // accumulate clutter stats

  _nTimes = 0;
  if (_params.input_intermediate_results) {
    _accumStatsFromIntermediate(times);
  } else {
    _accumStatsFromRaw(times);
  }

  if (_params.output_intermediate_results) {

    // compute mean dbz field

    _computeMeanDbz();

    // write out intermediate results
    
    if (_writeIntermediate(periodStart, periodEnd, periodEnd,
			   _mhdr, _fhdr, _vhdr,
			   _nTimes, _nn, _meanDbz)) {
      cerr << "ERROR - ClutterMap::_respondToTrigger" << endl;
      cerr << "  Could not write out intermediate data" << endl;
      return -1;
    }

  } else {

    // check for min data times
    
    if (_nTimes < _params.min_data_times) {
      cerr << "ERROR - ClutterMap::_respondToTrigger()" << endl;
      cerr << "  Too few data times found: " << _nTimes << endl;
      return -1;
    }
    
    // identify the clutter

    _identifyClutter();

    // write out final results

    if (_writeFinal(periodStart, periodEnd, periodEnd,
		    _mhdr, _fhdr, _vhdr,
		    _nTimes, _frac, _meanDbz)) {
      cerr << "ERROR - ClutterMap::_respondToTrigger" << endl;
      cerr << "  Could not write out final data" << endl;
      return -1;
    }
    
  }

  return 0;

}

/////////////////////////////////////////////////////
// Initialize arrays from initial time

int ClutterMap::_initArrays(time_t initTime)

{
  
  // read in the dbz data field, from which to determine geometry
  
  DsMdvx mdvx;
  mdvx.setReadTime(Mdvx::READ_CLOSEST, _params.input_url, 0, initTime);
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  if (_params.set_vlevel_limits) {
    mdvx.setReadVlevelLimits(_params.min_v_level, _params.max_v_level);
  }
  string dbzFieldName = _params.dbz_field_name;
  if (_params.input_intermediate_results) {
    dbzFieldName = "DBZMean";
  }
  mdvx.addReadField(dbzFieldName);
  
  if (mdvx.readVolume()) {
    cerr << "ERROR - ClutterMap::_initArrays" << endl;
    cerr << "  " << mdvx.getErrStr() << endl;
    cerr << "  Cannot read in DBZ data" << endl;
    cerr << "  Check dbz_field_name parameter" << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Initializing using file: " << mdvx.getPathInUse() << endl;
  }
  
  _mhdr = mdvx.getMasterHeader();
  _fhdr = mdvx.getField(0)->getFieldHeader();
  _vhdr = mdvx.getField(0)->getVlevelHeader();

  const MdvxField &dbzFld = *mdvx.getField(0);
  const Mdvx::field_header_t &fhdr = dbzFld.getFieldHeader();

  // set geometry

  _nx = fhdr.nx;
  _ny = fhdr.ny;
  _nz = fhdr.nz;
  _nPointsGrid = _nx * _ny * _nz;

  // allocate arrays

  _allocArrays();

  // initialize arrays
  
  memset(_nn, 0, _nPointsGrid * sizeof(fl32));
  memset(_sumDbz, 0, _nPointsGrid * sizeof(fl32));
  memset(_frac, 0, _nPointsGrid * sizeof(fl32));
  for (int ii = 0; ii < _nPointsGrid; ii++) {
    _meanDbz[ii] = missingVal;
  }

  // if SNR data not available, set up minDbz grid

  if (!_params.use_snr_data) {
    _initMinDbz(fhdr);
  }

  return 0;
  
}

//////////////////////////
// allocate arrays

void ClutterMap::_allocArrays()

{
  
  _freeArrays();

  _nn = new fl32[_nPointsGrid];
  _sumDbz = new fl32[_nPointsGrid];
  _meanDbz = new fl32[_nPointsGrid];
  _frac = new fl32[_nPointsGrid];
  if (!_params.use_snr_data) {
    _dbzThresh = new fl32[_nPointsGrid];
  }

}

//////////////////////////
// delete arrays

void ClutterMap::_freeArrays()

{

  if (_dbzThresh) {
    delete[] _dbzThresh;
  }
  if (_nn) {
    delete[] _nn;
  }
  if (_sumDbz) {
    delete[] _sumDbz;
  }
  if (_meanDbz) {
    delete[] _meanDbz;
  }
  if (_frac) {
    delete[] _frac;
  }

}

///////////////////////////
// initialze min DBZ array

void ClutterMap::_initMinDbz(const Mdvx::field_header_t &fhdr)

{
  
  if (_params.threshold_field == Params::DBZ_THRESHOLD) {
    
    for (int ii = 0; ii < _nPointsGrid; ii++) {
      _dbzThresh[ii] = _params.min_dbz;
    }
    
  } else if (fhdr.proj_type == Mdvx::PROJ_FLAT) {

    int nxy = _nx * _ny;
    
    for (int iy = 0; iy < _ny; iy++) {
      
      double yy = fhdr.grid_miny + iy * fhdr.grid_dy;

      for (int ix = 0; ix < _nx; ix++) {
	
	double xx = fhdr.grid_minx + ix * fhdr.grid_dx;
	double range = sqrt(xx * xx + yy * yy);
	double diffDb = 20.0 * log10(range);
	double minDbz = _params.noise_dbz_at_1km + diffDb + _params.min_snr;
	fl32 *md = _dbzThresh + iy * _nx + ix;
	for (int iz = 0; iz < _nz; iz++, md += nxy) {
	  *md = minDbz;
	} // iz

      } // ix
    } // iy
    
  } else if (fhdr.proj_type == Mdvx::PROJ_POLAR_RADAR) {

    fl32 minDbz[_nx];
    double range = fhdr.grid_minx;
    for (int ix = 0; ix < _nx; ix++, range += fhdr.grid_dx) {
      double diffDb = 20.0 * log10(range);
      minDbz[ix] =
	(fl32) (_params.noise_dbz_at_1km + diffDb + _params.min_snr);
    }
    
    for (int iz = 0; iz < _nz; iz++) {
      for (int iy = 0; iy < _ny; iy++) {
	fl32 *md = _dbzThresh + (iz * _nx * _ny);
	memcpy(md, minDbz, sizeof(minDbz));
      }
    }

  } else {

    cerr << "WARNING - ClutterMap::_initMinDbz" << endl;
    cerr << "  Cannot use SNR threshold for this projection type" << endl;
    cerr << "  Proj type: " << Mdvx::projType2Str(fhdr.proj_type) << endl;
    cerr << "  Using min Dbz instead: " << _params.min_dbz << endl;
    
    for (int ii = 0; ii < _nPointsGrid; ii++) {
      _dbzThresh[ii] = _params.min_dbz;
    }
    
  }

}

//////////////////////////////////////////////////////
// accumulate stats from raw data

int ClutterMap::_accumStatsFromRaw(const vector<time_t> &times)

{
  
  // loop through times
  
  for (size_t itime = 0; itime < times.size(); itime++) {
    
    time_t fileTime = times[itime];

    // read in the file
    
    DsMdvx mdvx;
    mdvx.setReadTime(Mdvx::READ_CLOSEST, _params.input_url, 0, fileTime);
    mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    if (_params.set_vlevel_limits) {
      mdvx.setReadVlevelLimits(_params.min_v_level, _params.max_v_level);
    }
    mdvx.addReadField(_params.dbz_field_name);
    if (_params.use_snr_data) {
      mdvx.addReadField(_params.snr_field_name);
    }
    if (_params.use_vel_data) {
      mdvx.addReadField(_params.vel_field_name);
    }
    if (_params.use_width_data) {
      mdvx.addReadField(_params.width_field_name);
    }
    
    if (mdvx.readVolume()) {
      cerr << "ERROR - ClutterMap::_accumStatsFromRaw" << endl;
      cerr << "  Cannot read in raw data" << endl;
      cerr << "  " << mdvx.getErrStr() << endl;
      cerr << "  Check dbz_field_name parameter:"
	   << _params.dbz_field_name << endl;
      if (_params.use_snr_data) {
	cerr << "  Check snr_field_name parameter:"
	     << _params.snr_field_name << endl;
      }
      if (_params.use_vel_data) {
	cerr << "  Check vel_field_name parameter:"
	     << _params.vel_field_name << endl;
      }
      if (_params.use_width_data) {
	cerr << "  Check width_field_name parameter:"
	     << _params.width_field_name << endl;
      }
      return -1;
    }

    if (_params.debug) {
      cerr << "  Processing file: " << mdvx.getPathInUse() << endl;
    }
    
    // check grid size
    
    const MdvxField &dbzFld = *mdvx.getField(_params.dbz_field_name);
    const Mdvx::field_header_t &fhdr = dbzFld.getFieldHeader();
    
    if (_nx != fhdr.nx || _ny != fhdr.ny || _nz != fhdr.nz) {
      cerr << "ERROR - ClutterMap::_accumStatsFromRaw" << endl;
      cerr << "  Grid size has changed" << endl;
      cerr << "  Orig nx, ny, nz: "
	   << _nx << ", " << _ny << ", " << _nz << endl;
      cerr << "  This file nx, ny, nz: "
	   << fhdr.nx << ", " << fhdr.ny << ", " << fhdr.nz << endl;
      return -1;
    }

    // set fields

    fl32 *dbz = (fl32 *) dbzFld.getVol();
    fl32 *vel = NULL, *width = NULL, *snr = NULL;
    fl32 dbzMissing = dbzFld.getFieldHeader().missing_data_value;
    fl32 snrMissing = 0;
    if (_params.use_vel_data) {
      vel = (fl32 *) mdvx.getField(_params.vel_field_name)->getVol();
    }
    if (_params.use_width_data) {
      width = (fl32 *) mdvx.getField(_params.width_field_name)->getVol();
    }
    if (_params.use_snr_data) {
      snr = (fl32 *) mdvx.getField(_params.snr_field_name)->getVol();
      snrMissing =
	mdvx.getField(_params.snr_field_name)->getFieldHeader().missing_data_value;
    }

    // loop through grid, considering points for clutter comps
    
    for (int ii = 0; ii < _nPointsGrid; ii++) {
      
      // does this point meet the criteria?

      if (_params.use_snr_data) {
        if (snr[ii] == snrMissing ||
	    snr[ii] < _params.min_snr) {
          continue;
        }
      } else {
        if (dbz[ii] == dbzMissing ||
	    dbz[ii] < _dbzThresh[ii]) {
          continue;
        }
      }
      
      if (_params.use_vel_data) {
	if (fabs(vel[ii]) > _params.max_abs_vel) {
	  continue;
	}
      }

      if (_params.use_width_data) {
	if (width[ii] > _params.max_width) {
	  continue;
	}
      }

      _nn[ii]++;
      _sumDbz[ii] += dbz[ii];

    } // ii

    _nTimes++;
    
  } // itime

  return 0;

}

//////////////////////////////////////////////////////
// accumulate stats from intermediate data

int ClutterMap::_accumStatsFromIntermediate(const vector<time_t> &times)

{
  
  // loop through times
  
  for (size_t itime = 0; itime < times.size(); itime++) {
    
    time_t fileTime = times[itime];
    
    // read in the file
    
    DsMdvx mdvx;
    mdvx.setReadTime(Mdvx::READ_CLOSEST, _params.input_url, 0, fileTime);
    mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    
    if (mdvx.readVolume()) {
      cerr << "ERROR - ClutterMap::_accumStatsFromIntermediate" << endl;
      cerr << "  Cannot read in intermediate data" << endl;
      cerr << "  " << mdvx.getErrStr() << endl;
      return -1;
    }

    if (_params.debug) {
      cerr << "  Processing file: " << mdvx.getPathInUse() << endl;
    }

    // check fields exist

    const MdvxField *dbzMeanFld = mdvx.getField("DBZMean");
    const MdvxField *nnFld = mdvx.getField("nn");
    
    if (dbzMeanFld == NULL || nnFld == NULL) {
      cerr << "ERROR - ClutterMap::_accumStatsFromIntermediate" << endl;
      cerr << "  Cannot find 'dbzMean' and/or 'nn' fields" << endl;
      cerr << "  Check that this directory has intermediate files" << endl;
      cerr << "  Input URL: " << _params.input_url << endl;
      return -1;
    }

    // check grid geometry
    
    const Mdvx::field_header_t &dbzFhdr = dbzMeanFld->getFieldHeader();
    
    if (_nx != dbzFhdr.nx || _ny != dbzFhdr.ny || _nz != dbzFhdr.nz) {
      cerr << "ERROR - ClutterMap::_clutterIdent" << endl;
      cerr << "  Grid size has changed" << endl;
      cerr << "  Orig nx, ny, nz: "
	   << _nx << ", " << _ny << ", " << _nz << endl;
      cerr << "  This file nx, ny, nz: "
	   << dbzFhdr.nx << ", " << dbzFhdr.ny << ", " << dbzFhdr.nz << endl;
      return -1;
    }

    // set field data
    
    fl32 *dbzMean = (fl32 *) dbzMeanFld->getVol();
    fl32 *nn = (fl32 *) nnFld->getVol();
    int nTimesIntermediate = mdvx.getMasterHeader().user_data;

    // loop through grid, accumulating stats from intermediate file
    
    for (int ii = 0; ii < _nPointsGrid; ii++) {
      
      _nn[ii] += nn[ii];
      _sumDbz[ii] += (dbzMean[ii] * nn[ii]);
      
    } // ii

    // number of times used to compute intermediate data is stored in the
    // master header user_data field

    _nTimes += nTimesIntermediate;
    
  } // itime

  return 0;

}


//////////////////////////////////////////////////////
// identify the clutter from stats

void ClutterMap::_computeMeanDbz()

{
  
  // loop through grid, identifying clutter, compute mean dbz
  
  for (int ii = 0; ii < _nPointsGrid; ii++) {
    
    if (_nn[ii] > 0) {
      _meanDbz[ii] = _sumDbz[ii] / _nn[ii];
    }
    
  } // ii

}

//////////////////////////////////////////////////////
// identify the clutter from stats

void ClutterMap::_identifyClutter()

{
  
  // loop through grid, identifying clutter, compute mean dbz
  
  for (int ii = 0; ii < _nPointsGrid; ii++) {
    
    fl32 fraction = (double) _nn[ii] / (double) _nTimes;

    // if not persistent enough, clear this point
    
    if (fraction >=  _params.min_clutter_time_fraction) {
      _meanDbz[ii] = _sumDbz[ii] / _nn[ii];
    }
    _frac[ii] = fraction;
    
  } // ii

}

////////////////////////////////////////
// writeIntermediate()
//
// Write out intermediate data
//

int ClutterMap::_writeIntermediate(time_t start_time,
				   time_t end_time,
				   time_t centroid_time,
				   const Mdvx::master_header_t &mhdrIn,
				   const Mdvx::field_header_t &fhdrIn,
				   const Mdvx::vlevel_header_t &vhdrIn,
				   int nTimes,
				   const fl32 *nn,
				   const fl32 *meanDbz)

{
  
  DsMdvx mdvx;
  
  // set the master header
  
  Mdvx::master_header_t mhdr = mhdrIn;
  
  mhdr.time_begin = start_time;
  mhdr.time_end = end_time;
  mhdr.time_centroid = centroid_time;
  mhdr.time_expire = end_time;
  
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;

  // store number of times in master_header user_data

  mhdr.user_data = nTimes;
  
  // info strings
  
  mdvx.setDataSetInfo
    ("These intermediate fields will be used by ClutterMap "
     "to compute final fields");
  mdvx.setDataSetName("Intermediate fields generated by ClutterMap");
  mdvx.setDataSetSource("ClutterMap");
  
  mdvx.setMasterHeader(mhdr);
  
  // fill in field headers and vlevel headers
  
  mdvx.clearFields();
  
  Mdvx::field_header_t fhdr = fhdrIn;
  Mdvx::vlevel_header_t vhdr = vhdrIn;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.bad_data_value = missingVal;
  fhdr.missing_data_value = missingVal;
  
  // add mean dbz field
  
  MdvxField *dbzField = new MdvxField(fhdr, vhdr, meanDbz);
  dbzField->setFieldName("DBZMean");
  dbzField->setFieldNameLong("Mean clutter DBZ");
  dbzField->setUnits("dBZ");
  dbzField->setTransform("dBZ");
  dbzField->convertRounded((Mdvx::encoding_type_t) _params.output_encoding_type,
                           Mdvx::COMPRESSION_GZIP);
  mdvx.addField(dbzField);
    
  // add nn field
  
  MdvxField *nnField = new MdvxField(fhdr, vhdr, nn);
  nnField->setFieldName("nn");
  nnField->setFieldNameLong("n times clutter found");
  nnField->setUnits("count");
  nnField->setTransform("");
  nnField->convertRounded((Mdvx::encoding_type_t) _params.output_encoding_type,
                          Mdvx::COMPRESSION_GZIP);
  mdvx.addField(nnField);

  // set to write latest data info

  mdvx.setWriteLdataInfo();
  
  if (_params.debug) {
    cerr << "Writing intermediate data, for time: "
	 << DateTime::str(centroid_time) << " to url:" 
	 << _params.output_url << endl;
  }
  
  if (mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - OutputFile::writeIntermediate()" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  return 0;
  
}

////////////////////////////////////////
// writeFinal()
//
// Write out final data
//

int ClutterMap::_writeFinal(time_t start_time,
			    time_t end_time,
			    time_t centroid_time,
			    const Mdvx::master_header_t &mhdrIn,
			    const Mdvx::field_header_t &fhdrIn,
			    const Mdvx::vlevel_header_t &vhdrIn,
			    int nTimes,
			    const fl32 *frac,
			    const fl32 *meanDbz)
  
{
  
  DsMdvx mdvx;
  
  // set the master header
  
  Mdvx::master_header_t mhdr = mhdrIn;
  
  mhdr.time_begin = start_time;
  mhdr.time_end = end_time;
  mhdr.time_centroid = centroid_time;
  mhdr.time_expire = end_time;
  
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;

  // store number of times in master_header user_data
  
  mhdr.user_data = nTimes;
  
  // info strings
  
  mdvx.setDataSetInfo
    ("The meanDbz field is the mean value of the clutter at the point");
  mdvx.setDataSetName("Final fields generated by ClutterMap");
  mdvx.setDataSetSource("ClutterMap");
  
  mdvx.setMasterHeader(mhdr);
  
  // fill in field headers and vlevel headers
  
  mdvx.clearFields();
  
  Mdvx::field_header_t fhdr = fhdrIn;
  Mdvx::vlevel_header_t vhdr = vhdrIn;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  int npts = fhdr.nx * fhdr.ny * fhdr.nz;
  fhdr.volume_size = npts * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.bad_data_value = missingVal;
  fhdr.missing_data_value = missingVal;
  
  // add mean dbz field
  
  MdvxField *dbzField = new MdvxField(fhdr, vhdr, meanDbz);
  dbzField->setFieldName("DBZMean");
  dbzField->setFieldNameLong("Mean clutter DBZ");
  dbzField->setUnits("dBZ");
  dbzField->setTransform("dBZ");
  dbzField->convertRounded((Mdvx::encoding_type_t) _params.output_encoding_type,
                           Mdvx::COMPRESSION_GZIP);
  mdvx.addField(dbzField);
    
  // add clutter fraction field

  MdvxField *fracField = new MdvxField(fhdr, vhdr, frac);
  fracField->setFieldName("TimeFrac");
  fracField->setFieldNameLong("Fraction of time clutter found");
  fracField->setUnits("fraction");
  fracField->setTransform("");
  fracField->convertRounded((Mdvx::encoding_type_t) _params.output_encoding_type,
                            Mdvx::COMPRESSION_GZIP);
  mdvx.addField(fracField);
  
  // set to write latest data info
  
  mdvx.setWriteLdataInfo();
  
  if (_params.debug) {
    cerr << "Writing final data, for time: "
	 << DateTime::str(centroid_time) << " to url:" 
	 << _params.output_url << endl;
  }
  
  if (mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - OutputFile::writeFinal()" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  return 0;
  
}





