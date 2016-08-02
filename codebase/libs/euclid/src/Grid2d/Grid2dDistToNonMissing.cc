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
/**
 * @file Grid2dDistToNonMissing.cc
 */

#include <euclid/Grid2dDistToNonMissing.hh>
#include <toolsa/LogStream.hh>
#include <cmath>

//----------------------------------------------------------------
Grid2dDistToNonMissing::Grid2dDistToNonMissing(int maxSearch,
					       int searchScale) :
  _maxSearch(maxSearch), _searchScale(searchScale), _nx(0), _ny(0)
{
}

//----------------------------------------------------------------
Grid2dDistToNonMissing::~Grid2dDistToNonMissing()
{
}

//----------------------------------------------------------------
void Grid2dDistToNonMissing::update(const Grid2d &g)
{
  int nx = g.getNx();
  int ny = g.getNy();
  if (nx != _nx || ny != _ny)
  {
    _nx = nx;
    _ny = ny;
    _xIndex = Grid2d("xIndex", nx, ny, MISSING);
    _yIndex = Grid2d("yIndex", nx, ny, MISSING);
    _rebuild(g);
  }
  else
  {
    if (_missingChanged(g))
    {
      _rebuild(g);
    }
  }
}

//----------------------------------------------------------------
bool Grid2dDistToNonMissing::distanceToNonMissing(const Grid2d &data,
						  Grid2d &distOut,
						  Grid2d &valOut)
{      
  // check dimensions for consistency
  if (!data.dimensionsEqual(distOut) || !data.dimensionsEqual(valOut))
  {
    LOG(ERROR) << "Dimensions inconsistent";
    return false;
  }

  // update if update is needed
  update(data);

  double maxRadius = _maxSearch;

  // At each grid point x,y find and record minimum distances to non-missing
  // data and the non missing data value  
  for (int j = 0; j < _ny; j++)
  {
    for (int i = 0; i < _nx; i++)
    { 
      
      // Taxicab metric distance to non-missing data from (i,j)
      double minDist = maxRadius;   

      // Value of data at minimum distance
      double closeVal;

      // Flag to indicate successful search
      bool distFound = false;
      
      // Check right away if point is not missing or bad
      // since we need not go further and distance = 0 at such points
      if (!data.isMissing(i, j))
      { 
	distFound = true;
	minDist = 0.0;
	closeVal = data(i,j);
      }
      else
      {
	int nearI, nearJ;
	if ((distFound = nearestPoint(i, j, nearI, nearJ)))
	{
	  double di = fabs(double(nearI - i));
	  double dj = fabs(double(nearJ - j));
	  if (dj > di)
	  {
	    di = dj;
	  }
	  minDist = di;
	  if (!data.getValue(nearI, nearJ, closeVal))
	  {
	    LOG(WARNING) << "Unexpected missing where not expected "
			 << "(" << nearI << "," << nearJ << ")";
	    distFound = false;
	  }
	}
      }
      
      if (!distFound)
      {
	// At points where no nearby data is non-missing, set result to max
	distOut(i, j) = maxRadius;
	valOut.setMissing(i, j);
      }
      else
      {
	// Set the minimum distance to non missing data and the value 
	distOut(i, j) = minDist;
	valOut(i, j) = closeVal;
      }
    } // End for
  }// End for

  return true;
}

//----------------------------------------------------------------
bool Grid2dDistToNonMissing::replaceMissing(const Grid2d &data,
					    Grid2d &dataOut)
{      
  // initialize output to input
  dataOut = data;

  // update if update is needed
  update(data);

  // At each grid point x,y find and record minimum distances to non-missing
  // data and the non missing data value  
  for (int j = 0; j < _ny; j++)
  {
    for (int i = 0; i < _nx; i++)
    { 
      if (data.isMissing(i, j))
      { 
	int nearI, nearJ;
	if (nearestPoint(i, j, nearI, nearJ))
	{
	  double closeVal;
	  if (!data.getValue(nearI, nearJ, closeVal))
	  {
	    LOG(WARNING) << "Unexpected missing where not expected "
			 << "(" << nearI << "," << nearJ << ")";
	  }
	  else
	  {
	    dataOut(i, j) = closeVal;
	  }
	}
      }
    }
  }
  return true;
}

