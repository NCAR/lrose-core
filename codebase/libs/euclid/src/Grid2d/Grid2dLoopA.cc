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
 * @file Grid2dLoopA.cc
 */
#include <euclid/Grid2dLoopA.hh>
#include <euclid/Grid2dLoopAlg.hh>
#include <euclid/Grid2d.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>
#include <cstdlib>
#include <cmath>
using std::string;

//----------------------------------------------------------------
Grid2dLoopA::Grid2dLoopA(int nx, int ny, int sx, int sy) : _nx(nx), _ny(ny),
  _sx(sx), _sy(sy)
{
  if (_nx < 2 || _ny < 2)
  {
    LOG(FATAL) << "too few x,y " << nx << " " << ny;
    exit(1);
  }

  // lower left, initial state
  _x = 0;
  _y = 0;
  _minx = _x - _sx;
  _miny = _y - _sy;
  _maxx = _x + _sx;
  _maxy = _y + _sy;
  _state = INIT;
}

//----------------------------------------------------------------
Grid2dLoopA::~Grid2dLoopA()
{
}

//----------------------------------------------------------------
void Grid2dLoopA::reinit(void)
{
  // lower left
  _x = 0;
  _y = 0;
  _minx = _x - _sx;
  _miny = _y - _sy;
  _maxx = _x + _sx;
  _maxy = _y + _sy;
  _state = INIT;
}

//----------------------------------------------------------------
Grid2dLoopA::State_t Grid2dLoopA::getXy(int &x, int &y) const
{
  x = _x;
  y = _y;
  return _state;
}

//----------------------------------------------------------------
bool Grid2dLoopA::getXyAndResult(const Grid2dLoopAlg &alg,
				 int minGood, int &x, int &y, 
				 double &result) const
{
  x = _x;
  y = _y;
  return alg.getResult(minGood, result);
}

//----------------------------------------------------------------
bool Grid2dLoopA::increment(const Grid2d &G, Grid2dLoopAlg &alg)
{
  switch (_state)
  {
  case INIT:
    // initially start moving up in y
    _state = INC_Y;

    // compute the full thing initially
    _fullCompute(G, alg);
    break;
  case INC_Y:
    // keep moving up in y if you can
    if (_y+1 < _ny)
    {
      ++_y;
      _subtractY(_miny, G, alg);
      _miny++;
      _maxy++;
      _addY(_maxy, G, alg);
    }
    else
    {
      if (++_x >= _nx)
      {
	// upper right will be last point depending on nx
	return false;
      }
      // move to right when hit the top
      _state = INC_X;
      _subtractX(_minx, G, alg);
      _minx++;
      _maxx++;
      _addX(_maxx, G, alg);
    }
    break;
  case DEC_Y:
    // keep moving down in y if you can
    if (_y-1 >= 0)
    {
      --_y;
      _subtractY(_maxy, G, alg);
      _miny--;
      _maxy--;
      _addY(_miny, G, alg);
    }
    else
    {
      if (++_x >= _nx)
      {
	// lower right will be last point depending on nx
	return false;
      }
      // start moving to the right
      _state = INC_X;
      _subtractX(_minx, G, alg);
      _minx++;
      _maxx++;
      _addX(_maxx, G, alg);
    }
    break;
  case INC_X:
  default:
    // increment x only by one, then move up or down
    // depending on where you are at
    if (_y >= _ny-1)
    {
      // down we go
      _state = DEC_Y;
      --_y;
      _subtractY(_maxy, G, alg);
      _miny--;
      _maxy--;
      _addY(_miny, G, alg);
    }
    else if (_y == 0)
    {
      // up up and away
      _state = INC_Y;
      ++_y;
      _subtractY(_miny, G, alg);
      _miny++;
      _maxy++;
      _addY(_maxy, G, alg);
    }
    else
    {
      // ?
      LOG(ERROR) << "state bad";
    }
    break;
  }

  return true;
}

//----------------------------------------------------------------
string Grid2dLoopA::printState(const State_t s)
{
  string ret;
  switch (s)
  {
  case INIT:
    ret = "INIT";
    break;
  case INC_Y:
    ret = "INC_Y";
    break;
  case INC_X:
    ret = "INC_X";
    break;
  case DEC_Y:
    ret = "DEC_Y";
    break;
  default:
    ret = "UNKNOWN";
    break;
  }
  return ret;
}

//----------------------------------------------------------------
void Grid2dLoopA::_fullCompute(const Grid2d &G, Grid2dLoopAlg &alg) const
{
  for (int y=_miny; y<=_maxy; ++y)
  {
    if (y >= 0 && y < _ny)
    {
      for (int x=_minx; x<=_maxx; ++x)
      {
	if (x >= 0 && x < _nx)
	{
	  alg.increment(x, y, G);
	}
      }
    }
  }
}

//----------------------------------------------------------------
void Grid2dLoopA::_addY(int y, const Grid2d &G, Grid2dLoopAlg &alg) const
{
  if (y <0 || y >= _ny)
  {
    return;
  }
  for (int x=_minx; x<=_maxx; ++x)
  {
    if (x >= 0 && x < _nx)
    {
      alg.increment(x, y, G);
    }
  }
}

//----------------------------------------------------------------
void Grid2dLoopA::_subtractY(int y, const Grid2d &G, Grid2dLoopAlg &alg) const
{
  if (y <0 || y >= _ny)
  {
    return;
  }
  for (int x=_minx; x<=_maxx; ++x)
  {
    if (x >= 0 && x < _nx)
    {
      alg.decrement(x, y, G);
    }
  }
}

//----------------------------------------------------------------
void Grid2dLoopA::_addX(int x, const Grid2d &G, Grid2dLoopAlg &alg) const
{
  if (x <0 || x >= _nx)
  {
    return;
  }
  for (int y=_miny; y<=_maxy; ++y)
  {
    if (y >= 0 && y < _ny)
    {
      alg.increment(x, y, G);
    }
  }
}

//----------------------------------------------------------------
void Grid2dLoopA::_subtractX(int x, const Grid2d &G,  Grid2dLoopAlg &alg) const
{
  if (x <0 || x >= _nx)
  {
    return;
  }
  for (int y=_miny; y<=_maxy; ++y)
  {
    if (y >= 0 && y < _ny)
    {
      alg.decrement(x, y, G);
    }
  }
}


