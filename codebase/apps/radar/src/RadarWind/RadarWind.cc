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


// Given multiple doppler observations, calculate the U, V, W winds.
//
// For sample invocations see:
//   mkwind.sh     Invoke using Eldora aircraft data
//   mkchill.sh    Invoke using CSU-Chill data
//   testa.gdb     Debug using Eldora Aircraft data
//   testb.gdb     Debug using CSU-Chill data
//
//==================================================================
// 
// Future to do:
//
// Examine: how many neighbors to use?
//
// Do 3D or 4D linear interpolation to calculate scalar means
// for dbz, ncp, etc.
//
// Maybe weight neighbors by their distance from cell centers.
// Output geometric uncertainty factors: func of angle, vg uncertainty
//
// Make and study artificial quantization errors on vg: 2^(-8) or 2^(-16)
//
// Have the radarKdTree use a different metric for horizontal distance
// than for vertical distance.
//   cvs/libs/kd/src/include/kd/kd.hh
//   cvs/libs/kd/src/kd/kd.cc
//   Add parm: double * dimWeights
//     rnnEuclidean:
//       d = dimWeights[j] * (querpoint[j] - _points[_perm[i]][j]);
//
// ==========
//
// Speed this up.  Example run times are:
//
// ./mkwind.sh  10        0.5         0.5         0.5         zetaBeltrami   0,0,0     0,0       0                3,1,-7,0.1  /d1/steves/tda/tdwind
//
//   runTime:                start: 1e-06
//   runTime:                 init: 0.000253
//   runTime:              readDir: 0.003311
//   runTime:       readRadarFiles: 43.08
//   runTime:         build KD mat: 0.27992
//   runTime:        build KD tree: 3.0377
//   runTime:        alloc cellMat: 0.13854
//   runTime:            calcAllVU: 542.88
//   runTime:             calcAllW: 0.70067
//   runTime:  interpMissing for W: 0.056154
//   runTime:           checkVerif: 34.11723
//
// Speed this up by:
// 1. Replace the eigen linear algebra library with a closed
// form solution.  Could use either:
//   Miller's implementation of Cramer's rule
//   Bullock's rotation of axes in 2D
//
// 2. Use a parallel approach.  Use pthreads.
// Use a thread pool of, say, the number of available processors - 1,
// assuming numProc > 1.
// In calcAllVU have a separate thread for each z layer.
// 
//
//==================================================================
// 
//
// Some rough file sizes:
//
//   Bell's synthetic:   240 files, 371,000 valid obs per file
//   garden city vortex: 269 files, 437,000 valid obs per file
//   brodzik:            365 files, 585,000 valid obs per file
//
// If we have 1000 files, at 500,000 obs per file, that's 5e8 obs.
// Each obs has:
//   lat, lon, alt, vg, dbz, ncp
//   about 6 * 4 = 24 bytes
//   Plus some memory allocation overhead.
// 
// So the total memory in this case is about 1.2e10 bytes.
//
//
//==================================================================
// 
//
// Eldora fields:
//   name: VT   longName: Radial Velocity, Combined              
//   name: ZZ   longName: Radar Reflectivity Factor, Combined    
//   name: VV   longName: Radial Velocity, Combined              
//   name: NCP  longName: Normalized Coherent Power, Combined    
//   name: SW   longName: Spectral Width, Combined               
//   name: DBZ  longName: Radar Reflectivity Factor, Combined    
//   name: VR   longName: Radial Velocity, Combined              
//   name: VG   longName: Ground relative velocity
//   name: GG   longName: Ground Gates
//   name: SWZ  longName: Ratio
// 
// 
// CSU-Chill fields:
//   name: DZ  longName: reflectivity
//   name: VE  longName: radial velocity
//   name: DR  longName: differential reflectivity
//   name: DP  longName: differential phase
//   name: RH  longName: correlation H-to-V
//   name: LH  longName: Linear depolarization ratio, v-rx
//   name: LV  longName: Linear depolarization ratio, v-tx, h-rx
//   name: NC  longName: normalized coherent power
//   name: CH  longName: mag correlation HH to VH
//   name: CV  longName: mag correlation VV to HV
//   name: XH
//   name: XV
//
//==================================================================
// 





#include <algorithm>
#include <cerrno>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <istream>
#include <map>
#include <ctype.h>
#include <math.h>
#include <regex.h>
#include <sstream>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <vector>

#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>



// See Radx doc at:
// http://www.ral.ucar.edu/projects/titan/docs/radial_formats/

#include <Radx/Radx.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxVol.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/DoradeRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>



// See kdtree source at:
//   cvs/libs/kd/src/include/kd/kd.hh
//   cvs/libs/kd/src/kd/kd.cc

#include <kd/kd.hh>


// NetCDF: See doc at:
//   netcdfC/tda/netcdf-4.1.3/examples/CXX/pres_temp_4D_wr.cpp
//   /usr/local/netcdf4/include/netcdfcpp.h
#include <Ncxx/Nc3File.hh>


// GeographicLib.  See doc at:
//   http://geographiclib.sourceforge.net
//   http://geographiclib.sourceforge.net/html/organization.html
//   http://geographiclib.sourceforge.net/html/geodesic.html
//   http://geographiclib.sourceforge.net/html/classGeographicLib_1_1TransverseMercatorExact.html

#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/TransverseMercatorExact.hpp>

// Eigen linear algebra library
// http://eigen.tuxfamily.org/index.php?title=Main_Page
#include <Eigen/Dense>


#include "Bbox.hh"
#include "Cell.hh"
#include "FileSpec.hh"
#include "Point.hh"
#include "Statistic.hh"
#include "RadarWind.hh"

using namespace std;


//==================================================================

// Driver

int main( int argc, const char * argv[]) {

  RadarWind * rwind = new RadarWind( argc, argv);
  return 0;
}


//==================================================================


// Print error message and usage info, and exit 1.

void RadarWind::badparms( const string msg, ...) {
  va_list vaList;
  va_start( vaList, msg);
  long bufLen = 10000;
  char * buf = new char[bufLen];
  vsnprintf( buf, bufLen, msg.c_str(), vaList);
  va_end( vaList);

  cout << "RadarWind: error:" << endl;
  cout << buf << endl;
  cout << "Parameters:\n" << endl;
  cout << "  -bugs           debug level" << endl;
  cout << endl;
  cout << "  -testMode       test mode:" << endl;
  cout << endl;
  cout << "     alpha:" << endl;
  cout << "       Use simple synthetic wind data." << endl;
  cout << "       Skip the radar files." << endl;
  cout << "       Must specify synWinds: W,V,U" << endl;
  cout << "       Obs are located at the verif cell centers." << endl;
  cout << endl;
  cout << "     beta:" << endl;
  cout << "       Use simple synthetic wind data." << endl;
  cout << "       Read swp files and use swp obs locations," << endl;
  cout << "       but replace radial vel with synthetic." << endl;
  cout << "       Must specify synWinds: W,V,U" << endl;
  cout << "       radFiles beg,lim" << endl;
  cout << "       Use radar files i: beg <= i < lim." << endl;
  cout << "       If beg==lim==0, read all files" << endl;
  cout << endl;
  cout << "     zeta:" << endl;
  cout << "       Operational: use specified radar data." << endl;
  cout << "       Specify synWinds: 0,0,0" << endl;
  cout << "       radFiles beg,lim" << endl;
  cout << "       Use radar files i: beg <= i < lim." << endl;
  cout << "       If beg==lim==0, read all files" << endl;
  cout << endl;
  cout << "     zetaBeltrami:" << endl;
  cout << "       Like zeta, but compare the results with" << endl;
  cout << "       the test Beltrami flow and print statistics." << endl;
  cout << "       Specify synWinds: 0,0,0" << endl;
  cout << "       This is only useful for swp files generated" << endl;
  cout << "       by Michael Bell's Beltrami flow simulation." << endl;
  cout << endl;
  cout << endl;
  cout << "  -synWinds       Specify wind field for any testMode" << endl;
  cout << "                    other than zeta." << endl;
  cout << "                    Specify a comma sep triplet: W,V,U" << endl;
  cout << "                    If a wind spec is numeric," << endl;
  cout << "                      use that constant uniform value." << endl;
  cout << "                    Else the wind spec is the name for" << endl;
  cout << "                      one of SYNFUNC_* values:" << endl;
  cout << "                      \"sinx\": wind component = sin(locx)" << endl;
  cout << "                      \"siny\": wind component = sin(locy)" << endl;
  cout << "                      \"sinz\": wind component = sin(locz)" << endl;
  cout << "                    Example:" << endl;
  cout << "                      -synWinds 3,sinx,sinx" << endl;
  cout << "                      Means uniform Z wind at 3 m/s." << endl;
  cout << "                      Both V and U winds = sin(locx)," << endl;
  cout << "                        so the wind would be from the SW" << endl;
  cout << "                        to the NW, varying as sin(locx)." << endl;
  cout << endl;
  cout << "  -zgrid          z grid spec: min,max,inc" << endl;
  cout << "                    To get min,max from the data," << endl;
  cout << "                    specify only one value: increment" << endl;
  cout << "  -ygrid          like zgrid" << endl;
  cout << "  -xgrid          like zgrid" << endl;
  cout << endl;
  cout << "  -projName       projection name: must be transverseMercator" << endl;
  cout << "  -projLat0       projection lat0.  For example 16.5" << endl;
  cout << "  -projLon0       projection lon0.  For example 148.0" << endl;
  cout << endl;
  cout << "  -baseW          W wind into the base of the lowest layer" << endl;
  cout << "  -epsilon        epsilon" << endl;
  cout << endl;
  cout << "  -maxDeltaAltKm  max abs delta altitude of observations" << endl;
  cout << "                  from aircraft, km, or 0";
  cout << endl;
  cout << "  -maxAbsElevDeg  max abs elevation angle observations," << endl;
  cout << "                  degrees, or 0";
  cout << endl;
  cout << "  -minRadialDistKm  min radial distance of observations" << endl;
  cout << "                  from aircraft, km, or 0";
  //xxxxa
  cout << endl;
  cout << "  -numNbrMax      max num nearest nbrs" << endl;
  cout << "  -maxDistBase    max pt dist = base + factor*aircraftDist" << endl;
  cout << "  -maxDistFactor  max pt dist = base + factor*aircraftDist" << endl;
  cout << endl;
  cout << "  -forceOk        y/n: force ncp and dbz to ok on all points" << endl;
  cout << "  -useEigen       y/n: y: use Eigen.  n: use Cramer" << endl;
  cout << endl;
  cout << "  -inDir          dir with radx input files" << endl;
  cout << "                  Used airborne radars." << endl;
  cout << "                  Mutually exclusive with -fileList." << endl;
  cout << "  -fileRegex      regex for files in inDir, like '^swp'" << endl;
  cout << "                  Mutually exclusive with -fileList." << endl;

  cout << endl;
  cout << "  -fileList       file containing list of input files." << endl;
  cout << "                  Used for fixed location radars." << endl;
  cout << "                  Mutually exclusive with -inDir, -fileRegex." << endl;
  cout << "                  Format is one entry per line." << endl;
  cout << "                  # in the first column starts a comment." << endl;
  cout << "                  Each line has the format:" << endl;
  cout << "                    fileName altKmMsl latDeg lonDeg" << endl;
  cout << "                   where altKmMsl latDeg lonDeg" << endl;
  cout << "                   give the location of the fixed radar." << endl;
  cout << endl;
  cout << "  -radialName     Name of the radial velocity field" << endl;
  cout << "                  in the input files." << endl;
  cout << "                  For Eldora, this is VG." << endl;
  cout << "                  For CSU-Chill, this is VE." << endl;
  cout << endl;
  cout << "  -dbzName        Name of the reflectivity field" << endl;
  cout << "                  in the input files." << endl;
  cout << "                  For Eldora, this is DBZ." << endl;
  cout << "                  For CSU-Chill, this is DZ." << endl;
  cout << endl;
  cout << "  -ncpName        Name of the net coherent power field" << endl;
  cout << "                  in the input files." << endl;
  cout << "                  For Eldora, this is NCP." << endl;
  cout << "                  For CSU-Chill, this is NC." << endl;
  cout << endl;
  cout << "  -outTxt         output text file:" << endl;
  cout << "                    has verif or grid results." << endl;
  cout << endl;
  cout << "  -outNc          output netcdf file" << endl;
  cout << "                    If outNc ends in a slash, \"x/\"," << endl;
  cout << "                    it is a directory." << endl;
  cout << "                    We make a subdir and write to:" << endl;
  cout << "                    x/yyyymmdd/ncf_yyyymmdd_hhmmss.nc" << endl;
  cout << endl;
  cout << "  -detailSpec     z,y,x,radius     (optional)" << endl;
  cout << "                  If none, it's the same as omitting it." << endl;
  cout << "                  Specifies a region for detailed logging." << endl;
  cout << "                  Used for debugging only." << endl;
  cout << endl;
  cout << endl;

  exit(1);
}


//==================================================================


// Constructor: does everything.
//
// Acquire command line parms.
// Call readRadarFile to read swp radar files.
// Or if TESTMODE_ALPHA, generate synthetic data.
//
// Generate a grid of Cells.
// Call calcAllVU to calculate V and U winds.
// Call calcAllW to calculate W winds.
// Call checkVerif to calc verification deltas and statistics (if testing),
// and to write a text format of the cellMat to the outTxt file.
// Call writeNetcdf to write the variables in the cellMat
// to the Netcdf file outNc.



int cntTimeh = 0;
double sumTimeh = 0;
int cntTimei = 0;
double sumTimei = 0;
int cntTimej = 0;
double sumTimej = 0;
int cntTimek = 0;
double sumTimek = 0;
int cntTimel = 0;
double sumTimel = 0;
int cntTimem = 0;
double sumTimem = 0;
int cntTimen = 0;
double sumTimen = 0;

