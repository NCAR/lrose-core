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
/*
 * Module: DtfMwave.cc
 *
 * Author: Sue Dettling
 *
 * Date:   5/16/01
 *
 * Description: Methods for class DtfMwave. Translated from
 *              mwave.f and dtf_driver.f
 *     
 */

// Include files 


#include <physics/DtfMwave.hh>
#include <stdlib.h>
#include <string.h>
using namespace std;

DtfMwave::DtfMwave()
{
  
  //
  // initialize pointers
  //
  p = NULL;
  t = NULL;
  u = NULL;
  v = NULL;
  agw = NULL;
  z__ = NULL;
  tv = NULL;
  vpt = NULL;
  rh = NULL;
  q = NULL;
  sig = NULL;
  brnt  = NULL;
  brnts = NULL;
  shr = NULL;
  ri = NULL;
  drag = NULL;
  epsilon = NULL;
  tke_kh3__ = NULL;
  tke_kh4__ = NULL;
  tke_kh5__ = NULL;
  tke_kh__ = NULL;
  tke_we__ = NULL;
  tke_sn__ = NULL;
  tke_gwb__ =  NULL;
  tke_dtf__ = NULL;
  tke_cit__ = NULL;
  tke = NULL;
}
 

DtfMwave::~DtfMwave()
{
  if (sig){
    delete[] sig;
  } 
   if (brnt){
    delete[] brnt;
  } 
  if (brnts){
    delete[] brnts;
  } 
  if (shr){
    delete[] shr;
  } 
  if (ri){
    delete[] ri;
  } 
  if (drag){
    delete[] drag;
  } 
  if (epsilon){
    delete[] epsilon;
  } 
  if (tke_kh3__){
    delete[] tke_kh3__;
  } 
  if (tke_kh4__){
    delete[] tke_kh4__;
  } 
   if (tke_kh5__){
    delete[] tke_kh5__;
  } 
  if (tke_kh__){
    delete[] tke_kh__;
  } 
  if (tke_we__){
    delete[] tke_we__;
  } 

  if (tke_sn__){
    delete[] tke_sn__;
  } 
  if (tke_gwb__){
    delete[] tke_gwb__;
  } 

  if (tke_dtf__){
    delete[] tke_dtf__;
  } 

  if (tke_cit__){
    delete[] tke_cit__;
  } 

  if (tke){
    delete[] tke;
  }  
}

int DtfMwave::init(int vert_levs, int lowFluxLev, int upFluxLev,
		   float zter, float varSN, float varWE, 
		   float *pres, float *tk, float *uw, float *vw,
		   float *ww, float *zz, float* ptemp , float *vptemp,
		   float *relH, float *mixRat)
{ 

  //
  // assign input args
  //
  klev = vert_levs;
  kl = lowFluxLev;
  ku = upFluxLev;
  zterain = zter;
  tervarsn = varSN;
  tervarwe = varWE;
  p = pres;
  t = tk;
  u = uw;
  v = vw;
  agw = ww;
  z__ = zz;
  rh = relH;
  q = mixRat;
  vpt = vptemp;
  tv = ptemp;
 
  //
  // Allocate space for arrays
  //
  sig = new float[klev];
  brnt  = new float[klev];
  brnts = new float[klev];
  shr = new float[klev];
  ri = new float[klev];
  drag = new float[klev];
  epsilon = new float[klev];
  tke_kh3__ = new float[klev];
  tke_kh4__ = new float[klev];
  tke_kh5__ = new float[klev];
  tke_kh__ = new float[klev];
  tke_we__ = new float[klev];
  tke_sn__ = new float[klev];
  tke_gwb__ =  new float[klev];
  tke_dtf__ = new float[klev];
  tke_cit__ = new float[klev];
  tke = new float[klev];

  //
  // Check that memory was successfully allocated
  // 
  if ( sig == NULL ||
       brnt == NULL ||
       brnts == NULL ||
       shr == NULL ||
       ri == NULL ||
       drag == NULL ||
       epsilon == NULL ||
       tke_kh3__ == NULL ||
       tke_kh4__ == NULL ||
       tke_kh5__ == NULL ||
       tke_kh__ == NULL ||
       tke_kh__ == NULL ||
       tke_we__ == NULL ||
       tke_sn__ == NULL ||
       tke_gwb__ == NULL ||
       tke_dtf__ == NULL ||
       tke_cit__ == NULL ||
       tke == NULL)
    return(-1);

  //
  // Zero out unassigned data members
  //

  terasmwe = 0;
  tercnvwe = 0;
  terasmsn = 0;
  tercnvsn = 0;
  memset(sig , 0, sizeof(float) * klev);
  memset( brnt, 0, sizeof(float) * klev);
  memset( brnts, 0, sizeof(float) * klev);
  memset(shr , 0, sizeof(float) * klev);
  memset(ri  , 0, sizeof(float) * klev);
  memset( drag, 0, sizeof(float) * klev);
  memset( epsilon, 0, sizeof(float) * klev);
  memset(tke_kh3__ , 0, sizeof(float) * klev);
  memset(tke_kh4__ , 0, sizeof(float) * klev);
  memset(tke_kh5__ , 0, sizeof(float) * klev);
  memset(tke_kh__ , 0, sizeof(float) * klev);
  memset(tke_we__ , 0, sizeof(float) * klev);
  memset(tke_sn__ , 0, sizeof(float) * klev);
  memset(tke_gwb__ , 0, sizeof(float) * klev);
  memset(tke_dtf__ , 0, sizeof(float) * klev);
  memset(tke_cit__ , 0, sizeof(float) * klev);
  memset(tke , 0 , sizeof(float) * klev);

  return 0;
 
}


int DtfMwave::dtf_()
{

    /* System generated locals */
    int i__1;

    /* Local variables */
    float tkek;
    int k,n;
    float com_dtf__, com_agw__;
    
    /* initializations */
    i__1 =  klev;
    for (k = 1; k <= i__1; ++k) 
      {
	shr[k - 1] = (float)0.;
	brnt[k - 1] = (float)0.;
	ri[k - 1] = (float)0.;
	sig[k - 1] = (float)0.;
	epsilon[k - 1] = (float)0.;
	tke_we__[k - 1] = (float)0.;
	tke_sn__[k - 1] = (float)0.;
	tke_gwb__[k - 1] = (float)0.;
	tke_kh3__[k - 1] = (float)0.;
	tke_kh4__[k - 1] = (float)0.;
	tke_kh5__[k - 1] = (float)0.;
	tke_cit__[k - 1] = (float)0.;
	tke_dtf__[k - 1] = (float)0.;
	tke[k - 1] = (float)0.;
      }

    new_richson_v__();
    
    /* compute the tke from KH */
    
    /* for DTF3.0 */
    dtf3_code__();
    i__1 =  klev;
    for (k = 1; k <= i__1; ++k) 
      {
	tke_kh3__[k - 1] =  tke_kh__[k - 1];
      }
    
    /* for DTF4.0 */
    dtf4_code__();
    i__1 =  klev;
    for (k = 1; k <= i__1; ++k) 
      {
	tke_kh4__[k - 1] =  tke_kh__[k - 1];
      }
    
    /* for DTF5.0 */
    dtf5_code__();
    i__1 =  klev;
    for (k = 1; k <= i__1; ++k) 
      {
	tke_kh5__[k - 1] =  tke_kh__[k - 1];
      }
    
    /* tke from grav. wave breaking WE comp */
    
    n = 1;
    we_gwriflux__(&n, &n);
    
    /* tke from grav. wave breaking SN comp */
    sn_gwriflux__(&n, &n);
    
    /* The total tke is computed just down below */
    /*  TKE = TKE(KH) + TKE(GWD) */
    
    i__1 =  klev;
    for (k = 1; k <= i__1; ++k) 
      {
	tke_gwb__[k - 1] = sqrt((double) tke_we__[k - 1] * 
				tke_we__[k - 1] + 
				tke_sn__[k - 1] * 
				tke_sn__[k - 1]);
	tkek =  tke_kh__[k - 1] +  tke_gwb__[k - 1];
	
	if (tkek < (float)0.) 
	  {
	    tkek = (float)0.;
	  } 
	else 
	  {
	    if (tkek > (float)100.) 
	      {
		tkek = (float)100.;
	      }
	  }
	tke_dtf__[k - 1] = tkek;
      }
    
    /* now put together tke and agw fields */
    
    i__1 =  klev;
    for (k = 1; k <= i__1; ++k) 
      {
	tke_cit__[k - 1] =  agw[k - 1] * (float).1 * 
	  agw[k - 1];
	
	com_agw__ =  tke_cit__[k - 1];
	
	com_dtf__ =  tke_dtf__[k - 1];
	
	tke[k - 1] = sqrt(com_agw__ * com_agw__ + com_dtf__ * 
			  com_dtf__) * (float)2.;
      }
    
    return 0;
    
} /* dtf_ */

