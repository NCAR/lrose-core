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
//   $Date: 2016/03/04 02:22:11 $
//   $Id: MdvModel2Vil.cc,v 1.26 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.26 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvModel2Vil: MdvModel2Vil program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsFcstTimeListTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <Mdv/MdvxChunk.hh>

#include "MdvModel2Vil.hh"
#include "Params.hh"

#include "CalcRwpHandler.hh"
#include "FieldRwpHandler.hh"

using namespace std;

// Global variables

MdvModel2Vil *MdvModel2Vil::_instance =
     (MdvModel2Vil *)NULL;


/*********************************************************************
 * Constructor
 */

MdvModel2Vil::MdvModel2Vil(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvModel2Vil::MdvModel2Vil()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvModel2Vil *)NULL);
  
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

MdvModel2Vil::~MdvModel2Vil()
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

MdvModel2Vil *MdvModel2Vil::Inst(int argc, char **argv)
{
  if (_instance == (MdvModel2Vil *)NULL)
    new MdvModel2Vil(argc, argv);
  
  return(_instance);
}

MdvModel2Vil *MdvModel2Vil::Inst()
{
  assert(_instance != (MdvModel2Vil *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvModel2Vil::init()
{
  static const string method_name = "MdvModel2Vil::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MdvModel2Vil::run()
{
  static const string method_name = "MdvModel2Vil::run()";
  
  // Process each of the input files, printing out the coverage values
  // for each as they are processed.

  TriggerInfo trigger_info;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_info.getIssueTime(),
		      trigger_info.getForecastTime()))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data:" << endl;
      cerr << "   Issue time: "
	   << DateTime::str(trigger_info.getIssueTime()) << endl;
      cerr << "   Fcst time: "
	   << DateTime::str(trigger_info.getForecastTime()) << endl;
    
      continue;
    }

    
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createDigitalVil() - Create the digital VIL field.
 *
 * Returns a pointer to the created field on success, 0 on failure.
 */

MdvxField *MdvModel2Vil::_createDigitalVil(MdvxField &rwp_field) const
{
  // Create the blank field object

  MdvxField *vil_field =
    _createSurfaceField(rwp_field.getFieldHeader(), -999.0,
			_params->output_field_name, _params->output_field_name, "none");

  if (vil_field == 0)
    return 0;
  
  // Fill in the digital vil values

  Mdvx::field_header_t rwp_field_hdr = rwp_field.getFieldHeader();
  Mdvx::field_header_t vil_field_hdr = vil_field->getFieldHeader();
  
  fl32 *rwp_data = (fl32 *)rwp_field.getVol();
  fl32 *vil_data = (fl32 *)vil_field->getVol();
  
  int plane_size = rwp_field_hdr.nx * rwp_field_hdr.ny;

  for (int plane_index = 0; plane_index < plane_size; ++plane_index)
  {
    fl32 rwp = rwp_data[plane_index];
    
    if (rwp == rwp_field_hdr.missing_data_value ||
	rwp == rwp_field_hdr.bad_data_value)
    {
      vil_data[plane_index] = vil_field_hdr.bad_data_value;
    }
    else
    {
      //
      // Calculate digital vil from rwp
      // if rwp <= low_thresh then vil = 0  
      // low_thresh < rwp < high_thresh then vil =   a0 + a1 * rwp
      // if rwp >= high_thresh then vil = cc * (b0 + b1 * log (b2 * (hrrrVIL)^ee ) )
      //
      // FOR conversion to mitll digitalVIL use:
      // a0 =2 , a1 = 90.659, b0 = 83.903, b1 = 38.88

      if (rwp <= _params->low_thresh)
	vil_data[plane_index] = 0.0;
      else if ( _params->low_thresh < rwp && rwp < _params->high_thresh)
	vil_data[plane_index] =  _params->a0 +  _params->a1 * rwp;
      else  
	vil_data[plane_index] = _params->cc * (_params->b0 + _params->b1 * 
					      (log (_params->b2 ) + _params->ee * log (rwp)));

      if ( vil_data[plane_index] > 254)
	vil_data[plane_index] = 254;

    }
  } /* endfor - plane_index */
  
  return vil_field;
}


/*********************************************************************
 * _createSurfaceField() - Create a blank surface field so the values can be
 *                         filled in later.  The field will have the
 *                         same X/Y dimensions and forecast time as the
 *                         given field header.
 *
 * Returns a pointer to the created field on success, 0 on failure.
 */

MdvxField *MdvModel2Vil::_createSurfaceField(const Mdvx::field_header_t in_field_hdr,
					     const fl32 bad_data_value,
					     const string &field_name_long,
					     const string &field_name,
					     const string &units) const
{
  // Set up the VIL field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));

  field_hdr.forecast_delta = in_field_hdr.forecast_delta;
  field_hdr.forecast_time = in_field_hdr.forecast_time;
  field_hdr.nx = in_field_hdr.nx;
  field_hdr.ny = in_field_hdr.ny;
  field_hdr.nz = 1;
  field_hdr.proj_type = in_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = in_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  field_hdr.proj_origin_lat = in_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = in_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = in_field_hdr.proj_param[i];
  field_hdr.grid_dx = in_field_hdr.grid_dx;
  field_hdr.grid_dy = in_field_hdr.grid_dy;
  field_hdr.grid_dz = 1.0;
  field_hdr.grid_minx = in_field_hdr.grid_minx;
  field_hdr.grid_miny = in_field_hdr.grid_miny;
  field_hdr.grid_minz = 0.5;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = bad_data_value;
  field_hdr.missing_data_value = bad_data_value;
  field_hdr.proj_rotation = in_field_hdr.proj_rotation;
  STRcopy(field_hdr.field_name_long, field_name_long.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  
  // Set up the VIL vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.5;
  
  // Create and return the new field

  return new MdvxField(field_hdr, vlevel_hdr,
		       (void *)0, true);
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool MdvModel2Vil::_initTrigger(void)
{
  static const string method_name = "MdvModel2Vil::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  case Params::LATEST_NONFCST_DATA :
  {
    if (_params->debug)
    {
      cerr << "Initializing LATEST_DATA trigger: " << endl;
      cerr << "   url: " << _params->input_url << endl;
    }
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
		      _params->max_valid_age,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
      cerr << "   url: " << _params->input_url << endl;

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
    
    if (_params->debug)
    {
      cerr << "Initializing TIME_LIST trigger: " << endl;
      cerr << "   url: " << _params->input_url << endl;
      cerr << "   start time: " << DateTime::str(start_time) << endl;
      cerr << "   end time: " << DateTime::str(end_time) << endl;
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
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }

  case Params::TIME_LIST_NONFCST_DATA :
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
    
    if (_params->debug)
    {
      cerr << "Initializing TIME_LIST trigger: " << endl;
      cerr << "   url: " << _params->input_url << endl;
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
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
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
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool MdvModel2Vil::_processData(const DateTime &gen_time,
				const DateTime &fcst_time)
{
  static const string method_name = "MdvModel2Vil::_processData()";
  
  if (_params->debug) 
    if ( _params->trigger_mode != Params::TIME_LIST_NONFCST_DATA &&
         _params->trigger_mode != Params::LATEST_NONFCST_DATA)
      {
	cerr << _params->trigger_mode  << endl;
	cerr << endl << "*** Processing data:" << endl;
	cerr << "      Gen time: " << gen_time << endl;
	cerr << "      Fcst time: " << fcst_time << endl;
      }
    else
      {
	cerr << endl << "*** Processing data:" << endl;
	cerr << "      request time: " << gen_time << endl;
      }
  
  // Get the RWP field from the input file

  RwpHandler *rwp_handler;
  
  if (_params->rwp_field_supplied)
    {

      FieldRwpHandler *handler;
      if (_params->trigger_mode != Params::TIME_LIST_NONFCST_DATA
          && _params->trigger_mode != Params::LATEST_NONFCST_DATA)
	{
	  handler = new FieldRwpHandler(_params->input_url,
					gen_time, fcst_time);
	}
      else
	{
	  DateTime timeCentroid = gen_time;
	  handler = new FieldRwpHandler(_params->input_url,
					timeCentroid);
	}

      if (!handler->init(_params->input_fields.rwp_field_name))
	return false;
      
      rwp_handler = handler;
    }
  else
  {

    CalcRwpHandler *handler;

    if (_params->trigger_mode != Params::TIME_LIST_NONFCST_DATA
	&& _params->trigger_mode != Params::LATEST_NONFCST_DATA)
      {
	handler = new CalcRwpHandler(_params->input_url,
				     gen_time, fcst_time);
      }  
    else
      {
	DateTime timeCentroid = gen_time;
	handler = new CalcRwpHandler(_params->input_url,
						     timeCentroid);
      }
    if (!handler->init(_params->get_pressure_from_field,
		       _params->input_fields.rnw_field_name,
		       _params->input_fields.snow_field_name,
		       _params->input_fields.temp_field_name,
		       _params->input_fields.hgt_field_name,
		       _params->input_fields.press_field_name,
		       _params->height_increasing))
      return false;

    rwp_handler = handler;
  }
  
  MdvxField *rwp_field = rwp_handler->getRwpField();

  if (rwp_field == 0)
  {
    delete rwp_handler;
    
    return false;
  }
  
  // Save the master header for use in the output file

  Mdvx::master_header_t in_master_hdr = rwp_handler->getMasterHeader();

  // save copies of chunks for use in the output file

  int n = rwp_handler->getNChunks();
  vector<MdvxChunk *> chunks;
  for (int k=0; k<n; ++k)
  {
    MdvxChunk *c;
    c = new MdvxChunk(*rwp_handler->getChunkByNum(k));
    chunks.push_back(c);
  }

  delete rwp_handler;
  
  // Create the digital VIL field

  MdvxField *vil_field = _createDigitalVil(*rwp_field);
  
  if (vil_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating digital VIL field" << endl;
    
    delete rwp_field;
    
    return false;
  }
  
  // Create and write the output file

  DsMdvx out_mdvx;
  
  _updateMasterHeader(in_master_hdr, out_mdvx);
  
  rwp_field->convertType(Mdvx::ENCODING_INT8,
			 Mdvx::COMPRESSION_BZIP,
			 Mdvx::SCALING_DYNAMIC);
  out_mdvx.addField(rwp_field);

  for (int k=0; k<(int)chunks.size(); ++k)
    out_mdvx.addChunk(chunks[k]);
  
  vil_field->convertType(Mdvx::ENCODING_INT8,
			 Mdvx::COMPRESSION_BZIP,
			 Mdvx::SCALING_DYNAMIC);
  out_mdvx.addField(vil_field);
  
  out_mdvx.setWriteLdataInfo();

   if ( _params->trigger_mode != Params::TIME_LIST_NONFCST_DATA &&
        _params->trigger_mode != Params::LATEST_NONFCST_DATA)
     out_mdvx.setWriteAsForecast();
 
  if (_params->debug)
   {
       DateTime dateTime;

       cerr << "writing to output URL: " << _params->output_url << endl;
       
       cerr << "gen_time " << gen_time.dtime() << " lead time " 
            << fcst_time.utime() - gen_time.utime() << endl;      
   }
 
  if (out_mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing to output URL: " << _params->output_url << endl;
    cerr << out_mdvx.getErrStr() << endl;
    
    return false;
  }
  
  
  return true;
}


/*********************************************************************
 * _updateMasterHeader() - Update the master header of the output file
 *                         based on the master header of the input file.
 */

void MdvModel2Vil::_updateMasterHeader(const Mdvx::master_header_t &in_master_hdr,
				       DsMdvx &out_mdvx) const
{
  if (_params->debug)
  {
    cerr << "Creating output master header:" << endl;
    cerr << "   time_gen = " << DateTime::str(in_master_hdr.time_gen) << endl;
    cerr << "   time_begin = " << DateTime::str(in_master_hdr.time_begin) << endl;
    cerr << "   time_end = " << DateTime::str(in_master_hdr.time_end) << endl;
    cerr << "   time_centroid = " << DateTime::str(in_master_hdr.time_centroid) << endl;
  }
  
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));

  if( _params->trigger_mode != Params::TIME_LIST_NONFCST_DATA 
      && _params->trigger_mode != Params :: LATEST_NONFCST_DATA)
    master_hdr.time_gen = in_master_hdr.time_gen;
  else
    master_hdr.time_gen = in_master_hdr.time_centroid;
  master_hdr.time_begin = in_master_hdr.time_begin;
  master_hdr.time_end = in_master_hdr.time_end;
  master_hdr.time_centroid = in_master_hdr.time_centroid;
  master_hdr.time_expire = in_master_hdr.time_expire;
  master_hdr.data_dimension = 2;
  if( _params->trigger_mode != Params::TIME_LIST_NONFCST_DATA &&
      _params->trigger_mode != Params::LATEST_NONFCST_DATA)
     master_hdr.data_collection_type = Mdvx::DATA_FORECAST;	
  else 
     master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.native_vlevel_type = in_master_hdr.native_vlevel_type;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = in_master_hdr.grid_orientation;
  master_hdr.data_ordering = in_master_hdr.data_ordering;
  STRcopy(master_hdr.data_set_info, "MdvModel2Vil", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "MdvModel2Vil", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source,  "MdvModel2Vil", MDV_NAME_LEN);
  
  out_mdvx.setMasterHeader(master_hdr);
}
