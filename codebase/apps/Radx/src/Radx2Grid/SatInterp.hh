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
// SatInterp.hh
//
// SatInterp class - derived from Interp.
// Handles satellite RADAR or LIDAR data
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2013
//
///////////////////////////////////////////////////////////////

#ifndef SatInterp_HH
#define SatInterp_HH

#include "Interp.hh"
#include <kd/kd.hh>
#include <iostream>

class SatInterp : public Interp {
  
public:

  // constructor
  
  SatInterp(const string &progName,
            const Params &params,
            RadxVol &readVol,
            vector<Field> &interpFields,
            vector<Ray *> &interpRays);
  
  // destructor
  
  virtual ~SatInterp();
  
  // interpolate a volume
  // assumes volume has been read
  // and _interpFields and _interpRays vectors are populated
  // returns 0 on succes, -1 on failure

  virtual int interpVol();
  
  // get methods for threading

  const Params &getParams() const { return _params; }
  pthread_mutex_t *getDebugPrintMutex() { return &_debugPrintMutex; }

protected:
private:

   // threading
  
  deque<SatThread *> _activeThreads;
  deque<SatThread *> _availThreads;
  pthread_mutex_t _debugPrintMutex;
  pthread_mutex_t _kdTreeMutex;
  
  // keeping track of points in instr space

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
  } instr_point_t;
  vector<instr_point_t> _instrPoints;

  typedef struct {
    instr_point_t first;
    instr_point_t second;
  } ray_closest_t;
  
  // KD tree for instr points
  
  static const int KD_DIM = 3;
  
  vector<KD_real *> _kdVec;
  KD_real **_kdMat;
  KD_tree *_kdTree;

  // tag gates - use to identify rays closest to grid points

  vector<instr_point_t> _tagPoints;
  vector<bool> _tagGates;
  double _tagStartRangeKm, _tagGateSpacingKm;

  // search criteria

  double _maxSearchRadius;
  double _maxZ;
  double _zSearchRatio;

  // class for point neighbors

  class NeighborProps {
  public:
    const GridLoc *loc;
    int iz, iy, ix;
    vector<int> tagIndexes;
    vector<double> distSq;
  };
  
  void _initZLevels();
  void _initGrid();
  
  void _initThreads();
  static void *_computeInThread(void *thread_data);
  
  void _allocOutputArrays();
  void _freeOutputArrays();
  void _initOutputArrays();
  
  void _computeInstrPoints();
  void _computeTagGates(int nGates);

  void _computeGridRelative();
  void _computeGridRelMultiThreaded();
  void _computeGridRelRow(int iz, int iy);

  void _buildKdTree();
  void _freeKdTree();

  void _doInterp();
  void _interpSingleThreaded();
  void _interpMultiThreaded();

  void _interpPlane(int iz);

  void _interpBlock(int blocky, int blockx);
  void _interpPoint(const NeighborProps &neighborProps);
  
  void _findClosestGates(const GridLoc &loc,
                         const instr_point_t &pt,
                         ray_closest_t &closestPts);

  double _computeDistSq(const GridLoc &loc, const instr_point_t &pt);

  void _computeNearestGridPt(int ifield,
                             int iz, int iy, int ix,
                             const vector<instr_point_t> &interpPts);
  void _computeFoldedGridPt(int ifield,
                            int iz, int iy, int ix,
                            const vector<instr_point_t> &interpPts);
  void _computeInterpGridPt(int ifield,
                            int iz, int iy, int ix,
                            const vector<instr_point_t> &interpPts);

  int _writeOutputFile();

};

#endif
