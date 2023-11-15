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
// Era5Data.cc
//
// Read in WRF file. Compute derived fields.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
//////////////////////////////////////////////////////////

#include "Era5Data.hh"
#include <toolsa/file_io.h>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pjg.h>
#include <toolsa/TaArray.hh>

#include <physics/thermo.h>
#include <physics/PhysicsLib.hh>
#include <physics/AdiabatTempLookupTable.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <sys/stat.h>
#include <math.h>
#include <cerrno>
#include <iostream>
using namespace std;

const double Era5Data::MISSING_DOUBLE = -9999.9;
const double Era5Data::BAD_LAT_LON = -9999;

void null_heartbeat(const char *label)
{
  // do nothing
}


//////////////
// Constructor

Era5Data::Era5Data() :
  forecastTime(0),
  genTime(0),
  forecastLeadTime(0),
  _nLon(0),
  _nLat(0),
  _nEta(0),
  _projType(UNKNOWN),
  _projCenterLat(BAD_LAT_LON),
  _projCenterLon(BAD_LAT_LON),
  _trueLat1(BAD_LAT_LON),
  _trueLat2(BAD_LAT_LON),
  _gridDx(0.0),
  _gridDy(0.0),
  _gridMinX(0.0),
  _gridMinY(0.0),
  _llLat(BAD_LAT_LON),
  _llLon(BAD_LAT_LON),
  _inputFileType(UNKNOWN_FILETYPE),
  _unlimited_ix(0),
  _progName(""),
  _path(""),
  _heartbeatFunc(0),
  _ncf(0),
  _ncfError(0),
  _dataDimension(-1),
  _mif(0),
  _mrf(0),
  _halfEta(0),
  _znw(0),
  _field3d(0),
  _field2d(0),
  _fileSize(0),
  _startDateKey(""),
  _gridIdKey(""),
  _latFieldKey(""),
  _lonFieldKey(""),
  _uu(0),
  _vv(0),
  _ppt(0),
  _qq(0),
  _clw(0),
  _rnw(0),
  _ice(0),
  _snow(0),
  _nRain(0),
  _nCloud(0),
  _graupel(0),
  _ww(0),
  _pp(0),
  _pb(0),
  _ph(0),
  _phb(0),
  _DNW(0),
  _MUB(0),
  _MU(0),
  _REFL3D(0),
  _uu_C(0),
  _vv_C(0),
  _ww_C(0),
  _ph_C(0),
  _phb_C(0),
  _uuTn(0),
  _vvTn(0),
  _uuOut(0),
  _vvOut(0),
  _tc(0),
  _wspd(0),
  _wdir(0),
  _zz(0),
  _pres(0),
  _tk(0),
  _rh(0),
  _spec_h(0),
  _spec_h2m(0),
  _dewpt(0),
  _icing(0),
  _clw_g(0),
  _rnw_g(0),
  _q_g(0),
  _theta(0),
  _dbz_3d(0),
  _geo_hgt(0),
  _geo_pot(0),
  _itfadef(0),
  _cape3d(0),
  _cin3d(0),
  _cape(0),
  _cin(0),
  _lcl3d(0),
  _lfc3d(0),
  _el3d(0),
  _lcl(0),
  _lfc(0),
  _el(0),
  _surfP(0),
  _land_mask(0),
  _ground_t(0),
  _rainc(0),
  _rainnc(0),
  _terrain(0),
  _lat(0),
  _lon(0),
  _land_use(0),
  _snowcovr(0),
  _tseasfc(0),
  _pbl_hgt(0),
  _hfx(0),
  _lh(0),
  _snow_we(0),
  _snow_nc(0),
  _graupel_nc(0),
  _soil_t_1(0),
  _soil_t_2(0),
  _soil_t_3(0),
  _soil_t_4(0),
  _soil_t_5(0),
  _soil_m_1(0),
  _soil_m_2(0),
  _soil_m_3(0),
  _soil_m_4(0),
  _soil_m_5(0),
  _soil_d_1(0.0),
  _soil_d_2(0.0),
  _soil_d_3(0.0),
  _soil_d_4(0.0),
  _soil_d_5(0.0),
  _soil_am_1(0),
  _soil_am_2(0),
  _soil_am_3(0),
  _soil_am_4(0),
  _soil_am_5(0),
  _soil_type(0),
  _twp(0),
  _rwp(0),
  _vil(0),
  _sfcrnoff(0),
  _t2(0),
  _q2(0),
  _u10(0),
  _v10(0),
  _snowh(0),
  _th2(0),
  _gridRotation(0),
  _t2c(0),
  _fzlevel(0),
  _rain_total(0),
  _dbz_2d(0),
  _rh2(0),
  _wspd10(0),
  _wdir10(0),
  _q2_g(0),
  _landusef1(0),
  _landusef2(0),
  _landusef6(0),
  _landusef15(0),
  _greenfrac7(0)
{
  // declare an error object - This object continues
  // to affect error handling while it exists. 
  // So we cannot declare it a local variable, because
  // it would go out of scope at the end of the 
  // function.

  _ncfError = new Nc3Error(Nc3Error::silent_nonfatal);
}

//////////////
// init()

bool Era5Data::init(const string &prog_name,
		   const Params &params,
		   const heartbeat_t heartbeat_func)
{
  // Initialize members

  _progName = prog_name;
  _params = params;
  _heartbeatFunc = heartbeat_func;
  if (_heartbeatFunc == 0)
    _heartbeatFunc = null_heartbeat;
  
  return true;
}

//////////////
// initFile()

bool Era5Data::initFile(const string &path)
{
  // Open the file

  _path = path;
  _ncf = new Nc3File(_path.c_str());

  if (!_ncf->is_valid())
  {
    cerr << "ERROR - Era5Data::Era5Data" << endl;
    cerr << "  File: " << _path << endl;
    return false;
  }

  _setDims();

  return true;
}

//////////////
// clearData()

void Era5Data::clearData()
{
  _freeGrids();
}

/////////////
// Destructor

Era5Data::~Era5Data()
{
  _freeGrids();
  
  delete _ncfError;
}


/////////////////////////////////
// read() 
//
// Loads the WRF netcdf file into memory
//
// Returns true on success, false on failure.

bool Era5Data::read()
{
  _heartbeatFunc("Entering Era5Data::read()");
  
  //lets see what kind of WRF file we have.
  if (!_findFileVersion())
  {
    cerr << "FATAL ERROR: Cannot determine the type of the input file\n";
    return false;
  }
  
  _setAttKeys();  //now that we know the file version, set the keys.
  
  if (!_setTimes())
  {
    cerr << "Error setting times\n";
    return false;
  }
  else if (_params.debug >= Params::DEBUG_VERBOSE) 	
  {
    cerr << "Finished setting Times\n";
  }

  _setProjection();
  if (_params.debug >= Params::DEBUG_VERBOSE) 	
  {
    cerr << "Finished setting Projection\n";
  }

  _setMinXY();
  if (_params.debug >= Params::DEBUG_VERBOSE) 	
  {
    cerr << "Finished setting minx & miny\n";
  }
  


  _unlimited_ix++;

  return true;
}


/////////////
// _freeGrids()

void Era5Data::_freeGrids()
{
  if (_uu != 0)
    ufree3((void ***)_uu);
  _uu = 0;
  
  if (_vv != 0)
    ufree3((void ***)_vv);
  _vv = 0;
  
  if (_ppt  != 0)
    ufree3((void ***)_ppt);
  _ppt = 0;
  
  if (_qq != 0)
    ufree3((void ***)_qq);
  _qq = 0;
  
  if (_clw != 0)
    ufree3((void ***)_clw);
  _clw = 0;
  
  if (_rnw != 0)
    ufree3((void ***)_rnw);
  _rnw = 0;
  
  if (_ice != 0)
    ufree3((void ***)_ice);
  _ice = 0;
  
  if (_nCloud != 0)
    ufree3((void ***)_nCloud);
  _nCloud = 0;
  
  if (_nRain != 0)
    ufree3((void ***)_nRain);
  _nRain = 0;
  

  
  if (_snow != 0)
    ufree3((void ***)_snow);
  _snow = 0;
  
  if (_graupel != 0)
    ufree3((void ***)_graupel);
  _graupel = 0;
  
  if (_ww != 0)
    ufree3((void ***)_ww);
  _ww = 0;
  
  if (_pp != 0)
    ufree3((void ***)_pp);
  _pp = 0;
  
  if (_pb != 0)
    ufree3((void ***)_pb);
  _pb = 0;
  
  if (_ph != 0)
    ufree3((void ***)_ph);
  _ph = 0;
  
  if (_phb  != 0)
    ufree3((void ***)_phb);
  _phb = 0;
  
  ////////////////////

  if (_uu_C  != 0)
    ufree3((void ***)_uu_C);
  _uu_C = 0;
  
  if (_vv_C != 0)
    ufree3((void ***)_vv_C);
  _vv_C = 0;
  
  if (_ww_C  != 0)
    ufree3((void ***)_ww_C);
  _ww_C = 0;
  
  if (_ph_C != 0)
    ufree3((void ***)_ph_C);
  _ph_C = 0;
  
  if (_phb_C != 0)
    ufree3((void ***)_phb_C);
  _phb_C = 0;

  if (_DNW != 0)
    ufree3((void ***)_DNW);
  _DNW = 0;

  if (_MUB != 0)
    ufree3((void ***)_MUB);
  _MUB = 0;

  if (_MU != 0)
    ufree3((void ***)_MU);
  _MU = 0;

  if (_REFL3D != 0)
    ufree3((void ***)_REFL3D);
  _REFL3D = 0;
  
  //////////////////////

  if (_uuTn != 0)
    ufree3((void ***)_uuTn);
  _uuTn = 0;
  
  if (_vvTn != 0)
    ufree3((void ***)_vvTn);
  _vvTn = 0;
  
  if (_uuOut != 0)
    ufree3((void ***)_uuOut);
  _uuOut = 0;
  
  if (_vvOut != 0)
    ufree3((void ***)_vvOut);
  _vvOut = 0;
  
  if (_tc != 0)
    ufree3((void ***)_tc);
  _tc = 0;
  
  if (_wspd != 0)
    ufree3((void ***)_wspd);
  _wspd = 0;
  
  if (_wdir != 0)
    ufree3((void ***)_wdir);
  _wdir = 0;
  
  if (_zz != 0)
    ufree3((void ***)_zz);
  _zz = 0;
  
  if (_pres != 0)
    ufree3((void ***)_pres);
  _pres = 0;
  
  if (_tk != 0)
    ufree3((void ***)_tk);
  _tk = 0;
  
  if (_rh != 0)
    ufree3((void ***)_rh);
  _rh = 0;
  
  if (_spec_h != 0)
    ufree3((void ***)_spec_h);
  _spec_h = 0;
  
  if (_spec_h2m != 0)
    ufree2((void **)_spec_h2m);
  _spec_h2m = 0;
  
  if (_dewpt != 0)
    ufree3((void ***)_dewpt);
  _dewpt = 0;
  
  if (_icing != 0)
    ufree3((void ***)_icing);
  _icing = 0;
  
  if (_clw_g != 0)
    ufree3((void ***)_clw_g);
  _clw_g = 0;
  
  if (_rnw_g != 0)
    ufree3((void ***)_rnw_g);
  _rnw_g = 0;
  
  if (_q_g != 0)
    ufree3((void ***)_q_g);
  _q_g = 0;
  
  if (_theta != 0)
    ufree3((void ***)_theta);
  _theta = 0;
  
  if (_dbz_3d != 0)
    ufree3((void ***)_dbz_3d);
  _dbz_3d = 0;
  
  if (_geo_hgt != 0)
    ufree3((void ***)_geo_hgt);
  _geo_hgt = 0;
  
  if (_geo_pot != 0)
    ufree3((void ***)_geo_pot);
  _geo_pot = 0;
  
  if (_itfadef != NULL)
    ufree3((void ***)_itfadef);
  _itfadef = 0;
  
  if (_cape3d != 0)
    ufree3((void ***)_cape3d);
  _cape3d = 0;
  
  if (_cin3d != 0)
    ufree3((void ***)_cin3d);
  _cin3d = 0;
  
  if (_cape != 0)
    ufree2((void **)_cape);
  _cape = 0;
  
  if (_cin != 0)
    ufree2((void **)_cin);
  _cin = 0;
  
  if (_lcl3d != 0)
    ufree3((void ***)_lcl3d);
  _lcl3d = 0;
  
  if (_lfc3d != 0)
    ufree3((void ***)_lfc3d);
  _lfc3d = 0;
  
  if (_el3d != 0)
    ufree3((void ***)_el3d);
  _el3d = 0;
  
  if (_lcl != 0)
    ufree2((void **)_lcl);
  _lcl = 0;
  
  if (_lfc != 0)
    ufree2((void **)_lfc);
  _lfc = 0;
  
  if (_el != 0)
    ufree2((void **)_el);
  _el = 0;

  /////////////////

  if (_surfP != 0)
    ufree2((void **)_surfP);
  _surfP = 0;
  
  if (_land_mask != 0)
    ufree2((void **)_land_mask);
  _land_mask = 0;
  
  /////////////////

  if (_ground_t != 0)
    ufree2((void **)_ground_t);
  _ground_t = 0;
  
  if (_rainc != 0)
    ufree2((void **)_rainc);
  _rainc = 0;
  
  if (_rainnc != 0)
    ufree2((void **)_rainnc);
  _rainnc = 0;
  
  if (_terrain != 0)
    ufree2((void **)_terrain);
  _terrain = 0;
  
  if (_lat != 0)
    ufree2((void **)_lat);
  _lat = 0;
  
  if (_lon != 0)
    ufree2((void **)_lon);
  _lon = 0;
  
  if (_land_use != 0)
    ufree2((void **)_land_use);
  _land_use = 0;
  
  if (_snowcovr != 0)
    ufree2((void **)_snowcovr);
  _snowcovr = 0;
  
  if (_tseasfc != 0)
    ufree2((void **)_tseasfc);
  _tseasfc = 0;
  
  if (_pbl_hgt != 0)
    ufree2((void **)_pbl_hgt);
  _pbl_hgt = 0;
  
  if (_hfx != 0)
    ufree2((void **)_hfx);
  _hfx = 0;
  
  if (_lh != 0)
    ufree2((void **)_lh);
  _lh = 0;
  
  if (_snow_we != 0)
    ufree2((void **)_snow_we);
  _snow_we = 0;
  
  if (_snow_nc != 0)
    ufree2((void **)_snow_nc);
  _snow_nc = 0;
  
  if (_graupel_nc != 0)
    ufree2((void **)_graupel_nc);
  _graupel_nc = 0;
  
  if (_soil_t_1 != 0)
    ufree2((void **)_soil_t_1);
  _soil_t_1 = 0;
  
  if (_soil_t_2 != 0)
    ufree2((void **)_soil_t_2);
  _soil_t_2 = 0;
  
  if (_soil_t_3 != 0)
    ufree2((void **)_soil_t_3);
  _soil_t_3 = 0;
  
  if (_soil_t_4 != 0)
    ufree2((void **)_soil_t_4);
  _soil_t_4 = 0;
  
  if (_soil_t_5 != 0)
    ufree2((void **)_soil_t_5);
  _soil_t_5 = 0;
  
  if (_soil_m_1 != 0)
    ufree2((void **)_soil_m_1);
  _soil_m_1 = 0;
  
  if (_soil_m_2 != 0)
    ufree2((void **)_soil_m_2);
  _soil_m_2 = 0;
  
  if (_soil_m_3 != 0)
    ufree2((void **)_soil_m_3);
  _soil_m_3 = 0;
  
  if (_soil_m_4 != 0)
    ufree2((void **)_soil_m_4);
  _soil_m_4 = 0;
  
  if (_soil_m_5 != 0)
    ufree2((void **)_soil_m_5);
  _soil_m_5 = 0;

  /////////////////

  if (_soil_am_1 != 0)
    ufree2((void **)_soil_am_1);
  _soil_am_1 = 0;
  
  if (_soil_am_2 != 0)
    ufree2((void **)_soil_am_2);
  _soil_am_2 = 0;
  
  if (_soil_am_3 != 0)
    ufree2((void **)_soil_am_3);
  _soil_am_3 = 0;
  
  if (_soil_am_4 != 0)
    ufree2((void **)_soil_am_4);
  _soil_am_4 = 0;
  
  if (_soil_am_5 != 0)
    ufree2((void **)_soil_am_5);
  _soil_am_5 = 0;

  /////////////////

  if (_soil_type != 0)
    ufree2((void **)_soil_type);
  _soil_type = 0;
  
  if (_twp != 0)
    ufree2((void **)_twp);
  _twp = 0;
  
  if (_rwp != 0)
    ufree2((void **)_rwp);
  _rwp = 0;
  
  if (_vil != 0)
    ufree2((void **)_vil);
  _vil = 0;
  
  if (_sfcrnoff != 0)
    ufree2((void **)_sfcrnoff);
  _sfcrnoff = 0;
  
  if (_t2 != 0)
    ufree2((void **)_t2);
  _t2 = 0;
  
  if (_q2 != 0)
    ufree2((void **)_q2);
  _q2 = 0;
  
  if (_u10 != 0)
    ufree2((void **)_u10);
  _u10 = 0;
  
  if (_v10 != 0)
    ufree2((void **)_v10);
  _v10 = 0;
  
  if (_snowh != 0)
    ufree2((void **)_snowh);
  _snowh = 0;
  
  if (_th2 != 0)
    ufree2((void **)_th2);
  _th2 = 0;
  
  /////////////////

  if (_gridRotation != 0)
    ufree2((void **)_gridRotation);
  _gridRotation = 0;
  
  if (_t2c != 0)
    ufree2((void **)_t2c);
  _t2c = 0;
  
  if (_fzlevel != 0)
    ufree2((void **)_fzlevel);
  _fzlevel = 0;
  
  if (_rain_total != 0)
    ufree2((void **)_rain_total);
  _rain_total = 0;
  
  if (_dbz_2d != 0)
    ufree2((void **)_dbz_2d);
  _dbz_2d = 0;
  
  if (_rh2 != 0)
    ufree2((void **)_rh2);
  _rh2 = 0;
  
  if (_wspd10 != 0)
    ufree2((void **)_wspd10);
  _wspd10 = 0;
  
  if (_wdir10 != 0)
    ufree2((void **)_wdir10);
  _wdir10 = 0;
  
  if (_q2_g != 0)
    ufree2((void **)_q2_g);
  _q2_g = 0;
  
  /////////////////

  if (_landusef1 != 0)
    ufree2((void **)_landusef1);
  _landusef1 = 0;
  
  if (_landusef2 != 0)
    ufree2((void **)_landusef2);
  _landusef2 = 0;
  
  if (_landusef6 != 0)
    ufree2((void **)_landusef6);
  _landusef6 = 0;
  
  if (_landusef15 != 0)
    ufree2((void **)_landusef15);
  _landusef15 = 0;
  
  /////////////////

  if (_greenfrac7 != 0)
    ufree2((void **)_greenfrac7);
  _greenfrac7 = 0;
  
  /////////////////

  delete _ncf;
  _ncf = 0;
  
  /////////////////

  if (_mif != 0)
    ufree2((void **)_mif);
  _mif = 0;
  
  if (_mrf != 0)
    ufree2((void **)_mrf);
  _mrf = 0;
  
  if (_halfEta != 0)
    ufree(_halfEta);
  _halfEta = 0;
  
  if (_znw != 0)
    ufree(_znw);
  _znw = 0;
  
  /////////////////

  if (_field3d != 0)
    ufree3((void ***)_field3d);
  _field3d = 0;
  
  if (_field2d != 0)
    ufree2((void **)_field2d);
  _field2d = 0;
}


