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
// GTGIndices.cc
//
// Compute GTGIndices from MM5 data.
//
// Sue Dettling June 1999 
//   
////////////////////////////////////////////////////////////////////////


#include <toolsa/mem.h>
#include <mm5/GTGIndices.hh>
#include <iomanip>
#include <ctype.h>
#include <cmath>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>
using namespace std;

/* #define PRINT_DEBUG 1 */

GTGIndices::GTGIndices(const MM5Data &mm5,
			 const heartbeat_t heartbeat_func /* = NULL */) :
  _mm5(mm5),
  _heartbeatFunc(heartbeat_func)
  
{

  _debug = false;

  siii = (fl32 ***) NULL;
  nva = (fl32 ***) NULL;
  ncsui = (fl32 ***) NULL;
  t_grad = (fl32 ***) NULL;
  divergence = (fl32 ***) NULL;
  brown1 = (fl32 ***) NULL;
  brown2 = (fl32 ***) NULL;
  ccat = (fl32 ***) NULL;
  colson_panofsky = (fl32 ***) NULL;
  def_sqr = (fl32 ***) NULL;
  ellrod1 = (fl32 ***) NULL;
  ellrod2 = (fl32 ***) NULL;
  dutton = (fl32 ***) NULL;
  endlich = (fl32 ***) NULL;
  hshear = (fl32 ***) NULL;
  laz = (fl32 ***) NULL;
  pvort = (fl32 ***) NULL;
  pvort_gradient = (fl32 ***) NULL; 
  ngm1 = (fl32 ***) NULL;
  ngm2 = (fl32 ***) NULL;
  richardson = (fl32 ***) NULL;
  rit = (fl32 ***) NULL;
  sat_ri = (fl32 ***) NULL;
  stability = (fl32 ***) NULL;
  tke_gwb = (fl32 ***) NULL;
  tke_kh3 = (fl32 ***) NULL;
  tke_kh4 = (fl32 ***) NULL;
  tke_kh5 = (fl32 ***) NULL;
  tke_total = (fl32 ***) NULL;
  vort_sqr = (fl32 ***) NULL;
  vwshear = (fl32 ***) NULL;
  combined = (fl32 ***) NULL;
  sumWeights = (fl32 ***) NULL;
  terrainVarEW = (fl32**) NULL;
  terrainVarNS = (fl32**) NULL;

  phi_m_factor = 0.3;
  ri_crit = 0.75;
  laz_upper_bound = 100.0;
  ri_upper_bound = 100.0;


  // Pull MM5 single valued variables, dimensions of grid, and whatnot.

  absia = 0;
  center_lat = _mm5.get_center_lat();
  center_lon = _mm5.get_center_lon();
  cone_factor = _mm5.get_cone_factor();
  ds = _mm5.get_grid_distance() * 1000.0; //units = meters
  //ds = _mm5.get_grid_distance() ; //units = km
  nI = _mm5.get_nLon();
  nJ = _mm5.get_nLat();
  nK = _mm5.get_nSigma();
  nPoints = nI * nJ * nK;
  pTop = _mm5.get_pTop();
  tlp = _mm5.get_tlp(); 
  true_lat1 = _mm5.get_true_lat1();
  true_lat2 = _mm5.get_true_lat2();
  tso = _mm5.get_tso();

  // Allocate space for indices
  // umalloc3 puts the data in a contiguous array

  siii = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  nva = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  ncsui = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  
  t_grad = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32)); 
  divergence = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32)); 
  brown1 = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  brown2 = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  ccat = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  colson_panofsky = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  def_sqr = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  ellrod1 = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  ellrod2 = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  dutton = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  endlich = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  hshear  = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  laz = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  pvort = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  pvort_gradient = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  ngm1 = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  ngm2 = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  richardson = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  rit = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32)); 
  sat_ri = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32)); 
  stability = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  tke_gwb = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  tke_kh3 = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  tke_kh4 = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  tke_kh5 = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32));
  tke_total = (fl32 ***)umalloc3(nK, nJ, nI, sizeof(fl32));  
  vort_sqr = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32)); 
  vwshear = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32)); 
  combined = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32)); 
  sumWeights = (fl32 ***) umalloc3(nK, nJ, nI, sizeof(fl32)); 

  terrainVarEW = (fl32 **) umalloc2(nJ, nI, sizeof(fl32));
  terrainVarNS = (fl32 **) umalloc2(nJ, nI, sizeof(fl32));

  // initialize to 0

  int nBytes = nPoints * sizeof(fl32);


  memset(**siii, 0, nBytes);
  memset(**nva, 0, nBytes);
  memset(**ncsui, 0, nBytes);
  
  memset(**t_grad, 0, nBytes);
  memset(**divergence, 0, nBytes);
  memset(**brown1, 0, nBytes);
  memset(**brown2, 0, nBytes);
  memset(**ccat, 0, nBytes);
  memset(**colson_panofsky, 0, nBytes);
  memset(**def_sqr, 0, nBytes);
  memset(**ellrod1, 0, nBytes);
  memset(**ellrod2, 0, nBytes);
  memset(**dutton, 0, nBytes);
  memset(**endlich, 0, nBytes);
  memset(**hshear , 0, nBytes);
  memset(**laz, 0, nBytes);
  memset(**pvort, 0, nBytes);
  memset(**pvort_gradient, 0, nBytes);
  memset(**ngm1, 0, nBytes);
  memset(**ngm2, 0, nBytes);
  memset(**richardson, 0, nBytes);
  memset(**rit, 0, nBytes);
  memset(**sat_ri, 0, nBytes);
  memset(**stability, 0, nBytes);
  memset(**tke_gwb, 0, nBytes);
  memset(**tke_kh3, 0, nBytes);   
  memset(**tke_kh4, 0, nBytes);
  memset(**tke_kh5, 0, nBytes);
  memset(**tke_total, 0, nBytes);  
  memset(**vort_sqr, 0, nBytes);
  memset(**vwshear, 0, nBytes);
  memset(**combined, 0, nBytes);
  memset(**sumWeights, 0, nBytes);

  memset(*terrainVarNS, 0, nI*nJ*sizeof(fl32));
  memset(*terrainVarEW, 0, nI*nJ*sizeof(fl32));
  

}

GTGIndices::~GTGIndices(){

  /* free 3D arrays */

  if (siii != NULL){
    ufree3((void ***)siii);
  }
  if (nva != NULL){
    ufree3((void ***)nva);
  }
  if (ncsui != NULL){
    ufree3((void ***)ncsui);
  }
 
  if (t_grad != NULL){
    ufree3((void ***)t_grad);
  }  
  if (divergence != NULL){
    ufree3((void ***)divergence);
  }  
  if (brown1 != NULL){
    ufree3((void ***)brown1);
  } 
  if (brown2 != NULL){
    ufree3((void ***)brown2);
  }
  if (ccat != NULL){
    ufree3((void ***)ccat);
  }
  if (colson_panofsky != NULL){
    ufree3((void ***)colson_panofsky);
  }
  if (def_sqr != NULL){
    ufree3((void ***)def_sqr);
  }
  if (ellrod1 != NULL){
    ufree3((void ***)ellrod1);
  }
  if (ellrod2 != NULL){
    ufree3((void ***)ellrod2);
  }
  if (dutton != NULL){
    ufree3((void ***)dutton);
  }
  if (endlich != NULL){
    ufree3((void ***)endlich);
  }
  if (hshear != NULL){
    ufree3((void ***)hshear);
  }  
  if (laz != NULL){
    ufree3((void ***)laz);
  }  
  if (pvort != NULL){
    ufree3((void ***)pvort);
  }  
  if (pvort_gradient != NULL){
    ufree3((void ***)pvort_gradient);
  }  
  if (ngm1 != NULL){
    ufree3((void ***)ngm1);
  }  
  if (ngm2 != NULL){
    ufree3((void ***)ngm2);
  }  
  if (richardson != NULL){
    ufree3((void ***)richardson);
  }  
  if (rit != NULL){
    ufree3((void ***)rit);
  }  
  if (sat_ri != NULL){
    ufree3((void ***)sat_ri);
  }  
  if (stability != NULL){
    ufree3((void ***)stability);
  }  
  if ( tke_gwb!= NULL){
    ufree3((void ***)tke_gwb);
  }  
  if ( tke_kh3!= NULL){
    ufree3((void ***)tke_kh3);
  }  
  if ( tke_kh4!= NULL){
    ufree3((void ***)tke_kh4);
  }  
  if ( tke_kh5!= NULL){
    ufree3((void ***)tke_kh5);
  }  
  if ( tke_total!= NULL){
    ufree3((void ***)tke_total);
  }  
  if (vort_sqr != NULL){
    ufree3((void ***)vort_sqr);
  }  
  if (vwshear != NULL){
    ufree3((void ***)vwshear);
  }  
  if (combined != NULL){
    ufree3((void ***)combined);
  }  
  if (sumWeights != NULL){
    ufree3((void ***)sumWeights);
  }  
  if (terrainVarEW) {
    ufree2((void **) terrainVarEW);
  }
  if (terrainVarNS) {
    ufree2((void **) terrainVarNS);
  }

}

/////////////////////////////////////////////////////////////////////////////////
// Function Name: init_fields_to_nan()
// 
// Description: initialize all field values to NAN
//
// Returns: 
//
// Notes: 
//
//

void GTGIndices::init_all_fields_to_nan()

{

  // compute a nan

  fl32 nan = log(-1.0);

  for(int i = 0; i < nI; i++) {
    
    for(int j = 0; j < nJ; j++) {
      
      terrainVarNS[j][i] = nan;
      terrainVarEW[j][i] = nan;

      for( int k = 0; k < nK; k++ ) {
  
	brown1[k][j][i] = nan;
	brown2[k][j][i] = nan;
	ccat[k][j][i] = nan;
	colson_panofsky[k][j][i] = nan;
	def_sqr[k][j][i] = nan;
	ellrod1[k][j][i] = nan;
	ellrod2[k][j][i] = nan;
	dutton[k][j][i] = nan;
	endlich[k][j][i] = nan;
	hshear[k][j][i] = nan;
	laz[k][j][i] = nan;
	pvort[k][j][i] = nan;
	pvort_gradient[k][j][i] = nan;
	ngm1[k][j][i] = nan;
	ngm2[k][j][i] = nan;
	richardson[k][j][i] = nan;
	rit[k][j][i] = nan;
	sat_ri[k][j][i] = nan;
	stability[k][j][i] = nan;
	tke_gwb[k][j][i] = nan;
	tke_kh3[k][j][i] = nan;
	tke_kh4[k][j][i] = nan;
	tke_kh5[k][j][i] = nan;
	tke_total[k][j][i] = nan;
	vort_sqr[k][j][i] = nan;
	vwshear[k][j][i] = nan;
        combined[k][j][i] = nan;
        sumWeights[k][j][i] = nan;

      } // k

    } // j

  } // i

}   

/////////////////////////////////////////////////////////////////////////////
//
// Function Name: fillEdgesCombined()
// 
// Description: The combined index is not computed all the way to the edge
// of the grid, because surrounding values are required to compute the
// indices at a point. This function fills in the edges by copying the
// data from the closest interior point out to the edges.

void GTGIndices::fillEdgesCombined()

