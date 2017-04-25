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
// PolarInterp.hh
//
// PolarInterp class - derived from Interp.
// Used for interpolation onto a regular polar grid
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2012
//
///////////////////////////////////////////////////////////////

#ifndef PolarInterp_HH
#define PolarInterp_HH

#include "Interp.hh"
#include <toolsa/TaThread.hh>
#include <toolsa/TaThreadPool.hh>

class PolarInterp : public Interp {
  
public:

  // constructor
  
  PolarInterp(const string &progName,
              const Params &params,
              RadxVol &readVol,
              vector<Field> &interpFields,
              vector<Ray *> &interpRays);
  
  // destructor
  
  virtual ~PolarInterp();
  
  // interpolate a volume
  // assumes volume has been read
  // and _interpFields and _interpRays vectors are populated
  // returns 0 on succes, -1 on failure

  virtual int interpVol();
  
  // get methods

  const Params &getParams() const { return _params; }

protected:
private:
  
  // class for search matrix

  class SearchPoint {
  public:
    inline SearchPoint() {
      clear();
    }
    inline void clear() {
      ray = NULL;
      rayEl = 0.0;
      rayAz = 0.0;
      interpAz = 0.0;
    }
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

  int _nAz;
  double _minAz, _deltaAz;
  int _nEl;

  // class for neighboring points

  class Neighbors {
  public:
    double left;
    double right;
  };

  // private methods
  
  virtual void _initProjection();

  int _getSearchAzIndex(double az);
  double _getSearchAz(int index);

  inline double _angDist(double deltaAz) {
    double dist = fabs(deltaAz);
    if (dist == 0) {
      dist = 1.0e-6;
    }
    return dist;
  }
  
  void _computeAzGridDetails();
  void _computeSearchLimits();
  void _initZLevels();
  void _initGrid();
  
  void _createThreads();
  void _freeThreads();
  
  void _allocOutputArrays();
  void _freeOutputArrays();
  void _initOutputArrays();
  
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
  void _interpAz(int iel, int iaz);

  void _loadWtsFor1ValidRay(const SearchPoint &left,
                            const SearchPoint &right,
                            Neighbors &wts);
  
  void _loadWtsFor2ValidRays(double condAz,
                             const SearchPoint &left,
                             const SearchPoint &right,
                             Neighbors &wts);
  
  void _loadNearestGridPt(int ifield,
                          int ptIndex,
                          int igate,
                          const SearchPoint &left,
                          const SearchPoint &right,
                          const Neighbors &wts);
  
  void _loadInterpGridPt(int ifield,
                         int ptIndex,
                         int igate,
                         const SearchPoint &left,
                         const SearchPoint &right,
                         const Neighbors &wts);

  void _loadFoldedGridPt(int ifield,
                         int ptIndex,
                         int igate,
                         const SearchPoint &left,
                         const SearchPoint &right,
                         const Neighbors &wts);

  double _conditionAz(double az);

  int _writeOutputFile();

  //////////////////////////////////////////////////////////////
  // inner thread class for performing interpolation
  
  class PerformInterp : public TaThread
  {
  public:
    // constructor
    PerformInterp(PolarInterp *obj);
    // set the el and az index
    inline void setElIndex(int elIndex) { _elIndex = elIndex; }
    inline void setAzIndex(int azIndex) { _azIndex = azIndex; }
    // override run method
    virtual void run();
  private:
    PolarInterp *_this; // context
    int _elIndex; // elevation index in regular volume
    int _azIndex; // azimuth index in regular volume
  };
  // instantiate thread pool for interpolation
  TaThreadPool _threadPoolInterp;

};

#endif
