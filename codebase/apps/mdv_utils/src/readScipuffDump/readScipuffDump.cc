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
// This is the module of the readScipuffDump application that does most
// of the work, reading the input and writing the MDV files. Include
// both MDV and netCDF header files.
//
#include "readScipuffDump.hh"

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
readScipuffDump::readScipuffDump(Params *P){
  _params = P;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
readScipuffDump::~readScipuffDump(){
  return;
}
//
// Main method
//
void readScipuffDump::readScipuffDumpFile( ){

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
  // Set up the vlevel header.
  //
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = _params->elevation;
  //
  // Then set up the field header with the metadata.
  //
  fhdr.nx = _params->grid_def.nx;
  fhdr.ny = _params->grid_def.ny;
  fhdr.nz = 1;
  //
  fhdr.grid_dx = _params->grid_def.dx;
  fhdr.grid_dy = _params->grid_def.dy;
  //
  fhdr.missing_data_value = -99.0;  fhdr.bad_data_value = -99.0;
  //
  fhdr.proj_type = Mdvx::PROJ_FLAT; // This is implied for these data.
  //
  fhdr.proj_origin_lat =  _params->grid_def.lat_orig;
  fhdr.proj_origin_lon =  _params->grid_def.lon_orig;
  //
  //
  fhdr.grid_minx = -1.0 * fhdr.nx * _params->grid_def.dx / 2.0;
  fhdr.grid_miny = -1.0 * fhdr.ny * _params->grid_def.dy / 2.0;
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
  sprintf(Mhdr.data_set_info,"%s","readScipuffDump");
  sprintf(Mhdr.data_set_name,"%s","readScipuffDump");
  sprintf(Mhdr.data_set_source,"%s", "readScipuffDump");
  //
  fhdr.forecast_delta = 0;

  if (sizeof(double) != 8){
    cerr << "Size mismatch." << endl;
    exit(-1);
  }

  double *data = (double *) malloc( _params->grid_def.nx *
				    _params->grid_def.ny *
				    sizeof(double));

  float *mdvData = (float *) malloc( fhdr.nx *
				     fhdr.ny *
				     sizeof(float));

  if ((data == NULL) || (mdvData == NULL)){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }
  //
  sprintf( fhdr.transform,"%s","none");
  //
  // Loop through the input files.
  //
  for (int iDataNum = 0; iDataNum < _params->nFiles; iDataNum++){

    char inFilename[MAX_PATH_LEN];
    sprintf(inFilename,"%s%03d", _params->baseFilename, iDataNum+1);

    cerr << "Reading " << inFilename << endl;

    FILE *fp = fopen(inFilename, "r");
    
    if (fp == NULL){
      cerr << "Failed to open " << inFilename << endl;
      exit(-1);
    }
    
    int numRead = fread(data, sizeof(double),  
			_params->grid_def.nx *
			_params->grid_def.ny,
			fp);

    fclose(fp);

    if (numRead != _params->grid_def.nx *
	_params->grid_def.ny){

      cerr << "Read fail on " << inFilename << endl;
      exit(-1);

    }

    //
    // Set the times in the master and field headers.
    //
    date_time_t T;
    T.year = _params->_time[0];  T.month = _params->_time[1];
    T.day = _params->_time[2];   T.hour = _params->_time[3];
    T.min = _params->_time[4];   T.sec = _params->_time[5];
    uconvert_to_utime( &T );
    T.unix_time +=  iDataNum * _params->timeStep;
    uconvert_from_utime( &T );

    if (_params->debug){
      cerr << "Read data from " << inFilename << " associating with time ";
      cerr << utimstr( T.unix_time ) << endl;
    }

    Mhdr.time_gen = T.unix_time;
    Mhdr.time_begin = T.unix_time;
    Mhdr.time_end = T.unix_time;
    Mhdr.time_expire = T.unix_time + _params->Expiry;
    Mhdr.time_centroid = T.unix_time;
    //
    fhdr.forecast_time = Mhdr.time_centroid;
    //
    // Fill up the MDV data.
    //
    for (int index = 0; index < fhdr.nx * fhdr.ny; index++){

      if (data[index] < -90.0){
	mdvData[index] = 0.0;
      } else {
	mdvData[index] = (float ) pow(10.0, data[index]);
      }
    }

    sprintf( fhdr.field_name_long,"%s", "conc" );
    sprintf( fhdr.field_name,"%s", "conc" );
    sprintf( fhdr.units,"%s", "Kg/m3");
    //
    MdvxField *field;
    //
    field = new MdvxField(fhdr, vhdr, mdvData);
    //
    if (field->convertRounded(Mdvx::ENCODING_FLOAT32,
			      Mdvx::COMPRESSION_ZLIB)){
      fprintf(stderr, "conversion of field failed - I cannot go on.\n");
      exit(-1);
    }


    //
    // Declare a DsMdvx object so we can load it up with fields.
    //
    DsMdvx outMdvx;
    //
    outMdvx.setMasterHeader( Mhdr ); 
    outMdvx.clearFields();
    
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
  }

  free(data); free(mdvData);
  return;

}
