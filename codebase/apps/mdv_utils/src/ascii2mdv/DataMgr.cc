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
////////////////////////////////////////////////////////////////////////////////
//
//  Ascii ingest and mdv/grid data output
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  June 1999
//
//  $Id: DataMgr.cc,v 1.13 2019/03/04 00:22:25 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <cstdio>
#include <cstdlib>
#include <sys/time.h>

#include <toolsa/MsgLog.hh>
#include <toolsa/DateTime.hh>

#include "Ascii2Mdv.hh"
#include "DataMgr.hh"

using namespace std;


const int DataMgr::HEADER_LINE_LEN = 1000;


DataMgr::DataMgr() :
  _reverseOrder(false),
  _timeSpecified(false),
  _numHeaderLines(0),
  _forceIntegralScaling(false),
  _dataTime(0)
{
}

DataMgr::~DataMgr()
{
}

int DataMgr::init(Params &params)
{
  POSTMSG( DEBUG, "Initializing output grid and mdvFile." );

  // Get the data time from the parameter file.  If no time is
  // specified, use the current time.

  if (ISEMPTY(params.data_time))
  {
    // No specified data time

    _timeSpecified = false;
    _dataTime = time( NULL );
    POSTMSG(DEBUG, "No data time specified -- using current time");
  }
  else
  {
    // User specified a data time

    _timeSpecified = true;
    _dataTime  = DateTime::parseDateTime( params.data_time );
    POSTMSG(DEBUG, "Using output data time: %s", params.data_time);
  }

  // Setup the output mdv file

  Mdvx::master_header_t master_hdr;
  
  _initMasterHeader(master_hdr,
		    _dataTime,
		    _projection);
  
  _outputMdv.setMasterHeader(master_hdr);
  
   if ( params.force_integral_scaling )
     _forceIntegralScaling = true;

  _outputDir = params.output_data_dir;

  // See if we need to reverse the order of the input data

  if (params.input_origin == Params::UPPER_LEFT)
  {
    POSTMSG(DEBUG, "Setting flag to reverse order of data");
    _reverseOrder = true;
  }

  // Save the number of header lines to skip

  _numHeaderLines = params.num_header_lines;
   
  // Save the field information

  _fieldName = params.mdv_field_name;
  _dataUnits = params.data_units;
  _missingDataValue = params.missing_value;
   
  switch (params.grid_projection)
  {
  case Params::FLAT :
    _projection.initFlat(params.grid_origin_lat,
			 params.grid_origin_lon,
			 params.grid_rotation,
			 params.grid_nx,
			 params.grid_ny,
			 params.grid_nz,
			 params.grid_dx,
			 params.grid_dy,
			 params.grid_dz,
			 params.grid_minx,
			 params.grid_miny,
			 params.grid_minz);
    break;
     
  case Params::LATLON :
    _projection.initLatlon(params.grid_nx,
			   params.grid_ny,
			   params.grid_nz,
			   params.grid_dx,
			   params.grid_dy,
			   params.grid_dz,
			   params.grid_minx,
			   params.grid_miny,
			   params.grid_minz);
    break;
  } /* endswitch - params.grid_projection */
   
  return 0;
}