RadarWind::RadarWind( int argc, const char **argv)
{
  long bugs = MISS_PARM;
  int testMode = MISS_PARM;
  double * synWinds = NULL;
  long * radFiles = NULL;

  // Init timing
  struct timeval timea;
  if (gettimeofday( &timea, NULL) != 0) throwerr("gettimeofday err");
  printRunTime("start", &timea);

  double zgridmin = MISS_PARM;
  double zgridmax = MISS_PARM;
  double zgridinc = MISS_PARM;

  double ygridmin = MISS_PARM;
  double ygridmax = MISS_PARM;
  double ygridinc = MISS_PARM;

  double xgridmin = MISS_PARM;
  double xgridmax = MISS_PARM;
  double xgridinc = MISS_PARM;

  string projName = "";
  double projLat0 = MISS_PARM;
  double projLon0 = MISS_PARM;

  double baseW = MISS_PARM;
  double epsilon = MISS_PARM;
  double maxDeltaAltKm = MISS_PARM;
  double maxAbsElevDeg = MISS_PARM;
  double minRadialDistKm = MISS_PARM;
  long numNbrMax = MISS_PARM;
  double maxDistBase = MISS_PARM;
  double maxDistFactor = MISS_PARM;
  bool forceOk = false;
  bool useEigen = false;
  //xxx also spec max dist of observation from cell center
  string inDir = "";
  string fileRegex = "";
  string fileList = "";
  string radialName = "";
  string dbzName = "";
  string ncpName = "";
  string outTxt = "";
  string outNc = "";
  double * detailSpec = NULL;

  if (argc % 2 != 1) badparms("args must be key/value pairs");
  for (long iarg = 1; iarg < argc; iarg += 2) {
    string key = argv[iarg];
    string val = argv[iarg+1];
    if (key == "-bugs") bugs = parseLong("bugs", val);
    else if (key == "-testMode") {
      if (val == "alpha") testMode = TESTMODE_ALPHA;
      else if (val == "beta") testMode = TESTMODE_BETA;
      else if (val == "zeta") testMode = TESTMODE_ZETA;
      else if (val == "zetaBeltrami") testMode = TESTMODE_ZETA_BELTRAMI;
      else badparms("unknown testMode");
    }

    else if (key == "-synWinds") {
      vector<string> tokvec;
      splitString( val, ",", tokvec);
      if (tokvec.size() != 3) badparms("invalid synWinds");
      synWinds = new double[ 3];
      for (int ii = 0; ii < 3; ii++) {
        string tok = tokvec.at(ii);
        if (isdigit(tok.at(0)) || tok.at(0) == '-')
          synWinds[ii] = parseDouble("synWinds", tokvec.at(ii));
        else if (tokvec.at(ii) == "sinz") synWinds[ii] = SYNFUNC_SINZ;
        else if (tokvec.at(ii) == "siny") synWinds[ii] = SYNFUNC_SINY;
        else if (tokvec.at(ii) == "sinx") synWinds[ii] = SYNFUNC_SINX;
        else throwerr("unknown synWinds function");
        cout << "  synWinds[" << ii << "]: " << synWinds[ii] << endl;
      }
    }

    else if (key == "-radFiles") {
      vector<string> tokvec;
      splitString( val, ",", tokvec);
      if (tokvec.size() != 2) badparms("invalid radFiles");
      radFiles = new long[ 2];
      for (int ii = 0; ii < 2; ii++) {
        string tok = tokvec.at(ii);
        radFiles[ii] = parseLong("radFiles", tokvec.at(ii));
        cout << "  radFiles[" << ii << "]: " << radFiles[ii] << endl;
      }
    }

    else if (key == "-zgrid") {
      vector<string> tokvec;
      splitString( val, ",", tokvec);
      if (tokvec.size() == 1) {
        zgridinc = parseDouble("zgridinc", tokvec.at(0));
      }
      else if (tokvec.size() == 3) {
        zgridmin = parseDouble("zgridmin", tokvec.at(0));
        zgridmax = parseDouble("zgridmax", tokvec.at(1));
        zgridinc = parseDouble("zgridinc", tokvec.at(2));
      }
      else badparms("invalid zgrid");
    }

    else if (key == "-ygrid") {
      vector<string> tokvec;
      splitString( val, ",", tokvec);
      if (tokvec.size() == 1) {
        ygridinc = parseDouble("ygridinc", tokvec.at(0));
      }
      else if (tokvec.size() == 3) {
        ygridmin = parseDouble("ygridmin", tokvec.at(0));
        ygridmax = parseDouble("ygridmax", tokvec.at(1));
        ygridinc = parseDouble("ygridinc", tokvec.at(2));
      }
      else badparms("invalid ygrid");
    }

    else if (key == "-xgrid") {
      vector<string> tokvec;
      splitString( val, ",", tokvec);
      if (tokvec.size() == 1) {
        xgridinc = parseDouble("xgridinc", tokvec.at(0));
      }
      else if (tokvec.size() == 3) {
        xgridmin = parseDouble("xgridmin", tokvec.at(0));
        xgridmax = parseDouble("xgridmax", tokvec.at(1));
        xgridinc = parseDouble("xgridinc", tokvec.at(2));
      }
      else badparms("invalid xgrid");
    }

    else if (key == "-projName") projName = val;
    else if (key == "-projLat0") projLat0 = parseDouble("projLat0", val);
    else if (key == "-projLon0") projLon0 = parseDouble("projLon0", val);

    else if (key == "-baseW") baseW = parseDouble("baseW", val);
    else if (key == "-epsilon") epsilon = parseDouble("epsilon", val);
    else if (key == "-maxDeltaAltKm")
      maxDeltaAltKm = parseDouble("maxDeltaAltKm", val);
    else if (key == "-maxAbsElevDeg")
      maxAbsElevDeg = parseDouble("maxAbsElevDeg", val);
    else if (key == "-minRadialDistKm")
      minRadialDistKm = parseDouble("minRadialDistKm", val);
    else if (key == "-numNbrMax") numNbrMax = parseLong("numNbrMax", val);
    else if (key == "-maxDistBase")
      maxDistBase = parseDouble("maxDistBase", val);
    else if (key == "-maxDistFactor")
      maxDistFactor = parseDouble("maxDistFactor", val);
    else if (key == "-forceOk") forceOk = parseBool("forceOk", val);
    else if (key == "-useEigen") useEigen = parseBool("useEigen", val);

    else if (key == "-inDir") inDir = val;
    else if (key == "-fileRegex") fileRegex = val;
    else if (key == "-fileList") fileList = val;
    else if (key == "-radialName") radialName = val;
    else if (key == "-dbzName") dbzName = val;
    else if (key == "-ncpName") ncpName = val;
    else if (key == "-outTxt") outTxt = val;
    else if (key == "-outNc") outNc = val;

    else if (key == "-detailSpec") {
      if (val != "none") {
        vector<string> tokvec;
        splitString( val, ",", tokvec);
        if (tokvec.size() != 4) badparms("invalid detailSpec");
        detailSpec = new double[4];
        for (int ii = 0; ii < 4; ii++) {
          detailSpec[ii] = parseDouble("detailSpec", tokvec.at(ii));
        }
      }
    }

    else badparms("unknown parameter: \"%s\"", key.c_str());
  }
  cout << setprecision(15);
  cout << "bugs: " << bugs << endl;
  cout << "testMode: " << testMode << endl;
  cout << "zgrid: " << zgridmin << "," << zgridmax << "," << zgridinc << endl;
  cout << "ygrid: " << ygridmin << "," << ygridmax << "," << ygridinc << endl;
  cout << "xgrid: " << xgridmin << "," << xgridmax << "," << xgridinc << endl;
  cout << "projName: " << projName << endl;
  cout << "projLat0: " << projLat0 << endl;
  cout << "projLon0: " << projLon0 << endl;
  cout << "baseW: " << baseW << endl;
  cout << "epsilon: " << epsilon << endl;
  cout << "maxDeltaAltKm: " << maxDeltaAltKm << endl;
  cout << "maxAbsElevDeg: " << maxAbsElevDeg << endl;
  cout << "minRadialDistKm: " << minRadialDistKm << endl;
  cout << "numNbrMax: " << numNbrMax << endl;
  cout << "maxDistBase: " << maxDistBase << endl;
  cout << "maxDistFactor: " << maxDistFactor << endl;
  cout << "forceOk: " << forceOk << endl;
  cout << "useEigen: " << useEigen << endl;
  cout << "inDir: " << inDir << endl;
  cout << "fileRegex: " << fileRegex << endl;
  cout << "fileList: " << fileList << endl;
  cout << "radialName: " << radialName << endl;
  cout << "dbzName: " << dbzName << endl;
  cout << "ncpName: " << ncpName << endl;
  cout << "outTxt: " << outTxt << endl;
  cout << "outNc: " << outNc << endl;

  if (detailSpec == NULL) cout << "detailSpec: (none)" << endl;
  else {
    cout << "detailSpec:"
      << "  z: " << detailSpec[0] << "  y: " << detailSpec[1]
      << "  x: " << detailSpec[2] << "  delta: " << detailSpec[3] << endl;
  }

  if (bugs == MISS_PARM) badparms("parm not specified: -bugs");
  if (testMode == MISS_PARM) badparms("parameter not specified: -testMode");
  if (synWinds == NULL) badparms("parameter not specified: -synWinds");
  if (radFiles == NULL) badparms("parameter not specified: -radFiles");
  if (zgridinc == MISS_PARM) badparms("parm not specified: -zgrid");
  if (ygridinc == MISS_PARM) badparms("parm not specified: -ygrid");
  if (xgridinc == MISS_PARM) badparms("parm not specified: -xgrid");
  if (projName == "") badparms("parm not specified: -projName");
  if (projLat0 == MISS_PARM) badparms("parm not specified: -projLat0");
  if (projLon0 == MISS_PARM) badparms("parm not specified: -projLon0");
  if (baseW == MISS_PARM) badparms("parm not specified: -baseW");
  if (epsilon == MISS_PARM) badparms("parm not specified: -epsilon");
  if (maxDeltaAltKm == MISS_PARM)
    badparms("parm not specified: -maxDeltaAltKm");
  if (maxAbsElevDeg == MISS_PARM)
    badparms("parm not specified: -maxAbsElevDeg");
  if (minRadialDistKm == MISS_PARM)
    badparms("parm not specified: -minRadialDistKm");
  if (numNbrMax == MISS_PARM) badparms("parm not specified: -numNbrMax");
  if (maxDistBase == MISS_PARM) badparms("parm not specified: -maxDistBase");
  if (maxDistFactor == MISS_PARM)
    badparms("parm not specified: -maxDistFactor");
  if (testMode == TESTMODE_ZETA && forceOk)
    throwerr("forceOk not allowed with testMode zeta");

  if (inDir == "") {
    if (fileList == "") badparms("must spec either -inDir or -fileList");
    if (fileRegex != "") badparms("parm -inDir required for -fileRegex");
  }
  else {
    if (fileList != "") badparms("cannot spec both -inDir and -fileList");
    if (fileRegex == "") badparms("parm -fileRegex required with -inDir");
  }

  if (radialName == "") badparms("parameter not specified: -radialName");
  if (dbzName == "") badparms("parameter not specified: -dbzName");
  if (ncpName == "") badparms("parameter not specified: -ncpName");
  if (outTxt == "") badparms("parameter not specified: -outTxt");
  if (outNc == "") badparms("parameter not specified: -outNc");

  if (projName != "transverseMercator") badparms("unknown projName");
  printRunTime("init", &timea);



  long randSeed = 1;
  ///struct random_data randInfo;
  ///char randState[256];
  ///if (0 != initstate_r( randSeed, randState, 256, &randInfo))
  ///  throwerr("initstate_r error");
  ///if (0 != srandom_r( randSeed, &randInfo)) throwerr("srandom_r error");
  srandom( randSeed);





  // Set up geodesic model and transverseMercator model
  double earthRadiusMeter = GeographicLib::Constants::WGS84_a<double>();
  //double earthRadiusMeter = 6378137;

  // flattening of WGS84 ellipsoid (1/298.257223563).
  double flattening = GeographicLib::Constants::WGS84_f<double>();
  //double flattening = 1/298.257223563;

  const GeographicLib::Geodesic * geodesic = new GeographicLib::Geodesic(
    earthRadiusMeter, flattening);

  const GeographicLib::TransverseMercatorExact * const tranMerc =
    & GeographicLib::TransverseMercatorExact::UTM;


  // Given projLon0 = 148.0, projLat0 = 16.5,
  // calculate basex = 0, basey = 1.82424e+06 / 1000 = 1842
  double basey;
  double basex;
  latLonToYX(
    tranMerc,          // TransverseMercatorExact
    projLon0,          // central meridian of projection
    0,                 // basey: coord y base, km
    0,                 // basex: coord x base, km
    projLat0,          // input lat
    projLon0,          // input lon
    basey,             // output value
    basex);            // output value

  cout << setprecision(15);
  cout << "readRadarFile: basex: " << basex << endl;
  cout << "readRadarFile: basey: " << basey << endl;



  // Read and parse input verification file, if specified
  vector<double> * verifXs = new vector<double>();
  vector<double> * verifYs = new vector<double>();
  vector<double> * verifZs = new vector<double>();
  vector<double> * verifUs = new vector<double>();
  vector<double> * verifVs = new vector<double>();
  vector<double> * verifWs = new vector<double>();

  long ndim = 3;


  // NOTE: inVerif and readVerifFile are no longer used.
  // Now we calculate the verification values internally.
  //
  // If inVerif specified, read the verification file
  //
  // inVerif file format:
  //
  // The inVerif file consists of a header,
  // with one attribute and value per line, followed
  // by a data section with one point per line.
  // Comments start with #.
  // Items in angle brackets <> are placeholders.
  // for actual values.
  //
  // projection: transverseMercator <lon0> <lat0>
  // z: <numz> <minz> <maxz> <incz>
  // y: <numy> <miny> <maxy> <incy>
  // x: <numx> <minx> <maxx> <incx>
  // data:
  // <x> <y> <z> <u> <v> <w>
  // <x> <y> <z> <u> <v> <w>
  // ...

  if (false) {
    string inVerif = NULL;       // verification input file name
    long zverNum = 0;
    double zvergridmin = 0;
    double zvergridmax = 0;
    double zvergridinc = 0;
    long yverNum = 0;
    double yvergridmin = 0;
    double yvergridmax = 0;
    double yvergridinc = 0;
    long xverNum = 0;
    double xvergridmin = 0;
    double xvergridmax = 0;
    double xvergridinc = 0;
    double *** verifUmat = NULL;
    double *** verifVmat = NULL;
    double *** verifWmat = NULL;
    if (inVerif != "none") {
      readVerifFile(
        bugs,
        epsilon,
        projName, projLat0, projLon0,
        inVerif,
        zverNum, zvergridmin, zvergridmax, zvergridinc,    // returned values
        yverNum, yvergridmin, yvergridmax, yvergridinc,    // returned values
        xverNum, xvergridmin, xvergridmax, xvergridinc,    // returned values
        verifWmat, verifVmat, verifUmat);                  // returned values
    }
    printRunTime("readVerifFile", &timea);
  }



  vector<Point *> *pointVec = NULL;
  KD_tree * radarKdTree = NULL;

  Bbox * aircraftBbox = new Bbox();   // overall bounding box of aircraft locs
  Bbox * pointBbox = new Bbox();      // overall bounding box of point locs
  double timeMin = numeric_limits<double>::quiet_NaN();
  double timeMax = numeric_limits<double>::quiet_NaN();

  if (testMode == TESTMODE_ALPHA) {
    // Fill pointVec with all the observations.
    // Find the overall bounding box.

    if (zgridmin == MISS_PARM) badparms("parm not specified: -zgrid");
    if (ygridmin == MISS_PARM) badparms("parm not specified: -ygrid");
    if (xgridmin == MISS_PARM) badparms("parm not specified: -xgrid");
    if (zgridmax == MISS_PARM) badparms("parm not specified: -zgrid");
    if (ygridmax == MISS_PARM) badparms("parm not specified: -ygrid");
    if (xgridmax == MISS_PARM) badparms("parm not specified: -xgrid");
    if (zgridinc == MISS_PARM) badparms("parm not specified: -zgrid");
    if (ygridinc == MISS_PARM) badparms("parm not specified: -ygrid");
    if (xgridinc == MISS_PARM) badparms("parm not specified: -xgrid");
    long nradz = lround( (zgridmax - zgridmin) / zgridinc) + 1;
    long nrady = lround( (ygridmax - ygridmin) / ygridinc) + 1;
    long nradx = lround( (xgridmax - xgridmin) / xgridinc) + 1;
    checkGrid( "radz", epsilon, nradz, zgridmin, zgridmax, zgridinc);
    checkGrid( "rady", epsilon, nrady, ygridmin, ygridmax, ygridinc);
    checkGrid( "radx", epsilon, nradx, xgridmin, xgridmax, xgridinc);

    pointVec = new vector<Point *>();

    for (long iz = 0; iz < nradz; iz++) {
      for (long iy = 0; iy < nrady; iy++) {
        for (long ix = 0; ix < nradx; ix++) {

          double centerLoc[3];
          centerLoc[0] = zgridmin + iz * zgridinc;
          centerLoc[1] = ygridmin + iy * ygridinc;
          centerLoc[2] = xgridmin + ix * xgridinc;
          bool showDetail = testDetail(
            centerLoc[0],         // z
            centerLoc[1],         // y
            centerLoc[2],         // x
            detailSpec);          // z, y, x, delta
          if (showDetail) {
            cout << "const.init: showDetail:"
              << "  iz: " << iz << "  iy: " << iy << "  ix: " << ix << endl;
            cout << "const: showDetail: centerLoc z: " << centerLoc[0] << endl;
            cout << "const: showDetail: centerLoc y: " << centerLoc[1] << endl;
            cout << "const: showDetail: centerLoc x: " << centerLoc[2] << endl;
          }

          aircraftBbox->addOb( centerLoc[0], centerLoc[1], centerLoc[2]);
          pointBbox->addOb( centerLoc[0], centerLoc[1], centerLoc[2]);

          // Make numNbrMax points, each one exactly at the cell center.
          for (long inbr = 0; inbr < numNbrMax; inbr++) {
            // We must use different thetas so the problem has a solution.
            double synThetaRad = inbr * M_PI / 4;
            double synElevRad = 0;
            double synVels[3];    // W, V, U
            calcSyntheticWinds(
              showDetail,
              synWinds,           // user specified winds
              centerLoc[0],       // locz
              centerLoc[1],       // locy
              centerLoc[2],       // locx
              synVels);           // returned W, V, U

            double velRadial = calcRadialVelocity(
              showDetail,
              synThetaRad,        // polar coord angle from observer, radians
              synElevRad,         // elevation angle from observer, radians
              synVels);           // W, V, U

            pointVec->push_back( new Point(
              0,             // ifile
              0,             // iray
              0,             // ipt
              centerLoc[0],  // aircraftz
              centerLoc[1],  // aircrafty
              centerLoc[2],  // aircraftx
              centerLoc[0],  // altKmMsl
              centerLoc[1],  // coordy
              centerLoc[2],  // coordx
              1329336556,    // UTC seconds since 1970 jan 1.
              synThetaRad,   // angle from aircraft in horiz plane from north
              synElevRad,    // angle from the aircraft between horiz and pt
              velRadial,     // radial velocity
              30.,           // dbz: reflectivity
              1.0));         // valNcp: radar net coherent power
          } // for inbr
        } // for ix
      } // for iy
    } // for iz

  } // if testMode == TESTMODE_ALPHA

  else {  // else not ALPHA
    // Fill pointVec with all the observations.
    // Find the overall bounding box.

    // Read dir to get list of radar file names.
    vector<FileSpec *>* fspecList;
    if (inDir != "") fspecList = readDir( inDir, fileRegex);
    else fspecList = readFileList( fileList);
    printRunTime("readDir", &timea);

    cout << endl << "fsubsetList:" << endl;
    vector<FileSpec *>* fsubsetList = new vector<FileSpec *>();
    for (long ifile = 0; ifile < fspecList->size(); ifile++) {
      FileSpec * fspec = fspecList->at( ifile);
      if (radFiles[0] == 0 && radFiles[1] == 0
        || ifile >= radFiles[0] && ifile < radFiles[1])
      {
        fsubsetList->push_back( fspec);
        cout << "  fsubsetList: ifile: " << ifile
          << "  fspec: " << fspec->fpath << endl;
      }
    }

    // Fill pointVec with all the observations from all files.
    // Find the overall bounding box.
    pointVec = new vector<Point *>();

    double maxAbsErrHoriz = 0;
    double maxAbsErrVert = 0;
    Statistic statVg;
    Statistic statDbz;
    Statistic statNcp;

    for (long ifile = 0; ifile < fsubsetList->size(); ifile++) {
      FileSpec * fspec = fsubsetList->at( ifile);
      cout << "main: start fpath: " << fspec->fpath << endl;

      readRadarFile(
        bugs,
        testMode,
        synWinds,
        ndim,
        forceOk,
        geodesic,
        tranMerc,
        ifile,
        fspec,
        &radialName,
        &dbzName,
        &ncpName,
        projLat0,
        projLon0,
        basey,
        basex,
        maxDeltaAltKm,
        maxAbsElevDeg,
        minRadialDistKm,
        aircraftBbox,
        pointBbox,
        &statVg,
        &statDbz,
        &statNcp,
        pointVec,                 // appended
        &maxAbsErrHoriz,
        &maxAbsErrVert,
        zgridmin, zgridmax,
        ygridmin, ygridmax,
        xgridmin, xgridmax,
        timeMin,                  // returned
        timeMax,                  // returned
        detailSpec);              // z, y, x, delta
    } // for ifile
    printRunTime("readRadarFiles", &timea);

    // xxx del maxAbsErrHoriz, vert
    cout << setprecision(5);
    cout << "final maxAbsErrHoriz: " << maxAbsErrHoriz << endl;
    cout << "final maxAbsErrVert: " << maxAbsErrVert << endl;

    cout << endl;
    cout << "aircraftBbox:" << endl;
    aircraftBbox->print();

    cout << endl;
    cout << "pointBbox:" << endl;
    pointBbox->print();
    cout << endl;

    cout << endl;
    cout << "statVg:" << endl;
    statVg.print( 7);
    cout << endl;

    cout << endl;
    cout << "statNcp:" << endl;
    statNcp.print( 7);
    cout << endl;

    cout << endl;
    cout << setprecision(20);
    cout << "timeMin: " << timeMin << endl;
    cout << "timeMax: " << timeMax << endl;
    cout << endl;

  } // else testMode != TESTMODE_ALPHA



  cout << "cntTimeh: " << cntTimeh << "  sumTimeh: " << sumTimeh << endl;
  cout << "cntTimei: " << cntTimei << "  sumTimei: " << sumTimei << endl;
  cout << "cntTimej: " << cntTimej << "  sumTimej: " << sumTimej << endl;
  cout << "cntTimek: " << cntTimek << "  sumTimek: " << sumTimek << endl;
  cout << "cntTimel: " << cntTimel << "  sumTimel: " << sumTimel << endl;
  cout << "cntTimem: " << cntTimem << "  sumTimem: " << sumTimem << endl;
  cout << "cntTimen: " << cntTimen << "  sumTimen: " << sumTimen << endl;



  // Build the KD tree for 3D radar observations
  long npt = pointVec->size();
  cout << "npt: " << npt << endl;
  KD_real **radarKdMat = new KD_real*[npt];     // npt x ndim
  for (long ii = 0; ii < npt; ii++) {
    radarKdMat[ii] = new KD_real[ndim];
    Point * pt = pointVec->at(ii);
    radarKdMat[ii][0] = pt->coordz;        // z
    radarKdMat[ii][1] = pt->coordy;        // y
    radarKdMat[ii][2] = pt->coordx;        // x
    if (ii % 1000 == 0) {
      cout << setprecision(5);
    }
  }
  printRunTime("build KD mat", &timea);
  radarKdTree = new KD_tree(
    (const KD_real **) radarKdMat,
    npt,
    ndim);
  printRunTime("build KD tree", &timea);










  // If the limits for zgrid,ygrid,xgrid were not specified,
  // then try to get them from the bounding box.
  if (pointBbox != NULL) {
    if (zgridmin == MISS_PARM) {
      zgridmin = zgridinc * floor( pointBbox->coordzMin / zgridinc);
      zgridmax = zgridinc * ceil( pointBbox->coordzMax / zgridinc);
    }
    if (ygridmin == MISS_PARM) {
      ygridmin = ygridinc * floor( pointBbox->coordyMin / ygridinc);
      ygridmax = ygridinc * ceil( pointBbox->coordyMax / ygridinc);
    }
    if (xgridmin == MISS_PARM) {
      xgridmin = xgridinc * floor( pointBbox->coordxMin / xgridinc);
      xgridmax = xgridinc * ceil( pointBbox->coordxMax / xgridinc);
    }
  } // if pointBbox != NULL
  if (zgridmin == MISS_PARM) badparms("parm not specified: -zgrid");
  if (ygridmin == MISS_PARM) badparms("parm not specified: -ygrid");
  if (xgridmin == MISS_PARM) badparms("parm not specified: -xgrid");

  // Check the grids and calc nradz, nrady, nradx.
  long nradz = lround( (zgridmax - zgridmin) / zgridinc) + 1;
  long nrady = lround( (ygridmax - ygridmin) / ygridinc) + 1;
  long nradx = lround( (xgridmax - xgridmin) / xgridinc) + 1;
  checkGrid( "radz", epsilon, nradz, zgridmin, zgridmax, zgridinc);
  checkGrid( "rady", epsilon, nrady, ygridmin, ygridmax, ygridinc);
  checkGrid( "radx", epsilon, nradx, xgridmin, xgridmax, xgridinc);
  cout << setprecision(5);
  cout << "main:" << endl;
  cout << "radz: nradz: " << nradz
       << "  zgridmin: " << zgridmin
       << "  zgridmax: " << zgridmax
       << "  zgridinc: " << zgridinc
       << endl;
  cout << "rady: nrady: " << nrady
       << "  ygridmin: " << ygridmin
       << "  ygridmax: " << ygridmax
       << "  ygridinc: " << ygridinc
       << endl;
  cout << "radx: nradx: " << nradx
       << "  xgridmin: " << xgridmin
       << "  xgridmax: " << xgridmax
       << "  xgridinc: " << xgridinc
       << endl;

  //xxx all new: delete
  // Allocate cellMat.
  // Init Cell.ww = 0.
  Cell *** cellMat = new Cell**[nradz];
  for (long iz = 0; iz < nradz; iz++) {
    cellMat[iz] = new Cell*[nrady];
    for (long iy = 0; iy < nrady; iy++) {
      cellMat[iz][iy] = new Cell[nradx];
      for (long ix = 0; ix < nradx; ix++) {
        cellMat[iz][iy][ix].ww = 0;
      }
    }
  }
  printRunTime("alloc cellMat", &timea);


  // One approach is to iterate:
  //   calculate V and U winds
  //   calculate W winds
  //   recalculate V and U winds using the new W winds
  //   recalculate W with the new V, U.
  //   etc.
  //
  // However, tests show that there is negligible gain
  // in accuracy after the first iteration.
  // So the loop limit is hardcoded at 1.

  for (int imain = 0; imain < 1; imain++) {
    bool showStat = true;
    // Based on current w, calc u, v.
    calcAllVU(
      bugs,
      testMode,
      synWinds,
      ndim,            // == 3
      numNbrMax,       // max num nearest nbrs
      maxDistBase,     // max pt dist = base + factor*aircraftDist
      maxDistFactor,   // max pt dist = base + factor*aircraftDist
      pointVec,        // all observations
      radarKdTree,     // nearest nbr tree for pointVec
      baseW,
      nradz, zgridmin, zgridmax, zgridinc,
      nrady, ygridmin, ygridmax, ygridinc,
      nradx, xgridmin, xgridmax, xgridinc,
      cellMat,
      useEigen,               // true: use Eigen.  false: use Cramer
      detailSpec);
    printRunTime("calcAllVU", &timea);

    // NO LONGER USED:
    // Interpolate missing values from neighbors.
    //
    // The problem is that some cells don't have enough valid
    // neighbors to calculate their winds, so their values are
    // left as NaN.
    //
    // The idea was to use fill the NaN cells
    // by interpolating the wind values from nearby cells.
    //
    // But what happens is that generally if a cell doesn't have
    // enough good neighbors for a valid wind calculation,
    // any neighbor cells we might use for interpolation are
    // pretty flakey.
    //xxx del interpolation?
    // Interpolate NaN values in cellMat.uu, cellMat.vv.
    //interpMissing( bugs, nradz, nrady, nradx, cellMat, tmpmat);
    //printRunTime("interpMissing for V,U", &timea);

    //xxx del:
    checkInvalid( bugs, "main A: V", nradz, nrady, nradx, cellMat);
    checkInvalid( bugs, "main A: U", nradz, nrady, nradx, cellMat);

    // Based on u, v, calc w
    calcAllW(
      bugs,
      testMode,
      synWinds,
      ndim,            // == 3
      baseW,
      epsilon,
      nradz, zgridmin, zgridmax, zgridinc,
      nrady, ygridmin, ygridmax, ygridinc,
      nradx, xgridmin, xgridmax, xgridinc,
      cellMat,
      detailSpec);
    printRunTime("calcAllW", &timea);
    checkInvalid( bugs, "main B: W", nradz, nrady, nradx, cellMat);

    // NO LONGER USED:
    // Interpolate missing values from neighbors.
    // See doc above.
    ///xxx interpMissing( bugs, nradz, nrady, nradx, cellMat, tmpmat);
    printRunTime("interpMissing for W", &timea);

    //xxx del:
    checkInvalid( bugs, "main C: W", nradz, nrady, nradx, cellMat);


    // Calc deltas using verification data, if any.
    // Write outTxt file.
    checkVerif(
      bugs,
      testMode,
      synWinds,
      imain,
      &outTxt,
      nradz, zgridmin, zgridmax, zgridinc,
      nrady, ygridmin, ygridmax, ygridinc,
      nradx, xgridmin, xgridmax, xgridinc,
      cellMat,
      detailSpec);
    printRunTime("checkVerif", &timea);


  } // for imain




  // Create netcdf file
  writeNetcdf(
    bugs, earthRadiusMeter, flattening, tranMerc,
    projLat0, projLon0, basey, basex,
    timeMin, timeMax,
    nradz, zgridmin, zgridmax, zgridinc,
    nrady, ygridmin, ygridmax, ygridinc,
    nradx, xgridmin, xgridmax, xgridinc,
    cellMat, outNc);


  // Free matrices
  for (long iz = 0; iz < nradz; iz++) {
    for (long iy = 0; iy < nrady; iy++) {
      delete[] cellMat[iz][iy];
    }
    delete[] cellMat[iz];
  }
  delete[] cellMat;


  struct rusage ruse;
  getrusage( RUSAGE_SELF, &ruse);
  cout << setprecision(5);
  cout << "final   ru_maxrss: " << ruse.ru_maxrss << endl;
  cout << "final   ru_ixrss: " << ruse.ru_ixrss << endl;
  cout << "final   ru_idrss: " << ruse.ru_idrss << endl;
  cout << "final   ru_isrss: " << ruse.ru_isrss << endl;

  return;
} // end constructor



