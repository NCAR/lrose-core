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
// CartInterp.hh
//
// CartInterp class - derived from Interp.
// Used for full 3-D Cartesian interpolation.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2012
//
///////////////////////////////////////////////////////////////

#ifndef CartInterp_HH
#define CartInterp_HH

#include "Interp.hh"
#include <toolsa/TaThread.hh>
#include <toolsa/TaThreadPool.hh>
#include <radar/ConvStrat.hh>
class DsMdvx;

class CartInterp : public Interp {
  
public:

  // constructor
  
  CartInterp(const string &progName,
             const Params &params,
             RadxVol &readVol,
             vector<Field> &interpFields,
             vector<Ray *> &interpRays);
  
  // destructor
  
  virtual ~CartInterp();
  
  // interpolate a volume
  // assumes volume has been read
  // and _interpFields and _interpRays vectors are populated
  // returns 0 on succes, -1 on failure

  virtual int interpVol();
  
  // get methods

  const Params &getParams() const { return _params; }

  // set RHI mode

  void setRhiMode(bool state) { _rhiMode = state; }
  
protected:
private:

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
    const Ray *ray;
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

#ifdef JUNK
  DerivedField *_convStratDbzMax;
  DerivedField *_convStratDbzCount;
  DerivedField *_convStratDbzSum;
  DerivedField *_convStratDbzSqSum;
  DerivedField *_convStratDbzSqSqSum;
  DerivedField *_convStratDbzTexture;
  DerivedField *_convStratFilledTexture;
  DerivedField *_convStratDbzSqTexture;
  DerivedField *_convStratFilledSqTexture;

  DerivedField *_convStratDbzColMax;
  DerivedField *_convStratMeanTexture;
  DerivedField *_convStratMeanSqTexture;
  DerivedField *_convStratCategory;
#endif

  vector<DerivedField *> _derived3DFields;
  vector<DerivedField *> _derived2DFields;

  // convective / stratiform split

  ConvStrat _convStrat;
  bool _gotConvStrat;

#ifdef JUNK
  typedef enum {
    CATEGORY_MISSING = 0,
    CATEGORY_STRATIFORM = 1,
    CATEGORY_CONVECTIVE = 2,
    CATEGORY_UNKNOWN
  } category_t;
  
  int _nxTexture, _nyTexture;
  int _nxConv, _nyConv;

  double _convectiveRadiusKm;
  double _textureRadiusKm;

  typedef struct {
    double distance;
    ssize_t offset;
  } kernel_t;

  vector<kernel_t> _textureKernel;
  vector<kernel_t> _convKernel;

  // compute the lower and upper bounds of the vert levels for the
  // texture computation, for each 0.01 km from 0 to 30 km.
  // These are lookup tables.

  static const int NLOOKUP = 3000;
  int _textureIzLower[NLOOKUP];
  int _textureIzUpper[NLOOKUP];

#endif

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

  int _writeOutputFile();

  int _writeSearchMatrices();

  void _addMatrixField(DsMdvx &mdvx,
                       const fl32 *data,
                       fl32 missing,
                       const string &name,
                       const string &longName,
                       const string &units);

  int _convStratCompute();

#ifdef JUNK

  void _convStratPrepare();
  static bool _compareKernels(kernel_t x, kernel_t y);
  void _convStratComputeKernels();
  void _convStratComputeVertLookups();

#endif

  //////////////////////////////////////////////////////////////
  // Classes for threads
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  // inner thread class for filling out the lower-left search
  
  class FillSearchLowerLeft : public TaThread
  {  
  public:
    // constructor
    FillSearchLowerLeft(CartInterp *obj);
    // override run method
    virtual void run();
  private:
    CartInterp *_this; // context
  };

  //////////////////////////////////////////////////////////////
  // inner thread class for filling out the lower-right search
  
  class FillSearchLowerRight : public TaThread
  {  
  public:
    // constructor
    FillSearchLowerRight(CartInterp *obj);
    // override run method
    virtual void run();
  private:
    CartInterp *_this; // context
  };

