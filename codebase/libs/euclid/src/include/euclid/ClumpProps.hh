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

  int id() const { return _id; }
  const Clump_order *clumpOrder() const { return _clump; }
  Interval **intervals() { return _clump->ptr; }
  const vector<Interval> &intvLocal() const { return _intvLocal; }
  size_t nIntervals() const { return _nIntervals; }
  size_t nPoints3D() const { return _nPoints3D; }
  size_t nPoints2D() const { return _nPoints2D; }
  
  // clump shrink-wrapped variables

  int startIxLocal() const { return _startIxLocal; }
  int startIyLocal() const { return _startIyLocal; }

  double offsetX() const { return _offsetX; }
  double offsetY() const { return _offsetY; }
  double startXLocal() const { return _startXLocal; }
  double startYLocal() const { return _startYLocal; }

  int nXLocal() const { return _nXLocal; }
  int nYLocal() const { return _nYLocal; }

  double dXKmAtCentroid() const { return _dXKmAtCentroid; }
  double dYKmAtCentroid() const { return _dYKmAtCentroid; }

  // grid Z properties
  
  const vector<double> &zKm() const { return _gridGeom.zKm(); }
  size_t nZ() const { return _gridGeom.nz(); }
  double meanDz() const { return _gridGeom.meanDz(); }
  double minZ() const { return _gridGeom.minz(); }
  
  // clump properties

  double clumpSize() const { return _clumpSize; } // vol or area as appropriate
  double projAreaKm2() const { return _projAreaKm2; }
  double volumeKm3() const { return _volumeKm3; }
  double vertExtentKm() const { return _vertExtentKm; }

  double dAreaAtCentroid() const { return _dAreaAtCentroid; }
  const vector<double> &dVolAtCentroid() const { return _dVolAtCentroid; }
  double kmPerGridUnit() const { return _kmPerGridUnit; }

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
  double _kmPerGridUnit;

  double _clumpSize;
  double _projAreaKm2;
  double _volumeKm3;
  double _vertExtentKm;

  void _shrinkWrap();
  void _compute2DGrid();
  void _computeProps();
  
};

#endif