//==================================================================



RadarWind::~RadarWind() {}


//==================================================================



// Read a radar file and append the Points to pointVec.

// Refraction calculations
// See the accompanying inkscape diagram in file refraction.svg.
//
// Let:
//   re = radius of earth, assumed spherical
//   rv = radius of curvature of the radar beam
//   elev = beam elevation angle
//         = angle of the radar beam with respect to the horizontal
//           plane through the aircraft.
//   k = constant of rv / re when elev == 0.  Typically k = 4/3.
//   ha = height of aircraft above MSL
//
// rv = k re / cos(elev)                      eqn 1
//
// Consider a 2 dim projection of the earth and aircraft with
// the earth center at (0,0), the ground beneath the aircraft at (0,re),
// and the aircraft at (0, re+hp).
// Assume the radar beam lies in the 2 dim projection
// and has angle elev with respect to the x axis.
//
// Let
//   cx = x coord of the center of the arc created by the radar beam.
//     This arc has radius rv.
//   cy = y coord of the center of the arc created by the radar beam.
//
// cx = rv * sin(elev)
//
// re + ha - cy = rv * cos(elev) = k * re          // from eqn 1
// cy = re + hp - k * re = re * (1 - k) + hp        // generally cy < 0
//
// Let
//   px,py = coords of the reflecting hydrometeoroid.
//   sp = distance along the radar beam from the aircraft to
//     the reflecting hydrometeoroid.
//   phi = angle portended at cx,cy by the arc of the radar beam
//     from the aircraft to px,py.
//   gamma = angle, at cx,cy, from the parallel to the x axis,
//     to px,py.
//
// phi = sp / rv
// gamma = pi/2 + elev - phi
//
// px = cx + rv * cos(gamma)
// py = cy + rv * sin(gamma)
//
// Let
//   hp = height of the hydrometeoroid above MSL
//   se = distance along the earth surface from the point directly under
//     the aircraft to the point directly under the reflecting hydrometeoroid.
//
// (re + hp)^2 = px^2 + py^2
// hp = sqrt( px^2 + py^2) - re
//
//
//
// Rough checks:
// angle zeta = arctan( py/px)
// se/re = pi/2 - zeta
// se = re * (pi/2 - zeta)
//
// For sp << re, the path is essentially a straight line
// at angle elev, and we should have:
//   se =about sp * cos(elev)
//   hp =about ha + sp * sin(elev)


