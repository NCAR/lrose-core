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
// GribRecord - Main class for manipulating GRIB records.
//
////////////////////////////////////////////

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>

#include <grib/constants.h>
#include <grib/GribRecord.hh>
#include <grib/EquidistantCylind.hh>
#include <grib/PolarStereographic.hh>
#include <toolsa/file_io.h>

using namespace std;

GribRecord::GribRecord()
{
  // Use all of the default sections so don't change anything here yet.
  
  _gds = new GDS();
  
  // Set the initial record size

  _resetRecordSize();
}

GribRecord::~GribRecord() 
{
  delete _gds;
}

const fl32 *GribRecord::getData(Pjg &projection,
				GribVertType &vert_type) const
{
  projection = _gds->getProjection();
  vert_type = _pds.getVertType();
  
  return _bds.getData();
}
  

void GribRecord::setData(const Pjg &projection,
			 const GribVertType::vert_type_t vert_level_type,
			 const int vert_level_value_top,
			 const int vert_level_value_bottom,
			 const double min_data_value,
			 const double max_data_value,
			 const int precision,
			 const int max_bit_len,
			 fl32 *new_data,
			 ui08 *bitmap)
{
  _pds.setGdsUsed(true);

  if((projection.getProjType() == PjgTypes::PROJ_LC1) ||
     (projection.getProjType() == PjgTypes::PROJ_LC2))
  {
    // do nothing
  }
  else if((projection.getProjType() == PjgTypes::PROJ_POLAR_STEREO) ||
	  (projection.getProjType() == PjgTypes::PROJ_STEREOGRAPHIC))
  {
    delete _gds;
    _gds = new PolarStereographic();    
  }
  else if((projection.getProjType() == PjgTypes::PROJ_LATLON) ||
	  (projection.getProjType() == PjgTypes::PROJ_CYL_EQUIDIST))
  {
    delete _gds;
    _gds = new EquidistantCylind();
  }

  assert(_gds != 0);

  _gds->setProjection(projection);
  _gds->setRegular(projection.getNy(), projection.getNx());	// assumes regular grid
  _pds.setVertLevel(vert_level_type,
		    vert_level_value_top, vert_level_value_bottom);
  _pds.setDecimalScale(precision);
  
  if (bitmap != 0)
  {
    _pds.setBmsUsed(true);
    _bms.setBitmap(bitmap, projection.getNx() * projection.getNy());
  }
    
  _bds.setData(new_data,
	       projection.getNx() * projection.getNy(),
	       min_data_value, max_data_value, _pds.getDecimalScale(), max_bit_len);

  _resetRecordSize();
}
  

// void GribRecord::setDataScaling(const int decimal_scale,
// 				const int bit_len)
// {
//  _pds.setDecimalScale(decimal_scale);
//  _bds.setOutputBitLen(bit_len);

//  _resetRecordSize();
// }
  
  
int GribRecord::unpack(ui08 **file_ptr)
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
    if ((return_value = _gds->unpack(section_ptr))
	!= GRIB_SUCCESS)
    {
      cerr << "Error unpacking GDS" << endl;
      return return_value;
    }
    
    int proj_type = _gds->getProjType();
    
    if (proj_type == GDS::EQUIDISTANT_CYL_PROJ_ID || proj_type == GDS::GAUSSIAN_LAT_LON_PROJ_ID)
    {
      delete _gds;

      EquidistantCylind *gds = new EquidistantCylind();
      gds->unpack(section_ptr);

      _gds = gds;
    }
    else if (proj_type == GDS::POLAR_STEREOGRAPHIC_PROJ_ID)
    {
      delete _gds;

      PolarStereographic *gds = new PolarStereographic();
      gds->unpack(section_ptr);

      _gds = gds;
    }
    
    section_ptr += _gds->getSize();
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
				  _gds->getGridDim(),
				  _pds.getDecimalScale(),
				  _gds->getProjection().getNx(),
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

ui08 *GribRecord::pack()
{
  static const string method_name = "GribRecord::pack()";
  
  // Pack the Binary Data Section into a temporary buffer.  This must
  // be done first  so that we get the actual packed data size so that
  // the correct total message size gets into the Indicator Section.
  // The buffer will be allocated at the current BDS size, which is the
  // unpacked size, but will be filled to the actual packed size.

  ui08 *packed_bds = new ui08[_bds.getSize()];
  memset(packed_bds, 0, _bds.getSize());
  
  ui08 *bitmap = 0;
  
  if (_pds.bmsUsed())
    bitmap = _bms.getBitMap();
  
  if (_bds.pack(packed_bds, bitmap) != GRIB_SUCCESS)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error packing Binary Data Section" << endl;
    
    delete [] packed_bds;
    
    return 0;
  }
  
  _resetRecordSize();

  // Allocate space for the GRIB message.  Note that at this point
  // we are actually allocating a larger buffer than is truly needed
  // since the BDS section could get smaller as it is packed.

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
    if (_gds->pack(section_ptr) != GRIB_SUCCESS)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error packing Grid Description Section" << endl;
      
      delete [] grib_contents;
      
      return 0;
    }
  
    section_ptr += _gds->getSize();
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
  
  // Copy the packed BDS into the message

  memcpy(section_ptr, packed_bds, _bds.getSize());

  section_ptr += _bds.getSize();
  delete [] packed_bds;

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

void GribRecord::print(FILE *stream, const bool print_bitmap_flag,
		       const bool print_data_flag,
		       const bool print_min_datum_flag,
		       const bool print_max_datum_flag) const
{
  _is.print(stream);
  _pds.print(stream);
  _gds->print(stream);
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


void GribRecord::print(ostream &stream, const bool print_bitmap_flag,
		       const bool print_data_flag,
		       const bool print_min_datum_flag,
		       const bool print_max_datum_flag) const
{
  _is.print(stream);
  _pds.print(stream);
  _gds->print(stream);
  _bms.print(stream, print_bitmap_flag);
  _bds.print(stream);
  if (print_min_datum_flag)
    stream << "Minimum data value = " << _bds.getMinDataValue() << endl;
  if (print_max_datum_flag)
    stream << "Maximum data value = " << _bds.getMaxDataValue() << endl;
  if (print_data_flag)
    _bds.printData(stream);
  _es.print(stream);
}