/* ----------------------------------------------------------------- */

/* Subroutine dtf4_code__*/ 

/* Adrian Marroquin FSL */
/* version modified for GTG */
/* 11/25/98 */

/* Parameters calibrated to optimize PODs */
/* e0 =3.5,dt=500. */

/* The following is a table of POD's (PODn = PODy) and */
/* thresholds for each one of the months from Nov 97 */
/* to June 1998. */

/*  Nov   Dec   Jan   Feb   Mar   Apr   May   Jun */

/*  62.5  62.   61.   60.   63.   61.   60.   59.   SA (I>1, >20kft) all w */
/* 2.5   2.7   2.6   3.0   2.75  2.75  1.7   1.65   thresholds */
/*  61.   63.5  58.   60.   65.0  62.   64.   56.   HA (I>1, >w120, >20kft) */
/* 2.6   2.85  2.9   3.4   2.8   2.9   2.7   1.85   thresholds */
/*  61.   79.   67.   64.   74.   64.   57.   67.   SA (I>4, >20kft) all w */
/* 2.5   4.0   3.0   3.2   3.85  2.8   1.6   2.0    thresholds */
/*  59.   79.   65.   61.   72.5  62.   58.   64.   HA (I>4, >w120, >20kft) */
/* 2.35  4.25  3.4   3.5   3.65  3.0   1.5   2.1    thresholds */

/* In the above table SA means Special Aircraft, */
/* (please make this correction in DTF3 explanation) */
/* HA heavy aircraft (commercial) */
/* I>1 turbulence intensities light or greater, */
/* >20kft aircraft flying above 20,000 ft, */
/* >w120 aircraft heavier than 120,000 lbs, */
/* and I>4 turbulence intensities moderate-to-severe or greater. */

/* WARNING: The above table was generated with TKE from DTF4 verified */
/* with PIREPs. The model output was from RUC2 (40-km), 40 isentropic */
/* levels. PIREPs from turbulence related to convection were not */
/* removed. DTF4 formulation is only applicable to turbulence from */
/* shear intabilities (clear-air turbulence) specially found in */
/* upper fronts. */
/* ------------------------------------------------------------------------- */
/* DTF4 works well for moderate-to-severe turbulence or greater */
/* that affect heavy (commercial aircraft). */

/* ------------------------------------------------------------------------- */
/* Constants from Stull (1988), page 219 */



int DtfMwave::dtf4_code__()
{
  /* Initialized data */

  static float akm = (float)75.;

  /* Local variables */
  float crff, epln, prnd, zsfc, zlev, ztop;
  int k;
  float depth;
  float a2, br;
  float dz, alb, arg, all, rff, als;
  float tke_dtf4__;
  float c1, c2, c3, c13, c23, ce, alinf,
    akarm, cr, c5, crf, e0, dt, cmu;
  
  /* System generated locals */
  int i__1;
  
  c1 = 1.44;
  c2 = 1.0;
  c3 = 1.92;
  c13 = c1/c3;
  c23 = c2/c3;
  ce = 0.19;
  alinf = 200.0;
  akarm = 0.35;
  cr = 0.54;
  c5 = 0.31;
  crf = 1.5;
  e0 = 3.5;
  dt = 500.0;
  cmu = 0.09;
  
  ztop =  zterain + (float)3e3;
  zsfc =  zterain - (float)10.;
  
  i__1 =  klev;
  for (k = 1; k <= i__1; ++k) 
    {
      if ( ri[k - 1] > (float).01) 
	{
	  rff = rfkondo_(& ri[k - 1]);
	} 
      else 
	{
	  rff = rf_(& ri[k - 1]);
	}
      crff = rff * crf;
      if (crff < (float)1.) 
	{
	  a2 =  shr[k - 1] * cmu * ((float)1. - crff);
	  
	  arg = sqrt(a2) * c5;
	  
	  tke_dtf4__ = exp(arg * dt) * e0;
	} 
      else 
	{
	  tke_dtf4__ = e0;
	}
      
      tke_dtf4__ = (tke_dtf4__ - (float)3.50) * (float)3. - (float)1.;
      
      if (tke_dtf4__ < (float)0.) 
	{
	  tke_dtf4__ = (float)0.;
	}
      
      tke_kh__[k - 1] = tke_dtf4__;
      
      /* introduce correction for the boundary layer */
      
      zlev =  z__[k - 1];
      if (zsfc <= zlev && zlev <= ztop) 
	{
	  depth = ztop - zsfc;
	  
	  prnd = prand_(& ri[k - 1]);
	  
	  epln = akm * ( shr[k - 1] * c13 - 
			 c23 *  brnt[k - 1] / prnd);
	  if (epln < (float)0.) 
	    {
	      epln = (float)0.;
	    }
	  
	  if ( brnt[k - 1] < (float)0.) 
	    {
	      tke_dtf4__ = (float)0.;
	    } 
	  else 
	    {
	      br = sqrt( brnt[k - 1]);
	      
	      dz = zlev - zsfc;
	      
	      alb = alinf*akarm*dz/(akarm*dz+alinf) ;
	      
	      als = sqrt(tke_dtf4__) * cr / br;
	      
	      all = dmin(alb,als);
	      
	      tke_dtf4__ = pow((double)all * epln,(double) .666666);
	    }
	  
	  tke_kh__[k - 1] = tke_dtf4__;
	}
      
    } /* K-loop (klev) */
  
  return 0;
  
} /* dtf4_code__ */

/* ----------------------------------------------------------------- */

/* Subroutine dtf5_code__ */ 

/* Adrian Marroquin FSL */
/* version modified for GTG */
/* 11/25/98 */

/* Parameters calibrated to optimize PODs */
/* dt=45.,e0=2.7,ep0=1.e-4 */

/* The following is a table of POD's (PODn = PODy) and */
/* thresholds for each one of the months from Nov 97 */
/* to June 1998. */

/*   Nov   Dec   Jan   Feb   Mar   Apr   May   Jun */

/*  61.0  61.0  60.0  59.0  61.0  60.0  60.0  59.0  SA (I>1, >20kft) all w */
/*  .150  .170  .165  .195  .180  .170  .120  .115 */
/*  60.0  61.0  57.0  56.0  64.0  60.0  62.0  59.0  HA (I>1, >w120, >20kft) */
/*  .170  .190  .190  .230  .180  .195  .120  .123 */
/*  61.0  76.0  66.0  62.0  74.0  61.0  56.0  65.0  SA (I>4, >20kft) all w */
/*  .144  .28   .2    .25   .26   .18   .15   .125 */
/*  58.0  76.0  64.0  58.0  72.0  60.0  58.0  63.0  HA (I>4, >w120, >20kft) */
/*  .157  .280  .225  .240  .245  .190  .110  .130 */

/* In the above table SA means Special Aircraft, */
/* HA heavy aircraft (commercial) */
/* I>1 turbulence intensities light or greater, */
/* >20kft aircraft flying above 20,000 ft, */
/* >w120 aircraft heavier than 120,000 lbs, */
/* and I>4 turbulence intensities moderate-to-severe or greater. */

/* WARNING: The above table was generated with TKE from DTF5 verified */
/* with PIREPs. The model output was from RUC2 (40-km), 40 isentropic */
/* levels. PIREPs from turbulence related to convection were not */
/* removed. DTF5 formulation is only applicable to turbulence from */
/* shear intabilities (clear-air turbulence) specially found in */
/* upper fronts. */
/* ------------------------------------------------------------------------- */
/* DTF5 works well for moderate-to-severe turbulence or greater */
/* that affect heavy (commercial aircraft). */

/* ------------------------------------------------------------------------- */
/* Constants from Stull (1988), page 219 */