//////////////////
// Try and figure out what type of input file we are looking at
// This may have to be a bit smarter as it encompases more 
// file varieties
int Era5Data::_findFileVersion()
{
  _heartbeatFunc("Entering Era5Data::_findFileVersion()");
  
  //first check to make sure we have a ARW type WRF file
  // the global attribute 'GRIDTYPE' is C for ARW and E for NMM. (J. Bresch)

  Nc3Att* att;

  att = _ncf->get_att("GRIDTYPE");
  if (att == NULL)
  {
    cerr << "ERROR: - attribute 'GRIDTYPE' missing, cannot determine file version.\n";
    return 0;
  }

  char *gridtype = att->as_string(0);
  delete att;
  if (gridtype[0] != 'C')
  {
    cerr << "ERROR: Expecting GRIDTYPE='C', but got " << gridtype
	 << "this netcdf file type is unsupported\n\n";
    return 0;
  }

  if(_ncf->get_var("XLAT") != NULL)
    _inputFileType = V2;
  else if (_ncf->get_var("XLAT_M") != NULL)
    _inputFileType = GEOGRID;
  else 
  {
    _inputFileType = UNKNOWN_FILETYPE;
    return 0;
  }

  return 1;
}

void Era5Data::_setAttKeys()
{
  _heartbeatFunc("Entering Era5Data::_setAttKeys()");
  
  if (_inputFileType == V2)
  {
    _startDateKey = "START_DATE";
    _gridIdKey = "GRID_ID";
    _latFieldKey = "XLAT";
    _lonFieldKey = "XLONG";
  }
  else if (_inputFileType == GEOGRID)
  {
    _startDateKey = "SIMULATION_START_DATE";
    _gridIdKey = "grid_id";
    _latFieldKey = "XLAT_M";
    _lonFieldKey = "XLONG_M";
  }

  if (_params.debug) {
    cerr << "_startDateKey: " << _startDateKey << endl;
    cerr << "_gridIdKey: " << _gridIdKey << endl;
    cerr << "_latFieldKey: " << _latFieldKey << endl;
    cerr << "_lonFieldKey: " << _lonFieldKey << endl;
  }

}

//////////////////
// Set max values for the dimensions (non staggered)

void Era5Data::_setDims()
{
  _heartbeatFunc("Entering Era5Data::_setDims()");
  
  _nLon = _ncf->get_dim("west_east")->size();
  _nLat = _ncf->get_dim("south_north")->size();
  if (dataDimension() > 2)
    _nEta = _ncf->get_dim("bottom_top")->size();
  else
    _nEta = 1;
}


//////////////
// _load1dField()

void Era5Data::_load1dField(const string &ncd_field_name,
			   const int neta,
			   fl32 * &field)
{
  // If the data is already loaded, don't do anything

  if (field != 0)
    return;
  
  // Access the variable in the netCDF file

  Nc3Var* var = _ncf->get_var(ncd_field_name.c_str());

  if (var == 0)
  {
    cerr << "WARNING: '" << ncd_field_name
	 << "' variable missing from netCDF file" << endl;
    cerr << "Some fields may not be available" << endl;
    return;
  }

  // Allocate space for the data and retrieve it from the file

  field = (fl32 *)umalloc(neta * sizeof(fl32));
  var->get(field, 1, neta);
}

void Era5Data::_load1dField(const string &ncd_field_name,
			   fl32 * &field)
{
  _load1dField(ncd_field_name, _nEta, field);
}


//////////////
// _load2dField()

void Era5Data::_load2dField(const string &ncd_field_name,
			   const int nlat,
			   const int nlon,
			   fl32 ** &field)
{
  // If the data is already loaded, don't do anything

  if (field != 0)
    return;
  
  // Access the variable in the netCDF file

  Nc3Var* var = _ncf->get_var(ncd_field_name.c_str());

  if (var == 0)
  {
    cerr << "WARNING: '" << ncd_field_name
	 << "' variable missing from netCDF file" << endl;
    cerr << "Some fields may not be available" << endl;
    return;
  }

  // Allocate space for the data and retrieve it from the file

  field = (fl32 **)umalloc2(nlat, nlon, sizeof(fl32));
  var->get(*field, 1, nlat, nlon);
}

void Era5Data::_load2dField(const string &ncd_field_name,
			   fl32 ** &field)
{
  _load2dField(ncd_field_name, _nLat, _nLon, field);
}


//////////////
// _load3dField()

void Era5Data::_load3dField(const string &ncd_field_name,
			   const int neta,
			   const int nlat,
			   const int nlon,
			   fl32 *** &field)
{
  // If the data is already loaded, don't do anything

  if (field != 0)
    return;
  
  // Access the variable in the netCDF file

  Nc3Var* var = _ncf->get_var(ncd_field_name.c_str());

  if (var == 0)
  {
    cerr << "WARNING: '" << ncd_field_name
	 << "' variable missing from netCDF file" << endl;
    cerr << "Some fields may not be available" << endl;
    return;
  }

  // Allocate space for the data and retrieve it from the file

  field = (fl32 ***)umalloc3(neta, nlat, nlon, sizeof(fl32));
  var->get(**field, 1, neta, nlat, nlon);
}

void Era5Data::_load3dField(const string &ncd_field_name,
			   fl32 *** &field)
{
  _load3dField(ncd_field_name, _nEta, _nLat, _nLon, field);
}


//////////////
// _loadTemp()

void Era5Data::_loadTemp()
{
  _heartbeatFunc("Entering Era5Data::_loadTemp()");
  
  _load3dField("T", _ppt);
}


//  Tk = (T + 300) / ( (1000 / P)^0.28571)
// Where:
//  Tk is the temperature in Kelvin
//  T is the temperature in the WRF output.
//  P is the pressure in milibars.
void Era5Data::_computeTemp()
{
  _heartbeatFunc("Entering Era5Data::_computeTemp()");
  
  // If we already have the field, don't need to do anything

  if (_tk != 0)
    return;
  
  // Load the needed fields

  _loadTemp();
  
  if (_ppt == 0)
  {
    cerr << "WARNING: PPT field not available" << endl;
    cerr << "Cannot calculate temperature field" << endl;
    
    return;
  }

  _computePressure();
  
  if (_pres == 0)
  {
    cerr << "WARNING: Pressure field not available" << endl;
    cerr << "Cannot calculate temperature field" << endl;
    
    return;
  }

  // Allocate space for the field

  _tk = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Compute the field

  for (int iEta = 0; iEta < _nEta; iEta++)
  {
    for (int iLat = 0; iLat < _nLat; iLat++)
    {
      for (int iLon = 0; iLon < _nLon; iLon++)
	_tk[iEta][iLat][iLon] = 
	  (_ppt[iEta][iLat][iLon] + 300) /
	  pow(( (double)1000 / (double)_pres[iEta][iLat][iLon]),0.28571);
    } // endfor - iLat
  } // endfor - iEta
}


//////////////
// _loadTemp()

void Era5Data::_computeGeopot()
{
  _heartbeatFunc("Entering Era5Data::_computeGeopot()");
  
  // If we have the field already, do nothing

  if (_geo_pot != 0)
    return;
  
  // Load the needed fields

  _loadPertGeo();
  
  if (_ph == 0)
  {
    cerr << "WARNING: perturbation geopotential not available" << endl;
    cerr << "Cannot calculate geopotential" << endl;
    return;
  }

  _loadBSPG();
  
  if (_phb == 0)
  {
    cerr << "WARNING: base state geopotential not available" << endl;
    cerr << "Cannot calculate geopotential" << endl;
    return;
  }

  // Allocate space for the field

  _geo_pot = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Calculate the field

  for (int iEta = 0; iEta < _nEta; iEta++)
  {
    for (int iLat = 0; iLat < _nLat; iLat++)
    {
      for (int iLon = 0; iLon < _nLon; iLon++)
      {
	_geo_pot[iEta][iLat][iLon] = 
	  (_ph[iEta][iLat][iLon] + _phb[iEta][iLat][iLon]);
      } // endfor - iLon
    } // endfor - iLat
  } // endfor - iEta
  
}


//////////////
// _computeGeopotHeight()

