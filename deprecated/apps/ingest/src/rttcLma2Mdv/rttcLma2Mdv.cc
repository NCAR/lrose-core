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
// This is the module of the rttcLma2Mdv application that does most
// of the work, reading the netCDF and writing the MDV files. Include
// both MDV and netCDF header files.
//
#include "rttcLma2Mdv.hh"

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
rttcLma2Mdv::rttcLma2Mdv(Params *P){
  _params = P;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
rttcLma2Mdv::~rttcLma2Mdv(){
  return;
}
//
// Main method - the netCDF -> MDV conversion.
//
void rttcLma2Mdv::rttcLma2MdvFile( char *FilePath ){
  //
  // If the input file is gzipped, apply gunzip with a system() call.
  // If I had to open the file I'd use the toolsa open uncompress
  // call, but I use the netCDF stuff to get at the file, so that
  // isn't really an option...
  //
  char *p = FilePath + strlen(FilePath) - 3;
  if (!(strcmp(p, ".gz"))){
    char com[1024];
    sprintf(com,"gunzip -f %s", FilePath);
    if (_params->debug){
      fprintf(stderr,"Applying command %s\n", com);
    }
    system(com);
    sleep(1);
    *p = char(0); // truncate .gz from end of filename.
  }

  if (_params->debug){
    fprintf(stderr,"Processing file %s\n", FilePath );
  }

  //
  // Parse the data time from the filename (it does
  // not seem to be in the file anywhere).
  //
  p = FilePath + strlen(FilePath) - strlen("20060209_2026");
  date_time_t dataTime;
  dataTime.sec = 0;
  if (5 != sscanf(p, "%4d%2d%2d_%2d%2d",
		  &dataTime.year, &dataTime.month, &dataTime.day,
		  &dataTime.hour, &dataTime.min)){
    fprintf(stderr,"Unable to parse time from filename %s - skipping ...\n",
	    FilePath);
    return;
  }
  uconvert_to_utime( &dataTime );
  //
  // Open the netCDF input file.
  //
  int netID;
  int status = nc_open(FilePath, NC_NOWRITE, &netID);
  _checkStatus(status, "Failed to open input file.");

  int varID;

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
  for (int i=0; i < _params->Zlevels_n; i++){
    vhdr.type[i] = Mdvx::VERT_TYPE_Z;
    vhdr.level[i] = _params->_Zlevels[i];
  }
  //
  // Then set up the field header with the metadata.
  //
  fhdr.nx = _params->gridDef.nx;
  fhdr.ny = _params->gridDef.ny;
  fhdr.nz = _params->Zlevels_n;
  //
  fhdr.grid_dx = _params->gridDef.dx;
  fhdr.grid_dy = _params->gridDef.dy;
  //
  fhdr.proj_type = Mdvx::PROJ_FLAT; // Implied for these data.
  //
  fhdr.proj_origin_lat =  _params->gridDef.orgLat;
  fhdr.proj_origin_lon =  _params->gridDef.orgLon;
  //
  //
  fhdr.grid_minx = _params->gridDef.minx;
  fhdr.grid_miny = _params->gridDef.miny;
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
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
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
  sprintf(Mhdr.data_set_info,"%s","rttcLma2Mdv");
  sprintf(Mhdr.data_set_name,"%s","rttcLma2Mdv");
  sprintf(Mhdr.data_set_source,"%s", "rttcLma2Mdv");
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
  // Use the name to get the netCDF variable ID.
  //
  status = nc_inq_varid(netID, "rfd", &varID);
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
  float *buffer = (float *) malloc(sizeof(float)*fhdr.nx*fhdr.ny*fhdr.nz);
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
  // For interest's sake, count the number of non-missing grid points.
  //
  int numGood = 0;
  for (int k=0; k < fhdr.nx*fhdr.ny*fhdr.nz; k++){
    if (buffer[k] != fillVal) numGood++;
  }
  //
  // Complete filling out the field header with this information.
  //
  fhdr.bad_data_value = fillVal;   fhdr.missing_data_value = fillVal;
  //
  sprintf( fhdr.field_name_long,"%s", longName);
  sprintf( fhdr.field_name,"%s", "rfd");
  sprintf( fhdr.units,"%s", units);
  sprintf( fhdr.transform,"%s","none");
  //
  // Print some debugging, if requested.
  //
  if (_params->debug){
    double pg = 100.0*double(numGood)/double(fhdr.nx*fhdr.ny*fhdr.nz);
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
  if (field->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }
  
  outMdvx.addField(field);
  
  free(buffer);

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
void rttcLma2Mdv::_checkStatus(int status, char *exitStr){
  
  if (status != NC_NOERR){
    cerr << exitStr << endl;
    exit(0);
  }

}
