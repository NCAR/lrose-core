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
//   File: $RCSfile: KernelTemplate.cc,v $
//   Version: $Revision: 1.11 $  Dated: $Date: 2016/03/04 02:39:07 $

/**
 * @file KernelTemplate.cc
 * @brief KernelTemplate main class
 */
#include "KernelTemplate.hh"
#include <euclid/Grid2d.hh>
#include <toolsa/LogStream.hh>
#include "Kernel.hh"

const int
KernelTemplate::_x_off[30] = {1, 2,  0, 0,  1,  1,  2, 2,  0, 0,  1, 1,  2,  2,
			      -1, -1, 3, 3, -1, -1, 3, 3, -2, -2, 4, 4, -2, -2,
			      4, 4};

const int
KernelTemplate::_y_off[30] = {0, 0, -1, 1, -1, 1, -1, 1, -2, 2, -2, 2, -2, 2, 
			      -1, 1, -1, 1, -2, 2, -2, 2, -1, 1, -1, 1, -2, 2,
			      -2, 2};

const int
KernelTemplate::_x_off_out[30] = {-1, -2,  0, 0, -1, -1, -2, -2, 0, 0, -1, -1,
				  -2, -2, 1, 1, -3, -3, 1, 1, -3, -3, 2, 2,
				  -4, -4, 2, 2, -4, -4};

const int
KernelTemplate::_y_off_out[30] = {0, 0, -1, 1, -1, 1, -1, 1, -2, 2, -2, 2,
				  -2, 2, -1, 1, -1, 1, -2, 2, -2, 2, -1, 1, 
				  -1, 1, -2, 2, -2, 2};


const int KernelTemplate::_prev_needed[30][3] = 
{
  {-1, -1, -1},  // 0
  { 0, -1, -1},  // 1
  {-1, -1, -1},  // 2
  {-1, -1, -1},  // 3
  { 0,  2, -1},  // 4
  { 0,  3, -1},  // 5
  { 0,  1,  4},  // 6
  { 0,  1,  5},  // 7
  { 2,  4, -1},  // 8
  { 3,  5, -1},  // 9
  { 2,  4,  8},  // 10
  { 3,  5,  9},  // 11
  { 4,  6, 10},  // 12
  { 5,  7, 11},  // 13
  {-1, -1, -1},  // 14
  {-1, -1, -1},  // 15
  { 1,  6, 12},  // 16
  { 1,  7, 13},  // 17
  { 2,  8, 14},  // 18
  { 3,  9, 15},  // 19
  { 6, 12, 16},  // 20
  { 7, 13, 17},  // 21
  {14, 18, -1},  // 22
  {15, 19, -1},  // 23
  {16, 20, -1},  // 24
  {17, 21, -1},  // 25
  {14, 18, 22},  // 26
  {15, 19, 23},  // 27
  {16, 20, 24},  // 28
  {17, 21, 25}   // 29 
};

/*----------------------------------------------------------------*/
KernelTemplate::KernelTemplate(const int nx, const int ny, const int x,
			       const int y, const bool moving_in)
{
  _nx = nx; 
  _ny = ny;
  _x = x;
  _y = y;
  _moving_in = moving_in;
}

/*----------------------------------------------------------------*/
KernelTemplate::~KernelTemplate()
{
}

/*----------------------------------------------------------------*/
void KernelTemplate::add_kernel_points(const Grid2d &mask, const int maxpt,
				       Kernel &k)
{
  if (_moving_in)
    _add_kernel_points(mask, maxpt, _x_off, _y_off, k);
  else
    _add_kernel_points(mask, maxpt, _x_off_out, _y_off_out, k);
}

/*----------------------------------------------------------------*/
void KernelTemplate::add_kernel_outside_points(const Grid2d &mask,
					       const int maxpt,
					       Kernel &k)
{
  // now add the outside points. The starting point should be one point in
  // x closer to (or farther from if moving_in=false) the _x0,_y0 point
  // (the starting point)
  int x0;
  if (_moving_in)
    x0 = _x-1;
  else
    x0 = _x+1;
  if (mask.isMissing(x0, _y))
  {
    LOG(ERROR) << "unexpected missing value";
    return;
  }
  else
    k.add_outside(x0, _y);

  // the inside points are an 'upside down' kernel using same offsets,
  // but offset in x by 1
  if (_moving_in)
    _add_kernel_outside_points(mask, maxpt, x0, _x_off_out, _y_off_out, k);
  else
    _add_kernel_outside_points(mask, maxpt, x0, _x_off, _y_off, k);
}

/*----------------------------------------------------------------*/
void KernelTemplate::_add_kernel_points(const Grid2d &mask, const int maxpt,
					const int *x_off, const int *y_off,
					Kernel &k)
{
  for (int i=0; i<30; ++i)
  {
    _added[i] = false;
    int xi = _x + x_off[i];
    if (xi<0 || xi >= _nx)
      continue;
    int yi = _y + y_off[i];
    if (yi<0 || yi >= _ny)
      continue;
    if (!mask.isMissing(xi, yi))
    {
      if (_previous_ok(i))
      {
	k.add(xi, yi);
	_added[i] = true;
	if (k.npt() >= maxpt)
	  return;
      }
    }
  }
}

/*----------------------------------------------------------------*/
void KernelTemplate::_add_kernel_outside_points(const Grid2d &mask,
						const int maxpt,
						const int x0, 
						const int *x_off, 
						const int *y_off,
						Kernel &k)
{
  for (int i=0; i<30; ++i)
  {
    _added[i] = false;
    int xi = x0 + x_off[i];
    if (xi<0 || xi >= _nx)
      continue;
    int yi = _y + y_off[i];
    if (yi<0 || yi >= _ny)
      continue;

    if (mask.isMissing(xi, yi))
      continue;
    if (_previous_ok(i))
    {
      k.add_outside(xi, yi);
      _added[i] = true;
      if (k.npt_outside() >= maxpt)
	return;
    }
  }
}

/*----------------------------------------------------------------*/
bool KernelTemplate::_previous_ok(const int i) const
{
  // prev needed is 0, 1, 2, or 3 things, with -1 marking the
  // end of what is previously needed.

  if (_prev_needed[i][0] == -1)
    // nothing previous
    return true;

  for (int j=0; j<3; ++j)
  {
    int ki = _prev_needed[i][j];
    if (ki >= 0)
    {
      if (_added[ki])
	return true;
    }
    else
      break;
  }
  // never found anything, but there was at least 1 thing needed
  return false;
}

