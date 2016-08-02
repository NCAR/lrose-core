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
//////////////////////////////////////////////////////////
// AccumData.cc
//
// Accumulation data object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
// Updated to Mdvx, Dixon, Nov 2000
//////////////////////////////////////////////////////////
//
// This module acummulates the precip data in a fl32 array.
//
// It has a list of input files to be used for accumulation.
// This list also shows which files have already been used.
//
// directory, computes the lookup table to convert this
// file to the output grid, and loads up the output
// grid with data from the file.
//
///////////////////////////////////////////////////////////

#include "AccumData.hh"
#include "OutputFile.hh"
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/TaArray.hh>
#include <toolsa/mem.h>
#include <physics/vil.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <Spdb/DsSpdb.hh>
#include <rapformats/zr.h>
#include <cmath>
using namespace std;

//////////////
// Constructor

AccumData::AccumData(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{

  _precip = NULL;
  _rate = NULL;
  _adjusted = NULL;
  _maxDbz = NULL;
  _vil = NULL;
  _maxVil = NULL;

  init();

}

/////////////
// Destructor

AccumData::~AccumData()

{

  _free();

}

//////////////////////////////
// initialize for computations

void AccumData::init()

{

  _dataFound = false;

  _nxy = 0;
  MEM_zero(_grid);
  
  _actualAccumPeriod = 0.0;
  _targetAccumPeriod = 0.0;

  _prevDataTime = 0;

  _free();

}

////////////////////////
// free up memory arrays

void AccumData::_free()

{

  if (_precip) {
    ufree(_precip);
    _precip = NULL;
  }
  if (_rate) {
    ufree(_rate);
    _rate = NULL;
  }
  if (_vil) {
    ufree(_vil);
    _vil = NULL;
  }
  if (_maxVil) {
    ufree(_maxVil);
    _maxVil = NULL;
  }
  if (_adjusted) {
    ufree(_adjusted);
    _adjusted = NULL;
  }
  if (_maxDbz) {
    ufree(_maxDbz);
    _maxDbz = NULL;
  }

}

///////////////////
// setTargetPeriod()
//
// Set the target period for accumulation.
//
// The actual accumulation period is determined from
// _dataStartTime and _dataEndTime. The precip depths
// are adjusted to the exact target period.

void AccumData::setTargetPeriod(double period)

{
  _targetAccumPeriod = period;
}

/////////////////
// processInput()
//
// Process input data from an MDV file.
//

int AccumData::processFile(const string &file_path)

{
  return processFile(file_path, 0, 0);
}

    
int AccumData::processFile(const string &file_path, time_t inputTime, int leadTime)

{

  PMU_auto_register("AccumData::processInput");

  // read in the file

  DsMdvx mdvx;

  if(_params.accum_method == Params::SINGLE_FILE_FORECAST)
  {
    mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, file_path,0,
		     inputTime, leadTime);
  }
  else
  {
    mdvx.setReadPath(file_path);    
  }

  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  if (_params.input_is_precip || _params.input_is_rate) {
    mdvx.addReadField(_params.precip_field_name);
  } else if (strlen(_params.dbz_field_name) > 0) {
    mdvx.addReadField(_params.dbz_field_name);
  } else {
    mdvx.addReadField(_params.dbz_field);
  }
  mdvx.setReadVlevelLimits(_params.composite_min_altitude,
 			   _params.composite_max_altitude);
  
  if (mdvx.readVolume()) {
    cerr << "ERROR - AccumData::processInput" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  // if first data, set things up. Else check that grid has
  // not changed

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  const MdvxField &inFld = *mdvx.getField(0);
  const Mdvx::field_header_t &fhdr = inFld.getFieldHeader();
  MdvxProj proj(mhdr, fhdr);

  if (!_dataFound) {
    
    _grid = proj.getCoord();
    _nxy = _grid.nx * _grid.ny;
    _precip = (fl32 *) ucalloc(_nxy, sizeof(fl32));
    _adjusted = (fl32 *) ucalloc(_nxy, sizeof(fl32));
    
    if (_params.generate_rate_grid) {
      _rate = (fl32 *) ucalloc(_nxy, sizeof(fl32));
    }
    if (_params.generate_max_dbz_grid) {
      _maxDbz = (fl32 *) ucalloc(_nxy, sizeof(fl32));
    }
    if (_params.generate_max_vil_grid) {
      _vil = (fl32 *) umalloc(_nxy * sizeof(fl32));
      _maxVil = (fl32 *) ucalloc(_nxy, sizeof(fl32));
    }

    if (_params.debug >= Params::DEBUG_NORM) {    
      cerr << "AccumData::processFile mhdr.time_begin = " << DateTime(mhdr.time_begin) << endl;
      cerr << "AccumData::processFile mhdr.time_end = " << DateTime(mhdr.time_end) << endl;
    }
  
    _dataStartTime = mhdr.time_begin;
    _dataEndTime = mhdr.time_end;
    _dataFound = true;

  } else {
    
    if (_params.check_input_geom) {

      const Mdvx::coord_t &coord = proj.getCoord();
      if (coord.proj_type != _grid.proj_type ||
	  fabs(coord.proj_origin_lat -_grid.proj_origin_lat) > 0.00001 ||
	  fabs(coord.proj_origin_lon - _grid.proj_origin_lon) > 0.00001 ||
	  coord.nx != _grid.nx ||
	  coord.ny != _grid.ny ||
	  fabs(coord.minx -_grid.minx) > 0.00001 ||
	  fabs(coord.miny - _grid.miny) > 0.00001 ||
	  fabs(coord.dx - _grid.dx) > 0.00001 ||
	  fabs(coord.dy - _grid.dy) > 0.00001) {
	  
	fprintf(stderr, "ERROR - %s:AccumData::processInput\n",
		_progName.c_str());
	fprintf(stderr, "Input file grid has changed.\n");
	fprintf(stderr, "Original grid:\n");
	fprintf(stderr, "==============\n");
	proj.printCoord(_grid, cerr);
	fprintf(stderr, "Grid for time %s\n",
		utimstr(mhdr.time_centroid));
	fprintf(stderr, "===============================\n");
	proj.printCoord(proj.getCoord(), cerr);
	return (-1);
	
      } // if (coord.proj_type != ...
      
    } // if (_params.check_input_geom)
    
  } // if (!_dataFound) 

  if (_params.debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "AccumData: processing file at time %s\n",
	    utimstr(mhdr.time_centroid));
  }

  // set the times
  
  _dataStartTime = MIN(_dataStartTime, mhdr.time_begin);
  _dataEndTime = MAX(_dataEndTime, mhdr.time_end);
  _dataCentroidTime = mhdr.time_centroid;
  _latestDataTime = mhdr.time_centroid;

  double mdvDuration = (double) mhdr.time_end - mhdr.time_begin;
  if (mdvDuration < 1.0 || mdvDuration > _params.max_duration_seconds) {
    mdvDuration = _params.volume_duration;
  }
  if (_params.compute_volume_duration_from_data_times) {
    if (_prevDataTime != 0) {
      _volDuration = _dataCentroidTime - _prevDataTime;
      if (_volDuration > _params.max_computed_volume_duration) {
        _volDuration = mdvDuration;
      }
    } else {
      _volDuration = mdvDuration;
    }
  } else if (_params.override_volume_duration) {
    _volDuration = _params.volume_duration;
  } else {
    _volDuration = mdvDuration;
  }

  _actualAccumPeriod += _volDuration;
  _prevDataTime = _dataCentroidTime;
  
  if (_params.input_is_precip) {
    
    _updateAccumFromPrecip(inFld);
    
  } else if (_params.input_is_rate) {
    
    _updateAccumFromRate(inFld);
    
  } else {
    
    // create composite field
    
    MdvxField compFld(inFld);
    compFld.convert2Composite(_params.composite_min_altitude,
			      _params.composite_max_altitude);

    // compute the relevant reflectivity field from the vertical
    // column

    fl32 *dbz = (fl32 *) umalloc(_nxy * sizeof(fl32));
    _computeDbzFromVert(inFld, dbz);
    _updateAccumFromDbz(inFld, dbz, mhdr.time_centroid);
    ufree(dbz);
    
    // update max dbz
    
    if (_params.generate_max_dbz_grid) {
      _updateMaxDbz(compFld);
    }
    
    // compute the vil stuff if required, using all vlevels
    // so read the file in again
    
    if (_params.generate_max_vil_grid) {
      
      mdvx.clearReadVertLimits();
      if (mdvx.readVolume()) {
	cerr << "ERROR - AccumData::processInput" << endl;
	cerr << mdvx.getErrStr() << endl;
	return -1;
      }
      _computeVil(*mdvx.getField(0));
      _updateMaxVil();
    }
    
  }

  return (0);

}

