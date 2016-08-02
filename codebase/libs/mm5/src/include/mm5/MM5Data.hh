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
/////////////////////////////////////////////////////////////
// MM5Data.hh
//
// Abstract base class
//
// Read in an MM5 file, compute derived fields
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
/////////////////////////////////////////////////////////////

#ifndef MM5Data_H
#define MM5Data_H

#include <dataport/bigend.h>
#include <physics/IcaoStdAtmos.hh>
#include <cstdio>
#include <vector>
#include <string>
using namespace std;

class MM5Data {
  
public:

  // heartbeat function 

  typedef void (*heartbeat_t)(const char *label);

  typedef enum {
    LAMBERT_CONF = 3,
    MERCATOR = 4,
    STEREOGRAPHIC = 5,
    UNKNOWN = 99
  } projection_t;

  static const double MissingDouble;

  ///////////////
  // Constructor
  //
  // If flight_level is non-NULL, the freezing level is computed
  // as flight level.
  // If flight_level is NULL, the freezing level is computed as mb.
  // If you want registration with the procmap, set the heartbeat_func
  //    to PMU_auto_register() from toolsa.
  
  MM5Data (const string &prog_name,
	   const string &path,
	   bool debug = false,
	   const heartbeat_t heartbeat_func = NULL,
	   bool dbzConstIntercepts = true);
  
  /////////////
  // destructor

  virtual ~MM5Data();
  
  // getVersion() - static function
  // Gets the MM5 file version number
  // Returns 0 on success, -1 on failure.
  
  static int getVersion (const string &input_file_path,
			 int &version);

  //////////////////////////
  // find the header records

  virtual void findHdrRecords() = 0;

  /////////////////
  // read file

  virtual int read() = 0;

  /////////////////////////////////
  // is there more data in the file

  bool more() { return _more; }

  /////////////////////////////////
  // get file size in bytes
  
  long long getFileSize() { return _fileSize; }

  ///////////////////////
  // derive extra fields
  // Have to do read first

  void computeDerivedFields(bool wspd_in_knots = false);

  /////////////////////////////////////////////////////////
  // loadTurbField()
  // Load turb severity field from outside source, e.g. GTG
  
  void loadTurbField(fl32 ***turb_data);
  
  //////////////////////////////////////////////////////////////////////
  // loadIcingField()
  //
  // Load icing severity field from cloud mixing ratio and temperature
  //
  // Index of severity ranges from 0.0 to 1.0
  
  void loadIcingField(double trace_clw = 0.01,
		      double light_clw = 0.1,
		      double moderate_clw = 0.6,
		      double severe_clw = 1.2,
		      double clear_ice_temp = -10.0);
  
  // print headers

  virtual void printHeaders(FILE *out)  const = 0;

  // print model data fields

  void printModelFields(FILE *out, bool full = true) const;
  
  // print derived data fields

  void printDerivedFields(FILE *out, bool full = true) const;

  // constructor status

  int OK;

  // version number

  int _version;
  
  // get the FDDA start and end times - minutes

  virtual double getFddaStartTime() const = 0;
  virtual double getFddaEndTime() const = 0;

  // get MIF or MRF values, given the label
  // The first match with the label will be used.
  // You do not need to specify the entire label.

  virtual int getMifVal(const char *label) const = 0;
  virtual double getMrfVal(const char *label) const = 0;

  // get the MIF or MRF arrays

  si32 **getMifArray() { return _mif; }
  fl32 **getMrfArray() { return _mrf; }
  
  // "get" functions. Let i,j,k correspond east-west,north-south
  // and vertical grid coordinates respectively.
  
  // 3D fields:
  inline fl32 get_clw(int ilon, int ilat, int isig) const {
    return( clw[isig][ilat][ilon]);
  }
  inline fl32 get_pp(int ilon, int ilat, int isig) const {
    return( pp[isig][ilat][ilon] );
  }
  inline fl32 get_pres(int ilon, int ilat, int isig) const {
    return( pres[isig][ilat][ilon] );
  }
  inline fl32 get_q(int ilon, int ilat, int isig) const {
    return( qq[isig][ilat][ilon] );
  }
  inline fl32 get_rh(int ilon, int ilat, int isig) const {
    return( rh[isig][ilat][ilon] );
  }
  inline fl32 get_dewpt(int ilon, int ilat, int isig) const {
    return( dewpt[isig][ilat][ilon] );
  }
  inline fl32 get_rnw(int ilon, int ilat, int isig) const {
    return( rnw[isig][ilat][ilon]);
  }
  inline fl32 get_tk(int ilon, int ilat, int isig) const {
    return( tk[isig][ilat][ilon] );
  }
  inline fl32 get_u_dot(int ilon, int ilat, int isig) const {
    return( uu_dot[isig][ilat][ilon] );
  }
  
