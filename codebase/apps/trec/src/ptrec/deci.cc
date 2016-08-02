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
 * deci
 *
 * Routine to decimate the data by getting rid of uncertain values - 
 * these are added back in later using least-squares method
 *
 ****************************************************************************/

#include "trec.h"

void deci(dimension_data_t *dim_data,
	  float ***data,
	  int nz,
	  int dec_rad,
	  double dec_npts)

{
  
  static int first_call = TRUE;
  static float ***temp;

  int nx = dim_data->nx;
  int ny = dim_data->ny;
  int i,j,k,i1,i2,j1,j2,ic,jc;
  float sum,div,avg;


  PMU_auto_register("deci - decimating");

  /*
   * allocate memory
   */
  
  if (first_call) {
    temp = (float ***) umalloc3(dim_data->nx, dim_data->ny,
				dim_data->nz, sizeof(float));
    first_call = FALSE;
  }

  for(k=0; k<nz;  k++) {
    for(j=0; j<ny; j++) {
      for(i=0; i<nx; i++) {
	temp[i][j][k]=data[i][j][k];
      }
    }
  } 

  for(k=0; k<nz; k++) {
    for(j=0; j<ny; j++) {
      j1=MAX(0,j-dec_rad);
      j2=MIN(ny-1,j+dec_rad);
      for(i=0; i<nx; i++) {
	i1=MAX(0,i-dec_rad);
	i2=MIN(nx-1,i+dec_rad);
	if (temp[i][j][k] != BAD) {         
	  div=0.;
	  sum=0.;
	  data[i][j][k] = BAD;
	  for(jc=j1; jc<=j2; jc++) {
	    for(ic=i1; ic<=i2; ic++) {
	      if (temp[ic][jc][k] != BAD) {
		div++;
		sum += temp[ic][jc][k];
	      }
	    } /* ic */
	  } /* jc */
	  if(div >= dec_npts) {
	    avg=sum/div;
	    if (fabs(temp[i][j][k] - avg) <= Glob->params.dif_mean_thr)
	      data[i][j][k]=temp[i][j][k];
	  }
	} /* if{temp != BAD) */
      } /* i */
    } /* j */
  } /* k */
}