////////////////////////////////////////////////////////
// Compute the precip for this file, and add to the
// accumulated grid.
//

void AccumData::_updateAccumFromDbz(const MdvxField &inFld,
				    const fl32 *dbz,
				    time_t radar_time)
  
{

  // get the ZR params
  
  double zrCoeff, zrExpon;
  _getZrParams(radar_time, zrCoeff, zrExpon);
  
  // accumulate the data in the precip array
  
  double invExpon = 1.0 / zrExpon;
  fl32 missing = inFld.getFieldHeader().missing_data_value;
  fl32 minDbz = _params.low_dbz_threshold;
  fl32 *p = _precip;
  for (int i = 0; i < _nxy; i++, dbz++, p++) {
    if (*dbz != missing && *dbz >= minDbz) {
      double dbzForPrecip = *dbz;
      if (dbzForPrecip > _params.hail_dbz_threshold) {
	dbzForPrecip = _params.hail_dbz_threshold;
      }
      double z = pow(10.0, dbzForPrecip / 10.0);
      double rate = pow((z / zrCoeff), invExpon) / 3600.0; // mm/sec
      (*p) += rate * _volDuration;
    }
    else if (_params.set_missing_to_zero) {
      (*p) += 0.0;
    }
  } // i
  
}

//////////////////////////////////////////////////////
// Using supplied rate, add to the accumulated grid.
//

