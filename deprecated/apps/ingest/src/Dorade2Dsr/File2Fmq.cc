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
// File2Fmq takes data from a Dorade radar file and writes it
// to a radar FMQ
//
////////////////////////////////////////////////////////////////

#include "Dorade2Dsr.hh"
#include "File2Fmq.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/os_config.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <math.h>
using namespace std;

const fl32 File2Fmq::MISSING_FL32 = -9999.0;
const ui16 File2Fmq::MISSING_UI16 = 0;
const ui08 File2Fmq::MISSING_UI08 = 0;

// Constructor

File2Fmq::File2Fmq(const Params &params,
		   dd_mapper &mapr,
		   dd_sweepfile_access &sac,
		   DsRadarQueue &r_queue) :
  _params(params),
  _mapr(mapr),
  _sac(sac),
  _rQueue(r_queue)
  
{

  _dataByteWidth = 1;
  if (_params.output_type == Params::OUTPUT_UI08) {
    _dataByteWidth = 1;
  } else if (_params.output_type == Params::OUTPUT_UI16) {
    _dataByteWidth = 2;
  } else if (_params.output_type == Params::OUTPUT_FL32) {
    _dataByteWidth = 4;
  }

  if (_params.output_type == Params::OUTPUT_FL32) {
    _missingVal = (int) floor(MISSING_FL32 + 0.5);
  } else {
    _missingVal = 0;
  }

  _scanId = -1;
  
  // find the output fields

  _findFields();

}

// destructor

File2Fmq::~File2Fmq()

{

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    delete _fields[ii];
  }
  
}

//////////////////////////////////
// Find the fields

void File2Fmq::_findFields()

{

  // set flag array so we know if a field has been found

  TaArray<bool> fieldFound_;
  bool *fieldFound = fieldFound_.alloc(_params.output_fields_n);
  for (int iout = 0; iout < _params.output_fields_n; iout++) {
    fieldFound[iout] = false;
  }
  
  // search for the field variables

  for (int ifldin = 0; ifldin < _mapr.num_fields(); ifldin++) {
    
    PARAMETER *fparam = _mapr.parms[ifldin];
    string fieldName = Dorade2Dsr::label(fparam->parameter_name, 8);

    for (int iout = 0; iout < _params.output_fields_n; iout++) {
      
      int pos = _findNameInList(fieldName,
				_params._output_fields[iout].dorade_name);
      
      if (pos >= 0 && fieldFound[iout]) {
	if (_params.debug) {
	  cerr << "Ignoring input field: " << fieldName << endl;
	}
      }
      
      if (pos >= 0 && !fieldFound[iout]) {
	
	fieldFound[iout] = true;
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "Processing input field: " << fieldName << endl;
	}
	
	// found the field - construct a field object
	// Values will be deleted in the destructor
	
	Field *fld = new Field(fieldName,
			       _params._output_fields[iout].name,
			       _params._output_fields[iout].units,
			       fparam->parameter_scale, 
			       fparam->parameter_bias,
			       _params._output_fields[iout].scale,
			       _params._output_fields[iout].bias,
			       fparam->bad_data,
			       ifldin, 1);

	if (_params.debug) {
	  cerr << endl;
	  cerr << "Adding DORADE field: " << fieldName << endl;
	  cerr << "  Output field name will be: "
	       << _params._output_fields[iout].name << endl << endl;
	}
	
	_fields.push_back(fld);
	
      } // if (!strcmp ...
      
    } // iout

  } // ifldin
  
}

////////////////////////////////////////
// Write radar and field params to queue
//
// Returns 0 on success, -1 on failure

int File2Fmq::writeParams(const char *input_path)