int DataMgr::format2mdv(const char *input_file_path)
{

  Path    input_path(input_file_path);


  // Clear out any old data

  POSTMSG( INFO, "Processing file '%s'", input_file_path );

  _outputMdv.clearFields();

  Mdvx::master_header_t master_hdr = _outputMdv.getMasterHeader();
  STRcopy(master_hdr.data_set_source, input_file_path, MDV_NAME_LEN);
  _outputMdv.setMasterHeader(master_hdr);
   
  MdvxField *output_field =
    _createField("Formatted ascii data: " + _fieldName,
		 _fieldName,
		 _dataUnits,
		 _dataTime,
		 _projection,
		 _missingDataValue);
  
  // Open the ascii input file as a stream

  PMU_auto_register("fetching input data");
  ifstream input_file(input_file_path);
  if (!input_file.is_open())
  {
    POSTMSG(ERROR, "Unable to open input ascii file '%s'", input_file_path);
    return -1;
  }

  // Skip any header lines

  char *header_line = new char[HEADER_LINE_LEN];
   
  for (int i = 0; i < _numHeaderLines; ++i)
  {
    input_file.getline(header_line, HEADER_LINE_LEN);
    POSTMSG( DEBUG, "Skipping header line: %s", header_line);
  }
   
  delete[] header_line;
   
  // Read in the input data from the ascii file

  fl32 *output_data = (fl32 *)output_field->getVol();
   
  float input_value;
  int num_values_read = 0;
   
  int nx = _projection.getNx();
  int ny = _projection.getNy();
  int nz = _projection.getNz();
   
  for (int z = 0; z < nz; ++z)
  {
    for (int input_y = 0; input_y < ny; ++input_y)
    {
      int y;
       
      if (_reverseOrder)
	y = _projection.getNy() - input_y - 1;
      else
	y = input_y;
       
      for (int x = 0; x < nx; ++x)
      {
	if (!(input_file >> input_value))
	{
	  POSTMSG(WARNING,
		  "Unexpected number of input values (%d) for output "
		  "grid specification (%d x %d x %d) in file '%s'.",
		  num_values_read,
		  nx, ny, nz, input_file_path);
	  return -1;
	}
	 
	num_values_read++;
	 
	int index = x + (y * nx) + (z * nx * ny);
	output_data[index] = input_value;
	 
      }  /* endfor - x */
    }  /* endfor - input_y */
  }  /* endfor - z */
   
  // Make sure the geometry of the input grid 
  // matches that specified for the output grid
  
  int i = 0;
  for (i = 0; input_file >> input_value; ++i);
  if (i)
    POSTMSG(WARNING, "Extra %d values in ascii input file '%s'.",
	    i, input_file_path );

  // Add the new field to the output file

  if (_forceIntegralScaling)
    output_field->convertType(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_BZIP,
			      Mdvx::SCALING_INTEGRAL);
  else
    output_field->convertType(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_BZIP,
			      Mdvx::SCALING_DYNAMIC);
			     
  _outputMdv.addField(output_field);
   
  // If no data time was specified,
  //    write the mdv output file to the same name as the input file

  PMU_auto_register( "writing mdv output" );

  if (_timeSpecified)
  {
    // Write the mdv output file according to its data time

    POSTMSG( DEBUG, "About to write output file for time '%s'", 
	     DateTime::str(_dataTime).c_str() );
     
    if (_outputMdv.writeToDir(_outputDir) != 0)
    {
      POSTMSG( ERROR, "Unable to write mdv output." );
      return -1;
    }
  }
  else
  {
    string output_path = _outputDir + "/" + input_file_path;
     
    POSTMSG( DEBUG, "About to write output file '%s'", 
	     output_path.c_str() );

    if (_outputMdv.writeToPath(output_path) != 0)
    {
      POSTMSG( ERROR, "Unable to open output mdv file '%s'", 
	       output_path.c_str() ); 
      return -1;
    }

  }

  return 0;
}


MdvxField *DataMgr::_createField(const string &field_name_long,
				 const string &field_name,
				 const string &units,
				 const time_t data_time,
				 const MdvxPjg &projection,
				 const float missing_data_value)
{
  // Create the field header

  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.forecast_delta = 0;
  field_hdr.forecast_time = data_time;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  field_hdr.dz_constant = 1;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = missing_data_value;
  field_hdr.missing_data_value = missing_data_value;
  STRcopy(field_hdr.field_name_long, field_name_long.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  
  // Fill in the projection information in the field header.  Note
  // that this must be done after data_element_nbytes is set above
  // or the volume_size value in the header will be wrong.

  projection.syncToFieldHdr(field_hdr);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  for (int z = 0; z < projection.getNz(); ++z)
  {
    vlevel_hdr.type[z] = Mdvx::VERT_TYPE_Z;
    vlevel_hdr.level[z] = projection.getMinz() +
      (z * projection.getDz());
  }
  
  // Create the new field.  Don't bother setting the field values
  // to the missing data value because we will be over-writing all
  // of the values from the values read from the ASCII file.

  return new MdvxField(field_hdr, vlevel_hdr);
}


void DataMgr::_initMasterHeader(Mdvx::master_header_t &master_hdr,
				const time_t data_time,
				const MdvxPjg &projection)
{
  // First initialize everything to 0

  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = data_time;
  master_hdr.time_end = data_time;
  master_hdr.time_centroid = data_time;
  master_hdr.time_expire = data_time;
  
  if (projection.getNz() > 0)
    master_hdr.data_dimension = 3;
  else
    master_hdr.data_dimension = 2;
  
  master_hdr.data_collection_type = Mdvx::DATA_MIXED;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.sensor_lon = projection.getOriginLon();
  master_hdr.sensor_lat = projection.getOriginLat();
  STRcopy(master_hdr.data_set_info, "Formatted ascii data", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "ascii2mdv", MDV_NAME_LEN);
  
}
