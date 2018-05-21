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
/////////////////////////////////////////////
// gribrecord - Main class for manipulating GRIB records.
//
////////////////////////////////////////////

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>

#include <grib/constants.h>
#include <grib/gribrecord_.hh>
#include <toolsa/file_io.h>

using namespace std;

gribrecord::gribrecord()
{
  // Use all of the default sections so don't change anything here yet.
  
  // Set the initial record size

  _resetRecordSize();
}

gribrecord::~gribrecord() 
{
  if(_gds_ptr != NULL)
    delete _gds_ptr;
}

const fl32 *gribrecord::getData(gds::gds_t &projection,
				GribVertType &vert_type) const
{
  if(_gds_ptr == NULL) {
    cout << "ERROR: Cannot return GDS projection information untill GDS section is unpacked." << endl;
    return NULL;
  }
  projection = _gds_ptr->getProjection();
  vert_type = _pds.getVertType();
  
  return _bds.getData();
}
  

void gribrecord::setData(const gds::gds_t &projection,
			 const GribVertType::vert_type_t vert_level_type,
			 const int vert_level_value_top,
			 const int vert_level_value_bottom,
			 const double min_data_value, const double max_data_value,
			 const int precision,
			 const int max_bit_len,
			 fl32 *new_data,
			 ui08 *bitmap)
{
  _setGdsClass(projection.gridType);

  _pds.setGdsUsed(true);
  _gds_ptr->setProjection(projection);
  //_gds_ptr->setRegular(projection.ny, projection.nx);	// assumes regular grid
  _pds.setVertLevel(vert_level_type,
		    vert_level_value_top, vert_level_value_bottom);
  _pds.setDecimalScale(precision);
  
  if (bitmap != 0)
  {
    _pds.setBmsUsed(true);
    _bms.setBitmap(bitmap, projection.nx * projection.ny);
  }
    
  _bds.setData(new_data,
	       projection.nx * projection.ny,
	       min_data_value, max_data_value, _pds.getDecimalScale(), max_bit_len);

  _resetRecordSize();
}
  

// void gribrecord::setDataScaling(const int decimal_scale,
// 				const int bit_len)
// {
//  _pds.setDecimalScale(decimal_scale);
//  _bds.setOutputBitLen(bit_len);

//  _resetRecordSize();
// }
  
  
int gribrecord::unpack(ui08 **file_ptr)
{
  ui08 *section_ptr = *file_ptr;
  
  int return_value;
  
  // Unpack the ID Section

  if ((return_value = _is.unpack(section_ptr))
      != GRIB_SUCCESS)
  {
    cerr << "Error unpacking ID section" << endl;
    return return_value;
  }
  
  section_ptr += _is.getSize();
  
  // Unpack the Product Definition Section
  
  if ((return_value = _pds.unpack(section_ptr))
      != GRIB_SUCCESS)
  {
    cerr << "Error unpacking PDS" << endl;
    return return_value;
  }
  
  section_ptr += _pds.getSize();
  
  // Unpack the Grid Description Section

  if (_pds.gdsUsed())
  {
    int gridType = (int)section_ptr[5];
    _setGdsClass(gridType);

    if ((return_value = _gds_ptr->unpack(section_ptr))
	!= GRIB_SUCCESS)
    {
      cerr << "Error unpacking GDS" << endl;
      return return_value;
    }
    
    section_ptr += _gds_ptr->getSize();
  }
  
  // Unpack the Bit-map section

  if (_pds.bmsUsed())
  {
    if ((return_value = _bms.unpack(section_ptr)) != GRIB_SUCCESS)
    {
      cerr << "Error unpacking BMS" << endl;
      return return_value;
    }
    
    section_ptr += _bms.getSize();
  }
  
  // Unpack the Binary Data Section

  if ((return_value = _bds.unpack(section_ptr,
				  _gds_ptr->getGridDim(),
				  _pds.getDecimalScale(),
				  _gds_ptr->getProjection().nx,
				  _bms.getBitMap()))
      != GRIB_SUCCESS)
  {
    cerr << "Error unpacking BDS" << endl;
    return return_value;
  }
  
  section_ptr += _bds.getSize();
  
  // Unpack the End Section
  
  if ((return_value = _es.unpack(section_ptr)) != GRIB_SUCCESS)
  {
    cerr << "Error unpacking End section" << endl;
    return return_value;
  }
  
  section_ptr += _es.getSize();
  
  *file_ptr = section_ptr;
  
  return GRIB_SUCCESS;
}

ui08 *gribrecord::pack()
{
  static const string method_name = "gribrecord::pack()";
  
  int record_size = _is.getTotalSize();
  
  ui08 *grib_contents = new ui08[record_size];
  memset(grib_contents, 0, record_size);
  ui08 *section_ptr = grib_contents;
  
  // Pack the Indicator Section

  if (_is.pack(section_ptr) != GRIB_SUCCESS)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error packing Indicator Section" << endl;
    
    delete [] grib_contents;
    
    return 0;
  }
  
  section_ptr += _is.getSize();
  
  // Pack the Product Definition Section

  if (_pds.pack(section_ptr) != GRIB_SUCCESS)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error packing Product Definition Section" << endl;
    
    delete [] grib_contents;
    
    return 0;
  }
  
  section_ptr += _pds.getSize();
  
  // Pack the Grid Description Section

  if (_pds.gdsUsed())
  {
    if (_gds_ptr->pack(section_ptr) != GRIB_SUCCESS)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error packing Grid Description Section" << endl;
      
      delete [] grib_contents;
      
      return 0;
    }
  
    section_ptr += _gds_ptr->getSize();
  }
  
  // Pack the Bit-map section

  if (_pds.bmsUsed())
  {
    if (_bms.pack(section_ptr) != GRIB_SUCCESS)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error packing Bit-map Section" << endl;
    
      delete [] grib_contents;
      
      return 0;
    }
  
    section_ptr += _bms.getSize();
  }
  
  // Pack the Binary Data Section

  ui08 *bitmap = 0;
  
  if (_pds.bmsUsed())
    bitmap = _bms.getBitMap();
  
  if (_bds.pack(section_ptr, bitmap) != GRIB_SUCCESS)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error packing Binary Data Section" << endl;
    
    delete [] grib_contents;
    
    return 0;
  }
  
  section_ptr += _bds.getSize();

  // Pack the End Section
  
  if (_es.pack(section_ptr) != GRIB_SUCCESS)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error packing End Section" << endl;
    
    delete [] grib_contents;
    
    return 0;
  }
  
  return grib_contents;
}

void gribrecord::print(FILE *stream, const bool print_bitmap_flag,
		       const bool print_data_flag,
		       const bool print_min_datum_flag,
		       const bool print_max_datum_flag) const
{
  _is.print(stream);
  _pds.print(stream);
  _gds_ptr->print(stream);
  _bms.print(stream, print_bitmap_flag);
  _bds.print(stream);
  if (print_min_datum_flag)
    fprintf(stream, "Minimum data value = %f\n", _bds.getMinDataValue());
  if (print_max_datum_flag)
    fprintf(stream, "Maximum data value = %f\n", _bds.getMaxDataValue());
  if (print_data_flag)
    _bds.printData(stream);
  _es.print(stream);
}
