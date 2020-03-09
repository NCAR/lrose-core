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
 * CombineLtgRadar.cc: combine_ltg_radar program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <signal.h>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "CombineLtgRadar.hh"

// Global variables

CombineLtgRadar *CombineLtgRadar::_instance = (CombineLtgRadar *)NULL;

// Global constants

const int FOREVER = true;

/*********************************************************************
 * Constructor
 */

CombineLtgRadar::CombineLtgRadar(int argc, char **argv)
{
  static const string method_name = "CombineLtgRadar::CombineLtgRadar()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (CombineLtgRadar *)NULL);
  
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
  
  if (!_args->okay)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with command line arguments." << endl;
    
    okay = false;
    
    return;
  }
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *) "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file <" << params_path <<
      ">" << endl;
    
    okay = false;
    
    return;
  }

}


/*********************************************************************
 * Destructor
 */

CombineLtgRadar::~CombineLtgRadar()
{
  // unregister process

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

CombineLtgRadar *CombineLtgRadar::Inst(int argc, char **argv)
{
  if (_instance == (CombineLtgRadar *)NULL)
    new CombineLtgRadar(argc, argv);
  
  return(_instance);
}

CombineLtgRadar *CombineLtgRadar::Inst()
{
  assert(_instance != (CombineLtgRadar *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool CombineLtgRadar::init()
{
  // Check for inconsistencies

  if (_params->mode != Params::REALTIME_MODE &&
      _params->data_time_stamp_flag == Params::USE_CURRENT_TIME)
  {
    cerr << "ERROR:  Cannot use USE_CURRENT_TIME time stamp in ARCHIVE_MODE." <<
      endl;
    
    return false;
  }
  
  // Create the input path objects

  switch (_params->mode)
  {
  case Params::ARCHIVE_MODE :
  {
    vector< string > file_list;
      
    for (int i = 0; i < _args->numInputFiles; ++i)
      file_list.push_back(_args->inputFileList[i]);
      
    DsFileListTrigger *data_trigger = new DsFileListTrigger();
    data_trigger->init(file_list);
      
    _dataTrigger = data_trigger;

    break;
  }

  case Params::REALTIME_MODE :
  {
    DsLdataTrigger *data_trigger = new DsLdataTrigger();

    if (_params->trigger == Params::RADAR)
      data_trigger->init(_params->radar_input_url,
			 _params->max_valid_file_age,
			 PMU_auto_register);
    else
      data_trigger->init(_params->ltg_input_url,
			 _params->max_valid_file_age,
			 PMU_auto_register);
    
    _dataTrigger = data_trigger;

    break;
  }

  case Params::TIME_LIST_MODE :
  {
    DsTimeListTrigger *data_trigger = new DsTimeListTrigger();
    
    if (_params->trigger == Params::RADAR)
    {
      data_trigger->init(_params->radar_input_url,
			 _args->startTime.utime(),
			 _args->endTime.utime());

      if (_params->debug_level >= Params::DEBUG_NORM)
      {
	cerr << "DsTimeListTrigger: url: " << _params->radar_input_url << endl;
	cerr << "                   startTime: " << _args->startTime << endl;
	cerr << "                   endTime: " << _args->endTime << endl;
      }
      
    }
    else
    {
      data_trigger->init(_params->ltg_input_url,
			 _args->startTime.utime(),
			 _args->endTime.utime());

      if (_params->debug_level >= Params::DEBUG_NORM)
      {
	cerr << "DsTimeListTrigger: url: " << _params->ltg_input_url << endl;
	cerr << "                   startTime: " << _args->startTime << endl;
	cerr << "                   endTime: " << _args->endTime << endl;
      }
      
    }
    
    _dataTrigger = data_trigger;
    
    break;
  }
  
  } /* endswitch - _params->mode */
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void CombineLtgRadar::run()
{
  static const string method_name = "CombineLtgRadar::run()";
  
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
      cerr << "Error processing data for time: " <<
	DateTime::str(trigger_time.utime()) << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/***************************************************************************
 * _getLtgDataValue() - Determines the lightning data value to use for
 *                      the given number of strikes.  Returns 0.0 if the
 *                      given number of strikes isn't found in the table.
 */

double CombineLtgRadar::_getLtgDataValue(const int num_strikes) const
{
  // Make sure the number of strikes is reasonable

  if (num_strikes < 0)
    return(0.0);
  
  // Look through the table for the appropriate entry

  for (int i = 0; i < _params->ltg_factors_n - 1; i++)
  {
    if (num_strikes >= _params->_ltg_factors[i].num_strikes &&
	num_strikes < _params->_ltg_factors[i+1].num_strikes)
      return(_params->_ltg_factors[i].ltg_value);
  }
  
  int last_entry = _params->ltg_factors_n - 1;
  
  if (num_strikes >= _params->_ltg_factors[last_entry].num_strikes)
    return(_params->_ltg_factors[last_entry].ltg_value);
  
  return(0.0);
  
}


/*********************************************************************
 * _processData() - Process the data for the given time.
 */

bool CombineLtgRadar::_processData(const time_t trigger_time) const
{
  static const string method_name = "CombineLtgRadar::_processData()";
  
  // Let the user know we are processing the fields

  PMU_auto_register("Processing fields");
    
  if (_params->debug_level >= Params::DEBUG_NORM)
  {
    cerr << endl;
    cerr << "Processing data for trigger time: " <<
      DateTime::str(trigger_time) << endl;
  }
  
  // Read in the radar data

  MdvxField *radar_field;
  Mdvx::master_header_t radar_master_hdr;
  
  if ((radar_field = _readField(trigger_time,
				_params->radar_input_url,
				_params->radar_field_name,
				_params->radar_field_pos,
				_params->time_offset_max,
				radar_master_hdr)) == NULL)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "  Radar URL: " << _params->radar_input_url << endl;
    if (strlen(_params->radar_field_name) == 0) {
      cerr << "Error reading radar field # "
	   << _params->radar_field_pos << endl;
    } else {
      cerr << "Error reading radar field "
	   << _params->radar_field_name << endl;
    }
    
    return false;
  }

  Mdvx::field_header_t radar_field_hdr = radar_field->getFieldHeader();
  MdvxProj radar_proj(radar_field_hdr);
  
  // Read in the lightning data

  MdvxField *ltg_field = NULL;
  Mdvx::master_header_t ltg_master_hdr;
  
  if ((ltg_field = _readField(trigger_time,
			      _params->ltg_input_url,
			      _params->ltg_field_name,
			      _params->ltg_field_pos,
			      _params->time_offset_max,
			      ltg_master_hdr,
			      &radar_proj)) == NULL)
  {

    cerr << "ERROR: " << method_name << endl;
    cerr << "  Ltg URL: " << _params->ltg_input_url << endl;
    if (strlen(_params->ltg_field_name) == 0) {
      cerr << "Error reading lightning field # "
	   << _params->ltg_field_pos << endl;
    } else {
      cerr << "Error reading lightning field "
	   << _params->ltg_field_name << endl;
    }

    // lightning data is missing

    if (!_params->process_if_no_lightning) {
      delete radar_field;
      return false;
    } else {
      ltg_field = NULL;
    }

  }
  
  
  // Make sure the field projections match.  We know that the horizontal
  // projections match because we force that, so we only have to check
  // the vertical projection values.

  if (ltg_field != NULL) {

    Mdvx::field_header_t ltg_field_hdr = ltg_field->getFieldHeader();
    MdvxPjg ltg_proj(ltg_field_hdr);
  
    if (radar_field_hdr.nz != ltg_field_hdr.nz ||
        (radar_field_hdr.nz > 1 &&
         (radar_field_hdr.grid_minz != ltg_field_hdr.grid_minz ||
          radar_field_hdr.grid_dz != ltg_field_hdr.grid_dz)))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Cannot process radar and lightning fields" << endl;
      cerr << "Field projections don't match" << endl;
      
      delete radar_field;
      delete ltg_field;
      
      return false;
    }
    
    if (radar_master_hdr.grid_orientation != ltg_master_hdr.grid_orientation ||
        radar_master_hdr.data_ordering != ltg_master_hdr.data_ordering)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Cannot process radar and lightning fields" << endl;
      cerr << "Data ordering doesn't match" << endl;
      
      delete radar_field;
      delete ltg_field;
      
      return false;
    }

  } // if (ltg_field != NULL)
    
  // Create the output file

  DsMdvx output_file;
  
  switch (_params->data_time_stamp_flag)
  {
  case Params::USE_RADAR_DATA_TIME :
  {
    _updateMasterHeader(output_file,
			radar_master_hdr.time_begin,
			radar_master_hdr.time_centroid,
			radar_master_hdr.time_end,
			radar_master_hdr.time_expire,
			radar_master_hdr);
    break;
  }
  
  case Params::USE_LTG_DATA_TIME :
  {
    if (ltg_field != NULL) {
      _updateMasterHeader(output_file,
                          ltg_master_hdr.time_begin,
                          ltg_master_hdr.time_centroid,
                          ltg_master_hdr.time_end,
                          ltg_master_hdr.time_expire,
                          ltg_master_hdr);
    } else {
      _updateMasterHeader(output_file,
                          radar_master_hdr.time_begin,
                          radar_master_hdr.time_centroid,
                          radar_master_hdr.time_end,
                          radar_master_hdr.time_expire,
                          radar_master_hdr);
    }
    break;
  }
  
  case Params::USE_CURRENT_TIME :
  {
    time_t current_time = time(0);
    
    if (ltg_field != NULL) {
      _updateMasterHeader(output_file,
                          current_time - _params->data_start_time_offset,
                          current_time,
                          current_time + _params->data_end_time_offset,
                          current_time + _params->data_end_time_offset,
                          ltg_master_hdr);
    } else {
      _updateMasterHeader(output_file,
                          current_time - _params->data_start_time_offset,
                          current_time,
                          current_time + _params->data_end_time_offset,
                          current_time + _params->data_end_time_offset,
                          radar_master_hdr);
    }
    break;
  }
  
  } /* endswitch - _params->data_time_stamp_flag */
  
  // Create the output field

  MdvxField *combined_field;
  Mdvx::master_header_t combined_master_hdr = output_file.getMasterHeader();

  if (ltg_field != NULL) {

    if ((combined_field = _combineFields(combined_master_hdr.time_centroid,
                                         *radar_field, *ltg_field)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating combined field" << endl;
      
      delete radar_field;
      delete ltg_field;
      
      return false;
    }
    
    delete radar_field;
    delete ltg_field;

  } else {

    if ((combined_field =
         _combineFromRadarOnly(combined_master_hdr.time_centroid,
                               *radar_field)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating field from radar only" << endl;
      
      delete radar_field;
      
      return false;
    }
    
    delete radar_field;

  } // if (ltg_field != NULL) {
  
  // Write the output file

  combined_field->convertType(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_RLE,
			      Mdvx::SCALING_DYNAMIC);
  
  output_file.addField(combined_field);
  
  output_file.setWriteLdataInfo();
  
  if (output_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output MDV file to URL: " <<
      _params->output_url << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _createCombinedField() - Create a blank for for storing the combined
 *                          data.
 *
 * Returns a pointer to the created field on success, 0 on failure.
 */

MdvxField *CombineLtgRadar::_createCombinedField(const time_t data_time,
						 const Mdvx::field_header_t &input_field_hdr,
						 const Mdvx::vlevel_header_t &input_vlevel_hdr) const
{
  // Create the field header

  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.forecast_delta = 0;
  field_hdr.forecast_time = data_time;
  field_hdr.nx = input_field_hdr.nx;
  field_hdr.ny = input_field_hdr.ny;
  field_hdr.nz = input_field_hdr.nz;
  field_hdr.proj_type = input_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = input_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = input_field_hdr.vlevel_type;
  field_hdr.dz_constant = input_field_hdr.dz_constant;
  field_hdr.proj_origin_lat = input_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = input_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = input_field_hdr.proj_param[i];
  field_hdr.vert_reference = input_field_hdr.vert_reference;
  field_hdr.grid_dx = input_field_hdr.grid_dx;
  field_hdr.grid_dy = input_field_hdr.grid_dy;
  field_hdr.grid_dz = input_field_hdr.grid_dz;
  field_hdr.grid_minx = input_field_hdr.grid_minx;
  field_hdr.grid_miny = input_field_hdr.grid_miny;
  field_hdr.grid_minz = input_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = input_field_hdr.bad_data_value;
  field_hdr.missing_data_value = input_field_hdr.missing_data_value;
  field_hdr.proj_rotation = input_field_hdr.proj_rotation;
  STRcopy(field_hdr.field_name_long, "Combined Radar/Ltg Data",
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "radar/ltg", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "interest", MDV_UNITS_LEN);
  field_hdr.transform[0] = '\0';
  
  // Return the new blank field

  return new MdvxField(field_hdr,
		       input_vlevel_hdr,
		       (void *)0,
		       true);
}

/*********************************************************************
 * _combineFields() - Process the matching radar data field and lightning
 *                    data field.
 */

MdvxField *CombineLtgRadar::_combineFields(const time_t data_time,
					   const MdvxField &radar_field,
					   const MdvxField &ltg_field) const
{
  static const string method_name = "CombineLtgRadar::_combineFields()";
  
  // Get the radar field header for reference

  
  Mdvx::field_header_t radar_field_hdr = radar_field.getFieldHeader();
  Mdvx::vlevel_header_t radar_vlevel_hdr = radar_field.getVlevelHeader();
  Mdvx::field_header_t ltg_field_hdr = ltg_field.getFieldHeader();
  
  // Create the blank combined field

  MdvxField *combined_field = _createCombinedField(data_time,
						   radar_field_hdr,
						   radar_vlevel_hdr);
  // Mdvx::field_header_t combined_field_hdr = combined_field->getFieldHeader();
  
  // Get the data for the input fields

  fl32 *ltg_data = (fl32 *)ltg_field.getVol();
  fl32 *radar_data = (fl32 *)radar_field.getVol();
  fl32 *combined_data = (fl32 *)combined_field->getVol();
  
   // Combine the grids.

  for (int i = 0;
       i < radar_field_hdr.nx * radar_field_hdr.ny * radar_field_hdr.nz;
       ++i)
  {
    bool radar_data_bad = false;
    bool ltg_data_bad = false;
    
    if (radar_data[i] == radar_field_hdr.bad_data_value ||
	radar_data[i] == radar_field_hdr.missing_data_value)
      radar_data_bad = true;
    
    if (ltg_data[i] == ltg_field_hdr.bad_data_value ||
	ltg_data[i] == ltg_field_hdr.missing_data_value)
      ltg_data_bad = true;
    
    // If both input data points are bad, leave the combined point as bad


    if (radar_data_bad && ltg_data_bad)
      continue;
    
    // If the radar data is bad, just use the lightning data

    if (radar_data_bad)
    {
      combined_data[i] = _getLtgDataValue((int)ltg_data[i]);
      continue;
    }
    
    // If the lightning data is bad, just use the radar data

    if (ltg_data_bad)
    {
      combined_data[i] = radar_data[i];
      continue;
    }
    
    // If we have data from both datasets, calculate the grid normally

    double ltg_component = _getLtgDataValue((int)ltg_data[i]);
    combined_data[i] = MAX(radar_data[i], ltg_component);
    
  } /* endfor - i */
  
  return combined_field;
}


/*********************************************************************
 * _combineFromRadarOnly() - Create 'combined' field from radar
 *                           data only because ltg is missing
 */

MdvxField
  *CombineLtgRadar::_combineFromRadarOnly(const time_t data_time,
                                          const MdvxField &radar_field) const
{
  static const string method_name = "CombineLtgRadar::_combineFromRadarOnly()";
  
  // Get the radar field header for reference

  Mdvx::field_header_t radar_field_hdr = radar_field.getFieldHeader();
  Mdvx::vlevel_header_t radar_vlevel_hdr = radar_field.getVlevelHeader();
  
  // Create the blank combined field

  MdvxField *combined_field = _createCombinedField(data_time,
						   radar_field_hdr,
						   radar_vlevel_hdr);
  // Mdvx::field_header_t combined_field_hdr = combined_field->getFieldHeader();
  
  // Get the data for the fields

  fl32 *radar_data = (fl32 *)radar_field.getVol();
  fl32 *combined_data = (fl32 *)combined_field->getVol();

  // copy radar data across

  int npts = radar_field_hdr.nx * radar_field_hdr.ny * radar_field_hdr.nz;
  memcpy(combined_data, radar_data, npts * sizeof(fl32));

  return combined_field;

}


/*********************************************************************
 * _readField() - Read the indicated field data.
 */

MdvxField *CombineLtgRadar::_readField(const time_t data_time,
				       const string &url,
				       const string &field_name,
				       const int field_num,
				       const int max_input_valid_secs,
				       Mdvx::master_header_t &master_hdr,
				       MdvxProj *remap_proj) const
{
  static const string method_name = "CombineLtgRadar::_readField()";
  
  // Set up the read request

  DsMdvx input_file;
  
  input_file.setReadTime(Mdvx::READ_CLOSEST,
			 url,
			 max_input_valid_secs,
			 data_time);

  if (field_name.size() == 0) {
    input_file.addReadField(field_num);
  } else {
    input_file.addReadField(field_name);
  }
  
  input_file.setReadNoChunks();
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);

  if (_params->request_composite_fields) {
    input_file.setReadComposite();
  }
  
  // Now read the volume

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading volume:" << endl;
    cerr << "   url: " << url << endl;
    cerr << "   field num: " << field_num << endl;
    
    return 0;
  }
  
  if (_params->debug_level >= Params::DEBUG_NORM)
  {
    cerr << endl;
    cerr << "path in use:: " <<
      input_file.getPathInUse()  << endl;
  }
  
  master_hdr = input_file.getMasterHeader();
  
  // Pull out the appropriate field and make a copy to be returned.
  // We must make a copy here because getField() returns a pointer
  // into the DsMdvx object and the object is automatically deleted
  // when we exit this method.

  MdvxField *field = input_file.getField(0);
  
  if (field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving field from volume:" << endl;
    cerr << "   url: " << url << endl;
    cerr << "   field: " << field_num << endl;
    
    return 0;
  }

  static MdvxRemapLut lut; // keep the lookup table around for susequent calls

  if (remap_proj != 0)
  {
    field->remap(lut, *remap_proj);
  }
 
  return new MdvxField(*field);
}


/*********************************************************************
 * _updateMasterHeader() - Update the master header for the output file.
 */

void CombineLtgRadar::_updateMasterHeader(DsMdvx &output_file,
					  const time_t start_time,
					  const time_t centroid_time,
					  const time_t end_time,
					  const time_t expire_time,
					  const Mdvx::master_header_t &input_master_hdr) const
{
  Mdvx::master_header_t master_hdr;
  
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = start_time;
  master_hdr.time_end = end_time;
  master_hdr.time_centroid = centroid_time;
  master_hdr.time_expire = expire_time;
  master_hdr.data_dimension = input_master_hdr.data_dimension;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = input_master_hdr.native_vlevel_type;
  master_hdr.vlevel_type = input_master_hdr.vlevel_type;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = input_master_hdr.grid_orientation;
  master_hdr.data_ordering = input_master_hdr.data_ordering;
  STRcopy(master_hdr.data_set_info,
	  "Generated by CombineLtgRadar", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name,
	  "CombineLtgRadar", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source,
	  "CombineLtgRadar", MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
}
