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
/***************************************************************************
 * wair
 *
 * Compute w (vertival velocity) from u and v
 *
 ****************************************************************************/

#include "trec.h"

void wair(dimension_data_t *dim_data,
	  float ***ucart,
	  float ***vcart,
	  float ***wcart,
	  float ***conv)

{

  int i,j,k;
  float delx;

  if (Glob->params.debug) {
    fprintf(stderr, "*** wair ***\n");
  }

  delx=2.0*dim_data->del_x;

  for(k=0; k<dim_data->nz-1; k++) {
    for(j=0; j<dim_data->ny; j++) {
      for(i=0; i<dim_data->nx; i++) {
	wcart[i][j][k] = Glob->params.bad;
	conv[i][j][k] = Glob->params.bad;
	if(k < 2)wcart[i][j][k]=0.;
      }
    }
  }

  for(k=0; k<dim_data->nz-1; k++) {
    for(j=1; j<dim_data->ny-2; j++) {
      for(i=1; i<dim_data->nx-2; i++) {

	if(ucart[i-1][j][k] != Glob->params.bad && ucart[i+1][j][k]
	   != Glob->params.bad && vcart[i][j-1][k] != Glob->params.bad &&
	   vcart[i][j+1][k] != Glob->params.bad) {
	  conv[i][j][k] =
	    -10.*((ucart[i+1][j][k]-ucart[i-1][j][k])/
		  delx + (vcart[i][j+1][k]-vcart[i][j-1][k])/delx);
	}

      }
    }
  }

  for(k=2; k<dim_data->nz-1; k++) {
    for(j=0; j<dim_data->ny-1; j++) {
      for(i=0; i<dim_data->nx-1; i++) {
	
	if (wcart[i][j][k-1] != Glob->params.bad && 
	    conv[i][j][k-1] != Glob->params.bad &&
	    conv[i][j][k] != Glob->params.bad) {
	  wcart[i][j][k] = wcart[i][j][k-1] + dim_data->del_z * 0.5 *
	    (conv[i][j][k-1] + conv[i][j][k]);
	}
	
      }
    }   
  }

  for(k=2; k<dim_data->nz-1; k++) {
    for(j=0; j<dim_data->ny-1; j++) {
      for(i=0; i<dim_data->nx-1; i++) {
	if(wcart[i][j][k] != Glob->params.bad) {
	  wcart[i][j][k] = 0.1*wcart[i][j][k];
	}
      }
    }
  }
  
}

