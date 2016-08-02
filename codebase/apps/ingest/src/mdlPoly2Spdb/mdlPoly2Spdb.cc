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
#include <toolsa/umisc.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>

#include "mdlPoly2Spdb.hh"

using namespace std;


//
// Constructor and destructor do nothing.
//
mdlPoly2Spdb::mdlPoly2Spdb(){
  return;
}

//
// Destructor.
//
mdlPoly2Spdb::~mdlPoly2Spdb(){
  return;
}

void mdlPoly2Spdb::ProcFile(char *FilePath, Params *P){


  FILE *fp = fopen(FilePath, "r");
  if (fp == NULL){
    fprintf(stderr,"Could not open %s\n", FilePath);
    return;
  }
 
  char Line[1024];

  while(NULL!=fgets(Line, 1024, fp)){
    //
    // Read until we get a line that starts with the pound sign,
    // then start reading the boundary after that.
    //
    if (strncmp(Line, "#----", strlen("#----"))) continue;

    if (NULL == fgets(Line, 1024, fp)){
      fprintf(stderr,"No boundary ID found!\n");
      fclose(fp);
      return;
    }
    int ID;
    if (1 != sscanf(Line, "%d", &ID)){
      fprintf(stderr,"Failed to decode ID from ", Line);
      fclose(fp);
      return;
    }

    if (NULL == fgets(Line, 1024, fp)){
      fprintf(stderr,"No time found!\n");
      fclose(fp);
      return;
    }
    date_time_t T;
    if ( 7 != sscanf(Line, "%d/%d/%d %d:%d:%d %ld",
		     &T.year, &T.month, &T.day, &T.hour, &T.min, &T.sec,
		     &T.unix_time)){
      fprintf(stderr,"Failed to decode time from ", Line);
      fclose(fp);
      return;
    }

    if (NULL == fgets(Line, 1024, fp)){
      fprintf(stderr,"No number of points line found!\n");
      fclose(fp);
      return;
    }
    int numPoints;
    if (1 != sscanf(Line, "%d", &numPoints)){
      fprintf(stderr,"Failed to decode number of points from ", Line);
      fclose(fp);
      return;
    }

    double *lat=(double *) malloc(numPoints*sizeof(double));
    double *lon=(double *) malloc(numPoints*sizeof(double));

    if ((lat == NULL) || (lon == NULL)){
      fprintf(stderr,"Malloc failed!\n"); // unlikely
      exit(-1);
    }

    for(int ip=0; ip < numPoints; ip++){

      if (NULL == fgets(Line, 1024, fp)){
	fprintf(stderr,"Boundary entry %d not found!\n", ip+1);
	fclose(fp);
	free(lat); free(lon);
	return;
      }
      if (2 != sscanf(Line, "%lf %lf", 
		      lat+ip, lon+ip)){
	fprintf(stderr,"Failed to decode %d point from ", ip+1, Line);
	fclose(fp);
	free(lat); free(lon);
	return;
      }
      
    }

    if (NULL == fgets(Line, 1024, fp)){
      fprintf(stderr,"No weight found!\n");
      fclose(fp);
      return;
    }
    double weight;
    if (1 != sscanf(Line, "%lf", &weight)){
      fprintf(stderr,"Failed to decode weight from ", Line);
      fclose(fp);
      return;
    }

    _S.WriteOut(P, numPoints, lat, lon, T.unix_time, ID, weight);

    free(lat); free(lon);

  }

  fclose(fp);
  return;


}

