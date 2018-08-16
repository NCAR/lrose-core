/**
 * @file FiltClutter2dQual.cc
 */
#include "Clutter2dQualFilter.hh"
#include "RayData.hh"
#include <Radx/RayxData.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
bool Clutter2dQualFilter::filter(const std::string &scrName,
				 const std::string &velName,
				 const std::string &widthName,
				 double scale, double vr_shape,
				 double sw_shape, const RadxRay *ray,
				 const std::vector<RayLoopData> &data,
				 RayLoopData *output)
{
  RayxData fscr, vel, width;
  if (!RayData::retrieveRay(scrName, *ray, data, fscr))
  {
    return false;
  }
  if (!RayData::retrieveRay(velName, *ray, data, vel))
  {
    return false;
  }
  if (!RayData::retrieveRay(widthName, *ray, data, width))
  {
    return false;
  }

  // copy contents of vel into output
  RayLoopData *rl = (RayLoopData *)output;
  rl->transferData(vel);
  rl->abs();
  rl->multiply(vr_shape);
  
  // copy width into a new variable
  RayxData arg2(width);
  arg2.multiply(sw_shape);
  
  rl->inc(arg2, false);
  rl->qscale1(-scale, true);
  return true;
}
