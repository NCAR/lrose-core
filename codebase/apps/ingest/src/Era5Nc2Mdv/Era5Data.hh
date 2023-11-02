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
// Era5Data.hh
//
// Read in an WRF file, compute derived fields
//
// Paul Prestopnik, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2007
//
/////////////////////////////////////////////////////////////

#ifndef Era5Data_H
#define Era5Data_H

#include "Params.hh"
#include <Ncxx/Nc3File.hh>
#include <dataport/bigend.h>
#include <physics/IcaoStdAtmos.hh>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <Mdv/MdvxProj.hh>

using namespace std;

class Era5Data
{
  
public:

  //////////////////
  // Public types //
  //////////////////

  // heartbeat function 

  typedef void (*heartbeat_t)(const char *label);

  typedef enum
  {
    LAMBERT_CONF = 3,
    MERCATOR = 4,
    STEREOGRAPHIC = 5,
    UNKNOWN = 99
  } projection_t;


  //////////////////////
  // Public constants //
  //////////////////////

  static const double MISSING_DOUBLE;


  ////////////////////
  // Public members //
  ////////////////////

  time_t forecastTime;  //valid time for the forecast
  time_t genTime; //When the model was run (hot or cold?)
  int forecastLeadTime;



  ////////////////////
  // Public methods //
  ////////////////////

  ///////////////
  // Constructor
  //
  // If flight_level is non-NULL, the freezing level is computed
  // as flight level.
  // If flight_level is NULL, the freezing level is computed as mb.
  // If you want registration with the procmap, set the heartbeat_func
  //    to PMU_auto_register() from toolsa.
  
  Era5Data();
  
  bool init(const string &prog_name,
	    const Params &params,
	    const heartbeat_t heartbeat_func = 0);
  
  bool initFile(const string &path);
  

  void clearData();
  
  /////////////
  // destructor

  virtual ~Era5Data();
  
  /////////////////
  // read from _ncf 

  bool read();

  // 1D fields:

  inline fl32 get_halfEta(int isig)
  {
    _loadHalfEta();
    
    if (dataDimension() > 2)
      return _halfEta[isig];
    else
      return 0;  //need this for odd cases of only 2D data.
  }
 
  // Grid info and distance between grid points:

  inline int getNLon() const { return _nLon; }
  inline int getNLat() const { return _nLat; }
  inline int getNEta() const { return _nEta; }

  inline projection_t getProjType() const { return _projType; }

  inline fl32 getProjCenterLat() const { return _projCenterLat; }
  inline fl32 getProjCenterLon() const { return _projCenterLon; }

  inline fl32 getTrueLat1() const { return _trueLat1; }
  inline fl32 getTrueLat2() const { return _trueLat2; }
  
  inline fl32 getGridDx() const { return _gridDx; }
  inline fl32 getGridDy() const { return _gridDy; }
  
  inline fl32 getGridMinX() const { return _gridMinX; }
  inline fl32 getGridMinY() const { return _gridMinY; }
  

  // initProjection() will initializ the proj object by calling
  // the correctin MdvxProj::init*() function based upon _projType
  // and passing it the appropriate projection parameters
  // returns 1 on sucess, 0 on failure

  int initProjection(MdvxProj &proj);

  // This function returns 3 if there is 3d data, or 2 otherwise
  // it currently does this by just looking to see if any pressure fields
  // are available or not based on the assumption that the pressence of 
  // 3d data will require a pres field.  If this turns out to be false,
  // a more exhaustive search for 3d fields may be required.

  int dataDimension() const
  {
    if (_dataDimension < 0)
    {
      if (_ncf->get_var("P") == NULL &&
	  _ncf->get_var("PB") == NULL &&
	  _ncf->get_var("PH") == NULL &&
	  _ncf->get_var("PHB") == NULL)
	_dataDimension = 2;
      else
	_dataDimension = 3;
    }
    
    return _dataDimension;
  }

  // interp3dField()
  //
  // Load up the eta field array interpolated for a given point.
  void interp3dField(int ilat, int ilon,
		     const char *name, fl32 ***field,
		     double wt_sw, double wt_nw,
		     double wt_ne, double wt_se,
		     vector<double> &interp_data,
		     const vector<bool> *eta_needed = NULL) const;
  
