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
 * tkcomp
 *
 * Main trec routine - this computes the X-correlation values
 * on a grid. A 'window' of set size is moved around, using
 * a set jump interval. If a good correlation is found, a vector
 * is set in u, v, w, x, y, z, and dop.
 *
 ****************************************************************************/

#include "trec.h"

void tkcomp(dimension_data_t *dim_data,
	    float ***az,
	    float ***el,
	    int nel,
	    int **el_time,
	    int k1,
	    int k2,
	    int **num_az,
	    double vel_scale,
	    double vel_bias,
	    double dbz_scale,
	    double dbz_bias,
	    float *u,
	    float *v,
	    float *x,
	    float *y,
	    float *z,
	    float *dop,
	    int max_vec,
	    int *num_vec,
	    float *rng,
	    int num_gates,
	    int *iaz1,
	    int *iaz2,
	    int date,
	    ui08 ****vel,
	    ui08 ****dbz,
	    ui08 ***flg)

{

  static int first_call = TRUE;
  static long **base;
  static float **store_cor;
  static ui08 *numnoi;
  
  long d,idbz_max;
  long sum1,sum2,sumsq1,sumsq2,sum12,sumv,sumsqv;
  float uu,vv,azsum,avgangl,delaz,cosel1,cosel2,deltaa,ritera,tkmove;
  float radsq,ref;
  float s1,s2,s12,ssq1,ssq2;
  float dt,sinel1,sinel2,var,avgd,corcoef,cormax,vrt,vdif;
  float ridxa,ridxb,a1,riterb,rninv,az2,angl1,x1,x2,z1,r1,y1,y2,rg;
  float dtsecx,dtsecy,a2,sinang,cosang,angl2;
  float vrad,xpa,xpb,xnd,ynd,sumc,avgu,avgv,avgcor,deltab;
  float avg_sumv,avg_sqv,numpts,fraction,vtan,numst,fractst;
  float nvpts,half_npts;
  int i,j,k,iel,idista,isiza,ioffa,lasta,nvec;
  int idxa,idxb,ia1,ia2,ia3,ia4,lastb,ib3,ib4,iib2,iia2,ia,ib;
  int iamx,ibmx,nc,idistb,isizb,ioffb,iib,iia;
  int iref,numdif;
  int ndop,nbin[50],ibin;
  int vbyte;
  float percent[50],sump,velmin,velmax,sumdif;
  FILE *fp,*sp;
  char output_file_path[MAX_PATH_LEN];
  char stats_file_path[MAX_PATH_LEN];

  if (Glob->params.debug) {
    fprintf(stderr, "*** tkcomp ***\n");
  }

  PMU_auto_register("starting tkcomp");

  /*
   * allocate memory
   */
  
  if (first_call) {
    base = (long **) umalloc2(200, 200, sizeof(long));
    store_cor = (float **) umalloc2(200, 200, sizeof(float));
    numnoi = (ui08 *) umalloc(Glob->params.max_naz * sizeof(ui08));
    first_call = FALSE;
  }

  iaz1[0]=1;

  sprintf(stats_file_path, "%s%s%s", Glob->params.local_dir, PATH_DELIM, 
	  "stats.out");

  if ((sp = fopen(stats_file_path,"w")) == NULL) {
    fprintf(stderr, "ERROR - %s:tkcomp\n", Glob->prog_name);
    fprintf(stderr, "Unable to create file %s\n", stats_file_path);
    perror(stats_file_path);
    tidy_and_exit(-1);
  }
  
  sprintf(output_file_path, "%s%s%s", Glob->params.local_dir, PATH_DELIM, 
	  "trec.out");
  
  if ((fp = fopen(output_file_path,"w")) == NULL) {
    fprintf(stderr, "ERROR - %s:tkcomp\n", Glob->prog_name);
    fprintf(stderr, "Unable to create file %s\n", output_file_path);
    perror(output_file_path);
    tidy_and_exit(-1);
  }
  
  nvec=0;
  idbz_max = (Glob->params.dbz_max - dbz_bias) / dbz_scale; 
  if (Glob->params.debug) {
    fprintf(stderr, "idbz= %ld \n",idbz_max);
  }

  for(iel=0; iel<=nel; iel++) {

    PMU_auto_register("tkcomp - computing");

    velmin=999.;
    velmax=-999.;
    if (Glob->params.debug) {
      fprintf(fp,"iel,k1,k2= %d %d %d\n",iel,k1,k2);
    }
    ndop=0;
    for(i=0; i<50; i++) {
      nbin[i]=0;
    }

    radius(el_time,iel,k1,k2,&tkmove,&dt);
    
    if (Glob->params.debug) {
      fprintf(stderr, "trecing elevation = %6.1f\n", el[0][iel][k1]);
      fprintf(stderr, "iel,el_time1,el_time2,dt= %d %d %d %6.1f\n", iel, 
	      el_time[iel][k1], el_time[iel][k2], dt);
      fprintf(fp,"el_time1,el_time2,date,el,tkmove,dt= "
	      "%7d %7d %7d %6.1f %6.1f %6.1f\n",
	      el_time[iel][k1],
	      el_time[iel][k2],date,el[50][iel][k1],tkmove,dt);
      fprintf(fp,"ngates= %d\n",num_gates);
      if(iel==0)
	fprintf(stderr, "num_az,k1= %d %d\n", num_az[iel][k1], k1);
    }
      
    azsum = 0.;

    for(i=0; i<num_az[iel][k1]-1; i++) {
      numnoi[i]=0;
      delaz=fabs(az[i][iel][k1]-az[i+1][iel][k1]);
      if(delaz > 180.)delaz=360.-delaz;
      azsum += delaz;
      if (Glob->params.debug) {
	fprintf(fp,"i,iel,az,el= %5d %5d %6.1f %6.1f\n",
		i,iel,az[i][iel][k1],
		el[i][iel][k1]);
      }
    }

    i=num_az[iel][k1];
    numnoi[i]=0;
    avgangl=azsum/num_az[iel][k1];
    if (Glob->params.debug) {
      fprintf(stderr, "naz,avgangl= %d %6.1f\n", 
	      num_az[iel][k1], avgangl);
    }
    uu=0.;
    vv=0.;
    nc=0;
    sumdif=0.;
    sumc=0.;
    numdif=0;
    cosel1 = cos(el[0][iel][k1]*PI/180.);
    cosel2 = cos(el[0][iel][k2]*PI/180.);
    sinel1 = sin(el[0][iel][k1]*PI/180.);
    sinel2 = sin(el[0][iel][k2]*PI/180.);
    deltaa = Glob->params.gate_spacing * cosel1;
    idista = tkmove / deltaa;    
    isiza = Glob->params.box_size / deltaa + 1.5;

    ioffa = (isiza-1) * 0.5;    
    ritera = Glob->params.box_spacing / deltaa;
    if (ritera < 1.) 
      ritera = 1.;
    lasta=num_gates-idista-isiza+1;  
    ridxa=idista+1.0+5.0/deltaa;
    idxa=ridxa;

    while(idxa<lasta) { 

      PMU_auto_register("tkcomp - computing");

      r1 = rng[idxa+ioffa];
      a1=rng[idxa+ioffa]*cosel2;
      ia1 = idxa - idista;
      ia2 = idxa + idista;
      deltab = 2*a1*sin(avgangl*(0.5*PI/180.));
      idistb = tkmove/deltab;
      isizb = Glob->params.box_size / deltab + 1.5;    
      
      /*    IF(MOD(ISIZB,2).EQ.0)ISIZB=ISIZB+1    */

      ioffb = (isizb-1)*0.5;
      riterb = Glob->params.box_spacing / deltab;
      if(riterb < 1.0)riterb=1.;
      lastb = num_az[iel][k2]-idistb-isizb+1;
      rninv = 1./(isiza*isizb);
      half_npts=0.5*(float)(isiza*isizb);
      ridxb = idistb+1.;
      idxb = ridxb;
      
      while(idxb < lastb) {
                                                       
	PMU_auto_register("tkcomp - computing");

	angl1 = az[idxb+ioffb][iel][k2]*0.0174532;
	x1 = a1*sin(angl1);
	y1 = a1*cos(angl1);          
	/* specify r1 and sinel1 */
	z1 = r1 * sinel1 + r1 * r1 / 17014. + Glob->params.radar_altitude;

	if(x1 >= dim_data->min_x && x1 <= dim_data->max_x &&
	   y1 >= dim_data->min_y && y1 <= dim_data->max_y) {

	  az2 = az[idxb][iel][k2];

	  azindx(az2,az,idistb,iel,k1,num_az[iel][k1],&ib3,&ib4,isizb);
	  
	  if(ib3 != -1) {
         
	    cormax=-999.;
	    numpts = 0.;
	    numst = 0.;
	    for(j=0; j<isizb; j++) {
	      for(k=0; k<isiza; k++) {
		base[j][k] = dbz[k2][iel][idxb+j][idxa+k];
		if(base[j][k] > idbz_max)
		  numpts += 1.0;
		if(flg[iel][idxb+j][idxa+k] == 2)
		  numst += 1.0;
	      }
	    }
	    ref = dbz_scale * (float) dbz[k2][iel][idxb+ioffb][idxa+ioffa] + 
	      dbz_bias;
	    iref = dbz[k2][iel][idxb+ioffb][idxa+ioffa];
	    fraction = numpts / ((float)(isiza*isizb));
	    fractst  = numst / ((float)(isiza*isizb));  
	    
	    if (fraction <= Glob->params.fract &&
		fractst <= Glob->params.fractst) {
	      rg=sqrt(x1*x1+y1*y1);
	      dtsecx=16.6667*x1/(rg*dt);
	      dtsecy=16.6667*y1/(rg*dt);
	      sum1 = 0;
	      sumsq1 = 0;
	      sumv=0;
	      sumsqv=0;
	      nvpts=0.;
	      for(j=0; j < isizb; j++) {
		for(i=0; i < isiza; i++) {
		  d=base[j][i];
		  vbyte = vel[k2][iel][idxb+j][idxa+i];
		  if(vbyte != 0){
		    sumv += vbyte;
		    sumsqv += vbyte * vbyte;
		    nvpts +=1.;
		  }
		  
		  sum1 += d;
		  sumsq1 += d*d;
		}
	      }
	      x[nvec] = x1;
	      y[nvec] = y1;
	      z[nvec] = z1;
	      
	      if(nvpts > half_npts){
		avg_sumv = (float)sumv/nvpts;
		avg_sqv = (float)sumsqv/nvpts;
		avgd = (float)vel_scale*(sumv/nvpts)+vel_bias;
		var  = (avg_sqv - avg_sumv*avg_sumv)*vel_scale*vel_scale;
	      } else {
		avgd = -999.;
		var = -999.;
	      }
	      
	      if (var <= Glob->params.var_thr) {
		for(ib=ib3; ib<=ib4; ib++) {
		  iib2 = ib + isizb-1;
		  angl2 = az[ib+ioffb][iel][k1]*0.01745329;
		  sinang = sin(angl2);
		  cosang = cos(angl2);
		  for (ia = ia1; ia <= ia2; ia += Glob->params.rng_skip) {                                       
		    a2 = rng[ia+ioffa]*cosel2; 
		    iia2 = ia +isiza -1;

		    x2 = a2*sinang;
		    y2 = a2*cosang;
		    radsq=(x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
		    if(radsq <= tkmove*tkmove) {
		      vrad = (x2-x1) * dtsecx + (y2-y1) * dtsecy;
		      if ((vrad >= (avgd - Glob->params.vel_rad) && 
			   vrad <= (avgd + Glob->params.vel_rad)) ||
			  avgd<-900.) {
			sum2=0;
			sumsq2 = 0;
			sum12 = 0;
			for(iib=ib; iib<=iib2; iib++) {
			  for(iia=ia; iia<=iia2; iia++) {
			    d=dbz[k1][iel][iib][iia];
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

		if (cormax > 0.) {
		  if (Glob->params.rng_skip > 1) {
		    ib3 = ibmx - 1;
		    ib4 = ibmx + 1;
		    ia3 = iamx - Glob->params.rng_skip + 1;
		    ia4 = iamx + Glob->params.rng_skip - 1;
		    for(ib=ib3; ib<=ib4; ib++) {
		      iib2 = ib+isizb-1;
		      
		      angl2 = az[ib+ioffb][iel][k1];
		      for(ia=ia3; ia<=ia4; ia++) {
			a2 = rng[ia+ioffa]*cosel2;
			iia2 = ia+isiza-1;

			x2 = a2*sin(angl2);
			y2 = a2*cos(angl2);
			sum2 = 0;
			sumsq2 = 0;
			sum12 = 0;
			for(iib=ib; iib<=iib2; iib++) {
			  for(iia=ia; iia<=iia2; iia++) {
			    d=dbz[k1][iel][iib][iia];
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
		  vcterp(store_cor,rng,iamx,az,ibmx,ia1,ib3,
			 ioffa,ioffb,iel,k1,k2,&xpa,&xpb);
#endif		  

		  xpa = rng[iamx+ioffa];
		  xpb = az[ibmx+ioffb][iel][k1];

		  xnd = xpa*cosel1*sin(xpb*0.0174532);
		  ynd = xpa*cosel1*cos(xpb*0.0174532);
		  u[nvec] = (xnd-x1)/dt*(1000./60.);
		  v[nvec] = (ynd-y1)/dt*(1000./60.);
		  
#ifdef NOTNOW
		  if (Glob->params.anal_fun) {
		    amp= 7.*z[nvec];
		    u[nvec] = amp*cos(2.*PI*x[nvec]/50.);
		    v[nvec] = 0.;
		  }
#endif

		  rg = sqrt(x1*x1+y1*y1);
		  vrt = u[nvec]*x1/rg+v[nvec]*y1/rg;
		  vtan= -u[nvec]*y1/rg+v[nvec]*x1/rg;
		  vdif = fabs(vrt-avgd);
		  if(avgd == -999.)
		    vdif=0.;

#ifdef NOTNOW
		  u[nvec]=avgd*x1/rg-vtan*y1/rg;
		  v[nvec]=avgd*y1/rg+vtan*x1/rg;
		  vrt = u[nvec]*x1/rg+v[nvec]*y1/rg;
		  vdif = fabs(vrt-avgd);
#endif
		  dop[nvec]=avgd;
		  
		  if (cormax < Glob->params.cor_min_thr || 
		      cormax > Glob->params.cor_max_thr ||
		      vdif > Glob->params.vel_dif_thr) {
		    cormax = -999.;
		    if (Glob->params.debug) {
		      fprintf(fp,"bad vector-x,y,cor,var,vdif,avgd= "
			      "%6.1f %6.1f %5.2f %6.1f %6.1f %6.1f\n",
			      x1,y1,cormax,var,vdif,avgd);
		    }

		  } else {

		    if (Glob->params.debug) {
		      fprintf(fp,"n,x,y,u,v,cor,var,vdif,avgd,ref= "
			      "%6d %6.1f %6.1f %6.1f %6.1f %5.2f "
			      "%6.1f %6.1f %6.1f %6.1f\n",
			      nvec,x1,y1,u[nvec],v[nvec],cormax,
			      var,vdif,avgd,ref);
		    }
		    
		    uu += u[nvec];
		    vv += v[nvec];
		    
		    if(avgd > -50. ){
		      numdif++;
		      sumdif +=avgd;
		      if(avgd > velmax)
			velmax=avgd;
		      if(avgd < velmin)
			velmin=avgd;
		    }
		    ibin= (int)vdif;
		    if(avgd != -999.){
		      nbin[ibin]++;
		      ndop++;
		    }
		    
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
	idxb = ridxb;
      } /* while(idxb<lastb) */
      ridxa += ritera;
      idxa = ridxa;
    } /* while(idxa<lasta) */
    
    if(nc > 0) {
      avgu = uu/nc;
      avgv = vv/nc;
      avgcor = sumc/nc;
      for(i=0; i<50; i++){
	sump=0.;
	for(j=i; j<50; j++){
	  sump = sump + (float)nbin[j];
	  
	}
	percent[i]=100.*sump/(float)ndop;
      }

      if (Glob->params.debug) {
	fprintf(stderr, "percent 0,1,2,3,4,= %6.0f %6.0f %6.0f %6.0f %6.0f\n",
		percent[0],percent[1],percent[2],percent[3],percent[4]);
	fprintf(sp,"%7d %7d %7d %6.0f %6.0f %6.0f %6.0f "
		"%6.0f %6.0f %6.1f %6.1f %6.1f %5d %5.2f %5d %6.1f\n",
		el_time[iel][k1],el_time[iel][k2],date,percent[0],
		percent[1],percent[2],percent[3],percent[4],
		percent[5],velmin,velmax,el[0][iel][k1],
		nc,avgcor,numdif,sumdif/numdif);
	fprintf(stderr,
		"avg u= %6.1f  avg v= %6.1f nvec= %5d avg cor= %6.2f\n",
		avgu,avgv,nc,avgcor);
      }
    }
    
    iaz2[iel] = iaz1[iel]+nc-1;
    iaz1[iel+1] = iaz2[iel]+1;

  } /* for(iel=0; iel<nel; iel++) */

  *num_vec=nvec;
  
  if (Glob->params.debug) {
    fprintf(stderr, "total num of vectors= %5d\n",nvec);
  }

  fclose(fp);
  fclose(sp);
  
} /*end routine */