void RadarWind::readRadarFile(
  long bugs,
  int testMode,         // one of TESTMODE_*
  double * synWinds,    // user specified winds
  int ndim,             // == 3
  long forceOk,         // if != 0, force all point ncp and dbz to ok
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
  double maxAbsElevDeg,
  double minRadialDistKm,
  Bbox * aircraftBbox,              // overall bounding box of aircraft locs
  Bbox * pointBbox,                 // overall bounding box of point locs
  Statistic * statVg,               // statistics for radial velocity
  Statistic * statDbz,              // statistics for DBZ
  Statistic * statNcp,              // statistics for net coherent power
  vector<Point *> *pointVec,        // appended
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
  double * detailSpec)              // z, y, x, delta
{
  RadxFile rxfile;
  RadxVol rxvol;
  if (0 != rxfile.readFromPath( fspec->fpath, rxvol))
    badparms("cannot read file: \"%s\"", fspec->fpath.c_str());

  double tmStart = rxvol.getStartTimeSecs() + 1.e-9 * rxvol.getStartNanoSecs();
  double tmEnd = rxvol.getEndTimeSecs() + 1.e-9 * rxvol.getEndNanoSecs();
  cout << "\n\nreadRadarFile:" << endl
    << "    fpath:         " << fspec->fpath << endl
    << "    startTime:     " << formatTime( tmStart) << endl
    << "    endTime:       " << formatTime( tmEnd) << endl
    << "    latitudeDeg:   " << rxvol.getLatitudeDeg() << endl
    << "    longitudeDeg:  " << rxvol.getLongitudeDeg() << endl
    << "    altitudeKm:    " << rxvol.getAltitudeKm() << endl;
  if (bugs >= 10) {
    cout << endl << endl << "readRadarFile: RadxVol: " << endl;
    rxvol.print( cout);
    cout << "end RadxVol" << endl;
  }



  // Get coords for debug only
  double tmpy;            // northing in meters
  double tmpx;            // easting in meters
  latLonToYX(
    tranMerc,             // TransverseMercatorExact
    projLon0,             // central meridian of projection
    basey,                // coord y base, km
    basex,                // coord x base, km
    fspec->latitudeDeg,
    fspec->longitudeDeg,
    tmpy,                 // output value
    tmpx);                // output value

  cout << endl << "readRadarFile: fspec:" << endl;
  fspec->print();
  cout << "readRadarFile: x (km): " << tmpx << endl;
  cout << "readRadarFile: y (km): " << tmpy << endl;
  cout << "set arrow " << (9000000 + ifile)
    << " from " << tmpx << "," << tmpy
    << " to " << tmpx << "," << tmpy
    << endl;

  vector<RadxRay *> rays = rxvol.getRays();
  long itotpt = 0;
  long numGoodPoint = 0;
  long numInvalidPoint = 0;
  long numMissLocPoint = 0;

  struct timeval timeh;
  addDeltaTime( &timeh, NULL);

  for (size_t iray = 0; iray < rays.size(); iray++) {
    RadxRay * ray = rays[iray];
    if (ray == NULL)
      badparms("cannot read file: \"%s\"", fspec->fpath.c_str());

    // RadxRay extends RadxRangeGeom
    double aircraftLatDeg = numeric_limits<double>::quiet_NaN();
    double aircraftLonDeg = numeric_limits<double>::quiet_NaN();
    double aircraftAltKmMsl = numeric_limits<double>::quiet_NaN();

    // gref is NULL for some ground-based stations.
    const RadxGeoref * gref = ray->getGeoreference();
    if (gref == NULL) {
      aircraftAltKmMsl = fspec->altitudeKmMsl;
      aircraftLatDeg = fspec->latitudeDeg;
      aircraftLonDeg = fspec->longitudeDeg;
    }
    else {
      aircraftLatDeg = gref->getLatitude();
      aircraftLonDeg = gref->getLongitude();
      aircraftAltKmMsl = gref->getAltitudeKmMsl();
    }
    if (std::isnan( aircraftLatDeg)
      || std::isnan( aircraftLonDeg)
      || std::isnan( aircraftAltKmMsl))
    {
      cout << "Error: incomplete file spec or radx Georeference" << endl;
      fspec->print();
      cout.flush();
      throwerr("incomplete file spec or radx Georeference");
    }

    double aircrafty;       // northing in meters
    double aircraftx;       // easting in meters
    latLonToYX(
      tranMerc,             // TransverseMercatorExact
      projLon0,             // central meridian of projection
      basey,                // coord y base, km
      basex,                // coord x base, km
      aircraftLatDeg,
      aircraftLonDeg,
      aircrafty,            // output value
      aircraftx);           // output value

    aircraftBbox->addOb( aircraftAltKmMsl, aircrafty, aircraftx);
    double rayTime = ray->getTimeDouble();
    if (std::isnan(timeMin) || rayTime < timeMin) timeMin = rayTime;
    if (std::isnan(timeMax) || rayTime > timeMax) timeMax = rayTime;

    // Print info for first ray only
    if (bugs >= 0 && iray == 0) {
      cout << setprecision(7);
      cout << "first_ray_loc:"
        << "  fpath: " << fspec->fpath
        << "  time: " << setprecision(16) << ray->getTimeDouble()
        << "  " << formatTime( ray->getTimeDouble()) << endl
        << "  aircraftAltKmMsl: " << aircraftAltKmMsl
        << "  aircraftLatDeg: " << aircraftLatDeg
        << "  aircraftLonDeg: " << aircraftLonDeg << endl
        << "  aircrafty: " << aircrafty
        << "  aircraftx: " << aircraftx << endl;
      cout << "  RadxRangeGeom:" << endl;
      cout << "  getStartRangeKm(): " << ray->getStartRangeKm() << endl;
      cout << "  getGateSpacingKm(): " << ray->getGateSpacingKm() << endl;

      cout << "  RadxRay:" << endl;
      cout << "  getAzimuthDeg(): " << ray->getAzimuthDeg() << endl;
      cout << "  getElevationDeg(): " << ray->getElevationDeg() << endl;

      if (gref != NULL) {
        cout << "  RadxGeoref:" << endl;
        cout << "  getRotation(): " << gref->getRotation() << endl;
        cout << "  getEwVelocity(): " << gref->getEwVelocity() << endl;
        cout << "  getNsVelocity(): " << gref->getNsVelocity() << endl;
        cout << "  getVertVelocity(): " << gref->getVertVelocity() << endl;
        cout << "  getHeading(): " << gref->getHeading() << endl;
        cout << "  getRoll(): " << gref->getRoll() << endl;
        cout << "  getPitch(): " << gref->getPitch() << endl;
        cout << "  getDrift(): " << gref->getDrift() << endl;
        cout << "  getRotation(): " << gref->getRotation() << endl;
        cout << "  getTilt(): " << gref->getTilt() << endl;
      }

      // http://www.ral.ucar.edu/projects/titan/docs/radial_formats/
      // Standard names for moments variables:
      //   radial_velocity_of_scatterers_away_from_instrument VEL m/s

      // Eldora: "VG".  CSU-Chill: "VE".
      RadxField * fieldVg = ray->getField( *radialName);
      if (fieldVg == NULL) throwerr("radialName not found");
      cout << endl << endl
        << "readRadarFile: first ray fieldVg print: " << endl;
      fieldVg->print( cout);
      cout << endl;

      // Eldora: "DBZ".  CSU-Chill: "DZ".
      RadxField * fieldDbz = ray->getField( *dbzName);
      if (fieldDbz == NULL) throwerr("dbzName not found");
      cout << endl << endl
        << "readRadarFile: first ray fieldDbz print: " << endl;
      fieldDbz->print( cout);
      cout << endl;

      // Eldora: "NCP".  CSU-Chill: "NC".
      RadxField * fieldNcp = ray->getField( *ncpName);
      if (fieldNcp == NULL) throwerr("ncpName not found");
      cout << endl << endl
        << "readRadarFile: first ray fieldNcp print: " << endl;
      fieldNcp->print( cout);
      cout << endl;
    } // if bugs >= 0 && iray == 0

    if (bugs >= 10) {
      cout << setprecision(15);
      cout << endl << endl << "readRadarFile: RadxRay: " << endl;
      ray->print( cout);
      cout << endl;
      cout << "end RadxRay" << endl;
      cout << "readRadarFile: iray: " << iray
        << "  rotation: " << ray->getGeoreference()->getRotation()
        << "  ray time: " << formatTime( ray->getTimeDouble()) << endl;
    }

    RadxField * fieldVg = ray->getField( *radialName);
    RadxField * fieldDbz = ray->getField( *dbzName);
    RadxField * fieldNcp = ray->getField( *ncpName);
    if (fieldVg == NULL) throwerr("radialName not found");
    if (fieldDbz == NULL) throwerr("dbzName not found");
    if (fieldNcp == NULL) throwerr("ncpName not found");

    if (bugs >= 0 && iray == 0) {
      cout << setprecision(15);
      cout << "readRadarFile: fieldVg getNRays:    "
        << fieldVg->getNRays() << endl;
      cout << "readRadarFile: fieldDbz getNRays:   "
        << fieldDbz->getNRays() << endl;
      cout << "readRadarFile: fieldNcp getNRays:   "
        << fieldNcp->getNRays() << endl;

      cout << "readRadarFile: fieldVg getNPoints:  "
        << fieldVg->getNPoints() << endl;
      cout << "readRadarFile: fieldDbz getNPoints: "
        << fieldDbz->getNPoints() << endl;
      cout << "readRadarFile: fieldNcp getNPoints: "
        << fieldNcp->getNPoints() << endl;
    }

    double aziDeg = ray->getAzimuthDeg();
    double elevDeg = ray->getElevationDeg();

    double startDistKm = ray->getStartRangeKm();
    double spacingKm = ray->getGateSpacingKm();


    // Missing values in Radx:
    //
    // getDataType()      returns 1 == SI16
    // getDoubleValue(i)  returns -9999.00000
    // getStoredValue(i)  returns -9999.00000
    //
    // getMissing()       returns -32768
    // getMissingFl64()   returns -9999
    // getMissingSi16()   returns -32768
    //
    // If a program retrieves via getDoubleValue(),
    // it must compare with getMissingFl64(), not getMissing().
    //
    // For missing values getStoredValue has the same behavior
    // as getDoubleValue - both return _missingFl64.

    double missVg = fieldVg->getMissingFl64();
    double missDbz = fieldDbz->getMissingFl64();
    double missNcp = fieldNcp->getMissingFl64();

    Radx::DataType_t typeVg = fieldVg->getDataType();
    Radx::DataType_t typeDbz = fieldDbz->getDataType();
    Radx::DataType_t typeNcp = fieldNcp->getDataType();
    if (bugs >= 0 && iray == 0) {
      cout << setprecision(15);
      cout << "  missVg: " << missVg << endl;
      cout << "  missDbz: " << missDbz << endl;
      cout << "  missNcp: " << missNcp << endl;
      cout << "  typeVg: " << typeVg << endl;
      cout << "  typeDbz: " << typeDbz << endl;
      cout << "  typeNcp: " << typeNcp << endl;
      cout << "  vg getMissingFl64(): " << fieldVg->getMissingFl64() << endl;
      cout << "  vg getMissingSi16(): " << fieldVg->getMissingSi16() << endl;
    }

    //xxx
    //if (typeVg == Radx::SI08) missVg = fieldVg->getMissingSi08();
    //else if (typeVg == Radx::SI16) missVg = fieldVg->getMissingSi16();
    //else if (typeVg == Radx::SI32) missVg = fieldVg->getMissingSi32();
    //else badparms("unknown typeVg");

    struct timeval timei;
    addDeltaTime( &timei, NULL);
    for (size_t ipt = 0; ipt < fieldVg->getNPoints(); ipt++) {
      struct timeval timej;
      addDeltaTime( &timej, NULL);

      // Calcs that assume a locally flat earth and no refraction
      double elevRad = elevDeg * M_PI / 180;
      double slantDistKm = startDistKm + ipt * spacingKm;
      double flatHorizDistKm = cos( elevRad) * slantDistKm;
      double flatVertDistKm = sin( elevRad) * slantDistKm;
      double flatAltKmMsl = aircraftAltKmMsl + flatVertDistKm;

      // Calcs that assume a spherical earth with refraction.
      // See notes at the top.
      double krefract = 4.0 / 3.0;
      double earthRadiusKm = 0.001 * geodesic->MajorRadius();
      double refractRadiusKm = krefract * earthRadiusKm / cos( elevRad);
      double cx = refractRadiusKm * sin( elevRad);
      double cy = earthRadiusKm * (1 - krefract) + aircraftAltKmMsl;
      double phi = slantDistKm / refractRadiusKm;
      double gamma = M_PI / 2 + elevRad - phi;
      double px = cx + refractRadiusKm * cos( gamma);
      double py = cy + refractRadiusKm * sin( gamma);
      double altKmMsl = sqrt( px*px + py*py) - earthRadiusKm;  // hp
      double zeta = atan2( py, px);
      double horizDistKm = earthRadiusKm * (M_PI/2 - zeta);    // se

      // Calc error of the simple version
      double errHoriz = flatHorizDistKm - horizDistKm;
      double errVert = flatAltKmMsl - altKmMsl;
      if (fabs(errHoriz) > *maxAbsErrHoriz)
        *maxAbsErrHoriz = fabs(errHoriz);
      if (fabs(errVert) > *maxAbsErrVert)
        *maxAbsErrVert = fabs(errVert);

      if (bugs >= 10) {
        cout << setprecision(15);
        cout << "readRadarFile calcs:" << endl;
        cout << "  Slant dist: " << slantDistKm
          << "  elevDeg: " << elevDeg << endl;
        cout << "  cx: " << cx << "  cy: " << cy << endl;
        cout << "  px: " << px << "  py: " << py << endl;
        cout << "  elevRad: " << elevRad << endl;
        cout << "  phi: " << phi << endl;
        cout << "  zeta: " << zeta << endl;
        cout << "  Horiz dist: flatHorizDistKm: " << flatHorizDistKm
          << "  with refract: horizDistKm: " << horizDistKm
          << "  err: " << errHoriz
          << "  maxAbs: " << *maxAbsErrHoriz << endl;
        cout << "  flatAltKmMsl: " << flatAltKmMsl
          << "  with refract: altKmMsl: " << altKmMsl
          << "  err: " << errVert
          << "  maxAbs: " << *maxAbsErrVert << endl;
      }

      double valVg = fieldVg->getDoubleValue(ipt);
      double valDbz = fieldDbz->getDoubleValue(ipt);
      double valNcp = fieldNcp->getDoubleValue(ipt);

      cntTimej++;
      addDeltaTime( &timej, &sumTimej);
      struct timeval timek;
      addDeltaTime( &timek, NULL);


      // In one test of 237 files, this is called 28766400 times
      // with a total time of 31.8 seconds, or 1.1e-6 seconds per call.
      double latDeg, lonDeg, azi2Deg, m12;
      double a12 = geodesic->Direct(
        aircraftLatDeg, aircraftLonDeg,
        aziDeg, 1000 * horizDistKm,
        latDeg, lonDeg, azi2Deg, m12);

      cntTimek++;
      addDeltaTime( &timek, &sumTimek);
      struct timeval timem;
      addDeltaTime( &timem, NULL);

      // Make sure the data are valid ...
      // before going to the expense of finding the y,x coords.
      // If not missing and ...

      double minDbz = -20;           // xxx
      double minNcp = 0.3;           // xxx
      bool usePoint = false;
      if ( valVg != missVg
        && valDbz != missDbz && (forceOk || valDbz >= minDbz)
        && valNcp != missNcp && (forceOk || valNcp >= minNcp)
        && (maxDeltaAltKm == 0
          || fabs( altKmMsl - aircraftAltKmMsl) <= maxDeltaAltKm)
        && (maxAbsElevDeg == 0 || fabs( elevDeg) <= maxAbsElevDeg)
        && (minRadialDistKm == 0 || slantDistKm >= minRadialDistKm))
      {

        struct timeval timel;
        addDeltaTime( &timel, NULL);

        double coordy;       // northing in meters
        double coordx;       // easting in meters

        // Caution: the following call is slow!
        // In one test of 237 files, it is called 28766400 times
        // with a total time of 180.9 seconds,
        // or 6.3e-6 seconds per call.

        latLonToYX(
          tranMerc,          // TransverseMercatorExact
          projLon0,          // central meridian of projection
          basey,             // coord y base, km
          basex,             // coord x base, km
          latDeg,
          lonDeg,
          coordy,            // output value
          coordx);           // output value
        if (bugs >= 10) {
          cout << setprecision(15);
          cout << "    add ob: altKmMsl: " << altKmMsl
            << "  coordx: " << coordx
            << "  coordy: " << coordy << endl;
        }

        // Convert azimuth degrees to polar coordinates radians
        double thetaRad = 0.5 * M_PI - aziDeg * M_PI / 180;
        while (thetaRad < 0) thetaRad += 2*M_PI;
        while (thetaRad > 2*M_PI) thetaRad -= 2*M_PI;

        cntTimel++;
        addDeltaTime( &timel, &sumTimel);

        // Make sure coordz,y,x are within the grid
        double coordz = altKmMsl;
        if (
             (zgridmin==MISS_PARM || coordz >= zgridmin && coordz < zgridmax)
          && (ygridmin==MISS_PARM || coordy >= ygridmin && coordy < ygridmax)
          && (xgridmin==MISS_PARM || coordx >= xgridmin && coordx < xgridmax))
        {
          usePoint = true;
          numGoodPoint++;
          bool showDetail = testDetail(
            coordz,             // z
            coordy,             // y
            coordx,             // x
            detailSpec);        // z, y, x, delta
          if (showDetail) {
            cout << "readRadarFile: showDetail: coordz: " << coordz << endl;
            cout << "readRadarFile: showDetail: coordy: " << coordy << endl;
            cout << "readRadarFile: showDetail: coordx: " << coordx << endl;
          }

          pointBbox->addOb( coordz, coordy, coordx);

          if (testMode != TESTMODE_ZETA) {

            double synVels[3];    // W, V, U

            if (testMode == TESTMODE_BETA) {
              calcSyntheticWinds(
                showDetail,
                synWinds,           // user specified winds
                coordz,             // locz
                coordy,             // locy
                coordx,             // locx
                synVels);           // returned W, V, U
            }
            else if (testMode == TESTMODE_ZETA_BELTRAMI) {
              calcBeltramiFlow(
                showDetail,
                coordz,             // locz
                coordy,             // locy
                coordx,             // locx
                synVels);           // returned W, V, U
            }

            valVg = calcRadialVelocity(
              showDetail,
              thetaRad,           // polar coord angle from observer, radians
              elevRad,            // elevation angle from observer, radians
              synVels);           // W, V, U
            valDbz = 30;
            valNcp = 1;
          } // if ZETA




          if (fabs(valVg) > 200) throwerr("invalid valVg: %g", valVg);
          if (fabs(valDbz) > 200) throwerr("invalid valDbz: %g", valDbz);
          if (valNcp < 0 || valNcp > 1) throwerr("invalid valNcp: %g", valNcp);
          pointVec->push_back( new Point(
            ifile,
            iray,
            ipt,
            aircraftAltKmMsl,
            aircrafty,
            aircraftx,
            altKmMsl,
            coordy,
            coordx,
            ray->getTimeDouble(),      // UTC seconds since 1970 jan 1.
            thetaRad,     // polar coords angle
            elevRad,      // angle from the aircraft between horizontal and pt
            valVg,        // radial velocity
            valDbz,       // log reflectivity
            valNcp));     // radar net coherent power
          statVg->addOb( valVg);
          statDbz->addOb( valDbz);
          statNcp->addOb( valNcp);
          if (bugs >= 20 || itotpt % 10000 == 0) {
            cout << setprecision(5);
            cout << "readRadarFile: ok:"
              << "  iray: " << iray
              << "  ipt: " << ipt
              << "  itotpt: " << itotpt << endl
              << "  elevDeg: " << elevDeg
              << "  altKmMsl: " << altKmMsl
              << "  coordy: " << coordy
              << "  coordx: " << coordx << endl
              << "  valVg: " << valVg
              << "  valDbz: " << valDbz
              << "  valNcp: " << valNcp << endl;
          }







        } // if point is within the grid
        else numMissLocPoint++;
      } // if point is valid
      else numInvalidPoint++;

      cntTimem++;
      addDeltaTime( &timem, &sumTimem);
      struct timeval timen;
      addDeltaTime( &timen, NULL);


      cntTimen++;
      addDeltaTime( &timen, &sumTimen);

      itotpt++;
    } // for ipt
    cntTimei++;
    addDeltaTime( &timei, &sumTimei);

  } // for iray

  cntTimeh++;
  addDeltaTime( &timeh, &sumTimeh);


  cout << "readRadarFile: numGoodPoint: " << numGoodPoint << endl;
  cout << "readRadarFile: numInvalidPoint: " << numInvalidPoint << endl;
  cout << "readRadarFile: numMissLocPoint: " << numMissLocPoint << endl;
} // end readRadarFile




//======================================================================

double sumTimea = 0;
int cntTimeb = 0;
double sumTimeb = 0;
double sumTimec = 0;
double sumTimee = 0;
double sumTimef = 0;



// Calculate V and U winds at all cells in the z,y,x grid.

void RadarWind::calcAllVU(
  long bugs,
  int testMode,                // one of TESTMODE_*
  double * synWinds,
  long ndim,                   // == 3
  long numNbrMax,              // max num nearest nbrs
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
  Cell ***& cellMat,           // we set Cell.uu, vv
  bool useEigen,               // true: use Eigen.  false: use Cramer
  double * detailSpec)         // z, y, x, delta
{

  KD_real * centerLoc = new KD_real[ndim];

  for (long iz = 0; iz < nradz; iz++) {
    for (long iy = 0; iy < nrady; iy++) {
      for (long ix = 0; ix < nradx; ix++) {

        centerLoc[0] = zgridmin + iz * zgridinc;
        centerLoc[1] = ygridmin + iy * ygridinc;
        centerLoc[2] = xgridmin + ix * xgridinc;

        if (bugs >= 5) {
          cout << setprecision(5);
          cout << endl << "calcAllVU: iz: " << iz
            << "  iy: " << iy
            << "  ix: " << ix
            << "  z: " << centerLoc[0]
            << "  y: " << centerLoc[1]
            << "  x: " << centerLoc[2]
            << endl;
        }

        Cell * pcell = & cellMat[iz][iy][ix];
        calcCellVU(
          bugs,
          testMode,
          synWinds,
          ndim,                  // == 3
          centerLoc,             // query point
          numNbrMax,             // max num nearest nbrs
          maxDistBase,           // max pt dist = base + factor*aircraftDist
          maxDistFactor,         // max pt dist = base + factor*aircraftDist
          pointVec,              // all observations
          radarKdTree,           // nearest nbr tree for pointVec
          detailSpec,
          pcell,                 // Cell.vv, uu are set.
          useEigen);             // true: use Eigen.  false: use Cramer

        bool ok = false;
        if (isOkDouble( pcell->vv)
          && isOkDouble( pcell->uu))
        {
          ok = true;
        }

        if (bugs >= 10) {
          cout << setprecision(7);
          cout << "calcAllVU: ok:" << ok
            << "  iz: " << iz
            << "  iy: " << iy
            << "  ix: " << ix
            << "  loc:"
            << "  " << centerLoc[0]
            << "  " << centerLoc[1]
            << "  " << centerLoc[2]
            << "  W: " << cellMat[iz][iy][ix].ww
            << "  V: " << cellMat[iz][iy][ix].vv
            << "  U: " << cellMat[iz][iy][ix].uu << endl;
        }

      } // for ix
    } // for iy
  } // for iz

  cout << "sumTimea: " << sumTimea << endl;
  cout << "cntTimeb: " << cntTimeb << "  sumTimeb: " << sumTimeb << endl;
  cout << "sumTimec: " << sumTimec << endl;
  cout << "sumTimee: " << sumTimee << endl;
  cout << "sumTimef: " << sumTimef << endl;

  delete[] centerLoc;

} // end calcAllVU


//======================================================================



// Calculate V and U winds for a single location.
//
// At a given location centerLoc, find the nearest nbrs
// in pointVec, and use the radial velocities to calculate
// the winds V, U, (not W).
//
// Let vx, vy, vz be estimates of the wind velocity,
// and vr be the radial velocity.
//
// For each radar m, we use the linear model:
//    vr_est[m] = cos(theta) cos(elev) vx
//      + sin(theta) cos(elev) vy
//      + sin(elev) vz
//    vr_est[m] = conx vx + cony * vy + conz * vz
//
// Let E[m] = error = vr_true[m] - vr_est[m]
//
// We want to minimize Q = sum_m E[m]^2
//
//
// Let
//   sumxx = sum(conx*conx)
//   sumxr = sum(conx*vr)
//   etc.
//
// Normal equations, derived from
// 0 = d(Q)/d(vx), 0 = d(Q)/d(vy), 0 = d(Q)/d(vz),
// give:
// vx sumxx + vy sumxy + vz sumxz = sumxr
// vx sumxy + vy sumyy + vz sumyz = sumyr
// vx sumxz + vy sumyz + vz sumzz = sumzr
//
// Or for 2 dim,
// vx sumxx + vy sumxy = sumxr - vz sumxz
// vx sumxy + vy sumyy = sumyr - vz sumyz
//
// Using Cramer's rule on the 2-dim case:
// Let demom = sumxx sumyy - sumxy^2
// vx_hat = ( (sumxr - vz sumxz) sumyy - (sumyr - vz sumyz) sumxy ) / denom
// vy_hat = ( (sumyr - vz sumyz) sumxx - (sumxr - vz sumxz) sumxy ) / denom



