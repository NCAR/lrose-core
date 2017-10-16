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
// Interp.hh
//
// Interp base class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2012
//
///////////////////////////////////////////////////////////////

#ifndef Interp_HH
#define Interp_HH

#include "Params.hh"
#include <string>
#include <cmath>
#include <deque>
#include <Mdv/MdvxProj.hh>
#include <Radx/Radx.hh>
#include <radar/BeamHeight.hh>
class RadxRay;
class RadxVol;
using namespace std;

class Interp {
  
public:

  // field object class for interpolation

  class Field {
  public:
    string radxName;
    string outputName;
    string longName;
    string standardName;
    string units;
    bool fieldFolds;
    double foldLimitLower;
    double foldLimitUpper;
    double foldRange;
    bool isDiscrete;
    bool isBounded;
    double boundLimitLower;
    double boundLimitUpper;
    Radx::DataType_t inputDataType;
    double inputScale;
    double inputOffset;
    Field() {
      fieldFolds = false;
      isDiscrete = false;
      isBounded = false;
      boundLimitLower = 0.0;
      boundLimitUpper = 0.0;
      inputDataType = Radx::FL32;
      inputScale = 1.0;
      inputOffset = 0.0;
    }
  };

  // ray object class for interpolation

  class Ray {
  public:
    Ray(RadxRay *ray,
        int isweep,
        const vector<Field> &fields,
        bool use_fixed_angle_for_interpolation,
        bool use_fixed_angle_for_data_limits);
    ~Ray();
    RadxRay *inputRay;
    int sweepIndex;
    double el;
    double cosEl;
    double az;
    double elForLimits;
    double azForLimits;
    int nFields;
    int nGates;
    fl32 **fldData;
    fl32 *missingVal;
  };

  // class for output grid locations
  
  class GridLoc {
  public:
    GridLoc() {
      el = az = slantRange = gndRange = 0.0;
      xxInstr = yyInstr = zzInstr = zz = 0;
    }
    ~GridLoc() { }
    double el;
    double az;
    double slantRange;
    double gndRange;
    double xxInstr, yyInstr, zzInstr, zz;
  };
  GridLoc ****_gridLoc;

  // constructor
  
  Interp(const string &progName,
         const Params &params,
         RadxVol &readVol,
         vector<Field> &interpFields,
         vector<Ray *> &interpRays);
  
  // destructor
  
  virtual ~Interp();

  // interpolate a volume
  // assumes volume has been read
  // and _interpFields and _interpRays vectors are populated
  // returns 0 on succes, -1 on failure

  virtual int interpVol() = 0;

protected:

  static const double PseudoDiamKm; // for earth curvature correction
  static const double missingDouble;
  static const fl32 missingFl32;

  // references to main object
  
  string _progName;
  const Params &_params;
  RadxVol &_readVol;
  bool _rhiMode;
  vector<Field> &_interpFields;
  vector<Ray *> &_interpRays;

  // checking timing performance

  struct timeval _timeA;

  // radar location
  
  double _radarLat, _radarLon, _radarAltKm;
  double _prevRadarLat, _prevRadarLon, _prevRadarAltKm;
  
  // gate geometry

  int _maxNGates;
  double _startRangeKm;
  double _gateSpacingKm;
  double _maxRangeKm;

  // beam width

  double _beamWidthDegH;
  double _beamWidthDegV;

  // locating sectors

  class Sector {
  public:
    int startAzDeg;
    int endAzDeg;
    int width;
    Sector() {
      startAzDeg = -1;
      endAzDeg = -1;
      width = 0;
    }
    Sector(int start, int end) {
      startAzDeg = start;
      endAzDeg = end;
      computeWidth();
    }
    void computeWidth() {
      width = endAzDeg - startAzDeg + 1;
    }
  };

  bool _isSector;
  bool _spansNorth;
  double _dataSectorStartAzDeg;
  double _dataSectorEndAzDeg;

  // class for debug fields

  class DerivedField {
  public:
    string name;
    string longName;
    string units;
    vector<double> vertLevels;
    fl32 *data;
    bool writeToFile;
    DerivedField(const string &nameStr,
                 const string &longNameStr, 
                 const string &unitsStr,
                 bool writeOut) :
            name(nameStr),
            longName(longNameStr),
            units(unitsStr),
            data(NULL),
            writeToFile(writeOut),
            _nGrid(0)
    {
    }
    ~DerivedField() {
      if (data) {
        delete[] data;
      }
    }
    void alloc(size_t nGrid, const vector<double> &zLevels) {
      vertLevels = zLevels;
      if (nGrid == _nGrid) {
        return;
      }
      if (data) {
        delete[] data;
      }
      data = new fl32[nGrid];
      _nGrid = nGrid;
      for (size_t ii = 0; ii < _nGrid; ii++) {
        data[ii] = missingFl32;
      }
    }
    void setToZero() {
      for (size_t ii = 0; ii < _nGrid; ii++) {
        data[ii] = 0.0;
      }
    }
  private:
    size_t _nGrid;
  };

  // scan angle delta

  double _scanDeltaAz;
  double _scanDeltaEl;

  // output projection and grid

  MdvxProj _proj;
  double _gridOriginLat, _gridOriginLon;
  int _gridNx, _gridNy, _gridNz;
  int _nPointsVol, _nPointsPlane;
  double _gridMinx, _gridMiny;
  double _gridDx, _gridDy;
  vector<double> _gridZLevels;
  double _radarX, _radarY;
  fl32 **_outputFields;

  // protected methods

  virtual void _initProjection();

  void _accumNearest(const Ray *ray,
                     int ifield,
                     int igateInner,
                     int igateOuter,
                     double wtInner,
                     double wtOuter,
                     double &closestVal,
                     double &maxWt,
                     int &nContrib);

  void _accumNearest(const Ray *ray,
                     int ifield,
                     int igate,
                     double wt,
                     double &closestVal,
                     double &maxWt,
                     int &nContrib);
  
  void _accumInterp(const Ray *ray,
                    int ifield,
                    int igateInner,
                    int igateOuter,
                    double wtInner,
                    double wtOuter,
                    double &sumVals,
                    double &sumWts,
                    int &nContrib);
  
  void _accumInterp(const Ray *ray,
                    int ifield,
                    int igate,
                    double wt,
                    double &sumVals,
                    double &sumWts,
                    int &nContrib);

  void _accumFolded(const Ray *ray,
                    int ifield,
                    int igateInner,
                    int igateOuter,
                    double wtInner,
                    double wtOuter,
                    double &sumX,
                    double &sumY,
                    double &sumWts,
                    int &nContrib);
  
  void _accumFolded(const Ray *ray,
                    int ifield,
                    int igate,
                    double wt,
                    double &sumX,
                    double &sumY,
                    double &sumWts,
                    int &nContrib);
  
  double _getFoldAngle(double val,
                       double foldLimitLower,
                       double foldRange) const;

  double _getFoldValue(double angle,
                       double foldLimitLower,
                       double foldRange) const;

  int _setRadarParams();

  int _locateDataSector();

  void _computeAzimuthDelta();
  void _computeElevationDelta();

  void _printRunTime(const string& str, bool verbose = false);

  int _writeCedricFile(bool isPpi);

  void _transformForOutput();

private:

};

#endif
