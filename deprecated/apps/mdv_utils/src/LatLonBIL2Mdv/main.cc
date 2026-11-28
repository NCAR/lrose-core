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
// This is a small program that reads latlon .bil
// files and produces MDV files. The .bil file is assumed
// to be accompanied by a .hdr header file and a .blw
// world file - these contain the metadata. The format is
// discussed more at the web page :
//
// http://www.softwright.com/faq/support/toposcript_bil_file_format.html
//
// This code is not well developed at all, I just used it
// to read some Pentagon terrain data into MDV.
//
// Niles Oien April 2004
//
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/DsMdvx.hh>


void usage(){
  fprintf(stderr,"USAGE : LatLonBIL2Mdv <basename>\n\n");
  fprintf(stderr,"ie. for files tile25_dem_a1.blw, tile25_dem_a1.hdr\n");
  fprintf(stderr,"and tile25_dem_a1.bil, the command is :\n\n");
  fprintf(stderr,"LatLonBIL2Mdv tile25_dem_a1\n\n");
  fprintf(stderr,"Output is written to the current directory in a\n");
  fprintf(stderr,"subdirectory named after the basename.\n\n");
  fprintf(stderr,"Niles Oien April 2004\n");
  return;
}

int main(int argc, char *argv[]){

  if (argc < 2){
    usage();
    exit(-1);
  }


  char filename[1024];
  //
  // Get Nx, Ny from the header file.
  //
  sprintf(filename,"%s.hdr", argv[1]);

  FILE *fp = fopen(filename,"r");

  if (fp == NULL){
    fprintf(stderr,"Unable to open %s\n", filename);
    usage();
    exit(-1);
  }

  int Nx = 0;
  int Ny = 0;

  char Line[1024];
  while(NULL!=fgets(Line,1024,fp)){
    if (strstr(Line, "NROWS")!=NULL){
      sscanf(Line + strlen("NROWS"), "%d", &Ny);
    }
    if (strstr(Line, "NCOLS")!=NULL){
      sscanf(Line + strlen("NCOLS"), "%d", &Nx);
    }
  }
  

  int elementSize = 4;

  fclose(fp);

  fprintf(stderr,"SIZE : %d by %d\n", Nx, Ny);
  //
  // Get origin, dx, dy from .blw file
  //
  sprintf(filename,"%s.blw", argv[1]);

  fp = fopen(filename,"r");

  if (fp == NULL){
    fprintf(stderr,"Unable to open %s\n", filename);
    exit(-1);
  }

  double dx = 0.0;
  double dy = 0.0;

  double ulLat = 0.0;
  double ulLon = 0.0;

  int i=1;
  while(NULL!=fgets(Line,1024,fp)){

    double tempDoub;
    sscanf(Line, "%lf", &tempDoub);
    if (i == 1) dx = fabs(tempDoub);
    if (i == 4) dy = fabs(tempDoub);
    if (i == 5) ulLon = tempDoub;
    if (i == 6) ulLat = tempDoub;
    i++;
  }

  fclose(fp);

  double llLat = ulLat - dy*Ny;

  fprintf(stderr,"llLat=%g llLon=%g dx=%g dy=%g\n", llLat, ulLon, dx, dy);

  //
  // Get actual data from .bil file.
  //
  sprintf(filename,"%s.bil", argv[1]);

  fp = fopen(filename,"r");

  if (fp == NULL){
    fprintf(stderr,"Unable to open %s\n", filename);
    exit(-1);
  }

  float *data = (float *)malloc(elementSize*Nx*Ny);

  fread(data, elementSize, Nx*Ny, fp);
  fclose(fp);

  float min = data[0];
  float max = data[0];

  double total = 0.0;
  for (int i=0; i < Nx*Ny; i++){
    total += data[i];
    if (min > data[i]) min = data[i];
    if (max < data[i]) max = data[i];
  }

  //
  // Flip in Y
  //
  
  float *temp = (float *)malloc(elementSize*Nx*Ny);

  for(int ix=0; ix < Nx; ix++){
    for(int iy=0; iy < Ny; iy++){
      int revIy = Ny -1 - iy;
      int index = ix + Nx*iy;
      int revIndex = ix + Nx*revIy;
      temp[revIndex] = data[index];
    }
  }

  for (int i=0; i < Nx*Ny; i++){
    data[i]=temp[i];
  }

  free(temp);

  fprintf(stderr,"%g -> %g (%g)\n", min, max, total/(Nx*Ny));

  //
  // Blast out an MDV file.
  //

  Mdvx::field_header_t fhdr;
  Mdvx::vlevel_header_t vhdr;
  Mdvx::master_header_t mhdr;

  memset(&mhdr, 0, sizeof(mhdr));
  memset(&vhdr, 0, sizeof(vhdr));
  memset(&fhdr, 0, sizeof(fhdr));

  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0.0;


  fhdr.nx = Nx; fhdr.ny = Ny; fhdr.nz = 1;
  fhdr.grid_dx = dx; fhdr.grid_dy = dy; fhdr.grid_dz = 0.0;
  
  fhdr.proj_type = Mdvx::PROJ_LATLON;

  fhdr.grid_minx = ulLon; fhdr.grid_miny = llLat;

  fhdr.proj_origin_lat = fhdr.grid_miny;
  fhdr.proj_origin_lon = fhdr.grid_minx;

  fhdr.bad_data_value = min - 10.0;
  fhdr.missing_data_value = min - 10.0;

  sprintf( fhdr.field_name_long,"%s", "Elevation");
  sprintf( fhdr.field_name,"%s", "Elevation");
  sprintf( fhdr.units,"%s", "m");
  sprintf( fhdr.transform,"%s","none");

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;     
  //
  //
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = 1;


  mhdr.data_dimension = 2;
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.num_data_times = 1;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;  
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;

  mhdr.n_fields = 1;
  mhdr.max_nx = Nx;
  mhdr.max_ny = Ny;
  mhdr.max_nz = 1; 
  //
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  //
  sprintf(mhdr.data_set_info,"%s",argv[1]);
  sprintf(mhdr.data_set_name,"%s","Terrain");
  sprintf(mhdr.data_set_source,"%s", argv[1]);


  time_t now = time(NULL);
  mhdr.time_gen = now;
  mhdr.time_begin = now;
  mhdr.time_end = now;
  mhdr.time_expire = now;
  mhdr.time_centroid = now;

  fhdr.forecast_time = mhdr.time_centroid;


  DsMdvx outMdvx;

  outMdvx.setMasterHeader( mhdr ); 
  outMdvx.clearFields();

  MdvxField *field = new MdvxField(fhdr, vhdr, data);    
  if (field->convertRounded(Mdvx::ENCODING_INT16,
			    Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "Convert failed - I cannot go on.\n");
    exit(-1);
  }
  //
  // add field to mdvx object
  //
  outMdvx.addField(field); 
  
  char outUrl[1024];
  sprintf(outUrl,"./%s",argv[1]);

  if (outMdvx.writeToDir( outUrl )) {
    fprintf(stderr, "ERROR - Output::write %s\n", outUrl);
    exit(-1);
  }
  //
  free(data);

}