  // interp3dLevel()
  //
  // Interpolate a 3 d field at a given eta level
  //
  // returns val on success, MISSING_DOUBLE on failure.

  double interp3dLevel(int isig, int ilat, int ilon,
		       const char *name, fl32 ***field,
		       double wt_sw, double wt_nw,
		       double wt_ne, double wt_se) const;
  
  // closest3dField()
  //
  // Load up the eta field array from model point closest to the given point.
  //
  // returns ptr to array on success, NULL on failure.

  void closest3dField(int ilat, int ilon,
		      const char *name, fl32 ***field,
		      double wt_sw, double wt_nw,
		      double wt_ne, double wt_se,
		      vector<double> &closest_data,
		      const vector<bool> *eta_needed = NULL) const;
  
  // interp2dField()
  //
  // Load up interp_val_p with value interpolated for a given point.
  //
  // returns val on success, MISSING_DOUBLE on failure.
  
  double interp2dField(int ilat, int ilon,
                       const char *name, fl32 **field,
                       double wt_sw, double wt_nw,
                       double wt_ne, double wt_se) const;
  
  void computeUVOutput(const MdvxProj &proj);  // Compute the wind components relative to output grid

  // Print _ncf for debugging
  void printFile();

  //reads the dimesion variables from _ncf
  void readDimensions();

  //only useful for stereographic projections
  double calculate_central_scale(); 
 
  // Grid access methods //

  inline fl32 ***getUu()
  {
    _loadUWind();
    
    return _uu;
  }
  
  inline fl32 ***getVv()
  {
    _loadVWind();
    
    return _vv;
  }
  
  inline fl32 ***getPpt()
  {
    _loadTemp();
    
    return _ppt;
  }
  
  inline fl32 ***getQq()
  {
    _loadWaterMixingRatio();
    
    return _qq;
  }
  
  inline fl32 ***getClw()
  {
    _loadCloudMixingRatio();
    
    return _clw;
  }
  
  inline fl32 ***getRnw()
  {
    _loadRainMixingRatio();
    
    return _rnw;
  }
  
  inline fl32 ***getIce()
  {
    _loadIceMixingRatio();
    
    return _ice;
  }

    inline fl32 ***getNRain()
  {
    _loadNRain();
    
    return _nRain;
  }

  inline fl32 ***getNCloud()
  {
    _loadNCloud();
    
    return _nCloud;
  }

  inline fl32 ***getSnow()
  {
    _loadSnowMixingRatio();
    
    return _snow;
  }
  
  inline fl32 ***getGraupel()
  {
    _loadGraupelMixingRatio();
    
    return _graupel;
  }
  
  inline fl32 ***getWw()
  {
    _loadWWind();
    
    return _ww;
  }
  
  inline fl32 ***getPp()
  {
    _loadPertPres();
    
    return _pp;
  }
  
  inline fl32 ***getPb()
  {
    _loadBSP();
    
    return _pb;
  }
  
  inline fl32 ***getPh()
  {
    _loadPertGeo();
    
    return _ph;
  }
  
  inline fl32 ***getPhb()
  {
    _loadBSPG();
    
    return _phb;
  }
  
  inline fl32 ***getUuC()
  {
    _loadUCWind();
    
    return _uu_C;
  }
  
  inline fl32 ***getVvC()
  {
    _loadVCWind();
    
    return _vv_C;
  }
  
  inline fl32 ***getWwC()
  {
    _loadWCWind();
    
    return _ww_C;
  }
  
  inline fl32 ***getPhC()
  {
    _loadPertGeoC();
    
    return _ph_C;
  }
  
  inline fl32 ***getPhbC()
  {
    _loadBSPGC();
    
    return _phb_C;
  }

  inline fl32 ***getDNW()
  {
    _loadDNW();
    
    return _DNW;
  }

  inline fl32 ***getMUB()
  {
    _loadMUB();
    
    return _MUB;
  }

  inline fl32 ***getMU()
  {
    _loadMU();
    
    return _MU;
  }

  inline fl32 ***getREFL3D()
  {
    _loadREFL3D();
    
    return _REFL3D;
  }
  
  inline fl32 ***getUuTn()
  {
    _computeUVRelToTN();
    
    return _uuTn;
  }
  
  inline fl32 ***getVvTn()
  {
    _computeUVRelToTN();
    
    return _vvTn;
  }
  