void RadarWind::calcCellVU(
  long bugs,
  int testMode,                  // one of TESTMODE_*
  double * synWinds,
  long ndim,                     // == 3
  KD_real * centerLoc,           // query point: z, y, x
  long numNbrMax,                // max num nearest nbrs
  double maxDistBase,            // max pt dist = base + factor*aircraftDist
  double maxDistFactor,          // max pt dist = base + factor*aircraftDist
  vector<Point *> *pointVec,     // all observations
  KD_tree * radarKdTree,         // nearest nbr tree for pointVec
  double * detailSpec,           // z, y, x, delta
  Cell * pcell,                  // we fill vv, uu.
  bool useEigen)                 // true: use Eigen.  false: use Cramer
{

  struct timeval timea;
  addDeltaTime( &timea, NULL);

  pcell->uu = numeric_limits<double>::quiet_NaN();
  pcell->vv = numeric_limits<double>::quiet_NaN();

  bool showDetail = testDetail(
    centerLoc[0],         // z
    centerLoc[1],         // y
    centerLoc[2],         // x
    detailSpec);          // z, y, x, delta

  if (bugs >= 20 || showDetail) {
    cout << setprecision(7);
    cout << "calcCellVU.entry: showDetail:" << endl;
    cout << "    centerLoc: z: " << centerLoc[0] << endl;
    cout << "    centerLoc: y: " << centerLoc[1] << endl;
    cout << "    centerLoc: x: " << centerLoc[2] << endl;
    cout << "    ww: " << pcell->ww << endl;
    cout << "    vv: " << pcell->vv << endl;
    cout << "    uu: " << pcell->uu << endl;
    cout << "    meanNbrDbz: " << pcell->meanNbrDbz << endl;
    cout << "    meanNbrNcp: " << pcell->meanNbrNcp << endl;
    cout << "    meanNbrElevDeg: " << pcell->meanNbrElevDeg << endl;
    cout << "    meanNbrKeepDist: " << pcell->meanNbrKeepDist << endl;
    cout << "    meanNbrOmitDist: " << pcell->meanNbrOmitDist << endl;
    cout << "    condNum: " << pcell->conditionNumber << endl;
  }

  if (numNbrMax == 0) throwerr("cell numNbrMax == 0");
  int nbrIxs[numNbrMax];
  KD_real nbrDistSqs[numNbrMax];              // nbr dist^2
  for (long inbr = 0; inbr < numNbrMax; inbr++) {
    nbrIxs[inbr] = -1;
  }


  Statistic nbrDbzStat;
  Statistic nbrNcpStat;
  Statistic nbrElevDegStat;
  Statistic nbrKeepDistStat;
  Statistic nbrOmitDistStat;

  Point** nearPts = new Point *[numNbrMax];



  addDeltaTime( &timea, &sumTimea);
  struct timeval timeb;
  addDeltaTime( &timeb, NULL);



  // Find nearest nbrs
  radarKdTree->nnquery(
    centerLoc,        // query point
    numNbrMax,        // desired num nearest nbrs
    KD_EUCLIDEAN,     // Metric
    1,                // MinkP
    nbrIxs,           // out: parallel array, indices of nearest nbrs
    nbrDistSqs);      // out: parallel array, squares of distances of nbrs


  cntTimeb++;
  addDeltaTime( &timeb, &sumTimeb);
  struct timeval timec;
  addDeltaTime( &timec, NULL);



  if (bugs >= 10 || showDetail) {
    cout << setprecision(7);
    cout << "  calcCellVU: showDetail:  nearPts for centerLoc: z: "
      << centerLoc[0] << "  y: " << centerLoc[1]
      << "  x: " << centerLoc[2] << endl;
  }

  int numNbrActual = 0;

  for (long inbr = 0; inbr < numNbrMax; inbr++) {
    if (nbrIxs[inbr] < 0) throwerr("nbrIxs < 0");

    Point * nearPt = pointVec->at( nbrIxs[inbr]);
    double aircraftDist = calcDistPtAircraft( nearPt);
    double localDist = calcDistLocPt( centerLoc, nearPt);
    double maxDist = maxDistBase + maxDistFactor * aircraftDist;
    const char * msg;

    if (localDist < maxDist) {
      nbrDbzStat.addOb( nearPt->dbz);
      nbrNcpStat.addOb( nearPt->ncp);
      nbrElevDegStat.addOb( nearPt->elevRad * 180 / M_PI);
      nbrKeepDistStat.addOb( localDist);
      nearPts[numNbrActual++] = nearPt;
      msg = "KEEP";
    } // if localDist < maxDist

    else {
      nbrOmitDistStat.addOb( localDist);
      msg = "OMIT";
    }

    if (bugs >= 10 || showDetail) {
    cout << setprecision(5);
    cout << "    " << msg
      << "  inbr: " << inbr
      << "  dist: " << localDist
      << "  pt:"
      << "  coordz: " << nearPt->coordz
      << "  coordy: " << nearPt->coordy
      << "  coordx: " << nearPt->coordx
      << "  vg: " << nearPt->vg
      << endl;
    }

  } // for inbr

  if (bugs >= 20 || showDetail) {
    cout << "  calcCellVU: showDetail: cell z: "
      << centerLoc[0] << "  y: " << centerLoc[1]
      << "  x: " << centerLoc[0] << "  numNbrActual: " << numNbrActual << endl;
  }

  if (numNbrActual >= 2) {    // if numNbrActual is ok

    pcell->meanNbrDbz = nbrDbzStat.dsum / nbrDbzStat.numGood;
    pcell->meanNbrNcp = nbrNcpStat.dsum / nbrNcpStat.numGood;
    pcell->meanNbrElevDeg = nbrElevDegStat.dsum / nbrElevDegStat.numGood;
    pcell->meanNbrKeepDist = nbrKeepDistStat.dsum / nbrKeepDistStat.numGood;
    pcell->meanNbrOmitDist = nbrOmitDistStat.dsum / nbrOmitDistStat.numGood;

    if (bugs >= 20 || showDetail) {
      for (long inbr = 0; inbr < numNbrActual; inbr++) {
        Point * pt = nearPts[inbr];
        cout << setprecision(7);
        cout << "  calcCellVU: showDetail: nbr: inbr: " << inbr
          << "  vg: " << pt->vg
          << "  dbz: " << pt->dbz
          << "  ncp: " << pt->ncp
          << "  theta deg: " << (pt->thetaRad * 180 / M_PI)
          << "  elev deg: " << (pt->elevRad * 180 / M_PI)
          << setprecision(15)
          << "  deltaTime: "
          << (pt->rayTime - nearPts[0]->rayTime)
          << endl;
      }
      for (long inbr = 0; inbr < numNbrActual; inbr++) {
        Point * pt = nearPts[inbr];
        cout << "  showDetail.from.aircraft.to.nbr: set arrow "
          << (inbr + 1)
          << " from " << pt->aircraftx << "," << pt->aircrafty
          << " to " << pt->coordx << "," << pt->coordy
          << endl;
      }
    } // if showDetail

    addDeltaTime( &timec, &sumTimec);
    struct timeval timee;
    addDeltaTime( &timee, NULL);


    // 3d solve
    //    vr = cos(theta) cos(elev) vx
    //      + sin(theta) cos(elev) vy
    //      + sin(elev) vz

    // 2d solve
    //    vr - sin(elev) vz = cos(theta) cos(elev) vx
    //      + sin(theta) cos(elev) vy


    // Find wwind = W wind estimate near the pt.
    double wwind = pcell->ww;
    bool isCellOk = false;

    if (useEigen) {

      Eigen::MatrixXd amat( numNbrActual, 2);
      Eigen::VectorXd bvec( numNbrActual);

      for (long inbr = 0; inbr < numNbrActual; inbr++) {
        Point * pt = nearPts[inbr];
        amat( inbr, 0) = cos( pt->thetaRad) * cos( pt->elevRad);
        amat( inbr, 1) = sin( pt->thetaRad) * cos( pt->elevRad);

        // Find wwind = W wind estimate near the pt.
        double wwind = pcell->ww;

        bvec( inbr) = pt->vg - wwind * sin( pt->elevRad);
      }
      if (bugs >= 20 || showDetail) {
        cout << "\n  calcCellVU: eigen: amat:\n" << amat << endl;
        cout << "\n  calcCellVU: eigen: bvec:\n" << bvec << endl;
      }

      Eigen::JacobiSVD<Eigen::MatrixXd> svd(
        amat, Eigen::ComputeThinU | Eigen::ComputeThinV);
      Eigen::VectorXd singVals = svd.singularValues();
      long slen = singVals.size();

      pcell->conditionNumber = numeric_limits<double>::infinity();
      if (slen > 0 && singVals[slen-1] != 0)
        pcell->conditionNumber = singVals[0] / singVals[slen-1];

      if (bugs >= 20 || showDetail) {
        cout << "\n  calcCellVU: eigen: singVals:\n" << singVals << endl;
        cout << "\n  calcCellVU: num nonzeroSingularValues:\n"
          << svd.nonzeroSingularValues() << endl;
        cout << "  calcCellVU: conditionNumber: "
          << pcell->conditionNumber << endl;
      }

      // Using a conditionNumberCutoff > 10 causes some cells
      // to have extreme values for U and V.
      double conditionNumberCutoff = 10;     // xxxx
      if (pcell->conditionNumber < conditionNumberCutoff) isCellOk = true;
      if (isCellOk) {
        Eigen::VectorXd xvec = svd.solve( bvec);
        Eigen::VectorXd errvec = amat * xvec - bvec;
        double maxAbsErr = errvec.array().abs().maxCoeff();
        double meanSqErr = errvec.dot( errvec) / numNbrActual;   // dot product

        pcell->vv = xvec(1);       // v
        pcell->uu = xvec(0);       // u
      } // if isCellOk
    } // if useEigen

    // Else use cramer's method
    else {
      double sxx = 0;
      double sxy = 0;
      double syy = 0;
      double sxz = 0;
      double syz = 0;

      for (long inbr = 0; inbr < numNbrActual; inbr++) {
        Point * pt = nearPts[inbr];
        double xval = cos( pt->thetaRad) * cos( pt->elevRad);
        double yval = sin( pt->thetaRad) * cos( pt->elevRad);
        double zval = pt->vg - wwind * sin( pt->elevRad);

        sxx += xval * xval;
        sxy += xval * yval;
        syy += yval * yval;
        sxz += xval * zval;
        syz += yval * zval;
      }
      double detbase = sxx*syy - sxy*sxy;
      double det1    = sxz*syy - syz*sxy;
      double det2    = sxx*syz - sxy*sxz;
      if (bugs >= 20 || showDetail) {
        cout << "  calcCellVU: cramer: showDetail: "
          << "  sxx: " << sxx
          << "  sxy: " << sxy
          << "  syy: " << syy
          << "  sxz: " << sxz
          << "  syz: " << syz << endl;
        cout << "  calcCellVU: cramer: showDetail: "
          << "  detbase: " << detbase
          << "  det1: " << det1
          << "  det2: " << det2 << endl;
      }

      // Using a detCutoff <= 0.01 causes some cells
      // to have extreme values for U and V.
      // A detCutoff of 0.1 gives roughly similar results to using
      // a conditionNumber cutoff of 10.
      double detCutoff = 0.1;    // xxxxxxxxx
      if (fabs(detbase) > detCutoff) {
        pcell->uu = det1 / detbase;
        pcell->vv = det2 / detbase;
      }
    }

    addDeltaTime( &timee, &sumTimee);
  } // else numNbrActual is ok

  struct timeval timef;
  addDeltaTime( &timef, NULL);

  if (bugs >= 20 || showDetail) {
    cout << setprecision(7);
    cout << "calcCellVU.exit: showDetail:" << endl;
    cout << "    centerLoc: z: " << centerLoc[0] << endl;
    cout << "    centerLoc: y: " << centerLoc[1] << endl;
    cout << "    centerLoc: x: " << centerLoc[2] << endl;
    cout << "    ww: " << pcell->ww << endl;
    cout << "    vv: " << pcell->vv << endl;
    cout << "    uu: " << pcell->uu << endl;
    cout << "    meanNbrDbz: " << pcell->meanNbrDbz << endl;
    cout << "    meanNbrNcp: " << pcell->meanNbrNcp << endl;
    cout << "    meanNbrElevDeg: " << pcell->meanNbrElevDeg << endl;
    cout << "    meanNbrKeepDist: " << pcell->meanNbrKeepDist << endl;
    cout << "    meanNbrOmitDist: " << pcell->meanNbrOmitDist << endl;
    cout << "    condNum: " << pcell->conditionNumber << endl;
  }

} // end calcCellVU




// xxx all alloc: delete

//======================================================================


// Calculate the W (vertical) winds for all cells in the z,y,x grid.
//
// Consider the vertical column of cells for some
// given horizontal coordinates iy, ix.
// By the mass continuity equation, the total volume of air coming
// into the column must equal the total volume of air leaving the column.
//
// 0 = totalFlow = sum_iz (density_iz * sideFlow_iz) + endFlow
// where
//   density_iz = the density at level iz
//   sideFlow_iz = flow through the sides of the cell at level iz
//   endFlow = flow in the bottom and out the top end of the column
//           = densityBot * bottomFlow - densityTop * topFlow
//
// We assume the end flows are 0, so
//   bottendFlow = 0
//   topFlow = 0
//   endFlow = 0
//
// sideFlow_iz = uFlow_iz + vFlow_iz
// uFlow_iz = (flow in from cell ix-1) - (flow out to cell ix+1)
// uFlow_iz = 0.5 * (u[ix-1] + u[ix]) - 0.5 * (u[ix] + u[ix+1])
//          = 0.5 * (u[ix-1] - u[ix+1])
//
// sideFlow_iz = 0.5 * (u[ix-1] - u[ix+1] + v[ix-1] - v[ix+1])
//
// 0 = totalFlow = sum_iz (density_iz * 0.5 * (
//       v[iy-1] - v[iy+1]
//     + u[ix-1] - u[ix+1]))
//
// But in real life the totalFlow sum is not 0.
//
// Now we ask what modifications to the U and V values
// would make the totalFlow 0.
// We want to modify those U,V values with larger elevation angles more.
// xxx future: handle geometric uncertainty values too.
//
// At each layer, we have 4 terms to adjust: 2 U terms and 2 V terms.
// Let us subtract h[iz] from each term on level iz.
//
// Find h[iz] such that:
//   0 = sum_iz (density_iz * 0.5 * (
//         (v[iy-1]-h[iz]) - (v[iy+1]+h[iz])
//       + (u[ix-1]-h[iz]) - (u[ix+1])+h[iz]))
//
//     = totalFlow - sum_iz (density_iz * 0.5 * 4 * h[iz])
//
//   totalFlow = 2 * sum_iz (density_iz * h[iz])
//
// Let h[iz] be weighted by the elevation angle, so
//   h[iz] = H * wgt[iz]
//   wgt[iz] = cos(elevationAngle)
//
//   totalFlow = 2 * sum_iz (density[iz] * H * wgt[iz])
//   H = totalFlow / (2 * sum_iz (density[iz] * wgt[iz]))


void RadarWind::calcAllW(
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
  Cell ***& cellMat,           // We set Cell.ww
  double * detailSpec)         // z, y, x, delta
{

  double * density = new double[nradz];
  double * wgts = new double[nradz];
  for (long iz = 0; iz < nradz; iz++) {
    density[iz] = calcDensity( zgridmin + iz * zgridinc);
  }

  // Omit the edges of the region as we use ix-1, ix+1, iy-1, iy+1.
  for (long iy = 1; iy < nrady - 1; iy++) {
    for (long ix = 1; ix < nradx - 1; ix++) {

      for (long iz = 0; iz < nradz; iz++) {
        // xxx Future:
        // Find c = the nearest cell to this one
        // through which the aircraft flew.
        // Let cosElev = horizDistToC / slantDistToC
        // wgt = cosElev

        wgts[iz] = 1.0;
      }

      // Calc totalFlow = sum of everything flowing into the column,
      // not counting the top or bottom faces.
      double totalFlow = 0;
      double sumWgt = 0;
      for (long iz = 0; iz < nradz; iz++) {
        if ( isOkDouble( cellMat[iz][iy][ix-1].uu)
          && isOkDouble( cellMat[iz][iy][ix+1].uu)
          && isOkDouble( cellMat[iz][iy-1][ix].vv)
          && isOkDouble( cellMat[iz][iy+1][ix].vv))
        {
          totalFlow += density[iz] * 0.5
            * ( cellMat[iz][iy][ix-1].uu - cellMat[iz][iy][ix+1].uu
             +  cellMat[iz][iy-1][ix].vv - cellMat[iz][iy+1][ix].vv);
          sumWgt += density[iz] * wgts[iz];

          bool showDetail = testDetail(
            zgridmin + iz * zgridinc,      // z
            ygridmin + iy * ygridinc,      // y
            xgridmin + ix * xgridinc,      // x
            detailSpec);                   // z, y, x, delta

          if (showDetail) {
            cout << setprecision(7);
            cout << "calcAllW: showDetail:" << endl
              << "    iz: " << iz << endl
              << "    iy: " << iy << endl
              << "    ix: " << ix << endl
              << "    den: " << density[iz] << endl
              << "    wgt: " << wgts[iz] << endl
              << "    U-: " << cellMat[iz][iy][ix-1].uu << endl
              << "    U+: " << cellMat[iz][iy][ix+1].uu << endl
              << "    V-: " << cellMat[iz][iy-1][ix].vv << endl
              << "    V+: " << cellMat[iz][iy+1][ix].vv << endl;
          }
        }
      } // for iz

      double hcon;
      if (fabs(sumWgt) < epsilon) hcon = 0;
      else hcon = totalFlow / (2 * sumWgt);

      // Calc W wind = totalFlow, starting at the bottom,
      // using the modified U, V winds.
      double wwind = baseW;
      for (long iz = 0; iz < nradz; iz++) {
        if ( isOkDouble( cellMat[iz][iy][ix-1].uu)
          && isOkDouble( cellMat[iz][iy][ix+1].uu)
          && isOkDouble( cellMat[iz][iy-1][ix].vv)
          && isOkDouble( cellMat[iz][iy+1][ix].vv))
        {
          wwind += density[iz] * 0.5
            * (  cellMat[iz][iy][ix-1].uu - cellMat[iz][iy][ix+1].uu
               + cellMat[iz][iy-1][ix].vv - cellMat[iz][iy+1][ix].vv
               - 4 * hcon * wgts[iz]);
          if (! isOkDouble( wwind))
            throwerr("calcAllW: invalid w wind");
          cellMat[iz][iy][ix].ww = wwind;

          bool showDetail = testDetail(
            zgridmin + iz * zgridinc,      // z
            ygridmin + iy * ygridinc,      // y
            xgridmin + ix * xgridinc,      // x
            detailSpec);                   // z, y, x, delta

          if (showDetail) {
            cout << setprecision(7);
            cout << "calcAllW: showDetail:" << endl
              << "  iz: " << iz << endl
              << "  iy: " << iy << endl
              << "  ix: " << ix << endl
              << "  den: " << density[iz] << endl
              << "  U-: " << cellMat[iz][iy][ix-1].uu << endl
              << "  U+: " << cellMat[iz][iy][ix+1].uu << endl
              << "  V-: " << cellMat[iz][iy-1][ix].vv << endl
              << "  V+: " << cellMat[iz][iy+1][ix].vv << endl
              << "  hcon: " << hcon << endl
              << "  wgt: " << wgts[iz] << endl
              << "  con: " << (4 * hcon * wgts[iz]) << endl
              << "  wwind: " << wwind << endl;
          }
        }
        else cellMat[iz][iy][ix].ww = numeric_limits<double>::quiet_NaN();
      } // for iz
      if (fabs(wwind - baseW) > epsilon) {
        cout << setprecision(15);
        cout << "calcAllW: iy: " << iy << "  ix: " << ix
          << "  baseW: " << baseW << "  wwind: " << wwind << endl;
        cout.flush();
        throwerr("wwind error");
      }
    } // for ix
  } // for iy


  //xxx del:
  checkInvalid( bugs, "calcAllW A: W", nradz, nrady, nradx, cellMat);

//xxx maybe omit:
  // We only calculated the inner points for iy, ix.
  // But we also will need the edge values when
  // we calc the next iteration of U, V values.
  // So set the edges from the nearest interior points.
  // May be NaN.

  for (long iz = 0; iz < nradz; iz++) {
    // Corners
    cellMat[iz][0][0].ww             = cellMat[iz][1][1].ww;
    cellMat[iz][0][nradx-1].ww       = cellMat[iz][1][nradx-2].ww;
    cellMat[iz][nrady-1][0].ww       = cellMat[iz][nrady-2][1].ww;
    cellMat[iz][nrady-1][nradx-1].ww = cellMat[iz][nrady-2][nradx-2].ww;

    // Side edges
    for (long iy = 1; iy < nrady - 1; iy++) {
      cellMat[iz][iy][0].ww       = cellMat[iz][iy][1].ww;
      cellMat[iz][iy][nradx-1].ww = cellMat[iz][iy][nradx-2].ww;
    }
    // Top and bottom edges
    for (long ix = 1; ix < nradx - 1; ix++) {
      cellMat[iz][0][ix].ww       = cellMat[iz][1][ix].ww;
      cellMat[iz][nrady-1][ix].ww = cellMat[iz][nrady-2][ix].ww;
    }
  } // for iz

  delete[] density;
  delete[] wgts;

  //xxx del:
  checkInvalid( bugs, "calcAllW B: W", nradz, nrady, nradx, cellMat);

} // end calcAllW



