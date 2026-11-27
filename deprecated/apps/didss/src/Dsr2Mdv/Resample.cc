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
// Resample.cc
//
// Resample class
//
// Samples from an MDV cartesian file to generate test radar data
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1998
//
///////////////////////////////////////////////////////////////

#include "Resample.hh"
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/udatetime.h>
#include <toolsa/toolsa_macros.h>
using namespace std;

//////////////
// Constructor

Resample::Resample(char *prog_name,
                   Dsr2Mdv_tdrp_struct *params,
		   Lookup *lookup)
  
{

  _progName = STRdup(prog_name);
  _params = params;
  _lookup = lookup;

  _volNum = 0;
  _nBeams = 0;
  _printCount = 0;

  _nX = 0;
  _nY = 0;
  _nZ = 0;

  _dbzFieldPos = -1;
  _timeFieldPos = -1;

  _nFieldsIn = 0;
  _nFieldsOut = 0;

  _nElev = 0;
  _nAz = 0;
  _nGates = 0;
  _noiseGateStart = 0;
  _nBytesBeam = 0;

  _nPlanes = 0;
  _nPointsPlane = 0;
  _nPointsField = 0;
  _nPointsVol = 0;
  _nBeamsArray = 0;
  _nBeamsTarget = 0;
  _maxNpointsBeam = 0;

  _volStartTime = 0;
  _volEndTime = 0;
  _latestDataTime = 0;
  _volReferenceTime = 0;
  _volDuration = 300;

  _azNum = 0;
  _elevNum = 0;
  _prevAzNum = -999;
  _prevElevNum = -999;
  _beamReferenceTime = 0;

  _fieldPos = NULL;
  _volGrid = NULL;
  _accumGrid = NULL;
  _thisBeamData = NULL;
  _prevBeamData = NULL;
  _interpBeamData = NULL;
  _beamCheck = NULL;
  _rData = NULL;
  _dbzThresholdLevel = NULL;

  if (_params->specify_output_fields) {
    _nFieldsOut = _params->output_field_names.len;
  } else {
    _nFieldsOut = 1;
  }
  if (_params->create_time_field) {

     bool found = false;
     for (int i = 0; i < _params->output_field_names.len; i++) {
        if (!strcmp( _params->output_field_names.val[i], 
                     _params->input_time_field_name)) {
           found = true;
           break;
        }
     }
     if (!found) {
        _nFieldsOut += 1;
     }
  }
  
  _fieldPos = (int *) umalloc (_nFieldsOut * sizeof(int)); 

  // local copies of parameters to improve performance

  _checkSn = _params->check_sn;
  _minValidRun = _params->sn_min_valid_run;
  _removeClutter = _params->remove_clutter;
  _azimuthOffset = _params->azimuth_offset;
  _useRepeatedElevations = _params->use_repeated_elevations;

  _output = new MdvOutput(_progName, _params, _lookup);
  if (!_output->OK) {
    OK = FALSE;
  }

  OK = TRUE;

}

/////////////
// destructor

Resample::~Resample()

{

  STRfree(_progName);

  if (_volGrid) {
    ufree2((void **) _volGrid);
    _volGrid = NULL;
  }

  if (_accumGrid) {
    ufree2((void **) _accumGrid);
    _accumGrid = NULL;
  }

  if (_fieldPos) {
    ufree(_fieldPos);
  }

  if (_thisBeamData) {
    ufree(_thisBeamData);
  }

  if (_prevBeamData) {
    ufree(_prevBeamData);
  }

  if (_interpBeamData) {
    ufree(_interpBeamData);
  }

  if (_beamCheck) {
    ufree2((void **) _beamCheck);
  }

  if (_rData) {
    ufree2((void **) _rData);
  }

  if (_dbzThresholdLevel) {
    ufree(_dbzThresholdLevel);
  }

  delete (_output);

}

///////////////
// prepareVol()
//
// Prepare for new volume

#define NOISE_GATES 20

int Resample::prepareVol(DsRadarMsg &radarMsg)