  inline fl32 ***getUuOut()
  {
    // Note that the caller must know to cal computeUVOutput() before
    // calling this method.
    
    return _uuOut;
  }
  
  inline fl32 ***getVvOut()
  {
    // Note that the caller must know to cal computeUVOutput() before
    // calling this method.
    
    return _vvOut;
  }
  
  inline fl32 ***getTc()
  {
    _computeTempC();
    
    return _tc;
  }
  
  inline fl32 ***getWspd()
  {
    _computeWspdDirnField();
    
    return _wspd;
  }
  
  inline fl32 ***getWdir()
  {
    _computeWspdDirnField();
    
    return _wdir;
  }
  
  inline fl32 ***getZz()
  {
    _computeHeightField();
    
    return _zz;
  }
  
  inline fl32 ***getPres()
  {
    _computePressure();
    
    return _pres;
  }
  
  inline fl32 ***getTk()
  {
    _computeTemp();
    
    return _tk;
  }
  
  inline fl32 ***getRh()
  {
    _computeRh();
    
    return _rh;
  }
  
  inline fl32 ***getSpecH()
  {
    _computeSpecHumidity();
    
    return _spec_h;
  }
  
  inline fl32 **getSpecH2M()
  {
    _computeSpecHumidity2m();
    
    return _spec_h2m;
  }
  
  inline fl32 ***getDewpt()
  {
    _computeDewPt();
    
    return _dewpt;
  }
  
  inline fl32 ***getIcing()
  {
    _computeIcing();
    
    return _icing;
  }
  
  inline fl32 ***getClwG()
  {
    _computeCLW_GField();
    
    return _clw_g;
  }
  
  inline fl32 ***getRnwG()
  {
    _computeRNW_GField();
    
    return _rnw_g;
  }
  
  inline fl32 ***getQG()
  {
    _computeQ_GField();
    
    return _q_g;
  }
  
  inline fl32 ***getTheta()
  {
    _computeTHETAField();
    
    return _theta;
  }
  
  inline fl32 ***getDbz3d()
  {
    _computeDbz3DField();
    
    return _dbz_3d;
  }
  
  inline fl32 ***getGeoHgt()
  {
    _computeGeopotHeight();
    
    return _geo_hgt;
  }
  
  inline fl32 ***getGeoPot()
  {
    _computeGeopot();
    
    return _geo_pot;
  }
  
  inline fl32 ***getItfadef()
  {
    _loadItfadef();
    
    return _itfadef;
  }
  
  inline fl32 ***getCape3d()
  {
    _computeCAPECIN_3D();
    
    return _cape3d;
  }
  
  inline fl32 ***getCin3d()
  {
    _computeCAPECIN_3D();
    
    return _cin3d;
  }
  
  inline fl32 **getCape()
  {
    _computeCAPECIN_2D();
    
    return _cape;
  }
  
  inline fl32 **getCin()
  {
    _computeCAPECIN_2D();
    
    return _cin;
  }
  
  inline fl32 ***getLcl3d()
  {
    _computeCAPECIN_3D();
    
    return _lcl3d;
  }
  
  inline fl32 ***getLfc3d()
  {
    _computeCAPECIN_3D();
    
    return _lfc3d;
  }
  
  inline fl32 ***getEl3d()
  {
    _computeCAPECIN_3D();
    
    return _el3d;
  }
  
  inline fl32 **getLcl()
  {
    _computeCAPECIN_2D();
    
    return _lcl;
  }
  
  inline fl32 **getLfc()
  {
    _computeCAPECIN_2D();
    
    return _lfc;
  }
  
  inline fl32 **getEl()
  {
    _computeCAPECIN_2D();
    
    return _el;
  }
  
  inline fl32 **getSurfP()
  {
    _loadSurfacePressure();
    
    return _surfP;
  }
  
  inline fl32 **getLandMask()
  {
    _loadLandMask();
    
    return _land_mask;
  }
  
  inline fl32 **getGroundT()
  {
    _loadGroundTemperature();
    
    return _ground_t;
  }
  
  inline fl32 **getRainC()
  {
    _loadRainC();
    
    return _rainc;
  }
  
  inline fl32 **getRainNC()
  {
    _loadRainNC();
    
    return _rainnc;
  }
  
