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
// Sampling.cc
//
// Sampling class
//
// Samples from an MDV cartesian file to generate test radar data
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1998
//
///////////////////////////////////////////////////////////////

#include "Sampling.hh"
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <rapmath/math_macros.h>
#include <rapmath/trig.h>
#include <Mdv/mdv/mdv_read.h>
using namespace std;

#ifndef RAND_MAX
#define RAND_MAX 2147483000
#endif

//////////////
// Constructor

Sampling::Sampling(const string &prog_name,
                   const Params &params,
		   int n_fields) :
  _progName(prog_name), _params(params)
  
{

  _sinPhi = NULL;
  _cosPhi = NULL;
  _gndRange = NULL;
  _beamHt = NULL;
  _beamBuf = NULL;
  _nFields = n_fields;
  _nGates = _params.radar_params.num_gates;
  _currElev = 0;
  _currAz = 0;
  _volNum = 0;
  _tiltNum = 0;
  _count = 0;
  _scanTableActive = false;
  OK = TRUE;

  // read in MDV file

  MDV_init_handle(&_mdv);
  if (MDV_read_all(&_mdv, _params.radar_sample_file_path,
		   MDV_INT8)) {
    fprintf(stderr, "ERROR - %s:Sampling::Sampling\n", _progName.c_str());
    fprintf(stderr, "Cannot open MDV file '%s'\n",
	    _params.radar_sample_file_path);
    OK = FALSE;
    return;
  }

  // set grid params

  _nX = _mdv.fld_hdrs[0].nx;
  _nY = _mdv.fld_hdrs[0].ny;
  _nZ = _mdv.fld_hdrs[0].nz;
  
  _minX = _mdv.fld_hdrs[0].grid_minx;
  _minY = _mdv.fld_hdrs[0].grid_miny;
  _minZ = _mdv.fld_hdrs[0].grid_minz;
  
  _dX = _mdv.fld_hdrs[0].grid_dx;
  _dY = _mdv.fld_hdrs[0].grid_dy;
  _dZ = _mdv.fld_hdrs[0].grid_dz;

  // Z and velocity field scale and bias

  _zField = _params.sample_dbz_field;
  _zScale = _mdv.fld_hdrs[_zField].scale;
  _zBias = _mdv.fld_hdrs[_zField].bias;

  if (_params.output_vel_field) {
    _vField = _params.sample_vel_field;
    _vScale = _mdv.fld_hdrs[_vField].scale;
    _vBias = _mdv.fld_hdrs[_vField].bias;
  }

  _htScale = 0.125;
  _htBias = -1.0;

  // set start time

  _startTime = time(NULL);

  // set up the noise array for SNR field

  _setupNoiseArray();

  // set up the scan table

  if (_setupScanTable()) {
    OK = FALSE;
    return;
  }
  _scanTableActive = true;

  // calculate the sampling geometry

  _calcSamplingGeom();

  // create the beam buffer

  _nData = _nGates * _nFields;
  _beamBuf = (ui08 *) umalloc(_nData);
  
}

/////////////
// destructor

Sampling::~Sampling()

{

  MDV_free_handle(&_mdv);

  if (_scanTableActive) {
    RadarFreeScanTableArrays(&_scanTable);
    _scanTableActive = false;
  }

  if (_sinPhi) {
    ufree(_sinPhi);
  }

  if (_cosPhi) {
    ufree(_cosPhi);
  }

  if (_gndRange) {
    ufree2((void **) _gndRange);
  }

  if (_beamHt) {
    ufree2((void **) _beamHt);
  }

  if (_beamBuf) {
    ufree(_beamBuf);
  }

}

//////////////////
// loadBeam()
//
// Samples for the beam.
// 
// Returns a pointer to the beam buffer
//

ui08 *Sampling::loadBeam(DsBeamHdr_t *beamHdr, int *ndata_p,
			 int *new_scan_type_p,
			 int *end_of_tilt_p,
			 int *end_of_volume_p)