void AccumData::_updateAccumFromRate(const MdvxField &inFld)
  
{
  
  fl32 *rate = (fl32 *) inFld.getVol();
  fl32 missing = inFld.getFieldHeader().missing_data_value;
  fl32 *p = _precip;
  for (int i = 0; i < _nxy; i++, rate++, p++) {
    if (*rate != missing) {
      (*p) += *rate * _volDuration / 3600.0; // mm from mm/hr
    }
  } // i
  
}

/////////////////////////////////////////////////////////
// Add the precip from this file to the accumulated grid.
//

void AccumData::_updateAccumFromPrecip(const MdvxField &inFld)
  
{
  
  // accumulate the data in the precip array
  
  fl32 *pcpin = (fl32 *) inFld.getVol();
  fl32 missing = inFld.getFieldHeader().missing_data_value;
  fl32 *p = _precip;
  for (int i = 0; i < _nxy; i++, pcpin++, p++) {
    if (*pcpin != missing) {
      (*p) += *pcpin;
    }
    else if (_params.set_missing_to_zero) {
      (*p) += 0.0;
    }
  } // i
  
}

////////////////
// _computeVil()
//
// Compute the vil grid.
//

void AccumData::_computeVil(const MdvxField &dbzFld)
  
{

  memset(_vil, 0, _nxy * sizeof(fl32));
  TaArray<double> dht_;
  double *dht = dht_.alloc(_grid.nz);
  const Mdvx::vlevel_header_t &vhdr = dbzFld.getVlevelHeader();

  if (_grid.nz < 2) {
    dht[0] = _grid.dz;
  } else {
    for (int iz = 0; iz < _grid.nz; iz++) {
      if(_grid.dz_constant) {
	dht[iz] = _grid.dz;
      } else {
	if (iz == 0) {
	  dht[iz] = vhdr.level[1] - vhdr.level[0];
	} else if (iz == _grid.nz - 1) {
	  dht[iz] = vhdr.level[_grid.nz - 1] - vhdr.level[_grid.nz - 2];
	} else {
	  dht[iz] = (vhdr.level[iz+1] - vhdr.level[iz-1]) / 2.0;
	}
      }
    } // iz
  } // if (_grid.nz < 2)
  
  fl32 missing = dbzFld.getFieldHeader().missing_data_value;

  for (int i = 0; i < _nxy; i++) {
    fl32 *dbz =(fl32 *) dbzFld.getVol() + i;
    vil_init();
    for (int iz = 0; iz < _grid.nz; iz++, dbz += _nxy) {
      if (*dbz != missing) {
	vil_add(*dbz, dht[iz]);
      }
    }
    _vil[i] = vil_compute();
  }

}

