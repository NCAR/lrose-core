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
//////////////////////////////////////////////////////////
// Interest.cc:  Interest values
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////

#include <toolsa/str.h>

#include "Interest.h"

using namespace std;

//
// Constants
//
const double Interest::_BAD_INTEREST_VALUE = -2;
const int Interest::_BAD_HT_VALUE = -2;


//////////////
// Constructor

Interest::Interest(const bool debug_flag) :
  interest_values(0),
  ht(0),
  npoints(0),
  _debug(debug_flag),
  _dbzField(0)
{
}

/////////////
// destructor

Interest::~Interest ()
{
  delete [] interest_values;
  delete [] ht;
}

//////////////////
// addInterestFields
//
// Add the interest fields to the given MDV file.

bool Interest::addInterestFields(DsMdvx &output_file,
				 const char *long_field_name,
				 const char *short_field_name) const
{
  static const string method_name = "Interest::addInterestFields()";
  
  // Set the interest field header values

  Mdvx::field_header_t field_hdr = _dbzField->getFieldHeader();
   
  field_hdr.nz = 1;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.data_dimension = 2;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = _BAD_INTEREST_VALUE;
  field_hdr.missing_data_value = _BAD_INTEREST_VALUE;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  STRcopy(field_hdr.units, "none", MDV_UNITS_LEN);
  STRcopy(field_hdr.transform, "none", MDV_TRANSFORM_LEN);
  STRcopy(field_hdr.field_name_long, long_field_name, MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, short_field_name, MDV_SHORT_FIELD_LEN);

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
   
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_Z;
  vlevel_hdr.level[0] = field_hdr.grid_minz;
   
  MdvxField *interest = new MdvxField(field_hdr, vlevel_hdr,
				      (void *)interest_values);
   
  if (interest == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating MDV interest field" << endl;
     
    return false;
  }
   
  interest->convertType(Mdvx::ENCODING_INT16,
                        Mdvx::COMPRESSION_GZIP);
  
  output_file.addField(interest);
   
  // ht field - initialize field header

  field_hdr = _dbzField->getFieldHeader();
   
  string long_field_name_str = string(long_field_name) + " HEIGHT";
  string short_field_name_str = string(short_field_name) + " HT";
   
  field_hdr.nz = 1;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.data_dimension = 2;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = _BAD_HT_VALUE;
  field_hdr.missing_data_value = _BAD_HT_VALUE;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  STRcopy(field_hdr.units, "km", MDV_UNITS_LEN);
  STRcopy(field_hdr.transform, "none", MDV_TRANSFORM_LEN);
  STRcopy(field_hdr.field_name_long, long_field_name_str.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, short_field_name_str.c_str(),
	  MDV_SHORT_FIELD_LEN);

  fl32 *ht_data = new fl32[field_hdr.nx * field_hdr.ny];

  for (int i = 0; i < field_hdr.nx * field_hdr.ny; ++i)
  {
    if (ht[i] == _BAD_HT_VALUE)
      ht_data[i] = _BAD_HT_VALUE;
    else
      ht_data[i] = ht[i] * _dbzFieldHdr.grid_dz + _dbzFieldHdr.grid_minz;
  } /* endfor - i */

  MdvxField *interest_ht = new MdvxField(field_hdr, vlevel_hdr,
					 (void *)ht_data);
   
  delete [] ht_data;
  
  if (interest_ht == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating MDV interest height field" << endl;
     
    return false;
  }
   
  interest_ht->convertType(Mdvx::ENCODING_INT16,
			   Mdvx::COMPRESSION_GZIP);
  
  output_file.addField(interest_ht);
  
  return true;
}


//////////////////
// _initInterestFields
//
// Initialize the interest fields

void Interest::_initInterestFields(MdvxField *dbz_field)
{
  // Set the global members for use by other methods

  _dbzField = dbz_field;
  _dbzFieldHdr = dbz_field->getFieldHeader();
  
  // set npoints

  int new_npoints = _dbzFieldHdr.nx * _dbzFieldHdr.ny;

  if (new_npoints != npoints)
  {
    npoints = new_npoints;
    
    // allocate space for interest data

    delete [] interest_values;
    delete [] ht;
    
    interest_values = new fl32[npoints];
    ht = new int[npoints];
  }

  for (int i = 0; i < npoints; i++)
  {
    interest_values[i] = _BAD_INTEREST_VALUE;
    ht[i] = _BAD_HT_VALUE;
  }
}
