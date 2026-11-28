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

#include <toolsa/umisc.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>

#include "Params.hh"


int main(int argc, char *argv[]){


  Params P;

  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }

  //
  // Allocate memory.
  //
  float *data = (float *) malloc( P.nx*P.ny*sizeof(float) );
  if (data==NULL){
    fprintf(stderr,"Malloc failed.\n");
    exit(-1);
  }
  //
  // Parse out the start and end times.
  //
  date_time_t T;

  time_t timeStart;
  if (6 != sscanf(P.startTime, "%d %d %d %d %d %d",
		  &T.year, &T.month, &T.day,
		  &T.hour, &T.min, &T.sec)){
    fprintf(stderr,"Failed to parse start time from %s\n", P.startTime);
    exit(-1);
  }
  uconvert_to_utime( &T );
  timeStart = T.unix_time;

  time_t timeEnd;
  if (6 != sscanf(P.endTime, "%d %d %d %d %d %d",
		  &T.year, &T.month, &T.day,
		  &T.hour, &T.min, &T.sec)){
    fprintf(stderr,"Failed to parse end time from %s\n", P.endTime);
    exit(-1);
  }
  uconvert_to_utime( &T );
  timeEnd = T.unix_time;
  
  time_t currentTime = timeStart;

  do{

    //
    // Run the program to write the ascii file.
    // Parameters are passed on the command line.
    //
    fprintf(stderr,"Processing at %s\n",
	    utimstr(currentTime));

    int seed = -rand();

    char com[1024];
    sprintf(com, "/bin/csh -c \"./simbetavk2d_dhs %d %d %14.6f %14.6f %14.6f %14.6f %d |& grep -v # >& a.dat\"", 
	    P.nx, P.ny, 1000.0*P.dx, 1000.0*P.dy, P.sigmaBetaOverMeanBeta, P.L0, seed);
    fprintf(stdout,"%s\n", com);
    system(com);

    //
    // Read the data in.
    //
    FILE *ifp = fopen("a.dat", "r");
    for (int k=0; k < P.nx*P.ny; k++){
      if (1 != fscanf(ifp, "%f", &data[k])){
	fprintf(stderr,"Read failed.\n");
	exit(-1);
      }
      data[k] = data[k] * P.scale + P.bias;
    }
    fclose(ifp);

    //
    // OK - now have data, write it out as MDV.
    //

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
    vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
    vhdr.level[0] = 0.0;
    //
    // Then set up the field header with the metadata.
    //
    fhdr.nx = P.nx;
    fhdr.ny = P.ny;
    fhdr.nz = 1;
    //
    fhdr.grid_dx = P.dx;
    fhdr.grid_dy = P.dy;
    //
    fhdr.proj_type = Mdvx::PROJ_FLAT; // This is implied for these data.
    //
    fhdr.proj_origin_lat =  P.grid_origin_lat;
    fhdr.proj_origin_lon =  P.grid_origin_lon;
    //
    //
    fhdr.grid_minx = -P.dx*P.nx/2.0 + P.dx / 2.0;
    fhdr.grid_miny = -P.dy*P.ny/2.0 + P.dy / 2.0;
    //
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
    // Set up some things in the master header.
    //
    Mhdr.data_collection_type = Mdvx::DATA_FORECAST;
    Mhdr.num_data_times = 1;
    Mhdr.data_ordering = Mdvx::ORDER_XYZ;
    Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
    Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
    Mhdr.vlevel_included = 1;
    //
    //
    sprintf(Mhdr.data_set_info,"%s","Noise");
    sprintf(Mhdr.data_set_name,"%s","Noise");
    sprintf(Mhdr.data_set_source,"%s", "Noise");
    //
    // Set the times in the master and field headers.
    //
    Mhdr.time_gen = currentTime;
    Mhdr.time_begin = currentTime;
    Mhdr.time_end = currentTime;
    Mhdr.time_expire = currentTime + P.timeStep;
    Mhdr.time_centroid = currentTime;
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
    // Complete filling out the field header with this information.
    //
    fhdr.bad_data_value = -9999.0;   fhdr.missing_data_value = -9999.0;
    //
    sprintf( fhdr.field_name_long,"%s", "noise");
    sprintf( fhdr.field_name,"%s", "noise");
    sprintf( fhdr.units,"%s", "none");
    sprintf( fhdr.transform,"%s","none");
  
    MdvxField *field;
    //
    field = new MdvxField(fhdr, vhdr, data);
    //
    if (field->convertRounded(Mdvx::ENCODING_INT16,
			      Mdvx::COMPRESSION_ZLIB)){
      fprintf(stderr, "conversion of field failed - I cannot go on.\n");
      exit(-1);
    }
  
    outMdvx.addField(field);

    if (outMdvx.writeToDir(P.OutUrl)) {
      cerr << "ERROR - Output::write" << endl;
      cerr << "  Cannot write to url: " << P.OutUrl << endl;
      cerr << outMdvx.getErrStr() << endl;
    }

    system ("\\rm -f a.dat");

    currentTime = currentTime + P.timeStep;
  } while(currentTime <= timeEnd);


  free(data);

  return 0;

}
