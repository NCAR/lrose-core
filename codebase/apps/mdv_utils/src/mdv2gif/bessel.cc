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
#include <stdlib.h>
#include <math.h>

#include "bessel.hh"

#define SMALL 0.0001
#define DELTA 0.5

double Bsslint(float *p,float *ai,float *aj,int *m,int *n)
{
  float tp[4];        
  int ii,k;
  float dx,dy,c1,c2,c3,c4,t1,t2;
  int i,j;
  int i_j,i_j1,i_j2,i_jm1,i1_j,i1_j1;

  i = (int)(*ai);                   
  j = (int)(*aj);
  dx= *ai-i;
  dy= *aj-j;

   if ((i == (*m-1)) && ((SMALL-dx) > 0.0)) 
  {
	if (j == (*n-1))
      return(p[*m*j+i]);
	else
	{
		return ((dy * (p[*m*(j+1)+i] - p[*m*j+i])) + p[*m*j+i]);
	}
  }
  
  if ((j == (*n-1)) && ((SMALL-dy) > 0.0)) 
  {
	if (i == (*m-1))
      return(p[*m*j+i]);
	else
	{
		return ((dx * (p[*m*j+i+1] - p[*m*j+i])) + p[*m*j+i]);
	}
  }

  if ((i == 0)||(i >= (*m-2)) || (j == 0) || (j >= (*n-3)))
  {
    if (j >= (*n-2))
      j = (*n-2);
    if (i >= (*m-2))
      i = (*m-2);

    i_j   = (*m * j    ) + i;
    i_j1  = (*m * (j+1)) + i;
    i1_j  = (*m * j    ) + i + 1;
    i1_j1 = (*m * (j+1)) + i + 1;
    return(p[i_j] + (p[i1_j] - p[i_j]) * dx + (p[i_j1] - p[i_j]) * dy
           + (p[i1_j1] + p[i_j] - p[i_j1] - p[i1_j]) * dx * dy);
  }
  else
  {
    c1=(float)(dy-DELTA);
    c2=(float)(dy* DELTA*(dy-1));
    c3=(float)(DELTA*c2);
    c4=(float)(c1*c2);
    ii=i-1;
 
    for (k = 0;k < 4;k++)
    {
      i_j   = (*m * j    ) + ii;
      i_j1  = (*m * (j+1)) + ii;
      i_j2  = (*m * (j+2)) + ii;
      i_jm1 = (*m * (j-1)) + ii;

      t1 = p[i_j1] + p[i_j];
      t2 = p[i_j1] - p[i_j];
      tp[k] = (float)(DELTA * t1 + c1 * t2 + c3 * (p[i_j2] + p[i_jm1] - t1) +
              c4 * ((p[i_j2] - p[i_jm1])/3 - t2));
      ii++;
    }/*for*/
 
    c1=(float)(dx-DELTA);
    c2=(float)(dx*DELTA*(dx-1));
    c3=(float)(DELTA*c2);
    c4=(float)(c1*c2);
    t1=tp[2]+tp[1];
    t2=tp[2]-tp[1];

    return(DELTA*t1+c1*t2+c3*(tp[3]+tp[0]-t1)+c4*((tp[3]-tp[0])/3-t2));
  }/*else*/
}/*bessel*/

