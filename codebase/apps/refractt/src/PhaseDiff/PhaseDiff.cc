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
 * @file PhaseDiff.cc
 *
 * @class PhaseDiff
 *
 * PhaseDiff is the top level application class.
 *  
 * @date 2/18/2010
 *
 */

#include <assert.h>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <string>

#include <toolsa/toolsa_macros.h>
#include <toolsa/os_config.h>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "PhaseDiff.hh"
#include "Params.hh"

using namespace std;

// Global variables

PhaseDiff *PhaseDiff::_instance = (PhaseDiff *)NULL;

const fl32 PhaseDiff::MISSING_DATA_VALUE = -9999.9;

const double PhaseDiff::SNR_NOISE_MAX = 0.25;
const int PhaseDiff::VERY_LARGE = 2147483647;
const double PhaseDiff::OFFSET_ABOVE_AVERAGE = 0.2;  // Noise threshold set to .2*10 dB above average
const double PhaseDiff::DM_NOISE = -114.4132;

/*********************************************************************
 * Constructor
 */

PhaseDiff::PhaseDiff(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "PhaseDiff::PhaseDiff()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (PhaseDiff *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = new char[strlen("unknown")+1];
  strcpy(params_path, "unknown");
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
  }
}


/*********************************************************************
 * Destructor
 */

PhaseDiff::~PhaseDiff()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst()
 */

PhaseDiff *PhaseDiff::Inst(int argc, char **argv)
{
  if (_instance == (PhaseDiff *)NULL)
    new PhaseDiff(argc, argv);
  
  return(_instance);
}

