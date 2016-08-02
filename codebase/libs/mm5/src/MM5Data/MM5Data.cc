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
//////////////////////////////////////////////////////////
// MM5Data.cc
//
// Read in MM5 file. Compute derived fields.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
//////////////////////////////////////////////////////////

#include <mm5/MM5Data.hh>
#include <toolsa/file_io.h>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pjg.h>

#include <physics/thermo.h>
#include <physics/PhysicsLib.hh>
#include <mm5/VisCalc.hh>
#include <toolsa/toolsa_macros.h>
#include <sys/stat.h>
#include <math.h>
#include <cerrno>
#include <iostream>
using namespace std;

const double MM5Data::MissingDouble = -9999.9;

//////////////
// Constructor

MM5Data::MM5Data (const string &prog_name,
		  const string &path,
		  bool debug /* = false */,
		  const heartbeat_t heartbeat_func /* = NULL */,
		  bool dbzConstIntercepts /* = true */) :
  _progName(prog_name),
  _path(path),
  _debug(debug),
  _heartbeatFunc(heartbeat_func)

{

  OK = TRUE;
  _version = 0;
  _dbzConstIntercepts = dbzConstIntercepts;

  uu = (fl32 ***) NULL;
  vv = (fl32 ***) NULL;
  tk = (fl32 ***) NULL;
  qq = (fl32 ***) NULL;
  clw = (fl32 ***) NULL;
  rnw = (fl32 ***) NULL;
  ice = (fl32 ***) NULL;
  snow = (fl32 ***) NULL;
  graupel = (fl32 ***) NULL;
  nci = (fl32 ***) NULL;
  rad_tend = (fl32 ***) NULL;
  ww = (fl32 ***) NULL;
  pp = (fl32 ***) NULL;

  uu_dot = (fl32 ***) NULL;
  vv_dot = (fl32 ***) NULL;
  ww_full = (fl32 ***) NULL;

  uuTn = (fl32 ***) NULL;
  vvTn = (fl32 ***) NULL;
  uuOut = (fl32 ***) NULL;
  vvOut = (fl32 ***) NULL;
  tc = (fl32 ***) NULL;
  wspd = (fl32 ***) NULL;
  wdir = (fl32 ***) NULL;
  zz = (fl32 ***) NULL;
  divergence = (fl32 ***) NULL;
  pres = (fl32 ***) NULL;
  rh = (fl32 ***) NULL;
  dewpt = (fl32 ***) NULL;
  turb = (fl32 ***) NULL;
  icing = (fl32 ***) NULL;
  clw_g = (fl32 ***) NULL;
  rnw_g = (fl32 ***) NULL;
  q_g = (fl32 ***) NULL;
  theta = (fl32 ***) NULL;
  thetae = (fl32 ***) NULL;
  thetav = (fl32 ***) NULL;
  dbz_3d = (fl32 ***) NULL;
  tot_cld_con = (fl32 ***) NULL;


  pstar = (fl32 **) NULL;
  ground_t = (fl32 **) NULL;
  rain_con = (fl32 **) NULL;
  rain_non = (fl32 **) NULL;
  terrain = (fl32 **) NULL;
  coriolis = (fl32 **) NULL;
  res_temp = (fl32 **) NULL;
  lat = (fl32 **) NULL;
  lon = (fl32 **) NULL;
  land_use = (fl32 **) NULL;
  snowcovr = (fl32 **) NULL;
  tseasfc = (fl32 **) NULL;
  pbl_hgt = (fl32 **) NULL;
  regime = (fl32 **) NULL;
  shflux = (fl32 **) NULL;
  lhflux = (fl32 **) NULL;
  ust = (fl32 **) NULL;
  swdown = (fl32 **) NULL;
  lwdown = (fl32 **) NULL;
  soil_t_1 = (fl32 **) NULL;
  soil_t_2 = (fl32 **) NULL;
  soil_t_3 = (fl32 **) NULL;
  soil_t_4 = (fl32 **) NULL;
  soil_t_5 = (fl32 **) NULL;
  soil_t_6 = (fl32 **) NULL;
  soil_m_1 = (fl32 **) NULL;
  soil_m_2 = (fl32 **) NULL;
  soil_m_3 = (fl32 **) NULL;
  soil_m_4 = (fl32 **) NULL;
  sfcrnoff = (fl32 **) NULL;
  t2 = (fl32 **) NULL;
  q2 = (fl32 **) NULL;
  u10 = (fl32 **) NULL;
  v10 = (fl32 **) NULL;
  mapf_x = (fl32 **) NULL;
  weasd = (fl32 **) NULL;
  snowh = (fl32 **) NULL;
  hc_rain = (fl32 **) NULL;
  hn_rain = (fl32 **) NULL;

  swfrac = (fl32 **) NULL;
  sunalt = (fl32 **) NULL;
  sunazm = (fl32 **) NULL;
  moonalt = (fl32 **) NULL;
  moonazm = (fl32 **) NULL;
  sunill = (fl32 **) NULL;
  moonill = (fl32 **) NULL;
  totalill = (fl32 **) NULL;

  clwi = (fl32 **) NULL;
  rnwi = (fl32 **) NULL;
  icei = (fl32 **) NULL;
  snowi = (fl32 **) NULL;
  pwv = (fl32 **) NULL;

  sun_btw = (fl32 **) NULL;
  sun_etw = (fl32 **) NULL;
  sun_abtw = (fl32 **) NULL;
  sun_aetw = (fl32 **) NULL;
  sun_rise = (fl32 **) NULL;
  sun_set = (fl32 **) NULL;
  sun_aris = (fl32 **) NULL;
  sun_aset = (fl32 **) NULL;
  moon_ris = (fl32 **) NULL;
  moon_set = (fl32 **) NULL;
  moon_ari = (fl32 **) NULL;
  moon_ase = (fl32 **) NULL;

  mapf_dot = (fl32 **) NULL;
  coriolis_dot = (fl32 **) NULL;

  gridRotation = (fl32 **) NULL;
  t2c = (fl32 **) NULL;
  pres2 = (fl32 **) NULL;
  fzlevel = (fl32 **) NULL;
  rain_total = (fl32 **) NULL;
  hourly_rain_total = (fl32 **) NULL;
  dbz_2d = (fl32 **) NULL;
  rh2 = (fl32 **) NULL;
  dewpt2 = (fl32 **) NULL;
  wspd10 = (fl32 **) NULL;
  wdir10 = (fl32 **) NULL;
  mslp2 = (fl32 **) NULL;
  q2_g = (fl32 **) NULL;
  theta2 = (fl32 **) NULL;
  thetae2 = (fl32 **) NULL;
  thetav2 = (fl32 **) NULL;
  cloud_fract = (fl32 **) NULL;
  twp = (fl32 **) NULL;
  rwp = (fl32 **) NULL;
  tot_cld_conp = (fl32 **) NULL;
  clwp = (fl32 **) NULL;

  _field3d = (fl32 ***) NULL;
  _field2d = (fl32 **) NULL;

  _mif = (si32 **) NULL;
  _mrf = (fl32 **) NULL;
  _halfSigma = (fl32 *) NULL;

  // open the file

  _in = NULL;
  if ((_in = fopen(_path.c_str(), "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:MM5Data::MM5Data\n", _progName.c_str());
    fprintf(stderr, "Cannot open file for reading\n");
    perror(_path.c_str());
    OK = FALSE;
    return;
  }

  _fileSize = ta_stat_get_len(_path.c_str());
  if(_fileSize > 100) {
    _more = true;
  } else {
    _more = false;
  }

}

/////////////
// Destructor

MM5Data::~MM5Data()

{

  if (_in != NULL) {
    fclose(_in);
  }

  if (uu != NULL) {
    ufree3((void ***) uu);
  }
  if (vv != NULL) {
    ufree3((void ***) vv);
  }
  if (tk != NULL) {
    ufree3((void ***) tk);
  }
  if (qq != NULL) {
    ufree3((void ***) qq);
  }
  if (clw != NULL) {
    ufree3((void ***) clw);
  }
  if (rnw != NULL) {
    ufree3((void ***) rnw);
  }
  if (clw_g != NULL) {
    ufree3((void ***) clw_g);
  }
  if (rnw_g != NULL) {
    ufree3((void ***) rnw_g);
  }
  if (q_g != NULL) {
    ufree3((void ***) q_g);
  }
  if (theta != NULL) {
    ufree3((void ***) theta);
  }
  if (thetae != NULL) {
    ufree3((void ***) thetae);
  }
  if (thetav != NULL) {
    ufree3((void ***) thetav);
  }
  if (dbz_3d != NULL) {
    ufree3((void ***) dbz_3d);
  }
  if (tot_cld_con != NULL) {
    ufree3((void ***) tot_cld_con);
  }
  if (ice != NULL) {
    ufree3((void ***) ice);
  }
  if (snow != NULL) {
    ufree3((void ***) snow);
  }
  if (graupel != NULL) {
    ufree3((void ***) graupel);
  }
  if (nci != NULL) {
    ufree3((void ***) nci);
  }
  if (rad_tend != NULL) {
    ufree3((void ***) rad_tend);
  }
  if (ww != NULL) {
    ufree3((void ***) ww);
  }
  if (pp != NULL) {
    ufree3((void ***) pp);
  }
  if (uu_dot != NULL) {
    ufree3((void ***) uu_dot);
  }
  if (vv_dot != NULL) {
    ufree3((void ***) vv_dot);
  }
  if (ww_full != NULL) {
    ufree3((void ***) ww_full);
  }
  if (uuTn != NULL) {
    ufree3((void ***) uuTn);
  }
  if (vvTn != NULL) {
    ufree3((void ***) vvTn);
  }
  if (uuOut != NULL) {
    ufree3((void ***) uuOut);
  }
  if (vvOut != NULL) {
    ufree3((void ***) vvOut);
  }
  if (tc != NULL) {
    ufree3((void ***) tc);
  }
  if (wspd != NULL) {
    ufree3((void ***) wspd);
  }
  if (wdir != NULL) {
    ufree3((void ***) wdir);
  }
  if (zz != NULL) {
    ufree3((void ***) zz);
  }
  if (divergence != NULL) {
    ufree3((void ***) divergence);
  }
  if (pres != NULL) {
    ufree3((void ***) pres);
  }
  if (rh != NULL) {
    ufree3((void ***) rh);
  }
  if (dewpt != NULL) {
    ufree3((void ***) dewpt);
  }
  if (turb != NULL) {
    ufree3((void ***) turb);
  }
  if (icing != NULL) {
    ufree3((void ***) icing);
  }

  if (pstar != NULL) {
    ufree2((void **) pstar);
  }
  if (ground_t != NULL) {
    ufree2((void **) ground_t);
  }
  if (rain_con != NULL) {
    ufree2((void **) rain_con);
  }
  if (rain_non != NULL) {
    ufree2((void **) rain_non);
  }
  if (terrain != NULL) {
    ufree2((void **) terrain);
  }
  if (coriolis != NULL) {
    ufree2((void **) coriolis);
  }
  if (res_temp != NULL) {
    ufree2((void **) res_temp);
  }
  if (lat != NULL) {
    ufree2((void **) lat);
  }
  if (lon != NULL) {
    ufree2((void **) lon);
  }
  if (land_use != NULL) {
    ufree2((void **) land_use);
  }
  if (snowcovr != NULL) {
    ufree2((void **) snowcovr);
  }
  if (tseasfc != NULL) {
    ufree2((void **) tseasfc);
  }
  if (pbl_hgt != NULL) {
    ufree2((void **) pbl_hgt);
  }
  if (regime != NULL) {
    ufree2((void **) regime);
  }
  if (shflux != NULL) {
    ufree2((void **) shflux);
  }
  if (lhflux != NULL) {
    ufree2((void **) lhflux);
  }
  if (ust != NULL) {
    ufree2((void **) ust);
  }
  if (swdown != NULL) {
    ufree2((void **) swdown);
  }
  if (lwdown != NULL) {
    ufree2((void **) lwdown);
  }
  if (soil_t_1 != NULL) {
    ufree2((void **) soil_t_1);
  }
  if (soil_t_2 != NULL) {
    ufree2((void **) soil_t_2);
  }
  if (soil_t_3 != NULL) {
    ufree2((void **) soil_t_3);
  }
  if (soil_t_4 != NULL) {
    ufree2((void **) soil_t_4);
  }
  if (soil_t_5 != NULL) {
    ufree2((void **) soil_t_5);
  }
  if (soil_t_6 != NULL) {
    ufree2((void **) soil_t_6);
  }
  if (soil_m_1 != NULL) {
    ufree2((void **) soil_m_1);
  }
  if (soil_m_2 != NULL) {
    ufree2((void **) soil_m_2);
  }
  if (soil_m_3 != NULL) {
    ufree2((void **) soil_m_3);
  }
  if (soil_m_4 != NULL) {
    ufree2((void **) soil_m_4);
  }
  if (sfcrnoff != NULL) {
    ufree2((void **) sfcrnoff);
  }
  if (t2 != NULL) {
    ufree2((void **) t2);
  }
  if (q2 != NULL) {
    ufree2((void **) q2);
  }
  if (u10 != NULL) {
    ufree2((void **) u10);
  }
  if (v10 != NULL) {
    ufree2((void **) v10);
  }
  if (mapf_x != NULL) {
    ufree2((void **) mapf_x);
  }
  if (mapf_dot != NULL) {
    ufree2((void **) mapf_dot);
  }
  if (weasd != NULL) {
    ufree2((void **) weasd);
  }
  if (snowh != NULL) {
    ufree2((void **) snowh);
  }
  if (hc_rain != NULL) {
    ufree2((void **) hc_rain);
  }
  if (hn_rain != NULL) {
    ufree2((void **) hn_rain);
  }

  if (swfrac != NULL) { ufree2((void **) swfrac); }
  if (sunalt != NULL) { ufree2((void **) sunalt); }
  if (sunazm != NULL) { ufree2((void **) sunazm); }
  if (moonalt != NULL) { ufree2((void **) moonalt); }
  if (moonazm != NULL) { ufree2((void **) moonazm); }
  if (sunill != NULL) { ufree2((void **) sunill); }
  if (moonill != NULL) { ufree2((void **) moonill); }
  if (totalill != NULL) { ufree2((void **) totalill); }

  if (clwi != NULL) { ufree2((void **) clwi); }
  if (rnwi != NULL) { ufree2((void **) rnwi); }
  if (icei != NULL) { ufree2((void **) icei); }
  if (snowi != NULL) { ufree2((void **) snowi); }
  if (pwv != NULL) { ufree2((void **) pwv); }

  if (sun_btw != NULL) { ufree2((void **) sun_btw); }
  if (sun_etw != NULL) { ufree2((void **) sun_etw); }
  if (sun_abtw != NULL) { ufree2((void **) sun_abtw); }
  if (sun_aetw != NULL) { ufree2((void **) sun_aetw); }
  if (sun_rise != NULL) { ufree2((void **) sun_rise); }
  if (sun_set != NULL) { ufree2((void **) sun_set); }
  if (sun_aris != NULL) { ufree2((void **) sun_aris); }
  if (sun_aset != NULL) { ufree2((void **) sun_aset); }
  if (moon_ris != NULL) { ufree2((void **) moon_ris); }
  if (moon_set != NULL) { ufree2((void **) moon_set); }
  if (moon_ari != NULL) { ufree2((void **) moon_ari); }
  if (moon_ase != NULL) { ufree2((void **) moon_ase); }

  if (coriolis_dot != NULL) {
    ufree2((void **) coriolis_dot);
  }
  if (gridRotation != NULL) {
    ufree2((void **) gridRotation);
  }
  if (t2c != NULL) {
    ufree2((void **) t2c);
  }
  if (pres2 != NULL) {
    ufree2((void **) pres2);
  }
  if (fzlevel != NULL) {
    ufree2((void **) fzlevel);
  }
  if (rain_total != NULL) {
    ufree2((void **) rain_total);
  }
  if (hourly_rain_total != NULL) {
    ufree2((void **) hourly_rain_total);
  }
  if (dbz_2d != NULL) {
    ufree2((void **) dbz_2d);
  }
  if (cloud_fract != NULL) {
    ufree2((void **) cloud_fract);
  }
  if (rh2 != NULL) {
    ufree2((void **) rh2);
  }
  if (dewpt2 != NULL) {
    ufree2((void **) dewpt2);
  }
  if (wspd10 != NULL) {
    ufree2((void **) wspd10);
  }
  if (wdir10 != NULL) {
    ufree2((void **) wdir10);
  }
  if (mslp2 != NULL) {
    ufree2((void **) mslp2);
  }
  if (q2_g != NULL) {
    ufree2((void **) q2_g);
  }
  if (theta2 != NULL) {
    ufree2((void **) theta2);
  }
  if (thetae2 != NULL) {
    ufree2((void **) thetae2);
  }
  if (thetav2 != NULL) {
    ufree2((void **) thetav2);
  }

  if (twp != NULL) {
	ufree2((void **) twp);
  }
  if (rwp != NULL) {
	ufree2((void **) rwp);
  }
  if (tot_cld_conp != NULL) {
	ufree2((void **) tot_cld_conp);
  }
  if (clwp != NULL) {
	ufree2((void **) clwp);
  }
  
  if (_field3d != NULL) {
    ufree3((void ***) _field3d);
  }
  if (_field2d != NULL) {
    ufree2((void **) _field2d);
  }

  if (_mif != NULL) {
    ufree2((void **) _mif);
  }
  if (_mrf != NULL) {
    ufree2((void **) _mrf);
  }
  if (_halfSigma != NULL) {
    ufree(_halfSigma);
  }

}

/////////////////////////////////
// getVersion() - static function
//
// Gets the MM5 file version number
//
// Returns 0 on success, -1 on failure.

int MM5Data::getVersion (const string &input_file_path,
			 int &version)

{

  // open file
  FILE *in;
  if ((in = fopen(input_file_path.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - MM5Data::getVersion" << endl;
    cerr << "  Cannot open file for reading: " << input_file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read in enough of header to get at the set number
  si32 header[10];
  if (ufread(header, sizeof(si32), 10, in) != 10) {
    int errNum = errno;
    cerr << "ERROR - MM5Data::getVersion" << endl;
    cerr << "  Cannot read header in file: " << input_file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    fclose(in);
    return -1;
  }
  fclose(in);
  BE_to_array_32(header, sizeof(header));


  if (header[1] == 6) {
    version = 2;
    return 0;
  }
  
  if (header[1] == 0) {
    version = 3;
    return 0;
  }

  // could not determine version number
  cerr << "ERROR - MM5Data::getVersion" << endl;
  cerr << "  Cannot determine version number from file: " << input_file_path << endl;

  // help somebody figure out what's wrong
  cerr << "  The header values are as follows:  ";
  for (size_t hi = 0; hi < sizeof(header) / sizeof(si32); hi++) {
    cerr << header[hi] << " ";
  }
  cerr << endl;

  return -1;

}

//////////////////
// interp3dField()
//
// Load up the sigma field array interpolated for a given point.
//
// returns vector of interpolated data.

void MM5Data::interp3dField(int ilat, int ilon,
			    const char *name, fl32 ***field,
			    double wt_sw, double wt_nw,
			    double wt_ne, double wt_se,
			    vector<double> &interp_data,
			    const vector<bool> *sigma_needed) const
  
  
{
  
  interp_data.clear();

  if (field == NULL) {
    fprintf(stderr, "ERROR - %s:MM5Data::interp3dField\n", _progName.c_str());
    fprintf(stderr, "%s array not loaded yet, operation invalid.\n", name);
    return;
  }
  
  for (int isig = 0; isig < nSigma; isig++) {
    interp_data.push_back(MissingDouble);
  }

  if (wt_sw + wt_nw + wt_ne + wt_se == 0.0) {
    return;
  }
  
  for (int isig = 0; isig < nSigma; isig++) {
    if (!sigma_needed || (*sigma_needed)[isig]) {
      double wt_sum = 0.0;
      if (wt_sw != 0.0) {
	wt_sum += field[isig][ilat][ilon] * wt_sw;
      }
      if (wt_nw != 0.0) {
	wt_sum += field[isig][ilat+1][ilon] * wt_nw;
      }
      if (wt_ne != 0.0) {
	wt_sum += field[isig][ilat+1][ilon+1] * wt_ne;
      }
      if (wt_se != 0.0) {
	wt_sum += field[isig][ilat][ilon+1] * wt_se;
      }
      interp_data[isig] = wt_sum;
    }
  }
  
  return;
  
}

//////////////////
// interp3dLevel()
//
// Interpolate a 3 d field at a given sigma level
//
// returns val on success, MissingDouble on failure.

double MM5Data::interp3dLevel(int isig, int ilat, int ilon,
			      const char *name, fl32 ***field,
			      double wt_sw, double wt_nw,
			      double wt_ne, double wt_se) const
  
{
  
  if (field == NULL) {
    fprintf(stderr, "ERROR - %s:MM5Data::interp3dLevel\n", _progName.c_str());
    fprintf(stderr, "%s array not loaded yet, operation invalid.\n", name);
    return (MissingDouble);
  }

  if (wt_sw + wt_nw + wt_ne + wt_se == 0.0) {
    return (MissingDouble);
  }

  double interp_val = 0.0;
  if (wt_sw != 0.0) {
    interp_val += field[isig][ilat][ilon] * wt_sw;
  }
  if (wt_nw != 0.0) {
    interp_val += field[isig][ilat+1][ilon] * wt_nw;
  }
  if (wt_ne != 0.0) {
    interp_val += field[isig][ilat+1][ilon+1] * wt_ne;
  }
  if (wt_se != 0.0) {
    interp_val += field[isig][ilat][ilon+1] * wt_se;
  }

  return (interp_val);

}

////////////////////////////////////////////////////////////////////////////
// closest3dField()
//
// Load up the sigma field array from model point closest to the given point.
//
// returns vector of interpolated data.

void MM5Data::closest3dField(int ilat, int ilon,
			     const char *name, fl32 ***field,
			     double wt_sw, double wt_nw,
			     double wt_ne, double wt_se,
			     vector<double> &closest_data,
			     const vector<bool> *sigma_needed) const
  
  
{
  
  closest_data.clear();

  if (field == NULL) {
    fprintf(stderr, "ERROR - %s:MM5Data::closest3dField\n", _progName.c_str());
    fprintf(stderr, "%s array not loaded yet, operation invalid.\n", name);
    return;
  }
  
  for (int isig = 0; isig < nSigma; isig++) {
    closest_data.push_back(MissingDouble);
  }
  
  if (wt_sw + wt_nw + wt_ne + wt_se == 0.0) {
    return;
  }

  // determine which point is closest

  int jlat, jlon;
  if (wt_sw >=  wt_nw &&
      wt_sw >=  wt_ne &&
      wt_sw >=  wt_se) {
    // SW point closest
    jlat = ilat;
    jlon = ilon;
  } else if (wt_nw >= wt_ne &&
	     wt_nw >=wt_se) {
    // NW point closest
    jlat = ilat+1;
    jlon = ilon;
  } else if (wt_ne >= wt_se) {
    // NE point closest
    jlat = ilat+1;
    jlon = ilon+1;
  } else {
    // SE point closest
    jlat = ilat;
    jlon = ilon+1;
  }

  // load up with the closest data

  for (int isig = 0; isig < nSigma; isig++) {
    if (!sigma_needed || (*sigma_needed)[isig]) {
      closest_data[isig] = field[isig][jlat][jlon];
    }
  }
  
}

//////////////////
// interp2dField()
//
// Load up interp_val_p with value interpolated for a given point.
//
// returns val on success, MissingDouble on failure.

double MM5Data::interp2dField(int ilat, int ilon,
			      const char *name, fl32 **field,
			      double wt_sw, double wt_nw,
			      double wt_ne, double wt_se) const
  
{
  
  if (field == NULL) {

    fprintf(stderr, "ERROR - %s:MM5Data::interp2dField\n", _progName.c_str());
    fprintf(stderr, "%s array not loaded yet, operation invalid.\n", name);
    return (MissingDouble);

  }

  if (wt_sw + wt_nw + wt_ne + wt_se == 0.0) {
    return (MissingDouble);
  }

  double interp_val = 0.0;
  if (wt_sw != 0.0) {
    interp_val += field[ilat][ilon] * wt_sw;
  }
  if (wt_nw != 0.0) {
    interp_val += field[ilat+1][ilon] * wt_nw;
  }
  if (wt_ne != 0.0) {
    interp_val += field[ilat+1][ilon+1] * wt_ne;
  }
  if (wt_se != 0.0) {
    interp_val += field[ilat][ilon+1] * wt_se;
  }

  return (interp_val);
  
}

////////////////////////////////////////////////////////////////////////////
// closest2dField()
//
// Load up the field array from model point closest to the given point.
// (For instance, Land Use category)
//
// returns value from the closest point.

double MM5Data::closest2dField(int ilat, int ilon,
			     const char *name, fl32 **field,
			     double wt_sw, double wt_nw,
			     double wt_ne, double wt_se) const
  
  
{
  
  if (field == NULL) {

    fprintf(stderr, "ERROR - %s:MM5Data::closest2dField\n", _progName.c_str());
    fprintf(stderr, "%s array not loaded yet, operation invalid.\n", name);
    return (MissingDouble);

  }

  if (wt_sw + wt_nw + wt_ne + wt_se == 0.0) {
    return (MissingDouble);
  }

  // determine which point is closest

  int jlat, jlon;
  if (wt_sw >=  wt_nw &&
      wt_sw >=  wt_ne &&
      wt_sw >=  wt_se) {
    // SW point closest
    jlat = ilat;
    jlon = ilon;
  } else if (wt_nw >= wt_ne &&
	     wt_nw >=wt_se) {
    // NW point closest
    jlat = ilat+1;
    jlon = ilon;
  } else if (wt_ne >= wt_se) {
    // NE point closest
    jlat = ilat+1;
    jlon = ilon+1;
  } else {
    // SE point closest
    jlat = ilat;
    jlon = ilon+1;
  }

  double ret_val = field[jlat][jlon];

  return (ret_val);
}

//////////////////
// _printHeaders()
//
// Print out header labels etc.

void MM5Data::_printHeaders(FILE *out) const

{

  fprintf(out, "*******************************************************\n");
  fprintf(out, "MM5 file %s\n", _path.c_str());
  fprintf(out, "*******************************************************\n");

  fprintf(out, "Model  time: %s\n", utimstr(modelTime));
  fprintf(out, "Output time: %s\n", utimstr(outputTime));
  fprintf(out, "Forecast lead time: %d secs\n", forecastLeadTime);

  fprintf(out, "==============================================\n\n");
  fprintf(out, "  pTop: %g\n", pTop);
  fprintf(out, "  pos: %g\n", pos);
  fprintf(out, "  tso: %g\n", tso);
  fprintf(out, "  tlp: %g\n", tlp);
  fprintf(out, "  nyDot: %d\n", nyDot);
  fprintf(out, "  nxDot: %d\n", nxDot);
  fprintf(out, "  nyDotCoarse: %d\n", nyDotCoarse);
  fprintf(out, "  nxDotCoarse: %d\n", nxDotCoarse);
  fprintf(out, "  nPtsDotPlane: %d\n", nPtsDotPlane);
  fprintf(out, "  nLat: %d\n", nLat);
  fprintf(out, "  nLon: %d\n", nLon);
  fprintf(out, "  nSigma: %d\n", nSigma);
  for (int i = 0; i < nSigma; i++) {
    fprintf(out, "    sigma %d: %g\n", i, _halfSigma[i]);
  }
  switch (proj_type) {
  case LAMBERT_CONF:
    fprintf(out, "  proj_type: LAMBERT_CONF\n");
    break;
  case STEREOGRAPHIC:
    fprintf(out, "  proj_type: STEREOGRAPHIC\n");
    break;
  case MERCATOR:
    fprintf(out, "  proj_type: MERCATOR\n");
    break;
  case UNKNOWN:
    fprintf(out, "  proj_type: UNKNOWN\n");
    break;
  }

  fprintf(out, "  center_lat (deg): %g\n", center_lat);
  fprintf(out, "  center_lon (deg): %g\n", center_lon);
  fprintf(out, "  cone_factor: %g\n", cone_factor);
  fprintf(out, "  true_lat1 (deg): %g\n", true_lat1);
  fprintf(out, "  true_lat2 (deg): %g\n", true_lat2);
  fprintf(out, "  grid_distance (km): %g\n", grid_distance);
  fprintf(out, "  grid_distance_coarse (km): %g\n", grid_distance_coarse);
  fprintf(out, "  domain_scale_coarse: %g\n", domain_scale_coarse);
  fprintf(out, "  x1_in_coarse_domain: %g\n", x1_in_coarse_domain);
  fprintf(out, "  y1_in_coarse_domain: %g\n", y1_in_coarse_domain);
  fprintf(out, "  x1_in_mother_domain: %g\n", x1_in_mother_domain);
  fprintf(out, "  y1_in_mother_domain: %g\n", y1_in_mother_domain);
  fprintf(out, "  minx_dot_coarse: %g\n", minx_dot_coarse);
  fprintf(out, "  miny_dot_coarse: %g\n", miny_dot_coarse);
  fprintf(out, "  minx_dot: %g\n", minx_dot);
  fprintf(out, "  miny_dot: %g\n", miny_dot);
  fprintf(out, "  minx_cross: %g\n", minx_cross);
  fprintf(out, "  miny_cross: %g\n", miny_cross);

}

////////////////////
// printModelFields()
//
// Print out model data fields

void MM5Data::printModelFields(FILE *out, bool full /* = true*/ ) const

{
  
  int n_lat = nLat;
  if (!full) {
    n_lat = 1;
  }

  fprintf(out, "*******************************************************\n");
  fprintf(out, "Model output fields\n");
  fprintf(out, "MM5 file %s\n", _path.c_str());
  fprintf(out, "*******************************************************\n");
  
  for (int ilat = 0; ilat < n_lat; ilat++) {
    fprintf(out,
	    "[%3s][%3s]: "
	    "%10s %10s %10s %10s\n",
	    "iy", "ix",
	    "pstar", "lat", "lon", "terrain");
      
    for (int ilon = 0; ilon < nLon; ilon++) {
      fprintf(out, "[%.3d][%.3d]: %10g %10g %10g %10g\n",
	      ilat, ilon,
	      pstar[ilat][ilon],
	      lat[ilat][ilon],
	      lon[ilat][ilon],
	      terrain[ilat][ilon]);
    }
    fprintf(out,
	    "--------------------------------------"
	    "--------------------------------------\n");
  }
  
  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < n_lat; ilat++) {
      
      fprintf(out,
	      "[%3s][%3s][%3s]: "
	      "%6s %6s %6s %7s "
	      "%6s %6s %7s %7s\n",
	      "iz", "iy", "ix",
	      "uu", "vv", "ww", "tk",
	      "qq", "pp", "clw", "rnw");
      
      for (int ilon = 0; ilon < nLon; ilon++) {
	
	fprintf(out,
		"[%.3d][%.3d][%.3d]: "
		"%6.1f %6.1f %6.3f %7.1f "
		"%6.2f %6.0f %7.4f %7.4f\n",
		isig, ilat, ilon,
		uu[isig][ilat][ilon],
		vv[isig][ilat][ilon],
		ww[isig][ilat][ilon],
		tk[isig][ilat][ilon],
		qq[isig][ilat][ilon],
		pp[isig][ilat][ilon],
		clw[isig][ilat][ilon],
		rnw[isig][ilat][ilon]);
	
      }
      
      fprintf(out,
	      "--------------------------------------"
	      "--------------------------------------\n");
    }
    
    fprintf(out,
	    "======================================"
	    "======================================\n");
  }
  
}

////////////////////
// printDerivedFields()
//
// Print out derived data fields

void MM5Data::printDerivedFields(FILE *out, bool full /* = true*/ ) const

{

  int n_lat = nLat;
  if (!full) {
    n_lat = 1;
  }

  fprintf(out, "*******************************************************\n");
  fprintf(out, "2D & 3D Derived output fields\n");
  fprintf(out, "MM5 file %s\n", _path.c_str());
  fprintf(out, "*******************************************************\n");
    fprintf(out, "======================= 2D Fields=========================================\n");
  
  for (int ilat = 0; ilat < n_lat; ilat++) {
    fprintf(out,
	    "[%3s][%3s]: %10s %10s %10s %5s %5s %5s %5s %5s %5s %5s %5s %5s %5s %5s %5s %7s %7s\n",
	    "iy", "ix", "lat", "lon", "grid rot", "fzlevel", "rtot", "rhtot", "rh2",
            "dewpt2", "wspd10", "wdir10", "mslp2",
            "dbz_2d", "cloud_fract", "twp", "rwp" ,"tot_cld_conp", "clwp");
    
    for (int ilon = 0; ilon < nLon; ilon++) {
      fprintf(out, "[%.3d][%.3d]: %10g %10g %10g %5.0f %5.0f %5.0f %5.0f %5.0f %5.0f %5.0f %5.0f %5.0f %5.0f %5.0f %5.0f %7.0f %7.0f\n",
	      ilat, ilon,
	      lat[ilat][ilon],
	      lon[ilat][ilon],
              gridRotation[ilat][ilon],
              fzlevel[ilat][ilon],
              rain_total[ilat][ilon],
              hourly_rain_total[ilat][ilon],
              rh2[ilat][ilon],
              dewpt2[ilat][ilon],
              wspd10[ilat][ilon],
              wdir10[ilat][ilon],
              mslp2[ilat][ilon],
              dbz_2d[ilat][ilon],
              cloud_fract[ilat][ilon],
              twp[ilat][ilon],
              rwp[ilat][ilon],
              tot_cld_conp[ilat][ilon],
              clwp[ilat][ilon]);
    }
    fprintf(out, "\n---------------------------------------------------------------------------\n");
  }
  fprintf(out, "\n===========================================================================\n");
  fprintf(out, "\n========================== 3D  FIELDS =====================================\n");
  fprintf(out, "\n===========================================================================\n");
  
  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < n_lat; ilat++) {
  fprintf(out, "\n####################### 3D Fields  Plane %5d #######################\n",isig+1);
      
      fprintf(out,
	      "[%3s][%3s][%3s]: "
	      "%7s %6s %6s %12s %6s %6s %6s",
	      "iz", "iy", "ix",
	      "pres", "wspd", "zz", "div", "tc", "rh", "tot_cld_con");
      if (icing) {
	fprintf(out, " %5s", "icing");
      }
      if (turb) {
	fprintf(out, " %5s", "turb");
      }
      fprintf(out, "\n");
      
      for (int ilon = 0; ilon < nLon; ilon++) {
	
	fprintf(out,
		"[%.3d][%.3d][%.3d]: %7.2f %6.1f %6.1f %12.3e %6.1f %6.1f %6.1f ",
		isig, ilat, ilon,
		pres[isig][ilat][ilon],
		wspd[isig][ilat][ilon],
		zz[isig][ilat][ilon],
		divergence[isig][ilat][ilon],
		tc[isig][ilat][ilon],
		rh[isig][ilat][ilon],
		tot_cld_con[isig][ilat][ilon]);
	
	if (icing) {
	  fprintf(out, " %5.0f", icing[isig][ilat][ilon]);
	}
	if (turb) {
	  fprintf(out, " %5.0f", turb[isig][ilat][ilon]);
	}
	fprintf(out, "\n");

      }
      
    }
    
    fprintf(out, "\n=========================================================================\n");
 }
  
}