  inline fl32 **getTerrain()
  {
    _loadTerrainHeight();
    
    return _terrain;
  }
  
  inline fl32 **getLat()
  {
    _loadLat();
    
    return _lat;
  }
  
  inline fl32 **getLon()
  {
    _loadLon();
    
    return _lon;
  }
  
  inline fl32 **getLandUse()
  {
    _loadLandUse();
    
    return _land_use;
  }
  
  inline fl32 **getSnowCovr()
  {
    _loadSnowCover();
    
    return _snowcovr;
  }
  
  inline fl32 **getTSeaSfc()
  {
    _loadSeaSurfaceTemp();
    
    return _tseasfc;
  }
  
  inline fl32 **getPblHgt()
  {
    _loadPBLHeight();
    
    return _pbl_hgt;
  }
  
  inline fl32 **getHfx()
  {
    _loadHFX();
    
    return _hfx;
  }
  
  inline fl32 **getLh()
  {
    _loadLH();
    
    return _lh;
  }
  
  inline fl32 **getSnowWE()
  {
    _loadSnowWE();
    
    return _snow_we;
  }
  
  inline fl32 **getSnowNC()
  {
    _loadSnowNC();
    
    return _snow_nc;
  }
  
  inline fl32 **getGraupelNC()
  {
    _loadGraupelNC();
    
    return _graupel_nc;
  }
  
  inline fl32 **getSoilT1()
  {
    _loadSoilInfo();
    
    return _soil_t_1;
  }
  
  inline fl32 **getSoilT2()
  {
    _loadSoilInfo();
    
    return _soil_t_2;
  }
  
  inline fl32 **getSoilT3()
  {
    _loadSoilInfo();
    
    return _soil_t_3;
  }
  
  inline fl32 **getSoilT4()
  {
    _loadSoilInfo();
    
    return _soil_t_4;
  }
  
  inline fl32 **getSoilT5()
  {
    _loadSoilInfo();
    
    return _soil_t_5;
  }
  
  inline fl32 **getSoilM1()
  {
    _loadSoilInfo();
    
    return _soil_m_1;
  }
  
  inline fl32 **getSoilM2()
  {
    _loadSoilInfo();
    
    return _soil_m_2;
  }
  
  inline fl32 **getSoilM3()
  {
    _loadSoilInfo();
    
    return _soil_m_3;
  }
  
  inline fl32 **getSoilM4()
  {
    _loadSoilInfo();
    
    return _soil_m_4;
  }
  
  inline fl32 **getSoilM5()
  {
    _loadSoilInfo();
    
    return _soil_m_5;
  }
  
  inline fl32 getSoilD1()
  {
    _loadSoilInfo();
    
    return _soil_d_1;
  }
  
  inline fl32 getSoilD2()
  {
    _loadSoilInfo();
    
    return _soil_d_2;
  }
  
  inline fl32 getSoilD3()
  {
    _loadSoilInfo();
    
    return _soil_d_3;
  }
  
  inline fl32 getSoilD4()
  {
    _loadSoilInfo();
    
    return _soil_d_4;
  }
  
  inline fl32 getSoilD5()
  {
    _loadSoilInfo();
    
    return _soil_d_5;
  }
  
  inline fl32 **getSoilAM1()
  {
    _computeAvailMoist();
    
    return _soil_am_1;
  }
  
  inline fl32 **getSoilAM2()
  {
    _computeAvailMoist();
    
    return _soil_am_2;
  }
  
  inline fl32 **getSoilAM3()
  {
    _computeAvailMoist();
    
    return _soil_am_3;
  }
  
  inline fl32 **getSoilAM4()
  {
    _computeAvailMoist();
    
    return _soil_am_4;
  }
  
  inline fl32 **getSoilAM5()
  {
    _computeAvailMoist();
    
    return _soil_am_5;
  }
  
  inline fl32 **getSoilType()
  {
    _loadSoilType();
    
    return _soil_type;
  }
  
  inline fl32 **getTwp()
  {
    _computeTotalWaterPathFields();
    
    return _twp;
  }
  
  inline fl32 **getRwp()
  {
    _computeTotalWaterPathFields();
    
    return _rwp;
  }
  
  inline fl32 **getVil()
  {
    _computeTotalWaterPathFields();
    
    return _vil;
  }
  
