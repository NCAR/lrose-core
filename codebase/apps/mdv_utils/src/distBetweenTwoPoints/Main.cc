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

  if (argc < 5){
    usage(); exit(0);
  }

  double lat1 = atof(argv[1]);
  double lon1 = atof(argv[2]);
  double lat2 = atof(argv[3]);
  double lon2 = atof(argv[4]);

  double r,theta;

  PJGLatLon2RTheta(lat1, lon1, lat2, lon2, &r, &theta);

  fprintf(stderr,"Distance is %g Km at %g deg\n", r, theta);

  return 0;
    
}

void usage(){

  fprintf(stderr, "USAGE : distBetweenTwoPoints lat1 lon1 lat2 lon2\n");
  fprintf(stderr, "The distance between the two points is printed.\n");
  fprintf(stderr,"Niles Oien 2010\n");

  return;

}
