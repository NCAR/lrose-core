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

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh>
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
int Process::Derive(Params *P, time_t T){


  if (P->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }

  //
  // Set up for the new data.
  //
  DsMdvx New;


  New.setDebug( P->Debug);

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  //
  // Only read the fields we will actually use.
  //
  New.addReadField( P->terrainFieldName );
  for (int ifld=0; ifld < P->fields_n; ifld++){
    New.addReadField( P->_fields[ ifld ].name );
  }


  if (P->RemapGrid){
    switch ( P->grid_projection){

    case Params::FLAT:
      New.setReadRemapFlat(P->grid_nx, P->grid_ny,
			 P->grid_minx, P->grid_miny,
			 P->grid_dx, P->grid_dy,
			 P->grid_origin_lat, P->grid_origin_lon,
			 P->grid_rotation);

      break;
                                   
    case Params::LATLON:
      New.setReadRemapLatlon(P->grid_nx, P->grid_ny,
			   P->grid_minx, P->grid_miny,
			   P->grid_dx, P->grid_dy);

      break;            

    case Params::LAMBERT:
      New.setReadRemapLc2(P->grid_nx, P->grid_ny,
			P->grid_minx, P->grid_miny,
			P->grid_dx, P->grid_dy,
			P->grid_origin_lat, 
			P->grid_origin_lon,
			P->grid_lat1,  P->grid_lat2);
      
      break;
      
    default:
      cerr << "Unsupported projection." << endl;
      return -1;
      break;
      
    }               
  }

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     

  //
  // Set up the output.
  //
  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  Mdvx::master_header_t OutMhdr = InMhdr;

  if (P->sigma_Z){
    OutMhdr.vlevel_type = Mdvx::VERT_TYPE_SIGMA_Z;
  } else {
    OutMhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  }
  Out.setMasterHeader(OutMhdr);
  Out.clearFields();     

  const fl32 OutBadValue = -9999.0;

  //
  // Read the terrain field. This should be a 2D field.
  //
  MdvxField *terrainField;
    
  terrainField = New.getFieldByName( P->terrainFieldName );

  if ( terrainField == NULL){
    cerr << "New field " << P->terrainFieldName << " not found." << endl;
    return -1;
  }

  Mdvx::field_header_t terrainFhdr = terrainField->getFieldHeader();
  Mdvx::vlevel_header_t terrainVhdr = terrainField->getVlevelHeader();

  if (terrainFhdr.nz != 1){
    cerr << P->terrainFieldName << " is not two dimensional, I cannot cope." << endl;
    exit(-1);
  }

  fl32 *terrainData = (fl32 *) terrainField->getVol();

  double P_00 =  InMhdr.user_data_fl32[0]; // Base state sea-level pressure (Pa)
  double T_s0 =  InMhdr.user_data_fl32[1]; // Base state sea-level temperature (K)
  double A =     InMhdr.user_data_fl32[2]; // Base state lapse rate d(T)/d(ln P)
  double T_iso = InMhdr.user_data_fl32[3]; // Base state isothermal stratospheric temperature (K)
  double P_top = InMhdr.user_data_fl32[4]; // Top pressure (Pa)

  if (
      (P_00 == 0.0) &&
      (T_s0 == 0.0) &&
      (A == 0.0)    &&
      (T_iso == 0.0) &&
      (P_top == 0.0)
      ){
    cerr << "Base state varibles were not set" << endl;
    exit(-1);
  }

  const double r = 287.0;  // Gas constant
  const double   g = 9.81; // Gravitational constant

  //
  // Calculate the surface reference pressure.
  //

  if (P->Debug){
    cerr << "Pre-computing heights from field " << P->_fields[0].name << endl;
  }

  fl32 *p_s0 = (fl32 *) malloc(sizeof(fl32) * terrainFhdr.nx * terrainFhdr.ny);
  if ( p_s0 == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  for (int i=0; i < terrainFhdr.nx * terrainFhdr.ny; i++){
    p_s0[i] = P_00 * exp(-T_s0/A + sqrt( (T_s0/A)*(T_s0/A) - 2.0*g*terrainData[i]/(A*r))) - P_top;
    /* --------------
    if (terrainData[i] > 10.0){
      cerr << terrainData[i] << " : " << p_s0[i] << endl;
    }
    ----------------- */
  }

  //
  // Calculate the 3D reference pressures. This is done using the sigma_p
  // levels of the first field on the field list.
  //
  MdvxField *firstField;
    
  firstField = New.getFieldByName( P->_fields[0].name );

  if ( firstField == NULL ) {
    cerr << "First field " << P->_fields[0].name << " not found." << endl;
    exit(-1);
  }

  Mdvx::vlevel_header_t firstVhdr = firstField->getVlevelHeader();
  Mdvx::field_header_t firstFhdr = firstField->getFieldHeader();

  for (int iz = 0; iz < firstFhdr.nz; iz++){
    if ( firstVhdr.type[iz] != Mdvx::VERT_TYPE_SIGMA_P ){
      cerr << "First field is not of vlevel type sigma P" << endl;
      exit(-1);
    }
  }

  fl32 *P0 = (fl32 *) malloc(sizeof(fl32) * firstFhdr.nx * firstFhdr.ny * firstFhdr.nz);
  if ( P0 == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  for (int ix = 0; ix < firstFhdr.nx; ix++){
    for (int iy = 0; iy < firstFhdr.ny; iy++){

      int index2D = iy * firstFhdr.nx + ix;

      for (int iz = 0; iz < firstFhdr.nz; iz++){

	int index3D = iz * firstFhdr.nx * firstFhdr.ny +
	  iy * firstFhdr.nx + ix;

	P0[index3D] = p_s0[index2D] * firstVhdr.level[iz] + P_top;

	// cerr << iz << " : " << firstVhdr.level[iz] << " : " << P0[index3D] << endl;

      }
    }
  }

  //
  // Calculate the heights.
  //

  fl32 *heights = (fl32 *) malloc(sizeof(fl32) * firstFhdr.nx * firstFhdr.ny * firstFhdr.nz);
  if ( heights == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }
  
  for (int i=0; i < firstFhdr.nx * firstFhdr.ny * firstFhdr.nz; i++){

    double theLog = log(P0[i] / P_00 );
    heights[i] = - ( r*A*theLog*theLog/(2.0*g) + r*T_s0*theLog/g  ) / 1000.0; // Convert from m to Km

  }

  /*----------------------------------------------------
  for (int ix = 0; ix < firstFhdr.nx; ix++){
    for (int iy = 0; iy < firstFhdr.ny; iy++){
      for (int iz = 0; iz < firstFhdr.nz; iz++){

	int index3D = iz * firstFhdr.nx * firstFhdr.ny +
	  iy * firstFhdr.nx + ix;


	cerr << iz << " : " << firstVhdr.level[iz] << " : " << P0[index3D];
	cerr << " : " <<  heights[index3D]  << endl;

      }
    }
  }
  ----------------------------------------------------*/


  free ( p_s0 ); free(P0); 

  //
  // Set up a vlevel header suitable for output.
  //
  Mdvx::vlevel_header_t OutVhdr;
  memset(&OutVhdr,0,sizeof(OutVhdr));
  for (int il=0; il < P->outputHeights_n; il++){
    OutVhdr.level[il] = P->_outputHeights[il];
    if (P->sigma_Z){
      OutVhdr.type[il] = Mdvx::VERT_TYPE_SIGMA_Z;
    } else {
      OutVhdr.type[il] = Mdvx::VERT_TYPE_Z;
    }
  }
  //
  // Loop through the specified fields.
  //
  for (int ifld=0; ifld < P->fields_n; ifld++){
    //
    //
    //
    if (P->Debug){
      cerr << "Processing field " << P->_fields[ifld].name << endl;
    }
    //
    // Get the desired field.
    //
    MdvxField *InField;

    InField = New.getFieldByName( P->_fields[ifld].name );
   

    if (InField == NULL){
      cerr << "New field " << P->_fields[ifld].name << " not found." << endl;
      return -1;
    }
    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

    MdvxProj Proj(InMhdr, InFhdr);

    fl32 *InData = (fl32 *) InField->getVol();

    fl32 *OutData = (fl32 *) malloc(sizeof(float) * 
				    InFhdr.nx *
				    InFhdr.ny *
				    P->outputHeights_n );
    //
    // Init to missing.
    //
    for (int ik=0; ik < InFhdr.nx * InFhdr.ny *  P->outputHeights_n; ik++){
      OutData[ik] = OutBadValue;
    }

    for (int ix=0; ix < InFhdr.nx; ix++){
      for (int iy=0; iy < InFhdr.ny; iy++){
	for (int iz=0; iz < P->outputHeights_n; iz++){
	  //
	  //
	  //
	  int outIndex = iz * InFhdr.nx * InFhdr.ny + iy * InFhdr.nx + ix;
	  //
	  // The target height is specified.
	  //
	  double desiredHeight =  P->_outputHeights[iz];
	  if (P->sigma_Z){
	    //
	    // Do calculation to get actual Z
	    //
	    double Zterrain = terrainData[iy * terrainFhdr.ny + ix]/1000.0;
	    //
	    desiredHeight = Zterrain + (P->Z_top - Zterrain) * desiredHeight/P->Z_top;
	  }
	  //
	  // find the model heights that straddle that height in the computed heights.
	  //
	  for (int imz=0; imz < InFhdr.nz-1; imz++){
	    
	    int index3D = imz * InFhdr.nx * InFhdr.ny +
	      iy * InFhdr.nx + ix;

	    int index3Dabove = (imz + 1) * InFhdr.nx * InFhdr.ny +
	      iy * InFhdr.nx + ix;

	    if (
		(heights[index3D] <= desiredHeight) &&
		(heights[index3Dabove] >= desiredHeight)
		){

	      if (P->_fields[ifld].interpMethod == Params::INTERP_POWER){
		_interpPower(heights[index3D], heights[index3Dabove],
			     InData[index3D],  InData[index3Dabove],
			     InFhdr.bad_data_value, InFhdr.missing_data_value,
			     desiredHeight,
			     &OutData[outIndex],
			     P->surfaceSpeed,
			     P->surfaceHeight);
	      }
	      
	      if (P->_fields[ifld].interpMethod == Params::INTERP_LINEAR){
		_interpLinear(heights[index3D], heights[index3Dabove],
			      InData[index3D],  InData[index3Dabove],
			      InFhdr.bad_data_value, InFhdr.missing_data_value,
			      desiredHeight,
			      &OutData[outIndex]);
	      }

	    }
	  }

	}
      }
    }


    if (P->Debug){
      int first = 1;
      double min=0.0; double max = min;
      for (int i=0; i < InFhdr.nx*InFhdr.ny*P->outputHeights_n; i++){
	if (OutData[i] == OutBadValue) continue;
	if (std::isnan(OutData[i])){
	  cerr << "NAN!!" << endl;
	  exit(0);
	}
	if (first){
	  first = 0;
	  min = OutData[i]; max = OutData[i];
	} else {
	  if (OutData[i] < min) min = OutData[i];
	  if (OutData[i] > max) max = OutData[i];
	}
      }
      
      cerr << "Out values range from " << min << " to " << max << endl;
    }

    Mdvx::field_header_t OutFhdr;
    memset( &OutFhdr, 0, sizeof(OutFhdr));

    OutFhdr.nx = InFhdr.nx;
    OutFhdr.ny = InFhdr.ny;

    OutFhdr.nz =  P->outputHeights_n;
    OutFhdr.volume_size = sizeof(fl32)*OutFhdr.nx*OutFhdr.ny*OutFhdr.nz;
    //
    if (P->sigma_Z){
      OutFhdr.vlevel_type = Mdvx::VERT_TYPE_SIGMA_Z;
    } else {
      OutFhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
    }
    OutFhdr.field_code = InFhdr.field_code;
    OutFhdr.forecast_delta = InFhdr.forecast_delta;
    OutFhdr.forecast_time = InFhdr.forecast_time;
    OutFhdr.proj_type = InFhdr.proj_type;
    OutFhdr.encoding_type = InFhdr.encoding_type;
    OutFhdr.data_element_nbytes = InFhdr.data_element_nbytes;
    OutFhdr.transform_type = InFhdr.transform_type;
    OutFhdr.scaling_type = InFhdr.scaling_type;
    OutFhdr.compression_type = InFhdr.compression_type;
    OutFhdr.native_vlevel_type = InFhdr.native_vlevel_type;
    OutFhdr.proj_origin_lat = InFhdr.proj_origin_lat;
    OutFhdr.proj_origin_lon = InFhdr.proj_origin_lon;

    for (int ijk=0; ijk < MDV_MAX_PROJ_PARAMS; ijk++){
      OutFhdr.proj_param[ijk] = InFhdr.proj_param[ijk];
    }

    OutFhdr.grid_dx = InFhdr.grid_dx;
    OutFhdr.grid_dy = InFhdr.grid_dy;
    OutFhdr.grid_dz = InFhdr.grid_dz;
    OutFhdr.grid_minx = InFhdr.grid_minx;
    OutFhdr.grid_miny = InFhdr.grid_miny;
    OutFhdr.grid_minz = OutVhdr.level[0];

    OutFhdr.bad_data_value = OutBadValue;
    OutFhdr.missing_data_value = OutBadValue;
    OutFhdr.proj_rotation = InFhdr.proj_rotation;

    sprintf(OutFhdr.field_name_long ,"%s", InFhdr.field_name_long);
    sprintf(OutFhdr.field_name ,"%s", InFhdr.field_name);

    sprintf(OutFhdr.units ,"%s", InFhdr.units);
    sprintf(OutFhdr.transform ,"%s", InFhdr.transform);

    MdvxField *fld = new MdvxField(OutFhdr, OutVhdr, OutData);

    /*---
    fld->print(cerr);
    exit(0);
    */

    free(OutData);
    if (fld->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  

    /*---
    fld->print(cerr);
    exit(0);
    -- */

    Out.addField(fld);
  }


  free(heights);

  //
  // Add the terrain field.
  //

  MdvxField *tfld = new MdvxField(terrainFhdr, terrainVhdr, terrainData);

  if (tfld->convertRounded(Mdvx::ENCODING_INT8,
			   Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  
  Out.addField( tfld );

  //
  // Only do the write if fields were added.
  //

  Mdvx::master_header_t Mhdr = Out.getMasterHeader();

  if (Mhdr.n_fields > 0){
    Out.setWriteAsForecast();
    if (Out.writeToDir( P->OutUrl)) {
      cerr << "Failed to write to " << P->OutUrl << endl;
      exit(-1);
    }      
  }


  if (P->Debug){
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


void Process::_interpPower(fl32 height, fl32 heightAbove,
			   fl32 data,  fl32 dataAbove,
			   fl32 missing_data_value,
			   fl32 bad_data_value,
			   fl32 desiredHeight,
			   fl32 *OutData,
			   fl32 surfaceSpeed,
			   fl32 surfaceHeight){


  if (
      (data == missing_data_value) ||
      (dataAbove == missing_data_value) ||
      (data == bad_data_value) ||
      (dataAbove == bad_data_value)
      ){
    return;
  }

  double h0 = surfaceHeight;
  double u0 = surfaceSpeed;

  double h1=0.0, u1=0.0;

  if (fabs(height - desiredHeight) < fabs(heightAbove - desiredHeight)){
    h1 = height;
    u1 = data;
  } else {
    h1 = heightAbove;
    u1 = dataAbove;
  }
  //
  // Make sure u1 and u0 are of the same sign.
  //
  if (u1*u0 < 0.0){
    u0 = -u0;
  }

  //
  // Make sure u0 is smaller in magnitude than u1.
  //
  if (fabs(u0) > fabs(u1)/10.0 ){
    u0 = u1 /1000.0;
  }

  if ((u0 == 0.0) && (u1 == 0.0)){
    *OutData = 0.0;
    return;
  }

  double logA = (u1 * log(h0) - u0 * log(h1)) / (u0 - u1);

  if (logA < 0.0){
    cerr << "logA is negative, I cannot cope." << endl;
    cerr << "U0 : " << u0 << endl;
    cerr << "U1 : " << u1 << endl;
    cerr << "H0 : " << h0 << endl;
    cerr << "H0 : " << h1 << endl;
    cerr << "logA : " << logA << endl;
    exit(-1);
  }
  
  double a = exp(logA);
  double k = u1 / log(a*h1);
	      
  *OutData = k*log(a*desiredHeight);

  if (std::isnan(*OutData)){
    cerr << "NAN " << endl;
    cerr << height << " " << desiredHeight << " " << heightAbove << endl;
    cerr << logA << endl;
    cerr << h0 << endl;
    cerr << h1 << endl;
    cerr << u0 << endl;
    cerr << u1 << endl;
    cerr << a << endl;

    exit(0);
  }

  return;

}

void Process::_interpLinear(fl32 height, fl32 heightAbove,
			    fl32 data,  fl32 dataAbove,
			    fl32 missing_data_value,
			    fl32 bad_data_value,
			    fl32 desiredHeight,
			    fl32 *OutData){

  
  if (
      (data == missing_data_value) ||
      (dataAbove == missing_data_value) ||
      (data == bad_data_value) ||
      (dataAbove == bad_data_value)
      ){
    return;
  }

  double t = (desiredHeight - height)/(heightAbove - height);
  *OutData = t*dataAbove + (1.0 -t)*data;

  return;

}




