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
// ClumpGrid.hh
//
// ClumpGrid class - combines a clump with grid geometry so that
// computations may be done on the clump using that grid geometry.
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2021
//
///////////////////////////////////////////////////////////////

#ifndef ClumpGrid_HH
#define ClumpGrid_HH

#include <euclid/clump.h>
#include <euclid/PjgGridGeom.hh>
#include <vector>
using namespace std;

////////////////////////////////
// ClumpGrid

class ClumpGrid {
  
public:

  // constructor

  ClumpGrid();
  
  // initializer

  void init(const Clump_order *clump,
            int nx, int ny, int nz,
            double dx, double dy, double dz,
            double minx, double miny, double minz,
            bool isLatLon,
	    int start_ix, int start_iy);

  // destructor
  
  virtual ~ClumpGrid();

  PjgGridGeom grid;

  vector<Interval> intervals;
  int nIntervals;
  int nPoints;
  
  int startIx, startIy;
  double offsetX, offsetY;
  double startX, startY;
  int nX, nY;
  double dX, dY;

  double clumpSize;
  double dVolAtCentroid;
  double dAreaAtCentroid;
  double dAreaEllipse;
  double kmPerGridUnit;

protected:
  
private:

  int _initDone;

  int _minIx, _minIy;
  int _maxIx, _maxIy;

  double _dXAtEquator;
  double _dVolFlat, _dVolAtEquator;
  double _dAreaFlat, _dAreaAtEquator;

  void _shrinkWrap(const Clump_order *clump_order);
  void _computeGeometry();
  
};

#endif