{

  ui08 *bptr;
  ui08 **dbz_field, **vel_field;
  ui08 this_dbz;
  ui08 relv_byte = 0;
  int zByte;

  int igate;
  int ix, iy, iz;
  int index_xy;
  int output_vel_field;
  int output_geom_fields;
  int irelv = 0;
  int end_of_tilt = FALSE;
  int end_of_volume = FALSE;
  
  double grange, xsample, ysample, zsample;
  double move_x, move_y;
  double x0, y0;
  double dhr;
  double az;
  double relv = 0.0;
  double dist;

  time_t now;
  radar_scan_table_elev_t *elev;
  const Params::sampling_origin_t *origin;

  // set _tiltNum, _volNum,
  // end of tilt and end of volume flags
 
  if (_currAz >= _scanTable.elevs[_currElev].naz) {
    
    _currAz = 0;
    _tiltNum++;
    
    _currElev++;
    if (_currElev >= _scanTable.nelevations) {
      _currElev = 0;
      _tiltNum = 0;
      _volNum++;
    }
    
  }
  
  if (_currAz == _scanTable.elevs[_currElev].naz - 1) {
    end_of_tilt = TRUE;
    if (_currElev == _scanTable.nelevations - 1) {
      end_of_volume = TRUE;
    }
  }

  // compute the sampling origin

  now = time(NULL);
  dhr = (now - _startTime) / 3600.0;

  origin = &_params.sampling_origin;
  dist = origin->speed * dhr;
  dist = fmod(dist, origin->max_dist);
  double radDirn = origin->dirn * DEG_TO_RAD;
  double sinDirn, cosDirn;
  rap_sincos(radDirn, &sinDirn, &cosDirn);
  move_x = dist * sinDirn;
  move_y = dist * cosDirn;

  x0 = origin->start_x + move_x;
  y0 = origin->start_y + move_y;

  elev = _scanTable.elevs + _currElev;
  az = elev->azs[_currAz].angle;

  // relative velocity
  
  if (_params.override_vel) {
    relv = (origin->speed * cos((az - origin->dirn) * DEG_TO_RAD)) / (-3.6);
    irelv = (int) ((relv - _vBias) / _vScale + 0.5);
    irelv = MIN(irelv, 254);
    irelv = MAX(irelv, 0);
    relv_byte = irelv;
  }

  if (_params.debug) {
    if ((_count % 360) == 0) {
      fprintf(stderr, "speed, dirn: %g, %g\n", origin->speed, origin->dirn);
      fprintf(stderr, "vel bias, scale: %g, %g\n", _vBias,  _vScale);
      fprintf(stderr, "elev, az, originx, originy, relv, irelv: "
	      "%g, %g, %.1f, %.1f, %f, %d\n",
	      _scanTable.elev_angles[_currElev], az, x0, y0, relv, irelv);
    }
  }

  bptr = _beamBuf;
  dbz_field = (unsigned char **) _mdv.field_plane[_params.sample_dbz_field];
  vel_field = (unsigned char **) _mdv.field_plane[_params.sample_vel_field];
  output_vel_field = _params.output_vel_field;
  output_geom_fields = _params.output_geom_fields;

  for (igate = 0; igate < _nGates; igate++) {

    grange = _gndRange[_currElev][igate] * _params.range_scale;
    double rad = az * DEG_TO_RAD;
    double sinAz, cosAz;
    rap_sincos(rad, &sinAz, &cosAz);
    xsample = x0 + grange * sinAz;
    ysample = y0 + grange * cosAz;
    zsample = _beamHt[_currElev][igate];
    zByte = (int) ((zsample - _htBias) / _htScale + 0.5);
    if (zByte < 0) {
      zByte = 0;
    }
    if (zByte > 255) {
      zByte = 255;
    }

    ix = (int) ((xsample - _minX) / _dZ + 0.5);
    iy = (int) ((ysample - _minY) / _dY + 0.5);
    iz = (int) ((zsample - _minZ) / _dZ + 0.5);

    if (ix >= 0 && ix < _nX && iy >= 0 && iy < _nY && iz >= 0 && iz < _nZ) {

      index_xy = iy * _nX + ix;

      // dBZ fields

      this_dbz = dbz_field[iz][index_xy];
      int dnoise = (int) ((((double) rand() - 0.5) / (double) RAND_MAX) * 6);
      int noise = _noise[igate] + dnoise;
      if (this_dbz >= noise) {
	*bptr = this_dbz;
      } else {
	*bptr = noise;
      }
      bptr++;

      // velocity field

      if (output_vel_field) {
	if (this_dbz >= _noise[igate]) {
	  if (_params.override_vel) {
	    *bptr = relv_byte;
	  } else {
	    *bptr = vel_field[iz][index_xy];
	  }
	} else {
	  *bptr = (int) (((double) rand() / (double) RAND_MAX) * 250);
	}
	bptr++;
      }

    } else {

      *bptr = _noise[igate];
      bptr++;
      if (output_vel_field) {
	*bptr = (int) (((double) rand() / (double) RAND_MAX) * 250);
	bptr++;
      }

    }

    // geometry fields - elev, az and range bin #
    // These fields fold at a value of 10
    
    if (output_geom_fields) {
      *bptr = (ui08) zByte;
      bptr++;
      *bptr = (ui08) ((_currElev % 10) + 1);
      bptr++;
      *bptr = (ui08) ((_currAz % 10) + 1);
      bptr++;
      *bptr = (ui08) (((igate / 10) % 10) + 1);
      bptr++;
    }

   } /* igate */

  // load beam header

  beamHdr->time = time(NULL);
  beamHdr->azimuth = _scanTable.elevs[_currElev].azs[_currAz].angle;
  beamHdr->elevation = _scanTable.elev_angles[_currElev];
  beamHdr->target_elev = beamHdr->elevation;
  beamHdr->vol_num = _volNum;
  beamHdr->tilt_num = _tiltNum;
  if (_count == 0) {
    *new_scan_type_p = TRUE;
  } else {
    *new_scan_type_p = FALSE;
  }
  *end_of_tilt_p = end_of_tilt;
  *end_of_volume_p = end_of_volume;

  _count++;
  _currAz++;

  // return pointer to data buffer

  *ndata_p = _nData;
  return (_beamBuf);

}

