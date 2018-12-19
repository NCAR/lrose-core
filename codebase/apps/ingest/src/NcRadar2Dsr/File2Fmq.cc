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
// File2Fmq.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2001
//
///////////////////////////////////////////////////////////////
//
// File2Fmq takes data from a netCDF radar file and writes it
// to a radar FMQ
//
////////////////////////////////////////////////////////////////

#include "File2Fmq.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/uusleep.h>
#include <toolsa/mem.h>
#include <toolsa/os_config.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <math.h>
using namespace std;

// Constructor

File2Fmq::File2Fmq(const Params &params,
		   const Nc3File &ncf,
		   DsRadarQueue &r_queue) :
  _params(params),
  _ncf(ncf),
  _rQueue(r_queue)

{

  // initialize

  _baseTimeVar = NULL;
  _timeOffsetVar = NULL;
  _azimuthVar = NULL;
  _elevationVar = NULL;
  _timeOffsetVals = NULL;
  _elevationVals = NULL;
  _azimuthVals = NULL;

  // set the variable pointers

  _setVars();

  // find the output fields

  _findFields();

}

// destructor

File2Fmq::~File2Fmq()

{

  if (_timeOffsetVals) {
    delete _timeOffsetVals;
  }
  if (_elevationVals) {
    delete _elevationVals;
  }
  if (_azimuthVals) {
    delete _azimuthVals;
  }
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    delete _fields[ii];
  }
  
}

//////////////////////////////////
// Set the variables

void File2Fmq::_setVars()

{

  _timeDimId = _ncf.rec_dim()->id();
  _maxCellsDimId = _ncf.get_dim("maxCells")->id();
  
  _nTimes = _ncf.rec_dim()->size();
  _maxCells = _ncf.get_dim("maxCells")->size();
  _baseTime = _ncf.get_var("base_time")->as_long(0);

  _baseTimeVar = _ncf.get_var("base_time");
  _timeOffsetVar = _ncf.get_var("time_offset");
  _azimuthVar = _ncf.get_var("Azimuth");
  _elevationVar = _ncf.get_var("Elevation");

  _timeOffsetVals = _ncf.get_var("time_offset")->values();
  _timeOffsetData = (double *) _timeOffsetVals->base();
  
  _elevationVals = _ncf.get_var("Elevation")->values();
  _elevationData = (float *) _elevationVals->base();
  
  _azimuthVals = _ncf.get_var("Azimuth")->values();
  _azimuthData = (float *) _azimuthVals->base();

  Nc3Var *fixedAngleVar = _ncf.get_var("Fixed_Angle");
  if (fixedAngleVar) {
    _fixedAngle = fixedAngleVar->as_float(0);
  } else {
    // not in file, so compute it
    _fixedAngle = 0.05;
    for (int ii = 0; ii <= _nTimes; ii++) {
      if (_elevationData[ii] >= 0 && _elevationData[ii] <= 90) {
	_fixedAngle += _elevationData[ii] / _nTimes;
      }
    }
    _fixedAngle = rint(_fixedAngle * 20.0) / 20.0;
  }

}

//////////////////////////////////
// Find the fields

void File2Fmq::_findFields()

