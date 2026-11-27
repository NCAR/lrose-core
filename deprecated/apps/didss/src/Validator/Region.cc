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

#define MAX_PT 500

#include "Params.hh"
#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <toolsa/pjg_flat.h>
#include <euclid/geometry.h> 

#include <Mdv/MdvxProj.hh>

#include "Region.hh"

using namespace std;


void Region(Params *P,
	    float *data,
	    Mdvx::field_header_t Fhdr,
	    Mdvx::master_header_t Mhdr){

  //
  // Read the map file.
  //

  float lat[MAX_PT];
  float lon[MAX_PT];

  Point_d PolyLine[MAX_PT];

  FILE *fp = fopen (P->RegionMapFile,"rt");
  if (fp==NULL){
    fprintf(stderr,"Failed to open map file %s\n",
	    P->RegionMapFile);
    exit(-1);
  }

  //
  // Read the file of lat/lon points.
  //
  int num=0;
  char line[1024];
  while ( NULL != fgets(line,1024,fp)){
    if ((2==sscanf(line, "%f %f",lat + num, lon + num)) &&
	(fabs(lat[num]) <= 90.0) && (fabs(lon[num]) <= 360.0)){
	num++;
      if (num > MAX_PT){
	fprintf(stderr,"Too many points in %s\n",P->RegionMapFile);
	fclose(fp);
	exit(-1);
      }
    }
  }
  
  fclose(fp);
  if (P->Debug){
    fprintf(stderr,"Read %d points from %s\n",num,P->RegionMapFile);
  }
  //
  // Convert the points into i,j space.
  //
  MdvxProj Proj(Mhdr, Fhdr);

  int first=1;
  int minx=0,maxx=0,miny=0,maxy=0; // Set to 0 to avoid compiler warnings.

  for (int i=0; i<num; i++){

    int ixx,iyy;
    if (Proj.latlon2xyIndex(lat[i], lon[i], ixx, iyy)){
      cerr << "ERROR : Point " << lat[i] << ", " << lon[i];
      cerr << " is outside of region - I cannot cope." << endl;
      exit(-1);
    } 

    PolyLine[i].x = (double)(ixx);
    PolyLine[i].y = (double)(iyy);
    if (first){
      first=0;
      minx= (int)PolyLine[i].x; maxx=minx;
      miny= (int)PolyLine[i].y; maxy=miny;
    } else {
      if (PolyLine[i].x < minx) minx =  (int)PolyLine[i].x;
      if (PolyLine[i].x > maxx) maxx =  (int)PolyLine[i].x;
      if (PolyLine[i].y < miny) miny =  (int)PolyLine[i].y;
      if (PolyLine[i].y > maxy) maxy =  (int)PolyLine[i].y;
    }
  }

  if (P->Debug){
    cerr << "Region is " << P->grid_nx << " by " << P->grid_ny << endl;
  }
  int lastPd = -1;
  int pd = 0;
  for (int i=0; i<P->grid_nx; i++){

    if (P->Debug){
      pd = int(100.0 * double(i) / double(P->grid_nx-1) );
      pd = 10*(pd/10);
      if (pd > lastPd){
	cerr << pd << " percent done with region." << endl;
	lastPd = pd;
      }
    }

    for (int j=0; j<P->grid_ny; j++){
      Point_d Pt;
      Pt.x = i;
      Pt.y = j;
 
      if ((i < minx) || (i > maxx) ||
	  (j < miny) || (j > maxy)){
	data[j*P->grid_nx + i]=-1;
      } else { 
    	if (!(EG_point_in_polygon2(Pt, PolyLine, num))) { 
	  data[j*P->grid_nx + i]=-1;
	}
      }
    }
  }  
}



