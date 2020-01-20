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
// Orient.hh
//
// Orient class.
// Compute echo orientation:
//   vertical (convective)
//   horizontal (stratiform, bright-band, anvil)
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2020
//
///////////////////////////////////////////////////////////////

#ifndef Orient_HH
#define Orient_HH

#include "Interp.hh"
#include <toolsa/TaThread.hh>
#include <toolsa/TaThreadPool.hh>
#include <radar/ConvStrat.hh>
class DsMdvx;
class RhiOrient;

class Orient {

public:

  // constructor

  Orient(const string &progName,
         const Params &params,
         RadxVol &readVol,
         vector<Interp::Field> &interpFields,
         vector<Interp::Ray *> &interpRays);

  // destructor
  
  virtual ~Orient();

  // determine the echo orientation

  int findEchoOrientation();
  
  // get methods

  const Params &getParams() const { return _params; }

  // set RHI mode

  void setRhiMode(bool state) { _rhiMode = state; }
  
protected:
private:

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

  // class for search matrix

  class SearchPoint {
  public:
    inline SearchPoint() {
      clear();
    }
    inline void clear() {
      level = 0;
      elDist = 0;
      azDist = 0;
      ray = NULL;
      rayEl = 0.0;
      rayAz = 0.0;
      interpEl = 0.0;
      interpAz = 0.0;
    }
    int level;
    int elDist;
    int azDist;
    const Interp::Ray *ray;
    double rayEl; // el in search matrix coords
    double rayAz; // az in search matrix coords
    double interpEl; // el used for interp
    double interpAz; // az used for interp
  };

  class SearchIndex {
  public:
    int elIndex;
    int azIndex;
    SearchIndex(int iel, int iaz) {
      elIndex = iel;
      azIndex = iaz;
    }
  };
  
  SearchPoint **_searchMatrixLowerLeft;
  SearchPoint **_searchMatrixUpperLeft;
  SearchPoint **_searchMatrixLowerRight;
  SearchPoint **_searchMatrixUpperRight;
  
  static const double _searchResEl;
  static const double _searchResAz;
  static const double _searchAzOverlapDeg;
  static const double _searchAzOverlapHalf;

  int _searchMaxCount;

  double _searchMinEl;
  double _searchMaxEl;
  int _searchNEl;
  double _searchRadiusEl;
  int _searchMaxDistEl;

  int _searchNAz;
  double _searchMinAz;
  double _searchRadiusAz;
  int _searchMaxDistAz;

  // class for neighboring points

  class Neighbors {
  public:
    double ll_inner;
    double ll_outer;
    double ul_inner;
    double ul_outer;
    double lr_inner;
    double lr_outer;
    double ur_inner;
    double ur_outer;
  };

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
        data[ii] = Interp::missingFl32;
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

  // references to main object
  
  string _progName;
  const Params &_params;
  RadxVol &_readVol;
  bool _rhiMode;
  vector<Interp::Field> &_interpFields;
  vector<Interp::Ray *> &_interpRays;

  // rhis

  vector<RhiOrient *> _rhis;

  // debug 

  DerivedField *_nContribDebug;
  DerivedField *_gridAzDebug;
  DerivedField *_gridElDebug;
  DerivedField *_gridRangeDebug;
  DerivedField *_llElDebug;
  DerivedField *_llAzDebug;
  DerivedField *_lrElDebug;
  DerivedField *_lrAzDebug;
  DerivedField *_ulElDebug;
  DerivedField *_ulAzDebug;
  DerivedField *_urElDebug;
  DerivedField *_urAzDebug;

  vector<DerivedField *> _derived3DFields;
  vector<DerivedField *> _derived2DFields;

  // convective / stratiform split

  ConvStrat _convStrat;
  bool _gotConvStrat;

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

  // private methods

  void _createThreads();
  void _freeThreads();

  void _createDebugFields();
  void _createConvStratFields();
  void _freeDerivedFields();

  void _initZLevels();
  void _freeZLevels();

  void _initGrid();
  void _freeGridLoc();
  
  void _allocOutputArrays();
  void _freeOutputArrays();
  void _initOutputArrays();
  
