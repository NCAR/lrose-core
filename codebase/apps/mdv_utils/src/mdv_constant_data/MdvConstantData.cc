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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:22:15 $
//   $Id: MdvConstantData.cc,v 1.6 2016/03/04 02:22:15 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvConstantData: MdvConstantData program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2002
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvConstantData.hh"
#include "Params.hh"
using namespace std;


// Global variables

MdvConstantData *MdvConstantData::_instance =
     (MdvConstantData *)NULL;


/*********************************************************************
 * Constructor
 */

MdvConstantData::MdvConstantData(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvConstantData::MdvConstantData()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvConstantData *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *) "unknown";
  
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

MdvConstantData::~MdvConstantData()
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
 * Inst() - Retrieve the singleton instance of this class.
 */

MdvConstantData *MdvConstantData::Inst(int argc, char **argv)
{
  if (_instance == (MdvConstantData *)NULL)
    new MdvConstantData(argc, argv);
  
  return(_instance);
}

MdvConstantData *MdvConstantData::Inst()
{
  assert(_instance != (MdvConstantData *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvConstantData::init()
{
  static const string method_name = "MdvConstantData::init()";
  
  // Initialize the data trigger

  switch (_params->mode)
  {
  case Params::REALTIME :
  {
    if (_params->debug_flag)
      cerr << "Initializing REALTIME trigger using url: " <<
	_params->input_url << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->input_url << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::ARCHIVE :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->begtime);
    time_t end_time
      = DateTime::parseDateTime(_params->endtime);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for TIME_LIST trigger: " <<
	_params->begtime << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for TIME_LIST trigger: " <<
	_params->endtime << endl;
      
      return false;
    }
    
    if (_params->debug_flag)
    {
      cerr << "Initializing TIME_LIST trigger using url:" << endl;
      cerr << "   URL: " <<_params->input_url << endl;
      cerr << "   start time: " << DateTime::str(start_time) << endl;
      cerr << "   end time: " << DateTime::str(end_time) << endl;
    }

    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->input_url << endl;
      cerr << "    Start time: " << _params->begtime <<
	endl;
      cerr << "    End time: " << _params->endtime << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MdvConstantData::run()
{
  static const string method_name = "MdvConstantData::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time.utime()))
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
 * _createConstantField() - Create a constant valued field with the
 *                          same projection as the given field.
 *
 * Returns a pointer to the created field on success, 0 on failure.
 */

MdvxField *MdvConstantData::_createConstantField(const MdvxField &input_field) const
{
  // Copy the input field as the base for the constant field

  MdvxField *const_field = new MdvxField(input_field);
  
  // Update the field header values

  Mdvx::field_header_t field_hdr = const_field->getFieldHeader();
  
  if (_params->new_data_value == 0.0)
  {
    field_hdr.missing_data_value = 1.0;
    field_hdr.bad_data_value = 1.0;
  }
  else
  {
    field_hdr.missing_data_value = 0.0;
    field_hdr.bad_data_value = 0.0;
  }
  
  field_hdr.min_value = _params->new_data_value;
  field_hdr.max_value = _params->new_data_value;
  
  const_field->setFieldHeader(field_hdr);
  
  // Update the data values themselves

  fl32 *const_field_data = (fl32 *)const_field->getVol();
  
  for (int i = 0; i < field_hdr.nx * field_hdr.ny * field_hdr.nz; ++i)
    const_field_data[i] = _params->new_data_value;
  
  return const_field;
}


/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool MdvConstantData::_processData(const time_t trigger_time)
{
  static const string method_name = "MdvConstantData::_processData()";
  
  // Read in the input file

  DsMdvx input_file;
  
  input_file.setReadTime(Mdvx::READ_CLOSEST,
			 _params->input_url,
			 0, trigger_time);
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_params->debug_flag)
  {
    cerr << endl;
    input_file.printReadRequest(cerr);
  }
  
  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading MDV volume for time: " <<
      DateTime::str(trigger_time) << endl;
    
    return false;
  }
  
  // Create the output file

  DsMdvx output_file;
  
  _updateOutputMasterHeader(output_file,
			    input_file.getMasterHeader(),
			    input_file.getPathInUse());
  
  // Create a constant field for each of the input fields

  for (int field_num = 0; field_num < input_file.getMasterHeader().n_fields;
       ++field_num)
  {
    // Retrieve the field information from the input file

    MdvxField *input_field;
    
    if ((input_field = input_file.getField(field_num)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting field number " << field_num << 
	" from input file" << endl;
      cerr << "*** Skipping field ***" << endl;
      
      continue;
    }
    
    // Create the constant field

    MdvxField *const_field;
    
    if ((const_field = _createConstantField(*input_field)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating constant field for input field <" <<
	input_field->getFieldHeader().field_name << ">" << endl;
      cerr << "*** Skipping field ***" << endl;
      
      continue;
    }
    
    // Add the constant field to the output file

    switch (_params->output_type)
    {
    case Params::OUTPUT_INT8 :
      const_field->convertType(Mdvx::ENCODING_INT8,
			       Mdvx::COMPRESSION_NONE,
			       Mdvx::SCALING_DYNAMIC);
      break;
      
    case Params::OUTPUT_INT16 :
      const_field->convertType(Mdvx::ENCODING_INT16,
			       Mdvx::COMPRESSION_NONE,
			       Mdvx::SCALING_DYNAMIC);
      break;
      
    case Params::OUTPUT_FLOAT32 :
      const_field->convertType(Mdvx::ENCODING_FLOAT32,
			       Mdvx::COMPRESSION_NONE,
			       Mdvx::SCALING_NONE);
      break;
      
    case Params::OUTPUT_PLANE_RLE8 :
      const_field->convertType(Mdvx::ENCODING_INT8,
			       Mdvx::COMPRESSION_RLE,
			       Mdvx::SCALING_DYNAMIC);
      break;
    } /* endswitch - _params->output_type */
    
    output_file.addField(const_field);
    
  } /* endfor - field_num */
  
  // Write the output file

  output_file.setWriteLdataInfo();
  
  if (output_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing to output URL: " << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _updateOutputMasterHeader() - Update the master header in the
 *                               output file.
 */

void MdvConstantData::_updateOutputMasterHeader(DsMdvx &output_file,
						const Mdvx::master_header_t &input_master_hdr,
						const string &dataset_source)
{
  // Load the master header values

  Mdvx::master_header_t master_hdr = input_master_hdr;
  
  master_hdr.time_gen = time(0);
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  
  STRcopy(master_hdr.data_set_info, "mdv_constant_data output", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "mdv_constant_data", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, dataset_source.c_str(), MDV_NAME_LEN);
  
  // Set the master header in the output file

  output_file.setMasterHeader(master_hdr);
}

