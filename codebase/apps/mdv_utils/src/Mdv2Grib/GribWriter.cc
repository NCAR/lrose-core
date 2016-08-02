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
//   $Date: 2016/03/04 02:22:09 $
//   $Id: GribWriter.cc,v 1.26 2016/03/04 02:22:09 dixon Exp $
//   $Revision: 1.26 $
//   $State: Exp $
//

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * GribWriter: Class for writing MDV data to a GRIB file.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>

#include <grib/BDS.hh>
#include <grib/GribVertType.hh>
#include <grib/PDS.hh>
#include <Mdv/MdvxPjg.hh>

#include "GribWriter.hh"

using namespace std;

const float GribWriter::FEET_TO_METERS = 0.3048;


/*********************************************************************
 * Constructor
 */

GribWriter::GribWriter(const bool debug_flag, const bool is_forecast_data) :
  _debug(debug_flag),
  _is_forecast_data(is_forecast_data),
  _outputFilePath(""),
  _outputFile(0)
{
}


/*********************************************************************
 * Destructor
 */

GribWriter::~GribWriter()
{
  if (_outputFile != 0)
    fclose(_outputFile);
}


/*********************************************************************
 * openFile() - Open the indicated file for output
 */

bool GribWriter::openFile(const string &file_path)
{
  static const string method_name = "GribWriter::openFile()";

  // Close any old files that haven't been closed

  _outputFilePath = "";

  if (_outputFile != 0)
    fclose(_outputFile);

  // Open the new file

  if ((_outputFile = fopen(file_path.c_str(), "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening file for output" << endl;
    perror(file_path.c_str());

    return false;
  }

  _outputFilePath = file_path;

  return true;
}


/*********************************************************************
 * writeField() - Write the given field to the GRIB file.
 */

bool GribWriter::writeField(const Mdvx::master_header_t &master_hdr,
                            const MdvxField &mdv_field,
                            const DataConverter &converter,
                            const int grib_tables_version,
                            const int originating_center,
                            const int subcenter_id,
                            const int generating_process_id,
                            const int grid_id,
                            const int grib_code,
                            const int precision,
                            const int max_bit_length,
                            const Params::forecast_interval_type_t forecast_interval,
                            const int time_range_id,
                            const bool override_vert_level,
                            const int vert_level_type,
                            const int vert_level_bottom,
                            const int vert_level_top,
                            const double data_addend,
			    const field_type_t field_type,
			    const int accum_secs)
{
  static const string method_name = "GribWriter::writeField()";

  // Make sure that we have a current open file.

  if (_outputFile == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No current open file." << endl;
    cerr << "Must call openFile() method before calling this method" << endl;

    return false;
  }

  // Process the field

  Mdvx::field_header_t field_hdr = mdv_field.getFieldHeader();
  Mdvx::vlevel_header_t vlevel_hdr = mdv_field.getVlevelHeader();

  if (_debug)
  {
    cerr << "---> Processing field: " << field_hdr.field_name_long << endl;
    cerr << "     Grib code: " << grib_code << endl;
    cerr << "     Precision: " << precision << endl;
    cerr << "     Max. bit length: " << max_bit_length << endl;
  }

  // Each level in the field becames a different GRIB record

  mdv_field.setPlanePtrs();

  for (int z = 0; z < field_hdr.nz; ++z)
  {
    // Create the new GRIB record object

    GribRecord grib_record;

    grib_record.setGribTablesVersion(grib_tables_version);
    grib_record.setOriginatingCenter(originating_center);
    grib_record.setSubcenterId(subcenter_id);
    grib_record.setGeneratingProcessId(generating_process_id);

    grib_record.setGribCode(grib_code);

    grib_record.setGridId(grid_id);

    _setTimeValues(grib_record, time_range_id, forecast_interval,
		   field_type, accum_secs,
		   master_hdr, field_hdr);
    
    // Determine the GRIB level information for the data

    GribVertType::vert_type_t grib_level_type;
    int grib_level_value_top, grib_level_value_bottom;

    if (override_vert_level)
    {
      grib_level_type = (GribVertType::vert_type_t)vert_level_type;
      grib_level_value_bottom = vert_level_bottom;
      grib_level_value_top = vert_level_top;
    }
    else
    {
      if (!_convertMdv2GribLevelType(vlevel_hdr.type[z], vlevel_hdr.level[z],
                                     grib_level_type,
                                     grib_level_value_top,
                                     grib_level_value_bottom))
        return false;
    }

    if (_debug)
    {
      cerr << "      grib level type = " << grib_level_type << endl;
      cerr << "      grib level value top = " << grib_level_value_top << endl;
      cerr << "      grib level value bottom = " << grib_level_value_bottom << endl;
    }

    // Create the data array for the GRIB file

    int plane_size = field_hdr.nx * field_hdr.ny;
    fl32 *grib_data = new fl32[plane_size];
    fl32 *mdv_data = (fl32 *)mdv_field.getPlane(z);
    ui08 *bitmap = new ui08[plane_size];
    bool have_missing_data = false;
    double min_plane_value=0.0, max_plane_value=0.0;
    bool data_value_found = false;

    memset(bitmap, 0, plane_size * sizeof(ui08));

    for (int i = 0; i < plane_size; ++i)
    {
      if (mdv_data[i] == field_hdr.missing_data_value ||
          mdv_data[i] == field_hdr.bad_data_value)
      {
        grib_data[i] = BDS::MISSING_DATA;
        have_missing_data = true;
      }
      else
      {
        grib_data[i] = converter.convertValue(mdv_data[i]);
        grib_data[i] += data_addend;

        bitmap[i] = 1;

        if (data_value_found)
        {
          if (min_plane_value > grib_data[i]) min_plane_value = grib_data[i];
          if (max_plane_value < grib_data[i]) max_plane_value = grib_data[i];
        }
        else
        {
          min_plane_value = grib_data[i];
          max_plane_value = grib_data[i];
          data_value_found = true;
        }
      }
    } /* endfor - i */

    MdvxPjg projection(master_hdr, field_hdr);

    if (have_missing_data)
      grib_record.setData(projection,
                          grib_level_type,
                          grib_level_value_top, grib_level_value_bottom,
                          min_plane_value, max_plane_value,
                          precision,
                          max_bit_length,
                          grib_data,
                          bitmap);
    else
      grib_record.setData(projection,
                          grib_level_type,
                          grib_level_value_top, grib_level_value_bottom,
                          min_plane_value, max_plane_value,
                          precision,
                          max_bit_length,
                          grib_data);

    delete [] bitmap;

    // Pack the GRIB record so it's ready for writing

    if (_debug)
      grib_record.print(stderr, false);

    ui08 *packed_grib_record = grib_record.pack();

    if (packed_grib_record == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error packing GRIB record for field: "
           << field_hdr.field_name_long << ", level number: " << z << endl;
      cerr << "Skipping level....." << endl;

      continue;
    }

    int grib_record_size = grib_record.getRecordSize();

    // Write the GRIB record to the output file

    int bytes_written;

    if ((bytes_written = fwrite(packed_grib_record, 1, grib_record_size,
                                _outputFile)) != grib_record_size)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing GRIB record to output file." << endl;
      cerr << "Expected to write " << grib_record_size << " bytes." << endl;
      cerr << "Wrote " << bytes_written << " bytes." << endl;
      perror(_outputFilePath.c_str());

      delete [] packed_grib_record;

      closeFile();

      return false;
    }

    delete [] packed_grib_record;

  } /* endfor - z */

  return true;
}


/*********************************************************************
 * closeFile() - Close the current output file.
 */

void GribWriter::closeFile()
{
  if (_outputFile != 0)
    fclose(_outputFile);

  _outputFile = 0;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _convertGribWriterLevel() - Convert the given MDV vertical level info
 *                           to the equivalent GRIB level info.
 */

bool GribWriter::_convertMdv2GribLevelType(const int mdv_level_type,
                                           const double mdv_level_value,
                                           GribVertType::vert_type_t &grib_level_type,
                                           int &grib_level_value_top,
                                           int &grib_level_value_bottom)
{
  static const string method_name = "GribWriter::_convertMdv2GribLevelType()";

  switch (mdv_level_type)
  {
  case Mdvx::VERT_TYPE_SURFACE :
    grib_level_type = GribVertType::SURFACE;
    grib_level_value_top = 0;
    grib_level_value_bottom = 0;
    break;

  case Mdvx::VERT_TYPE_PRESSURE :
    grib_level_type = GribVertType::ISOBARIC;
    grib_level_value_top = (int)mdv_level_value;
    grib_level_value_bottom = (int)mdv_level_value;
    break;

  case Mdvx::VERT_TYPE_Z :
    grib_level_type = GribVertType::ALTITUDE_ABOVE_MSL;
    grib_level_value_top = (int)(mdv_level_value * 1000.0);
    grib_level_value_bottom = (int)(mdv_level_value * 1000.0);
    break;

  case Mdvx::VERT_FLIGHT_LEVEL :
    // note flight levels are defined as 100's of feet above MSL
    // we want meters abve MSL
    grib_level_type = GribVertType::ALTITUDE_ABOVE_MSL;
    grib_level_value_top = (int)(mdv_level_value * 100.0 * FEET_TO_METERS);
    grib_level_value_bottom = (int)(mdv_level_value * 100.0 * FEET_TO_METERS);
    break;

  case Mdvx::VERT_TYPE_COMPOSITE :
    // this is the closest category to a composite that we could identify in the
    // the GRIB documentation.
    grib_level_type = GribVertType::ENTIRE_ATMOSPHERE;
    grib_level_value_top = 0;
    grib_level_value_bottom = 0;
    break;

  case Mdvx::VERT_TYPE_ZAGL_FT :
    // Convert to meters .. GRIB doesn't like feet.
    grib_level_type = GribVertType::HEIGHT_ABOVE_GROUND;
    grib_level_value_top = (int)(mdv_level_value * FEET_TO_METERS);
    grib_level_value_bottom = (int)(mdv_level_value * FEET_TO_METERS);
    break;
/*
  case Mdvx::VERT_TYPE_SIGMA_P :
    grib_level_type = GribVertType::SIGMA;
    grib_level_value_top = (int)0;
    grib_level_value_bottom = (int)1;
    break;
*/
  default:
    cerr << "WARNING: " << method_name << endl;
    cerr << "Vertical level type " << Mdvx::vertType2Str(mdv_level_type)
         << " not yet implemented" << endl;
    cerr << "Setting vlevel type to SURFACE" << endl;
    grib_level_type = GribVertType::SURFACE;
    grib_level_value_top = 0;
    grib_level_value_bottom = 0;
  }

  return true;

}


/*********************************************************************
 * _setTimeValues() - Set the time values in the grib record.
 */

void GribWriter::_setTimeValues(GribRecord &grib_record,
				const int time_range_id,
				const int forecast_interval,
				const field_type_t field_type,
				const int accum_secs,
				const Mdvx::master_header_t master_hdr,
				const Mdvx::field_header_t field_hdr) const
{
  switch (field_type)
  {
  case FIELD_TYPE_ACCUM :
  case FIELD_TYPE_AVG :
  {
    grib_record.setTimeRangeId(PDS::TIME_RANGE_ACCUMULATION);

    int data_reference_time;
    int time_p1;
    int time_p2;
      
    if (_is_forecast_data)
    {
      if (field_hdr.forecast_delta >= accum_secs)
      {
	data_reference_time = master_hdr.time_gen;
	time_p1 = field_hdr.forecast_delta - accum_secs;
	time_p2 = field_hdr.forecast_delta;
      }
      else
      {
	data_reference_time = master_hdr.time_gen - accum_secs;
	time_p1 = field_hdr.forecast_delta;
	time_p2 = field_hdr.forecast_delta + accum_secs;
      }
    }
    else 
    {
      data_reference_time = master_hdr.time_centroid - accum_secs;
      time_p1 = 0;
      time_p2 = accum_secs;
    }

    switch (forecast_interval)
    {
    case Params::FORECAST_INTERVAL_SECONDS :
      grib_record.setTime(data_reference_time,
			  time_p1, time_p2,
			  PDS::TIME_UNIT_SECONDS);
      break;
      
    case Params::FORECAST_INTERVAL_MINUTES :
      grib_record.setTime(data_reference_time,
			  time_p1/60, time_p2/60,
			  PDS::TIME_UNIT_MINUTES);
      break;
      
    case Params::FORECAST_INTERVAL_HOURS :
      grib_record.setTime(data_reference_time,
			  time_p1/3600, time_p2/3600,
			  PDS::TIME_UNIT_HOURS);
    } /* endswitch - forecast_interval */

    break;
  }

  case FIELD_TYPE_NORMAL :
  {
    grib_record.setTimeRangeId(time_range_id);

    int data_reference_time;
    if (_is_forecast_data)
      data_reference_time = master_hdr.time_gen;
    else 
      data_reference_time = master_hdr.time_centroid;    
    
    switch (forecast_interval)
    {
    case Params::FORECAST_INTERVAL_SECONDS :
      grib_record.setTime(data_reference_time,
			  field_hdr.forecast_delta,
			  PDS::TIME_UNIT_SECONDS);
      break;
      
    case Params::FORECAST_INTERVAL_MINUTES :
      grib_record.setTime(data_reference_time,
			  field_hdr.forecast_delta/60,
			  PDS::TIME_UNIT_MINUTES);
      break;
      
    case Params::FORECAST_INTERVAL_HOURS :
      grib_record.setTime(data_reference_time,
			  field_hdr.forecast_delta/3600,
			  PDS::TIME_UNIT_HOURS);
	break;
    } /* endswitch - forecast_interval */

    break;
  }
  
  } /* endswitch - field_type */
  
}