  //////////////////////////////////////////////////////////////
  // inner thread class for filling out the upper-left search
  
  class FillSearchUpperLeft : public TaThread
  {  
  public:
    // constructor
    FillSearchUpperLeft(CartInterp *obj);
    // override run method
    virtual void run();
  private:
    CartInterp *_this; // context
  };

  //////////////////////////////////////////////////////////////
  // inner thread class for filling out the upper-right search
  
  class FillSearchUpperRight : public TaThread
  {  
  public:
    // constructor
    FillSearchUpperRight(CartInterp *obj);
    // override run method
    virtual void run();
  private:
    CartInterp *_this; // context
  };

  // instantiate threads for fill search
  FillSearchLowerLeft *_threadFillSearchLowerLeft;
  FillSearchLowerRight *_threadFillSearchLowerRight;
  FillSearchUpperLeft *_threadFillSearchUpperLeft;
  FillSearchUpperRight *_threadFillSearchUpperRight;

  //////////////////////////////////////////////////////////////
  // inner thread class for computing the grid locations
  // relative to the radar
  
  class ComputeGridRelative : public TaThread
  {  
  public:
    // constructor
    ComputeGridRelative(CartInterp *obj);
    // set the y and z index
    inline void setYIndex(int yIndex) { _yIndex = yIndex; }
    inline void setZIndex(int zIndex) { _zIndex = zIndex; }
    // override run method
    virtual void run();
  private:
    CartInterp *_this; // context
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
    PerformInterp(CartInterp *obj);
    // set the y and z index
    inline void setYIndex(int yIndex) { _yIndex = yIndex; }
    inline void setZIndex(int zIndex) { _zIndex = zIndex; }
    // override run method
    virtual void run();
  private:
    CartInterp *_this; // context
    int _yIndex; // grid index of y column
    int _zIndex; // grid index of z plane
  };
  // instantiate thread pool for interpolation
  TaThreadPool _threadPoolInterp;

#ifdef JUNK

  //////////////////////////////////////////////////////////////
  // inner thread class for computing 2D texture

  class ComputeTexture : public TaThread
  {  
  public:   

    // constructor saves _sd3c pointer
    ComputeTexture(int iz);

    // destructor
    virtual ~ComputeTexture();

    // set parameters
    void setGridSize(int nx, int ny)
    {
      _nx = nx;
      _ny = ny;
    }
    void setKernelSize(int nx, int ny)
    {
      _nxTexture = nx;
      _nyTexture = ny;
    }
    void setKernel(const vector<kernel_t> &kernel)
    {
      _kernel = kernel;
    }
    void setMinValidFraction(double val)
    {
      _minValidFraction = val;
    }
    void setFields(const fl32 *dbzCount,
                   const fl32 *dbzSum,
                   const fl32 *dbzSqSum,
                   const fl32 *dbzSqSqSum,
                   fl32 *dbzTexture,
                   fl32 *dbzSqTexture,
                   fl32 *filledTexture,
                   fl32 *filledSqTexture)
    {
      _dbzCount = dbzCount;
      _dbzSum = dbzSum;
      _dbzSqSum = dbzSqSum;
      _dbzSqSqSum = dbzSqSqSum;
      _dbzTexture = dbzTexture;
      _dbzSqTexture = dbzSqTexture;
      _filledTexture = filledTexture;
      _filledSqTexture = filledSqTexture;
    }
    
    // override run method
    virtual void run();
    
  private:
    
    int _iz;
    int _nx, _ny;
    int _nxTexture, _nyTexture;
    double _minValidFraction;
    vector<kernel_t> _kernel;
    const fl32 *_dbzCount;
    const fl32 *_dbzSum;
    const fl32 *_dbzSqSum;
    const fl32 *_dbzSqSqSum;
    fl32 *_dbzTexture;
    fl32 *_dbzSqTexture;
    fl32 *_filledTexture;
    fl32 *_filledSqTexture;

  };

  //////////////////////////////////////////////////////////////

#endif

};

#endif
