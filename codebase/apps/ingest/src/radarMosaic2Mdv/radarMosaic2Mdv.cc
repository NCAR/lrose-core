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
// Read netCDF radar grids from Dick Oye, write
// mdv data.
//
#include "radarMosaic2Mdv.hh"

#include <toolsa/umisc.h>
#include <netcdf.h> 

#include <cstdlib>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <cstring>
using namespace std;

////////////////////////////////////////////////////////////
//
// constructor. That which there is to do, is done herein.
//
radarMosaic2Mdv::radarMosaic2Mdv (char *FilePath,Params *P){

  if (P->debug){
    cerr << "Processing file " << FilePath << endl;
  }
  
  int ncID;
  if (NC_NOERR != nc_open(FilePath, 0, &ncID)){
    cerr << "Failed to open " << FilePath << endl;
    return;
  }
  //
  // Get the dimensional IDs.
  //
  int timeDimID, latDimID, lonDimID, altDimID;

  if (
      (NC_NOERR != nc_inq_dimid(ncID, "time", &timeDimID)) ||
      (NC_NOERR != nc_inq_dimid(ncID, "latitude", &latDimID)) ||
      (NC_NOERR != nc_inq_dimid(ncID, "longitude", &lonDimID)) ||
      (NC_NOERR != nc_inq_dimid(ncID, "altitude", &altDimID)) 
      ){
    cerr << "Dimensional variables time, latitude, longitude and altitude ";
    cerr << "must be present - I cannot process " << FilePath << endl;
    nc_close(ncID);
    return;
  }
  //
  // Get the dimensional variable IDs.
  //
  int timeVarID, latVarID, lonVarID, altVarID;

  if (
      (NC_NOERR != nc_inq_varid(ncID, "time", &timeVarID)) ||
      (NC_NOERR != nc_inq_varid(ncID, "latitude", &latVarID)) ||
      (NC_NOERR != nc_inq_varid(ncID, "longitude", &lonVarID)) ||
      (NC_NOERR != nc_inq_varid(ncID, "altitude", &altVarID)) 
      ){
    cerr << "Dimensional variables time, latitude, longitude and altitude ";
    cerr << "must be present - I cannot process " << FilePath << endl;
    nc_close(ncID);
    return;
  }
  //
  // OK - now have variable and dimension IDs for the dimension
  // variables. Now get their lengths.
  //
  //
  unsigned timeLen, latLen, lonLen, altLen;

  if (
      (NC_NOERR != nc_inq_dimlen(ncID, timeDimID, &timeLen)) ||
      (NC_NOERR != nc_inq_dimlen(ncID, latDimID, &latLen)) ||
      (NC_NOERR != nc_inq_dimlen(ncID, lonDimID, &lonLen)) ||
      (NC_NOERR != nc_inq_dimlen(ncID, altDimID, &altLen))
      ){
    cerr << "Lengths unavailable for time, latitude, longitude and altitude ";
    cerr << "I cannot process " << FilePath << endl;
    nc_close(ncID);
    return;
  }
  
  if (P->debug){
    cerr << "Dimensions for " << FilePath << endl;
    cerr << "Time : " << timeLen << endl;
    cerr << "Latitude : " << latLen << endl;
    cerr << "Longitude : " << lonLen << endl;
    cerr << "Altitude : " << altLen << endl;

  }
  //
  // Allocate space for, and read in, the dimensional variables.
  //
  //
  float *alt, *lat, *lon;
  double *dataTime;
  alt = (float *)malloc(sizeof(float) * altLen);
  lat = (float *)malloc(sizeof(float) * latLen);
  lon = (float *)malloc(sizeof(float) * lonLen);
  dataTime = (double *)malloc(sizeof(double) * timeLen);
  if ((alt == NULL) || (lat == NULL) || (lon == NULL) || (dataTime == NULL)){
    cerr << "Malloc failed." << endl;
    nc_close(ncID);
    exit(-1);
  }
  //
  // Read the variables.
  //
  if (
      (NC_NOERR != nc_get_var_double(ncID, timeVarID, dataTime)) ||
      (NC_NOERR != nc_get_var_float(ncID, latVarID, lat)) ||
      (NC_NOERR != nc_get_var_float(ncID, lonVarID, lon)) ||
      (NC_NOERR != nc_get_var_float(ncID, altVarID, alt))
      ){
    cerr << "Couldn't read time, latitude, longitude and altitude ";
    cerr << "I cannot process " << FilePath << endl;
    nc_close(ncID);
    return;
  }
  //
  // Get the extremes in alt, lat and lon.
  //
  double minLat, maxLat;
  minLat = lat[0]; maxLat = lat[0];
  for (unsigned i=1; i < latLen; i++){
    if (minLat > lat[i]) minLat = lat[i];
    if (maxLat < lat[i]) maxLat = lat[i];
  }
  //
  double minLon, maxLon;
  minLon = lon[0]; maxLon = lon[0];
  for (unsigned i=1; i < lonLen; i++){
    if (minLon > lon[i]) minLon = lon[i];
    if (maxLon < lon[i]) maxLon = lon[i];
  }
  //
  double minAlt, maxAlt;
  minAlt = alt[0]; maxAlt = alt[0];
  for (unsigned i=1; i < altLen; i++){
    if (minAlt > alt[i]) minAlt = alt[i];
    if (maxAlt < alt[i]) maxAlt = alt[i];
  }
  //
  // Caluclate dx, dy, dz. Equal spacing in lat, lon, alt assumed.
  //
  double dx = (maxLon - minLon) / (lonLen - 1);
  double dy = (maxLat - minLat) / (latLen - 1);
  double dz;
  if (altLen <= 1)
    dz = 0;
  else
    dz = (maxAlt - minAlt) / (altLen -1);
  //
  //
  //
  if (P->debug){
    cerr << " Available times : " << endl;
    for (unsigned i=0; i < timeLen; i++){
      cerr << utimstr(long(dataTime[i])) << endl;
    }
    cerr << altLen << " altitude steps from " << minAlt << " to " << maxAlt << endl;
    cerr << latLen << " latitude steps from " << minLat << " to " << maxLat << endl;
    cerr << "Increments of " << dy << endl;
    cerr << lonLen << " longitude steps from " << minLon << " to " << maxLon << endl;
    cerr << "Increments of " << dx << endl;
  }
  //
  // Ignore file if time dimension has length > 1.
  // May have to revisit this in future.
  //
  if (timeLen > 1){
    cerr << "More than one time defined in " << FilePath << endl;
    cerr << "Cannot convert this file." << endl;
    nc_close(ncID);
    return;
  }

  //
  // Set up a generic sort of master, field and vlevel header.
  //

  //
  // Vlevel
  //
  Mdvx::vlevel_header_t Vhdr;
  memset(&Vhdr,0,sizeof(Vhdr));
  //
  for(unsigned i=0; i < altLen; i++){
    Vhdr.level[i]= alt[i];
    Vhdr.type[i] = Mdvx::VERT_TYPE_Z;
  }
  //
  // Master header.
  //
  Mdvx::master_header_t Mhdr;
  memset(&Mhdr, 0, sizeof(Mhdr));

  Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  if (altLen <=1)
    Mhdr.data_dimension = 2;
  else 
    Mhdr.data_dimension = 3;
  Mhdr.time_begin = Mhdr.time_end = Mhdr.time_centroid = (long) dataTime[0];
  Mhdr.time_expire = 0;
  Mhdr.time_gen = time(NULL);
  Mhdr.num_data_times = 1;
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  Mhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  Mhdr.vlevel_included = 1;
  Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  Mhdr.data_ordering = Mdvx::ORDER_XYZ;
  Mhdr.sensor_lon = Mhdr.sensor_lat = Mhdr.sensor_alt = 0.0;
  //
  sprintf(Mhdr.data_set_info,"%s","Radar mosaic");
  sprintf(Mhdr.data_set_name,"%s","Radar mosaic");
  sprintf(Mhdr.data_set_source,"%s","netCDF file");
  //

  //
  // Field header.
  //
  Mdvx::field_header_t Fhdr;
  memset(&Fhdr, 0, sizeof(Fhdr));

  //
  Fhdr.nx = lonLen;        Fhdr.ny = latLen;        Fhdr.nz = altLen;
  Fhdr.grid_minx = minLon; Fhdr.grid_miny = minLat; Fhdr.grid_minz = minAlt; 
  Fhdr.grid_dx = dx;       Fhdr.grid_dy = dy;       Fhdr.grid_dz = dz; 

  Fhdr.proj_origin_lat = Fhdr.grid_miny;  Fhdr.proj_origin_lon = Fhdr.grid_minx;
  Fhdr.proj_type = Mdvx::PROJ_LATLON;
  Fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  Fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  Fhdr.data_element_nbytes =sizeof(float);
  Fhdr.volume_size = Fhdr.nx*Fhdr.ny*Fhdr.nz*Fhdr.data_element_nbytes;
  Fhdr.forecast_delta = 0; Fhdr.forecast_time = Mhdr.time_centroid;
  Fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;

  Fhdr.field_code = 0;
  Fhdr.user_time1 = Fhdr.user_time2 = Fhdr.user_time3 = 0;
  Fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  Fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  Fhdr.dz_constant = 1;

  sprintf(Fhdr.transform, "%s","none");


  //
  // name, missing_data_value and units set for each var.
  //
  // Allocate space to read into.
  //
  float *data = (float *)malloc(sizeof(float)*altLen*lonLen*latLen);
  //
  // Set up DsMdvx object to write to.
  //
  DsMdvx Out;
  Out.setMasterHeader(Mhdr);
  Out.clearFields();     
  //
  // Loop through the fields.
  //
  MdvxRemapLut lut; // Used in the remapping of fields.
  //
  for(int ifield=0; ifield < P->InFields_n; ifield++){
    //
    if (P->debug){
      cerr << "Adding field " << P->_InFields[ifield] << endl;
    }
    //
    // Try to read in the data, skip the field if we fail.
    //
    int varID;
    if  (
	 (NC_NOERR != nc_inq_varid(ncID, P->_InFields[ifield], &varID)) ||
	 (NC_NOERR != nc_get_var_float(ncID, varID, data))
	 ){
      cerr << "ERROR : Could not find field " << P->_InFields[ifield] << endl;
      cerr << "This field will not be added." << endl;
      continue;
    }
    //
    // Get the units and missing_value attributes.
    //
    char units[64];  float missing_value;
    if (NC_NOERR != nc_get_att_text(ncID, varID, "units", units)){
      cerr << "WARNING : units not found for field " << P->_InFields[ifield];
      cerr << " - setting units to \"none\"" << endl;
      sprintf(units,"%s","none");
    }
    //
    if (NC_NOERR != nc_get_att_float(ncID, varID, "missing_value", &missing_value)){
      missing_value = -99999.0;
      cerr << "WARNING : missing_value not found for field " << P->_InFields[ifield];
      cerr << " - setting missing value to " << missing_value << endl;
    }

    sprintf(Fhdr.units,"%s",units);
    Fhdr.bad_data_value = missing_value;
    Fhdr.missing_data_value = missing_value;
    sprintf(Fhdr.field_name,"%s",P->_InFields[ifield]);
    //
    // For Fhdr.field_name_long, try to use the long_name attribute, but if can't find it, use
    // the short name. Rose by any other nomenclature.
    //
    char long_name[64];
    if (NC_NOERR != nc_get_att_text(ncID, varID, "long_name", long_name)){
      sprintf(Fhdr.field_name_long,"%s", P->_InFields[ifield]);
    } else {
      sprintf(Fhdr.field_name_long,"%s",long_name);
    }


    if (P->debug){
      cerr << "Units " << units << endl;
      cerr << "Missing_value " << missing_value << endl;
      cerr << "Long name : " << Fhdr.field_name_long << endl;
    }
    //
    // Create field.
    //
    MdvxField *fld = new MdvxField(Fhdr, Vhdr, (void *)data);
    //
    // Remap data, if desired.
    //
    if (P->RemapGrid){

      switch ( P->grid_projection){
	
      case Params::FLAT:

	if (fld->remap2Flat(lut, P->grid_nx, P->grid_ny,
			    P->grid_minx, P->grid_miny,
			    P->grid_dx, P->grid_dy,
			    P->grid_origin_lat, P->grid_origin_lon,
			    P->grid_rotation)){
	  cerr << "Re-map failed." << endl;
	  exit(-1);
	}
	
	break;
	
      case Params::LATLON:

	if (fld->remap2Latlon(lut, P->grid_nx, P->grid_ny,
			      P->grid_minx, P->grid_miny,
			      P->grid_dx, P->grid_dy)){
	  cerr << "Re-map failed." << endl;
	  exit(-1);
	}
	
	break;            
	
      case Params::LAMBERT:

  
	if (fld->remap2Lc2(lut, P->grid_nx, P->grid_ny,
			   P->grid_minx, P->grid_miny,
			   P->grid_dx, P->grid_dy,
			   P->grid_origin_lat, 
			   P->grid_origin_lon,
			   P->grid_lat1,  P->grid_lat2)){
	  cerr << "Re-map failed." << endl;
	  exit(-1);
	}
	
	break;
      
      default:
	cerr << "Unsupported projection." << endl;
	exit(-1);
	break;
      
      }   
    }            
    //
    // Write to MDV object.
    //

    
    if (fld->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  
    
    Out.addField(fld);
    
  } // End of loop through fields.
  //
  // Free up the data and dimensional variables.
  //
  free(data);
  free(alt); 
  free(lat); 
  free(lon); 
  free(dataTime);

  if (NC_NOERR != nc_close(ncID)){
    cerr << "Failed to close " <<  FilePath << endl;
  }
  //
  // Write MDV output.
  //  
  Mdvx::master_header_t OMhdr = Out.getMasterHeader();

  if (OMhdr.n_fields > 0){
    if (Out.writeToDir(P->OutUrl)) {
      cerr << "Failed to wite to " << P->OutUrl << endl;
      exit(-1);
    }      
  }

  return;

}

////////////////////////////////////////////////////////////
//
// destructor
//
radarMosaic2Mdv::~radarMosaic2Mdv (){



}
