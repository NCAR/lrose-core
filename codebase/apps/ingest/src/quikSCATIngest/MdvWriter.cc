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
 * MdvWriter: Class that writes observation information to an MDV file.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2006
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <cstdio>
#include <string>

#include <Mdv/MdvxPjg.hh>
#include "MdvWriter.hh"
#include <physics/physics.h>
#include <physics/PhysicsLib.hh>
#include <toolsa/str.h>

using namespace std;

// Global variables

const fl32 MdvWriter::MISSING_DATA_VALUE = -9999.0;

const string MdvWriter::WIND_SPEED_FIELD_NAME = "windSpeedField";
const string MdvWriter::WIND_SPEED_UNITS_NAME = "m/s";

const string MdvWriter::WIND_DIRECTION_FIELD_NAME = "windDirectionField";
const string MdvWriter::WIND_DIRECTION_UNITS_NAME = "degrees";

const string MdvWriter::U_FIELD_NAME = "uField";
const string MdvWriter::U_UNITS_NAME = "degrees";

const string MdvWriter::V_FIELD_NAME = "vField";
const string MdvWriter::V_UNITS_NAME = "degrees";

const string MdvWriter::RAIN_FLAG_FIELD_NAME = "rainField";
const string MdvWriter::RAIN_FLAG_UNITS_NAME = "None";

const string MdvWriter::NSOL_FLAG_FIELD_NAME = "nsolField";
const string MdvWriter::NSOL_FLAG_UNITS_NAME = "None";



/*********************************************************************
 * Constructors
 */

MdvWriter::MdvWriter(const string &output_url,
		     const int expire_secs,
		     const MdvxPjg &projection,
		     const bool debug_flag) :
  Writer(output_url, expire_secs, debug_flag),
  _windSpeedField(0),
  _windDirectionField(0),
  _uField(0),
  _vField(0),
  _rainFlagField(0),
  _nsolFlagField(0),
  _projection(projection),
  _startTime(DateTime::NEVER),
  _endTime(DateTime::NEVER),
  _windSpeedDataPtr(0),
  _windDirDataPtr(0),
  _uDataPtr(0),
  _vDataPtr(0),
  _rainFlagDataPtr(0),
  _nsolFlagDataPtr(0)

{
}

  
/*********************************************************************
 * Destructor
 */

