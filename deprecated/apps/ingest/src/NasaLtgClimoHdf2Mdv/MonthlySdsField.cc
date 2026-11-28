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
 * @file MonthlySdsField.cc
 *
 * @class MonthlySdsField
 *
 * Class representing a monthly climotology SDS field.
 *  
 * @date 11/18/2008
 *
 */

#include <iostream>

#include <Mdv/MdvxPjg.hh>
#include <toolsa/str.h>

#include "MonthlySdsField.hh"

using namespace std;


// Global constants

const int MonthlySdsField::CLIMO_YEAR = 2003;
const int MonthlySdsField::CLIMO_DAY = 15;
const int MonthlySdsField::CLIMO_HOUR = 12;
const int MonthlySdsField::CLIMO_MINUTE = 0;
const int MonthlySdsField::CLIMO_SECOND = 0;

/*********************************************************************
 * Constructors
 */

MonthlySdsField::MonthlySdsField(const string &sds_field_name,
				 const string &prog_name,
				 const string &output_url,
				 const bool debug_flag,
				 const bool verbose_flag) :
  SdsDataField(sds_field_name, prog_name, output_url, debug_flag, verbose_flag)
{
}


/*********************************************************************
 * Destructor
 */

MonthlySdsField::~MonthlySdsField()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _checkDimensions()
 */

bool MonthlySdsField::_checkDimensions(const int num_lats, const int num_lons,
				       const vector< int > &dimensions) const
{
  static const string method_name = "MonthlySdsField::_checkDimensions()";
  
  // Check the number of dimensions

  if (dimensions.size() != NUM_DIMENSIONS)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong number of dimensions in the SDS data array for field: "
	 << _sdsFieldName << endl;
    cerr << "Expected 3 dimensions" << endl;
    cerr << "Found " << dimensions.size() << " dimensions" << endl;
    
    return false;
  }
  
  // Check the number of latitudes

  if (dimensions[LAT_DIM_NUMBER] != num_lats)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong number of latitudes in SDS data array for field: "
	 << _sdsFieldName << endl;
    cerr << "Expected " << num_lats << " latitudes" << endl;
    cerr << "Found " << dimensions[LAT_DIM_NUMBER] << " latitudes" << endl;
   
    return false;
  }
  
  // Check the number of longitudes

  if (dimensions[LON_DIM_NUMBER] != num_lons)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong number of longitudes in SDS data array for field: "
	 << _sdsFieldName << endl;
    cerr << "Expected " << num_lons << " longitudes" << endl;
    cerr << "Found " << dimensions[LON_DIM_NUMBER] << " longitudes" << endl;
    
    return false;
  }
  
  // Check the number of months

  if (dimensions[MONTH_DIM_NUMBER] != 12)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong number of months in SDS data array for field: "
	 << _sdsFieldName << endl;
    cerr << "Expected 12 months" << endl;
    cerr << "Found " << dimensions[MONTH_DIM_NUMBER] << " months" << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _createClimoField()
 */

MdvxField *MonthlySdsField::_createClimoField(const float64 *raw_data,
					      const Pjg &raw_proj,
					      const int month_index,
					      const string &field_name_long,
					      const string &field_units,
					      const double missing_data_value) const
{
  static const string method_name = "MonthlySdsField::_createClimoField()";
  
  // Create the field header

  MdvxPjg mdv_proj(raw_proj);
  
  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  mdv_proj.syncToFieldHdr(field_hdr);
  
  field_hdr.nz = 1;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = missing_data_value;
  field_hdr.missing_data_value = missing_data_value;
  STRcopy(field_hdr.field_name_long, field_name_long.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, _sdsFieldName.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, field_units.c_str(), MDV_UNITS_LEN);

  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.0;
  
  // Create the blank field

  MdvxField *field;
  
  if ((field = new MdvxField(field_hdr, vlevel_hdr, (void *)0)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output MDV field" << endl;
    
    return 0;
  }
  
  // Set the data values in the field

  fl32 *mdv_data = (fl32 *)field->getVol();
  
  for (int lat_num = 0; lat_num < raw_proj.getNy(); ++lat_num)
  {
    for (int lon_num = 0; lon_num < raw_proj.getNx(); ++lon_num)
    {
      int raw_index = (lat_num * 12 * raw_proj.getNx()) +
	(lon_num * 12) + month_index;
      int mdv_index = (lat_num * raw_proj.getNx()) + lon_num;
      
      mdv_data[mdv_index] = (fl32)raw_data[raw_index];
    } /* endfor - lon_num */
  } /* endfor - lat_num */
  
  return field;
}


/*********************************************************************
 * _createMdvFiles()
 */

bool MonthlySdsField::_createMdvFiles(const string &input_path,
				      const float64 *raw_data,
				      const Pjg &raw_proj,
				      const vector< int > &dimensions,
				      const string &field_name_long,
				      const string &field_units,
				      const double missing_data_value)
{
  static const string method_name = "MonthlySdsField::_createMdvFiles()";
  
  // There will be an output file for each month of the year

  for (int month = 1; month <= 12; ++month)
  {
    if (_verbose)
      cerr << "Processing month " << month << endl;
    
    // Create the output file

    DsMdvx out_mdvx;
    _setMasterHeader(out_mdvx, month, input_path);
    
    // Create the climo field and add it to the output file

    MdvxField *climo_field;
    
    if ((climo_field = _createClimoField(raw_data, raw_proj, month-1,
					 field_name_long, field_units,
					 missing_data_value)) == 0)
      continue;
    
    climo_field->convertType(Mdvx::ENCODING_INT8,
			     Mdvx::COMPRESSION_BZIP,
			     Mdvx::SCALING_DYNAMIC);
    
    out_mdvx.addField(climo_field);
    
    // Write the output file

    if (out_mdvx.writeToDir(_outputUrl) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing output file to URL: " << _outputUrl << endl;
      cerr << "Output month: " << month << endl;
      cerr << out_mdvx.getErrStr() << endl;
      
      continue;
    }
    
  } /* endfor - month */
  
  return true;
}


/*********************************************************************
 * _setMasterHeader()
 */

void MonthlySdsField::_setMasterHeader(DsMdvx &out_mdvx,
				       const int month,
				       const string &input_path)
{
  static const string method_name = "MonthlySdsField::_setMasterHeader()";
  
  // Create the time information

  DateTime start_time(CLIMO_YEAR, month, 1, 0, 0, 0);
  DateTime end_time(CLIMO_YEAR, month+1, 1, 0, 0, 0);
  DateTime centroid_time(CLIMO_YEAR, month, CLIMO_DAY,
			 CLIMO_HOUR, CLIMO_MINUTE, CLIMO_SECOND);
  
  // Fill in the header

  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = start_time.utime();
  master_hdr.time_end = end_time.utime() - 1;
  master_hdr.time_centroid = centroid_time.utime();
  master_hdr.time_expire = master_hdr.time_end;
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_CLIMO_OBS;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  STRcopy(master_hdr.data_set_info, _progName.c_str(), MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, _progName.c_str(), MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, input_path.c_str(), MDV_NAME_LEN);
  
  out_mdvx.setMasterHeader(master_hdr);
}
