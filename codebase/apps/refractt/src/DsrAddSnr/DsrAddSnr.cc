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
 * @file DsrAddSnr.cc
 *
 * @class DsrAddSnr
 *
 * DsrAddSnr is the top level application class.
 *  
 * @date 3/15/2010
 *
 */

#include <assert.h>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "DsrAddSnr.hh"
#include "Params.hh"

using namespace std;

// Global variables

DsrAddSnr *DsrAddSnr::_instance = (DsrAddSnr *)NULL;

const string DsrAddSnr::SNR_FIELD_NAME = "SNR";
const string DsrAddSnr::SNR_UNITS = "dB";
const float DsrAddSnr::MIN_SNR_VALUE = -20.0;
const float DsrAddSnr::MAX_SNR_VALUE = 100.0;
const double DsrAddSnr::MISSING_DATA_VALUE = -9999.0;


/*********************************************************************
 * Constructor
 */

DsrAddSnr::DsrAddSnr(int argc, char **argv)
{
  static const string method_name = "DsrAddSnr::DsrAddSnr()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (DsrAddSnr *)NULL);
  
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

DsrAddSnr::~DsrAddSnr()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst()
 */

DsrAddSnr *DsrAddSnr::Inst(int argc, char **argv)
{
  if (_instance == (DsrAddSnr *)NULL)
    new DsrAddSnr(argc, argv);
  
  return(_instance);
}

DsrAddSnr *DsrAddSnr::Inst()
{
  assert(_instance != (DsrAddSnr *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool DsrAddSnr::init()
{
  static const string method_name = "DsrAddSnr::init()";
  
  // Initialize the input queue

  if (_inputQueue.initReadBlocking(_params->input_fmq_url,
				   "DsrAddSnr",
				   _params->verbose,
				   DsFmq::END) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr <<  "Could not initialize input fmq: "
	 << _params->input_fmq_url << endl;
    cerr << _inputQueue.getErrStr() << endl;
    
    return false;
  }

  // Initialize the output queue

  if (_outputQueue.initCreate(_params->output_fmq_url,
			      "DsrAddSnr",
			      _params->verbose,
			      false,
			      _params->output_fmq_nslots,
			      _params->output_fmq_size) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not initialize output fmq: "
	 << _params->output_fmq_url << endl;
    cerr << _outputQueue.getErrStr() << endl;
    
    return false;
  }

  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void DsrAddSnr::run()
{
  static const string method_name = "DsrAddSnr::run()";
  
  // Process messages from the input queue as they appear.  Note that the
  // DsRadarMsg object must be declared outside of the loop because the object
  // maintains state between messages received.

  bool radar_params_found = false;
  bool field_params_found = false;
  vector< DsFieldParams* > input_field_params;

  DsRadarMsg msg;

  while (true)
  {
    // Wait for the next message in the input queue

    int contents;

    if (_inputQueue.getDsMsg(msg, &contents) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Could not get beam from input fmq" << endl;
      cerr << _inputQueue.getErrStr() << endl;
      
      umsleep(100);
      continue;
    }

    // If this message includes radar params, save a flag so we know we can
    // process data

    if (contents & DsRadarMsg::RADAR_PARAMS)
      radar_params_found = true;
    
    // If this message includes field params, add the SNR field information

    if (contents & DsRadarMsg::FIELD_PARAMS)
    {
      if (!_addSnrFieldParams(msg))
	return;
	
      if (_params->verbose)
      {
	const vector< DsFieldParams* > field_params = msg.getFieldParams();
	vector< DsFieldParams* >::const_iterator params;
	
	for (params = field_params.begin(); params != field_params.end();
	     ++params)
	{
	  (*params)->print(cerr);
	}
	
      }
      
      field_params_found = true;
    }
    
    // If this message includes a beam, add the SNR field to the beam

    if (contents & DsRadarMsg::RADAR_BEAM &&
	field_params_found && radar_params_found)
    {
      if (!_addSnrField(msg))
	return;
    }
    
    // Write the message to the output queue

    if (_outputQueue.putDsMsg(msg, contents) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing message to output fmq: "
	   << _params->output_fmq_url << endl;
      cerr << _outputQueue.getErrStr() << endl;
      
      continue;
    }
    
  } /* endwhile - true */
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addSnrFieldParams()
 */

bool DsrAddSnr::_addSnrFieldParams(DsRadarMsg &msg)
{
  static const string method_name = "DsrAddSnr::_addSnrFieldParams()";
  
  // First make sure that the input includes the power field.  If it
  // doesn't, we can't calculate SNR so don't want to add the field parameter

  const vector< DsFieldParams* > field_params = msg.getFieldParams();

  _dmFieldIndex = -1;
  
  for (size_t i = 0; i < field_params.size(); ++i)
  {
    if (field_params[i]->name == (string)_params->dm_field_name)
    {
      _dmFieldIndex = i;
      
      break;
    }
    
  } /* endfor - i */
  
  if (_dmFieldIndex < 0)
    return true;
  
  // Calculate the SNR field values

  switch (field_params[_dmFieldIndex]->byteWidth)
  {
  case 1 :
  {
    _snrNumBytes = 1;
    _snrScale =
      (MAX_SNR_VALUE - MIN_SNR_VALUE) / (float)(UCHAR_MAX - 1);
    _snrBias = MIN_SNR_VALUE;
    _snrScaledMissingValue = UCHAR_MAX;
    
    break;
  }
  
  case 2 :
  {
    _snrNumBytes = 2;
    _snrScale =
      (MAX_SNR_VALUE - MIN_SNR_VALUE) / (float)(USHRT_MAX - 1);
    _snrBias = MIN_SNR_VALUE;
    _snrScaledMissingValue = USHRT_MAX;
    
    break;
  }
  
  case 4 :
  {
    _snrNumBytes = 4;
    _snrScale = 1.0;
    _snrBias = 0.0;
    _snrScaledMissingValue = MISSING_DATA_VALUE;
    
    break;
  }
  
  default:
    cerr << "ERROR: " << method_name << endl;
    cerr << "Invalid byte width: "
	 << field_params[_dmFieldIndex]->byteWidth << endl;
    
    return false;
  } /* endswitch - byteWidth */
  
  // Create the SNR field params and add it to the message
  
  DsFieldParams snr_field_params(SNR_FIELD_NAME.c_str(),
				 SNR_UNITS.c_str(),
				 _snrScale, _snrBias, _snrNumBytes,
				 _snrScaledMissingValue);
  
  msg.addFieldParams(snr_field_params);

  // Update the number of fields in the radar params

  DsRadarParams radar_params = msg.getRadarParams();
  radar_params.numFields++;
  msg.setRadarParams(radar_params);
  
  return true;
}


/*********************************************************************
 * _addSnrField()
 */

bool DsrAddSnr::_addSnrField(DsRadarMsg &msg) const
{
  static const string method_name = "DsrAddSnr::_addSnrField()";
  
  // Make sure there is a power field in the input.  If there isn't a
  // power field, don't do anything.

  if (_dmFieldIndex < 0)
    return true;
  
  // Get the parameters and beam information from the message

  const DsRadarParams radar_params = msg.getRadarParams();
  const vector< DsFieldParams* > field_params = msg.getFieldParams();
  DsRadarBeam beam_info = msg.getRadarBeam();
  
  int out_num_fields = radar_params.getNumFields();
  int in_num_fields = out_num_fields - 1;
  
  // Put the power field into an array

  int num_gates = radar_params.getNumGates();
  DsFieldParams *dm_field_params = field_params[_dmFieldIndex];
  
  float *dm_data = new float[num_gates];
  
  switch (dm_field_params->byteWidth)
  {
  case 1 :
  {
    ui08 *beam_ptr = (ui08 *)beam_info.getData() + _dmFieldIndex;
    
    for (int gate = 0; gate < num_gates; ++gate)
    {
      if (*beam_ptr == dm_field_params->missingDataValue)
	dm_data[gate] = MISSING_DATA_VALUE;
      else
	dm_data[gate] =
	  ((float)(*beam_ptr) * dm_field_params->scale) + dm_field_params->bias;
      
      beam_ptr += in_num_fields;
    } /* endfor - gate */
    
    break;
  }

  case 2 :
  {
    break;
  }
  
  case 4 :
  {
    break;
  }
  
  default:
    cerr << "ERROR: " << method_name << endl;
    cerr << "Invalid byte width in input queue: "
	 << dm_field_params->byteWidth << endl;
      
    return false;
  } /* endswitch - dm_field_params->byteWidth */
    
  // Calculate the SNR field

  float *snr_data = new float[num_gates];
  
  _calcSnr(dm_data, snr_data, num_gates);
  
  delete [] dm_data;
  
  // Add the SNR data to the beam data

  switch (beam_info.byteWidth)
  {
  case 1 :
  {
    ui08 *old_beam_ptr = (ui08 *)beam_info.getData();

    ui08 *new_beam_data = new ui08[(field_params.size() + 1) * num_gates];
    ui08 *new_beam_ptr = new_beam_data;
    
    for (int gate = 0; gate < num_gates; ++gate)
    {
      memcpy(new_beam_ptr, old_beam_ptr, in_num_fields * sizeof(ui08));
      
      new_beam_ptr += in_num_fields;
      old_beam_ptr += in_num_fields;
      
      if (snr_data[gate] == MISSING_DATA_VALUE)
	*new_beam_ptr = _snrScaledMissingValue;
      else
	*new_beam_ptr = (snr_data[gate] - _snrBias) / _snrScale;
      
      ++new_beam_ptr;
      
    } /* endfor - gate */
    
    beam_info.loadData(new_beam_data,
		       out_num_fields * num_gates * sizeof(ui08));
    
    break;
  }
  
  case 2 :
  {
    break;
  }
  
  case 4 :
  {
    break;
  }
  
  default:
    cerr << "ERROR: " << method_name << endl;
    cerr << "Invalid byte width in input queue: "
	 << dm_field_params->byteWidth << endl;
      
    return false;
  } /* endswitch - beam_info.byteWidth */

  msg.setRadarBeam(beam_info);
  
  return true;
}


/*********************************************************************
 * _calcSnr()
 */

void DsrAddSnr::_calcSnr(const float *dm_data, float *snr_data,
			 const int num_gates) const
{
  static const string method_name = "DsrAddSnr::_calcSnr()";

  // Calculate SNR

  double noise_mw = pow(10.0, _params->DM_noise_dbm / 10.0);

  for (int i = 0; i < num_gates; ++i)
  {
    if (dm_data[i] == MISSING_DATA_VALUE)
    {
      snr_data[i] = MISSING_DATA_VALUE;
      continue;
    }
    
    double power_mw = pow(10.0, dm_data[i] / 10.0);
    double power_minus_noise_mw = power_mw - noise_mw;
    double snr_linear = power_minus_noise_mw / noise_mw;
    if (snr_linear < 0.0)
      snr_data[i] = -20.0;
    else
      snr_data[i] = 10.0 * log10(snr_linear);
  } /* endfor - i */
}


///*********************************************************************
// * _createBlankSnrField()
// */
//
//MdvxField *DsrAddSnr::_createBlankSnrField(const Mdvx::field_header_t base_field_hdr,
//					   const Mdvx::vlevel_header_t base_vlevel_hdr,
//					   const string &field_name) const
//{
//  static const string method_name = "DsrAddSnr::_createBlankSnrField()";
//  
//  // Fill in the field header
//
//  Mdvx::field_header_t field_hdr = base_field_hdr;
//  field_hdr.scale = 1.0;
//  field_hdr.bias = 0.0;
//  field_hdr.bad_data_value = MISSING_DATA_VALUE;
//  field_hdr.missing_data_value = MISSING_DATA_VALUE;
//  STRcopy(field_hdr.field_name_long, field_name.c_str(), MDV_LONG_FIELD_LEN);
//  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
//  STRcopy(field_hdr.units, "dB", MDV_UNITS_LEN);
//
//  // Create the field
//
//  return new MdvxField(field_hdr, base_vlevel_hdr, (void *)0, true, false);
//}
//
//
///*********************************************************************
// * _initTrigger()
// */
//
//bool DsrAddSnr::_initTrigger(void)
//{
//  static const string method_name = "DsrAddSnr::_initTrigger()";
//  
//  switch (_params->trigger_mode)
//  {
//  case Params::LATEST_DATA :
//  {
//    if (_params->debug)
//    {
//      cerr << "Initializing LATEST_DATA trigger: " << endl;
//      cerr << "    URL = " << _params->input_url << endl;
//    }
//    
//    DsLdataTrigger *trigger = new DsLdataTrigger();
//    if (trigger->init(_params->input_url,
//		      _params->max_valid_secs,
//		      PMU_auto_register) != 0)
//    {
//      cerr << "ERROR: " << method_name << endl;
//      cerr << "Error initializing LATEST_DATA trigger: " << endl;
//      cerr << "    URL = " << _params->input_url << endl;
//      cerr << trigger->getErrStr() << endl;
//      
//      return false;
//    }
//
//    _dataTrigger = trigger;
//    
//    break;
//  }
//  
//  case Params::TIME_LIST :
//  {
//    DateTime start_time = _args->getStartTime();
//    DateTime end_time = _args->getEndTime();
//    
//    if (start_time == DateTime::NEVER ||
//	end_time == DateTime::NEVER)
//    {
//      cerr << "ERROR: " << method_name << endl;
//      cerr << "Must specify start and end times on command line" << endl;
//      
//      return false;
//    }
//    
//    if (_params->debug)
//    {
//      cerr << "Initializing TIME_LIST trigger: " << endl;
//      cerr << "   URL: " << _params->input_url << endl;
//      cerr << "   start time: " << start_time << endl;
//      cerr << "   end time: " << end_time << endl;
//    }
//    
//    DsTimeListTrigger *trigger = new DsTimeListTrigger();
//    if (trigger->init(_params->input_url,
//		      start_time.utime(), end_time.utime()) != 0)
//    {
//      cerr << "ERROR: " << method_name << endl;
//      cerr << "Error initializing TIME_LIST trigger:" << endl;
//      cerr << "    URL: " << _params->input_url << endl;
//      cerr << "    Start time: " << start_time << endl;
//      cerr << "    End time: " << end_time << endl;
//      cerr << trigger->getErrStr() << endl;
//      
//      return false;
//    }
//    
//    _dataTrigger = trigger;
//    
//    break;
//  }
//  
//  } /* endswitch - _params->trigger_mode */
//
//  return true;
//}
//
//
///*********************************************************************
// * _processData()
// */
//
//bool DsrAddSnr::_processData(const DateTime &trigger_time)
//{
//  static const string method_name = "DsrAddSnr::_processData()";
//  
//  if (_params->debug)
//    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
//  
//  // Read the input file
//
//  DsMdvx mdvx;
//  
//  if (!_readInputFile(mdvx, trigger_time))
//    return false;
//  
//  // Write the output file
//
//  _updateMasterHeader(mdvx);
//  
//  if (mdvx.writeToDir(_params->output_url) != 0)
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "Error writing output file to URL: "
//	 << _params->output_url << endl;
//    cerr << mdvx.getErrStr() << endl;
//    
//    return false;
//  }
//  
//  return true;
//}
//
//
///*********************************************************************
// * _readInputFile()
// */
//
//bool DsrAddSnr::_readInputFile(DsMdvx & mdvx,
//				const DateTime &data_time) const
//{
//  static const string method_name = "DsrAddSnr::_readInputField()";
//  
//  // Set up the read request
//
//  mdvx.clearRead();
//  
//  mdvx.setReadTime(Mdvx::READ_CLOSEST,
//		   _params->input_url,
//		   _params->max_valid_secs, data_time.utime());
//  
//  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
//  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
//  mdvx.setReadScalingType(Mdvx::SCALING_NONE);
//  
//  if (_params->verbose)
//    mdvx.printReadRequest(cerr);
//  
//  // Read the file
//
//  if (mdvx.readVolume() != 0)
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "Error reading input file:" << endl;
//    cerr << "   URL = " << _params->input_url << endl;
//    cerr << "   time = " << data_time << endl;
//    cerr << mdvx.getErrStr() << endl;
//    
//    return false;
//  }
//  
//  // Add the SNR field
//
//  if (!_addSnrField(mdvx))
//    return false;
//  
//  // Make sure the file has the needed fields.  Also, scale the NIQ data
//  // values so they are ready for the later calculations.
//
//  MdvxField *field;
//  
//  if ((field = mdvx.getField(_params->niq_field_name)) == 0)
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "No " << _params->niq_field_name << " field in input file" << endl;
//    
//    return false;
//  }
//  
//  // Scale the NIQ values
//
//  Mdvx::field_header_t niq_field_hdr = field->getFieldHeader();
//  fl32 *niq_data = (fl32 *)field->getVol();
//  
//  for (int i = 0; i < niq_field_hdr.nx * niq_field_hdr.ny * niq_field_hdr.nz;
//       ++i)
//  {
//    if (niq_data[i] == niq_field_hdr.bad_data_value ||
//	niq_data[i] == niq_field_hdr.missing_data_value)
//      continue;
//    
//    niq_data[i] *= _params->input_niq_scale;
//  } /* endfor - i */
//  
//  if ((field = mdvx.getField(_params->aiq_field_name)) == 0)
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "No " << _params->aiq_field_name << " field in input file" << endl;
//    
//    return false;
//  }
//  
//  if ((field = mdvx.getField(_params->dm_field_name)) == 0)
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "No " << _params->dm_field_name << " field in input file" << endl;
//    
//    return false;
//  }
//  
//  return true;
//}
//
//
///*********************************************************************
// * _updateMasterHeader()
// */
//
//void DsrAddSnr::_updateMasterHeader(DsMdvx &mdvx) const
//{
//  Mdvx::master_header_t master_hdr = mdvx.getMasterHeader();
//  
//  STRcopy(master_hdr.data_set_info, "DsrAddSnr", MDV_INFO_LEN);
//  STRcopy(master_hdr.data_set_name, "DsrAddSnr", MDV_NAME_LEN);
//  STRcopy(master_hdr.data_set_source, _params->input_url,
//	  MDV_NAME_LEN);
//  
//  mdvx.setMasterHeader(master_hdr);
//}
