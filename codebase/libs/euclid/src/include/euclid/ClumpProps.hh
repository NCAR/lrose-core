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
// ClumpProps.hh
//
// ClumpProps class - combines a clump with grid geometry so that
// computations may be done on the clump using that grid geometry.
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2021
//
///////////////////////////////////////////////////////////////

#ifndef ClumpProps_HH
#define ClumpProps_HH

#include <euclid/clump.h>
#include <euclid/PjgGridGeom.hh>
#include <toolsa/TaArray2D.hh>
#include <vector>
using namespace std;

////////////////////////////////
// ClumpProps

class ClumpProps {
  
public:

  // constructor

  ClumpProps();
  
  // initializer

  void init(const Clump_order *clump,
            const PjgGridGeom &gridGeom,
	    int startIx, int startIy);

  // destructor
  
  virtual ~ClumpProps();

  const PjgGridGeom &gridGeom() const { return _gridGeom; }

  // clump details

  inline int id() const { return _id; }
  inline const Clump_order *clumpOrder() const { return _clump; }
  inline Interval **intervals() { return _clump->ptr; }
  inline size_t nIntervals() const { return _nIntervals; }
  inline size_t nPoints3D() const { return _nPoints3D; }
  inline size_t nPoints2D() const { return _nPoints2D; }
  
  inline int startIxGlobal() const { return _startIxGlobal; }
  inline int startIyGlobal() const { return _startIyGlobal; }

  // clump shrink-wrapped variables

  inline int startIxLocal() const { return _startIxLocal; }
  inline int startIyLocal() const { return _startIyLocal; }

  inline double offsetX() const { return _offsetX; }
  inline double offsetY() const { return _offsetY; }
  inline double startXLocal() const { return _startXLocal; }
  inline double startYLocal() const { return _startYLocal; }

  inline int nXLocal() const { return _nXLocal; }
  inline int nYLocal() const { return _nYLocal; }

  inline double dXKmAtCentroid() const { return _dXKmAtCentroid; }
  inline double dYKmAtCentroid() const { return _dYKmAtCentroid; }

  inline const vector<Interval> &intvLocal() const { return _intvLocal; }
  inline const Interval &intvLocal(int n) const { return _intvLocal[n]; }

  // grid Z properties
  
  inline const vector<double> &zKm() const { return _gridGeom.zKm(); }
  inline double zKm(int iz) const { return _gridGeom.zKm(iz); }
  inline size_t nZ() const { return _gridGeom.nz(); }
  inline double meanDz() const { return _gridGeom.meanDz(); }
  inline double minZ() const { return _gridGeom.minz(); }
  
  // clumpSize returns vol or area as appropriate
  inline double clumpSize() const { return _clumpSize; }

  // clump properties
  
  inline double projAreaKm2() const { return _projAreaKm2; }
  inline double volumeKm3() const { return _volumeKm3; }

  inline double minZKm() const { return _minZKm; }
  inline double maxZKm() const { return _maxZKm; }
  inline double vertExtentKm() const { return _vertExtentKm; }

  inline double dAreaAtCentroid() const { return _dAreaAtCentroid; }
  inline const vector<double> &dVolAtCentroid() const { return _dVolAtCentroid; }
  inline double dVolAtCentroid(int iz) const { return _dVolAtCentroid[iz]; }

  inline double centroidX() const { return _centroidX; }
  inline double centroidY() const { return _centroidY; }
  inline double centroidZ() const { return _centroidZ; }

protected:
  
private:

  int _initDone;

  const Clump_order *_clump;
  int _id;
  PjgGridGeom _gridGeom;

  int _startIxGlobal, _startIyGlobal;
  int _minIxGlobal, _minIyGlobal;
  int _maxIxGlobal, _maxIyGlobal;

  vector<Interval> _intvLocal;
  size_t _nIntervals;
  size_t _nPoints3D;
  
  TaArray2D<unsigned char> _grid2DArray;
  unsigned char **_grid2DVals;
  size_t _nPoints2D;

  int _startIxLocal, _startIyLocal;
  double _offsetX, _offsetY;
  double _startXLocal, _startYLocal;
  int _nXLocal, _nYLocal;
  double _dXKmAtCentroid, _dYKmAtCentroid;
  
  double _dAreaAtCentroid;
  vector<double> _dVolAtCentroid;

  double _clumpSize;
  double _projAreaKm2;
  double _volumeKm3;

  double _minZKm, _maxZKm;
  double _vertExtentKm;

  double _centroidX;
  double _centroidY;
  double _centroidZ;

  void _shrinkWrap();
  void _compute2DGrid();
  void _computeProps();
  
};

#endif