void Era5Data::_computeGeopotHeight()
{
  _heartbeatFunc("Entering Era5Data::_computeGeopotHeight()");
  
  // If we already have the field, do nothing

  if (_geo_hgt != 0)
    return;
  
  // Load needed fields

  _computeGeopot();
  
  if (_geo_pot == 0)
  {
    cerr << "WARNING: geo_pot not available" << endl;
    cerr << "Cannot compute geopotential height" << endl;
    return;
  }

  // Allocate space for the field

  _geo_hgt = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Compute the field

  const double GRAVITY_CONSTANT = 9.806;    // m/s^2

  for (int iEta = 0; iEta < _nEta; iEta++)
  {
    for (int iLat = 0; iLat < _nLat; iLat++)
    {
      for (int iLon = 0; iLon < _nLon; iLon++)
      {
	_geo_hgt[iEta][iLat][iLon] = 
	  _geo_pot[iEta][iLat][iLon] / GRAVITY_CONSTANT;
      } // endfor - iLon
    } // endfor - iLat
  } // endfor - iEta
  
}


//////////////
// _computeRH2()

void Era5Data::_computeRH2()
{
  _heartbeatFunc("Entering Era5Data::_computeRH2()");
  
  // If we already have the field, do nothing

  if (_rh2 != 0)
    return;
  
  // Load the needed fields

  _computeGeopotHeight();
  
  if (_geo_hgt == 0)
  {
    cerr << "WARNING: geopotential height not available" << endl;
    cerr << "Cannot compute RH2" << endl;
    return;
  }

  _loadQ2();
  
  if (_q2 == 0)
  {
    cerr << "WARNING: q2 not available" << endl;
    cerr << "Cannot compute RH2" << endl;
    return;
  }

  // _computeT2C();
  
  if (_t2c == 0)
  {
    cerr << "WARNING: t2c not available" << endl;
    cerr << "Cannot compute RH2" << endl;
    return;
  }

  // Allocate space for the field

  _rh2 = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));

  // Compute the field

  for (int iLat = 0; iLat < _nLat; iLat++)
  {
    for (int iLon = 0; iLon < _nLon; iLon++)
    {
      fl32 p2m = _find2MPres(iLat,iLon);
      _rh2[iLat][iLon] = PHYrhmr(_q2[iLat][iLon]*1000, p2m, _t2c[iLat][iLon]);
    } // endfor - iLon
  } // endfor - iLat
  
}

fl32 Era5Data::_find2MPres(int lat, int lon)
{
  _heartbeatFunc("Entering Era5Data::_find2MPres()");
  
  // Load the needed fields

  _computeGeopotHeight();
  
  if (_geo_hgt == 0)
  {
    cerr << "WARNING: geo height not available" << endl;
    cerr << "Cannot compute 2M pressure" << endl;
    return MISSING_DOUBLE;
  }
  
  _computePressure();
  
  if (_pres == 0)
  {
    cerr << "WARNING: pressure not available" << endl;
    cerr << "Cannot compute 2M pressure" << endl;
    return MISSING_DOUBLE;
  }
  
  // Compute the value

  int low_ix;  

  for (low_ix = 0; low_ix<_nEta; low_ix++)
  {
    if( (_geo_hgt[low_ix+1][lat][lon]/9.8) > 2)
      break;
  }
  
  fl32 low_ht = _geo_hgt[low_ix][lat][lon]/9.8;
  fl32 hi_ht = _geo_hgt[low_ix+1][lat][lon]/9.8;

  fl32 low_p = log10(_pres[low_ix][lat][lon]);
  fl32 hi_p = log10(_pres[low_ix+1][lat][lon]);

  //find slope & intercept for simple 2pt interpolation
  fl32 m =( (hi_p - low_p) / (hi_ht - low_ht) );
  fl32 b = hi_p - m*hi_ht;

  if (_params.debug >= Params::DEBUG_VERBOSE)
  {
    cerr << "H,log(p):" << endl
	 << _geo_hgt[low_ix][lat][lon]/9.8 << ","
	 << log10(_pres[low_ix][lat][lon]) << endl
	 << _geo_hgt[low_ix+1][lat][lon]/9.8 << ","
	 << log10(_pres[low_ix+1][lat][lon]) << endl
	 << _geo_hgt[low_ix+2][lat][lon]/9.8 << ","
	 << log10(_pres[low_ix+2][lat][lon]) << endl
	 << _geo_hgt[low_ix+3][lat][lon]/9.8 << ","
	 << log10(_pres[low_ix+3][lat][lon]) << endl;

    cerr << "m,b,p2m: " << m << "," << b << "," << m*2+b << endl;

  }

  return pow(10,m*2+b);
}

//////////////
// _computePressure()

void Era5Data::_computePressure()
{
  _heartbeatFunc("Entering Era5Data::_computePressure()");
  
  // If we've already computed it, don't do it again

  if (_pres != 0)
    return;
  
  // Load the needed fields

  _loadPertPres();
  
  if (_pp == 0)
  {
    cerr << "WARNING: perturbation pressure not available" << endl;
    cerr << "Cannot compute pressure" << endl;
    return;
  }

  _loadBSP();
  
  if (_pb == 0)
  {
    cerr << "WARNING: base state pressure not available" << endl;
    cerr << "Cannot compute pressure" << endl;
    return;
  }

  // Allocate space for the field

  _pres = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Compute the field

  for (int iEta = 0; iEta < _nEta; iEta++)
  {
    for (int iLat = 0; iLat < _nLat; iLat++)
    {
      for (int iLon = 0; iLon < _nLon; iLon++)
      {
	_pres[iEta][iLat][iLon] = 
	  (_pp[iEta][iLat][iLon] + _pb[iEta][iLat][iLon])/100.;
      } // endfor - iLon
    } // endfor - iLat
  } // endfor - iEta
  
}

void Era5Data::_computeTotalRain()
{
  _heartbeatFunc("Entering Era5Data::_computeTotalRain()");
  
  // If we already have the field, do nothing

  if (_rain_total != 0)
    return;
  
  // Load needed fields

  _loadRainC();
  
  if (_rainc == 0)
  {
    cerr << "WARNING: rainc not available" << endl;
    cerr << "Cannot compute total rain" << endl;
    return;
  }

  _loadRainNC();
  
  if (_rainnc == 0)
  {
    cerr << "WARNING: rainnc not available" << endl;
    cerr << "Cannot compute total rain" << endl;
    return;
  }

  // Allocate space for the field

  _rain_total = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));

  // Compute the field

  for (int iLat = 0; iLat < _nLat; iLat++)
  {
    for (int iLon = 0; iLon < _nLon; iLon++)
    {
      _rain_total[iLat][iLon] = (_rainc[iLat][iLon] + _rainnc[iLat][iLon]);    
    } // endfor - iLon
  } // endfor - iLat
  
}


//////////////
// _loadPertPres()

void Era5Data::_loadPertPres()
{
  _heartbeatFunc("Entering Era5Data::_loadPertPres()");
  
  _load3dField("P", _pp);
}


//////////////
// _loadBSP()

void Era5Data::_loadBSP()
{
  _heartbeatFunc("Entering Era5Data::_loadBSP()");
  
  _load3dField("PB", _pb);
}


//////////////
// _loadPertGeoC()

void Era5Data::_loadPertGeoC()
{
  _heartbeatFunc("Entering Era5Data::_loadPertGeoC()");
  
  _load3dField("PH", _nEta+1, _nLat, _nLon, _ph_C);
}


//////////////
// _loadPertGeo()

void Era5Data::_loadPertGeo()
{
  _heartbeatFunc("Entering Era5Data::_loadPertGeo()");
  
  // If we already have the field, don't do anything

  if (_ph != 0)
    return;
  
  // Load the PH field

  _loadPertGeoC();
  
  if (_ph_C == 0)
  {
    cerr << "WARNING: pert geo C field not available" << endl;
    cerr << "Cannot compute pert geo" << endl;
    
    return;
  }
  
  // Allocate space for the destaged field

  _ph = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Destage the field

  int iEta, iLon, iLat;

  for (iEta = 0; iEta < _nEta; iEta++)
  {
    for (iLat = 0; iLat < _nLat; iLat++)
    {
      for(iLon = 0; iLon < _nLon; iLon++)
      {
	_ph[iEta][iLat][iLon] = 
	  (_ph_C[iEta][iLat][iLon] + _ph_C[iEta+1][iLat][iLon]) / 2.0;
      } // endfor - iLon
    } // endfor - iLat
  } // endfor - iEta
  
}


//////////////
// _loadDNW()

void Era5Data::_loadDNW()
{
  _heartbeatFunc("Entering Era5Data::_loadDNW()");
  
  _load3dField("DNW", _nEta+1, _nLat, _nLon, _DNW);
}

//////////////
// _loadMUB()

void Era5Data::_loadMUB()
{
  _heartbeatFunc("Entering Era5Data::_loadMUB()");
  
  _load3dField("MUB", _nEta+1, _nLat, _nLon, _MUB);
}

//////////////
// _loadMU()

void Era5Data::_loadMU()
{
  _heartbeatFunc("Entering Era5Data::_loadMU()");
  
  _load3dField("MU", _nEta+1, _nLat, _nLon, _MU);
}

//////////////
// _loadREFL3D()

void Era5Data::_loadREFL3D()
{
  _heartbeatFunc("Entering Era5Data::_loadREFL3D()");
  
  _load3dField("REFL_10CM", _nEta, _nLat, _nLon, _REFL3D);
}

//////////////
// _loadBSPGC()

void Era5Data::_loadBSPGC()
{
  _heartbeatFunc("Entering Era5Data::_loadBSPGC()");
  
  _load3dField("PHB", _nEta+1, _nLat, _nLon, _phb_C);
}


//////////////
// _loadBSPG()

void Era5Data::_loadBSPG()
{
  _heartbeatFunc("Entering Era5Data::_loadBSPG()");
  
  // If the field already exists, don't do anything

  if (_phb != 0)
    return;
  
  // Load the PHB variable from the netCDF file

  _loadBSPGC();
  
  if (_phb_C == 0)
  {
    cerr << "WARNING: PHB C field not available" << endl;
    cerr << "Cannot compute PHB" << endl;
    return;
  }
  
  // Allocate space for the destaged field

  _phb = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Destage the field

  int iEta, iLon, iLat;

  for (iEta = 0; iEta < _nEta; iEta++)
  {
    for (iLat = 0; iLat < _nLat; iLat++)
    {
      for (iLon = 0; iLon < _nLon; iLon++)
      {
	_phb[iEta][iLat][iLon] = 
	  (_phb_C[iEta][iLat][iLon] + _phb_C[iEta+1][iLat][iLon]) / 2.0;
      } // endfor - iLon
    } // endfor - iLat
  } // endfor - iEta
  
}


//////////////
// _loadItfadef()

void Era5Data::_loadItfadef()
{
  _heartbeatFunc("Entering Era5Data::_loadItfadef()");
  
  _load3dField("ITFADEF",_itfadef);
}


//////////////
// _loadUCWind()

void Era5Data::_loadUCWind()
{
  _heartbeatFunc("Entering Era5Data::_loadUCWind()");
  
  _load3dField("U", _nEta, _nLat, _nLon+1, _uu_C);
}

//////////////
// _loadUWind()

void Era5Data::_loadUWind()
{
  _heartbeatFunc("Entering Era5Data::_loadUWind()");
  
  // If the data is already loaded, don't need to do anything

  if (_uu != 0)
    return;
  
  // Load the raw field from the netCDF file

  _loadUCWind();
  
  if (_uu_C == 0)
  {
    cerr << "WARNING: UU_C not available" << endl;
    cerr << "Cannot compute UU" << endl;
    return;
  }
  
  // Allocate space for the destaged U data

  _uu = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Calculate the destaged U data

  int iEta, iLon, iLat;

  for (iEta = 0; iEta < _nEta; iEta++)
  {
    for (iLat = 0; iLat < _nLat; iLat++)
    {
      for (iLon = 0; iLon < _nLon; iLon++)
      {
	_uu[iEta][iLat][iLon] = 
	  (_uu_C[iEta][iLat][iLon] + _uu_C[iEta][iLat][iLon+1]) / 2.0;

      } // endfor - iLon
    } // endfor - iLat
  } // endfor - iEta
}

//////////////
// _loadWCWind()

void Era5Data::_loadWCWind()
{
  _heartbeatFunc("Entering Era5Data::_loadWCWind()");
  
  _load3dField("W", _nEta+1, _nLat, _nLon, _ww_C);
}


//////////////
// _loadWWind()

void Era5Data::_loadWWind()
{
  _heartbeatFunc("Entering Era5Data::_loadWWind()");
  
  // If the data is already loaded, don't need to do anything

  if (_ww != 0)
    return;
  
  // Load the raw W field from the netCDF file

  _loadWCWind();
  
  if (_ww_C == 0)
  {
    cerr << "WARNING: WW_C not available" << endl;
    cerr << "Cannot compute WW" << endl;
    return;
  }
  
  // Allocate space for the destaged data

  _ww = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Destage the data

  int iEta, iLon, iLat;

  for (iEta = 0; iEta < _nEta; iEta++)
  {
    for (iLat = 0; iLat < _nLat; iLat++)
    {
      for (iLon = 0; iLon < _nLon; iLon++)
      {
	_ww[iEta][iLat][iLon] = 
	  (_ww_C[iEta][iLat][iLon] + _ww_C[iEta+1][iLat][iLon]) / 2.0;
      } // endfor - iLon
    } // endfor - iLat
  } // endfor - iEta
  
}


//////////////
// _loadVCWind()

void Era5Data::_loadVCWind()
{
  _heartbeatFunc("Entering Era5Data::_loadVCWind()");
  
  _load3dField("V", _nEta, _nLat+1, _nLon, _vv_C);
}


//////////////
// _loadVWind()

void Era5Data::_loadVWind()
{
  _heartbeatFunc("Entering Era5Data::_loadVWind()");
  
  // If we've already loaded the data, we don't need to do it again

  if (_vv != 0)
    return;
  
  // Load the raw V field from the netCDF file

  _loadVCWind();

  if (_vv_C == 0)
  {
    cerr << "WARNING: VV_C not available" << endl;
    cerr << "Cannot compute VV" << endl;
    return;
  }
  
  // Allocate space for the destaged data

  _vv = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Destage the data

  int iEta, iLon, iLat;

  for (iEta = 0; iEta < _nEta; iEta++)
  {
    for (iLat = 0; iLat < _nLat; iLat++)
    {
      for (iLon = 0; iLon < _nLon; iLon++)
      {
	_vv[iEta][iLat][iLon] = 
	  (_vv_C[iEta][iLat][iLon] + _vv_C[iEta][iLat+1][iLon]) / 2.0;

      } // endfor - iLon
    } // endfor - iLat
  } // endfor - iEta
}