MdvWriter::~MdvWriter()
{
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvWriter::init()
{
  static const string method_name = "MdvWriter::init()";
  return true;
}


/*********************************************************************
 * addInfo(quikSCATObs obs) - Add observation information
 *
 * Returns true on success, false otherwise.
 */

bool MdvWriter::addInfo(const quikSCATObs &obs)
{

  static const string method_name = "MdvWriter::addInfo()";

  //_startTime and _endTime are global variables (of DateTime type hopefully initialized to DateTime::NEVER).
  // First, check to see if they are still set to DateTime::NEVER. If so, set them to the obs time.
  // If not, check to see if _startTime is less than the obs time and if _endTime is greater than the obs time.

  if (obs.getObsTime() <= 0)
    {
      cerr << "WARNING: obs.getObsTime(): " << obs.getObsTime() << endl;
      cerr << "Skipping this observation since the time is not valid." << endl;
      return true;
    }

  if (_startTime == DateTime::NEVER)
    {
      _startTime = obs.getObsTime();
    }
  else
    {
      if (_startTime > obs.getObsTime())
	{
	  _startTime = obs.getObsTime();
	}
    }

  if (_endTime == DateTime::NEVER)
    {
      _endTime = obs.getObsTime();
    }
  else
    {
      if (_endTime < obs.getObsTime())
	_endTime = obs.getObsTime();
    }

  // Begin by checking to see if the fields exist - if not, call createField()
  if (_windSpeedField == 0 || _windDirectionField == 0 || _uField == 0 || _vField == 0 || _rainFlagField == 0 || _nsolFlagField == 0)
    if (!_createFields())
      {
        cerr << "ERROR: " << method_name << endl;
        cerr << "Error creating mdv fields. Really cannot continue." << endl;
        return false;
      }

  // Now that I have the mdv fields, I need a pointer to the data in those fields.
  // getVol() returns this pointer.
  _windSpeedDataPtr = (fl32*)_windSpeedField->getVol();
  _windDirDataPtr = (fl32*)_windDirectionField->getVol();
  _uDataPtr = (fl32*)_uField->getVol();
  _vDataPtr = (fl32*)_vField->getVol();
  _rainFlagDataPtr = (fl32*)_rainFlagField->getVol();
  _nsolFlagDataPtr = (fl32*)_nsolFlagField->getVol();

  // Normalize the longitude.
  double lon = obs.getLongitude();
  double minx = _projection.getMinx();

  while (lon < minx)
    lon += 360.0;
  while (lon >= minx + 360.0)
    lon -= 360.0;

  // To determine where in space the data point is, use latlon2arrayIndex()
  // Check results from latlon2array index - needs to be a positive or a zero (non-negative)
  // -1 means it is outside of the grid.

  int array_index;

  // add a check here: lat greater than 90, less than -90 and lon 0 to 360 or -180 to +180 (check the raw files)

  //  if (_projection.latlon2arrayIndex(obs.getLatitude(), obs.getLongitude(), array_index) != 0)
  if (_projection.latlon2arrayIndex(obs.getLatitude(), lon, array_index) != 0)
    {
      if (_debug)
	{
	  cerr << "array_index: " << array_index << endl;
	  cerr << "obs.getLatitude(): " << obs.getLatitude() << endl;
	  cerr << "obs.getLongitude(): " << obs.getLongitude() << endl;
	  cerr << "Normalized longitude: " << lon << endl;
	}

      if (_debug)
	cerr << "WARNING: quikSCATObs lat/lon is outside of projection: "
	     << obs.getLatitude() << ", " << lon << endl;
      return true;
    }
  else
    {
      if (_debug)
	cerr << "Good data?!" << endl;
    }

  _windSpeedDataPtr[array_index] = obs.getWindSpeed();
  _windDirDataPtr[array_index] = obs.getWindDir();
  _uDataPtr[array_index] = PHYwind_u(obs.getWindSpeed(), obs.getWindDir());
  _vDataPtr[array_index] = PHYwind_v(obs.getWindSpeed(), obs.getWindDir());
  _rainFlagDataPtr[array_index] = obs.getRainFlag();
  _nsolFlagDataPtr[array_index] = obs.getNsolFlag();

  return true;
}


/*********************************************************************
 * writeInfo() - Write the observation information.
 *
 * Returns true if the write was successful, false otherwise.
 */

bool MdvWriter::writeInfo()
{
  static const string method_name = "MdvWriter::writeInfo()";

  cerr << " LOCATION CHECK: " << method_name << endl;

  DsMdvx output_mdv;

  // Call _initializeMasterHeader() here
  // Need start and end times in order to fill out master header.
  // Make sure that number of fields is set to zero.
  _initializeMasterHeader(output_mdv);

  // Compress the data fields

  _windSpeedField->convertType(Mdvx::ENCODING_INT8,
			       Mdvx::COMPRESSION_BZIP,
			       Mdvx::SCALING_DYNAMIC);
  
  _windDirectionField->convertType(Mdvx::ENCODING_INT8,
				   Mdvx::COMPRESSION_BZIP,
				   Mdvx::SCALING_DYNAMIC);
  
  _uField->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_BZIP,
		       Mdvx::SCALING_DYNAMIC);
  
  _vField->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_BZIP,
		       Mdvx::SCALING_DYNAMIC);
  
  _rainFlagField->convertType(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_BZIP,
			      Mdvx::SCALING_DYNAMIC);
  
  _nsolFlagField->convertType(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_BZIP,
			      Mdvx::SCALING_DYNAMIC);
  
  // DsMdvx object takes control of the pointers. I need to set to zero but I do not delete them.
  // See include/Mdv/Mdvx.hh for addField()
  output_mdv.addField(_windSpeedField);
  output_mdv.addField(_windDirectionField);
  output_mdv.addField(_uField);
  output_mdv.addField(_vField);
  output_mdv.addField(_rainFlagField);
  output_mdv.addField(_nsolFlagField);

  _windSpeedField = 0;
  _windDirectionField = 0;
  _uField = 0;
  _vField = 0;
  _rainFlagField = 0;
  _nsolFlagField = 0;

  _windSpeedDataPtr = 0;
  _windDirDataPtr = 0;
  _uDataPtr = 0;
  _vDataPtr = 0;
  _rainFlagDataPtr = 0;
  _nsolFlagDataPtr = 0;

  // Re-set start and end times to DateTime::NEVER
  _startTime = DateTime::NEVER;
  _endTime = DateTime::NEVER;

  // See include/Mdv/Mdvx_write.hh
  output_mdv.setWriteLdataInfo();
  if (output_mdv.writeToDir(_outputUrl) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing output MDV file to URL: "
	   << _outputUrl << endl;
      return false;
    }
  output_mdv.clearFields();
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _createMdvField() - Create a blank MDV field from the given information.
 *
 * Returns a pointer to the new field on success, 0 on failure.
 */