  inline fl32 get_u_cross(int ilon, int ilat, int isig) const {
    return( uu[isig][ilat][ilon] );
  }

  inline fl32 get_v_dot(int ilon, int ilat, int isig) const {
    return( vv_dot[isig][ilat][ilon] );
  }

  inline fl32 get_v_cross(int ilon, int ilat, int isig) const {
    return( vv[isig][ilat][ilon] );
  }

  inline fl32 get_w(int ilon, int ilat, int isig) const {
    return( ww[isig][ilat][ilon] );
  }

  inline fl32 get_z(int ilon, int ilat, int isig) const {
    return( zz[isig][ilat][ilon] );
  }

  inline fl32 get_divergence(int ilon, int ilat, int isig) const {
    return( divergence[isig][ilat][ilon] );
  }

  // 2D fields:
  inline fl32 get_coriolis(int ilon, int ilat) const {
    return( coriolis[ilat][ilon] );
  }
  inline fl32 get_coriolis_dot(int ilon, int ilat) const {
    return( coriolis_dot[ilat][ilon] );
  }
  inline fl32 get_ground_t(int ilon, int ilat) const {
    return( ground_t[ilat][ilon] );
  }
  inline fl32 get_lat_x(int ilon, int ilat) const {
    return(lat[ilat][ilon]);
  }
  inline fl32 get_lhflux(int ilon, int ilat) const {
    return( lhflux[ilat][ilon] );
  }
  inline fl32 get_lon_x(int ilon, int ilat) const {
    return(lon[ilat][ilon]);
  }
  inline fl32 get_mapf_dot(int ilon, int ilat) const {
    return(mapf_dot[ilat][ilon] );
  }
  inline fl32 get_mapf_x(int ilon, int ilat) const {
    return( mapf_x[ilat][ilon] );
  }
  inline fl32 get_pbl_hgt(int ilon, int ilat) const {
    return(pbl_hgt[ilat][ilon] );
  }
  inline fl32 get_regime(int ilon, int ilat) const {
    return(regime[ilat][ilon] );
  }
  inline fl32 get_pstar(int ilon, int ilat) const {
    return( pstar[ilat][ilon] );
  }
  inline fl32 get_shflux(int ilon, int ilat) const {
    return( shflux[ilat][ilon]);
  }
  inline fl32 get_terrain(int ilon, int ilat) const {
    return(terrain[ilat][ilon]);
  }
  inline fl32 get_ust(int ilon, int ilat) const {
    return(ust[ilat][ilon] );
  }
  
  // 1D fields:
  inline fl32 get_halfSigma(int isig) const {
    return( _halfSigma[isig] );
  }
  
  // Grid info and distance between grid points:

  inline int get_nLon() const { return nLon; }
  inline int get_nLat() const { return nLat; }
  inline int get_nSigma() const { return nSigma; }
  inline fl32 get_pTop() const { return pTop; }
  inline fl32 get_pos() const { return pos; }
  inline fl32 get_tso() const { return tso; }
  inline fl32 get_tlp() const { return tlp; }

  // Projection variables:

  inline projection_t get_proj_type() { return proj_type; }
  inline fl32 get_center_lat() const {  return center_lat; }
  inline fl32 get_center_lon() const { return center_lon; }
  inline fl32 get_true_lat1() const { return true_lat1; }
  inline fl32 get_true_lat2() const {return true_lat2; }
  inline fl32 get_grid_distance() const { return grid_distance; }
  inline fl32 get_cone_factor() const { return cone_factor; }
  inline fl32 get_domain_scale_coarse() const { return domain_scale_coarse; }
  inline fl32 get_x1_in_coarse_domain() const { return x1_in_coarse_domain; }
  inline fl32 get_y1_in_coarse_domain() const { return y1_in_coarse_domain; }
  inline fl32 get_x1_in_mother_domain() const { return x1_in_mother_domain; }
  inline fl32 get_y1_in_mother_domain() const { return y1_in_mother_domain; }
  
  // grid info

  int nLat;
  int nLon;
  int nSigma;
  int nyDot;
  int nxDot;
  int nyDotCoarse;
  int nxDotCoarse;
  int nPtsDotPlane;

