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
// This is the module of the windtracerPrf2mdv application that does most
// of the work, reading the netCDF and writing the MDV files. Include
// both MDV and netCDF header files.
//
#include "windtracerPrf2mdv.hh"

using namespace std;

#include <iostream>
#include <stdlib.h>
#include <cstdio>
#include <vector>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/pjg_flat.h>
#include <physics/physics.h>
//
// Constructor - makes a copy of a pointer to the TDRP parameters.
//
windtracerPrf2mdv::windtracerPrf2mdv(Params *P){
  _params = P;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
windtracerPrf2mdv::~windtracerPrf2mdv(){
  return;
}
//
// Main method - the netCDF -> MDV conversion.
//
void windtracerPrf2mdv::windtracerPrf2mdvFile( char *FilePath ){
  //

  FILE *ifp = fopen(FilePath, "r");
  if (ifp == NULL) return;



  vector<double> heights;
  vector<double> speeds;
  vector<double> dirs;

  date_time_t dataTime;
  double lat;
  double lon;

  bool first = true;
  double maxHt, minHt;

  maxHt = 0.0; minHt = maxHt;

  char Line[1024];
  while (NULL != fgets(Line, 1024, ifp)){

    //
    // Decode the date, if that's what we have
    //
    if (NULL != strstr(Line, "DATE")){
      if (6 != sscanf(Line+strlen("# DATE:"), "%d-%d-%d %d:%d:%d",
		      &dataTime.year, &dataTime.month, &dataTime.day,
		      &dataTime.hour, &dataTime.min, &dataTime.sec)){
	cerr << "Could not parse date from " << Line;
	fclose(ifp);
	return;
      }
      uconvert_to_utime(&dataTime);
    }

    //
    // Position, if that's what we have
    //
    if (0==strncmp(Line, "ID:", 3)){
      if (2 != sscanf(Line+strlen("ID: CTI01    080820    15.17  "),
		      "%lf %lf", &lat, &lon)){
	cerr << "Could not parse lat, lon from " << Line;
	fclose(ifp);
	return;
      }
    }

    //
    // An entry, if that's what we have.
    //
    double height, speed, dir;

    if (3==sscanf(Line, "%lf %lf %lf", &height, &speed, &dir)){

      if (
	  (fabs(speed) < 400) &&
	  (fabs(dir) < 361)
	  ){

      height = (_params->heightRound * rint(height / _params->heightRound))/1000.0; // Round, convert to Km

      heights.push_back( height );
      speeds.push_back( speed );
      dirs.push_back( dir );

      if (first){
	first = false;
	minHt = height; maxHt = minHt;
      } else {
	if (height > maxHt) maxHt = height;
	if (height < minHt) minHt = height;
      }

    }
  }
  }

  fclose(ifp);

  int nz = 1+(int)rint((maxHt-minHt)/(_params->heightRound/1000.0));


  if (_params->debug){
    cerr << "At " << utimstr(dataTime.unix_time) << " " << lat << ", " << lon;
    cerr << " " << heights.size() << " entries found." << endl;

    for (unsigned i=0; i < heights.size(); i++){
      cerr << i << " " << heights[i] << " " << speeds[i] << " " << dirs[i] << endl;
    } 

    cerr << nz << " levels in MDV file" << endl;

  }

  if (heights.size() == 0) return;

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
  vhdr.type[0] = Mdvx::VERT_TYPE_Z;
  for (int j=0; j < nz; j++){
    vhdr.level[j] = minHt + double(j) * _params->heightRound/1000.0;
  }
  //
  // Then set up the field header with the metadata.
  //
  fhdr.nx = _params->grid.nx;
  fhdr.ny = _params->grid.ny;
  fhdr.nz = nz;
  //
  fhdr.grid_dx = _params->grid.dx;
  fhdr.grid_dy = _params->grid.dy;
  fhdr.grid_dz = minHt;
  //
  fhdr.proj_type = Mdvx::PROJ_FLAT;
  fhdr.proj_origin_lat = lat;
  fhdr.proj_origin_lon = lon;
  //
  //
  fhdr.grid_minx = _params->grid.dx/2.0 - (_params->grid.nx*_params->grid.dx)/2.0;
  fhdr.grid_miny = _params->grid.dy/2.0 - (_params->grid.ny*_params->grid.dy)/2.0;
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
  sprintf(Mhdr.data_set_info,"%s","windtracerPrf2mdv");
  sprintf(Mhdr.data_set_name,"%s","windtracerPrf2mdv");
  sprintf(Mhdr.data_set_source,"%s", "windtracerPrf2mdv");
  //
  // Set the times in the master and field headers.
  //

  Mhdr.time_gen = dataTime.unix_time;
  Mhdr.time_begin = dataTime.unix_time;
  Mhdr.time_end = dataTime.unix_time;
  Mhdr.time_expire = dataTime.unix_time;
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
  // Allocate enough space for the actual grid data ...
  //
  float *buffer = (float *) malloc(sizeof(float)*fhdr.nx*fhdr.ny*fhdr.nz);
  if (buffer == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  double fillVal = -999.0;
  //
  fhdr.bad_data_value = fillVal;   fhdr.missing_data_value = fillVal;
  //

  /////////////////// Speed field ///////////////////////////////////

  sprintf( fhdr.field_name_long,"%s", "speed");
  sprintf( fhdr.field_name,"%s", "speed");
  sprintf( fhdr.units,"%s", "m/s");
  sprintf( fhdr.transform,"%s","none");

  //
  // Set all to missing.
  //
  for (int k=0; k < fhdr.nx*fhdr.ny*fhdr.nz; k++){
    buffer[k] = fillVal;
  }

  //
  // Loop through entries, filling planes.
  //
  for (unsigned ie=0; ie < speeds.size(); ie++){

    unsigned iz = (int)rint((heights[ie] - minHt) / (_params->heightRound/1000.0));

    for (int iy=0; iy < _params->grid.ny; iy++){
      for (int ix=0; ix < _params->grid.nx; ix++){
	buffer[iz*fhdr.ny*fhdr.nx + iy*fhdr.nx + ix] = speeds[ie];
      }
    }

  }


  //
  //
  MdvxField *speedField = new MdvxField(fhdr, vhdr, buffer);
  //
  if (speedField->convertRounded(Mdvx::ENCODING_INT16,
			    Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }

  outMdvx.addField(speedField);


  /////////////////// dir field ///////////////////////////////////

  sprintf( fhdr.field_name_long,"%s", "dir");
  sprintf( fhdr.field_name,"%s", "dir");
  sprintf( fhdr.units,"%s", "deg");
  sprintf( fhdr.transform,"%s","none");

  //
  // Set all to missing.
  //
  for (int k=0; k < fhdr.nx*fhdr.ny*fhdr.nz; k++){
    buffer[k] = fillVal;
  }

  //
  // Loop through entries, filling planes.
  //
  for (unsigned ie=0; ie < dirs.size(); ie++){

    unsigned iz = (int)rint((heights[ie] - minHt) / (_params->heightRound/1000.0));

    for (int iy=0; iy < _params->grid.ny; iy++){
      for (int ix=0; ix < _params->grid.nx; ix++){
	buffer[iz*fhdr.ny*fhdr.nx + iy*fhdr.nx + ix] = dirs[ie];
      }
    }

  }


  //
  //
  MdvxField *dirField = new MdvxField(fhdr, vhdr, buffer);
  //
  if (dirField->convertRounded(Mdvx::ENCODING_INT16,
			       Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }
  
  outMdvx.addField(dirField);

  /////////////////// U ///////////////////////////////////

  sprintf( fhdr.field_name_long,"%s", "U");
  sprintf( fhdr.field_name,"%s", "U");
  sprintf( fhdr.units,"%s", "m/s");
  sprintf( fhdr.transform,"%s","none");

  //
  // Set all to missing.
  //
  for (int k=0; k < fhdr.nx*fhdr.ny*fhdr.nz; k++){
    buffer[k] = fillVal;
  }

  //
  // Loop through entries, filling planes.
  //
  for (unsigned ie=0; ie < heights.size(); ie++){

    unsigned iz = (int)rint((heights[ie] - minHt) / (_params->heightRound/1000.0));

    for (int iy=0; iy < _params->grid.ny; iy++){
      for (int ix=0; ix < _params->grid.nx; ix++){
	buffer[iz*fhdr.ny*fhdr.nx + iy*fhdr.nx + ix] = PHYwind_u( speeds[ie], dirs[ie] );
      }
    }

  }

  //
  //
  MdvxField *uField = new MdvxField(fhdr, vhdr, buffer);
  //
  if (uField->convertRounded(Mdvx::ENCODING_INT16,
			     Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }
  
  outMdvx.addField(uField);

  /////////////////// V ///////////////////////////////////

  sprintf( fhdr.field_name_long,"%s", "V");
  sprintf( fhdr.field_name,"%s", "V");
  sprintf( fhdr.units,"%s", "m/s");
  sprintf( fhdr.transform,"%s","none");

  //
  // Set all to missing.
  //
  for (int k=0; k < fhdr.nx*fhdr.ny*fhdr.nz; k++){
    buffer[k] = fillVal;
  }

  //
  // Loop through entries, filling planes.
  //
  for (unsigned ie=0; ie < heights.size(); ie++){

    unsigned iz = (int)rint((heights[ie] - minHt) / (_params->heightRound/1000.0));

    for (int iy=0; iy < _params->grid.ny; iy++){
      for (int ix=0; ix < _params->grid.nx; ix++){
	buffer[iz*fhdr.ny*fhdr.nx + iy*fhdr.nx + ix] = PHYwind_v( speeds[ie], dirs[ie] );
      }
    }

  }


  //
  //
  MdvxField *vField = new MdvxField(fhdr, vhdr, buffer);
  //
  if (vField->convertRounded(Mdvx::ENCODING_INT16,
			     Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }
  
  outMdvx.addField(vField);


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
  return;

}
