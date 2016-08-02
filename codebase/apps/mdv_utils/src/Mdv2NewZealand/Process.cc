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
  // Set up for the new data.
  //
  DsMdvx New;


  New.setDebug( P->Debug);

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_INT8);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);


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
  // Set up the existing data - which we will have to write if
  // it does not exist.
  //
  

  //
  // Set up the output.
  //
  Mdvx::master_header_t InMhdr = New.getMasterHeader();

  //
  // Commence loop through fields.
  //
  for (int i=0; i< P->InFieldName_n; i++){
    //
    // Get the desired field.
    //
    MdvxField *InField;

    if (!(strncmp(P->_InFieldName[i],"#",1))){
      int Fnum = atoi(P->_InFieldName[i] + 1);
      InField = New.getFieldByNum( Fnum ); 
    } else {
      InField = New.getFieldByName( P->_InFieldName[i] );
    }

    if (InField == NULL){
      cerr << "New field " << P->_InFieldName[i] << " not found." << endl;
      return -1;
    }

    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

    //
    // Write the header for this field.
    //
    date_time_t D;
    D.unix_time = T;
    uconvert_from_utime( &D );

    char HeaderFile[MAX_PATH_LEN];
    sprintf(HeaderFile,"%s/%s_%d%02d%02d_%02d%02d%02d.hdr",
	    P->OutDir,
	    InFhdr.field_name,
	    D.year, D.month, D.day,
	    D.hour, D.min, D.sec);

    char DataFile[MAX_PATH_LEN];
    char FullDataFile[MAX_PATH_LEN];

    sprintf(DataFile,"%s_%d%02d%02d_%02d%02d%02d.bytes",
	    InFhdr.field_name,
	    D.year, D.month, D.day,
	    D.hour, D.min, D.sec);
  
    sprintf(FullDataFile,"%s/%s_%d%02d%02d_%02d%02d%02d.bytes",
	    P->OutDir,
	    InFhdr.field_name,
	    D.year, D.month, D.day,
	    D.hour, D.min, D.sec);
    //
    // Make the path.
    //
    Path A(HeaderFile);
    if (A.makeDirRecurse()){
      fprintf(stderr,"Failed to make path for %s\n", HeaderFile);
      exit(-1);
    }

    FILE *hfp = fopen(HeaderFile,"wt");
    if (hfp == NULL){
      cerr << "Failed to open " << HeaderFile << endl;
      exit(-1);
    }

    fprintf(hfp,"DATE_TIME = %d/%02d/%02d %02d:%02d:%02d GMT;\n",
	    D.year, D.month, D.day,
	    D.hour, D.min, D.sec);

    fprintf(hfp,"NX NY NZ = %d %d %d;\n",
	    InFhdr.nx, InFhdr.ny, InFhdr.nz);

    fprintf(hfp,"DX DY = %g %g;\n",
	    InFhdr.grid_dx, InFhdr.grid_dy);

    
    fprintf(hfp,"ZLEVELS = ");
    for (int k=0; k < InFhdr.nz; k++){
      fprintf(hfp,"%g  ",InVhdr.level[k]);
    }
    fprintf(hfp,";\n");

    fprintf(hfp,"FIELD = %s;\n",InFhdr.field_name);
    fprintf(hfp,"UNITS = %s;\n",InFhdr.units);

    fprintf(hfp,"SCALE = %g;\n", InFhdr.scale);
    fprintf(hfp,"BIAS = %g;\n", InFhdr.bias);
    fprintf(hfp,"BAD_DATA = %g;\n", InFhdr.bad_data_value);
    fprintf(hfp,"MISSING_DATA = %g;\n", InFhdr.missing_data_value);

    fprintf(hfp,"ORIGIN_LONGITUDE = %g;\n",InFhdr.proj_origin_lon);
    fprintf(hfp,"ORIGIN_LATITUDE = %g;\n",InFhdr.proj_origin_lat);

    fprintf(hfp,"MINX, MINY, MINZ = %g %g %g;\n",
	    InFhdr.grid_minx,
	    InFhdr.grid_miny,
	    InFhdr.grid_minz);

    fprintf(hfp,"PROJECTION_TYPE = ");

    switch (InFhdr.proj_type){

    case Mdvx::PROJ_NATIVE :
       fprintf(hfp,"NATIVE");
       break;

    case Mdvx::PROJ_LATLON :
      fprintf(hfp,"LATLON");
      break;

    case Mdvx::PROJ_ARTCC :
      fprintf(hfp,"ARTCC");
       break;

    case Mdvx::PROJ_OBLIQUE_STEREO :
      fprintf(hfp,"STEREOGRAPHIC");
       break;

    case Mdvx::PROJ_LAMBERT_CONF :
      fprintf(hfp,"LAMBERT_CONF");
       break;

    case Mdvx::PROJ_MERCATOR :
      fprintf(hfp,"MERCATOR");
       break;

    case Mdvx::PROJ_POLAR_STEREO :
      fprintf(hfp,"POLAR_STEREO");
       break;

    case Mdvx::PROJ_POLAR_ST_ELLIP :
      fprintf(hfp,"POLAR_ST_ELLIP");
       break;

    case Mdvx::PROJ_CYL_EQUIDIST :
      fprintf(hfp,"EQUIDIST");
       break;

    case Mdvx::PROJ_FLAT :
      fprintf(hfp,"FLAT");
       break;

    case Mdvx::PROJ_POLAR_RADAR :   
      fprintf(hfp,"POLAR_RADAR");
       break;

    case Mdvx::PROJ_RADIAL :   
      fprintf(hfp,"RADIAL");
       break;

    case Mdvx::PROJ_VSECTION :
      fprintf(hfp,"VSECTION");
       break;
       
    case Mdvx::PROJ_UNKNOWN :
      fprintf(hfp,"UNKNOWN");
      break;

    default :
      fprintf(hfp,"UNKNOWN (CODE %d)", InFhdr.proj_type);
      break;

    }
    fprintf(hfp,";\n");

    fprintf(hfp,"PROJECTION_PARAMETERS = ");

    for (int i=0; i<MDV_MAX_PROJ_PARAMS; i++){
      fprintf(hfp,"%g ", InFhdr.proj_param[i]);
    }
    fprintf(hfp,";\n");

    fprintf(hfp,"DATA_FILE = \"%s\";\n\n", DataFile);

    fprintf(hfp,"Data file is a simple byte dump. Physical values are\n");
    fprintf(hfp,"computed by phys = scale * byte + bias\n");
    fprintf(hfp,"unless the byte is the bad or missing data value\n");
    fprintf(hfp,"in which case there is no data at that grid point.\n\n");
    fprintf(hfp,"Data start at the lower left (south west) corner and\n");
    fprintf(hfp,"raster up, with X changing fastest in memory, then Y\n");
    fprintf(hfp,"finally Z. Geographical location of the data depends on\n");
    fprintf(hfp,"the projection parameters. If the projection type\n");
    fprintf(hfp,"is LATLON then the data are on a lat/lon grid,\n");
    fprintf(hfp,"and the data start at MINX by MINY with DX and DY being\n");
    fprintf(hfp,"in degrees. For FLAT projection type, the data are\n");
    fprintf(hfp,"a flat earth projection, starting at MINX by MINY Km\n");
    fprintf(hfp,"from the origin and DX and DY are in Km.\n");
    fprintf(hfp,"Other projection types have different meanings.\n");
    fprintf(hfp,"Niles Oien oien@ucar.edu November 2001\n");

    fclose(hfp);

    unsigned char *InData = (unsigned char *) InField->getVol();

    FILE *dfp = fopen(FullDataFile,"wb");
    if (dfp == NULL){
      fprintf(stderr,"Failed to create %s\n", DataFile);
      exit(-1);
    }

    fwrite(InData, sizeof(unsigned char), 
	   InFhdr.nx * InFhdr.ny * InFhdr.nz,  dfp);

    fclose(dfp);

    
  } // End of loop through the fields.

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