//////////////
// _loadWaterMixingRatio()

void Era5Data::_loadWaterMixingRatio()
{
  _heartbeatFunc("Entering Era5Data::_loadWaterMixingRatio()");
  
  _load3dField("QVAPOR", _qq);
}

void Era5Data::_loadGraupelMixingRatio()
{
  _heartbeatFunc("Entering Era5Data::_loadGraupelMixingRatio()");
  
  _load3dField("QGRAUP", _graupel);
}


//////////////
// _loadCloudMixingRatio()

void Era5Data::_loadCloudMixingRatio()
{
  _heartbeatFunc("Entering Era5Data::_loadCloudMixingRatio()");
  
  _load3dField("QCLOUD", _clw);
}


//////////////
// _loadRainMixingRatio()

void Era5Data::_loadRainMixingRatio()
{
  _heartbeatFunc("Entering Era5Data::_loadRainMixingRatio()");
  
  _load3dField("QRAIN", _rnw);
}

void Era5Data::_loadSnowMixingRatio()
{
  _heartbeatFunc("Entering Era5Data::_loadSnowMixingRatio()");
  
  _load3dField("QSNOW", _snow);
}

void Era5Data::_loadNRain()
{
  _heartbeatFunc("Entering Era5Data::_loadNRain()");
  
  _load3dField("QNRAIN", _nRain);
}

void Era5Data::_loadNCloud()
{
  _heartbeatFunc("Entering Era5Data::_loadNCloud()");
  
  _load3dField("QNCLOUD", _nCloud);
}




//////////////
// _loadIceMixingRatio()

void Era5Data::_loadIceMixingRatio()
{
  _heartbeatFunc("Entering Era5Data::_loadIceMixingRatio()");
  
  _load3dField("QICE", _ice);
}

void Era5Data::_loadLat()
{
  _heartbeatFunc("Entering Era5Data::_loadLat()");
  
  _load2dField(_latFieldKey, _lat);
  
  if (_lat == 0)
    return;

  // Now check if _llLat never got set, we can set it from this field

  if (_llLat == BAD_LAT_LON)
  {
    _llLat = _lat[0][0];
    if (_params.debug >= Params::DEBUG_VERBOSE)
      cerr << "INFO: Using lat[0][0] (" << _lat[0][0] << ") for ll_lat\n";
  }

}

void Era5Data::_loadLon()
{
  _heartbeatFunc("Entering Era5Data::_loadLon()");
  
  _load2dField(_lonFieldKey, _lon);
  
  if (_lon == 0)
    return;
  
  // Now check if _llLon never got set, we can set it from this field

  if (_llLon == BAD_LAT_LON)
  {
    _llLon = _lon[0][0];
    if (_params.debug >= Params::DEBUG_VERBOSE)
      cerr << "INFO: Using _lon[0][0] (" << _lon[0][0] << ") for ll_lon\n";
  }

}

void Era5Data::_loadSeaSurfaceTemp()
{
  _heartbeatFunc("Entering Era5Data::_loadSeaSurfaceTemp()");
  
  _load2dField("SST", _tseasfc);
}

void Era5Data::_loadGroundTemperature()
{
  _heartbeatFunc("Entering Era5Data::_loadGroudTemperature()");
  
  _load2dField("TSK", _ground_t);
}

void Era5Data::_loadLandMask()
{
  _heartbeatFunc("Entering Era5Data::_loadLandMask()");
  
  _load2dField("LANDMASK", _land_mask);
}

void Era5Data::_loadT2()
{
  _heartbeatFunc("Entering Era5Data::_loadT2()");
  
  _load2dField("T2", _t2);
}

void Era5Data::_loadQ2()
{
  _heartbeatFunc("Entering Era5Data::_loadQ2()");
  
  _load2dField("Q2", _q2);
}

void Era5Data::_loadU10()
{
  _heartbeatFunc("Entering Era5Data::_loadU10()");
  
  _load2dField("U10", _u10);
  
  // If there was an error loading the field, don't do anything else

  if (_u10 == 0)
    return;
  

}

void Era5Data::_loadV10()
{
  _heartbeatFunc("Entering Era5Data::_loadV10()");
  
  _load2dField("V10", _v10);
  
  // If there was an error loading the field, don't do anything else

  if (_v10 == 0)
    return;
  

}

void Era5Data::_loadRainC()
{
  _heartbeatFunc("Entering Era5Data::_loadRainC()");
  
  _load2dField("RAINC", _rainc);
}

void Era5Data::_loadRainNC()
{
  _heartbeatFunc("Entering Era5Data::_loadRainNC()");
  
  _load2dField("RAINNC", _rainnc);
}

void Era5Data::_loadSurfaceRunoff()
{
  _heartbeatFunc("Entering Era5Data::_loadSurfaceRunoff()");
  
  _load2dField("SFROFF", _sfcrnoff);
}

void Era5Data::_loadPBLHeight()
{
  _heartbeatFunc("Entering Era5Data::_loadPBLHeight()");
  
  _load2dField("PBLH", _pbl_hgt);
}

void Era5Data::_loadSurfacePressure()
{
  _heartbeatFunc("Entering Era5Data::_loadSurfacePressure()");
  
  _load2dField("PSFC", _surfP);
}

void Era5Data::_loadSnowHeight()
{
  _heartbeatFunc("Entering Era5Data::_loadSnowHeight()");
  
  _load2dField("SNOWH", _snowh);
}

void Era5Data::_loadSnowCover()
{
  _heartbeatFunc("Entering Era5Data::_loadSnowCover()");
  
  _load2dField("SNOWC", _snowcovr);
}

void Era5Data::_loadLandUse()
{
  _heartbeatFunc("Entering Era5Data::_loadLandUse()");
  
  _load2dField("LU_INDEX", _land_use);
}

void Era5Data::_loadLandUsef()
{
  _heartbeatFunc("Entering Era5Data::_loadLandUsef()");
  
  // If we already have the data, do nothing

  if (_landusef1 != 0 && _landusef2 != 0 && _landusef6 != 0 && _landusef15 != 0)
    return;
  
  // Access the variable in the netCDF file

  Nc3Var* lu_var = _ncf->get_var("LANDUSEF");
  if(lu_var == NULL)
  {
    cerr << "WARNING: 'LANDUSEF' variable missing from netCDF file" << endl;
    cerr << "some fields may not be available" << endl;
    return;
  }

  // Allocate space for the fields and load the data

  _landusef1 = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));
  _landusef2 = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));
  _landusef6 = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));
  _landusef15 = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));

  lu_var->set_cur(0,0);
  lu_var->get(*_landusef1, 1, 1, _nLat, _nLon);     

  lu_var->set_cur(0,1);
  lu_var->get(*_landusef2, 1, 1, _nLat, _nLon);     

  lu_var->set_cur(0,5);
  lu_var->get(*_landusef6, 1, 1, _nLat, _nLon);     

  lu_var->set_cur(0,14);
  lu_var->get(*_landusef15, 1, 1, _nLat, _nLon);     
}

void Era5Data::_loadGreenFrac7()
{
  _heartbeatFunc("Entering Era5Data::_loadGreenFrac7()");
  
  _load2dField("GREENFRAC", _greenfrac7);
}


void Era5Data::_loadTerrainHeight()
{
  _heartbeatFunc("Entering Era5Data::_loadTerrainHeight()");
  
  _load2dField("HGT", _terrain);
}

void Era5Data::_loadTH2()
{
  _heartbeatFunc("Entering Era5Data::_loadTH2()");
  
  _load2dField("TH2", _th2);
}

void Era5Data::_loadHFX()
{
  _heartbeatFunc("Entering Era5Data::_loadHFX()");
  
  _load2dField("HFX", _hfx);
}

void Era5Data::_loadLH()
{
  _heartbeatFunc("Entering Era5Data::_loadLH()");
  
  _load2dField("LH", _lh);
}

void Era5Data::_loadSnowWE()
{
  _heartbeatFunc("Entering Era5Data::_loadSnowWE()");
  
  _load2dField("SNOW", _snow_we);
}

void Era5Data::_loadSnowNC()
{
  _heartbeatFunc("Entering Era5Data::_loadSnowNC()");
  
  _load2dField("SNOWNC", _snow_nc);
}

void Era5Data::_loadGraupelNC()
{
  _heartbeatFunc("Entering Era5Data::_loadGraupelNC()");
  
  _load2dField("GRAUPELNC", _graupel_nc);
}

void Era5Data::_loadSoilType()
{
  _heartbeatFunc("Entering Era5Data::_loadSoilType()");
  
  _load2dField("ISLTYP", _soil_type);
}

void Era5Data::_loadASoil(string WRF_var_name, fl32**& soil_data, int layer)
{
  _heartbeatFunc(("Entering Era5Data::_loadASoil(): " + WRF_var_name).c_str());
  
  // If we already have this field, do nothing

  if (soil_data != 0)
    return;
  
  // Access the field in the netCDF file

  Nc3Var* soilVar = _ncf->get_var(WRF_var_name.c_str());
  if (soilVar == 0)
  {
    cerr << "WARNING: '" << WRF_var_name
	 << "' variable missing from netCDF file" << endl;
    cerr << "some fields may not be available" << endl;
    return;
  }

  // Access the dimension in the netCDF file

  Nc3Dim *dim = soilVar->get_dim(1);
  if (dim == 0 || dim->size() <= layer)
  {
    cerr << "WARNING: layer: " << layer << " not valid for '" 
	 << WRF_var_name << "' - some fields may not be available\n";
    return;
  }

  // Allocate space for the field and load the data

  soil_data = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));

  soilVar->set_cur(0,layer);
  soilVar->get(*soil_data, 1, 1, _nLat, _nLon);
}

void Era5Data::_loadSoilInfo()
{
  _heartbeatFunc("Entering Era5Data::_loadSoilInfo()");
  
  _loadASoil("TSLB", _soil_t_1, 0);
  _loadASoil("TSLB", _soil_t_2, 1);
  _loadASoil("TSLB", _soil_t_3, 2);
  _loadASoil("TSLB", _soil_t_4, 3);
  _loadASoil("TSLB", _soil_t_5, 4);

  _loadASoil("SMOIS", _soil_m_1, 0);
  _loadASoil("SMOIS", _soil_m_2, 1);
  _loadASoil("SMOIS", _soil_m_3, 2);
  _loadASoil("SMOIS", _soil_m_4, 3);
  _loadASoil("SMOIS", _soil_m_5, 4);

  Nc3Var* var = _ncf->get_var("ZS");
  if(var == NULL) 
  {
    cerr << "WARNING: 'ZS' variable missing - some fields may not be available\n";
    return;
  }

  var->set_cur(0,0);
  var->get(&_soil_d_1, 1, 1);
  var->set_cur(0,1);
  var->get(&_soil_d_2, 1, 1);
  var->set_cur(0,2);
  var->get(&_soil_d_3, 1, 1);
  var->set_cur(0,3);
  var->get(&_soil_d_4, 1, 1);
  var->set_cur(0,4);
  var->get(&_soil_d_5, 1, 1);
}


void Era5Data::_loadHalfEta()
{
  _heartbeatFunc("Entering Era5Data::_loadHalfEta()");
  
  _load1dField("ZNU", _halfEta);
}

void Era5Data::_loadZnw()
{
  _heartbeatFunc("Entering Era5Data::_loadZnw()");
  
  _load1dField("ZNW", _nEta+1, _znw);
}

// calculate and return the central scale for stereographic projections.
double Era5Data::calculate_central_scale()
{
  _heartbeatFunc("Entering Era5Data::calculate_central_scale()");
  
  //central_scale magick courtesy of Arnoud & Frank H.
  double central_scale;
  central_scale = ( 1 + cos((90 - _trueLat1)*PI/180) ) / 2;   
  return central_scale;

}


