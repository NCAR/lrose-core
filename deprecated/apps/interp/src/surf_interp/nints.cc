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

#include <math.h>
#include <toolsa/umisc.h>
#include <toolsa/pjg_flat.h>

#include "ProjType.hh"
using namespace std;

//
// Nearest neighbor interpolation.
//
void nints(float *z, 
	    // Single dimensional, 0..(*nx)*(*ny)-1, referenced as z[j*(*nx)+i]
	   float *x, // 0..n-1
	   float *y, // 0..n-1
	   float *d, // 0..n-1
	   int numpts,
	   float bad,
	   float MaxInterpDist,
	   ProjType Proj)
{


  for (int ix=0; ix < Proj.Nx; ix++){
    for (int iy=0; iy < Proj.Ny; iy++){

      int first=1, best_ip=0; // set to 0 to avoid compiler warnings.
      double dist,mindist;
      mindist = MaxInterpDist + 1.0;

      for (int ip=0; ip < numpts; ip++){
	//
	// Work out the distance between x[ip],y[ip]
	// and ix,iy.
	//

	if ( d[ip] != bad ) {

	  if (Proj.flat){

	    float x1,y1,x2,y2;
	    x1=x[ip] * Proj.Dx; y1=y[ip] * Proj.Dy;
	    x2=ix*Proj.Dx;     y2=iy*Proj.Dy;

	    dist = sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));

	  } else { // x and y are lon and lat
	    double theta,r,plat,plon,qlat,qlon;
		 
	    plon = Proj.Origin_lon + ix * Proj.Dx;
	    plat = Proj.Origin_lat + iy * Proj.Dy;
	    
	    qlon = Proj.Origin_lon + x[ip] * Proj.Dx;
	    qlat = Proj.Origin_lat + y[ip] * Proj.Dy;

	    PJGLatLon2RTheta(qlat, qlon,
			     plat, plon, &r, &theta);
	    dist = r;

		 
	  }

	  if (first){
	    first=0;  
	    mindist = dist;
	    best_ip = ip;
	  } else {
	    if (dist < mindist) {
	      best_ip = ip;
	      mindist = dist;
	    }
	  }

	} // end of if not bad
      } // End of loop through x[ip],y[ip]
       


      if ((mindist > MaxInterpDist) || (first)){
	z[iy * Proj.Nx + ix]=bad;
      } else {
	z[iy * Proj.Nx + ix]=d[best_ip];
      }

    }
  }

}

