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
 * @file Grid2dLoop.cc
 */
#include <cstdio>
#include <cstdlib>
#include <euclid/Grid2dLoop.hh>
#include <toolsa/LogStream.hh>
using std::vector;
using std::pair;
using std::string;

static bool debug=false;

//----------------------------------------------------------------
Grid2dLoop::Grid2dLoop(const int nx, const int ny)
{
  _nx = nx;
  _ny = ny;
  if (_nx < 2 || _ny < 2)
  {
    LOG(FATAL) << "too few x,y " << nx << " " << ny;
    exit(1);
  }

  // lower left
  _x = 0;
  _y = 0;
  _state = INIT;
}

//----------------------------------------------------------------
Grid2dLoop::~Grid2dLoop()
{
}

//----------------------------------------------------------------
void Grid2dLoop::reinit(void)
{
  // lower left
  _x = 0;
  _y = 0;
  _state = INIT;
}

//----------------------------------------------------------------
Grid2dLoop::State_t Grid2dLoop::getXy(int &x, int &y) const
{
  x = _x;
  y = _y;
  return _state;
}

//----------------------------------------------------------------
bool Grid2dLoop::increment(void)
{
  switch (_state)
  {
  case INIT:
    // initially start moving up in y
    _state = INC_Y;
    _y++;
    break;
  case INC_Y:
    // keep moving up in y if you can
    if (_y+1 < _ny)
    {
      ++_y;
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
    }
    break;
  case DEC_Y:
    // keep moving down in y if you can
    if (_y-1 >= 0)
    {
      --_y;
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
    }
    else if (_y == 0)
    {
      // up up and away
      _state = INC_Y;
      ++_y;
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
vector<pair<int,int> > Grid2dLoop::newXy(const int sx, const int sy) const
{
  vector<pair<int,int> > ret;
  int x0, y0;
  if (debug)  printf("New xy at (%d,%d)", _x, _y);
  int xmin=-1, xmax=-1, ymin=-1, ymax=-1;
  bool firstx=true, firsty=true;
  switch (_state)
  {
  case INIT:
    for (int y=-sy; y<=sy; ++y)
    {
      int yi=_y+y;
      if (yi < 0 || yi >= _ny)
      {
	continue;
      }
      if (firsty)
      {
	ymin = ymax = yi;
	firsty = false;
      }
      else
      {
	ymax = yi;
      }

      for (int x=-sx; x<=sx; ++x)
      {
	int xi = _x + x;
	if (xi < 0 || xi >= _nx)
	{
	  continue;
	}
	if (firstx)
	{
	  xmin = xmax = xi;
	  firstx = false;
	}
	else
	{
	  xmax = xi;
	}

	pair<int,int> p(xi,yi);
	ret.push_back(p);
      }
    }
    if (debug)  printf("init add in add points x=[%d,%d], y=[%d,%d]\n", xmin,
		       xmax, ymin, ymax);
    break;
  case INC_Y:
    //add ing a new y value for current range of x, at the top (sx above)
    y0 = _y+sy;
    if (y0 < _ny && y0 >= 0)
    {
      for (int x=-sx; x<=sx; ++x)
      {
	int xi = _x + x;
	if (xi < 0 || xi >= _nx)
	{
	  continue;
	}
	if (firstx)
	{
	  xmin = xmax = xi;
	  firstx = false;
	}
	else
	{
	  xmax = xi;
	}

	pair<int,int> p(xi,y0);
	ret.push_back(p);
      }
    }
    if (debug) printf("inc y add in add points for x=[%d,%d] y=%d\n", 
		      xmin, xmax, y0);
    break;
  case DEC_Y:
    // adding a new y value for curent range of x at bottome (sx below)
    y0 = _y-sy;
    if (y0 < _ny && y0 >= 0)
    {
      for (int x=-sx; x<=sx; ++x)
      {
	int xi = _x + x;
	if (xi < 0 || xi >= _nx)
	{
	  continue;
	}
	if (firstx)
	{
	  xmin = xmax =xi;
	  firstx = false;
	}
	else
	{
	  xmax = xi;
	}

	pair<int,int> p(xi,y0);
	ret.push_back(p);
      }
    }
    if (debug) printf("dec y add points for x=[%d,%d] y=%d\n", 
		      xmin, xmax, y0);
    break;
  case INC_X:
  default:
    // adding a new x value at top for curent range of y 
    x0 = _x+sx;
    if (x0 < _nx && x0 >= 0)
    {
      for (int y=-sy; y<=sy; ++y)
      {
	int yi = _y + y;
	if (yi < 0 || yi >= _ny)
	{
	  continue;
	}
	if (firsty)
	{
	  ymin = ymax =yi;
	  firsty = false;
	}
	else
	{
	  ymax = yi;
	}

	pair<int,int> p(x0,yi);
	ret.push_back(p);
      }
    }
    if (debug) printf("inc x add in  points for x=%d y=[%d,%d]\n",
		      x0, ymin, ymax);
    break;
  }
  return ret;
}

//----------------------------------------------------------------
vector<pair<int,int> > Grid2dLoop::oldXy(const int sx, const int sy) const
{
  vector<pair<int,int> > ret;
  int x0, y0;
  if (debug) printf("Old xy at (%d,%d)", _x, _y);
  int xmin=-1, xmax=-1, ymin=-1, ymax=-1;
  bool firstx=true, firsty=true;
  switch (_state)
  {
  case INIT:
    break;
  case INC_Y:
    // y has incremented, hence subtract one more
    y0 = _y-sy-1;
    if (y0 < _ny && y0 >= 0)
    {
      for (int x=-sx; x<=sx; ++x)
      {
	int xi = _x + x;
	if (xi < 0 || xi >= _nx)
	{
	  continue;
	}
	if (firstx)
	{
	  xmin = xmax = xi;
	  firstx = false;
	}
	else
	{
	  xmax = xi;
	}

	pair<int,int> p(xi,y0);
	ret.push_back(p);
      }
    }
    if (debug)     printf("inc y remove points for x=[%d,%d] y=%d\n", 
			  xmin, xmax, y0);
    break;
  case DEC_Y:
    // y has decremented, hence add one more
    y0 = _y+sy+1;
    if (y0 < _ny && y0 >= 0)
    {
      for (int x=-sx; x<=sx; ++x)
      {
	int xi = _x + x;
	if (xi < 0 || xi >= _nx)
	{
	  continue;
	}
	if (firstx)
	{
	  xmin = xmax =xi;
	  firstx = false;
	}
	else
	{
	  xmax = xi;
	}

	pair<int,int> p(xi,y0);
	ret.push_back(p);
      }
    }
    if(debug) printf("dec y remove points for x=[%d,%d] y=%d\n", 
		     xmin, xmax, y0);
    break;
  case INC_X:
  default:
    // x has incremented, hence subrtract one more
    x0 = _x-sx-1;
    if (x0 < _nx && x0 >= 0)
    {
      for (int y=-sy; y<=sy; ++y)
      {
	int yi = _y + y;
	if (yi < 0 || yi >= _ny)
	{
	  continue;
	}
	if (firsty)
	{
	  ymin = ymax = yi;
	  firsty = false;
	}
	else
	{
	  ymax = yi;
	}

	pair<int,int> p(x0,yi);
	ret.push_back(p);
      }
    }
    if (debug) printf("inc x remove points for x=%d, y=[%d,%d]\n",
		      x0, ymin, ymax);
    break;
  }
  return ret;
}

//----------------------------------------------------------------
string Grid2dLoop::printState(const State_t s)
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
