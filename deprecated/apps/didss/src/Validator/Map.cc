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
#include <cstring>
#include <cstdlib>
#include <toolsa/pjg_flat.h>   
#include <math.h>

#include "Params.hh"
#include "Map.hh"
using namespace std;

Map::Map(){
  Num=0;
  Pt=NULL;
  MinX=0; MinY=0; MaxX=0; MaxY=0;
  sprintf(Name,"Unassigned");
}

//////////////////////////////////////

Map::~Map(){
  free(Pt);
}

//////////////////////////////////////


int Map::Init(char *FileName, Params *P)
{

  FILE *fp;

  fp = fopen (FileName,"rt");
  if (fp==NULL){
    fprintf(stderr,"Dismal failure opening %s\n",FileName);
    return -1;
  }

  const int LineSize = 1024;
  char Line[LineSize];

  //
  // Check that this map file is intended for use in this way.
  //
  fgets(Line,LineSize,fp);
  if (strncmp("#VerifyMap",Line,10)){
    fprintf(stderr,"%s is not a verification map.\n",FileName);
    fclose(fp);
    return -1;
  }

  int i;
  Num=0;

  do {
    fgets(Line,LineSize,fp);
    i=sscanf(Line,"POLYLINE %s %d",Name,&Num);
  } while ((i != 2) && (!(feof(fp))));

  if (Num==0){
    fprintf(stderr,"No POLYLINE statement is %s\n",FileName);
    fclose(fp);
    return -1;
  }
  // fprintf(stderr,"Name : %s\nNum : %d\n",Name,Num);

  float *lat, *lon;
  lat = (float *)malloc(sizeof(float)*Num);
  lon = (float *)malloc(sizeof(float)*Num);

  if ((lat==NULL) || (lon == NULL)){
    fprintf(stderr,"Malloc fail on map %s\n",FileName);
    fclose(fp);
    return -1;
  }

  for(i=0;i<Num;i++){
    if (2!=fscanf(fp,"%f %f",lat + i, lon + i)){
      fprintf(stderr,"%s is incomplete.\n",FileName);
      fclose(fp);
      return -1;
    }
  }

  fclose(fp);

  if ((lat[Num-2]!=lat[0]) || (lon[Num-2]!=lon[0])){
    fprintf(stderr,"Polygon is not closed!\n");
    return -1;
  }
  Num=Num-2;

  Pt = (Point_d *)malloc(sizeof(Point_d)*Num);

  if (P->grid_projection == Params::FLAT) 
    PJGflat_init(P->grid_origin_lat, P->grid_origin_lon,
		 P->grid_rotation);
 
  for(i=0;i<Num;i++) {

    if (P->grid_projection == Params::FLAT) {

      double xd,yd;
      PJGflat_latlon2xy(lat[i], lon[i], &xd, &yd);
      xd = xd - P->grid_minx;
      yd = yd - P->grid_miny;

      Pt[i].x = (double)(xd / P->grid_dx);
      Pt[i].y = (double)(yd / P->grid_dy);

    } else { // LATLON projection.

      Pt[i].y = (double)((lat[i] - P->grid_miny) / P->grid_dy);
      Pt[i].x = (double)((lon[i] - P->grid_minx) / P->grid_dx);

    }

    if (i == 0){
      MinX= (int)floor(Pt[i].x);
      MinY= (int)floor(Pt[i].y);
      MaxX= (int)ceil(Pt[i].x);
      MaxY= (int)ceil(Pt[i].y);

    } else {
      if (Pt[i].x < MinX) MinX =  (int)floor(Pt[i].x);
      if (Pt[i].x > MaxX) MaxX =  (int)ceil(Pt[i].x);
      if (Pt[i].y < MinY) MinY =  (int)floor(Pt[i].y);
      if (Pt[i].y > MaxY) MaxY =  (int)ceil(Pt[i].y);
    }                                         


  }

  free(lat); free(lon);

  //
  // Do not wander past the dragons at the end of the map.
  // Points in the Pt array can be outside this, though.
  //
  if (MaxX > P->grid_nx-1) MaxX = P->grid_nx-1;
  if (MaxY > P->grid_ny-1) MaxY = P->grid_ny-1;
  if (MinX < 0) MinX = 0;
  if (MinY < 0) MinY = 0;

  return 0;

}