//////////////////////////////////////////////////////////
// _computeDbzFromVert()
//
// Compute dbz from the vertical levels
//

void AccumData::_computeDbzFromVert(const MdvxField &inFld,
                                    fl32 *dbz)
  
{

  const Mdvx::field_header_t &fhdr = inFld.getFieldHeader();
  fl32 missing = fhdr.missing_data_value;
  
  if (_params.vert_method == Params::VERT_MAX) {
    
    for (int ii = 0; ii < _nxy; ii++, dbz++) {
      fl32 maxDbz = missing;
      const fl32 *data = ((fl32 *) inFld.getVol()) + ii;
      for (int iz = 0; iz < fhdr.nz; iz++, data += _nxy) {
	if (*data != missing && (maxDbz == missing || *data > maxDbz)) {
	  maxDbz = *data;
	}
      }
      *dbz = maxDbz;
    }

  } else if (_params.vert_method == Params::VERT_MEAN_Z) {

    for (int ii = 0; ii < _nxy; ii++, dbz++) {
      fl32 sum = 0.0;
      double count = 0.0;
      const fl32 *data = ((fl32 *) inFld.getVol()) + ii;
      for (int iz = 0; iz < fhdr.nz; iz++, data += _nxy) {
	if (*data != missing) {
	  sum += pow(10.0, *data / 10.0);
	  count++;
	}
      }
      if (count > 0) {
	*dbz = 10.0 * log10(sum / count);
      } else {
	*dbz = missing;
      }
    }

  } else if (_params.vert_method == Params::VERT_MEAN_DBZ) {

    for (int ii = 0; ii < _nxy; ii++, dbz++) {
      fl32 sum = 0.0;
      double count = 0.0;
      const fl32 *data = ((fl32 *) inFld.getVol()) + ii;
      for (int iz = 0; iz < fhdr.nz; iz++, data += _nxy) {
	if (*data != missing) {
	  sum += *data;
	  count++;
	}
      }
      if (count > 0) {
	*dbz = sum / count;
      } else {
	*dbz = missing;
      }
    }

  }

}

///////////////////
// _updateMaxDbz()
//
// Update the Max dBZ grid.
//

void AccumData::_updateMaxDbz(const MdvxField &compFld)
  
{

  fl32 *dbz = (fl32 *) compFld.getVol();
  fl32 missing = compFld.getFieldHeader().missing_data_value;
  fl32 *mdbz = _maxDbz;
  
  for (int i = 0; i < _nxy; i++, dbz++, mdbz++) {
    if (*dbz != missing && *dbz > *mdbz) {
      *mdbz = *dbz;
    }
  }

}

///////////////////
// _updateMaxVil()
//
// Update the Max Vil grid.
//

void AccumData::_updateMaxVil()
  
{

  fl32 *vil = _vil;
  fl32 *mvil = _maxVil;
  
  for (int i = 0; i < _nxy; i++, vil++, mvil++) {
    if (*vil > *mvil) {
      *mvil = *vil;
    }
  }

}

/////////////////
// _setZrParams()
//

void AccumData::_getZrParams(time_t radar_time,
			     double &zr_coeff,
			     double &zr_expon)

{
  
  PMU_auto_register("Accumdata::_getZrParams");

  zr_coeff = _params.zr.coeff;
  zr_expon = _params.zr.expon;

  if (!_params.get_zr_from_database) {
    return;
  }

  // get ZR params from latest data the radar time
  
  DsSpdb spdb;
  
  if (spdb.getFirstBefore(_params.zr_spdb_url,
			  radar_time, 86400)) {
    
    if (_params.debug >= Params::DEBUG_WARNINGS) {
      cerr << "WARNING - Accumdata::_getZrParams" << endl;
      cerr << "  Cannot get ZR data from URL: " << _params.zr_spdb_url << endl;
      cerr << "  Time: " << utimstr(radar_time) << endl;
    }
    return;
  }

  if (spdb.getNChunks()) {
    return;
  }

  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  zr_params_t *zrparams = (zr_params_t *) chunks[0].data;
  zr_coeff = zrparams->coeff;
  zr_expon = zrparams->expon;
  
  return;

}

