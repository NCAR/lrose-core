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

#include "ReadDEM.hh"
#include "Output.hh"
#include "Params.hh"
#include "Args.hh"

#include <cstdio>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pjg.h>
#include <toolsa/pjg_flat.h>
using namespace std;

int main(int argc, char *argv[])
{

  // set programe name

  char *_progName;

  _progName = STRdup("ReadDEM");
  ucopyright(_progName);

  // get command line args

  Args *_args = new Args(argc, argv, _progName);

  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    exit(-1);
  }

  // get TDRP params

  Params *params = new Params();

    if (params->loadFromArgs(argc,argv,
			      _args->override.list,
			      &_args->paramsFilePath)){
      fprintf(stderr,"Problem with TDRP parameters.\n");
      exit(-1);
    }


  ReadDEM *H = new ReadDEM(params->BaseName,params->debug);

  if (H->Error){
    fprintf(stderr,"Problem reading DEM from %s\n",params->BaseName);
    exit(-1);
  }

  

  float *data,min,max;
  int NumOut = 0;

  data = (float *)umalloc(params->Nx * params->Ny*sizeof(float));
  float lat,lon;

  float Bad=-100000.0;

  min=1000; max=-1000.0;

  float x0,y0;

  if (params->flat){
    x0=-params->Nx*params->dx/2.0; 
    y0=-params->Ny*params->dy/2.0;
  } else {
    x0=params->lon_origin; 
    y0=params->lat_origin;
  }

PJGstruct prjStruct;
prjStruct = *PJGs_lc2_init(25.0, -95.0, 25.000000, 25.05);

  float x,y;
  for(int i=0; i<params->Nx; i++){
    for(int j=0; j<params->Ny; j++){

// if (i != 45 || j != 61) {
//   continue;
// }

/*
// Todo: Take this out after initial testing.
if (i < 40 || i > 60 || j < 50 || j > 80) {
  data[i+j * params->Nx] = 0.0f;
  continue;
}
*/
      
//       if (!(params->flat)){
// 
// 	lon = params->lon_origin + i * params->dx;
// 	lat = params->lat_origin + j * params->dy;
// 
//       } else {

        double lat2,lon2;
        double nextLat, nextLon;
        float dLat, dLon;

        x = x0 + i * params->dx; 
        y = y0 + j * params->dy;

        // // Add a weird offset to fix registration problem.
        // //  -- Pick a point to the NE a bit to use at this point.
        // // 
        // x += 60.952497;
        // y += 60.952497;

// 
// 
// 	PJGLatLonPlusDxDy(double(params->lat_origin), 
// 			  double(params->lon_origin),
// 			  double(x), double(y),
// 			  &lat2, &lon2);   
// 
// 
        PJGs_lc2_xy2latlon(&prjStruct, (double) x, (double) y, &lat2, &lon2);
        PJGs_lc2_xy2latlon(&prjStruct, (double) (x - params->dx), (double) (y - params->dy), &nextLat, &nextLon);
        dLat = (float) (lat2 - nextLat);
        dLon = (float) (lon2 - nextLon);

        lat = (float) lat2;
        lon = (float) lon2;
//       }

      cerr << "Computing elevation for point at: "
           << x << ", " << y << " " << endl;
      // 
      // cerr << "    with lat, lon: "
      //      << lat2 << ", " << lon2 << endl;
      // 
      // cerr << "    with nextLat, nextLon: "
      //      << nextLat << ", " << nextLon << endl;

      data[i+j * params->Nx]=H->MaxElevation(lat,lon,Bad, dLon, dLat);
      // data[i+j * params->Nx]=H->Elevation(lat,lon,Bad);

      // cerr << "    " << data[i+j * params->Nx] << endl;

      if (data[i+j * params->Nx] != Bad){
	if (data[i+j * params->Nx] < min) min = data[i+j * params->Nx];
	if (data[i+j * params->Nx] > max) max = data[i+j * params->Nx];
      }

      // NumOut=0;

      if (H->OutsideGrid) {
	if (!(params->AllowOutsideDEM)){
	  fprintf(stdout,"Point (%g,%g) outside grid.\n",lat,lon);
	  exit(-1);
	} else {
	  NumOut++;
	}
      }

    }
  }


  delete H;

  fprintf(stdout,"Elevation from %gm to %gm\n",min,max);
  if (NumOut) fprintf(stdout,"%d points were outside DEM grid.\n",NumOut);


  date_time_t DataTime;
  ugmtime(&DataTime);

  if (params->SubstituteMissing){
    for (int k=0;k<params->Nx*params->Ny; k++){
      if (data[k]==Bad) data[k]=params->SubstituteMissingValue;
    }
  }

  Output *Q = new Output(DataTime,
			 3600,
			 1,
			 params->Nx,
			 params->Ny,
			 1,
			 params->lon_origin,
			 params->lat_origin,
			 0.0,
			 "Elevation",
			 "Terrain",
			 "DEM",
			 params->flat);

  Q->AddField("Terrain", "Elevation",
	      "m",0, 1,data, 0, 0,
	      params->dx,params->dy,0.5,
	      x0, y0,
	      0.5,5,Bad);
  
  sprintf(Q->outpath,"%s",params->OutName);
  
  delete Q; // Destructor does the mdv file writing.
                        
  free(data);
  
}


