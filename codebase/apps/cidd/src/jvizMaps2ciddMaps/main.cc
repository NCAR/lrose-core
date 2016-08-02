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

#include <vector>
#include <cstdio>

using namespace std;

int main(int argc, char *argv[]){

  vector <double> lats;
  vector <double> lons;

  if (argc < 3){
    fprintf(stderr, 
	    "USAGE : %s <jvizMapFile> <ciddMapFile> [matlab file]\n",
	    argv[0]);
    fprintf(stderr, "maps should be simple polylines\n");
    return -1;
  }

  int writingMatlab = 0;
  if (argc > 3) writingMatlab = 1;

  FILE *ifp = fopen(argv[1], "r");
  if (ifp == NULL){
    fprintf(stderr, "Failed to open %s\n", argv[1]);
    return -1;
  }

  FILE *ofp = fopen(argv[2], "w");
  if (ofp == NULL){
    fprintf(stderr, "Failed to create %s\n", argv[2]);
    return -1;
  }

  FILE *mfp;
  if (writingMatlab){
    mfp = fopen(argv[3], "w");
    if (mfp == NULL){
    fprintf(stderr, "Failed to create matlab file %s\n", argv[3]);
    writingMatlab = 0;
    }
  }

  fprintf(ofp, "MAP_NAME : %s\n", argv[1]);

  char Line[1024];

  int polyCount = 0;

  while (NULL != fgets(Line, 1024, ifp)){

    double lat, lon;
    if (2==sscanf(Line,"%lf, %lf", &lon, &lat)){
      lons.push_back(lon); lats.push_back(lat);
    } else {
      if (lats.size() > 0){

	fprintf(ofp, "POLYLINE line_%03d %d\n", polyCount, lats.size());
	polyCount++;
	for (unsigned i=0; i < lats.size(); i++){
	  fprintf(ofp,"%g %g\n", lats[i], lons[i]);
	  if (writingMatlab) 
	    fprintf(mfp,"%g %g\n", lats[i], lons[i]);
	}
	lats.clear(); lons.clear();
	fprintf(mfp,"%g %g\n", -999.0, -999.0);
      }
    }

  }

  fclose(ifp); fclose(ofp);
  if (writingMatlab) fclose(mfp);

  fprintf(stdout, "%d line segments in %s\n", polyCount, argv[2]);

  return 0;

}