int DtfMwave::dtf5_code__()
{
  /* System generated locals */
  int i__1;
  float r__1;
  double d__1 = 0.0;
  
  /* Local variables */
  float cnhf, cnhn, snhf, fnot, snhn;
  int k;
  float g1, g2, ad, r11;
  float den, co34, arg, rff, cu34, dpo, epn;
  float c3, c4, cu, dt, r1, r2, e0, ep0, e0ep0;
  
  c3 = 1.4; 
  c4 = 1.9; 
  cu = 0.09;
  r1 = c3/(c3-1.0);
  r2 = c4/(c4-1.0);
  dt = 45.0;
  e0 = 2.7;
  ep0 = 1.e-4;
  e0ep0 = e0/ep0;
  
  cu34 = sqrt(cu*(c3-1)*(c4-1));
  co34 = sqrt(cu*(c3-1)/(c4-1));
  
  i__1 =  klev;
  for (k = 1; k <= i__1; ++k) 
    {
      r11 = r1;
      
      rff = rf_(& ri[k - 1]);
      
      arg = sqrt( shr[k - 1] * ((float)1. - rff));
      
      ad = cu34 * arg;
      
      den = co34 * arg;
      
      dpo = den * (float)e0ep0;
      
      tke_kh__[k - 1] = (float)0.0;
      
      if (fabs(dpo) <= (float)1.0) 
	{
	  continue;
	}
      
      fnot = log((dpo + (float)1.) / (dpo - (float)1.)) * (float).5;
      
      if (dpo > (float)1.) 
	{
	  
	  snhf = fabs(sinh(fnot)) + (float)1e-10;
	  
	  cnhf = cosh(fnot);
	  
	  r__1 = sinh(ad * dt + fnot);
	  snhn = fabs(r__1);
	  
	  cnhn = cosh(ad * dt + fnot) + (float)1e-10;
	  
	  g1 = pow((double)snhn / snhf, (double)r11 );
	  
	  g2 = pow((double) cnhf / cnhn, (double) r2);
	  
	  epn = g1 * ep0* g2;
	} 
      else 
	{
	  epn = ep0;
	}
      
      if (epn < (float)0.) 
	{
	  epn = (float)0.;
	}
      
      d__1 = (double) epn;
      
      tke_kh__[k - 1] = pow((double) epn, (double) .333333333);
      
    } /* K-loop (klev) */
  
  return 0;
  
} /* dtf5_code__ */


/* Subroutine */
/* Version by Adrian Marroquin */
/* FSL, NOAA, ERL */
/* 26 June 1997 */

int DtfMwave::new_richson_v__()
{
  /* System generated locals */
  int i__1;
  double d__1;
  
  /* Local variables */
  float beta, shru, shrv;
  int k;
  float brunt;
  float pi1, pi2, pi3, th1, th2, th3;
  
  
  i__1 =  klev;
  
  pi1 = pow((double) p[0] / p00, (double)rocp);
  
  pi2 = pow((double) p[1] /p00, (double) rocp);
  
  th1 =  t[0] / pi1;
  
  th2 =  t[1] / pi2;
  i__1 =  klev - 1;
  for (k = 2; k <= i__1; ++k) 
    {
      d__1 =  p[k] / p00;
      
      pi3 = pow((double) p[k] / p00 , (double) rocp);
      
      th3 =  t[k] / pi3;
    
      /* vertical derivatives using pressure */
    
      brunt = vertirreg_(&th1, &th2, &th3, & p[k - 2], 
			 & p[k - 1], & p[k]);
      
      shru = vertirreg_(& u[k - 2], & u[k - 1], 
			& u[k], & p[k - 2], 
			& p[k - 1],& p[k]);
      
      shrv = vertirreg_(& v[k - 2], & v[k - 1], 
			& v[k], & p[k - 2], & p[k - 1],
			& p[k]);
      
      beta =  p[k - 1] * g * g / (pi2 * rd * th2 * th2);
      
      brnt[k - 1] = -beta * brunt;
      
      shr[k - 1] = beta *  p[k - 1] * 
	(shru * shru + shrv *shrv) / (pi2 * rd);
      
      th1 = th2;
      th2 = th3;
      pi1 = pi2;
      pi2 = pi3;
      
      ri[k - 1] =  brnt[k - 1] / ( shr[k - 1] + 
				   (float)1e-10);
    }
  
  brnt[0] =  brnt[1];
  
  shr[0] =  shr[1];
  
  ri[0] =  ri[1];
  
  brnt[ klev - 1] =  brnt[ klev - 2];
  
  shr[ klev - 1] =  shr[ klev - 2];
  
  ri[ klev - 1] =  ri[ klev - 2];
  
  i__1 =  klev;
  for (k = 1; k <= i__1; ++k) 
    {
      if ( ri[k - 1] > (float)120.) 
	{
	  ri[k - 1] = (float)120.;
	}
    }
  
  return 0;
  
} /* new_richson_v__ */


/* Subroutine dtf3_code__*/ 
/* Adrian Marroquin FSL */
/* version modified for GTG */
/* 11/09/98 */

/* Modified 12/04/2002 by Curtis Caravone
   -- Multiply final tke_kh__ value by .588 */

/* Km parameter calibrated to optimize PODn and */
/* PODy, Km = 75.0 m^2/s */
/* The following is a table of POD's (PODn = PODy) and */
/* thresholds for each one of the months from Nov 97 */
/* to June 1998. */


/* Nov   Dec   Jan   Feb   Mar   Apr   May   Jun */

/*  66.   66.   61.   62.   68.   65.   64.   60.  GA (I>1, >20kft) all w */
/*  .58   .71   .72   .86   .73   .73   .42   .47  Thresholds */
/*  67.   66.   64.   63.   67.   64.   62.   59.  HA (I>1, >w120, >20kft) */
/*  .65   .67   .63   .71   .70   .68   .4    .4   Thresholds */
/*  63.   82.   72.   68.   76.   70.   60.   68.  GA (I>4, >20kft) all w */
/*  .55  1.105  .85   .8    .975  .76   .39   .5   Thresholds */
/*  62.   82.   72.   66.   75.   68.   59.   67.  HA (I>4, >w120, >20kft) */
/*  .6    1.15  .95   .91   .93   .78   .39   .55  Thresholds */

/* In the above table GA means General Aviation, */
/* HA heavy aircraft (commercial) */
/* I>1 turbulence intensities light or greater, */
/* >20kft aircraft flying above 20,000 ft, */
/* >w120 aircraft heavier than 120,000 lbs, */
/* and I>4 turbulence intensities moderate-to-severe or greater. */

/* WARNING: The above table was generated with TKE from DTF3 verified */
/* with PIREPs. The model output was from RUC2 (40-km), 40 isentropic */
/* levels. PIREPs from turbulence related to convection were not */
/* removed. DTF3 formulation is only applicable to turbulence from */
/* shear intabilities (clear-air turbulence) specially found in */
/* upper fronts. */
/* ------------------------------------------------------------------------- */
/* DTF3 has been tested with 12-15 December 97 case study. In this case */
/* convective activity was at a minimum in the first half of December 97. */
/* During 12-15 Dec 97 a quasi-steady front moved across the US accompanied */
/* with a severe turbulence outbreak. For this case PODy = 93.6% and PODn = */
/* 76.1% obtained using thresholds for the month of December (see table */
/* above, PODn = PODy = 82% for December). The difference in PODy's is */
/* attributed to the inclusion of PIREPs from convection during the second */
/* half of December 97. */

/* DTF3 works well for moderate-to-severe turbulence or greater */
/* that affect heavy (commercial aircraft). */

/* ------------------------------------------------------------------------- */
/* Constants from Stull (1988), page 219 */