  fl32 pTop;
  fl32 pos;
  fl32 tso;
  fl32 tlp;

  // projection info

  projection_t proj_type;
  fl32 center_lat, center_lon;
  fl32 true_lat1, true_lat2; // lambert
  fl32 cone_factor;
  fl32 grid_distance; // dx = dy, this domain
  fl32 grid_distance_coarse; // dx = dy, coarse domain
  fl32 domain_scale_coarse; // scale of coarse domain rel to this one
  fl32 x1_in_coarse_domain; // x posn in coarse domain of SW point
  fl32 y1_in_coarse_domain; // y posn in coarse domain of SW point
  fl32 x1_in_mother_domain; // x posn in mother domain of SW point
  fl32 y1_in_mother_domain; // y posn in mother domain of SW point

  // coords (km) of SW corner dot point of coarse domain rel to center pt
  fl32 minx_dot_coarse;
  fl32 miny_dot_coarse;
  
  // coords (km) of SW corner dot point of this domain rel to center pt
  fl32 minx_dot;
  fl32 miny_dot;

  // coords (km) of SW corner cross point of this domain rel to center pt
  fl32 minx_cross;
  fl32 miny_cross;

  // raw 3d fields - cross grid.
  
  fl32 ***uu;      // U wind
  fl32 ***vv;      // V Wind.
  fl32 ***tk;      // Temperature K
  fl32 ***qq;      // Water Vapor Mixing Ratio Kg/Kg
  fl32 ***clw;     // Cloud Water Mixing Ratio Kg/Kg
  fl32 ***rnw;     // Rain Water Mixing Ratio Kg/Kg
  fl32 ***ice;     // Cloud Ice Mixing Ratio Kg/Kg
  fl32 ***snow;    // Falling Snow Mixing Ratio Kg/Kg
  fl32 ***graupel; // Falling Greaupel Mixing Ratio Kg/Kg
  fl32 ***nci;     // Nuclear Condensation Index # per M^3
  fl32 ***rad_tend; // Radiation Tendancy
  fl32 ***ww;      // Vertical Velovity m/s
  fl32 ***pp;      // Presure Pertibation

  // raw 3d fields - dot grid.
  
  fl32 ***uu_dot;
  fl32 ***vv_dot;

  // w field on full sigma levels

  fl32 ***ww_full;

  // derived 3d fields - cross grid.
  
  fl32 ***uuTn; // uu relative to TN
  fl32 ***vvTn; // vv relative to TN
  fl32 ***uuOut; // uu relative to output grid
  fl32 ***vvOut; // vv relative to output grid
  fl32 ***tc;
  fl32 ***wspd;
  fl32 ***wdir;
  fl32 ***zz;
  fl32 ***divergence;
  fl32 ***pres;
  fl32 ***rh;
  fl32 ***dewpt;
  fl32 ***turb;
  fl32 ***icing;
  fl32 ***clw_g;
  fl32 ***rnw_g;
  fl32 ***q_g;
  fl32 ***theta;
  fl32 ***thetae;
  fl32 ***thetav;
  fl32 ***dbz_3d;
  fl32 ***tot_cld_con;

  // raw 2d fields - cross grid.
  
  fl32 **pstar;       // Pressure Scale
  fl32 **ground_t;    // Ground Temperature deg K
  fl32 **rain_con;    // Parameterized Convective Rain -cm
  fl32 **rain_non;    // Non-Convective Rain  -cm.
  fl32 **terrain;     // Terrain Height - meters
  fl32 **coriolis;    // Coriolis Factor.
  fl32 **res_temp;    
  fl32 **lat;
  fl32 **lon;
  fl32 **land_use;
  fl32 **snowcovr;
  fl32 **tseasfc;
  fl32 **pbl_hgt;
  fl32 **regime;
  fl32 **shflux;
  fl32 **lhflux;
  fl32 **ust;
  fl32 **swdown;
  fl32 **lwdown;
  fl32 **soil_t_1;
  fl32 **soil_t_2;
  fl32 **soil_t_3;
  fl32 **soil_t_4;
  fl32 **soil_t_5;
  fl32 **soil_t_6;
  fl32 **soil_m_1;
  fl32 **soil_m_2;
  fl32 **soil_m_3;
  fl32 **soil_m_4;
  fl32 **sfcrnoff;
  fl32 **t2;
  fl32 **q2;
  fl32 **u10;
  fl32 **v10;
  fl32 **mapf_x;
  fl32 **weasd;
  fl32 **snowh;
  fl32 **hc_rain;
  fl32 **hn_rain;

