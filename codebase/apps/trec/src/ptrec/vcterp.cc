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
/***************************************************************************
 * vcterp
 ****************************************************************************/

#include "trec.h"

static void tklin3(float r1,
		   float r2,
		   float r3,
		   float x1,
		   float x2,
		   float x3,
		   float *xp);

void vcterp(float **store_cor,
	    float *range,
	    int iamx,
	    float *azim,
	    int ibmx,
	    int ia1,
	    int ib3,
	    int ioffa,
	    int ioffb,
	    float *xpa,
	    float *xpb)

{

  float x[3],r[3],xp;
  
  
  if (Glob->params.debug) {
    fprintf(stderr, "*** vcterp ***\n");
  }

  x[0] = range[iamx+ioffa-1];
  x[1] = range[iamx+ioffa];
  x[2] = range[iamx+ioffa+1];
  r[0] = store_cor[iamx-ia1-1][ibmx-ib3];
  r[1] = store_cor[iamx-ia1][ibmx-ib3];
  r[2] = store_cor[iamx-ia1+1][ibmx-ib3];
  tklin3(r[0],r[1],r[2],x[0],x[1],x[2],&xp);
  *xpa = xp;

  x[0] = azim[ibmx+ioffb-1];
  x[1] = azim[ibmx+ioffb];
  x[2] = azim[ibmx+ioffb+1];
  r[0] = store_cor[iamx-ia1][ibmx-ib3-1];
  r[2] = store_cor[iamx-ia1][ibmx-ib3+1];
  tklin3(r[0],r[1],r[2],x[0],x[1],x[2],&xp);
  *xpb = xp;

}

static void tklin3(float r1,
		   float r2,
		   float r3,
		   float x1,
		   float x2,
		   float x3,
		   float *xp)

{

  float a0,a1,a2;

  a2 = ((r2-r1)*(x3-x1)+(r1-r3)*(x2-x1))/((x2-x1)*(x3-x1)*(x2-x3));
  a1 = (r2-r1-(x2*x2-x1*x1)*a2)/(x2-x1);
  a0 = r1-x1*a1-x1*x1*a2;
  *xp = -a1/(a2+a2);

}