//======================================================================





// Returns the air density at a given height,
// using the US Standard Atmosphere.
//
// Hypsometric equation for US Standard Atmosphere (1976), from
// Seymour L. Hess, Introduction to Theoretical Meteorology, 1979, p 82-83.
//
// Caution: the current wikipedia article assumes constant temperature;
// Seymour Hess does not.
//
// http://scipp.ucsc.edu/outreach/balloon/atmos/1976%20Standard%20Atmosphere.htm


double RadarWind::calcDensity(
  double height)                // height in km MSL
{
  double t1 = 288.15;           // kelvin, at mean sea level
  double t2;                    // kelvin

  if (height <= 11) t2 = t1 - 6.5 * height;
  else t2 = 216.65;

  double Rd = 287.053072047065;        // J kg-1 K-1
  double gval = 9.80665;               // m s-2
  double gamma = 0.0065;               // K m-1
  double expon = gval / (Rd * gamma);
  double p1 = 101325;                  // Pa = N m-2 = kg m-1 s-2
                                       // at mean sea level
  double p2 = p1 * pow( t2 / t1, expon);

  // Ideal gas law
  // p v = n r t
  // density = n r / v = p / t
  double density1 = 1.2250;            // kg m-3, at mean sea level
  double density2 = density1 * (p2 / p1) * (t1 / t2);
  return density2;
} // end calcDensity



//======================================================================


// NO LONGER USED:
// Interpolate missing values from neighbors.
//
// The problem is that some cells don't have enough valid
// neighbors to calculate their winds, so their values are
// left as NaN.
//
// The idea was to use fill the NaN cells
// by interpolating the wind values from nearby cells.
//
// But what happens is that generally if a cell doesn't have
// enough good neighbors for a valid wind calculation,
// any neighbor cells we might use for interpolation are
// pretty flakey.

void RadarWind::interpMissing(
  long bugs,
  long nradz,                  // grid z dim
  long nrady,                  // grid y dim
  long nradx,                  // grid x dim
  double ***& radmat,          // interpolate to fill NaN values
  double ***& tmpmat)          // temp work
{

  //xxx make this a parm
  char nbrType = 'f';          // f: faces, c: corners + faces

  long nnbr = 0;
  // Parallel arrays:
  long * izs;         // offsets in the z direction
  long * iys;         // offsets in the y direction
  long * ixs;         // offsets in the x direction

  if (nbrType == 'f') {     // nbrs at faces only
    nnbr = 6;
    izs = new long[ nnbr];
    iys = new long[ nnbr];
    ixs = new long[ nnbr];
    long ii = 0;
    // Each row gives the offsets to a neighbor.
    izs[ii] = -1;  iys[ii] =  0;  ixs[ii] =  0;  ii++;   // nbr below
    izs[ii] =  0;  iys[ii] = -1;  ixs[ii] =  0;  ii++;   // nbrs at same z
    izs[ii] =  0;  iys[ii] =  0;  ixs[ii] = -1;  ii++;
    izs[ii] =  0;  iys[ii] =  0;  ixs[ii] =  1;  ii++;
    izs[ii] =  0;  iys[ii] =  1;  ixs[ii] =  0;  ii++;
    izs[ii] =  1;  iys[ii] =  0;  ixs[ii] =  0;  ii++;   // nbr above
  }

  else if (nbrType == 'c') {     // nbrs at corners + faces
    nnbr = 26;
    izs = new long[ nnbr];
    iys = new long[ nnbr];
    ixs = new long[ nnbr];
    long ii = 0;
    // Each row gives the offsets to a neighbor.
    izs[ii] = -1;  iys[ii] = -1;  ixs[ii] = -1;  ii++;   // nbrs below
    izs[ii] = -1;  iys[ii] = -1;  ixs[ii] =  0;  ii++;
    izs[ii] = -1;  iys[ii] = -1;  ixs[ii] =  1;  ii++;

    izs[ii] = -1;  iys[ii] =  0;  ixs[ii] = -1;  ii++;
    izs[ii] = -1;  iys[ii] =  0;  ixs[ii] =  0;  ii++;
    izs[ii] = -1;  iys[ii] =  0;  ixs[ii] =  1;  ii++;

    izs[ii] = -1;  iys[ii] =  1;  ixs[ii] = -1;  ii++;
    izs[ii] = -1;  iys[ii] =  1;  ixs[ii] =  0;  ii++;
    izs[ii] = -1;  iys[ii] =  1;  ixs[ii] =  1;  ii++;

    izs[ii] =  0;  iys[ii] = -1;  ixs[ii] = -1;  ii++;   // nbrs at same z
    izs[ii] =  0;  iys[ii] = -1;  ixs[ii] =  0;  ii++;
    izs[ii] =  0;  iys[ii] = -1;  ixs[ii] =  1;  ii++;

    izs[ii] =  0;  iys[ii] =  0;  ixs[ii] = -1;  ii++;   // nbrs at same z,y
    izs[ii] =  0;  iys[ii] =  0;  ixs[ii] =  1;  ii++;

    izs[ii] =  0;  iys[ii] =  1;  ixs[ii] = -1;  ii++;
    izs[ii] =  0;  iys[ii] =  1;  ixs[ii] =  0;  ii++;
    izs[ii] =  0;  iys[ii] =  1;  ixs[ii] =  1;  ii++;

    izs[ii] =  1;  iys[ii] = -1;  ixs[ii] = -1;  ii++;   // nbrs above
    izs[ii] =  1;  iys[ii] = -1;  ixs[ii] =  0;  ii++;
    izs[ii] =  1;  iys[ii] = -1;  ixs[ii] =  1;  ii++;

    izs[ii] =  1;  iys[ii] =  0;  ixs[ii] = -1;  ii++;
    izs[ii] =  1;  iys[ii] =  0;  ixs[ii] =  0;  ii++;
    izs[ii] =  1;  iys[ii] =  0;  ixs[ii] =  1;  ii++;

    izs[ii] =  1;  iys[ii] =  1;  ixs[ii] = -1;  ii++;
    izs[ii] =  1;  iys[ii] =  1;  ixs[ii] =  0;  ii++;
    izs[ii] =  1;  iys[ii] =  1;  ixs[ii] =  1;  ii++;
  }
  else throwerr("invalid nbrType");


  // Copy radmat to tmpmat, substituting interpolated values
  // for non-ok cells.
  for (long iz = 0; iz < nradz; iz++) {
    for (long iy = 0; iy < nrady; iy++) {
      for (long ix = 0; ix < nradx; ix++) {

        if (isOkDouble( radmat[iz][iy][ix])) {
          tmpmat[iz][iy][ix] = radmat[iz][iy][ix];
        }

        else {
          long numval = 0;
          double tsum = 0;

          for (long inb = 0; inb < nnbr; inb++) {
            // Set jz,jy,jx = indices of the neighbor.
            long jz = iz + izs[inb];
            long jy = iy + iys[inb];
            long jx = ix + ixs[inb];
            if ( jz >= 0 && jz < nradz
              && jy >= 0 && jy < nrady
              && jx >= 0 && jx < nradx
              && isOkDouble( radmat[jz][jy][jx]))
            {
              numval++;
              tsum += radmat[jz][jy][jx];
            }
            if (numval > 0)
              tmpmat[iz][iy][ix] = tsum / numval;
            else
              tmpmat[iz][iy][ix] = numeric_limits<double>::quiet_NaN();
          }

        } // else not ok
      } // for ix
    } // for iy
  } // for iz

  // Copy tmpmat back to radmat
  for (long iz = 0; iz < nradz; iz++) {
    for (long iy = 0; iy < nrady; iy++) {
      for (long ix = 0; ix < nradx; ix++) {
        radmat[iz][iy][ix] = tmpmat[iz][iy][ix];
      } // for ix
    } // for iy
  } // for iz

} // end interpMissing



//==================================================================


// Check that all winds (W,V,U) are valid in all cells.
// No longer needed.

void RadarWind::checkInvalid(
  long bugs,
  const string msg,
  long nradz,                  // grid z dim
  long nrady,                  // grid y dim
  long nradx,                  // grid x dim
  Cell ***& cellMat)           // check for NaN values
{
  long numbad = 0;
  for (long iz = 0; iz < nradz; iz++) {
    for (long iy = 1; iy < nrady - 1; iy++) {
      for (long ix = 1; ix < nradx - 1; ix++) {
        Cell * cell = & cellMat[iz][iy][ix];
        if (! (isOkDouble( cell->ww)
            && isOkDouble( cell->vv)
            && isOkDouble( cell->uu)))
        {
          numbad++;
          if (bugs >= 5) {
            cout << "  checkInvalid: invalid wind for: " << msg
              << "  iz: " << iz
              << "  iy: " << iy
              << "  ix: " << ix
              << "  ww: " << cell->ww
              << "  vv: " << cell->vv
              << "  uu: " << cell->uu
              << endl;
            //cout.flush();
            //throwerr("main B: invalid w wind");
          }
        }
      }
    }
  }
  cout << msg << ": num invalid wind: " << numbad
    << "  out of " << (nradz*nrady*nradx) << endl;
}

//======================================================================


// Calc verification deltas and statistics (if testing),
// and to write a text format of the cellMat to the outTxt file.

void RadarWind::checkVerif(
  long bugs,
  int testMode,                // one of TESTMODE_*
  double * synWinds,
  long imain,
  string * outTxt,             // output file name
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
  //xxx rename rad to estim
  Cell ***& cellMat,           // we set Cell.uu, vv

  double * detailSpec)         // z, y, x, delta
{

  Statistic statVerifDist;
  Statistic statTruew;
  Statistic statTruev;
  Statistic statTrueu;
  Statistic statCalcw;
  Statistic statCalcv;
  Statistic statCalcu;
  Statistic statDiffw;
  Statistic statDiffv;
  Statistic statDiffu;
  Statistic statAbsDiffw;
  Statistic statAbsDiffv;
  Statistic statAbsDiffu;

  ofstream * ostm = new ofstream();
  ostm->open( outTxt->c_str());

  for (long iz = 0; iz < nradz; iz++) {
    for (long iy = 0; iy < nrady; iy++) {
      for (long ix = 0; ix < nradx; ix++) {

        double centerLocZ = zgridmin + iz * zgridinc;
        double centerLocY = ygridmin + iy * ygridinc;
        double centerLocX = xgridmin + ix * xgridinc;

        bool showDetail = testDetail(
          centerLocZ,           // z
          centerLocY,           // y
          centerLocX,           // x
          detailSpec);          // z, y, x, delta

        if (showDetail) {
          cout << endl << "===== checkVerif: showDetail"
            << "  centerLoc: z: " << centerLocZ
            << "  y: " << centerLocY
            << "  x: " << centerLocX << endl << endl;
        }

        // Get the true wind values
        double verVels[3];    // W, V, U

        if (testMode == TESTMODE_ALPHA || testMode == TESTMODE_BETA) {
          // Get verVelW, verVelV, verVelU
          double synThetaRad = 0;
          double synElevRad = 0;
          calcSyntheticWinds(
            showDetail,
            synWinds,           // user specified winds
            centerLocZ,         // locz
            centerLocY,         // locy
            centerLocX,         // locx
            verVels);           // returned W, V, U
        } // if testMode == TESTMODE_ALPHA or TESTMODE_BETA

        else if (testMode == TESTMODE_ZETA_BELTRAMI) {
          calcBeltramiFlow(
            showDetail,
            centerLocZ,         // locz
            centerLocY,         // locy
            centerLocX,         // locx
            verVels);           // returned W, V, U

        } // if testMode == TESTMODE_ZETA_BELTRAMI

        else if (testMode == TESTMODE_ZETA) {
          for (int ii = 0; ii < 3; ii++) {
            verVels[ii] = numeric_limits<double>::quiet_NaN();
          }
        } // if testMode == TESTMODE_ZETA

        else throwerr("invalid testMode");

        double wdiff = cellMat[iz][iy][ix].ww - verVels[0];   // W
        double vdiff = cellMat[iz][iy][ix].vv - verVels[1];   // V
        double udiff = cellMat[iz][iy][ix].uu - verVels[2];   // U

        statTruew.addOb( verVels[0]);    // W
        statTruev.addOb( verVels[1]);    // V
        statTrueu.addOb( verVels[2]);    // U

        statCalcw.addOb( cellMat[iz][iy][ix].ww);
        statCalcv.addOb( cellMat[iz][iy][ix].vv);
        statCalcu.addOb( cellMat[iz][iy][ix].uu);

        statDiffw.addOb( wdiff);
        statDiffv.addOb( vdiff);
        statDiffu.addOb( udiff);

        statAbsDiffw.addOb( fabs( wdiff));
        statAbsDiffv.addOb( fabs( vdiff));
        statAbsDiffu.addOb( fabs( udiff));

        bool okFlag = false;
        if ( isOkDouble( cellMat[iz][iy][ix].ww)
          && isOkDouble( cellMat[iz][iy][ix].vv)
          && isOkDouble( cellMat[iz][iy][ix].uu))
        {
          okFlag = true;
        }

        Cell * pcell = & cellMat[iz][iy][ix];
        ostringstream msgstm;
        msgstm << setprecision(7);
        msgstm << "ckv:ok: " << okFlag
          << "  izyx: " << iz << "  " << iy << "  " << ix
          << "  locZYX:"
          << "  " << centerLocZ
          << "  " << centerLocY
          << "  " << centerLocX
          << "  verifWVU:"
          << "  " << verVels[0]
          << "  " << verVels[1]
          << "  " << verVels[2]
          << "  calcWVU:"
          << "  " << pcell->ww
          << "  " << pcell->vv
          << "  " << pcell->uu
          << "  diffWVU:"
          << "  " << (pcell->ww - verVels[0])
          << "  " << (pcell->vv - verVels[1])
          << "  " << (pcell->uu - verVels[2])
          << "  meanNbrElevDeg: " << pcell->meanNbrElevDeg
          << "  meanNbrKeepDist: " << pcell->meanNbrKeepDist
          << "  meanNbrOmitDist: " << pcell->meanNbrOmitDist
          << "  condNum: " << pcell->conditionNumber
          << endl;
        (*ostm) << msgstm.str();
        if (showDetail) cout << msgstm.str();

      } // for ix
    } // for iy
  } // for iz


  cout << setprecision(7);
  cout << endl;
  cout << "checkVerif: imain: " << imain << endl;
  cout << "statVerifDist: ";  statVerifDist.print( 7);
  cout << endl;
  cout << "statTruew: ";  statTruew.print( 7);
  cout << "statTruev: ";  statTruev.print( 7);
  cout << "statTrueu: ";  statTrueu.print( 7);
  cout << endl;
  cout << "statCalcw: ";  statCalcw.print( 7);
  cout << "statCalcv: ";  statCalcv.print( 7);
  cout << "statCalcu: ";  statCalcu.print( 7);
  cout << endl;
  cout << "statDiffw: ";  statDiffw.print( 7);
  cout << "statDiffv: ";  statDiffv.print( 7);
  cout << "statDiffu: ";  statDiffu.print( 7);
  cout << endl;
  cout << "statAbsDiffw: ";  statAbsDiffw.print( 7);
  cout << "statAbsDiffv: ";  statAbsDiffv.print( 7);
  cout << "statAbsDiffu: ";  statAbsDiffu.print( 7);
  cout << endl;

  ostm->close();
  delete ostm;


} // end checkVerif


//==================================================================


