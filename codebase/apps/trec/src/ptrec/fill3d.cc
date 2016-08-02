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
 * fill3d
 *
 * Fill data back into grid using least squares method
 *
 * This is  done because bad data has been previously removed
 *
 ****************************************************************************/

#include "trec.h"

void fill3d(dimension_data_t *dim_data,
	    float ***data,
	    int nzst,
	    int nz,
	    int fill_rad,
	    int num_oct,
	    float fill_npts)

{

  static int first_call = TRUE;

  int nx = dim_data->nx;
  int ny = dim_data->ny;
  int i,j,k,l,i0,j0,k0,ioct[4],ix,iy;
  int i1,i2,j1,j2;
  float sx,sy,sx2,sy2,sxy,sd,sdx,sdy;
  float t1,t2,t3,denom,num,npts;

  static float ***temp;

  /*
   * allocate memory
   */
  
  if (first_call) {
    temp = (float ***) umalloc3(dim_data->nx, dim_data->ny,
				dim_data->nz, sizeof(float));
    first_call = FALSE;
  }

  for(k=nzst; k<nz;  k++) {
    for(j=0; j<ny; j++) {
      for(i=0; i<nx; i++) {
	temp[i][j][k]=data[i][j][k];
      }
    }
  } 
  for(k0=nzst; k0<nz; k0++) {
    for(j0=0; j0<ny; j0++) {
      for(i0=0; i0<nx; i0++){
	if (temp[i0][j0][k0] == BAD) {
	  for(l=0; l < 4; l++) {
	    ioct[l]=0;
	  }
	  l = fill_rad;
	  j1=MAX(0,j0-l);
	  j2=MIN(ny-1,j0+l);
	  i1=MAX(0,i0-l);
	  i2=MIN(nx-1,i0+l);
	  npts=0.;
	  sx=0.;
	  sy=0.;
	  sx2=0.;
	  sy2=0.;
	  sxy=0.;
	  sdx=0.;
	  sdy=0.;
	  sd=0.;
	  for(j=j1; j<=j2; j++) {
	    iy=j-j0;
	    for(i=i1; i<=i2; i++) {
	      ix=i-i0;
	      if (temp[i][j][k0] != BAD) {
		if(ix >= 0 && iy >0)ioct[0]=1;
		if(ix > 0 && iy <= 0)ioct[1]=1;
		if(ix <= 0 && iy < 0)ioct[2]=1;
		if(ix < 0 && iy >= 0)ioct[3]=1;
		npts += 1.0;
		sx += ix;
		sy += iy;
		sx2 += ix*ix;
		sy2 += iy*iy;
		sxy += ix*iy;
		sd += temp[i][j][k0];
		sdx += ix*temp[i][j][k0];
		sdy += iy*temp[i][j][k0];
	      } /* if (temp != bad) */
	    } /* i */   
	  } /* j */    

	  if((ioct[0]+ioct[1]+ioct[2]+ioct[3]) >= num_oct &&
	     npts >= fill_npts) {
	    t1 = sx2*sy2-sxy*sxy;
	    t2 = sx*sy2-sxy*sy;
	    t3 = sx*sxy-sx2*sy;
	    denom = npts*t1-sx*t2+sy*t3;
	    num = sd*t1-sdx*t2+sdy*t3;
	    if(denom > 1.0) {
	      data[i0][j0][k0]=num/denom;
	    } 
	  } /* if(ioct  */
	} /* if (temp != bad) */
      } /* i0 */
    } /* j0 */
  } /* k0 */
}