int DtfMwave::dtf3_code__()
{
    /* Initialized data */

  static int iepn3 = 0;
  static float akm = (float)75.0;
  
  /* System generated locals */
  int i__1;
  double d__1;
  
  /* Local variables */
  float zsfc, zlev, ztop;
  int k;
  float br;
  float dz, alb, all, rff, als;
  float c1, c2, c3, c13, c23, ce, alinf, akarm, cr;
  
  
  c1=1.44;
  c2=1.0;
  c3=1.92;
  c13 = c1/c3;
  c23 = c2/c3;
  ce = 0.19;
  alinf = 200.0;
  akarm = 0.35;
  cr = 0.54;
  
  /* now compute dissipation */
  /* ztop equivalent to cpbl */
  
  ztop =  zterain + (float)3e3;
  zsfc =  zterain - (float)10.;
  
  i__1 =  klev;
  for (k = 1; k <= i__1; ++k) 
    {
      zlev =  z__[k - 1];
      if ( ri[k - 1] > (float).01) 
	{
	  rff = rfkondo_(& ri[k - 1]);
	} 
      else 
	{
	  rff = rf_(& ri[k - 1]);
	}
      
      epsilon[k - 1] = akm *  shr[k - 1] * (c13 - rff * c23);
      
      if ( epsilon[k - 1] < (float)0.) 
	{
	  epsilon[k - 1] = (float)0.;
	}
      
      if (iepn3 == 0) 
	{
	  if ( brnt[k - 1] <= (float)0.) 
	    {
	      tke_kh__[k - 1] = (float)0.;
	    } 
	  else 
	    {
	      br = sqrt( brnt[k - 1]);
	      
	      tke_kh__[k - 1] =  epsilon[k - 1] * (float).7 
		/ (br * ce);
	      if (zsfc <= zlev && zlev <= ztop) 
		{
		  dz = zlev - zsfc;
		  
		  alb = dz * alinf * akarm / (dz * akarm + alinf);
		  
		  als = sqrt( tke_kh__[k - 1]) * cr / br;
		  
		  all = dmin(alb,als);
		  
		  d__1 = (double) (all *  epsilon[k - 1]);
		  tke_kh__[k - 1] = pow((double) all *  epsilon[k - 1], .666666);
		  
		}
	    }
	}
      /* Added 12/04/2002 by Curtis Caravone */
      tke_kh__[k - 1] = (float) .588 * tke_kh__[k - 1];

    }/* end of K-loop (klev) */
  
  return 0;
  
} /* dtf3_code__ */


/* --------------------------------------------------------------------- */
double DtfMwave::rfkondo_(float *ri)
{
  /* System generated locals */
  float ret_val = 0.0;
  
  /* Local variables */
  float d1, ahm;
  float c0,c1;
  
  c0=6.873;
  c1=7.0;
  
  /* Rfc (critical flux Ri) = 0.143 */
  
  if (*ri > (float)1.) 
    {
      ret_val = (float)1. / (*ri * c1);
    } 
  else 
    {
      if ((float).01 < *ri && *ri <= (float)1.) 
	{
	  d1 = *ri * c0 + (float)1.;
	  ahm = (float)1. / (*ri * c0 + (float)1.0 / d1);
	  ret_val = *ri * ahm;
	}
    }
  
  return ret_val;
  
} /* rfkondo_ */


/* Subroutine new_kep_anl__*/ 
/* Version by Adrian Marroquin */
/* FSL, NOAA, ERL */
/* 26 June 1997 */
/* notes: */
/* epr      TKE/(disipation rate) ratio */
/* epn      dissipation rate of TKE */
/* rfx      flux Richardson number */

int DtfMwave::new_kep_anl__()
{
  /* Initialized data */
  
  static float scale = (float)10.;
  static float dt = (float)1200.;
  static float e0ep0 = (float)0.;
  static float ep0 = (float)1e-4;
  
  /* System generated locals */
  int i__1;
  float r__1;
  
  /* Local variables */
  float tkek, eprk, a;
  
  int k;
  
  float bb;
  
  float rr, rff, tke_sfc__, 
    arg1, arg2, arg3, arg4, arg5, arg6;
  
  float c1,c2,c3,cmu,c13,c23,tke0,time,exp0,ce1,ce2,cu,a1,
      b1,b2,cc2,cc3,e1,e2,rfc,b;

  float *epn = new float[klev];
  float *epr = new float[klev];
  float *rfx = new float[klev];

  memset(epn,0, sizeof(float) * klev);
  memset(epr,0, sizeof(float) * klev);
  memset(rfx,0, sizeof(float) * klev);

  c1 = 1.44;
  c2 = 1.0;
  c3 = 1.92;
  cmu = 0.09;
  c13 = c1/c3;
  c23 = c2/c3;
  tke0 = 3.0;
  time = 1000.0;
  exp0 = 2.5;
  ce1 = 1.4; 
  ce2 = 1.9; 
  cu = 0.09;
  a1 = .78;
  b1 = 15.0;
  b2 = 8.0;
  cc2 = .3;
  cc3 = .333;
  e1 = b1-6.*a1;
  e2 = b1+12.*a1*(1.-cc2)+3.*b2*(1.-cc3);
  rfc = e1/e2 + 0.15;
  b = ce2-1.0;
  
  i__1 =  klev;
  for (k = 1; k <= i__1; ++k) 
    {
      rff = rf_(& ri[k - 1]);
      rfx[k - 1] = rff;
      
      
      if (rff < rfc) 
	{
	  a = fabs((1.-ce1)*(1.-rff)*cu*( shr[k - 1] + 1.e-9));
	  arg1 = sqrt(b / a);
	  if (fabs(e0ep0) < (float)1e-12) 
	    {
	      eprk = arg1;
	      epr[k - 1] = eprk;
	    } 
	  else 
	    {
	      arg2 = a * arg1;
	      arg3 = e0ep0 / arg1;
	      r__1 = e0ep0 / arg1;
	      arg4 = atanh_(&r__1);
	      arg5 = (float)1. / arg4;
	      arg6 = tanh(dt * arg2 + arg5);
	      eprk = arg1 / arg6;
	      epr[k - 1] = eprk;
	    }
	  
	  /* compute epsilon into epn */
	  
	  bb = ce1 * cu * shr[k - 1] *  ((float)1. - rff);
	  
	  rr = epr[k - 1] * bb - ce2 / epr[k - 1];
	  
	  epn[k - 1] = ep0 * exp(dt * rr);
	  
	  tkek = epr[k - 1] * epn[k - 1];
	  
	  if (tkek > (float)50.) 
	    tkek = (float)50.;
	  
	  tke_kh__[k - 1] = tkek;
	}
      else 
	{
	  epr[k - 1] = (float)0.;
	  
	  epn[k - 1] = (float)0.;
	  
	  tke_kh__[k - 1] = (float)0.;
	} 
    }
  
  /* estimate tke in the surface layer */
  
  rff = rf_( ri);
  
  tke_kh__[0] = (float)0.;
  
  if (rff < (float)1.) 
    {
      
      tke_sfc__ = cu *  shr[0] * ((float)1. - rff) * scale *
	scale;
      
      if (tke_sfc__ > (float)50.) 
	{
	  tke_sfc__ = (float)50.;
	}
      tke_kh__[0] = tke_sfc__;
    }
  
  delete[] epn;
  delete[] epr;
  delete[] rfx;

  return 0;
  
} /* new_kep_anl__ */



/* ------------------------------------------------------ */
double DtfMwave::rf_(float *ri)
{
  /* Initialized data */
  static float c1 = (float).056;
  static float c2 = (float).3;
  static float c3 = (float).3333;
  static float a1 = (float).78;
  static float a2 = (float).79;
  static float b1 = (float)15.;
  static float b2 = (float)8.;
  
  /* System generated locals */
  float ret_val;
  
  /* Local variables */
  float e1, e2, e3, e4, e5, f1, f2, f3, f4, f42;
  
  
  e1 = b1 - (float)6. * a1;
  
  e2 = b1 + a1 * (float)12. * ((float)1. - c2) + b2 * (float)3. * 
    ((float)1. - c3);
  
  e3 = b1 * ((float)1. - c1 * (float)3.) - a1 * (float)6.;
  
  e4 = b1 * ((float)1. - c1 * (float)3.) + a1 * (float)12. * 
    ((float)1. - c2) + a2 * (float)9. * ((float)1. - c2);
  
  e5 = b1 + a1 * (float)3. * ((float)1. - c2) + b2 * (float)3. * 
    ((float)1. - c3);
  
  f1 = a2 * (float).5 * e5 / (a1 * e4);
  
  f2 = a1 * e3 / (a2 * e5);
  
  f3 = a1 * (float)2. * (e3 * e5 - e1 * (float)2. * e4) / (a2 * e5 * e5);
  
  f4 = a1 * e3 / (a2 * e5);
  
  f42 = f4 * f4;
  
  ret_val = f1 * (*ri + f2 - sqrt(*ri * *ri + f3 * *ri + f42));
  
  return ret_val;
  
} /* rf_ */


/* Subroutine new_kep_expo__*/ 
/* Version by Adrian Marroquin */
/* FSL, NOAA, ERL */
/* 26 June 1997 */
/* Seudo-analytic TKE model based on the K-epsilon formulation */
/* with no diffusion and no advection */