{

  if (_fields.size() < 1) {
    cerr << "WARNING - File2Fmq::writeBeams" << endl;
    cerr << "  Trying to send params with no fields" << endl;
    cerr << "  File: " << input_path << endl;
    return 0;
  }
  
  if (_params.debug) {
    cerr << endl;
    cerr << "Number of output fields: " << _fields.size() << endl;
  }

  // determine the scan type id

  _scanId = _params.scan_type_id;
  if (_params.get_scan_id_type_from_file_name) {
    for (int ii = 0; ii < _params.scan_type_lookup_n; ii++) {
      int id = _params._scan_type_lookup[ii].scan_type_id;
      string label = _params._scan_type_lookup[ii].scan_type_label;
      Path path(input_path);
      string fileName = path.getFile();
      if (fileName.find(label) != string::npos) {
	_scanId = id;
	if (_params.debug) {
	  cerr << "Found scan type label: " << label << endl;
	  cerr << "  Setting scan type id to: " << _scanId << endl;
	}
      }
    }
  }

  // Set radar parameters
  
  DsRadarMsg msg;
  DsRadarParams &rp = msg.getRadarParams();

  // first get params from the parameter file
  
  rp.radarId = 0;
  rp.radarType = _mapr.radar_type();
  rp.numFields = _fields.size();
  rp.numGates = _mapr.number_of_cells();
  if (_params.remove_test_pulse) {
    rp.numGates -= _params.ngates_test_pulse;
  }
  rp.samplesPerBeam = _mapr.return_num_samples(0);
  rp.scanType = _scanId;
  rp.scanMode = _mapr.scan_mode();
  rp.polarization = _mapr.parms[0]->polarization;
  
  rp.radarConstant = _mapr.radd->radar_const;
  rp.altitude = _mapr.altitude_km();
  rp.latitude = _mapr.latitude();
  rp.longitude = _mapr.longitude();
  
  rp.gateSpacing = _mapr.meters_between_cells() / 1000.0;
  rp.startRange = _mapr.meters_to_first_cell() / 1000.0 + rp.gateSpacing / 2.0;
  rp.horizBeamWidth = _mapr.radd->horz_beam_width;
  rp.vertBeamWidth = _mapr.radd->vert_beam_width;
  
  // rp.pulseWidth = _mapr.radd->pulsewidth;
  rp.pulseWidth =  _mapr.meters_between_cells() / 150.0;
  rp.pulseRepFreq = 1.0e3 / _mapr.radd->interpulse_per1; // s-1
  rp.wavelength = 30.0 / _mapr.radd->freq1; // cm
  
  rp.xmitPeakPower = _mapr.radd->peak_power;
  rp.receiverMds = _params.receiver_mds;
  rp.receiverGain = _mapr.radd->receiver_gain;
  rp.antennaGain = _mapr.radd->antenna_gain;
  rp.systemGain = _mapr.radd->system_gain;
  
  rp.unambigVelocity = _mapr.radd->eff_unamb_vel;
  rp.unambigRange = _mapr.radd->eff_unamb_range;
  
  rp.radarName = Dorade2Dsr::label(_mapr.radar_name(), 8);
  rp.scanTypeName = _params.scan_type_name;
  
  if (_params.override_radar_location) {
    rp.altitude = _params.radar_altitude;
    rp.latitude = _params.radar_latitude;
    rp.longitude = _params.radar_longitude;
  }

  if (_params.override_gate_geom) {
    rp.gateSpacing = _params.gate_spacing;
    rp.startRange = _params.start_range;
  }

  if (_params.debug) {
    cerr << endl;
    cerr << "Radar params for output queue" << endl << endl;
    rp.print(cerr);
  }

  // Add field parameters to the message
  
  vector<DsFieldParams*> &fp = msg.getFieldParams();

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    
    Field *fld = _fields[ii];
    int ipos = fld->doradeFieldPos;
    PARAMETER *fparam = _mapr.parms[ipos];
      
    if (fparam->binary_format == DORADE_SI08) {
      fld->doradeByteWidth = 1;
    } else if (fparam->binary_format == DORADE_SI16) {
      fld->doradeByteWidth = 2;
    } else {
      fld->doradeByteWidth = 4;
    }
    
    if (_params.use_dorade_scale_and_bias &&
	fld->doradeByteWidth == _dataByteWidth) {
      if (fparam->binary_format == DORADE_SI08) {
	fld->outScale = 1.0 / fld->doradeScale;
	fld->outBias = fld->doradeBias - 127.0 / fld->doradeScale;
      } else if (fparam->binary_format == DORADE_SI16) {
	fld->outScale = 1.0 / fld->doradeScale;
	fld->outBias = fld->doradeBias - 32767.0 / fld->doradeScale;
      }
    }
    if (_dataByteWidth == 4) {
      fld->outScale = 1.0;
      fld->outBias = 0.0;
    }
    
    DsFieldParams* thisParams =
      new DsFieldParams(fld->outName.c_str(),
			fld->outUnits.c_str(),
			fld->outScale, fld->outBias,
			_dataByteWidth, _missingVal);
    
    if (_params.debug) {
      cerr << endl;
      cerr << "Field params for output queue" << endl << endl;
      thisParams->print(cerr);
    }
    
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
  int ncells = _mapr.number_of_cells();
  if (_params.remove_test_pulse) {
    ncells -= _params.ngates_test_pulse;
  }		   

  // alloc output data

  int nfields = _fields.size();
  int nbytesOut = ncells * nfields * _dataByteWidth;
  void *outputData = umalloc(nbytesOut);

  int iret = 0;
  double fixedAngle = _mapr.fixed_angle();
  
  // loop through the beams
  
  do {

    double az = _mapr.azimuth();
    double elev = _mapr.elevation();
    if (elev >= 180.0) {
      elev -= 360.0;
    }
    DateTime beamTime(_mapr.year(), _mapr.month(), _mapr.day(),
		      _mapr.hour(), _mapr.minute(), _mapr.second());

    // Override the time, if the user has asked us to or if we
    // are in SIMULATE mode.
    
    if ((_params.mode == Params::SIMULATE) ||
	_params.override_radar_time){
      beamTime.set(time(NULL));
    }
    
    // check elevation error

    if (_scanId != DS_RADAR_RHI_MODE &&
	_params.check_elev) {
      double elevDiff = fabs(elev - fixedAngle);
      if (elevDiff > _params.max_elev_error) {
	continue;
      }
    }

    // params
    
    beam.dataTime = beamTime.utime();
    beam.volumeNum = vol_num;
    beam.tiltNum = tilt_num;
    beam.azimuth = az;
    beam.elevation = elev;
    beam.scanMode = _scanId;
    if (_scanId == DS_RADAR_RHI_MODE) {
      beam.targetAz = fixedAngle;
    } else {
      beam.targetElev = fixedAngle;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      
      fprintf(stderr,
	      " Vol Tilt El_tgt El_act     Az Ngat Time\n");
      
      fprintf(stderr,
	      "%4d %4d %6.2f %6.2f %6.2f %4d %s\n",
	      beam.volumeNum, beam.tiltNum,
	      beam.targetElev, beam.elevation,
	      beam.azimuth, ncells,
	      DateTime::str(beamTime.utime()).c_str());

    }

    // outputData
    
    for (int ifield = 0; ifield < nfields; ifield++) {

      Field *fld = _fields[ifield];
      int ipos = fld->doradeFieldPos;
      PARAMETER *fparam = _mapr.parms[ipos];
      
      double doradeScale = fld->doradeScale;
      double doradeBias = fld->doradeBias;
      double outScale = fld->outScale;
      double outBias = fld->outBias;
      si32 doradeBad = fld->doradeBad;

      if (_params.debug >= Params::DEBUG_DATA) {
	_printCells(fparam, ipos, ncells,
		    doradeBad, doradeScale, doradeBias);
      }

      // convert input data into fl32s

      TaArray<fl32> fdata_;
      fl32 *fdata = fdata_.alloc(ncells * nfields);
      _convertToFl32(ipos, ncells,
		     doradeBad, doradeScale, doradeBias, fdata);
      
      // load up output buffer
      
      _convertToOutput(fdata, ifield, nfields, ncells,
		       outScale, outBias, outputData);
      
    } // ifield

    beam.loadData(outputData, nbytesOut, _dataByteWidth);

    // write the message
    
    if (_rQueue.putDsMsg(msg, DsRadarMsg::RADAR_BEAM)) {
      iret = -1;
    }

    if (_params.mode != Params::REALTIME && _params.beam_wait_msecs > 0) {
      umsleep(_params.beam_wait_msecs);
    }

  } while (_sac.next_ray() != LAST_RAY);
  
  if (iret) {
    cerr << "ERROR - File2Fmq::writeBeams" << endl;
    cerr << "  Cannot put radar beams to queue" << endl;
  }

  // free up

  ufree(outputData);

  return iret;

}

