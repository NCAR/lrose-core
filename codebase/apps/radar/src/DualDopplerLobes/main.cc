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

#include <toolsa/pjg_flat.h>
#include <euclid/geometry.h>

#include <cstdio>
#include <cmath>
#include <cstdlib>

#include "Params.hh"

void rotatePoint(double *x, double *y, double radarDir);

int main(int argc, char *argv[]){

  Params P;

  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    fprintf(stderr, "Specify params file with -params option.\n");
    return(-1);
  }     

  for (int ig=0; ig < P.geometries_n; ig++){
    //
    // Open the output file for this radar pair.
    //
    FILE *fp = fopen(P._geometries[ig].outMapFile, "w");
    if (fp == NULL){
      fprintf(stderr, "Failed to create %s, skipping...\n",
	      P._geometries[ig].outMapFile);
      continue;
    }

    //
    // Get the distance and direction between the two radars.
    //
    double radarDist, radarDir;
    PJGLatLon2RTheta(P._geometries[ig].latRadar1, 
		     P._geometries[ig].lonRadar1, 
		     P._geometries[ig].latRadar2, 
		     P._geometries[ig].lonRadar2, 
		     &radarDist, &radarDir);

    fprintf(stdout, "Generating %s, radar distance is %g Km, direction %g degrees\n",
	    P._geometries[ig].outMapFile, radarDist, radarDir);
    


    double startPhi = 0.0;
    double endPhi = 180.0 - P._geometries[ig].minDiffAngleDegrees;
    double incPhi = (endPhi - startPhi) / double(P._geometries[ig].nPointsPerLobe-1);
    const double toRad = 3.1415927 / 180.0;

    //
    // Fill in the first set of points, the outside of the lobe.
    //
    Point_d a,b,c,d,e;
    double t;

    a.x = 0;  a.y = 0;
    d.x = radarDist;  d.y = 0;

    double *x1 = (double *)malloc(sizeof(double)*P._geometries[ig].nPointsPerLobe);
    double *x2 = (double *)malloc(sizeof(double)*P._geometries[ig].nPointsPerLobe);
    double *x3 = (double *)malloc(sizeof(double)*P._geometries[ig].nPointsPerLobe);
    double *x4 = (double *)malloc(sizeof(double)*P._geometries[ig].nPointsPerLobe);

    double *y1 = (double *)malloc(sizeof(double)*P._geometries[ig].nPointsPerLobe);
    double *y2 = (double *)malloc(sizeof(double)*P._geometries[ig].nPointsPerLobe);
    double *y3 = (double *)malloc(sizeof(double)*P._geometries[ig].nPointsPerLobe);
    double *y4 = (double *)malloc(sizeof(double)*P._geometries[ig].nPointsPerLobe);

    if ((x1 == NULL) || (x2 == NULL) || (x3 == NULL) || (x4 == NULL) ||
	(y1 == NULL) || (y2 == NULL) || (y3 == NULL) || (y4 == NULL)){
      fprintf(stderr,"Malloc failed\n"); // Unlikely
      return -1;
    }

    double phi = startPhi;
    int i=0;

    do {

      b.x = cos(toRad*phi); b.y = sin(toRad*phi);
      double theta = 180.0 - P._geometries[ig].minDiffAngleDegrees - phi;
      c.x = radarDist - cos(toRad*theta); c.y = sin(toRad*theta);
      //
      // Get the point of inersection, if any.
      //
      int iret = EG_line_intersect(&a, &b, &c, &d, &e, &t);

      if (iret == 1){
	x1[i] = e.x; y1[i] = e.y;
	x2[i] = e.x; y2[i] = -e.y;
	i++;
      }

      phi += incPhi;

    } while ((phi <= endPhi) && (i < P._geometries[ig].nPointsPerLobe));
    
    int num1 = i;

    //
    // Similar loop for inside lobes.
    //
    phi = startPhi;
    i=0;
    double newAlpha = 180.0 - P._geometries[ig].minDiffAngleDegrees;
    endPhi = 180.0 - newAlpha;
    incPhi = (endPhi - startPhi) / double(P._geometries[ig].nPointsPerLobe-1);

    do {

      b.x = cos(toRad*phi); b.y = sin(toRad*phi);
      double theta = 180.0 - newAlpha - phi;
      c.x = radarDist - cos(toRad*theta); c.y = sin(toRad*theta);

      int iret = EG_line_intersect(&a, &b, &c, &d, &e, &t);

      if (iret == 1){
	x3[i] = e.x; y3[i] = e.y;
	x4[i] = e.x; y4[i] = -e.y;
	i++;
      }

      phi += incPhi;
      
    } while ((phi <= endPhi) && (i < P._geometries[ig].nPointsPerLobe));
    
    int num2 = i;

    for (int k = 0; k < num1; k++){
      rotatePoint(&x1[k], &y1[k], radarDir);
      rotatePoint(&x2[k], &y2[k], radarDir);
    }

    for (int k = 0; k < num2; k++){
      rotatePoint(&x3[k], &y3[k], radarDir);
      rotatePoint(&x4[k], &y4[k], radarDir);
    }


    double lat, lon;

    fprintf(fp,"POLYLINE SIDELOBE %d\n", num1);
    for (int k = 0; k < num1; k++){
      PJGLatLonPlusDxDy(P._geometries[ig].latRadar1, P._geometries[ig].lonRadar1, 
			x1[k], y1[k], &lat, &lon);
      fprintf(fp,"%g %g\n", lat, lon);
    }

    fprintf(fp,"POLYLINE SIDELOBE %d\n", num1);
    for (int k = 0; k < num1; k++){
      PJGLatLonPlusDxDy(P._geometries[ig].latRadar1, P._geometries[ig].lonRadar1, 
			x2[k], y2[k], &lat, &lon);
      fprintf(fp,"%g %g\n", lat, lon);
    }

    fprintf(fp,"POLYLINE SIDELOBE %d\n", num2);
    for (int k = 0; k < num2; k++){
      PJGLatLonPlusDxDy(P._geometries[ig].latRadar1, P._geometries[ig].lonRadar1, 
			x3[k], y3[k], &lat, &lon);
      fprintf(fp,"%g %g\n", lat, lon);
    }
    
    fprintf(fp,"POLYLINE SIDELOBE %d\n", num2);
    for (int k = 0; k < num2; k++){
      PJGLatLonPlusDxDy(P._geometries[ig].latRadar1, P._geometries[ig].lonRadar1, 
			x4[k], y4[k], &lat, &lon);
      fprintf(fp,"%g %g\n", lat, lon);
    }

    fclose(fp);

    free(x1); free(x2); free(x3); free(x4);
    free(y1); free(y2); free(y3); free(y4);

  }

  return 0;

}

void rotatePoint(double *x, double *y, double radarDir){

  double angleToRotateDeg = 90.0 - radarDir;

  double angle = atan2(*y, *x);
  double r = sqrt (*y * *y + *x * *x);
  
  angle += 3.1415927*angleToRotateDeg/180.0;

  *x = r * cos(angle);
  *y = r * sin(angle);

  return;

}