///////////////////////////////
// _readFortRecLen()
//
// Read a fortran record length
//

long MM5Data::_readFortRecLen()
  
{
  
  si32 reclen;
  
  if (ufread(&reclen, sizeof(si32), 1, _in) != 1) {
    if (feof(_in)) {
      return -1;
    }
    fprintf(stderr,
	    "ERROR - %s:MM5Data::_readFortRecLen\n", _progName.c_str());
    fprintf(stderr, "Cannot read fortran rec len\n");
    perror(_path.c_str());
    return -1;
  }
  
  BE_from_array_32(&reclen, sizeof(si32));

  if (_debug) {
    fprintf(stderr, "Fortran rec len: %d\n", reclen);
  }

  return reclen;

}

/////////////////////////////
// compute the derived fields

void MM5Data::computeDerivedFields(bool wspd_in_knots) 

{

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("computeGridRotation()");
  }
  _computeGridRotation();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadTempCField()");
  }
  _loadTempCField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadPressureField()");
  }
  _loadPressureField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadDivergence()");
  }
  _loadDivergence();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadRhField()");
  }
  _loadRhField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadDewPtField()");
  }
  _loadDewPtField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadT2CField()");
  }
  _loadT2CField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadPressure2Field()");
  }
  _loadPressure2Field();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadFzlevelField()");
  }
  _loadFzlevelField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadRainTotalField()");
  }
  _loadRainTotalField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadHourlyRainTotalField()");
  }
  _loadHourlyRainTotalField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadWspdDirnField()");
  }
  _loadWspdDirnField(wspd_in_knots);

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadRH2Field()");
  }
  _loadRH2Field();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadDewPt2Field()");
  }
  _loadDewPt2Field();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadSpeed10Field(wspd_in_knots)");
  }
  _loadSpeed10Field();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadMSLP2Field()");
  }
  _loadMSLP2Field();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadT2CField()");
  }
  _loadT2CField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadQ2_GField()");
  }
  _loadQ2_GField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadQ_GField()");
  }
  _loadQ_GField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadTHETA2Field()");
  }
  _loadTHETA2Field();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadTHETAE2Field()");
  }
  _loadTHETAE2Field();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadTHETAV2Field()");
  }
  _loadTHETAV2Field();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadCLW_GField()");
  }
  _loadCLW_GField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadRNW_GField()");
  }
  _loadRNW_GField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadTHETAField()");
  }
  _loadTHETAField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadTHETAEField()");
  }
  _loadTHETAEField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadTHETAVField()");
  }
  _loadTHETAVField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadDbz3DField()");
  }
  _loadDbz3DField();

  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadDbz2DField()");
  }
  _loadDbz2DField();
  
  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("loadCloudFractField()");
  }
  _loadCloudFractField();

  // Make sure required fields are present 
  if(clw != NULL && ice != NULL) {
    if (_heartbeatFunc != NULL)  _heartbeatFunc("loadTotalCldConField()");
    _loadTotalCldConField();
    if (_heartbeatFunc != NULL)  _heartbeatFunc("_loadTotalCldConPField()");
    _loadTotalCldConPField();
  }
  
  // Make sure clw is present
  if(clw != NULL)  {
    if (_heartbeatFunc != NULL) _heartbeatFunc("loadCLWPField()");
    _loadCLWPField();
  }

  // Make sure required fields are present 
  if(ice != NULL && snow != NULL && pres != NULL ){
    if (_heartbeatFunc != NULL) {
      _heartbeatFunc("loadTotalWaterPathFields()");
    }
	 _loadTotalWaterPathFields();
  }

}