{

  _nFieldsIn = radarMsg.numFields();
  _volStartTime = radarMsg.getRadarBeam().dataTime;
  _volReferenceTime = radarMsg.getRadarBeam().referenceTime;

  P2mdv_lookup_params_t *lookup_params = _lookup->handle.lookup_params;

  // determine the dbz field number 

  _dbzFieldPos = -1;
  for (int i = 0; i < _nFieldsIn; i++) {
    const DsFieldParams *fparams = radarMsg.getFieldParams(i);
    if (!strcmp(fparams->name.c_str(), "DBZ")) {
      _dbzFieldPos = i;
      break;
    }
  }

  // determine the time field number if necessary (and scale)

  _timeFieldPos = -1;
  _timeScale = 1.0;
  if (_params->create_time_field) {
     for (int i = 0; i < _nFieldsIn; i++) {
        const DsFieldParams *fparams = radarMsg.getFieldParams(i);
        if (!strcmp(fparams->name.c_str(), _params->input_time_field_name)) {
           _timeFieldPos = i;
           _timeScale = fparams->scale;
           break;
        }
     }
  }
  
//   if (_dbzFieldPos < 0) {
//     fprintf(stderr, "ERROR - %s:Resample::prepareVol\n", _progName);
//     fprintf(stderr, "No DBZ data in the input stream.\n");
//     fprintf(stderr, "DBZ field required.\n");
//     return (-1);
//   }

  // check the lookup geometry against the radar parameters

  if (_lookup->checkGeom(radarMsg, _dbzFieldPos)) {
    return (-1);
  }

  // set output field positions

  int n_fields_out;
  
  if (_params->specify_output_fields) {

    bool outputTimeFound = false;
    n_fields_out = _params->output_field_names.len;

    for (int i = 0; i < n_fields_out; i++) {

      int fieldPos = -1;
      char *fieldName = _params->output_field_names.val[i];

      if (_params->create_time_field &&
          !strcmp(fieldName, _params->input_time_field_name)) {
         outputTimeFound = true;
      }

      for (int j = 0; j < _nFieldsIn; j++) {
	const DsFieldParams *fparams = radarMsg.getFieldParams(j);
	if (!strcmp(fparams->name.c_str(), fieldName)) {
	  fieldPos = j;
	  break;
	}
      } // j

      if (fieldPos < 0) {
	fprintf(stderr, "ERROR - %s:Resample::prepareVol\n", _progName);
	fprintf(stderr, "No %s data in the input stream.\n", fieldName);
	fprintf(stderr, "Check output_field_names[] parameter\n");
	return (-1);
      }
      
      _fieldPos[i] = fieldPos;

    } // i

    // The time field was not included in the list of output
    // fields, but it was requested elsewhere in the parameter
    // file

    if (_params->create_time_field && !outputTimeFound ) {

       // Set the field position to an impossible value so
       // that we can tell if it has been set or not.

       int fieldPos = -1;
       
       // Look through the input field names to see if we
       // have an input time field

       for (int j = 0; j < _nFieldsIn; j++) {

          const DsFieldParams *fparams = radarMsg.getFieldParams(j);
          if (!strcmp(fparams->name.c_str(), _params->input_time_field_name)) {
             fieldPos = j;
             break;
          }
       }

       // We never found the time field in the list of input
       // fields

       if (fieldPos < 0) {
          fprintf(stderr, "ERROR - %s:Resample::prepareVol\n", _progName);
          fprintf(stderr, "No %s data in the input stream.\n",
                  _params->input_time_field_name);
          fprintf(stderr, "Check input_time_field_name parameter\n");
          return (-1);
       } 

       // We did find the time field.  Now set the field position at
       // n_fields_out since that is how many fields had until we
       // considered the time field.  Now that we have the time field
       // we should increment the number of output fields.

       _fieldPos[n_fields_out] = fieldPos;
       n_fields_out++;
       
    }

  } else {

    // use all fields in input stream

    n_fields_out = _nFieldsIn;
    _fieldPos = (int *) urealloc (_fieldPos, n_fields_out * sizeof(int)); 
    for (int i = 0; i < n_fields_out; i++) {
      _fieldPos[i] = i;
    }

    // if the user asked for the time field, make sure it is there

    if (_params->create_time_field) {
       
       bool inputTimeFound = false;
    
       for (int i = 0; i < _nFieldsIn; i++) {
          const DsFieldParams *fparams = radarMsg.getFieldParams(i);
          if (!strcmp(fparams->name.c_str(), _params->input_time_field_name)) {
             inputTimeFound = true;
             break;
          }
       }

       if (!inputTimeFound) {
          fprintf(stderr, "ERROR - %s:Resample::prepareVol\n", _progName);
          fprintf(stderr, "No %s data in the input stream.\n",
                  _params->input_time_field_name);
          fprintf(stderr, "Check input_time_field_name parameter\n");
          return (-1);
       }
    }

  }

  // check array sizes, realloc as necessary

  MDV_radar_grid_t *grid =
    (MDV_radar_grid_t *) &_lookup->handle.lookup_params->grid;

  if (grid->nx != _nX ||
      grid->ny != _nY ||
      grid->nz != _nZ ||
      n_fields_out != _nFieldsOut) {
    
    if (_volGrid) {
      ufree2((void **) _volGrid);
      _volGrid = NULL;
    }
    if (_accumGrid) {
      ufree2((void **) _accumGrid);
      _accumGrid = NULL;
    }

    _nX = grid->nx;
    _nY = grid->ny;
    _nZ = grid->nz;
    _nFieldsOut = n_fields_out;

    _nPlanes = _nZ;
    _nPointsPlane = _nX * _nY;
    _nPointsField = _nPointsPlane * _nPlanes;
    _nPointsVol = _nPointsField * _nFieldsOut;
    
    _volGrid = (ui08 **)
      umalloc2(_nFieldsOut, _nPointsField, sizeof(ui08));
    _accumGrid = (ui08 **)
      umalloc2(_nFieldsOut, _nPointsField, sizeof(ui08));

    // only clear accumGrid when it is allocated
    // Note: the double de-reference points to the contiguous
    // memory which may be cleared as a block

    memset(*_accumGrid, MISSING_DATA_VAL, _nPointsVol);

  }

  // clear vol grid at start of each vol
  // Note: the double de-reference points to the contiguous
  // memory which may be cleared as a block

  memset(*_volGrid, MISSING_DATA_VAL, _nPointsVol);

  // set other variables locally in the object - this helps 
  // keep the processing fast

  _nElev = lookup_params->nelevations;
  _nAz = lookup_params->nazimuths;
  _nGates = lookup_params->ngates;
  _noiseGateStart = _nGates - NOISE_GATES;

  // arrays for beam data and beam-to-beam interpolation

  _nBytesBeam = _nFieldsIn * _nGates;
  _thisBeamData = (ui08 *) urealloc(_thisBeamData, _nBytesBeam);
  _prevBeamData = (ui08 *) urealloc(_prevBeamData, _nBytesBeam);
  _interpBeamData = (ui08 *) urealloc(_interpBeamData, _nBytesBeam);

  // beam flag array

  radar_scan_table_t *scanTable = _lookup->handle.scan_table;
  _nBeamsArray = _nAz * _nElev;
  if (scanTable->use_azimuth_table) {
    _nBeamsTarget = scanTable->nbeams_vol;
  } else {
    _nBeamsTarget = _nBeamsArray;
  }

  // set up beam check array

  if (_beamCheck) {
    ufree2((void **) _beamCheck);
  }
  _beamCheck = (ui08 **) ucalloc2 (_nAz, _nElev, sizeof(ui08));
  _nBeams = 0;
  
  // determine the max number of cartesian points associated
  // with any beam
  
  _maxNpointsBeam = 0;
  P2mdv_lookup_index_t **lookup_index = _lookup->handle.lookup_index;
  for (int ielev = 0; ielev < _nElev; ielev++) {
    for (int iaz = 0; iaz < _nAz; iaz++) {
      if (lookup_index[ielev][iaz].npoints > _maxNpointsBeam) {
	_maxNpointsBeam = lookup_index[ielev][iaz].npoints;
      }
    } // iaz
  } // ielev
  
  // allocate the rdata array to handle this max number of points -
  // this array is used to temporarily store the radial data
  
  if (_rData) {
    ufree2((void **) _rData);
  }
  _rData = (ui08 **) ucalloc2 (_nFieldsOut, _maxNpointsBeam, sizeof(ui08));
  
  // Set dbz test level according to the signal/noise threshold.

  if (_checkSn && _dbzFieldPos >= 0) {

    if (_dbzThresholdLevel) {
      ufree(_dbzThresholdLevel);
    }
    _dbzThresholdLevel = (int *) umalloc(_nGates * sizeof(int));
  
    const DsFieldParams *dbzParams = radarMsg.getFieldParams(_dbzFieldPos);
    double dbz_scale = dbzParams->scale;
    double dbz_bias = dbzParams->bias;

    double range = lookup_params->start_range;
    for (int igate = 0; igate < _nGates;
	 igate++, range += lookup_params->gate_spacing) {
      
      double noise_dbz =
	(_params->sn_threshold + _params->noise_dbz_at_100km +
	 20.0 * (log10(range) - log10(100.0)));
      
      int dbz_level = (int) ((noise_dbz - dbz_bias) / dbz_scale + 0.5);
      
      if (dbz_level < 0)
	dbz_level = 0;
      if (dbz_level > 255)
	dbz_level = 255;
      
      _dbzThresholdLevel[igate] = dbz_level;

    } // igate
    
  } // if (_checkSn) 

  // initialize the MDV output module

  _output->setVolHdrs(radarMsg,
		      (MDV_radar_grid_t *)
		      &_lookup->handle.lookup_params->grid,
		      _nFieldsOut, _fieldPos,
		      MISSING_DATA_VAL,
		      _lookup->handle.scan_table);
  
  return (0);

}

