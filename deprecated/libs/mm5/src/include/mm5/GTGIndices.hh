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
///////////////////////////////////////////////////////////////////////
// GTGIndices.hh
//
// Compute GTGIndices from MM5 data.
//
// Sue Dettling June 1999 
// 
////////////////////////////////////////////////////////////////////////

#ifndef GTGIndices_H
#define GTGIndices_H


#include <mm5/MM5Data.hh>
#include <physics/DtfMwave.hh>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/file.h>
#include <cstdlib>
#include <math.h>
#include <ctype.h>
using namespace std;

// Indices not computed within this distance in grid points to the boundary.
// These are the RUC defaults.

#define XY_BNDRY 2
#define Z_BNDRY 1

const fl32 p0 = 1000; // mb
const fl32 Rd = 287.04;
const fl32 Rv = 461.5; 
const fl32 Cp = 1004.0;
const fl32 eps = Rd/Rv;
const fl32 nu = Rd/Cp;
const fl32 G  = 9.81;
const fl32 eta = .285714;
//const fl32 PI = 3.14159265358979323846;

class GTGIndices {

public:

  // heartbeat function 

  typedef void (*heartbeat_t)(const char *label);

  typedef enum {
    LESS_THAN,
    GREATER_THAN,
    INSIDE_INTERVAL,
    OUTSIDE_INTERVAL
  } test_sense_t;

  // If you want registration with the procmap, set the heartbeat_func
  // to PMU_auto_register() from toolsa.
  
  GTGIndices(const MM5Data &mm5,
	      const heartbeat_t heartbeat_func = NULL);

  ~GTGIndices(); // Destructor

  // Initialize all fields to "nan"
  void init_all_fields_to_nan(); 

  // The combined index is not computed all the way to the edge
  // of the grid, because surrounding values are required to compute the
  // indices at a point. This function fills in the edges by copying the
  // data from the closest interior point out to the edges.

  void fillEdgesCombined();
   
  void setDebug(bool debug) { _debug = debug; }
  void calculate_indices(bool develop_mode = false);
  void test_indices();
  void write_data_ascii(int, int, const char *proj_type = NULL);
  void write_latlon();
  void write_field(fl32***, const char*, int *);
  void write_field(fl32**, const char*, int *);
  void printInternals(ostream &out);
  void printFields(ostream &out);
  void printField(ostream &out, const string &name, fl32 ***field);

  // override parameter defaults

  void setPhiMFactor(double fac) { phi_m_factor = fac; }
  void setRiCrit(double crit) { ri_crit = ri_crit; }
  void setLazUpperBound(double bound) { laz_upper_bound = bound; }
  void setRiUpperBound(double bound) { ri_upper_bound = bound; }

  // create the combined field

  void clearCombined();

  // add to the combined field

  void addToCombined(string name,
		     fl32 ***field,
		     double weight,
		     test_sense_t sense,
		     double threshold_1,
		     double threshold_2,
		     double min_pressure = 0.0,
		     double max_pressure = 0.0);

  void normalizeCombined();
  void printCombined(ostream &out);

  // arrays access

  int getnI() { return nI; }
  int getnJ() { return nJ; }
  int getnK() { return nK; }
  int getnPoints() { return nPoints; }

  fl32 ***getcalc_siii() const { return siii; }
  fl32 ***getcalc_nva() const { return nva; }
  fl32 ***getcalc_ncsui() const { return ncsui; }
  fl32 ***getcalc_tgrad() const { return t_grad; }
  fl32 ***getcalc_divergence() const { return divergence; }
  fl32 ***getBrown1() const { return brown1; }
  fl32 ***getBrown2() const { return brown2; }
  fl32 ***getCcat() const { return ccat; }
  fl32 ***getColson_panofsky() const { return colson_panofsky; }
  fl32 ***getDef_sqr() const { return def_sqr; }
  fl32 ***getEllrod1() const { return ellrod1; }
  fl32 ***getEllrod2() const { return ellrod2; }
  fl32 ***getDutton() const { return dutton; }
  fl32 ***getEndlich() const { return endlich; }
  fl32 ***getHshear() const { return hshear; }
  fl32 ***getLaz() const { return laz; }
  fl32 ***getPvort() const { return pvort; }
  fl32 ***getPvort_gradient() const { return pvort_gradient; }
  fl32 ***getNgm1() const { return ngm1; }
  fl32 ***getNgm2() const { return ngm2; }
  fl32 ***getRichardson() const { return richardson; }
  fl32 ***getRit() const { return rit; }
  fl32 ***getSat_ri() const { return sat_ri; }
  fl32 ***getStability() const { return stability; }
  fl32 ***getTke_gwb() const { return tke_gwb; }
  fl32 ***getTke_kh3() const { return tke_kh3; }
  fl32 ***getTke_kh4() const { return tke_kh4; }
  fl32 ***getTke_kh5() const { return tke_kh5; }
  fl32 ***getTke_total() const { return tke_total; }
  fl32 ***getVort_sqr() const { return vort_sqr; }
  fl32 ***getVwshear() const { return vwshear; }
  fl32 ***getCombined() const { return combined; }

 
protected:

