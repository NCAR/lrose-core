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
// This is the module of the VaisalaLtg2Mdv application that does most
// of the work, reading the netCDF and writing the MDV files. Include
// both MDV and netCDF header files.
//
#include "VaisalaLtg2Mdv.hh"

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
VaisalaLtg2Mdv::VaisalaLtg2Mdv(Params *P){
  _params = P;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
VaisalaLtg2Mdv::~VaisalaLtg2Mdv(){
  return;
}
//
// Main method - the netCDF -> MDV conversion.
//
void VaisalaLtg2Mdv::VaisalaLtg2MdvFile( char *FilePath ){
  //
  // Open the netCDF input file.
  //
  int netID;
  int status = nc_open(FilePath, NC_NOWRITE, &netID);
  _checkStatus(status, "Failed to open input file.");

  //
  // See if we can parse the date/time from the filename.
  //
  date_time_t T;
  T.sec = 0;
  char *p = FilePath + strlen(FilePath) - strlen("20050426_0958");
  if (5 != sscanf(p, "%4d%2d%2d_%2d%2d",
		  &T.year, &T.month, &T.day,
		  &T.hour, &T.min)){
    cerr << "Failed to parse data time from " << FilePath << endl;
    return;
  }
  uconvert_to_utime( &T );

  int varID, dimID;

  //
  // Get the X and Y dimensions.
  //
  unsigned Nx;
  status = nc_inq_dimid(netID, "x", &dimID);
  _checkStatus(status, "Failed to get x dimension ID"); 
  //
  status = nc_inq_dimlen(netID, dimID, &Nx);
  _checkStatus(status, "Failed to get x dimension"); 


  unsigned Ny;
  status = nc_inq_dimid(netID, "y", &dimID);
  _checkStatus(status, "Failed to get y dimension ID"); 
  //
  status = nc_inq_dimlen(netID, dimID, &Ny);
  _checkStatus(status, "Failed to get y dimension"); 

  //
  // Get the record dimension. If this is 0, there is no data,
  // and we should write out all missing data.
  //
  unsigned nRecords;
  status = nc_inq_dimid(netID, "record", &dimID);
  _checkStatus(status, "Failed to get record dimension ID"); 
  //
  status = nc_inq_dimlen(netID, dimID, &nRecords);
  _checkStatus(status, "Failed to get record dimension"); 

  //
  // Read the global attributes - lat, lon, Dxy
  //
  float latC;
  status =  nc_get_att_float(netID, NC_GLOBAL, "CenterLat", &latC);
  _checkStatus(status, "Failed to get center lat.");
  //
  float lonC;
  status =  nc_get_att_float(netID, NC_GLOBAL, "CenterLon", &lonC);
  _checkStatus(status, "Failed to get center lon.");
  //  
  float Dxy;
  status =  nc_get_att_float(netID, NC_GLOBAL, "UnitSquareSideSize", &Dxy);
  _checkStatus(status, "Failed to get center Dxy.");

  //
  // Print the metadata we have read so far, if we are in debug mode.
  //
  if (_params->debug){
    fprintf(stderr,"Nx=%d\n",Nx);
    fprintf(stderr,"Ny=%d\n",Ny);
    fprintf(stderr,"Lat center=%g\n",latC);
    fprintf(stderr,"Lon center=%g\n",lonC);
    fprintf(stderr,"Dxy=%g\n",Dxy);
    fprintf(stderr,"Valid time : %s\n", utimstr(T.unix_time));
  }


  //
  // The variables are : gsd, fed, fidt. gsd is three dimensional with
  // a composite level at the bottom. The others are two dimensional.
  // calloc enough space (use calloc so that the data are set to 0).
  //
  float *gsd  = (float *) calloc( Nx*Ny*(_params->nGsdLevels + 1), sizeof(float) );
  float *fed  = (float *) calloc( Nx*Ny, sizeof(float) );
  float *fidt = (float *) calloc( Nx*Ny, sizeof(float) );
  //
  if ( (gsd == NULL) || (fed == NULL) || (fidt == NULL)){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }
  //
  float gsdBadVal  = 0.0;
  float fedBadVal  = 0.0;
  float fidtBadVal = 0.0;
  //
  // Prepare to write MDV files. Declare a master, field
  // and vlevel header.
  //
  Mdvx::field_header_t fhdr;
  Mdvx::vlevel_header_t vhdr;
  Mdvx::vlevel_header_t vhdr3D;
  Mdvx::master_header_t Mhdr;
  //
  // Use 'memset' to set all the bytres in these structures to 0.
  //
  memset(&fhdr, 0, sizeof(fhdr));
  memset(&vhdr, 0, sizeof(vhdr));
  memset(&vhdr3D, 0, sizeof(vhdr3D));
  memset(&Mhdr, 0, sizeof(Mhdr));
  //
  // Set up the vlevel header. This is pretty simple as its is
  // just surface data.
  //
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0.0;
  //
  // Similar for the 3D data.
  //
  vhdr3D.type[0] = Mdvx::VERT_TYPE_Z;
  for (int i=0; i < _params->nGsdLevels; i++){
    vhdr3D.level[i] =  _params->minGsdLevel + i*_params->deltaGsdLevel;
  }
  //
  // Then set up the field header with the metadata.
  //
  fhdr.nx = Nx;
  fhdr.ny = Ny;
  fhdr.nz = _params->nGsdLevels;
  //
  fhdr.grid_dx = Dxy;
  fhdr.grid_dy = Dxy;
  //
  fhdr.proj_type = Mdvx::PROJ_FLAT; // This is implied for these data.
  //
  fhdr.proj_origin_lat =  latC;
  fhdr.proj_origin_lon =  lonC;
  //
  //
  fhdr.grid_minx = -Dxy*Nx/2.0;
  fhdr.grid_miny = -Dxy*Ny/2.0;
  //
  //
  // Set up an uncompressed grid of floating point values.
  //
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
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
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  Mhdr.vlevel_included = 1;
  //
  //
  sprintf(Mhdr.data_set_info,"%s","VaisalaLtg2Mdv");
  sprintf(Mhdr.data_set_name,"%s","VaisalaLtg2Mdv");
  sprintf(Mhdr.data_set_source,"%s", "VaisalaLtg2Mdv");
  //
  // Set the times in the master and field headers.
  //
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
  // ===================== GSD ================================
  //
  status = nc_inq_varid(netID, "gsd", &varID);
  _checkStatus(status, "Failed to get variable ID");
  //
  // Get the string that holds the units for this variable.
  //
  char gsdUnits[128]; for(int l=0; l<128; l++) gsdUnits[l]=char(0);
  status =  nc_get_att_text(netID, varID, "units", gsdUnits);
  _checkStatus(status, "Failed to get the gsd units string.");
  //
  // Read in the long version of the variable name.
  //
  char gsdLongName[128]; for(int l=0; l<128; l++) gsdLongName[l]=char(0);
  status =  nc_get_att_text(netID, varID, "long_name", gsdLongName);
  _checkStatus(status, "Failed to get the gsd long_name string.");
  //
  // If the number of records is one, then
  // get the value used for missing data at a gridpoint,
  // and get the data.
  //
  if (nRecords == 1){

    status =  nc_get_att_float(netID, varID, "_FillValue", &gsdBadVal);
    _checkStatus(status, "Failed to get the gsd fill value.");
    //
    // 
    status = nc_get_var_float(netID, varID, gsd);
    _checkStatus(status, "Failed to read gsd variable.");
  }

  //
  // ===================== FED ================================
  //
  status = nc_inq_varid(netID, "fed", &varID);
  _checkStatus(status, "Failed to get variable ID");
  //
  // Get the string that holds the units for this variable.
  //
  char fedUnits[128]; for(int l=0; l<128; l++) fedUnits[l]=char(0);
  status =  nc_get_att_text(netID, varID, "units", fedUnits);
  _checkStatus(status, "Failed to get the fed units string.");
  //
  // Read in the long version of the variable name.
  //
  char fedLongName[128]; for(int l=0; l<128; l++) fedLongName[l]=char(0);
  status =  nc_get_att_text(netID, varID, "long_name", fedLongName);
  _checkStatus(status, "Failed to get the fed long_name string.");
  //
  // If the number of records is one, then
  // get the value used for missing data at a gridpoint,
  // and get the data.
  //
  if (nRecords == 1){

    status =  nc_get_att_float(netID, varID, "_FillValue", &fedBadVal);
    _checkStatus(status, "Failed to get the fed fill value.");
    //
    // 
    status = nc_get_var_float(netID, varID, fed);
    _checkStatus(status, "Failed to read fed variable.");
  }


  //
  // ===================== FIDT ================================
  //
  status = nc_inq_varid(netID, "fidt", &varID);
  _checkStatus(status, "Failed to get variable ID");
  //
  // Get the string that holds the units for this variable.
  //
  char fidtUnits[128]; for(int l=0; l<128; l++) fidtUnits[l]=char(0);
  status =  nc_get_att_text(netID, varID, "units", fidtUnits);
  _checkStatus(status, "Failed to get the fidt units string.");
  //
  // Read in the long version of the variable name.
  //
  char fidtLongName[128]; for(int l=0; l<128; l++) fidtLongName[l]=char(0);
  status =  nc_get_att_text(netID, varID, "long_name", fidtLongName);
  _checkStatus(status, "Failed to get the fidt long_name string.");
  //
  // If the number of records is one, then
  // get the value used for missing data at a gridpoint,
  // and get the data.
  //
  if (nRecords == 1){

    status =  nc_get_att_float(netID, varID, "_FillValue", &fidtBadVal);
    _checkStatus(status, "Failed to get the fidt fill value.");
    //
    // 
    status = nc_get_var_float(netID, varID, fidt);
    _checkStatus(status, "Failed to read fidt variable.");
  }


  //
  // Save out the three and two D gsd data.
  //
  fhdr.bad_data_value = gsdBadVal;   fhdr.missing_data_value = gsdBadVal;
  //
  sprintf( fhdr.field_name_long,"%s", gsdLongName);
  sprintf( fhdr.field_name,"%s", "gsd3d" );
  sprintf( fhdr.units,"%s", gsdUnits);
  sprintf( fhdr.transform,"%s","none");
  fhdr.nz = _params->nGsdLevels;
  //
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  //
  MdvxField *gsd3Dfield;
  //
  gsd3Dfield = new MdvxField(fhdr, vhdr3D, gsd + Nx*Ny);
  //
  if (gsd3Dfield->convertRounded(Mdvx::ENCODING_INT16,
				 Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }
  
  outMdvx.addField(gsd3Dfield);
  
  fhdr.nz = 1;
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  sprintf( fhdr.field_name,"%s", "gsd" );
  //
  MdvxField *gsdfield;
  //
  gsdfield = new MdvxField(fhdr, vhdr, gsd );
  //
  if (gsdfield->convertRounded(Mdvx::ENCODING_INT16,
			       Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }
  
  outMdvx.addField(gsdfield);

  //
  // Save out the two D fed data.
  //
  fhdr.bad_data_value = fedBadVal;   fhdr.missing_data_value = fedBadVal;
  //
  sprintf( fhdr.field_name_long,"%s", fedLongName);
  sprintf( fhdr.field_name,"%s", "fed" );
  sprintf( fhdr.units,"%s", fedUnits);
  //
  //
  MdvxField *fedfield;
  //
  fedfield = new MdvxField(fhdr, vhdr, fed );
  //
  if ( fedfield->convertRounded(Mdvx::ENCODING_INT16,
				Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }
  
  outMdvx.addField(fedfield);

  //
  // Save out the two D fidt data.
  //
  fhdr.bad_data_value = fidtBadVal;   fhdr.missing_data_value = fidtBadVal;
  //
  sprintf( fhdr.field_name_long,"%s", fidtLongName);
  sprintf( fhdr.field_name,"%s", "fidt" );
  sprintf( fhdr.units,"%s", fidtUnits);
  //
  //
  MdvxField *fidtfield;
  //
  fidtfield = new MdvxField(fhdr, vhdr, fidt );
  //
  if ( fidtfield->convertRounded(Mdvx::ENCODING_INT16,
				Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }
  
  outMdvx.addField(fidtfield);
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
  free(gsd); free(fidt); free(fed);

}
//
// The following is a small method that exits if things
// have gone wrong with a netCDF read. An error string is printed first.
//
void VaisalaLtg2Mdv::_checkStatus(int status, char *exitStr){

  if (status != NC_NOERR){
    cerr << exitStr << endl;
    exit(0);
  }

}