//////////////////
// processBeam()
//
// Process a beam.

int Resample::processBeam(DsRadarMsg &radarMsg)

{ 

  const DsRadarBeam &beam = radarMsg.getRadarBeam();
  radar_scan_table_t *scanTable = _lookup->handle.scan_table;

  // get the beam reference time

  _beamReferenceTime = beam.referenceTime;

  // copy the beam data - truncate or pad out as necessary

  int dataLen = beam.dataLen();
  if (_nBytesBeam <= dataLen) {
    memcpy(_thisBeamData, beam.data(), _nBytesBeam);
  } else {
    memcpy(_thisBeamData, beam.data(), dataLen);
    memset(_thisBeamData + dataLen, MISSING_DATA_VAL,
	   _nBytesBeam - dataLen);
  }
  
  /*
   * determine elevation index
   */
  
  _elevNum = _lookup->elev_handle(beam.targetElev);
  
  /*
   * determine azimuth index
   */

  if (scanTable->use_azimuth_table) {
    
    _azNum = RadarScanTableAng2AzNum(scanTable->elevs + _elevNum,
				     beam.azimuth);

  } else {
    
    _azNum = (int) floor
      ((beam.azimuth + _azimuthOffset - scanTable->start_azimuth) /
       scanTable->delta_azimuth);
    
    if (_azNum >= scanTable->nazimuths)
      _azNum -= scanTable->nazimuths;
    else if (_azNum < 0)
      _azNum += scanTable->nazimuths;

  }

  if (_params->debug >= DEBUG_VERBOSE) {
    if (_printCount == 0) {
      fprintf(stderr,
	      "Processing el, elnum, az, aznum, time: "
	      "%5g, %3d, %5g, %3d, %s\n",
	      beam.elevation, _elevNum, beam.azimuth, _azNum,
	      utimstr(beam.dataTime));
    }
    _printCount = (_printCount + 1) % 180;
  }

  if (_prevElevNum == _elevNum) {

    // we're on the same _elevNum as last beam
    // check for single missing beams and interpolate as
    // needed
  
    int nbeams_missing = _azNum - _prevAzNum - 1;

    if (_params->debug >= DEBUG_VERBOSE && nbeams_missing > 0) {
      fprintf(stderr, "---->> %d beams missing, from aznum %d to %d\n",
	      nbeams_missing,  _prevAzNum, _azNum);
    }

    if (nbeams_missing == 1) {

      // interpolate missing beam
      
      if (_params->debug >= DEBUG_VERBOSE) {
	fprintf(stderr, "---->> Interpolating missing beam\n");
      }

      _interpolateBeam(_thisBeamData, _prevBeamData,
		       _interpBeamData, _nBytesBeam);
      
      // transform the interpolated beam

      _transformBeam(_azNum - 1, _elevNum, _interpBeamData);
      
    } // if (nbeams_missing == 1)
    
  } else {

    // Reset _printCount so get print at start of new elev. 

    _printCount = 0; 

  } // if (_prevElevNum == _elevNum)

  _transformBeam(_azNum, _elevNum, _thisBeamData);
  _volEndTime = beam.dataTime;
  _latestDataTime = beam.dataTime;

  // set values for use next time through

  _prevElevNum = _elevNum;
  _prevAzNum = _azNum;
  memcpy(_prevBeamData, _thisBeamData, _nBytesBeam);
  
  return (0);

}