////////////////////////////////////
// _setupNoiseArray()
//
// Set up noise array for SNR field
//

void Sampling::_setupNoiseArray()

{

  int igate;
  int dbz_field;
  int dbz_level;
  double range, noise_dbz;
  double dbz_scale, dbz_bias;

  _noise = (ui08 *) umalloc(_nGates * sizeof(ui08));
  
  dbz_field = _params.sample_dbz_field;
  dbz_bias = _mdv.fld_hdrs[dbz_field].bias;
  dbz_scale = _mdv.fld_hdrs[dbz_field].scale;

  for (igate = 0; igate < _nGates; igate++) {
    
    range = (_params.radar_params.start_range +
	     igate * _params.radar_params.gate_spacing);
    
    noise_dbz = (_params.noise_dbz_at_100km +
		 20.0 * (log10(range) - log10(100.0)));
    
    dbz_level = (int) ((noise_dbz - dbz_bias) / dbz_scale + 0.5);
    
    if (dbz_level < 0)
      dbz_level = 0;
    if (dbz_level > 255)
      dbz_level = 255;

    _noise[igate] = dbz_level;

  } /* igate */
  
}

////////////////////
// _setupScanTable()
//
// Set up the scan table, using either a pre-defined table (as for TASS)
// or fixed scan geometry.
//

int Sampling::_setupScanTable()