{

  // set flag array so we know if a field has been found
  
  bool fieldFound[_params.output_fields_n];
  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    fieldFound[ifield] = false;
  }
  
  // search for the field variables

  for (int ivar = 0; ivar < _ncf.num_vars(); ivar++) {

    // first get the dimensions
    
    Nc3Var *var = _ncf.get_var(ivar);
    int nDims = var->num_dims();
    const Nc3Dim *dims[nDims];
    long dimSizes[nDims];
    for (int ii = 0; ii < nDims; ii++) {
      dims[ii] = var->get_dim(ii);
      dimSizes[ii] = dims[ii]->size();
    }

    if (nDims == 2 &&
	var->get_dim(0)->id() == _timeDimId &&
	var->get_dim(1)->id() == _maxCellsDimId) {

      // this is a data field, find the params for this field

      for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
	
	int pos = _findNameInList(var->name(),
				  _params._output_fields[ifield].nc_name);
	
	if (pos >= 0 && fieldFound[ifield]) {
	  if (_params.debug >= Params::DEBUG_VERBOSE) {
	    cerr << "Ignoring input field: " << var->name() << endl;
	  }
	}

	if (pos >= 0 && !fieldFound[ifield]) {
	  
	  fieldFound[ifield] = true;
	  if (_params.debug >= Params::DEBUG_VERBOSE) {
	    cerr << "Processing input field: " << var->name() << endl;
	  }

	  // found the field - construct a field object
	  // Values will be deleted in the destructor

	  Field *fld = new Field(var->name(),
				 _params._output_fields[ifield].name,
				 _params._output_fields[ifield].units,
				 1.0, 0.0, 0.0,
				 _params._output_fields[ifield].scale,
				 _params._output_fields[ifield].bias,
				 var,
				 var->values());

	  // modify nc scale, bias and missing as appropriate
	  
	  Nc3Att *scaleAtt = var->get_att("scale_factor");
	  if (scaleAtt) {
	    fld->ncScale = scaleAtt->as_float(0);
	    delete scaleAtt;
	  } else {
	    cerr << "WARNING - File2Fmq::_findFields" << endl;
	    cerr << "  'scale_factor' attribute missing for field: "
		 << fld->ncName << endl;
	    cerr << "  Using " << fld->ncScale << " as default" << endl;
	  }
	  
	  Nc3Att *biasAtt = var->get_att("add_offset");
	  if (biasAtt) {
	    fld->ncBias = biasAtt->as_float(0);
	    delete biasAtt;
	  } else {
	    cerr << "WARNING - File2Fmq::_findFields" << endl;
	    cerr << "  'add_offse' attribute missing for field: "
		 << fld->ncName << endl;
	    cerr << "  Using " << fld->ncBias << " as default" << endl;
	  }
	    
	  Nc3Att *missingAtt = var->get_att("missing_value");
	  if (missingAtt) {
	    fld->ncMissing = missingAtt->as_float(0);
	    delete missingAtt;
	  } else {
	    cerr << "WARNING - File2Fmq::_findFields" << endl;
	    cerr << "  'missing_valu' attribute missing for field: "
		 << fld->ncName << endl;
	    cerr << "  Using " << fld->ncMissing << " as default" << endl;
	  }
	    
	  _fields.push_back(fld);
	  
	} // if (!strcmp ...

      } // ifield

    } // if (nDims == 2 ...
	  
  } // ivar
  
}

////////////////////////////////////////
// Write radar and field params to queue
//
// Returns 0 on success, -1 on failure

int File2Fmq::writeParams()