  bool _debug;

  const MM5Data &_mm5;

  // Methods used to calculate index components:
  
  void calc_common_factors(int,int,int);
  void calc_horizontal_wind_derivs();
  fl32 calc_HorizontalShear();
  void calc_vertical_derivs();
  fl32 calc_zeta();
  fl32 calc_dsh();
  fl32 calc_dst();
  void calc_dtfs();
  fl32 dSigma_dz(fl32);
  fl32 reference_density(fl32);
  fl32 calc_thetav(fl32,fl32,fl32);
  fl32 calc_vws(int,int,int);
  fl32 derivWrtSigma(fl32,fl32,fl32);
  void get_MM5Data_variables(int,int,int);
  int in_mtn_region(fl32, fl32 );
  void loadTerrainVariance();
  fl32 diff_quotient(fl32,fl32,fl32,fl32); 
  fl32 Tv(fl32, fl32);
  

  // Index calculating methods
  
  fl32 Brown1();
  fl32 Brown2();
  fl32 CCAT();
  fl32 ColsonPanofsky();
  fl32 DefSqr();
  fl32 Ellrod1();
  fl32 Ellrod2();
  fl32 Dutton();
  fl32 Endlich();
  fl32 HorizontalShear();
  fl32 LAZ();
  fl32 Pvort();
  fl32 PvortGrad();
  fl32 Ngm1();
  fl32 Ngm2();
  fl32 Ri();
  fl32 Rit();
  fl32 SatRi();
  fl32 Stab();
  fl32 VortSqr();
  fl32 Vws();
  fl32 calc_siii();
  fl32 calc_nva();
  fl32 calc_ncsui();
  fl32 calc_tgrad();
  fl32 calc_divergence();

  // indices arrays: 

  fl32 ***brown1;
  fl32 ***brown2;
  fl32 ***ccat;
  fl32 ***colson_panofsky;
  fl32 ***def_sqr; 
  fl32 ***ellrod1;
  fl32 ***ellrod2;
  fl32 ***dutton;
  fl32 ***endlich;
  fl32 ***hshear;
  fl32 ***laz;
  fl32 ***pvort;
  fl32 ***pvort_gradient;
  fl32 ***ngm1;
  fl32 ***ngm2;
  fl32 ***richardson;
  fl32 ***rit;
  fl32 ***sat_ri;
  fl32 ***stability;
  fl32 ***tke_gwb;  
  fl32 ***tke_kh3; 
  fl32 ***tke_kh4;
  fl32 ***tke_kh5; 
  fl32 ***tke_total; 
  fl32 ***vort_sqr;
  fl32 ***vwshear;
  fl32 ***divergence;
  fl32 ***t_grad;
  fl32 ***siii;
  fl32 ***nva;
  fl32 ***ncsui;

  // combined GTG interest array
  
  fl32 ***combined;
  fl32 ***sumWeights;

  // terrain stats

  fl32 **terrainVarEW;
  fl32 **terrainVarNS;


  // tunable parameters - use the set() functions to override
  // the defaults

  double phi_m_factor;
  double ri_crit;
  double laz_upper_bound;
  double ri_upper_bound;

  // Variables pulled from MM5Data object: 

  fl32 center_lat;
  fl32 center_lon;
  fl32 cone_factor;
  fl32 ds;          // distance between grid points in the horizontal plane. 
  int nI;           // number of coordinates in east-west direction
  int nJ;           // number of coordinates in north-south direction 
  int nK;           // number of vertical coordinates
  int nPoints;      // nI * nJ * nK
  fl32 true_lat1;
  fl32 true_lat2;

