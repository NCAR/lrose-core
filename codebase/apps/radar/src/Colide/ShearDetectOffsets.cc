/**
 * offset indices into images, used as a lookup table.
 */

#include <stdio.h>
#include "ShearDetectOffsets.hh"
#include "Window.hh"

/*----------------------------------------------------------------*/
ShearDetectOffset::ShearDetectOffset(const Grid2dOffset &left,
				     const Grid2dOffset &right,
				     double a)
{
  __sider = right;
  __sidel = left;
  __angle = a;
}

/*----------------------------------------------------------------*/
/* setup the two lines */
/*
 * Dave Albo:
 * Contruct offset tables for the templates on either side of the point:
 *    sider  = right side of point
 *    sidel  = left side of point.
 *    angle 0 is vertical, and then all the other rotations are rotated.
 */
ShearDetectOffset::ShearDetectOffset(const Parms &p, const Window &window,
				     int nx, int ny,
				     int i, int width, double missing)
{
    double a, half_wid, step, len;
    // int npts, nang;
    
    len = nx;
    // npts = p._window.num_points();
    // nang = p._window.num_angles();
    step = window.step_size();
    a = window.ith_angle(i);
    half_wid = .5*ny;
    
    __sider = Grid2dOffset(len, half_wid, a, step, width, missing);
    __sidel = Grid2dOffset(len, -half_wid, a, step, width, missing);
    __angle = a;
}

/*----------------------------------------------------------------*/
ShearDetectOffset::~ShearDetectOffset()
{
}

/*----------------------------------------------------------------*/
int ShearDetectOffset::_max_int_2(int v0, int v1)
{
  if (v0 < v1)
    return v1;
  else
    return v0;
}

/*----------------------------------------------------------------*/
ShearDetectOffsets::ShearDetectOffsets(const Parms &p, const Window &window,
				       int nx, int ny, int width,
				       double missing)
{
  int nang, i;
    
  nang = window.num_angles();
  for (i = 0; i < nang; i++)
  {
    ShearDetectOffset sdo(p, window, nx, ny, i, width, missing);
    __o.push_back(sdo);
  }
}

/*----------------------------------------------------------------*/
ShearDetectOffsets::~ShearDetectOffsets()
{
}

/*----------------------------------------------------------------*/
const ShearDetectOffset *ShearDetectOffsets::matchingDirection(double dir) const
{
  for (size_t i=0; i < __o.size(); ++i)
  {
    double a = __o[i].get_angle();
    if (a == dir)
    {
      return &__o[i];
    }
  }
  return NULL;
}

