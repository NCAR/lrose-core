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

#include "GribNc2Mdv.hh"

#include <iostream>
#include <netcdf.h>
#include <stdlib.h>
#include <cstdio>

#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>


//
// Constructor and destructore - minimal.
//
GribNc2Mdv::GribNc2Mdv(Params *P){
  _params = P;
}

GribNc2Mdv::~GribNc2Mdv(){

}
//
// Main deal - the netCDF -> MDV conversion.
//
void GribNc2Mdv::GribNc2MdvFile( char *FilePath ){

  int netID;
  int status = nc_open(FilePath, NC_NOWRITE, &netID);
  _checkStatus(status, "Failed to open input file.");

  int varID;


  //////////////////////////////////////////////////////

  char datetime[128];
  char grid_type[128];
  char grid_name[128];

  status = nc_inq_varid(netID, "datetime", &varID);
  _checkStatus(status, "Failed to get datetime variable ID");

  status = nc_get_var_text(netID, varID, datetime);
  _checkStatus(status, "Failed to read time string");


  //////////////////////////////////////////////////////

  status = nc_inq_varid(netID, "grid_name", &varID);
  _checkStatus(status, "Failed to get grid_name variable ID");

  status = nc_get_var_text(netID, varID, grid_name);
  _checkStatus(status, "Failed to read grid_name string");

  //////////////////////////////////////////////////////

  status = nc_inq_varid(netID, "grid_type", &varID);
  _checkStatus(status, "Failed to get grid_type variable ID");

  status = nc_get_var_text(netID, varID, grid_type);
  _checkStatus(status, "Failed to read grid_type string");

  //////////////////////////////////////////////////////

  int Ni; // Latitude
  status = nc_inq_varid(netID, "Ni", &varID);
  _checkStatus(status, "Failed to get Ni variable ID");

  status = nc_get_var_int(netID, varID, &Ni);
  _checkStatus(status, "Failed to read Ni integer");

  //////////////////////////////////////////////////////

  int Nj; // Longitude
  status = nc_inq_varid(netID, "Nj", &varID);
  _checkStatus(status, "Failed to get Nj variable ID");

  status = nc_get_var_int(netID, varID, &Nj);
  _checkStatus(status, "Failed to read Nj integer");

  //////////////////////////////////////////////////////

  float La1; // Latitude 1
  status = nc_inq_varid(netID, "La1", &varID);
  _checkStatus(status, "Failed to get La1 variable ID");

  status = nc_get_var_float(netID, varID, &La1);
  _checkStatus(status, "Failed to read La1");
  
  //////////////////////////////////////////////////////

  float Lo1; // Longitude 1
  status = nc_inq_varid(netID, "Lo1", &varID);
  _checkStatus(status, "Failed to get Lo1 variable ID");

  status = nc_get_var_float(netID, varID, &Lo1);
  _checkStatus(status, "Failed to read Lo1");

  //////////////////////////////////////////////////////

  float Dj; // Latitude inc
  status = nc_inq_varid(netID, "Dj", &varID);
  _checkStatus(status, "Failed to get Dj variable ID");

  status = nc_get_var_float(netID, varID, &Dj);
  _checkStatus(status, "Failed to read Dj");
  
  //////////////////////////////////////////////////////
  
  float Di; // Longitude inc
  status = nc_inq_varid(netID, "Di", &varID);
  _checkStatus(status, "Failed to get Di variable ID");

  status = nc_get_var_float(netID, varID, &Di);
  _checkStatus(status, "Failed to read Di");
 
  //////////////////////////////////////////////////////
  
  float valtime_offset; // Forecast lead time, hours
  status = nc_inq_varid(netID, "valtime_offset", &varID);
  _checkStatus(status, "Failed to get valtime_offset variable ID");

  status = nc_get_var_float(netID, varID, &valtime_offset);
  _checkStatus(status, "Failed to read valtime_offset");
 
  time_t leadSeconds = time_t(3600*valtime_offset);

  //////////////////////////////////////////////////////

  if (strcmp("Latitude/Longitude", grid_type)){
    cerr << "Data are not on lat/lon grid, exiting" << endl;
    nc_close(netID);
    exit(-1);
  }
  //
  // Read the levels.
  //
  int dimID;
  status = nc_inq_dimid(netID, "level", &dimID);
  _checkStatus(status, "Failed to read dimID;");

  size_t numLevels;
  status = nc_inq_dimlen(netID, dimID, &numLevels);
  _checkStatus(status, "Failed to read numLevels;");

  status = nc_inq_varid(netID, "level", &varID);
  _checkStatus(status, "Failed to get levels variable ID");


  float *level = (float *) malloc(numLevels*sizeof(float));
  if (level == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  status = nc_get_var_float(netID, varID, level);
  _checkStatus(status, "Failed to read level");
  //
  if (_params->debug){
    cerr << "Generate time : " << datetime << endl;
    cerr << "Lead hours : " << valtime_offset << endl;
    cerr << "Grid name : " << grid_name << endl;
    cerr << "Grid type : " << grid_type << endl;
    cerr << "Grid size : " << Nj << " (latitude) by ";
    cerr << Ni << " (longitude)" << endl;
    cerr << "First point lat, lon : " << La1 << ", " << Lo1 << endl;
    cerr << "Delta lat, lon : " << Dj << ", " << Di << endl;
    
    cerr << numLevels << " levels : " << endl;
    for(unsigned i=0; i < numLevels; i++){
      cerr << i << ", " << level[i] << endl;
    }
  }
  //
  // Get set up to write MDV.
  //

  Mdvx::field_header_t twoDfhdr,  threeDfhdr;
  Mdvx::vlevel_header_t twoDvhdr, threeDvhdr;
  Mdvx::master_header_t Mhdr;

  memset(&twoDfhdr, 0, sizeof(twoDfhdr));
  memset(&threeDfhdr, 0, sizeof(threeDfhdr));
  //
  memset(&twoDvhdr, 0, sizeof(twoDvhdr));
  memset(&threeDvhdr, 0, sizeof(threeDvhdr));
  //
  memset(&Mhdr, 0, sizeof(Mhdr));
  //
  // Set up the vlevel headers.
  //
  twoDvhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  twoDvhdr.level[0] = 0.0;
  //
  for (unsigned iz = 0; iz < numLevels; iz++) {
    threeDvhdr.type[iz] = Mdvx::VERT_TYPE_PRESSURE;
    threeDvhdr.level[iz] = level[iz];
  }
  //
  // Then the field headers.
  //
  twoDfhdr.nx = Ni; threeDfhdr.nx = Ni;
  twoDfhdr.ny = Nj; threeDfhdr.ny = Nj;
  twoDfhdr.nz = 1; threeDfhdr.nz = numLevels;
  //
  twoDfhdr.grid_dx = Di; threeDfhdr.grid_dx = Di;
  twoDfhdr.grid_dy = Dj; threeDfhdr.grid_dy = Dj;
  threeDfhdr.grid_dz = 0.0;
  //
  twoDfhdr.grid_minx = Lo1; threeDfhdr.grid_minx = Lo1;
  twoDfhdr.grid_miny = La1; threeDfhdr.grid_miny = La1;
  threeDfhdr.grid_minz = level[0];
  //
  twoDfhdr.proj_type = Mdvx::PROJ_LATLON;
  threeDfhdr.proj_type = Mdvx::PROJ_LATLON;
  //
  twoDfhdr.proj_origin_lat =  La1;
  twoDfhdr.proj_origin_lon =  Lo1;
  threeDfhdr.proj_origin_lat =  La1;
  threeDfhdr.proj_origin_lon =  Lo1;
  //
  twoDfhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  twoDfhdr.data_element_nbytes = sizeof(fl32);
  twoDfhdr.volume_size = twoDfhdr.nx * twoDfhdr.ny * sizeof(fl32);
  twoDfhdr.compression_type = Mdvx::COMPRESSION_NONE;
  twoDfhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  twoDfhdr.scaling_type = Mdvx::SCALING_NONE;     
  //
  //
  threeDfhdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  threeDfhdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  threeDfhdr.dz_constant = 0;
  twoDfhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  twoDfhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  twoDfhdr.dz_constant = 0;
  //
  threeDfhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  threeDfhdr.data_element_nbytes = sizeof(fl32);
  threeDfhdr.volume_size = threeDfhdr.nx * threeDfhdr.ny * threeDfhdr.nz * sizeof(fl32);
  threeDfhdr.compression_type = Mdvx::COMPRESSION_NONE;
  threeDfhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  threeDfhdr.scaling_type = Mdvx::SCALING_NONE;     
  //
  //
  Mhdr.data_collection_type = Mdvx::DATA_FORECAST;
  Mhdr.num_data_times = 1;
  Mhdr.data_ordering = Mdvx::ORDER_ZYX;  
  Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  Mhdr.vlevel_included = 1;
  //
  //
  sprintf(Mhdr.data_set_info,"%s","GribNc2Mdv");
  sprintf(Mhdr.data_set_name,"%s","GribNc2Mdv");
  sprintf(Mhdr.data_set_source,"%s", "GribNc2Mdv");
  //
  // Set the times in the master and field headers.
  //
  date_time_t T;
  if (6 != sscanf(datetime,"%d-%d-%d %d:%d:%d",
		  &T.year, &T.month, &T.day,
		  &T.hour, &T.min, &T.sec)){
    cerr << "Failed to decode time " << datetime << endl;
    exit(-1);
  }
  uconvert_to_utime( &T );
  Mhdr.time_gen = T.unix_time;
  Mhdr.time_begin = T.unix_time + leadSeconds;
  Mhdr.time_end = T.unix_time + leadSeconds;
  Mhdr.time_expire = T.unix_time + leadSeconds + _params->Expiry;
  Mhdr.time_centroid = T.unix_time + leadSeconds;
  //
  twoDfhdr.forecast_time = Mhdr.time_centroid;
  threeDfhdr.forecast_time = Mhdr.time_centroid;
  //
  twoDfhdr.forecast_delta = leadSeconds;
  threeDfhdr.forecast_delta = leadSeconds;
  //
  DsMdvx outMdvx;
  //
  outMdvx.setMasterHeader( Mhdr ); 
  outMdvx.clearFields();
  //
  // Loop through the field names.
  //
  for (int i=0; i < _params->fieldNames_n; i++){

    char *name = _params->_fieldNames[i];

    status = nc_inq_varid(netID, name, &varID);
    _checkStatus(status, "Failed to get variable ID");
    //
    // Get the number of dimensions.
    //
    int ndims;
    status = nc_inq_varndims(netID, varID, &ndims);
    _checkStatus(status, "Failed to get number of dimensions");

    if (_params->debug){
      cerr << name << " has " << ndims << " dimensions." << endl;
    }

    int *dimIDs = (int *) malloc(ndims*sizeof(int));
    size_t *dimLens = (size_t *) malloc(ndims*sizeof(size_t));
    if ((dimLens == NULL) || (dimIDs == NULL)){
      cerr << "Malloc failed." << endl;
      exit(-1);
    }

    status = nc_inq_vardimid(netID, varID, dimIDs);
    _checkStatus(status, "Failed to get dimension IDs");

    int totalSize = 1;

    for(int i=0; i < ndims; i++){
      status = nc_inq_dimlen(netID, dimIDs[i], &dimLens[i] );
      _checkStatus(status, "Failed to read numLevels;");
      if (_params->debug){
	cerr << "  dimension " << i << " : " << dimLens[i] << endl;
      }
      totalSize *= dimLens[i];
    }
    if (_params->debug){
      cerr << "Total size : " << totalSize << endl;
    }

    bool sizeOK = false;
    bool twoDfield = false;

    if (totalSize == (int)(Ni*Nj*numLevels)){
      if (_params->debug){
	cerr << "3D field." << endl;
      }
      sizeOK = true;
    }
    if (totalSize == (int)(Ni*Nj)){
      if (_params->debug){
	cerr << "2D field." << endl;
      }
      sizeOK = true;
      twoDfield = true;
    }

    if (!(sizeOK)){
      cerr << "Unrecognized size - I cannot cope." << endl;
      exit(-1);
    }

    free(dimIDs);


    char units[128]; for(int l=0; l<128; l++) units[l]=char(0);
    status =  nc_get_att_text(netID, varID, "units", units);
    _checkStatus(status, "Failed to get the units string.");

    char longName[128]; for(int l=0; l<128; l++) longName[l]=char(0);
    status =  nc_get_att_text(netID, varID, "long_name", longName);
    _checkStatus(status, "Failed to get the long_name string.");

    float fillVal;
    status =  nc_get_att_float(netID, varID, "_FillValue", &fillVal);
    _checkStatus(status, "Failed to get the fill value.");

    if (_params->debug){
      cerr << longName << endl;
      cerr << "Units : " << units << " Fillvalue : " << fillVal << endl;
    }

    float *buffer = (float *) malloc(sizeof(float)*totalSize);
    if (buffer == NULL){
      cerr << "Malloc failed." << endl;
      exit(-1);
    }

    status = nc_get_var_float(netID, varID, buffer);
    _checkStatus(status, "Failed to read variable.");

    int numGood = 0;
    for (int k=0; k < totalSize; k++){
      if (buffer[k] != fillVal) numGood++;
    }

    twoDfhdr.bad_data_value = fillVal;   twoDfhdr.missing_data_value = fillVal;
    threeDfhdr.bad_data_value = fillVal; threeDfhdr.missing_data_value = fillVal;

    sprintf( twoDfhdr.field_name_long,"%s", longName);
    sprintf( twoDfhdr.field_name,"%s", name);
    sprintf( twoDfhdr.units,"%s", units);
    sprintf( twoDfhdr.transform,"%s","none");
    //
    sprintf( threeDfhdr.field_name_long,"%s", longName);
    sprintf( threeDfhdr.field_name,"%s", name);
    sprintf( threeDfhdr.units,"%s", units);
    sprintf( threeDfhdr.transform,"%s","none");
    
    if (_params->debug){
      double pg = 100.0*double(numGood)/double(totalSize);
      cerr << "Field is " << pg << " percent good." << endl << endl;
    }
    //
    MdvxField *field;
    //
    if (twoDfield){
      field = new MdvxField(twoDfhdr, twoDvhdr, buffer);
    } else {
      field = new MdvxField(threeDfhdr, threeDvhdr, buffer);
    }
    //
    if (field->convertRounded(Mdvx::ENCODING_INT16,
				    Mdvx::COMPRESSION_ZLIB)){
      fprintf(stderr, "3D convert failed - I cannot go on.\n");
      exit(-1);
    }

    outMdvx.addField(field);

    free(buffer);

  }

  outMdvx.setWriteAsForecast();
  outMdvx.setAppName("GribNc2Mdv");
  if (outMdvx.writeToDir(_params->output_url)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << _params->output_url << endl;
    cerr << outMdvx.getErrStr() << endl;
  }

  nc_close(netID);
  free(level);

}

void GribNc2Mdv::_checkStatus(int status, const char *exitStr){

  if (status != NC_NOERR){
    cerr << exitStr << endl;
    exit(0);
  }

}