///////////////////////
// _loadTempCField()
//
// Load Temp in C from Temp in K
//

void MM5Data::_loadTempCField()
  
{

  if (tk == NULL)
    return;
  
  if (tc == NULL) {
    tc = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  
  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	tc[isig][ilat][ilon] = tk[isig][ilat][ilon] - 273.15;
      }
    }
  }

}
   
///////////////////////
// _loadPressureField()
//
// Load pressure (MB) field from pstar and pTop
//

#define RR 287.04
#define GG 9.80665

void MM5Data::_loadPressureField()
  
{

  if (pstar == NULL || pp == NULL || _halfSigma == NULL)
    return;

  if (pres == NULL) {
    pres = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  if (zz == NULL) {
    zz = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  
  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	double pnot = (pstar[ilat][ilon] / 100.0) * _halfSigma[isig] + pTop;
	pres[isig][ilat][ilon] = pnot + pp[isig][ilat][ilon] / 100.0;
	double aa = log(pnot / pos);
	zz[isig][ilat][ilon] =
	  -1.0 * ((RR * tlp * pow(aa, 2.0)) / (2.0 * GG) +
		  (RR * tso * aa) / GG);
      }
    }
  }
  
}

///////////////////////
// _loadRhField()
//
// Load relative humidity (%) field from mixing ratio,
// pressure and temperature
//

