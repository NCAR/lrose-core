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
// This is the module of the tec2mdv application that does most
// of the work, reading the netCDF and writing the MDV files. Include
// both MDV and netCDF header files.
//
#include "tec2mdv.hh"

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
tec2mdv::tec2mdv(Params *P){
  _params = P;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
tec2mdv::~tec2mdv(){
  return;
}
//
// Main method - the netCDF -> MDV conversion.
//
void tec2mdv::tec2mdvFile( char *FilePath ){
  //
  // Open the netCDF input file.
  //
  int netID;
  int status = nc_open(FilePath, NC_NOWRITE, &netID);
  _checkStatus(status, "Failed to open input file.");

  int varID;

  //
  // Start reading the metadata associated with the grid,
  // first off the data time.
  //
  //////////////////////////////////////////////////////
  //
  // Read the year
  //
  int year;
  status = nc_get_att_int(netID, NC_GLOBAL,"year", &year);
  _checkStatus(status, "Failed to get year");

  //
  // Read the day of the year, [1..365]
  //
  int day;
  status = nc_get_att_int(netID, NC_GLOBAL, "day", &day);
  _checkStatus(status, "Failed to get day");


  //
  // Read the hour
  //
  int hour;
  status = nc_get_att_int(netID, NC_GLOBAL, "hour", &hour);
  _checkStatus(status, "Failed to get hour");


  //
  // Read the minute
  //
  int minute;
  status = nc_get_att_int(netID, NC_GLOBAL, "minute", &minute);
  _checkStatus(status, "Failed to get minute");


  date_time_t dataTime;
  dataTime.year = year; dataTime.month = 1; dataTime.day = 1;
  dataTime.hour = 0;    dataTime.min = 0;   dataTime.sec = 0;

  uconvert_to_utime( &dataTime );
  dataTime.unix_time += (day-1)*86400 + hour * 3600 + minute * 60;
  uconvert_from_utime( &dataTime );


  ////////////////////////////////////////////////
  //
  // Get the dimensions of what we have to read.
  //
  int dimID;
  size_t nLat;
  status = nc_inq_dimid(netID, "lat", &dimID);
  _checkStatus(status, "Failed to get latitude dimension ID");

  status = nc_inq_dimlen(netID, dimID, &nLat);
  _checkStatus(status, "Failed to read nLat");

  size_t nLon;
  status = nc_inq_dimid(netID, "lon", &dimID);
  _checkStatus(status, "Failed to get longitude dimension ID");

  status = nc_inq_dimlen(netID, dimID, &nLon);
  _checkStatus(status, "Failed to read nLon");


  fl32 *tec  = (fl32 *)malloc(sizeof(fl32)*nLat*nLon);
  fl32 *lats = (fl32 *)malloc(sizeof(fl32)*nLat);
  fl32 *lons = (fl32 *)malloc(sizeof(fl32)*nLon);

  if ((tec == NULL) || (lats == NULL) || (lons == NULL)){
    cerr << "TEC Malloc failed." << endl;
    exit(-1);
  }

  status = nc_inq_varid(netID, "tec", &varID);
  _checkStatus(status, "Failed to get tec variable ID");

  status = nc_get_var_float(netID, varID, tec);
  _checkStatus(status, "Failed to read tec");

  float missingVal;
  status = nc_get_att_float(netID, varID, "missing_value", &missingVal);
  _checkStatus(status, "Failed to get missing value");

  char tecUnits[1024];
  status = nc_get_att_text(netID, varID, "units", tecUnits);
  _checkStatus(status, "Failed to get units");



  status = nc_inq_varid(netID, "lat", &varID);
  _checkStatus(status, "Failed to get lat variable ID");

  status = nc_get_var_float(netID, varID, lats);
  _checkStatus(status, "Failed to read lats");

  float deltaLat;
  status = nc_get_att_float(netID, varID, "delta", &deltaLat);
  _checkStatus(status, "Failed to get delta lat");



  status = nc_inq_varid(netID, "lon", &varID);
  _checkStatus(status, "Failed to get lon variable ID");

  status = nc_get_var_float(netID, varID, lons);
  _checkStatus(status, "Failed to read lons");

  float deltaLon;
  status = nc_get_att_float(netID, varID, "delta", &deltaLon);
  _checkStatus(status, "Failed to get delta lon");


  if (_params->debug){
    cerr << "Data time " << utimstr(dataTime.unix_time) << endl;
    cerr << "Data grid is " << nLat << " by " << nLon << " in lat,lon" << endl;
  }

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
  fhdr.nx = nLon;
  fhdr.ny = nLat;
  fhdr.nz = 1;
  //
  fhdr.grid_dx = deltaLon;
  fhdr.grid_dy = deltaLat;
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
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
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
  Mhdr.data_collection_type = Mdvx::DATA_FORECAST;
  Mhdr.num_data_times = 1;
  Mhdr.data_ordering = Mdvx::ORDER_XYZ;
  Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  Mhdr.vlevel_included = 1;
  //
  //
  sprintf(Mhdr.data_set_info,"%s","tec2mdv");
  sprintf(Mhdr.data_set_name,"%s","tec2mdv");
  sprintf(Mhdr.data_set_source,"%s", "tec2mdv");
  //
  // Set the times in the master and field headers.
  //

  Mhdr.time_gen = dataTime.unix_time;
  Mhdr.time_begin = dataTime.unix_time;
  Mhdr.time_end = dataTime.unix_time;
  Mhdr.time_expire = dataTime.unix_time + _params->Expiry;
  Mhdr.time_centroid = dataTime.unix_time;
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
  //
  // Complete filling out the field header with this information.
  //
  fhdr.bad_data_value = missingVal;   fhdr.missing_data_value = missingVal;
  //
  sprintf( fhdr.field_name_long,"tec");
  sprintf( fhdr.field_name,"tec");
  sprintf( fhdr.units,"%s", "e/cm2");
  sprintf( fhdr.transform,"%s","none");
  //
  //
  // Now we are ready to add these data to our DsMdvx object.
  // We fdo this by combining the field header, vlevel header
  // and the actual grid data into a MdvxField object, then
  // setting compression and encoding type for this field and
  // finally adding the field to the DsMdvx object.
  //
  MdvxField *field;
  //
  field = new MdvxField(fhdr, vhdr, tec);
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

  free(tec); free(lats); free(lons);

  //
  // Close the netCDF file.
  //
  nc_close(netID);

  return;

}
//
// The following is a small method that exits if things
// have gone wrong with a netCDF read. An error string is printed first.
//
void tec2mdv::_checkStatus(int status, char *exitStr){

  if (status != NC_NOERR){
    cerr << exitStr << endl;
    exit(0);
  }

  return;

}