  /// Illuminance

  fl32 **swfrac;	// soil moisture ratio from saturation
  fl32 **sunalt;	// sun altitude
  fl32 **sunazm;	// sun azimuth
  fl32 **moonalt;	// moon altitude
  fl32 **moonazm;	// moon azimuth
  fl32 **sunill;	// sun illuminance
  fl32 **moonill;	// moon illuminance
  fl32 **totalill;	// total illuminance

  /// Cloud fields
  fl32 **clwi;		// cloud liquid water
  fl32 **rnwi;		// rain water
  fl32 **icei;		// ice water
  fl32 **snowi;		// snow water
  fl32 **pwv;		// precipitable water

  fl32 **sun_btw;	// sun begin twilight
  fl32 **sun_etw;	// sun end twilight
  fl32 **sun_abtw;	// sun azimuth begin twilight
  fl32 **sun_aetw;	// sun asimuth end twilight
  fl32 **sun_rise;	// sun rise
  fl32 **sun_set;	// sun set
  fl32 **sun_aris;	// sun azimuth rise
  fl32 **sun_aset;	// sun azimuth set
  fl32 **moon_ris;	// moon rise
  fl32 **moon_set;	// moon set
  fl32 **moon_ari;	// moon azimuth rise
  fl32 **moon_ase;	// moon asimuth set

  // raw 2d fields - dot grid
  
  fl32 **mapf_dot;
  fl32 **coriolis_dot;

  // derived 2d fields - cross grid.
  
  fl32 **gridRotation; // grid rotation relative to TN
  fl32 **t2c;
  fl32 **pres2;
  fl32 **fzlevel;
  fl32 **rain_total;
  fl32 **hourly_rain_total;
  fl32 **dbz_2d;
  fl32 **rh2;
  fl32 **dewpt2;
  fl32 **wspd10;
  fl32 **wdir10;
  fl32 **mslp2;
  fl32 **q2_g;
  fl32 **theta2;
  fl32 **thetae2;
  fl32 **thetav2;
  fl32 **cloud_fract;
  fl32 **twp;
  fl32 **rwp;
  fl32 **tot_clw;
  fl32 **tot_cld_conp;
  fl32 **clwp;

  // field numbers

  int uFieldNum;
  int vFieldNum;
  int tFieldNum;
  int qFieldNum;
  int clwFieldNum;
  int rnwFieldNum;
  int iceFieldNum;
  int snowFieldNum;
  int graupelFieldNum;
  int nciFieldNum;
  int radTendFieldNum;
  int wFieldNum;
  int ppFieldNum;
  
  int pstarFieldNum;
  int groundTFieldNum;
  int rainConFieldNum;
  int rainNonFieldNum;
  int terrainFieldNum;
  int coriolisFieldNum;
  int resTempFieldNum;
  int latFieldNum;
  int lonFieldNum;
  int landUseFieldNum;
  int snowcovrFieldNum;
  int tseasfcFieldNum;
  int pblHgtFieldNum;
  int regimeFieldNum;
  int shfluxFieldNum;
  int lhfluxFieldNum;
  int ustFieldNum;
  int swdownFieldNum;
  int lwdownFieldNum;
  int soilT1FieldNum;
  int soilT2FieldNum;
  int soilT3FieldNum;
  int soilT4FieldNum;
  int soilT5FieldNum;
  int soilT6FieldNum;
  int soilM1FieldNum;
  int soilM2FieldNum;
  int soilM3FieldNum;
  int soilM4FieldNum;
  int sfcrnoffFieldNum;
  int t2FieldNum;
  int q2FieldNum;
  int u10FieldNum;
  int v10FieldNum;
  int mapfXFieldNum;
  int mapfDotFieldNum;
  int weasdFieldNum;
  int snowhFieldNum;
  int hc_rainFieldNum;
  int hn_rainFieldNum;

  int swfracFieldNum;
  int sunaltFieldNum;
  int sunazmFieldNum;
  int moonaltFieldNum;
  int moonazmFieldNum;
  int sunillFieldNum;
  int moonillFieldNum;
  int totalillFieldNum;

  int clwiFieldNum;
  int rnwiFieldNum;
  int iceiFieldNum;
  int snowiFieldNum;
  int pwvFieldNum;