void MM5Data::_loadRhField()
  
{

  if (qq == NULL || pres == NULL || tc == NULL)
    return;

  if (rh == NULL) {
    rh = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }

  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	rh[isig][ilat][ilon] =
	  PHYrhmr(qq[isig][ilat][ilon] * 1000.0,
		  pres[isig][ilat][ilon],
		  tc[isig][ilat][ilon]);
      }
    }
  }

}
   
///////////////////////
// _loadDewPtField()
//
// Load dewpt (C) field from temperature and RH
//

void MM5Data::_loadDewPtField()
  
{

  if (tc == NULL || rh == NULL)
    return;
  
  if (dewpt == NULL) {
    dewpt = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  
  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	dewpt[isig][ilat][ilon] =
	  PHYrhdp(tc[isig][ilat][ilon],
		  rh[isig][ilat][ilon]);
      }
    }
  }

}
   
///////////////////////
// _loadWspdDirnField()
//
// Load wind speed and dirn from U and V components.
// Also translate the U and V into knots if required
//

void MM5Data::_loadWspdDirnField(bool wspd_in_knots)
  
{
  
  // convert wspd to knots if required

  if (wspd_in_knots) {

    if (uu != NULL && vv != NULL) {

      for (int isig = 0; isig < nSigma; isig++) {
        for (int ilat = 0; ilat < nLat; ilat++) {
          for (int ilon = 0; ilon < nLon; ilon++) {
	    (uu[isig][ilat][ilon]) *= MS_TO_KNOTS;
	    (vv[isig][ilat][ilon]) *= MS_TO_KNOTS;
	  }
        }
      }

    } // if (uu != NULL && vv != NULL)
  } // if (wspd_in_knots

  // compute U and V relative to TN

  _computeUVRelToTN();

  if (uuTn == NULL || vvTn == NULL)
    return;

  // allocate arrays

  if (wspd == NULL) {
    wspd = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  if (wdir == NULL) {
    wdir = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }

  // load up speed and dirn
  
  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	double u = uuTn[isig][ilat][ilon];
	double v = vvTn[isig][ilat][ilon];
	wspd[isig][ilat][ilon] = sqrt(u * u + v * v);
	if (u == 0.0 && v == 0.0) {
	  wdir[isig][ilat][ilon] = 0.0;
	} else {
	  wdir[isig][ilat][ilon] = atan2(-u, -v) / DEG_TO_RAD;
	}
      }
    }
  }

}
   
//////////////////////////////////////////////////////////////////////
// loadIcingField()
//
// Load icing severity field from cloud mixing ratio and temperature
//
// Index of severity ranges from 0.0 to 1.0

#define RPRIME 287.04

void MM5Data::loadIcingField(double trace_clw /* = 0.01*/,
			     double light_clw /* = 0.1*/,
			     double moderate_clw /* = 0.6*/,
			     double severe_clw /* = 1.2*/,
			     double clear_ice_temp /* = -10.0*/ )
  