//////////////////////////////////
// Set the map projection from _ncf.MAP_PROJ
// This handles many possible values:
// 1 = LambertConformal, 2 = Stereographic, 3 = Mercator
//
// Also case insensitive subsets of a name will match
// (i.e. Lambert, lambert, etc.)
void  Era5Data::_setProjection()
{
  _heartbeatFunc("Entering Era5Data::_setProjection()");
  
  //set projection first
  
  Nc3Att* att;

  att = _ncf->get_att("MAP_PROJ");
  if (att == NULL)
    {
      cerr << "WARNING: attribute 'MAP_PROJ' missing, some fields may be unavailable.\n";
      return;
    }

  char *map_proj = att->as_string(0);
  delete att;


  for(unsigned int i = 0; i<strlen(map_proj); i++)
    map_proj[i] = toupper(map_proj[i]);

  if((map_proj[0] == '1') ||  (strstr(map_proj,"LAMBERT")))
    _projType = LAMBERT_CONF;
  else if ((map_proj[0] == '2')  ||  (strstr(map_proj,"STEREOGRAPHIC")))
    _projType = STEREOGRAPHIC;
  else if ((map_proj[0] == '3') ||  (strstr(map_proj,"MERCATOR")))
    _projType = MERCATOR;

  if (_projType != LAMBERT_CONF)
    cerr << "WARNING:!!!\n"
	 << "WRF Projections other than Lambert Conformal have not\n"
	 << "been thoroughly tested.  Please make sure that the projection\n"
	 << "parameters like projection origin, etc. are being set correctly\n"
	 << "then modify this statment in Era5Data::_setProjection() to allow\n"
	 << "your projection type through without this warning\n";

  delete [] map_proj;


  char *attString;
  //set  grid origins
  att = _ncf->get_att("CEN_LAT");
  if(att == NULL)
    cerr << "WARNING: 'CEN_LAT' attribute unavailable - projection may be wrong.\n";
  else
    {
      attString = att->as_string(0);
      _projCenterLat = atof(attString);
      delete [] attString;
    }
  delete att;


  att = _ncf->get_att("STAND_LON");
  if(att == NULL)
    cerr << "WARNING: 'STAND_LON' attribute unavailable - projection may be wrong.\n";
  else
    {
      attString = att->as_string(0);
      _projCenterLon = atof(attString);
      delete [] attString;
    }
  delete att;  

  // set  _trueLat1, _trueLat2; (should we only do this if we are a LAMBERT proj?)
  // certainly not because these are used by other projections.  For example
  //truelat1 is used in the case of a polar stereographic proj.
  att = _ncf->get_att("TRUELAT1");
  if(att == NULL)
    cerr << "WARNING: 'TRUELAT1' attribute unavailable - some fields may be missing\n";
  else
     {
      attString = att->as_string(0);
      _trueLat1 = atof(attString);
      delete [] attString;
    }
  delete att;

  att = _ncf->get_att("TRUELAT2");
  if(att == NULL)
    cerr << "WARNING: 'TRUELAT2' attribute unavailable - some fields may be missing\n";
  else
    {
      attString = att->as_string(0);
      _trueLat2 = atof(attString);
      delete [] attString;
    }
  delete att;

  //set dx & dy
  att = _ncf->get_att("DX");
  if(att == NULL)
    cerr << "WARNING: 'DX' attribute unavailable - some fields may be missing\n";
  else
    {
      attString = att->as_string(0);
      _gridDx =  atof(attString)/1000.;	
      delete [] attString;
    }
  delete att;

  att = _ncf->get_att("DY");
  if(att == NULL)
    cerr << "WARNING: 'DY' attribute unavailable - some fields may be missing\n";
  else
    {
      attString = att->as_string(0);
      _gridDy =  atof(attString)/1000.;	
      delete [] attString;
    }
  delete att;


  //get _llLat & long

  Nc3Var *ncvar;
  ncvar = _ncf->get_var("LON_LL_T");
  if (ncvar == NULL)
  {
    cerr << "WARNING: 'LON_LL_T' unavailable\n";
    // If we can't find the variable, get _llLat from the lat grid
    _loadLon();
  }
  else
  {
    ncvar->get(&_llLon,1,1);
  }
  
  ncvar = _ncf->get_var("LAT_LL_T");
  if (ncvar == NULL)
  {
    cerr << "WARNING: 'LAT_LL_T' unavailable\n";
    // If we can't find the variable, get _llLon from the lon grid
    _loadLat();
  }
  else
  {
    ncvar->get(&_llLat,1,1);
  }
  
}

//this function sets the _gridMinX & _gridMinY
// _projType, _projCenterLat, _projCenterLon, _llLon, & _llLat must
// be set before calling _setMinXY().
// returns 1 on sucess, 0 on failure
int Era5Data::_setMinXY()
{
  _heartbeatFunc("Entering Era5Data::_setMinXY()");
  

  if (_llLat == BAD_LAT_LON || _llLon == BAD_LAT_LON)
    {
      cerr << 
	"ERROR: minx & miny cannot be set until after ll_lat & ll_lon are set.\n\n";
      return 0;
    }

  MdvxProj proj;
  if (!initProjection(proj))
    return 0;

  double x,y;
  proj.latlon2xy(_llLat,_llLon,x,y);
  _gridMinX = x;
  _gridMinY = y;

  return 1;

}

//initProjection() will initialize the proj object by calling
// the correctin MdvxProj::init*() function based upon _projType
// and passing it the appropriate projection parameters
// returns 1 on sucess, 0 on failure
  
int Era5Data::initProjection(MdvxProj &proj)
{
  _heartbeatFunc("Entering Era5Data::initProjection()");
  
  if (_projType == UNKNOWN)
  {
    cerr << 
      "ERROR: proj cannot be initialized until _projType is set.\n\n";
    return 0;
  }
  if (_projCenterLat == BAD_LAT_LON || _projCenterLon == BAD_LAT_LON)
  {
    cerr <<
      "ERROR: proj cannot be initialized until _projCenterLat & _projCenterLon are set.\n\n";
    return 0;
  }

  if (_projType == LAMBERT_CONF && 
      (_trueLat1 == BAD_LAT_LON || _trueLat2 == BAD_LAT_LON))
  {
    cerr <<
      "ERROR: LAMBERT_CONF requires _trueLat1 & _trueLat2 to be set "
      "in Era5Data::initProjection()\n\n";
    return 0;
  }

  if (_projType == STEREOGRAPHIC && _trueLat1 == BAD_LAT_LON)
  {
    cerr <<
      "ERROR: STEREOGRAPHIC requires _trueLat1 to be set "
      "in Era5Data::_initProjection()\n\n";
    return 0;
  }
    

  switch (_projType)
  {
  case  LAMBERT_CONF:
    proj.initLc2(_projCenterLat,_projCenterLon,
		 _trueLat1, _trueLat2);
    break;
   
  case  STEREOGRAPHIC:
    cerr <<  "central scale: " << calculate_central_scale() << endl;
    proj.initPolarStereo(_projCenterLat, _projCenterLon,
			 _projCenterLon,
			 (_trueLat1 >= 0 ? Mdvx::POLE_NORTH : Mdvx::POLE_SOUTH),
			 calculate_central_scale());
    break;
   
  case  MERCATOR:
    proj.initMercator(_projCenterLat, _projCenterLon);
    break;

  case UNKNOWN:
    cerr << "Warning _projType is UNKNOWN in Era5Data::initProjection()" << endl;
    break;
  }

  return 1;
}

//////////////////////////////////
// Get the gen time from the filename and the
// start time from the file
//
bool  Era5Data::_setTimes()
{
  _heartbeatFunc("Entering Era5Data::_setTimes()");
  

  // int gyear, gmonth, gday, ghour, gmin, gsec;
  // int fyear, fmonth, fday, fhour, fmin, fsec;
  // int numtokens, domain;
  // DateTime gDateTime,fDateTime;

#ifdef JUNK
  if (_params.get_times_from_filenames)
    {
      //get forecast & gen time from the filename
      //  d1.yyyymmddhh.tmHHMM.wrf
      //where yyyymmddhh is the gen time
      //HHMM is the forecast lead time

      //Example:
      //d1.2007050112.tm0000.wrf
      int leadH,leadM;
      string filename = _path.substr(_path.rfind('/')+1);
      if((numtokens = sscanf(filename.c_str(), "d%1d.%4d%2d%2d%2d.tm%2d%2d.wrf",
			     &domain, &gyear,&gmonth,&gday,&ghour,&leadH,&leadM)) != 7)
	{ 
	  cerr << "ERROR - Filename: " << filename 
	       << " does not include needed date string" << endl;

	  return false;
	}	
      
      gDateTime.set(gyear, gmonth, gday, ghour);
      fDateTime = gDateTime;
      fDateTime += (leadH*60*60+leadM*60);
    }
  else //get times from the file data
    {
      //get forecast time from the Times var.
      int dateStringLen = _ncf->get_dim("DateStrLen")->size();
      TaArray<char> forecastTime_;
      char *forecastTime = forecastTime_.alloc(dateStringLen+1);
      memset(forecastTime, 0, dateStringLen + 1);
      Nc3Var *time_var = _ncf->get_var("Times");
      time_var->get(forecastTime,1,dateStringLen);

      // Date looks like this: 2007-01-08_12:00:00
      if((numtokens = sscanf(forecastTime, "%4d-%2d-%2d_%2d:%2d:%2d",
			     &fyear,&fmonth,&fday,&fhour,&fmin,&fsec)) != 6)
	{	
	  cerr << "ERROR - forecastTime: " << forecastTime
	       << " in wrong format" << endl;

	  return false;
	}	
      fDateTime.set(fyear,fmonth,fday,fhour,fmin,fsec);

      //get the genTime
      Nc3Att* att;
      //      char *attString;
      att =  _ncf->get_att(_startDateKey.c_str());
      if (att == NULL)
	{
	  cerr << "Can't find start date attribute with key: "
	       << _startDateKey << endl;
	  return true;
	}
      char *genTimeString = att->as_string(0);
      delete att;

      if((numtokens = sscanf(genTimeString, "%4d-%2d-%2d_%2d:%2d:%2d",
			     &gyear,&gmonth,&gday,&ghour,&gmin,&gsec)) != 6)
	{	
	  cerr << "ERROR - Attribute: " << genTimeString
	       << " does not include needed date string" << endl;

	  delete [] genTimeString;
	  return false;
	}	
      delete [] genTimeString; //we have to delete this.
      gDateTime.set(gyear,gmonth,gday,ghour,gmin,gsec);

      //get the domain
      
      att = _ncf->get_att(_gridIdKey.c_str());
      if (att == NULL)
	{
	  cerr << "Can't find grid id attribute with key: "
	       << _gridIdKey << endl;
	  return true;
	}
      char *domainString = att->as_string(0);
      delete att;

      domain = atoi(domainString);
      delete [] domainString;
    }
#endif
  
  // if (_params.debug >= Params::DEBUG_NORM) 	
  //   {
  //     cerr << "\nFORECAST TIME\n";
  //     cerr << fDateTime.getStrn() << endl;
  //     cerr << "domain = " << domain << endl;
  //   }

  // Define the model forecast time using time from model output filename
  // forecastTime = fDateTime.utime();
  
  // if (_params.debug >= Params::DEBUG_NORM) 	
  //   {
  //     cerr << "\nGENERATION TIME\n";
  //     cerr << gDateTime.getStrn() << endl;
  //     cerr << "domain = " << domain << endl;
  //   }

  //   // Set the model gen time
  // genTime = gDateTime.utime();

  // forecastLeadTime = forecastTime - genTime;

  return true;
}


//////////////////
// interp3dField()
//
// Load up the eta field array interpolated for a given point.
//
// returns vector of interpolated data.

void Era5Data::interp3dField(int ilat, int ilon,
			    const char *name, fl32 ***field,
			    double wt_sw, double wt_nw,
			    double wt_ne, double wt_se,
			    vector<double> &interp_data,
			    const vector<bool> *eta_needed) const

{
  static const string method_name = "Era5Data::interp3dField()";
  
  _heartbeatFunc("Entering Era5Data::interp3dField()");
  
  interp_data.clear();

  for (int isig = 0; isig < _nEta; isig++)
    interp_data.push_back(MISSING_DOUBLE);

  if (field == 0)
  {
    cerr << "ERROR - " << method_name << endl;
    cerr << name << " array not loaded yet, operation invalid." << endl;
    return;
  }
  
  if (wt_sw + wt_nw + wt_ne + wt_se == 0.0)
    return;
  
  for (int isig = 0; isig < _nEta; isig++)
  {
    if (!eta_needed || (*eta_needed)[isig])
    {
      double wt_sum = 0.0;

      if (wt_sw != 0.0)
	wt_sum += field[isig][ilat][ilon] * wt_sw;

      if (wt_nw != 0.0)
	wt_sum += field[isig][ilat+1][ilon] * wt_nw;

      if (wt_ne != 0.0)
	wt_sum += field[isig][ilat+1][ilon+1] * wt_ne;

      if (wt_se != 0.0)
	wt_sum += field[isig][ilat][ilon+1] * wt_se;

      interp_data[isig] = wt_sum;
    }
  }
}

//////////////////
// interp3dLevel()
//
// Interpolate a 3 d field at a given eta level
//
// returns val on success, MISSING_DOUBLE on failure.

double Era5Data::interp3dLevel(int isig, int ilat, int ilon,
			      const char *name, fl32 ***field,
			      double wt_sw, double wt_nw,
			      double wt_ne, double wt_se) const
  
{
  static const string method_name = "Era5Data::interp3dLevel()";
  
  _heartbeatFunc("Entering Era5Data::interp3dLevel()");
  
  if (field == NULL)
  {
    cerr << "ERROR - " << method_name << endl;
    cerr << name << " array not loaded yet, operation invalid." << endl;

    return MISSING_DOUBLE;
  }

  if (wt_sw + wt_nw + wt_ne + wt_se == 0.0)
    return MISSING_DOUBLE;

  double interp_val = 0.0;
  if (wt_sw != 0.0)
    interp_val += field[isig][ilat][ilon] * wt_sw;

  if (wt_nw != 0.0)
    interp_val += field[isig][ilat+1][ilon] * wt_nw;

  if (wt_ne != 0.0)
    interp_val += field[isig][ilat+1][ilon+1] * wt_ne;

  if (wt_se != 0.0)
    interp_val += field[isig][ilat][ilon+1] * wt_se;

  return interp_val;
}

////////////////////////////////////////////////////////////////////////////
// closest3dField()
//
// Load up the eta field array from model point closest to the given point.
//
// returns vector of interpolated data.

