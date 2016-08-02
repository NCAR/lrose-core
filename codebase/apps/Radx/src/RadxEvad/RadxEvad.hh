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
// RadxEvad.hh
//
// RadxEvad object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2013
//
///////////////////////////////////////////////////////////////

#ifndef RadxEvad_H
#define RadxEvad_H

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <Radx/Radx.hh>
#include <Radx/RadxVol.hh>
class RadxFile;
using namespace std;

class RadxEvad {
  
public:

  // constructor
  
  RadxEvad (int argc, char **argv);

  // destructor
  
  ~RadxEvad();

  // run 

  int Run();

  // data members

  int OK;

  const static double missingVal;
  const static double pseudoEarthDiamKm;

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  vector<string> _readPaths;

  /////////////////////////////////////////
  // input data

  RadxVol _readVol;

  int _nGates;
  double _nyquist;
  double _radxStartRange;
  double _radxGateSpacing;

  double _radarHtMeters;

  string _radarName;
  double _radarLatitude;
  double _radarLongitude;
  double _radarAltitude;
  
  // These are pointers into the input Radx object.
  // This memory is managed by the Radx class and should not be freed
  // by the calling class.

  const Radx::fl32 *_dbz;
  const Radx::fl32 *_vel;
  const Radx::fl32 *_snr;
  const Radx::fl32 *_censor;

  Radx::fl32 _dbzMiss;
  Radx::fl32 _snrMiss;
  Radx::fl32 _velMiss;
  Radx::fl32 _censorMiss;

  // matrices for wind computations

  const static int nFourierCoeff = 7;
  double _aa[nFourierCoeff];
  double _bb[nFourierCoeff];
  double _ff[nFourierCoeff];
  double **_AA;
  double **_AAinverse;

  // marshalling the velocity data around the circle

  int _nRanges;

  class VelPt {
  public:
    double az;
    double vel;
  };

  class AzSlice {
  public:
    int num;
    bool valid;
    double startAz;
    double meanAz;
    double endAz;
    double ff[nFourierCoeff];
    vector<VelPt> pts;
    double meanVel; // in sample volume
    double medianVel; // around the circle
    double modelVel; // velocity from the model fit
    bool zeroIsodop;
    bool foldBoundary;
    double unfoldedVel;
    int foldInterval;
  };

  int _nAzSlices;
  double _sliceDeltaAz;

  class VelRing {

  public:

    int sweepNum;
    int rangeNum;
    int levelNum;

    int startGate;
    int endGate;
    
    double elev;
    double midHt;
    double elevStar;   // elev at the ring range
    double elevCoeff; // (2 * sin(elevStar)) / (range * cos(elevStar))

    double startRange;
    double endRange;
    double midRange;

    vector<AzSlice> slices;
    vector<int> zeroIndices;
    vector<int> foldIndices;

    double fitVariance;
    double fitRmsError;

    double maxVr;
    double minVr;

    double azForMaxVr;
    double azForMinVr;
    double azError;
    
    double windSpeed;
    double windDirn;
    bool windValid;

    double aa[nFourierCoeff];
    double aaVariance[nFourierCoeff];

    double nk;     // number of rings at this level, for same elevation
    double weight; // for computing divergence

    double YY;
    double YYestimated;

    void clear() {
      slices.clear();
      zeroIndices.clear();
      foldIndices.clear();
    }

  };

  // working ring data
  
  VelRing _ring;

  // array of computed ring data, for valid results only

  vector<VelRing> _rings;

  // profile on constant height intervals

  double _profileMinHt;
  double _profileMaxHt;
  double _profileDeltaHt;
  int _profileNLevels;

  class ProfilePt {
  public:
    // constructor
    ProfilePt() :
            nrings(0),
            ht(missingVal),
            windSpeed(missingVal),
            windDirn(missingVal),
            uu(missingVal),
            vv(missingVal),
            ww(missingVal),
            wp(missingVal),
            div(missingVal),
            divPrime(missingVal),
            varDiv(missingVal),
            rmsDiv(missingVal),
            rho(missingVal),
            rmsRhoD(missingVal),
            varRhoD(missingVal) {}
    int nrings;
    double ht;
    double windSpeed;
    double windDirn;
    double uu;
    double vv;
    double ww;
    double wp;
    double div;
    double divPrime;
    double varDiv;
    double rmsDiv;
    double rho;
    double rmsRhoD;
    double varRhoD;
  };

  class Profile {
  public:
    vector<ProfilePt> raw;
    vector<ProfilePt> interp;
    void clear() {
      raw.clear();
      interp.clear();
    }
  };

  Profile _profile;

  int _ibotDiv;  // bottom interp layer with good divergence data
  int _itopDiv;  // top interp layer with good divergence data
  
  // method for comparing ProfilePt, for sorting in height

  class ProfilePtCompare {
  public:
    bool operator()(const ProfilePt &a, const ProfilePt &b) const {
      return a.ht < b.ht;
    }
  };

  // matrices for DIV computations

  const static int nDivCoeff = 2;
  double _pp[nDivCoeff];
  double _qq[nDivCoeff];
  double **_PP;
  double **_PPinverse;

  // methods
  
  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupRead(RadxFile &file);
  int _processFile(const string &filePath);

  int _processDataSet();

  int _computeSolutionForRing(int isweep, double elev, int irange);
  void _computeMeanVel(AzSlice &slice);
  void _computeMedianVel(int sliceNum);
  void _copyMedianIntoGaps();

  void _identifyFolds();
  int _unfoldFromZeroIsodop(int zeroIsodopIndex);
  int _unfoldIteratively();
  void _unfoldFromPreviousRing(double ht);

  void _computeWindForRing();

  void _loadProfile();

  void _computeDivergence();
  void _interpDivergence();

  void _computeVertVel(double wtop);
  void _computeFf(double az, double *ff);
  void _invertAA();
  void _invertPP();
  void _invertMatrix(double *data, int nn) const;

  void _printResultsForRing(ostream &out, const VelRing &ring);

  int _writeNetcdfOutput();
  int _writeSpdbOutput();

  int _getNValidLevels();

  static double _norm(double **a,int n);
  static int _doubleCompare(const void *i, const void *j);


};

#endif