{

  if (pres == NULL || tk == NULL || clw == NULL)
    return;
  
  if (icing != NULL) {
    ufree3((void ***) icing);
  }
  icing = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  
  double freezingPt = 273.15;
  double tClear = freezingPt + clear_ice_temp;
  double trace_range = light_clw - trace_clw;
  double light_range = moderate_clw - light_clw;
  double moderate_range = severe_clw - moderate_clw;
  double total = 0.0, first = 0.0, second = 0.0, third = 0.0, fourth = 0.0;

  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	double pressure = pres[isig][ilat][ilon];
	double tempk = tk[isig][ilat][ilon];
	double Clw = clw[isig][ilat][ilon];
	double dens = (pressure * 100.0) / (RPRIME * tempk);
	double g_per_m3 = Clw * dens * 1000.0;
	double severity;
	if (g_per_m3 < trace_clw) {
	  severity = 0.0;
	} else if (g_per_m3 < light_clw) {
	  severity = ((g_per_m3 - trace_clw) / trace_range) * 0.25;
	} else if (g_per_m3 < moderate_clw) {
	  severity = 0.25 +
	    ((g_per_m3 - light_clw) / light_range) * 0.25;
	} else if (g_per_m3 < severe_clw) {
	  severity = 0.5 +
	    ((g_per_m3 - moderate_clw) / moderate_range) * 0.25;
	} else {
	  severity = 0.75;
	}
	if (tempk > freezingPt) {
	  severity = 0.0;
	} else if (tempk > tClear) {
	  severity *= 1.5;
	}
	
	if (severity > 1.0) {
	  severity = 1.0;
	}

	icing[isig][ilat][ilon] = severity;

	total++;
	if (severity < 0.25) {
	  first++;
	} else {
	  if (severity < 0.5) {
	    second++;
	  } else if (severity < 0.75) {
	    third++;
	  } else {
	    fourth++;
	  }
	}

      } // ilon
    } // ilat
  } // isig

  if (_debug) {
    cerr << "Icing combined percentages:" << endl;
    cerr << "  0.00 - 0.25: " << (first / total) * 100.0 << endl;
    cerr << "  0.25 - 0.50: " << (second / total) * 100.0 << endl;
    cerr << "  0.50 - 0.75: " << (third / total) * 100.0 << endl;
    cerr << "  0.75 - 1.00: " << (fourth / total) * 100.0 << endl;
  }
  
}
   
///////////////////////
// loadTurbField()
//
// Load turb severity field from outside source, e.g. GTG.
// Input data array must have been allocated using umalloc3.

void MM5Data::loadTurbField(fl32 ***turb_data)
  
{
  if (turb != NULL) {
    ufree3((void ***) turb);
  }
  turb = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  memcpy(**turb, **turb_data, nSigma * nLat * nLon * sizeof(fl32));
}

///////////////////////
// _loadT2CField()
//
// Load Temp in C from Temp in K at 2 meters
//

void MM5Data::_loadT2CField()
  
{
  if (t2 == NULL)
    return;

  if (t2c == NULL) {
    t2c = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }
  
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      t2c[ilat][ilon] = t2[ilat][ilon] - 273.15;
    }
  }

}
   
///////////////////////
// _loadPressure2Field()
//
// Load pressure (mB) at 2 meters field from pstar and pTop
//

void MM5Data::_loadPressure2Field()
  
{

  if (pstar == NULL || pp == NULL || _halfSigma == NULL)
    return;

  if (pres2 == NULL) {
    pres2 = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }
  
  int surfaceIndex = 0;
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      double pnot = (pstar[ilat][ilon] / 100.0) * _halfSigma[surfaceIndex] + pTop;
      pres2[ilat][ilon] = pnot + pp[surfaceIndex][ilat][ilon] / 100.0;
    }
  }

}

///////////////////////
///////////////////////
// _loadFzlevelField()
//
// Load the freezing level field
//
// Freezing level is defined as the lowest occurrence of freezing
// temperature.
//

void MM5Data::_loadFzlevelField()
  
{

  if (tc == NULL || pres == NULL)
    return;
  
  if (fzlevel == NULL) {
    fzlevel = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }
  
  for (int ilat = 0; ilat < nLat; ilat++) {

    for (int ilon = 0; ilon < nLon; ilon++) {

      fzlevel[ilat][ilon] = 0.0;
      
      for (int isig = 0; isig < nSigma - 1; isig++) {
	
	double t1 = tc[isig][ilat][ilon];
	double t2 = tc[isig+1][ilat][ilon];
	
	if (t1 >= 0.0 && t2 <= 0.0) {
	  
	  double fraction = t1 / (t1 - t2);
	  
	  double p1 = pres[isig][ilat][ilon];
	  double p2 = pres[isig+1][ilat][ilon];
	  
	  double pressure = p1 + fraction * (p2 - p1);

	  double flight_level = _isa.pres2flevel(pressure);
	  fzlevel[ilat][ilon] = flight_level;

	  break;
	  
	} // if (t1 >= 0.0 && t2 <= 0.0) {

      } // isig

    } // ilon

  } // ilat
  
}

/////////////////////////
// _loadCloudFractField()
// 
//
void MM5Data::_loadCloudFractField()
{

  if (pres == NULL || rh == NULL)
    return;

  double clfrlo; //fraction of low clouds
  double clfrmi; //fraction of middle clouds
  double clfrhi; //fraction of high clouds
  double clfrtot; // total fraction of high clouds

  int kclo,kcmd,kchi; // sigma indicies - Where the levels break.

  if(rh == NULL) _loadRhField(); // Make sure RH is loaded.
	
  if (cloud_fract == NULL) {
    cloud_fract = (fl32 **) umalloc2( nLat, nLon, sizeof(fl32));
  }

  for (int ilat = 0; ilat < nLat; ilat++) {
    
    for (int ilon = 0; ilon < nLon; ilon++) {
      // Compute where each level ends.
      kclo = kcmd = kchi = 0;
      for (int isig = 1; isig < nSigma; isig++) {      
        if(pres[isig][ilat][ilon] < 97000.0) kclo=isig;
        if(pres[isig][ilat][ilon] < 80000.0) kcmd=isig;
        if(pres[isig][ilat][ilon] < 45000.0) kchi=isig;
      } // each isig

      clfrlo = clfrmi = clfrhi = 0.0; //  clear fractional values

      for (int isig = 1; isig < nSigma; isig++) {      

        if (isig < kclo && isig > kcmd)
          clfrlo = 
            (clfrlo > rh[isig][ilat][ilon]) ? clfrlo : rh[isig][ilat][ilon];

        if (isig < kcmd && isig > kchi)
          clfrmi = 
            (clfrmi > rh[isig][ilat][ilon]) ? clfrmi : rh[isig][ilat][ilon];

        if (isig < kchi)
          clfrhi = 
            (clfrhi > rh[isig][ilat][ilon]) ? clfrhi : rh[isig][ilat][ilon];
      } // each isig

      // Convert to fractional values
      clfrlo =  (4.0 * clfrlo / 100.0) - 3.0;
      clfrmi =  (4.0 * clfrmi / 100.0) - 3.0;
      clfrhi =  (2.5 * clfrhi / 100.0) - 1.5;

      // Clamp value to 0.0 - 1.0 inclusive.
      if(clfrlo <  0.0) clfrlo = 0.0;
      if(clfrmi <  0.0) clfrmi = 0.0;
      if(clfrhi <  0.0) clfrhi = 0.0;

      if(clfrlo >  1.0) clfrlo = 1.0;
      if(clfrmi >  1.0) clfrmi = 1.0;
      if(clfrhi >  1.0) clfrhi = 1.0;
  
      //
      // Set  cloud_fract[ilat][ilon]  = clfrlo + clfrmi + clfrhi
      //  & clamp to 0.0 to 1.0
      //
      clfrtot = clfrlo + clfrmi + clfrhi;
      if(clfrtot <  0.0) clfrtot = 0.0;
      if(clfrtot >  1.0) clfrtot = 1.0;
      cloud_fract[ilat][ilon] = clfrtot;

    } // each ilon
  } // each ilat

}

/////////////////////////
// _loadTotalWaterPathFields()
// Calculates total water path and rain water path (snow/rain) 
//
void MM5Data::_loadTotalWaterPathFields()
{

  if (
    pres == NULL || tk == NULL || ice == NULL ||
    clw == NULL || rnw == NULL || snow == NULL || zz == NULL
  )
    return;

  int lv;

  //local arrays
  fl32 ***rhoa   = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  fl32 ***twater = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  fl32 ***rwater = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  fl32 ***dz     = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));

  if (twp == NULL) {
    twp = (fl32 **) umalloc2( nLat, nLon, sizeof(fl32));
  }

  if (rwp == NULL) {
    rwp = (fl32 **) umalloc2( nLat, nLon, sizeof(fl32));
  }

  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      for (int isig = 0; isig < nSigma; isig++) {
			
        rhoa[isig][ilat][ilon] = pres[isig][ilat][ilon] / (tk[isig][ilat][ilon] * RR);
			
      }
    }
  }

  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      for (int isig = 0; isig < nSigma; isig++) {
			
        // convert/combine ice, snow, and rain from mixing ratio to total water density in air
        twater[isig][ilat][ilon] =
          (ice[isig][ilat][ilon] + clw[isig][ilat][ilon] +
          rnw[isig][ilat][ilon] + snow[isig][ilat][ilon]) *
          rhoa[isig][ilat][ilon] * 1000.0;

        rwater[isig][ilat][ilon] = (rnw[isig][ilat][ilon] + snow[isig][ilat][ilon]) *
          rhoa[isig][ilat][ilon] * 1000.0;

        // compute dz at each level except the bottom

        lv = (nSigma-1) - isig;
        if (isig < (nSigma-1)) {
          dz[isig][ilat][ilon] = zz[lv][ilat][ilon] - zz[lv-1][ilat][ilon];
        } else {
          dz[isig][ilat][ilon] = zz[0][ilat][ilon];	// (should be) zero height
        }

      }
    }
  }

  for (int isig = 0; isig < nSigma; isig++) {

    for (int ilon = 0; ilon < nLon; ilon++) {
      for (int ilat = 0; ilat < nLat; ilat++) {

	twp[ilat][ilon] += (twater[isig][ilat][ilon] * dz[isig][ilat][ilon]);
	rwp[ilat][ilon] += (rwater[isig][ilat][ilon] * dz[isig][ilat][ilon]);

      }
    }
  }

  ufree3((void ***) rhoa);
  ufree3((void ***) twater);
  ufree3((void ***) rwater);
  ufree3((void ***) dz);

}

/////////////////////////
// _loadTotalCldConPField()
//  Vertically integrate tot_cld_con
//
void MM5Data::_loadTotalCldConPField()
{

  if (zz == NULL || tot_cld_con == NULL)
    return;

  int lv;  // level
  fl32 ***dz = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));

  if (tot_cld_conp == NULL) {
    tot_cld_conp = (fl32 **) umalloc2( nLat, nLon, sizeof(fl32));
  }

  for (int isig = 0; isig < nSigma; isig++) {      
    lv = (nSigma-1) - isig;
    for (int ilat = 0; ilat < nLat; ilat++) {
        for (int ilon = 0; ilon < nLon; ilon++) {
  	  // compute dz at each level except the bottom
	  if (isig < (nSigma-1)) {
		dz[isig][ilat][ilon] = zz[lv][ilat][ilon] - zz[lv-1][ilat][ilon];
	  } else {
		dz[isig][ilat][ilon] = zz[0][ilat][ilon];	// (should be) zero height
	  }
         // Total cloud condensate path g/m^2
         tot_cld_conp[ilat][ilon] += tot_cld_con[isig][ilat][ilon] * dz[isig][ilat][ilon];
      }
    }
  }

  ufree3((void ***) dz);
}

