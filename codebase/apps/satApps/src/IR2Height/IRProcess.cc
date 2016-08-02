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

#include <Mdv/MdvxField.hh>

#include <Mdv/MdvxProj.hh>

#include "Output.hh"
#include "IRProcess.hh"
#include "Params.hh"


IRProcess::IRProcess(){
}

IRProcess::~IRProcess(){
}



int IRProcess::IRProcessData( time_t IRTime, Params *P){


  PMU_auto_register("Reading data");

  DsMdvx IR, Model;

  //
  // Remap the satellite data grid, if desired.
  //
  if (P->DoRemap){

    switch (P->grid_projection){

    case Params::FLAT:
      IR.setReadRemapFlat(P->grid_nx, P->grid_ny,
			  P->grid_minx, P->grid_miny,
			  P->grid_dx, P->grid_dy,
			  P->grid_origin_lat, P->grid_origin_lon,
			  P->grid_rotation);
      break;

    case Params::LATLON:
      IR.setReadRemapLatlon(P->grid_nx, P->grid_ny,
			    P->grid_minx, P->grid_miny,
			    P->grid_dx, P->grid_dy);
      break;

    case Params::LAMBERT:
      IR.setReadRemapLc2(P->grid_nx, P->grid_ny,
			 P->grid_minx, P->grid_miny,
			 P->grid_dx, P->grid_dy,
			 P->grid_origin_lat, P->grid_origin_lon,
			 P->grid_lat1,  P->grid_lat2);
      break;

    default:
      cerr << "Unsupported projection." << endl;
      exit(-1);
      break;

    }                         

  }

  //
  // Read the satellite data.
  //


  IR.setReadTime(Mdvx::READ_CLOSEST,
		 P->IR_URL,
		 0,
		 IRTime );

  IR.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  IR.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (P->Debug){
    cerr << "Reading IR data at " << utimstr(IRTime) << endl;
  }

  PMU_auto_register("Reading IR data");
  if (IR.readVolume()) {
    string Error(IR.getErrStr());
    cerr <<  Error << endl;
    return -1;
  }                                     


  MdvxField *IRField, *ModelField;

  if (P->IRFieldNum < 0){ // Use field name, not number.
    IRField = IR.getFieldByName(P->IRFieldName);
    if (IRField == NULL){
      cerr << "No IR field name " <<  P->IRFieldName << endl;
      exit(-1);
    } 
  } else { // Use field number, not name.
    IRField = IR.getFieldByNum(P->IRFieldNum);
    if (IRField == NULL){
      cerr << "No IR field number " <<  P->IRFieldNum << endl;
      exit(-1);
    } 
  }

  const Mdvx::field_header_t &IRFhdr = IRField->getFieldHeader();  
  const Mdvx::master_header_t &IRMhdr = IR.getMasterHeader();

  MdvxProj IR_Proj(IRMhdr,IRFhdr);

  // compute IR extents in lat/lon

  double minx = IRFhdr.grid_minx - IRFhdr.grid_dx * 0.5;
  double maxx = IRFhdr.grid_minx + IRFhdr.grid_dx * (IRFhdr.nx + 0.5);
  double miny = IRFhdr.grid_miny - IRFhdr.grid_dy * 0.5;
  double maxy = IRFhdr.grid_miny + IRFhdr.grid_dy * (IRFhdr.ny + 0.5);
  if (P->Debug){
    cerr << "Read model:" << endl;
    cerr << "minx: " << minx << "  maxx: " << maxx << endl;
    cerr << "miny: " << miny << "  maxy: " << maxy << endl;
  }
  
  double minLat, minLon, maxLat, maxLon;
  IR_Proj.xy2latlon(minx, miny, minLat, minLon);
  IR_Proj.xy2latlon(maxx, maxy, maxLat, maxLon);
  if (P->Debug){
    cerr << "minLat: " << minLat << "  maxLat: " << maxLat << endl;
    cerr << "minLon: " << minLon << "  maxLon: " << maxLon << endl;
  }

  //
  // Read the model data.
  //
  Model.setReadTime(Mdvx::READ_CLOSEST,
		    P->ModelURL,
		    P->MaxTimeMismatch,
		    IRTime );

  Model.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  Model.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  Model.setReadHorizLimits(minLat, minLon, maxLat, maxLon);

  if (P->Debug){
    cerr << "Reading Model data with tolerance " << P->MaxTimeMismatch << endl;
  }
  if (Model.readVolume()) {
    string Error(Model.getErrStr());
    cerr <<  Error << endl;
    return -1;
  }


  ModelField = Model.getFieldByName(P->ModelFieldName);
  if (ModelField == NULL){
    cerr << "No IR field name " <<  P->ModelFieldName << endl;
    exit(-1);
  } 


  const Mdvx::field_header_t &ModelFhdr = ModelField->getFieldHeader();  
  const Mdvx::master_header_t &ModelMhdr = Model.getMasterHeader();

  MdvxProj ModelProj(ModelMhdr, ModelFhdr);


  //
  // Do the processing.
  //

  PMU_auto_register("Processing");


  int PlaneSize;
  //if (P->DoRemap){
  //  PlaneSize = P->grid_nx*P->grid_ny;
  //} else {

  PlaneSize = IRFhdr.nx *IRFhdr.ny;
     // }

  float *outData = (float *)malloc(sizeof(float)*PlaneSize);
  if (outData == NULL){
    cerr << "Allocation failed." << endl;
    exit(-1);
  }


  float *ModelData = (float *)ModelField->getVol();
  float *IRData = (float *)IRField->getVol();
  float Bad = -1.0;

  for (int ix=0; ix < IRFhdr.nx; ix++){
    for (int iy=0; iy < IRFhdr.ny; iy++){

      int point = IRFhdr.nx * iy + ix;

      PMU_auto_register("Processing data");

      if (( IRData[point] == IRFhdr.missing_data_value) ||
	  ( IRData[point] == IRFhdr.bad_data_value)){
	outData[point] = Bad; // The bad value.
      } else {

	if (IRData[point] > P->MaxIRTemperature){
	  outData[point] = Bad;
	} else {

	  outData[point] = (float) ModelFhdr.nz-1; // Initial value.
	  // This will not be overwritten if the clouds are very cold.

	  int NumGoodModelTemps = 0;

	  for (int Plane = ModelFhdr.nz - 1; Plane > -1; Plane--){
	    //
	    // Get the lat,lon of the IR point.
	    //
	    double lat,lon;
	    IR_Proj.xyIndex2latlon(ix,iy,lat,lon);

	    //
	    // Get the x,y for the  Model point.
	    //
	    int mx,my;
	    if (ModelProj.latlon2xyIndex(lat,lon,mx,my)){
	      mx = -1; // Model is outside of grid.
	    }


	    int index = Plane * ModelFhdr.nx*ModelFhdr.ny +
	      ModelFhdr.nx* my + mx;

	    if ((ModelData[index] != ModelFhdr.missing_data_value) &&
		(ModelData[index] != ModelFhdr.bad_data_value) &&
		(mx > -1)){
	      NumGoodModelTemps++;
	      if (ModelData[index] <= IRData[point]) outData[point] = Plane;
	    } else {
	      if (NumGoodModelTemps == 0) outData[point]=Bad;
	    }
	  }
	}
      }
    }
  }

  //
  // Translate data from plane number to vlevel type.
  //

  const Mdvx::vlevel_header_t Vlev = ModelField->getVlevelHeader();

  for (int i=0; i<PlaneSize; i++){

    PMU_auto_register("Converting to height ...");

    if (outData[i] != Bad){

      int k = (int)rint(outData[i]);
      if (k == ModelFhdr.nz-1){
	outData[i] = P->MaxHeight;
      } else {
	outData[i] = Vlev.level[k+1]*P->Scale;
      }
      if (outData[i] < P->MinHeight) outData[i] = Bad;
    }
  }

  if (P->Debug){
    int BadNum = 0;
    int first=1;
    float min=0,max=0;
    for (int j=0; j < PlaneSize; j++){
      if (outData[j] != Bad){
	if (first){
	  first=0;
	  min=outData[j]; max=min;
	} else {
	  if (outData[j] < min) min = outData[j];
	  if (outData[j] > max) max = outData[j];
	}

      } else {
	BadNum++;
      }
    }
    cerr << "Data range from " << min << " to " << max << endl;
    cerr << BadNum << " bad data points." << endl;
  }


  PMU_auto_register("Writing output");
  
  Params::projection_t OutProj;
  switch(IRFhdr.proj_type){
    
  case Mdvx::PROJ_FLAT :
    OutProj = Params::FLAT;
    break;
    
  case Mdvx::PROJ_LATLON :
    OutProj = Params::LATLON;
    break;
    
  case Mdvx::PROJ_LAMBERT_CONF :
    OutProj = Params::LAMBERT;
    break;

  default :
    cerr << "Bad projection type. " << endl;
    exit(-1);
    break;

  }


  Output O(IRTime, P->DataDuration, 1,
	   IRFhdr.nx, IRFhdr.ny, 1,
	   IRFhdr.proj_origin_lon, IRFhdr.proj_origin_lat, 0.0,
	   "Cloud height", "Cloud height", "IR2Height",
	   OutProj);

  O.AddField("IRHeight", "IRHeight", P->HeightUnits,
	     0, 1, outData, Bad, Bad,
	     IRFhdr.grid_dx, IRFhdr.grid_dy, 0.0,
	     IRFhdr.grid_minx, IRFhdr.grid_miny, 0.0, 0);
 
  O.write(P->OutputURL);
    

  free (outData);

  return 0;

}


