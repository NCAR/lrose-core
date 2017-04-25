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
// ReorderInterp.hh
//
// ReorderInterp class - derived from Interp.
// Used for full 3-D Cartesian interpolation, following the 
// REORDER strategy of interpolation using the closest N points.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2012
//
///////////////////////////////////////////////////////////////

#ifndef ReorderInterp_HH
#define ReorderInterp_HH

#include "Interp.hh"
#include <kd/kd.hh>
#include <iostream>
#include <toolsa/TaThread.hh>
#include <toolsa/TaThreadPool.hh>

class ReorderInterp : public Interp {
  
  friend class SvdData;

public:

  // constructor
  
  ReorderInterp(const string &progName,
                const Params &params,
                RadxVol &readVol,
                vector<Field> &interpFields,
                vector<Ray *> &interpRays);
  
  // destructor
  
  virtual ~ReorderInterp();
  
  // interpolate a volume
  // assumes volume has been read
  // and _interpFields and _interpRays vectors are populated
  // returns 0 on succes, -1 on failure

  virtual int interpVol();
  
  // get params

  const Params &getParams() const { return _params; }

protected:
private:

  // keeping track of points in radar space

  typedef struct {
    int index;
    int rayStartIndex;
    int rayEndIndex;
    int iray;
    int igate;
    double xx;
    double yy;
    double zz;
    double wt;
    bool isTagPt;
    const Ray *ray;
    double distSq;
  } radar_point_t;
  vector<radar_point_t> _radarPoints;

  typedef struct {
    radar_point_t first;
    radar_point_t second;
  } ray_closest_t;
  
  // KD tree for radar points
  
  static const int KD_DIM = 3;
  
  vector<KD_real *> _kdVec;
  KD_real **_kdMat;
  KD_tree *_kdTree;

  // tag gates - use to identify rays closest to grid points

  vector<radar_point_t> _tagPoints;
  vector<bool> _tagGates;
  double _tagStartRangeKm, _tagGateSpacingKm;

  // search criteria

  double _maxSearchRadius;
  double _maxZ;
  double _zSearchRatio;

  // class for point neighbors

  class NeighborProps {
  public:
    const GridLoc *loc;   // elevation (degrees), azimuth(degrees) slantrange(km), instrument location km
    int iz, iy, ix;       // output grid indices
    vector<int> tagIndexes;
    vector<double> distSq;
  };
  
  void _initZLevels();
  void _initGrid();
  
  void _createThreads();
  void _freeThreads();
  
  void _allocOutputArrays();
  void _freeOutputArrays();
  void _initOutputArrays();
  
  void _computeRadarPoints();
  void _computeTagGates(int nGates);

  void _initProjectionLocal();
  void _computeGridRelRow(int iz, int iy, GridLoc **loc);

  void _buildKdTree();
  void _freeKdTree();

  void _doInterp();
  void _interpSingleThreaded();
  void _interpMultiThreaded();

  void _interpPlane(int iz);
  void _interpPoint(const NeighborProps &neighborProps, const GridLoc &loc);
  
  void _findClosestGates(const GridLoc &loc,
                         const radar_point_t &pt,
                         ray_closest_t &closestPts);

  double _computeDistSq(const GridLoc &loc, const radar_point_t &pt);
  void _printRadarPoint(ostream &out, const radar_point_t &pt);

  void _computeNearestGridPt(int ifield,
                             int iz, int iy, int ix,
                             const vector<radar_point_t> &interpPts);
  void _computeFoldedGridPt(int ifield,
                            int iz, int iy, int ix,
			    const GridLoc &loc,
                            const vector<radar_point_t> &interpPts);
  void _computeInterpGridPt(int ifield,
                            int iz, int iy, int ix,
			    const GridLoc &loc,
                            const vector<radar_point_t> &interpPts);
  bool _computeSvd(int ifield, int iz, int iy, int ix, const GridLoc &loc,
		   const vector<radar_point_t> &interpPts, double &v);
  void _computeMinMax(const vector<double> &bb,
                      double &minVal, double &maxVal);
  void _computeWeightedFoldedGridPt(int ifield,
				    int iz, int iy, int ix,
				    const vector<radar_point_t> &interpPts);
  void _computeWeightedInterpGridPt(int ifield,
				    int iz, int iy, int ix,
				    const vector<radar_point_t> &interpPts);

  bool _getData(int ifield, const Interp::Ray *ray, 
                int igate, double &v);

  bool _getDataAllowMissing(int ifield, const Interp::Ray *ray,
                            int igate, double &v, double &missing,
                            bool &isMissing);
    
  bool _collectLocalData(int ifield, vector<radar_point_t> &interpPts,
			 const GridLoc &loc, vector<double> &b);

  bool _collectLocalFoldedData(int ifield, 
			       vector<radar_point_t> &interpPts,
			       vector<double> &x,
			       vector<double> &y);

  int _writeOutputFile();

  //////////////////////////////////////////////////////////////
  // inner thread class for performing interpolation
  
  class PerformInterp : public TaThread
  {  
  public:
    // constructor
    PerformInterp(ReorderInterp *obj);
    // set the z index
    inline void setZIndex(int zIndex) { _zIndex = zIndex; }
    // override run method
    virtual void run();
  private:
    ReorderInterp *_this; // context
    int _zIndex; // grid index of z plane
  };
  // instantiate thread pool for interpolation
  TaThreadPool _threadPoolInterp;

  // threading control
  
  pthread_mutex_t _kdTreeMutex;
  

};

#endif