void Era5Data::closest3dField(int ilat, int ilon,
			     const char *name, fl32 ***field,
			     double wt_sw, double wt_nw,
			     double wt_ne, double wt_se,
			     vector<double> &closest_data,
			     const vector<bool> *eta_needed) const
  
  
{
  static const string method_name = "Era5Data::closest3dField()";
  
  _heartbeatFunc("Entering Era5Data::closest3dField()");
  
  closest_data.clear();

  if (field == NULL) {
    cerr << "ERROR - " << method_name << endl;
    cerr << name << " array not loaded yet, operation invalid." << endl;
    return;
  }
  
  for (int isig = 0; isig < _nEta; isig++) {
    closest_data.push_back(MISSING_DOUBLE);
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

  for (int isig = 0; isig < _nEta; isig++) {
    if (!eta_needed || (*eta_needed)[isig]) {
      closest_data[isig] = field[isig][jlat][jlon];
    }
  }
  
}

//////////////////
// interp2dField()
//
// Load up interp_val_p with value interpolated for a given point.
//
// returns val on success, MISSING_DOUBLE on failure.

double Era5Data::interp2dField(int ilat, int ilon,
			      const char *name, fl32 **field,
			      double wt_sw, double wt_nw,
			      double wt_ne, double wt_se) const
  
{
  static const string method_name = "Era5Data::interp2dField()";
  
  _heartbeatFunc("Entering Era5Data::interp2dField()");
  
  if (field == NULL)
  {
    cerr << "ERROR - " << method_name << endl;
    cerr << name << " array not loaded yet, operation invalid." << endl;
    
    return MISSING_DOUBLE;
  }

  if (wt_sw + wt_nw + wt_ne + wt_se == 0.0) {
    return (MISSING_DOUBLE);
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


//helper function for _computeAvailMoist()
void Era5Data::_computeSingleAM(fl32**&am_soil_data,
			       fl32**moist_soil_data, int layer)
{
  _heartbeatFunc("Entering Era5Data::_computeSingleAM()");
  
  // Allocate space for the field

  am_soil_data = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));

  // Compute the field

  for (int ilat = 0; ilat < _nLat; ilat++)
  {
    for (int ilon = 0; ilon < _nLon; ilon++)
    {
      if ((_soil_type[ilat][ilon] >= _soilWiltMoist.size()) ||
	  (_soil_type[ilat][ilon] >= _soilSatMoist.size()))
      {
	cerr << "ERROR - soil_type index: " << _soil_type[ilat][ilon]
	     << " is >= the soil_wilt_moist(" << _soilWiltMoist.size()
	     << ") or soil_sat_moist(" << _soilSatMoist.size() << ")" << endl;
	return;
      } // endif

      fl32 soil_wilt = _soilWiltMoist[(int)(_soil_type[ilat][ilon])];
      fl32 soil_sat = _soilSatMoist[(int)(_soil_type[ilat][ilon])];
      am_soil_data[ilat][ilon] =
	(moist_soil_data[ilat][ilon] - soil_wilt) / (soil_sat - soil_wilt);
    }  // endfor - ilon
  } // endfor - ilat

}


// _computeAvailMoist()
//
// Compute _soil_am_{1:5}
//

void Era5Data::_computeAvailMoist() //  _soil_am_N;
{
  _heartbeatFunc("Entering Era5Data::_computeAvailMoist()");
  
  // If we already have the field, do nothing

  if (_soil_am_1 != 0 && _soil_am_2 != 0 && _soil_am_3 != 0 &&
      _soil_am_4 != 0 && _soil_am_5 != 0)
    return;
  
  // Load the needed fields

  _loadSoilInfo();
  
  if (_soil_m_1 == 0 || _soil_m_2 == 0 || _soil_m_3 == 0 ||
      _soil_m_4 == 0 || _soil_m_5 == 0)
  {
    cerr << "WARNING: soil moisture fields not available" << endl;
    cerr << "Cannot compute available moisture fields" << endl;
    return;
  }
  
  _loadSoilType();
  
  if (_soil_type == 0)
  {
    cerr << "WARNING: soil type not available" << endl;
    cerr << "Cannot compute available moisture fields" << endl;
    return;
  }
  
  // Open the SOILPARM.TBL file 

  ifstream spt(_params.soilparm_path);

  if (spt.is_open())
  {
    _loadSoilParamFile(spt);
  }
  else
  {
    cerr << "Warning SOILPARM.TBL file not found at: "
         << _params.soilparm_path << endl
         << "Using default soilparams\n";
    _loadDefaultSoilParams();
  }
  
  // Compute the fields.  The space for each field is allocated
  // inside of _computeSingleAM().

  _computeSingleAM(_soil_am_1, _soil_m_1, 1);
  _computeSingleAM(_soil_am_2, _soil_m_2, 2);
  _computeSingleAM(_soil_am_3, _soil_m_3, 3);
  _computeSingleAM(_soil_am_4, _soil_m_4, 4);
  _computeSingleAM(_soil_am_5, _soil_m_5, 5);
}

void Era5Data::_loadDefaultSoilParams()
{
  _heartbeatFunc("Entering Era5Data::_loadDefaultSoilParams()");
  
  _soilWiltMoist.clear();
  _soilSatMoist.clear();

  _soilWiltMoist.push_back(.01);
  _soilWiltMoist.push_back(.028);
  _soilWiltMoist.push_back(.047);
  _soilWiltMoist.push_back(.084);
  _soilWiltMoist.push_back(.084);
  _soilWiltMoist.push_back(.066);
  _soilWiltMoist.push_back(.067);
  _soilWiltMoist.push_back(.12);
  _soilWiltMoist.push_back(.103);
  _soilWiltMoist.push_back(.1);
  _soilWiltMoist.push_back(.126);
  _soilWiltMoist.push_back(.138);
  _soilWiltMoist.push_back(.066);
  _soilWiltMoist.push_back(0);
  _soilWiltMoist.push_back(.006);
  _soilWiltMoist.push_back(.028);
  _soilWiltMoist.push_back(.03);
  _soilWiltMoist.push_back(.006);
  _soilWiltMoist.push_back(.01);

  _soilSatMoist.push_back(.339); 	 
  _soilSatMoist.push_back(.421); 	 
  _soilSatMoist.push_back(.434); 	 
  _soilSatMoist.push_back(.476); 	 
  _soilSatMoist.push_back(.476); 	 
  _soilSatMoist.push_back(.439); 	 
  _soilSatMoist.push_back(.404); 	 
  _soilSatMoist.push_back(.464); 	 
  _soilSatMoist.push_back(.465); 	 
  _soilSatMoist.push_back(.406); 	 
  _soilSatMoist.push_back(.468); 	 
  _soilSatMoist.push_back(.439); 	 
  _soilSatMoist.push_back(1); 	 
  _soilSatMoist.push_back(.2); 	 
  _soilSatMoist.push_back(.421); 	 
  _soilSatMoist.push_back(.468); 	 
  _soilSatMoist.push_back(.2); 	 
  _soilSatMoist.push_back(.339); 	 

}

void Era5Data::_loadSoilParamFile(ifstream &spt)
{
  _heartbeatFunc("Entering Era5Data::_loadSoilParamFile()");
  
  const int MAXLINESIZE = 512;
  const int MAXHEADSIZE = 32;
  
  char line[MAXLINESIZE];

  spt.getline(line,MAXLINESIZE);
  if(strcmp(line,"Soil Parameters") != 0)
  {
    cerr << "ERROR! - " << _params.soilparm_path 
	 << " first line: '" << line << "'" << endl;
    return;
  }
  
  //throwout 'STAS' line.
  spt.getline(line,MAXLINESIZE);

  char fields[12][MAXHEADSIZE];
  //get headers
  spt >> fields[0] >> fields[1] >> fields[2] >> fields[3] 
      >> fields[4] >> fields[5] >> fields[6] >> fields[7] 
      >> fields[8] >> fields[9] >> fields[10] >> fields[11];

  if (strcmp(fields[4],"MAXSMC") != 0)
    {
      cerr << "ERROR! - header #7 (" << fields[6]
	   << ") in " << _params.soilparm_path
	   << " should be 'MAXSMC'.\n";
    }

  if (strcmp(fields[9],"WLTSMC") != 0)
    {
      cerr << "ERROR! - header #10 (" << fields[9] 
	   << ") in " << _params.soilparm_path
	   << " should be 'WLTSMC'.\n";
    }

  _soilWiltMoist.clear();
  _soilSatMoist.clear();

  //get # of lines from first header
  int numLines;
  char *commaplace;
  commaplace = strchr(fields[0],',');
  *commaplace= '\0';
  numLines = atoi(fields[0]);

  for(int i=0; i<numLines; i++)
    {
      std::string line;
      std::getline(spt, line, '\n');
	if (!line.empty())
	  {
	    std::istringstream buffer(line);
	    buffer >> fields[0] >> fields[1] >> fields[2] >> fields[3]
		   >> fields[4] >> fields[5] >> fields[6] >> fields[7]
		   >> fields[8] >> fields[9] >> fields[10] >> fields[11];
                                         
	    _soilWiltMoist.push_back(atof(fields[9]));
	    _soilSatMoist.push_back(atof(fields[4])); 
	  }
    }
}



/////////////////////////////
// _computeTempC()
//
// ComputeTemp in C from Temp in K
//

void Era5Data::_computeTempC()
{
  _heartbeatFunc("Entering Era5Data::_computeTempC()");
  
  // If we already have the field, no need to recompute it

  if (_tc != 0)
    return;
  
  // Load needed fields

  _computeTemp();
  
  if (_tk == NULL)
  {
    cerr << "WARNING: temp K field not available" << endl;
    cerr << "Cannot compute tmep C" << endl;
    return;
  }
  
  // Allocate space for this field

  _tc = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));
  
  // Compute the field

  for (int isig = 0; isig < _nEta; isig++)
  {
    for (int ilat = 0; ilat < _nLat; ilat++)
    {
      for (int ilon = 0; ilon < _nLon; ilon++)
      {
	_tc[isig][ilat][ilon] = _tk[isig][ilat][ilon] - 273.15;
      } // endfor - ilon
    } // endfor - ilat
  } // endfor - isig

}

// For which method are these defined? 
#define RR 287.04
#define GG 9.80665


///////////////////////
// _computeHeightField()
//
// Compute height (AGL)

void Era5Data::_computeHeightField() 
{
  _heartbeatFunc("Entering Era5Data::_computeHeightField()");
  
  // If we've already computed it, don't do it again

  if (_zz != 0)
    return;
  
  // Load the needed fields

  _loadPertGeoC();
  
  if (_ph_C == 0)
  {
    cerr << "WARNING: pert geo C field not available" << endl;
    cerr << "Cannot compute height" << endl;
    return;
  }
  
  _loadBSPGC();
  
  if (_phb_C == 0)
  {
    cerr << "WARNING: base state geo C field not available" << endl;
    cerr << "Cannot compute height" << endl;
    return;
  }
  
  _loadTerrainHeight();
  
  if (_terrain == 0)
  {
    cerr << "WARNING: terrain field not available" << endl;
    cerr << "Cannot compute height" << endl;
    return;
  }
  
  _loadHalfEta();
  
  if (_halfEta == 0)
  {
    cerr << "WARNING: half eta field not available" << endl;
    cerr << "Cannot compute height" << endl;
    return;
  }
  
  _loadZnw();
  
  if (_znw == 0)
  {
    cerr << "WARNING: znw field not available" << endl;
    cerr << "Cannot compute height" << endl;
    return;
  }
  
  // Define some constants
  
  fl32 c1 = 1.0;
  fl32 g0 = 9.80665;
  fl32 Rd = 287;
  fl32 scaleHt = Rd * 256.0/g0;
  
  // define and allocate memory for local variables

  fl32 *znWeight = (fl32 *) ucalloc(_nEta, sizeof(fl32));
  fl32 ***geoExp = (fl32 ***) umalloc3(_nEta +1, _nLat, _nLon, sizeof(fl32));
  
  // Allocate for member variable

  _zz = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));
  
  // Calculate znWeight

  float *znu = _halfEta;

  for (int i = 0; i < _nEta; i++)
    znWeight[i] = (znu[i] - _znw[i])/(_znw[i+1] - _znw[i]);

  // Calculate geoExp

  for (int isig = 0; isig < _nEta + 1; isig++)
  {
    for (int ilat = 0; ilat < _nLat; ilat++)
    {
      for (int ilon = 0; ilon < _nLon; ilon++)
      {
	geoExp[isig][ilat][ilon] =
	  exp(-(_phb_C[isig][ilat][ilon] +
		_ph_C[isig][ilat][ilon])/(g0*scaleHt));
      } // endfor - ilon
    } // endfor - ilat
  } // endfor - isig

  // Calculate height above ground at half Eta levels 

  for (int isig = 0; isig < _nEta; isig++)
  {
    for (int ilat = 0; ilat < _nLat; ilat++)
    {
      for (int ilon = 0; ilon < _nLon; ilon++)
      {
        double zzz =  znWeight[isig] * geoExp[isig + 1][ilat][ilon] + 
          (c1 - znWeight[isig]) * geoExp[isig][ilat][ilon];
        if (zzz <= 0)
          zzz = 1.0e-6;

        double zz2 = -scaleHt * log(zzz);

        _zz[isig][ilat][ilon] = zz2 - _terrain[ilat][ilon];
      } // endfor - ilon
    } // endfor - ilat
  } // endfor - isig
  
  // Clean up

  if (znWeight != 0)
    ufree((void*)znWeight);

  if (geoExp != 0)
    ufree3((void***)geoExp);
}

///////////////////////
// _computeRh()
//
// Load relative humidity (%) field from mixing ratio,
// pressure and temperature
//

void Era5Data::_computeRh()
{
  _heartbeatFunc("Entering Era5Data::_computeRh()");
  
  // If we already have the field, don't do anything

  if (_rh != 0)
    return;
  
  // Load the needed data

  _loadWaterMixingRatio();
  
  if (_qq == 0)
  {
    cerr << "WARNING: mixing ratio not available in netCDF file" << endl;
    cerr << "Cannot calculate RH" << endl;
    
    return;
  }
  
  _computePressure();
  
  if (_pres == 0)
  {
    cerr << "WARNING: pressure not available in netCDF file" << endl;
    cerr << "Cannot calculate RH" << endl;
    
    return;
  }
  
  _computeTempC();
  
  if (_tc == 0)
  {
    cerr << "WARNING: temperature not available in netCDF file" << endl;
    cerr << "Cannot calculate RH" << endl;
    
    return;
  }
  
  // Allocate space for the new field

  _rh = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Calculate RH

  for (int isig = 0; isig < _nEta; isig++)
  {
    for (int ilat = 0; ilat < _nLat; ilat++)
    {
      for (int ilon = 0; ilon < _nLon; ilon++)
      {
	_rh[isig][ilat][ilon] =
	  PHYrhmr(_qq[isig][ilat][ilon] * 1000.0,
		  _pres[isig][ilat][ilon],
		  _tc[isig][ilat][ilon]);
      } // endfor - ilon
    } // endfor - ilat
  } // endfor -- isig

}

///////////////////////
// _computeSpecHumidity()
//
// Compute specific humidity (%) field from 
// water vapor mixing ratio (MR/(1+MR))
//

void Era5Data::_computeSpecHumidity()
{
  _heartbeatFunc("Entering Era5Data::_computeSpecHumidity()");
  
  // If we've already computed it, don't do anything

  if (_spec_h != 0)
    return;
  
  // Load the needed fields

  _loadWaterMixingRatio();
  
  if (_qq == 0)
  {
    cerr << "WARNING: mixing ratio not available" << endl;
    cerr << "Cannot calculate specific humidity" << endl;
    
    return;
  }

  // Allocate space for the new field

  _spec_h = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Calculate the field

  for (int isig = 0; isig < _nEta; isig++)
  {
    for (int ilat = 0; ilat < _nLat; ilat++)
    {
      for (int ilon = 0; ilon < _nLon; ilon++)
      {
	_spec_h[isig][ilat][ilon] = 
	  (_qq[isig][ilat][ilon] / (_qq[isig][ilat][ilon] + 1.));
      } // endfor - ilon
    } // endfor - ilat
  } // endfor - isig

}
   
