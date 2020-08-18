/**
 * @file EnhancementOffsetD.cc
 */

#include "EnhancementOffsetD.hh"

/*----------------------------------------------------------------*/
EnhancementOffsetD::EnhancementOffsetD(int i,
				       const Window &w,
				       int width, int missing)
{
  double a, half_wid, step, length;
  // int npix;
    
  a = w.ith_angle(i);
  length = w._length;
  half_wid = w._width/2.0;
  step = w.step_size();
  // npix = w.num_points();
    
  _center = Grid2dOffset(length, 0.0, a, step, width, missing);
  _left = Grid2dOffset(length, -half_wid, a, step*2.0, width, missing);
  _right = Grid2dOffset(length, half_wid, a, step*2.0, width, missing);
  _angle = a;
}

/*----------------------------------------------------------------*/
EnhancementOffsetD::~EnhancementOffsetD()
{
}

/*----------------------------------------------------------------*/
EnhancementOffsetsD::EnhancementOffsetsD(const Window &w,
					       int xwidth, int missing)
{
  int i, n_angs;
    
  n_angs = w.num_angles();
  for (i=0; i< n_angs; i++)
  {
    /*
     * Build templates with center points and half as many points
     * on each side of center
     */
    EnhancementOffsetD d(i, w, xwidth, missing);
    _o.push_back(d);
  }
}

/*----------------------------------------------------------------*/
EnhancementOffsetsD::~EnhancementOffsetsD()
{
}

