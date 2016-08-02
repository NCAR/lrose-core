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
// PrevReorderInterp.hh
//
// PrevReorderInterp class - derived from Interp.
// Used for full 3-D Cartesian interpolation, following the 
// REORDER strategy of interpolation using the closest N points.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2012
//
///////////////////////////////////////////////////////////////

#ifndef PrevReorderInterp_HH
#define PrevReorderInterp_HH

#include "Interp.hh"
#include <kd/kd.hh>
#include <iostream>

class PrevReorderInterp : public Interp {
  
public:

  // constructor
  
  PrevReorderInterp(const string &progName,
                const Params &params,
                RadxVol &readVol,
                vector<Field> &interpFields,
                vector<Ray *> &interpRays);
  
  // destructor
  
  virtual ~PrevReorderInterp();
  
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
  
  deque<PrevReorderThread *> _activeThreads;
  deque<PrevReorderThread *> _availThreads;
  pthread_mutex_t _debugPrintMutex;

  // number of rows and columns for blocks

  int _nrows, _ncols;

  // KD tree for radar points

  static const int KD_DIM = 3;
  
  typedef struct {
    int iray;
    int igate;
    double xx;
    double yy;
    double zz;
    double wt;
  } radar_point_t;
  vector<radar_point_t> _radarPoints;

  double _maxSearchRadius;
  double _maxZ;
  double _zSearchRatio;
  
  void _initZLevels();
  void _initGrid();
  
  void _initThreads();
  static void *_computeInThread(void *thread_data);
  
  void _allocOutputArrays();
  void _freeOutputArrays();
  void _initOutputArrays();
  
  void _computeRadarPoints();

  void _computeGridRelative();
  void _computeGridRelMultiThreaded();
  void _computeGridRelRow(int iz, int iy);

  void _doInterp();
  void _interpSingleThreaded();
  void _interpMultiThreaded();
  void _interpBlock(int blocky, int blockx);
  void _interpPoint(int iz, int iy, int ix,
                    KD_tree &kdTree,
                    const vector<radar_point_t> &radarPoints);
  
  void _computeNearestGridPt(int ifield,
                             int iz, int iy, int ix,
                             const vector<radar_point_t> &interpPts);
  void _computeFoldedGridPt(int ifield,
                            int iz, int iy, int ix,
                            const vector<radar_point_t> &interpPts);
  void _computeInterpGridPt(int ifield,
                            int iz, int iy, int ix,
                            const vector<radar_point_t> &interpPts);

  int _writeOutputFile();

};

#endif
