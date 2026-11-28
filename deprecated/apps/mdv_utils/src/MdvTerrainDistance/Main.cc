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

#include <toolsa/str.h>
#include <toolsa/port.h>
#include <signal.h>

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pjg_flat.h>

void usage();

int main(int argc, char **argv)

{

  for (int i=1; i < argc; i++){
    if (!(strcmp(argv[i], "-h"))){
      usage(); exit(0);
    }
  }

  if (argc < 4){
    usage(); exit(0);
  }

  double latC = atof(argv[1]);
  double lonC = atof(argv[2]);
  double dist = atof(argv[3]);


  double lllon,lllat,urlon,urlat;

  PJGLatLonPlusRTheta(latC, lonC, dist, 225.0, &lllat, &lllon);
  PJGLatLonPlusRTheta(latC, lonC, dist, 45.0,  &urlat, &urlon);
 
  int Nx = (int)ceil((urlon-lllon)/0.00833333333333);
  int Ny = (int)ceil((urlat-lllat)/0.00833333333333);

  fprintf(stderr,"%g %g %d %d\n",
	  lllat, lllon, Nx, Ny);



  return 0;
    
}

void usage(){

  fprintf(stderr,"MdvTerrainDistance is a small program to calulate\n");
  fprintf(stderr,"the distances needed to generate a terrain file. The\n");
  fprintf(stderr,"usage is :\n\n");
  fprintf(stderr,"MdvTerrainDistance lat lon dist\n");
  fprintf(stderr,"\n");
  fprintf(stderr,"Where : \n");
  fprintf(stderr,"\n");
  fprintf(stderr,"lat is the latitude the terrain will be centered at,\n");
  fprintf(stderr,"lon is the longitude the terrain will be centered at, and\n");
  fprintf(stderr,"Dist is the extent in Km that you want to cover.\n");
  fprintf(stderr,"Setting dist to 500Km means that a 1000Km square will be covered.\n\n");
  fprintf(stderr,"The program then prints four numbers, these being the\n");
  fprintf(stderr,"latitude and longitude of the lower left corner of\n");
  fprintf(stderr,"the domain, followed by the number of points in X and Y\n");
  fprintf(stderr,"needed to reach the upper right corner, assuming the\n");
  fprintf(stderr,"maximum resolution of the USGS dataset.\n");
  fprintf(stderr,"\n");
  fprintf(stderr,"These values can then be used to run WorldTerrain on\n");
  fprintf(stderr,"The USGS data files and generate a latlon projection\n");
  fprintf(stderr,"terrain file. MdvResampleTerrain can then be used to\n");
  fprintf(stderr,"generate a file on the flat earth projection.\n");
  fprintf(stderr,"\n");
  fprintf(stderr,"Niles Oien 2002.\n");
  fprintf(stderr,"\n");

  return;

}