/////////////////////////
// _loadCldWField()
//  Vertically integrate clw 
//
void MM5Data::_loadCLWPField()
{
  int lv;  // level
  fl32 ***dz  = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));


  if (clw == NULL)
    return;
  if (clwp == NULL) {
    clwp = (fl32 **) umalloc2( nLat, nLon, sizeof(fl32));
  }

  for (int isig = 0; isig < nSigma; isig++) {      
    lv = (nSigma-1) - isig;
    for (int ilat = 0; ilat < nLat; ilat++) {
        for (int ilon = 0; ilon < nLon; ilon++) {
  	  // compute dz at each level except the bottom
	  if (isig < (nSigma-1)) {
		dz[isig][ilat][ilon] = zz[lv][ilat][ilon] - zz[lv-1][ilat][ilon];
	  } else {
		dz[isig][ilat][ilon] = zz[0][ilat][ilon];	// (should be) zero height
	  }
         clwp[ilat][ilon] += clw[isig][ilat][ilon] * dz[isig][ilat][ilon];
      }
    }
  }
  ufree3((void ***) dz);
}


/////////////////////////
// _loadTotalCldConField()
//  sum of clw and ice. - Total Cloud Condensate
//
void MM5Data::_loadTotalCldConField()
{

  if (pres == NULL || tk == NULL || clw == NULL || ice == NULL)
    return;

  fl32 ***rhoa = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));

  if (tot_cld_con == NULL) {
    tot_cld_con = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }

  for (int isig = 0; isig < nSigma; isig++) {      
    for (int ilat = 0; ilat < nLat; ilat++) {
        for (int ilon = 0; ilon < nLon; ilon++) {
	  rhoa[isig][ilat][ilon] = pres[isig][ilat][ilon] / (tk[isig][ilat][ilon] * RR);
          // Total Cloud Condensate content -  g/m^3
          tot_cld_con[isig][ilat][ilon] =
            (clw[isig][ilat][ilon]  + ice[isig][ilat][ilon]) *
            rhoa[isig][ilat][ilon] * 1000.0;
      }
    }
  }
  ufree3((void ***) rhoa);
}


/////////////////////////
// _loadDbz2DField()
// This Version takes the max of the column at each lat lon.
//
void MM5Data::_loadDbz2DField()
{

  if (dbz_3d == NULL)
    return;

  if (dbz_2d == NULL) {
    dbz_2d = (fl32 **) umalloc2( nLat, nLon, sizeof(fl32));
  }

  for (int ilat = 0; ilat < nLat; ilat++) {
    
    for (int ilon = 0; ilon < nLon; ilon++) {
      
      //
      // Set  dbz_2d[ilat][ilon] to the max in the [ilat][ilon] column
      //
      dbz_2d[ilat][ilon] = dbz_3d[0][ilat][ilon];
      
      for (int isig = 1; isig < nSigma; isig++) {      
	  
	if ( dbz_3d[isig][ilat][ilon] > dbz_2d[ilat][ilon] )
	   dbz_2d[ilat][ilon] = dbz_3d[isig][ilat][ilon];
      }
    }
  }
}

/////////////////////////
// _loadDbz3DField()
//
// Estimate the Ground level DBZ. This code adapted from the dbzcalc.f
//     module of the "rip" pacakge.
//
//     This routine computes equivalent reflectivity factor (in dBZ) at
//     each model grid point.  In calculating Ze, the RIP algorithm makes
//     assumptions consistent with those made in an early version
//     (ca. 1996) of the bulk mixed-phase microphysical scheme in the MM5
//     model (i.e., the scheme known as "Resiner-2").  For each species:
//
//     1. Particles are assumed to be spheres of constant density.  The
//     densities of rain drops, snow particles, and graupel particles are
//     taken to be rho_r = rho_l = 1000 kg m^-3, rho_s = 100 kg m^-3, and
//     rho_g = 400 kg m^-3, respectively. (l refers to the density of
//     liquid water.)
//
//     2. The size distribution (in terms of the actual diameter of the
//     particles, rather than the melted diameter or the equivalent solid
//     ice sphere diameter) is assumed to follow an exponential
//     distribution of the form N(D) = N_0 * exp( lambda*D ).
//
//     3. If ivarint=0, the intercept parameters are assumed constant (as
//     in early Reisner-2), with values of 8x10^6, 2x10^7, and 4x10^6 m^-4,
//     for rain, snow, and graupel, respectively.  If ivarint=1, variable
//     intercept parameters are used, as calculated in Thompson, Rasmussen,
//     and Manning (2004, Monthly Weather Review, Vol. 132, No. 2, pp. 519-542.)
//
//     NOTE - here, instead of using the "ivarint" integer, the boolean
//     _dbzConstIntercepts is used to determine the ice physics scheme.
//     This is set in the constructor of the MM5Data object.
//
//     More information on the derivation of simulated reflectivity in RIP
//     can be found in Stoelinga (2005, unpublished write-up).  Contact
//     Mark Stoelinga (stoeling@atmos.washington.edu) for a copy.

// This macro returns virtual temperature in K, given temperature
// in K and mixing ratio in kg/kg
//
#define VIRTUAL(t,r) ( t * (0.622 + r) / (0.622 *(1+r)) )

void MM5Data::_loadDbz3DField()
  
{

  if (
    dbz_3d == NULL || tk == NULL || pres == NULL || qq == NULL ||
    rnw == NULL || snow == NULL || graupel == NULL
  )
    return;

  double rn0_r = 8.e6; // m^-4
  double rn0_s = 2.e7;
  double rn0_g = 4.e6;

  double r1=1.e-15;
  // double ron=8.e6;
  double ron2=1.e10;
  // double son=2.e7;
  double gon=5.e7;
  double ron_min = 8.e6;
  double ron_qr0 = 0.00010;
  
  double celkel = 273.15;
  double rhowat = 1000.0;
  double rgas = 287.04;
  
  double ron_delqr0 = 0.25*ron_qr0;
  double ron_const1r = (ron2-ron_min)*0.5;
  double ron_const2r = (ron2+ron_min)*0.5;
  
  // Other constants

  double gamma_seven = 720.0;
  double rho_r = rhowat; // 1000. kg m^-3
  double rho_s = 100.0;  // kg m^-3
  double rho_g = 400.0;  // kg m^-3
  double alpha = 0.224;
  double factor_r = gamma_seven * 1.e18 * pow((1./(M_PI*rho_r)),1.75);
  double factor_s = gamma_seven * 1.e18 * pow((1./(M_PI*rho_s)),1.75)
    * pow((rho_s/rhowat),2.0) * alpha;
  double  factor_g = gamma_seven * 1.e18 * pow((1./(M_PI*rho_g)),1.75)
    * pow((rho_g/rhowat),2.0) * alpha;
  
  if (dbz_3d == NULL) {
    dbz_3d = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }

  fl32 *db_ptr = &dbz_3d[0][0][0];
  fl32 *tk_ptr = &tk[0][0][0];  // Temperature kelvin
  fl32 *pr_ptr = &pres[0][0][0];  // Pressure 
  fl32 *qvp_ptr = &qq[0][0][0];  // Water Vapor Mixing Ratio 
  fl32 *qra_ptr = &rnw[0][0][0];  // Rain  Mixing Ratio 
  fl32 *qsn_ptr = NULL;
  if (snow != NULL) {
    qsn_ptr = &snow[0][0][0];  // Snow Mixing Ratio 
  }
  fl32 *qgr_ptr = NULL;
  if (graupel != NULL) {
    qgr_ptr = &graupel[0][0][0];  // Graupel Mixing Ratio 
  }

  int num_pts = nLat * nLon * nSigma;
  
  for(int i = 0; i < num_pts ;i++) {

	  double rhoair = *pr_ptr * 100.0 / (rgas * VIRTUAL(*tk_ptr,*qvp_ptr));

	  // Adjust factor for brightband, where snow or graupel particle
	  // scatters like liquid water (alpha=1.0) because it is assumed to
	  // have a liquid skin.

          double factorb_s, factorb_g;

	  if(*tk_ptr > celkel) {
		factorb_s=factor_s/alpha;
		factorb_g=factor_g/alpha;
	  } else {
		 factorb_s=factor_s;
		 factorb_g=factor_g;
	  }

	  double ronv=0.0;
	  double sonv = 0.0;
	  double gonv = 0.0; // Set to 0 to avoid compiler warnings

	  if ( _dbzConstIntercepts ) {
	    // Scheme without Ice physics, same as ivarint=0
	    ronv = rn0_r;
	    sonv = rn0_s;
	    gonv = rn0_g;
	  } else {
	    // Scheme with ice physics, same as ivarint=1

            double temp_c =  *tk_ptr - celkel;
	    if (temp_c > -0.001) temp_c = -0.001;

	    sonv = 2.0e6*exp(-0.12*temp_c);
	    if (sonv > 2.0e8) sonv = 2.0e8;

            gonv = gon;
	    if (qgr_ptr != NULL){
	      if ( *qgr_ptr > r1){
		gonv = 2.38 * pow(M_PI*rho_g / (rhoair* *qgr_ptr ),0.92);
		double minVal = gonv;
		if (minVal > gon) minVal = gon;
		gonv = minVal;
		if (gonv < 1.e4) gonv = 1.e4;
	      }
	    }

            ronv = ron2;
            if ( *qra_ptr > r1) {
	      ronv = ron_const1r*tanh((ron_qr0 - *qra_ptr) / ron_delqr0) + ron_const2r;
	    }
	  } // End of if intercepts are constant

	  // Total equivalent reflectivity factor (z_e, in mm^6 m^-3) is

	  // Note - significant difference from RIP code - if we are above freezing, use
	  // qra as perscribed. If we are below freezing, use the qra value as qsn.

	  double z_e = 0.0; // Set to 0.0 to avoid compiler warnings
	  if (*tk_ptr > celkel){
	    //
	    // Above freezing
	    //
	    z_e = factor_r * pow((rhoair * *qra_ptr),1.75) / pow(ronv,0.75);
	    if (qsn_ptr) {
	      z_e += factorb_s * pow((rhoair * *qsn_ptr),1.75) / pow(sonv,0.75);
	    }
	    if (qgr_ptr) {
	      z_e += factorb_g * pow((rhoair * *qgr_ptr),1.75) / pow(gonv,0.75);
	    }

	  } else {
	    //
	    // Below freezing
	    //
	    z_e = factorb_s * pow((rhoair * *qra_ptr),1.75) / pow(sonv,0.75); // Using qra with sno co-efficients below freezing
	    if (qsn_ptr) {
	      z_e += factorb_s * pow((rhoair * *qsn_ptr),1.75) / pow(sonv,0.75);
	    }
	    if (qgr_ptr) {
	      z_e += factorb_g * pow((rhoair * *qgr_ptr),1.75) / pow(gonv,0.75);
	    }
	  }

	  //  Adjust small values of Z_e so that dBZ is no lower than -40
	  if(z_e < 0.0001) z_e = 0.0001;

          *db_ptr = 10.0 * log10(z_e);

          db_ptr++;
          tk_ptr++;
          pr_ptr++;
          qvp_ptr++;
          qra_ptr++;
          if (qsn_ptr) {
            qsn_ptr++;
          }
          if (qgr_ptr) {
            qgr_ptr++;
          }

  } // All points
  
}

