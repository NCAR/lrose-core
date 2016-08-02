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
 * @file MdvAddSnr.cc
 *
 * @class MdvAddSnr
 *
 * MdvAddSnr is the top level application class.
 *  
 * @date 3/11/2010
 *
 */

#include <assert.h>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvAddSnr.hh"
#include "Params.hh"

using namespace std;

// Global variables

MdvAddSnr *MdvAddSnr::_instance = (MdvAddSnr *)NULL;

const double MdvAddSnr::MISSING_DATA_VALUE = -9999.0;
const double MdvAddSnr::VERY_LARGE = 2147483647.0;


/*********************************************************************
 * Constructor
 */

MdvAddSnr::MdvAddSnr(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvAddSnr::MdvAddSnr()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvAddSnr *)NULL);
  
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

MdvAddSnr::~MdvAddSnr()
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

MdvAddSnr *MdvAddSnr::Inst(int argc, char **argv)
{
  if (_instance == (MdvAddSnr *)NULL)
    new MdvAddSnr(argc, argv);
  
  return(_instance);
}

MdvAddSnr *MdvAddSnr::Inst()
{
  assert(_instance != (MdvAddSnr *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool MdvAddSnr::init()
{
  static const string method_name = "MdvAddSnr::init()";
  
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

void MdvAddSnr::run()
{
  static const string method_name = "MdvAddSnr::run()";
  
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
 * _addSnrField()
 */

bool MdvAddSnr::_addSnrField(DsMdvx &mdvx) const
{
  static const string method_name = "MdvAddSnr::_addSnrField()";
  
  // Get pointers to the DM field information.  We know this field exist
  // because we checked for it when we read the file in.

  MdvxField *dm_field = mdvx.getField(_params->dm_field_name);
  Mdvx::field_header_t dm_field_hdr = dm_field->getFieldHeader();
  Mdvx::vlevel_header_t dm_vlevel_hdr = dm_field->getVlevelHeader();
  
  // Create the SNR field and add it to the file

  MdvxField *snr_field;
  
  if ((snr_field = _createBlankSnrField(dm_field_hdr, dm_vlevel_hdr,
					"SNR")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating blank SNR field" << endl;
    
    return false;
  }
  
  if (!_calcSnr(*dm_field, *snr_field))
    return false;
  
  mdvx.addField(snr_field);
  
  return true;
}


/*********************************************************************
 * _calcSnr()
 */

bool MdvAddSnr::_calcSnr(const MdvxField &dm_field,
			 MdvxField &snr_field) const
{
  static const string method_name = "MdvAddSnr::_calcSnr()";
  
  // Get pointers to the fields

  Mdvx::field_header_t dm_field_hdr = dm_field.getFieldHeader();
  fl32 *dm_data = (fl32 *)dm_field.getVol();
  
  // Mdvx::field_header_t snr_field_hdr = snr_field.getFieldHeader();
  fl32 *snr_data = (fl32 *)snr_field.getVol();
  
  // Calculate SNR

  int volume_size = dm_field_hdr.nx * dm_field_hdr.ny * dm_field_hdr.nz;
  double noise_mw = pow(10.0, _params->DM_noise_dbm / 10.0);

  for (int i = 0; i < volume_size; ++i)
  {
    if (dm_data[i] == dm_field_hdr.bad_data_value ||
	dm_data[i] == dm_field_hdr.missing_data_value)
      continue;
    
    double power_mw = pow(10.0, dm_data[i] / 10.0);
    double power_minus_noise_mw = power_mw - noise_mw;
    double snr_linear = power_minus_noise_mw / noise_mw;
    if (snr_linear < 0.0)
      snr_data[i] = -20.0;
    else
      snr_data[i] = 10.0 * log10(snr_linear);
  } /* endfor - i */

  return true;
}


/*********************************************************************
 * _createBlankSnrField()
 */

MdvxField *MdvAddSnr::_createBlankSnrField(const Mdvx::field_header_t base_field_hdr,
					   const Mdvx::vlevel_header_t base_vlevel_hdr,
					   const string &field_name) const
{
  static const string method_name = "MdvAddSnr::_createBlankSnrField()";
  
  // Fill in the field header

  Mdvx::field_header_t field_hdr = base_field_hdr;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = MISSING_DATA_VALUE;
  field_hdr.missing_data_value = MISSING_DATA_VALUE;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  STRcopy(field_hdr.field_name_long, field_name.c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "dB", MDV_UNITS_LEN);

  // Create the field

  return new MdvxField(field_hdr, base_vlevel_hdr, (void *)0, true, false);
}


/*********************************************************************
 * _initTrigger()
 */

bool MdvAddSnr::_initTrigger(void)
{
  static const string method_name = "MdvAddSnr::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
    {
      cerr << "Initializing LATEST_DATA trigger: " << endl;
      cerr << "    URL = " << _params->input_url << endl;
    }
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
		      _params->max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger: " << endl;
      cerr << "    URL = " << _params->input_url << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
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
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger:" << endl;
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

bool MdvAddSnr::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvAddSnr::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read the input file

  DsMdvx mdvx;
  
  if (!_readInputFile(mdvx, trigger_time))
    return false;
  
  // Add the SNR field

  if (!_addSnrField(mdvx))
    return false;
  
  // Compress the fields and write the output file

  for (int i = 0; i < mdvx.getNFields(); ++i)
  {
    MdvxField *field = mdvx.getField(i);
    field->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_BZIP,
		       Mdvx::SCALING_DYNAMIC);
  }
  
  _updateMasterHeader(mdvx);
  
  if (mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: "
	 << _params->output_url << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readInputFile()
 */

bool MdvAddSnr::_readInputFile(DsMdvx & mdvx,
				const DateTime &data_time) const
{
  static const string method_name = "MdvAddSnr::_readInputField()";
  
  // Set up the read request

  mdvx.clearRead();
  
  mdvx.setReadTime(Mdvx::READ_CLOSEST,
		   _params->input_url,
		   _params->max_valid_secs, data_time.utime());
  
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_params->verbose)
    mdvx.printReadRequest(cerr);
  
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
  
  // Make sure the file has the needed fields.  Also, scale the NIQ data
  // values so they are ready for the later calculations.

  MdvxField *field;
  
  if ((field = mdvx.getField(_params->dm_field_name)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No " << _params->dm_field_name << " field in input file" << endl;
    
    return false;
  }

  return true;
}


/*********************************************************************
 * _updateMasterHeader()
 */

void MdvAddSnr::_updateMasterHeader(DsMdvx &mdvx) const
{
  Mdvx::master_header_t master_hdr = mdvx.getMasterHeader();
  
  STRcopy(master_hdr.data_set_info, "MdvAddSnr", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "MdvAddSnr", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, _params->input_url,
	  MDV_NAME_LEN);
  
  mdvx.setMasterHeader(master_hdr);
}