{

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("fillEdgesCombined()");
  }

  // check dimensions

  if (nK < Z_BNDRY || nJ < XY_BNDRY || nI < XY_BNDRY) {
    return;
  }

  // first fill egde points in in dimension i
  
  for (int k = Z_BNDRY; k < nK - Z_BNDRY ; k++) {
    
    for (int j = XY_BNDRY; j < nJ - XY_BNDRY; j++) {
      
      for (int i = 0; i < XY_BNDRY; i++) {
	combined[k][j][i] = combined[k][j][XY_BNDRY];
      }

      for (int i = nI - XY_BNDRY; i < nI; i++) {
	combined[k][j][i] = combined[k][j][nI - 1 - XY_BNDRY];
      }

    } // j

  } // k

  // then fill in edge rows in dimension j

  for (int k = Z_BNDRY; k < nK - Z_BNDRY ; k++) {
    
    for (int j = 0; j < XY_BNDRY; j++) {
      // for (int i = 0; i < nI; i++) {
      //   combined[k][j][i] = combined[k][XY_BNDRY][i];
      // }
      memcpy(combined[k][j], combined[k][XY_BNDRY],
 	     nI * sizeof(fl32));
    }
    
    for (int j = nJ - XY_BNDRY; j < nJ; j++) {
      // for (int i = 0; i < nI; i++) {
      //   combined[k][j][i] = combined[k][nJ - 1 - XY_BNDRY][i];
      // }
      memcpy(combined[k][j], combined[k][nJ - 1 - XY_BNDRY],
 	     nI * sizeof(fl32));
    }

  } // k

  // then fill in edge planes in dimension k

  for (int k = 0; k < Z_BNDRY; k++) {
    //     for (int j = 0; j < nJ; j++) {
    //       for (int i = 0; i < nI; i++) {
    // 	combined[k][j][i] = combined[Z_BNDRY][j][i];
    //       }
    //     }
    memcpy(*combined[k], *combined[Z_BNDRY],
	   nI * nJ * sizeof(fl32));
  }
  
  for (int k = nK - Z_BNDRY; k < nK; k++) {
    //     for (int j = 0; j < nJ; j++) {
    //       for (int i = 0; i < nI; i++) {
    // 	combined[k][j][i] = combined[nK - 1 - Z_BNDRY][j][i];
    //       }
    //     }
    memcpy(*combined[k], *combined[nK - 1 - Z_BNDRY],
    //memcpy(*combined[k], *combined[nK - Z_BNDRY],
	   nI * nJ * sizeof(fl32));
  }

}
/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: calc_common_factors
// 
// Description: calculates factors of the indices which are common to more
//              than one index.
//
// Returns: 
//
// Notes: 
//
//
void GTGIndices::calc_common_factors(int i, int j, int k) {

  dsigma_up1= halfSigma_up - halfSigma;
  dsigma_down1 = halfSigma - halfSigma_down;

  // Calculate weights for upper and lower finite differences
  // in derivatives with respect to sigma:

  weight_up1 = fabs(dsigma_down1)/(fabs(dsigma_up1) + fabs(dsigma_down1));
  weight_down1 = fabs(dsigma_up1)/(fabs(dsigma_up1) + fabs(dsigma_down1));
  
  calc_horizontal_wind_derivs();
  
  // Calculate the mean potential temp:

  thetav = calc_thetav(p, q, t);

  //printf("t,q,p,thetav= %f %f %f %f\n", t,q,p,thetav);

  thetav_up1 =  calc_thetav(p_up1, q_up1, t_up1);
  thetav_down1 =  calc_thetav(p_down1, q_down1, t_down1);

  calc_vertical_derivs(); // dt_dz, dthetav_dz, dsigma_dz
  
  thetav_m = ((thetav_up1 + thetav)/2 * weight_up1 +
	      (thetav + thetav_down1)/2 * weight_down1);

  div = calc_divergence();
  dsh = calc_dsh();
  dst = calc_dst();

  // Calculate vertical wind shear.
  // Note this should probably be calculated once and
  // loaded into an array.

  vws =  calc_vws(i,j,k);

  if (k >= 2) {
    vws_down1 = calc_vws(i,j,k-1);
  } else {
    vws_down1 = 0.0;
  }

  if (k < nK - 2) {
    vws_up1 =  calc_vws(i, j,k+1);
  } else {
    vws_up1 = 0.0;
  }

  zeta = calc_zeta();
  zeta_a = zeta + coriolis;
  def = sqrt( dst * dst + dsh * dsh);
  stab = G/thetav_m * dthetav_dz;
  dz_up1 = z_up1 - z_down1;
  ri =  stab/(vws*vws); // requires vws and stab to be defined.
  //  printf ("thetav_m,dthetav_dz = %f %f \n",thetav_m,dthetav_dz);
  //  printf ("stab,vws,ri = %f %f %f \n",stab,vws,ri);

  ave_u = (u + u_east + u_north + u_northeast)/4;
  ave_v = (v + v_east + v_north + v_northeast)/4;
  wsp = sqrt(ave_u * ave_u + ave_v * ave_v) ;

  //fl32 ave_u_up1 = (u_up1 + u_northup1 + u_northeastup1 + u_eastup1)/4;
  //fl32 ave_v_up1 = (v_up1 + v_northup1 + v_northeastup1 + v_eastup1)/4; 
  
  //fl32 wsp_up1 = sqrt(ave_u_up1 * ave_u_up1 + ave_v_up1 * ave_v_up1) ;
  //printf ("ave_u,ave_v,wsp = %f %f %f \n",ave_u,ave_v,wsp);
  //   printf ("ave_u_up1,ave_v_up1,wsp_up1 = %f %f %f \n",
  // 	  ave_u_up1,ave_v_up1,wsp_up1);
  
  calc_horizontal_wind_derivs();
  horizontal_shear =  calc_HorizontalShear();
  phi_m = sqrt(phi_m_factor * zeta_a * zeta_a + dsh * dsh + dst * dst);
  
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: calc_horizontal_wind_derivs
// 
// Description: Calculates du_dx, dv_dx, du_dy, dv_dy 
//
// Returns: 
//
// Notes: The following variables are defined in get_MM5Data_variables:
//        u, u_north, u_northeast, u_east,
//        v, v_north, v_northeast, v_east,
//        mapf_x
//
//        ds is defined in function init.
//
void GTGIndices::calc_horizontal_wind_derivs(){
  
  // calculate horizontal wind derivatives:

  du_dx = ((u_northeast + u_east) - (u_north + u))/(2*ds) * mapf_x;
  dv_dx = ((v_northeast + v_east) - (v_north + v))/(2*ds) * mapf_x;
  du_dy = ((u_north + u_northeast) - (u + u_east))/(2*ds) * mapf_x;
  dv_dy = ((v_north + v_northeast) - (v + v_east))/(2*ds) * mapf_x;

}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: calc_divergence
// 
// Description: Calculates divergence = m^2 *( d(u/m)/dx + d(v/m)/dy)
//              where m is the map scale factore
//
// Returns: divergence value
//
// Notes: The following variables are defined in get_MM5Data_variables:
//        u, u_north, u_northeast, u_east,
//        v, v_north, v_northeast, v_east,
//        mapf_dot, mapf_dot_north, mapf_dot_northeast,mapf_dot_east,
//        mapf_x
//
//        ds is defined in init.
fl32 GTGIndices:: calc_divergence(){  
  
  fl32 duoverm_dx =
    ((u_northeast/mapf_dot_northeast + u_east/mapf_dot_east)
     - (u_north/mapf_dot_north + u/mapf_dot))/(2*ds);

  fl32 dvoverm_dy =
    (( v_north/mapf_dot_north + v_northeast/ mapf_dot_northeast)
     -(v/mapf_dot + v_east/mapf_dot_east))/(2*ds);

  return (mapf_x * mapf_x * (duoverm_dx + dvoverm_dy));

}


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: calc_dsh
// 
// Description: Calculates shearing deformation = d(mv)/dx + d(mu)/dy
//
// Returns: shearing deformation value
//
// Notes:  The following variables are defined in get_MM5Data_variables:
//         u,u_north,u_northeast, u_east,
//         v, v_north, v_northeast, v_east,
//         mapf_dot, mapf_dot_north, mapf_dot_northeast,mapf_dot_east, 
//
//         ds is defined in init.
//
fl32 GTGIndices:: calc_dsh(){
  
  fl32 dmv_dx =
    ((mapf_dot_northeast * v_northeast + mapf_dot_east * v_east) -
     (mapf_dot_north * v_north + mapf_dot * v)) / (2 * ds );

  fl32 dmu_dy =
    ((mapf_dot_north * u_north + mapf_dot_northeast * u_northeast) -
     (mapf_dot * u + mapf_dot_east * u_east )) / (2 * ds);

  return(dmv_dx + dmu_dy);

}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: calc_dtfs
// 
// Description: Calculates tke_kh3, tke_kh4, tke_kh5 , tke_gwb, and tke_total
//
// Returns: 
//
// Notes: calc_dtfs fills up a struct,dtfMwaveVars, declared in DtfMwv.hh. This 
//        struct corresponds to a common block in flds.inc which is included 
//        the fortran file in dtf_driver.f. Unfortunately , the number of 
//        vertical levels in the model are hardwired in fortran code and
//        in DtfMwv.hh. 
//        
//        dtfmwv_.kL, dtfmwv_.ku should be in a param file since they
//        vary with model.
//        CHECK DEF OF dtfmwv_.agw[k] !!!!!!!!!!!!!!!

void GTGIndices::calc_dtfs()
{

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("calc_dtfs()");
  }

  fl32 zter, varSN, varWE;

  loadTerrainVariance();

  TaArray<fl32> _agw, _p, _t, _u, _v, _z, _rh, _q, _vpt, _Tv, _temperature;
  fl32 *agw = _agw.alloc(nK);
  fl32 *p = _p.alloc(nK);
  fl32 *t = _t.alloc(nK);
  fl32 *u = _u.alloc(nK);
  fl32 *v = _v.alloc(nK);
  fl32 *z = _z.alloc(nK);
  fl32 *rh = _rh.alloc(nK);
  fl32 *q = _q.alloc(nK);
  fl32 *vpt = _vpt.alloc(nK);
  fl32 *Tv = _Tv.alloc(nK);
  fl32 *temperature = _temperature.alloc(nK);
  
  for(int i = 0; i < nI; i++) {
    for(int j = 0; j < nJ; j++) {
      
      memset(agw, 0, sizeof(fl32) * nK);
      memset(p, 0, sizeof(fl32) * nK);
      memset(t, 0, sizeof(fl32) * nK);
      memset(u, 0, sizeof(fl32) * nK);
      memset(v, 0, sizeof(fl32) * nK);
      memset(z, 0, sizeof(fl32) * nK);
      memset(rh, 0, sizeof(fl32) * nK);
      memset(q, 0, sizeof(fl32) * nK);
      memset(vpt, 0, sizeof(fl32) * nK);
      memset(Tv, 0, sizeof(fl32) * nK);
      memset(temperature, 0, sizeof(fl32) * nK);

      //
      // define 2D vars 
      //
      zter = _mm5.get_terrain(i,j);
      varSN =  terrainVarNS[j][i];
      varWE =  terrainVarEW[j][i];

      //
      // get columns of model data
      //
      for( int k=0; k<nK; k++ ) {
	agw[k] = _mm5.get_w(i, j, k);   
	p[k] = _mm5.get_pres(i, j, k);
	t[k] = _mm5.get_tk(i, j, k);
	u[k] = _mm5.get_u_cross(i, j, k);
	v[k] = _mm5.get_v_cross(i, j, k);
	z[k] = _mm5.get_z(i, j, k);
	rh[k] = _mm5.get_rh(i, j, k);
	q[k] = _mm5.get_q(i, j, k);
	vpt[k] = t[k] * ( 1 + q[k]/eps)/(1 + q[k]);
	Tv[k] = t[k] * ( 1 + q[k]/eps)/(1 + q[k]);
	//Tv[k] = vpt[k] * pow((double) p0/p[k], (double) eta);
      }
      
      // 
      // Initialize DtfMwave object
      //
      DtfMwave dtfmwave;

      dtfmwave.init(nK, 1, 8, zter, varSN, varWE, p, t, u, v, 
		    agw, z, Tv, vpt, rh, q); 

      //
      // Calculate dtf turb indices;
      //
      dtfmwave.dtf_();
      
      //
      // Get the index values for each column
      //
      for(int k = 0; k < nK; k++) {
	tke_kh3[k][j][i] = dtfmwave.get_tke_kh3(k);
	tke_kh4[k][j][i] = dtfmwave.get_tke_kh4(k);
	tke_kh5[k][j][i] = dtfmwave.get_tke_kh5(k);
	tke_gwb[k][j][i] = dtfmwave.get_tke_gwb(k);
	tke_total[k][j][i] = dtfmwave.get_tke(k);
	//printf("tke_kh3,tke_kh5= %f %f \n", tke_kh3[k][j][i],tke_kh5[k][j][i]);
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: in_mtn_region
// 
// Description: Tests whether a give (lat,lon) pair corresponds to
//              a point in a mountainous region.
//
// Returns: 1 if pair is in mtn region, 0 otherwise
//
// Notes: This would have to be modified depending on region covered bu model
//        
int GTGIndices::in_mtn_region(fl32 lat, fl32 lon) {

  // Cascades:
  if ( (46.5 <= lat) && (lat <= 48.0) && (-121 <= lon) && (lon <= -119))
    return(1);

  // S. Sierra Nevada:
  if ( ( lat <= 38 ) && ( lat >= 36) && ( lat <= -2 * lon - 198) &&
       (lat >=  -2 * lon - 202))
    return(1);

  // N. Sierra Nevada:
  if( (lat <= 40) && (lat >= 38) && ( lat <=  -2 * lon - 198) &&
      ( lat >= -2 * lon - 202) )
    return(1);

  // Ruby Ridge:  if( (lat <= 39) && (lat >= 37.5) && (lon <= -112) && (lon >= -114))
  // return(1);

  // Wasatch:
  if( (lat <= 42) && (lat >= 39) && (lon <= -112) && (lon >= -114) )
    return(1);

  // N. Montana Rockies:
  if( (lat <= 47) && (lat >= .2 * lon + 67.8) && (lon >= -114) &&
      (lon <= -109))
    return(1);

  // mid Montana Rockies:
  if( (lat >= .2 * lon + 66.6) && (lat <= .2 * lon + 67.8) &&
      (lat <= -lon - 63) && ( lat >= -lon - 69) )
    return(1);

  // S. Montana Rockies:
  if ( (lat >= 42) && (lon <= -108) && ( lat <= .2 * lon + 66.6 ) &&
       (lat >= -2 * lon - 182) )
    return(1);

  // Wyoming Rockies:
  if ( (lat <= 42) && (lat >= 40.5) && (lon >= -107) && (lon <= -104))
    return(1);

  // Colorado Rockies:
  if( (lat <= 40.5) && (lat >= 37.5 ) && (lon >= -107) && (lon <= -104))
    return(1);

  // S. Colorado Rockies:
  if ( (lat <= 37.5) && (lat >= 35) && (lon >= -107) && (lon <= -104) )
    return(1);

  // Big Horn:
  if ( (lat <= 45) && (lat >= 42) && (lon >= -107) && (lon <= -104) )
    return(1);

  return(0);

}




/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: calc_HorizontalShear
// 
// Description: horiz shear =
//  (u^2* du/dy - uv*dv/dy + uv*du/dx + v^2*dv/dx)/wsp^2
//   where u and v are the horizontal components of the wind
//   and wsp is wind speed.
//
// Returns: value of horiz shear
//
// Notes:  The following variables are defined in calc_common_factors:
//     ave_u,
//     ave_v,
//     wsp
//
//     The following variables are defined in calc_horizontal_wind_derivs():
//     du_dx, du_dy,
//     dv_dx, dv_dy
//

fl32 GTGIndices::calc_HorizontalShear(){

  fl32 shear1 = (- ave_u * ave_u * du_dy + - ave_u * ave_v * dv_dy 
		 + ave_u * ave_v * du_dx + ave_v * ave_v * dv_dx)/(wsp * wsp);
  return(shear1);

}

#ifdef NOTNOW
fl32 GTGIndices::calc_HorizontalShear(){
  fl32 wsp_jp1 =
    sqrt((u_north + u_northeast) * (u_north + u_northeast) /4 +  
 	 (v_north + v_northeast) * (v_north + v_northeast)/4);
  fl32 wsp_j = sqrt((u + u_east) * (u + u_east) /4 +  
 		    (v + v_east) * (v + v_east)/4);
  fl32 dwsp_dy = (wsp_jp1 - wsp_j)/ds * mapf_x;
  fl32 wsp_i = sqrt((u_north + u) * (u_north + u)/4 
 		    + (v_north + v) * (v_north + v)/4);
  fl32 wsp_ip1 = sqrt((u_northeast + u_east) * (u_northeast + u_east)/4 
 		      + (v_northeast + v_east) * (v_northeast + v_east) /4);
  fl32 dwsp_dx = (wsp_ip1 - wsp_i)/ds * mapf_x;
  fl32 shear2 =  - ave_u/wsp * dwsp_dy + ave_v/wsp * dwsp_dx;
  return (shear2);
}
#endif

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: reference_density
// 
// Description:  calculates reference density
//
// Returns: 
//
//
// Notes:  The following variables are defined in get_MM5Data_variables:
//         pstar
//
//         The following variables are defined in function init:
//         pTop,
//         tso,
//         tlp 
//
//         p0 = 1000mb
//         Rd = 287.04
//
fl32 GTGIndices:: reference_density(fl32 sigma){
  
  fl32 pref = sigma * pstar + pTop*100; // pressures in Pa
  fl32 ref_temp = tso + tlp * log(pref/(p0*100)); //p0 in mb 
  fl32 ref_density = pref /(Rd*ref_temp);
  return(ref_density);

}



/////////////////////////////////////////////////////////////////////////////////
//
// Function Name:  dSigma_dz
// 
// Description: dSigma_dz calculates the derivative of sigma with respect to 
//              altitude.
//
// Returns: 
//
// Notes: pstar is defined in get_MM5Data_variables:
//        
//        G is the acceleration of gravity.
//
fl32 GTGIndices:: dSigma_dz( fl32 sigma){
  
  fl32 ref_density =  reference_density(sigma); 
  fl32 dsigma_dz = -ref_density * G / pstar;
  //printf("refd,g,pstar= %f %f %f \n", ref_density, G, pstar);
  return(dsigma_dz);
}


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name:  calc_dst
// 
// Description: calculates stretching deformation,dst.
//              dst = d(mu)/dx - d(mv)/dy where u and v are horizontal
//              components of the wind and m is a map scale factor.
//
// Returns: dst
//
// Notes: All variables defined in function get_MM5Data_variables
//        except ds defined in init.
//
fl32 GTGIndices:: calc_dst(){
  
 
  fl32 dmu_dx =
    ((mapf_dot_northeast * u_northeast + mapf_dot_east * u_east) -
     (mapf_dot_north * u_north + mapf_dot * u)) / (2 * ds);
  
  fl32 dmv_dy =
    ((mapf_dot_north * v_north + mapf_dot_northeast * v_northeast) -
     (mapf_dot * v + mapf_dot_east * v_east )) / (2 * ds);	
  
  return(dmu_dx - dmv_dy);

}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: 
// 
// Description: 
//
// Returns: 
//
// Notes: 
//
//
void GTGIndices::calculate_indices(bool develop_mode /*= false*/)

{

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("calculate_indices()");
  }

  double min_rit = 1.0e99;
  double max_rit = -1.0e99;

  for (int k = Z_BNDRY; k < nK - Z_BNDRY ; k++) {

    for (int j = XY_BNDRY; j < nJ - XY_BNDRY; j++) {
      if (_heartbeatFunc != NULL) {
	_heartbeatFunc("calculate_indices()");
      }
      for (int i = XY_BNDRY; i < nI - XY_BNDRY; i++) {
	
	get_MM5Data_variables(i,j,k);
	calc_common_factors(i,j,k);
	
	brown1[k][j][i] = Brown1();
	brown2[k][j][i] = Brown2();
	ccat[k][j][i] = CCAT();
	colson_panofsky[k][j][i] = ColsonPanofsky();
	def_sqr[k][j][i] = DefSqr();
	ellrod1[k][j][i] = Ellrod1();
	ellrod2[k][j][i] = Ellrod2();
	dutton[k][j][i] = Dutton();
	endlich[k][j][i] = Endlich();
	hshear[k][j][i] = HorizontalShear();
	laz[k][j][i] = LAZ();
	pvort[k][j][i] = Pvort();
	//pvort_gradient[k][j][i] =  PvortGrad();
	ngm1[k][j][i] = Ngm1();
	ngm2[k][j][i] = Ngm2();
	richardson[k][j][i] =  Ri();
	rit[k][j][i] =  Rit();
	min_rit = MIN(min_rit, rit[k][j][i]);
	max_rit = MAX(max_rit, rit[k][j][i]);
	//sat_ri[k][j][i] = SatRi();
	stability[k][j][i] = Stab();
	vort_sqr[k][j][i] = VortSqr();
	vwshear[k][j][i] = Vws();
        divergence[k][j][i] = calc_divergence();
        t_grad[k][j][i] = calc_tgrad();
	siii[k][j][i] = calc_siii();
	nva[k][j][i] = calc_nva();
	ncsui[k][j][i] = calc_ncsui();
  
#ifdef PRINT_DEBUG
	printf(" brown1= %e\n", brown1[k][j][i]);
	printf(" brown2= %e\n", brown2[k][j][i]);
	printf(" ccat= %e\n", ccat[k][j][i]);
	printf(" colson_panofsky= %e\n", colson_panofsky[k][j][i]);
	printf(" def_sqr= %e\n", def_sqr[k][j][i]);
	printf(" ellrod1= %e\n", ellrod1[k][j][i]);
	printf(" ellrod2= %e\n", ellrod2[k][j][i]);
	printf(" dutton= %e\n", dutton[k][j][i]);
	printf(" endlich= %e\n", endlich[k][j][i]);
	printf(" hshear= %e\n", hshear[k][j][i]);
	printf(" pvort= %e\n", pvort[k][j][i]);
	printf(" ngm1= %e\n", ngm1[k][j][i]);
	printf(" ngm2= %e\n", ngm2[k][j][i]);
	printf(" richardson= %e\n", richardson[k][j][i]);
	printf(" rit= %e\n", rit[k][j][i]);
	printf(" stability= %e\n", stability[k][j][i]);
	printf(" vort_sqr= %e\n", vort_sqr[k][j][i]);
	printf(" tke_gwb= %e\n", tke_gwb[k][j][i]);
	printf(" tke_kh3= %e\n", tke_kh3[k][j][i]);
	printf(" tke_kh4= %e\n", tke_kh4[k][j][i]);
	printf(" tke_kh5= %e\n", tke_kh5[k][j][i]);
	printf(" tke_total= %e\n", tke_total[k][j][i]);
	printf(" divergence= %e\n", divergence[k][j][i]);
	printf(" t_grad= %e\n", t_grad[k][j][i]);
	printf(" siii= %e\n", siii[k][j][i]);
	printf(" nva= %e\n", nva[k][j][i]);
	printf(" ncsui= %e\n", ncsui[k][j][i]);
#endif
  

      } // i
    } // j
  } // k

  if (develop_mode) {
    calc_dtfs();
  }

  if (_debug) {
    cerr << "Min RIT: " << min_rit << endl;
    cerr << "Max RIT: " << max_rit << endl;
  }

  if (develop_mode) {
    int dim_array[3] = { nI, nJ, nK};
    write_field(brown1, "400", dim_array);
    write_field(brown2 , "401", dim_array);
    write_field(ccat , "406", dim_array);
    write_field(colson_panofsky , "402", dim_array);
    write_field(def_sqr , "422", dim_array);
    write_field(ellrod1 , "403", dim_array);
    write_field(ellrod2 , "404", dim_array);
    write_field(dutton , "412", dim_array);
    write_field(endlich , "413", dim_array);
    write_field(hshear , "420", dim_array);
    write_field(laz , "414", dim_array);
    write_field(pvort , "424", dim_array);
    write_field(pvort_gradient , "430", dim_array);
    write_field(ngm1 , "415", dim_array);
    write_field(ngm2 , "416", dim_array);
    write_field(richardson , "405", dim_array);
    write_field(rit , "429", dim_array);
    write_field(sat_ri , "429", dim_array);
    write_field(stability, "421", dim_array);
    write_field(tke_gwb, "408", dim_array);
    write_field(tke_kh3, "409", dim_array);
    write_field(tke_kh4, "431", dim_array);
    write_field(tke_kh5, "432", dim_array);
    write_field(tke_total,"407", dim_array);
    write_field(vort_sqr, "423", dim_array);
    write_field(vwshear , "418", dim_array);
    write_field(divergence , "434", dim_array);
    write_field(t_grad , "438", dim_array);
    write_field(siii , "447", dim_array);
    write_field(nva , "448", dim_array);
    write_field(ncsui , "449", dim_array) ;
  
    
    dim_array[0] = nI;
    dim_array[1]= nJ;
    dim_array[2] = 1;
    write_field(terrainVarEW,"terrainVarEW",dim_array);
    write_field(terrainVarNS,"terrainVarNS",dim_array);
  }
    
  
}


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: loadTerrainVariance()
// 
// Description: calculates and loads the variance
//              into arrays terrainVarEW, terrainVarNS
//
// Returns:
//
// Notes: This could be calculated once and read from a file rather than 
//        calculating it every time we compute the dtf indices.
//
void GTGIndices::loadTerrainVariance(){

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadTerrainVariance()");
  }

  //
  // Compute EW variance
  //

  for(int j = 0; j < nJ; j++) {
    for(int i = 1; i < nI-1; i++) {
      
      fl32 zij = _mm5.get_terrain(i, j);
      fl32 zim1j = _mm5.get_terrain(i-1, j);
      fl32 zip1j = _mm5.get_terrain(i+1, j);

      fl32 trav = (zij + zim1j + zip1j)/3;
      fl32 zmax = MAX(MAX(zij, zim1j), MAX(zim1j, zip1j));
      fl32 zmin = MIN(MIN(zij, zim1j), MIN(zim1j, zip1j));
      fl32 zv = fabs((zmax - trav)*(zmin - trav));
      terrainVarEW[j][i] = sqrt(zv);
    }
  }

  //
  // Assign values to edge of grid
  //

  for(int j = 0; j < nK; j++) {
    terrainVarEW[j][0] = terrainVarEW[j][1];
    terrainVarEW[j][nI - 1] = terrainVarEW[j][nI - 2];
  }

  //
  // Compute NS variance
  //
  
  // calculate the S-N terrain variance

  for(int i = 0; i < nI; i++) {
    for(int j = 1; j < nJ-1; j++) {
      
      fl32 zij = _mm5.get_terrain(i, j);
      fl32 zijm1 = _mm5.get_terrain(i, j-1);
      fl32 zijp1 = _mm5.get_terrain(i, j+1);
      
      fl32 trav = (zij + zijm1 + zijp1)/3;
      fl32 zmax = MAX(MAX(zij, zijm1), MAX(zijm1, zijp1));
      fl32 zmin = MIN(MIN(zij, zijm1), MIN(zijm1, zijp1));
      fl32 zv = fabs((zmax-trav)*(zmin-trav));
      terrainVarNS[j][i] = sqrt(zv);
    }
  }

  //
  // assign valuess to edge of grid.
  //

  for(int i = 0; i < nK; i++) {
    terrainVarNS[0][i] = terrainVarNS[1][i];
    terrainVarNS[nJ - 1][i] = terrainVarNS[nJ -2][i];
  }

  
}
 
/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: calc_thetav
// 
// Description: calculates virtual potential temperature
//
// Returns: the value of virtual potential temperature
//
// Notes: p0 = 1000mb
//        eps = Rd/Rv = 0.6220 where Rd = 287.04 and Rv = 461.5
//
//
fl32 GTGIndices::calc_thetav(fl32 p, fl32 q, fl32 t){

  // p must be in mb, t must be in degrees K

  fl32 Tv; // virtual temperature
  fl32 theta_v; // virtual potential temperature

  Tv = t * ( 1 + q/eps)/(1 + q);

  theta_v = Tv * pow((double) p0/p, (double) nu);
  

  return(theta_v);

}


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: calc_vertical_derivs()
// 
// Description: 
//
// Returns: 
//
// Notes: 
//
//
void GTGIndices::calc_vertical_derivs(){

  // Calculate dthetav_dz:

  fl32 dthetav_dsigma = derivWrtSigma(thetav_up1,thetav,thetav_down1);
  dsigma_dz =  dSigma_dz(halfSigma);
  dthetav_dz = dthetav_dsigma * dsigma_dz; 
  
  // Calculate first derivatives of t with respsect to z:

  dt_dz = derivWrtSigma(t_up1,t,t_down1)* dsigma_dz;

}


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: calc_vws
// 
// Description: vws = sqrt( (dU/dz)^2 + (dV/dz)^2 )
//              vws is computed at the dot points surrounding a cross point and
//              the max of these values is taken as the vws at the cross point.
//              
// Returns: 
//
// Notes: 
//
//
fl32 GTGIndices:: calc_vws(int i, int j, int k){

  // Calculate derivatives of u and v with respect to z:
  //   NOTE: In calculating derivatives of u or v with respect to z,
  //   we multiply a derivative with respect to sigma by dsigma_dz.
  //   derivatives of u or v with respect to sigma are valid for dot points,
  //   while dsigma_dz is computed using variables at cross points.
  //   We accept dsigma_dz at the cross point as an approximation
  //   for dsigma_dz at a dot point.
  
  // get values of u,v,HalfSigma  from MM5Data object:

  fl32 u = _mm5.get_u_dot(i, j, k);
  fl32 u_up1 = _mm5.get_u_dot(i, j, k+1);
  fl32 u_down1 = _mm5.get_u_dot(i, j, k-1);
  fl32 u_east = _mm5.get_u_dot(i+1,j,k);
  fl32 u_east_up1 =  _mm5.get_u_dot(i+1, j, k+1);
  fl32 u_east_down1 = _mm5.get_u_dot(i+1, j, k-1);
  fl32 u_north = _mm5.get_u_dot(i,j+1,k);
  fl32 u_north_up1 = _mm5.get_u_dot(i,j+1,k+1);
  fl32 u_north_down1 = _mm5.get_u_dot(i,j+1,k-1);
  fl32 u_northeast = _mm5.get_u_dot(i+1,j+1,k);
  fl32 u_northeast_up1 = _mm5.get_u_dot(i+1,j+1,k+1);
  fl32 u_northeast_down1 = _mm5.get_u_dot(i+1,j+1,k-1);

  fl32 v = _mm5.get_v_dot(i, j, k);
  fl32 v_up1 = _mm5.get_v_dot(i, j, k+1);
  fl32 v_down1 = _mm5.get_v_dot(i, j, k-1);
  fl32 v_east = _mm5.get_v_dot(i+1,j,k);
  fl32 v_east_up1 =  _mm5.get_v_dot(i+1, j, k+1);
  fl32 v_east_down1 = _mm5.get_v_dot(i+1, j, k-1);
  fl32 v_north = _mm5.get_v_dot(i,j+1,k);
  fl32 v_north_up1 = _mm5.get_v_dot(i,j+1,k+1);
  fl32 v_north_down1 = _mm5.get_v_dot(i,j+1,k-1);
  fl32 v_northeast = _mm5.get_v_dot(i+1,j+1,k);
  fl32 v_northeast_up1 = _mm5.get_v_dot(i+1,j+1,k+1);
  fl32 v_northeast_down1 = _mm5.get_v_dot(i+1,j+1,k-1);

  fl32 halfSigma = _mm5.get_halfSigma(k);
  fl32 halfSigma_up = _mm5.get_halfSigma(k+1);
  fl32 halfSigma_down = _mm5.get_halfSigma(k-1);

  fl32 dsigma_up1 = halfSigma_up - halfSigma;
  fl32 dsigma_down1 = halfSigma - halfSigma_down;


  // Calculate weights for upper and lower finite differences in derivatives
  // with respect to sigma:

  fl32 weight_up1 = fabs(dsigma_down1)/(fabs(dsigma_up1) + fabs(dsigma_down1));
  fl32 weight_down1 = fabs(dsigma_up1)/(fabs(dsigma_up1) + fabs(dsigma_down1));

  fl32 dsigma_dz =  dSigma_dz(halfSigma);

  // calculate du_dz:
  fl32 du_dsigma =
    (u_up1 - u) / dsigma_up1 * weight_up1  
    + (u - u_down1) / dsigma_down1 * weight_down1; 
  fl32 du_dz = du_dsigma * dsigma_dz;

  // Calculate du_dz_east:
  fl32 du_dsigma_east =
    (u_east_up1 - u_east) / dsigma_up1 * weight_up1
    + (u_east - u_east_down1)/ dsigma_down1   * weight_down1;
  fl32 du_dz_east = du_dsigma_east * dsigma_dz;

  // Calculate du_dz_north:
  fl32 du_dsigma_north =
    (u_north_up1 - u_north) / dsigma_up1 * weight_up1 
    + (u_north - u_north_down1) / dsigma_down1 *  weight_down1;
  fl32 du_dz_north = du_dsigma_north * dsigma_dz;

  // Calculate du_dz_northeast:
  fl32 du_dsigma_northeast =
    (u_northeast_up1 - u_northeast) / dsigma_up1 * weight_up1 
    + (u_northeast - u_northeast_down1) / dsigma_down1 *  weight_down1;
  fl32 du_dz_northeast = du_dsigma_northeast * dsigma_dz;
  
  // Calculate dv_dz:
  fl32 dv_dsigma =
    (v_up1 - v) / dsigma_up1 * weight_up1 
    + (v - v_down1) / dsigma_down1 * weight_down1; 
  fl32 dv_dz = dv_dsigma * dsigma_dz;
  
  // Calculate dv_dz_east: 
  fl32 dv_dsigma_east =
    (v_east_up1 - v_east)  / dsigma_up1 * weight_up1
    + (v_east - v_east_down1)  / dsigma_down1 * weight_down1;
  fl32 dv_dz_east = dv_dsigma_east * dsigma_dz;
  
  // Calculate du_dz_north:
  fl32 dv_dsigma_north =
    (v_north_up1 - v_north) / dsigma_up1 * weight_up1 
    + (v_north - v_north_down1) / dsigma_down1 *  weight_down1;
  fl32 dv_dz_north = dv_dsigma_north * dsigma_dz;
  
  // Calculate dv_dz_northeast:
  fl32 dv_dsigma_northeast =
    (v_northeast_up1 - v_northeast) / dsigma_up1 * weight_up1 
    + (v_northeast - v_northeast_down1) / dsigma_down1 *  weight_down1;
  fl32 dv_dz_northeast = dv_dsigma_northeast * dsigma_dz;
  
  fl32 dV_dz = pow((double) du_dz * du_dz + dv_dz * dv_dz, 0.5); 
  fl32 dV_dz_east =
    sqrt(du_dz_east * du_dz_east + dv_dz_east * dv_dz_east);
  fl32 dV_dz_north =
    sqrt(du_dz_north * du_dz_north + dv_dz_north * dv_dz_north);
  fl32 dV_dz_northeast =
    sqrt(du_dz_northeast * du_dz_northeast + 
	 dv_dz_northeast * dv_dz_northeast);
  
  fl32 vws;

  if(dV_dz > dV_dz_east)
    vws = dV_dz;
  else
    vws = dV_dz_east;
  
  if(vws < dV_dz_north) 
    vws = dV_dz_north;
  
  if(vws < dV_dz_northeast)
    vws = dV_dz_northeast;

  return(vws);
}



fl32 GTGIndices:: calc_zeta(){  
 
  fl32 dvoverm_dx = ((v_northeast/mapf_dot_northeast + v_east/mapf_dot_east)
		     - (v_north/mapf_dot_north + v/mapf_dot) )/(2*ds);
  
  fl32 duoverm_dy = ((u_north/mapf_dot_north + u_northeast/mapf_dot_northeast)
		     - (u/mapf_dot + u_east/mapf_dot_east))/(2*ds);
  
  return(mapf_x * mapf_x * ( dvoverm_dx - duoverm_dy));
}

fl32 GTGIndices::derivWrtSigma(fl32 a_up1,fl32 a, fl32 a_down1){

  fl32 deriv = ((a_up1 - a)/dsigma_up1 * weight_up1 +
		(a - a_down1)/dsigma_down1 * weight_down1);
  return deriv;


} 

void GTGIndices::get_MM5Data_variables(int i, int j ,int k){

  
  halfSigma = _mm5.get_halfSigma(k);
  halfSigma_up = _mm5.get_halfSigma(k+1);
  halfSigma_down = _mm5.get_halfSigma(k-1);

  mapf_dot = _mm5.get_mapf_dot(i, j);
  mapf_dot_east = _mm5.get_mapf_dot(i+1, j);
  mapf_dot_north = _mm5.get_mapf_dot(i, j+1);
  mapf_dot_northeast = _mm5.get_mapf_dot(i+1, j+1);
  
  mapf_x = _mm5.get_mapf_x(i, j);

  p = _mm5.get_pres(i, j, k);
  p_east = _mm5.get_pres(i+1, j, k);
  p_west = _mm5.get_pres(i-1, j, k);
  p_north = _mm5.get_pres(i, j+1, k);
  p_south = _mm5.get_pres(i, j-1, k);
  p_up1 = _mm5.get_pres(i, j, k+1);
  p_down1 = _mm5.get_pres(i, j, k-1);

  pp = _mm5.get_pp(i,j,k);
  pp_east = _mm5.get_pp(i+1,j,k);
  pp_west = _mm5.get_pp(i-1,j,k);
  pp_north = _mm5.get_pp(i,j+1,k);
  pp_south = _mm5.get_pp(i,j-1,k);
  pp_up1 = _mm5.get_pp(i,j,k+1);
  pp_down1 = _mm5.get_pp(i,j,k-1);
  
  pstar = _mm5.get_pstar(i,j);
  pstar_east = _mm5.get_pstar(i+1,j);
  pstar_west = _mm5.get_pstar(i-1,j);
  pstar_north = _mm5.get_pstar(i,j+1);
  pstar_south = _mm5.get_pstar(i,j-1);
  
  q = _mm5.get_q(i, j, k);
  q_east = _mm5.get_q(i+1, j, k);
  q_west = _mm5.get_q(i-1, j, k);
  q_north = _mm5.get_q(i, j+1, k);
  q_south = _mm5.get_q(i, j-1, k);
  q_up1 = _mm5.get_q(i, j, k+1);
  q_down1 = _mm5.get_q(i, j, k-1);

  rh = _mm5.get_rh(i, j, k);
  rh_east = _mm5.get_rh(i+1, j, k);
  rh_west = _mm5.get_rh(i-1, j, k);
  rh_north = _mm5.get_rh(i, j+1, k);
  rh_south = _mm5.get_rh(i, j-1, k);

  t = _mm5.get_tk(i, j, k);
  t_east = _mm5.get_tk(i+1, j, k);
  t_west = _mm5.get_tk(i-1, j, k);
  t_north = _mm5.get_tk(i, j+1, k);
  t_south = _mm5.get_tk(i, j-1, k);
  t_up1 = _mm5.get_tk(i, j, k+1);
  t_down1 = _mm5.get_tk(i, j, k-1);    
  t_west_up1 = _mm5.get_tk(i-1,j,k+1);
  t_west_down1 = _mm5.get_tk(i-1,j,k-1);
  t_east_up1 = _mm5.get_tk(i+1,j,k+1);
  t_east_down1 = _mm5.get_tk(i+1,j,k-1);
  t_north_up1 = _mm5.get_tk(i,j+1,k+1);
  t_north_down1 = _mm5.get_tk(i,j+1,k-1);
  t_south_up1 = _mm5.get_tk(i,j-1,k+1);
  t_south_down1 = _mm5.get_tk(i,j-1,k-1);
  
  u = _mm5.get_u_dot(i, j, k);
  u_east = _mm5.get_u_dot(i+1 ,j ,k);
  u_north = _mm5.get_u_dot(i, j+1, k);
  u_northeast = _mm5.get_u_dot(i+1, j+1, k);
  u_up1 = _mm5.get_u_dot(i,j,k+1);
  u_eastup1 = _mm5.get_u_dot(i+1,j,k+1);
  u_northeastup1 = _mm5.get_u_dot(i+1,j+1,k+1);
  u_northup1 = _mm5.get_u_dot(i,j+1,k+1);
  u_down1 = _mm5.get_u_dot(i,j,k-1);
  u_eastdown1 = _mm5.get_u_dot(i+1,j,k-1);
  u_northeastdown1 = _mm5.get_u_dot(i+1,j+1,k-1);
  u_northdown1 = _mm5.get_u_dot(i,j+1,k-1);
    
  v = _mm5.get_v_dot(i, j, k);
  v_east = _mm5.get_v_dot(i+1 ,j ,k);
  v_north = _mm5.get_v_dot(i, j+1, k);
  v_up1 = _mm5.get_v_dot(i,j,k+1);
  v_northeast = _mm5.get_v_dot(i+1, j+1, k);
  v_eastup1 = _mm5.get_v_dot(i+1,j,k+1);
  v_northeastup1 = _mm5.get_v_dot(i+1,j+1,k+1);
  v_northup1 = _mm5.get_v_dot(i,j+1,k+1);
  v_down1 = _mm5.get_v_dot(i,j,k-1);
  v_eastdown1 = _mm5.get_v_dot(i+1,j,k-1);
  v_northeastdown1 = _mm5.get_v_dot(i+1,j+1,k-1);
  v_northdown1 = _mm5.get_v_dot(i,j+1,k-1);
  v_up1 = _mm5.get_v_dot(i,j,k+1);

  w = _mm5.get_w(i,j,k);
  w_up1 = _mm5.get_w(i,j,k+1);
  w_down1 = _mm5.get_w(i,j,k-1);

  z = _mm5.get_z(i,j,k);
  z_up1 = _mm5.get_z(i,j,k+1);
  z_down1 = _mm5.get_z(i,j,k-1);
  z_north = _mm5.get_z(i,j+1,k);
  z_south = _mm5.get_z(i,j-1,k);
  z_east = _mm5.get_z(i+1,j,k);
  z_west = _mm5.get_z(i-1,j,k);

  // 2/22/2002 Celia

  mapf_dot_nn= _mm5.get_mapf_dot(i ,j+2);
  mapf_dot_nne= _mm5.get_mapf_dot(i+1 ,j+2);
  mapf_dot_nee= _mm5.get_mapf_dot(i+2 ,j+1);
  mapf_dot_ee= _mm5.get_mapf_dot(i+2 ,j);
  mapf_dot_se= _mm5.get_mapf_dot(i+1 ,j-1);
  mapf_dot_s= _mm5.get_mapf_dot(i ,j-1);
  mapf_dot_w= _mm5.get_mapf_dot(i-1 ,j);
  mapf_dot_ne= _mm5.get_mapf_dot(i+1 ,j+1);
  mapf_dot_nw= _mm5.get_mapf_dot(i-1 ,j+1);
  mapf_x_east = _mm5.get_mapf_x(i+1, j);
  mapf_x_west = _mm5.get_mapf_x(i-1, j);
  mapf_x_north = _mm5.get_mapf_x(i, j+1);
  mapf_x_south = _mm5.get_mapf_x(i, j-1);
  u_nn= _mm5.get_u_dot(i ,j+2 ,k);
  u_nne= _mm5.get_u_dot(i+1 ,j+2 ,k);
  u_nee= _mm5.get_u_dot(i+2 ,j+1 ,k);
  u_ee= _mm5.get_u_dot(i+2 ,j ,k);

  u_se= _mm5.get_u_dot(i+1 ,j-1 ,k);
  u_s= _mm5.get_u_dot(i ,j-1 ,k);
  u_w= _mm5.get_u_dot(i-1 ,j ,k);
  u_nw= _mm5.get_u_dot(i-1 ,j+1 ,k);
  v_nn= _mm5.get_v_dot(i ,j+2 ,k);
  v_nne= _mm5.get_v_dot(i+1 ,j+2 ,k);
  v_nee= _mm5.get_v_dot(i+2 ,j+1 ,k);
  v_ee= _mm5.get_v_dot(i+2 ,j ,k);

  v_se= _mm5.get_v_dot(i+1 ,j-1 ,k);
  v_s= _mm5.get_v_dot(i ,j-1 ,k);
  v_w= _mm5.get_v_dot(i-1 ,j ,k);
  v_nw= _mm5.get_v_dot(i-1 ,j+1 ,k);

  //
  // 02/21/2002 (Mike/Celia)
  // coriolis is the value at the cross point
  // coriolis_dot is the value at the dot point
  
  coriolis_dot = _mm5.get_coriolis_dot(i,j);

  coriolis = _mm5.get_coriolis(i,j);
  coriolis_east = _mm5.get_coriolis(i+1,j);
  coriolis_north = _mm5.get_coriolis(i,j+1);
  coriolis_south = _mm5.get_coriolis(i, j-1);
  coriolis_west = _mm5.get_coriolis(i-1, j);

  if (k >= 2) {
    halfSigma_down2 = _mm5.get_halfSigma(k-2);
    p_down2 = _mm5.get_pres(i,j,k-2);
    q_down2 = _mm5.get_q(i,j,k-2);
    t_down2 = _mm5.get_tk(i,j,k-2);
  } else {
    halfSigma_down2 = _mm5.get_halfSigma(k-1);
    p_down2 = _mm5.get_pres(i,j,k-1);
    q_down2 = _mm5.get_q(i,j,k-1);
    t_down2 = _mm5.get_tk(i,j,k-1);
  }

  if (k < nK - 2) {
    halfSigma_up2 = _mm5.get_halfSigma(k+2);
    p_up2 = _mm5.get_pres(i,j,k+2);
    q_up2 = _mm5.get_q(i,j,k+2);
    t_up2 = _mm5.get_tk(i,j,k+2);
  } else {
    halfSigma_up2 = _mm5.get_halfSigma(k+1);
    p_up2 = _mm5.get_pres(i,j,k+1);
    q_up2 = _mm5.get_q(i,j,k+1);
    t_up2 = _mm5.get_tk(i,j,k+1);
  }
  
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: 
// 
// Description: 
//
// Returns: 
//
// Notes: 
//
//
void GTGIndices::test_indices(){
  
  int i,j,k;

  char *quit_flag;

  quit_flag = new char[2];

  // initialize quit_flag to something other than q or Q.
  sprintf(quit_flag,"a");

  while ( (strcmp(quit_flag,"q") != 0)  && (strcmp(quit_flag,"Q") != 0)) {

    cerr << "Enter i, j, k\n" << "\n";
    cin >> i >> j >> k;
    
    
    if( (i >= XY_BNDRY) && (i < nI - XY_BNDRY ) &&
	(j >= XY_BNDRY) && (j < nJ - XY_BNDRY) &&
	(k >=  Z_BNDRY) && (k < nK - Z_BNDRY) ) {	  

      get_MM5Data_variables(i,j,k);
      calc_common_factors(i,j,k);
      calc_dtfs();
      
      fl32 browns1 = Brown1();
      fl32 browns2 = Brown2();
      fl32 eti1 = Ellrod1();
      fl32 eti2 = Ellrod2();
      fl32 ccat = CCAT();
      fl32 cp = ColsonPanofsky();
      fl32 def2 = DefSqr();
      fl32 emp = Dutton();
      fl32 end = Endlich();
      fl32 hs = HorizontalShear();
      fl32 laz = LAZ();
      fl32 pvort = Pvort();
      fl32 pvortgrad = PvortGrad();
      fl32 ngm1 = Ngm1();
      fl32 ngm2 = Ngm2();
      fl32 rich =  Ri();
      fl32 rit =  Rit();
      fl32 satr = SatRi();
      fl32 stability = Stab();
      fl32 vort2 = VortSqr();
      fl32 vshear = Vws();
      fl32 divergence = calc_divergence();
      fl32 t_grad = calc_tgrad();
      fl32 siii = calc_siii();
      fl32 nva = calc_nva();
      fl32 ncsui = calc_ncsui();
      
      printf("browns1 = %g\n", browns1);
      printf("browns2 =%g\n", browns2);
      printf("eti1 = %g\n", eti1);
      printf("eti2 = %g\n", eti2);
      printf("ccat = %g\n", ccat);
      printf("cp = %g\n", cp );
      printf("def2 = %g\n", def2);
      printf("emp = %g\n", emp);
      printf("end = %g\n", end);
      printf("hs =  %g\n", hs);
      printf("laz = %g\n", laz);
      printf("pvort = %g\n", pvort);
      printf("pvortgrad = %g\n", pvortgrad);
      printf("ngm1 = %g\n", ngm1);
      printf("ngm2 = %g\n", ngm2);
      printf("rich = %g\n", rich);
      printf("rit = %g\n", rit);
      printf("satr = %g\n", satr);
      printf("stability = %g\n", stability);
      printf("vort2 = %g\n", vort2);
      printf("vshear = %g\n", vshear);
      printf("divergence = %g\n", divergence);
      printf("t_grad = %g\n", t_grad);
      printf("siii = %g\n", siii);
      printf("nva = %g\n", nva);
      printf("ncsui = %g\n", ncsui);

    } else {
      
      cerr << "\n" << i << " " << j << " " <<  k 
	   << " out of bounds\n";
      cerr << "min i = " << XY_BNDRY << ", min j =  " << XY_BNDRY  
	   << ", min k =  " << Z_BNDRY << "\n\n";
      cerr << "max i =  " << nI - XY_BNDRY - 1 << ", max j =  "
	   << nJ - XY_BNDRY -1  
	   << ", max k =  " << nK - Z_BNDRY - 1<< "\n\n";
    }

    cerr << "Enter Q or q to quit or any other letter to continue:\n";
    cin >> quit_flag;

  } // while

}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: 
// 
// Description: 
//
// Returns: 
//
// Notes: 
//
//

void GTGIndices::write_latlon()

{
  FILE *fptr;
  
  char outfile[36];

  sprintf(outfile,"LatLon.MM5");

  if ( (fptr = fopen(outfile,"w")) == NULL)
    {
      cerr << "Trouble opening file for writing data.";
      exit(1);
    } 
 
  fprintf(fptr,"true_lat1 true_lat: %g %g\n",
	  _mm5.get_true_lat1(), _mm5.get_true_lat2() );
  fprintf(fptr, "nLat,nLon: %d %d \n", nJ,nI);
  fprintf(fptr, "i,j,lat_x,lon_x:\n");
  for (int j = 0 ; j < nJ ; j++) {
    for (int i = 0  ; i < nI ; i++) {
      fprintf(fptr, "%d %d %g %g \n", i, j,
	      _mm5.get_lat_x(i,j), _mm5.get_lon_x(i,j));
    }
  }

}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: write_data_ascii
// 
// Description: 
//
// Returns: 
//
// Notes: Used get columns of data centered around I,J
//
//
void GTGIndices::write_data_ascii(int I, int J,
				   const char *proj_type /*= NULL */)

{
  
  FILE *fptr;
  char outfile[36];
  int i0,j0;

  sprintf(outfile,"MM5_data.%.2d_%.2d", I, J);
  
  if ( (fptr = fopen(outfile,"w")) == NULL) {
    cerr << "Trouble opening file for writing data.";
    exit(1);
  }     

  if (proj_type != NULL) {
    fprintf(fptr,"Proj type: %s\n", proj_type);
  }

  fprintf(fptr,"nX,nY,nSigma,ds %d %d %d %g\n", nI,nJ,nK,ds);

  fprintf(fptr,"center_lat,center_lon,cone_factor,true_lat1,true_lat2: "
	  "%g %g %g %g %g\n", 
	  _mm5.get_center_lat(),
	  _mm5.get_center_lon(),
	  _mm5.get_cone_factor(),
	  _mm5.get_true_lat1(),
	  _mm5.get_true_lat2() );

  fprintf(fptr,"ptop,pos,tso,tlp: %g %g %g %g\n",
	  _mm5.get_pTop(),
	  _mm5.get_pos(),
	  _mm5.get_tso(),
	  _mm5.get_tlp() );

  fprintf(fptr, "i,j,k,halfSigma,u,v,tk,qq,pp,rh,zz,pres,rnw,clw,pstar,f," 
	  "mapf_x,mapf_dot,terrain,ground_t,lhflux,shflux,"
	  "pbl_height,pbl_regime,"
	  "ust,lat,lon,w: \n");

  if (I == 0 && J == 0 ) {
    I=nI;
    J=nJ;
    j0=0;
    i0=0;
  }
  else {
    j0 = J-1;
    i0 = I-1;
  }

  fprintf(stderr, " I,J,K = %d %d %d\n" , I,J,nK);

  for (int k = 0; k < nK ; k++) {
    for (int j = j0  ; j < J   ; j++) {
      for (int i = i0 ; i < I; i++) {
	fprintf(fptr, "%d %d %d %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g"
		" %g %g %g %g %g %g %g %g %g %g\n",
		i, j, k,
		_mm5.get_halfSigma(k),
		_mm5.get_u_dot(i,j,k),
		_mm5.get_v_dot(i,j,k),
		_mm5.get_tk(i,j,k),
		_mm5.get_q(i,j,k),
		_mm5.get_pp(i,j,k),
		_mm5.get_rh(i,j,k),
		_mm5.get_z(i,j,k),
		_mm5.get_pres(i,j,k),
		_mm5.get_rnw(i,j,k),
		_mm5.get_clw(i,j,k),
      		_mm5.get_pstar(i,j),
		_mm5.get_coriolis_dot(i,j),
		_mm5.get_mapf_x(i,j),
		_mm5.get_mapf_dot(i,j),
		_mm5.get_terrain(i,j),
		_mm5.get_ground_t(i,j),
		_mm5.get_lhflux(i,j),
		_mm5.get_shflux(i,j),
		_mm5.get_pbl_hgt(i,j),
		_mm5.get_regime(i,j),
		_mm5.get_ust(i,j),
		_mm5.get_lat_x(i,j),
		_mm5.get_lon_x(i,j),
                _mm5.get_w(i,j,k));
      } // ilon
    } // ilat
  } // isig

  fclose(fptr);
     
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: write_field
// 
// Description: prints data from MM5Data field array into *.F format
//              so it can be read and rendered by existing matlab code for GTG.
//              
// Returns: 
//
// Notes: Existing matlab code requires order of data to be xyz
//
//

void GTGIndices::write_field(fl32*** field,
			     const char* filename_base,
			      int *dim_array){

  char filename[25];
  FILE *fld_file;
  int num_dim;
  int fld_size;
  int n;

  // create filename:
  sprintf(filename,"%s.F",filename_base);

  //open outfile:
  if( (fld_file = fopen(filename, "w")) == NULL ) {
    cerr << "\nTrouble opening " << filename << endl;
    exit(1);
  }

  // get array dimensions and fld_size:
  if( dim_array[2] == 1 ){
    num_dim = 2;
    fld_size = dim_array[0]*dim_array[1];
  }
  else{
    num_dim = 3;
    fld_size = dim_array[0]*dim_array[1]*dim_array[2];
  }

  /* writes number of dimensions */
  if( fwrite((char *) &num_dim, sizeof(int), 1, fld_file) != 1 ) {
    printf("Trouble writing num_dim\n\n");
    exit(1);
  }

  /* writes dimensions in order xyz */
  if( (int) fwrite((char *) dim_array, sizeof(int),num_dim , fld_file)
      != num_dim ) {
    printf("Trouble writing dim_array\n\n");
    exit(1);
  }
 
  
  /* write field array */
  for(int k = 0; k < nK ; k++)
    for (int j = 0; j< nJ; j++)
      for (int i = 0; i < nI ; i++)
	{
	  n = fwrite((char *) &field[k][j][i], sizeof(fl32), 1, fld_file); 
	}
  
  fclose( fld_file );
 
}   


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: write_field
// 
// Description: prints data from MM5Data field array into *.F format
//              so it can be read and rendered by existing GTG matlab code.
//              
// Returns: 
//
// Notes: Existing matlab code requires order of data to be xyz.
//
//
void GTGIndices::write_field(fl32** field,
			     const char* filename_base,
			      int *dim_array)

{

  char filename[25];
  FILE *fld_file;
  int num_dim;
  int fld_size;
  int n;

  // create filename:
  sprintf(filename,"%s.F",filename_base);

  //open outfile:
  if( (fld_file = fopen(filename, "w")) == NULL ) {
    cerr << "\nTrouble opening " << filename << endl;
    exit(1);
  }

  // get array dimensions and fld_size:
  if( dim_array[2] == 1 ){
    num_dim = 2;
    fld_size = dim_array[0]*dim_array[1];
  }
  else{
    num_dim = 3;
    fld_size = dim_array[0]*dim_array[1]*dim_array[2];
  }

  /* writes number of dimensions */
  if( fwrite((char *) &num_dim, sizeof(int), 1, fld_file) != 1 ) {
    printf("Trouble writing num_dim\n\n");
    exit(1);
  }

  /* writes dimensions in order xyz */
  if( (int) fwrite((char *) dim_array, sizeof(int), num_dim, fld_file)
      != num_dim ) {
    printf("Trouble writing dim_array\n\n");
    exit(1);
  }
 
  /* write field array */
  for (int j = 0; j<dim_array[1]; j++)
    for (int i = 0; i < dim_array[0] ; i++)
      {
	n = fwrite((char *) &field[j][i], sizeof(fl32), 1, fld_file);
	if(n !=1)
	  cerr << filename << ": write failed " << i << " " << j << endl;
      }

  fclose( fld_file );

}   

////////////////////////////////////////////////////////////////////////////////
//
// Function Name: Brown1
// 
// Description: Brown's first index is given by
//              Phi_m = ( phi_m_factor*zeta_a^2 + dsh^2 + dst^2)^(1/2)
//              where zeta_a = absolute vorticity,
//              dsh is shearing deformation and dst is stretching deformation.
//              dsh,dst,zeta_a, are computed in function 
//              calc_common_factors())
//
// Returns: The value of the index is returned.
//
// Notes:       dsh,dst,zeta_a are computed in function 
//              calc_common_factors().
//               

fl32 GTGIndices::Brown1(){
  return(phi_m);
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: Brown2
// 
// Description: Brown's second index is given by
//              Phi_m*(du^2 + dv^2)^2 /24 where
//              (du^2 + dv^2)^2 is taken across 500 m layers.
//              u,v represent the horizontal components of the wind,
//              For this quantity we use: (vws)^2*500^2. 
//              Phi_m = ( phi_m_factor*zeta_a + dsh^2 + dst^2)^(1/2),
//              zeta_a = absolute vorticity ,
//              dsh is shearing deformation and dst is stretching deformation.
//
// Returns: The value of the index is returned.
//
// Notes: vws, phi_m are computed in function calc_common_factors.
//
fl32 GTGIndices::Brown2(){
  
  fl32 browns2 = (vws*500) * (vws*500)* phi_m/24.0 * 10000; 
  // Note: the 10000 factor is a conversion factor from m^2 to cm^2

  return(browns2);  
}


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name:
// 
// Description: The CCAT index is given by 
//              CCAT = |g/T_bar * zeta/f * A(Tz) )|
//              where g is the acceleration of  gravity(m/s^2),
//              T_bar is a weighted average of T(k-1),T(k) and T(k+1),
//              zeta/f is vorticity. 
//              A(Tz) is advection of dT/dz on a constant z-surface.
//
// Returns: The value of the index is returned.
//
// Notes: The following variables are defined in function get_MM5Data_variables:
//        coriolis,
//        ds,
//        mapf_x,
//        t, t_up1, t_down1, t_west,t_west_up1,t_west_down1,t_east, t_east_up1,t_east_down1,
//               t_north,t_north_up1,t_north_down1,t_south,t_south_up1,t_south_down1,
//        u,
//        v,
//        z_east,z_west,z_north,z_south,z,z_down1
//        
//        The following variables are computed in function calc_common_factors:
//        dsigma_dz,
//        dz_up1,
//        weight_up1, weight_down1,
//        zeta_a,
//
fl32 GTGIndices::CCAT(){
  
  // calculate derivatives of z with repsect to x and y:
  
  fl32 dz_dx  = (z_east - z_west)/(2*ds) * mapf_x;

  fl32 dz_dy = (z_north - z_south)/(2*ds) * mapf_x;


  // Calculate derivatives of temp with respect to z:

  fl32 dt_dz_west = derivWrtSigma(t_west_up1,t_west,t_west_down1)* dsigma_dz;

  fl32 dt_dz_east = derivWrtSigma(t_east_up1, t_east,t_east_down1)* dsigma_dz;

  fl32 dt_dz_north = derivWrtSigma(t_north_up1,t_north,t_north_down1)* dsigma_dz;

  fl32 dt_dz_south = derivWrtSigma(t_south_up1,t_south,t_south_down1)* dsigma_dz;


  // Calculate second derivatives of t:
  
  fl32 dz_down1 = z - z_down1;
  
  fl32 d2t_dz2 = ( (t_up1 - t)/dz_up1 + (t - t_down1)/dz_down1 )
    * 2/(dz_up1 + dz_down1);
     
  fl32 d2t_dxdz =  (dt_dz_east - dt_dz_west)/(2*ds) * mapf_x;

  fl32 d2t_dydz = (dt_dz_north - dt_dz_south)/(2*ds) * mapf_x;
 
  
  // Calculate mean temp:

  fl32 temp_m = ((t_up1 + t)*weight_up1 + (t + t_down1)*weight_down1)/2;
  
  
  // Calculate the index:

  //  using ave_u and ave_v 12/13/2001 Celia 

  fl32 ccat =  fabs( G/temp_m * zeta_a/coriolis * ( ave_u * (d2t_dxdz - d2t_dz2 * dz_dx) +  
						    ave_v * (d2t_dydz - d2t_dz2 * dz_dy ) ) );

  //fl32 ccat =  fabs( G/temp_m * zeta_a/coriolis * ( u * (d2t_dxdz - d2t_dz2 * dz_dx) +  
  //						    v * (d2t_dydz - d2t_dz2 * dz_dy ) ) );
  return(ccat);
}

///////////////////////////////////////////////////////////////////////////////////////////
// Function Name: Colson_Panofsky()
// 
// Description: The Colson Panofsky index is given by
//              cp = (Delta_V)^2) )(1-Ri/Ri_crit).
//              (Delta_V)^2 =  (delta u)^2 + (delta v)^2 over 500m 
//              layers, and u and v are the horizontal components
//              of the wind respectively. For this quantity we use vertical wind 
//              shear as defined in function calculate_common_factors. 
//              Ri = Richardson's Number,
//
// Returns: The value of the index is returned.
//
// Globals:   
//
// Notes: The 1.94^2 factor is a conversion to knots from m/s.
//        The following variables are computed in function calc_common_factors:
//        dz_up1,
//        ri,
//        vws.
// 
//        Ri_crit should possibly be made a parameter instead of being hard wired.         
fl32 GTGIndices::ColsonPanofsky(){

  //fl32 cp = (dz_up1*dz_up1)*((vws*1.94)*(vws*1.94))*(1-ri/ri_crit);
  //printf("dz_up1,vws,ri_crit,cp= %f %f %f %f\n", dz_up1,vws,ri_crit,cp);
  fl32 cp = vws * vws *( 1 - ri/ri_crit) * (dz_up1 * dz_up1) * 1.94 * 1.94;

  return cp;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Function Name: DefSqr()
// 
// Description: def^2 =  dst * dst + dsh * dsh
//                       dst is stretching deformation and dsh is shearing defpormation. 
//
// Returns: The value of the index is returned.
//
// Globals:   
//
// Notes: The following variables are computed in function calc_common_factors:
//        dst,
//        dsh
//                 
fl32 GTGIndices::DefSqr(){

  return (def * def);
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: Ellrod1()
// 
// Description: The Ellrod1 index is given by
//              ellrod1 = vws * def where
//              vws is vertical wind shear and DEF is total deformation,
//              so def = (dst^2 + dsh^2)^(1/2) 
//              where dst is stretching deformation and dsh is shearing 
//              deformation.
//
// Returns: The value of the index is returned.
//
// Notes: vws and def are computed in function calculate_common_factors.
//
fl32 GTGIndices::Ellrod1(){
  
  fl32 ellrod1 = vws * def;
  
  return(ellrod1);  
}
  
/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: Ellrod2()
// 
// Description: The Ellrod2 index is given by
//              Ellrod2 = vws(def + cvg) where
//              def = (dst^2 + dsh^2)^(1/2)
//              dst is stretching deformation and dsh is shearing deformation,
//              cvg is convergence (which is the negative of divergence),
//              vws is vertical wind shear.
//
// Returns: The value of the index is returned.
//
// Notes: def,vws,and cvg are computed in function calculate_common_factors.
//
// 
fl32 GTGIndices::Ellrod2(){

  fl32 ellrod2 = vws * (def - div);

  return(ellrod2);
}


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: Dutton()
// 
// Description: The Empirical Dutton index is given by
//              E = 1.25*Sh*100000 + .25*Sv^2 + 10.5 where
//              Sh = horizontal wind shear.
//              Sv = vertical wind shear(vws) 
//              (sh and vws are computed in function calculate_common_quantities.)
// 
// Notes: horizontal_shear, vws are computed in function calculate_common_factors.
//
// 
fl32 GTGIndices::Dutton(){


  fl32 dutton = 1.25 * horizontal_shear * 100000 + .25 * vws * vws * 1000*1000 + 10.5;
  
  return (dutton);
}


/////////////////////////////////////////////////////////////////////////////////////
// Function Name: Endlich_index
//
// Description: The Endlich index is given by:
//              endlich = | ave wind speend in layer*Dalpha/dz |
//              where alpha is wind direction in degrees.
// 
//
// Returns: The value of the index is returned.
//           
// Notes: The following variables are defined in function get_MM5Data_variables:
//        halfSigma_up,
//        u,u_up1,
//        v,v_up1
//           
//        The following variables are defined in calc_common_factors:
//        dsigma_dz,
//        dsigma_up1,
//        wsp

fl32 GTGIndices:: Endlich(){
  
  //  printf("u,u_up1,v,v_up1,halfSigma_up= %f %f %f %f %f\n", u,u_up1,v,v_up1,halfSigma_up);

  //  fl32 wsp_up1 = pow((double) u_up1 * u_up1 + v_up1 * v_up1 , .5);

  // Mods: 12/12/2001 (Celia Chen)
  // Using ave_u_up1 and ave_v_up1 for wsp_up1 and alpha and alpha_up1 calculations - 12/12/2001 (Celia

  fl32 ave_u_up1 = (u_up1 + u_northup1 + u_northeastup1 + u_eastup1)/4;
  fl32 ave_v_up1 = (v_up1 + v_northup1 + v_northeastup1 + v_eastup1)/4;

  fl32 wsp_up1 = sqrt(ave_u_up1 * ave_u_up1 + ave_v_up1 * ave_v_up1) ;


  fl32 ave_wsp = (wsp +  wsp_up1)/2;
  //  printf ("ave_u_up1,ave_v_up1,wsp,wsp_up1,ave_wsp = %f %f %f %f %f \n",ave_u_up1,ave_v_up1,wsp,wsp_up1,ave_wsp);
       

  // calculate wind direction in degrees in level k and k+1:
  //fl32 alpha_up1 = atan2(v_up1,u_up1) * 180/PI;
  //fl32 alpha =  atan2(v,u) * 180/PI;  
  fl32 alpha_up1 = atan2(ave_v_up1,ave_u_up1) * 180/PI;
  fl32 alpha =  atan2(ave_v,ave_u) * 180/PI;  

  //fl32 draddeg=0.0174532924;
  //fl32 alpha = (1.5*PI - atan2(ave_v,ave_u))/draddeg;
  //fl32 alpha_up1 = (1.5*PI - atan2(ave_v_up1,ave_u_up1))/draddeg;


  // dalpha is difference in the wind vector directions in degrees between k 
  // and k+1 sigma levels. 
  // The following if-else structure makes sure that the difference of angles 
  // in the numerator of dalpha_dz is less than 180. Otherwise the numerator is 
  // 360 minus this difference.
 
  fl32 dalpha1 = (alpha_up1 - alpha);
  
  fl32 dsigma_dz_up1 =  dSigma_dz(halfSigma_up);
   
  fl32 ave_dsigma_dz = (dsigma_dz + dsigma_dz_up1)/2;

  fl32 dalpha_dz;
  if ( fabs(dalpha1) > 180.0){
    if(dalpha1>0)
      dalpha_dz = ( (360.0 - dalpha1)/dsigma_up1 * ave_dsigma_dz );  
    else //dalpha1 < 0
      dalpha_dz = -( (360 + dalpha1)/dsigma_up1 * ave_dsigma_dz ) ; 
  }
  else
    dalpha_dz = dalpha1/dsigma_up1 * ave_dsigma_dz;

  //  printf("alpha,alpha_up1,dalpha1= %f %f %f \n", alpha,alpha_up1,dalpha1);
  // printf("dsigma_dz,dsigma_dz_up1,dsigma_up1,ave_dsigma_dz,dalpha_dz= %f %f %f %f %f \n", dsigma_dz,dsigma_dz_up1,dsigma_up1,ave_dsigma_dz,dalpha_dz);

  fl32 endlich = ave_wsp * fabs(dalpha_dz);
   
  return(endlich);
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: HorizontalShear 
// 
// Description: horizontal shear  = (- u^2 * du_dy + - u * v * dv_dy 
//		 + u * v * du_dx + v^2 * dv_dx)/(wsp * wsp).
//              u or v at  cross points are taken to be the average of the
//              four surrounding values of u or v respectively at dot points. 
//              wsp is windspeed 
//              
//
// Returns: The value of the index is returned.
//
//
// Notes: horizontal shear is defined in function calc_common_factors.
//
//
fl32 GTGIndices::HorizontalShear(){

  return(horizontal_shear);
}


//////////////////////////////////////////////////////////////////////////////////
//
// Function Name: LAZ 
//
// Description: This is the Laikhtman, Alter- Zalik index.
//              Let phi = max( vws^2 - alpha_T * stability, 0 ) where
//              shear = (dU/dtheta)^2 + (dV_dtheta)^2,
//              stability = g/mean_potential_temp * dtheta_dz),
//              and alpha_T is approximately  equal to 1.
//              Let ctg_beta = (d/dz (phi^1/2))^(-1), then 
//              b = .75 * ctg_beta is the value of the index.
//  
//
// Returns: The value of the index is returned.
//
//
// Notes: The following variables are defined in function get_MM5Data_variables:
//        halfSigma_down,halfSigma_up,halfSigma_down2,halfSigma_up,halfSigma_up2
//        p_down2,p_up2
//        q_down2,q_up2 
//        t_down2,t_up2
// 
//        The following variables are defined in function calc_common_factors:
//        
//        dsigma_up1,dsigma_down1,
//        dsigma_dz,
//        thetav,thetav_up1,thetav_down1
//        vws, vws_up1,vws_down1,
//        weight_up1, weight_down1
//        
//        The following variables are defined in calc_vertical_derivs:
//        dsigma_dz
//
fl32 GTGIndices::LAZ(){

  // calculate thetav_m_up1:

  fl32 thetav_up2 =  calc_thetav(p_up2, q_up2, t_up2);
  
  fl32 dsigma_up2 = halfSigma_up2 - halfSigma_up;

  fl32 weight1 = fabs(dsigma_up1)/( fabs(dsigma_up2) + fabs(dsigma_up1));
  fl32 weight2 = fabs(dsigma_up2)/( fabs(dsigma_up2) + fabs(dsigma_up1));
  
  fl32 thetav_m_up1 = (thetav_up2 + thetav_up1)/2 * weight1 
    + (thetav_up1 + thetav)/2 * weight2;

  
  //calclulate thetav_m_down1:
  
  fl32 thetav_down2 =  calc_thetav(p_down2, q_down2, t_down2);
  
  fl32 dsigma_down2 = halfSigma_down - halfSigma_down2;

  fl32 weight3 = fabs(dsigma_down2)/(fabs(dsigma_down1) + fabs(dsigma_down2));
  fl32 weight4 = fabs(dsigma_down1)/(fabs(dsigma_down1) + fabs(dsigma_down2));

  fl32 thetav_m_down1 = (thetav + thetav_down1)/2 * weight3 
    + (thetav_down1 + thetav_down2)/2 * weight4;
   

  // calculate dthetav_dz_up1, dthetav_dz_down1:
 
  fl32 dthetav_dsigma_up1 = (thetav_up2 - thetav_up1)/dsigma_up2 * weight1 
    + (thetav_up1 - thetav)/dsigma_up1 * weight2;


  fl32 dsigma_dz_up1 =  dSigma_dz(halfSigma_up);

  fl32 dthetav_dz_up1 = dthetav_dsigma_up1 * dsigma_dz_up1;

  fl32 dthetav_dsigma_down1 = (thetav - thetav_down1)/dsigma_down1 * weight3
    + (thetav_down1 - thetav_down2)/dsigma_down2 * weight4;

  fl32 dsigma_dz_down1 =  dSigma_dz(halfSigma_down);

  fl32 dthetav_dz_down1 = dthetav_dsigma_down1 * dsigma_dz_down1;


  //calculate stability (up and down): 

  fl32 stab_up1 =  G/thetav_m_up1 * dthetav_dz_up1;

  fl32 stab_down1 =  G/thetav_m_down1 * dthetav_dz_down1;

  fl32 phi_up1 = vws_up1 * vws_up1 - stab_up1;
  if (phi_up1 < 0)
    phi_up1 =0;

  fl32 phi = vws * vws - stab ;
  if( phi < 0)
    phi = 0;

  fl32 phi_down1 = vws_down1 * vws_down1 - stab_down1;
  if (phi_down1 < 0)
    phi_down1 = 0;

  fl32 phi_m = (phi_up1 + phi)/2 * weight_up1 + (phi + phi_down1)/2 * weight_down1;

  if (phi_m < 0)
    return(0);
  else
    {
      fl32 dphi_dsigma = derivWrtSigma(phi_up1,phi,phi_down1);

      fl32 dphi_dz = dphi_dsigma * dsigma_dz;
      
      fl32 ctg_beta = -2* pow((double)phi_m,1.5) / dphi_dz;
	
      fl32 laz_index = ctg_beta * ctg_beta *.75;
      
      if (laz_index < laz_upper_bound)
	return (laz_index);
      else
	return(laz_upper_bound);
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: Pvort
// 
// Description: pvort = -zeta_a * dtheta_dp * g;
//              where zeta_a is absolute vorticity          
//
// Returns: The value of the index is returned.
//
// Globals: G
//
// Notes: 
//
fl32 GTGIndices:: Pvort(){

  fl32 dtheta_dp = (thetav_up1 - thetav)/(p_up1 -p)/100 * weight_up1  
    + ( thetav - thetav_down1 )/( p - p_down1)/100 * weight_down1;

  fl32 pv =  -zeta_a * dtheta_dp * G; 

  return(pv * 1.e6);

}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: PvortGrad
// 
// Description: PvortGrad =  
//
//
// Returns: The value of the index is returned.
//
// Globals:
//
// Notes:
//
fl32 GTGIndices:: PvortGrad(){

  return(0 * 1.0e9);

}


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: Ngm1
// 
// Description: Ngm1 = def*wind speed where def is total deformation.
// 
//
// Returns: The value of the index is returned.
//
//
// Notes: wsp and def are defined in function calc_common_factors.
//
//
fl32 GTGIndices::Ngm1(){
  
  fl32 ngm1 = wsp * def;
  
  return(ngm1);
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: Ngm2
// 
// Description: Ngm2 = def * dt/dz where def is
//              total deformation and dt/dz is the derivative of
//              temperature with respect to altitude.
//              (def is computed in function calculate_common quantities.)
//
//
// Returns: The value of the index is returned.
//
// Globals:
//
// Notes: def is defined in calc_common_factors,
//        dt_dz is defined in  calc_vertical_derivs.
//
//
fl32 GTGIndices::Ngm2(){

  fl32 ngm2 = def * dt_dz;

  return(ngm2);
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: Ri
// 
// Description: Ri = stab/(vws*vws).
//              stab( stability) = G/thetav_m * dthetav_dz where
//              G is acceleration of gravity, thetav_m is average
//              of virtual potential temperature and dthetav_dz
//              is the derivative of virtual potential temperature with
//              respect to altitude. vws is vertical wind shear.
// 
//
// Returns: The value of the index is returned.
//
//
// Notes: stab and vws are defined in function calc_common_factors.
//
//
fl32 GTGIndices::Ri(){

  
  if (ri < ri_upper_bound)
    return(ri);
  else
    return(ri_upper_bound);

}


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name:  Rit()
// 
// Description: 
//
// Returns: 
//
// Notes: 
//
//
fl32 GTGIndices::Rit(){

  // calculate -Phi3:
  fl32 dpstar_dx = (pstar_east - pstar_west)/(2*ds) * mapf_x;
  fl32 dpstar_dy = (pstar_north - pstar_south)/(2*ds) * mapf_x;
  
  fl32 negPhi3 = 1/pstar * ( ave_u * dpstar_dx + ave_v * dpstar_dy);


  // calculate -Phi2:
  fl32 dthetav_dsigma = derivWrtSigma(thetav_up1,thetav,thetav_down1);

  fl32 ave_u_up1 = (u_up1 + u_northup1 + u_northeastup1 + u_eastup1)/4;

  fl32 ave_v_up1 = (v_up1 + v_northup1 + v_northeastup1 + v_eastup1)/4; 
  
  fl32 ave_u_down1 = (u_down1 + u_northdown1 + u_northeastdown1 + u_eastdown1)/4;
   
  fl32 ave_v_down1 = (v_down1 + v_northdown1 + v_northeastdown1 + v_eastdown1)/4;
  
  fl32 du_dsigma = derivWrtSigma(ave_u_up1,ave_u, ave_u_down1);
  
  fl32 dv_dsigma = derivWrtSigma(ave_v_up1,ave_v,ave_v_down1);

  //calculate horizontal derivs:
  fl32 thetav_east =  calc_thetav(p_east, q_east, t_east);

  fl32 thetav_west =  calc_thetav(p_west, q_west, t_west);
 
  fl32 thetav_north =  calc_thetav(p_north, q_north, t_north);
 
  fl32 thetav_south =  calc_thetav(p_south, q_south, t_south);

  fl32 dthetav_dx = (thetav_east - thetav_west)/(2*ds) * mapf_x;

  fl32 dthetav_dy = (thetav_north - thetav_south)/(2*ds) * mapf_x;

  //calculate dsigmaDot_dsigma:
  fl32 ref_density =  reference_density(halfSigma);
  fl32 sigmaDot = -ref_density * G * w / pstar - halfSigma/pstar *
    ( ave_u * dpstar_dx + ave_v * dpstar_dy);
  
  fl32 ref_density_up1 =  reference_density(halfSigma_up);
  fl32 sigmaDot_up1 =  -ref_density_up1 * G * w_up1 /pstar 
    - halfSigma_up/pstar * 
    (ave_u_up1 * dpstar_dx + ave_v_up1 * dpstar_dy);
  
  fl32 ref_density_down1 =  reference_density(halfSigma_down);
  fl32 sigmaDot_down1 = - ref_density_down1 * G * w_down1/pstar 
    - halfSigma_down/pstar *
    (ave_u_down1 * dpstar_dx +  ave_v_down1 * dpstar_dy);

  fl32 dsigmaDot_dsigma = derivWrtSigma(sigmaDot_up1,sigmaDot,sigmaDot_down1);  

  fl32 negPhi2 = du_dsigma * dthetav_dx/dthetav_dsigma + 
    dv_dsigma * dthetav_dy/dthetav_dsigma +  dsigmaDot_dsigma;

  // calculate phi1:
  fl32 dpp_dx = (pp_east - pp_west)/(2*ds) * mapf_x;
  fl32 dpp_dy = (pp_north - pp_south)/(2*ds) * mapf_x;
  fl32 dpp_dsigma = derivWrtSigma(pp_up1,pp,pp_down1);

  fl32 piX = dpp_dx - (halfSigma/pstar * dpstar_dx * dpp_dsigma);
  fl32 piY = dpp_dy - (halfSigma/pstar * dpstar_dy * dpp_dsigma);

  fl32 Tv = (float) pow( (double) p/1000, (double) eta) * thetav;
  fl32 total_rho = p/(Rd*Tv)*100;
  fl32 phi1 = 2/(du_dsigma * du_dsigma + dv_dsigma * dv_dsigma) * 
    ( -du_dsigma /total_rho * piX  
      -dv_dsigma /total_rho * piY
      -du_dsigma * (du_dsigma * du_dx + dv_dsigma * du_dy
		    + dsigmaDot_dsigma * du_dsigma)
      -dv_dsigma * (du_dsigma * dv_dx + dv_dsigma * dv_dy
		    + dsigmaDot_dsigma * dv_dsigma));
  
  fl32 phi = -phi1 + negPhi2 + negPhi3;

  if( phi > 0)
    return( vws * vws * phi);
  else
    return( 0);
  
}


/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: SatRi
// 
// Description: SatRi
//
// Returns: The value of the index is returned.
//
// Globals:
//
// Notes: 
//
fl32 GTGIndices::SatRi(){

  return(0);
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: Stab
// 
// Description: Stability =  G/thetav_m * dthetav_dz;
//
// Returns: The value of the index is returned.
//
// Globals: G
//
// Notes: stab is computed in calc_common_factors.  
//
fl32 GTGIndices::Stab(){

  return(stab);
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: VortSqr
// 
// Description: VortSqr = zeta =  (mapf_x^2( d(v/m)/dx - d(u/m)/dy ))^2
//
// Returns: The value of the index is returned.
//
// Globals:
//
// Notes: zeta is computed in calc_common_factors()
//

fl32 GTGIndices::VortSqr(){

  return(zeta * zeta);
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function Name: Vws
// 
// Description: See calc_vws() for a description.
//
// Returns: The value of the index is returned
//
// Globals:
//
// Notes:  vws is defined in calc_common_factors.  
//

fl32 GTGIndices::Vws(){

  return(vws);
}

//////////////////////////////////////////////////////////////////////////////
//                                                               
// Function Name: Tv                         
//                                                 
// Description: Calculates virtual temp from mm5 output temp and q.
//                                                    
// Returns:                                                        
//                                                                      
// Notes:                                                   
//                                    
fl32 GTGIndices::Tv(fl32 t, fl32 q){
  fl32 Tv; // virtual temperature

  Tv = t * ( 1 + q/eps)/(1 + q);

  return(Tv);
}

/////////////////////////////////////////////////////////////////////////////
//
// Function Name: printFields
//
// Description: Print the field values
//
// Returns:
//
//
// Notes:
//

void GTGIndices::printFields(ostream &out)

{

  printField(out, "brown1", brown1);
  printField(out, "brown2", brown2);
  printField(out, "ccat", ccat);
  printField(out, "colson_panofsky", colson_panofsky);
  printField(out, "def_sqr ", def_sqr );
  printField(out, "ellrod1", ellrod1);
  printField(out, "ellrod2", ellrod2);
  printField(out, "dutton", dutton);
  printField(out, "endlich", endlich);
  printField(out, "hshear", hshear);
  printField(out, "laz", laz);
  printField(out, "pvort", pvort);
  printField(out, "pvort_gradient", pvort_gradient);
  printField(out, "ngm1", ngm1);
  printField(out, "ngm2", ngm2);
  printField(out, "richardson", richardson);
  printField(out, "rit", rit);
  printField(out, "sat_ri", sat_ri);
  printField(out, "stability", stability);
  printField(out, "vort_sqr", vort_sqr);
  printField(out, "vwshear", vwshear);
  printField(out, "divergence", divergence);
  printField(out, "tke_kh3", tke_kh3);
  printField(out, "tke_kh5", tke_kh5);
  printField(out, "t_grad", t_grad);
  printField(out, "siii", siii);
  printField(out, "nva", nva);
  printField(out, "ncsui", ncsui);

  out << endl << endl;

}

/////////////////////////////////////////////////////////////////////////////
//
// Function Name: printInternals
//
// Description: Print the internal data members
//
// Returns:
//
//
// Notes:
//

void GTGIndices::printInternals(ostream &out)

{

  out << "------>> Indices object <<---------" << endl;
  out << "absia : " << absia  << endl;
  out << "ave_u: " << ave_u << endl;
  out << "ave_v: " << ave_v << endl;
  out << "center_lat: " << center_lat << endl;
  out << "center_lon: " << center_lon << endl;
  out << "cone_factor: " << cone_factor << endl;
  out << "coriolis: " << coriolis << endl;
  out << "def: " << def << endl;
  //out << "divergence: " << divergence << endl;
  //out << "t_grad: " << t_grad << endl;
  out << "ds: " << ds << endl;
  out << "dsh: " << dsh << endl;
  out << "dsigma_down1: " << dsigma_down1 << endl;
  out << "dsigma_dz   : " << dsigma_dz    << endl;
  out << "dsigma_up1  : " << dsigma_up1   << endl;
  out << "dst: " << dst << endl;
  out << "dt_dz: " << dt_dz << endl;
  out << "dthetav_dz: " << dthetav_dz << endl;
  out << "du_dx: " << du_dx << endl;
  out << "du_dy: " << du_dy << endl;
  out << "dv_dx: " << dv_dx << endl;
  out << "dv_dy: " << dv_dy << endl;
  out << "dz_up1: " << dz_up1 << endl;
  out << "halfSigma: " << halfSigma << endl;
  out << "halfSigma_down2: " << halfSigma_down2 << endl;
  out << "halfSigma_down: " << halfSigma_down << endl;
  out << "halfSigma_up2: " << halfSigma_up2 << endl;
  out << "halfSigma_up: " << halfSigma_up << endl;
  out << "horizontal_shear: " << horizontal_shear << endl;
  out << "mapf_dot: " << mapf_dot << endl;
  out << "mapf_dot_east: " << mapf_dot_east << endl;
  out << "mapf_dot_north: " << mapf_dot_north << endl;
  out << "mapf_dot_northeast: " << mapf_dot_northeast << endl;
  out << "mapf_x: " << mapf_x << endl;
  out << "nI: " << nI << endl;
  out << "nJ: " << nJ << endl;
  out << "nK: " << nK << endl;
  out << "p: " << p << endl;
  out << "pTop: " << pTop << endl;
  out << "p_down1: " << p_down1 << endl;
  out << "p_down2: " << p_down2 << endl;
  out << "p_east: " << p_east << endl;
  out << "p_north: " << p_north << endl;
  out << "p_south: " << p_south << endl;
  out << "p_up1: " << p_up1 << endl;
  out << "p_up2: " << p_up2 << endl;
  out << "p_west: " << p_west << endl;
  out << "phi_m: " << phi_m << endl;
  out << "pp: " << pp << endl;
  out << "pp_down1: " << pp_down1 << endl;
  out << "pp_east: " << pp_east << endl;
  out << "pp_north: " << pp_north << endl;
  out << "pp_south: " << pp_south << endl;
  out << "pp_up1: " << pp_up1 << endl;
  out << "pp_west: " << pp_west << endl;
  out << "pstar: " << pstar << endl;
  out << "pstar_east: " << pstar_east << endl;
  out << "pstar_north: " << pstar_north << endl;
  out << "pstar_south: " << pstar_south << endl;
  out << "pstar_west: " << pstar_west << endl;
  out << "q: " << q << endl;
  out << "q_down1: " << q_down1 << endl;
  out << "q_down2: " << q_down2 << endl;
  out << "q_east: " << q_east << endl;
  out << "q_north: " << q_north << endl;
  out << "q_south: " << q_south << endl;
  out << "q_up1: " << q_up1 << endl;
  out << "q_up2: " << q_up2 << endl;
  out << "q_west: " << q_west << endl;
  out << "rh: " << rh << endl;
  out << "rh_east: " << rh_east << endl;
  out << "rh_north: " << rh_north << endl;
  out << "rh_south: " << rh_south << endl;
  out << "rh_west: " << rh_west << endl;
  out << "ri: " << ri << endl;
  out << "stab: " << stab << endl;
  out << "t: " << t << endl;
  out << "t_down1: " << t_down1 << endl;
  out << "t_down2: " << t_down2 << endl;
  out << "t_east: " << t_east << endl;
  out << "t_east_down1: " << t_east_down1 << endl;
  out << "t_east_up1: " << t_east_up1 << endl;
  out << "t_north: " << t_north << endl;
  out << "t_north_down1: " << t_north_down1 << endl;
  out << "t_north_up1: " << t_north_up1 << endl;
  out << "t_south: " << t_south << endl;
  out << "t_south_down1: " << t_south_down1 << endl;
  out << "t_south_up1: " << t_south_up1 << endl;
  out << "t_up1: " << t_up1 << endl;
  out << "t_up2: " << t_up2 << endl;
  out << "t_west: " << t_west << endl;
  out << "t_west_down1: " << t_west_down1 << endl;
  out << "t_west_up1: " << t_west_up1 << endl;
  out << "thetav: " << thetav << endl;
  out << "thetav_down1: " << thetav_down1 << endl;
  out << "thetav_m  : " << thetav_m   << endl;
  out << "thetav_up1 : " << thetav_up1  << endl;
  out << "tlp: " << tlp << endl;
  out << "true_lat1: " << true_lat1 << endl;
  out << "true_lat2: " << true_lat2 << endl;
  out << "tso: " << tso << endl;
  out << "u: " << u << endl;
  out << "u_down1: " << u_down1 << endl;
  out << "u_east: " << u_east << endl;
  out << "u_eastdown1: " << u_eastdown1 << endl;
  out << "u_eastup1: " << u_eastup1 << endl;
  out << "u_north: " << u_north << endl;
  out << "u_northdown1: " << u_northdown1 << endl;
  out << "u_northeast: " << u_northeast << endl;
  out << "u_northeastdown1: " << u_northeastdown1 << endl;
  out << "u_northeastup1: " << u_northeastup1 << endl;
  out << "u_northup1: " << u_northup1 << endl;
  out << "u_up1: " << u_up1 << endl;
  out << "v: " << v << endl;
  out << "v_down1: " << v_down1 << endl;
  out << "v_east: " << v_east << endl;
  out << "v_eastdown1: " << v_eastdown1 << endl;
  out << "v_eastup1: " << v_eastup1 << endl;
  out << "v_north: " << v_north << endl;
  out << "v_northdown1: " << v_northdown1 << endl;
  out << "v_northeast: " << v_northeast << endl;
  out << "v_northeastdown1: " << v_northeastdown1 << endl;
  out << "v_northeastup1: " << v_northeastup1 << endl;
  out << "v_northup1: " << v_northup1 << endl;
  out << "v_up1: " << v_up1 << endl;
  out << "vws: " << vws << endl;
  out << "vws_down1: " << vws_down1 << endl;
  out << "vws_up1: " << vws_up1 << endl;
  out << "w: " << w << endl;
  out << "w_down1: " << w_down1 << endl;
  out << "w_up1: " << w_up1 << endl;
  out << "weight_down1: " << weight_down1 << endl;
  out << "weight_up1 : " << weight_up1  << endl;
  out << "wsp: " << wsp << endl;
  out << "wsp_up1: " << wsp_up1 << endl;
  out << "z: " << z << endl;
  out << "z_down1: " << z_down1 << endl;
  out << "z_east: " << z_east << endl;
  out << "z_north: " << z_north << endl;
  out << "z_south: " << z_south << endl;
  out << "z_up1: " << z_up1 << endl;
  out << "z_west: " << z_west << endl;
  out << "zeta: " << zeta << endl;
  out << "zeta_a: " << zeta_a << endl;
  out << "phi_m_factor: " << phi_m_factor << endl;
  out << "ri_crit: " << ri_crit << endl;
  out << "laz_upper_bound: " << laz_upper_bound << endl;
  out << "ri_upper_bound: " << ri_upper_bound << endl;

}


/////////////////////////////////////////////////////////////////////////////
//
// Function Name: printField
//
// Description: Print an index field
//
// Returns:
//
//
// Notes:
//

void GTGIndices::printField(ostream &out,
			     const string &name,
			     fl32 ***field)

{

  out << "----------------------------------------------" << endl;
  out << "  Field: " << name << endl;
  
  for(int k = 0; k < nK ; k++) {
    for (int j = 0; j< nJ; j++) {
      out << "  [" << k << "][" << j << "]:";
      for (int i = 0; i < nI ; i++) {
	out << " " << field[k][j][i];
      }
      out << endl;
    }
  }
  out << "----------------------------------------------" << endl;

}


//////////////////////////////////////////////////////////////////////////
//
// Function Name: clearCombined()
//
// Description: Clear the combined interest field
//
// Returns:
//
//
// Notes:
//

void GTGIndices::clearCombined()

{
  memset(**sumWeights, 0, nPoints * sizeof(fl32));
  memset(**combined, 0, nPoints * sizeof(fl32));
}

//////////////////////////////////////////////////////////////////////////
//
// Function Name: addToCombined()
//
// Add a field to the combined array, using the weight
// provided.
//
// If pressure thresholds are non-zero, the value is only added
// if pressure is within the pressure interval (min_pressure inclusive,
// max_pressure exclusive).
//
// Test sense:
//   If LESS_THAN, add if value is less than or equal to threshold_1
//
// Test sense:
//   If GERATER_THAN, add if value is greater than or equal to threshold_1
//
// Test sense:
//   If INSIDE_INTERVAL, add if value is between threshold_1 and threshold_2
//   end-points included.
//
// Test sense:
//   If OUTSIDE_INTERVAL, add if value is outside threshold_1 and threshold_2.
//

void GTGIndices::addToCombined(string name,
				fl32 ***field,
				double weight,
				test_sense_t sense,
				double threshold_1,
				double threshold_2,
				double min_pressure /*= 0.0*/,
				double max_pressure /*= 0.0*/)

{

  if (_debug) {
    cerr << "GTGIndices::addToCombined:" << endl;
    cerr << "  name: " << name << endl;
    cerr << "  weight: " << weight << endl;
    cerr << "  sense: " << sense << endl;
    cerr << "  threshold_1: " << threshold_1 << endl;
    cerr << "  threshold_2: " << threshold_2 << endl;
    cerr << "  min_pressure: " << min_pressure << endl;
    cerr << "  max_pressure: " << max_pressure << endl;
  }

  if (weight == 0) {
    return;
  }

  bool check_pressure = true;
  if (min_pressure == 0.0 && max_pressure == 0.0) {
    check_pressure = false;
  }

  double triggered = 0;
  double total = 0;
  
  for (int k = Z_BNDRY; k < nK - Z_BNDRY ; k++) {
    for (int j = XY_BNDRY; j < nJ - XY_BNDRY; j++) {
      for (int i = XY_BNDRY; i < nI - XY_BNDRY; i++) {
	
	bool pressureOK = true;
	double pressure = 0;
	if (check_pressure) {
	  pressure = _mm5.pres[k][j][i];
	  if (pressure < min_pressure ||
	      pressure >= max_pressure) {
	    pressureOK = false;
	  }
	}

	if (pressureOK) {

	  sumWeights[k][j][i] += weight;
	  total++;
	  fl32 val = field[k][j][i];

	  switch (sense) {
	  case LESS_THAN:
	    if (val <= threshold_1) {
	      combined[k][j][i] += weight;
	      triggered++;
	    }
	    break;
	  case GREATER_THAN:
	    if (val >= threshold_1) {
	      combined[k][j][i] += weight;
	      triggered++;
	    }
	    break;
	  case INSIDE_INTERVAL:
	    if (val >= threshold_1 && val <= threshold_2) {
	      combined[k][j][i] += weight;
	      triggered++;
	    }
	    break;
	  case OUTSIDE_INTERVAL:
	    if (val <= threshold_1 && val >= threshold_2) {
	      combined[k][j][i] += weight;
	      triggered++;
	    }
	    break;
	  } // switch
	  
	} // pressureOK
	
      } // i
    }// j 
  } // k

  if (_debug) {
    cerr << "GTGIndices::addToCombined" << endl;
    if (total != 0.0) {
      cerr << "  Field: " << name << ", percent turbulent: "
	   << (triggered / total) * 100.0 << endl;
    } else {
      cerr << "  Field: " << name
	   << ", pressure invalid for all points" << endl;
    }
  }
  
}

//////////////////////////////////////////////////////////////////////////
//
// Function Name: normalizeCombined()
//
// Normalize combined field using the weights
//
// Returns:
//
//
// Notes:
//

void GTGIndices::normalizeCombined()

{

  double total = 0.0, first = 0.0, second = 0.0, third = 0.0, fourth = 0.0;

  for (int k = Z_BNDRY; k < nK - Z_BNDRY ; k++) {
    for (int j = XY_BNDRY; j < nJ - XY_BNDRY; j++) {
      for (int i = XY_BNDRY; i < nI - XY_BNDRY; i++) {
	total++;
	fl32 sumWt = sumWeights[k][j][i];
	if (sumWt > 0.0) {
	  double norm = combined[k][j][i] / sumWt;
	  combined[k][j][i] = norm;
	  if (norm < 0.25) {
	    first++;
	  } else {
	    if (norm < 0.5) {
	      second++;
	    } else if (norm < 0.75) {
	      third++;
	    } else {
	      fourth++;
	    }
	  }
	}
      } // i
    }// j 
  } // k

  if (_debug) {
    cerr << "GTG combined percentages:" << endl;
    cerr << "  0.00 - 0.25: " << (first / total) * 100.0 << endl;
    cerr << "  0.25 - 0.50: " << (second / total) * 100.0 << endl;
    cerr << "  0.50 - 0.75: " << (third / total) * 100.0 << endl;
    cerr << "  0.75 - 1.00: " << (fourth / total) * 100.0 << endl;
  }
  
}

/////////////////////////////////////////////////////////////////////////////
//
// Function Name: printField
//
// Description: Print an index field
//
// Returns:
//
//
// Notes:
//

void GTGIndices::printCombined(ostream &out)

{
  printField(out, "Combined", combined);
}

////////////////////////////////////////////////////////////////////////////
////
//
// Function Name: calc_tgrad
//
// Description:
//
// Returns: The value of the index is returned.
//
// Globals:
//
// Notes:
// This function is adopted from ruc indices calculation: 1/2/2002 Celia Chen
// temp_gradient from RUC calculation
//
// Returns: The value of the index is returned.
//
// Globals:
//
// Notes:
//

fl32 GTGIndices:: calc_tgrad(){     
  // DS is the distance between grid points. 
  //  ds = _mm5.get_grid_distance() * 1000.0; //units = meters      
   
  float denom, tv_north, tv_south, tv_east, tv_west;
  float dT_dx, dT_dy;

  denom = ds/mapf_x;

  //vpt[k] = t[k] * ( 1 + q[k]/eps)/(1 + q[k]);
  tv_east = t_east * ( 1 + q_east/eps)/(1 + q_east);
  tv_west = t_west * ( 1 + q_west/eps)/(1 + q_west);
  tv_north = t_north * ( 1 + q_north/eps)/(1 + q_north);
  tv_south = t_south * ( 1 + q_south/eps)/(1 + q_south);

  dT_dx = (tv_east-tv_west)/(2.*denom) ;
  dT_dy = (tv_north-tv_south)/(2.*denom) ;

  //  printf("tve,tvw,tvn,tvs= %f %f %f %f\n",tv_east,tv_west,tv_north,tv_south);
  //  printf("ds,mapfx,denom,dT_dx,dT_dy= %f %f %f %f %f\n", ds,mapf_x,denom,dT_dx,dT_dy);
 
  //     --- 17. Horizontal temperature gradient (438)
  //             dss = ds/msfx(i,j)
  //             TxX = (Tx(i+1,j,k)-Tx(i-1,j,k))/(2.*dss)
  //             TyY = (Tx(i,j+1,k)-Tx(i,j-1,k))/(2.*dss)
  //             gradT = SQRT(TxX**2 + TyY**2)
  //

  return(sqrt(dT_dx * dT_dx + dT_dy * dT_dy));
}
       
///////////////////////////////////////////////////////////////////////////
// Function Name: diff_quotient
//
// Description: diff_quotient takes two dependent variable values,
//              the difference of the dependent varaibles, and a weighting
//              factor and forms a weighted difference quotient.
//
// Returns: The function returns the difference quotient as a float.
//
// Globals:
//
// Notes:
//
fl32 GTGIndices::diff_quotient(fl32 y2, fl32 y1, fl32 dx, fl32 weight)
{
  fl32 diff_quot;
  if( (y2 - y1) == 0)
    return(0);
  else if(weight == 0)
    {   
      diff_quot = (y2-y1)/dx;
      return(diff_quot);
    }
  else
    {
      diff_quot = weight* (y2-y1)/dx;
      return(diff_quot);
    }
}  
//////
//////
// Function Name: calc_siii()
// Stone inertial instability index (443) 
//
// Description: The Stone's Inertial Instability Index is given by 
//     S = f*(f*(1.-(1./Ri))+vorticity, where f is Earth's vorticity
// f is the coriolis force: coriolis = _mm5.get_coriolis_(i,j);
// coriolis at the cross pt is what is needed for this calculation
//     Stonei=|S| is S<0
//     Stonei=0 if S>0 or S=0
//
//              Ri = Richardson's Number,
//
// Returns: The value of the index is returned.
//
// Globals:
//

fl32 GTGIndices::calc_siii(){

  //printf("dz_up1,vws,ri_crit,cp= %f %f %f %f\n", dz_up1,vws,ri_crit,cp);
  // f is the coriolis force:     coriolis = _mm5.get_coriolis(i,j);
  // zeta is vortX in Bob's code.

  fl32 siii = coriolis*(coriolis*(1.-(1./ri))+zeta);

  //printf("coriolis,ri,zeta,siii= %f %f %f %f \n", coriolis,ri,zeta,siii);
  if(siii < 0.0) 
    siii = fabs(siii);
  else
    siii = 0.0;

  return siii;
}

//////////////////////////////////////////////////////////////////////////////////
///
//////
// Function Name: calc_nva()
// Vorticity advection (444)
//
// Description: The Negative vorticity advection is given by:
//         VA = -(U*d/dx(avort) + V*d/dy(avort))
//         NVA = |VA| if VA<0
//         NVA = 0 if VA>0
//
//
// Returns: The value of the index is returned.
//
// Globals:
//

fl32 GTGIndices::calc_nva(){

  denom = ds/mapf_x;

  //printf("u_north,u_northeast,mapf_dot_north,mapf_dot_northeast= %f %f %f %f \n", u_north,u_northeast,mapf_dot_north,mapf_dot_northeast);

  fl32 dv_N_dx = ((v_nne/mapf_dot_nne + v_northeast/mapf_dot_ne)
		  - (v_nn/mapf_dot_nn + v_north/mapf_dot_north) )/(2*ds);
  fl32 du_N_dy = ((u_nn/mapf_dot_nn + u_nne/mapf_dot_nne)
		  - (u_north/mapf_dot_north + u_northeast/mapf_dot_northeast) )/(2*ds);
  fl32 dv_S_dx = ((v_east/mapf_dot_east + v_se/mapf_dot_se)
		  - (v/mapf_dot + v_s/mapf_dot_s) )/(2*ds);
  //printf("v_east,mapf_dot_east,v_se,mapf_dot_se,v,mapf_dot,v_s,mapf_dot_s= %f %f %f %f %f %f %f %f \n", v_east,mapf_dot_east,v_se,mapf_dot_se,v,mapf_dot,v_s,mapf_dot_s);
  fl32 du_S_dy = ((u/mapf_dot + u_east/mapf_dot_east)
		  - (u_s/mapf_dot_s + u_se/mapf_dot_se) )/(2*ds);
  //printf("u,mapf_dot,u_east,mapf_dot_east,u_s,mapf_dot_s,u_se,mapf_dot_se= %f %f %f %f %f %f %f %f \n", u,mapf_dot,u_east,mapf_dot_east,u_s,mapf_dot_s,u_se,mapf_dot_se);
  fl32 dv_E_dx = ((v_nee/mapf_dot_nee + v_ee/mapf_dot_ee)
		  - (v_northeast/mapf_dot_northeast + v_east/mapf_dot_east) )/(2*ds);
  fl32 du_E_dy = ((u_northeast/mapf_dot_northeast + u_nee/mapf_dot_nee)
		  - (u_east/mapf_dot_east + u_ee/mapf_dot_ee) )/(2*ds);
  fl32 dv_W_dx = ((v_north/mapf_dot_north + v/mapf_dot)
		  - (v_nw/mapf_dot_nw + v_w/mapf_dot_w) )/(2*ds);
  //printf("v_north,mapf_dot_north,v,mapf_dot,v_nw,mapf_dot_nw,v_w,mapf_dot_w= %f %f %f %f %f %f %f %f \n", v_north,mapf_dot_north,v,mapf_dot,v_nw,mapf_dot_nw,v_w,mapf_dot_w);
  fl32 du_W_dy = ((u_nw/mapf_dot_nw + u_north/mapf_dot_north)
		  - (u_w/mapf_dot_w + u/mapf_dot) )/(2*ds);
  //printf("u_nw,mapf_dot_nw,u_north,mapf_dot_north,u_w,mapf_dot_w,u,mapf_dot= %f %f %f %f %f %f %f %f \n", u_nw,mapf_dot_nw,u_north,mapf_dot_north,u_w,mapf_dot_w,u,mapf_dot);

  //printf("u_nn,u_nne,mapf_dot_nn,mapf_dot_nne= %f %f %f %f \n", u_nn,u_nne,mapf_dot_nn,mapf_dot_nne);
  //printf("u_n,u_ne,mapf_dot_n,mapf_dot_ne= %f %f %f %f \n", u_north,u_northeast,mapf_dot_north,mapf_dot_northeast);
  //printf("v_nn,v_nne,mapf_dot_nn,mapf_dot_nne= %f %f %f %f \n", v_nn,v_nne,mapf_dot_nn,mapf_dot_nne);
  //printf("u_n,u_ne,mapf_dot_n,mapf_dot_ne= %f %f %f %f \n", u_north,u_northeast,mapf_dot_north,mapf_dot_northeast);

  fl32 zeta_N =mapf_x_north * mapf_x_north * ( dv_N_dx - du_N_dy);
  fl32 zeta_S =mapf_x_south * mapf_x_south * ( dv_S_dx - du_S_dy);
  fl32 zeta_E =mapf_x_east * mapf_x_east * ( dv_E_dx - du_E_dy);
  fl32 zeta_W =mapf_x_west * mapf_x_west * ( dv_W_dx - du_W_dy);

  fl32 avort_ip1 = zeta_E + coriolis_east;
  fl32 avort_im1 = zeta_W + coriolis_west;
  fl32 avort_jp1 = zeta_N + coriolis_north;
  fl32 avort_jm1 = zeta_S + coriolis_south;
  fl32 avortx = (avort_ip1-avort_im1)/(2.*denom);
  fl32 avorty = (avort_jp1-avort_jm1)/(2.*denom);
  //printf("vort_ip1,vort_im1,vort_jp1,vort_jm1= %f %f %f %f\n",zeta_E,zeta_W,zeta_N,zeta_S);
  //printf("avort_ip1,avort_im1,avort_jp1,avort_jm1= %f %f %f %f\n", avort_ip1,avort_im1,avort_jp1,avort_jm1);
  fl32 va = -(u*avortx + v*avorty);
  //printf("u,v,avortx,avorty,va=  %f %f %f %f %f \n", u,v,avortx,avorty,va);
  fl32 nva=0.;
  if(va < 0.) nva=fabs(va);
  return nva;
}             

//////////////////////////////////////////////////////////////////////////////////
///
//////
// Function Name: calc_ncsui()
// NCSU index (441)
//
// Description: The NASA/NCSU turbulence index is given by
//             NCSUi = v*delv*|del(vort)|/|Ri|
//
//              Ri = Richardson's Number,
//
// Returns: The value of the index is returned.
//
// Globals:
//

fl32 GTGIndices::calc_ncsui()

{

  denom = ds/mapf_x;
  
  fl32 ncsui = 0.;
  //printf("mapf_dot_nn,mapf_dot_nne= %f %f \n", mapf_dot_nn,mapf_dot_nne);
  //printf("mapf_dot_ee,mapf_dot_nee= %f %f \n", mapf_dot_ee,mapf_dot_nee);
  //printf("mapf_dot_se,mapf_dot_nw= %f %f \n", mapf_dot_se,mapf_dot_nw);
  //printf("mapf_x_east,mapf_x_west= %f %f \n", mapf_x_east,mapf_x_west);
  //printf("mapf_x_north,mapf_x_south= %f %f \n", mapf_x_north,mapf_x_south);
  //printf("u_nn,u_nne,u_ee,u_nee= %f %f %f %f \n", u_nn,u_nne,u_ee,u_nee);
  //printf("u_w,u_nw,u_s,u_se= %f %f %f %f \n", u_w,u_nw,u_s,u_se);

  //--- compute v dot delv

  fl32 dUdx = (u_east - u_w)/(2.*denom);
  // fl32 dVdx = (v_east-v_w)/(2.*denom);
  // fl32 dUdy = (u_north-u_s)/(2.*denom);
  fl32 dVdy = (v_north - v_s)/(2.*denom);
  // fl32 dmdx = (mapf_x_east -mapf_x_west)/(2.*ds);
  // fl32 dmdy = (mapf_x_north -mapf_x_south)/(2.*ds);
  
  vdelv = u*dUdx + v*dVdy;
  //printf("vdelv= %f \n", vdelv);
  if(vdelv >= 0.){
    fl32 du_N_dy = ((u_nn/mapf_dot_nn + u_nne/mapf_dot_nne)
		    - (u_north/mapf_dot_north + u_northeast/mapf_dot_northeast) )/(2*ds);
    fl32 du_S_dy = ((u/mapf_dot + u_east/mapf_dot_east)
		    - (u_s/mapf_dot_s + u_se/mapf_dot_se) )/(2*ds);
    fl32 du_E_dy = ((u_northeast/mapf_dot_northeast + u_nee/mapf_dot_nee)
		    - (u_east/mapf_dot_east + u_ee/mapf_dot_ee) )/(2*ds);
    fl32 du_W_dy = ((u_nw/mapf_dot_nw + u_north/mapf_dot_north)
		    - (u_w/mapf_dot_w + u/mapf_dot) )/(2*ds);
    
    fl32 dv_N_dx = ((v_nne/mapf_dot_nne + v_northeast/mapf_dot_ne)
		    - (v_nn/mapf_dot_nn + v_north/mapf_dot_north) )/(2*ds);
    fl32 dv_S_dx = ((v_east/mapf_dot_east + v_se/mapf_dot_se)
		    - (v/mapf_dot + v_s/mapf_dot_s) )/(2*ds);
    fl32 dv_E_dx = ((v_nee/mapf_dot_nee + v_ee/mapf_dot_ee)
		    - (v_northeast/mapf_dot_northeast + v_east/mapf_dot_east) )/(2*ds);
    fl32 dv_W_dx = ((v_north/mapf_dot_north + v/mapf_dot)
		    - (v_nw/mapf_dot_nw + v_w/mapf_dot_w) )/(2*ds);

    fl32 zeta_N =mapf_x * mapf_x * ( dv_N_dx - du_N_dy);
    fl32 zeta_S =mapf_x * mapf_x * ( dv_S_dx - du_S_dy);
    fl32 zeta_E =mapf_x * mapf_x * ( dv_E_dx - du_E_dy);
    fl32 zeta_W =mapf_x * mapf_x * ( dv_W_dx - du_W_dy);
    //printf("dv_E_dx,dv_W_dx,dv_N_dx,dv_S_dx= %f %f %f %f\n", dv_E_dx,dv_W_dx,dv_N_dx,dv_S_dx);
    //printf("du_E_dy,du_W_dy,du_N_dy,du_S_dy= %f %f %f %f\n", du_E_dy,du_W_dy,du_N_dy,du_S_dy);
    //printf("zeta_E,zeta_W,zeta_N,zeta_S= %f %f %f %f\n", zeta_E,zeta_W,zeta_N,zeta_S);

    fl32 vort_ip1 = zeta_E ;
    fl32 vort_im1 = zeta_W ;
    fl32 vort_jp1 = zeta_N ;
    fl32 vort_jm1 = zeta_S ;

    fl32 vortx = (vort_ip1-vort_im1)/(2.*denom);
    fl32 vorty = (vort_jp1-vort_jm1)/(2.*denom);
    //printf("vort_ip1,vort_im1,vort_jp1,vort_jm1,vortx,vorty= %f %f %f %f %f %f\n", vort_ip1,vort_im1,vort_jp1,vort_jm1,vortx,vorty);

    delvort = sqrt(vortx*vortx + vorty*vorty);
    ncsui = vdelv*fabs(delvort)/fabs(ri);
    //printf("vdelv,delvort,ri,ncsui= %f %f %f %f\n", vdelv,delvort,ri,ncsui);
  }  

  return ncsui;
}             