MdvxField *MdvWriter::_createMdvField(const string &field_name,
				      const string &field_name_long,
				      const string &units) const
{
  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  MdvxPjg mdv_proj(_projection);

  // See include/Mdv/Mdvx_typedefs.hh for field_header_t
  field_hdr.field_code = 0;
  field_hdr.forecast_delta = 0;
  field_hdr.forecast_time = 0;
  field_hdr.nz = 1;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;   /*****/
  field_hdr.vlevel_type = field_hdr.native_vlevel_type;
  field_hdr.dz_constant = true;
  field_hdr.data_dimension = 2;
  field_hdr.grid_dz = 1.0;
  field_hdr.grid_minz = 0.5;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  // MISSING_DATA_VALUE is set to -9999.0. I do not differentiate between bad or missing.
  field_hdr.bad_data_value = MISSING_DATA_VALUE;
  field_hdr.missing_data_value = MISSING_DATA_VALUE;
  STRcopy(field_hdr.field_name_long, field_name_long.c_str(),
          MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  
  // syncToFieldHdr() will fill in things related to x/y or projection. Make sure volume_size gets set and is not left at zero.
  mdv_proj.syncToFieldHdr(field_hdr);
  
  // See include/Mdv/Mdvx_typedefs.hh
  // Create the vlevel header. I only deal with type and level. In my case, the type is VERT_TYPE_SURFACE
  // 1 vlevel - this is an array of type and height - in my case it's of type surface. Set value to zero (or 0.5)
  // Mdv/Mdvx_typedefs.hh
  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.5;

  // look at constructor for MdvxField - I want a field that is filled w/ missing data values.
  return new MdvxField(field_hdr, vlevel_hdr,
                       (void *)0, true, false);
}


/*********************************************************************
 * _initializeMasterHeader() - Initialize the master header in the output
 *                             MDV file.
 */

void MdvWriter::_initializeMasterHeader(DsMdvx &output_mdv) const
{
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));

  // See include/Mdv/Mdvx_typedefs.hh for master_header_t

  master_hdr.time_begin = _startTime.utime();
  master_hdr.time_end = _endTime.utime();
  master_hdr.time_centroid = (master_hdr.time_begin / 2) + (master_hdr.time_end / 2); // This is the average of the begin and end times.
  if (_debug)
    {
      DateTime cutoff_time(2000, 1, 1);
      if (cutoff_time > master_hdr.time_centroid)
	{
	  cerr << "*** Bad centroid time:" << master_hdr.time_centroid << endl;
	  cerr << " master_hdr.time_begin: " << master_hdr.time_begin << endl;
	  cerr << " master_hdr.time_end: " << master_hdr.time_end << endl;
	}

    }
  master_hdr.time_gen = master_hdr.time_centroid;  //same as time_centroid or do a time call to get current time
  master_hdr.data_dimension = 2;   // I only have 1 vertical level.
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  STRcopy(master_hdr.data_set_info, "Generated by quikSCATIngest", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "quikSCATIngest", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, "quikSCATIngest", MDV_NAME_LEN);
  
  output_mdv.setMasterHeader(master_hdr);
}

/*********************************************************************
 * _createFields() - 
 *
 * Returns true if the fields were successfully created. False otherwise.
 */

bool MdvWriter::_createFields()
{
  static const string method_name = "MdvWriter::_createFields()";

  // createMdvField() returns a pointer to the new field upon success; 0 on failure.
  // If the first field is successfully created, then create the second. If that goes well, return true.
  cerr << " LOCATION CHECK: " << method_name << endl;

  _windSpeedField = _createMdvField(WIND_SPEED_FIELD_NAME, WIND_SPEED_FIELD_NAME, WIND_SPEED_UNITS_NAME);
  _windDirectionField = _createMdvField(WIND_DIRECTION_FIELD_NAME, WIND_DIRECTION_FIELD_NAME, WIND_DIRECTION_UNITS_NAME);
  _uField = _createMdvField(U_FIELD_NAME, U_FIELD_NAME, U_UNITS_NAME);
  _vField = _createMdvField(V_FIELD_NAME, V_FIELD_NAME, V_UNITS_NAME);
  _rainFlagField = _createMdvField(RAIN_FLAG_FIELD_NAME, RAIN_FLAG_FIELD_NAME, RAIN_FLAG_UNITS_NAME);
  _nsolFlagField = _createMdvField(NSOL_FLAG_FIELD_NAME, NSOL_FLAG_FIELD_NAME, NSOL_FLAG_UNITS_NAME);


  // If the fields are not created successfully, delete them, set them to zero, and return false. Cannot continue without fields.
  if (_windSpeedField == 0 || _windDirectionField == 0 || _uField == 0 || _vField == 0 || _rainFlagField == 0 || _nsolFlagField == 0)
    {
      delete _windSpeedField;
      delete _windDirectionField;
      delete _uField;
      delete _vField;
      delete _rainFlagField;
      delete _nsolFlagField;

      _windSpeedField = 0;
      _windDirectionField = 0;
      _uField = 0;
      _vField = 0;
      _rainFlagField = 0;
      _nsolFlagField = 0;

      // Set my data pointers to zero as well. We cannot delete them since they are owned by their respective field ptrs.

      _windSpeedDataPtr = 0;
      _windDirDataPtr = 0;
      _uDataPtr = 0;
      _vDataPtr = 0;
      _rainFlagDataPtr = 0;
      _nsolFlagDataPtr = 0;

      return false;
    }
  else
    {
      return true;
    }
}