/////////////////////////
// _loadRainTotalField()
//
// Load the total rain field - sum the rain_con and rain_non fields.
// Store in mm instead of cm.

void MM5Data::_loadRainTotalField()
  
{

  if (rain_con == NULL || rain_non == NULL)
    return;
  
  if (rain_total == NULL) {
    rain_total = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }
  
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      rain_total[ilat][ilon] =
	(rain_con[ilat][ilon] + rain_non[ilat][ilon]) * 10.0;
    } // ilon
  } // ilat
  
}

/////////////////////////
// _loadHourlyRainTotalField()
//
// Load the hourly total rain field - sum the hc_rain and hn_rain fields.
// Store in mm/hr instead of cm/hr.

void MM5Data::_loadHourlyRainTotalField()
  
{

  if (hc_rain == NULL || hn_rain == NULL)
    return;
  
  if (hourly_rain_total == NULL) {
    hourly_rain_total = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }
  
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      hourly_rain_total[ilat][ilon] =
	(hc_rain[ilat][ilon] + hn_rain[ilat][ilon]) * 10.0;
    } // ilon
  } // ilat
  
}

///////////////////////
// _loadDivergence()
//
// Load the divergence field

void MM5Data::_loadDivergence()
  
{

  if (
    mapf_x == NULL || mapf_dot == NULL ||
    uu_dot == NULL || vv_dot == NULL
  )
    return;

  if (divergence == NULL) {
    divergence = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  
  double ds = grid_distance * 1000.0; //units = meters
 
  for (int ilat = 0; ilat < nLat; ilat++) {
    
    for (int ilon = 0; ilon < nLon; ilon++) {
      
      double mapf_cross = mapf_x[ilat][ilon];
      double mapf = mapf_dot[ilat][ilon];
      double mapf_east = mapf_dot[ilat][ilon+1];
      double mapf_north = mapf_dot[ilat+1][ilon];
      double mapf_northeast = mapf_dot[ilat+1][ilon+1];
      
      for (int isig = 0; isig < nSigma; isig++) {
	
	double u = uu_dot[isig][ilat][ilon];
	double u_east = uu_dot[isig][ilat][ilon+1];
	double u_north = uu_dot[isig][ilat+1][ilon];
	double u_northeast = uu_dot[isig][ilat+1][ilon+1];
	
	double v = vv_dot[isig][ilat][ilon];
	double v_east = vv_dot[isig][ilat][ilon+1];
	double v_north = vv_dot[isig][ilat+1][ilon];
	double v_northeast = vv_dot[isig][ilat+1][ilon+1];
  
	double duoverm_dx =
	  ((u_northeast/mapf_northeast + u_east/mapf_east)
	   - (u_north/mapf_north + u/mapf))/(2*ds);
	
	double dvoverm_dy =
	  (( v_north/mapf_north + v_northeast/ mapf_northeast)
	   -(v/mapf + v_east/mapf_east))/(2*ds);
	
	divergence[isig][ilat][ilon] =
	  mapf_cross * mapf_cross * (duoverm_dx + dvoverm_dy);

      } // isig

    } // ilon
    
  } // ilat

}


/////////////////////////
// _loadRH2DField()
//
// Load the Relative Humidity 2-meter field.

void MM5Data::_loadRH2Field()
  
{

  if (q2 == NULL || pres2 == NULL || t2c == NULL)
    return;
  
  if (rh2 == NULL) {
    rh2 = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }
  
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      rh2[ilat][ilon] = PHYrhmr(q2[ilat][ilon] * 1000.0,
          pres2[ilat][ilon], t2c[ilat][ilon]);
    } // ilon
  } // ilat
  
}


/////////////////////////
// _loadDewPt2Field()
//
// Load the dew point 2-meter field.

void MM5Data::_loadDewPt2Field()
  
{

  if (t2c == NULL || rh2 == NULL)
    return;
  
  if (dewpt2 == NULL) {
    dewpt2 = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }
  
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      dewpt2[ilat][ilon] = PHYrhdp(t2c[ilat][ilon], rh2[ilat][ilon]);
    } // ilon
  } // ilat
  
}

/////////////////////////
// _loadSpeed10Field()
//
// Load the wind speed and direction at 10 meters fields.

void MM5Data::_loadSpeed10Field(bool wspd_in_knots)
  
{

  if (u10 == NULL || v10 == NULL)
    return;
  
  if (wspd10 == NULL) {
    wspd10 = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }
  if (wdir10 == NULL) {
    wdir10 = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }
  
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      if (wspd_in_knots) {
        // convert units to knots
        u10[ilat][ilon] *= MS_TO_KNOTS;
        v10[ilat][ilon] *= MS_TO_KNOTS;
      }

      // use temp to simplify notation
      double u = u10[ilat][ilon];
      double v = v10[ilat][ilon];

      // do the math
      wspd10[ilat][ilon] = sqrt(u * u + v * v);
      if (u == 0.0 && v == 0.0) {
        wdir10[ilat][ilon] = 0.0;
      } else {
        wdir10[ilat][ilon] = atan2(-u, -v) / DEG_TO_RAD;
      }
    }
  }

}

/////////////////////////
// _loadMSLP2Field()
//
// Load the mean sea level pressure 2-meter field.
// Translated from Fortran slpbmcalc.f by Carl Drews - June 2005.
//
//   By default this routine calculates SLP using a modified form of the
//   Benjamin and Miller (1990) method.  Instead of using 700-hPa
//   temperature, it uses the temperature at a sigma level (the same
//   sigma level for all points).  The sigma level that is chosen is
//   that which would be closest to 850 hPa for an ocean point with
//   surface pressure of 1000 hPa.
//
//   Alternate formulation:
//   LHSL = lowest half sigma level
//   First, get z at LHSL using altimeter equation between LHSL and
//   surface. Then, get p at sea level using alt. eqn. between LHSL
//   and sea level. Assume p. pert. at surface equals p. pert. at LHSL.

void MM5Data::_loadMSLP2Field(bool benjaminMiller)
  
{

  if (
    pres == NULL || pstar == NULL || tk == NULL ||
    qq == NULL || terrain == NULL || _halfSigma == NULL
  )
    return;

  // allocate the array 
  if (mslp2 == NULL) {
    mslp2 = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }

  // set up physical constants
  double standard_lapse_rate = 0.0065;	// 6.5 deg C per km
  double expon = PhysicsLib::RGAS * standard_lapse_rate / PhysicsLib::GRAVITY_CONSTANT;
  double exponi = 1.0 / expon;

  // set up the full sigma levels
  fl32 *fullSigma = (fl32 *)umalloc((nSigma + 1) * sizeof(fl32));
  fullSigma[0] = 1.0;
  fullSigma[nSigma] = 0.0;
  for (int si = 1; si < nSigma; si++) {
    fullSigma[si] = (_halfSigma[si-1] + _halfSigma[si]) / 2.0;
  }

  // get desired sigma level to use temperature from
  int kUpper = 0;
  double sigmaC = (850.0 - pTop) / (1000.0 - pTop);
  for (int ki = 0; ki < nSigma; ki++) {
    // If a match occurs exactly on a full sigma level,
    // we take the index closest to the surface.
    if (fullSigma[ki] > sigmaC && sigmaC >= fullSigma[ki + 1]) {
      kUpper = ki;
    }
  }

  // free the full sigma levels
  ufree(fullSigma);

  // calculate the sea-level pressure for each grid point
  int surfaceIndex = 0;
  VisCalc myCalculator;
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      if (benjaminMiller) {
        // Benjamin and Miller
        double pSurface = pres[surfaceIndex][ilat][ilon]
          + (1.0 - _halfSigma[surfaceIndex]) * (pstar[ilat][ilon] / 100.0);
        double pUpper = pres[kUpper][ilat][ilon];
        double temp0 = tk[kUpper][ilat][ilon] * pow(pSurface / pUpper, expon);
        double temp0Virtual = myCalculator.virtualTemp(temp0, qq[surfaceIndex][ilat][ilon]);

        mslp2[ilat][ilon] = pSurface
          * pow(1.0 + (standard_lapse_rate / temp0Virtual) * terrain[ilat][ilon], exponi);

      } else {
        // alternate formulation
        double tempVirtualLHSL = myCalculator.virtualTemp(tk[surfaceIndex][ilat][ilon],
          qq[surfaceIndex][ilat][ilon]);
        double pressureLHSL = pres[surfaceIndex][ilat][ilon];
        double pSurface = pressureLHSL + (1.0 - _halfSigma[surfaceIndex]) * (pstar[ilat][ilon] / 100.0);
        double geoHeightLHSL = terrain[ilat][ilon] + tempVirtualLHSL / standard_lapse_rate
          * (pow(pSurface / pressureLHSL, expon) - 1.0);

        mslp2[ilat][ilon] = pressureLHSL
          * pow(1.0 + (standard_lapse_rate / tempVirtualLHSL) * geoHeightLHSL, exponi);
      }
    } // ilon
  } // ilat
  
}

///////////////////////
// _loadCLW_GField()
//
// Load cloud liquid water in g/kg from kg/kg
//

void MM5Data::_loadCLW_GField()
  
{

  if (clw == NULL)
    return;

  if (clw_g == NULL) {
    clw_g = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }

  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	clw_g[isig][ilat][ilon] = clw[isig][ilat][ilon] * 1000.0;
      }
    }
  }

}

///////////////////////
// _loadRNW_GField()
//
// Load rain liquid water in g/kg from kg/kg
//

void MM5Data::_loadRNW_GField()
  
{

  if (rnw == NULL)
    return;

  if (rnw_g == NULL) {
    rnw_g = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }

  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	rnw_g[isig][ilat][ilon] = rnw[isig][ilat][ilon] * 1000.0;
      }
    }
  }

}

///////////////////////
// _loadQ_GField()
//
// Load mixing ratio in g/kg from kg/kg
//

void MM5Data::_loadQ_GField()
  
{

  if (qq == NULL)
    return;
  
  if (q_g == NULL) {
    q_g = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }

  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	q_g[isig][ilat][ilon] = qq[isig][ilat][ilon] * 1000.0;
      }
    }
  }

}

///////////////////////
// _loadQ2_GField()
//
// Load 2meter mixing ratio in g/kg from kg/kg
//

