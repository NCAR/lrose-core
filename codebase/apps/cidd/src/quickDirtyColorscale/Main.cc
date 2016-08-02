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
//
// This is a small program that generates a quick-and-dirty colorscale
// for use in CIDD. The resulting colorscale is probably OK for use
// by engineers diagnosing data woes but is probably not suitable for
// more formal use.
//
// Niles Oien April 2005.
//
#include <cstdio>
#include <cstdlib>

int main(int argc, char *argv[]){
  //
  // Make sure we have enough command line args.
  //
  if (argc < 3){
    fprintf(stderr,"USAGE : quickDirtyColorscale [minVal maxVal]\n");
    fprintf(stderr,"Output is printed to stdout for re-direction.\n");
    exit(-1);
  }
  //
  // Parse out the min and max.
  //
  double min = atof( argv[1] ); double max = atof( argv[2] ); 

  if (!(max > min)){
    fprintf(stderr,"Max is not greater than min, I cannot cope.\n");
    exit(-1);
  }
  
  const int numSteps = 20;
  double delta = (max - min ) / numSteps;
     
  for (int i=0; i < numSteps; i++){
    double startVal = min + i * delta;
    double endVal = min + (i+1) * delta;

    fprintf(stdout,"%g\t%g\t", startVal, endVal);

    if (i == numSteps-1) {
      fprintf(stdout,"white\n");
    } else {
      fprintf(stdout,"gray%d\n", (i+1)*5);
    }

  }


  return 0;

}