int DtfMwave::new_kep_expo__()
{
  /* System generated locals */
  int i__1;
  double d__1;
  
  /* Local variables */
  float anum, prnd, zsfc, test, ztop;
  int k;
  float depth, br, dz = 0.0, prandl, alb, den, arg, all, als, tke1, tke2 = 0.0;
  float c1,c2,c3,cmu,c13,c23,tke0,time,exp0,ce,akm,alinf,akarm,cr;
  
  c1=1.44;
  c2=1.0;
  c3=1.92;
  cmu=0.09;
  c13=c1/c3;
  c23=c2/c3;
  tke0=3.0;
  time=1200.0;
  exp0=2.5;
  ce=0.19;
  akm = 2.25e+2;
  alinf = 200.0;
  akarm = 0.35;
  cr = 0.54;
  
  i__1 =  klev;
  for (k = 1; k <= i__1; ++k) 
    {
      tke[k - 1] = (float)0.;
      
      prandl = prand_(& ri[k - 1]);
      
      den = c13 *  shr[k - 1] * c23 -  brnt[k - 1] / prandl;
      
      if (den <= (float)0.) 
	{
	  den = (float)1e-10;
	}
      
      test = prandl * c13/c23;
      
      if ( ri[k - 1] < test) 
	{
	  den = sqrt(cmu * den);
	  
	  anum = cmu * ( (1-c13) * shr[k - 1] *  - 
			 brnt[k - 1] *(1-c23)/ prandl);
	  
	  arg = time * anum  / den;
	  
	  tke[k - 1] = tke0 *(exp(arg) - exp0);
	  
	  epsilon[k - 1] = den *  tke[k - 1];
	} 
      else 
	{
	  tke[k - 1] = (float)0.;
	  
	  epsilon[k - 1] = (float)0.;
	}
      
      if ( tke[k - 1] < (float)0.) 
	{
	  tke[k - 1] = (float)0.;
	}
    }
  
  /* introduce correction for the boundary layer */
  
  ztop =  zterain + (float)2e3;
  zsfc =  zterain;
  depth = ztop - zsfc;
  
  i__1 =  klev;
  for (k = 1; k <= i__1; ++k) 
    {
      if (zsfc <=  z__[k - 1] &&  z__[k - 1] <= ztop) 
	{
	  prnd = prand_(& ri[k - 1]);
	  
	  epsilon[k - 1] = akm * (c13 *  shr[k - 1] -  
				  c23 *  brnt[k - 1] / prnd) ;
	  if ( epsilon[k - 1] < (float)0.) 
	    {
	      epsilon[k - 1] = (float)0.;
	    }
	  
	  if ( brnt[k - 1] < (float)0.) 
	    {
	      tke1 = (float)0.;
	    } 
	  else 
	    {
	      br = sqrt( brnt[k - 1]);
	      
	      dz =  z__[k - 1] - zsfc;
	      
	      alb = alinf * akarm * dz  / (akarm * dz + alinf);
	      
	      als = sqrt( tke[k - 1]) * (float).54 / br;
	      
	      all = dmin(alb,als);
	      
	      d__1 = (double) (all *  epsilon[k - 1]);
	      tke2 = pow( (double) (all *  epsilon[k - 1]) , .666666);
	    }
	  
	  tke2 = ((float)1.0 - dz / depth) * tke2;
	  
	  tke[k - 1] = tke2;
	}
    }
  
  return 0;
  
} /* new_kep_expo__ */


/* ____________________________________________________________ */


/* Subroutine sn_gwriflux__  */ 
/* PURPOSE: */

/* To compute TKE from gravity wave breaking based on quadratic eqn for */
/* Froud number (and function of Ri). All computations are done with */
/* variables defined on wind points including brunt, Ri which come */
/* from rich_onv.f subroutine. */

/* Notes: */
/* factor is a tunable parameter that was taken as shown below (1/meter). */
/* It could be set to 2*dx (dx grid spacing). */

/* All computations are done on wind points */

/* kl,ku are indices that determine the layer near the surface */
/*       to compute average variables */

/* ric is the critical Richardson number */

int DtfMwave::sn_gwriflux__(int *nxp, int *nyp)
{
  /* Initialized data */
  
  static float factor = (float).001;
  static float ric = (float).25;
  
  /* System generated locals */
  int i__1;
  float r__1;
  
  /* Local variables */
  float frud, brns, rigw, rhos, taus, sumr, sumu, sumv;
  int k;
  float wproj, ul, vl, us, vs, uu, ux, vy;
  float tervar, sumbrn, ri2, ri4, den, brn, rii, epp, 
    ubs, dzl, sum, 
    tau_tke__, dzl2;
  
  float *dsig = new float[klev];
  float *rho = new float[klev];
  float *tau = new float[klev + 1];

  memset(dsig,0, sizeof(float) * klev);
  memset(rho,0, sizeof(float) * klev);
  memset(tau,0, sizeof(float) * (klev + 1));

  tervar =  tervarsn;
  
  /* now, compute terrain variance on wind points */
  
  i__1 =  klev;
  for (k = 1; k <= i__1; ++k) 
    {
      rho[k - 1] =  p[k - 1] / ( t[k - 1] * rd);
    }
  
  sig[0] = (float)1.0;
  
  i__1 =  klev - 1;
  for (k = 2; k <= i__1; ++k) 
    {
      dsig[k - 1] = ( p[k - 2] -  p[k]) * (float).5;
      sig[k - 1] =  sig[k - 2] + dsig[k - 1];
    }
  
  dsig[0] =  p[0] -  p[1];
  
  sumbrn = (float)0.;
  sumr = (float)0.;
  sum = (float)0.;
  sumu = (float)0.;
  sumv = (float)0.;
  brns = (float)0.;
  taus = (float)0.;
  
  i__1 =  ku;
  for (k =  kl; k <= i__1; ++k) 
    {
      if ( brnt[k - 1] >= (float)0.) 
	{
	  brn = sqrt( brnt[k - 1]);
	  sumbrn += brn * dsig[k - 1];
	}
      
      sumu +=  u[k - 1] * dsig[k - 1];
      
      sumv +=  v[k - 1] * dsig[k - 1];
      
      sumr += rho[k - 1] * dsig[k - 1];
      
      sum += dsig[k - 1];
    }
  
  us = sumu / sum;
  vs = sumv / sum;
  brns = sumbrn / sum;
  rhos = sumr / sum;
  
  ubs = sqrt(us * us + vs * vs);
  ux = us / (ubs + (float)1e-10);
  vy = vs / (ubs + (float)1e-10);
  
  /* only the E-W component */
  
  taus = factor * rhos * vs * brns * tervar * tervar;
  
  tau[ ku - 1] = taus;
  
  
  /* lift-off */
  
  i__1 =  klev;
  for (k =  ku; k <= i__1; ++k) 
    {
      /* compute new wave-displacement using the flux from below */
      
      if ( brnt[k - 1] <= 0.0 ||  ri[k - 1] <= 0.0) 
	{
	  
	  /* don't breake waves if there are unstable layers */
	  
	  tau[k] = tau[k - 1];
	} 
      else 
	{
	  brn = sqrt( brnt[k - 1]);
	  
	  r__1 =  u[k - 1] * ux +  v[k - 1] * vy; 
	  wproj = fabs(r__1);
	  
	  ul = wproj * ux;
	  
	  vl = wproj * vy;
	  
	  uu = sqrt(ul * ul + vl * vl);
	  
	  dzl2 = tau[k - 1] / (rho[k - 1] * brn * vl * factor + (float)
			       1e-10);
	  dzl = sqrt(dzl2);
	  
	  /* compute gwri */
	  
	  frud = brn * dzl / (uu + (float)1e-10);
	  
	  ri2 = sqrt( ri[k - 1]);
	  
	  den = (ri2 * frud + (float)1.) * (ri2 * frud + (float)1.);
	  
	  rigw =  ri[k - 1] * ((float)1. - frud) / den;
	  
	  if (rigw >= ric) 
	    {
	      tau[k] = tau[k - 1];
	    } 
	  else 
	    {
	      
	      /* compute a new wave-displacement */
	      
	      ri4 = sqrt(ri2);
	      
	      rii = sqrt(ri2 * (float)2. + (float)1.);
	      
	      epp = rifunc_(&ri2, &ri4, &ric);
	      
	      if (epp < (float)0.) 
		{
		  epp = (float)0.;
		}
	      dzl = epp * uu / brn;
	      
	      dzl2 = dzl * dzl;
	      
	      tau[k] = factor * rho[k - 1] * brn * vl * dzl2;
	    }	    
	}
      
      /* to make TKE multiply flux by 1/rho */
      
      
      tau_tke__ = -(tau[k] - tau[k - 1]) / rho[k - 1];
      
      tke_sn__[k - 1] = tau_tke__;
      
    }
  
  delete[] dsig;
  delete[] rho;
  delete[] tau;

  return 0;
  
} /* sn_gwriflux__ */


