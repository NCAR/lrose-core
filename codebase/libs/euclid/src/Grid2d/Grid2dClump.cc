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
 * @file Grid2dClump.cc
 */

#include <vector>
#include <euclid/Grid2dClump.hh>
#include <euclid/PointList.hh>
#include <toolsa/LogStream.hh>
using std::vector;
using std::pair;

//----------------------------------------------------------------
Grid2dClump::Grid2dClump(const Grid2d &g) : Grid2d(g), _iwork(g)
{
  _nx = g.getNx();
  _ny = g.getNy();
  _iwork.setAllToValue(NO_MARKER);
  for (int i=0; i<_nx*_ny; ++i)
  {
    if (isMissing(i))
    {
      _iwork[i] = PROCESSED;
    }
  }
  _n.clear();
}

//----------------------------------------------------------------
Grid2dClump::Grid2dClump(const Grid2d &g, const double bad) : Grid2d(g),
							     _iwork(g)
{
  _nx = g.getNx();
  _ny = g.getNy();
  _iwork.setAllToValue(NO_MARKER);
  for (int i=0; i<_nx*_ny; ++i)
  {
    double  v;
    if (getValue(i, v))
    {
      if (v == bad)
      {
	_iwork[i] = PROCESSED;
      }
    }
    else
    {
      _iwork[i] = PROCESSED;
    }
  }
  _n.clear();
}

//----------------------------------------------------------------
Grid2dClump::~Grid2dClump()
{
}

//----------------------------------------------------------------
std::vector<clump::Region_t> Grid2dClump::buildRegions(void)
{
  int x, y;
  clump::Region_t l;
  vector<clump::Region_t> ret;

  for (y=0; y<_ny; ++y)
  {
    for (x=0; x<_nx; ++x)
    {
      if (_iwork(x,y) == PROCESSED)
      {
	continue;
      }
      _buildRegion(x, y);
      if (!_n.empty())
      {
	ret.push_back(_n);
      }
    }
  }
  return ret;
}

//----------------------------------------------------------------
std::vector<PointList> Grid2dClump::buildRegionPointlists(void)
{
  int x, y;
  clump::Region_t l;
  vector<PointList> ret;

  for (y=0; y<_ny; ++y)
  {
    for (x=0; x<_nx; ++x)
    {
      if (_iwork(x,y) == PROCESSED)
      {
	continue;
      }
      _buildRegion(x, y);
      if (!_n.empty())
      {
	PointList P(_nx, _ny, _n);
	ret.push_back(P);
      }
    }
  }
  return ret;
}

//----------------------------------------------------------------
vector<clump::Region_t> Grid2dClump::buildRegionsRecursive(void)
{
  int x, y;
  clump::Region_t l;
  vector<clump::Region_t> ret;

  for (y=0; y<_ny; ++y)
  {
    for (x=0; x<_nx; ++x)
    {
      if (_iwork(x,y) == PROCESSED)
      {
	continue;
      }

      _buildRegionRecursive(x, y);
      if (!_n.empty())
      {
	ret.push_back(_n);
      }
    }
  }
  return ret;
}

//----------------------------------------------------------------
void Grid2dClump::_buildRegion(int ix, int iy)
{
  _n.clear();
  
  while (true)
  {
    bool done;
    _iwork(ix, iy) = THISCLUMP_NOTDONE;
    pair<int, int> p = _growNonrecursive(ix, iy, done);
    if (done)
    {
      pair<int, int> p2 = _findNondone(done);
      if (done)
      {
	break;
      }
      ix = p2.first;
      iy = p2.second;
    }
    else
    {
      ix = p.first;
      iy = p.second;
    }
  }
  _buildN();
}

//----------------------------------------------------------------
void Grid2dClump::_buildRegionRecursive(int ix, int iy)
{
  _n.clear();
  _growRecursive(ix, iy);
  _buildN();
}

