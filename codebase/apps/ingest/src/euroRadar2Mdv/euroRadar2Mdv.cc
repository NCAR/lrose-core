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
// euroRadar2Mdv.cc
//
// euroRadar2Mdv object
//
//
///////////////////////////////////////////////////////////////

#include "euroRadar2Mdv.hh"
#include "GermanProj.hh"

#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <toolsa/toolsa_macros.h> 

using namespace std;
const float euroRadar2Mdv::_MdvBadValue = -5.0;
  
// constructor.
euroRadar2Mdv::euroRadar2Mdv (Params *TDRP_params){
  // Make local copy of parameters.
  _params = TDRP_params;
  return;
}

// 
void euroRadar2Mdv::processFile(char *fileName){

  if (_params->debug){
    cerr << "Processing " << fileName << endl;
  }

  //
  // Open the file and read the ASCII header.
  //
  FILE *fp = fopen(fileName,"r");
  if (fp == NULL){
    cerr << "Failed to open " << fileName << endl;
    return;
  }

  //
  // The first two characters should be 'PI'
  //
  char firstChar, secondChar;

  fread( &firstChar, sizeof(char), 1, fp);
  fread( &secondChar, sizeof(char), 1, fp);

  if (
      (firstChar != 'P') ||
      (secondChar != 'I')
      ){
    cerr << fileName << " is not a euroRadar file!" << endl;
    fclose(fp);
    return;
  }
  //
  // Read the time.
  //
  date_time_t dataTime;
  dataTime.sec = 0;

  int numClassLimits, compositeID, fileSize;

  if (8 != fscanf(fp,"%2d%2d%2d%5d%2d%2dBY%dLV %d",
		  &dataTime.day, &dataTime.hour, &dataTime.min, &compositeID,
		  &dataTime.month, &dataTime.year, &fileSize, &numClassLimits)){
    fclose(fp);
    cerr << "Failed to read header from " << fileName << endl;
    return;
  }
      
  if (dataTime.year < 50){
    dataTime.year += 2000;
  } else {
    dataTime.year += 1900;
  }
  uconvert_to_utime( &dataTime );

  if (_params->debug){
    cerr << "Data time : " << utimstr( dataTime.unix_time ) << endl;
    cerr << "File size : " << fileSize << endl;
    cerr << numClassLimits << " class limits." << endl;
  }

  float *classLimits = (float *)malloc(numClassLimits*sizeof(float));

  if (classLimits == NULL){
    cerr << "Malloc failed!" << endl;
    exit(-1);
  }

  for (int i=0; i < numClassLimits; i++){
    fscanf(fp, "%f", &classLimits[i]);
    if (_params->debug){
      cerr << "Class limit " << i+1 << " : " << classLimits[i] << endl;
    }
  }

  int imageIndex;
  float maxHt;

  if (2 != fscanf(fp,"CS%dMX%f", &imageIndex, &maxHt)){
    cerr << "Failed to get index, height" << endl;
    free(classLimits);
    fclose(fp);
    return;
  }

  //
  // Look for the image dimensions, which follow the characters 'BG'
  // as in 'BG360360' which means a 360x360 image.
  //
  fread( &firstChar, sizeof(char), 1, fp);
  fread( &secondChar, sizeof(char), 1, fp);
  int go=1;

  int Nx, Ny;
  do{

    if (
	(firstChar == 'B') &&
	(secondChar == 'G')
	){
      fscanf(fp,"%3d%3d", &Nx, &Ny);
      go = 0;
    } else {
      firstChar = secondChar;
      fread( &secondChar, sizeof(char), 1, fp);
    }

  } while(go);

  if (_params->debug){
    cerr << "Image size : " << Nx << " by " << Ny << endl;
    cerr << "Remainder of header : " << endl;
  }

  //
  // Read until we encounter char(3) which means that the binary
  // data are starting.
  //
  char fileChar;
  do {
    fread( &fileChar, sizeof(char), 1, fp);
    if ((_params->debug) && (fileChar != char(3))) cerr << fileChar;
  } while (fileChar != char(3));
  //
  if (_params->debug) cerr << endl;
  //
  // Allocate space for the image data.
  //
  float *imgBuffer = (float *) malloc(Nx * Ny * sizeof(float));
  if (imgBuffer == NULL){
    cerr << "Malloc failed!" << endl;
    exit(-1);
  }
  //
  // Initially, set all data to bad/missing.
  //
  for (int k=0; k < Nx * Ny; k++){
    imgBuffer[k] = _MdvBadValue;
  }
  //
  // Start reading these data, which are stored according
  // to a compression scheme.
  //
  unsigned char byte;
  //
  for (int k=0; k < Ny; k++){
    //
    // Read the line number byte.
    //
    fread( &byte, sizeof(unsigned char), 1, fp);
    int lineNum = (int) byte;
    lineNum -= 16;
    //
    // The next byte is either 0x0a (which means end of line)
    // or an offset. Let's see.
    //
    fread( &byte, sizeof(unsigned char), 1, fp);
    if (byte == 0x0a){
      continue; // Forget it.
    } else {

      int currentOffset = (int) byte;
      currentOffset -= 16;
      //
      // If the byte was 0xFF then the offset was split across two
      // bytes - read the next byte.
      //
      if (byte == 0xFF){
	fread( &byte, sizeof(unsigned char), 1, fp);
	currentOffset += (int) byte - 15;
      }

      do {
	//
	// Read a byte. If it is 0x0a then it
	// is the end of this line. Otherwise we
	// split it into high and low parts.
	// These are the run length and the class index
	// of the run.
	//
	fread( &byte, sizeof(unsigned char), 1, fp);
	
	if (byte != 0x0a){
	  
	  int runLen = (byte & 0xf0) / 16;
	  int classIndex = (byte & 0x0f);
	  //
	  // classIndex of 9 indicates the edge of the image
	  // classIndex of 0 indicates missing data
	  // We skip them both.
	  //
	  if ((classIndex != 0) && (classIndex != 9)){

	    for (int l = currentOffset; l < currentOffset + runLen; l++){
	      //
	      // Assign the specified class limit.
	      //
	      int ind = k * Nx + l;
	      imgBuffer[ind] = classLimits[classIndex - 1];
	    }
	  }
	  //
	  // Increment the current offset.
	  //
	  currentOffset += runLen;
	  //
	  // Do a check on the offset.
	  //
	  if (currentOffset > Nx){
	    cerr << "Line length exceeded error!" << endl;
	    exit(-1);
	  }
	}	
      } while (byte != 0x0a);
    } // End of IF we have to do anything for this line.
  }

  fclose(fp);
  free(classLimits);

  /* --- The following code used to load the original grid into MatLab for debugging.
  fp = fopen("a.dat","w");

  for (int iy = 0; iy < Ny; iy++){
    for (int ix = 0; ix < Nx; ix++){
      fprintf(fp,"%f\t", imgBuffer[iy * Nx + ix]);
    }
    fprintf(fp,"\n");
  }
  fclose(fp);
  ------------------------------------------------------------------*/

  //
  // Remap from the German Polar Stereographic grid to our lat/lon grid.
  //
  GermanProj G(_params->germanGridSpec.TangentLat,
	       _params->germanGridSpec.TangentLon,
	       _params->germanGridSpec.OriginLat,
	       _params->germanGridSpec.OriginLon,
	       _params->germanGridSpec.OriginIX,
	       _params->germanGridSpec.OriginIY,
	       _params->germanGridSpec.Dx,
	       _params->germanGridSpec.Dy);

  float *ourData = (float *)malloc(_params->ourGridSpec.Nx * _params->ourGridSpec.Ny * sizeof(float));
  if (ourData == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }


  for (int ix = 0; ix < _params->ourGridSpec.Nx; ix++){
    for (int iy = 0; iy <  _params->ourGridSpec.Ny; iy++){
      int index = iy *  _params->ourGridSpec.Nx + ix;
      double lat = _params->ourGridSpec.minLat + iy * _params->ourGridSpec.Dy;
      double lon = _params->ourGridSpec.minLon + ix * _params->ourGridSpec.Dx;

      int gix, giy;
      G.getGermanXYIndex(lat, lon, &gix, &giy);
      if (
	  (gix < 0) ||
	  (giy < 0) ||
	  (gix > Nx-1) ||
	  (giy > Ny-1)
	  ){
	ourData[index] = _MdvBadValue;
      } else {
	int gindex = giy * Nx + gix;
	ourData[index] = imgBuffer[gindex];
      }
    }
  }
  //
  // Write it out as MDV.
  //
  Mdvx::field_header_t fhdr;
  Mdvx::vlevel_header_t vhdr;
  Mdvx::master_header_t Mhdr;
  //
  memset(&fhdr, 0, sizeof(fhdr));
  memset(&vhdr, 0, sizeof(vhdr));
  memset(&Mhdr, 0, sizeof(Mhdr));
  //
  // Set up the vlevel header.
  //
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0.0;

  //
  // Then the field header.
  //
  fhdr.nx = _params->ourGridSpec.Nx;
  fhdr.ny = _params->ourGridSpec.Ny;
  fhdr.nz = 1;
  //
  fhdr.grid_dx = _params->ourGridSpec.Dx;
  fhdr.grid_dy = _params->ourGridSpec.Dx;
  //
  //
  fhdr.proj_type = Mdvx::PROJ_LATLON;
  //
  fhdr.proj_origin_lat =  _params->ourGridSpec.minLat;
  fhdr.proj_origin_lon =  _params->ourGridSpec.minLon;
  //
  //
  // Set the grid minimums. Note that because of the flipped grid
  // in north-south, we have to flip the minimum Y.
  //
  fhdr.grid_minx =  _params->ourGridSpec.minLon;
  fhdr.grid_miny =  _params->ourGridSpec.minLat;
  //
  //
  //
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * sizeof(fl32);
  //
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  //
  //
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = 0;
  //
  //
  Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  Mhdr.num_data_times = 1;
  Mhdr.data_ordering = Mdvx::ORDER_XYZ;
  Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  Mhdr.vlevel_included = 1;
  Mhdr.max_nx = fhdr.nx;
  Mhdr.max_ny = fhdr.ny;
  Mhdr.max_nz = 1;
  //
  Mhdr.sensor_lat =  _params->ourGridSpec.minLat;
  Mhdr.sensor_lon =  _params->ourGridSpec.minLat;
  //
  sprintf(Mhdr.data_set_info,"%s","European Radar Composite");
  sprintf(Mhdr.data_set_name,"%s","European Radar Composite");
  sprintf(Mhdr.data_set_source,"%s", "Europe");
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
  //
  DsMdvx outMdvx;
  //
  outMdvx.setMasterHeader( Mhdr );
  outMdvx.clearFields();
  //
  fhdr.bad_data_value = _MdvBadValue;
  fhdr.missing_data_value = _MdvBadValue;
  //
  sprintf( fhdr.field_name_long, "%s", "DBZ");
  sprintf( fhdr.field_name,"%s", "DBZ" );
  sprintf( fhdr.units,"%s", "DBZ");
  sprintf( fhdr.transform,"%s","none");
  //
  //
  MdvxField *field;
  //
  field = new MdvxField(fhdr, vhdr, ourData );
  //
  if (field->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }

  outMdvx.addField(field);

  if (outMdvx.writeToDir(_params->output_url)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << _params->output_url << endl;
    cerr << outMdvx.getErrStr() << endl;
  }

  free(imgBuffer);
  free(ourData);

  return;
}
  
// destructor.
euroRadar2Mdv::~euroRadar2Mdv(){
  return;
}