///////////////////////////////
// convert input data to fl32

void File2Fmq::_convertToFl32(int ipos,
			      int ncells,
			      si32 doradeBad,
			      double doradeScale,
			      double doradeBias,
			      fl32 *fdata)
{

  PARAMETER *fparam = _mapr.parms[ipos];
  switch (fparam->binary_format) {
    
  case DORADE_FL32: {
    memcpy(fdata, _mapr.raw_data_ptr(ipos), ncells * sizeof(fl32));
  }
  break;
  
  case DORADE_SI32: {
    TaArray<si32> tmpBuf_;
    si32 *tmpBuf = tmpBuf_.alloc(ncells);
    memcpy(tmpBuf, _mapr.raw_data_ptr(ipos), ncells * sizeof(si32));
    si32 *in = tmpBuf;
    fl32 *fd = fdata;
    for (int igate = 0; igate < ncells; igate++, fd++, in++) {
      if (*in == doradeBad) {
	*fd = MISSING_FL32;
      } else {
	*fd = (fl32) ((double) *in / doradeScale + doradeBias);
      }
    } // igate
  }
  break;
  
  case DORADE_SI16: {
    TaArray<si16> tmpBuf_;
    si16 *tmpBuf = tmpBuf_.alloc(ncells);
    memcpy(tmpBuf, _mapr.raw_data_ptr(ipos), ncells * sizeof(si16));
    si16 *in = tmpBuf;
    fl32 *fd = fdata;
    for (int igate = 0; igate < ncells; igate++, fd++, in++) {
      if (*in == doradeBad) {
	*fd = MISSING_FL32;
      } else {
	*fd = (fl32) ((double) *in / doradeScale + doradeBias);
      }
    } // igate
  }
  break;
  
  case DORADE_SI08:
  default: {
    TaArray<si08> tmpBuf_;
    si08 *tmpBuf = tmpBuf_.alloc(ncells);
    memcpy(tmpBuf, _mapr.raw_data_ptr(ipos), ncells * sizeof(si08));
    si08 *in = tmpBuf;
    fl32 *fd = fdata;
    for (int igate = 0; igate < ncells; igate++, fd++, in++) {
      if (*in == doradeBad) {
	*fd = MISSING_FL32;
      } else {
	*fd = (fl32) ((double) *in / doradeScale + doradeBias);
      }
    } // igate
  }
  break;
  
  } // switch

}

