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
// LordOfTheMdv.cc
//
// LordOfTheMdv object
//
// Watches over MDV data in/out
//
// October 2002
//
///////////////////////////////////////////////////////////////

#include <cmath>
#include <toolsa/mem.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh> 
#include <Mdv/MdvxField.hh> 
#include "LordOfTheMdv.hh"
using namespace std;


/////////////////////////////////////////////////////////////// 
//
// constructor. Sets up DsMdvx object.
//
LordOfTheMdv::LordOfTheMdv (Params *TDRP_params,
			    time_t start,
			    time_t end){
  //
  // Make a copy of the TDRP parameters and start and end times.
  //
  _params = TDRP_params;
  _start = start;
  _end = end;
  //
  // Ititialize the Mdv object
  //

  LordOfTheMdv::_startMdvObject();



}


///////////////////////////////////////////////////////////////
//
// addGaussian. Adds Gaussian to the output grid.
//
void LordOfTheMdv::addGaussian(double lat, double lon, 
			       double Duration, double Area){

                            
  MdvxField *weightField; 
  weightField = _Mdv->getFieldByName( "initWeight" );      
  Mdvx::field_header_t weightFhdr = weightField->getFieldHeader();        
  fl32 *Data = (fl32 *) weightField->getVol(); 

  //
  // Make a copy of the data to work with.
  //
  fl32 *countData = (fl32 *) malloc(sizeof( fl32 ) * weightFhdr.nx*weightFhdr.ny*weightFhdr.nz);
  if (countData == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  for (int i=0; i < weightFhdr.nx*weightFhdr.ny*weightFhdr.nz; i++){
    countData[i] = Data[i];
  }
  
  Mdvx::master_header_t master_hdr = _Mdv->getMasterHeader();
  MdvxProj Proj(master_hdr, weightFhdr);  

  //
  // Get the extrema of the indicies for the gaussian.
  //
  int maxix=0, maxiy=0, minix=0, miniy=0; // Set to 0 to avoid compiler warnings.
  bool first = true;
  for (double dir=0; dir <357.5; dir = dir + 5.0){
    //
    // Move out by the radius to check the extrema.
    //
    double Lat2, Lon2;
    PJGLatLonPlusRTheta(lat, lon, _params->radius,
			dir, &Lat2, &Lon2);
    //
    // 
    //
    int ix, iy;
    if (0 == Proj.latlon2xyIndex(Lat2, Lon2, ix, iy)){ 
      
      if (first){
	first = false;
	maxix = minix = ix;    maxiy = miniy = iy;
      } else {
	if (maxix < ix) maxix = ix;
	if (maxiy < iy) maxiy = iy;
	if (minix > ix) minix = ix;
	if (miniy > iy) miniy = iy;
      }
    }
  }

  if (first){
    //
    // None of the radial points were encompassed, use whole grid.
    //
    minix = 0; maxix = weightFhdr.nx-1;
    miniy = 0; maxiy = weightFhdr.ny-1;
  }

  const double decay = log(0.5) / _params->halfLife;
  double weight;
  switch ( _params->weighting ){

  case Params::WEIGHT_NONE :
    //
    // Use this if something weird is in the param file.
    //
  default :
    weight = 1.0;
    break;

  case Params::WEIGHT_DURATION :
    weight = Duration;
    break;

  case Params::WEIGHT_AREA :
    weight = Area;
    break;

  }

  if (weight < 0.0){ // The bad value.
    cerr << "Weight is less than zero, I cannot go on." << endl;
    exit(01);
  }


  for (int ix = minix; ix <= maxix; ix++){
    for (int iy = miniy; iy <= maxiy; iy++){
      //
      int index = ix + weightFhdr.nx*iy;
      //
      // Get the lat/lon of this point.
      //
      double latp, lonp;
      Proj.xyIndex2latlon( ix, iy, latp, lonp);
      //
      // Get the radial distance between these two points.
      //
      double dist, theta;
      PJGLatLon2RTheta(lat, lon, latp, lonp, &dist, &theta);
      //
      // Fill it in if it is within the radius.
      //
      if (dist <= _params->radius){
	countData[index] = countData[index] + weight * exp( dist * decay );
      }

    }
  }

  weightField->setVolData( countData, 
			  weightFhdr.nx*weightFhdr.ny*weightFhdr.nz*sizeof(fl32),
			  Mdvx::ENCODING_FLOAT32,
			  Mdvx::SCALING_NONE);

  free(countData);
  return;


}
///////////////////////////////////////////////////////////////  
//
// destructor. Write Mdv data out.
//
LordOfTheMdv::~LordOfTheMdv(){
  //
  // Set the time on the output data.
  //
  Mdvx::master_header_t master_hdr = _Mdv->getMasterHeader();

  time_t Now; Now=time(NULL);
  master_hdr.time_gen = Now;

  master_hdr.time_begin = _start;
  master_hdr.time_end = _end;

  switch ( _params->timestampMode) {
     case Params::TIMESTAMP_START :
        master_hdr.time_centroid = _start;
        break;

     case Params::TIMESTAMP_MIDDLE :
        master_hdr.time_centroid = _start + (_end - _start) / 2;
        break;

     case Params::TIMESTAMP_END :
        master_hdr.time_centroid = _end;
        break;

     default :
        cerr << "Unrecognized timestamp option." << endl;
        exit(-1);
        break;

  }

  master_hdr.time_expire = _end + (_end - _start);

  _Mdv->setMasterHeader( master_hdr );


  if (_params->debug){
    cerr << "Writing MDV output to " << _params->mdvUrl;
    cerr << " for time " << utimstr( master_hdr.time_centroid ) << endl;
  }

  //
  // Set the times in the field headers.
  //
  MdvxField *weightField; 
  weightField = _Mdv->getFieldByName( "initWeight" );      
  Mdvx::field_header_t weightFhdr = weightField->getFieldHeader();        
  fl32 *countData = (fl32 *) weightField->getVol(); 
  weightFhdr.forecast_delta = 0;
  weightFhdr.forecast_time =  master_hdr.time_centroid;
  //
  // Find the maximum of the unnormalized data.
  //
  double max=0.0;
  for(int i=0; i < weightFhdr.nx*weightFhdr.ny*weightFhdr.nz; i++){
    if (countData[i] > max) max = countData[i];
  }
 weightField->setFieldHeader( weightFhdr );

  //
  // Compress the fields appropriately.
  //
  if (weightField->convertRounded(Mdvx::ENCODING_INT8,
				 Mdvx::COMPRESSION_ZLIB)){
    cerr << "count convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }

  //
  // Write output.
  //
  if (_Mdv->writeToDir( _params->mdvUrl )) {
    cerr << "Failed to wite to " << _params->mdvUrl << endl;
    exit(-1);
  }

  delete _Mdv;

}

///////////////////////////////////////////////////////////////  
//
// Start an MDV object from scratch.
//
void LordOfTheMdv::_startMdvObject(){

  //
  // Set up the master header.
  //
  Mdvx::master_header_t master_hdr;

  time_t Now; Now=time(NULL);
  master_hdr.time_gen = Now;

  master_hdr.time_begin = _start;
  master_hdr.time_end = _end;

  switch ( _params->timestampMode) {

     case Params::TIMESTAMP_START :
        master_hdr.time_centroid = _start;
        break;
 
     case Params::TIMESTAMP_MIDDLE :
        master_hdr.time_centroid = _start + (_end - _start) / 2;
        break;
 
     case Params::TIMESTAMP_END :
        master_hdr.time_centroid = _end;
        break;
 
     default :
        cerr << "Unrecognized timestamp option." << endl;
        exit(-1);
        break;
 
  }


  master_hdr.time_expire = _end + (_end - _start);

  sprintf(master_hdr.data_set_info,"%s","StormInit weights");
  sprintf(master_hdr.data_set_name,"%s","StormInit weights");
  sprintf(master_hdr.data_set_source,"%s", _params->spdbUrl );

  _Mdv = new DsMdvx();

  _Mdv->setMasterHeader(master_hdr);
  _Mdv->clearFields();    

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.nx =  _params->output_grid.nx;
  fhdr.ny =  _params->output_grid.ny;
  fhdr.nz =  1; // Surface data.

  switch ( _params->output_projection){

  case Params::OUTPUT_PROJ_LATLON :
    fhdr.proj_type = Mdvx::PROJ_LATLON;
    break;

  case Params::OUTPUT_PROJ_FLAT :
    fhdr.proj_type = Mdvx::PROJ_FLAT;
    break;

  case Params::OUTPUT_PROJ_LAMBERT :
    fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
    break;

  }

  fhdr.proj_origin_lat =   _params->output_origin.lat;
  fhdr.proj_origin_lon =   _params->output_origin.lon;
  fhdr.grid_dx =  _params->output_grid.dx;
  fhdr.grid_dy =  _params->output_grid.dy;
  fhdr.grid_dz =  0.0;
 
  fhdr.grid_minx =  _params->output_grid.minx;
  fhdr.grid_miny =  _params->output_grid.miny;
  fhdr.grid_minz =  0.0;

  fhdr.bad_data_value = -1.0;
  fhdr.missing_data_value = fhdr.bad_data_value;
  fhdr.proj_rotation = _params->output_rotation;

  sprintf(fhdr.field_name_long,"%s", "initWeight");
  sprintf(fhdr.field_name,"%s", "initWeight");
  sprintf(fhdr.units,"%s", "None");
  sprintf(fhdr.transform,"%s"," ");

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);

  for (int iz = 0; iz < fhdr.nz; iz++) {
    vhdr.type[iz] = Mdvx::VERT_TYPE_SURFACE;
    vhdr.level[iz] = fhdr.grid_minz + iz * fhdr.grid_dz;
  }

  //
  // Create data, all zero.
  //
  fl32 *data = (fl32 *)calloc(fhdr.nx*fhdr.ny*fhdr.nz, sizeof(fl32));
  if (data == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }


  // create field
 
  MdvxField *field = new MdvxField(fhdr, vhdr, data);
 
  // add field to mdvx object
 
  _Mdv->addField(field);


  free(data);

  return;

} 



