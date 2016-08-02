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
// StatsData.cc
//
// Compute stats
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2003
//
//////////////////////////////////////////////////////////
//
// This module computes the stats
//
// It has a list of input files to be used.
// This list also shows which files have already been used.
//
///////////////////////////////////////////////////////////

#include "StatsData.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
using namespace std;

//////////////
// Constructor

StatsData::StatsData(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{

  _ntimes = 0;
  _npoints = 0;
  _prev = NULL;
  _nn = NULL;
  _sumx = NULL;
  _sumy = NULL;
  _sumxx = NULL;
  _sumyy = NULL;
  _sumxy = NULL;
  _dbzThresh = NULL;
  _mean = NULL;
  _frac = NULL;
  _sdev = NULL;
  _corr = NULL;
  MEM_zero(_grid);
  
}

/////////////
// Destructor

StatsData::~StatsData()

{

  if (_prev) {
    ufree(_prev);
  }
  if (_nn) {
    ufree(_nn);
  }
  if (_sumx) {
    ufree(_sumx);
  }
  if (_sumy) {
    ufree(_sumy);
  }
  if (_sumxx) {
    ufree(_sumxx);
  }
  if (_sumyy) {
    ufree(_sumyy);
  }
  if (_sumxy) {
    ufree(_sumxy);
  }
  if (_dbzThresh) {
    ufree(_dbzThresh);
  }
  if (_mean) {
    ufree(_mean);
  }
  if (_frac) {
    ufree(_frac);
  }
  if (_sdev) {
    ufree(_sdev);
  }
  if (_corr) {
    ufree(_corr);
  }

}

//////////////////////////////////////
// Process input data for a given time
//

int StatsData::processTime(time_t fileTime)
  
{
  
  PMU_auto_register("StatsData::processFile");
  
  // read in the file
  
  DsMdvx mdvx;
  mdvx.setReadTime(Mdvx::READ_CLOSEST, _params.input_url, 0, fileTime);
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.setReadVlevelLimits(_params.min_v_level, _params.max_v_level);
  mdvx.addReadField(_params.dbz_field_name);
  
  if (mdvx.readVolume()) {
    cerr << "ERROR - StatsData::processFile" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  // if first data, set things up. Else check that grid has
  // not changed
  
  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  const MdvxField &dbzFld = *mdvx.getField(0);
  const Mdvx::field_header_t &fhdr = dbzFld.getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = dbzFld.getVlevelHeader();
  MdvxProj proj(mhdr, fhdr);

  if (_ntimes == 0) {
    
    _mhdr = mhdr;
    _fhdr = fhdr;
    _vhdr = vhdr;
    
    _grid = proj.getCoord();
    _npoints = _grid.nx * _grid.ny * _grid.nz;

    _prev = (fl32 *) ucalloc(_npoints, sizeof(fl32));
    _dbzThresh = (fl32 *) ucalloc(_npoints, sizeof(fl32));
    _mean = (fl32 *) ucalloc(_npoints, sizeof(fl32));
    _frac = (fl32 *) ucalloc(_npoints, sizeof(fl32));
    _sdev = (fl32 *) ucalloc(_npoints, sizeof(fl32));
    _corr = (fl32 *) ucalloc(_npoints, sizeof(fl32));
    
    _nn = (double *) ucalloc(_npoints, sizeof(double));
    _sumx = (double *) ucalloc(_npoints, sizeof(double));
    _sumy = (double *) ucalloc(_npoints, sizeof(double));
    _sumxx = (double *) ucalloc(_npoints, sizeof(double));
    _sumyy = (double *) ucalloc(_npoints, sizeof(double));
    _sumxy = (double *) ucalloc(_npoints, sizeof(double));

    _dataStartTime = mhdr.time_begin;
    _dataEndTime = mhdr.time_end;

    _initDbzThresh();

  } else {
    
    const Mdvx::coord_t &coord = proj.getCoord();
    if (coord.proj_type != _grid.proj_type ||
	fabs(coord.proj_origin_lat -_grid.proj_origin_lat) > 0.00001 ||
	fabs(coord.proj_origin_lon - _grid.proj_origin_lon) > 0.00001 ||
	coord.nx != _grid.nx ||
	coord.ny != _grid.ny ||
	coord.nz != _grid.nz ||
	fabs(coord.minx -_grid.minx) > 0.00001 ||
	fabs(coord.miny - _grid.miny) > 0.00001 ||
	fabs(coord.dx - _grid.dx) > 0.00001 ||
	fabs(coord.dy - _grid.dy) > 0.00001) {
      
      cerr << "ClutterAuto::StatsData::processFile" << endl;
      cerr << "  Input file grid has changed." << endl;
      cerr << "  Original grid:" << endl;
      cerr << "  ==============" << endl;
      proj.printCoord(_grid, cerr);
      cerr << "  Grid for time: " << utimstr(mhdr.time_centroid) << endl;
      cerr << "  =================================" << endl;
      proj.printCoord(proj.getCoord(), cerr);
      return (-1);
      
    } // if (coord.proj_type != ...
      
  } // if (_ntimes == 0)

  _ntimes++;

  // set the times

  _dataStartTime = MIN(_dataStartTime, mhdr.time_begin);
  _dataEndTime = MAX(_dataEndTime, mhdr.time_end);
  _dataCentroidTime = mhdr.time_centroid;
  _latestDataTime = mhdr.time_centroid;
  
  if (_ntimes > 1) {

    // accum the stats
    
    fl32 *vol = (fl32 *) dbzFld.getVol();
    for (int ii = 0; ii < _npoints; ii++) {
      double xx = _prev[ii];
      double yy = vol[ii];
      fl32 minDbz = _dbzThresh[ii];
      if (xx >= minDbz && yy >= minDbz) {
	_nn[ii]++;
	_sumx[ii] += xx;
	_sumy[ii] += yy;
	_sumxx[ii] += xx * xx;
	_sumyy[ii] += yy * yy;
	_sumxy[ii] += xx * yy;
      }
    }
    
  }
    
  // store the vol in the _prev array, ready for use next time
  
  memcpy(_prev, dbzFld.getVol(), _npoints * sizeof(fl32));

  return 0;

}

////////////////////
// compute the stats
//
// returns 0 on success, -1 on failure

int StatsData::compute()

{

  if (_ntimes < 5) {
    cerr << "ERROR - StatsData::compute()" << endl;
    cerr << "  Too few times available, ntimes: " << _ntimes << endl;
    return -1;
  }

  for (int ii = 0; ii < _npoints; ii++) {

    double nn = _nn[ii];
    double frac = nn / (_ntimes - 1.0);
    
    if (nn > 3) {

      double sumx = _sumx[ii];
      double sumy = _sumy[ii];
      double sumxx = _sumxx[ii];
      double sumyy = _sumyy[ii];
      double sumxy = _sumxy[ii];
    
      double mean = sumy / nn;
    
      double var = sumyy / nn - (mean * mean);
      if (var < 0.0) {
	var = 0.0;
      }
      double sdev = sqrt(var);
      
      double num = nn * sumxy - sumx * sumy;
      double denom = (nn * sumxx - sumx * sumx) * (nn * sumyy - sumy * sumy);
      double corr = 0.0;
      if (denom > 0.0) {
	corr = num / sqrt(denom);
      }

      _mean[ii] = mean;
      _frac[ii] = frac;
      _sdev[ii] = sdev;
      _corr[ii] = corr;

    }

  }

  return 0;

}

///////////////////////////////////
// initialize DBZ thresholding grid
//

void StatsData::_initDbzThresh()

{
  
  if (_params.threshold_method == Params::DBZ_THRESHOLD) {

    for (int i = 0; i < _npoints; i++) {
      _dbzThresh[i] = _params.min_dbz;
    }
    
  } else {

    int nxy = _grid.nx * _grid.ny;
    
    for (int iy = 0; iy < _grid.ny; iy++) {
      
      double yy = _grid.miny + iy * _grid.dy;

      for (int ix = 0; ix < _grid.nx; ix++) {
	
	double xx = _grid.minx + ix * _grid.dx;
	double range = sqrt(xx * xx + yy * yy);
	double diffDb = 10.0 * log10(range / 100.0);
	double threshVal = _params.noise_dbz_at_100km + diffDb + _params.min_snr;

	fl32 *dt = _dbzThresh + iy * _grid.nx + ix;
	for (int iz = 0; iz < _grid.nz; iz++, dt += nxy) {
	  *dt = threshVal;
	} // iz

      } // ix
    } // iy

  }

}
