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
// This is the module of the timrexSat2mdv application that does most
// of the work, reading the netCDF and writing the MDV files. Include
// both MDV and netCDF header files.
//
#include "timrexSat2mdv.hh"

#include <iostream>
#include <dataport/swap.h>
#include <stdlib.h>
#include <cstdio>

#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/pjg_flat.h>
//
// Constructor - makes a copy of a pointer to the TDRP parameters.
//
timrexSat2mdv::timrexSat2mdv(Params *P){
  _params = P;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
timrexSat2mdv::~timrexSat2mdv(){
  return;
}
//
// Main method - the netCDF -> MDV conversion.
//
void timrexSat2mdv::timrexSat2mdvFile( char *FilePath ){

  if (_params->debug){
    fprintf(stderr,"Processing file %s\n", FilePath );
  }

  //
  // Parse the data time from the filename (it does
  // not seem to be in the file anywhere).
  //
  char *p = FilePath + strlen(FilePath) - strlen("YYYY-MM-DD_hhmm.VIS.2byte");
  date_time_t dataTime;
  dataTime.sec = 0;
  if (5 != sscanf(p, "%4d-%2d-%2d_%2d%2d",
		  &dataTime.year, &dataTime.month, &dataTime.day,
		  &dataTime.hour, &dataTime.min)){
    fprintf(stderr,"Unable to parse time from filename %s - skipping ...\n",
	    FilePath);
    return;
  }
  uconvert_to_utime( &dataTime );
  //
  // Open the input file and read the unsigned 2 byte count data.
  //
  FILE *fp = fopen(FilePath, "r");
  if (fp == NULL){
    cerr << "Unable to open " << FilePath << endl;
    return;
  }


  ui16 *countData = (ui16 *) malloc(2* _params->gridDef.nx * _params->gridDef.ny );
  if (countData == NULL){
    cerr << "Malloc failed for count data!" << endl;
    exit(-1);
  }

  if (_params->gridDef.nx * _params->gridDef.ny !=
     (int) fread(countData, sizeof(ui16),  _params->gridDef.nx * _params->gridDef.ny, fp)){
    free(countData);
    fclose(fp);
    cerr << "Read failed from " << FilePath << endl;
    return;
  }

  fclose(fp);

  if (_params->byteSwap){
    SWAP_array_16(countData, 2* _params->gridDef.nx * _params->gridDef.ny );
  }

  //
  // Convert the data to albedoes.
  //

  fl32 *albedoData = (fl32 *) malloc(sizeof(fl32) * _params->gridDef.nx * _params->gridDef.ny );
  if (albedoData == NULL){
    cerr << "Malloc failed for albedo data!" << endl;
    exit(-1);
  }

  double min, max;
  min = max = 100.0*(countData[0])/1023.0;
  for (int i=0; i <  _params->gridDef.nx * _params->gridDef.ny; i++){
    albedoData[i] = 100.0*(countData[i])/1023.0;
    if (albedoData[i] < min) min = albedoData[i];
    if (albedoData[i] > max) max = albedoData[i];
  }

  free(countData);

  if (_params->debug)
    cerr << "Data range from " << min << " to " << max << endl;

  fl32 *buffer = (fl32 *) malloc(sizeof(fl32) * _params->gridDef.nx * _params->gridDef.ny );
  if (buffer == NULL){
    cerr << "Malloc failed for albedo workspace!" << endl;
    exit(-1);
  }

  for (int ix=0; ix < _params->gridDef.nx; ix++){
    for (int iy=0; iy < _params->gridDef.ny; iy++){
      int revIy = _params->gridDef.ny - iy - 1;
      buffer[ix + _params->gridDef.nx * iy] = albedoData[ix + _params->gridDef.nx * revIy];
    }
  }

  for (int j=0; j < _params->gridDef.nx * _params->gridDef.ny; j++)
    albedoData[j] = buffer[j];

  free(buffer);

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
   
  //
  // Then set up the field header with the metadata.
  //
  fhdr.nx = _params->gridDef.nx;
  fhdr.ny = _params->gridDef.ny;
  fhdr.nz = 1;
  //
  fhdr.grid_dx = _params->gridDef.dx;
  fhdr.grid_dy = _params->gridDef.dy;
  //
  fhdr.proj_type = Mdvx::PROJ_LATLON; // Implied for these data.
  //
  fhdr.proj_origin_lat =  _params->gridDef.orgLat;
  fhdr.proj_origin_lon =  _params->gridDef.orgLon;
  //
  //
  fhdr.grid_minx = fhdr.proj_origin_lon;
  fhdr.grid_miny = fhdr.proj_origin_lat;
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
  Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  Mhdr.num_data_times = 1;
  Mhdr.data_ordering = Mdvx::ORDER_XYZ;  
  Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  Mhdr.vlevel_included = 1;
  //
  //
  sprintf(Mhdr.data_set_info,"%s","timrexSat2mdv");
  sprintf(Mhdr.data_set_name,"%s","timrexSat2mdv");
  sprintf(Mhdr.data_set_source,"%s", "timrexSat2mdv");
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
  //
  //
  //
  fhdr.bad_data_value = -1.0;   fhdr.missing_data_value = -1.0;
  //
  sprintf( fhdr.field_name_long,"vis" );
  sprintf( fhdr.field_name,"vis");
  sprintf( fhdr.units,"percent" );
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
  field = new MdvxField(fhdr, vhdr, albedoData);
  //
  if (field->convertRounded(Mdvx::ENCODING_INT16,
			    Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }
  
  outMdvx.addField(field);
  
  free(albedoData);

  //
  // Finally, now that the DsMdvx object is all loaded up, use it
  // to write out the data.
  //
  if (outMdvx.writeToDir(_params->output_url)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << _params->output_url << endl;
    cerr << outMdvx.getErrStr() << endl;
  }
  
}