  inline fl32 **getSfcRnOff()
  {
    _loadSurfaceRunoff();
    
    return _sfcrnoff;
  }
  
  inline fl32 **getT2()
  {
    _loadT2();
    
    return _t2;
  }
  
  inline fl32 **getQ2()
  {
    _loadQ2();
    
    return _q2;
  }
  
  inline fl32 **getU10()
  {
    _loadU10();
    
    return _u10;
  }
  
  inline fl32 **getV10()
  {
    _loadV10();
    
    return _v10;
  }
  
  inline fl32 **getSnowH()
  {
    _loadSnowHeight();
    
    return _snowh;
  }
  
  inline fl32 **getTh2()
  {
    _loadTH2();
    
    return _th2;
  }
  
  inline fl32 **getT2C()
  {
    _computeT2C();
    
    return _t2c;
  }
  
  inline fl32 **getFzLevel()
  {
    _computeFzlevel();
    
    return _fzlevel;
  }
  
  inline fl32 **getRainTotal()
  {
    _computeTotalRain();
    
    return _rain_total;
  }
  
  inline fl32 **getDbz2D()
  {
    _computeDbz2DField();
    
    return _dbz_2d;
  }
  
  inline fl32 **getRH2()
  {
    _computeRH2();
    
    return _rh2;
  }
  
  inline fl32 **getWspd10()
  {
    _computeSpeedDir10();
    
    return _wspd10;
  }
  
  inline fl32 **getWdir10()
  {
    _computeSpeedDir10();
    
    return _wdir10;
  }
  
  inline fl32 **getQ2G()
  {
    _loadQ2_GField();
    
    return _q2_g;
  }
  
  inline fl32 **getLanduseF1()
  {
    _loadLandUsef();
    
    return _landusef1;
  }
  
  inline fl32 **getLanduseF2()
  {
    _loadLandUsef();
    
    return _landusef2;
  }
  
  inline fl32 **getLanduseF6()
  {
    _loadLandUsef();
    
    return _landusef6;
  }
  
  inline fl32 **getLanduseF15()
  {
    _loadLandUsef();
    
    return _landusef15;
  }
  
  inline fl32 **getGreenFrac7()
  {
    _loadGreenFrac7();
    
    return _greenfrac7;
  }
  

protected:

  /////////////////////
  // Protected types //
  /////////////////////

  typedef enum
  {
    V2 = 1, // currently tested against WRF V2.1.2, but probably ok for 
            // other versions of WRF if they haven't changed too much
    GEOGRID = 2,  //generated by the WRF preprocessing system (WPS) program, geogrid 
    UNKNOWN_FILETYPE = 99
  } WRF_file_type_t;


  /////////////////////////
  // Protected constants //
  /////////////////////////

  // This is used to initialize lat/lon values

  static const double BAD_LAT_LON;


  ///////////////////////
  // Protected members //
  ///////////////////////

  // grid info

  int _nLon;  //x
  int _nLat;  //y
  int _nEta;  //z

  // projection info

  projection_t _projType;
  fl32 _projCenterLat, _projCenterLon;
  fl32 _trueLat1, _trueLat2; // lambert & polar stereographic 
  fl32 _gridDx, _gridDy;
  fl32 _gridMinX, _gridMinY;
  fl32 _llLat, _llLon;  //the lat,lon of the center of the SW grid cell.

  // type of the input file

  WRF_file_type_t _inputFileType;

  int _unlimited_ix;  //index into the unlimited dimension (time)

  string _progName;
  string _path;
  Params _params;
  IcaoStdAtmos _isa;
  heartbeat_t _heartbeatFunc;
  Nc3File *_ncf;
  Nc3Error *_ncfError;

  mutable int _dataDimension;
  
  si32 **_mif;
  fl32 **_mrf;
  fl32 *_halfEta;
  fl32 *_znw;

  fl32 ***_field3d;
  fl32 **_field2d;

  //  FILE *_in;

  long _fileSize;

  // WRF NetCDF data component names

  string _startDateKey;
  string _gridIdKey;
  string _latFieldKey;
  string _lonFieldKey;

  // raw 3d fields - cross grid.
  
