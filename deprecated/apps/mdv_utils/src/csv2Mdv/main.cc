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

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "Params.hh"


int main(int argc, char *argv[]){
  
  Params params;
  
  if (params.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }  
  
  for (int i=0; i < params.mdvInfo_n; i++){
    
    fl32 max=0.0, min=0.0, x, y;
    int numRead = 0;
    int first = 1;
    
    fl32 *data = (fl32 *)malloc(sizeof(fl32)*
				params._mdvInfo[i].nx *
				params._mdvInfo[i].ny);
    
    if (data == NULL){
      fprintf(stderr,"Malloc!\n");
      return -1;
    }
    
    FILE *fp = fopen(params._mdvInfo[i].inFile, "r");
    if (fp == NULL){
      fprintf(stderr, "Failed to open input file %s\n", 
	      params._mdvInfo[i].inFile );
      continue;
    }


    char Line[1024];
    int lineNum = 0;
    int maxLineNum = 0;
    while (
	   (numRead != params._mdvInfo[i].nx * params._mdvInfo[i].ny) &&
	   (NULL!=fgets(Line, 1024, fp))
	   ){

      lineNum++; // We read a line.

      if (3 == sscanf(Line,"%f  %f %f",
		      &x, &y, &data[numRead])){

	if (params.debug){
	  fprintf(stderr,"%lf %lf %lf %s",
		  x, y, data[numRead], Line);
	}

	// fprintf(stderr," %d : %s", numRead+1, Line);

	data[numRead] *= params._mdvInfo[i].factor;

	if (first){
	  first = 0;
	  min = max = data[numRead];
	  maxLineNum = lineNum;
	} else {
	  if (data[numRead] < min) min = data[numRead];
	  if (data[numRead] > max){
	    max = data[numRead];   
	    maxLineNum = lineNum;
	  }
	}
  	numRead++;
      }
    }
  
    fclose(fp);

    fprintf(stdout,"For %s, %d points from %g to %g, max on line %d\n",
	    params._mdvInfo[i].inFile, numRead, min, max, maxLineNum);

    if (numRead != params._mdvInfo[i].nx * params._mdvInfo[i].ny){
      fprintf(stderr, "ERROR - found %d points found in %s not %d\n",
	      numRead, params._mdvInfo[i].inFile,
	      params._mdvInfo[i].nx * params._mdvInfo[i].ny);
      continue;
    }

    //
    // Now, output MDV file.
    //
    Mdvx::master_header_t mhdr;
    Mdvx::field_header_t fhdr;
    Mdvx::vlevel_header_t vhdr;

    memset(&mhdr, 0, sizeof(mhdr));
    memset(&fhdr, 0, sizeof(fhdr));
    memset(&vhdr, 0, sizeof(vhdr));

    sprintf(mhdr.data_set_info, "From ACSII file");
    sprintf(mhdr.data_set_name, "%s", params._mdvInfo[i].fieldName);
    sprintf(mhdr.data_set_source, "%s", "csv2Mdv");

    mhdr.time_centroid = time(NULL);
    mhdr.time_expire = mhdr.time_gen = mhdr.time_end = 
      mhdr.time_begin =  mhdr.time_centroid;
    
    mhdr.data_dimension =2;

    mhdr.data_collection_type = Mdvx::DATA_FORECAST;

    mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;


    fhdr.forecast_time = mhdr.time_centroid;
    fhdr.nx = params._mdvInfo[i].nx;
    fhdr.ny = params._mdvInfo[i].ny;
    fhdr.nz = 1;
    fhdr.proj_rotation = params._mdvInfo[i].rotation;
    fhdr.proj_param[0] = params._mdvInfo[i].rotation;

    fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    fhdr.scaling_type = Mdvx::SCALING_DYNAMIC;
    fhdr.proj_type = Mdvx::PROJ_FLAT;
    fhdr.data_element_nbytes = sizeof(fl32);
    fhdr.volume_size = sizeof(fl32)*fhdr.nx*fhdr.ny;
    fhdr.compression_type = Mdvx::COMPRESSION_NONE;
    fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    fhdr.data_dimension = 2;
    fhdr.proj_origin_lat = params._mdvInfo[i].lat;
    fhdr.proj_origin_lon = params._mdvInfo[i].lon;
    fhdr.grid_dx = params._mdvInfo[i].dx/1000.0;
    fhdr.grid_dy = params._mdvInfo[i].dy/1000.0;

    fhdr.grid_minx = -fhdr.nx*fhdr.grid_dx/2.0 + fhdr.grid_dx/2.0;
    fhdr.grid_miny = -fhdr.ny*fhdr.grid_dy/2.0 + fhdr.grid_dy/2.0;

    fhdr.grid_minx += params._mdvInfo[i].tx;
    fhdr.grid_miny += params._mdvInfo[i].ty;

    sprintf(fhdr.field_name,"%s", params._mdvInfo[i].fieldName);
    sprintf(fhdr.field_name_long,"%s", params._mdvInfo[i].fieldName);
    sprintf(fhdr.transform,"%s", "none");
    sprintf(fhdr.units,"%s", params._mdvInfo[i].units);

    vhdr.type[0] =  Mdvx::VERT_TYPE_SURFACE;
    vhdr.level[0] = 0.0;

    DsMdvx outMdvx;
    outMdvx.setMasterHeader( mhdr );
    outMdvx.clearFields();

    MdvxField *fld = new MdvxField(fhdr, vhdr, data);

    outMdvx.addField( fld );

    
    if (outMdvx.writeToPath(params._mdvInfo[i].outFile)) {
      cerr << "Failed to wite to " << params._mdvInfo[i].outFile << endl;
      cerr << outMdvx.getErrStr() << endl;
    }      


    free(data);

  }

  return 0;

}