///////////////////////
// _computeSpecHumidity2m()
//
// Compute specific humidity (%) field from 
// water vapor mixing ratio (MR/(1+MR))
//

void Era5Data::_computeSpecHumidity2m()
{
  _heartbeatFunc("Entering Era5Data::_computeSpecHumidity2m()");
  
  // If we already have the field, don't do anything

  if (_spec_h2m != 0)
    return;
  
  // Load the needed fields

  _loadQ2();
  
  if (_q2 == 0)
  {
    cerr << "WARNING: q2 not available" << endl;
    cerr << "Cannot compute specific humidity 2m field" << endl;
    return;
  }

  // Allocate space for the field

  _spec_h2m = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));

  // Compute the field

  for (int ilat = 0; ilat < _nLat; ilat++)
  {
    for (int ilon = 0; ilon < _nLon; ilon++)
    {
      _spec_h2m[ilat][ilon] = 
	(_q2[ilat][ilon] / (_q2[ilat][ilon] + 1.));
    } // endfor - ilon
  } // endfor - ilat

}
   
///////////////////////
// _computeDewPt()
//
// Load dewpt (C) field from temperature and RH
//

void Era5Data::_computeDewPt()
{
  _heartbeatFunc("Entering Era5Data::_computeDewPt()");
  
  // Don't need to compute if we've already got it

  if (_dewpt != 0)
    return;
  
  // Load the needed fields

  _computeTempC();
  
  if (_tc == 0)
  {
    cerr << "WARNING: temp C not available" << endl;
    cerr << "Cannot compute dew point" << endl;
    return;
  }
  
  _computeRh();
  
  if (_rh == 0)
  {
    cerr << "WARNING: temp C not available" << endl;
    cerr << "Cannot compute dew point" << endl;
    return;
  }
  
  // Allocate space for this field

  _dewpt = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));
  
  // Compute the field

  for (int isig = 0; isig < _nEta; isig++)
  {
    for (int ilat = 0; ilat < _nLat; ilat++)
    {
      for (int ilon = 0; ilon < _nLon; ilon++)
      {
	_dewpt[isig][ilat][ilon] =
	  PHYrhdp(_tc[isig][ilat][ilon],
		  _rh[isig][ilat][ilon]);
      } // endfor - ilon
    } // endfor - ilat
  } // endfor - isig

}
   
///////////////////////
// _computeWspdDirnField()
//
// Load wind speed and dirn from U and V components.
// Also translate the U and V into knots if required
//

void Era5Data::_computeWspdDirnField()
{
  _heartbeatFunc("Entering Era5Data::_computeWspdDirnField()");
  
  // If we've already computed it, don't do it again

  if (_wspd != 0 && _wdir != 0)
    return;
  
  // Load the needed datasets

  _loadUWind();
  
  if (_uu == 0)
  {
    cerr << "WARNING: U wind field not available" << endl;
    cerr << "Cannot calculate wind speed/direction fields" << endl;
    
    return;
  }

  _loadVWind();
  
  if (_vv == 0)
  {
    cerr << "WARNING: V wind field not available" << endl;
    cerr << "Cannot calculate wind speed/direction fields" << endl;
    
    return;
  }

  // compute U and V relative to TN

  _computeUVRelToTN();

  if (_uuTn == 0)
  {
    cerr << "WARNING: UU true north not available" << endl;
    cerr << "Cannot compute wind speed/dir" << endl;
    return;
  }
  
  if (_vvTn == 0)
  {
    cerr << "WARNING: VV true north not available" << endl;
    cerr << "Cannot compute wind speed/dir" << endl;
    return;
  }
  
  // Allocate space for the fields

  _wspd = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));
  _wdir = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Compute the fields
  
  for (int isig = 0; isig < _nEta; isig++)
  {
    for (int ilat = 0; ilat < _nLat; ilat++)
    {
      for (int ilon = 0; ilon < _nLon; ilon++)
      {
	double u = _uuTn[isig][ilat][ilon];
	double v = _vvTn[isig][ilat][ilon];
	_wspd[isig][ilat][ilon] = sqrt(u * u + v * v);
	if (u == 0.0 && v == 0.0)
	  _wdir[isig][ilat][ilon] = 0.0;
	else
	  _wdir[isig][ilat][ilon] = atan2(-u, -v) / DEG_TO_RAD;
      } // endfor - ilon
    } // endfor - ilat
  } // endfor - isig

}
   
///////////////////////
// _computeQ_GField()
//
// Load mixing ratio in g/kg from kg/kg
//

void Era5Data::_computeQ_GField()
{
  _heartbeatFunc("Entering Era5Data::_computeQ_GField()");
  
  // If we already have the field, do nothing

  if (_q_g != 0)
    return;
  
  // Load the needed fields

  _loadWaterMixingRatio();
  
  if (_qq == 0)
  {
    cerr << "WARNING: mixing ratio field not available" << endl;
    cerr << "Cannot calculate g/kg mixing ratio field" << endl;
    return;
  }

  // Allocate space for the field

  _q_g = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));
    
  // Calculate the field

  for (int isig = 0; isig < _nEta; isig++)
  {
    for (int ilat = 0; ilat < _nLat; ilat++)
    {
      for (int ilon = 0; ilon < _nLon; ilon++)
      {
	_q_g[isig][ilat][ilon] = _qq[isig][ilat][ilon] * 1000.0;
      } // endfor - ilon
    } // endfor - ilat
  } // endfor - isig

}

///////////////////////
// _loadQ2_GField()
//
// Load 2meter mixing ratio in g/kg from kg/kg
//

void Era5Data::_loadQ2_GField()
{
  _heartbeatFunc("Entering Era5Data::_loadQ2_GField()");
  
  // If we already have the field, do nothing

  if (_q2_g != 0)
    return;
  
  // Load the needed fields

  _loadQ2();
  
  if (_q2 == 0)
  {
    cerr << "WARNING: 2 meter mixing ratio not available" << endl;
    cerr << "Cannot compute 2 meter g/kg mixing ratio" << endl;
    
    return;
  }

  // Allocate space for the field

  _q2_g = (fl32 **) umalloc2(_nLat, _nLon, sizeof(fl32));
  
  // Compute the field

  for (int ilat = 0; ilat < _nLat; ilat++)
  {
    for (int ilon = 0; ilon < _nLon; ilon++)
    {
      _q2_g[ilat][ilon] = _q2[ilat][ilon] * 1000.0;
    } // endfor - ilon
  } // endfor - ilat

}
  
///////////////////////
// _computeTHETAField()
//
// Calculate potential temperature (theta)
//

void Era5Data::_computeTHETAField()
{
  _heartbeatFunc("Entering Era5Data::_computeTHETAField()");
  
  // If we already have the field, do nothing

  if (_theta != 0)
    return;
  
  // Load the needed fields

  _loadTemp();
  
  if (_ppt == 0)
  {
    cerr << "WARNING: Ppt field not available" << endl;
    cerr << "Cannot calculate theta" << endl;
    return;
  }

  // Allocate space for the field

  _theta = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Compute the field

  for (int isig = 0; isig < _nEta; isig++)
  {
    for (int ilat = 0; ilat < _nLat; ilat++)
    {
      for (int ilon = 0; ilon < _nLon; ilon++)
      {
	_theta[isig][ilat][ilon] = _ppt[isig][ilat][ilon] + 300;
      } // endfor - ilon
    } // endfor - ilat
  } // endfor - isig

}

/////////////////////////////////////////////
// Compute rotation of grid relative to TN
//

void Era5Data::_computeGridRotation()
{
  _heartbeatFunc("Entering Era5Data::_computeGridRotation()");
  
  // If we've already got it, do nothing

  if (_gridRotation != 0)
    return;
  
  // Load needed fields

  _loadLat();
  
  if (_lat == 0)
  {
    cerr << "WARNING: lat field not available" << endl;
    cerr << "Cannot compute grid rotation" << endl;
    return;
  }

  _loadLon();
  
  if (_lon == 0)
  {
    cerr << "WARNING: lat field not available" << endl;
    cerr << "Cannot compute grid rotation" << endl;
    return;
  }

  // Allocate space for the field

  _gridRotation = (fl32 **) ucalloc2(_nLat, _nLon, sizeof(fl32));

  // Compute the field based for the MERCATOR projection

  if (_projType == MERCATOR)
  {
    // mercator is aligned with TN.

    for (int ilat = 0; ilat < _nLat - 1; ilat++)
    {
      for (int ilon = 0; ilon < _nLon; ilon++)
      {
        _gridRotation[ilat][ilon] = 0.0;
      } // endfor - ilon
    } // endfor - ilat
    return;
  }

  // compute azimuth from this point to the one above
  // for all rows except top

  for (int ilat = 0; ilat < _nLat - 1; ilat++)
  {
    for (int ilon = 0; ilon < _nLon; ilon++)
    {
      double range, theta;
      PJGLatLon2RTheta(_lat[ilat][ilon], _lon[ilat][ilon],
                       _lat[ilat+1][ilon], _lon[ilat+1][ilon],
                       &range, &theta);
      _gridRotation[ilat][ilon] = (fl32) theta;
    } // endfor - ilon
  } // endfor - ilat

  // for top row, use value from the second-to-top row

  for (int ilon = 0; ilon < _nLon; ilon++)
    _gridRotation[_nLat - 1][ilon] = _gridRotation[_nLat - 2][ilon];

}

/////////////////////////////////////////////
// Compute the wind components
// relative to TN instead of the grid

void Era5Data::_computeUVRelToTN()
{
  _heartbeatFunc("Entering Era5Data::_computeUVRelToTN()");
  
  // If we already have the fields, do nothing

  if (_uuTn != 0 && _vvTn != 0)
    return;
  
  // Load the needed datasets

  _loadUWind();
  
  if (_uu == 0)
  {
    cerr << "WARNING: U wind field not available" << endl;
    cerr << "Cannot calculate U/V relative to true north" << endl;
    return;
  }

  _loadVWind();
  
  if (_vv == 0)
  {
    cerr << "WARNING: V wind field not available" << endl;
    cerr << "Cannot calculate U/V relative to true north" << endl;
    return;
  }
  
  // Load the optional fields

  _computeGridRotation();
  
  // Allocate space for the new fields

  _uuTn = (fl32 ***) ucalloc3(_nEta, _nLat, _nLon, sizeof(fl32));
  _vvTn = (fl32 ***) ucalloc3(_nEta, _nLat, _nLon, sizeof(fl32));
  
  // Calculate the fields

  if (_projType == MERCATOR || _gridRotation == 0)
  {
    // mercator is aligned with TN.
    for (int ilat = 0; ilat < _nLat - 1; ilat++)
    {
      for (int ilon = 0; ilon < _nLon; ilon++)
      {
        for (int isig = 0; isig < _nEta; isig++)
	{
          _uuTn[isig][ilat][ilon] = _uu[isig][ilat][ilon];
          _vvTn[isig][ilat][ilon] = _vv[isig][ilat][ilon];
        }
      }
    }
    return;
  }

  for (int ilat = 0; ilat < _nLat; ilat++)
  {
    for (int ilon = 0; ilon < _nLon; ilon++)
    {
      double theta = _gridRotation[ilat][ilon] * DEG_TO_RAD;
      double sinTheta = sin(theta);
      double cosTheta = cos(theta);
      for (int isig = 0; isig < _nEta; isig++)
      {
        double u = _uu[isig][ilat][ilon];
        double v = _vv[isig][ilat][ilon];
        _uuTn[isig][ilat][ilon] = u * cosTheta + v * sinTheta;
        _vvTn[isig][ilat][ilon] = v * cosTheta - u * sinTheta;
      }
    }
  }

}

/////////////////////////////////////////////
// Compute the wind components
// relative to output grid
//
// rotation of output grid relative to TN is input

void Era5Data::computeUVOutput(const MdvxProj &proj)
{
  _heartbeatFunc("Entering Era5Data::_computeUVOutput()");
  
  // If the fields exist, we don't need to compute them again

  if (_uuOut != 0 && _vvOut != 0)
    return;
  
  // Load the needed fields

  _computeUVRelToTN();

  if (_uuTn == 0)
  {
    cerr << "WARNING: UU true north not available" << endl;
    cerr << "Cannot compute uuOut and vvOut" << endl;
    return;
  }
  
  if (_vvTn == 0)
  {
    cerr << "WARNING: VV true north not available" << endl;
    cerr << "Cannot compute uuOut and vvOut" << endl;
    return;
  }
  
  _loadLat();
  
  if (_lat == 0)
  {
    cerr << "WARNING: latitude not available" << endl;
    cerr << "Cannot compute uuOut and vvOut" << endl;
    return;
  }
  
  _loadLon();
  
  if (_lon == 0)
  {
    cerr << "WARNING: longitude not available" << endl;
    cerr << "Cannot compute uuOut and vvOut" << endl;
    return;
  }
  
  // Calculate the grid rotations

  fl32 **output_grid_rotation =
    (fl32 **)umalloc2(_nLat, _nLon, sizeof(fl32));
  
  for (int ilat = 0; ilat < _nLat; ilat++)
  {
    for (int ilon = 0; ilon < _nLon; ilon++)
    {
      // get lat/lon of point 0
      
      double lat0 = _lat[ilat][ilon];
      double lon0 = _lon[ilat][ilon];
      lon0 = proj.conditionLon2Origin(lon0);
      
      // compute x0, y0
      
      double x0, y0;
      proj.latlon2xy(lat0, lon0, x0, y0);
      
      // compute x1, y1 10 km north (this is only done for an LC projection,
      // so we know X and Y are in km)
      
      double x1 = x0;
      double y1 = y0 + 10.0;

      // compute lat1, lon1
      
      double lat1, lon1;
      proj.xy2latlon(x1, y1, lat1, lon1);

      // compute rotation theta

      double range, theta;
      PJGLatLon2RTheta(lat0, lon0, lat1, lon1,
                       &range, &theta);

      output_grid_rotation[ilat][ilon] = (fl32) theta;
      
    } // endfor - ilon
  } // endfor - ilat

  // Allocate space for the output grids

  _uuOut = (fl32 ***) ucalloc3(_nEta, _nLat, _nLon, sizeof(fl32));
  _vvOut = (fl32 ***) ucalloc3(_nEta, _nLat, _nLon, sizeof(fl32));
  
  // Compute the output U and V

  for (int ilat = 0; ilat < _nLat; ilat++)
  {
    for (int ilon = 0; ilon < _nLon; ilon++)
    {
      double theta = output_grid_rotation[ilat][ilon] * DEG_TO_RAD * -1.0;
      double sinTheta = sin(theta);
      double cosTheta = cos(theta);

      for (int isig = 0; isig < _nEta; isig++)
      {
        double u = _uuTn[isig][ilat][ilon];
        double v = _vvTn[isig][ilat][ilon];
        _uuOut[isig][ilat][ilon] = u * cosTheta + v * sinTheta;
        _vvOut[isig][ilat][ilon] = v * cosTheta - u * sinTheta;
      } // endfor - isig
    } // endfor - ilon
  } // endfor - ilat

  // Clean up local memory

  ufree2((void **)output_grid_rotation);
}



