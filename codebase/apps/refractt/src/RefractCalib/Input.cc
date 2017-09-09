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
 * @file Input.cc
 *
 * @class Input
 *
 * Base class for Refract input classes.
 *  
 * @date 12/1/2008
 *
 */

#include "Input.hh"
#include "Calib.hh"
#include <Refract/RefractConstants.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <rapmath/math_macros.h>
#include <toolsa/LogStream.hh>
#include <toolsa/str.h>
#include <cmath>
#include <cstdlib>
using std::string;

// Globals

const int Input::I_FIELD_INDEX = 0;
const int Input::Q_FIELD_INDEX = 1;
const int Input::SNR_FIELD_INDEX = 2;

const double Input::OFFSET_ABOVE_AVERAGE = 0.2;  // Noise threshold set to .2*10 dB above average
const double Input::SNR_NOISE_MAX = 0.25;
const double Input::DM_NOISE = -114.4132;


//------------------------------------------------------------------------
Input::Input(const bool raw_iq_in_input,
	     const std::string &raw_i_field_name,
	     const std::string &raw_q_field_name,
	     const std::string &niq_field_name,
	     const std::string &aiq_field_name,
	     const std::string &snr_field_name,
	     const double input_niq_scale,
	     const bool invert_target_angle_sign,
	     const int elevation_num,
	     const int num_output_beams, const int num_output_gates,
	     const bool debug_flag, const bool verbose_flag) :
  _debug(debug_flag || verbose_flag),
  _verbose(verbose_flag),
  _rawIQinInput(raw_iq_in_input),
  _rawIFieldName(raw_i_field_name),
  _rawQFieldName(raw_q_field_name),
  _niqFieldName(niq_field_name),
  _aiqFieldName(aiq_field_name),
  _snrFieldName(snr_field_name),
  _inputNiqScale(input_niq_scale),
  _invertTargetAngleSign(invert_target_angle_sign),
  _useElevationNum(true),
  _elevationNum(elevation_num),
  _minElevationAngle(-1.0),
  _maxElevationAngle(-1.0),
  _numOutputGates(num_output_gates),
  _numOutputBeams(num_output_beams)
{
}

//------------------------------------------------------------------------
Input::Input(const bool raw_iq_in_input,
	     const std::string &raw_i_field_name,
	     const std::string &raw_q_field_name,
	     const std::string &niq_field_name,
	     const std::string &aiq_field_name,
	     const std::string &snr_field_name,
	     const double input_niq_scale,
	     const bool invert_target_angle_sign,
	     const double min_elevation_angle,
	     const double max_elevation_angle,
	     const int num_output_beams, const int num_output_gates,
	     const bool debug_flag, const bool verbose_flag) :
  _debug(debug_flag || verbose_flag),
  _verbose(verbose_flag),
  _rawIQinInput(raw_iq_in_input),
  _rawIFieldName(raw_i_field_name),
  _rawQFieldName(raw_q_field_name),
  _niqFieldName(niq_field_name),
  _aiqFieldName(aiq_field_name),
  _snrFieldName(snr_field_name),
  _inputNiqScale(input_niq_scale),
  _invertTargetAngleSign(invert_target_angle_sign),
  _useElevationNum(false),
  _elevationNum(0),
  _minElevationAngle(min_elevation_angle),
  _maxElevationAngle(max_elevation_angle),
  _numOutputGates(num_output_gates),
  _numOutputBeams(num_output_beams)
{
}

  
//------------------------------------------------------------------------
Input::~Input()
{
}

//------------------------------------------------------------------------
bool Input::getNextScan(const std::string &file_path, const std::string &host,
			DsMdvx &mdvx)
{
  // Read the raw data from the file.  On return, the file contains the
  // following fields:  I, Q, SNR.

  if (!_readInputFile(file_path, host, mdvx))
  {
    return false;
  }
  
  // Reposition the data so the gates will match up between scans
  _repositionData(mdvx);
  return true;
}

