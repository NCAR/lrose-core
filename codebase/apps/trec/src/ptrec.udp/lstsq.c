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
 * lstsq.c      
 *
 * interpolates random data to a cartesian grid using a local linear least
 * squares fit.  only data within the radius of influence of a grid point
 * affect the grid point. interpolation is constrained by two input patameters:
 *     nquad= minimum number of quadrants that must have data in it before
 *            proceeding with the interpolation.
 *     pts  = minimum number of data points that must be within the radius of
 *            influence before interpolating.
 *  if either of the constraints above are not met, the grid point is flagged
 *  as missing.  the initial radius of influence is set equal to glob->rad_min
 *
 * John Tuttle  
 *
 * NCAR, Boulder, Colorado, USA
 *
 * Feb  1992
 *
 ****************************************************************************/

#include "trec.h" 

void lstsq(dimension_data_t *dim_data,
	   float *u,
	   float *x,
	   float *y,
	   float *z,
	   int nvec,
	   float ***ucart,
	   int *iaz1,
	   int *iaz2,
	   int nel,
	   float ***el)

{

  static int first_call = TRUE;

  static short ***ioct;
  
  int i,j,k,iel;
  int ix,iy,ix1,ix2,iy1,iy2,ih;
  float r,rad_sq,xg,yg,xd,yd;
  float denom,t1,t2,t3;
  float numu,rg,sinel;
  char file_name[MAX_PATH_LEN];

  static float **npts, **sumx, **sumy, **sumux, **sumy2, **sumuy;
  static float **sumu, **sumx2, **sumxy, **meanu;
  static float ***zgrid, ***ucones;

  FILE *fp;

  int dec_rad=3,fill_rad=3,num_oct=3;
  float dec_npts=7.,fill_npts=7.;

  if (Glob->params.debug) {
    fprintf(stderr, "*** lst_sq ***\n");
  }

  PMU_auto_register("lst_sq - interpolating");

  /*
   * allocate memory
   */

  if (first_call) {

    ioct = (short ***) umalloc3(dim_data->nx,
				dim_data->ny, 4, sizeof(short));
    npts = (float **) umalloc2(dim_data->nx,
			       dim_data->ny, sizeof(float));
    sumx = (float **) umalloc2(dim_data->nx,
			       dim_data->ny, sizeof(float));
    sumy = (float **) umalloc2(dim_data->nx,
			       dim_data->ny, sizeof(float));
    sumux = (float **) umalloc2(dim_data->nx,
				dim_data->ny, sizeof(float));
    sumy2 = (float **) umalloc2(dim_data->nx,
				dim_data->ny, sizeof(float));
    sumuy = (float **) umalloc2(dim_data->nx,
				dim_data->ny, sizeof(float));
    sumu = (float **) umalloc2(dim_data->nx,
			       dim_data->ny, sizeof(float));
    sumx2 = (float **) umalloc2(dim_data->nx,
				dim_data->ny, sizeof(float));
    sumxy = (float **) umalloc2(dim_data->nx,
				dim_data->ny, sizeof(float));
    meanu = (float **) umalloc2(dim_data->nx,
				dim_data->ny, sizeof(float));
    zgrid = (float ***) umalloc3(dim_data->nx,
				 dim_data->ny, dim_data->nz, sizeof(float));
    ucones = (float ***) umalloc3(dim_data->nx,
				  dim_data->ny, dim_data->nz, sizeof(float));

    first_call = FALSE;

  } 

  /* initialize least squares storage arrays*/

  sprintf(file_name, "%s%s%s", Glob->params.local_dir, PATH_DELIM, 
	  "interp.out");
  fp = fopen(file_name,"w");

  ih = Glob->params.horz_rad / dim_data->del_x + 1.0001;
  rad_sq = Glob->params.horz_rad * Glob->params.horz_rad;
  
  if (Glob->params.debug) {

    fprintf(fp,"params - minx,dx,nx= %6.1f %6.1f %6d \n",
	    dim_data->min_x,
	    dim_data->del_x,
	    dim_data->nx);

    fprintf(fp,"params - miny,dy,ny= %6.1f %6.1f %6d \n",
	    dim_data->min_y,
	    dim_data->del_y,
	    dim_data->ny);

    fprintf(fp,"dim_data-x1,x2,dx= %g %g %g\n",
	    dim_data->min_x,
	    dim_data->max_x,
	    dim_data->del_x);
    
    fprintf(fp,"dim_data-y1,y2,dy= %g %g %g\n",
	    dim_data->min_y,
	    dim_data->max_y,
	    dim_data->del_y);

  }

  for (iel=0; iel<=nel; iel++) {

    sinel=sin(el[10][iel][1]*PI/180.);
    init_ls(dim_data,ucones,iel,npts,sumx,sumy,sumx2,sumy2,
	    sumxy,sumu,sumux,sumuy,
	    ioct,meanu);
    
    for(i=iaz1[iel]; i<iaz2[iel]; i++) { /*loop over number of data points*/
      
#ifdef NOTNOW
      fprintf(fp,"x,y,z,u= %6.1f %6.1f %6.1f %6.1f\n",x[i],y[i],z[i],u[i]);
#endif
      
      if(u[i] != -999.) {
	
        /*
	 * find index of grid point that is nearest the i_th data poin
	 */
	
        ix = (x[i]-dim_data->min_x) / dim_data->del_x;
        iy = (y[i]-dim_data->min_y) / dim_data->del_y;
	
        ix1=ix-ih+1;
        ix2=ix+ih;
        if(ix1 < 0)
          ix1=0;
        if(ix2 >= dim_data->nx)
          ix2=dim_data->nx-1;
        iy1=iy-ih+1;
        iy2=iy+ih;
        if(iy1 < 0)
          iy1=0;
        if(iy2 >= dim_data->ny)
          iy2=dim_data->ny-1;

	/*
	 * loop over all grid points that are within the
	 * radius of influence of the
	 * i_th data point */

	for (iy=iy1; iy<iy2; iy++) {

	  yg = iy * dim_data->del_y + dim_data->min_y;

	  for (ix=ix1; ix<ix2; ix++) {                              
	    xg = ix * dim_data->del_x + 
	      dim_data->min_x;

	    /*find distance from current grid point to the i_th data point*/

	    r=(xg-x[i])*(xg-x[i]) + (yg-y[i])*(yg-y[i]);


	    /* if distance is less than the radius of influence,
	     * sum up terms for least squares fit and keep track of
	     * which quadrant the data point
	     * is in.
	     */
	    
	    if(r <= rad_sq) {
	      npts[ix][iy] += 1.0;
	      meanu[ix][iy] += u[i];
	    } /* if */
	  } /* ix */
	} /* iy */

      }	/* if (coeff[i] >= Glob->params.cor_min_thr */
    } /* i */

    for(i=0; i<dim_data->nx; i++) {
      for (j=0; j<dim_data->ny; j++) {

	if (npts[i][j] > 0.) {
	  meanu[i][j] = meanu[i][j]/npts[i][j];
	  npts[i][j] = 0.;
	} /* if */
      } /* for */
    } /* for */

    for (i=iaz1[iel]; i<iaz2[iel]; i++) { /*loop over number of data points*/

      if (u[i] != -999.) {

	/*
	 * find index of grid point that is nearest the i_th data poin
	 */

        ix = (x[i] - dim_data->min_x) / dim_data->del_x;
        iy = (y[i] - dim_data->min_y) / dim_data->del_y;
        ix1 = ix - ih + 1;
        ix2 = ix + ih;
        if (ix1 < 0)
          ix1=0;
        if (ix2 >= dim_data->nx)
          ix2 = dim_data->nx - 1;
        iy1 = iy - ih + 1;
        iy2 = iy + ih;
        if (iy1 < 0)
          iy1 = 0;
        if (iy2 >= dim_data->ny)
          iy2 = dim_data->ny - 1;

	/*
	 * loop over all grid points that are within the radius of
	 * influence of i_th data point
	 */
	
	for (iy = iy1; iy < iy2; iy++) {

	  yg = iy * dim_data->del_y + dim_data->min_y;

	  for (ix = ix1; ix < ix2; ix++) {                              

	    xg = ix * dim_data->del_x + 
	      dim_data->min_x;

	    /*
	     * find distance from current grid point to the i_th data poin
	     */

	    r=(xg-x[i])*(xg-x[i]) + (yg-y[i])*(yg-y[i]);


	    /* if distance is less than the radius of influence, sum up
	     * terms for least squares fit and keep track of which 
	     * quadrant the data point is in.
	     */
       
	    if (r <= rad_sq && fabs(u[i]-meanu[ix][iy])
		<= Glob->params.dif_mean_thr) {

	      npts[ix][iy] += 1.0;
	      xd=x[i]-xg;
	      yd=y[i]-yg;
	      
	      if(xd >= 0. && yd > 0.)
		ioct[ix][iy][0]=1;  
	      if(xd >  0. && yd <=0.)
		ioct[ix][iy][1]=1;  
	      if(xd <= 0. && yd < 0.)
		ioct[ix][iy][2]=1;  
	      if(xd <  0. && yd >=0.)
		ioct[ix][iy][3]=1;  
	      sumx[ix][iy] += xd;
	      sumy[ix][iy] += yd;
	      sumx2[ix][iy] += xd*xd;
	      sumy2[ix][iy] += yd*yd;
	      sumxy[ix][iy] += xd*yd;
	      sumux[ix][iy] += xd*u[i];
	      sumuy[ix][iy] += yd*u[i];
	      sumu[ix][iy] += u[i];
	    }	/* if(r <= rad_sq) */
	  } /* ix */
	} /* iy */
      }	/* if (coeff[i] >= Glob->params.cor_min_thr */
    } /* i */

    /*
     * done with summing terms; now for all grid points which satisfy the 
     * number of quadrants and number of data points constraints find the
     * value at the grid point
     */

    for(i=0; i<dim_data->nx; i++) {

      xg = i * dim_data->del_x + dim_data->min_x;

      for(j=0; j<dim_data->ny; j++) {                              

	yg = j * dim_data->del_y + dim_data->min_y;
	rg=sqrt(xg*xg+yg*yg);
	zgrid[i][j][iel] = rg * sinel + rg * rg / 17014. +
	  Glob->params.radar_altitude;
	
	if (ioct[i][j][0] + ioct[i][j][1] + ioct[i][j][2] +
	    ioct[i][j][3] >= Glob->params.num_oct
	    && npts[i][j] >= Glob->params.num_vectors)
	  {

	    /*constraints have been satisfied;
	     *calculate denominator of grid point
	     * value.*/

	    t1 = sumx2[i][j]*sumy2[i][j]-sumxy[i][j]*sumxy[i][j];
	    t2 = sumx[i][j]*sumy2[i][j]-sumxy[i][j]*sumy[i][j];
	    t3 = sumx[i][j]*sumxy[i][j]-sumx2[i][j]*sumy[i][j];
	    denom = npts[i][j]*t1-sumx[i][j]*t2+sumy[i][j]*t3;
	    numu = sumu[i][j]*t1-sumux[i][j]*t2+sumuy[i][j]*t3;

	    if(denom <= 1.) {
	      ucones[i][j][iel] = Glob->params.bad;
	      if (Glob->params.debug) {
		fprintf(fp,"denom= %6.2f\n",denom);
	      }
	    } else {
	      ucones[i][j][iel]=numu/denom;
	    } /* if */

	  } /* if(ioct  */
	
      }	/* j */
    } /* i */
  } /* iel */

  deci(dim_data,ucones,nel,dec_rad,dec_npts);
  fill3d(dim_data,ucones,0,nel,fill_rad,num_oct,fill_npts);

#ifdef NOTNOW
     
  for (k=2; k<dim_data->nz; k++) {
    
    zlev = dim_data->min_z + k * dim_data->del_z;
    for(j=0; j<dim_data->ny; j++) {
      
      for(i=0; i<dim_data->nx; i++) {
	
	ucart[i][j][k] = Glob->params.bad;
	
	for(iel=0; iel < nel && zgrid[i][j][iel]<=zlev; iel++) {
	  
	  if(zgrid[i][j][iel]<=zlev && zgrid[i][j][iel+1]> zlev) {
	    
	    if (ucones[i][j][iel] != Glob->params.bad && 
		ucones[i][j][iel+1] != Glob->params.bad) {
	      
	      wgt=(zlev-zgrid[i][j][iel])/(zgrid[i][j][iel+1]-
					   zgrid[i][j][iel]);
	      ucart[i][j][k]=ucones[i][j][iel]+wgt*(ucones[i][j][iel+1]-
						    ucones[i][j][iel]);
	    } 
	  }   
	}
      }  
    }  
  }  
  
#endif

  for (k=0; k<nel; k++) {
    for (j=0; j<dim_data->ny; j++) {
      for (i=0; i<dim_data->nx; i++) {
	ucart[i][j][k]=ucones[i][j][k];
      } /* i */
    } /* j */
    
  } /* k */

  fill3d(dim_data,ucart,4,dim_data->nz,7,num_oct,fill_npts);

  fclose(fp);  

}