//----------------------------------------------------------------
bool Grid2dDistToNonMissing::nearestPoint(int x, int y, int &nearX,
					  int &nearY) const
{
  double dx, dy;
  if (_xIndex.getValue(x, y, dx) && _yIndex.getValue(x, y, dy))
  {
    if (dx == HAS_DATA)
    {
      nearX = x;
      nearY = y;
      return true;
    }
    else
    {
      nearX = static_cast<int>(dx);
      nearY = static_cast<int>(dy);
      return true;
    }
  }
  else
  {
    // nothing
    return false;
  }
}

//----------------------------------------------------------------
bool Grid2dDistToNonMissing::_missingChanged(const Grid2d &g) const
{
  int diff = 0;
  for (int i=0; i<g.getNdata(); ++i)
  {
    // is the new grid data missing at i?
    bool newMissing = g.isMissing(i);

    // the state says data was missing at i if either an index is set
    // indicating an offset 
    // (the value is not missing and is not HAS_DATA) or the
    // index is not set (the point has missing data and is out of the
    // max search distance)
    bool oldMissing;
    double v;
    if (_xIndex.getValue(i, v))
    {
      oldMissing = v != HAS_DATA;
    }
    else
    {
      oldMissing = true;
    }
    if (oldMissing != newMissing)
    {
      ++diff;
    }
  }
  if (diff > 0)
  {
    LOG(DEBUG_VERBOSE) << "Rebuild because " << diff << " points different";
    return true;
  }
  else
  {
    LOG(DEBUG_VERBOSE) << "Do not Rebuild because 0 points different";
    return false;
  }
}

//----------------------------------------------------------------
void Grid2dDistToNonMissing::_rebuild(const Grid2d &g)
{
  _xIndex.setAllMissing();
  _yIndex.setAllMissing();


  // create a copy of the input grid  (gradually to be eroded away)
  Grid2d s(g);

  for (int r=_searchScale; r<_maxSearch; r += _searchScale)
  {
    LOG(DEBUG_VERBOSE) << "Checking r=" << r << ", ngood=", s.numGood();
    int totalNew=0;
    for (int y=0; y<_ny; ++y)
    {
      for (int x=0; x<_nx; ++x)
      {
	if (!s.isMissing(x, y))
	{
	  int nnew = 0;
	  nnew = _rebuild1(r, x, y, g);
	  totalNew += nnew;
	  if (nnew == 0)
	  {
	    s.setMissing(x, y);
	  }
	}
      }
    }
    LOG(DEBUG_VERBOSE) << "R=" << r << " had " << totalNew << " new";
  }

  for (int y=0; y<_ny; ++y)
  {
    for (int x=0; x<_nx; ++x)
    {
      if (!g.isMissing(x, y))
      {
	_xIndex(x, y) = HAS_DATA;
	_yIndex(x, y) = HAS_DATA;
      }
    }
  }
}

//----------------------------------------------------------------
int Grid2dDistToNonMissing::_rebuild1(int r, int x, int y, const Grid2d &g)
{
  // x,y is the point with data

  int nnew = 0;

  //
  // Search horizontal edges of box surrounding point (x, y)
  // with box lengths 2r
  for (int iy=y-r; iy<=y+r; iy += 2*r)
  {
    if (iy < 0 || iy >= _ny)
    {
      continue;
    }
    for (int ix = x-r; ix<=x+r; ix += _searchScale)
    {
      if (ix >= 0 && ix < _nx)
      {
	if (g.isMissing(ix, iy))  
	{
	  if (_xIndex.isMissing(ix, iy))
	  {
	    // not yet set, set now
	    _xIndex(ix, iy) = x;
	    _yIndex(ix, iy) = y;
	    ++nnew;
	  }
	}
      }
    }
    //
    // Search vertical edges of box surrounding point (i,j)
    // with box lengths 2r
    // 
    for (int ix=x-r; ix<=x+r; ix+= 2*r)
    {
      if (ix < 0 || ix >= _nx)
      {
	continue;
      }
      for (int iy=y-r; iy<=y+r; iy += _searchScale)
      {
	if (iy >= 0 && iy < _ny)
	{
	  if (g.isMissing(ix, iy))
	  {
	    if (_xIndex.isMissing(ix, iy))
	    {
	      _xIndex(ix, iy) = x;
	      _yIndex(ix, iy) = y;
	      ++nnew;
	    }
	  }
	}
      }
    }
  }
  return nnew;
}

