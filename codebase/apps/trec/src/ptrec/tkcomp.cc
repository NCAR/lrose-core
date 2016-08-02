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
 * tkcomp
 *
 * Main trec routine - this computes the X-correlation values
 * on a grid. A 'window' of set size is moved around, using
 * a set jump interval. If a good correlation is found, a vector
 * is set in u, v, x, y, z, and dop.
 *
 ****************************************************************************/

#include "trec.h"

void tkcomp( unsigned char ***dbz0, unsigned char ***dbz1, 
             unsigned char ***vel1,
             float dbz_scale0, float dbz_bias0, 
             float vel_scale0, float vel_bias0,
             float max_distance, float delta_time, 
             float delta_azim, float delta_gate,
             float *azim, int num_azim, float *elev, int num_elev,
             float *range, int num_gate, float radar_altitude,
             dimension_data_t *dim_data, int max_vec, int *iaz1, int *iaz2,
             float *u, float *v, float *x, float *y, float *z, float *dop, 
             int *num_vec )
 {

  static int first_call = TRUE;
  static long **base;
  static float **store_cor;
  
  long d,idbz_max;
  long sum1,sum2,sumsq1,sumsq2,sum12,sumv,sumsqv;
  float uu,vv,cosel,deltaa,ritera;
  float radsq,ref;
  float s1,s2,s12,ssq1,ssq2;
  float sinel,var,avgd,corcoef,cormax,vrt,vdif;
  float ridxa,ridxb,a1,riterb,rninv,az2,angl1,x1,x2,z1,r1,y1,y2,rg;
  float dt, dtsecx,dtsecy,a2,sinang,cosang,angl2;
  float vrad,xpa,xpb,xnd,ynd,sumc,avgu,avgv,avgcor,deltab;
  float avg_sumv,avg_sqv,numpts,fraction,vtan,numst,fractst;
  float nvpts,half_npts;
  float sumd,sumsqd,mindbz,maxdbz,ndbz;

  int i,j,k,iel,idista,isiza,ioffa,lasta,nvec;
  int idxa,idxb,ia1,ia2,ia3,ia4,lastb,ib3,ib4,iib2,iia2,ia,ib;
  int iamx = 0, ibmx = 0;
  int nc,idistb,isizb,ioffb,iib,iia;
  int iref,numdif;
  int ibin;
  int vbyte;
  float velmin,velmax,sumdif;

  FILE *fp1;

  if (Glob->params.debug) {
     printf( "%s: Beginning correlation analysis (tkcomp)\n", 
              Glob->prog_name );
  }

  PMU_auto_register("starting tkcomp");

  /*
   * allocate memory
   */
  
  if (first_call) {
    base = (long **) umalloc2(200, 200, sizeof(long));
    store_cor = (float **) umalloc2(200, 200, sizeof(float));
    first_call = FALSE;
  }



  fp1=fopen("trec.out","w");



  iaz1[0]=1;
  nvec=0;
  dt = delta_time / 60.;

  fprintf(fp1,"\n tkmove,dt=  %6.1f %6.1f ",max_distance,dt);
  idbz_max = (int)((Glob->params.dbz_max - dbz_bias0) / dbz_scale0); 
  if (Glob->params.debug) {
    printf( "%s:    scaled dbz_max = %ld \n", Glob->prog_name, idbz_max);
  }

  for( iel=0; iel < num_elev; iel++ ) {

    PMU_auto_register("tkcomp - computing");

    //
    // Do some initialization for each PPI to be processed
    //
    velmin=BAD;
    velmax=-BAD;

    uu=0.;
    vv=0.;
    nc=0;
    sumdif=0.;
    sumc=0.;
    numdif=0;
    cosel = cos(elev[iel]*PI/180.);
    sinel = sin(elev[iel]*PI/180.);
    deltaa = delta_gate * cosel;
    idista = (int)(max_distance / deltaa);    
    isiza = (int)(Glob->params.box_size / deltaa + 1.5);
    fprintf(fp1,"\n deltaa,isiza,idista= %6.2f %5d %5d",deltaa,isiza,idista);
    ioffa = (int)((isiza-1) * 0.5);    
    ritera = (int)(Glob->params.box_spacing / deltaa);
    if (ritera < 1.) 
      ritera = 1.;
    lasta=num_gate-idista-isiza+1;  
    ridxa=idista+1.0+5.0/deltaa;
    idxa=(int)ridxa;

    while(idxa<lasta) { 

      PMU_auto_register("tkcomp - computing");

      r1 = range[idxa+ioffa];
      a1 = range[idxa+ioffa]*cosel;
      ia1 = idxa - idista;
      ia2 = idxa + idista;
      deltab = 2*a1*sin(delta_azim*(0.5*PI/180.));
      idistb = (int)(max_distance/deltab);
      isizb = (int)(Glob->params.box_size / deltab + 1.5);    
      
      /*    IF(MOD(ISIZB,2).EQ.0)ISIZB=ISIZB+1    */

      ioffb = (int)((isizb-1)*0.5);
      riterb = Glob->params.box_spacing / deltab;
      if(riterb < 1.0)riterb=1.;
      lastb = num_azim-idistb-isizb+1;
      rninv = 1./(isiza*isizb);
      half_npts=0.5*(float)(isiza*isizb);
      ridxb = idistb+1.;
      idxb = (int)ridxb;
      
      while(idxb < lastb) {
                                                       
	PMU_auto_register("tkcomp - computing");

	angl1 = azim[idxb+ioffb]*0.0174532;
	x1 = a1*sin(angl1);
	y1 = a1*cos(angl1);          

        fprintf(fp1,"\n idxb,x1,y1= %5d %6.1f %6.1f",idxb,x1,y1);

	/* specify r1 and sinel */
	z1 = r1 * sinel + r1 * r1 / 17014. + radar_altitude;

	if(x1 >= dim_data->min_x && x1 <= dim_data->max_x &&
	   y1 >= dim_data->min_y && y1 <= dim_data->max_y) {

	  az2 = azim[idxb];

	  azindx(az2,azim,idistb,num_azim,&ib3,&ib4,isizb);

	  if(ib3 != -1) {
         
	    cormax=-BAD;
	    numpts = 0.;
	    numst = 0.;
	    for(j=0; j<isizb; j++) {
	      for(k=0; k<isiza; k++) {
		base[j][k] = dbz0[iel][idxb+j][idxa+k];
		if(base[j][k] > idbz_max)
		  numpts += 1.0;
	      }
	    }
	    ref = dbz_scale0 * (float) dbz0[iel][idxb+ioffb][idxa+ioffa] + dbz_bias0;
	    iref = dbz0[iel][idxb+ioffb][idxa+ioffa];
	    fraction = numpts / ((float)(isiza*isizb));
	    fractst  = numst / ((float)(isiza*isizb));  

	    if ( fraction <= Glob->params.fract ) {
	      rg=sqrt(x1*x1+y1*y1);
	      dtsecx=16.6667*x1/(rg*dt);
	      dtsecy=16.6667*y1/(rg*dt);
	      sumd = 0.;
	      sumsqd = 0.;
	      sumv=0;
	      sumsqv=0;
	      nvpts=0.;
	      sum1=0;
	      sumsq1=0;

	      mindbz=999.;
	      maxdbz=-999.;
	      ndbz=0.;
      

	      for(j=0; j < isizb; j++) {
		for(i=0; i < isiza; i++) {
		  d=(long) ((float)dbz_scale0*(float)base[j][i]+(float)dbz_bias0 + 0.5);
		  sum1=sum1+base[j][i];
		  sumsq1=sumsq1+base[j][i]*base[j][i];

		  
		  vbyte = vel1[iel][idxb+j][idxa+i];
		  if(vbyte != 0){
		    sumv += vbyte;
		    sumsqv += vbyte * vbyte;
		    nvpts +=1.;
		  }
		}
	      }
	      x[nvec] = x1;
	      y[nvec] = y1;
	      z[nvec] = z1;


	      
	      if(nvpts > half_npts){
		avg_sumv = (float)sumv/nvpts;
		avg_sqv = (float)sumsqv/nvpts;
		avgd = (float)vel_scale0*(sumv/nvpts)+vel_bias0;
		var  = (avg_sqv - avg_sumv*avg_sumv)*vel_scale0*vel_scale0;
	      } else {
		avgd = -BAD;
		var = -BAD;
	      }


              fprintf(fp1,"\n nvpts,halfnpts= %6.1f %6.1f ",nvpts,half_npts);
              fprintf(fp1,"\n avgd,var= %6.1f %6.1f",avgd,var);

	      if (var <= Glob->params.var_thr) {
		for(ib=ib3; ib<=ib4; ib++) {
		  iib2 = ib + isizb-1;
		  iib2 = MIN(iib2, num_azim - 1);
		  angl2 = azim[ib+ioffb]*0.01745329;
		  sinang = sin(angl2);
		  cosang = cos(angl2);
		  for (ia = ia1; ia <= ia2; ia += Glob->params.rng_skip) {                                       
		    a2 = range[ia+ioffa]*cosel; 
		    iia2 = ia +isiza -1;
		    iia2 = MIN(iia2, num_gate - 1);

		    x2 = a2*sinang;
		    y2 = a2*cosang;
		    radsq=(x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
		    if(radsq <= max_distance*max_distance) {
		      vrad = (x2-x1) * dtsecx + (y2-y1) * dtsecy;
		      if ((vrad >= (avgd - Glob->params.vel_rad) && 
			   vrad <= (avgd + Glob->params.vel_rad)) ||
			  avgd<-900.) {
			sum2=0;
			sumsq2 = 0;
			sum12 = 0;
			for(iib=ib; iib<=iib2; iib++) {
			  for(iia=ia; iia<=iia2; iia++) {
			    d=dbz1[iel][iib][iia];
			    sum2 += d;
			    sumsq2 += d*d;
			    sum12 += base[iib-ib][iia-ia]*d;
			  }
			}
			s12 = (float)sum12;
			s1 = (float)sum1;
			s2 = (float)sum2;
			ssq1 = (float)sumsq1;
			ssq2 = (float)sumsq2;
			corcoef = (s12-rninv*s1*s2)/
			  sqrt((ssq1-rninv*s1*s1)*
			       (ssq2-rninv*s2*s2));
#ifdef NOTNOW
			store_cor[ia-ia1][ib-ib3] = corcoef;
#endif

			if(corcoef > cormax) {
			  cormax = corcoef;                
			  iamx = ia;
			  ibmx = ib;
			}
		      } /* if(vrad>=      */
		    } /* if((x2-x1)*x2-x1)  */
		  } /* for(ia=ia1      */
		} /*  for(ib=ib1  */
                fprintf(fp1,"\n cormax= %7.2f",cormax);
		if (cormax > 0.) {
		  if (Glob->params.rng_skip > 1) {
		    ib3 = ibmx - 1;
		    ib4 = ibmx + 1;
		    ia3 = iamx - Glob->params.rng_skip + 1;
		    ia4 = iamx + Glob->params.rng_skip - 1;
		    for(ib=ib3; ib<=ib4; ib++) {

		      iib2 = ib+isizb-1;
		      iib2 = MIN(iib2, num_azim - 1);
		      
		      angl2 = azim[ib+ioffb];
		      for(ia=ia3; ia<=ia4; ia++) {
			a2 = range[ia+ioffa]*cosel;
			iia2 = ia+isiza-1;
			iia2 = MIN(iia2, num_gate - 1);
		    
			x2 = a2*sin(angl2);
			y2 = a2*cos(angl2);
			sum2 = 0;
			sumsq2 = 0;
			sum12 = 0;
			for(iib=ib; iib<=iib2; iib++) {
			  for(iia=ia; iia<=iia2; iia++) {
			    d=dbz1[iel][iib][iia];
			    sum2 += d;
			    sumsq2 += d*d;
			    sum12 += base[iib-ib][iia-ia]*d;
			  }
			}
			s12 = (float)sum12;
			s1 = (float)sum1;
			s2 = (float)sum2;
			ssq1 = (float)sumsq1;
			ssq2 = (float)sumsq2;
			corcoef = (s12-rninv*s1*s2)/
			  sqrt((ssq1-rninv*s1*s1)*
			       (ssq2-rninv*s2*s2));
#ifdef NOTNOW
			store_cor[ia-ia1][ib-ib3] = corcoef;
#endif
			if(corcoef > cormax) {
			  cormax = corcoef;                
			  iamx = ia;
			  ibmx = ib;
			}
		      } /* for(ia=ia3  */
		    } /* for(ib=ib3  */
		  } /* if (Glob->params.rng_skip > 1) */

#ifdef NOTNOW
		  vcterp(store_cor,range,iamx,azim,ibmx,ia1,ib3,ioffa,ioffb,&xpa,&xpb);
#endif		  

		  xpa = range[iamx+ioffa];
		  xpb = azim[ibmx+ioffb];

		  xnd = xpa*cosel*sin(xpb*0.0174532);
		  ynd = xpa*cosel*cos(xpb*0.0174532);
		  u[nvec] = (xnd-x1)/dt*(1000./60.);
		  v[nvec] = (ynd-y1)/dt*(1000./60.);


		  /*		  
		  if (Glob->params.use_synthetic_data) {
		    u[nvec] = 15.*cos(2.*PI*x[nvec]/50.);
		    v[nvec] = 0.;
		  }

		  */

		  rg = sqrt(x1*x1+y1*y1);
		  vrt = u[nvec]*x1/rg+v[nvec]*y1/rg;
		  vtan= -u[nvec]*y1/rg+v[nvec]*x1/rg;
		  vdif = fabs(vrt-avgd);
		  if(avgd == -BAD)
		    vdif=0.;

#ifdef NOTNOW
		  u[nvec]=avgd*x1/rg-vtan*y1/rg;
		  v[nvec]=avgd*y1/rg+vtan*x1/rg;
		  vrt = u[nvec]*x1/rg+v[nvec]*y1/rg;
		  vdif = fabs(vrt-avgd);
#endif
		  dop[nvec]=avgd;



                  fprintf(fp1,
			  "\n n,x,y,u,v,cor,var,vdif,avgd= %6d %6.1f "
			  "%6.1f %6.1f %6.1f %5.2f %6.1f %6.1f %6.1f",
			  nvec,x1,y1,u[nvec],v[nvec],
			  cormax,var,vdif,avgd);

		  if (cormax < Glob->params.cor_min_thr || 
		      cormax > Glob->params.cor_max_thr ||
		      vdif > Glob->params.vel_dif_thr) {
		    cormax = -BAD;
		  } else {
		    uu += u[nvec];
		    vv += v[nvec];



                    fprintf(fp1,"\n x,y,u,v= %6.1f %6.1f %6.1f %6.1f",x1,y1,
                      u[nvec],v[nvec]);

		    if(avgd > -50. ){
		      numdif++;
		      sumdif +=avgd;
		      if(avgd > velmax)
			velmax=avgd;
		      if(avgd < velmin)
			velmin=avgd;
		    }
		    ibin= (int)vdif;
		    
		    sumc += cormax;
		    nc++;

		    if (nvec < max_vec - 1) {
		      nvec++;
		    }

		  } 
		}	/* if(cormax > 0.)  */
	      } /* if (var <= Glob->params.var_thr) */
	    } /* if(numpts/        */
	  } /* if (ib3 != -1)  */
	} /* if (x1 >= dim_data->min_x */
	ridxb += riterb;
	idxb = (int)ridxb;
      } /* while(idxb<lastb) */
      ridxa += ritera;
      idxa = (int)ridxa;
    } /* while(idxa<lasta) */
    
    if(nc > 0) {
      avgu = uu/nc;
      avgv = vv/nc;
      avgcor = sumc/nc;
    }
    
    iaz2[iel] = iaz1[iel]+nc-1;
    iaz1[iel+1] = iaz2[iel]+1;

  } /* for(iel=0; iel < num_elev; iel++) */

  *num_vec=nvec;
  fclose(fp1);
  
  if (Glob->params.debug) {
    printf( "%s:    exiting tkcomp with a total num of vectors = %d\n", 
             Glob->prog_name, nvec );

    //
    // print out min and max for vectors
    //
    float umin, vmin, dmin;
    float umax, vmax, dmax;

    umin = vmin = dmin =  32767;
    umax = vmax = dmax = -32768;

    for( i=0; i < nvec; i++ ) {
       if ( u[i] < umin )
          umin = u[i];                                                 
       if ( u[i] > umax )
          umax = u[i];
       if ( v[i] < vmin )
          vmin = v[i];                                                 
       if ( v[i] > vmax )
          vmax = v[i];
       if ( dop[i] < dmin )
          dmin = dop[i];                                                 
       if ( dop[i] > dmax )
          dmax = dop[i];
    }
    printf( "%s:    %s:\tmin = %.2f\tmax = %.2f\n" 
            "%s:    %s:\tmin = %.2f\tmax = %.2f\n" 
            "%s:    %s:\tmin = %.2f\tmax = %.2f\n",
             Glob->prog_name, "u-motion",
             umin, umax,
             Glob->prog_name, "v-motion",
             vmin, vmax,
             Glob->prog_name, "avg-dop ",
             dmin, dmax );
  }
} /*end routine */

