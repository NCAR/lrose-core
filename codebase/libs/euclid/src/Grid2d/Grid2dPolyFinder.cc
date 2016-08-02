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
 * @file Grid2dPolyFinder.cc
 */

#include <euclid/Grid2dPolyFinder.hh>
#include <euclid/ConvexHull.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>
using std::vector;
using std::string;

#define BAD -99.0
#define GOOD 1.0

//----------------------------------------------------------------
/**
 * @param[in] grid Full grid with clump
 * @param[in,out] edge  Grid to turn into an edge grid
 * @param[out] xe  min x found at ye
 * @param[out] ye  min y with data
 *
 * @return true if edge grid was created, and interior points
 * exist, with xe and ye set to first edge point
 */
static bool _makeEdge(const GridAlgs &grid, GridAlgs &edge, int &xe, int &ye)
{
  edge.changeMissing(BAD);
  edge.setName("edge");
  edge.setAllMissing();
  
  bool has_interior = false;
  int nx = grid.getNx();
  int ny = grid.getNy();
  bool first = true;
  for (int y=0; y<ny; ++y)
  {
    for (int x=0; x<nx; ++x)
    {
      if (!grid.isMissing(x, y))
      {
	if (grid.isEdge(x, y))
	{
	  if (first)
	  {
	    first = false;
	    xe = x;
	    ye = y;
	  }
	  edge.setValue(x, y, GOOD);
	}
	else
	{
	  has_interior = true;
	}
      }
    }
  }
  if (!has_interior)
  {
    return false;
  }
  return (!first);
}

