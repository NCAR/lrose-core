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
// Simple program to reformat ASCII terrain data into MDV.
// Niles Oien June 2005.
//
#include <cstdio>
#include <iostream>
#include <toolsa/pjg_flat.h>
#include "Params.hh"
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>

using namespace std;


int main( int argc, char *argv[] )
{

  
  Params P;

  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }

  //
  // Open the input file and read the ASCII header.
  //
  FILE *fp = fopen(P.inFileName, "r");
  if (fp == NULL){
    cerr << "Failed to open " << P.inFileName << endl;
    exit(-1);
  }

  int nx;
  if (1 != fscanf(fp,"ncols %d\n", &nx)) {
    cerr << "Read failed for nx\n" << endl;
    exit(-1);
  }

  int ny;
  if (1 != fscanf(fp,"nrows %d\n", &ny)) {
    cerr << "Read failed for ny\n" << endl;
    exit(-1);
  }

  //
  // Might as well bail now if we don't have memory.
  //
  float *data = (float *)malloc(sizeof(float)*nx*ny);
  float *temp = (float *)malloc(sizeof(float)*nx*ny);
  if ((data == NULL) || (temp == NULL)){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  double ll_lon;
  if (1 != fscanf(fp,"xllcorner %lf\n", &ll_lon)) {
    cerr << "Read failed for ll_lon\n" << endl;
    exit(-1);
  }

  double ll_lat;
  if (1 != fscanf(fp,"yllcorner %lf\n", &ll_lat)) {
    cerr << "Read failed for ll_lat\n" << endl;
    exit(-1);
  }

  double dxy;
  if (1 != fscanf(fp,"cellsize %lf\n", &dxy)) {
    cerr << "Read failed for dxy\n" << endl;
    exit(-1);
  }

  float missing;
  if (1 != fscanf(fp,"NODATA_value %f\n", &missing)) {
    cerr << "Read failed for missing\n" << endl;
    exit(-1);
  }
  //
  // Read the data.
  //
  int i=0;
  for (int iy=0; iy < ny; iy++){
    for (int ix=0; ix < nx-1; ix++){
      if (1 != fscanf(fp,"%f ", &data[i])) {
	cerr << "Line read failed on data at point " << i << endl;
	cerr << "ix= " << ix << "iy=" << iy << endl; 
	exit(-1);
      }
      i++;
    }
    if (1 != fscanf(fp,"%f\n", &data[i])) {
      cerr << "Return read failed on data at point " << i << endl;
      cerr << "iy=" << iy << endl; 
     exit(-1);
    }
    i++;
  }

  fclose(fp);

  //
  // Flip the data north-south.
  //

  for (int iy=0; iy < ny; iy++){
    for (int ix=0; ix < nx; ix++){
      temp[iy * nx + ix] = data[(ny-1-iy) * nx + ix];
    }
  }

  for (int i=0; i < nx*ny; i++){
    data[i]=temp[i];
  }

  free(temp);

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
  fhdr.grid_dx = dxy;
  fhdr.grid_dy = dxy;
  //
  fhdr.proj_type = Mdvx::PROJ_LATLON; // This is implied for these data.
  //
  fhdr.proj_origin_lat =  ll_lat;
  fhdr.proj_origin_lon =  ll_lon;
  //
  fhdr.grid_minx = ll_lon;
  fhdr.grid_miny = ll_lat;
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
  sprintf(Mhdr.data_set_info,"%s","asciiTerrain2Mdv");
  sprintf(Mhdr.data_set_name,"%s","asciiTerrain2Mdv");
  sprintf(Mhdr.data_set_source,"%s", "asciiTerrain2Mdv");
  //
  // Set the times in the master and field headers.
  //
  Mhdr.time_gen = time(NULL);
  Mhdr.time_begin = time(NULL);
  Mhdr.time_end = time(NULL);
  Mhdr.time_expire = time(NULL);
  Mhdr.time_centroid = time(NULL);
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

  // Complete filling out the field header with this information.
  //
  fhdr.bad_data_value = missing;   fhdr.missing_data_value = missing;
  //
  sprintf( fhdr.field_name_long,"%s", "Elevation" );
  sprintf( fhdr.field_name,"%s", "Elevation");
  sprintf( fhdr.units,"%s", "m");
  sprintf( fhdr.transform,"%s","none");
  
  MdvxField *field;
  //
  field = new MdvxField(fhdr, vhdr, data);
  //
  if (field->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }
  
  outMdvx.addField(field);
  
  //
  // Finally, now that the DsMdvx object is all loaded up, use it
  // to write out the data.
  //
  if (outMdvx.writeToPath(P.outFileName)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << P.outFileName << endl;
    cerr << outMdvx.getErrStr() << endl;
  }



  free(data);

  return 0;

}











