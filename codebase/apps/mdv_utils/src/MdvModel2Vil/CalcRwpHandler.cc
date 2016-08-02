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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:22:11 $
//   $Id: CalcRwpHandler.cc,v 1.4 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * CalcRwpHandler: Class that supplies the RWP field by calculating it
 *                 from the RNW, SNOW, TEMP and HGT fields and the
 *                 available pressure data.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/MdvxPjg.hh>
#include <toolsa/str.h>

#include "CalcRwpHandler.hh"

#include "FieldPressHandler.hh"
#include "VlevelPressHandler.hh"

using namespace std;


// Global variables

const double CalcRwpHandler::RR = 287.04;


/*********************************************************************
 * Constructors
 */

CalcRwpHandler::CalcRwpHandler(const string &url,
			       const DateTime &gen_time,
			       const DateTime &fcst_time) :
  RwpHandler(url, gen_time, fcst_time),
  _rnwField(0),
  _snowField(0),
  _tkField(0),
  _hgtField(0),
  _pressHandler(0)
{
   _readForecast = true;
}

CalcRwpHandler::CalcRwpHandler(const string &url,
			       const DateTime &time_centroid) :
  RwpHandler(url, time_centroid),
  _rnwField(0),
  _snowField(0),
  _tkField(0),
  _hgtField(0),
  _pressHandler(0)
{
   _readForecast = false;
}
  
/*********************************************************************
 * Destructor
 */

CalcRwpHandler::~CalcRwpHandler()
{
  delete _pressHandler;
}

/*********************************************************************
 * init() - Initialize the data.
 *
 * Returns true on success, false on failure.
 */

bool CalcRwpHandler::init(const bool get_pressure_from_field,
			  const string &rnw_field_name,
			  const string &snow_field_name,
			  const string &tk_field_name,
			  const string &hgt_field_name,
			  const string &press_field_name,
			  const bool height_increasing)
{
  static const string method_name = "CalcRwpHandler::init()";
  
  // Save needed flags

  _heightIncreasing = height_increasing;
  
  // Read in the input file.  The input fields will appear in the MDV file
  // in the order that they are put into the input_field_names vector.

  vector< string > input_field_names;
  
  input_field_names.push_back(rnw_field_name);
  input_field_names.push_back(snow_field_name);
  input_field_names.push_back(tk_field_name);
  input_field_names.push_back(hgt_field_name);
  if (get_pressure_from_field)
    input_field_names.push_back(press_field_name);
 
  if (!_readInputData(input_field_names))
    return false;
  
 
  // Get pointers to the input fields.  These pointers will be useful later.

  _rnwField = _mdvx.getField(0);
  _snowField = _mdvx.getField(1);
  _tkField = _mdvx.getField(2);
  _hgtField = _mdvx.getField(3);
  
  if (!_projectionsMatch(get_pressure_from_field))
    return false;
  
  // Create the pressure handler object

  if (get_pressure_from_field)
  {
    FieldPressHandler *handler = new FieldPressHandler();
    MdvxField *press_field = _mdvx.getField(4);
    
    if (!handler->init(press_field))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FieldPressHandler object" << endl;
      
      return false;
    }
    
    _pressHandler = handler;
  }
  else
  {
    VlevelPressHandler *handler = new VlevelPressHandler();
    Mdvx::vlevel_header_t vlevel_hdr = _rnwField->getVlevelHeader();
    
    if (!handler->init(vlevel_hdr))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing VlevelPressHandler object" << endl;
      
      return false;
    }
    
    _pressHandler = handler;
  }
  
  return true;
}

/*********************************************************************
 * getRwpField() - Get the RWP field.
 *
 * Returns a pointer to the field on success, 0 on failure.  The pointer
 * is then owned by the calling method and must be deleted there.
 */