///////////////////////////////
// convert to output

void File2Fmq::_convertToOutput(fl32 *fdata,
				int ifield,
				int nfields,
				int ncells,
				double outScale,
				double outBias,
				void *outputData)
  
{
  
  // load up output buffer
  
  switch (_params.output_type) {
    
  case Params::OUTPUT_UI08: {
    
    ui08 *out = (ui08 *) outputData + ifield;
    fl32 *fd = fdata;
    for (int igate = 0; igate < ncells; igate++, out += nfields, fd++) {
      if (*fd == MISSING_FL32) {
	*out = MISSING_UI08;
      } else {
	int byteval = (int) ((*fd - outBias) / outScale + 0.5);
	if (byteval < 1) {
	  byteval = 1;
	}
	if (byteval > 255) {
	  byteval = 255;
	}
	*out = byteval;
      }
    } // igate
  }
  break;
  
  case Params::OUTPUT_UI16: {
    
    ui16 *out = (ui16 *) outputData + ifield;
    fl32 *fd = fdata;
    for (int igate = 0; igate < ncells; igate++, out += nfields, fd++) {
      if (*fd == MISSING_FL32) {
	*out = MISSING_UI16;
      } else {
	int shortval = (int) ((*fd - outBias) / outScale + 0.5);
	if (shortval < 1) {
	  shortval = 1;
	}
	if (shortval > 65535) {
	  shortval = 65535;
	}
	*out = shortval;
      }
    } // igate
  }
  break;
  
  case Params::OUTPUT_FL32: {
    
    fl32 *out = (fl32 *) outputData + ifield;
    fl32 *fd = fdata;
    for (int igate = 0; igate < ncells; igate++, out += nfields, fd++) {
      *out = *fd;
    } // igate
  }
  break;
  
  } // switch

}

