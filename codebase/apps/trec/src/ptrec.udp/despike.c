/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/******************************************************************
 * despike.c
 *
 * routine to remove noise spikes from data and then fill back in 
 * using a least squares fit. 
 *
 *****************************************************************/

#include "trec.h"

void despike(dimension_data_t *dim_data,
	     int k1, int nel, int naz, int ngates,
	     double vel_scale,
	     ui08 ****vel)

{

  static int first_call = TRUE;
  static ui08 **temp;

  float am[3],bm[3],cm[3],dm[3],t1,t2,t3,deno,anum,eps;
  int i,j,j1,j2,i1,i2,idif,ii,jj,ix,iy;
  long cnt,sum,sumsq,avg;

  if (Glob->params.debug) {
    fprintf(stderr, "*** despike ***\n");
  }

  /*
   * allocate memory
   */
  
  if (first_call) {
    temp = (ui08 **) umalloc2(dim_data->nx, dim_data->ny,
			      sizeof(ui08));
    first_call = FALSE;
  }

  eps=0.00001;
  idif = Glob->params.despike_dif / vel_scale;

  /*  eliminate noise spikes in the data  */

  for (j = 0; j < naz; j++) {
    j1 = j - Glob->params.delta;
    j2 = j + Glob->params.delta;
    if (j1 < 0) j1=0;
    if (j2 > naz) j2=naz;
    for (i = 0; i < ngates; i++) {
      temp[j][i] = 0;
      i1 = i - Glob->params.delta;
      i2 = i + Glob->params.delta;
      if (i1 < 0) i1=0;
      if (i2 > ngates) i2=ngates;
      cnt=0;
      sum=0;
      sumsq=0;
      if(vel[k1][nel][j][i] != 0){
        for(jj=j1; jj<=j2; jj++){
	  for(ii=i1; ii<=i2; ii++){
	    if((ii != i || jj != j) && vel[k1][nel][j][i] != 0){
	      sum = sum + vel[k1][nel][jj][ii];
	      cnt++;
	    }
	  }
	}

        if (cnt >= Glob->params.cntmin) {
	  avg = sum / cnt;
          if (fabs(avg - vel[k1][nel][j][i]) <= idif)
  	    temp[j][i] = vel[k1][nel][j][i];
        }
      }
    }
  }

  /*  fill missing data points using a linear least squares fill   */


  for (j=0; j<naz; j++) {
    j1 = j - Glob->params.delta;
    j2 = j + Glob->params.delta;
    if (j1 < 0) j1=0;
    if (j2 > naz) j2=naz;
    for (i=0; i<ngates; i++) {
      i1 = i - Glob->params.delta;
      i2 = i + Glob->params.delta;
      if (i1 < 0) i1=0;
      if (i2 > ngates) i2=ngates;
      if (temp[j][i] != 0){
	vel[k1][nel][j][i] = temp[j][i];
      }
      else {
	for(ii=0; ii<3; ii++){
	  am[ii]=0;
	  bm[ii]=0;
	  cm[ii]=0;
	  dm[ii]=0;
	}
	for (ii=i1; ii<=i2; ii++) {
	  ix = ii-1;
	  for (jj=j1; jj<=j2; jj++) {
	    iy = jj-j;
	    if (temp[jj][ii] != 0) {
	      am[1] += 1.0;
	      am[2] = am[2] + ix;
	      am[3] = am[3] + iy;
	      bm[2] = bm[2] + ix * ix;
	      bm[3] = bm[3] + ix * iy;
	      cm[3] = cm[3] + iy * iy;
	      dm[1] = dm[1] + temp[jj][ii];
	      dm[2] = dm[2] + ix * (float) temp[jj][ii];
	      dm[3] = dm[3] + iy * (float) temp[jj][ii];
	    }
	  }
	}
	bm[1]=am[2];
	cm[1]=am[3];
	cm[2]=bm[3];
	t1=bm[2]*cm[3]-bm[3]*cm[2];
	t2=bm[1]*cm[3]-bm[3]*cm[1];
	t3=bm[1]*cm[2]-bm[2]*cm[1];
	deno=am[1]*t1-am[2]*t2+am[3]*t3;
	if(deno>eps){
	  anum=dm[1]*t1-dm[2]*t2+dm[3]*t3;
	  vel[k1][nel][j][i]= (int)((anum/deno)+0.5);
	}
      }
    }
  }
}




