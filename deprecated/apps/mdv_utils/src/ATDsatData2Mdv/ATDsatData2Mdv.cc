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
//
// This is the module of the ATDsatData2Mdv application that does most
// of the work, reading the netCDF and writing the MDV files. Include
// both MDV and netCDF header files.
//
#include "ATDsatData2Mdv.hh"

#include <iostream>
#include <netcdf.h>
#include <stdlib.h>
#include <cstdio>

#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/pjg_flat.h>
//
// Constructor - makes a copy of a pointer to the TDRP parameters.
//
ATDsatData2Mdv::ATDsatData2Mdv(Params *P){
  _params = P;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
ATDsatData2Mdv::~ATDsatData2Mdv(){
  return;
}
//
// Main method - the netCDF -> MDV conversion.
//
void ATDsatData2Mdv::ATDsatData2MdvFile( char *FilePath ){
  //
  // Open the netCDF input file.
  //
  int netID;
  int status = nc_open(FilePath, NC_NOWRITE, &netID);
  _checkStatus(status, "Failed to open input file.");

  int varID;
  int dimID;

  unsigned int numLat;
  status = nc_inq_dimid(netID, "latitude", &dimID);
  _checkStatus(status, "Failed to get numLat variable ID");

  status = nc_inq_dimlen(netID, dimID, &numLat);
  _checkStatus(status, "Failed to read numLat integer");

  unsigned int numLon;
  status = nc_inq_dimid(netID, "longitude", &dimID);
  _checkStatus(status, "Failed to get numLon variable ID");

  status = nc_inq_dimlen(netID, dimID, &numLon);
  _checkStatus(status, "Failed to read numLon integer");

  if (_params->debug){
    cerr << "Grid is " << numLon << " by " << numLat << endl;
  }


  int timeInt;
  status = nc_inq_varid(netID, "base_time", &varID);
  _checkStatus(status, "Failed to get time variable ID");

  status = nc_get_var_int(netID, varID, &timeInt);
  _checkStatus(status, "Failed to read timeInt integer");

  time_t dataTime = timeInt;

  if (_params->debug){
    cerr << "Data are at " << utimstr(dataTime) << endl;
  }

  unsigned char *visBytes = (unsigned char *) malloc(numLat * numLon);
  if (visBytes == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }


  status = nc_inq_varid(netID, "vis", &varID);
  _checkStatus(status, "Failed to get vis variable ID");

  status = nc_get_var_uchar(netID, varID, visBytes);
  _checkStatus(status, "Failed to read main variable.");

  float *visFloats = (float *) malloc(numLat * numLon * sizeof(float));
  if (visFloats == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  float min= _params->factorToApply * (float)visBytes[0];
  float max=min;

  double total = 0.0;
  for (unsigned int il=0; il < numLat*numLon; il++){
    visFloats[il] =  _params->factorToApply * (float) visBytes[il];
    if (visFloats[il] < min) min = visFloats[il];
    if (visFloats[il] > max) max = visFloats[il];
    total += visFloats[il];
  }

  double mean = total / double( numLat*numLon );

  if (_params->debug){
    cerr << "Data run from " << min << " to " << max << " mean " << mean << endl;
  }

  free(visBytes);


  float *lats = (float *) malloc(numLat * sizeof(float));
  if (lats == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  float *lons = (float *) malloc(numLon * sizeof(float));
  if (lons == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  status = nc_inq_varid(netID, "latitude", &varID);
  _checkStatus(status, "Failed to get lat variable ID");

  status = nc_get_var_float(netID, varID, lats );
  _checkStatus(status, "Failed to read lat variable.");



  status = nc_inq_varid(netID, "longitude", &varID);
  _checkStatus(status, "Failed to get lon variable ID");

  status = nc_get_var_float(netID, varID, lons );
  _checkStatus(status, "Failed to read lon variable.");


  //
  // Prepare to write MDV files. Declare a master, field
  // and vlevel header.
  //
  Mdvx::field_header_t fhdr;
  Mdvx::vlevel_header_t vhdr;
  Mdvx::master_header_t Mhdr;
  //
  // Use 'memset' to set all the bytres in these structures to 0.
  //
  memset(&fhdr, 0, sizeof(fhdr));
  memset(&vhdr, 0, sizeof(vhdr));
  memset(&Mhdr, 0, sizeof(Mhdr));
  //
  // Set up the vlevel header. This is pretty simple as its is
  // just surface data.
  //
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0.0;
  //
  // Then set up the field header with the metadata.
  //
  fhdr.nx = numLon;
  fhdr.ny = numLat;
  fhdr.nz = 1;
  //
  fhdr.grid_dx = lons[1]-lons[0];
  fhdr.grid_dy = lats[1]-lats[0]; 
  //
  fhdr.proj_type = Mdvx::PROJ_LATLON; // This is implied for these data.
  //
  fhdr.proj_origin_lat =  lats[0];
  fhdr.proj_origin_lon =  lons[0];
  //
  //
  fhdr.grid_minx = lons[0];
  fhdr.grid_miny = lats[0];
  //
  //
  // Set up an uncompressed grid of floating point values.
  //
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;     
  //
  // State what vlevel type we have.
  //
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = 0;
  //
  // Set up some things in the master header.
  //
  Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  Mhdr.num_data_times = 1;
  Mhdr.data_ordering = Mdvx::ORDER_XYZ;  
  Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  Mhdr.vlevel_included = 1;
  //
  //
  sprintf(Mhdr.data_set_info,"%s","ATDsatData2Mdv");
  sprintf(Mhdr.data_set_name,"%s","ATDsatData2Mdv");
  sprintf(Mhdr.data_set_source,"%s", "ATDsatData2Mdv");
  //
  // Set the times in the master and field headers.
  //
  date_time_t T;
  T.unix_time = dataTime;
  uconvert_from_utime( &T );

  Mhdr.time_gen = T.unix_time;
  Mhdr.time_begin = T.unix_time;
  Mhdr.time_end = T.unix_time;
  Mhdr.time_expire = T.unix_time + _params->Expiry;
  Mhdr.time_centroid = T.unix_time;
  //
  fhdr.forecast_time = Mhdr.time_centroid;
  //
  fhdr.forecast_delta = 0;
  //
  // Declare a DsMdvx object so we can load it up with fields.
  //
  DsMdvx outMdvx;
  //
  outMdvx.setMasterHeader( Mhdr ); 
  outMdvx.clearFields();
  //
 
  //
  // Complete filling out the field header with this information.
  //
  fhdr.bad_data_value = -999.0;   fhdr.missing_data_value = -999.0;
  //
  sprintf( fhdr.field_name_long,"%s", "visCounts");
  sprintf( fhdr.field_name,"%s", "visCounts" );
  sprintf( fhdr.units,"%s",  "counts");
  sprintf( fhdr.transform,"%s","none");

  //
  // Now we are ready to add these data to our DsMdvx object.
  // We fdo this by combining the field header, vlevel header
  // and the actual grid data into a MdvxField object, then
  // setting compression and encoding type for this field and
  // finally adding the field to the DsMdvx object.
  //
  MdvxField *field;
  //
  field = new MdvxField(fhdr, vhdr, visFloats);
  //
  if (field->convertRounded(Mdvx::ENCODING_INT16,
			    Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }
  
  outMdvx.addField(field);


  //
  // Finally, now that the DsMdvx object is all loaded up, use it
  // to write out the data.
  //
  if (outMdvx.writeToDir(_params->output_url)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << _params->output_url << endl;
    cerr << outMdvx.getErrStr() << endl;
  }
  //
  // Close the netCDF file.
  //
  nc_close(netID);

  free(visFloats); free(lats); free(lons);

  return;

}
//
// The following is a small method that exits if things
// have gone wrong with a netCDF read. An error string is printed first.
//
void ATDsatData2Mdv::_checkStatus(int status, char *exitStr){

  if (status != NC_NOERR){
    cerr << exitStr << endl;
    exit(0);
  }

}
