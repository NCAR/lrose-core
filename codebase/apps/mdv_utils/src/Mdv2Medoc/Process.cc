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
#include <toolsa/Path.hh>
#include <Mdv/Mdvx_typedefs.hh>
using namespace std;
//
// Constructor
//
Process::Process(){
  
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
  int twoDvarSize = 0;
  //
  // Set up for the new data.
  //
  DsMdvx New;


  New.setDebug( P->Debug);

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (P->applyVlevels){
    New.setReadVlevelLimits( P->min_vlevel, P->max_vlevel );
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
  //
  // Add the field names we want to read. These should all
  // be on the same grid.
  //
  for (int ifld=0; ifld< P->In3DFieldName_n; ifld++){
    New.addReadField( P->_In3DFieldName[ifld] );
  }
  for (int ifld = 0; ifld < P->In2DFieldName_n; ifld++) {
    New.addReadField( P->_In2DFieldName[ifld] );
  }

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    cerr << New.getErrStr() << endl;
    return -1;
  }     

  // terrain data
  DsMdvx terrainMdv;

  terrainMdv.setDebug(P->Debug);
  terrainMdv.setReadTime(Mdvx::READ_LAST, P->TerrainUrl);
  terrainMdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  terrainMdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  terrainMdv.addReadField(P->InTerrainName);

  if (terrainMdv.readVolume()) {
    cerr << "Read terrain data failed from ";
    cerr << P->TerrainUrl << endl;
    cerr << terrainMdv.getErrStr() << endl;
    return -1;
  }

  //
  // Set up the output.
  //
  Mdvx::master_header_t InMhdr = New.getMasterHeader();

  //
  // Commence loop through 3D fields.
  //
  FILE *ofp = NULL;
  int iprint = 0;
  for (int ifld=0; ifld < P->In3DFieldName_n; ifld++){
    //
    // Get the desired field.
    //
    MdvxField *InField = New.getFieldByName( P->_In3DFieldName[ifld] );

    if (InField == NULL){
      cerr << "New field " << P->_In3DFieldName[ifld] << " not found." << endl;
      return -1;
    }

    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();
    //
    twoDvarSize = InFhdr.nx * InFhdr.ny;
    //
    // Write the header for the first field.
    //
    if (ifld == 0){
      date_time_t D;
      D.unix_time = T;
      uconvert_from_utime( &D );
      
      char outputFile[MAX_PATH_LEN];
      sprintf(outputFile,"%s/%s_%d%02d%02d_%02d%02d%02d.fmt",
	      P->OutDir, P->baseName,
	      D.year, D.month, D.day,
	      D.hour, D.min, D.sec);
      
      //
      // Make the path, open the file.
      //
      Path A(outputFile);
      if (A.makeDirRecurse()){
	fprintf(stderr,"Failed to make path for %s\n", outputFile);
	exit(-1);
      }

      ofp = fopen(outputFile,"wt");
      if (ofp == NULL){
	cerr << "Failed to open " << outputFile << endl;
	exit(-1);
      }
      //
      // Write the medoc header information.
      //
      fprintf(ofp,"FFFFFFFF\n");
      fprintf(ofp,"Mdv2Medoc\n");

      // n 2D fields + n 2D constant fields + terrain field
      int n_2d_fields = P->In2DFieldName_n + P->two_d_variables_n + 1;

      //
      // Timing information.
      //
      fprintf(ofp,"          %02d           %02d         %04d           %02d           %02d           %02d\n",
	      D.day, D.month, D.year, D.hour, D.min, D.sec );
      fprintf(ofp,"          %02d           %02d         %04d           %02d           %02d           %02d\n",
	      D.day, D.month, D.year, D.hour, D.min, D.sec ); 
      fprintf(ofp,"%12d %12d %12d %12d %12d %12d\n",
	      InMhdr.max_nx, InMhdr.max_ny, InMhdr.max_nz, 0, 
	      P->In3DFieldName_n, n_2d_fields);
      fprintf(ofp,"%12d %12d %12d %12d %12d %12d\n",
	      0, 0, 0, 0, 0, 0);
      fprintf(ofp,"%12d %12d %12d\n",
	      0, 0, 0 );

      // SZ array

      iprint=0;
      if (P->use_sigma_z) {
        for (int iz = 0; iz < P->sigma_z_values_n; iz++) {
          fprintf(ofp, "%12.4f ", P->_sigma_z_values[iz]);
          iprint++;
          if (iprint == 6) {
            iprint = 0;
            fprintf(ofp, "\n");
          }
        }

      } else {

        for (int iz = 0; iz < InFhdr.nz; iz++){
          float vlevel = InVhdr.level[iz];
          if (InVhdr.type[iz] != Mdvx::VERT_TYPE_SIGMA_P) {
            vlevel *= 1000.0;
          }

          fprintf(ofp,"%12.4f ", vlevel );
          iprint++;
          if (iprint == 6){
            iprint = 0;
            fprintf(ofp,"\n");
          }
        }
      }

      // DX, DY
      float dx = InFhdr.grid_dx;
      float dy = InFhdr.grid_dy;
      if (InFhdr.proj_type != Mdvx::PROJ_LATLON) {
        // km -> m
        dx *= 1000.0;
        dy *= 1000.0;
      }

      fprintf(ofp,"%12.4f ", dx );
      iprint++; if (iprint == 6){ iprint = 0; fprintf(ofp,"\n"); }

      fprintf(ofp,"%12.4f ", dy );
      iprint++; if (iprint == 6){ iprint = 0; fprintf(ofp,"\n"); }

      // X0, Y0: -999999 for spherical coordinate system

      float x0 = -999999.0, y0 = -999999.0;
      if (InFhdr.proj_type != Mdvx::PROJ_LATLON){
	x0 = 0.0; y0 = 0.0;
      }

      fprintf(ofp,"%12.4f ", x0 );
      iprint++; if (iprint == 6){ iprint = 0; fprintf(ofp,"\n"); }

      fprintf(ofp,"%12.4f ", y0 );
      iprint++; if (iprint == 6){ iprint = 0; fprintf(ofp,"\n"); }

      //
      // Get the lat, lon of the SW corner.
      //
      double lat0 = 0.0, lon0 = 0.0;
      if (InFhdr.proj_type == Mdvx::PROJ_LATLON){
	lat0 = InFhdr.grid_miny; lon0 = InFhdr.grid_minx;
      } else {
	 PJGLatLonPlusDxDy(InFhdr.proj_origin_lat, InFhdr.proj_origin_lon,
			   InFhdr.grid_minx, InFhdr.grid_miny,
			   &lat0, &lon0 );
      }
      
      if (P->overrideMdvLatLon){
	lat0 = P->lat; lon0 = P->lon;
      }

      fprintf(ofp,"%12.4f ", lat0 );
      iprint++; if (iprint == 6){ iprint = 0; fprintf(ofp,"\n"); }

      fprintf(ofp,"%12.4f ", lon0 );
      iprint++; if (iprint == 6){ iprint = 0; fprintf(ofp,"\n"); }


      for (int idum=0; idum < 4; idum++){
	fprintf(ofp,"%12.4f ", 0.0);
	iprint++; if (iprint == 6){ iprint = 0; fprintf(ofp,"\n"); }
      }

      // ZTOP

      if (P->use_sigma_z) {
        fprintf(ofp, "%12.4f ", P->_sigma_z_values[P->sigma_z_values_n - 1]);
      } else {
        fprintf(ofp,"%12.4f ", InVhdr.level[InFhdr.nz-1]);
      }
      iprint++; if (iprint == 6){ iprint = 0; fprintf(ofp,"\n"); }

      if (iprint != 0) {
	fprintf(ofp,"\n");
      }
      iprint = 0;

      //
      // Names and units for 3D fields.
      //
      for (int ofld=0; ofld< P->Out3DFieldName_n; ofld++){
	fprintf(ofp,"%-8s ", P->_Out3DFieldName[ofld] );
	iprint++; if (iprint == 6){ iprint = 0; fprintf(ofp,"\n"); }
      }
      
      for (int iunits=0; iunits < P->Out3DUnits_n; iunits++){
	fprintf(ofp,"%-8s ", P->_Out3DUnits[iunits] );
	iprint++; if (iprint == 6){ iprint = 0; fprintf(ofp,"\n"); }
      }

      //
      // Same for 2D fields.
      //
      // Names
      for (int ofld = 0; ofld < P->Out2DFieldName_n; ofld++) {
        fprintf(ofp, "%-8s ", P->_Out2DFieldName[ofld]);
        iprint++; if (iprint == 6) { iprint = 0; fprintf(ofp, "\n"); }
      }
      for (int ofld=0; ofld< P->two_d_variables_n; ofld++){
	fprintf(ofp,"%-8s ", P->_two_d_variables[ofld].name );
	iprint++; if (iprint == 6){ iprint = 0; fprintf(ofp,"\n"); }
      }
      fprintf(ofp, "%-8s ", P->OutTerrainName);
      iprint++; if (iprint == 6) { iprint = 0; fprintf(ofp, "\n"); }

      // Units
      for (int ounits = 0; ounits < P->Out2DUnits_n; ounits++) {
        fprintf(ofp, "%-8s ", P->_Out2DUnits[ounits]);
        iprint++; if (iprint == 6) { iprint = 0; fprintf(ofp, "\n"); }
      }
      for (int ofld=0; ofld< P->two_d_variables_n; ofld++){
	fprintf(ofp,"%-8s ", P->_two_d_variables[ofld].units );
	iprint++; if (iprint == 6){ iprint = 0; fprintf(ofp,"\n"); }
      }
      fprintf(ofp, "%-8s ", P->OutTerrainUnits);
      iprint++; if (iprint == 6) { iprint = 0; fprintf(ofp, "\n"); }

      if (iprint != 0) {
	fprintf(ofp,"\n");
      }
      iprint = 0;

    } // End of if this is the first field.

    //
    // Dump the data.
    //
    float *data = (float *) InField->getVol();
    for (int idata=0; idata < InFhdr.nx * InFhdr.ny * InFhdr.nz; idata++){

      if (
	  (data[idata] == InFhdr.bad_data_value) ||
	  (data[idata] == InFhdr.missing_data_value)
	  ){
	fprintf(ofp, "%12.4f ", -99999.0);
      } else {
	fprintf(ofp, "%12.4f ", data[idata]);
      }
      iprint++;
      if (iprint == 6){
	fprintf(ofp,"\n");
	iprint = 0;
      }
    }
    //
    if (iprint != 0) fprintf(ofp,"\n");
    iprint = 0;
    //
  } // End of loop through the 3D fields.

  //
  // 
  // Now, add the 2D variables.
  //
  for (int ifld = 0; ifld < P->In2DFieldName_n; ifld++) {
    //
    // Get the desired field.
    //
    MdvxField *In2DField = New.getFieldByName(P->_In2DFieldName[ifld]);

    if (In2DField == NULL){
      cerr << "New field " << P->_In2DFieldName[ifld] << " not found." << endl;
      return -1;
    }

    Mdvx::field_header_t InFhdr = In2DField->getFieldHeader();

    //
    // Dump the data.
    //
    float *data = (float *) In2DField->getVol();
    for (int idata=0; idata < InFhdr.nx * InFhdr.ny; idata++){

      if (
	  (data[idata] == InFhdr.bad_data_value) ||
	  (data[idata] == InFhdr.missing_data_value)
	  ){
	fprintf(ofp, "%12.4f ", -99999.0);
      } else {
	fprintf(ofp, "%12.4f ", data[idata]);
      }
      iprint++;
      if (iprint == 6){
	fprintf(ofp,"\n");
	iprint = 0;
      }
    }
    //
    if (iprint != 0) fprintf(ofp,"\n");
    iprint = 0;

  }

  // 2D constant
  for (int ivar = 0; ivar < P->two_d_variables_n; ivar++){
    for (int idata=0; idata < twoDvarSize; idata++){
      fprintf(ofp, "%12.4f ", P->_two_d_variables[ivar].value );
      iprint++;
      if (iprint == 6){
	fprintf(ofp,"\n");
	iprint = 0;
      }
    }
    if (iprint != 0) fprintf(ofp,"\n");
    iprint = 0;
  }

  // terrain
  MdvxField* terrainField = terrainMdv.getFieldByName(P->InTerrainName);
  if (terrainField == NULL) {
    cerr << "Terrain field " << P->InTerrainName << " not found." << endl;
    return -1;
  }
  Mdvx::field_header_t InFhdr = terrainField->getFieldHeader();
  float *data = (float *) terrainField->getVol();
  for (int idata = 0; idata < InFhdr.nx * InFhdr.ny; idata++) {
    if (
      (data[idata] == InFhdr.bad_data_value) ||
      (data[idata] == InFhdr.missing_data_value)
    ) {
      fprintf(ofp, "%12.4f ", -99999.0);
    } else {
      fprintf(ofp, "%12.4f ", data[idata]);
    }
    iprint++;
    if (iprint == 6){
      fprintf(ofp,"\n");
      iprint = 0;
    }
  }
  if (iprint != 0) fprintf(ofp,"\n");
  iprint = 0;

  fclose( ofp );
  
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

}










