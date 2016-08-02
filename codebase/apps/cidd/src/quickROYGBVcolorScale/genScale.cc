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
// Small program to read the colors in colors.dat and take two command line
// arguments for the minimum and maximum physical values and generate
// a colorscale for CIDD based on that. Niles Oien.
//
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char *argv[]){

  if (argc < 3) {
    fprintf(stderr,"USAGE : genScale minVal maxVal [white]\n\n");
    fprintf(stderr,"  Specifying white means that the low end of the\n");
    fprintf(stderr,"  color scale will be white, not blue, which can be\n");
    fprintf(stderr,"  useful given that ROYGBV tends to fade to black.\n\n");
    fprintf(stderr,"  Niles Oien March 2006.\n");
    exit(-1);
  }

  double min = atof(argv[1]);
  double max = atof(argv[2]);

  double delta = (max-min)/65;

  int doWhite = 0;
  if (
      (argc > 3) &&
      (!(strcmp("white", argv[3])))
      ){
    doWhite = 1;
  }

  int whiteLevel = 50; // Start out 50% gray


  FILE *ifp = fopen("colors.dat", "r");

  if (ifp == NULL){
    fprintf(stderr,"Failed to open colors.dat, I need to run in $RAP_DIR/apps/cidd/src/quickROYGBVcolorScale\n");
    exit(-1);
  }

  char line[32];

  for(int i=0; i < 65; i++){

    if (fgets(line, 32, ifp) == NULL){
      fprintf(stderr," Problem with colors.dat\n");
    }

    if (doWhite == 0){
      fprintf(stdout, "%g\t%g\t#%s",
	      min + i * delta,
	      min + (i + 1) * delta,
	      line);
    } else {

      if (whiteLevel > 100){
	fprintf(stdout, "%g\t%g\t#%s",
		min + i * delta,
		min + (i + 1) * delta,
		line);
      } else {

	char whiteCol[32];
	if (whiteLevel == 100){
	  sprintf(whiteCol, "white");
	} else {
	  sprintf(whiteCol, "gray%d", whiteLevel);
	}
	fprintf(stdout, "%g\t%g\t%s\n",
		min + i * delta,
		min + (i + 1) * delta,
		whiteCol);
      }
      whiteLevel += 5;
    }
  }


  fclose(ifp);


}