  fl32 ***_uu;      // U wind
  fl32 ***_vv;      // V Wind.
  fl32 ***_ppt;	   // pertubation potential temperature (theta-t0)
  fl32 ***_qq;      // Water Vapor Mixing Ratio Kg/Kg
  fl32 ***_clw;     // Cloud Water Mixing Ratio Kg/Kg
  fl32 ***_rnw;     // Rain Water Mixing Ratio Kg/Kg
  fl32 ***_ice;     // Cloud Ice Mixing Ratio Kg/Kg
  fl32 ***_snow;    // Falling Snow Mixing Ratio Kg/Kg
  fl32 ***_nRain;     // number concentration of Rain (unitless)
  fl32 ***_nCloud;    // number concentration of Cloud (unitless)
  fl32 ***_graupel; // Falling Greaupel Mixing Ratio Kg/Kg
  fl32 ***_ww;      // Vertical Velovity m/s
  fl32 ***_pp;      // Perturbation Presure (in Pa)
  fl32 ***_pb; 	   // base state pressure (in Pa)
  fl32 ***_ph; 	   // pertubation geopotential (m2 s-2)
  fl32 ***_phb; 	   // base state geopotential (m2 s-2)
  fl32 ***_DNW;            // d(eta) values between full (w) levels
  fl32 ***_MUB;            // base state dry air mass in column
  fl32 ***_MU;             // perturbation dry air mass in column
  fl32 ***_REFL3D;         // Radar Reflectivity
  
  // raw 3d fields - C grid.
  
  fl32 ***_uu_C;
  fl32 ***_vv_C;
  fl32 ***_ww_C;
  fl32 ***_ph_C; 	   // pertubation geopotential (m2 s-2)
  fl32 ***_phb_C; 	   // base state geopotential (m2 s-2)

  // derived 3d fields - cross grid.
  
  fl32 ***_uuTn; // _uu relative to TN
  fl32 ***_vvTn; // _vv relative to TN
  fl32 ***_uuOut; // _uu relative to output grid
  fl32 ***_vvOut; // _vv relative to output grid
  fl32 ***_tc;
  fl32 ***_wspd;
  fl32 ***_wdir;
  fl32 ***_zz;
  fl32 ***_pres;  //pressure _pp+_pb (in mb)
  fl32 ***_tk;      // Temperature K
  fl32 ***_rh;
  fl32 ***_spec_h;
  fl32 **_spec_h2m; //specific humidity at 2m
  fl32 ***_dewpt;
  fl32 ***_icing;
  fl32 ***_clw_g;
  fl32 ***_rnw_g;
  fl32 ***_q_g;  // Q in g/kg
  fl32 ***_theta;
  fl32 ***_dbz_3d;
  fl32 ***_geo_hgt;
  fl32 ***_geo_pot;
  fl32 ***_itfadef;
  fl32 ***_cape3d;  //cape,cin,lcl,lfc,&el are used by CIP/FIP
  fl32 ***_cin3d;
  fl32 **_cape;
  fl32 **_cin;
  fl32 ***_lcl3d;
  fl32 ***_lfc3d;
  fl32 ***_el3d;
  fl32 **_lcl;
  fl32 **_lfc;
  fl32 **_el;


  // raw 2d fields - cross grid.

  fl32 **_surfP;   // surface pressure (in Pa) PSFC
  fl32 **_land_mask;   // LANDMASK

  fl32 **_ground_t;    // Ground Temperature deg K
  fl32 **_rainc;    // Accumulated Total Cumulus Precipitation (mm) 
  fl32 **_rainnc;    // Accumulated Total Grid Scale Precipitation (mm)
  fl32 **_terrain;     // Terrain Height - meters
  fl32 **_lat;
  fl32 **_lon;
  fl32 **_land_use;   
  fl32 **_snowcovr;
  fl32 **_tseasfc;  //sea surface temp in K
  fl32 **_pbl_hgt;
  fl32 **_hfx;  //UPWARD HEAT FLUX AT THE SURFACE
  fl32 **_lh;  //LATENT HEAT FLUX AT THE SURFACE
  fl32 **_snow_we; //SNOW WATER EQUIVALENT
  fl32 **_snow_nc; //ACCUMULATED TOTAL GRID SCALE SNOW AND ICE
  fl32 **_graupel_nc; //ACCUMULATED TOTAL GRID SCALE GRAUPEL
  fl32 **_soil_t_1;	//soil temperatures
  fl32 **_soil_t_2;
  fl32 **_soil_t_3;
  fl32 **_soil_t_4;
  fl32 **_soil_t_5;
  fl32 **_soil_m_1;	//soil moisture
  fl32 **_soil_m_2;
  fl32 **_soil_m_3;
  fl32 **_soil_m_4;
  fl32 **_soil_m_5;
  fl32 _soil_d_1;	//soil depth (corresponds to moisture & temp)
  fl32 _soil_d_2;
  fl32 _soil_d_3;
  fl32 _soil_d_4;
  fl32 _soil_d_5;