void MM5Data::_loadQ2_GField()
  
{

  if (q2 == NULL)
    return;

  if (q2_g == NULL) {
    q2_g = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }
  
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      q2_g[ilat][ilon] = q2[ilat][ilon] * 1000.0;
    }
  }

}
   
///////////////////////
// _loadTHETAField()
//
// Calculate potential temperature (theta)
//

void MM5Data::_loadTHETAField()
  
{

  if (qq == NULL || pres == NULL || tk == NULL)
    return;

  double rgas = 287.04;  // J/K/kg
  double rgasmd = .608;   // rgas_moist = rgas*(1.+rgasmd*qvp)
  double cp = 1004.;     // J/K/kg  Note: not using Bolton's value of 1005.7
  double cpmd = .887;   // cp_moist = cp*(1.+cpmd*qvp)
  double gamma = rgas/cp;
  double gammamd = rgasmd-cpmd;  // gamma_moist = gamma*(1.+gammamd*qvp)

  if (theta == NULL) {
    theta = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }

  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
        double gammam = gamma * (1.0 + gammamd*qq[isig][ilat][ilon]);
        double factor1 = ::pow( (1000.0/pres[isig][ilat][ilon]),gammam ); 
	theta[isig][ilat][ilon] = tk[isig][ilat][ilon] * factor1;
      }
    }
  }

}

///////////////////////
// _loadTHETAEField()
//
// Calculate equivalent potential temperature (theta)
//

void MM5Data::_loadTHETAEField()
  
{

  if (tk == NULL || qq == NULL || pres == NULL)
    return;

  fl32 thtecon1 = 3376.; // K
  fl32 thtecon2 = 2.54;
  fl32 thtecon3 = .81;
  fl32 tlclc1 = 2840.;
  fl32 tlclc2 = 3.5;
  fl32 tlclc3 = 4.805;
  fl32 tlclc4 = 55.;
  fl32 eps = 0.622;
  fl32 epsilon =  1.0E-15;
  fl32 rgas = 287.04;  // J/K/kg
  fl32 rgasmd = .608;   // rgas_moist = rgas*(1.+rgasmd*qvp)
  fl32 cp = 1004.;     // J/K/kg  Note: not using Bolton's value of 1005.7
  fl32 cpmd = .887;   // cp_moist = cp*(1.+cpmd*qvp)
  fl32 gamma = rgas/cp;
  fl32 gammamd = rgasmd-cpmd;  // gamma_moist = gamma*(1.+gammamd*qvp)

  if (thetae == NULL) {
    thetae = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }

  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
        fl32 t = tk[isig][ilat][ilon];
        fl32 q = max(qq[isig][ilat][ilon], epsilon);
        fl32 p = pres[isig][ilat][ilon];
        fl32 e = q*p/(eps+q);
        fl32 tlcl = tlclc1/(log(pow(t,tlclc2/e)) - tlclc3) + tlclc4;
	thetae[isig][ilat][ilon] = t* pow ( 1000.0/p , gamma*(1.+gammamd*q)) *
               exp( (thtecon1/tlcl-thtecon2)*q*(1.+thtecon3*q));
      }
    }
  }

}

///////////////////////
// _loadTHETAVField()
//
// Calculate virtual potential temperature (theta)
//

void MM5Data::_loadTHETAVField()
  
{

  if (clw == NULL)
    return;

  // double rgas = 287.04; // J/K/kg
  // double rgasmd = .608; // rgas_moist = rgas*(1.+rgasmd*qvp)
  // double cp = 1004.; // J/K/kg  Note: not using Bolton's value of 1005.7
  // double cpmd = .887; // cp_moist = cp*(1.+cpmd*qvp)
  // double gamma = rgas/cp;
  // double gammamd = rgasmd-cpmd; // gamma_moist = gamma*(1.+gammamd*qvp)
  
  if (thetav == NULL) {
    thetav = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  
  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	thetav[isig][ilat][ilon] = clw[isig][ilat][ilon] * 1000.0;
      }
    }
  }

}

///////////////////////
// _loadTHETA2Field()
//
// Calculate potential temperature (theta) - surface 2D
//

void MM5Data::_loadTHETA2Field()
  
{

  if (q2 == NULL || pres2 == NULL || t2 == NULL)
    return;

  double rgas = 287.04;  // J/K/kg
  double rgasmd = .608;  // rgas_moist = rgas*(1.+rgasmd*qvp)
  double cp = 1004.; // J/K/kg  Note: not using Bolton's value of 1005.7
  double cpmd = .887; // cp_moist = cp*(1.+cpmd*qvp)
  double gamma = rgas/cp;
  double gammamd = rgasmd-cpmd; // gamma_moist = gamma*(1.+gammamd*qvp)
  
  if (theta2 == NULL) {
    theta2 = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }
  
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      double gammam = gamma * (1.0 + gammamd*q2[ilat][ilon]);
      double factor1 = ::pow( (1000.0/pres2[ilat][ilon]),gammam ); 
      theta2[ilat][ilon] = t2[ilat][ilon] * factor1;
    }
  }
  
}

///////////////////////
// _loadTHETAE2Field()
//
// Calculate equivalent potential temperature (theta) - surface 2D
//

void MM5Data::_loadTHETAE2Field()
  
{

  if (t2 == NULL || q2 == NULL || pres2 == NULL)
    return;

  double thtecon1 = 3376.; // K
  double thtecon2 = 2.54;
  double thtecon3 = .81;
  double tlclc1 = 2840.;
  double tlclc2 = 3.5;
  double tlclc3 = 4.805;
  double tlclc4 = 55.;
  double eps = 0.622;
  fl32 epsilon =  1.0E-15;
  double rgas = 287.04;  // J/K/kg
  double rgasmd = .608;   // rgas_moist = rgas*(1.+rgasmd*qvp)
  double cp = 1004.;     // J/K/kg  Note: not using Bolton's value of 1005.7
  double cpmd = .887;   // cp_moist = cp*(1.+cpmd*qvp)
  double gamma = rgas/cp;
  double gammamd = rgasmd-cpmd;  // gamma_moist = gamma*(1.+gammamd*qvp)
      
  if (thetae2 == NULL) {
    thetae2 = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }

  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      double t = t2[ilat][ilon];
      double q = max(q2[ilat][ilon], epsilon);
      double p = pres2[ilat][ilon];
      double e = q*p/(eps+q);
      double tlcl = tlclc1/(log(pow(t,tlclc2/e)) - tlclc3) + tlclc4;
      thetae2[ilat][ilon] = t* pow ( 1000.0/p , gamma*(1.+gammamd*q)) *
        exp( (thtecon1/tlcl-thtecon2)*q*(1.+thtecon3*q));
    }
  }
  
}

///////////////////////
// _loadTHETAV2Field()
//
// Calculate virtual potential temperature (theta) - surface 2D
//

void MM5Data::_loadTHETAV2Field()
  
{

  if (t2 == NULL)
    return;

  // double rgas = 287.04;  // J/K/kg
  // double rgasmd = .608; // rgas_moist = rgas*(1.+rgasmd*qvp)
  // double cp = 1004.;  // J/K/kg  Note: not using Bolton's value of 1005.7
  // double cpmd = .887;   // cp_moist = cp*(1.+cpmd*qvp)
  // double gamma = rgas/cp;
  // double gammamd = rgasmd-cpmd;  // gamma_moist = gamma*(1.+gammamd*qvp)
  
  if (thetav2 == NULL) {
    thetav2 = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }
  
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      thetav2[ilat][ilon] = t2[ilat][ilon] * 1000.0;
    }
  }
  
}

/////////////////////////////////////////////
// Compute rotation of grid relative to TN
//

void MM5Data::_computeGridRotation()
  
{
  
  if (lat == NULL || lon == NULL) {
    return;
  }

  if (gridRotation == NULL) {
    gridRotation = (fl32 **) ucalloc2(nLat, nLon, sizeof(fl32));
  }

  if (proj_type == MERCATOR) {
    // mercator is aligned with TN.
    for (int ilat = 0; ilat < nLat - 1; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
        gridRotation[ilat][ilon] = 0.0;
      }
    }
    return;
  }

  // compute azimuth from this point to the one above
  // for all rows except top

  for (int ilat = 0; ilat < nLat - 1; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      double range, theta;
      PJGLatLon2RTheta(lat[ilat][ilon], lon[ilat][ilon],
                       lat[ilat+1][ilon], lon[ilat+1][ilon],
                       &range, &theta);
      gridRotation[ilat][ilon] = (fl32) theta;
    }
  }

  // for top row, use value from the second-to-top row

  for (int ilon = 0; ilon < nLon; ilon++) {
    gridRotation[nLat - 1][ilon] = gridRotation[nLat - 2][ilon];
  }

}

/////////////////////////////////////////////
// Compute the wind components
// relative to TN instead of the grid

void MM5Data::_computeUVRelToTN()
  
{
  
  if (uu == NULL || vv == NULL) {
    return;
  }
  
  if (uuTn == NULL) {
    uuTn = (fl32 ***) ucalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  if (vvTn == NULL) {
    vvTn = (fl32 ***) ucalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  
  if (proj_type == MERCATOR || gridRotation == NULL) {
    // mercator is aligned with TN.
    for (int ilat = 0; ilat < nLat - 1; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
        for (int isig = 0; isig < nSigma; isig++) {
          uuTn[isig][ilat][ilon] = uu[isig][ilat][ilon];
          vvTn[isig][ilat][ilon] = vv[isig][ilat][ilon];
        }
      }
    }
    return;
  }

  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      double theta = gridRotation[ilat][ilon] * DEG_TO_RAD;
      double sinTheta = sin(theta);
      double cosTheta = cos(theta);
      for (int isig = 0; isig < nSigma; isig++) {
        double u = uu[isig][ilat][ilon];
        double v = vv[isig][ilat][ilon];
        uuTn[isig][ilat][ilon] = u * cosTheta + v * sinTheta;
        vvTn[isig][ilat][ilon] = v * cosTheta - u * sinTheta;
      }
    }
  }

}

/////////////////////////////////////////////
// Compute the wind components
// relative to output grid
//
// rotation of output grid relative to TN is input

void MM5Data::loadUVOutput(fl32 **outputGridRotation)
  
{
  
  if (uuTn == NULL || vvTn == NULL) {
    return;
  }
  
  if (uuOut == NULL) {
    uuOut = (fl32 ***) ucalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  if (vvOut == NULL) {
    vvOut = (fl32 ***) ucalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  
  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      double theta = outputGridRotation[ilat][ilon] * DEG_TO_RAD * -1.0;
      double sinTheta = sin(theta);
      double cosTheta = cos(theta);
      for (int isig = 0; isig < nSigma; isig++) {
        double u = uuTn[isig][ilat][ilon];
        double v = vvTn[isig][ilat][ilon];
        uuOut[isig][ilat][ilon] = u * cosTheta + v * sinTheta;
        vvOut[isig][ilat][ilon] = v * cosTheta - u * sinTheta;
      }
    }
  }

}


