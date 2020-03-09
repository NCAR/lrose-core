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
//   $Date: 2016/03/06 23:15:37 $
//   $Id: ThetaEAdvect.cc,v 1.7 2016/03/06 23:15:37 dixon Exp $
//   $Revision: 1.7 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ThetaEAdvect: ThetaEAdvect program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2002
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsIntervalTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <physics/PhysicsLib.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "ThetaEAdvect.hh"
#include "Params.hh"


// Global variables

ThetaEAdvect *ThetaEAdvect::_instance =
     (ThetaEAdvect *)NULL;

const fl32 ThetaEAdvect::THETA_E_MISSING_DATA_VALUE = -999.0;


/*********************************************************************
 * Constructor
 */

ThetaEAdvect::ThetaEAdvect(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "ThetaEAdvect::ThetaEAdvect()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (ThetaEAdvect *)NULL);
  
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

ThetaEAdvect::~ThetaEAdvect()
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

ThetaEAdvect *ThetaEAdvect::Inst(int argc, char **argv)
{
  if (_instance == (ThetaEAdvect *)NULL)
    new ThetaEAdvect(argc, argv);
  
  return(_instance);
}

ThetaEAdvect *ThetaEAdvect::Inst()
{
  assert(_instance != (ThetaEAdvect *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool ThetaEAdvect::init()
{
  static const string method_name = "ThetaEAdvect::init()";
  
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
  
  case Params::INTERVAL :
  {
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
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void ThetaEAdvect::run()
{
  static const string method_name = "ThetaEAdvect::run()";
  
  
  while (!_dataTrigger->endOfData())
  {
    TriggerInfo triggerInfo;

    if (_dataTrigger->next(triggerInfo) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(triggerInfo.getIssueTime(),
                      triggerInfo.getForecastTime() - triggerInfo.getIssueTime()))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " <<
	triggerInfo.getIssueTime() << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _calc3DThetaEField() - Calculate the 3D theta-e field.
 *
 * Returns a pointer to the created field, or 0 if the field could
 * not be created for some reason.
 */

MdvxField *ThetaEAdvect::_calc3DThetaEField(const MdvxField &mixing_ratio_field,
					    const MdvxField &temperature_field,
					    const MdvxField &pressure_field) const
{
  static const string method_name = "ThetaEAdvect::_calc3DThetaEField()";
  
  MdvxField *theta_e_field;
  
  // Create the blank 3D theta-e field

  if ((theta_e_field = _create3DThetaEField(mixing_ratio_field)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating 3D theta-e field" << endl;
    
    return 0;
  }
  
  // Calculate the theta-e field

  Mdvx::field_header_t mixing_ratio_field_hdr =
    mixing_ratio_field.getFieldHeader();
  Mdvx::field_header_t temperature_field_hdr =
    temperature_field.getFieldHeader();
  Mdvx::field_header_t pressure_field_hdr =
    pressure_field.getFieldHeader();
  
  if (!PhysicsLib::calcThetaE3D((fl32 *)mixing_ratio_field.getVol(),
				mixing_ratio_field_hdr.missing_data_value,
				mixing_ratio_field_hdr.bad_data_value,
				(fl32 *)temperature_field.getVol(),
				temperature_field_hdr.missing_data_value,
				temperature_field_hdr.bad_data_value,
				(fl32 *)pressure_field.getVol(),
				pressure_field_hdr.missing_data_value,
				pressure_field_hdr.bad_data_value,
				(fl32 *)theta_e_field->getVol(),
				THETA_E_MISSING_DATA_VALUE,
				pressure_field_hdr.nx, pressure_field_hdr.ny,
				pressure_field_hdr.nz))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error calculating 3D theta-e field" << endl;
    
    delete theta_e_field;
    
    return 0;
  }
  
  return theta_e_field;
}


/*********************************************************************
 * _calcThetaEAdvectField() - Calculate the theta-e advection field.
 *
 * Returns a pointer to the created field, or 0 if the field could
 * not be created for some reason.
 */

MdvxField *ThetaEAdvect::_calcThetaEAdvectField(const MdvxField &u_field,
						const MdvxField &v_field,
						const MdvxField &theta_e_field) const
{
  static const string method_name = "ThetaEAdvect::_calcThetaEAdvectField()";
  
  MdvxField *theta_e_adv_field;
  
  // Create the blank theta-e advection field

  if ((theta_e_adv_field = _createThetaEAdvectField(theta_e_field)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating theta-e advection field" << endl;
    
    return 0;
  }
  
  // Calculate the theta-e advection field

  Mdvx::field_header_t u_field_hdr =
    u_field.getFieldHeader();
  Mdvx::field_header_t v_field_hdr =
    v_field.getFieldHeader();
  Mdvx::field_header_t theta_e_field_hdr =
    theta_e_field.getFieldHeader();
  Mdvx::field_header_t theta_e_adv_field_hdr =
    theta_e_adv_field->getFieldHeader();
  
  Mdvx::vlevel_header_t theta_e_vlevel_hdr =
    theta_e_field.getVlevelHeader();
  
  MdvxPjg u_proj(u_field_hdr);
  MdvxPjg v_proj(v_field_hdr);
  MdvxPjg theta_e_proj(theta_e_field_hdr);
  MdvxPjg theta_e_adv_proj(theta_e_adv_field_hdr);
  
  fl32 *u_data = (fl32 *)u_field.getVol();
  fl32 *v_data = (fl32 *)v_field.getVol();
  fl32 *theta_e_data = (fl32 *)theta_e_field.getVol();
  fl32 *theta_e_adv_data = (fl32 *)theta_e_adv_field->getVol();
  
  for (int x = 0; x < u_proj.getNx(); ++x)
  {
    for (int y = 0; y < u_proj.getNy(); ++y)
    {
      for (int z = 0; z < u_proj.getNz(); ++z)
      {
	// Calculate all of the indices used in the calculation to try to
	// make the calculations easier to read.  The U and V indices are
	// guaranteed to be within the grid because the projections are
	// previously checked to be matching.  For the theta e indices
	// (named using "eth"), we have to check to see if the point is
	// within the grid because we are looking at neighboring points.

	int u_index = u_proj.xyIndex2arrayIndex(x, y, z);
	int v_index = v_proj.xyIndex2arrayIndex(x, y, z);

	int eth_yplus_index =
	  theta_e_proj.xyIndex2arrayIndex(x, y+1, z);
	if (eth_yplus_index < 0)
	  eth_yplus_index = theta_e_proj.xyIndex2arrayIndex(x, y, z);
      
	int eth_yminus_index =
	  theta_e_proj.xyIndex2arrayIndex(x, y-1, z);
	if (eth_yminus_index < 0)
	  eth_yminus_index = theta_e_proj.xyIndex2arrayIndex(x, y, z);
      
	int eth_xplus_index =
	  theta_e_proj.xyIndex2arrayIndex(x+1, y, z);
	if (eth_xplus_index < 0)
	  eth_xplus_index = theta_e_proj.xyIndex2arrayIndex(x, y, z);
      
	int eth_xminus_index =
	  theta_e_proj.xyIndex2arrayIndex(x-1, y, z);
	if (eth_xminus_index < 0)
	  eth_xminus_index = theta_e_proj.xyIndex2arrayIndex(x, y, z);
      
	int eth_adv_index = theta_e_adv_proj.xyIndex2arrayIndex(x, y, z);
      
	// Make sure none of the data is missing.  We only have to check
	// for the bad_data_value for the theta_e data because we know that
	// we set both the bad_data_value and the missing_data_value to the
	// same value for this data previously.

	if (u_data[u_index] == u_field_hdr.missing_data_value ||
	    u_data[u_index] == u_field_hdr.bad_data_value ||
	    v_data[v_index] == v_field_hdr.missing_data_value ||
	    v_data[v_index] == v_field_hdr.bad_data_value ||
	    theta_e_data[eth_yplus_index] == theta_e_field_hdr.bad_data_value ||
	    theta_e_data[eth_yminus_index] == theta_e_field_hdr.bad_data_value ||
	    theta_e_data[eth_xplus_index] == theta_e_field_hdr.bad_data_value ||
	    theta_e_data[eth_xminus_index] == theta_e_field_hdr.bad_data_value)
	{
	  theta_e_adv_data[eth_adv_index] =
	    theta_e_adv_field_hdr.bad_data_value;
	
	  continue;
	}
      
      
	// Calculate the advection in each direction

	double v_advection =
	  v_data[v_index] *
	  (theta_e_data[eth_yplus_index] - theta_e_data[eth_yminus_index]) /
	  (2.0 * theta_e_proj.yGrid2km(1.0) * 1000.0);
      
//	cerr << "meters per grid space: " <<
//	  (theta_e_proj.xGrid2km(1.0, y) * 1000.0) << endl;
	
	double u_advection =
	  u_data[u_index] *
	  (theta_e_data[eth_xplus_index] - theta_e_data[eth_xminus_index]) /
	  (2.0 * theta_e_proj.xGrid2km(1.0, y) * 1000.0);
      
	// Calculate theta e advection
	
	theta_e_adv_data[eth_adv_index] =
	  -(v_advection + u_advection);
	
      } /* endfor - z */
    }  /* endfor - y */
  }  /* endfor - x */
  
  return theta_e_adv_field;
}


/*********************************************************************
 * _calcVertDeriveField() - Calculate the vertical derivative field.
 *
 * Returns a pointer to the created field, or 0 if the field could
 * not be created for some reason.
 */

MdvxField *ThetaEAdvect::_calcVertDeriveField(const MdvxField &theta_e_adv_field) const
{
  static const string method_name = "ThetaEAdvect::_calcVertDeriveField()";
  
  MdvxField *vert_der_field;
  
  // Create the blank vertical derivative field

  if ((vert_der_field = _createVertDeriveField(theta_e_adv_field)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating vertical derivative field" << endl;
    
    return 0;
  }
  
  // Calculate the vertical derivative field

  Mdvx::field_header_t theta_e_adv_field_hdr =
    theta_e_adv_field.getFieldHeader();
  Mdvx::field_header_t vert_der_field_hdr =
    vert_der_field->getFieldHeader();
  
  Mdvx::vlevel_header_t theta_e_vlevel_hdr =
    theta_e_adv_field.getVlevelHeader();
  
  MdvxPjg theta_e_adv_proj(theta_e_adv_field_hdr);
  MdvxPjg vert_der_proj(vert_der_field_hdr);
  
  fl32 *theta_e_adv_data = (fl32 *)theta_e_adv_field.getVol();
  fl32 *vert_der_data = (fl32 *)vert_der_field->getVol();
  
  for (int x = 0; x < theta_e_adv_proj.getNx(); ++x)
  {
    for (int y = 0; y < theta_e_adv_proj.getNy(); ++y)
    {
      // Calculate all of the indices used in the calculation to try to
      // make the calculations easier to read.  The U and V indices are
      // guaranteed to be within the grid because the projections are
      // previously checked to be matching.  For the theta e indices
      // (named using "eth"), we have to check to see if the point is
      // within the grid because we are looking at neighboring points.

      int upper_eth_index =
	theta_e_adv_proj.xyIndex2arrayIndex(x, y, theta_e_adv_proj.getNz()-1);
      int lower_eth_index =
	theta_e_adv_proj.xyIndex2arrayIndex(x, y, 0);

      int vert_der_index =
	vert_der_proj.xyIndex2arrayIndex(x, y, 0);
      
      // Make sure none of the data is missing.  We only have to check
      // for the bad_data_value for the theta_e data because we know that
      // we set both the bad_data_value and the missing_data_value to the
      // same value for this data previously.

      if (theta_e_adv_data[upper_eth_index] == theta_e_adv_field_hdr.bad_data_value ||
	  theta_e_adv_data[lower_eth_index] == theta_e_adv_field_hdr.bad_data_value)
	{
	  vert_der_data[vert_der_index] = vert_der_field_hdr.bad_data_value;
	
	  continue;
	}
      
      
	// Calculate the upper and lower Z levels used in the calculation

	double upper_z = theta_e_vlevel_hdr.level[theta_e_adv_proj.getNz()-1];
	double lower_z = theta_e_vlevel_hdr.level[0];


	// Calculate vertical derivative
	
	vert_der_data[vert_der_index] =
	  (theta_e_adv_data[upper_eth_index] -
	   theta_e_adv_data[lower_eth_index]) /
	  (upper_z - lower_z);
	
    }  /* endfor - y */
  }  /* endfor - x */
  
  return vert_der_field;
}


/*********************************************************************
 * _create3DThetaEField() - Create the blank 3D theta-e field.  Upon
 *                          return, the field values will be set to the
 *                          missing data value.
 *
 * Returns a pointer to the created field, or 0 if the field could
 * not be created for some reason.
 */

MdvxField *ThetaEAdvect::_create3DThetaEField(const MdvxField &base_field) const
{
  // Create the new field header

  Mdvx::field_header_t base_field_hdr = base_field.getFieldHeader();
  
  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.field_code = 0;
  field_hdr.forecast_delta = base_field_hdr.forecast_delta;
  field_hdr.forecast_time = base_field_hdr.forecast_time;
  field_hdr.nx = base_field_hdr.nx;
  field_hdr.ny = base_field_hdr.ny;
  field_hdr.nz = base_field_hdr.nz;
  field_hdr.proj_type = base_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = base_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = base_field_hdr.vlevel_type;
  field_hdr.dz_constant = base_field_hdr.dz_constant;
  
  field_hdr.proj_origin_lat = base_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = base_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = base_field_hdr.proj_param[i];
  field_hdr.vert_reference = 0;
  field_hdr.grid_dx = base_field_hdr.grid_dx;
  field_hdr.grid_dy = base_field_hdr.grid_dy;
  field_hdr.grid_dz = base_field_hdr.grid_dz;
  field_hdr.grid_minx = base_field_hdr.grid_minx;
  field_hdr.grid_miny = base_field_hdr.grid_miny;
  field_hdr.grid_minz = base_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = THETA_E_MISSING_DATA_VALUE;
  field_hdr.missing_data_value = THETA_E_MISSING_DATA_VALUE;
  field_hdr.proj_rotation = base_field_hdr.proj_rotation;
  
  STRcopy(field_hdr.field_name_long, "theta-e",
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "theta-e", MDV_SHORT_FIELD_LEN);
  
  STRcopy(field_hdr.units, "K", MDV_UNITS_LEN);
  field_hdr.transform[0] = '\0';
  
  // Create the blank field

  return new MdvxField(field_hdr, base_field.getVlevelHeader(),
		       (void *)0, true);
}


/*********************************************************************
 * _createThetaEAdvectField() - Create the blank theta-e advection field.
 *                              Upon return, the field values will be
 *                              set to the missing data value.
 *
 * Returns a pointer to the created field, or 0 if the field could
 * not be created for some reason.
 */

MdvxField *ThetaEAdvect::_createThetaEAdvectField(const MdvxField &base_field) const
{
  // Create the new field header

  Mdvx::field_header_t base_field_hdr = base_field.getFieldHeader();
  
  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.field_code = 0;
  field_hdr.forecast_delta = base_field_hdr.forecast_delta;
  field_hdr.forecast_time = base_field_hdr.forecast_time;
  field_hdr.nx = base_field_hdr.nx;
  field_hdr.ny = base_field_hdr.ny;
  field_hdr.nz = base_field_hdr.nz;
  field_hdr.proj_type = base_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = base_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = base_field_hdr.vlevel_type;
  field_hdr.dz_constant = base_field_hdr.dz_constant;
  
  field_hdr.proj_origin_lat = base_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = base_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = base_field_hdr.proj_param[i];
  field_hdr.vert_reference = 0;
  field_hdr.grid_dx = base_field_hdr.grid_dx;
  field_hdr.grid_dy = base_field_hdr.grid_dy;
  field_hdr.grid_dz = base_field_hdr.grid_dz;
  field_hdr.grid_minx = base_field_hdr.grid_minx;
  field_hdr.grid_miny = base_field_hdr.grid_miny;
  field_hdr.grid_minz = base_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = THETA_E_MISSING_DATA_VALUE;
  field_hdr.missing_data_value = THETA_E_MISSING_DATA_VALUE;
  field_hdr.proj_rotation = base_field_hdr.proj_rotation;
  
  STRcopy(field_hdr.field_name_long, "theta-e advection",
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "theta-e adv", MDV_SHORT_FIELD_LEN);
  
  STRcopy(field_hdr.units, "K/s", MDV_UNITS_LEN);
  field_hdr.transform[0] = '\0';
  
  // Create the blank field

  return new MdvxField(field_hdr, base_field.getVlevelHeader(),
		       (void *)0, true);
}


/*********************************************************************
 * _createVertDeriveField() - Create the blank vertical derivative field.
 *                            Upon return, the field values will be
 *                            set to the missing data value.
 *
 * Returns a pointer to the created field, or 0 if the field could
 * not be created for some reason.
 */

MdvxField *ThetaEAdvect::_createVertDeriveField(const MdvxField &base_field) const
{
  // Create the new field header

  Mdvx::field_header_t base_field_hdr = base_field.getFieldHeader();
  
  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.field_code = 0;
  field_hdr.forecast_delta = base_field_hdr.forecast_delta;
  field_hdr.forecast_time = base_field_hdr.forecast_time;
  field_hdr.nx = base_field_hdr.nx;
  field_hdr.ny = base_field_hdr.ny;
  field_hdr.nz = 1;
  field_hdr.proj_type = base_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = base_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.dz_constant = base_field_hdr.dz_constant;
  
  field_hdr.proj_origin_lat = base_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = base_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = base_field_hdr.proj_param[i];
  field_hdr.vert_reference = 0;
  field_hdr.grid_dx = base_field_hdr.grid_dx;
  field_hdr.grid_dy = base_field_hdr.grid_dy;
  field_hdr.grid_dz = 1.0;
  field_hdr.grid_minx = base_field_hdr.grid_minx;
  field_hdr.grid_miny = base_field_hdr.grid_miny;
  field_hdr.grid_minz = 0.5;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = THETA_E_MISSING_DATA_VALUE;
  field_hdr.missing_data_value = THETA_E_MISSING_DATA_VALUE;
  field_hdr.proj_rotation = base_field_hdr.proj_rotation;
  
  STRcopy(field_hdr.field_name_long, "theta-e advection vert der",
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "theta-e adv vert der", MDV_SHORT_FIELD_LEN);
  
  STRcopy(field_hdr.units, "K/s/mb", MDV_UNITS_LEN);
  field_hdr.transform[0] = '\0';
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.5;
  
  // Create the blank field

  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}


/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool ThetaEAdvect::_processData(const time_t trigger_time, const int lead_time)
{
  static const string method_name = "ThetaEAdvect::_processData()";
  
  if (_params->debug)
    cerr << "*** Processing data for trigger time: " <<
      DateTime::str(trigger_time) << endl;
  
  // Read in the input fields

  Mdvx::master_header_t mixing_ratio_master_hdr;
  MdvxField *mixing_ratio_field;
  
  if ((mixing_ratio_field =
       _readFieldData(_params->mixing_ratio_field_info.url,
		      _params->mixing_ratio_field_info.field_name,
		      _params->mixing_ratio_field_info.field_num,
		      _params->pressure_limits.lower_level,
		      _params->pressure_limits.upper_level,
		      trigger_time,
                      lead_time,
		      _params->max_input_valid_secs,
		      &mixing_ratio_master_hdr)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading in mixing ratio field from url: " <<
      _params->mixing_ratio_field_info.url << endl;
    
    return false;
  }
  
  MdvxField *temperature_field;
  
  if ((temperature_field =
       _readFieldData(_params->temp_field_info.url,
		      _params->temp_field_info.field_name,
		      _params->temp_field_info.field_num,
		      _params->pressure_limits.lower_level,
		      _params->pressure_limits.upper_level,
		      trigger_time,
	              lead_time,
		      _params->max_input_valid_secs)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading in temperature field from url: " <<
      _params->temp_field_info.url << endl;
    
    delete mixing_ratio_field;
    
    return false;
  }
  
  MdvxField *pressure_field;
  
  if ((pressure_field =
       _readFieldData(_params->pressure_field_info.url,
		      _params->pressure_field_info.field_name,
		      _params->pressure_field_info.field_num,
		      _params->pressure_limits.lower_level,
		      _params->pressure_limits.upper_level,
		      trigger_time,
	              lead_time,
		      _params->max_input_valid_secs)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading in pressure field from url: " <<
      _params->pressure_field_info.url << endl;
    
    delete mixing_ratio_field;
    delete temperature_field;
    
    return false;
  }
  
  MdvxField *u_field;
  
  if ((u_field =
       _readFieldData(_params->u_field_info.url,
		      _params->u_field_info.field_name,
		      _params->u_field_info.field_num,
		      _params->pressure_limits.lower_level,
		      _params->pressure_limits.upper_level,
		      trigger_time,
	              lead_time,
		      _params->max_input_valid_secs)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading in U field from url: " <<
      _params->u_field_info.url << endl;
    
    delete mixing_ratio_field;
    delete temperature_field;
    delete pressure_field;
    
    return false;
  }
  
  MdvxField *v_field;
  
  if ((v_field =
       _readFieldData(_params->v_field_info.url,
		      _params->v_field_info.field_name,
		      _params->v_field_info.field_num,
		      _params->pressure_limits.lower_level,
		      _params->pressure_limits.upper_level,
		      trigger_time,
	              lead_time,
		      _params->max_input_valid_secs)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading in V field from url: " <<
      _params->v_field_info.url << endl;
    
    delete mixing_ratio_field;
    delete temperature_field;
    delete pressure_field;
    delete u_field;
    
    return false;
  }
  
  // Make sure that the input fields all have the same projections

  Mdvx::field_header_t mixing_ratio_field_hdr =
    mixing_ratio_field->getFieldHeader();
  Mdvx::field_header_t temperature_field_hdr =
    temperature_field->getFieldHeader();
  Mdvx::field_header_t pressure_field_hdr =
    pressure_field->getFieldHeader();
  Mdvx::field_header_t u_field_hdr =
    u_field->getFieldHeader();
  Mdvx::field_header_t v_field_hdr =
    v_field->getFieldHeader();
  
  MdvxPjg mixing_ratio_proj(mixing_ratio_field_hdr);
  MdvxPjg temperature_proj(temperature_field_hdr);
  MdvxPjg pressure_proj(pressure_field_hdr);
  MdvxPjg u_proj(u_field_hdr);
  MdvxPjg v_proj(v_field_hdr);
  
  if (mixing_ratio_proj != temperature_proj ||
      mixing_ratio_proj != pressure_proj ||
      mixing_ratio_proj != u_proj ||
      mixing_ratio_proj != v_proj)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Projections of input fields don't match" << endl;
    cerr << "Cannot calculate 3D theta-e field" << endl;
    
    delete mixing_ratio_field;
    delete temperature_field;
    delete pressure_field;
    delete u_field;
    delete v_field;
    
    return false;
  }
  
  // Calculate the 3D theta-e field

  MdvxField *theta_e_field;
  
  if ((theta_e_field = _calc3DThetaEField(*mixing_ratio_field,
					  *temperature_field,
					  *pressure_field)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error calculating 3D theta-e field" << endl;
    
    delete mixing_ratio_field;
    delete temperature_field;
    delete pressure_field;
    delete u_field;
    delete v_field;
    
    return false;
  }
  
  // Get rid of the fields we don't need anymore

  delete mixing_ratio_field;
  delete temperature_field;
  delete pressure_field;
  
  // Now calculate the theta-e advection field

  MdvxField *theta_e_adv_field;
  
  if ((theta_e_adv_field = _calcThetaEAdvectField(*u_field,
						  *v_field,
						  *theta_e_field)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error calculating theta-e advection field" << endl;
    
    delete u_field;
    delete v_field;
    delete theta_e_field;
    
    return false;
  }
  
  delete u_field;
  delete v_field;
  
  // Finally, calculate the vertical derivative of the theta-e
  // advection field

  MdvxField *vert_der_field;
  
  if ((vert_der_field = _calcVertDeriveField(*theta_e_adv_field)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error calculating vertical derivative field" << endl;
    
    delete theta_e_field;
    delete theta_e_adv_field;
    
    return false;
  }
  
  // Create and write the output file

  DsMdvx output_file;
  
  _updateOutputMasterHeader(output_file,
			    mixing_ratio_master_hdr);
  
  theta_e_field->convertType(Mdvx::ENCODING_INT8,
			     Mdvx::COMPRESSION_BZIP,
			     Mdvx::SCALING_DYNAMIC);
  
  output_file.addField(theta_e_field);
  
  theta_e_adv_field->convertType(Mdvx::ENCODING_INT8,
				 Mdvx::COMPRESSION_BZIP,
				 Mdvx::SCALING_DYNAMIC);
  
  output_file.addField(theta_e_adv_field);
  
  vert_der_field->convertType(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_BZIP,
			      Mdvx::SCALING_DYNAMIC);
  
  output_file.addField(vert_der_field);
 
  if (_params->write_as_forecast)
     output_file.setWriteAsForecast();
 
  output_file.setWriteLdataInfo();
  
  if (output_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: " <<
      _params->output_url << endl;
    cerr << output_file.getErrStr() << endl;
    
    return false;
  }
    
  return true;
}


/*********************************************************************
 * _readFieldData() - Read the indicated field data.
 */

MdvxField *ThetaEAdvect::_readFieldData(const string &url,
					const string &field_name,
					const int field_num,
					const double lower_level,
					const double upper_level,
					const time_t data_time,
	                                const int lead_time,
					const int max_input_valid_secs,
					Mdvx::master_header_t *master_hdr)
{
  static const string method_name = "ThetaEAdvect::_readFieldData()";
  
  // Set up the read request

  DsMdvx input_file;
  
 if(_params->is_forecast_data)
     input_file.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                            url,
                            max_input_valid_secs,
                            data_time,
                            lead_time);
 else
    input_file.setReadTime(Mdvx::READ_CLOSEST,
              		 url,
			 max_input_valid_secs,
			 data_time);
  
  if (field_name.length() > 0)
    input_file.addReadField(field_name);
  else
    input_file.addReadField(field_num);
  
  input_file.setReadNoChunks();
  
  // I think we really want to use all of the input data in the
  // calculations, but just do the calculations between these
  // levels.  Didn't want to actually delete this until I knew
  // for sure.

  input_file.setReadVlevelLimits(lower_level, upper_level);
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  // Now read the volume

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading volume:" << endl;
    cerr << "   url: " << url << endl;
    cerr << "   field: \"" << field_name << "\" (" << field_num << ")" << endl;
    
    return 0;
  }
  
  // Make sure the data in the volume is ordered in the way we expect.

  Mdvx::master_header_t local_master_hdr = input_file.getMasterHeader();
  
  if (local_master_hdr.grid_orientation != Mdvx::ORIENT_SN_WE)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Input grid has incorrect orientation" << endl;
    cerr << "Expecting " << Mdvx::orientType2Str(Mdvx::ORIENT_SN_WE) <<
      " orientation, got " <<
      Mdvx::orientType2Str(local_master_hdr.grid_orientation) <<
      " orientation" << endl;
    cerr << "Url: " << url << endl;
    
    return 0;
  }
  
  if (local_master_hdr.data_ordering != Mdvx::ORDER_XYZ)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Input grid has incorrect data ordering" << endl;
    cerr << "Expectin " << Mdvx::orderType2Str(Mdvx::ORDER_XYZ) <<
      " ordering, got " << Mdvx::orderType2Str(local_master_hdr.data_ordering) <<
      " ordering" << endl;
    cerr << "Url: " << url << endl;
    
    return 0;
  }
  
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
    cerr << "   field: \"" << field_name << "\" (" << field_num << ")" << endl;
    
    return 0;
  }
  
  // Return the master header, if requested

  if (master_hdr != 0)
    *master_hdr = local_master_hdr;
    
  return new MdvxField(*field);
}


/*********************************************************************
 * _updateOutputMasterHeader() - Update the master header values for
 *                               the output file.
 */

void ThetaEAdvect::_updateOutputMasterHeader(DsMdvx &output_file,
					     const Mdvx::master_header_t &input_master_hdr)
{
  Mdvx::master_header_t master_hdr;
  
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  if(_params->write_as_forecast)
  {
    master_hdr.time_gen = input_master_hdr.time_gen;
  }
  else
  {
    master_hdr.time_gen = time(0);
  }

  master_hdr.time_begin = input_master_hdr.time_begin;
  master_hdr.time_end = input_master_hdr.time_end;
  master_hdr.time_centroid = input_master_hdr.time_centroid;
  master_hdr.time_expire = input_master_hdr.time_expire;
  master_hdr.data_dimension = input_master_hdr.data_dimension;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = input_master_hdr.native_vlevel_type;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = input_master_hdr.grid_orientation;
  master_hdr.data_ordering = input_master_hdr.data_ordering;
  
  master_hdr.sensor_lon = input_master_hdr.sensor_lon;
  master_hdr.sensor_lat = input_master_hdr.sensor_lat;
  master_hdr.sensor_alt = input_master_hdr.sensor_alt;
  
  STRcopy(master_hdr.data_set_info, "ThetaEAdvect output", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "ThetaEAdvect", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, "ThetaEAdvect", MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
}