  fl32 **_soil_am_1;  //available moisture
  fl32 **_soil_am_2;
  fl32 **_soil_am_3;
  fl32 **_soil_am_4;
  fl32 **_soil_am_5;

  vector<fl32> _soilWiltMoist; //store wilt moisture from SOILPARM.TBL
  vector<fl32> _soilSatMoist; //store saturation moisture from SOILPARM.TBL


  fl32 **_soil_type;    //dominant soil category - ISLTYP
  fl32 **_twp;
  fl32 **_rwp;
  fl32 **_vil;

  fl32 **_sfcrnoff;
  fl32 **_t2;
  fl32 **_q2;
  fl32 **_u10;
  fl32 **_v10;
  fl32 **_snowh;
  fl32 **_th2;  // POT TEMP at 2 M

  // derived 2d fields - cross grid.
  
  fl32 **_gridRotation; // grid rotation relative to TN
  fl32 **_t2c;
  fl32 **_fzlevel;
  fl32 **_rain_total;
  fl32 **_dbz_2d;
  fl32 **_rh2;
  fl32 **_wspd10;
  fl32 **_wdir10;
  fl32 **_q2_g;

  // raw 2d GEOGRID fields

  fl32 **_landusef1;
  fl32 **_landusef2;
  fl32 **_landusef6;
  fl32 **_landusef15;

  fl32 **_greenfrac7;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  // This function sets the above keys depending on the _inputFileType

  void _setAttKeys();

  //helper functions for _printFile
  void _printAtt(Nc3Att *att);
  void _printVarVals(Nc3Var *var);

  // determine if we are looking at a normal WRF file or a 
  // file created by the WRF preprocessing system (WPS) program,
  // geogrid 
  // returns 1 if it could determine the file type, 0 if file
  // type is unknown.
  int _findFileVersion();

  //sets times from the filename and netcdf files
  bool _setTimes();

  //set the projection from MAP_PROJ in _netcdf.MAP_PROJ
  //also set other projection paramaters
  void _setProjection();


  //this function sets the _gridMinX & _gridMinY
  // _projType, _projCenterLat, _projCenterLon, _llLon, & _llLat must
  // be set before calling _setMinXY().
  int _setMinXY();

  //sets dims indexes (_nLat, _nLon, _nEta)
  void _setDims();


  void _load1dField(const string &ncd_field_name,
		    const int neta,
		    fl32 * &field);
  void _load1dField(const string &ncd_field_name,
		    fl32 * &field);
  
  void _load2dField(const string &ncd_field_name,
		    const int nlat,
		    const int nlon,
		    fl32 ** &field);
  void _load2dField(const string &ncd_field_name,
		    fl32 ** &field);
  
  void _load3dField(const string &ncd_field_name,
		    const int neta,
		    const int nlat,
		    const int nlon,
		    fl32 *** &field);
  void _load3dField(const string &ncd_field_name,
		    fl32 *** &field);


  //loading of 3d fields [W][[V][U] [sig][lat][lon]
  void _loadUCWind();		// _uu_C
  void _loadUWind();		// _uu
  void _loadVCWind();		// _vv_C
  void _loadVWind();		// _uu
  void _loadWCWind();		// _ww_C
  void _loadWWind();		// _ww
  void _loadTemp();		// _ppt
  void _loadWaterMixingRatio(); // _qq
  void _loadCloudMixingRatio(); // _clw
  void _loadRainMixingRatio();	// _rnw
  void _loadSnowMixingRatio();	// _snow
  void _loadIceMixingRatio();	// _ice
  void _loadGraupelMixingRatio();   // _graupel
  void _loadNRain();              // _nRain
  void _loadNCloud();           // _nCloud