//------------------------------------------------------------------------
void Input::_calcIQ(MdvxField &niq_field, MdvxField &aiq_field,
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

  for (int i = 0; i < scan_size; ++i)
  {
    if (niq_data[i] > 35.0 || niq_data[i] < -35.0)
      niq_data[i] = niq_field_hdr.missing_data_value;
    if (aiq_data[i] < -180.0 || aiq_data[i] > 360.0)
      aiq_data[i] = aiq_field_hdr.missing_data_value;
  }

  // Calculate raw I/Q from NIQ/AIQ and calculate the average NIQ noise

  int num_noise_values = 0;
  float noise_sum = 0.0;
  
  for (int i = 0; i < scan_size; ++i)
  {
    // Calculate the initial I/Q values.  These values will be updated
    // below based on the noise found in the input fields

    if (niq_data[i] != niq_field_hdr.bad_data_value &&
	niq_data[i] != niq_field_hdr.missing_data_value &&
	aiq_data[i] != aiq_field_hdr.bad_data_value &&
	aiq_data[i] != aiq_field_hdr.missing_data_value)
    {
      if (_invertTargetAngleSign)
      {
	i_data[i] =
	  pow((double)10.0,
	      (double)niq_data[i]) * sin(aiq_data[i] * DEG_TO_RAD);
	q_data[i] =
	  pow((double)10.0,
	      (double)niq_data[i]) * cos(aiq_data[i] * DEG_TO_RAD);
      }
      else
      {
	i_data[i] =
	  pow((double)10.0,
	      (double)niq_data[i]) * cos(aiq_data[i] * DEG_TO_RAD);
	q_data[i] =
	  pow((double)10.0,
	      (double)niq_data[i]) * sin(aiq_data[i] * DEG_TO_RAD);
      }
    }

    // Get the total noise in the NIQ field

    int gate_index = (i % niq_field_hdr.nx);
    if ((gate_index >= 9 * niq_field_hdr.nx / 10) &&
	niq_data[i] != niq_field_hdr.bad_data_value &&
	niq_data[i] != niq_field_hdr.missing_data_value &&
	snr_data[i] != snr_field_hdr.bad_data_value &&
	snr_data[i] != snr_field_hdr.missing_data_value &&
	snr_data[i] < SNR_NOISE_MAX)
    {
      noise_sum += pow((double)10.0, (double)niq_data[i]);
      ++num_noise_values;
    }
  } /* endfor - i */
    
  double av_noise_niq;
    
  if (num_noise_values > 1)
    av_noise_niq = log10(noise_sum / (float)num_noise_values);
  else
    av_noise_niq = -refract::VERY_LARGE;
    
  // Get the best estimate on the average NIQ/AIQ vector introduced by PIRAQ

  num_noise_values = 0;
  float noise_i_sum = 0.0;
  float noise_q_sum = 0.0;
  
  for (int i = 0; i < scan_size; ++i)
  {
    if (niq_data[i] != niq_field_hdr.bad_data_value &&
	niq_data[i] != niq_field_hdr.missing_data_value &&
	niq_data[i] < av_noise_niq + OFFSET_ABOVE_AVERAGE)
    {
      noise_i_sum += i_data[i];
      noise_q_sum += q_data[i];
      num_noise_values++;
    }
  }
  
  float noise_i = 0.0;
  float noise_q = 0.0;

  if (num_noise_values > 0)
  {
    noise_i = noise_i_sum / (float)num_noise_values;
    noise_q = noise_q_sum / (float)num_noise_values;
  }

  // Subtract it from the NIQ/AIQ in vector form (rawi, rawq)

  for (int i = 0; i < scan_size; ++i)
  {
    if (niq_data[i] != niq_field_hdr.bad_data_value &&
	niq_data[i] != niq_field_hdr.missing_data_value &&
	aiq_data[i] != aiq_field_hdr.bad_data_value &&
	aiq_data[i] != aiq_field_hdr.missing_data_value)
    {
      i_data[i] -= noise_i;
      q_data[i] -= noise_q;
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
bool Input::_readInputFile(const std::string &file_path,
			   const std::string &host,
			   DsMdvx &mdvx) const
{
  // Set up the read request.  Note that if you change the order of the 
  // fields in the request, you will have to change code in other places.

  mdvx.clearRead();
  
  string fpath = "mdvp:://";
  fpath = fpath + host;
  fpath = fpath + "::";
  fpath = fpath + file_path;
  mdvx.setReadPath(fpath);
  
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_useElevationNum)
    mdvx.setReadPlaneNumLimits(_elevationNum, _elevationNum);
  else
    mdvx.setReadVlevelLimits(_minElevationAngle,
			     _maxElevationAngle);
  
  if (_rawIQinInput)
  {
    mdvx.addReadField(_rawIFieldName);
    mdvx.addReadField(_rawQFieldName);
  }
  else
  {
    mdvx.addReadField(_niqFieldName);
    mdvx.addReadField(_aiqFieldName);
  }
  
  mdvx.addReadField(_snrFieldName);

  // Read the file

  if (mdvx.readVolume() != 0)
  {
    LOG(ERROR) << "reading input file: " << fpath;
    LOG(ERROR) << mdvx.getErrStr();
    return false;
  }

  if (mdvx.getNFields() != 3)
  {
    LOG(ERROR) << "File did not have requested fields";
    LOG(ERROR) << "File:" << fpath;
    return false;
  }
  
  // Check the field projections to make sure they match and make sure 
  // the data meets all of the requirements of this algorithm

  Mdvx::field_header_t field_hdr0 = mdvx.getField(0)->getFieldHeader();
  Mdvx::field_header_t field_hdr1 = mdvx.getField(1)->getFieldHeader();
  Mdvx::field_header_t field_hdr2 = mdvx.getField(2)->getFieldHeader();
  
  MdvxPjg proj0(field_hdr0);
  MdvxPjg proj1(field_hdr1);
  MdvxPjg proj2(field_hdr2);
  
  if (proj0 != proj1 || proj0 != proj2)
  {
    LOG(ERROR) << "Input field projections don't match";
    LOG(ERROR) << "Field 0 (NIQ or I) projection:";
    proj0.print(cerr);
    LOG(ERROR) << "Field 1 (AIQ or Q) projection:";
    proj1.print(cerr);
    LOG(ERROR) << "Field 2 (SNR or power) projection:";
    proj2.print(cerr);
    return false;
  }
  
  if (proj0.getProjType() != Mdvx::PROJ_POLAR_RADAR)
  {
    LOG(ERROR) << "Input file contains " <<
      Mdvx::projType2Str(proj0.getProjType()) << " projection data";
    LOG(ERROR)<< "The projection must be polar radar for this algorithm";
    return false;
  }
  
  // Calculate I and Q if they are not taken directly from the input file.

  if (!_rawIQinInput)
  {
    _calcIQ(*(mdvx.getField(0)), *(mdvx.getField(1)), *(mdvx.getField(2)));
  }
  
  return true;
}


//------------------------------------------------------------------------
void Input::_repositionData(DsMdvx &mdvx) const
{
  // Handle each field individually

  for (int field_num = 0; field_num < mdvx.getNFields(); ++field_num)
  {
    MdvxField *field = mdvx.getField(field_num);
    
    // Get the needed field information and allocate space for the
    // new data.  Our new data field will have 360 degrees of data
    // containing the specified number of gates.

    Mdvx::field_header_t field_hdr = field->getFieldHeader();
    fl32 *in_data = (fl32 *)field->getVol();

    int num_output_azim = (int)(360.0 / field_hdr.grid_dy);

    fl32 *out_data = new fl32[num_output_azim * _numOutputGates];
    
    for (int az_out = 0; az_out < num_output_azim; ++az_out)
    {
      // Calculate the index of the input beam for this output beam

      double az = ((double)az_out * field_hdr.grid_dy);
      while (az < field_hdr.grid_miny)
	az += 360.0;
      while (az >= field_hdr.grid_miny + 360.0)
	az -= 360.0;

      int az_in = (int)((az - field_hdr.grid_miny) / field_hdr.grid_dy);
      
      if (az_in < 0)
      {
	LOG(ERROR) << "Error in az_in calculation, az_in = " << az_in;
	exit(0);
      }
      
      // Copy the beam data and fill out the beam.  If we are outside of the
      // data in the input scan then fill it in with missing data values.

      int out_index = az_out * _numOutputGates;

      if (az_in >= field_hdr.ny)
      {
	// In this case, the input data only contains a sector so we need to
	// fill in the missing beams with missing data

	for (int gate = 0; gate < _numOutputGates; ++gate)
	{
	  if ((string)field_hdr.field_name == _rawIFieldName ||
	      (string)field_hdr.field_name_long == _rawIFieldName ||
	      (string)field_hdr.field_name == _rawQFieldName ||
	      (string)field_hdr.field_name_long == _rawQFieldName)
	    out_data[out_index + gate] = 0.0;
	  else
	    out_data[out_index + gate] = field_hdr.missing_data_value;
	}
	
      }
      else
      {
	// In this case, we have an input beam for this azimuth.  Copy the
	// data from the beam into the output scan.  Truncate any gates
	// past the end of the output beam; fill in short beams with missing
	// data in the farther gates.

	int in_index = az_in * field_hdr.nx;
      
	if (field_hdr.nx >= _numOutputGates)
	{
	  memcpy(&out_data[out_index], &in_data[in_index],
		 _numOutputGates * sizeof(fl32));
	}
	else
	{
	  memcpy(&out_data[out_index], &in_data[in_index],
		 field_hdr.nx * sizeof(fl32));
	  
	  for (int gate = field_hdr.nx; gate < _numOutputGates; ++gate)
	    out_data[out_index + gate] = field_hdr.missing_data_value;
	}
      }
      
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
