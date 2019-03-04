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
/*********************************************************************
 * MdvArithmetic: MdvArithmetic program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2005
 *
 * Dan Megenhardt
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsFcstTimeListTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvArithmetic.hh"
#include "Params.hh"


// Global variables

MdvArithmetic *MdvArithmetic::_instance =
     (MdvArithmetic *)NULL;


/*********************************************************************
 * Constructor
 */

MdvArithmetic::MdvArithmetic(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvArithmetic::MdvArithmetic()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvArithmetic *)NULL);
  
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
  char *params_path = (char *)"unknown";
  
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

MdvArithmetic::~MdvArithmetic()
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

MdvArithmetic *MdvArithmetic::Inst(int argc, char **argv)
{
  if (_instance == (MdvArithmetic *)NULL)
    new MdvArithmetic(argc, argv);
  
  return(_instance);
}

MdvArithmetic *MdvArithmetic::Inst()
{
  assert(_instance != (MdvArithmetic *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvArithmetic::init()
{
  static const string method_name = "MdvArithmetic::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
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
  
  case Params::TIME_LIST :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->time_list_trigger.start_time);
    time_t end_time
      = DateTime::parseDateTime(_params->time_list_trigger.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for TIME_LIST trigger: " <<
	_params->time_list_trigger.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for TIME_LIST trigger: " <<
	_params->time_list_trigger.end_time << endl;
      
      return false;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->input_url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }

  case Params::FCST_TIME_LIST :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->time_list_trigger.start_time);
    time_t end_time
      = DateTime::parseDateTime(_params->time_list_trigger.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for TIME_LIST trigger: " <<
	_params->time_list_trigger.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for TIME_LIST trigger: " <<
	_params->time_list_trigger.end_time << endl;
      
      return false;
    }
    
    DsFcstTimeListTrigger *trigger = new DsFcstTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->input_url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      
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

void MdvArithmetic::run()
{
  static const string method_name = "MdvArithmetic::run()";
  
  TriggerInfo trigger_info;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_info))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: "
	   << trigger_info.getIssueTime() << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processData() - Process the data for the given time.
 */

bool MdvArithmetic::_processData(const TriggerInfo &trigger_info)
{
  static const string method_name = "MdvArithmetic::_processData()";
  
  if (_params->debug)
  {
    cerr << endl << "*** Processing data for time: "
	 << DateTime::str(trigger_info.getIssueTime());
    if (trigger_info.getForecastTime() != DateTime::NEVER)
      cerr << ", fcst time = " << DateTime::str(trigger_info.getForecastTime());
    cerr << endl;
  }
  
  // Read in the input file

  DsMdvx input_mdv;
  
  cerr << "****_readInput **** " << endl;
  cerr << "trigger_info.getIssueTime() =    " << DateTime::str(trigger_info.getIssueTime()) << endl;
  cerr << "trigger_info.getForecastTime() = " << DateTime::str(trigger_info.getForecastTime()) << endl;

  if (!_readInput(_params->input_url,
		  trigger_info.getIssueTime(),
		  trigger_info.getForecastTime(),
		  600,
		  input_mdv))
    return false;

  // Remap the data if requested, and not done on read

  if (_params->auto_remap_to_latlon) {
    _autoRemapToLatLon(input_mdv);
  } else if (_params->remap_xy && !_params->remap_at_source) {
    _remap(input_mdv);
  }

  // Get a pointer to the field.

  MdvxField *input_field = input_mdv.getField(0);
  
  if (input_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting pointer to  field _params->field_num in MDV volume for time: " <<
      DateTime::str(trigger_info.getForecastTime()) << endl;
    
    return false;
  }
  
  Mdvx::field_header_t input_field_hdr = input_field->getFieldHeader();
  
  // Create the new field

  Mdvx::field_header_t new_field_hdr;
  
  memset(&new_field_hdr, 0, sizeof(new_field_hdr));
  
  new_field_hdr.field_code = 1;
  new_field_hdr.forecast_delta = input_field_hdr.forecast_delta;
  new_field_hdr.forecast_time = input_field_hdr.forecast_time;
  new_field_hdr.nx = input_field_hdr.nx;
  new_field_hdr.ny = input_field_hdr.ny;
  new_field_hdr.nz = input_field_hdr.nz;
  new_field_hdr.proj_type = input_field_hdr.proj_type;
  new_field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  new_field_hdr.data_element_nbytes = 4;
  new_field_hdr.volume_size =
    new_field_hdr.nx * new_field_hdr.ny * new_field_hdr.nz *
    new_field_hdr.data_element_nbytes;
  new_field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  new_field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  new_field_hdr.scaling_type = Mdvx::SCALING_NONE;
//  new_field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_NEW;
//  new_field_hdr.vlevel_type = Mdvx::VERT_TYPE_NEW;
  new_field_hdr.dz_constant = input_field_hdr.dz_constant;
  new_field_hdr.proj_origin_lat = input_field_hdr.proj_origin_lat;
  new_field_hdr.proj_origin_lon = input_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    new_field_hdr.proj_param[i] = input_field_hdr.proj_param[i];
  new_field_hdr.vert_reference = input_field_hdr.vert_reference;
  new_field_hdr.grid_dx = input_field_hdr.grid_dx;
  new_field_hdr.grid_dy = input_field_hdr.grid_dy;
  new_field_hdr.grid_dz = input_field_hdr.grid_dz;
  new_field_hdr.grid_minx = input_field_hdr.grid_minx;
  new_field_hdr.grid_miny = input_field_hdr.grid_miny;
  new_field_hdr.grid_minz = input_field_hdr.grid_minz;
  new_field_hdr.scale  = 1;
  new_field_hdr.bias = 0;

 if(_params->redefine_header_bad)
   {
     new_field_hdr.bad_data_value = _params->header_bad;
   }
 else
   {
     new_field_hdr.bad_data_value = input_field_hdr.bad_data_value;
   }

  if(_params->redefine_header_missing)
   {
     new_field_hdr.missing_data_value = _params->header_missing;
   }
 else
   {
     new_field_hdr.missing_data_value = input_field_hdr.missing_data_value;
   } 
  new_field_hdr.proj_rotation = input_field_hdr.proj_rotation;
  STRcopy(new_field_hdr.field_name_long, input_field_hdr.field_name_long, MDV_LONG_FIELD_LEN);
  STRcopy(new_field_hdr.field_name, input_field_hdr.field_name, MDV_SHORT_FIELD_LEN);
  STRcopy(new_field_hdr.units, input_field_hdr.units, MDV_UNITS_LEN);

  Mdvx::vlevel_header_t new_vlevel_hdr =
    input_field->getVlevelHeader();
  
  MdvxField *new_field = new MdvxField(new_field_hdr,
				       new_vlevel_hdr,
				       (void *)0,
				       false, false);
  
  if (new_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating new new field" << endl;
    
    return false;
  }
  
  // Loop through each plane

  fl32 *input_ptr = (fl32 *)input_field->getVol();
  fl32 *output_ptr = (fl32 *)new_field->getVol();
  int num_pts = input_field_hdr.nx * input_field_hdr.ny * input_field_hdr.nz;
  float min_output_value = _params->min_value;
  float max_output_value = _params->max_value;

  float missing_data_value;
  if(_params->fill_missing)
    missing_data_value = _params->new_missing_value;
  else
    missing_data_value = input_field_hdr.missing_data_value;
  
  float bad_data_value;
  if(_params->fill_bad)
      bad_data_value = _params->new_bad_value;
  else
      bad_data_value = input_field_hdr.bad_data_value;

  switch (_params->arithmetic_operator)
  {
    case Params::ADDITION :
    {
	for (int i = 0; i < num_pts; ++i) 
	{
	  if(input_ptr[i] == input_field_hdr.missing_data_value)
	    output_ptr[i] = missing_data_value;
	  else if (input_ptr[i] == input_field_hdr.bad_data_value)
	    output_ptr[i] = bad_data_value;
	  else if (input_ptr[i] > input_field_hdr.max_value)
	    output_ptr[i] = input_field_hdr.max_value + _params->apply_constant;
	  else
	    output_ptr[i] = input_ptr[i] + _params->apply_constant;
	  
	  if(_params->set_min_value && 
	     (input_ptr[i] != input_field_hdr.missing_data_value ||
	      input_ptr[i] != input_field_hdr.bad_data_value) &&
	     output_ptr[i] < min_output_value)
	  {
	    output_ptr[i] = min_output_value;
	  }
	  if(_params->set_min_output_value && 
	     (input_ptr[i] != input_field_hdr.missing_data_value ||
	      input_ptr[i] != input_field_hdr.bad_data_value) &&
	     output_ptr[i] < min_output_value &&
	     input_ptr[i] >= min_output_value)
	  {
	    output_ptr[i] = min_output_value;
	  }

	  if(_params->set_max_value && 
	     (input_ptr[i] != input_field_hdr.missing_data_value ||
	      input_ptr[i] != input_field_hdr.bad_data_value) &&
	     output_ptr[i] > max_output_value)
	  {
	    output_ptr[i] = max_output_value;
	  }
	  if(_params->set_max_output_value && 
	     (input_ptr[i] != input_field_hdr.missing_data_value ||
	      input_ptr[i] != input_field_hdr.bad_data_value) &&
	     output_ptr[i] > max_output_value &&
	     input_ptr[i] <= max_output_value)
	  {
	    output_ptr[i] = max_output_value;
	  }
	}
	break;
    }
    case Params::SUBTRACTION :
    {
	for (int i = 0; i < num_pts; ++i) 
	{
	  if(input_ptr[i] == input_field_hdr.missing_data_value)
	    output_ptr[i] = missing_data_value;
	  else if (input_ptr[i] == input_field_hdr.bad_data_value)
	    output_ptr[i] = bad_data_value;
	  else if (input_ptr[i] > input_field_hdr.max_value)
	    output_ptr[i] = input_field_hdr.max_value - _params->apply_constant;
	  else
	    output_ptr[i] = input_ptr[i] - _params->apply_constant;

	  if(_params->set_min_value && 
	     (input_ptr[i] != input_field_hdr.missing_data_value ||
	      input_ptr[i] != input_field_hdr.bad_data_value) &&
	     output_ptr[i] < min_output_value)
	  {
	    output_ptr[i] = min_output_value;
	  }
	  if(_params->set_min_output_value && 
	     (input_ptr[i] != input_field_hdr.missing_data_value ||
	      input_ptr[i] != input_field_hdr.bad_data_value) &&
	     output_ptr[i] < min_output_value &&
	     input_ptr[i] >= min_output_value)
	  {
	    output_ptr[i] = min_output_value;
	  }

	  if(_params->set_max_value && 
	     (input_ptr[i] != input_field_hdr.missing_data_value ||
	      input_ptr[i] != input_field_hdr.bad_data_value) &&
	     output_ptr[i] > max_output_value)
	  {
	    output_ptr[i] = max_output_value;
	  }
	  if(_params->set_max_output_value && 
	     (input_ptr[i] != input_field_hdr.missing_data_value ||
	      input_ptr[i] != input_field_hdr.bad_data_value) &&
	     output_ptr[i] > max_output_value &&
	     input_ptr[i] <= max_output_value)
	  {
	    output_ptr[i] = max_output_value;
	  }
	}
	break;
    }
    case Params::MULTIPLICATION :
    {
	for (int i = 0; i < num_pts; ++i) 
	{
	  if(input_ptr[i] == input_field_hdr.missing_data_value)
	    output_ptr[i] = missing_data_value;
	  else if (input_ptr[i] == input_field_hdr.bad_data_value)
	    output_ptr[i] = bad_data_value;
	  else if (input_ptr[i] > input_field_hdr.max_value)
	    output_ptr[i] = input_field_hdr.max_value * _params->apply_constant;
	  else 
	    output_ptr[i] = input_ptr[i] * _params->apply_constant;
	  
	  if(_params->set_min_value && 
	     (input_ptr[i] != input_field_hdr.missing_data_value ||
	      input_ptr[i] != input_field_hdr.bad_data_value) &&
	     output_ptr[i] < min_output_value)
	  {
	    output_ptr[i] = min_output_value;
	  }
	  if(_params->set_min_output_value && 
	     (input_ptr[i] != input_field_hdr.missing_data_value ||
	      input_ptr[i] != input_field_hdr.bad_data_value) &&
	     output_ptr[i] < min_output_value &&
	     input_ptr[i] >= min_output_value)
	  {
	    output_ptr[i] = min_output_value;
	  }

	  if(_params->set_max_value && 
	     (input_ptr[i] != input_field_hdr.missing_data_value ||
	      input_ptr[i] != input_field_hdr.bad_data_value) &&
	     output_ptr[i] > max_output_value)
	  {
	    output_ptr[i] = max_output_value;
	  }
	  if(_params->set_max_output_value && 
	     (input_ptr[i] != input_field_hdr.missing_data_value ||
	      input_ptr[i] != input_field_hdr.bad_data_value) &&
	     output_ptr[i] > max_output_value &&
	     input_ptr[i] <= max_output_value)
	  {
	    output_ptr[i] = max_output_value;
	  }
	}
 	for (int i = 0; i < num_pts; ++i) 
	  {
	    if (output_ptr[i] ==   -32768 )
	      cerr << " missing_data_value not reset!!" << endl;
	  }
	break;
    }
     case Params::SQUARE_ROOT :
    {
	for (int i = 0; i < num_pts; ++i) 
	{      
	  if(input_ptr[i] == input_field_hdr.missing_data_value)
	    output_ptr[i] = missing_data_value;
	  else if (input_ptr[i] == input_field_hdr.bad_data_value || input_ptr[i] < 0 )
	    output_ptr[i] = bad_data_value; 
	  else 
	    output_ptr[i] = sqrt(input_ptr[i]);
	  
	  if(_params->set_min_value &&
	     output_ptr[i] != missing_data_value &&
	     output_ptr[i] < min_output_value)
	    output_ptr[i] = min_output_value;
	  
	  if(_params->set_min_output_value &&
	     output_ptr[i] != missing_data_value &&
	     output_ptr[i] < min_output_value &&
	     input_ptr[i] >= min_output_value)
	    output_ptr[i] = min_output_value;
	  
	  if(_params->set_max_value &&
	     output_ptr[i] != missing_data_value &&
	     output_ptr[i] > max_output_value)
	    output_ptr[i] = max_output_value;

	  if(_params->set_max_output_value &&
	     output_ptr[i] != missing_data_value &&
	     output_ptr[i] > max_output_value &&
	     input_ptr[i] <= max_output_value)
	    output_ptr[i] = max_output_value;
	}
	
	break;
    }
    
     case Params::POWER :
    {
        for (int i = 0; i < num_pts; ++i)
        {
          if(input_ptr[i] == input_field_hdr.missing_data_value)
            output_ptr[i] = missing_data_value;
          else if (input_ptr[i] == input_field_hdr.bad_data_value || input_ptr[i] < 0 )
            output_ptr[i] = bad_data_value;
          else
            output_ptr[i] = pow(input_ptr[i], _params->apply_constant );

	  if(_params->set_min_value &&
	     output_ptr[i] != missing_data_value &&
	     output_ptr[i] < min_output_value)
	    output_ptr[i] = min_output_value;
	  
	  if(_params->set_min_output_value &&
	     output_ptr[i] != missing_data_value &&
	     output_ptr[i] < min_output_value &&
	     input_ptr[i] >= min_output_value)
	    output_ptr[i] = min_output_value;
	  
	  if(_params->set_max_value &&
	     output_ptr[i] != missing_data_value &&
	     output_ptr[i] > max_output_value)
	    output_ptr[i] = max_output_value;

	  if(_params->set_max_output_value &&
	     output_ptr[i] != missing_data_value &&
	     output_ptr[i] > max_output_value &&
	     input_ptr[i] <= max_output_value)
	    output_ptr[i] = max_output_value;

        }

        break;
    }

     
  }

  DsMdvx mdv;

  Mdvx::master_header_t mhdr = input_mdv.getMasterHeader();
  mdv.setMasterHeader(mhdr);


  // Compress the new new field and add it to the input file (which
  // is also the output file).

  // input_field->convertType(Mdvx::ENCODING_INT8,
  // 			   Mdvx::COMPRESSION_RLE,
  // 			   Mdvx::SCALING_DYNAMIC);

  //
  // Set encoding type.
  //
  Mdvx::encoding_type_t encoding = Mdvx::ENCODING_ASIS;
  switch (_params->outputEncodingType) {

  case Params::ENCODING_MDV_ASIS :
    encoding = Mdvx::ENCODING_ASIS;
    break;

  case Params::ENCODING_MDV_INT8 :
    encoding = Mdvx::ENCODING_INT8;
    break;

  case Params::ENCODING_MDV_INT16 :
    encoding = Mdvx::ENCODING_INT16;
    break;

  case Params::ENCODING_MDV_FLOAT32 :
    encoding = Mdvx::ENCODING_FLOAT32;
    break;

  default :
    cerr << "Unrecognized encoding type " 
	 << _params->outputEncodingType 
	 << " I cannot cope." << endl;
    exit(-1);
    break;

  }
  new_field->convertType(encoding,
			 Mdvx::COMPRESSION_GZIP,
			 Mdvx::SCALING_DYNAMIC);

  mdv.addField(new_field);
  
  // Finally, write the output file

  mdv.setWriteLdataInfo();
  if(_params->output_forecast_directory)
    mdv.setWriteAsForecast();

  if (mdv.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: " << _params->output_url <<endl;
    
    return false;
  }
  
  return true;
}

/*********************************************************************
 * _readInput() - Read the input data.
 *
 * Returns true on success, false on failure.  The input data is returned
 * in the input_mdvx parameter.
 */

bool MdvArithmetic::_readInput(const string &url,
			       const DateTime &data_time,
			       const DateTime &fcst_time,
			       const int max_input_secs,
			       DsMdvx &input_mdv) const
{
  static const string method_name = "MdvArithmetic::_readInput()";
  
  // Set up the read request

  if (fcst_time == DateTime::NEVER)
  {
    cerr << "READ_CLOSEST\n";
    input_mdv.setReadTime(Mdvx::READ_CLOSEST,
			   url,
			   max_input_secs,
			   data_time.utime());
  }
  else
  {
    cerr << "READ_SPECIFIED_FORECAST\n";
    cerr << "max_input_secs = " << max_input_secs << endl;
    cerr << "data_time.utime() = " << data_time.utime() << endl;
    cerr << "fcst_time.utime() - data_time.utime() = " 
	 << fcst_time.utime() - data_time.utime() << endl;
    cerr << "url = " << url << endl;
    
    input_mdv.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
			   url,
			   max_input_secs,
			   data_time.utime(),
			   fcst_time.utime() - data_time.utime());
  }
  

  if(_params->use_field_name)
      input_mdv.addReadField(_params->field_name);
  else
      input_mdv.addReadField(_params->field_num);

  input_mdv.setReadNoChunks();
  
  input_mdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_mdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_mdv.setReadScalingType(Mdvx::SCALING_NONE);
  
  input_mdv.setReadFieldFileHeaders();

  if (_params->remap_xy && _params->remap_at_source &&
      !_params->auto_remap_to_latlon) {

    if (_params->remap_projection == Params::PROJ_LATLON) {
      input_mdv.setReadRemapLatlon(_params->remap_grid.nx,
                                   _params->remap_grid.ny,
                                   _params->remap_grid.minx,
                                   _params->remap_grid.miny,
                                   _params->remap_grid.dx,
                                   _params->remap_grid.dy);
    } else if (_params->remap_projection == Params::PROJ_FLAT) {
      input_mdv.setReadRemapFlat(_params->remap_grid.nx,
                                 _params->remap_grid.ny,
                                 _params->remap_grid.minx,
                                 _params->remap_grid.miny,
                                 _params->remap_grid.dx,
                                 _params->remap_grid.dy,
                                 _params->remap_origin_lat,
                                 _params->remap_origin_lon,
                                 _params->remap_rotation);
    } else if (_params->remap_projection == Params::PROJ_LAMBERT_CONF) {
      input_mdv.setReadRemapLambertConf(_params->remap_grid.nx,
                                        _params->remap_grid.ny,
                                        _params->remap_grid.minx,
                                        _params->remap_grid.miny,
                                        _params->remap_grid.dx,
                                        _params->remap_grid.dy,
                                        _params->remap_origin_lat,
                                        _params->remap_origin_lon,
                                        _params->remap_lat1,
                                        _params->remap_lat2);
    } else if (_params->remap_projection == Params::PROJ_POLAR_STEREO) {
      Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
      if (!_params->remap_pole_is_north) {
        poleType = Mdvx::POLE_SOUTH;
      }
      input_mdv.setReadRemapPolarStereo(_params->remap_grid.nx,
                                        _params->remap_grid.ny,
                                        _params->remap_grid.minx,
                                        _params->remap_grid.miny,
                                        _params->remap_grid.dx,
                                        _params->remap_grid.dy,
                                        _params->remap_origin_lat,
                                        _params->remap_origin_lon,
                                        _params->remap_tangent_lon,
                                        poleType,
                                        _params->remap_central_scale);
    } else if (_params->remap_projection == Params::PROJ_OBLIQUE_STEREO) {
      input_mdv.setReadRemapObliqueStereo(_params->remap_grid.nx,
                                          _params->remap_grid.ny,
                                          _params->remap_grid.minx,
                                          _params->remap_grid.miny,
                                          _params->remap_grid.dx,
                                          _params->remap_grid.dy,
                                          _params->remap_origin_lat,
                                          _params->remap_origin_lon,
                                          _params->remap_tangent_lat,
                                          _params->remap_tangent_lon,
                                          _params->remap_central_scale);
    } else if (_params->remap_projection == Params::PROJ_MERCATOR) {
      input_mdv.setReadRemapMercator(_params->remap_grid.nx,
                                     _params->remap_grid.ny,
                                     _params->remap_grid.minx,
                                     _params->remap_grid.miny,
                                     _params->remap_grid.dx,
                                     _params->remap_grid.dy,
                                     _params->remap_origin_lat,
                                     _params->remap_origin_lon);
    } else if (_params->remap_projection == Params::PROJ_TRANS_MERCATOR) {
      input_mdv.setReadRemapTransverseMercator(_params->remap_grid.nx,
                                               _params->remap_grid.ny,
                                               _params->remap_grid.minx,
                                               _params->remap_grid.miny,
                                               _params->remap_grid.dx,
                                               _params->remap_grid.dy,
                                               _params->remap_origin_lat,
                                               _params->remap_origin_lon,
                                               _params->remap_central_scale);
    } else if (_params->remap_projection == Params::PROJ_ALBERS) {
      input_mdv.setReadRemapAlbers(_params->remap_grid.nx,
                                   _params->remap_grid.ny,
                                   _params->remap_grid.minx,
                                   _params->remap_grid.miny,
                                   _params->remap_grid.dx,
                                   _params->remap_grid.dy,
                                   _params->remap_origin_lat,
                                   _params->remap_origin_lon,
                                   _params->remap_lat1,
                                   _params->remap_lat2);
    } else if (_params->remap_projection == Params::PROJ_LAMBERT_AZIM) {
      input_mdv.setReadRemapLambertAzimuthal(_params->remap_grid.nx,
                                             _params->remap_grid.ny,
                                             _params->remap_grid.minx,
                                             _params->remap_grid.miny,
                                             _params->remap_grid.dx,
                                             _params->remap_grid.dy,
                                             _params->remap_origin_lat,
                                             _params->remap_origin_lon);
    } else if (_params->remap_projection == Params::PROJ_VERT_PERSP) {
      input_mdv.setReadRemapVertPersp(_params->remap_grid.nx,
                                      _params->remap_grid.ny,
                                      _params->remap_grid.minx,
                                      _params->remap_grid.miny,
                                      _params->remap_grid.dx,
                                      _params->remap_grid.dy,
                                      _params->remap_origin_lat,
                                      _params->remap_origin_lon,
                                      _params->remap_persp_radius);
    }
  }

    // Read the data

  if (input_mdv.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << url << endl;
    cerr << input_mdv.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}

////////////////////////////////////////////
// remap

void MdvArithmetic::_remap(DsMdvx &mdvx)

{

  for (int ifld = 0; ifld < mdvx.getNFields(); ifld++) {

    MdvxField *field = mdvx.getField(ifld);

    if (field == NULL) {
      cerr << "ERROR - MdvxConvert::_remap" << endl;
      cerr << "  Error remapping field #" << ifld <<
           " in output file" << endl;
      return;
    }

    if (_params->remap_projection == Params::PROJ_LATLON) {
      field->remap2Latlon(_remapLut,
                          _params->remap_grid.nx,
                          _params->remap_grid.ny,
                          _params->remap_grid.minx,
                          _params->remap_grid.miny,
                          _params->remap_grid.dx,
                          _params->remap_grid.dy);
    } else if (_params->remap_projection == Params::PROJ_FLAT) {
      field->remap2Flat(_remapLut,
                        _params->remap_grid.nx,
                        _params->remap_grid.ny,
                        _params->remap_grid.minx,
                        _params->remap_grid.miny,
                        _params->remap_grid.dx,
                        _params->remap_grid.dy,
                        _params->remap_origin_lat,
                        _params->remap_origin_lon,
                        _params->remap_rotation,
                        _params->remap_false_northing,
                        _params->remap_false_easting);
    } else if (_params->remap_projection == Params::PROJ_LAMBERT_CONF)	{
      field->remap2LambertConf(_remapLut,
                               _params->remap_grid.nx,
                               _params->remap_grid.ny,
                               _params->remap_grid.minx,
                               _params->remap_grid.miny,
                               _params->remap_grid.dx,
                               _params->remap_grid.dy,
                               _params->remap_origin_lat,
                               _params->remap_origin_lon,
                               _params->remap_lat1,
                               _params->remap_lat2,
                               _params->remap_false_northing,
                               _params->remap_false_easting);
    } else if (_params->remap_projection == Params::PROJ_POLAR_STEREO) {
      Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
      if (!_params->remap_pole_is_north) {
        poleType = Mdvx::POLE_SOUTH;
      }
      field->remap2PolarStereo(_remapLut,
                               _params->remap_grid.nx,
                               _params->remap_grid.ny,
                               _params->remap_grid.minx,
                               _params->remap_grid.miny,
                               _params->remap_grid.dx,
                               _params->remap_grid.dy,
                               _params->remap_origin_lat,
                               _params->remap_origin_lon,
                               _params->remap_tangent_lon,
                               poleType,
                               _params->remap_central_scale,
                               _params->remap_false_northing,
                               _params->remap_false_easting);
    } else if (_params->remap_projection == Params::PROJ_OBLIQUE_STEREO) {
      field->remap2ObliqueStereo(_remapLut,
                                 _params->remap_grid.nx,
                                 _params->remap_grid.ny,
                                 _params->remap_grid.minx,
                                 _params->remap_grid.miny,
                                 _params->remap_grid.dx,
                                 _params->remap_grid.dy,
                                 _params->remap_origin_lat,
                                 _params->remap_origin_lon,
                                 _params->remap_tangent_lat,
                                 _params->remap_tangent_lon,
                                 _params->remap_false_northing,
                                 _params->remap_false_easting);
    } else if (_params->remap_projection == Params::PROJ_MERCATOR) {
      field->remap2Mercator(_remapLut,
                            _params->remap_grid.nx,
                            _params->remap_grid.ny,
                            _params->remap_grid.minx,
                            _params->remap_grid.miny,
                            _params->remap_grid.dx,
                            _params->remap_grid.dy,
                            _params->remap_origin_lat,
                            _params->remap_origin_lon,
                            _params->remap_false_northing,
                            _params->remap_false_easting);
    } else if (_params->remap_projection == Params::PROJ_TRANS_MERCATOR) {
      field->remap2TransverseMercator(_remapLut,
                                      _params->remap_grid.nx,
                                      _params->remap_grid.ny,
                                      _params->remap_grid.minx,
                                      _params->remap_grid.miny,
                                      _params->remap_grid.dx,
                                      _params->remap_grid.dy,
                                      _params->remap_origin_lat,
                                      _params->remap_origin_lon,
                                      _params->remap_central_scale,
                                      _params->remap_false_northing,
                                      _params->remap_false_easting);
    } else if (_params->remap_projection == Params::PROJ_ALBERS) {
      field->remap2Albers(_remapLut,
                          _params->remap_grid.nx,
                          _params->remap_grid.ny,
                          _params->remap_grid.minx,
                          _params->remap_grid.miny,
                          _params->remap_grid.dx,
                          _params->remap_grid.dy,
                          _params->remap_origin_lat,
                          _params->remap_origin_lon,
                          _params->remap_lat1,
                          _params->remap_lat2,
                          _params->remap_false_northing,
                          _params->remap_false_easting);
    } else if (_params->remap_projection == Params::PROJ_LAMBERT_AZIM) {
      field->remap2LambertAzimuthal(_remapLut,
                                    _params->remap_grid.nx,
                                    _params->remap_grid.ny,
                                    _params->remap_grid.minx,
                                    _params->remap_grid.miny,
                                    _params->remap_grid.dx,
                                    _params->remap_grid.dy,
                                    _params->remap_origin_lat,
                                    _params->remap_origin_lon,
                                    _params->remap_false_northing,
                                    _params->remap_false_easting);
    } else if (_params->remap_projection == Params::PROJ_VERT_PERSP) {
      field->remap2VertPersp(_remapLut,
                             _params->remap_grid.nx,
                             _params->remap_grid.ny,
                             _params->remap_grid.minx,
                             _params->remap_grid.miny,
                             _params->remap_grid.dx,
                             _params->remap_grid.dy,
                             _params->remap_origin_lat,
                             _params->remap_origin_lon,
                             _params->remap_persp_radius,
                             _params->remap_false_northing,
                             _params->remap_false_easting);
    }
  }

}

////////////////////////////////////////////
// auto remap to latlon grid
//
// Automatically picks the grid resolution and extent
// from the existing data.

void MdvArithmetic::_autoRemapToLatLon(DsMdvx &mdvx)

{

  for (int ifld = 0; ifld < mdvx.getNFields(); ifld++) {

    MdvxField *field = mdvx.getField(ifld);

    if (field == NULL) {
      cerr << "ERROR - MdvxConvert::_autoRemapToLatLon" << endl;
      cerr << "  Error remapping field #" << ifld <<
           " in output file" << endl;
      return;
    }

    field->autoRemap2Latlon(_remapLut);
  }

}