  // Used in ccat, brown1, brown2: 

  fl32 coriolis; 

  // Used in calc_divergence,calc_dst,calc_zeta,calc_divergence: 

  fl32 mapf_dot, mapf_dot_east, mapf_dot_north, mapf_dot_northeast;
  
  // Used in calc_divergence,calc_horizontal_wind_derivs,calc_HorizontalShear,
  // calc_zeta,CCAT,RIT, dthetav_dx,dthetav_dy, calc_tgrad: 

  fl32 mapf_x; 
  
  // Used in thetav, theta_v_up1, theta_v_down1 respectively, pvort: 

  fl32 p, p_up1, p_down1;

  // Used in Rit, calc_tgrad 
  fl32 p_east, p_west, p_north, p_south;

  // Used in LAZ: 
  fl32  p_up2, p_down2;

  // Used in Rit 
  fl32 pp, pp_east, pp_west,pp_north,pp_south, pp_up1, pp_down1; 
  
  // Used in dSigma_dz, reference_density,Rit 
  fl32 pstar;

  // Used in Rit 
  fl32 pstar_east, pstar_west, pstar_north, pstar_south;
  
  // Used in reference_density: 
  fl32 pTop;

  // Used in thetav, theta_v_up1, theta_v_down1 respectively: 
  fl32 q, q_up1, q_down1;

  // Used in Rit, calc_tgrad: 
  fl32 q_east, q_west, q_north, q_south;

  // Used in LAZ:
  fl32 q_up2, q_down2;
  fl32 rh, rh_east, rh_west, rh_north, rh_south;
  
  // Used in thetav, theta_v_up1, theta_v_down1 respectively, CCAT: 
  fl32 t, t_up1, t_down1;

  // Used in Rit, CCAT,calc_tgrad: 
  fl32 t_east, t_west, t_north, t_south;

  // Used in CCAT: 
  fl32 t_west_up1, t_west_down1,t_east_up1,t_east_down1,
       t_north_up1,t_north_down1, t_south_up1,t_south_down1;


  // Used in LAZ: 
  fl32 t_up2, t_down2;

  // Used in reference_density: 
  fl32 tlp; 

  // Used in reference_density: 
  fl32 tso; 

  // Used in calc_dst, calc_common_factors(ave_u), calc_horizontal_wind_derivs,
  // calc_divergence, calc_dsh, calc_zeta, Endlich: 
  fl32 u;
 
  // Used in calc_dst, calc_common_factors(ave_u),
  // calc_horizontal_wind_derivs, calc_divergence
  // calc_dsh, calc_zeta:
  fl32 u_east,  u_north, u_northeast;

  // Used in Endlich, Rit: 
  fl32 u_up1;

  // Used in Rit: 
  fl32 u_eastup1,u_northup1,u_northeastup1;
  fl32 u_down1,u_eastdown1,u_northdown1,u_northeastdown1;

  fl32 u_east_up1,u_north_up1,u_northeast_up1;
  fl32 u_east_down1,u_north_down1,u_northeast_down1;

  // Used in calc_dst, calc_common_factors(ave_u),
  // calc_horizontal_wind_derivs, calc_divergence
  // calc_dsh, calc_zeta, Endlich : 

  fl32 v;

  // Used in calc_dst, calc_common_factors(ave_u)
  // calc_horizontal_wind_derivs, calc_divergence
  // calc_dsh, calc_zeta
  fl32 v_east, v_north, v_northeast;

  // Used in Endlich, Rit 
  fl32 v_up1;

  // Used in Rit: 
  fl32 v_eastup1,v_northup1,v_northeastup1;
  fl32 v_down1,v_eastdown1,v_northdown1,v_northeastdown1;

  fl32 v_east_up1,v_north_up1,v_northeast_up1;
  fl32 v_east_down1,v_north_down1,v_northeast_down1;

  // Used in Rit: 
  fl32 w, w_up1, w_down1;

  // Used in calc_common factors( dz_up1:CCAT,Colson_Panofsky) 
  fl32 z, z_up1;
  
  // Used in CCAT 
  fl32 z_down1, z_north, z_south, z_east, z_west;
 
