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
/**
 *
 * @file Reader.cc
 *
 * @class Reader
 *
 * Base class for Refract input classes.
 *  
 * @date 12/1/2008
 *
 */

#include <Refract/RefractConstants.hh>
#include <Refract/FieldWithData.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <rapmath/math_macros.h>
#include <toolsa/LogStream.hh>
#include <toolsa/str.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "Reader.hh"
using std::string;

// Globals

// Noise threshold set to .2*10 dB above average
const double Reader::OFFSET_ABOVE_AVERAGE = 0.2;  
const double Reader::SNR_NOISE_MAX = 0.25;
const double Reader::DM_NOISE = -114.4132;

//------------------------------------------------------------------------

Reader::Reader(const Params &params) :
        _params(params),
        _rawIQinInput(_params.raw_iq_in_input),
        _rawIFieldName(_params.raw_i_field_name),
        _rawQFieldName(_params.raw_q_field_name),
        _niqFieldName(_params.niq_field_name),
        _aiqFieldName(_params.aiq_field_name),
        _qualityFromWidth(_params.quality_source == Params::QUALITY_FROM_WIDTH),
        _qualityFieldName(_params.quality_field_name),
        _snrInInput(_params.snr_available),
        _snrFieldName(_params.snr_field_name),
        _dbzFieldName(_params.dbz_field_name),
        _phaseErrorFieldName("phase_er"), // fix this later
        _inputNiqScale(_params.input_niq_scale),
        _invertTargetAngleSign(_params.invert_target_angle_sign),
        _minElevationAngle(_params.elevation_angle.min_angle),
        _maxElevationAngle(_params.elevation_angle.max_angle),
        _numOutputGates(_params.num_range_bins),
        _numOutputBeams(_params.num_azim),
        _debugLat(_params.debug_lat),
        _debugLon(_params.debug_lon),
        _debugNpt(_params.debug_npt)

{

  _debugX = -9999.0;
  _debugX = -9999.0;
  _debugIpt.clear();

}

//------------------------------------------------------------------------
Reader::~Reader()
{
}

//-------------------------------------------------------------------------
bool Reader::getScan(const DateTime &data_time, int search_margin,
                     const std::string &url, DsMdvx &mdvx)
{
  mdvx.clearRead();
  mdvx.setReadTime(Mdvx::READ_CLOSEST, url, search_margin, data_time.utime());
  if (!_readInputFile(mdvx)) {
    return false;
  }
  // Reposition the data so the gates will match up between scans
  _repositionData(mdvx);
  return true;
}


//------------------------------------------------------------------------
bool Reader::getNextScan(const std::string &file_path,
                         DsMdvx &mdvx)
{

  // Set up the read request.  Note that if you change the order of the 
  // fields in the request, you will have to change code in other places.

  mdvx.clearRead();
  mdvx.setReadPath(file_path);
  
  // Read the raw data from the file.  On return, the file contains the
  // fields configured for
  if (!_readInputFile(mdvx)) {
    return false;
  }
  
  // Reposition the data so the gates will match up between scans
  _repositionData(mdvx);
  return true;
}

//------------------------------------------------------------------------
FieldWithData Reader::getI(DsMdvx &source) const
{
  FieldWithData f(source.getFieldByName(_rawIFieldName));
  return f;
}

//------------------------------------------------------------------------
FieldWithData Reader::getQ(DsMdvx &source) const
{
  FieldWithData f(source.getFieldByName(_rawQFieldName));
  return f;
}

//------------------------------------------------------------------------
FieldWithData Reader::getSNR(DsMdvx &source) const
{
  FieldWithData f(source.getFieldByName(_snrFieldName));
  return f;
}

//------------------------------------------------------------------------
FieldWithData Reader::getQuality(DsMdvx &source) const
{
  FieldWithData f(source.getFieldByName(_qualityFieldName));
  return f;
}

//------------------------------------------------------------------------
FieldWithData Reader::getPhaseError(DsMdvx &source) const
{
  FieldWithData f(source.getFieldByName(_phaseErrorFieldName));
  return f;
}