/////////////////////////
// _computeTotalWaterPathFields()
// Calculates total water path and rain water path (snow/rain) 
//

void Era5Data::_computeTotalWaterPathFields()
{
  _heartbeatFunc("Entering Era5Data::_computeTotalWaterPathFields()");
  
  // If we already have the fields, do nothing

  if (_twp != 0 && _rwp != 0 && _vil != 0)
    return;
  
  // Load the needed fields

  _computePressure();
  
  if (_pres == 0)
  {
    cerr << "WARNING: pressure not available" << endl;
    cerr << "Cannot calculate total water path" << endl;
    return;
  }

  _computeTemp();
  
  if (_tk == 0)
  {
    cerr << "WARNING: temperature not available" << endl;
    cerr << "Cannot calculate total water path" << endl;
    return;
  }

  _loadIceMixingRatio();
  
  if (_ice == 0)
  {
    cerr << "WARNING: ice mixing ratio not available" << endl;
    cerr << "Cannot calculate total water path" << endl;
    return;
  }

  _loadCloudMixingRatio();
  
  if (_clw == 0)
  {
    cerr << "WARNING: cloud mixing ratio not available" << endl;
    cerr << "Cannot calculate total water path" << endl;
    return;
  }

  _loadRainMixingRatio();
  
  if (_rnw == 0)
  {
    cerr << "WARNING: rain mixing ratio not available" << endl;
    cerr << "Cannot calculate total water path" << endl;
    return;
  }

  _loadSnowMixingRatio();
  
  if (_snow == 0)
  {
    cerr << "WARNING: snow mixing ratio not available" << endl;
    cerr << "Cannot calculate total water path" << endl;
    return;
  }

  _computeHeightField();
  
  if (_zz == 0)
  {
    cerr << "WARNING: height not available" << endl;
    cerr << "Cannot calculate total water path" << endl;
    return;
  }

  // Allocate local arrays

  // NOTE: I didn't look carefully, but it looks like all of the calculations
  // are done using only the information for the current point.  Can we combine
  // the loops below into a single loop and get rid of these temporary arrays?

  fl32 ***rhoa   = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));
  fl32 ***twater = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));
  fl32 ***rwater = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));
  fl32 ***dz     = (fl32 ***) umalloc3(_nEta, _nLat, _nLon, sizeof(fl32));

  // Allocate space for the calculated fields

  _twp = (fl32 **) ucalloc2( _nLat, _nLon, sizeof(fl32));
  _rwp = (fl32 **) ucalloc2( _nLat, _nLon, sizeof(fl32));
  _vil = (fl32 **) ucalloc2( _nLat, _nLon, sizeof(fl32));

  // Calculate the fields

  for (int ilat = 0; ilat < _nLat; ilat++)
  {
    for (int ilon = 0; ilon < _nLon; ilon++)
    {
      for (int isig = 0; isig < _nEta; isig++)
      {
	rhoa[isig][ilat][ilon] =
	  _pres[isig][ilat][ilon] / (_tk[isig][ilat][ilon] * RR);
      } // endfor - isig
    } // endfor - ilon
  } // endfor - ilat
	
  for (int ilat = 0; ilat < _nLat; ilat++)
  {
    for (int ilon = 0; ilon < _nLon; ilon++)
    {
      for (int isig = 0; isig < _nEta; isig++) 
      {
	// convert/combine ice, snow, and rain from mixing ratio to
	// total water density in air

	twater[isig][ilat][ilon] =
	  (_ice[isig][ilat][ilon] + _clw[isig][ilat][ilon] +
	   _rnw[isig][ilat][ilon] + _snow[isig][ilat][ilon]) 
	  * rhoa[isig][ilat][ilon] * 1000.0;

	rwater[isig][ilat][ilon] =
	  (_rnw[isig][ilat][ilon] + _snow[isig][ilat][ilon]) 
	  * rhoa[isig][ilat][ilon] * 1000.0;

	// compute dz at each level except the bottom
	
 	if (isig > 0)
	{
 	  dz[isig][ilat][ilon] =
	    _zz[isig][ilat][ilon] - _zz[isig-1][ilat][ilon];
 	}
	else
	{
 	  dz[isig][ilat][ilon] = _zz[0][ilat][ilon]; // (should be) zero height
	}
      }
    }
  }

  for (int isig = 0; isig < _nEta; isig++)
  {
    for (int ilon = 0; ilon < _nLon; ilon++)
    {
      for (int ilat = 0; ilat < _nLat; ilat++)
      {
	_twp[ilat][ilon] += (twater[isig][ilat][ilon] * dz[isig][ilat][ilon]);
	_rwp[ilat][ilon] += (rwater[isig][ilat][ilon] * dz[isig][ilat][ilon]);
      }
    }
  }

  for (int ilon = 0; ilon < _nLon; ilon++)
  {
    for (int ilat = 0; ilat < _nLat; ilat++)
    {
      if ( _rwp[ilat][ilon] < 0.4)
	_vil[ilat][ilon] = 40.6591*_rwp[ilat][ilon]+2;
      else
	_vil[ilat][ilon] = 53.0+38.9*log(_rwp[ilat][ilon])+1;
    }
  }
 
  // Clean up memory

  ufree3((void ***) rhoa);
  ufree3((void ***) twater);
  ufree3((void ***) rwater);
  ufree3((void ***) dz);
  
}


/////////////////////////
// _scale3DField
//

void Era5Data::_scale3DField(fl32*** field, double factor, int nz)
{
  _heartbeatFunc("Entering Era5Data::_scale3DField()");
  

  for (int isig = 0; isig < nz; isig++)
    for (int ilon = 0; ilon < _nLon; ilon++)
      for (int ilat = 0; ilat < _nLat; ilat++)
	if ((fl32)MISSING_DOUBLE != field[isig][ilat][ilon])
	  field[isig][ilat][ilon] *= factor;
  
}


/////////////////////////
// _scale2DField
//

void Era5Data::_scale2DField(fl32** field, double factor)
{
  _heartbeatFunc("Entering Era5Data::_scale2DField()");
  

  for (int ilon = 0; ilon < _nLon; ilon++)
    for (int ilat = 0; ilat < _nLat; ilat++)
      if ((fl32)MISSING_DOUBLE != field[ilat][ilon])
	field[ilat][ilon] *= factor;

}


///////////////////////////////
// print data in file

void Era5Data::printFile()

{

  cout << "ndims: " << _ncf->num_dims() << endl;
  cout << "nvars: " << _ncf->num_vars() << endl;
  cout << "ngatts: " << _ncf->num_atts() << endl;
  Nc3Dim *unlimd = _ncf->rec_dim();
  cout << "unlimdimid: " << unlimd->size() << endl;
  
  // dimensions

  TaArray<Nc3Dim *> dims_;
  Nc3Dim **dims = dims_.alloc(_ncf->num_dims());
  for (int idim = 0; idim < _ncf->num_dims(); idim++) {
    dims[idim] = _ncf->get_dim(idim);

    cout << endl;
    cout << "Dim #: " << idim << endl;
    cout << "  Name: " << dims[idim]->name() << endl;
    cout << "  Length: " << dims[idim]->size() << endl;
    cout << "  Is valid: " << dims[idim]->is_valid() << endl;
    cout << "  Is unlimited: " << dims[idim]->is_unlimited() << endl;
    
  } // idim
  
  cout << endl;

  // global attributes

  cout << "Global attributes:" << endl;

  for (int iatt = 0; iatt < _ncf->num_atts(); iatt++) {
    cout << "  Att num: " << iatt << endl;
    Nc3Att *att = _ncf->get_att(iatt);
    _printAtt(att);
    delete att;
  }

  // loop through variables

  TaArray<Nc3Var *> vars_;
  Nc3Var **vars = vars_.alloc(_ncf->num_vars());
  for (int ivar = 0; ivar < _ncf->num_vars(); ivar++) {

    vars[ivar] = _ncf->get_var(ivar);
    cout << endl;
    cout << "Var #: " << ivar << endl;
    cout << "  Name: " << vars[ivar]->name() << endl;
    cout << "  Is valid: " << vars[ivar]->is_valid() << endl;
    cout << "  N dims: " << vars[ivar]->num_dims();
    TaArray<Nc3Dim *> vdims_;
    Nc3Dim **vdims = vdims_.alloc(vars[ivar]->num_dims());
    if (vars[ivar]->num_dims() > 0) {
      cout << ": (";
      for (int ii = 0; ii < vars[ivar]->num_dims(); ii++) {
	vdims[ii] = vars[ivar]->get_dim(ii);
	cout << " " << vdims[ii]->name();
	if (ii != vars[ivar]->num_dims() - 1) {
	  cout << ", ";
	}
      }
      cout << " )";
    }
    cout << endl;
    cout << "  N atts: " << vars[ivar]->num_atts() << endl;
    
    for (int iatt = 0; iatt < vars[ivar]->num_atts(); iatt++) {

      cout << "  Att num: " << iatt << endl;
      Nc3Att *att = vars[ivar]->get_att(iatt);
      _printAtt(att);
      delete att;

    } // iatt

    cout << endl;
    _printVarVals(vars[ivar]);
    
  } // ivar
  
}

/////////////////////
// print an attribute

void Era5Data::_printAtt(Nc3Att *att)

{

  cout << "    Name: " << att->name() << endl;
  cout << "    Num vals: " << att->num_vals() << endl;
  cout << "    Type: ";
  
  Nc3Values *values = att->values();

  switch(att->type()) {
    
    case nc3Byte: {
      cout << "BYTE: ";
      unsigned char *vals = (unsigned char *) values->base();
      for (long ii = 0; ii < att->num_vals(); ii++) {
        cout << " " << vals[ii];
      }
      break;
    }
  
    case nc3Char: {
      cout << "CHAR: ";
      TaArray<char> vals_;
      char *vals = vals_.alloc(att->num_vals() + 1);
      memset(vals, 0, att->num_vals() + 1);
      memcpy(vals, values->base(), att->num_vals());
      cout << vals;
      break;
    }
  
    case nc3Short: {
      cout << "SHORT: ";
      short *vals = (short *) values->base();
      for (long ii = 0; ii < att->num_vals(); ii++) {
        cout << " " << vals[ii];
      }
      break;
    }
  
    case nc3Int: {
      cout << "INT: ";
      int *vals = (int *) values->base();
      for (long ii = 0; ii < att->num_vals(); ii++) {
        cout << " " << vals[ii];
      }
      break;
    }
  
    case nc3Float: {
      cout << "FLOAT: ";
      float *vals = (float *) values->base();
      for (long ii = 0; ii < att->num_vals(); ii++) {
        cout << " " << vals[ii];
      }
      break;
    }
  
    case nc3Double: {
      cout << "DOUBLE: ";
      double *vals = (double *) values->base();
      for (long ii = 0; ii < att->num_vals(); ii++) {
        cout << " " << vals[ii];
      }
      break;
    }
  
    case nc3NoType:
    default: {
      cout << "No type: ";
      break;
    }
      
  }
  
  cout << endl;

  delete values;

}

    
void Era5Data::_printVarVals(Nc3Var *var)

{

  int nprint = var->num_vals();
  if (nprint > 100) {
    nprint = 100;
  }

  Nc3Values *values = var->values();

  cout << "  Variable vals:";
  
  switch(var->type()) {
    
    case nc3Byte: {
      cout << "(byte)";
      unsigned char *vals = (unsigned char *) values->base();
      for (long ii = 0; ii < nprint; ii++) {
        cout << " " << vals[ii];
      }
      break;
    }
  
    case nc3Char: {
      cout << "(char)";
      TaArray<char> str_;
      char *str = str_.alloc(nprint + 1);
      memset(str, 0, nprint + 1);
      memcpy(str, values->base(), nprint);
      cout << " " << str;
      break;
    }
  
    case nc3Short: {
      cout << "(short)";
      short *vals = (short *) values->base();
      for (long ii = 0; ii < nprint; ii++) {
        cout << " " << vals[ii];
      }
      break;
    }
  
    case nc3Int: {
      cout << "(int)";
      int *vals = (int *) values->base();
      for (long ii = 0; ii < nprint; ii++) {
        cout << " " << vals[ii];
      }
      break;
    }
  
    case nc3Float: {
      cout << "(float)";
      float *vals = (float *) values->base();
      for (long ii = 0; ii < nprint; ii++) {
        cout << " " << vals[ii];
      }
      break;
    }
  
    case nc3Double: {
      cout << "(double)";
      double *vals = (double *) values->base();
      for (long ii = 0; ii < nprint; ii++) {
        cout << " " << vals[ii];
      }
      break;
    }
  
    case nc3NoType:
    default: {
      break;
    }
  
  }
  
  cout << endl;

  delete values;

}

