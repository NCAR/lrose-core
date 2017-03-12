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
// PpiInterp.hh
//
// PpiInterp class - derived from Interp.
// Used for 2-D Cartesian interpolation on ppi cones.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2012
//
///////////////////////////////////////////////////////////////

#ifndef PpiInterp_HH
#define PpiInterp_HH

#include "Interp.hh"
#include <toolsa/TaThread.hh>
#include <toolsa/TaThreadPool.hh>

class PpiInterp : public Interp {
  
public:

  // constructor
  
  PpiInterp(const string &progName,
            const Params &params,
            RadxVol &readVol,
            vector<Field> &interpFields,
            vector<Ray *> &interpRays);
  
  // destructor
  
  virtual ~PpiInterp();
  
  // interpolate a volume
  // assumes volume has been read
  // and _interpFields and _interpRays vectors are populated
  // returns 0 on succes, -1 on failure

  virtual int interpVol();
  
  // get methods

  const Params &getParams() const { return _params; }

protected:
private:
  
  // previous Z levels - check on changes
  
  vector<double> _prevZLevels;

  // class for search matrix

  class SearchPoint {
  public:
    inline SearchPoint() {
      clear();
    }
    inline void clear() {
      azDist = 0;
      ray = NULL;
      rayEl = 0.0;
      rayAz = 0.0;
      interpAz = 0.0;
    }
    int azDist;
    const Ray *ray;
    double rayEl; // el in search matrix coords
    double rayAz; // az in search matrix coords
    double interpAz; // az used for interp
  };
  
  SearchPoint **_searchLeft;
  SearchPoint **_searchRight;
  
  static const double _searchResAz;
  static const double _searchAzOverlapDeg;
  static const double _searchAzOverlapHalf;

  int _searchNAz;
  double _searchMinAz;
  double _searchRadiusAz;
  int _searchMaxDistAz;
  int _nEl;
  
  int _nzAlloc, _nyAlloc, _nxAlloc;

  // class for neighboring points

  class Neighbors {
  public:
    double left_inner;
    double left_outer;
    double right_inner;
    double right_outer;
  };

  // private methods

  int _getSearchAzIndex(double az);
  double _getSearchAz(int index);

  inline double _angDist(double deltaAz) {
    double dist = fabs(deltaAz);
    if (dist == 0) {
      dist = 1.0e-6;
    }
    return dist;
  }

  void _createThreads();
  void _freeThreads();

  void _initZLevels();
  void _freeZLevels();

  void _initGrid();
  void _freeGridLoc();
  
  bool _geomHasChanged();
  void _computeSearchLimits();

  void _allocOutputArrays();
  void _freeOutputArrays();
  void _initOutputArrays();
  
  void _computeGridRelative();
  void _computeGridRelMultiThreaded();
  void _computeGridRow(int iz, int iy);

  void _allocSearchMatrix();
  void _freeSearchMatrix();
  void _initSearchMatrix();
  void _fillSearchMatrix();
  void _printSearchMatrix(FILE *out, int res);
  void _printSearchMatrixPoint(FILE *out, int iel, int iaz);

  void _fillLeft();
  void _fillRight();
  
  void _doInterp();
  void _interpSingleThreaded();
  void _interpMultiThreaded();
  void _interpRow(int iz, int iy);

  void _loadWtsFor1ValidRay(const SearchPoint &left,
                            const SearchPoint &right,
                            double wtInner,
                            double wtOuter, 
                            Neighbors &wts);
  
  void _loadWtsFor2ValidRays(const GridLoc *loc,
                             const SearchPoint &left,
                             const SearchPoint &right,
                             double wtInner,
                             double wtOuter, 
                             Neighbors &wts);
  
  void _loadNearestGridPt(int ifield,
                          int ptIndex,
                          int igateInner,
                          int igateOuter,
                          const SearchPoint &left,
                          const SearchPoint &right,
                          const Neighbors &wts);
  
  void _loadInterpGridPt(int ifield,
                         int ptIndex,
                         int igateInner,
                         int igateOuter,
                         const SearchPoint &left,
                         const SearchPoint &right,
                         const Neighbors &wts);

  void _loadFoldedGridPt(int ifield,
                         int ptIndex,
                         int igateInner,
                         int igateOuter,
                         const SearchPoint &left,
                         const SearchPoint &right,
                         const Neighbors &wts);

  double _conditionAz(double az);

  int _writeOutputFile();

  //////////////////////////////////////////////////////////////
  // inner thread class for computing the grid locations
  // relative to the radar
  
  class ComputeGridRelative : public TaThread
  {  
  public:
    // constructor
    ComputeGridRelative(PpiInterp *obj);
    // set the y and z index
    inline void setYIndex(int yIndex) { _yIndex = yIndex; }
    inline void setZIndex(int zIndex) { _zIndex = zIndex; }
    // override run method
    virtual void run();
  private:
    PpiInterp *_this; // context
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
    PerformInterp(PpiInterp *obj);
    // set the y and z index
    inline void setYIndex(int yIndex) { _yIndex = yIndex; }
    inline void setZIndex(int zIndex) { _zIndex = zIndex; }
    // override run method
    virtual void run();
  private:
    PpiInterp *_this; // context
    int _yIndex; // grid index of y column
    int _zIndex; // grid index of z plane
  };
  // instantiate thread pool for interpolation
  TaThreadPool _threadPoolInterp;

};

#endif
