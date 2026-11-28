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


#include "Process.hh"
#include "fusionFile.hh"

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh>
#include <unistd.h>

using namespace std;

//
// Constructor
//
Process::Process(){
  return;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *TDRP_params, time_t T){

  if (TDRP_params->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }

  //
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
  if (TDRP_params->vlevels_n != TDRP_params->output_grid.nz){
    cerr << "ERROR - Expected " << TDRP_params->output_grid.nz;
    cerr << " entries in parameter array vlevels, got ";
    cerr << TDRP_params->vlevels_n << endl;
    exit(-1);
  }
  //
  vhdr.type[0] = Mdvx::VERT_TYPE_Z;
  for (int iz=0; iz < TDRP_params->output_grid.nz; iz++){
    vhdr.level[iz] = TDRP_params->_vlevels[iz];
  }
  //
  // Then set up the field header with the metadata.
  //
  fhdr.nx = TDRP_params->output_grid.nx;
  fhdr.ny = TDRP_params->output_grid.ny;
  fhdr.nz = TDRP_params->output_grid.nz;
  //
  fhdr.grid_dx = TDRP_params->output_grid.dx;
  fhdr.grid_dy = TDRP_params->output_grid.dy;
  //
  switch ( TDRP_params->output_projection){

  case Params::PROJ_FLAT :
    fhdr.proj_type = Mdvx::PROJ_FLAT;
    break;
    
  case Params::PROJ_LATLON :
    fhdr.proj_type = Mdvx::PROJ_LATLON;
    break;

  case Params::PROJ_LAMBERT :
    fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
    break;

  default :
    cerr << "Unspecified projection!" << endl;
    exit(-1);
    break;

  }

  //
  fhdr.proj_origin_lat =  TDRP_params->output_grid.latOrig;
  fhdr.proj_origin_lon =  TDRP_params->output_grid.lonOrig;
  //
  fhdr.grid_minx = TDRP_params->output_grid.minx;
  fhdr.grid_miny = TDRP_params->output_grid.miny;
  //
  // For Lambert Conformal, the first two proj params are set to the
  // lambert conformal latitudes.
  //
  if ( TDRP_params->output_projection == Params::PROJ_LAMBERT){
    fhdr.proj_param[0] = TDRP_params->output_grid.lambertLat1;
    fhdr.proj_param[1] = TDRP_params->output_grid.lambertLat2;
  }
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
  fhdr.bad_data_value = fusionFile::badVal;
  fhdr.missing_data_value = fusionFile::badVal;
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
  sprintf(Mhdr.data_set_info,"%s","fusion");
  sprintf(Mhdr.data_set_name,"%s","fusion");
  sprintf(Mhdr.data_set_source,"%s", "fusion");
  //
  // Set the times in the master and field headers.
  //
  Mhdr.time_gen = T;
  Mhdr.time_begin = T;
  Mhdr.time_end = T;
  Mhdr.time_expire = T + TDRP_params->Expiry;
  Mhdr.time_centroid = T;
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
  // Quick check ....
  //
  if (TDRP_params->fusionFileList_n !=
      TDRP_params->units_n) {
    cerr << "I need units for each output field!" << endl;
    exit(-1);
  }

  //
  // Loop through the list of fusion files.
  //
  for (int ip=0; ip < TDRP_params->fusionFileList_n; ip++){
    if (TDRP_params->Debug){
      cerr << "Processing file " << TDRP_params->_fusionFileList[ip];
      cerr << " for data time " << utimstr(T) << endl;
    }

    fusionFile *F = new fusionFile(TDRP_params, 
				   T, TDRP_params->_fusionFileList[ip]);

    sprintf( fhdr.field_name_long,"%s", F->getFieldName().c_str() );
    sprintf( fhdr.field_name,"%s", F->getFieldName().c_str());
    sprintf( fhdr.units,"%s", TDRP_params->_units[ip] );
    sprintf( fhdr.transform,"%s","none");
    //
    //
    if (TDRP_params->Debug){
      float *data =  F->getData();
      float min=0.0; float max = 0.0; float total = 0.0; int num=0; int first = 1;
      for (int it=0; 
	   it < TDRP_params->output_grid.nx*TDRP_params->output_grid.ny*TDRP_params->output_grid.nz;
	   it++){
	if (data[it] != fusionFile::badVal){
	  total += data[it]; num++;
	  if (first){
	    first = 0;
	    min = data[it];
	    max = min;
	  } else {
	    if (max < data[it]) max = data[it];
	    if (min > data[it]) min = data[it];
	  }
	}
      }
      float mean = total / float(num);
      float pg = 100.0*float(num)/
	float(TDRP_params->output_grid.nx*TDRP_params->output_grid.ny*TDRP_params->output_grid.nz);
      cerr << " Mean : " << mean << " Min : " << min << " Max : " << max << " Percent good : " << pg << endl;
    }

    MdvxField *field;
    //
    field = new MdvxField(fhdr, vhdr, F->getData());
    //
    if (field->convertRounded(Mdvx::ENCODING_INT16,
			      Mdvx::COMPRESSION_ZLIB)){
      fprintf(stderr, "Conversion of field failed - I cannot go on.\n");
      exit(-1);
    }

    outMdvx.addField(field);


    delete F;

  }

  //
  // OK - write out the data at this time.
  //
  if (outMdvx.writeToDir(TDRP_params->OutUrl)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << TDRP_params->OutUrl << endl;
    cerr << outMdvx.getErrStr() << endl;
  }
  
  if ( TDRP_params->syncFSpostWrite ){
    sync(); sleep(1); sync();
  }


  if (TDRP_params->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  return;
}