  void _computeSearchLimits();
  
  void _computeOrientInRhis();
  void _computeOrientMultiThreaded();
  void _computeOrientSingleThreaded();

  void _computeGridRelative();
  void _computeGridRelMultiThreaded();
  void _computeGridRow(int iz, int iy);

  void _allocSearchMatrix();
  void _freeSearchMatrix();
  void _initSearchMatrix();
  void _fillSearchMatrix();
  void _printSearchMatrix(FILE *out, int res);
  void _printSearchMatrixPoint(FILE *out, int iel, int iaz);

  int _fillSearchLowerLeft(int level,
                           vector<SearchIndex> &thisSearch,
                           vector<SearchIndex> &nextSearch);
  int _fillSearchUpperLeft(int level,
                           vector<SearchIndex> &thisSearch,
                           vector<SearchIndex> &nextSearch);
  int _fillSearchLowerRight(int level,
                            vector<SearchIndex> &thisSearch,
                            vector<SearchIndex> &nextSearch);
  int _fillSearchUpperRight(int level,
                            vector<SearchIndex> &thisSearch,
                            vector<SearchIndex> &nextSearch);
  
  int _getSearchElIndex(double el);
  int _getSearchAzIndex(double az);
  double _getSearchEl(int index);
  double _getSearchAz(int index);

  inline double _angDist(double deltaEl, double deltaAz) {
    double dist = sqrt(deltaAz * deltaAz + deltaEl * deltaEl);
    if (dist == 0) {
      dist = 1.0e-6;
    }
    return dist;
  }

  void _doInterp();
  void _interpSingleThreaded();
  void _interpMultiThreaded();
  void _interpRow(int iz, int iy);

  void _loadWtsFor2ValidRays(const GridLoc *loc,
                             const SearchPoint &ll,
                             const SearchPoint &ul,
                             const SearchPoint &lr,
                             const SearchPoint &ur,
                             double wtInner,
                             double wtOuter, 
                             Neighbors &wts);
  
  void _loadWtsFor3Or4ValidRays(const GridLoc *loc,
                                const SearchPoint &ll,
                                const SearchPoint &ul,
                                const SearchPoint &lr,
                                const SearchPoint &ur,
                                double wtInner,
                                double wtOuter, 
                                Neighbors &wts);
  
  int _loadNearestGridPt(int ifield,
                         int ptIndex,
                         int igateInner,
                         int igateOuter,
                         const SearchPoint &ll,
                         const SearchPoint &ul,
                         const SearchPoint &lr,
                         const SearchPoint &ur,
                         const Neighbors &wts);
  
  int _loadInterpGridPt(int ifield,
                        int ptIndex,
                        int igateInner,
                        int igateOuter,
                        const SearchPoint &ll,
                        const SearchPoint &ul,
                        const SearchPoint &lr,
                        const SearchPoint &ur,
                        const Neighbors &wts);

  int _loadFoldedGridPt(int ifield,
                        int ptIndex,
                        int igateInner,
                        int igateOuter,
                        const SearchPoint &ll,
                        const SearchPoint &ul,
                        const SearchPoint &lr,
                        const SearchPoint &ur,
                        const Neighbors &wts);
  
  double _conditionAz(double az);

  int _writeSearchMatrices();

  void _addMatrixField(DsMdvx &mdvx,
                       const fl32 *data,
                       fl32 missing,
                       const string &name,
                       const string &longName,
                       const string &units);

  int _convStratCompute();

  virtual void _initProjection();

  void _accumNearest(const Interp::Ray *ray,
                     int ifield,
                     int igateInner,
                     int igateOuter,
                     double wtInner,
                     double wtOuter,
                     double &closestVal,
                     double &maxWt,
                     int &nContrib);

  void _accumNearest(const Interp::Ray *ray,
                     int ifield,
                     int igate,
                     double wt,
                     double &closestVal,
                     double &maxWt,
                     int &nContrib);
  
  void _accumInterp(const Interp::Ray *ray,
                    int ifield,
                    int igateInner,
                    int igateOuter,
                    double wtInner,
                    double wtOuter,
                    double &sumVals,
                    double &sumWts,
                    int &nContrib);
  
