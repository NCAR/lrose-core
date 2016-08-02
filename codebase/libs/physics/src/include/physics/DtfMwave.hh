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
 *   Module: DtfMwave.hh
 *
 *   Author: Sue Dettling
 *
 *   Date:   5/16/01
 *
 *   Description: Header file for class which computes 
 *                Adrian Marroquin's mountain wave turb indices.
 *                 
 */

#ifndef DTFMWAVE_HH
#define DTFMWAVE_HH


#include <math.h>
using namespace std;

const float f00    =  1.45444E-4;   /* 2*Omega (1/sec) */
const float g      =  9.8;          /* m/s^2 */
const float p00    =  1.0E5;        /* Pa */
const float rd     =  287.04;       /* gas const dry air */ 
const float cp     =  1.005E3;      /* gas const dry air */
const float rocp   =  0.285714;     /* Rd/cp (unitless) - RUC2 */
const float Eps    =  0.62197;      /* Mv/Md (unitless) - RUC2 */
const float Lv     =  2.5E6;        /* latent heat of vaporization (J/kg) */
const float Re     =  6.3712E+6;    /* Earth radius (m) - RUC2 */
const float RMISSD =  -999;
const float Pi    =  M_PI;

class DtfMwave
{

public:

  //
  // Constructor
  //
  DtfMwave();
  
  //
  // Destructor
  // 
  ~DtfMwave();

  //
  // initialize 
  //
  int init(int vert_levs, int lowFluxLev, int upFluxLev,
	   float zter, float varSN, float varWE, 
	   float *pres, float *tk, float *uw, float *vw,
	   float *ww, float *zz, float* ptemp , float *vptemp,
	   float *relH, float *mixRat);
  
  //
  // Calculate dtf and mwave indices
  //
  int dtf_();
  
  int mwave_(float terasmWE, float terasmSN, float tercnvWE, float tercnvSN);

  //
  // get functions
  //
  float get_tke_kh3(int k) { return tke_kh3__[k];}
  float get_tke_kh4(int k) { return tke_kh4__[k];}
  float get_tke_kh5(int k) { return tke_kh5__[k];}
  float get_tke_gwb(int k) { return tke_gwb__[k];}
  float get_tke(int k)     { return tke_kh3__[k];}
  float get_drag(int k)    { return drag[k]; } 

private:
  
  int dtf4_code__();
  int dtf5_code__();
  int new_richson_v__();
  int dtf3_code__();
  double rfkondo_(float *ri);
  int new_kep_anl__();
  int new_kep_expo__();
  int sn_gwriflux__(int *nxp, int *nyp);
  int we_gwriflux__(int *nxp, int *nyp);
  double prand_(float *);
  double rf_(float *);
  double vertirreg_(float *f1, float *f2, float *f3, 
		    float *x1, float *x2, float *x3);
  double atanh_(float *);
  double rifunc_(float *, float *, float *);
  inline double dmin( double x , double y) {
    if (x <= y) return x; else return y;
  }

  //
  // number of vertical levels in the model
  //
  int klev;
  
  //
  // lowest vertical level in the model to compute surface fluxes 
  //
  int kl;

  //
  // uppermost vertical level in the model to compute surface fluxes
  //
  int ku;

  //
  // uppermost vertical level in the model to compute surface fluxes
  //
  float *p;

  //
  // pressure in pascals
  //
  float *t;

  //
  // EW wind component (m/s) 
  //
  float *u;

  //
  // NS wind component (m/s) 
  //
  float *v;

  //
  // spatially smoothed vertical velocity (m/s)
  //
  float *agw ;

  //
  // height(m)
  //
  float *z__;

  //
  //  terrain variance in the east-west direction(m)
  //
  float tervarwe;

  //
  //  terrain asymmetry in the east-west direction(m)
  //
  float terasmwe;

  //
  // terrain concavity in the east-west direction(m)
  //
  float tercnvwe;

  //
  // terrain variance in the north-south direction(m)
  //
  float tervarsn;

  //
  // terrain asymmetry in the north-south direction(m)
  //
  float terasmsn;

  //
  // terrain concavity in the north-south direction(m)
  //
  float tercnvsn;

  //
  // 
  //
  float zterain;

  //
  // virtual temperature(K)
  //
  float *tv;

  //
  // virtual potential temperature(K)
  //
  float *vpt;

  //
  // relative humidity (%)
  //
  float *rh;

  //
  // mixing ratio (g/g)
  //
  float *q;

  //
  // sigma level definitions
  //
  float *sig;

  //
  // Brunt-Vaisala frequency squared (s^-2) 
  //
  float *brnt;

  //
  //
  //
  float *brnts;

  //
  // square of the shear(s^-2)
  //
  float *shr;

  //
  // Richardson number 
  //
  float *ri;

  //
  // 
  //
  float *drag;

  //
  // dissipation rate (m^2/s^3)
  //
  float *epsilon;

  //
  //
  //
  float *tke_kh3__;

  //
  //
  //
  float *tke_kh4__;

  //
  //
  //
  float *tke_kh5__;

  //
  //  TKE from Kelvin-Helmholtz instabilities at upper levels
  //
  float *tke_kh__;

  //
  // TKE from grav. wave breaking (east-west comp)(m^2/s^2)
  //
  float *tke_we__;

  //
  // TKE from grav. wave breaking (north-south comp)(m^2/s^2)
  //
  float *tke_sn__;

  //
  // TKE from grav. wave breaking (total) (m^2/s^2)
  //
  float *tke_gwb__;

  //
  // TKE (total of KH + gwb) (m^2/s^2)
  //
  float *tke_dtf__;

  //
  // TKE from convective-induced turbulence
  //
  float *tke_cit__;

  //
  // TKE (total of KH + gwb + cit) (m^2/s^2)
  //
  float *tke;                   
                                    
};


#endif /* DTFMWAVE_HH */
