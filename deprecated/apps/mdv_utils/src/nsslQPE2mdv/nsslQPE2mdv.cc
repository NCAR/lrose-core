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
// This is the module of the nsslQPE2mdv application that does most
// of the work, reading the netCDF and writing the MDV files. Include
// both MDV and netCDF header files.
//
#include "nsslQPE2mdv.hh"

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
nsslQPE2mdv::nsslQPE2mdv(Params *P){
  _params = P;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
nsslQPE2mdv::~nsslQPE2mdv(){
  return;
}
//
// Main method - the netCDF -> MDV conversion.
//
void nsslQPE2mdv::nsslQPE2mdvFile( char *FilePath ){
  //
  // Open the netCDF input file.
  //
  int netID;
  int status = nc_open(FilePath, NC_NOWRITE, &netID);
  _checkStatus(status, "Failed to open input file.");

  int varID;


  // Get array sizes.

  int latDimId;
  status = nc_inq_dimid(netID, "Lat", &latDimId);
  _checkStatus(status, "Failed to get dimension ID for Lat");
  size_t ny;
  status = nc_inq_dimlen(netID, latDimId, &ny);
  _checkStatus(status, "Failed to get number of points in Y");

  int lonDimId;
  status = nc_inq_dimid(netID, "Lon", &lonDimId);
  _checkStatus(status, "Failed to get dimension ID for Lon");
  size_t nx;
  status = nc_inq_dimlen(netID, lonDimId, &nx);
  _checkStatus(status, "Failed to get number of points in X");


  // Get spacing in X and Y, stored as attributes.

  double dy;
  status = nc_get_att_double(netID, NC_GLOBAL, "LatGridSpacing", &dy);
  _checkStatus(status, "Failed to get spacing in Y");

  double dx;
  status = nc_get_att_double(netID, NC_GLOBAL, "LatGridSpacing", &dx);
  _checkStatus(status, "Failed to get spacing in X");


  // Get the time.

  time_t dataTime;
  status =  nc_get_att_long(netID, NC_GLOBAL, "Time", &dataTime);
  _checkStatus(status, "Failed to get data time");

  // Get the upper right lat, lon

  double urLat;
  status = nc_get_att_double(netID, NC_GLOBAL, "Latitude", &urLat);
  _checkStatus(status, "Failed to get upper right lat");

  double urLon;
  status = nc_get_att_double(netID, NC_GLOBAL, "Longitude", &urLon);
  _checkStatus(status, "Failed to get upper right lon");


  //
  // Print the metadata we have read so far, if we are in debug mode.
  //
  if (_params->debug){
    fprintf(stderr,"nx=%d\n",(int)nx);
    fprintf(stderr,"ny=%d\n",(int)ny);
    fprintf(stderr,"dx, dy=%g, %g degrees\n",dx, dy);
    fprintf(stderr,"Valid time : %s\n", utimstr(dataTime));    
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
  fhdr.nx = nx;
  fhdr.ny = ny;
  fhdr.nz = 1;
  //
  fhdr.grid_dx = dx;
  fhdr.grid_dy = dy;
  //
  fhdr.proj_type = Mdvx::PROJ_LATLON; // This is implied for these data.
  //
  fhdr.proj_origin_lat =  urLat - ny*dy;
  fhdr.proj_origin_lon =  urLon;
  //
  //
  fhdr.grid_minx = urLon;
  fhdr.grid_miny = urLat - ny*dy;
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
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  Mhdr.vlevel_included = 1;
  //
  //
  sprintf(Mhdr.data_set_info,"%s","nsslQPE2mdv");
  sprintf(Mhdr.data_set_name,"%s","nsslQPE2mdv");
  sprintf(Mhdr.data_set_source,"%s", "nsslQPE2mdv");
  //
  // Set the times in the master and field headers.
  //

  Mhdr.time_gen = dataTime;
  Mhdr.time_begin = dataTime;
  Mhdr.time_end = dataTime;
  Mhdr.time_expire = dataTime + _params->Expiry;
  Mhdr.time_centroid = dataTime;
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
    status =  nc_get_att_text(netID, varID, "Units", units);
    _checkStatus(status, "Failed to get the Units string.");
    //
    // Read in the long version of the variable name.
    //
    char longName[128]; for(int l=0; l<128; l++) longName[l]=char(0);
    status =  nc_get_att_text(netID, varID, "TypeName", longName);
    _checkStatus(status, "Failed to get the TypeName string.");
    //
    // Get the scale we have to apply to these data.
    //
    double scale;
    status =  nc_get_att_double(netID, varID, "Scale", &scale);
    _checkStatus(status, "Failed to get the scale.");
    //
    // Get the value used for missing data at a gridpoint.
    //
    float fillVal;
    status =  nc_get_att_float(netID, varID, "MissingData", &fillVal);
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
    si16 *bufferShort = (si16 *) malloc(sizeof(si16)*nx*ny);
    if (bufferShort == NULL){
      cerr << "Malloc failed." << endl;
      exit(-1);
    }
    //
    // ... and read the grid data.
    //
    status = nc_get_var_short(netID, varID, bufferShort);
    _checkStatus(status, "Failed to read variable.");


    // Then convert to fl32
    
    fl32 *buffer = (fl32 *) malloc(sizeof(fl32)*nx*ny);
    if (buffer == NULL){
      cerr << "Malloc failed." << endl;
      exit(-1);
    }

    for (unsigned i=0; i < nx*ny; i++){
      if (bufferShort[i] == fillVal){
	buffer[i] = fillVal;
      } else {
	buffer[i] = double(bufferShort[i])/scale;
      }
    }

    free(bufferShort);

    //
    // For intetest's sake, count the number of non-missing grid points.
    //
    int numGood = 0;
    for (unsigned k=0; k < nx*ny; k++){
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
      double pg = 100.0*double(numGood)/double(nx*ny);
      cerr << "Field is " << pg << " percent good." << endl << endl;
    }
    //
    // Now we are ready to add these data to our DsMdvx object.
    // We do this by combining the field header, vlevel header
    // and the actual grid data into a MdvxField object, then
    // setting compression and encoding type for this field and
    // finally adding the field to the DsMdvx object.
    //
    MdvxField *field;
    //
    field = new MdvxField(fhdr, vhdr, buffer);

    // If we are remapping the data, do that.

    if (_params->remapData){

      MdvxRemapLut lut;

      switch (_params->output_projection){


      case Params::OUTPUT_PROJ_FLAT :
	if (field->remap2Flat(lut,
			      _params->output_grid.nx,
			      _params->output_grid.ny,
			      _params->output_grid.minx,
			      _params->output_grid.miny,
			      _params->output_grid.dx,
			      _params->output_grid.dy,
			      _params->output_origin.lat,
			      _params->output_origin.lon,
			      _params->output_rotation)){
	  cerr << "Remap to flat grid failed" << endl;
	  exit(-1);
	}

	break;

      case Params::OUTPUT_PROJ_LATLON :

	if (field->remap2Latlon(lut,
			      _params->output_grid.nx,
			      _params->output_grid.ny,
			      _params->output_grid.minx,
			      _params->output_grid.miny,
			      _params->output_grid.dx,
			      _params->output_grid.dy)){
	  cerr << "Remap to latlon grid failed" << endl;
	  exit(-1);
	}

	break;

      case Params::OUTPUT_PROJ_LAMBERT :

	if (field->remap2Lc2(lut,
			     _params->output_grid.nx,
			     _params->output_grid.ny,
			     _params->output_grid.minx,
			     _params->output_grid.miny,
			     _params->output_grid.dx,
			     _params->output_grid.dy,
			     _params->output_origin.lat,
			     _params->output_origin.lon,
			     _params->output_std_parallels.lat_1,
			     _params->output_std_parallels.lat_2)){
	  cerr << "Remap to lambert grid failed" << endl;
	  exit(-1);
	}

	break;

      default :
	cerr << "Unreconised code for output projection : " << _params->output_projection << endl;
	exit(-1);
	break;

      }

    }

    //
    if (field->convertRounded(Mdvx::ENCODING_INT16,
				    Mdvx::COMPRESSION_ZLIB)){
      fprintf(stderr, "conversion of field failed - I cannot go on.\n");
      exit(-1);
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
void nsslQPE2mdv::_checkStatus(int status, char *exitStr){

  if (status != NC_NOERR){
    cerr << exitStr << endl;
    exit(0);
  }

}