  int sunBtwFieldNum;
  int sunEtwFieldNum;
  int sunAbtwFieldNum;
  int sunAetwFieldNum;
  int sunRiseFieldNum;
  int sunSetFieldNum;
  int sunArisFieldNum;
  int sunAsetFieldNum;
  int moonRisFieldNum;
  int moonSetFieldNum;
  int moonAriFieldNum;
  int moonAseFieldNum;

  // field names and units in the MM5 data file

  vector<string> fieldNames;
  vector<string> fieldUnits;

  // forecast times
  
  time_t modelTime;
  time_t outputTime;
  int forecastLeadTime;
  int forecastDelta;

  // interp3dField()
  //
  // Load up the sigma field array interpolated for a given point.
  //
  // returns ptr to array on success, NULL on failure.
  
  void interp3dField(int ilat, int ilon,
		     const char *name, fl32 ***field,
		     double wt_sw, double wt_nw,
		     double wt_ne, double wt_se,
		     vector<double> &interp_data,
		     const vector<bool> *sigma_needed = NULL) const;
  
  // interp3dLevel()
  //
  // Interpolate a 3 d field at a given sigma level
  //
  // returns val on success, MissingDouble on failure.

  double interp3dLevel(int isig, int ilat, int ilon,
		       const char *name, fl32 ***field,
		       double wt_sw, double wt_nw,
		       double wt_ne, double wt_se) const;
  
  // closest3dField()
  //
  // Load up the sigma field array from model point closest to the given point.
  //
  // returns ptr to array on success, NULL on failure.

  void closest3dField(int ilat, int ilon,
		      const char *name, fl32 ***field,
		      double wt_sw, double wt_nw,
		      double wt_ne, double wt_se,
		      vector<double> &closest_data,
		      const vector<bool> *sigma_needed = NULL) const;
  
  // interp2dField()
  //
  // Load up interp_val_p with value interpolated for a given point.
  //
  // returns val on success, MISSING_DOUBLE on failure.
  
  double interp2dField(int ilat, int ilon,
                       const char *name, fl32 **field,
                       double wt_sw, double wt_nw,
                       double wt_ne, double wt_se) const;
  
  // closest2dField()
  //
  // Load up the field array from model point closest to the given point.
  //
  // returns ptr to array on success, NULL on failure.

  double closest2dField(int ilat, int ilon,
		      const char *name, fl32 **field,
		      double wt_sw, double wt_nw,
		      double wt_ne, double wt_se) const;
  
  // Compute the wind components
  // relative to output grid
  //
  // rotation of output grid relative to TN is input
  
  void loadUVOutput(fl32 **outputGridRotation);
  
protected:
  
  const string &_progName;
  string _path;
  bool _debug;
  bool _dbzConstIntercepts;
  IcaoStdAtmos _isa;
  heartbeat_t _heartbeatFunc;

  si32 **_mif;
  fl32 **_mrf;
  fl32 *_halfSigma;

  fl32 ***_field3d;
  fl32 **_field2d;

  FILE *_in;

  long long _fileSize;
  bool _more;

  // functions

  long _readFortRecLen();

  void _loadTempCField();

  void _loadPressureField();

  void _loadDivergence();
  
  void _loadRhField();

  void _loadDewPtField();

  void _loadWspdDirnField(bool wspd_in_knots = false);

  void _loadIcingField();

  void _loadTempC2Field();

  void _loadPressure2Field();

  void _loadFzlevelField();

  void _loadRainTotalField();

  void _loadHourlyRainTotalField();
  
  void _loadCloudFractField();
  
  void _loadTotalWaterPathFields();

  void _loadDbz2DField();

  double _virtual(double temp, double ratmix);

  void _loadDbz3DField();

  void _loadRH2Field();

  void _loadDewPt2Field();

  void _loadSpeed10Field(bool wspd_in_knots = false);

  void _loadMSLP2Field(bool benjaminMiller = true);

  void _loadTotalCldConField();

  void _loadTotalCldConPField();

  void _loadCLWPField();

  void _loadT2CField();

  void _loadQ2_GField();

  void _loadCLW_GField();

  void _loadRNW_GField();

  void _loadTHETAField();

  void _loadTHETAEField();

  void _loadTHETAVField();

  void _loadTHETA2Field();

  void _loadTHETAE2Field();

  void _loadTHETAV2Field();

  void _loadQ_GField();

  void _printHeaders(FILE *out) const;

  void _computeGridRotation();

  void _computeUVRelToTN();

private:

};

#endif