///////////////////////////////////////////////////
// Search for a field name in an dorade_name list.
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

////////////////////////////////////////
// print cells for debugging
//

void File2Fmq::_printCells(PARAMETER *fparam, int ipos,
			   int ncells, int doradeBad,
			   double doradeScale, double doradeBias)
  
{

  switch (fparam->binary_format) {

  case 4: {
    TaArray<fl32> tmpBuf_;
    fl32 *tmpBuf = tmpBuf_.alloc(ncells);
    memcpy(tmpBuf, _mapr.raw_data_ptr(ipos), ncells * sizeof(fl32));
    fl32 *in = tmpBuf;
    for (int igate = 0; igate < ncells; igate++, in++) {
      if (*in == doradeBad) {
	cerr << "    gate, val: " << igate << ", bad" << endl;
      } else {
	cerr << "    gate, val: " << igate << ", " << *in << endl;
      }
    } // igate
  }
  break;
  
  case 3: {
    TaArray<si32> tmpBuf_;
    si32 *tmpBuf = tmpBuf_.alloc(ncells);
    memcpy(tmpBuf, _mapr.raw_data_ptr(ipos), ncells * sizeof(si32));
    si32 *in = tmpBuf;
    for (int igate = 0; igate < ncells; igate++, in++) {
      if (*in == doradeBad) {
	cerr << "    gate, val: " << igate << ", bad" << endl;
      } else {
	double dval = (double) *in / doradeScale + doradeBias;
	cerr << "    gate, int val, float val: " << igate
	     << ", " << *in
	     << ", " << dval << endl;
      }
    } // igate
  }
  break;
  
  case 2: {
    TaArray<si16> tmpBuf_;
    si16 *tmpBuf = tmpBuf_.alloc(ncells);
    memcpy(tmpBuf, _mapr.raw_data_ptr(ipos), ncells * sizeof(si16));
    si16 *in = tmpBuf;
    for (int igate = 0; igate < ncells; igate++, in++) {
      if (*in == doradeBad) {
	cerr << "    gate, val: " << igate << ", bad" << endl;
      } else {
	double dval = (double) *in / doradeScale + doradeBias;
	cerr << "    gate, int val, float val: " << igate
	     << ", " << *in
	     << ", " << dval << endl;
      }
    } // igate
  }
  break;
  
  case 1:
  default: {
    TaArray<si08> tmpBuf_;
    si08 *tmpBuf = tmpBuf_.alloc(ncells);
    memcpy(tmpBuf, _mapr.raw_data_ptr(ipos), ncells * sizeof(si08));
    si08 *in = tmpBuf;
    for (int igate = 0; igate < ncells; igate++, in++) {
      if (*in == doradeBad) {
	cerr << "    gate, val: " << igate << ", bad" << endl;
      } else {
	double dval = (double) *in / doradeScale + doradeBias;
	cerr << "    gate, int val, float val: " << igate
	     << ", " << *in
	     << ", " << dval << endl;
      }
    } // igate
  }
  break;
  
  } // switch
  
}

