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
///////////////////////////////////////////////////////////////
// OutputMdv.cc
//
// Handles output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#include "OutputMdv.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <Mdv/Mdvx.hh>
using namespace std;

//////////////
// Constructor

OutputMdv::OutputMdv(const string &prog_name) : _progName(prog_name)

{

}

/////////////
// destructor

OutputMdv::~OutputMdv()

{
  
}

///////////////////////////////////////////////////
// setRadarPos
//
// Set the radar position in the grid struct
//

void OutputMdv::setRadarPos(const double radar_lat,
                            const double radar_lon,
                            const double radar_ht,
                            const double elev_angle)

{
  _sensorLat = radar_lat;
  _sensorLon = radar_lon;
  _sensorAlt = radar_ht;
  _minAngle = elev_angle;
}

////////////////////////////////////////////////////
// clearVol()
//
// Clears out the volume data, initialize
//

void OutputMdv::clearVol()
  
{
  
  _outputter.clearFields();
  _outputter.clearChunks();

}

////////////////////////////////////////////////////
// loadScaleAndBias()
//
// Set scale and bias for a given field
//

void OutputMdv::loadScaleAndBias(const double scale, const double bias)
  
{

  MdvxField *fld = _outputter.getFieldByNum(0);
  Mdvx::field_header_t fhdr = fld->getFieldHeader();
  fhdr.scale = scale;
  fhdr.bias = bias;
  fld->setFieldHeader(fhdr);
    
}

void OutputMdv::initHeaders(int nx, int ny,
                            float dx, float dy,
                            float minx, float miny,
                            const char * radar_name,
                            const char * field_name_long,
                            const char * field_name,
                            const char * field_units,
                            const char * field_transform,
                            const int field_code)
 
{

  // set the master header
 
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  // Times will be set later on write.
  // mhdr.time_gen = time(NULL);
  // mhdr.time_begin = scan_time;
  // mhdr.time_end = scan_time ;
  // mhdr.time_centroid = scan_time;
  // mhdr.time_expire = scan_time + 600;

  mhdr.num_data_times = 1;
  mhdr.data_dimension = 2;

  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;

  mhdr.max_nx = nx;
  mhdr.max_ny = ny;
  mhdr.max_nz = 1;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = _sensorLon;
  mhdr.sensor_lat = _sensorLat;
  mhdr.sensor_alt = _sensorAlt;

  STRncopy(mhdr.data_set_info,
           "Nids data remapped using NewNidsRadial2Mdv", MDV_NAME_LEN);
  STRncopy(mhdr.data_set_name, radar_name, MDV_NAME_LEN);
  STRncopy(mhdr.data_set_source, "Nids", MDV_NAME_LEN);

  _outputter.setMasterHeader(mhdr);
 
  // fill in field headers and vlevel headers
 
  _outputter.clearFields();
 
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);

  fhdr.nx = nx;
  fhdr.ny = ny;
  fhdr.nz = 1;

  fhdr.proj_type = Mdvx::PROJ_FLAT;
  fhdr.encoding_type = Mdvx::ENCODING_INT8;
  fhdr.data_element_nbytes = sizeof(ui08);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;

  fhdr.scaling_type = Mdvx::SCALING_SPECIFIED;
  fhdr.scale = 1;
  fhdr.bias = 0;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  fhdr.dz_constant = true;


  fhdr.proj_origin_lat = _sensorLat;
  fhdr.proj_origin_lon = _sensorLon;

  fhdr.grid_dx = dx;
  fhdr.grid_dy = dy;
  fhdr.grid_dz = 1.0;
  fhdr.grid_minx = minx;
  fhdr.grid_miny = miny;
  fhdr.grid_minz = _minAngle;
 
  fhdr.proj_rotation = 0.0;
   
  fhdr.bad_data_value = 0.0;
  fhdr.missing_data_value = 0.0;

  fhdr.field_code = field_code;
  STRncopy(fhdr.field_name_long, field_name_long, MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.field_name, field_name, MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.units, field_units, MDV_UNITS_LEN);
  STRncopy(fhdr.transform, field_transform, MDV_TRANSFORM_LEN);

  // vlevel header
 
  vhdr.type[0] = Mdvx::VERT_TYPE_ELEV;
  vhdr.level[0] = _minAngle;

  // create field
 
  MdvxField *field = new MdvxField(fhdr, vhdr, NULL, true);

  // add field to mdvx object
 
  _outputter.addField(field);
 
}

////////////////////////////////////////
// writeVol()
//
// Write out merged volume in MDV format.
//

int OutputMdv::writeVol(const time_t scan_time,
                        const char *output_dir)

{


  // set the master header times.
  Mdvx::master_header_t mhdr = _outputter.getMasterHeader();;
  mhdr.time_gen = time(NULL);
  mhdr.time_begin = scan_time;
  mhdr.time_end = scan_time ;
  mhdr.time_centroid = scan_time;
  mhdr.time_expire = scan_time + 600;
  _outputter.setMasterHeader(mhdr);

  // Change the compression to the proper type.
  MdvxField *fld = _outputter.getFieldByNum(0);
  fld->convertType(Mdvx::ENCODING_INT8,
                   Mdvx::COMPRESSION_RLE,
                   Mdvx::SCALING_SPECIFIED);

  // Write the file.
  if (_outputter.writeToDir(output_dir)) {
    cerr << "ERROR - OutputFile::writeVol" << endl;
    cerr << _outputter.getErrStr() << endl;
    return -1;
  } else {
    return 0;
  }

}

