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

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <toolsa/pjg_flat.h>

#include <toolsa/umisc.h>


#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>   


#include "MdvFromCircles.hh"
using namespace std;
//
// constructor - does everything.
//
MdvFromCircles::MdvFromCircles(char *Filename, Params *P){

  if (P->debug){
    cerr << "Processing file " << Filename << endl;
  }
  //
  date_time_t T;
  //
  // See if we can get the time from the filename.
  //
  int gotFilenameTime = 0;
  if (strlen(Filename) < strlen("YYYYMMDD_hhmmss")){
    cerr << "Filename is not in format YYYYMMDD_hhmmss" << endl;
  } else {

    //
    // Back up to either the last '/' or the start of the filename.
    //
    char *parse = Filename + strlen(Filename);
    do {
      parse--;
    } while ((parse[0] != '/') && (parse != Filename));

    if (parse[0] == '/') parse++;

    if (6!= sscanf(parse,
		   "%4d%2d%2d_%2d%2d%2d",
		   &T.year, &T.month, &T.day,
		   &T.hour, &T.min,   &T.sec)){
      cerr << "Cannot parse time information from ";
      cerr << "filename " << parse << endl;
    } else {
      gotFilenameTime = 1;
      uconvert_to_utime( &T );
    }
  }

  if (!(gotFilenameTime)){
    struct stat buf;
    stat(Filename,&buf);
    T.unix_time = buf.st_ctime;
    uconvert_from_utime( &T );
  }
  if (P->debug){
    if (gotFilenameTime){
      cerr << "Timestamping with filename time : ";
    } else {
      cerr << "Timestamping with file date time : ";
    }
    cerr << utimstr(T.unix_time) << endl;
  }
  //
  // Open the file.
  //
  FILE *fp = fopen(Filename,"rt");
  if (fp == NULL){
    cerr << "Failed to open " << Filename << endl;
    return;
  }
  //
  // Allocate enough space for the grid.
  // Set to 0 with calloc initially.
  //
  fl32 *data = (fl32 *)calloc(P->Nx*P->Ny,sizeof(float));
  if (data == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }
  //
  // Take two hits at the file - first process
  // all the circles to be overwritten, then all
  // the ones to be added.
  //

  float lat,lon,val,radius,halfLife;
  int overwrite;
  const int LineLen = 1024;
  char Line[LineLen];

  int numOverwritePoints = 0;
  int numAddPoints = 0;

  while(NULL!=fgets(Line,LineLen,fp)){
    if (6==sscanf(Line,"%f %f %f %f %f %d",
		  &lat, &lon, &val, &radius, &halfLife, &overwrite )){
      if (overwrite){
	if (P->debug){
	  cerr << "Overwrite point : " << Line;
	}
	numOverwritePoints++;
	AddCircleToGrid(data,lat,lon,val,radius,halfLife,overwrite,P);
      }
    }
  }
  //
  // Second hit at the file. Process circles to be added in.
  //
  rewind(fp);
  while(NULL!=fgets(Line,LineLen,fp)){
    if (6==sscanf(Line,"%f %f %f %f %f %d",
		  &lat, &lon, &val, &radius, &halfLife, &overwrite )){
      if (!(overwrite)){
	if (P->debug){
	  cerr << "Add point : " << Line;
	}
	numAddPoints++;
	AddCircleToGrid(data,lat,lon,val,radius,halfLife,overwrite,P);
      }
    }
  }

  if (P->debug){
    cerr << numAddPoints + numOverwritePoints << " points in " << Filename << endl;
    cerr << numOverwritePoints << " in overwrite mode." << endl;
    cerr << numAddPoints << " in add mode." << endl;
  }

  fclose(fp);
  //
  // Write it out as MDV file.
  //

  DsMdvx Out;   

  Mdvx::master_header_t OutMhdr;

  time_t Now;
  Now=time(NULL);;

  OutMhdr.time_gen = Now;
  OutMhdr.time_begin = T.unix_time;
  OutMhdr.time_end = T.unix_time;
  OutMhdr.time_centroid = T.unix_time;


  OutMhdr.time_expire = T.unix_time + P->expiry;

  OutMhdr.data_collection_type = Mdvx::DATA_MEASURED;
  
  OutMhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  OutMhdr.vlevel_included = 1;
               

  OutMhdr.n_fields = 1;
  OutMhdr.max_nx = P->Nx;
  OutMhdr.max_ny = P->Ny;
  OutMhdr.max_nz = 1; 
      
  sprintf(OutMhdr.data_set_info,"%s","FromASCII");
  sprintf(OutMhdr.data_set_name,"%s","FromASCII");
  sprintf(OutMhdr.data_set_source,"%s","MdvFromCircles");


  Out.setMasterHeader(OutMhdr);

  Out.clearFields();     

  Mdvx::field_header_t Fhdr;
  MEM_zero(Fhdr);

  Mdvx::vlevel_header_t Vhdr;
  MEM_zero(Vhdr);               

  Fhdr.struct_id = Mdvx::FIELD_HEAD_MAGIC_COOKIE_64;

  Fhdr.user_time1 =     T.unix_time;
  Fhdr.forecast_delta = 0; 
  Fhdr.user_time2 =     T.unix_time;
  Fhdr.user_time3 =     T.unix_time;
  Fhdr.forecast_time =  T.unix_time;
  Fhdr.user_time4 =     T.unix_time; 
  Fhdr.nx =  P->Nx;
  Fhdr.ny =  P->Ny;
  Fhdr.nz =  1;

  Fhdr.proj_type = Mdvx::PROJ_LATLON;


  // Then reals
  Fhdr.proj_origin_lat =  P->lowerLeftLat;
  Fhdr.proj_origin_lon =  P->lowerLeftLon;
  Fhdr.proj_param[0] = 0.0;
  Fhdr.proj_param[1] = 0.0;
  Fhdr.proj_param[2] = 0.0;
  Fhdr.proj_param[3] = 0.0;
  Fhdr.proj_param[4] = 0.0;
  Fhdr.proj_param[5] = 0.0;
  Fhdr.proj_param[6] = 0.0;
  Fhdr.proj_param[7] = 0.0;
  Fhdr.vert_reference = 0.0;

  Fhdr.grid_dx =  P->Dx;
  Fhdr.grid_dy =  P->Dy;
  Fhdr.grid_dz =  0.0;

  Fhdr.grid_minx =  P->lowerLeftLon;
  Fhdr.grid_miny =  P->lowerLeftLat;
  Fhdr.grid_minz =  0;
  //
  // Find the minimum and use it to set the bad value.
  //
  fl32 min=data[0];
  for (int i=0; i < P->Nx*P->Ny; i++){
    if (min > data[i]) min = data[i];
  }

  Fhdr.bad_data_value = min-100.0;
  Fhdr.missing_data_value = min-100.0;
  Fhdr.proj_rotation = 0.0;

  //
  // Replace 0.0 with missing if requested.
  //
  if (P->replaceZeroWithMissing){
    for (int i=0; i < P->Nx*P->Ny; i++){
      if (0 == data[i]) data[i] = Fhdr.missing_data_value;
    }
  }

  // Then characters
  sprintf(Fhdr.field_name_long,"%s",P->FieldName);
  sprintf(Fhdr.field_name,"%s",P->FieldName);
  sprintf(Fhdr.units,"%s",P->FieldUnits);
  sprintf(Fhdr.transform,"%s"," ");

  Fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  Fhdr.data_element_nbytes = sizeof(fl32);
  Fhdr.volume_size = Fhdr.nx * Fhdr.ny * Fhdr.nz * sizeof(fl32);
  Fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  Fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  Fhdr.scaling_type = Mdvx::SCALING_NONE;     
  
  for (int iz = 0; iz < Fhdr.nz; iz++) {
    Vhdr.type[iz] = Mdvx::VERT_TYPE_SURFACE;
    Vhdr.level[iz] = Fhdr.grid_minz + iz * Fhdr.grid_dz;
  }
  
  // create field

  MdvxField *fld = new MdvxField(Fhdr, Vhdr, (void *)data);
    
  fld->setFieldName(P->FieldName);
  fld->setUnits(P->FieldUnits);
  fld->setFieldNameLong(P->FieldName);
  
  if (fld->convertRounded(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  
    
  Out.addField(fld);
  
  Out.writeToDir(P->OutUrl);

  free(data);


}
//
//
// destructor - does nothing.
//
MdvFromCircles::~MdvFromCircles (){
}
//
// Add circles into the grid.
//
void MdvFromCircles::AddCircleToGrid(fl32 *data,
				     fl32 lat,
				     fl32 lon,
				     fl32 val,
				     fl32 radius,
				     fl32 halfLife,
				     int overwrite,
				     Params *P){

  int minX, minY, maxX, maxY;


  double minLat, minLon;
  double maxLat, maxLon;
  double dummy;
  //
  // Get the lat/lon of the bounding points.
  //
  PJGLatLonPlusRTheta(lat, lon, radius, 0,   &maxLat, &dummy);
  PJGLatLonPlusRTheta(lat, lon, radius, 180, &minLat, &dummy);
  PJGLatLonPlusRTheta(lat, lon, radius, 270, &dummy, &minLon);
  PJGLatLonPlusRTheta(lat, lon, radius, 90,  &dummy, &maxLon);
  //
  // Add a bit to be sure we encomapss the area.
  //
  const double epsilon = 0.5;
  maxLat = maxLat + epsilon;
  minLat = minLat - epsilon;
  maxLon = maxLon + epsilon;
  minLon = minLon - epsilon;
  //
  // Get the indicies thereof.
  //
  minX = (int)rint((minLon - P->lowerLeftLon) / P->Dx);
  minY = (int)rint((minLat - P->lowerLeftLat) / P->Dy);
  maxX = (int)rint((maxLon - P->lowerLeftLon) / P->Dx);
  maxY = (int)rint((maxLat - P->lowerLeftLat) / P->Dy);


  for (int ix = minX; ix <= maxX; ix++){
    for (int iy = minY; iy <= maxY; iy++){
      //
      // See if we are even in the grid.
      //
      if (
	  (ix > -1) &&
	  (iy > -1) &&
	  (ix < P->Nx) &&
	  (iy < P->Ny)
	  ){
	//
	// Work out the lat, lon
	//
	double plat = P->lowerLeftLat + iy * P->Dy;
	double plon = P->lowerLeftLon + ix * P->Dx;
	//
	// Get the distance, proceed if it is less than or
	// equal to the radius.
	//
	double dist, theta;
	PJGLatLon2RTheta(lat, lon, plat, plon, &dist, &theta);
	//
	if (dist <= radius){
	  //
	  // Work out the value at that point.
	  //
	  double pval = val*exp(log(0.5)*dist/halfLife);
	  //
	  // And add it to the grid.
	  //
	  if (overwrite){
	    data[ix + iy*P->Nx] = pval;
	  } else {
	    data[ix + iy*P->Nx] = data[ix + iy*P->Nx] + pval;
	  }
	}
      }
    }
  }
}
  

