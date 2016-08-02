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

#include <Mdv/DsMdvx.hh>
#include <iostream>
#include <stdlib.h>
#include <toolsa/umisc.h>
#include <cstdio>
#include <Mdv/MdvxField.hh>
#include "SimpleSpline.hh"

#include "Params.hh"

int main(int argc, char *argv[]){

  //
  // Get the TDRP parameters.
  //

  Params TDRP_params;

  if (TDRP_params.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }

  //
  // Allocate the data array and the spline arrays in X and Y.
  //
  float *data = (float *) malloc(TDRP_params.output_grid.nx *
				 TDRP_params.output_grid.ny *
				 sizeof(float));

  double *xArray = (double *) malloc(TDRP_params.output_grid.nx *
				     sizeof(double));

  double *yArray = (double *) malloc(TDRP_params.output_grid.ny *
				     sizeof(double));

  double *tagX = (double *) malloc(2 * TDRP_params.tagPoints_n *
				   sizeof(double));

  double *tagY = (double *) malloc(2 * TDRP_params.tagPoints_n *
				   sizeof(double));




  if ((xArray == NULL) || (yArray == NULL) || (tagX == NULL) || (tagY == NULL) || (data == NULL)){
    cerr << "Malloc failed!" << endl;
    exit(-1);
  }

  //
  // Copy the tag points into place, make sure they are symetric.
  //
  for (int it=0; it < TDRP_params.tagPoints_n; it++){
    tagX[it] = TDRP_params._tagPoints[it].x; tagY[it] = TDRP_params._tagPoints[it].y;
    tagX[2*TDRP_params.tagPoints_n-1-it] = 1.0-tagX[it];
    tagY[2*TDRP_params.tagPoints_n-1-it] = tagY[it];
  }

  //
  // Fill up the arrays in X and Y.
  //
  double x0 = 0.0;
  double xStep = 1.0/double(TDRP_params.output_grid.nx-1);

  SimpleSpline Sx(tagX, tagY, 2*TDRP_params.tagPoints_n,
		  x0, xStep, TDRP_params.output_grid.nx, xArray, 0);

  xStep = 1.0/double(TDRP_params.output_grid.ny-1);
  SimpleSpline Sy(tagX, tagY, 2*TDRP_params.tagPoints_n,
		  x0, xStep, TDRP_params.output_grid.ny, yArray, 0);
  //
  // Fill up the data array.
  //
  for (int ix = 0; ix < TDRP_params.output_grid.nx; ix++){
    double xVal = xArray[ix];
    for (int iy = 0; iy < TDRP_params.output_grid.ny; iy++){
      //
      // Take the minimum value, be it the X spline or the Y spline.
      //
      double yVal = yArray[iy];
      double dataVal = yVal;
      if (xVal < yVal) dataVal = xVal;
      int index = iy*TDRP_params.output_grid.nx + ix;
      data[index] = dataVal * TDRP_params.scaleWeight;

    }
  }

  free(tagX); free(tagY); free(xArray); free(yArray);
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
  // just surface data at this point. Needs much more thought.
  //
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0.0;

  //
  // Then set up the field header with the metadata.
  //
  fhdr.nx = TDRP_params.output_grid.nx;
  fhdr.ny = TDRP_params.output_grid.ny;
  fhdr.nz = 1;
  //
  fhdr.grid_dx = TDRP_params.output_grid.dx;
  fhdr.grid_dy = TDRP_params.output_grid.dy;
  //
  switch ( TDRP_params.output_projection){
      
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
  fhdr.proj_origin_lat =  TDRP_params.output_grid.latOrig;
  fhdr.proj_origin_lon =  TDRP_params.output_grid.lonOrig;
  //
  fhdr.grid_minx = TDRP_params.output_grid.minx;
  fhdr.grid_miny = TDRP_params.output_grid.miny;
  //
  // For Lambert Conformal, the first two proj params are set to the
  // lambert conformal latitudes.
  //
  if ( TDRP_params.output_projection == Params::PROJ_LAMBERT){
    fhdr.proj_param[0] = TDRP_params.output_grid.lambertLat1;
    fhdr.proj_param[1] = TDRP_params.output_grid.lambertLat2;
  }
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
  fhdr.bad_data_value = -999.0;
  fhdr.missing_data_value = -999.0;
  //
  // Set up some things in the master header.
  //
  Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  Mhdr.num_data_times = 1;
  Mhdr.data_ordering = Mdvx::ORDER_XYZ;
  Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  Mhdr.vlevel_included = 1;
  //
  //
  sprintf(Mhdr.data_set_info,"Filter");
  sprintf(Mhdr.data_set_name,"Filter");
  sprintf(Mhdr.data_set_source,"Filter");
  //
  // Set the times in the master and field headers.
  //
  time_t dataTime = time(NULL);
  //
  Mhdr.time_gen = dataTime;
  Mhdr.time_begin = dataTime;
  Mhdr.time_end = dataTime;
  Mhdr.time_expire = dataTime;
  Mhdr.time_centroid = dataTime;
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


  sprintf( fhdr.field_name_long,"%s", "filter" );
  sprintf( fhdr.field_name,"%s", "filter");
  sprintf( fhdr.units,"%s", "none" );
  sprintf( fhdr.transform,"%s","none");
  //
  MdvxField *field;
  //
  field = new MdvxField(fhdr, vhdr, data );
  //
  if (field->convertRounded(Mdvx::ENCODING_INT16,
			    Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "Conversion of field failed - I cannot go on.\n");
    exit(-1);
  }

  outMdvx.addField(field);


  //
  // OK - write out the data at this time.
  //
  outMdvx.setWritePath(TDRP_params.OutPath);
  if (outMdvx.writeToPath()) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to path : " << TDRP_params.OutPath << endl;
    cerr << outMdvx.getErrStr() << endl;
  }

  free(data); 


  return 0;

}
