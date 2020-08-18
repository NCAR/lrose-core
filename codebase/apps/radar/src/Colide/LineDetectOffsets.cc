/**
 * @file LineDetectOffsets.cc
 *
 * offset indices into images, used as a lookup table, for line detection.
*/

#include "LineDetectOffsets.hh"
#include "Window.hh"
#include <stdio.h>

/*----------------------------------------------------------------*/
LineDetectOffset::LineDetectOffset(const Parms &p,
				   const Window &window,
				   int nx, int ny,
				   int i,  // angle index
				   int width, // width of template
				   double missing
				   )
{
  double a, half_wid, step, len;
  // int nang, npts;
    
  // get some dimensions for the offset table.
  // npts = p._window.num_points();
  // nang = p._window.num_angles();
  step = window.step_size();
  a = window.ith_angle(i);
  len = nx;
  half_wid = ny/2.0;

  _center = Grid2dOffset(len, 0.0, a, step, width, missing);
  _left = Grid2dOffset(len, -half_wid, a, step*2, width, missing);
  _leftd = Grid2dOffset(len, -half_wid, a, step, width, missing);
  _right = Grid2dOffset(len, half_wid, a, step*2, width, missing);
  _rightd = Grid2dOffset(len, half_wid, a, step, width, missing);
  _angle = a;
}

/*----------------------------------------------------------------*/
LineDetectOffset::~LineDetectOffset()
{
}

/*----------------------------------------------------------------*/
LineDetectOffsets::LineDetectOffsets(const Parms &p,
				     const Window &window,
				     int nx, int ny,
				     int width, double missing)
{
  int i, nang;
  nang = window.num_angles();

  for (i=0; i < nang; i++)
  {
    LineDetectOffset ldo(p, window, nx, ny, i, width, missing);
    _o.push_back(ldo);
  }
}

/*----------------------------------------------------------------*/
LineDetectOffsets::~LineDetectOffsets()
{
}

/*----------------------------------------------------------------*/
const LineDetectOffset *LineDetectOffsets::matchingDirection(double dir) const
{
  for (size_t i=0; i < _o.size(); ++i)
  {
    double a = _o[i].get_angle();
    if (a == dir)
    {
      return &_o[i];
    }
  }
  return NULL;
}

/*----------------------------------------------------------------*/
int LineDetectOffset::_max_int_2(int v0, int v1)
{
  if (v0 < v1)
    return v1;
  else
    return v0;
}