//----------------------------------------------------------------
void Grid2dClump::_growRecursive(int x, int y)
{
  int ix, iy;

  // make sure input point is part of the current clump.
  if (_iwork(x,y) != NO_MARKER)
  {
    return;
  }

  _iwork(x,y) = THISCLUMP_DONE;

  // Recursively add neighboring points
  for ( iy=y-1; iy<=y+1; ++iy )
  {
    for ( ix=x-1; ix<=x+1; ++ix )
    {
      if (!_growOk(ix, iy, x, y))
      {
	continue;
      }
      _growRecursive(ix, iy);
    }
  }
}

//----------------------------------------------------------------
pair<int,int> Grid2dClump::_growNonrecursive(int x, int y, bool &done)
{
  int ix, iy;
  int ix_ret, iy_ret;
  bool did_grow = false;

  // make sure input point is set properly.
  if (_iwork(x, y) != THISCLUMP_NOTDONE)
  {
    LOG(WARNING) << "in clump";
  }
  
  // by the time through here, it'll be done.
  _iwork(x, y) = THISCLUMP_DONE;

  // Recursively add neighboring points and set one for return 
  for ( iy=y-1; iy<=y+1; ++iy )
  {
    for ( ix=x-1; ix<=x+1; ++ix )
    {
      if (!_growOkNonrecursive(ix, iy, x, y))
      {
	continue;
      }
      _iwork(ix, iy) = THISCLUMP_NOTDONE;
      ix_ret = ix;
      iy_ret = iy;
      did_grow = true;
    }
  }

  if (did_grow)
  {
    pair<int, int> p(ix_ret, iy_ret);
    done = false;
    return p;
  }
  else
  {
    done = true;
    pair<int, int> p(0,0);
    return p;
  }
}

//----------------------------------------------------------------
void Grid2dClump::_buildN(void)
{
  _n.clear();

  int x, y;
  for (y=0; y<_ny; ++y)
  {
    for (x=0; x<_nx; ++x)
    {
      if (_iwork(x, y) == THISCLUMP_DONE)
      {
	pair<int, int> p(x, y);
	_n.push_back(p);
	_iwork(x, y) = PROCESSED;
      }
      else if (_iwork(x,  y) == THISCLUMP_NOTDONE)
      {
	LOG(ERROR) << "in Grid2dClump";
      }
    }
  }
}

//----------------------------------------------------------------
pair<int, int> Grid2dClump::_findNondone(bool &done) const
{
  int x, y;
  for (y=0; y<_ny; ++y)
  {
    for (x=0; x<_nx; ++x)
    {
      if (_iwork(x, y) == THISCLUMP_NOTDONE)
      {
	pair<int, int> p(x, y);
	done = false;
	return p;
      }
    }
  }
  pair<int, int> p(0,0);
  done = true;
  return p;
}

//----------------------------------------------------------------
bool Grid2dClump::_growOk(int ix, int iy, int x, int y) const
  {
    // The point has been dealt with (just added)
    if ( ix == x && iy == y )
    {
      return(false);
    }
    // Out of range of the data
    if ( ix < 0 || ix >= _nx)
    {
      return(false);
    }
    if ( iy < 0 || iy >= _ny)
    {
      return(false);
    }
    // Previously dealt with
    if (_iwork(ix, iy) != NO_MARKER)
    {
      return false;
    }
    return(true); 
  }

//----------------------------------------------------------------
bool Grid2dClump::_growOkNonrecursive(int ix, int iy, int x, int y) const
{
  // The point has been dealt with (just added)
  if ( ix == x && iy == y )
  {
    return(false);
  }
  // Out of range of the data
  if ( ix < 0 || ix >= _nx)
  {
    return(false);
  }
  if ( iy < 0 || iy >= _ny)
  {
    return(false);
  }

  // Previously dealt with
  if (_iwork(ix, iy) == PROCESSED || _iwork(ix, iy) == THISCLUMP_DONE)
  {
    return false;
  }

  // here either NO_MARKER or THISCLUMP_NOTDONE.
  return(true); 
}

