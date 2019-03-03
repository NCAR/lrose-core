/**
 * @file FiltClutter2dQual.cc
 */
#include "Clutter2dQualFilter.hh"
#include <radar/RadxAppRayLoopData.hh>
#include <Radx/RayxData.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
bool Clutter2dQualFilter::
filter(const RayxData &fscr, const RayxData &vel, const RayxData &width,
       double scale, double vr_shape, double sw_shape,
       RadxAppRayLoopData *output)
{

  // copy contents of vel into output
  output->transferData(vel);
  output->abs();
  output->multiply(vr_shape);
  
  // copy width into a new variable
  RayxData arg2(width);
  arg2.multiply(sw_shape);
  
  output->inc(arg2, false);
  output->qscale1(-scale, true);
  return true;
}
