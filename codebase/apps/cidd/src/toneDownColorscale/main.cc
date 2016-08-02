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
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctype.h>

int main(int argc, char *argv[]){

  if (argc < 3){
    fprintf(stderr,"USAGE : toneDownColorscale inFile factor\n");
    fprintf(stderr,"EXAMPLE : toneDownColorscale terrain.colors 0.6\n");
    exit(0);
  }

  double factor = atof(argv[2]);

  FILE *ifp = fopen(argv[1], "r");
  if (ifp == NULL){
    fprintf(stderr,"Failed to open %s\n", argv[1]);
    exit(0);
  }

  char Line[1024];

  while (NULL != fgets(Line, 1024, ifp)){

    double min, max;
    char color[256];

    if (3 == sscanf(Line, "%lf %lf %s",
		    &min, &max, color)){

      unsigned red=0, blue=0, green=0;

      if (color[0] == '#'){

	if (3 != sscanf(color+1,"%2x%2x%2x", &red, &blue, &green)){
	  fprintf(stderr, "Could not decode %s\n", color);
	  exit(-1);
	}
      } else {
	
	for (unsigned int k=0; k < strlen(color); k++){
	  color[k] = (unsigned char)tolower(int(color[k]));
	}

	FILE *xfp = fopen("/usr/X11R6/lib/X11/rgb.txt", "r");
	if (xfp == NULL){
	  fprintf(stderr,"Failed to open /usr/X11R6/lib/X11/rgb.txt\n");
	  exit(0);
	}
	
	while (NULL != fgets(Line, 1024, xfp)){
	  for (unsigned int k=0; k < strlen(Line); k++){
	    Line[k] = (unsigned char)tolower(int(Line[k]));
	  }

	  if (NULL != strstr(Line, color)){
	    int rTmp, gTmp, bTmp;
	    char xc[32];
	    if (4 == sscanf(Line,"%d %d %d %s", &rTmp, &bTmp, &gTmp, xc)){
	      if (!(strcmp(xc, color))){
		red = rTmp; green = gTmp; blue = bTmp;
		break;
	      }
	    }
	  }
	}
	fclose(xfp);
      }

      red = (int)rint(double(red)*factor);
      blue = (int)rint(double(blue)*factor);
      green = (int)rint(double(green)*factor);

      fprintf(stdout,"%g\t%g\t#%02x%02x%02x\n", min, max, red, blue, green);

    }

  }


  fclose(ifp);


}

