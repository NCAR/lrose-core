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
// transformBeam.cc
//
// Resample::transformBeam routine
//
// Performs the transformation from radar space to gridded space.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1998
//
///////////////////////////////////////////////////////////////

#include "Resample.hh"
#include <cstring>
using namespace std;

///////////////////
// _transformBeam()
//
// Transform a single beam

void Resample::_transformBeam(int az_num, int elev_num,
			      ui08 *field_data)

{

  // return early if indices are out of range
  
  if (elev_num < 0 || elev_num >= _nElev ||
      az_num < 0 || az_num >= _nAz) {
    return;
  }
    
  // check if beam has already been set
  
  if (_beamCheck[az_num][elev_num] == 1 &&
      !_useRepeatedElevations) {
    return;
  }

  // set polar mode if applicable - in polar mode the lookup
  // table has offsets at the start of each beam instead of
  // indices for each gate

  int polar_mode;
  if (_lookup->handle.lookup_params->geom == P2MDV_POLAR) {
    polar_mode = TRUE;
  } else {
    polar_mode = FALSE;
  }

  // set beam check flag and increment

  _beamCheck[az_num][elev_num] = 1;
  _nBeams++;
      
  P2mdv_lookup_index_t **lookup_index = _lookup->handle.lookup_index;

  int npoints = lookup_index[elev_num][az_num].npoints;

  if (_checkSn && npoints > 0) {
    
    // filter the data to ensure run lengths of non-noise at least
    // as long as _minValidRun
      
    int run_length = 0;
    ui08 *fd_ptr = field_data;
    ui08 *dbz_byte = fd_ptr + _dbzFieldPos;
	
    for (int igate = 0;
	 igate < (int) lookup_index[elev_num][az_num].last_gate_active;
	 igate++) {
	
      if (*dbz_byte > _dbzThresholdLevel[igate]) {
	    
	// increment counter for number of valid points in this run
	    
	run_length++;
	    
      } else {
	
	// if valid run is too short, zero out the short run
	    
	if (run_length > 0 && run_length < _minValidRun) {
	  
	  int nbytes_run = run_length * _nFieldsIn;
	  memset ((fd_ptr - nbytes_run), 0, nbytes_run);
	  
	}
	    
	run_length = 0;
	    
      } // if (*dbz_byte ... 
	  
      fd_ptr += _nFieldsIn;
      dbz_byte += _nFieldsIn;
      
    } // igate
	
  } // if (_checkSn && npoints > 0)
    
  // initialize rdata array with the missing data flag
    
  memset(*_rData, MISSING_DATA_VAL, _maxNpointsBeam * _nFieldsOut);
    
  // load up gates in rdata array for which the signal/noise value
  // exceeds the threshold

  if (polar_mode) {

    for (int gate_num = 0; gate_num < npoints; gate_num++) {

      ui08 *fdata = field_data + gate_num * _nFieldsIn;
      
      // check the signal-to-noise ratio exceeds threshold
      
      ui08 *dbz_byte = fdata + _dbzFieldPos;
      
      if (!_checkSn || (*dbz_byte > _dbzThresholdLevel[gate_num])) {
	
	for (int ifield = 0; ifield < _nFieldsOut; ifield++) {
	  ui08 rvalue = *(fdata + _fieldPos[ifield]);

          // If we have the time field, check the time
          // reference value.  If the time reference value
          // has changed, update the time value accordingly
          // so that a single volume has one time reference
          // value.  Note that _beamReferenceTime should
          // always be greater than or equal to _volReferenceTime,
          // since _volReferenceTime is updated at the start
          // of the volume

          if( _fieldPos[ifield] == _timeFieldPos ) {
             if( _beamReferenceTime != _volStartTime ) {
                time_t referenceDiff = _volStartTime - _beamReferenceTime;
                ui08 scaledDiff = (ui08) ((referenceDiff / _timeScale) + 0.5);
                rvalue -= scaledDiff;
             }
          }
	  _rData[ifield][gate_num] = rvalue;
	}
	
      }  // if (!_checkSn ...
      
    } // gate_num

  } else { // P2MDV_CART or P2MDV_PPI

    P2mdv_lookup_entry_t *lut_entry = lookup_index[elev_num][az_num].u.entry;
    for (int ipoint = 0; ipoint < npoints; ipoint++, lut_entry++) {

      int gate_num = lut_entry->gate;
      ui08 *fdata = field_data + gate_num * _nFieldsIn;
      
      // check the signal-to-noise ratio exceeds threshold
      
      ui08 *dbz_byte = fdata + _dbzFieldPos;
      
      if (!_checkSn || (*dbz_byte > _dbzThresholdLevel[gate_num])) {
	
	for (int ifield = 0; ifield < _nFieldsOut; ifield++) {
	  ui08 rvalue = *(fdata + _fieldPos[ifield]);

          // If we have the time field, check the time
          // reference value.  If the time reference value
          // has changed, update the time value accordingly
          // so that a single volume has one time reference
          // value.  Note that _beamReferenceTime should
          // always be greater than or equal to _volReferenceTime,
          // since _volReferenceTime is updated at the start
          // of the volume

          if( _fieldPos[ifield] == _timeFieldPos ) {
             if( _beamReferenceTime != _volStartTime ) {
                time_t referenceDiff = _volStartTime - _beamReferenceTime;
                ui08 scaledDiff = (ui08) ((referenceDiff / _timeScale) + 0.5);
                rvalue -= scaledDiff;
             }
          }
          
	  _rData[ifield][ipoint] = rvalue;
	}
	
      }  // if (!_checkSn ...
      
    } // ipoint

  } // if (polar_mode)
      
  // remove clutter from rdata array if required
    
  if (_removeClutter) {
      
    // get number of clutter points, and the address of the clutter
    // entries in the clutter list
      
    int nclut_points =
      _lookup->clut.table_index[elev_num][az_num].nclut_points;
    clut_table_entry_t *clutter_entry =
      _lookup->clut.table_index[elev_num][az_num].u.entry;
      
    // loop through the points associated with this beam
      
    for (int jpoint = 0; jpoint < nclut_points; jpoint++) {
	
      int ipoint = clutter_entry->ipoint;
	
      if (_rData[_dbzFieldPos][ipoint] <= clutter_entry->dbz) {
	  
	// if dbz val is below clutter value, set all fields to
	// the missing data value
	  
	for (int ifield = 0; ifield < _nFieldsOut; ifield++) {
	  _rData[ifield][ipoint] = MISSING_DATA_VAL;
	}

      }
	
      clutter_entry++;
	
    } // jpoint
      
  } // if (_removeClutter)
    
  // put data into the gridded array
    
  if (polar_mode) {

    int offset = lookup_index[elev_num][az_num].u.offset;
    for (int ipoint = 0; ipoint < npoints; ipoint++, offset++) {
      for (int ifield = 0; ifield < _nFieldsOut; ifield++) {
	_volGrid[ifield][offset] = _rData[ifield][ipoint];
	_accumGrid[ifield][offset] = _rData[ifield][ipoint];
      }
    } // ipoint
    
  } else {

    // CART or PPI mode

    P2mdv_lookup_entry_t *lut_entry = lookup_index[elev_num][az_num].u.entry;
    for (int ipoint = 0; ipoint < npoints; ipoint++, lut_entry++) {
      int offset = lut_entry->index;
      for (int ifield = 0; ifield < _nFieldsOut; ifield++) {
	_volGrid[ifield][offset] = _rData[ifield][ipoint];
	_accumGrid[ifield][offset] = _rData[ifield][ipoint];
      }
    } // ipoint

  }
    
}

 
