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

#ifndef RADARWIND_H
#define RADARWIND_H

using namespace std;

class RadarWind {
  
public:

static const int HANDLE_BBOX      = 1;
static const int HANDLE_GETSTATS  = 2;

static const int MISS_PARM = -99999;

static const int TESTMODE_ALPHA          = 1;
static const int TESTMODE_BETA           = 2;
static const int TESTMODE_ZETA           = 3;
static const int TESTMODE_ZETA_BELTRAMI  = 4;


static const long SYNFUNC_START = -999900;
static const long SYNFUNC_SINX  = -999901;
static const long SYNFUNC_SINY  = -999902;
static const long SYNFUNC_SINZ  = -999903;


static void badparms( const string msg, ...);
  

RadarWind( int argc, const char **argv);


~RadarWind();


static void readRadarFile(
  long bugs,
  int testMode,        // one of TESTMODE_*
  double * synWinds,    // user specified winds
  int ndim,            // == 3
  long forceOk,        // if != 0, force all point ncp and dbz to ok
  const GeographicLib::Geodesic * geodesic,
  const GeographicLib::TransverseMercatorExact * tranMerc,
  long ifile,
  FileSpec * fspec,
  string * radialName,
  string * dbzName,
  string * ncpName,
  double projLat0,
  double projLon0,
  double basey,
  double basex,
  double maxDeltaAltKm,
  double maxElevDeg,
  double minRadialDistKm,
  Bbox * aircraftBbox,              // overall bounding box of aircraft locs
  Bbox * pointBbox,                 // overall bounding box of point locs
  Statistic * statVg,               // statistics for radial velocity
  Statistic * statDbz,              // statistics for DBZ
  Statistic * statNcp,              // statistics for net coherent power
  vector<Point *> *pointVec,
  double *maxAbsErrHoriz,
  double *maxAbsErrVert,

  double zgridmin,
  double zgridmax,
  double ygridmin,
  double ygridmax,
  double xgridmin,
  double xgridmax,

  double &timeMin,                  // returned
  double &timeMax,                  // returned
  double * detailSpec);


static void calcAllVU(
  long bugs,
  int testMode,                // one of TESTMODE_*
  double * synWinds,
  long ndim,                   // == 3
  long maxNumNbr,              // max num nearest nbrs
  double maxDistBase,          // max pt dist = base + factor*aircraftDist
  double maxDistFactor,        // max pt dist = base + factor*aircraftDist
  vector<Point *> *pointVec,   // all observations
  KD_tree * radarKdTree,       // nearest nbr tree for pointVec
  double baseW,                // W wind into the lowest layer
  long nradz,                  // grid z dim
  double zgridmin,
  double zgridmax,
  double zgridinc,
  long nrady,                  // grid y dim
  double ygridmin,
  double ygridmax,
  double ygridinc,
  long nradx,                  // grid x dim
  double xgridmin,
  double xgridmax,
  double xgridinc,
  Cell *** & cellMat,          // we set Cell.uu, vv
  bool useEigen,               // true: use Eigen.  false: use Cramer
  double * detailSpec);


static void calcCellVU(
  long bugs,
  int testMode,                  // one of TESTMODE_*
  double * synWinds,
  long ndim,                     // == 3
  KD_real * centerLoc,           // query point
  long maxNumNbr,                // max num nearest nbrs
  double maxDistBase,            // max pt dist = base + factor*aircraftDist
  double maxDistFactor,          // max pt dist = base + factor*aircraftDist
  vector<Point *> *pointVec,     // all observations
  KD_tree * radarKdTree,         // nearest nbr tree for pointVec
  double * detailSpec,
  Cell * pcell,                  // we fill vv, uu.
  bool useEigen);                // true: use Eigen.  false: use Cramer


static void calcAllW(
  long bugs,
  int testMode,                // one of TESTMODE_*
  double * synWinds,
  long ndim,                   // == 3
  double baseW,                // W wind into the lowest layer
  double epsilon,
  long nradz,                  // grid z dim
  double zgridmin,
  double zgridmax,
  double zgridinc,
  long nrady,                  // grid y dim
  double ygridmin,
  double ygridmax,
  double ygridinc,
  long nradx,                  // grid x dim
  double xgridmin,
  double xgridmax,
  double xgridinc,
  Cell *** & cellMat,          // we set Cell.ww
  double * detailSpec);


static double calcDensity( double height);


static void interpMissing(
  long bugs,
  long nradz,                  // grid z dim
  long nrady,                  // grid y dim
  long nradx,                  // grid x dim
  double ***& radmat,
  double ***& tmpmat);


static void checkInvalid(
  long bugs,
  const string msg,
  long nradz,                  // grid z dim
  long nrady,                  // grid y dim
  long nradx,                  // grid x dim
  Cell ***& cellMat);          // we set Cell.uu, vv


static void checkVerif(
  long bugs,
  int testMode,                // one of TESTMODE_*
  double * synWinds,
  long imain,
  string * outTxt,             //
  long nradz,                  // grid z dim
  double zgridmin,
  double zgridmax,
  double zgridinc,
  long nrady,                  // grid y dim
  double ygridmin,
  double ygridmax,
  double ygridinc,
  long nradx,                  // grid x dim
  double xgridmin,
  double xgridmax,
  double xgridinc,
  Cell *** & cellMat,
  double * detailSpec);



static void readVerifFile(
  long bugs,
  double epsilon,
  string projName,           // projection name
  double projLat0,           // projection lat0
  double projLon0,           // projection lon0
  string inVerif,            // input file
  long & zverNum,            // returned
  double & zvergridmin,      // returned
  double & zvergridmax,      // returned
  double & zvergridinc,      // returned
  long & yverNum,            // returned
  double & yvergridmin,      // returned
  double & yvergridmax,      // returned
  double & yvergridinc,      // returned
  long & xverNum,            // returned
  double & xvergridmin,      // returned
  double & xvergridmax,      // returned
  double & xvergridinc,      // returned
  double ***& verifWmat,     // returned: we alloc and fill
  double ***& verifVmat,     // returned: we alloc and fill
  double ***& verifUmat);    // returned: we alloc and fill


static vector<FileSpec *>* readDir(
  string dirName,
  string filePrefix);


static vector<FileSpec *>* readFileList(
  string fileListName);


static void writeNetcdf(
  int bugs,
  double earthRadiusMeter,
  double flattening,
  const GeographicLib::TransverseMercatorExact * tranMerc,
  double projLat0,
  double projLon0,
  double basey,
  double basex,
  double timeMin,
  double timeMax,
  long nradz,
  double zgridmin,
  double zgridmax,
  double zgridinc,
  long nrady,
  double ygridmin,
  double ygridmax,
  double ygridinc,
  long nradx,
  double xgridmin,
  double xgridmax,
  double xgridinc,
  Cell *** & cellMat,
  string outNc);


static void checkGrid(
  const char * msg,
  double epsilon,
  long num,
  double minVal,
  double maxVal,
  double incVal);


static void calcSyntheticWinds(
  bool showDetail,
  double * synWinds,    // user specified winds
  double locz,          // wind location
  double locy,
  double locx,
  double * vels);       // returned W, V, U


static void calcBeltramiFlow(
  bool showDetail,
  double zz,            // location
  double yy,
  double xx,
  double * vels);       // returned W, V, U


static double calcRadialVelocity(
  bool showDetail,
  double thetaRad,      // polar coord angle from observer, radians
  double elevRad,       // elevation angle from observer, radians
  double * vels);       // W, V, U


static void latLonToYX(
  const GeographicLib::TransverseMercatorExact * tranMerc,
  double projLon0Deg,     // central meridian of projection
  double basey,           // coord y base, km
  double basex,           // coord x base, km
  double latDeg,          // latitude
  double lonDeg,          // longitude
  double & coordy,        // output coord y, km
  double & coordx);       // output coord x, km


static void yxToLatLon(
  const GeographicLib::TransverseMercatorExact * tranMerc,
  double projLon0Deg,     // central meridian of projection
  double basey,           // coord y base, km
  double basex,           // coord x base, km
  double coordy,          // coord y, km
  double coordx,          // coord x, km
  double & latDeg,        // output latitude
  double & lonDeg);       // output longitude


static double calcDistLocPt( double * loc, Point * pta);

static double calcDistPtPt( Point * pta, Point * ptb);

static double calcDistPtAircraft( Point * pta);


static bool testDetail(
  double zloc,
  double yloc,
  double xloc,
  double * detailSpec);        // z, y, x, delta


static long getLongRandom(
  long vmin,
  long vlim,
  random_data *randState);


static bool isOkDouble( double val);


static bool isOkFloat( float val);


static void printRunTime(
  const string& str,
  struct timeval * ptva);


static void addDeltaTime(
  struct timeval * ptva,
  double * psum);


static void splitString(
  const string& str,
  const string& delimiters,
  vector<string>& tokens);


static string formatTime( double dtm);

static long parseLong( string msg, string stg);

static double parseDouble( string msg, string stg);

static bool parseBool( string msg, string stg);

static void throwerr( const char * msg, ...);

}; // end class

#endif
