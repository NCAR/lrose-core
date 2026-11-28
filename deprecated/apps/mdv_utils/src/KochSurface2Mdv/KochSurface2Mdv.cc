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
// This is the module of the KochSurface2Mdv application that does most
// of the work, reading the netCDF and writing the MDV files. Include
// both MDV and netCDF header files.
//
#include "KochSurface2Mdv.hh"

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
KochSurface2Mdv::KochSurface2Mdv(Params *P){
  _params = P;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
KochSurface2Mdv::~KochSurface2Mdv(){
  return;
}
//
// Main method - the netCDF -> MDV conversion.
//
void KochSurface2Mdv::KochSurface2MdvFile( char *FilePath ){
  //
  // Open the netCDF input file.
  //
  int netID;
  int status = nc_open(FilePath, NC_NOWRITE, &netID);
  _checkStatus(status, "Failed to open input file.");

  int varID;

  //
  // Start reading the metadata associated with the grid - the
  // number of points, grid spacing, etc.
  //
  //////////////////////////////////////////////////////
  //
  // Read the number of points in X, Nx.
  //
  int Nx; // Number of X points.
  status = nc_inq_varid(netID, "Nx", &varID);
  _checkStatus(status, "Failed to get Nx variable ID");

  status = nc_get_var_int(netID, varID, &Nx);
  _checkStatus(status, "Failed to read Nx integer");

  //////////////////////////////////////////////////////
  //
  // Read the number of points in Y, Ny.
  //
  int Ny; // Number of Y points.
  status = nc_inq_varid(netID, "Ny", &varID);
  _checkStatus(status, "Failed to get Ny variable ID");

  status = nc_get_var_int(netID, varID, &Ny);
  _checkStatus(status, "Failed to read Ny integer");


  //////////////////////////////////////////////////////
  //
  // Read the central longitude of the projection.
  //
  float LoV; // Central longitude
  status = nc_inq_varid(netID, "LoV", &varID);
  _checkStatus(status, "Failed to get LoV variable ID");

  status = nc_get_var_float(netID, varID, &LoV);
  _checkStatus(status, "Failed to read LoV");

  if (LoV > 180.0){
    LoV -= 360.0;
  }

  //////////////////////////////////////////////////////
  //
  // Read the first lambert conformal latitude.
  //
  float Latin1; // First lambert conformal latitude
  status = nc_inq_varid(netID, "Latin1", &varID);
  _checkStatus(status, "Failed to get Latin1 variable ID");

  status = nc_get_var_float(netID, varID, &Latin1);
  _checkStatus(status, "Failed to read Latin1");

  //////////////////////////////////////////////////////
  //
  // Read the second lambert conformal latitude.
  //
  float Latin2; // Second lambert conformal latitude
  status = nc_inq_varid(netID, "Latin2", &varID);
  _checkStatus(status, "Failed to get Latin2 variable ID");

  status = nc_get_var_float(netID, varID, &Latin2);
  _checkStatus(status, "Failed to read Latin2");

  //////////////////////////////////////////////////////
  //
  // Read the grid spacing in X in meters.
  //
  float Dx; // X increment
  status = nc_inq_varid(netID, "Dx", &varID);
  _checkStatus(status, "Failed to get Dx variable ID");

  status = nc_get_var_float(netID, varID, &Dx);
  _checkStatus(status, "Failed to read Dx");

  //////////////////////////////////////////////////////
  //
  // Read the grid spacing in Y in meters.
  //
  float Dy; // Y increment
  status = nc_inq_varid(netID, "Dy", &varID);
  _checkStatus(status, "Failed to get Dy variable ID");

  status = nc_get_var_float(netID, varID, &Dy);
  _checkStatus(status, "Failed to read Dy");

  //////////////////////////////////////////////////////
  //
  // Read the valid time for these data.
  //
  double valtime; // Valid time for these data
  status = nc_inq_varid(netID, "valtime", &varID);
  _checkStatus(status, "Failed to get valtime variable ID");

  status = nc_get_var_double(netID, varID, &valtime);
  _checkStatus(status, "Failed to read valtime");

  //////////////////////////////////////////////////////
  //
  // Read the reference time for these data.
  //
  double reftime; // Ref time for these data
  status = nc_inq_varid(netID, "reftime", &varID);
  _checkStatus(status, "Failed to get reftime variable ID");

  status = nc_get_var_double(netID, varID, &reftime);
  _checkStatus(status, "Failed to read reftime");

  //////////////////////////////////////////////////////
  //
  // Read the latitude of the lower left point.
  //
  float La1; // First latitude
  status = nc_inq_varid(netID, "La1", &varID);
  _checkStatus(status, "Failed to get La1 variable ID");

  status = nc_get_var_float(netID, varID, &La1);
  _checkStatus(status, "Failed to read La1");

  //////////////////////////////////////////////////////
  //
  // Read the longitude of the lower left point.
  //
  float Lo1; // First longitude
  status = nc_inq_varid(netID, "Lo1", &varID);
  _checkStatus(status, "Failed to get Lo1 variable ID");

  status = nc_get_var_float(netID, varID, &Lo1);
  _checkStatus(status, "Failed to read Lo1");

  if (Lo1 > 180.0){
    Lo1 -= 360.0;
  }
  //
  // Print the metadata we have read so far, if we are in debug mode.
  //
  if (_params->debug){
    fprintf(stderr,"Nx=%d\n",Nx);
    fprintf(stderr,"Ny=%d\n",Ny);
    fprintf(stderr,"LoV=%g\n",LoV);
    fprintf(stderr,"Latin1, Latin2=%g, %g\n",Latin1, Latin2);
    fprintf(stderr,"Dx, Dy=%g, %g\n",Dx, Dy);
    fprintf(stderr,"Valid time : %s\n", utimstr((long)valtime));
    fprintf(stderr,"Reference time : %s\n", utimstr((long)reftime));
    fprintf(stderr,"La1, Lo1=%g, %g\n", La1, Lo1);
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
  fhdr.nx = Nx;
  fhdr.ny = Ny;
  fhdr.nz = 1;
  //
  fhdr.grid_dx = Dx/1000.0; // Factor of 1000.0 converts from meters 
  fhdr.grid_dy = Dy/1000.0; // to kilometers.
  //
  fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF; // This is implied for these data.
  //
  fhdr.proj_origin_lat =  Latin1;
  fhdr.proj_origin_lon =  LoV;
  //
  // Calculate the distance from the origin to the lower left grid point
  // to get the minx, miny distances in Kilometers.
  //
  double minx, miny;
  PJGLatLon2DxDy(Latin1, LoV,  La1, Lo1, &minx, &miny);
  //
  fhdr.grid_minx = minx;
  fhdr.grid_miny = miny;
  //
  // For Lambert Conformal, the first two proj params are set to the
  // lambert conformal latitudes.
  //
  fhdr.proj_param[0]=Latin1;
  fhdr.proj_param[1]=Latin2;
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
  Mhdr.data_collection_type = Mdvx::DATA_FORECAST;
  Mhdr.num_data_times = 1;
  Mhdr.data_ordering = Mdvx::ORDER_XYZ;  
  Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  Mhdr.vlevel_included = 1;
  //
  //
  sprintf(Mhdr.data_set_info,"%s","KochSurface2Mdv");
  sprintf(Mhdr.data_set_name,"%s","KochSurface2Mdv");
  sprintf(Mhdr.data_set_source,"%s", "KochSurface2Mdv");
  //
  // Set the times in the master and field headers.
  //
  date_time_t T;
  T.unix_time = (long) valtime;
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
  // Loop through the field names that we want, adding them to the
  // DsMdvx object as we go.
  //
  for (int i=0; i < _params->fieldNames_n; i++){
    //
    // Get the variable name.
    //
    char *name = _params->_fieldNames[i];
    //
    // Use the name to get the netCDF variable ID.
    //
     status = nc_inq_varid(netID, name, &varID);
    _checkStatus(status, "Failed to get variable ID");
    //
    // Get the string that holds the units for this variable.
    //
    char units[128]; for(int l=0; l<128; l++) units[l]=char(0);
    status =  nc_get_att_text(netID, varID, "units", units);
    _checkStatus(status, "Failed to get the units string.");
    //
    // Read in the long version of the variable name.
    //
    char longName[128]; for(int l=0; l<128; l++) longName[l]=char(0);
    status =  nc_get_att_text(netID, varID, "long_name", longName);
    _checkStatus(status, "Failed to get the long_name string.");
    //
    // Get the value used for missing data at a gridpoint.
    //
    float fillVal;
    status =  nc_get_att_float(netID, varID, "_FillValue", &fillVal);
    _checkStatus(status, "Failed to get the fill value.");
    //
    // Print some debugging messages, if appropriate.
    //
    if (_params->debug){
      cerr << longName << endl;
      cerr << "Units : " << units << " Fillvalue : " << fillVal << endl;
    }
    //
    // Allocate enough space for the actual grid data ...
    //
    float *buffer = (float *) malloc(sizeof(float)*Nx*Ny);
    if (buffer == NULL){
      cerr << "Malloc failed." << endl;
      exit(-1);
    }
    //
    // ... and read the grid data.
    //
    status = nc_get_var_float(netID, varID, buffer);
    _checkStatus(status, "Failed to read variable.");
    //
    // For intetest's sake, count the number of non-missing grid points.
    //
    int numGood = 0;
    for (int k=0; k < Nx*Ny; k++){
      if (buffer[k] != fillVal) numGood++;
    }
    //
    // Complete filling out the field header with this information.
    //
    fhdr.bad_data_value = fillVal;   fhdr.missing_data_value = fillVal;
    //
    sprintf( fhdr.field_name_long,"%s", longName);
    sprintf( fhdr.field_name,"%s", name);
    sprintf( fhdr.units,"%s", units);
    sprintf( fhdr.transform,"%s","none");
    //
    // Print some debugging, if requested.
    //
    if (_params->debug){
      double pg = 100.0*double(numGood)/double(Nx*Ny);
      cerr << "Field is " << pg << " percent good." << endl << endl;
    }
    //
    // Now we are ready to add these data to our DsMdvx object.
    // We fdo this by combining the field header, vlevel header
    // and the actual grid data into a MdvxField object, then
    // setting compression and encoding type for this field and
    // finally adding the field to the DsMdvx object.
    //
    MdvxField *field;
    //
    field = new MdvxField(fhdr, vhdr, buffer);
    //
    if (field->convertRounded(Mdvx::ENCODING_INT16,
				    Mdvx::COMPRESSION_ZLIB)){
      fprintf(stderr, "conversion of field failed - I cannot go on.\n");
      exit(-1);
    }

	//remap field
	if(_params->remap_output) {
	  _remap(field);
    }

    outMdvx.addField(field);

    free(buffer);

  } // End of loop through desired field names.

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

}
//
// The following is a small method that exits if things
// have gone wrong with a netCDF read. An error string is printed first.
//
void KochSurface2Mdv::_checkStatus(int status, char *exitStr){

  if (status != NC_NOERR){
    cerr << exitStr << endl;
    exit(0);
  }

}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	KochSurface2Mdv::_remap
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

void 
KochSurface2Mdv::_remap(MdvxField* field)
{
  const string methodName = "KochSurface2Mdv::_remap";

  if(_params->remap_output) {

    MdvxRemapLut lut;

    switch( _params->out_projection_info.type) {
    case Params::PROJ_FLAT:
      field->remap2Flat(lut, _params->out_grid_info.nx, _params->out_grid_info.ny, 
			_params->out_grid_info.minx, _params->out_grid_info.miny, 
			_params->out_grid_info.dx, _params->out_grid_info.dy, 
			_params->out_projection_info.origin_lat, _params->out_projection_info.origin_lon,
			_params->out_projection_info.rotation);
      break;
	  
    case Params::PROJ_LATLON:
      field->remap2Latlon(lut, _params->out_grid_info.nx, _params->out_grid_info.ny, 
			  _params->out_grid_info.minx, _params->out_grid_info.miny, 
			  _params->out_grid_info.dx, _params->out_grid_info.dy );
      break;

    case Params::PROJ_LAMBERT_CONF:
      field->remap2Lc2(lut, _params->out_grid_info.nx, _params->out_grid_info.ny, 
		       _params->out_grid_info.minx, _params->out_grid_info.miny, 
		       _params->out_grid_info.dx, _params->out_grid_info.dy,
		       _params->out_projection_info.origin_lat, _params->out_projection_info.origin_lon,
		       _params->out_projection_info.ref_lat_1, _params->out_projection_info.ref_lat_2 );
      break;
    default:
      cerr << "remap -- unknown projection; remapping failed." << endl;
    }

  } // endif -- _params->remap_output

}