////////////////////
// computeAdjusted()
//
// Computes the precip data adjusted for the ratio of the
// actual accum period to the target accum period.
//

void AccumData::_computeAdjusted()
  
{

  if (!_dataFound) {
    return;
  }
  
  if (_params.adjust_for_expected_total_duration &&
      !_params.compute_volume_duration_from_data_times &&
      !_params.override_volume_duration) {

    double ratio = _targetAccumPeriod / _actualAccumPeriod;
    fl32 *p = _precip;
    fl32 *a = _adjusted;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "AccumData::computeAdjusted\n");
      fprintf(stderr, "  _targetAccumPeriod: %g\n",_targetAccumPeriod); 
      fprintf(stderr, "  _actualAccumPeriod: %g\n",_actualAccumPeriod); 
      fprintf(stderr, "  ratio: %g\n", ratio);
    }
    
    if (ratio > 3.0 && !_params.use_elapsed_time) {
      ratio = 1.0;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr, "  ----> ratio excessive, resetting to 1.0\n");
      }
    }
    
    for (int i = 0; i < _nxy; i++, p++, a++) {
      double adjusted = *p * ratio;
      *a = adjusted;
    }
    
  } else { // if (_params.adjust_for_expected_total_duration ...

    memcpy(_adjusted, _precip, _nxy * sizeof(fl32));

  }
    
}

//////////////////////////////////////////
// Computes the precip rate from the depth

void AccumData::_computeRate()
  
{

  if (!_dataFound) {
    return;
  }

  if (!_params.generate_rate_grid) {
    return;
  }
  
  double hrs = _actualAccumPeriod / 3600.0;

  fl32 *p = _precip;
  fl32 *r = _rate;

  for (int i = 0; i < _nxy; i++, p++, r++) {
    *r = *p / hrs;
  }

}

//////////////////////////////////////////////////////////
// normalize by the number of seasons

void AccumData::_normalizeByNSeasons(double nSeasons)
  
{

  if (!_dataFound) {
    return;
  }

  fl32 *aa = _adjusted;
  for (int ii = 0; ii < _nxy; ii++, aa++) {
    *aa = *aa / nSeasons;
  }
      
}

//////////////////////////////////////////////////////////
// set max precip depth as appropriate

void AccumData::_setMaxDepth(double maxDepth)
  
{

  if (!_dataFound) {
    return;
  }

  fl32 *aa = _adjusted;
  for (int ii = 0; ii < _nxy; ii++, aa++) {
    if (*aa > maxDepth) {
      *aa = maxDepth;
    }
  }
      
}

///////////////////////////
// compute and write
//
// returns 0 on success, -1 on failure

int AccumData::computeAndWrite(time_t start_time,
                               time_t end_time,
                               time_t centroid_time,
                               int forecast_lead_time)
  
{

  PMU_auto_register("AccumData::computeAndWrite");

  if (_params.debug) {
    cerr << "Writing output file" << endl;
    cerr << "  targetAccumPeriod: " << _targetAccumPeriod << endl;
    cerr << "  actualAccumPeriod: " << _actualAccumPeriod << endl;
    cerr << "  volDuration: " << _volDuration << endl;
  }

  // compute

  _computeAdjusted();
  _computeRate();

  if (_params.normalize_by_number_of_seasons) {
    _normalizeByNSeasons(_params.climo_n_seasons);
  }

  if (_params.set_max_precip_depth) {
    _setMaxDepth(_params.max_precip_depth);
  }

  // write out
  
  OutputFile out(_progName, _params);
  
  if (out.write(start_time,
                end_time,
                centroid_time,
                forecast_lead_time,
                _actualAccumPeriod,
                _grid, 
                _adjusted,
                _rate,
                _maxDbz,
                _maxVil)) {
    cerr << "ERROR - AccumData::computeAndWrite" << endl;
    return -1;
  }

  return 0;

}
  