/* ____________________________________________________________ */
/* Subroutine we_gwriflux__*/ 
/* Version by Adrian Marroquin */
/* FSL, NOAA, ERL */
/* 26 June 1997 */


/* NAME: gwriflux(nxp,nyp) */
/* PURPOSE: */
/* To compute TKE from gravity wave breaking based on quadratic eqn for */
/* Froud number (and function of Ri). All computations are done with */
/* variables defined on wind points including brunt, Ri which come */
/* from rich_onv.f subroutine. */

int DtfMwave::we_gwriflux__(int *nxp, int *nyp)
{
  /* Initialized data */
  
  static float factor = (float).001;
  static float ric = (float).25;
  
  /* System generated locals */
  int i__1;
  float r__1;
  
  /* Local variables */
  float frud, brns, rigw, rhos, taus, sumr, sumu, sumv;
  int k;
  float wproj, ul, vl, us, vs, uu, ux, vy;
  float tervar, sumbrn, ri2, ri4, den, brn, rii, epp,
    ubs, dzl, sum, tau_tke__, dzl2;
  

  float *dsig = new float[klev];
  float *rho = new float[klev];
  float *tau = new float[klev +1];

  memset(dsig,0, sizeof(float) * klev);
  memset(rho,0, sizeof(float) * klev);
  memset(tau,0, sizeof(float) * (klev + 1));

  
  i__1 =  klev;
  for (k = 1; k <= i__1; ++k) 
    {
      tke_we__[k - 1] = (float)0.0;
      tke_sn__[k - 1] = (float)0.0;
    }
  
  /* now, compute terrain variance on wind points */
  
  i__1 =  klev;
  for (k = 1; k <= i__1; ++k) 
    {
      rho[k - 1] =  p[k - 1] / ( t[k - 1] * rd);
    }
  
  /* If sigma levels are defined for a particular */
  /* numerical model, use the following lines; */
  
  i__1 =  klev - 1;
  for (k = 2; k <= i__1; ++k) 
    {
      dsig[k - 1] = ( p[k - 2] -  p[k]) * (float).5;
    }
  dsig[0] =  p[0] -  p[1];
  
  sumbrn = (float)0.;
  sumr = (float)0.;
  sum = (float)0.;
  sumu = (float)0.;
  sumv = (float)0.;
  brns = (float)0.;
  taus = (float)0.;
  
  i__1 =  ku;
  for (k =  kl; k <= i__1; ++k) 
    {
      brn = (float)0.0;
      if ( brnt[k - 1] >= (float)0.) 
	{
	  brn = sqrt( brnt[k - 1]);
	  
	  sumbrn += brn * dsig[k - 1];
	}
      sumu +=  u[k - 1] * dsig[k - 1];
      sumv +=  v[k - 1] * dsig[k - 1];
      sumr += rho[k - 1] * dsig[k - 1];
      sum += dsig[k - 1];
      
      
      us = sumu / sum;
      vs = sumv / sum;
      brns = sumbrn / sum;
      rhos = sumr / sum;
      
      ubs = sqrt(us * us + vs * vs);
      ux = us / (ubs + (float)1e-10);
      vy = vs / (ubs + (float)1e-10);
      
      /* work the E-W component */
      
      tervar =  tervarwe;
      
      taus = factor * rhos * us * brns * tervar * tervar;
      
      tau[ ku - 1] = taus;
      
      
      /* lift-off */
      
      i__1 =  klev;
      for (k =  ku; k <= i__1; ++k) 
	{
	  
	  /* compute new wave-displacement using the flux from below */
	  
	  if ( brnt[k - 1] <= (float)0. || 
	       ri[k - 1] <= (float)0.) 
	    {
	      
	      /* don't breake waves if there are unstable layers */
	      
	      tau[k] = tau[k - 1];
	    } 
	  else 
	    {
	      brn = sqrt( brnt[k - 1]);
	      
	      r__1 =  u[k - 1] * ux +  v[k - 1] * vy;
	      wproj = fabs(r__1);
	      
	      ul = wproj * ux;
	      
	      vl = wproj * vy;
	      
	      uu = sqrt(ul * ul + vl * vl);
	      
	      dzl2 = tau[k - 1] / (rho[k - 1] * brn * ul * factor + 
				   (float)1e-10);
	      dzl = sqrt(dzl2);
	      
	      /* compute gwri */
	      
	      frud = brn * dzl / (uu + (float)1e-10);
	      
	      ri2 = sqrt( ri[k - 1]);
	      
	      den = (ri2 * frud + (float)1.0) * (ri2 * frud + (float)1.);
	      
	      rigw =  ri[k - 1] * ((float)1.0 - frud) / den;
	      
	      if (rigw >= ric) 
		{
		  tau[k] = tau[k - 1];
		} 
	      else 
		{		  
		  /* compute a new wave-displacement */
		  
		  ri4 = sqrt(ri2);
		  
		  rii = sqrt(ri2 * (float)2.0 + (float)1.0);
		  
		  epp = rifunc_(&ri2, &ri4, &ric);
		  
		  if (epp < (float)0.0) 
		    {
		      epp = (float)0.0;
		    }
		  
		  dzl = epp * uu / brn;
		  
		  dzl2 = dzl * dzl;
		    
		  tau[k] = factor * rho[k - 1] * brn * ul * dzl2;
		}
	    }
	  
	  /* to make TKE multiply flux by 1/rho */
	  
	  tau_tke__ = -(tau[k] - tau[k - 1]) / rho[k - 1];
	  
	  tke_we__[k - 1] = tau_tke__;
	}
    }

  delete[] dsig;
  delete[] rho;
  delete[] tau;

  return 0;
  
} /* we_gwriflux__ */

/* ____________________________________________________________ */

double DtfMwave::rifunc_(float *ri2, float *ri4, float *ric)
{
  /* System generated locals */
  float ret_val;

  /* Local variables */
  float fact, anum;

  fact = sqrt(*ri2 * (*ric * (float)4. + 1) / *ric + (float)4.);

  anum = -(*ri2 / *ric + (float)2.) + *ri4 * fact / sqrt(*ric);
  
  ret_val = anum / (*ri2 * (float)2.);
  
  return ret_val;

} /* rifunc_ */

/* ____________________________________________________________ */

double DtfMwave::vertirreg_(float *f1, float *f2, float *f3, 
		      float *x1, float *x2, float *x3)
{
  /* System generated locals */
  float ret_val;
  
  /* Local variables */
  float dx1, dx2, sdx, rat1, rat2;
  
  dx1 = *x2 - *x1;

  dx2 = *x3 - *x2;

  rat1 = dx1 / dx2;

  rat2 = (float)1. / rat1;

  sdx = (float)1. / (dx1 + dx2);

  ret_val = ((*f3 - *f2) * rat1 + (*f2 - *f1) * rat2) * sdx;

  return ret_val;

} /* vertirreg_ */

/* _____________________________________________________________ */

double DtfMwave::prand_(float *ri)
{
    /* Initialized data */

  static float b = (float)1.;
  static float a = (float)6.873;

  /* System generated locals */
  float ret_val;
  

  if (*ri > (float)1.) 
    {
      ret_val = *ri * (float)7. / b;
    } 
  else 
    {
      if ((float).01 < *ri && *ri < (float)1.) 
	{
	  ret_val = (a * *ri * (a * *ri + (float)1.) + 
		     (float)1.) / (b * (a * *ri + (float)1.));
	} 
      else 
	{
	  ret_val = (float)1. / b;
	}
    }
  
  return ret_val;
  
} /* prand_ */

/* _____________________________________________________________ */