{

  // Set radar parameters
  
  DsRadarMsg msg;
  DsRadarParams &rp = msg.getRadarParams();

  // first get params from the parameter file
  
  rp.radarId = _params.radar_params.radar_id;
  rp.radarType = DS_RADAR_GROUND_TYPE;
  rp.numFields = (int) _fields.size();
  rp.numGates = _maxCells;
  rp.samplesPerBeam = _params.radar_params.samples_per_beam;
  rp.scanType = _params.scan_type_id;
  rp.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  rp.polarization = DS_POLARIZATION_HORIZ_TYPE;
  
  rp.radarConstant = _params.radar_params.radar_constant;
  rp.altitude = _params.radar_params.altitude;
  rp.latitude = _params.radar_params.latitude;
  rp.longitude = _params.radar_params.longitude;
  
  rp.gateSpacing = _params.radar_params.gate_spacing;
  rp.startRange = _params.radar_params.start_range;
  rp.horizBeamWidth = _params.radar_params.beam_width;
  rp.vertBeamWidth = _params.radar_params.beam_width;
  
  rp.pulseWidth = _params.radar_params.pulse_width;
  rp.pulseRepFreq = _params.radar_params.prf;
  rp.wavelength = _params.radar_params.wavelength;
  
  rp.xmitPeakPower = _params.radar_params.xmit_peak_pwr;
  rp.receiverMds = _params.radar_params.receiver_mds;
  rp.receiverGain = _params.radar_params.receiver_gain;
  rp.antennaGain = _params.radar_params.antenna_gain;
  rp.systemGain = _params.radar_params.system_gain;
  
  rp.unambigVelocity = _params.radar_params.unambig_velocity;
  rp.unambigRange = _params.radar_params.unambig_range;
  
  rp.radarName = _params.radar_params.radar_name;
  rp.scanTypeName = _params.scan_type_name;
  
  // override from the data file as available

  Nc3Att *nsampleAtt = _ncf.get_att( "Num_Samples" );
  if (nsampleAtt) {
    rp.samplesPerBeam = nsampleAtt->as_int(0);
    delete nsampleAtt;
  }

  Nc3Att *inameAtt = _ncf.get_att( "Instrument_Name" );
  if (inameAtt) {
    rp.radarName = inameAtt->as_string(0);
    delete inameAtt;
  }

  if (_ncf.get_var("Radar_Constant")) {
    rp.radarConstant = _ncf.get_var("Radar_Constant")->as_float(0);
  }

  if (_ncf.get_var("Cell_Spacing")) {
    // meters to km
    rp.gateSpacing = _ncf.get_var("Cell_Spacing")->as_float(0) / 1000;
  }
  
  if (_ncf.get_var("Range_to_First_Cell")) {
    // meters to km
    rp.startRange =
      _ncf.get_var("Range_to_First_Cell")->as_float(0) / 1000;
  }

  if (_ncf.get_var("pulse_width")) {
    // seconds to microseconds
    rp.pulseWidth = _ncf.get_var("pulse_width")->as_float(0) * 1.0e6;
  }

  if (_ncf.get_var("PRF")) {
    rp.pulseRepFreq = _ncf.get_var("PRF")->as_float(0);
  }

  if (_ncf.get_var("Wavelength")) {
    // meters to cm
    rp.wavelength = _ncf.get_var("Wavelength")->as_float(0) * 100;
  }

  if (_ncf.get_var("bm_width")) {
    rp.horizBeamWidth = _ncf.get_var("bm_width")->as_float(0);
    rp.vertBeamWidth = _ncf.get_var("bm_width")->as_float(0);
  }

  if (_ncf.get_var("peak_pwr")) {
    rp.xmitPeakPower = _ncf.get_var("peak_pwr")->as_float(0);
  }

  if (_ncf.get_var("rcvr_gain")) {
    rp.receiverGain = _ncf.get_var("rcvr_gain")->as_float(0);
  }

  if (_ncf.get_var("ant_gain")) {
    rp.antennaGain = _ncf.get_var("ant_gain")->as_float(0);
  }

  if (_ncf.get_var("sys_gain")) {
    rp.systemGain = _ncf.get_var("sys_gain")->as_float(0);
  }

  if (_ncf.get_var("Nyquist_Velocity")) {
    rp.unambigVelocity = _ncf.get_var("Nyquist_Velocity")->as_float(0);
  }

  if (_ncf.get_var("Unambiguous_Range")) {
    // meters to km
    rp.unambigRange =
      _ncf.get_var("Unambiguous_Range")->as_float(0) / 1000;
  }

  if (_ncf.get_var("Latitude")) {
    rp.latitude = _ncf.get_var("Latitude")->as_float(0);
  }
  if (_ncf.get_var("Longitude")) {
    rp.longitude = _ncf.get_var("Longitude")->as_float(0);
  }
  if (_ncf.get_var("Altitude")) {
    rp.altitude = _ncf.get_var("Altitude")->as_float(0);
  }

  if (_params.override_radar_location) {
    rp.altitude = _params.radar_params.altitude;
    rp.latitude = _params.radar_params.latitude;
    rp.longitude = _params.radar_params.longitude;
  }

  if (_params.override_gate_spacing) {
    rp.gateSpacing = _params.radar_params.gate_spacing;
  }

  // Add field parameters to the message
  
  vector<DsFieldParams*> &fp = msg.getFieldParams();

  for (size_t ii = 0; ii < _fields.size(); ii++) {

    DsFieldParams* thisParams =
      new DsFieldParams(_fields[ii]->outName.c_str(),
			_fields[ii]->outUnits.c_str(),
			_fields[ii]->outScale, _fields[ii]->outBias);

    fp.push_back(thisParams);

  }
  
  // write the message
  
  if (_rQueue.putDsMsg(msg,
		       DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - File2Fmq::writeParams" << endl;
    cerr << "  Cannot put params to queue" << endl;
    return -1;
  }
  
  return 0;

}


////////////////////////////////////////
// Write beam data to queue
//
// Returns 0 on success, -1 on failure

int File2Fmq::writeBeams(int vol_num,
			 int tilt_num,
			 time_t start_time)

{

  if (_fields.size() < 1) {
    cerr << "WARNING - File2Fmq::writeBeams" << endl;
    cerr << "  Trying to send beams with no fields" << endl;
    return 0;
  }

  DsRadarMsg msg;
  DsRadarBeam &beam = msg.getRadarBeam();
  int nfields = _fields.size();
  int ndata = _maxCells * nfields;
  ui08 data[ndata];
  int iret = 0;
  
  // loop through the beams
  
  for (int itime = 0; itime < _nTimes; itime++) {

    // check elevation error

    if (_params.check_elev) {
      double elevDiff = fabs(_elevationData[itime] - _fixedAngle);
      if (elevDiff > _params.max_elev_error) {
	continue;
      }
    }

    // params

    beam.dataTime = start_time + (int) (_timeOffsetData[itime] + 0.5);
    beam.volumeNum = vol_num;
    beam.tiltNum = tilt_num;
    beam.azimuth = _azimuthData[itime];
    beam.elevation = _elevationData[itime];
    beam.targetElev = _fixedAngle;
    
    // data

    MEM_zero(data);
    for (int ifield = 0; ifield < nfields; ifield++) {

      Field *fld = _fields[ifield];
      ui08 *dptr = data + ifield;

      double ncScale = fld->ncScale;
      double ncBias = fld->ncBias;
      double ncMissing = fld->ncMissing;
      double outScale = fld->outScale;
      double outBias = fld->outBias;

      switch(fld->var->type()) {

      case nc3Byte: {
	unsigned char *vals = (unsigned char *)
	  fld->values->base() + itime * _maxCells;
	for (int igate = 0; igate < _maxCells;
	     igate++, dptr += nfields, vals++) {
	  if ((double) *vals != ncMissing) {
	    double dval = (double) *vals * ncScale + ncBias;
	    int byteval = (int) ((dval - outBias) / outScale + 0.5);
	    if (byteval < 1) {
	      byteval = 1;
	    }
	    if (byteval > 255) {
	      byteval = 255;
	    }
	    *dptr = byteval;
	  }
	} // igate
      }
      break;
      
      case nc3Short: {
	short *vals = (short *) fld->values->base() + itime * _maxCells;
	for (int igate = 0; igate < _maxCells;
	     igate++, dptr += nfields, vals++) {
	  if ((double) *vals != ncMissing) {
	    double dval = (double) *vals * ncScale + ncBias;
	    int byteval = (int) ((dval - outBias) / outScale + 0.5);
	    if (byteval < 1) {
	      byteval = 1;
	    }
	    if (byteval > 255) {
	      byteval = 255;
	    }
	    *dptr = byteval;
	  }
	} // igate
      }
      break;
      
      case nc3Int: {
	int *vals = (int *) fld->values->base() + itime * _maxCells;
	for (int igate = 0; igate < _maxCells;
	     igate++, dptr += nfields, vals++) {
	  if ((double) *vals != ncMissing) {
	    double dval = (double) *vals * ncScale + ncBias;
	    int byteval = (int) ((dval - outBias) / outScale + 0.5);
	    if (byteval < 1) {
	      byteval = 1;
	    }
	    if (byteval > 255) {
	      byteval = 255;
	    }
	    *dptr = byteval;
	  }
	} // igate
      }
      break;
      
      case nc3Float: {
	float *vals = (float *) fld->values->base() + itime * _maxCells;
	for (int igate = 0; igate < _maxCells;
	     igate++, dptr += nfields, vals++) {
	  if ((double) *vals != ncMissing) {
	    int byteval = (int) ((*vals - outBias) / outScale + 0.5);
	    if (byteval < 1) {
	      byteval = 1;
	    }
	    if (byteval > 255) {
	      byteval = 255;
	    }
	    *dptr = byteval;
	  }
	} // igate
      }
      break;
      
      case nc3Double: {
	double *vals = (double *) fld->values->base() + itime * _maxCells;
	for (int igate = 0; igate < _maxCells;
	     igate++, dptr += nfields, vals++) {
	  if ((double) *vals != ncMissing) {
	    int byteval = (int) ((*vals - outBias) / outScale + 0.5);
	    if (byteval < 1) {
	      byteval = 1;
	    }
	    if (byteval > 255) {
	      byteval = 255;
	    }
	    *dptr = byteval;
	  }
	} // igate
      }
      break;

      default: {}
	
      } // switch

    } // ifield

    beam.loadData(data, ndata);

    // write the message
    
    if (_rQueue.putDsMsg(msg, DsRadarMsg::RADAR_BEAM)) {
      iret = -1;
    }

    if (_params.mode != Params::REALTIME && _params.beam_wait_msecs > 0) {
      umsleep(_params.beam_wait_msecs);
    }

  } // itime

  if (iret) {
    cerr << "ERROR - File2Fmq::writeBeams" << endl;
    cerr << "  Cannot put radar beams to queue" << endl;
  }

  return iret;

}

/////////////////////////////////////////////
// Search for a field name in an nc_name list.
// The list is a comma-delimted string.
//
// Returns -1 on failure, position on success

int File2Fmq::_findNameInList(const string &name,
			      const string &list)
  
{

  string comma = ",";
  
  // if no commas, so check entire string
  
  if (list.find(comma, 0) == string::npos) {
    if (name == list) {
      return 0;
    } else {
      return -1;
    }
  }

  // search through subparts
  
  int nn = 0;
  size_t currPos = 0;
  size_t nextPos = 0;
  
  while (nextPos != string::npos) {

    nextPos = list.find(comma, currPos);
    string candidate;
    candidate.assign(list, currPos, nextPos - currPos);

    if (candidate == name) {
      return nn;
    }

    currPos = nextPos + 1;
    nn++;

  } // while

  return -1;

}