//------------------------------------------------------------------------
void Reader::_calcIQ(MdvxField &niq_field, MdvxField &aiq_field,
                     const MdvxField &snr_field) const
{
  // Get pointers to the NIQ/AIQ fields.  Note that the order of the fields
  // in the file matches the order in the read request.

  Mdvx::field_header_t niq_field_hdr = niq_field.getFieldHeader();
  fl32 *niq_data = (fl32 *)niq_field.getVol();
  
  Mdvx::field_header_t aiq_field_hdr = aiq_field.getFieldHeader();
  fl32 *aiq_data = (fl32 *)aiq_field.getVol();
  
  Mdvx::field_header_t snr_field_hdr = snr_field.getFieldHeader();
  fl32 *snr_data = (fl32 *)snr_field.getVol();
  
  int scan_size = niq_field_hdr.nx * niq_field_hdr.ny;
  
  fl32 *i_data = new fl32[scan_size];
  memset(i_data, 0, scan_size * sizeof(fl32));
  
  fl32 *q_data = new fl32[scan_size];
  memset(q_data, 0, scan_size * sizeof(fl32));
  
  // First, some preparation work: rescaling of NIQ
  for (int i = 0; i < scan_size; ++i)
    niq_data[i] *= _inputNiqScale;

  // Testing of bad individual points

  for (int i = 0; i < scan_size; ++i) {
    bool debug = isDebugPt(i);
    if (debug) {
      printf("niq[%d] = %lf    aiq[%d]=%lf\n", i, niq_data[i], i, aiq_data[i]);
    }
    if (niq_data[i] > 35.0 || niq_data[i] < -35.0) {
      if (debug) printf("set niq to missing\n");
      niq_data[i] = niq_field_hdr.missing_data_value;
    }
    if (aiq_data[i] < -180.0 || aiq_data[i] > 360.0) {
      if (debug) printf("set aiq to missing\n");
      aiq_data[i] = aiq_field_hdr.missing_data_value;
    }
  }

  // Calculate raw I/Q from NIQ/AIQ and calculate the average NIQ noise

  int num_noise_values = 0;
  float noise_sum = 0.0;
  
  for (int i = 0; i < scan_size; ++i) {

    bool debug = isDebugPt(i);

    // Calculate the initial I/Q values.  These values will be updated
    // below based on the noise found in the input fields

    if (niq_data[i] != niq_field_hdr.bad_data_value &&
	niq_data[i] != niq_field_hdr.missing_data_value &&
	aiq_data[i] != aiq_field_hdr.bad_data_value &&
	aiq_data[i] != aiq_field_hdr.missing_data_value) {
      if (_invertTargetAngleSign) {
	i_data[i] =
	  pow((double)10.0,
	      (double)niq_data[i]) * sin(aiq_data[i] * DEG_TO_RAD);
	q_data[i] =
	  pow((double)10.0,
	      (double)niq_data[i]) * cos(aiq_data[i] * DEG_TO_RAD);
      } else {
	i_data[i] =
	  pow((double)10.0,
	      (double)niq_data[i]) * cos(aiq_data[i] * DEG_TO_RAD);
	q_data[i] =
	  pow((double)10.0,
	      (double)niq_data[i]) * sin(aiq_data[i] * DEG_TO_RAD);
	if (debug) {
	  printf("Set i and q from niq and aiq, i[%d]=%lf  q[%d]=%lf\n",
		 i, i_data[i], i, q_data[i]);
	}
      }
    }

    // Get the total noise in the NIQ field

    int gate_index = (i % niq_field_hdr.nx);
    if ((gate_index >= 9 * niq_field_hdr.nx / 10) &&
	niq_data[i] != niq_field_hdr.bad_data_value &&
	niq_data[i] != niq_field_hdr.missing_data_value &&
	snr_data[i] != snr_field_hdr.bad_data_value &&
	snr_data[i] != snr_field_hdr.missing_data_value &&
	snr_data[i] < SNR_NOISE_MAX) {
      noise_sum += pow((double)10.0, (double)niq_data[i]);
      ++num_noise_values;
    }
  } /* endfor - i */
    
  double av_noise_niq;
    
  if (num_noise_values > 1) {
    av_noise_niq = log10(noise_sum / (float)num_noise_values);
  } else {
    av_noise_niq = -refract::VERY_LARGE;
  }
    
  // Get the best estimate on the average NIQ/AIQ vector introduced by PIRAQ

  num_noise_values = 0;
  float noise_i_sum = 0.0;
  float noise_q_sum = 0.0;
  
  for (int i = 0; i < scan_size; ++i) {
    if (niq_data[i] != niq_field_hdr.bad_data_value &&
	niq_data[i] != niq_field_hdr.missing_data_value &&
	niq_data[i] < av_noise_niq + OFFSET_ABOVE_AVERAGE) {
      noise_i_sum += i_data[i];
      noise_q_sum += q_data[i];
      num_noise_values++;
    }
  }
  
  float noise_i = 0.0;
  float noise_q = 0.0;
  if (num_noise_values > 0) {
    noise_i = noise_i_sum / (float)num_noise_values;
    noise_q = noise_q_sum / (float)num_noise_values;
  }
  LOG(DEBUG_VERBOSE) << "Number of points with niq less than average noise = "
		     << num_noise_values;
  LOG(DEBUG_VERBOSE) << "noise_i = " << noise_i << " noise_q = " << noise_q;
  

  // Subtract it from the NIQ/AIQ in vector form (rawi, rawq)

  for (int i = 0; i < scan_size; ++i) {
    bool debug = isDebugPt(i);
    if (niq_data[i] != niq_field_hdr.bad_data_value &&
	niq_data[i] != niq_field_hdr.missing_data_value &&
	aiq_data[i] != aiq_field_hdr.bad_data_value &&
	aiq_data[i] != aiq_field_hdr.missing_data_value) {
      if (debug) {
	printf("subtract noise from i[%d]=%lf and q[%d]=%lf",
	       i, i_data[i], i, q_data[i]);
      }
      i_data[i] -= noise_i;
      q_data[i] -= noise_q;
      if (debug) {
	printf("  result:  i[%d]=%lf and q[%d]=%lf\n",
	       i, i_data[i], i, q_data[i]);
      }
    }
  }

  // Update the I and Q field information

  memcpy(niq_data, i_data, scan_size * sizeof(fl32));

  niq_field_hdr.min_value = 0.0;
  niq_field_hdr.max_value = 0.0;
  niq_field_hdr.bad_data_value = refract::INVALID;
  niq_field_hdr.missing_data_value = refract::INVALID;
  STRcopy(niq_field_hdr.field_name_long, _rawIFieldName.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(niq_field_hdr.field_name, _rawIFieldName.c_str(),
	  MDV_SHORT_FIELD_LEN);
  niq_field_hdr.units[0] ='\0';
  niq_field.setFieldHeader(niq_field_hdr);
  
  memcpy(aiq_data, q_data, scan_size * sizeof(fl32));

  aiq_field_hdr.min_value = 0.0;
  aiq_field_hdr.max_value = 0.0;
  aiq_field_hdr.bad_data_value = refract::INVALID;
  aiq_field_hdr.missing_data_value = refract::INVALID;
  STRcopy(aiq_field_hdr.field_name_long, _rawQFieldName.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(aiq_field_hdr.field_name, _rawQFieldName.c_str(),
	  MDV_SHORT_FIELD_LEN);
  aiq_field_hdr.units[0] ='\0';
  aiq_field.setFieldHeader(aiq_field_hdr);

  // Reclaim memory

  delete [] i_data;
  delete [] q_data;
}

//------------------------------------------------------------------------
bool Reader::_readInputFile(DsMdvx &mdvx)
{

  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.setReadScalingType(Mdvx::SCALING_NONE);
  mdvx.setReadVlevelLimits(_minElevationAngle, _maxElevationAngle);
  
  std::vector<string> names;
  if (_rawIQinInput) {
    names.push_back(_rawIFieldName);
    names.push_back(_rawQFieldName);
  } else {
    names.push_back(_niqFieldName);
    names.push_back(_aiqFieldName);
  }
  if (_snrInInput) {
    // read in measured SNR
    names.push_back(_snrFieldName);
  } else {
    // we read in DBZ, convert to SNR
    names.push_back(_dbzFieldName);
  }

  // for now require this always
  names.push_back(_qualityFieldName);

  for (size_t i=0; i<names.size(); ++i) {
    mdvx.addReadField(names[i].c_str());
  }

  // Read the file
  if (mdvx.readVolume() != 0) {
    LOG(ERROR) << "reading input file:";
    LOG(ERROR) << mdvx.getErrStr();
    return false;
  }

  if (mdvx.getNFields() != 4) {
    LOG(ERROR) << "File did not have requested fields";
    LOG(ERROR) << "  tried to read: ";
    for (size_t i=0; i<names.size(); ++i) {
      LOG(ERROR) << "     " << names[i];
    }
    LOG(ERROR) << "Was able to read " << mdvx.getNFields() << " of them";
    return false;
  }
  
  // Check the field projections to make sure they match and make sure 
  // the data meets all of the requirements of this algorithm
  Mdvx::field_header_t field_hdr0 =
    mdvx.getField(names[0].c_str())->getFieldHeader();
  Mdvx::field_header_t field_hdr1 = 
    mdvx.getField(names[1].c_str())->getFieldHeader();
  Mdvx::field_header_t field_hdr2 = 
    mdvx.getField(names[2].c_str())->getFieldHeader();
  Mdvx::field_header_t field_hdr3 = 
    mdvx.getField(names[3].c_str())->getFieldHeader();
  
  if (field_hdr0.nz != 1 || field_hdr1.nz != 1 ||
      field_hdr2.nz != 1 || field_hdr3.nz != 1) {
    LOG(ERROR) << "More that 1 sweep read in, must be only 1";
    LOG(ERROR) << "Field " << field_hdr0.field_name << " nz:" << field_hdr0.nz;
    LOG(ERROR) << "Field " << field_hdr1.field_name << " nz:" << field_hdr1.nz;
    LOG(ERROR) << "Field " << field_hdr2.field_name << " nz:" << field_hdr2.nz;
    LOG(ERROR) << "Field " << field_hdr3.field_name << " nz:" << field_hdr3.nz;
    return false;
  }
  
  MdvxPjg proj0(field_hdr0);
  MdvxPjg proj1(field_hdr1);
  MdvxPjg proj2(field_hdr2);
  MdvxPjg proj3(field_hdr3);
  
  if (proj0 != proj1 || proj0 != proj2 || proj0 != proj3) {
    LOG(ERROR) << "Input field projections don't match";
    LOG(ERROR) << "Field 0 (NIQ or I) projection:";
    proj0.print(cerr);
    LOG(ERROR) << "Field 1 (AIQ or Q) projection:";
    proj1.print(cerr);
    LOG(ERROR) << "Field 2 (SNR or DBZ) projection:";
    proj2.print(cerr);
    LOG(ERROR) << "Field 3 (quality) projection:";
    proj3.print(cerr);
    return false;
  }
  
  if (proj0.getProjType() != Mdvx::PROJ_POLAR_RADAR) {
    LOG(ERROR) << "Input file contains " <<
      Mdvx::projType2Str(proj0.getProjType()) << " projection data";
    LOG(ERROR)<< "The projection must be polar radar for this algorithm";
    return false;
  }
  
  setDebug(proj0);

  // Calculate SNR if not available in the input file.

  MdvxField *snr = mdvx.getField(names[2].c_str());;
  if (!_snrInInput) {
    // overwrite and change names
    _calcSnr(*snr);
  }

  // Calculate I and Q if they are not taken directly from the input file.

  if (!_rawIQinInput) {
    // overwrite and change names
    _calcIQ(*(mdvx.getField(names[0].c_str())),
	    *(mdvx.getField(names[1].c_str())),
	    *snr);
  }
  
  return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Calculate SNR from dbz and replace the dbz values with the
// calculated SNR values

void Reader::_calcSnr(MdvxField &dbz_field) const
{

  // Get pointers to the dbz field info.  Note that the order of the fields
  // in the file matches the order in the read request.
  
  Mdvx::field_header_t dbz_field_hdr = dbz_field.getFieldHeader();
  fl32 *dbz_data = (fl32 *)dbz_field.getVol();
  
  // For polar radar in Mdvx, z is elevation, y is azimuth, x is range

  int nAz = dbz_field_hdr.ny;
  int nGates = dbz_field_hdr.ny;
  double startRangeKm = dbz_field_hdr.grid_minx;
  double gateSpacingKm = dbz_field_hdr.grid_dx;
  
  // first estimate noise at each range
  
  vector<double> noiseDbz(nGates);
  double range = startRangeKm;
  if (range == 0) {
    range = gateSpacingKm / 10.0;
  }
  for (int igate = 0; igate < nGates; igate++, range += gateSpacingKm) {
    noiseDbz[igate] = _params.noise_dbz_at_100km +
      20.0 * (log10(range) - log10(100.0));
  }

  // now estimate snr from dbz
  
  size_t jj = 0;
  for (int iaz = 0; iaz < nAz; iaz++) {
    for (int igate = 0; igate < dbz_field_hdr.nx; igate++, jj++) {

      if (dbz_data[jj] == dbz_field_hdr.bad_data_value ||
          dbz_data[jj] == dbz_field_hdr.missing_data_value) {
        dbz_data[jj] = -20.0;
      } else {
        dbz_data[jj] = dbz_data[jj] - noiseDbz[igate];
      }
      
    } // igate
  } // iaz
  
  // Update the SNR field information.  The SNR field replaces the
  // original DBZ field

  dbz_field_hdr.min_value = 0.0;
  dbz_field_hdr.max_value = 0.0;
  dbz_field_hdr.bad_data_value = refract::MISSING_DATA_VALUE;
  dbz_field_hdr.missing_data_value = refract::MISSING_DATA_VALUE;
  STRcopy(dbz_field_hdr.field_name_long, _snrFieldName.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(dbz_field_hdr.field_name, _snrFieldName.c_str(),
	  MDV_SHORT_FIELD_LEN);
  dbz_field_hdr.units[0] ='\0';
  dbz_field.setFieldHeader(dbz_field_hdr);
}

//------------------------------------------------------------------------
void Reader::_repositionData(DsMdvx &mdvx) const
{
  // Handle each field individually

  for (size_t field_num = 0; field_num < mdvx.getNFields(); ++field_num) {

    MdvxField *field = mdvx.getField(field_num);
    
    // Get the needed field information and allocate space for the
    // new data.  Our new data field will have the specified dimensions

    Mdvx::field_header_t field_hdr = field->getFieldHeader();
    fl32 *in_data = (fl32 *)field->getVol();

    int num_output_azim = (int)(_numOutputBeams / field_hdr.grid_dy);
    // int num_output_azim = (int)(360.0 / field_hdr.grid_dy);

    fl32 *out_data = new fl32[num_output_azim * _numOutputGates];
    
    for (int az_out = 0; az_out < num_output_azim; ++az_out) {

      // Calculate the index of the input beam for this output beam

      double az = ((double)az_out * field_hdr.grid_dy);
      while (az < field_hdr.grid_miny)
	az += 360.0;
      while (az >= field_hdr.grid_miny + 360.0)
	az -= 360.0;

      int az_in = (int)((az - field_hdr.grid_miny) / field_hdr.grid_dy);
      
      if (az_in < 0) {
	LOG(ERROR) << "Error in az_in calculation, az_in = " << az_in;
	exit(0);
      }
      
      // Copy the beam data and fill out the beam.  If we are outside of the
      // data in the input scan then fill it in with missing data values.

      int out_index = az_out * _numOutputGates;

      if (az_in >= field_hdr.ny) {

	// In this case, the input data only contains a sector so we need to
	// fill in the missing beams with missing data

	for (int gate = 0; gate < _numOutputGates; ++gate) {
	  if ((string)field_hdr.field_name == _rawIFieldName ||
	      (string)field_hdr.field_name_long == _rawIFieldName ||
	      (string)field_hdr.field_name == _rawQFieldName ||
	      (string)field_hdr.field_name_long == _rawQFieldName) {
	    out_data[out_index + gate] = 0.0;
          } else {
	    out_data[out_index + gate] = field_hdr.missing_data_value;
          }
	}
	
      } else {

	// In this case, we have an input beam for this azimuth.  Copy the
	// data from the beam into the output scan.  Truncate any gates
	// past the end of the output beam; fill in short beams with missing
	// data in the farther gates.

	int in_index = az_in * field_hdr.nx;
      
	if (field_hdr.nx >= _numOutputGates) {
	  memcpy(&out_data[out_index], &in_data[in_index],
		 _numOutputGates * sizeof(fl32));
	} else {
	  memcpy(&out_data[out_index], &in_data[in_index],
		 field_hdr.nx * sizeof(fl32));
          for (int gate = field_hdr.nx; gate < _numOutputGates; ++gate) {
            out_data[out_index + gate] = field_hdr.missing_data_value;
          }
        }

      } // if (az_in >= field_hdr.ny)
      
    } /* endfor - az_out */
    
    // Update the data and the field header in the field

    field_hdr.nx = _numOutputGates;
    field_hdr.ny = num_output_azim;
    field_hdr.volume_size = field_hdr.nx * field_hdr.ny * 4;
    field_hdr.grid_miny = 0.0;

    field->setFieldHeader(field_hdr);
    field->setVolData(out_data, field_hdr.volume_size,
		      Mdvx::ENCODING_FLOAT32, Mdvx::SCALING_NONE);

    delete [] out_data;
    
  } /* endfor - field_num */
  
}

///////////////////////////////////////////////////////////////////////////
// debugging

void Reader::setDebug(const MdvxPjg &proj)
{
  _debugX = _debugY = -9999.0;
  _debugIpt.clear();
  if (_debugLat < -9990.0 || _debugLon < -9990.0) {
    return;
  }
  proj.latlon2xyIndex(_debugLat, _debugLon, _debugX, _debugY);
  for (int y=_debugY-_debugNpt; y<=_debugY+_debugNpt; ++y) {
    if (y >= 0 && y<proj.getNy()) {
      for (int x=_debugX-_debugNpt; x<=_debugX+_debugNpt; ++x) {
	if (x >= 0 && x<proj.getNx()) {
	  _debugIpt.push_back(proj.getNx() + x);
	}
      }
    }
  }
}

bool Reader::isDebugPt(int i) const
{
  if (_debugIpt.empty()) {
    return false;
  } else {
    return find(_debugIpt.begin(), _debugIpt.end(), i) != _debugIpt.end();
  }
}