double DtfMwave::atanh_(float *x)
{
  /* System generated locals */
  float ret_val;
  
  if (fabs(*x) > (float)1.) 
    {
      if (*x < (float)0.) 
	{
	  ret_val = (float)-1e30;
	} 
      else 
	{
	  ret_val = (float)1e30;
	}
    } 
  else 
    {
      ret_val = log((*x + (float)1.) / ((float)1. - *x)) * (float).5;
    }
  
  return ret_val;

} /* atanh_ */

/* mwave_ */
/* This routine draws profiles of breaking pressure drag from * */
/* mountain waves and profiles of stability and wind speed.   * */
/* */
  
int DtfMwave::mwave_(float terasmWE, float terasmSN, float tercnvWE, 
		     float tercnvSN)
{
    
    /* System generated locals */
    long  i__1;
    float r__1, r__2;

    /* Local variables */
    float cldf, heff = 0.0, pbar, arfl, amax, hasm, 
      stab, asat, elev, refl, 
      xasm, yasm, 
      cosx, cosy, stab0, h0max = 0.0, h__;
    long  k;
    float ahalf, dthdp, zhalf, xcncv, ycncv, a0, zrefl, h0 = 0.0;
    long  nlyrs;
    float delta0;
    float lambda = 0.0;
    long  jj;
    float ar;
    long  kk, ll = 0, mm, nn;
    float es, qs, rcoeff;
    long havrfl, abvtop;
    float bv0, pi1, pi2, pi3, th1, th2, th3, zhydro, acs, dnl, drg, spd, 
	    spd0, pnu0 = 0.0;


    terasmwe = terasmWE;
    terasmsn = terasmSN;
    tercnvwe = tercnvWE;
    tercnvsn = tercnvSN;
 
    float *dden = new float[klev];
    float *relh = new float[klev];
    float *hght = new float[klev];
    float *ubar = new float[klev];
    float *vbar = new float[klev];
    float *tmpk = new float[klev];
    float *bvsq = new float[klev];
    float *a = new float[klev];
    float *bvsqs = new float[klev];
    float *thetaes = new float[klev];

    /*     --- initializations */
    i__1 =  klev;
    for (k = 1; k <= i__1; ++k) 
      {
	tmpk[k - 1] =  t[k - 1];
	relh[k - 1] =  rh[k - 1];
	hght[k - 1] =  z__[k - 1];
	dden[k - 1] = (float)0.;
	bvsq[k - 1] = (float)0.;
	bvsqs[k - 1] = (float)0.;
	thetaes[k - 1] = (float)0.;
	ubar[k - 1] =  u[k - 1];
	vbar[k - 1] =  v[k - 1];
	a[k - 1] = (float)0.;
	drag[k - 1] = (float)0.;
      }

    elev =  zterain;
    
    /* From the input temperature (K), and an empirical relation */
    /* for saturation vapor pressure wrt water, determine the */
    /* saturation mixing ratio qs(g/g), and equivalent potential */
    /* temperature thetaes (K). */
    
    i__1 =  klev;

    for (k = 1; k <= i__1; ++k) 
      {
	es = (float)-2937.4 /  t[k - 1] - log10( t[k - 1]) *
	  (float)4.9283 + (float)23.5518;
	
	/* compute water vapor sat pess (mb) */
	
	es = pow(10, es);
	
	/* sat mixing ratio (g/g) */
	qs = Eps * es / ( p[k - 1] - es);
	
	thetaes[k - 1] =  vpt[k - 1] *
	  exp( Lv * qs /(cp *  t[k - 1]) );	    
      }

    
    /* compute dry bvsq (1/s^2) */
    
    
    pi1 = pow( p[0] / p00 , rocp);
    
    pi2 = pow( p[1] / p00, rocp);
    
    th1 =  t[0] / pi1;
    
    th2 =  t[1] / pi2;
    
    i__1 =  klev - 1;
    
    for (k = 2; k <= i__1; ++k) 
      {
	pi3 = pow( p[k] / p00 , rocp);

	th3 =  t[k] / pi3;
	
	dthdp = vertirreg_(&th1, &th2, &th3, & p[k - 2], 
			   & p[ k - 1], & p[k]);
	
	 brnt[k - 1] = -  p[k - 1] * g * g / 
	  (pi2 * rd * th2 * th2) * dthdp;
	
	th1 = th2;
	
	th2 = th3;
      
	pi1 = pi2;
      
	pi2 = pi3;
      }
    
     brnt[0] =  brnt[1];
    
     brnt[ klev - 1] =  brnt[ klev - 2];
    
    /*     --- compute moist (saturated) bvsq (1/s^2) */
    th1 = thetaes[0];
    
    th2 = thetaes[1];
    
    i__1 =  klev - 1;
    
    for (k = 2; k <= i__1; ++k) {
      
      pi2 = pow( p[k - 1] / p00, rocp);
      
      th3 = thetaes[k];
      
      dthdp = vertirreg_(&th1, &th2, &th3, & p[k - 2], 
			 & p[k - 1], & p[k]);
      
       brnts[k - 1] = - p[k - 1] * g * g / 
	(pi2 * rd * th2 * th2) * dthdp;
      
      th1 = th2;
      
      th2 = th3;
    }
    
     brnts[0] =  brnts[1];
    
     brnts[ klev - 1] =  brnts[ klev - 2];

    /*  form layer averages */
    nlyrs =  klev - 1;
    
    i__1 = nlyrs;
    
    for (k = 1; k <= i__1; ++k) 
      {
	/* K */
	tmpk[k - 1] = ( t[k - 1] +  t[k]) * (float).5;
      
	/* percent */
	relh[k - 1] = ( rh[k - 1] +  rh[k]) * (float).5;
	
	/* m */
	hght[k - 1] = ( z__[k - 1] +  z__[k]) * (float).5;
	
	/* Pa */
	pbar = ( p[k - 1] +  p[k]) * (float).5;
	
	/* kg/m^3 */
	dden[k - 1] = pbar / (tmpk[k - 1] * rd);
	
	/* 1/s^2 */
	bvsq[k - 1] = ( brnt[k - 1] +  brnt[k]) * (float).5;
	
	/* 1/s^2 */
	bvsqs[k - 1] = ( brnts[k - 1] +  brnts[k]) * (float).5;
	
	/* m/s */
	ubar[k - 1] = ( u[k - 1] +  u[k]) * (float).5;
	
	/* m/s */
	vbar[k - 1] = ( v[k - 1] +  v[k]) * (float).5;
      }

    tmpk[ klev - 1] =  t[ klev - 1];
    
    relh[ klev - 1] =  rh[ klev - 1];
    
    hght[ klev - 1] =  z__[ klev - 1];
    
    dden[ klev - 1] =  p[ klev - 1] / 
      ( t[ klev - 1] * rd);
    
    bvsq[ klev - 1] = bvsq[ klev - 2];
    
    bvsqs[ klev - 1] = bvsqs[ klev - 2];
    
    ubar[ klev - 1] =  u[ klev - 1];
    
    vbar[ klev - 1] =  v[ klev - 1];
    
    
    /* Compute MWAVE. */

    abvtop = false;
    
    amax = (float)1.0;
    
    i__1 = nlyrs;
    
    for (jj = 1; jj <= i__1; ++jj) 
      {
	if (hght[jj - 1] < elev) 
	  {
	     drag[jj - 1] = RMISSD;
	  } 
	else 
	  {
	     drag[jj - 1] = (float)0.;
	    
	    if (! abvtop) 
	      {
	
		/* Computing 2nd power */
		
		r__1 = ubar[jj - 1];
		
		r__2 = vbar[jj - 1];
		
		spd0 = sqrt(r__1 * r__1 + r__2 * r__2);
		
		if (spd0 < (float).1) 
		  {
		    spd0 = (float).1;
		  }
	  
		xcncv =  tercnvwe;
	  
		xasm =  terasmwe;
		
		ycncv =  tercnvsn;
		
		yasm =  terasmsn;
		
		cosx = ubar[jj - 1] / spd0;
		
		cosy = vbar[jj - 1] / spd0;
		
		hasm = xasm * cosx + yasm * cosy;
		
		h__ = xasm * cosx - xcncv * fabs(cosx) + yasm * cosy 
		  - ycncv * fabs(cosy);
		if (h__ < (float)0.) 
		  {
		    h__ = (float)0.;
		  }
	 
	  
		if (bvsq[jj - 1] > (float)1e-6) 
		  {
	  
		    if (relh[jj - 1] > (float)50.) 
		      {
			cldf = (relh[jj - 1] - (float)50.) * (float).02;
	      
			bv0 = cldf * bvsqs[jj - 1] + (1 - cldf) * bvsq[jj - 1];
	      
			if (bv0 > (float)1e-6) 
			  {
			    stab0 = sqrt(bv0);
			  } 
			else 
			  {
			    stab0 = (float).001;
			  }
			
		      } 
		    else 
		      {
			stab0 = sqrt(bvsq[jj - 1]);
		      }
		  } 
		else 
		  {
		    stab0 = (float).001;
		  }
		pnu0 = dden[jj - 1] * stab0 * spd0;
	  
		lambda = spd0 * 2*Pi / stab0;
		
		a0 = stab0 * h__ / spd0;
		
		if (a0 > (float).985) 
		  {
		    heff = h__ * (float).985 / a0;
		  } 
		else 
		  {
		    heff = h__;
		  }
		
		a[jj - 1] = stab0 * heff / spd0;
		
		h0 = a[jj - 1];
		
		if (hght[jj - 1] > hasm + elev) 
		  {
		    abvtop = true;
		    
		    for (mm = jj; mm >= 1; --mm) 
		      {
	      
			if ( drag[mm - 1] != RMISSD) 
			  {
			    a[mm - 1] = a[jj - 1];
			  }
		      }
		  }
	      } else 
		{
	  
		  /* Computing 2nd power */
		  r__1 = ubar[jj - 1];
		  
		  /* Computing 2nd power */
		  r__2 = vbar[jj - 1];
		  
		  spd = sqrt(r__1 * r__1 + r__2 * r__2);
		  
		  if (spd < (float).1) 
		    {
		      spd = (float).1;
		    }
		  if (bvsq[jj - 1] > (float)1e-6) 
		    {
	   
		      if (relh[jj - 1] > (float)50.) 
			{
			  cldf = (relh[jj - 1] - (float)50.) * (float).02;
			  
			  bv0 = cldf * bvsqs[jj - 1] + (1 - cldf) * 
			    bvsq[jj - 1];
			  if (bv0 > (float)1e-6) 
			    {
			      stab = sqrt(bv0);
			    } 
			  else 
			    {
			      stab = (float).001;
			    }
			} 
		      else 
			{
			  stab = sqrt(bvsq[jj - 1]);
			}
		    } 
		  else 
		    {
		      stab = (float).001;
		    }
		  a[jj - 1]  = stab * heff / spd * sqrt(pnu0 / (dden[jj - 1] * 
								stab * spd));
		  if (a[jj - 1] > (float)2.5) 
		    {
		      a[jj - 1] = (float)2.5;
		    }
		  
		  asat = a[jj - 1] / amax;
		  
		  if (a[jj - 1] > (float)1.) 
		    {
		      amax = a[jj - 1];
		      
		      a[jj - 1] = asat;
		    }
	  
		}/*  .not. abvtop */
	    
	  }  /* ( hght(jj) .lt. elev ) */
	
      }
    
    /* *    Check for hydraulic jump-like and reflection/resonance */
    /* *    enhancements then compute breaking wave pressure drag. */
    
    if (abvtop) {
      
      /* *	Find max height of splitting streamline. */
      
      if (fabs(h0) < (float)1e-6) {
	
	acs = Pi/2;
	
	delta0 = (float)0.;
      } 
      else {
	
	delta0 = -sqrt((h0 * h0 + h0 * sqrt(h0 * h0 + 4)) / (float)2.);
	
	acs = acos(h0 / delta0);
      }
      
      kk = 0;
    
      i__1 = nlyrs;
      for (jj = 1; jj <= i__1; ++jj) {
	
	if (hght[jj - 1] < h0max) {
	  
	  kk = jj;
	}
      }
      
      if (kk < nlyrs && kk > 0) 
	{
	
	  amax = a[kk - 1] + (hght[kk - 1] - h0max) / 
	    (hght[kk - 1] - hght[kk]) * (a[kk - 1] - a[kk]);
	  
	  ll = kk;
	  
	  for (mm = kk; mm >= 1; --mm) 
	    {
	      if ( drag[mm - 1] != RMISSD && a[mm - 1] > amax)
		{
		  ll = mm;
		  
		  amax = a[mm - 1];
		  
		  zhydro = hght[mm - 1];
		}
	    }
	  
	  /* 'a' is increased at all levels below max 'a' */
	
	  for (mm = ll; mm >= 1; --mm) 
	    {
	      if ( drag[mm - 1] != RMISSD) 
		{
		  a[mm - 1] = amax;
		}
	    }
	}
      
      for (nn = 0; nn <= 3; ++nn) 
	{
	  /* Find 3/4 vertical wavelength (and 1 3/4, 2 3/4, etc. */
	
	  zrefl = (nn + (float).75) * lambda + elev;
	  
	  kk = 0;
	
	  i__1 = nlyrs;
	  
	  for (jj = 1; jj <= i__1; ++jj) 
	    {
	  
	      if (hght[jj - 1] < zrefl) 
		{
		  kk = jj;
		}
	    }
	  
	  if (kk < nlyrs && kk > 0) 
	    {
	      ar = a[kk - 1] + (hght[kk - 1] - zrefl) / 
		(hght[kk - 1] - hght[kk]) * (a[kk - 1] - a[kk]);
	  
	      /* Find 1/2 vertical wavelength (and 1 1/2, 2 1/2, etc.) */
	  
	      zhalf = (nn + (float).5) * lambda + elev;
	  
	      if (hght[0] > zhalf) 
		{
		  ll = 1;
		} 
	      else 
		{
		  i__1 = kk;
		  for (jj = 1; jj <= i__1; ++jj) 
		    {
		      if (hght[jj - 1] < zhalf) 
			{
			  ll = jj;
			}
		    }
		}
	      
	      ahalf = a[ll - 1] + (hght[ll - 1] - zhalf) / (hght[ll - 1] - hght[ll]) * (a[ll - 1] - a[ll]);
	  
	      if (ahalf <= ar && ahalf < (float).85 && 
		  (r__1 = ar + ahalf, fabs(r__1)) > (float)1e-12) 
		{
		  
		  /* Computing 2nd power */
		  r__1 = ar - ahalf;
		  
		  /* Computing 2nd power */
		  r__2 = ar + ahalf;
		  
		  rcoeff = r__1 * r__1 / (r__2 * r__2);
		  
		  refl = rcoeff * ar;
		  
		  havrfl = true;
		  
		  for (mm = kk; mm >= 1; --mm) {
		    
		    if ( drag[mm - 1] != (float)RMISSD && havrfl) 
		      {
			arfl = a[mm - 1] + refl;
			
			if (arfl > (float).85) 
			  {
			    a[mm - 1] = arfl;
			    if (a[mm - 1] > (float)2.5) 
			      {
				a[mm - 1] = (float)2.5;
			      }
			  } 
			else 
			  {
			    havrfl = false;
			  }
		      }
		  }
		}/* (ahalf .le. ar) .and. (ahalf .lt. 0.85) */
	
	    }/* kk .lt. nlyrs */
       
	}/* nn = 0,3 */
 
      
      i__1 = nlyrs;

      for (jj = 1; jj <= i__1; ++jj) 
	{
	  
	  if ( drag[jj - 1] != RMISSD) {
	    
	    /* dnl is non-linear enhancement coefficient. */
	    /* drg is surface linear wave drag times dnl. */
	    
	    /* Computing 2nd power */
	    r__1 = a[jj - 1];
	    
	    dnl = r__1 * r__1 * (float).4375 + (float)1.;
	  
	    drg = dnl * Pi/4 * heff * pnu0;
	    
	    /* * drg is breaking wave drag of a > 0.67. */
	    
	    if (a[jj - 1] < (float).67) 
	      {
		 drag[jj - 1] = (float)0.;
	      } 
	    else 
	      if (a[jj - 1] > (float).85) 
		{
		   drag[jj - 1] = drg;
		} 
	      else 
		{
		   drag[jj - 1] = (a[jj - 1] - (float).67) / 
		    (float) .18 * drg;
		}
	  }
	}/* jj = 1,nlyrs */
      
    }/* abvtop */
    
    delete[] dden;
    delete[] relh;
    delete[] hght;
    delete[] ubar;
    delete[] vbar;
    delete[] tmpk;
    delete[] bvsq;
    delete[] a;
    delete[] bvsqs;
    delete[] thetaes;

    return 0;

} /* mwave_ */