  // Used in LAZ , calc_common_factor(dsigma_up1,dsigma_down1),
  // calc_vertical_derivs.
  fl32 halfSigma;

  // LAZ , calc_common_factor(dsigma_up1,dsigma_down1), Endlich 
  fl32 halfSigma_up, halfSigma_down;

  // In LAZ:
  fl32 halfSigma_up2, halfSigma_down2;

  // common components of indices, and indices or functions in which
  // they are used in comments): 

  fl32 absia;  // ulturb, absia
  fl32 ave_u; //calc_common_factors(wsp),calc_HorizontalShear, Rit
  fl32 ave_v; //calc_common_factors(wsp),calc_HorizontalShear, Rit
  fl32 def; //ngm1,ngm2,eti1,eti2
  fl32 div; //eti2
  fl32 dsh; //phim,def
  fl32 dst; //phim,def
  fl32 dthetav_dz; //stab(Ri, Rit)
  fl32 dsigma_down1; // calc_common_factors(weight_up1,weight_down1),
                     //  derivWrtSigma,
                     // dthetav_dsigma,laz , calc_zeta
  fl32 dsigma_up1;   // calc_common_facotrs(weight_up1,weight_down1)
                     // dthetav_dsigma,end,laz
  fl32 dsigma_dz;    // ccat,dtheta_dz,ngm1, end,laz)
  fl32 dt_dz; //ngm2
  fl32 du_dx; //calc_HorizontalShear, RIT
  fl32 du_dy; //horiz shear, RIT
  fl32 dv_dx; //horiz shear, RIT
  fl32 dv_dy; //horiz shear, RIT
  fl32 dz_up1; //cp,ccat
  fl32 horizontal_shear; //dutton, horizontal shear
  fl32 phi_m; //b1,b2
  fl32 ri; //cp,ri 
  fl32 stab; //ri,endlich. RIT
  fl32 thetav,thetav_down1,thetav_up1;  // theta_m, dthetav_dsigma, dtheta_dz,
                                        //  LAZ, Rit,pvort
  fl32 thetav_m;   //stab#include "
  fl32 vws; //b2,cp,eti1,eti2,ri,Rit,dutton,LAZ
  fl32 vws_up1, vws_down1; //LAZ
  fl32 weight_up1;  //dthetav_dz,cc. 
  fl32 weight_down1;//dthetav_dz,cc.
  fl32 wsp; //ngm2,end
  fl32 wsp_up1; //end
  fl32 zeta; // zeta(calc_common factors), vort2
  fl32 zeta_a; //b1,ccat,pvort
//
// 2/22/2002 Celia
// used in calc_siii, calc_nva, calc_ncsui
  fl32 u_nn;
  fl32 u_nne;
  fl32 u_nee;
  fl32 u_ee;
  fl32 u_se;
  fl32 u_s;
  fl32 u_sw;
  fl32 u_w;
  fl32 u_nw;
//
  fl32 v_nn;
  fl32 v_nne;
  fl32 v_nee;
  fl32 v_ee;
  fl32 v_se;
  fl32 v_s;
  fl32 v_sw;
  fl32 v_w;
  fl32 v_nw;
               
// coriolis_dot is the dot point
  fl32 coriolis_dot;
  fl32 coriolis_deast;
  fl32 coriolis_dnorth;
  fl32 coriolis_dsouth;

  fl32 coriolis_east;
  fl32 coriolis_north;
  fl32 coriolis_south;
  fl32 coriolis_west;
  fl32 coriolis_nn;
  fl32 coriolis_nne;
  fl32 coriolis_nee;
  fl32 coriolis_ee;
  fl32 coriolis_se;
  fl32 coriolis_ne;
  fl32 coriolis_sw;
  fl32 coriolis_nw;

  fl32 mapf_dot_west, mapf_dot_south, mapf_dot_nn, mapf_dot_nne, mapf_dot_nee, mapf_dot_ee, mapf_dot_se, mapf_dot_sw, mapf_dot_nw, mapf_dot_ne, mapf_dot_s, mapf_dot_w;
             
  fl32 mapf_x_east, mapf_x_west, mapf_x_north, mapf_x_south;

  fl32 denom;
  fl32 delvort;
  fl32 vdelv;
                
protected:
  
  heartbeat_t _heartbeatFunc;

};

#endif