  void _loadPertPres();		// _pp
  void _loadBSP();		// _pb
  void _loadPertGeoC();		// _ph_C
  void _loadPertGeo(); 		// _ph
  void _loadBSPGC(); 		// _phb_C
  void _loadBSPG(); 		// _phb
  void _loadDNW();              // _DNW
  void _loadMUB();              // _MUB
  void _loadMU();               // _MU
  void _loadREFL3D();           // _REFL3D

  void _loadItfadef();

  //loading of 2d fields [V][U] [lat][lon]
  void _loadLat();
  void _loadLon();
  void _loadT2(); //Tk at 2 meters - _t2
  void _loadQ2(); //water mixing at 2 meters - _q2
  void _loadU10(); //U at 10 meters - _u10
  void _loadV10(); //V at 10 meters - _v10
  void _loadPBLHeight(); // _pbl_hgt
  void _loadSurfaceRunoff(); // _sfcrnoff
  void _loadSeaSurfaceTemp(); // _tseasfc
  void _loadGroundTemperature(); // _ground_t
  void _loadSoilInfo(); // _soil_t_*, _soil_m_*, _soil_d_*
 //helper function for _loadSoilInfo()
  void _loadASoil(string WRF_var_name, fl32**& soil_data, int layer);
  void _loadTerrainHeight(); // _terrain -  HGT
  void _loadSnowHeight(); // _snowh - SNOWH
  void _loadLandMask(); // _land_mask
  void _loadSnowCover(); // _snowcovr -- SNOWC
  void _loadLandUse(); // _land_use -- LU_INDEX
  void _loadSurfacePressure();	// _surfP 
  void _loadRainC(); // _rainc
  void _loadRainNC(); // _rainnc
  void _loadTH2(); // _th2
  void _loadHFX(); // _hfx
  void _loadLH(); // _lh
  void _loadSnowWE(); // _snow_we
  void _loadSnowNC(); // _snow_nc
  void _loadGraupelNC(); // _graupel_nc

  void _loadSoilType(); // _soil_type

  void _loadLandUsef(); //_landusef1 _landusef2 _landusef6 _landusef15
  void _loadGreenFrac7(); //_greenfrac7

 //loading of 1d fields
  void _loadHalfEta();
  void _loadZnw();

  //compute derived fields
  void _computeTemp();  // _tk
  void _computeTempC();	// _tc
  void _computePressure(); //_pres
  void _computeRh();	// _rh
  void _computeDewPt(); // _dewpt
  void _computeT2C();  //T at 2 meters in C
  void _computeFzlevel(); // _fzlevel
  void _computeDbz2DField(); // _dbz_2d
  void _computeDbz3DField(); // _dbz_3d
  void _computeTotalRain();  // _rain_total
  void _computeSpecHumidity(); // _spec_h
  void _computeSpecHumidity2m(); // _spec_h2m
  void _computeCLW_GField(); // _clw_g
  void _computeRNW_GField(); // _rnw_g
  void _computeTHETAField(); // _theta
  void _computeWspdDirnField(); //_wspd, _wdir
  void _computeSpeedDir10(); // _wspd10,_wdir10
  void _computeGeopot(); //
  void _computeGeopotHeight(); //
  void _computeAvailMoist(); //  _soil_am_N;
  //helper functions for _computeAvailMoist()
  void _computeSingleAM(fl32**&am_soil_data, fl32**moist_soil_data, int layer);
  void _loadSoilParamFile(ifstream &spt);
  void _loadDefaultSoilParams();
  void _computeRH2(); // _rh2
  fl32 _find2MPres(int lat, int lon); //helper function for _rh2
  void _computeIcing();
  void _computeCAPECIN_3D();

  void _computeHeightField();
  void _computeTotalWaterPathFields();
  
  void _computeCAPECIN_2D();  //compute 2-D cape,cin,el,lcl, &lfc
  void _scale3DField(fl32*** field, double factor, int nz); //used to get additive inverse of CIN field
  void _scale2DField(fl32** field, double factor); //used to get additive inverse of CIN field

  void _computeQ_GField();  //convert _qq from kg/kg to g/kg  

 
 
  void _loadQ2_GField();

  void _computeGridRotation();

  void _computeUVRelToTN();


  // Free the contained data grids.

  void _freeGrids();
  
};

#endif