/////////////////////
// _interpolateBeam()
//
// Interpolate a single missing beam

void Resample::_interpolateBeam(ui08 *field_data,
				ui08 *prev_field_data,
				ui08 *interp_field_data,
				int nbytes_interp)

{

  long i;

  for (i = 0; i < nbytes_interp; i++) {

    *interp_field_data = (*field_data + *prev_field_data) / 2;

    field_data++;
    prev_field_data++;
    interp_field_data++;

  }

}

///////////////////////////////////////////////
// writeCompleteVol()
//
// Write out completed volume in MDV format.
//

int Resample::writeCompleteVol()

{

  // check for missing beams

  if (_params->check_missing_beams) {
    int nmissing = _nBeamsTarget - _nBeams;
    if (nmissing > _params->max_missing_beams) {
      fprintf(stderr, "ERROR - %s:Resample::writeCompleteVol\n",
	      _progName);
      fprintf(stderr, "Too many missing beams - %d missing\n", nmissing);
      fprintf(stderr, "Max allowable %d\n", (int) _params->max_missing_beams);
      fprintf(stderr, "Cannot write vol for time %s\n",
	      utimstr(_volEndTime));
      return (-1);
    }
  }

  // save vol duration for using in intermediate writes

  _volDuration = abs(_volEndTime - _volStartTime);

  return (_output->writeCompleteVol(_volStartTime,
				    _volEndTime,
                                    _volReferenceTime,
				    _volGrid,
				    _nPointsPlane));

}

///////////////////////////////////////////////
// writeIntermediateVol()
//
// Write out intermediate volume in MDV format.
//

int Resample::writeIntermediateVol()

{

  return (_output->writeIntermediateVol(_latestDataTime,
					_volDuration,
					_accumGrid,
					_nPointsPlane));
  
}