MdvxField *CalcRwpHandler::getRwpField()
{
  return _createRwp();
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _createRwp() - Create the RWP (VIL) field.
 *
 * Returns a pointer to the created field on success, 0 on failure.
 */

MdvxField *CalcRwpHandler::_createRwp() const
{
  // Create the blank RWP field

  MdvxField *rwp_field =
    _createSurfaceField(_rnwField->getFieldHeader(), -999.0,
			"Total Precipitation Water Path", "RWP", "g/m**2");

  if (rwp_field == 0)
    return 0;
  
  // Get information for all of the fields

  Mdvx::field_header_t rnw_field_hdr = _rnwField->getFieldHeader();
  Mdvx::field_header_t snow_field_hdr = _snowField->getFieldHeader();
  Mdvx::field_header_t tk_field_hdr = _tkField->getFieldHeader();
  Mdvx::field_header_t hgt_field_hdr = _hgtField->getFieldHeader();
  Mdvx::field_header_t rwp_field_hdr = rwp_field->getFieldHeader();
  
  fl32 *rnw_data = (fl32 *)_rnwField->getVol();
  fl32 *snow_data = (fl32 *)_snowField->getVol();
  fl32 *tk_data = (fl32 *)_tkField->getVol();
  fl32 *hgt_data = (fl32 *)_hgtField->getVol();
  fl32 *rwp_data = (fl32 *)rwp_field->getVol();
  
  // Calculate the RWP values

  int plane_size = rnw_field_hdr.nx * rnw_field_hdr.ny;
  
  for (int plane_index = 0; plane_index < plane_size; ++plane_index)
  {
    rwp_data[plane_index] = 0.0;

    bool rwp_valid = true;
    
    int z_start;
    int z_end;
    int z_incr;
    
    if (_heightIncreasing)
    {
      z_start = 0;
      z_end = rnw_field_hdr.nz;
      z_incr = 1;
    }
    else
    {
      z_start = rnw_field_hdr.nz - 1;
      z_end = -1;
      z_incr = -1;
    }
    
    for (int z = z_start; z != z_end; z += z_incr)
    {
      int vol_index = (z * plane_size) + plane_index;
      
      // Calculate the height value

      if (hgt_data[vol_index] == hgt_field_hdr.bad_data_value ||
	  hgt_data[vol_index] == hgt_field_hdr.missing_data_value)
      {
	rwp_valid = false;
	break;
      }

      // Initialize with lowest level height

      double dz;
      
      if (z == z_start)
      {
	dz = hgt_data[vol_index];
      }
      else
      {
	int prev_vol_index = ((z - z_incr) * plane_size) + plane_index;
	dz = hgt_data[vol_index] - hgt_data[prev_vol_index];
      }
      
      // Get the pressure value

      fl32 pressure;
      
      if (!_pressHandler->getPressureValue(plane_index, z, pressure))
      {
	rwp_valid = false;
	break;
      }
      
      // Calculate the rhoa value

      if (tk_data[vol_index] == tk_field_hdr.bad_data_value ||
	  tk_data[vol_index] == tk_field_hdr.missing_data_value)
      {
	rwp_valid = false;
	break;
      }
      
      double rhoa = _calcRhoa(pressure, tk_data[vol_index]);
      
      // Calculate the rwater value

      if (rnw_data[vol_index] == rnw_field_hdr.bad_data_value ||
	  rnw_data[vol_index] == rnw_field_hdr.missing_data_value ||
	  snow_data[vol_index] == snow_field_hdr.bad_data_value ||
	  snow_data[vol_index] == snow_field_hdr.missing_data_value)
      {
	rwp_valid = false;
	break;
      }
      
      double rwater = _calcRwater(rnw_data[vol_index], snow_data[vol_index],
				  rhoa);
      
      // Calculate the rwp value

      rwp_data[plane_index] += rwater * dz;

    } /* endfor - z */
    
    if (rwp_valid)
    {
      // Convert the RWP value from g/m**2 to kg/m**2

      rwp_data[plane_index] /= 1000.0;
    }
    else
    {
      rwp_data[plane_index] = rwp_field_hdr.bad_data_value;
    }
    
  } /* endfor - plane_index */
  
  return rwp_field;
}


/*********************************************************************
 * _createSurfaceField() - Create a blank surface field so the values can be
 *                         filled in later.  The field will have the
 *                         same X/Y dimensions and forecast time as the
 *                         given field header.
 *
 * Returns a pointer to the created field on success, 0 on failure.
 */

MdvxField *CalcRwpHandler::_createSurfaceField(const Mdvx::field_header_t in_field_hdr,
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
 * _projectionsMatch() - Checks to see if the projections of the input
 *                       fields match.  If they don't match, we can't do
 *                       the calculations.
 *
 * Returns true if they match, false otherwise.
 */

bool CalcRwpHandler::_projectionsMatch(const bool check_press_field) const
{
  MdvxPjg rnw_proj(_rnwField->getFieldHeader());
  MdvxPjg snow_proj(_snowField->getFieldHeader());
  MdvxPjg tk_proj(_tkField->getFieldHeader());
  MdvxPjg hgt_proj(_hgtField->getFieldHeader());
  
  if (rnw_proj != snow_proj || rnw_proj != tk_proj ||
      rnw_proj != hgt_proj)
    return false;
  
  if (check_press_field)
  {
    MdvxPjg press_proj(_mdvx.getField(4)->getFieldHeader());
    
    if (rnw_proj != press_proj)
      return false;
  }
  
  return true;
}

