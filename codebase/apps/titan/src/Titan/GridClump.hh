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
// GridClump.hh
//
// GridClump class - wraps a clump with an mdv grid so that
// computations may be done on the clump with the grid geometry.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef GridClump_HH
#define GridClump_HH

#include <euclid/clump.h>
#include <rapformats/titan_grid.h>
#include <vector>
using namespace std;

////////////////////////////////
// GridClump

class GridClump {
  
public:

  // constructors

  GridClump();

  GridClump(Clump_order *clump,
	    const titan_grid_t &titan_grid,
	    int start_ix, int start_iy);

  // initializer

  void init(Clump_order *clump,
	    const titan_grid_t &titan_grid,
	    int start_ix, int start_iy);

  // destructor
  
  virtual ~GridClump();

  int OK;
  
  vector<Interval> intervals;
  int nIntervals;
  int nPoints;
  
  titan_grid_t grid;
  int startIx, startIy;
  double offsetX, offsetY;
  double startX, startY;
  int nX, nY;

  double stormSize;
  double dVolAtCentroid;
  double dAreaAtCentroid;
  double dAreaEllipse;
  double kmPerGridUnit;

protected:
  
private:

  int _initDone;
  int _isLatLon;

  int _minIx, _minIy;
  int _maxIx, _maxIy;

  double _dX, _dY, _dXAtEquator;
  double _dVolFlat, _dVolAtEquator;
  double _dAreaFlat, _dAreaAtEquator;

  void _shrinkWrap(Clump_order *clump_order);
  void _computeGeometry();
  
};

#endif