PhaseDiff *PhaseDiff::Inst()
{
  assert(_instance != (PhaseDiff *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool PhaseDiff::init()
{
  static const string method_name = "PhaseDiff::init()";
  
  // Create the colorscale files, if requested

  if (_params->create_phase_diff_colorscale)
    _createPhaseColorscale();

  if (_params->create_niq_colorscale)
    _createNiqColorscale();
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void PhaseDiff::run()
{
  static const string method_name = "PhaseDiff::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " << trigger_time << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _calcIQ()
 */

void PhaseDiff::_calcIQ(MdvxField &niq_field,
			MdvxField &aiq_field,
			const MdvxField &snr_field) const
{
  static const string method_name = "PhaseDiff::_calcIQ()";
  
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
    niq_data[i] *= _params->input_niq_scale;

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
      if (_params->invert_target_angle_sign)
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
    av_noise_niq = -VERY_LARGE;
    
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
  niq_field_hdr.bad_data_value = MISSING_DATA_VALUE;
  niq_field_hdr.missing_data_value = MISSING_DATA_VALUE;
  STRcopy(niq_field_hdr.field_name_long, _params->raw_i_field_name,
	  MDV_LONG_FIELD_LEN);
  STRcopy(niq_field_hdr.field_name, _params->raw_i_field_name,
	  MDV_SHORT_FIELD_LEN);
  niq_field_hdr.units[0] ='\0';
  niq_field.setFieldHeader(niq_field_hdr);
  
  memcpy(aiq_data, q_data, scan_size * sizeof(fl32));

  aiq_field_hdr.min_value = 0.0;
  aiq_field_hdr.max_value = 0.0;
  aiq_field_hdr.bad_data_value = MISSING_DATA_VALUE;
  aiq_field_hdr.missing_data_value = MISSING_DATA_VALUE;
  STRcopy(aiq_field_hdr.field_name_long, _params->raw_q_field_name,
	  MDV_LONG_FIELD_LEN);
  STRcopy(aiq_field_hdr.field_name, _params->raw_q_field_name,
	  MDV_SHORT_FIELD_LEN);
  aiq_field_hdr.units[0] ='\0';
  aiq_field.setFieldHeader(aiq_field_hdr);
}


/*********************************************************************
 * _calcPhaseDiff()
 */

bool PhaseDiff::_calcPhaseDiff(const DsMdvx &mdvx1,
			       DsMdvx &mdvx2) const
{
  static const string method_name = "PhaseDiff::_calcPhaseDiff()";
  
  // Get pointers to the fields

  MdvxField *i1_field = mdvx1.getField(0);
  Mdvx::field_header_t i1_field_hdr = i1_field->getFieldHeader();
  fl32 *i1_data = (fl32 *)i1_field->getVol();
  
  MdvxField *q1_field = mdvx1.getField(1);
  Mdvx::field_header_t q1_field_hdr = q1_field->getFieldHeader();
  fl32 *q1_data = (fl32 *)q1_field->getVol();
  
  MdvxField *i2_field = mdvx2.getField(0);
  Mdvx::field_header_t i2_field_hdr = i2_field->getFieldHeader();
  fl32 *i2_data = (fl32 *)i2_field->getVol();
  
  MdvxField *q2_field = mdvx2.getField(1);
  Mdvx::field_header_t q2_field_hdr = q2_field->getFieldHeader();
  fl32 *q2_data = (fl32 *)q2_field->getVol();
  
  // Create the blank output fields and get the needed information

  MdvxPjg proj(i1_field_hdr);

  fl32 elevation = i1_field->getVlevelHeader().level[0];
  
  MdvxField *change_i_field = _createBlankField(proj, "change i", elevation);
  MdvxField *change_q_field = _createBlankField(proj, "change q", elevation);
  MdvxField *diff_phase_i_field = _createBlankField(proj, "diff phase i",
						    elevation);
  MdvxField *diff_phase_q_field = _createBlankField(proj, "diff phase q",
						    elevation);
  MdvxField *phase_field = _createBlankField(proj, "phase diff", elevation);
  MdvxField *niq_field = _createBlankField(proj, "niq", elevation);
  
  if (change_i_field == 0 || change_q_field == 0 ||
      diff_phase_i_field == 0 || diff_phase_q_field == 0 ||
      phase_field == 0 || niq_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating blank fields for the output fields" << endl;
    
    delete change_i_field;
    delete change_q_field;
    delete diff_phase_i_field;
    delete diff_phase_q_field;
    delete phase_field;
    delete niq_field;
    
    return false;
  }
  
  fl32 *change_i_data = (fl32 *)change_i_field->getVol();
  fl32 *change_q_data = (fl32 *)change_q_field->getVol();
  fl32 *diff_phase_i_data = (fl32 *)diff_phase_i_field->getVol();
  fl32 *diff_phase_q_data = (fl32 *)diff_phase_q_field->getVol();
  fl32 *phase_data = (fl32 *)phase_field->getVol();
  fl32 *niq_data = (fl32 *)niq_field->getVol();
  
  // Calculate the phase difference fields

  int data_size = i1_field_hdr.nx * i1_field_hdr.ny;
  
  for (int i = 0; i < data_size; ++i)
  {
    // Check for missing data

    if (i1_data[i] == i1_field_hdr.missing_data_value ||
	i1_data[i] == i1_field_hdr.bad_data_value ||
	q1_data[i] == q1_field_hdr.missing_data_value ||
	q1_data[i] == q1_field_hdr.bad_data_value ||
	i2_data[i] == i2_field_hdr.missing_data_value ||
	i2_data[i] == i2_field_hdr.bad_data_value ||
	q2_data[i] == q2_field_hdr.missing_data_value ||
	q2_data[i] == q2_field_hdr.bad_data_value)
      continue;
    
    // Calculate the change fields

    change_i_data[i] =
      (i2_data[i] * i1_data[i]) + (q2_data[i] * q1_data[i]);
    change_q_data[i] =
      (i2_data[i] * q1_data[i]) - (q2_data[i] * i1_data[i]);
    
    // Calculate the phase diff fields

    diff_phase_i_data[i] = i1_data[i] * i1_data[i];
    diff_phase_q_data[i] = q1_data[i] * q1_data[i];
    
    // Calculate the phase field

    if (change_i_data[i] != 0.0 || change_q_data[i] != 0.0)
    {
      phase_data[i] = -atan2(change_q_data[i], change_i_data[i]) / DEG_TO_RAD;
    }
    
    // Calculate the niq field

    if (diff_phase_i_data[i] != 0.0 || diff_phase_q_data[i] != 0)
    {
      niq_data[i] =
	10.0 * log10(sqrt((diff_phase_i_data[i] * diff_phase_i_data[i]) +
			  (diff_phase_q_data[i] * diff_phase_q_data[i])));
    }
  } /* endfor - i */
  
  // Add the calculated fields to the second (base) MDV file

  mdvx2.addField(change_i_field);
  mdvx2.addField(change_q_field);
  mdvx2.addField(diff_phase_i_field);
  mdvx2.addField(diff_phase_q_field);
  mdvx2.addField(phase_field);
  mdvx2.addField(niq_field);
  
  return true;
}


/*********************************************************************
 * _calcSnr()
 */

void PhaseDiff::_calcSnr(MdvxField &power_field) const
{
  static const string method_name = "PhaseDiff::_calcSnr()";
  
  // Get pointers to the power field info.  Note that the order of the fields
  // in the file matches the order in the read request.

  Mdvx::field_header_t power_field_hdr = power_field.getFieldHeader();
  fl32 *power_data = (fl32 *)power_field.getVol();
  
  int scan_size = power_field_hdr.nx * power_field_hdr.ny;
  
  // Calculate SNR from power and replace the power values with the
  // calculated SNR values

  for (int i = 0; i < scan_size; ++i)
  {
    if (power_data[i] == power_field_hdr.bad_data_value ||
	power_data[i] == power_field_hdr.missing_data_value)
    {
      power_data[i] = MISSING_DATA_VALUE;
      continue;
    }

    double noise = pow(10.0, DM_NOISE / 10.0);
    double power_plus_noise = pow(10.0, power_data[i] / 10.0);
    double power = power_plus_noise - noise;
      
    if (power > 0.0)
      power_data[i] = 10.0 * log10(power);
    else
      power_data[i] = MISSING_DATA_VALUE;
  } /* endfor - i */

  // Update the SNR field information.  The SNR field replaces the
  // original power field

  power_field_hdr.min_value = 0.0;
  power_field_hdr.max_value = 0.0;
  power_field_hdr.bad_data_value = MISSING_DATA_VALUE;
  power_field_hdr.missing_data_value = MISSING_DATA_VALUE;
  STRcopy(power_field_hdr.field_name_long, _params->snr_field_name,
	  MDV_LONG_FIELD_LEN);
  STRcopy(power_field_hdr.field_name, _params->snr_field_name,
	  MDV_SHORT_FIELD_LEN);
  power_field_hdr.units[0] ='\0';
  power_field.setFieldHeader(power_field_hdr);
}


/*********************************************************************
 * _createBlankField()
 */

MdvxField *PhaseDiff::_createBlankField(const MdvxPjg &proj,
					const string &field_name,
					const fl32 elevation) const
{
  static const string method_name = "PhaseDiff::_createBlankField()";
  
  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  proj.syncToFieldHdr(field_hdr);
  
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  field_hdr.data_dimension = 2;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = MISSING_DATA_VALUE;
  field_hdr.missing_data_value = MISSING_DATA_VALUE;
  STRcopy(field_hdr.field_name_long, field_name.c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_ELEV;
  vlevel_hdr.level[0] = elevation;
  
  // Create the blank field

  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}


/*********************************************************************
 * _createNiqColorscale()
 */

void PhaseDiff::_createNiqColorscale() const
{
  static const string method_name = "PhaseDiff::_createNiqColorscale()";
  
  FILE *file;

  if ((file = fopen(_params->niq_colorscale_path, "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening colorscale file for output" << endl;
    perror(_params->niq_colorscale_path);
    
    return;
  }
  
  fprintf(file, "%8.2f  %8.2f   #800080\n",
	  -100.0, -50.0);

  for (int i = 0; i < 120; ++i)
  {
    float step = 6.0 * ((float)(119 - i) / 120.0);
    
    short int red = 0, green = 0, blue = 0;
    
    if (step <= 1.0)
    {
      // violet to red

      red = (short int)(128 + (step * 127));
      green = 0;
      blue = (short int)(128 - (step * 128));
    }
    else if (step < 2.0)
    {
      red = 255;
      green = (short int)((step - 1.0) * 255);
      blue = 0;
    }
    else if (step == 2.0)
    {
      red = 247;
      green = 247;
      blue = 0;
    }
    else if (step <= 3.0)
    {
      red = (short int)((3.0 - step) * 255);
      green = 255;
      blue = 0;
    }
    else if (step <= 4.0)
    {
      red = 0;
      green = (short int)(255 - ((step - 3.0) * 127));
      blue = (short int)((step - 3.0) * 128);
    }
    else if (step <= 5.0)
    {
      red = 0;
      green = (short int)((5.0 - step) * 128);
      blue = 255 + (short int)((step - 5.0) * 127);
    }
    else if (step <= 6.0)
    {
      red = (short int)((step - 5.0) * 128);
      green = 0;
      blue = 255 - (short int)((step - 5.0) * 127);
    }
    else
    {
      fprintf(file, "*** step = %f\n", step);
    }
    
    double min_value = ((60.0 / 120.0) * (double)i) - 50.0;
    double max_value = ((60.0 / 120.0) * (double)(i+1)) - 50.0;
    
    fprintf(file, "%8.2f  %8.2f   #%02x%02x%02x\n",
	    min_value, max_value, red, green, blue);
  }
  
  fprintf(file, "%8.2f  %8.2f   #ffffff\n",
	  10.0, 100.0);

  fclose(file);
}


/*********************************************************************
 * _createPhaseColorscale()
 */

void PhaseDiff::_createPhaseColorscale() const
{
  static const string method_name = "PhaseDiff::_createPhaseColorscale()";
  
  FILE *file;

  if ((file = fopen(_params->phase_diff_colorscale_path, "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening colorscale file for output" << endl;
    perror(_params->phase_diff_colorscale_path);
    
    return;
  }
  
  for (int i = 0; i < 120; ++i)
  {
    float step = 6.0 * ((float)(119 - i) / 120.0);
    
    short int red = 0, green = 0, blue = 0;

    if (step <= 1.0)
    {
      // violet to red

      red = (short int)(128 + (step * 127));
      green = 0;
      blue = (short int)(128 - (step * 128));
    }
    else if (step < 2.0)
    {
      red = 255;
      green = (short int)((step - 1.0) * 255);
      blue = 0;
    }
    else if (step == 2.0)
    {
      red = 247;
      green = 247;
      blue = 0;
    }
    else if (step <= 3.0)
    {
      red = (short int)((3.0 - step) * 255);
      green = 255;
      blue = 0;
    }
    else if (step <= 4.0)
    {
      red = 0;
      green = (short int)(255 - ((step - 3.0) * 127));
      blue = (short int)((step - 3.0) * 128);
    }
    else if (step <= 5.0)
    {
      red = 0;
      green = (short int)((5.0 - step) * 128);
      blue = 255 + (short int)((step - 5.0) * 127);
    }
    else if (step <= 6.0)
    {
      red = (short int)((step - 5.0) * 128);
      green = 0;
      blue = 255 - (short int)((step - 5.0) * 127);
    }
    else
    {
      fprintf(file, "*** step = %f\n", step);
    }
    
    double min_value = ((360.0 / 120.0) * (double)i) - 180.0;
    double max_value = ((360.0 / 120.0) * (double)(i+1)) - 180.0;
    
    fprintf(file, "%5.0f  %5.0f   #%02x%02x%02x\n",
	    min_value, max_value, red, green, blue);
  }
  
  fclose(file);
}


/*********************************************************************
 * _initTrigger()
 */

bool PhaseDiff::_initTrigger(void)
{
  static const string method_name = "PhaseDiff::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::TIME_LIST :
  {
    DateTime start_time = _args->getStartTime();
    DateTime end_time = _args->getEndTime();
    
    if (start_time == DateTime::NEVER ||
	end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Must specify start and end times on command line" << endl;
      
      return false;
    }
    
    if (_params->debug)
    {
      cerr << "Initializing TIME_LIST trigger: " << endl;
      cerr << "   URL: " << _params->input_url << endl;
      cerr << "   start time: " << start_time << endl;
      cerr << "   end time: " << end_time << endl;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time.utime(), end_time.utime()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;      cerr << "Error initializing TIME_LIST trigger:" << endl;
      cerr << "    URL: " << _params->input_url << endl;
      cerr << "    Start time: " << start_time << endl;
      cerr << "    End time: " << end_time << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processData()
 */

bool PhaseDiff::_processData(const DateTime &trigger_time)
{
  static const string method_name = "PhaseDiff::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read the radar file from the trigger time

  DsMdvx mdvx2;
  
  if (!_readInputFile(mdvx2, trigger_time, 0))
    return false;
  
  // Read in the matching file

  DsMdvx mdvx1;
  
  if (!_readInputFile(mdvx1, trigger_time - _params->lookback_secs,
		      _params->lookback_search_margin))
    return false;
  
  if (_params->debug)
  {
    cerr << "  Base MDV file time: "
	 << DateTime::str(mdvx1.getMasterHeader().time_centroid) << endl;
    cerr << "  Matching MDV file time: "
	 << DateTime::str(mdvx2.getMasterHeader().time_centroid) << endl;
  }
  
  // Calculate the phase diff fields.  This adds the calculated fields to
  // the base input file -- mdvx2.

  if (!_calcPhaseDiff(mdvx1, mdvx2))
    return false;
  
  // Write the output file

//  for (int i = 0; i < mdvx2.getNFields(); ++i)
//  {
//    MdvxField *field = mdvx2.getField(i);
//    
//    field->convertType(Mdvx::ENCODING_INT8,
//		       Mdvx::COMPRESSION_BZIP,
//		       Mdvx::SCALING_DYNAMIC);
//  }
  
  if (mdvx2.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing file to URL: " << _params->output_url << endl;
    cerr << mdvx2.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readInputFile()
 */

bool PhaseDiff::_readInputFile(DsMdvx &mdvx,
			       const DateTime &data_time,
			       const int search_margin) const
{
  static const string method_name = "PhaseDiff::_readInputFile()";
  
  // Set up the read request.  Note that if you change the order of the 
  // fields in the request, you will have to change code in other places.

  mdvx.clearRead();
  
  mdvx.setReadTime(Mdvx::READ_CLOSEST,
		   _params->input_url,
		   search_margin, data_time.utime());
  
  mdvx.setReadPlaneNumLimits(_params->elevation_num,
			     _params->elevation_num);
  
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_params->raw_iq_in_input)
  {
    mdvx.addReadField(_params->raw_i_field_name);
    mdvx.addReadField(_params->raw_q_field_name);
  }
  else
  {
    mdvx.addReadField(_params->niq_field_name);
    mdvx.addReadField(_params->aiq_field_name);

    if (_params->snr_in_input)
    {
      mdvx.addReadField(_params->snr_field_name);
    }
    else
    {
      mdvx.addReadField(_params->power_field_name);
    }
  }
  
  // Read the file

  if (mdvx.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input file:" << endl;
    cerr << "   URL = " << _params->input_url << endl;
    cerr << "   time = " << data_time << endl;
    cerr << mdvx.getErrStr() << endl;
    
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
    cerr << "ERROR: " << method_name << endl;
    cerr << "Input field projections don't match" << endl;
    cerr << "Field 0 projection:" << endl;
    proj0.print(cerr);
    cerr << "Field 1 projection:" << endl;
    proj1.print(cerr);
    cerr << "Field 2 projection:" << endl;
    proj2.print(cerr);
    
    return false;
  }
  
  if (proj0.getProjType() != Mdvx::PROJ_POLAR_RADAR)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Input file contains " << Mdvx::projType2Str(proj0.getProjType())
	 << " projection data" << endl;
    cerr << "The projection must be polar radar for this algorithm" << endl;
    
    return false;
  }
  
  // Calculate I and Q if they are not taken directly from the input file

  if (!_params->raw_iq_in_input)
  {
    if (!_params->snr_in_input)
      _calcSnr(*(mdvx.getField(2)));
    
    _calcIQ(*(mdvx.getField(0)),
	    *(mdvx.getField(1)),
	    *(mdvx.getField(2)));
  }
  
  return true;
}
