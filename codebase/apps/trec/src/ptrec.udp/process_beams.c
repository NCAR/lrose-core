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
 * process_beams.c
 *
 * reads in the beams, process and write to file
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * July 1991
 *
 ****************************************************************************/

#include "trec.h"
#include <sys/times.h>
#include <stdlib.h>

void process_beams(void)

{

  ui08 ****vel;
  ui08 ****dbz;
  ui08 ***flg;
 
  int **el_time;
  int **num_az;
  
  float ***az, ***el;
  float ***ucart, ***vcart, ***wcart;
  float ***conv, ***dcart;
  float *u, *v, *x, *y, *z, *dop;
  float *rng;
  float faz;

  dimension_data_t dim_data;

  int i,j,k;
  int forever = TRUE;
  int newvol,nel,naz,nvol,tiltold,k1,k2,tmp;
  int nvec,badscan;
  int iaz1[10],iaz2[10],date;
  int num_gates;
  int nx_vec, ny_vec, max_vec;

  float elevation,azang;
  float tkmove, dt;             
  float vel_bias, elold, dbz_bias;
  float cosel, cosaz, sinaz, x1, y1, r1;
  double gate_spacing;

  double vel_scale,dbz_scale;
  double before, after;
  
  char *beam_buf;
  ui08 *vel_ptr;
  ui08 *dbz_ptr;
  ui08 *flg_ptr;
  ll_params_t *beam;

  date_time_t sample_time;

  if (Glob->params.debug) {
    fprintf(stderr, "*** process_beams ***\n");
  }

  newvol=FALSE;

  k1=0;
  k2=1;
  tiltold=-1;
  nel=0; 
  nvol=0;
  naz=AZ_OFF;
  badscan = FALSE;
  elold = -99.;

  /*
   * set up dimension structure
   */
  
  dim_data.min_x = (short)Glob->params.grid_min_x;
  dim_data.min_y = (short)Glob->params.grid_min_y;
  dim_data.min_z = Glob->params.grid_min_z;
  
  dim_data.del_x = (short)Glob->params.grid_del_x;
  dim_data.del_y = (short)Glob->params.grid_del_y;
  dim_data.del_z = Glob->params.grid_del_z;
  
  dim_data.nx = Glob->params.grid_nx;
  dim_data.ny = Glob->params.grid_ny;
  dim_data.nz = Glob->params.grid_nz;
  
  dim_data.max_x = dim_data.min_x + dim_data.del_x * (dim_data.nx - 1);
  dim_data.max_y = dim_data.min_y + dim_data.del_y * (dim_data.ny - 1);
  dim_data.max_z = dim_data.min_z + dim_data.del_z * (dim_data.nz - 1);
  
  if (Glob->params.debug) {

    fprintf(stderr, "x1,x2,delx= %g %g %g\n", dim_data.min_x,
	    dim_data.max_x, dim_data.del_x);    
    
    fprintf(stderr, "y1,y2,dely= %g %g %g\n", dim_data.min_y,
	    dim_data.max_y, dim_data.del_y);    
    
    fprintf(stderr, "z1,z2,delz= %6.1f %6.1f %6.1f\n", dim_data.min_z,
	    dim_data.max_z, dim_data.del_z);    

    clock();

  }

  /*
   * compute max number of vectors possible, allowing an extra 25%
   * in x and y dimensions for safety
   */

  nx_vec = ((dim_data.max_x - dim_data.min_x) /
	    Glob->params.box_spacing) * 1.25;
  
  ny_vec = ((dim_data.max_y - dim_data.min_y) /
	    Glob->params.box_spacing) * 1.25;

  max_vec = nx_vec * ny_vec * dim_data.nz;
  
  /*
   * allocate arrays
   */
  
  vel = (ui08 ****) umalloc(2 * sizeof(ui08 ***));
  
  vel[0] = (ui08 ***) umalloc3(Glob->params.nel,
			       Glob->params.max_naz,
			       Glob->params.ngates,
			       sizeof(ui08));
  
  vel[1] = (ui08 ***) umalloc3(Glob->params.nel,
			       Glob->params.max_naz,
			       Glob->params.ngates,
			       sizeof(ui08));
  
  dbz = (ui08 ****) umalloc(2 * sizeof(ui08 ***));
  
  dbz[0] = (ui08 ***) umalloc3(Glob->params.nel,
			       Glob->params.max_naz,
			       Glob->params.ngates,
			       sizeof(ui08));
  dbz[1] = (ui08 ***) umalloc3(Glob->params.nel,
			       Glob->params.max_naz,
			       Glob->params.ngates,
			       sizeof(ui08));
  
  flg = (ui08 ***) umalloc3(Glob->params.nel,
			    Glob->params.max_naz,
			    Glob->params.ngates, 
			    sizeof(ui08));
  
  az = (float ***) umalloc3(Glob->params.max_naz,
			    Glob->params.nel, 2,
			    sizeof(float));
  
  el = (float ***) umalloc3(Glob->params.max_naz,
			    Glob->params.nel, 2,
			    sizeof(float));
  
  ucart = (float ***) umalloc3(dim_data.nx, dim_data.ny,
			       dim_data.nz, sizeof(float));
  vcart = (float ***) umalloc3(dim_data.nx, dim_data.ny,
			       dim_data.nz, sizeof(float));
  wcart = (float ***) umalloc3(dim_data.nx, dim_data.ny,
			       dim_data.nz, sizeof(float));
  conv = (float ***) umalloc3(dim_data.nx, dim_data.ny,
			      dim_data.nz, sizeof(float));
  dcart = (float ***) umalloc3(dim_data.nx, dim_data.ny,
			       dim_data.nz, sizeof(float));
  
  el_time = (int **) umalloc2(Glob->params.nel, 2, sizeof(int));
  num_az = (int **) umalloc2(Glob->params.nel, 2, sizeof(int));
  
  u = (float *) umalloc(max_vec * sizeof(float));
  v = (float *) umalloc(max_vec * sizeof(float));
  x = (float *) umalloc(max_vec * sizeof(float));
  y = (float *) umalloc(max_vec * sizeof(float));
  z = (float *) umalloc(max_vec * sizeof(float));
  dop = (float *) umalloc(max_vec * sizeof(float));

  rng = (float *) umalloc(Glob->params.ngates * sizeof(float));
  
  /*
   * set up range array
   */

  for(i=0; i< Glob->params.ngates; i++) {
    rng[i] = (i + 0.5) * Glob->params.gate_spacing + 0.001;
  }                                 
  
  if (Glob->params.debug) {
    fprintf(stderr, "initialized range array\n");
  }

  if (Glob->params.anal_fun) {
    fprintf(stderr, "*** warning - analytic functions being used ***\n");
  }

  while (forever) {
    
    /*
     * get the next beam
     */

    if (Glob->params.udp_pkt_format == NCAR_UDP) {
      beam_buf = read_ncar_udp_beam (Glob->params.udp_port,
				     (Glob->params.debug >= DEBUG_VERBOSE),
				     Glob->prog_name);
    } else {
      beam_buf = read_ll_udp_beam (Glob->params.udp_port,
				   (Glob->params.debug >= DEBUG_VERBOSE),
				   Glob->prog_name);
    }

    beam = (ll_params_t *) beam_buf;

    /*
     * check the gate spacing (km)
     */
    
    gate_spacing = (double) beam->range_seg[0].gate_spacing / 1000.0;

    if (fabs(gate_spacing - Glob->params.gate_spacing) > 0.001) {
      continue;
    }

    /*
     * correct time if necessary
     */

    adjust_time(beam, Glob->params.time_correction);

    /*
     * adjust azimuth for grid rotation if necessary
     */

    if (fabs(Glob->params.grid_rotation) > 0.01) {
      faz = ((float) beam->azimuth / 100.0 -
	     Glob->params.grid_rotation);
      if (faz < 0.0) {
	faz += 360.0;
      } else if (faz > 360.0) {
	faz -= 360.0;
      }
      beam->azimuth = (int) (faz * 100.0 + 0.5);
    }
    
    /*
     * print beam if required
     */

    print_beam(beam);
    
    /*
     * compute the elevation index and ngates
     */

    elevation =  (double) beam->elevation / 100.;
    azang = (double) beam->azimuth / 100.;
    
    /*
     * check for beginning of new volume
     */

    if (beam->tilt_num <= 2)
      newvol=TRUE;

    /*
     * if this is a high prf scan, process data
     */

    if (newvol) {
        
      /* 
       * check for a new scan
       */
      
      if (beam->tilt_num != tiltold) {

	if (Glob->params.debug) {
	  fprintf(stderr, "new scan: date,time,el,tiltnum= "
		  "%2d%2d%2d %2d%2d%2d %6.1f %6d\n",
		  beam->year, beam->month, beam->day, beam->hour, beam->min,
		  beam->sec, elevation, beam->tilt_num);
	}
           
	/*
	 * if elevation angle is larger than the max angle to process
	 * go do correlation analysis, else reset some counters and 
	 * start processing the next scan
	 */
	
	if (nel >= ((Glob->params.nel) - 1)) elevation=99.;
	
	if(elevation > elold) {

	  badscan = FALSE;
	  
	  if(elevation > Glob->params.el_max) {     

	    newvol = FALSE;                              
	    nvol++;
	    if (Glob->params.debug) {
	      fprintf(stderr, "end of volume %3d\n", nvol);
	    }
	    num_az[nel][k1]=naz;
	    num_gates = MIN(beam->range_seg[0].gates_per_beam,
			    Glob->params.ngates);

	    for(j=0; j<=AZ_OFF; j++) {

	      el[j][nel][k1]=el[naz-AZ_OFF+j][nel][k1];
	      az[j][nel][k1]=az[naz-AZ_OFF+j][nel][k1];
	      for(i=0; i< num_gates; i++){
		dbz[k1][nel][j][i]=dbz[k1][nel][naz-AZ_OFF+j][i];
		vel[k1][nel][j][i]=vel[k1][nel][naz-AZ_OFF+j][i];

	      } /* j */

	    } /* if(elevation > Glob->params.elmax) */

	    /*
	     * if tracking velocity field, despike data 
	     */

	    if (Glob->params.debug) {
	      before =  clock() / 1000000.0;
	    }

	    if (Glob->params.track_fld == 2) {
	      for (k = 0; k <= nel; k++) {
		despike(&dim_data,
			k1,k,num_az[k][k1],num_gates,
			vel_scale,vel);
		for(j=0; j<=num_az[k][k1]; j++){
		  for(i=0; i<num_gates; i++){
		    dbz[k1][k][j][i]=vel[k1][k][j][i];
		  } /* i */
		} /* j */
	      } /* k */
	    } /* if (Glob->params.track_fld == 2) */

	    if (Glob->params.debug) {
	      after =  clock() / 1000000.0;
	      fprintf(stderr, "despike took %g CPU secs\n",
		      after - before);
	    }

	    /*
	     * replace missing or thesholded data (=0) with random data
	     * This prevents the data from creating artifacts
	     */

	    for(k=0; k<=nel; k++){
	      for(j=0; j<=num_az[k][k1]; j++){
		for(i=0; i<num_gates; i++){
		  flg[k][j][i]=0;
		  if(dbz[k1][k][j][i] == 0){
		    dbz[k1][k][j][i]=20 + ((ui08)(random()))/10;
		    flg[k][j][i]=2;
		  }
		}
	      }
	    }
		 
	    if (Glob->params.debug) {
	      fprintf(stderr, "nel=%5d naz=%5d el=%6.1f k1=%4d\n", nel,
		      num_az[nel][k1], el[50][nel][k1], k1);
	    }
	    
	    if(nvol > 1) {

	      /*
	       * compute search radius
	       */
	      
	      radius(el_time,nel,k1,k2,&tkmove,&dt);

	      if (Glob->params.debug) {
		fprintf(stderr, "nel,el_time1,el_time2,dt= %d %d %d %6.1f\n",
			nel, el_time[nel][k1], el_time[nel][k2], dt);
	      }

	      if(dt <= 11.0) {

		if (Glob->params.debug) {
		  fprintf(stderr, "k1,k2,dt= %d %d %6.1f\n", k1, k2, dt);
		}

		vel_scale=(double) beam->scale[Glob->params.vel_field_pos]
		  / 100.0;
		vel_bias = beam->bias[Glob->params.vel_field_pos]/100.;
		dbz_scale=(double) beam->scale[Glob->params.dbz_field_pos]
		  / 100.0;
		dbz_bias= beam->bias[Glob->params.dbz_field_pos]/100.;
		
		if (Glob->params.debug) {
		  fprintf(stderr, "dbzsc,dbzbias,velsc,velbias= "
			  "%6.1f %6.1f %6.1f %6.1f\n",
			  dbz_scale,dbz_bias,vel_scale,vel_bias);
		}

		num_gates = MIN(beam->range_seg[0].gates_per_beam,
			       Glob->params.ngates-10);

		date = 10000*beam->year+100*beam->month+beam->day;

		/*
		 * perform x-correlation in polar space
		 */
		
		if (Glob->params.debug) {
		  before =  clock() / 1000000.0;
		}

		tkcomp(&dim_data,
		       az,el,nel,el_time,k1,k2,num_az,vel_scale,
		       vel_bias,dbz_scale,dbz_bias,u,v,x,y,z,
		       dop,max_vec,&nvec,rng,num_gates,
		       iaz1,iaz2,date,vel,dbz,flg);

		if (Glob->params.debug) {
		  after =  clock() / 1000000.0;
		  fprintf(stderr, "tkcomp took %g CPU secs\n",
			  after - before);
		  before = after;
		}

		/*
		 * resample polar data onto cart grid using least-squares
		 */

		lstsq(&dim_data,u,x,y,z,nvec,ucart,iaz1,iaz2,nel,el);
		lstsq(&dim_data,v,x,y,z,nvec,vcart,iaz1,iaz2,nel,el);
		lstsq(&dim_data,dop,x,y,z,nvec,dcart,iaz1,iaz2,nel,el);

		if (Glob->params.debug) {
		  after =  clock() / 1000000.0;
		  fprintf(stderr, "lstsq took %g CPU secs\n",
			  after - before);
		  before = after;
		}

		/*
		 * compute w - vertical velocity - from
		 * u and v
		 */
		
		wair(&dim_data,ucart,vcart,wcart,conv);

		if (Glob->params.debug) {
		  after =  clock() / 1000000.0;
		  fprintf(stderr, "wair took %g CPU secs\n",
			  after - before);
		}

		/*
		 * write file
		 */
		
		write_ced_file(&sample_time,beam,&dim_data,ucart,vcart,
			       wcart,conv,dcart);
		
	      } /* if(dt <= 11.0) */
	      
	    } /* if (nvol > 1 */

	    nel=0;
	    elold = -99.; 
	    tiltold=-1;
	    tmp=k1;
	    k1=k2;
	    k2=tmp;

	  } else { /* if (elevelation > ... */

	    num_az[nel][k1]=naz;
	    if (Glob->params.debug) {
	      fprintf(stderr, "nel=%5d naz=%5d el=%6.1f k1=%4d\n", nel,
		      num_az[nel][k1], el[50][nel][k1], k1);
	    }
	    /*
	     * add data to polar arrays
	     */

	    if(tiltold != -1) {
	      for(j=0; j<=AZ_OFF; j++) {
		el[j][nel][k1]=el[naz-AZ_OFF+j][nel][k1];
		az[j][nel][k1]=az[naz-AZ_OFF+j][nel][k1];
		for(i=0; i<num_gates; i++) {
		  dbz[k1][nel][j][i]=dbz[k1][nel][naz-AZ_OFF+j][i];
		  vel[k1][nel][j][i]=vel[k1][nel][naz-AZ_OFF+j][i];
		}
	      }
	      nel++;
	    }
	    naz=AZ_OFF;
	    tiltold=beam->tilt_num;
	    elold = elevation;
	    el_time[nel][k1]=10000*beam->hour+100*beam->min+beam->sec;
	    if (Glob->params.debug) {
	      fprintf(stderr, "nel,k1,el_time= %d %d %d\n", nel, k1, 
		      el_time[nel][k1]);
	    }
	    sample_time.year=beam->year;
	    sample_time.month=beam->month;
	    sample_time.day=beam->day;
	    sample_time.hour=beam->hour;
	    sample_time.min=beam->min;
	    sample_time.sec=beam->sec;
	  } /* if(elevation > Glob->params.elmax) */
	  
	} else {
	  tiltold = beam->tilt_num;
	  badscan = TRUE;
	} /* if(elevation > elold) */ 

      } /*tilt_num!=tiltold*/
      
      if(badscan == FALSE) {
  
	/*
	 * load the dbz and velocity data into array
	 */

	dbz_ptr = ((ui08 *) beam_buf + sizeof(ll_params_t) +
		   Glob->params.ngates_dropped_start *
		   Glob->params.nfields_raw +
		   Glob->params.dbz_field_pos);
	
	vel_ptr = ((ui08 *) beam_buf + sizeof(ll_params_t) +
		   Glob->params.ngates_dropped_start *
		   Glob->params.nfields_raw +
		   Glob->params.vel_field_pos);

	flg_ptr = ((ui08 *) beam_buf + sizeof(ll_params_t) +
		   Glob->params.ngates_dropped_start *
		   Glob->params.nfields_raw +
		   Glob->params.flg_field_pos);

	vel_scale=(double) beam->scale[Glob->params.vel_field_pos]
	  / 100.0;
	vel_bias = beam->bias[Glob->params.vel_field_pos] / 100.;
	dbz_scale=(double) beam->scale[Glob->params.dbz_field_pos]
	  / 100.0;
	dbz_bias= beam->bias[Glob->params.dbz_field_pos] / 100.;

	if (naz < Glob->params.max_naz - 2) {
	  naz++;
	}

#ifdef NOTNOW
	if (Glob->params.debug) {
	  fprintf(stderr, "naz,nel,k1,tilt,prf,el,az= \n"
		  "%4d %4d %4d %4d %4d %6.1f %6.1f",
		  naz,nel,k1,beam->tilt_num,
		  beam->prf,elevation,azang);
	}
#endif
	
	el[naz][nel][k1] = elevation;
	az[naz][nel][k1] = (float) beam->azimuth / 100.;

	if (Glob->params.anal_fun) {
	  cosel = cos(DTR*elevation);
	  sinaz = sin(DTR*az[naz][nel][k1]);
	  cosaz = cos(DTR*az[naz][nel][k1]);
	}

	num_gates = MIN(beam->range_seg[0].gates_per_beam,
			Glob->params.ngates);

	for (i = 0; i < num_gates; i++) {
  
	  vel[k1][nel][naz][i] = *vel_ptr;
	  dbz[k1][nel][naz][i] = *dbz_ptr;
	  flg[nel][naz][i] = ((*flg_ptr) & 7);
	  vel_ptr += Glob->params.nfields_raw;   
	  dbz_ptr += Glob->params.nfields_raw;
	  flg_ptr += Glob->params.nfields_raw;
	  
	  if (Glob->params.anal_fun) {
	    r1 = rng[i]*cosel;
	    x1 = r1*sinaz;
	    y1 = r1*cosaz;
	    dbz[k1][nel][naz][i] =
	      10+(ui08)(fabs(50.*sin(2.*PI*(x1+(float)nvol*2.0)/15.)*
			     sin(2.*PI*y1/15.)));
	  } 

	} /* i */


      } /* if(badscan == FALSE) */

    } /* if (newvol) */

  } /* while */

}