  void _accumInterp(const Interp::Ray *ray,
                    int ifield,
                    int igate,
                    double wt,
                    double &sumVals,
                    double &sumWts,
                    int &nContrib);

  void _accumFolded(const Interp::Ray *ray,
                    int ifield,
                    int igateInner,
                    int igateOuter,
                    double wtInner,
                    double wtOuter,
                    double &sumX,
                    double &sumY,
                    double &sumWts,
                    int &nContrib);
  
  void _accumFolded(const Interp::Ray *ray,
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

  void _transformForOutput();

  //////////////////////////////////////////////////////////////
  // Classes for threads
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  // inner thread class for filling out the lower-left search
  
  class FillSearchLowerLeft : public TaThread
  {  
  public:
    // constructor
    FillSearchLowerLeft(Orient *obj);
    // override run method
    virtual void run();
  private:
    Orient *_this; // context
  };

  //////////////////////////////////////////////////////////////
  // inner thread class for filling out the lower-right search
  
  class FillSearchLowerRight : public TaThread
  {  
  public:
    // constructor
    FillSearchLowerRight(Orient *obj);
    // override run method
    virtual void run();
  private:
    Orient *_this; // context
  };

  //////////////////////////////////////////////////////////////
  // inner thread class for filling out the upper-left search
  
  class FillSearchUpperLeft : public TaThread
  {  
  public:
    // constructor
    FillSearchUpperLeft(Orient *obj);
    // override run method
    virtual void run();
  private:
    Orient *_this; // context
  };

  //////////////////////////////////////////////////////////////
  // inner thread class for filling out the upper-right search
  
  class FillSearchUpperRight : public TaThread
  {  
  public:
    // constructor
    FillSearchUpperRight(Orient *obj);
    // override run method
    virtual void run();
  private:
    Orient *_this; // context
  };

  // instantiate threads for fill search
  FillSearchLowerLeft *_threadFillSearchLowerLeft;
  FillSearchLowerRight *_threadFillSearchLowerRight;
  FillSearchUpperLeft *_threadFillSearchUpperLeft;
  FillSearchUpperRight *_threadFillSearchUpperRight;

  //////////////////////////////////////////////////////////////
  // inner thread class for computing orientation in RHI
  
  class ComputeOrientInRhi : public TaThread
  {  
  public:
    // constructor
    ComputeOrientInRhi(Orient *obj);
    // set the RHI index to be used
    inline void setRhiIndex(size_t index) { _index = index; }
    // override run method
    virtual void run();
  private:
    Orient *_this; // context
    size_t _index; // index of RHI to be used
  };
  // instantiate thread pool for grid relative computations
  TaThreadPool _threadPoolOrientInRhi;

  //////////////////////////////////////////////////////////////
  // inner thread class for computing the grid locations
  // relative to the radar
  
  class ComputeGridRelative : public TaThread
  {  
  public:
    // constructor
    ComputeGridRelative(Orient *obj);
    // set the y and z index
    inline void setYIndex(int yIndex) { _yIndex = yIndex; }
    inline void setZIndex(int zIndex) { _zIndex = zIndex; }
    // override run method
    virtual void run();
  private:
    Orient *_this; // context
    int _yIndex; // grid index of y column
    int _zIndex; // grid index of z plane
  };
  // instantiate thread pool for grid relative computations
  TaThreadPool _threadPoolGridRel;

  //////////////////////////////////////////////////////////////
  // inner thread class for performing interpolation
  
  class PerformInterp : public TaThread
  {  
  public:
    // constructor
    PerformInterp(Orient *obj);
    // set the y and z index
    inline void setYIndex(int yIndex) { _yIndex = yIndex; }
    inline void setZIndex(int zIndex) { _zIndex = zIndex; }
    // override run method
    virtual void run();
  private:
    Orient *_this; // context
    int _yIndex; // grid index of y column
    int _zIndex; // grid index of z plane
  };
  // instantiate thread pool for interpolation
  TaThreadPool _threadPoolInterp;

};

#endif