{

  int i, j;
  int beam_num;
  double az;
  radar_scan_table_elev_t *elev;

  RadarInitScanTable(&_scanTable);

  /*
   * basic params 
   */
  
  _scanTable.extend_below = FALSE;
  _scanTable.ngates = _params.radar_params.num_gates;
  _scanTable.gate_spacing = _params.radar_params.gate_spacing;
  _scanTable.start_range = _params.radar_params.start_range;
  _scanTable.beam_width = _params.radar_params.beam_width;
  
  if (!_params.use_scan_table) {

    _scanTable.use_azimuth_table = TRUE;
    
    _scanTable.nazimuths = _params.nazimuths;
    _scanTable.delta_azimuth = _params.delta_azimuth;
    _scanTable.start_azimuth = _params.start_azimuth;
    _scanTable.nelevations = _params.elev_angles_n;
    
    RadarAllocScanTableElevs(&_scanTable, _scanTable.nelevations);
      
    for (i = 0; i < _scanTable.nelevations; i++) {
      _scanTable.elev_angles[i] = _params._elev_angles[i];
    } /* i */

    _scanTable.max_azimuths = _scanTable.nazimuths;
    _scanTable.nbeams_vol = _scanTable.nelevations * _scanTable.nazimuths;
    
    /*
     * set the start and end beam nums for each elev
     */

    beam_num = 0;
    elev = _scanTable.elevs;
    for (i = 0; i < _scanTable.nelevations; i++, elev++) {
      elev->naz = _scanTable.nazimuths;
      elev->start_beam_num = i * _scanTable.nazimuths;
      elev->end_beam_num = elev->start_beam_num + _scanTable.nazimuths - 1;
      az = _params.start_azimuth;
      RadarAllocScanTableAzArrays(&_scanTable, i, elev->naz);
      for (j = 0; j < _scanTable.nazimuths; j++) {
	elev->azs[j].beam_num = beam_num;
	elev->azs[j].angle = az;
	az += _params.delta_azimuth;
	beam_num++;
      }
    }    
    
  } else {
    
    /*
     * read in scan table
     */
    
    if (RadarReadScanTable(&_scanTable,
			   _params.scan_table_path,
			   "setup_scan")) {
      return(-1);
    }

    /*
     * compute the azimuth limits
     */
    
    RadarComputeScanTableAzLimits(&_scanTable);

    /*
     * load up extended elevs array
     */

    RadarLoadScanTableExtElevs(&_scanTable);

  } /* if (!Glob->use_azimuth_table) */

  /*
   * compute missing data index - one past the last index in the 
   * radar data array
   */
  
  _scanTable.missing_data_index =
    _scanTable.nbeams_vol * _scanTable.ngates;

  /*
   * compute elev limits
   */
  
  RadarComputeScanTableElevLimits(&_scanTable);

  /*
   * compute extrended elev array
   */
  
  RadarComputeScanTableExtElev(&_scanTable);
    
  if (_params.debug && _params.use_scan_table) {
    RadarPrintScanTable(stderr, "  ", &_scanTable);
  }

  return (0);

}

//////////////////////
// _calcSamplingGeom()
//

void Sampling::_calcSamplingGeom()

{

  int ielev, igate;
  int nelevs = _scanTable.nelevations;
  int ngates = _scanTable.ngates;
  
  double radar_altitude;
  double twice_radius;
  double slant_range;

  twice_radius = 2.0 * PSEUDO_RADIUS;
  radar_altitude = _params.radar_params.altitude;
  
  /*
   * allocate memory
   */

  _sinPhi = (double *) umalloc((ui32) (nelevs * sizeof(double)));
  _cosPhi = (double *) umalloc((ui32) (nelevs * sizeof(double)));

  _gndRange = (double **) ucalloc2
    ((ui32) nelevs, (ui32) ngates, sizeof(double));

  _beamHt = (double **) ucalloc2
    ((ui32) nelevs, ngates, sizeof(double));

  for (ielev = 0; ielev < nelevs; ielev++) {

    double radEl = _scanTable.elev_angles[ielev] * DEG_TO_RAD;
    double sinEl, cosEl;
    rap_sincos(radEl, &sinEl, &cosEl);

    _sinPhi[ielev] = sinEl;
    _cosPhi[ielev] = cosEl;

    slant_range = _scanTable.start_range;

    for (igate = 0; igate < _scanTable.ngates; igate++) {

      _gndRange[ielev][igate] = slant_range * _cosPhi[ielev];

      _beamHt[ielev][igate] =
	(radar_altitude + slant_range * _sinPhi[ielev] +
	 slant_range * slant_range / twice_radius);

      slant_range += _scanTable.gate_spacing;

    } /* igate */
    
  } /* ielev */
  
}