// NOTE: inVerif and readVerifFile are no longer used.
// Now we calculate the verification values internally.
//
// Read a text file containing verification wind values.
//
// The inVerif file consists of a header,
// with one attribute and value per line, followed
// by a data section with one point per line.
// Comments start with #.
// Items in angle brackets <> are placeholders
// for actual values.
// ==== snip ====
//   projection: transverseMercator <lon0> <lat0>
//   z: <numz> <minz> <maxz> <incz>
//   y: <numy> <miny> <maxy> <incy>
//   x: <numx> <minx> <maxx> <incx>
//   data:
//   <x> <y> <z> <u> <v> <w>
//   <x> <y> <z> <u> <v> <w>
//   ...
// ==== snip ====


void RadarWind::readVerifFile(
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
  double ***& verifUmat)     // returned: we alloc and fill
{
  cout << "readVerifFile: entry" << endl;
  ifstream istm( inVerif.c_str());
  if (istm == NULL) throwerr("inVerif file not found");
  string inLine;

  // Read header
  zverNum = 0;
  yverNum = 0;
  xverNum = 0;
  while (true) {
    getline( istm, inLine, '\n');
    if (istm.eof()) throwerr("eof on inVerif");
    if (inLine == "data:") break;

    if (inLine.size() > 0 && inLine.at(0) != '#') {
      vector<string> tokens;
      splitString( inLine, " \t", tokens);
      if (tokens.at(0) == "projection:") {
        if (tokens.size() != 4) throwerr("invalid header spec for projection");
        string synWindName = tokens.at(1);
        double synWindLat0 = parseDouble("projection lat0", tokens.at(2));
        double synWindLon0 = parseDouble("projection lon0", tokens.at(3));
        if (synWindName != projName) throwerr("projName mismatch");
        if (synWindLat0 != projLat0) throwerr("projLat0 mismatch");
        if (synWindLon0 != projLon0) throwerr("projLon0 mismatch");
      }
      else if (tokens.at(0) == "z:") {
        if (tokens.size() != 5) throwerr("invalid header spec for z");
        zverNum = parseLong("zverNum", tokens.at(1));
        zvergridmin = parseDouble("zvergridmin", tokens.at(2));
        zvergridmax = parseDouble("zvergridmax", tokens.at(3));
        zvergridinc = parseDouble("zvergridinc", tokens.at(4));
      }
      else if (tokens.at(0) == "y:") {
        if (tokens.size() != 5) throwerr("invalid header spec for y");
        yverNum = parseLong("yverNum", tokens.at(1));
        yvergridmin = parseDouble("yvergridmin", tokens.at(2));
        yvergridmax = parseDouble("yvergridmax", tokens.at(3));
        yvergridinc = parseDouble("yvergridinc", tokens.at(4));
      }
      else if (tokens.at(0) == "x:") {
        if (tokens.size() != 5) throwerr("invalid header spec for x");
        xverNum = parseLong("xverNum", tokens.at(1));
        xvergridmin = parseDouble("xvergridmin", tokens.at(2));
        xvergridmax = parseDouble("xvergridmax", tokens.at(3));
        xvergridinc = parseDouble("xvergridinc", tokens.at(4));
      }
      else throwerr("invalid inVerif header line");
    }
  }
  checkGrid( "verz", epsilon, zverNum, zvergridmin, zvergridmax, zvergridinc);
  checkGrid( "very", epsilon, yverNum, yvergridmin, yvergridmax, yvergridinc);
  checkGrid( "verx", epsilon, xverNum, xvergridmin, xvergridmax, xvergridinc);
  cout << setprecision(7);
  cout << "verz: zverNum: " << zverNum
       << "  zvergridmin: " << zvergridmin
       << "  zvergridmax: " << zvergridmax
       << "  zvergridinc: " << zvergridinc
       << endl;
  cout << "very: yverNum: " << yverNum
       << "  yvergridmin: " << yvergridmin
       << "  yvergridmax: " << yvergridmax
       << "  yvergridinc: " << yvergridinc
       << endl;
  cout << "verx: xverNum: " << xverNum
       << "  xvergridmin: " << xvergridmin
       << "  xvergridmax: " << xvergridmax
       << "  xvergridinc: " << xvergridinc
       << endl;

  // Allocate arrays
  verifWmat = new double**[ zverNum];
  verifVmat = new double**[ zverNum];
  verifUmat = new double**[ zverNum];
  for (long iz = 0; iz < zverNum; iz++) {
    verifWmat[iz] = new double*[yverNum];
    verifVmat[iz] = new double*[yverNum];
    verifUmat[iz] = new double*[yverNum];
    for (long iy = 0; iy < yverNum; iy++) {
      verifWmat[iz][iy] = new double[xverNum];
      verifVmat[iz][iy] = new double[xverNum];
      verifUmat[iz][iy] = new double[xverNum];
      for (long ix = 0; ix < xverNum; ix++) {
        verifWmat[iz][iy][ix] = numeric_limits<double>::quiet_NaN();
        verifVmat[iz][iy][ix] = numeric_limits<double>::quiet_NaN();
        verifUmat[iz][iy][ix] = numeric_limits<double>::quiet_NaN();
      } // for ix
    } // for iy
  } // for iz

  // Read the data
  Bbox * verLocBbox = new Bbox();     // overall bounding box of verif locs
  Bbox * verVelBbox = new Bbox();     // overall bounding box of verif velocities
  while (true) {
    getline( istm, inLine, '\n');
    if (istm.eof()) break;
    if (inLine.size() > 0 && inLine.at(0) != '#') {
      vector<string> tokens;
      splitString( inLine, " \t", tokens);
      if (tokens.size() != 10) throwerr("wrong num toks");
      double xval = parseDouble( "x", tokens.at(0));
      double yval = parseDouble( "y", tokens.at(1));
      double zval = parseDouble( "z", tokens.at(2));
      double uval = parseDouble( "u", tokens.at(3));
      double vval = parseDouble( "v", tokens.at(4));
      double wval = parseDouble( "w", tokens.at(5));
      long iz = roundl( (zval - zvergridmin) / zvergridinc);
      long iy = roundl( (yval - yvergridmin) / yvergridinc);
      long ix = roundl( (xval - xvergridmin) / xvergridinc);
      if (fabs( iz - (zval - zvergridmin) / zvergridinc) > epsilon)
        throwerr("invalid zval");
      if (fabs( iy - (yval - yvergridmin) / yvergridinc) > epsilon)
        throwerr("invalid yval");
      if (fabs( ix - (xval - xvergridmin) / xvergridinc) > epsilon)
        throwerr("invalid xval");
      verifWmat[iz][iy][ix] = wval;
      verifVmat[iz][iy][ix] = vval;
      verifUmat[iz][iy][ix] = uval;
      verLocBbox->addOb( zval, yval, xval);
      verVelBbox->addOb( wval, vval, uval);

      if (bugs >= 10) {
        cout << setprecision(7);
        cout << "verif: z y x:"
          << " " << zval
          << " " << yval
          << " " << xval
          << "  iz iy ix:"
          << " " << iz
          << " " << iy
          << " " << ix
          << "  w v u:"
          << " " << wval
          << " " << vval
          << " " << uval
          << endl;
      }
    } // if line is not a comment
  } // while true
  istm.close();

  cout << endl;
  cout << "verLocBbox:" << endl;
  verLocBbox->print();
  cout << endl;

  cout << endl;
  cout << "verVelBbox:" << endl;
  verVelBbox->print();
  cout << endl;

  // Make sure the verif arrays got filled
  for (long iz = 0; iz < zverNum; iz++) {
    for (long iy = 0; iy < yverNum; iy++) {
      for (long ix = 0; ix < xverNum; ix++) {
        if (! isOkDouble( verifWmat[iz][iy][ix])) throwerr("wmat not filled");
        if (! isOkDouble( verifVmat[iz][iy][ix])) throwerr("vmat not filled");
        if (! isOkDouble( verifUmat[iz][iy][ix])) throwerr("umat not filled");
      } // for ix
    } // for iy
  } // for iz
} // end readVerifFile


//======================================================================



// Return a list of filenames in the given dir that
// match the specified regex.

vector<FileSpec *>* RadarWind::readDir(
  string dirName,
  string fileRegex)
{
  vector<FileSpec *>* resvec = new vector<FileSpec *>();
  DIR * fdir = opendir( dirName.c_str());
  if (fdir == NULL) badparms("cannot open inDir");
  while (true) {
    struct dirent * dent = readdir( fdir);
    if (dent == NULL) break;
    string fname = dent->d_name;

    regex_t regexWk;
    if (0 != regcomp( &regexWk, fileRegex.c_str(), REG_EXTENDED))
      badparms("invalid fileRegex");
    if (0 == regexec( &regexWk, fname.c_str(), 0, NULL, 0)) {
      string fpath = dirName + "/" + fname;
      struct stat filestat;
      if (0 == stat( fpath.c_str(), &filestat)) {
        if (S_ISREG( filestat.st_mode)) {
          resvec->push_back( new FileSpec( fpath));
        }
      }
    }
  }
  return resvec;
}


//==================================================================


// Read a text file with one file spec per line.
// Return a vector of FileSpec.
// A # in the first column starts a comment.
// Each line has the format
//   fileName altKmMsl latDeg lonDeg

vector<FileSpec *>* RadarWind::readFileList(
  string fileListName)
{
  vector<FileSpec *>* resvec = new vector<FileSpec *>();

  ifstream istm( fileListName.c_str());
  if (istm == NULL) throwerr("fileList file not found");
  string inLine;

  while (true) {
    getline( istm, inLine, '\n');
    if (istm.eof()) break;
    if (inLine.size() > 0 && inLine.at(0) != '#') {
      vector<string> tokens;
      splitString( inLine, " \t", tokens);
      if (tokens.size() != 4) throwerr("wrong num toks");

      string fpath = tokens[0];
      double altKmMsl = parseDouble( "altKmMsl", tokens.at(1));
      double latDeg = parseDouble( "latDeg", tokens.at(2));
      double lonDeg = parseDouble( "lonDeg", tokens.at(3));

      resvec->push_back( new FileSpec( fpath, altKmMsl, latDeg, lonDeg));
    }
  }
  return resvec;
} // end readFileList





//==================================================================


// Write the variables in the cellMat to the Netcdf file outNc.