//----------------------------------------------------------------
static bool _good(GridAlgs &g, int x, int y)
{
  if (g.inRange(x, y))
  {
    return !g.isMissing(x, y);
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------
Grid2dPolyFinder::Grid2dPolyFinder()
{
  _x0 = _y0 = 0;
  _state = ZERO_ONE;
  _rescale = 1;
}

//----------------------------------------------------------------
Grid2dPolyFinder::~Grid2dPolyFinder()
{
}

//----------------------------------------------------------------
bool Grid2dPolyFinder::init(const GridAlgs &g,  const int rescale)
{
  int xe, ye;

  // make copy of input grid
  _hr_edge = g;

  // convert it to an edge grid if can do
  if (!_makeEdge(g, _hr_edge, xe, ye))
  {
    // no full -> no lower res
    return false;
  }
  
  if (rescale < 1)
  {
    _rescale = 1;
  }
  else
  {
    _rescale = rescale;
  }
  _g = g;
  if (_rescale > 1)
  {
    _g.reduceMax(rescale);
    _lr_edge = _g;
    if (!_makeEdge(_g, _lr_edge, xe, ye))
    {
      LOG(DEBUG) << "High res has an edge but not lowres";
      return false;
    }
  }
  else
  {
    _lr_edge = _hr_edge;
  }

  // assume this is 'lower left' point.
  _x.push_back(xe);
  _y.push_back(ye);
  _x.push_back(xe+1);
  _y.push_back(ye);
  _x0 = xe;
  _y0 = ye;
  _state = ZERO_ONE;
  return true;
}

//----------------------------------------------------------------
bool Grid2dPolyFinder::next(void)
{
  switch (_state)
  {
  case ZERO_ONE:
    if (_good(_lr_edge, _x0+1, _y0-1))
    {
      _x0 = _x0+1;
      _y0 = _y0 -1;
      _x.push_back(_x0);
      _y.push_back(_y0);
      _state = THREE_ZERO;
    }
    else if (_good(_lr_edge, _x0+1, _y0))
    {
      _x0 = _x0 + 1;  
      _x.push_back(_x0);
      _y.push_back(_y0);
      _state = ZERO_ONE;
    }
    else 
    {
      _state = ONE_TWO;
    }
    break;
  case ONE_TWO:
    if (_good(_lr_edge, _x0+1, _y0+1))
    {
      _x0 = _x0+1;
      _y0 = _y0+1;
      _x.push_back(_x0);
      _y.push_back(_y0);
      _state = ZERO_ONE;
    }
    else if (_good(_lr_edge, _x0, _y0+1))
    {
      _y0 = _y0 + 1;
      _x.push_back(_x0);
      _y.push_back(_y0);
      _state = ONE_TWO;
    }
    else 
    {
      _state = TWO_THREE;
    }
    break;
  case TWO_THREE:
    if (_good(_lr_edge, _x0-1, _y0+1))
    {
      _x0 = _x0-1;
      _y0 = _y0+1;
      _x.push_back(_x0);
      _y.push_back(_y0);
      _state = ONE_TWO;
    }
    else if (_good(_lr_edge, _x0-1, _y0))
    {
      _x0 = _x0 -1;
      _x.push_back(_x0);
      _y.push_back(_y0);
      _state = TWO_THREE;
    }
    else 
    {
      _state = THREE_ZERO;
    }
    break;
  case THREE_ZERO:
  default:
    if (_good(_lr_edge, _x0-1, _y0-1))
    {
      _x0 = _x0-1;
      _y0 = _y0-1;
      _x.push_back(_x0);
      _y.push_back(_y0);
      _state = TWO_THREE;
    }
    else if (_good(_lr_edge, _x0, _y0-1))
    {
      _y0 = _y0 -1;
      _x.push_back(_x0);
      _y.push_back(_y0);
      _state = THREE_ZERO;
    }
    else 
    {
      _state = ZERO_ONE;
    }
    break;
  }
  int n = _x.size();

  //   print();
  if (n == 1)
  {
    return true;
  }
  else
  {
    return (_x[n-1] != _x[0] || _y[n-1] != _y[0]);
  }
}
  
//----------------------------------------------------------------
void Grid2dPolyFinder::print(void) const
{
  int n = static_cast<int>(_x.size());
  printf("Grid2dPolyFinder box:(%d,%d) last:(%d,%d) state:%s\n",
	 _x0, _y0, _x[n-1], _y[n-1], _printState(_state).c_str());
}

//----------------------------------------------------------------
void Grid2dPolyFinder::printFull(void) const
{
  print();
  vector<int>::const_iterator ix, iy;
  int count = 0;
  for (ix=_x.begin(),iy=_y.begin(); ix!=_x.end() && iy!= _y.end(); ++ix,++iy)
  {
    printf("(%d,%d)", *ix, *iy);
    if (count++ > 10)
    {
      count=0;
      printf("\n");
    }
  }
  printf("\n");
}

//----------------------------------------------------------------
void Grid2dPolyFinder::removeLines(void)
{
  // look for lines, i.e. sets of indices that simply go "out and back".
  // start big and go to small.. 
  for (int k=static_cast<int>(_x.size()); k>=1; --k)
  {
    if (k < static_cast<int>(_x.size())/2)
    {
      _removeLines(k);
    }
  }

  // if a point equals the next one, it is out and back length 0
  vector<int>::iterator ix0, ix1, iy0, iy1;
  ix0 = _x.begin();
  iy0 = _y.begin();
  ix1 = ix0++;
  iy1 = iy0++;
  for (; ix1 != _x.end() && iy1 != _y.end(); ix1=ix0+1, iy1=iy0+1)
  {
    if (*ix0 == *ix1 && *iy0 == *iy1)
    {
      ix0 = _x.erase(ix0);
      iy0 = _y.erase(iy0);
    }
    else
    {
      ix0++;
      iy0++;
    }
  }
}

//----------------------------------------------------------------
void Grid2dPolyFinder::createConvexHull(void)
{
  ConvexHull C(_x, _y);
  _x = C.getX();
  _y = C.getY();
}

//----------------------------------------------------------------
string Grid2dPolyFinder::_printState(const state_e e) const
{
  string s;
  switch (e)
  {
  case ZERO_ONE:
    s = "ZERO_ONE";
    break;
  case ONE_TWO:
    s = "ONE_TWO";
    break;
  case TWO_THREE:
    s = "TWO_THREE";
    break;
  case THREE_ZERO:
    s = "THREE_ZERO";
    break;
  default:
    s = "?";
  }
  return s;
}

//----------------------------------------------------------------
// out and back of length k starting at point i is defined as:
//   i+0  = i+2*k
//   i+1  = i+2*k-1
//   i+2  = i+2*k-2
//   ...
//   i+k-1 = i+2*k-(k-1)
//    
void Grid2dPolyFinder::_removeLines(const int k)
{
  int i=0;
  while (true)
  {
    if (_isLine(i, k))
    {
      _removeLine(i, k);
    }
    if (!_incForLines(i, k))
    {
      break;
    }
  }
}

//----------------------------------------------------------------
bool Grid2dPolyFinder::_isLine(const int i, const int k) const
{
  if (i+2*k >= static_cast<int>(_x.size()))
  {
    return false;
  }

  for (int j=0; j<k; ++j)
  {
    if (_x[i+j] != _x[i+2*k-j])
    {
      return false;
    }
    if (_y[i+j] != _y[i+2*k-j])
    {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------
void Grid2dPolyFinder::_removeLine(const int i, const int k)
{
  vector<int>::iterator ix, iy;
  ix = _x.begin() + i;
  iy = _y.begin() + i;
  for (int j=0; j<2*k; ++j)
  {
    ix = _x.erase(ix);
    iy = _y.erase(iy);
  }
}

//----------------------------------------------------------------
bool Grid2dPolyFinder::_incForLines(int &i, const int k) const
{
  i = i + 1;
  return (i + 2*k < static_cast<int>(_x.size()));
}

