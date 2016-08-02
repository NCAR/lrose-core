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
 * @file NegBuoyancy.cc
 *
 * @class NegBuoyancy
 *
 * NegBuoyancy is the top level application class.
 *  
 * @date 6/10/2010
 *
 */

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsFcstTimeListTrigger.hh>
#include <dsdata/DsIntervalTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "NegBuoyancy.hh"
#include "Params.hh"
#include "Physics.hh"

#include "FlatInput.hh"
#include "FcstInput.hh"


// Global variables

NegBuoyancy *NegBuoyancy::_instance =
     (NegBuoyancy *)NULL;


const fl32 NegBuoyancy::CAPE_MISSING_DATA_VALUE = -999.0;
const fl32 NegBuoyancy::CIN_MISSING_DATA_VALUE = -999.0;
const fl32 NegBuoyancy::HEIGHT_MISSING_DATA_VALUE = -999.0;


/*********************************************************************
 * Constructor
 */

NegBuoyancy::NegBuoyancy(int argc, char **argv) :
  _dataTrigger(0),
  _pressureField(0),
  _temperatureField(0),
  _heightField(0),
  _terrainField(0),
  _boundingPresField(0),
  _capeField(0),
  _cinField(0),
  _lclRelHtField(0),
  _lfcRelHtField(0),
  _elRelHtField(0),
  _lclField(0),
  _lfcField(0),
  _kbminField(0),
  _bminField(0),
  _zbminField(0),
  _tlcField(0),
  _tliftField(0),
  _zparField(0),
  _kparField(0),
  _bmaxField(0)
{
  static const string method_name = "NegBuoyancy::NegBuoyancy()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (NegBuoyancy *)NULL);
  
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
  char *params_path = new char[strlen("unknown") + 1];
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

NegBuoyancy::~NegBuoyancy()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  delete _input;
  
  _clearData();
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

NegBuoyancy *NegBuoyancy::Inst(int argc, char **argv)
{
  if (_instance == (NegBuoyancy *)NULL)
    new NegBuoyancy(argc, argv);
  
  return(_instance);
}

NegBuoyancy *NegBuoyancy::Inst()
{
  assert(_instance != (NegBuoyancy *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool NegBuoyancy::init()
{
  static const string method_name = "NegBuoyancy::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the input object

  if (!_initInput())
    return false;
  
  // Set the temperature lookup file name

  _lookupTable.setFilename(_params->adiabat_temp_lookup_filename);

  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void NegBuoyancy::run()
{
  static const string method_name = "NegBuoyancy::run()";
  
  
  while (!_dataTrigger->endOfData())
  {
    PMU_auto_register("Waiting for data...");

    TriggerInfo trigger_info;

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
	   << DateTime::str(trigger_info.getIssueTime()) << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _calculateOutputFields()
 */

bool NegBuoyancy::_calculateOutputFields()
{
  static const string method_name = "NegBuoyancy::_calculateOutputFields()";
  
  PMU_auto_register("Creating output fields");

  // Create the output fields.  First we need to figure out the min and max
  // calculation levels so we know the vertical size for the 3D fields.

  int min_calc_level, max_calc_level;
  
  _pressureField->computePlaneLimits(_params->pressure_limits.lower_level,
				     _params->pressure_limits.upper_level,
				     min_calc_level, max_calc_level);
  
  if (_params->debug)
  {
    cerr << "Minimum level for calculations is plane number " << min_calc_level << endl;
    cerr << "Maximum level for calculations is plane number " << max_calc_level << endl;
  }
  
  if (max_calc_level < min_calc_level)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Minimum level lower than maximum level" << endl;
    cerr << "Min level = " << _params->pressure_limits.lower_level <<
      ", max level = " << _params->pressure_limits.upper_level << endl;
    cerr << "Switching levels" << endl;
    
    int temp = min_calc_level;
    min_calc_level = max_calc_level;
    max_calc_level = temp;
  }
  
  if (!_createOutputFields(min_calc_level, max_calc_level))
    return false;
  
  // Calculate the output fields

  PMU_force_register("Calculating CAPE and CIN");

  if (!Physics::calcCapeCin(_lookupTable,
			    *_pressureField,
			    *_temperatureField,
			    *_mixingRatioField,
			    *_heightField,
			    *_terrainField,
			    *_boundingPresField,
			    (fl32 *)_capeField->getVol(),
			    (fl32 *)_cinField->getVol(),
			    min_calc_level, max_calc_level,
			    _params->process_3d,
			    (fl32 *)_lclRelHtField->getVol(),
			    (fl32 *)_lfcRelHtField->getVol(),
			    (fl32 *)_elRelHtField->getVol(),
			    (fl32 *)_lclField->getVol(),
			    (fl32 *)_lfcField->getVol(),
			    (fl32 *)_kbminField->getVol(),
			    (fl32 *)_bminField->getVol(),
			    (fl32 *)_zbminField->getVol(),
			    (fl32 *)_tlcField->getVol(),
			    (fl32 *)_tliftField->getVol(),
			    (fl32 *)_zparField->getVol(),
			    (fl32 *)_kparField->getVol(),
			    (fl32 *)_bmaxField->getVol(),
			    _params->debug))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error calculating CAPE/CIN fields" << endl;
    
    return false;
  }

  return true;
}


/*********************************************************************
 * _clearData()
 */

void NegBuoyancy::_clearData()
{
  delete _pressureField;
  delete _temperatureField;
  delete _heightField;
  delete _terrainField;
  delete _boundingPresField;
  delete _capeField;
  delete _cinField;
  delete _lclRelHtField;
  delete _lfcRelHtField;
  delete _elRelHtField;
  delete _lclField;
  delete _lfcField;
  delete _kbminField;
  delete _bminField;
  delete _zbminField;
  delete _tlcField;
  delete _tliftField;
  delete _zparField;
  delete _kparField;
  delete _bmaxField;
  
}


/*********************************************************************
 * _createField()
 */

MdvxField *NegBuoyancy::_createField(const string &field_name,
				     const string &field_name_long,
				     const int field_code,
				     const string &units,
				     const fl32 missing_data_value,
				     const Mdvx::field_header_t &sample_field_hdr)
{
  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.field_code = field_code;
  field_hdr.forecast_delta = sample_field_hdr.forecast_delta;
  field_hdr.forecast_time = sample_field_hdr.forecast_time;
  field_hdr.nx = sample_field_hdr.nx;
  field_hdr.ny = sample_field_hdr.ny;
  field_hdr.nz = 1;
  field_hdr.proj_type = sample_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = sample_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = sample_field_hdr.vlevel_type;
  field_hdr.dz_constant = sample_field_hdr.dz_constant;
  field_hdr.proj_origin_lat = sample_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = sample_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = sample_field_hdr.proj_param[i];
  field_hdr.vert_reference = sample_field_hdr.vert_reference;
  field_hdr.grid_dx = sample_field_hdr.grid_dx;
  field_hdr.grid_dy = sample_field_hdr.grid_dy;
  field_hdr.grid_dz = sample_field_hdr.grid_dz;
  field_hdr.grid_minx = sample_field_hdr.grid_minx;
  field_hdr.grid_miny = sample_field_hdr.grid_miny;
  field_hdr.grid_minz = sample_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = missing_data_value;
  field_hdr.missing_data_value = missing_data_value;
  field_hdr.proj_rotation = sample_field_hdr.proj_rotation;
  STRcopy(field_hdr.field_name_long,
	  field_name_long.c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  
  Mdvx::vlevel_header_t vlevel_hdr;
  
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.0;
  
  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}


MdvxField *NegBuoyancy::_createField(const string &field_name,
				     const string &field_name_long,
				     const int field_code,
				     const string &units,
				     const fl32 missing_data_value,
				     const Mdvx::field_header_t &sample_field_hdr,
				     const Mdvx::vlevel_header_t &sample_vlevel_hdr,
				     const int min_vlevel,
				     const int max_vlevel)
{
  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.field_code = field_code;
  field_hdr.forecast_delta = sample_field_hdr.forecast_delta;
  field_hdr.forecast_time = sample_field_hdr.forecast_time;
  field_hdr.nx = sample_field_hdr.nx;
  field_hdr.ny = sample_field_hdr.ny;
  field_hdr.nz = max_vlevel - min_vlevel + 1;
  field_hdr.proj_type = sample_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = sample_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = sample_field_hdr.vlevel_type;
  field_hdr.dz_constant = sample_field_hdr.dz_constant;
  field_hdr.proj_origin_lat = sample_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = sample_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = sample_field_hdr.proj_param[i];
  field_hdr.vert_reference = sample_field_hdr.vert_reference;
  field_hdr.grid_dx = sample_field_hdr.grid_dx;
  field_hdr.grid_dy = sample_field_hdr.grid_dy;
  field_hdr.grid_dz = sample_field_hdr.grid_dz;
  field_hdr.grid_minx = sample_field_hdr.grid_minx;
  field_hdr.grid_miny = sample_field_hdr.grid_miny;
  field_hdr.grid_minz = sample_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = missing_data_value;
  field_hdr.missing_data_value = missing_data_value;
  field_hdr.proj_rotation = sample_field_hdr.proj_rotation;
  STRcopy(field_hdr.field_name_long,
	  field_name_long.c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  
  Mdvx::vlevel_header_t vlevel_hdr;
  
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  for (int i = min_vlevel; i <= max_vlevel; ++i)
  {
    vlevel_hdr.type[i-min_vlevel] = sample_vlevel_hdr.type[i];
    vlevel_hdr.level[i-min_vlevel] = sample_vlevel_hdr.level[i];
  }
  
  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}


/*********************************************************************
 * _createOutputFields()
 */

bool NegBuoyancy::_createOutputFields(const int min_calc_level,
				      const int max_calc_level)
{
  static const string method_name = "NegBuoyancy::_createOutputFields()";
  
  Mdvx::field_header_t pressure_field_hdr = _pressureField->getFieldHeader();
  Mdvx::vlevel_header_t pressure_vlevel_hdr = _pressureField->getVlevelHeader();
  
  if (_params->process_3d)
  {
    if ((_capeField = _createField("CAPE",
				   "convective available potential energy",
				   157, "J/kg",
				   CAPE_MISSING_DATA_VALUE,
				   pressure_field_hdr,
				   pressure_vlevel_hdr,
				   min_calc_level, max_calc_level)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating CAPE MDV field" << endl;
    
      return false;
    }
  
    if ((_cinField = _createField("CIN",
				  "convective inhibition",
				  156, "J/kg",
				  CIN_MISSING_DATA_VALUE,
				  pressure_field_hdr,
				  pressure_vlevel_hdr,
				  min_calc_level, max_calc_level)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating CIN MDV field" << endl;
    
      return false;
    }
  
    if ((_lclRelHtField = _createField("lclrel",
				       "lifted condensation level relative height",
				       0, "m",
				       HEIGHT_MISSING_DATA_VALUE,
				       pressure_field_hdr,
				       pressure_vlevel_hdr,
				       min_calc_level, max_calc_level)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating LCL MDV field" << endl;
    
      return false;
    }
    
    if ((_lfcRelHtField = _createField("lfcrel",
				       "level of free convection relative height",
				       0, "m",
				       HEIGHT_MISSING_DATA_VALUE,
				       pressure_field_hdr,
				       pressure_vlevel_hdr,
				       min_calc_level, max_calc_level)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating LFC MDV field" << endl;
      
      return false;
    }
    
    if ((_elRelHtField = _createField("elrel",
				      "equilibrium level relative height field",
				      0, "m",
				      HEIGHT_MISSING_DATA_VALUE,
				      pressure_field_hdr,
				      pressure_vlevel_hdr,
				      min_calc_level, max_calc_level)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating EL MDV field" << endl;
      
      return false;
    }
    
    if ((_lclField = _createField("lcl",
				  "lifted condensation level",
				  0, "m AGL",
				  HEIGHT_MISSING_DATA_VALUE,
				  pressure_field_hdr,
				  pressure_vlevel_hdr,
				  min_calc_level, max_calc_level)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating LCL relative height MDV field" << endl;
      
      return false;
    }
    
    if ((_lfcField = _createField("lfc",
				  "level of free convection",
				  0, "m AGL",
				  HEIGHT_MISSING_DATA_VALUE,
				  pressure_field_hdr,
				  pressure_vlevel_hdr,
				  min_calc_level, max_calc_level)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating LFC relative height MDV field" << endl;
      
      return false;
    }
  }
  else
  {
    if ((_capeField = _createField("CAPE",
				   "convective available potential energy",
				   157, "J/kg",
				   CAPE_MISSING_DATA_VALUE,
				   pressure_field_hdr)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating CAPE MDV field" << endl;
    
      return false;
    }
  
    if ((_cinField = _createField("CIN",
				  "convective inhibition",
				  156, "J/kg",
				  CIN_MISSING_DATA_VALUE,
				  pressure_field_hdr)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating CIN MDV field" << endl;
    
      return false;
    }
  
    if ((_lclRelHtField = _createField("lclrel",
				       "lifted condensation level relative height",
				       0, "m",
				       HEIGHT_MISSING_DATA_VALUE,
				       pressure_field_hdr)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating LCL MDV field" << endl;
    
      return false;
    }
    
    if ((_lfcRelHtField = _createField("lfcrel",
				       "level of free convection relative height",
				       0, "m",
				       HEIGHT_MISSING_DATA_VALUE,
				       pressure_field_hdr)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating LFC MDV field" << endl;
      
      return false;
    }
    
    if ((_elRelHtField = _createField("elrel",
				      "equilibrium level relative height field",
				      0, "m",
				      HEIGHT_MISSING_DATA_VALUE,
				      pressure_field_hdr)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating EL MDV field" << endl;
      
      return false;
    }
    
    if ((_lclField = _createField("lcl",
				  "lifted condensation level",
				  0, "m AGL",
				  HEIGHT_MISSING_DATA_VALUE,
				  pressure_field_hdr)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating LCL relative height MDV field" << endl;
      
      return false;
    }
    
    if ((_lfcField = _createField("lfc",
				  "level of free convection",
				  0, "m AGL",
				  HEIGHT_MISSING_DATA_VALUE,
				  pressure_field_hdr)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating LFC relative height MDV field" << endl;
      
      return false;
    }
  }
  
  if ((_kbminField = _createField("kbmin",
				  "model level of min buoyancy",
				  0, "none",
				  HEIGHT_MISSING_DATA_VALUE,
				  pressure_field_hdr)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating model level of min buoyancy MDV field" << endl;
      
    return false;
  }
  
  if ((_bminField = _createField("bmin",
				 "minimum buoyancy of lifted parcel",
				 0, "C",
				 HEIGHT_MISSING_DATA_VALUE,
				 pressure_field_hdr)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating minimum buoyancy of lifted parcel MDV field" << endl;
    
    return false;
  }
    
  if ((_zbminField = _createField("zbmin",
				  "height of min buoyancy",
				  0, "m AGL",
				  HEIGHT_MISSING_DATA_VALUE,
				  pressure_field_hdr)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating height of min buoyancy MDV field" << endl;
      
    return false;
  }
    
  if ((_tlcField = _createField("tlc",
				"lcl temperature",
				0, "K",
				HEIGHT_MISSING_DATA_VALUE,
				pressure_field_hdr)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating LCL temperature MDV field" << endl;
      
    return false;
  }
    
  if ((_tliftField = _createField("tlift",
				  "virtual temperature of lifted parcel",
				  0, "K",
				  HEIGHT_MISSING_DATA_VALUE,
				  pressure_field_hdr)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating virtual temperature of lifted parcel MDV field" << endl;
      
    return false;
  }
    
  if ((_zparField = _createField("zpar",
				 "original height of lifted parcel",
				 0, "m AGL",
				 HEIGHT_MISSING_DATA_VALUE,
				 pressure_field_hdr)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating original height of lifted parcel MDV field" << endl;
      
    return false;
  }
    
  if ((_kparField = _createField("kpar",
				 "original model level of lifted parcel",
				 0, "none",
				 HEIGHT_MISSING_DATA_VALUE,
				 pressure_field_hdr)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating original model level of lifted parcel MDV field" << endl;
      
    return false;
  }
    
  if ((_bmaxField = _createField("bmax",
				 "maximum buoyancy of lifted parcel",
				 0, "C",
				 HEIGHT_MISSING_DATA_VALUE,
				 pressure_field_hdr)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating maximum buoyancy MDV field" << endl;
    
    return false;
  }
    
  return true;
}


/*********************************************************************
 * _initInput()
 */

bool NegBuoyancy::_initInput()
{
  // Create the input processing object

  if (_params->read_forecast_data)
    _input = new FcstInput(_params->debug);
  else
    _input = new FlatInput(_params->debug);

  // Tell the input object to remap the input, if requested

  if (_params->remap_input)
  {
    MdvxPjg input_proj;
  
    switch (_params->remap_info.proj_type)
    {
    case Params::PROJ_LATLON :
      input_proj.initLatlon(_params->remap_info.nx,
			    _params->remap_info.ny,
			    1,
			    _params->remap_info.dx,
			    _params->remap_info.dy,
			    1.0,
			    _params->remap_info.minx,
			    _params->remap_info.miny,
			    0.0);
      break;
      
    case Params::PROJ_FLAT :
      input_proj.initFlat(_params->remap_info.origin_lat,
			  _params->remap_info.origin_lon,
			  _params->remap_info.rotation,
			  _params->remap_info.nx,
			  _params->remap_info.ny,
			  1,
			  _params->remap_info.dx,
			  _params->remap_info.dy,
			  1.0,
			  _params->remap_info.minx,
			  _params->remap_info.miny,
			  0.0);
      break;
    }
    
    _input->setRemap(input_proj);
    
  } /* endif - _params->remap_input */
  
  return true;
}


/*********************************************************************
 * _initTrigger()
 */

bool NegBuoyancy::_initTrigger()
{
  static const string method_name = "NegBuoyancy::_initTrigger()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->latest_data_trigger << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->latest_data_trigger,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->latest_data_trigger << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::TIME_LIST :
  {
    if (_params->debug)
    {
      cerr << "Initializing TIME_LIST trigger using url: " <<
	_params->latest_data_trigger << endl;
      cerr << "   start time string = "
	   << _params->time_list_trigger.start_time << endl;
      cerr << "   end time string = "
	   << _params->time_list_trigger.end_time << endl;
    }
    
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
    if (trigger->init(_params->time_list_trigger.url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->time_list_trigger.url << endl;
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
    if (_params->debug)
    {
      cerr << "Initializing FCST_TIME_LIST trigger using url: " <<
	_params->latest_data_trigger << endl;
      cerr << "   start time string = "
	   << _params->time_list_trigger.start_time << endl;
      cerr << "   end time string = "
	   << _params->time_list_trigger.end_time << endl;
    }
    
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
    if (trigger->init(_params->time_list_trigger.url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FCST_TIME_LIST trigger for url: " <<
	_params->time_list_trigger.url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INTERVAL :
  {
    if (_params->debug)
    {
      cerr << "Initializing INTERVAL trigger using url: " <<
	_params->latest_data_trigger << endl;
      cerr << "   start time string = "
	   << _params->interval_trigger.start_time << endl;
      cerr << "   end time string = "
	   << _params->interval_trigger.end_time << endl;
      cerr << "   interval = " << _params->interval_trigger.interval << endl;
    }
    
    time_t start_time =
      DateTime::parseDateTime(_params->interval_trigger.start_time);
    time_t end_time =
      DateTime::parseDateTime(_params->interval_trigger.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for INTERVAL trigger: " <<
	_params->interval_trigger.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for INTERVAL trigger: " <<
	_params->interval_trigger.end_time << endl;
      
      return false;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->interval_trigger.interval,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INTERVAL trigger" << endl;
      cerr << "     Start time: " << _params->interval_trigger.start_time << endl;
      cerr << "     End time: " << _params->interval_trigger.end_time << endl;
      cerr << "     Interval: " << _params->interval_trigger.interval <<
	" secs" << endl;
      
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

bool NegBuoyancy::_processData(const TriggerInfo &trigger_info)
{
  static const string method_name = "NegBuoyancy::_processData()";
  
  DateTime trigger_time(trigger_info.getIssueTime());
  DateTime trigger_fcst_time(trigger_info.getForecastTime());
  
  if (_params->debug)
  {
    if (trigger_fcst_time == DateTime::NEVER)
      cerr << "*** Processing data for time: " << trigger_time  << endl;
    else
      cerr << "*** Processing forecast data: gen_time = " << trigger_time
	   << ", fcst_time = " << trigger_fcst_time << endl;;
    
  }
  
  string pmu_string =
    "Processing data for time: " + trigger_time.str();

  PMU_force_register(pmu_string.c_str());

  // Clear out any old data

  _clearData();
  
  // Read in all of the fields

  if (!_readInputFields(trigger_info))
    return false;
  
  // Calculate the "bounding" pressure field

  if ((_boundingPresField =
       Physics::calcBoundingPressure(*_pressureField)) == 0)
  {
    return false;
  }
  
  // Calculate the output fields

  if (!_calculateOutputFields())
    return false;
  
  // Create and write the output file

  if (_params->debug)
    cerr << "Creating output file" << endl;
  
  DsMdvx output_file;
  
  _updateMasterHeader(output_file, trigger_time,
		      _pressureField->getFieldHeader());
  
  if (_params->include_input_fields)
  {
//    _pressureField->convertType(Mdvx::ENCODING_INT8,
//				Mdvx::COMPRESSION_BZIP,
//				Mdvx::SCALING_DYNAMIC);
//    _temperatureField->convertType(Mdvx::ENCODING_INT8,
//				   Mdvx::COMPRESSION_BZIP,
//				   Mdvx::SCALING_DYNAMIC);
//    _mixingRatioField->convertType(Mdvx::ENCODING_INT8,
//				    Mdvx::COMPRESSION_BZIP,
//				    Mdvx::SCALING_DYNAMIC);
//    _heightField->convertType(Mdvx::ENCODING_INT8,
//			      Mdvx::COMPRESSION_BZIP,
//			      Mdvx::SCALING_DYNAMIC);
    
    output_file.addField(_pressureField);
    output_file.addField(_temperatureField);
    output_file.addField(_mixingRatioField);
    output_file.addField(_heightField);
    output_file.addField(_terrainField);
    output_file.addField(_boundingPresField);

    _pressureField = 0;
    _temperatureField = 0;
    _mixingRatioField = 0;
    _heightField = 0;
    _terrainField = 0;
    _boundingPresField = 0;
    
  }
  
  // Add the CAPE/CIN fields to the output file.  Don't compress the CIN
  // values because the data range is too large.

//  _capeField->convertType(Mdvx::ENCODING_INT8,
//			  Mdvx::COMPRESSION_BZIP,
//			  Mdvx::SCALING_DYNAMIC);
  
  output_file.addField(_capeField);
  output_file.addField(_cinField);
  
  _capeField = 0;
  _cinField = 0;
  
  // Add the optional height fields to the output file

  if (_params->output_intermediate_fields)
  {
//    _lclRelHtField->convertType(Mdvx::ENCODING_INT8,
//				Mdvx::COMPRESSION_BZIP,
//				Mdvx::SCALING_DYNAMIC);
//    _lfcRelHtField->convertType(Mdvx::ENCODING_INT8,
//				Mdvx::COMPRESSION_BZIP,
//				Mdvx::SCALING_DYNAMIC);
//    _elRelHtField->convertType(Mdvx::ENCODING_INT8,
//			       Mdvx::COMPRESSION_BZIP,
//			       Mdvx::SCALING_DYNAMIC);
//    _lclField->convertType(Mdvx::ENCODING_INT8,
//			   Mdvx::COMPRESSION_BZIP,
//			   Mdvx::SCALING_DYNAMIC);
//    _lfcField->convertType(Mdvx::ENCODING_INT8,
//			   Mdvx::COMPRESSION_BZIP,
//			   Mdvx::SCALING_DYNAMIC);
//    _kbminField->convertType(Mdvx::ENCODING_INT8,
//			     Mdvx::COMPRESSION_BZIP,
//			     Mdvx::SCALING_DYNAMIC);
//    _bminField->convertType(Mdvx::ENCODING_INT8,
//			    Mdvx::COMPRESSION_BZIP,
//			    Mdvx::SCALING_DYNAMIC);
//    _zbminField->convertType(Mdvx::ENCODING_INT8,
//			     Mdvx::COMPRESSION_BZIP,
//			     Mdvx::SCALING_DYNAMIC);
//    _tlcField->convertType(Mdvx::ENCODING_INT8,
//			   Mdvx::COMPRESSION_BZIP,
//			   Mdvx::SCALING_DYNAMIC);
//    _tliftField->convertType(Mdvx::ENCODING_INT8,
//			     Mdvx::COMPRESSION_BZIP,
//			     Mdvx::SCALING_DYNAMIC);
//    _zparField->convertType(Mdvx::ENCODING_INT8,
//			    Mdvx::COMPRESSION_BZIP,
//			    Mdvx::SCALING_DYNAMIC);
//    _kparField->convertType(Mdvx::ENCODING_INT8,
//			    Mdvx::COMPRESSION_BZIP,
//			    Mdvx::SCALING_DYNAMIC);
//    _bmaxField->convertType(Mdvx::ENCODING_INT8,
//			    Mdvx::COMPRESSION_BZIP,
//			    Mdvx::SCALING_DYNAMIC);
    
    output_file.addField(_lclRelHtField);
    output_file.addField(_lfcRelHtField);
    output_file.addField(_elRelHtField);
    output_file.addField(_lclField);
    output_file.addField(_lfcField);
    output_file.addField(_kbminField);
    output_file.addField(_bminField);
    output_file.addField(_zbminField);
    output_file.addField(_tlcField);
    output_file.addField(_tliftField);
    output_file.addField(_zparField);
    output_file.addField(_kparField);
    output_file.addField(_bmaxField);
    
    _lclRelHtField = 0;
    _lfcRelHtField = 0;
    _elRelHtField = 0;
    _lclField = 0;
    _lfcField = 0;
    _kbminField = 0;
    _bminField = 0;
    _zbminField = 0;
    _tlcField = 0;
    _tliftField = 0;
    _zparField = 0;
    _kparField = 0;
    _bmaxField = 0;
  }
  else
  {
    delete _lclRelHtField;
    delete _lfcRelHtField;
    delete _elRelHtField;
    delete _lclField;
    delete _lfcField;
    delete _kbminField;
    delete _bminField;
    delete _zbminField;
    delete _tlcField;
    delete _tliftField;
    delete _zparField;
    delete _kparField;
    delete _bmaxField;
  }
  
  // Finally, write the output file

  if (_params->debug)
    cerr << "Writing file to URL: " << _params->output_url << endl;
  
  if(_params->write_as_forecast)
    output_file.setWriteAsForecast();

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
 * _readInputFields()
 */

bool NegBuoyancy::_readInputFields(const TriggerInfo &trigger_info)
{
  static const string method_name = "NegBuoyancy::_readInputFields()";
  
  // Pressure

  if ((_pressureField =
       _input->readField(trigger_info,
			 _params->pressure_field_info.url,
			 _params->pressure_field_info.field_name,
			 _params->pressure_field_info.field_num,
			 _params->max_input_valid_secs)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading pressure field from url: " <<
      _params->pressure_field_info.url << endl;
    cerr << "Field name = \"" << _params->pressure_field_info.field_name <<
      "\", field num = " << _params->pressure_field_info.field_num << endl;
    
    return false;
  }
  
  // Temperature

  PMU_auto_register("Reading temperature field");

  if ((_temperatureField =
       _input->readField(trigger_info,
			 _params->temperature_field_info.url,
			 _params->temperature_field_info.field_name,
			 _params->temperature_field_info.field_num,
			 _params->max_input_valid_secs)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading temperature field from url: " <<
      _params->temperature_field_info.url << endl;
    cerr << "Field name = \"" << _params->temperature_field_info.field_name <<
      "\", field num = " << _params->temperature_field_info.field_num << endl;
    
    return false;
  }
  
  // Mixing ratio -- convert from g/kg to g/g

  PMU_auto_register("Reading mixing ratio field");

  if ((_mixingRatioField =
       _input->readField(trigger_info,
			 _params->mixing_ratio_field_info.url,
			 _params->mixing_ratio_field_info.field_name,
			 _params->mixing_ratio_field_info.field_num,
			 _params->max_input_valid_secs)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading mixing_ratio field from url: " <<
      _params->mixing_ratio_field_info.url << endl;
    cerr << "Field name = \"" << _params->mixing_ratio_field_info.field_name <<
      "\", field num = " << _params->mixing_ratio_field_info.field_num << endl;
    
    return false;
  }
  
  Mdvx::field_header_t mr_field_hdr = _mixingRatioField->getFieldHeader();
  STRcopy(mr_field_hdr.units, "g/g", MDV_UNITS_LEN);
  _mixingRatioField->setFieldHeader(mr_field_hdr);
  
  fl32 *mr_data = (fl32 *)_mixingRatioField->getVol();
  for (int i = 0; i < mr_field_hdr.nx * mr_field_hdr.ny * mr_field_hdr.nz; ++i)
  {
    if (mr_data[i] == mr_field_hdr.bad_data_value ||
	mr_data[i] == mr_field_hdr.missing_data_value)
      continue;
    
    mr_data[i] /= 1000.0;
  } /* endfor - i */
  
  // Height

  PMU_auto_register("Reading height field");

  if ((_heightField =
       _input->readField(trigger_info,
			 _params->height_field_info.url,
			 _params->height_field_info.field_name,
			 _params->height_field_info.field_num,
			 _params->max_input_valid_secs)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading height field from url: " <<
      _params->height_field_info.url << endl;
    cerr << "Field name = \"" << _params->height_field_info.field_name <<
      "\", field num = " << _params->height_field_info.field_num << endl;
    
    return false;
  }
  
  // Terrain

  PMU_auto_register("Reading terrain field");

  if ((_terrainField =
       _input->readField(trigger_info,
			 _params->terrain_field_info.url,
			 _params->terrain_field_info.field_name,
			 _params->terrain_field_info.field_num,
			 _params->max_input_valid_secs)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading terrain field from url: " <<
      _params->terrain_field_info.url << endl;
    if (string(_params->terrain_field_info.field_name) == "")
      cerr << "Field num = " << _params->terrain_field_info.field_num << endl;
    else
      cerr << "Field name = \"" << _params->terrain_field_info.field_name
	   << "\"" << endl;

    return false;
  }
  
  // Make sure all of the fields are on the same projection

  Mdvx::field_header_t pressure_field_hdr = _pressureField->getFieldHeader();
  Mdvx::field_header_t temperature_field_hdr =
    _temperatureField->getFieldHeader();
  Mdvx::field_header_t mixing_ratio_field_hdr =
    _mixingRatioField->getFieldHeader();
  Mdvx::field_header_t height_field_hdr = _heightField->getFieldHeader();
  Mdvx::field_header_t terrain_field_hdr = _terrainField->getFieldHeader();
  
  MdvxPjg pressure_proj(pressure_field_hdr);
  MdvxPjg temperature_proj(temperature_field_hdr);
  MdvxPjg mixing_ratio_proj(mixing_ratio_field_hdr);
  MdvxPjg height_proj(height_field_hdr);
  MdvxPjg terrain_proj(terrain_field_hdr);
  
  if (pressure_proj != temperature_proj ||
      pressure_proj != mixing_ratio_proj ||
      pressure_proj != height_proj)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot process fields -- projections don't match" <<endl;
    
    return false;
  }

  if (pressure_proj.getProjType() != terrain_proj.getProjType() ||
      pressure_proj.getNx() != terrain_proj.getNx() ||
      pressure_proj.getNy() != terrain_proj.getNy() ||
      pressure_proj.getDx() != terrain_proj.getDx() ||
      pressure_proj.getDy() != terrain_proj.getDy() ||
      pressure_proj.getMinx() != terrain_proj.getMinx() ||
      pressure_proj.getMiny() != terrain_proj.getMiny())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot process fields -- terrain projection doesn't match" <<endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _updateMasterheader()
 */

void NegBuoyancy::_updateMasterHeader(DsMdvx &output_file,
				      const DateTime &data_time,
				      const Mdvx::field_header_t &field_hdr)
{
  Mdvx::master_header_t master_hdr;
  
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  if(_params->write_as_forecast)
  {
    master_hdr.time_gen = field_hdr.forecast_time - field_hdr.forecast_delta;
  }
  else
  {
    master_hdr.time_gen = time(0);
  }

  master_hdr.time_begin = data_time.utime();
  master_hdr.time_end = data_time.utime();
  master_hdr.time_centroid = data_time.utime();
  master_hdr.time_expire = data_time.utime();
  master_hdr.data_dimension = 3;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = field_hdr.vlevel_type;
  master_hdr.vlevel_type = field_hdr.vlevel_type;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  STRcopy(master_hdr.data_set_info,
	  "Generated by NegBuoyancy", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name,
	  "NegBuoyancy", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source,
	  "NegBuoyancy", MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
}