void RadarWind::writeNetcdf(
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
  Cell ***& cellMat,
  string outNc)
{

  // Create linear arrays
  long totalLen = nradz * nrady * nradx;
  float * wLinear = new float[ totalLen];
  float * vLinear = new float[ totalLen];
  float * uLinear = new float[ totalLen];
  float * dbzLinear = new float[ totalLen];
  float * ncpLinear = new float[ totalLen];
  float * condNumLinear = new float[ totalLen];
  for (long iz = 0; iz < nradz; iz++) {
    for (long iy = 0; iy < nrady; iy++) {
      for (long ix = 0; ix < nradx; ix++) {

        long kk = iz * nrady * nradx + iy * nradx + ix;    // linear index
        wLinear[kk] = cellMat[iz][iy][ix].ww;
        vLinear[kk] = cellMat[iz][iy][ix].vv;
        uLinear[kk] = cellMat[iz][iy][ix].uu;
        dbzLinear[kk] = cellMat[iz][iy][ix].meanNbrDbz;
        ncpLinear[kk] = cellMat[iz][iy][ix].meanNbrNcp;
        condNumLinear[kk] = cellMat[iz][iy][ix].conditionNumber;

      } // for ix
    } // for iy
  } // for iz

  string fname;
  if (outNc[outNc.length()-1] == '/') {
    // file name format: outNc/yyyymmdd/ncf_yyyymmdd_hhmmss.nc                                       
    time_t tm = timeMax;
    struct tm tmstr;
    gmtime_r( &tm, &tmstr);

    char tbuf[1000];
    strftime( tbuf, 1000, "%Y%m%d%H%M%S", &tmstr);
    string fulldate(tbuf, 0, 14);
    string subdirname = outNc + fulldate.substr( 0, 8);
    if (bugs >= 1) {
      cout << "fulldate: \"" << fulldate << "\"" << endl;
      cout << "subdirname: \"" << subdirname << "\"" << endl;
    }
    mkdir( subdirname.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
    
    fname = subdirname + "/ncf_" + fulldate.substr( 0, 8) + "_"
      + fulldate.substr( 8, 6) + ".nc";
  }
  else {
    fname = outNc;
  }
  if (bugs >= 1) cout << "fname: \"" << fname << "\"" << endl;
  Nc3File ncout( fname.c_str(), Nc3File::Replace);
  ncout.add_att("Conventions", "CF-1.5");

  // Dimension arrays
  int ntime = 1;
  double * tvals = new double[ ntime];
  tvals[0] = timeMax;

  double * zvals = new double[ nradz];
  for (long ii = 0; ii < nradz; ii++) {
    zvals[ii] = 1000 * (zgridmin + ii * zgridinc);  // convert km to m
  }

  double * yvals = new double[ nrady];
  for (long ii = 0; ii < nrady; ii++) {
    yvals[ii] = ygridmin + ii * ygridinc;
  }

  double * xvals = new double[ nradx];
  for (long ii = 0; ii < nradx; ii++) {
    xvals[ii] = xgridmin + ii * xgridinc;
  }

  // Dimensions
  Nc3Dim * tdim = ncout.add_dim("time", ntime);
  Nc3Dim * zdim = ncout.add_dim("z0", nradz);
  Nc3Dim * ydim = ncout.add_dim("y0", nrady);
  Nc3Dim * xdim = ncout.add_dim("x0", nradx);

  const Nc3Dim * dataDims4[] = { tdim, zdim, ydim, xdim};
  const Nc3Dim * dataDims2[] = { ydim, xdim};
  const Nc3Dim * dataDims1[] = { tdim};

  // Dimension vars
  Nc3Var * tvar = ncout.add_var("time", nc3Double, tdim);
  tvar->add_att("standard_name", "time");
  tvar->add_att("units", "seconds since 1970-01-01T00:00:00Z");
  tvar->add_att("calendar", "gregorian");

  Nc3Var * zvar = ncout.add_var("z0", nc3Double, zdim);
  zvar->add_att("standard_name", "height");
  zvar->add_att("units", "m");
  zvar->add_att("positive", "up");

  Nc3Var * yvar = ncout.add_var("y0", nc3Double, ydim);
  yvar->add_att("standard_name", "projection_y_coordinate");
  yvar->add_att("units", "m");

  Nc3Var * xvar = ncout.add_var("x0", nc3Double, xdim);
  xvar->add_att("standard_name", "projection_x_coordinate");
  xvar->add_att("units", "m");

  // Metadata vars
  Nc3Var * gridMapVar = ncout.add_var("grid_mapping_0", nc3Int);
  gridMapVar->add_att("grid_mapping_name", "transverse_mercator");
  gridMapVar->add_att("semi_major_axis", earthRadiusMeter);
  gridMapVar->add_att("semi_minor_axis", earthRadiusMeter);
  gridMapVar->add_att("inverse_flattening", 1.0 / flattening);
  gridMapVar->add_att("latitude_of_projection_origin", projLat0);
  gridMapVar->add_att("longitude_of_projection_origin", projLon0);
  gridMapVar->add_att("false_easting", 0);
  gridMapVar->add_att("false_northing", 0);

  Nc3Var * latVar = ncout.add_var("lat0", nc3Double, 2, dataDims2);
  latVar->add_att("standard_name", "latitude");
  latVar->add_att("units", "degrees_north");

  Nc3Var * lonVar = ncout.add_var("lon0", nc3Double, 2, dataDims2);
  lonVar->add_att("standard_name", "longitude");
  lonVar->add_att("units", "degrees_east");

  // Data variables

  Nc3Var * timeStartVar = ncout.add_var("start_time", nc3Double, 1, dataDims1);
  timeStartVar->add_att("standard_name", "start_time");
  timeStartVar->add_att("units", "s");

  Nc3Var * timeStopVar = ncout.add_var("stop_time", nc3Double, 1, dataDims1);
  timeStopVar->add_att("standard_name", "stop_time");
  timeStopVar->add_att("units", "s");

  Nc3Var * wVar = ncout.add_var("upward_air_velocity", nc3Float, 4, dataDims4);
  wVar->add_att("standard_name", "upward_air_velocity");
  wVar->add_att("units", "m s-1");
  wVar->add_att("grid_mapping", "grid_mapping_0");

  Nc3Var * vVar = ncout.add_var("northward_wind", nc3Float, 4, dataDims4);
  vVar->add_att("standard_name", "northward_wind");
  vVar->add_att("units", "m s-1");
  vVar->add_att("grid_mapping", "grid_mapping_0");

  Nc3Var * uVar = ncout.add_var("eastward_wind", nc3Float, 4, dataDims4);
  uVar->add_att("standard_name", "eastward_wind");
  uVar->add_att("units", "m s-1");
  uVar->add_att("grid_mapping", "grid_mapping_0");

  Nc3Var * dbzVar = ncout.add_var("meanNbrDbz", nc3Float, 4, dataDims4);
  dbzVar->add_att("standard_name", "mean_neighbor_dbz");
  dbzVar->add_att("units", "1");  // Apparently CF uses "1" to mean ratio
  dbzVar->add_att("grid_mapping", "grid_mapping_0");

  Nc3Var * ncpVar = ncout.add_var("meanNbrNcp", nc3Float, 4, dataDims4);
  ncpVar->add_att("standard_name", "mean_neighbor_ncp");
  ncpVar->add_att("units", "1");  // Apparently CF uses "1" to mean ratio
  ncpVar->add_att("grid_mapping", "grid_mapping_0");

  Nc3Var * condNumVar = ncout.add_var("conditionNumber", nc3Float, 4, dataDims4);
  condNumVar->add_att("standard_name", "matrix_condition_number");
  condNumVar->add_att("units", "1");
  condNumVar->add_att("grid_mapping", "grid_mapping_0");

  // Write coord vars
  tvar->put( tvals, ntime);
  zvar->put( zvals, nradz);
  yvar->put( yvals, nrady);
  xvar->put( xvals, nradx);

  // Write metadata vars
  int gridMapValue = 0;
  gridMapVar->put( &gridMapValue, 1);

  double * latLinear = new double[nrady * nradx];
  double * lonLinear = new double[nrady * nradx];
  for (long iy = 0; iy < nrady; iy++) {
    for (long ix = 0; ix < nradx; ix++) {
      double latDeg;
      double lonDeg;
      double coordy = ygridmin + iy * ygridinc;
      double coordx = xgridmin + ix * xgridinc;
      yxToLatLon( tranMerc, projLon0,
        basey,
        basex,
        coordy,
        coordx,
        latDeg,             // output value
        lonDeg);            // output value
      long kk = iy * nradx + ix;    // linear index
      latLinear[kk] = latDeg;
      lonLinear[kk] = lonDeg;
    } // for ix
  } // for iy
  long counts2[] = { nrady, nradx};
  latVar->put( latLinear, counts2);
  lonVar->put( lonLinear, counts2);

  delete[] latLinear;
  delete[] lonLinear;






  // Write data vars
  long counts1[] = { ntime};
  double * timeStarts = new double[ntime];
  double * timeStops = new double[ntime];
  for (int ii = 0; ii < ntime; ii++) {
    timeStarts[ii] = timeMin;
    timeStops[ii] = timeMax;
  }
  timeStartVar->put( timeStarts, counts1);
  timeStopVar->put( timeStops, counts1);
  delete[] timeStarts;
  delete[] timeStops;


  long counts4[] = { ntime, nradz, nrady, nradx};
  wVar->put( wLinear, counts4);
  vVar->put( vLinear, counts4);
  uVar->put( uLinear, counts4);
  dbzVar->put( dbzLinear, counts4);
  ncpVar->put( ncpLinear, counts4);
  condNumVar->put( condNumLinear, counts4);

  // The Nc3File destructor automatically closes the file.

  delete[] tvals;
  delete[] zvals;
  delete[] yvals;
  delete[] xvals;

  delete[] wLinear;
  delete[] vLinear;
  delete[] uLinear;
  delete[] dbzLinear;
  delete[] ncpLinear;
  delete[] condNumLinear;

} // end writeNetcdf


//==================================================================


// Check that the grid specification is valid.
// This is called 3 times: for z, y, and x grids.

void RadarWind::checkGrid(
  const char * msg,
  double epsilon,
  long num,
  double gridmin,
  double gridmax,
  double gridinc)
{
  string errmsg = "";
  if (num <= 0) errmsg = "grid num <= 0";
  if (fabs( gridmin - gridinc * (gridmin / gridinc)) > epsilon)
    errmsg = "grid min is not a multiple of gridinc";
  if (fabs( gridmax - gridinc * (gridmax / gridinc)) > epsilon)
    errmsg = "grid max is not a multiple of gridinc";
  long ntest = lround( (gridmax - gridmin) / gridinc) + 1;
  if (ntest != num) errmsg = "grid num mismatch";
  if (errmsg != "") {
    cout << setprecision(15);
    cout << endl;
    cout << "grid mismatch.  msg: " << msg << endl;
    cout << "  epsilon: " << epsilon << endl;
    cout << "  num: " << num << endl;
    cout << "  gridmin: " << gridmin << endl;
    cout << "  gridmax: " << gridmax << endl;
    cout << "  gridinc: " << gridinc << endl;
    cout << "  ntest: " << ntest << endl;
    throwerr("grid mismatch");
  }
} // end checkGrid


//==================================================================


// Given a user specified specification triple synWinds,
// calculate the synthetic winds at the specified point.
// The synWinds triple contains: (zspec, yspec, xspec).
// If the spec > SYNFUNC_START (which is way negative),
// just set the wind to the spec value.
// Otherwise the spec is some function id like SYNFUNC_FUNZ,
// which means windComponent = sin(locz).

void RadarWind::calcSyntheticWinds(
  bool showDetail,
  double * synWinds,    // user specified winds
  double locz,          // wind location
  double locy,
  double locx,
  double * vels)        // returned W, V, U
{
  for (int ii = 0; ii < 3; ii++) {
    double prm = synWinds[ii];
    if (prm > SYNFUNC_START) vels[ii] = prm;
    else if (prm == SYNFUNC_SINZ) vels[ii] = sin( locz);
    else if (prm == SYNFUNC_SINY) vels[ii] = sin( locy);
    else if (prm == SYNFUNC_SINX) vels[ii] = sin( locx);
    else throwerr("unknown synWinds");
  }
  if (showDetail) {
    cout << setprecision(5);
    cout << "      calcsyn:" << endl
      << "        locz: " << locz << "  locy: " << locy
      << "  locx: " << locx << endl
      << "        velW: " << vels[0] << "  velV: " << vels[1]
      << "  velU: " << vels[2] << endl;
  }
} // end calcSyntheticWinds

//==================================================================


// Beltrami flow equations
// See publications by Michael Bell

void RadarWind::calcBeltramiFlow(
  bool showDetail,
  double zz,            // location
  double yy,
  double xx,
  double * vels)        // returned W, V, U
{

  // Beltrami flow equations
                                    // Michael Bell's notation:
  double ubase = 10;                // U: mean U wind
  double vbase = 10;                // V: mean V wind
  double wbase = 10;                // A: peak vertical velocity
  double mc = 2 * M_PI / 32000;     // m: vertical wavelength
  double kc = 2 * M_PI / 16000;     // k: horizontal wavelength
  double lc = kc;                   // l

  double amp = wbase / (kc*kc + lc*lc);
  double wavenum = sqrt(kc*kc + lc*lc + mc*mc);
  double tm = 0.;     // time?
  double nu = 15.11e-6;

  zz *= 1000.;         // z in meters
  yy *= 1000.;         // y in meters
  xx *= 1000.;         // x in meters

  double uu = ubase
    - amp
      * exp(-nu * wavenum * wavenum * tm)
      * ( wavenum * lc * cos( kc * (xx - ubase*tm))
            * sin( lc * (yy-vbase*tm))
            * sin(mc*zz)
          + mc * kc * sin( kc*(xx-ubase*tm))
            * cos(lc * (yy-vbase*tm))*cos(mc*zz));

  double dudx = - amp
    * exp( -nu*wavenum*wavenum*tm)
    * ( - wavenum * kc * lc * sin( kc * (xx - ubase*tm))
      * sin( lc*(yy-vbase*tm))
      * sin( mc*zz)
    + mc * kc * kc *cos( kc*(xx-ubase*tm))
      * cos( lc * (yy-vbase*tm)) * cos( mc*zz));

  double dudy = - amp
    * exp( -nu*wavenum*wavenum*tm)
    * ( wavenum * lc * cos(kc * (xx - ubase*tm))
        * lc * cos( lc*(yy-vbase*tm)) * sin( mc*zz)
      - mc * lc * kc * sin(kc*(xx-ubase*tm))
        * sin( lc * (yy-vbase*tm)) * cos(mc*zz));

  double vv = vbase
    + amp
      * exp(-nu * wavenum * wavenum * tm)
      * ( wavenum * kc * sin( kc * (xx - ubase*tm))
            * cos( lc * (yy-vbase*tm))
            * sin( mc * zz)
          - mc * lc * cos( kc*(xx-ubase*tm))
            * sin( lc*(yy-vbase*tm))*cos(mc*zz));

  double dvdx = amp
    * exp( -nu*wavenum*wavenum*tm)
    * ( wavenum * kc * kc * cos( kc*(xx - ubase*tm))
          * cos( lc*(yy-vbase*tm)) * sin( mc*zz)
        + mc * lc * kc * sin( kc*(xx-ubase*tm))
          * sin( lc * (yy-vbase*tm)) * cos( mc*zz));

  double dvdy = amp
    * exp( -nu*wavenum*wavenum*tm)
    * ( -wavenum * kc * lc * sin( kc * (xx - ubase*tm))
      * sin( lc * (yy-vbase*tm)) * sin( mc*zz)
    - mc * lc * lc * cos( kc*(xx-ubase*tm))
      * cos( lc*(yy-vbase*tm)) * cos(mc*zz));

  double ww = wbase
    * cos( kc*(xx-ubase*tm))
    * cos( lc*(yy-vbase*tm))
    * sin(mc*zz)
    * exp( -nu*wavenum*wavenum*tm);

  double vort = 1e5 * (dvdx - dudy);
  double div = 1e5 * (dudx + dvdy);

  vels[0] = ww;
  vels[1] = vv;
  vels[2] = uu;

} // end calcBeltramiFlow


//==================================================================


// Given synthetic W, V, U, calculate the radial velocity
// the radar would have observed.

double RadarWind::calcRadialVelocity(
  bool showDetail,
  double thetaRad,      // polar coord angle from observer, radians
  double elevRad,       // elevation angle from observer, radians
  double * vels)        // W, V, U
{
  double velW = vels[0];
  double velV = vels[1];
  double velU = vels[2];

  // Calc radial velocity
  // The view angle can be expressed as a vector having length 1, as
  //   view = (x=cos(theta)*cos(elev), y=sin(theta)*cos(elev), z=sin(elev))
  //
  // The radial velocity is the dot product
  //   view dot (U, V, W)

  double velRadial
    =   cos(thetaRad) * cos(elevRad) * velU
      + sin(thetaRad) * cos(elevRad) * velV
      + sin(elevRad) * velW;

  if (showDetail) {
    cout << setprecision(5);
    cout << "      calcRadial:" << endl
      << "        thetaRad: " << thetaRad
      << "  thetaDeg: " << (thetaRad * 180 / M_PI) << endl
      << "        elevRad: " << elevRad
      << "  elevDeg: " << (elevRad * 180 / M_PI) << endl
      << "        velW: " << vels[0] << "  velV: " << vels[1]
      << "  velU: " << vels[2] << endl
      << "        velRadial: " << velRadial
      << endl;
  }
  return velRadial;
} // end calcRadialVelocity



//==================================================================


// Convert lat,lon coords to Y,X using our transverse mercator projection.

void RadarWind::latLonToYX(
  const GeographicLib::TransverseMercatorExact * tranMerc,
  double projLon0Deg,     // central meridian of projection
  double basey,           // coord y base, km
  double basex,           // coord x base, km
  double latDeg,          // latitude
  double lonDeg,          // longitude
  double & coordy,        // output coord y, km
  double & coordx)        // output coord x, km
{
  tranMerc->Forward(
    projLon0Deg,
    latDeg,
    lonDeg,
    coordx,             // output value
    coordy);            // output value
  coordy = 0.001 * coordy - basey;      // convert meters to km
  coordx = 0.001 * coordx - basex;      // convert meters to km
}





//==================================================================


// Convert Y, X to lat,lon coords using our transverse mercator projection.

void RadarWind::yxToLatLon(
  const GeographicLib::TransverseMercatorExact * tranMerc,
  double projLon0,        // central meridian of projection
  double basey,           // coord y base, km
  double basex,           // coord x base, km
  double coordy,          // coord y, km
  double coordx,          // coord x, km
  double & latDeg,        // output latitude
  double & lonDeg)        // output longitude
{
  tranMerc->Reverse(
    projLon0,
    1000 * (basex + coordx),
    1000 * (basey + coordy),
    latDeg,             // output value
    lonDeg);            // output value
}






//==================================================================


// Calc distance from a location to Point, in km.

double RadarWind::calcDistLocPt( double * loc, Point * pta) {
  double sumsq = 0;
  double delta;
  delta = loc[0] - pta->coordz;
  sumsq += delta*delta;
  delta = loc[1] - pta->coordy;
  sumsq += delta*delta;
  delta = loc[2] - pta->coordx;
  sumsq += delta*delta;
  return sqrt( sumsq);
}



//==================================================================


// Calc distance between two Points, in km.


double RadarWind::calcDistPtPt( Point * pta, Point * ptb) {
  double sumsq = 0;
  double delta;
  delta = pta->coordz - ptb->coordz;
  sumsq += delta*delta;
  delta = pta->coordy - ptb->coordy;
  sumsq += delta*delta;
  delta = pta->coordx - ptb->coordx;
  sumsq += delta*delta;
  return sqrt( sumsq);
}




//==================================================================


// Calc distance from between a Point and the aircraft


double RadarWind::calcDistPtAircraft( Point * pta) {
  double sumsq = 0;
  double delta;
  delta = pta->coordz - pta->aircraftz;
  sumsq += delta*delta;
  delta = pta->coordy - pta->aircrafty;
  sumsq += delta*delta;
  delta = pta->coordx - pta->aircraftx;
  sumsq += delta*delta;
  return sqrt( sumsq);
}


//==================================================================


// Return true if (z,y,x) is within the region specified
// by detailSpec.  The detailSpec contains:
//   [0]: zcenter
//   [1]: ycenter
//   [2]: xcenter
//   [3]: radius about the center

bool RadarWind::testDetail(
  double zloc,
  double yloc,
  double xloc,
  double * detailSpec)         // z, y, x, delta
{
  bool showDetail = false;
  if (detailSpec != NULL
    && zloc >= detailSpec[0] - detailSpec[3]
    && zloc <= detailSpec[0] + detailSpec[3]
    && yloc >= detailSpec[1] - detailSpec[3]
    && yloc <= detailSpec[1] + detailSpec[3]
    && xloc >= detailSpec[2] - detailSpec[3]
    && xloc <= detailSpec[2] + detailSpec[3])
  {
    showDetail = true;
  }
  return showDetail;
}



//==================================================================



// Not used.
// Someday we will use this for testing.

long RadarWind::getLongRandom(
  long vmin,
  long vlim,
  random_data *randInfo)
{
  int32_t rr = 0;
  if (0 != random_r( randInfo, &rr)) throwerr("random_r error");
  double frac = rr / (((double) RAND_MAX) + 1);
  long res = vmin + (long) (frac * (vlim - vmin));
  if (res < 0 || res >= vlim) throwerr("invalid getRandLong res");
  return res;
}



//==================================================================


// Return true if val is a normal value, not NaN or +/-inf.

bool RadarWind::isOkDouble( double val) {
  bool res = false;
  if (val == 0 || isnormal( val)) res = true;
  return res;
}


//==================================================================


// Return true if val is a normal value, not NaN or +/-inf.

bool RadarWind::isOkFloat( float val) {
  bool res = false;
  if (val == 0 || isnormal( val)) res = true;
  return res;
}


//==================================================================


// Print the elapsed run time since the previous call, in seconds.

void RadarWind::printRunTime(
  const string& str,
  struct timeval * ptva)
{
  struct timeval tvb;
  if (gettimeofday( &tvb, NULL) != 0) throwerr("gettimeofday err");
  double deltaSec = tvb.tv_sec - ptva->tv_sec
    + 1.e-6 * (tvb.tv_usec - ptva->tv_usec);
  cout << "runTime: " << setw(20) << str << ": " << deltaSec << endl;
  ptva->tv_sec = tvb.tv_sec;
  ptva->tv_usec = tvb.tv_usec;
}




//==================================================================


// Add the elapsed run time since the previous call, in seconds.

void RadarWind::addDeltaTime(
  struct timeval * ptva,
  double * psum)
{
  struct timeval tvb;
  if (gettimeofday( &tvb, NULL) != 0) throwerr("gettimeofday err");
  double deltaSec = tvb.tv_sec - ptva->tv_sec
    + 1.e-6 * (tvb.tv_usec - ptva->tv_usec);
  ptva->tv_sec = tvb.tv_sec;
  ptva->tv_usec = tvb.tv_usec;
  if (psum != NULL) (*psum) += deltaSec;
}




//==================================================================

// Split a string into a vector of tokens.
// Why does C++ make this so weird?

void RadarWind::splitString(
  const string& str,
  const string& delimiters,
  vector<string>& tokens)       // appended
{
  // Scan to find beginning of first token.
  long begPos = str.find_first_not_of(delimiters, 0);

  while (begPos != string::npos) {
    // Find end of this token
    long endPos = str.find_first_of(delimiters, begPos);
    tokens.push_back( str.substr( begPos, endPos - begPos));
    // Find start of next token
    begPos = str.find_first_not_of( delimiters, endPos);
  }
}


//==================================================================


// format a time, stored as double seconds since 1970.

string RadarWind::formatTime( double dtm) {
  time_t itime = (time_t) dtm;
  struct tm stm;
  gmtime_r( &itime, &stm);

  char bufa[1000];
  strftime( bufa, 1000, "%Y-%m-%dT%H:%M:%S", &stm);

  long ifrac = 1000 * (dtm - itime);
  char bufb[100];
  snprintf( bufb, 100, "%03d", ifrac);
  string stg = bufa;
  stg += ".";
  stg += bufb;
  return stg;
}


//==================================================================


// Converts string to long.

long RadarWind::parseLong( string msg, string stg) {
  char * endptr;
  const char * stgc = stg.c_str();
  long ires = strtol( stgc, &endptr, 10);
  if (endptr != stgc + strlen( stgc)) throwerr("invalid integer");
  return ires;
}


//==================================================================


// Converts string to double.

double RadarWind::parseDouble( string msg, string stg) {
  char * endptr;
  const char * stgc = stg.c_str();
  double dres = strtod( stgc, &endptr);
  if (endptr != stgc + strlen( stgc)) throwerr("invalid number");
  return dres;
}


//==================================================================


// Converts string (y or n) to bool.

bool RadarWind::parseBool( string msg, string stg) {
  bool bres = false;
  if (stg == "n") bres = false;
  else if (stg == "y") bres = true;
  else throwerr("invalid bool");
  return bres;
}



//==================================================================


// Throws an exception

void RadarWind::throwerr( const char * msg, ...) {
  int nbufa = 10000;
  char bufa[10000];

  va_list arglist;
  va_start( arglist, msg);
  vsnprintf( bufa, nbufa, msg, arglist);
  va_end( arglist);

  cerr << "RadarWind: throwerr: " << bufa << endl;
  cerr.flush();
  throw bufa;
}


//==================================================================


// Not used.
//
// Check that the values in smap( double value -> long numOfOccurances)
// represent a linear sequence.
//
//LinearSpec * getLinearSpec(
//  const char * name,
//  map<double,long> * smap)
//{
//  double firstVal = smap.begin()->first;
//  double lastVal = smap.rbegin()->first;
//  long nn = smap.size();
//  double incr = (lastVal - firstVal) / nn;
//
//  cout << endl;
//  map<double,long>::iterator itera = smap.begin();
//  for (int ii = 0; ii < nn; ii++) {
//    if (itera == smap.end()) throwerr("hit end");
//    double val = (*itera).first;
//    long cnt = (*itera).second;
//    cout << "  " << name << ": value: " << val
//      << "  count: " << cnt << endl;
//    if (fabs( val - (firstVal + ii * incr)) > epsilon)
//      throwerr("not linear");
//    itera++;
//  }
//  if (itera != smap.end()) throwerr("missing end");
//  return new LinearSpec( nn, firstVal, incr);
//
//}








