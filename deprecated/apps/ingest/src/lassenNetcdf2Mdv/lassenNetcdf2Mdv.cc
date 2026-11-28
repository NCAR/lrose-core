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

/////////////////////////////////////////////////////////////
//
// Inits FMQ in constructor, sends data in method.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#include <netcdf.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>


#include "lassenNetcdf2Mdv.hh"

// Constructor
lassenNetcdf2Mdv::lassenNetcdf2Mdv ( Params *TDRP_params ){
  
  _params = TDRP_params;

  return;
}

// Process volumes.
void lassenNetcdf2Mdv::processFile( char *File, time_t dataTime ){

  //
  // Pull both files into memory.
  //
  int netID;
  int status = nc_open(File, NC_NOWRITE, &netID);
  if ( _checkStatus(status, "Failed to open input file.")){
    cerr << "Input file was " << File << endl;
    return;
  }


  //
  // Get the dimensions of the array so we can allocate space and read it in.
  // I'm assuming that U and Z have the same dimensions.
  //
  int dimID;
  status = nc_inq_dimid(netID, "East-West", &dimID);
  if ( _checkStatus(status, "Failed to get nx dimension ID.")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }


  size_t nx;
  status = nc_inq_dimlen(netID, dimID, &nx);
  if ( _checkStatus(status, "Failed to get nx dimension.")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }


  status = nc_inq_dimid(netID, "North-South", &dimID);
  if ( _checkStatus(status, "Failed to get ny dimension ID.")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }

  size_t ny;
  status = nc_inq_dimlen(netID, dimID, &ny);
  if ( _checkStatus(status, "Failed to get ny dimension.")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }


  //
  // Get some other information
  //
  float lat, lon, elev;
  elev = 0.0; lat=0.0; lon = 0.0; // Just to avoid compiler warnings
  int varID;

  if (_params->useLocationChars){

    bool gotLoc = false;
    char locChar = File[strlen(File)-strlen("AP_070010000_I.nc")];

    if (locChar == 'A'){
      lat=47.28540; lon=8.51301; elev = 930; gotLoc = true;
    }

    if (locChar == 'D'){
      lat=46.42616; lon=6.10016; elev = 1680; gotLoc = true;
    }

    if (locChar == 'L'){
      lat=46.04179; lon=8.83436; elev = 1630; gotLoc = true;
    }

    if (locChar == 'C'){
      lat=46.6733; lon=7.46555; elev = 0.0; gotLoc = true;
    }

    if (!(gotLoc)){
      cerr << "Did not recognize location character in filename : ";
      cerr << locChar << endl;
      nc_close(netID);
      cerr << "Input file was " << File << endl;
      return;
    }


  } else {

    status = nc_inq_varid(netID, "Latitude", &varID);
    if ( _checkStatus(status, "Failed to get lat variable ID")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }

    status = nc_get_var_float(netID, varID, &lat);
    if ( _checkStatus(status, "Failed to read lat")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }

 
    status = nc_inq_varid(netID, "Longitude", &varID);
    if ( _checkStatus(status, "Failed to get lon variable ID")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }

    status = nc_get_var_float(netID, varID, &lon);
    if ( _checkStatus(status, "Failed to read lon")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }

  }

  float dx; 
  status = nc_inq_varid(netID, "ResolutionColumn", &varID);
  if ( _checkStatus(status, "Failed to get dx variable ID")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }

  status = nc_get_var_float(netID, varID, &dx);
  if ( _checkStatus(status, "Failed to read dx")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }


  float dy; 
  status = nc_inq_varid(netID, "ResolutionLine", &varID);
  if ( _checkStatus(status, "Failed to get dy variable ID")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }

  status = nc_get_var_float(netID, varID, &dy);
  if ( _checkStatus(status, "Failed to read dy")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }


  float missing; 
  status = nc_inq_varid(netID, "Missvalue", &varID);
  if ( _checkStatus(status, "Failed to get missing variable ID")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }
  status = nc_get_var_float(netID, varID, &missing);
  if ( _checkStatus(status, "Failed to read missing")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }


  if (_params->debug){
    cerr << "Data are " << nx << " by " << ny << " resolution " << dx << " by " << dy << endl;
    cerr << "Central location is " << lat << ", " << lon << endl;
  }

  float *data = (float *) malloc(sizeof(float)*nx*ny);

  if (data == NULL){
    cerr << "Malloc failed!" << endl;
    exit(-1);
  }

  status = nc_inq_varid(netID, "Datafield", &varID);
  if ( _checkStatus(status, "Failed to get datafield variable ID")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }

  status = nc_get_var_float(netID, varID, data);
  if ( _checkStatus(status, "Failed to read data")){
    nc_close(netID);
    cerr << "Input file was " << File << endl;
    return;
  }


  //
  // Close the netCDF file
  //
  nc_close(netID);

  //
  // Convert to rate, if desired.
  //
  if (_params->rrConvert.convertToRate){

    bool first = true;
    double min, max;
    min = 0.0; max = min;
    for (unsigned i=0; i < nx*ny; i++){
      if (data[i] != missing){

	double q = pow(10.0, data[i]/10.0)/_params->rrConvert.a;
	data[i] = pow(q, 1.0/_params->rrConvert.b);

	if (first){
	  first = false;
	  min = data[i]; max = min;
	} else {
	  if (data[i] < min) min = data[i];
	  if (data[i] > max) max = data[i];
	}

	if (data[i] < _params->rrConvert.min){
	  data[i] = missing;
	}

      }
    }

    if (_params->debug){
      if (first){
	cerr << "No valid rain rates could be calculated." << endl;
      } else {
	cerr << "Rain rate runs from " << min << " to " << max << endl;
      }
    }

  }


  Mdvx::field_header_t fhdr;
  Mdvx::vlevel_header_t vhdr;
  Mdvx::master_header_t Mhdr;
  //
  // Use 'memset' to set all the bytes in these structures to 0.
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


  fhdr.nx = nx;
  fhdr.ny = ny;
  fhdr.nz = 1;
  //
  fhdr.grid_dx = dx;
  fhdr.grid_dy = dy;
  //
  fhdr.proj_type = Mdvx::PROJ_FLAT; // This is implied for these data.
  //
  fhdr.proj_origin_lat =  lat;
  fhdr.proj_origin_lon =  lon;
  //
  // 
  fhdr.grid_minx = (dx - nx*dx)/2.0;
  fhdr.grid_miny = (dy - ny*dy)/2.0;
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
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  Mhdr.vlevel_included = 1;
  //
  //
  sprintf(Mhdr.data_set_info,"%s","lassenNetcdf2Mdv");
  sprintf(Mhdr.data_set_name,"%s","lassenNetcdf2Mdv");
  sprintf(Mhdr.data_set_source,"%s", "lassenNetcdf2Mdv");

  Mhdr.time_gen = dataTime;
  Mhdr.time_begin = dataTime;
  Mhdr.time_end = dataTime;
  Mhdr.time_expire = dataTime;
  Mhdr.time_centroid = dataTime;
  //
  fhdr.forecast_time = dataTime;
  //
  fhdr.forecast_delta = 0;

  fhdr.bad_data_value = missing;   fhdr.missing_data_value = missing;
  //
  sprintf( fhdr.field_name_long,"%s", _params->mdvSpec.fieldName);
  sprintf( fhdr.field_name,"%s", _params->mdvSpec.fieldName );
  sprintf( fhdr.units,"%s", _params->mdvSpec.units);
  sprintf( fhdr.transform,"%s","none");
  
  //
  // Declare a DsMdvx object so we can load it up with fields.
  //
  DsMdvx outMdvx;
  //
  outMdvx.setMasterHeader(Mhdr); 

  MdvxField *field;
  //
  field = new MdvxField(fhdr, vhdr, data);
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
  if (outMdvx.writeToDir(_params->mdvSpec.url)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << _params->mdvSpec.url << endl;
    cerr << outMdvx.getErrStr() << endl;
  }

  free(data);

  return;

}


// Destructor.
lassenNetcdf2Mdv::~lassenNetcdf2Mdv (){
  return;
}


//
// The following is a small method that exits if things
// have gone wrong with a netCDF read. An error string is printed first.
//
int lassenNetcdf2Mdv::_checkStatus(int status, char *exitStr){

  if (status != NC_NOERR){
    cerr << exitStr << endl;
    return -1;
  }

  return 0;

}



