/**
 * @file QscaleFilter.cc
 */
#include "QscaleFilter.hh"
#include "RayData.hh"
#include <Radx/RayxData.hh>
#include <toolsa/LogStream.hh>
#include <vector>
#include <string>

//------------------------------------------
QscaleFilter::QscaleFilter(void) {}

//------------------------------------------
QscaleFilter::~QscaleFilter(void) {}

//------------------------------------------
bool QscaleFilter::filter(const std::string &name, double scale, double topv,
			  double lowv, bool subtractFromOne, const RadxRay *ray,
			  const std::vector<RayLoopData> &data,
			  RayLoopData *output)
{
  RayxData r;
  if (!RayData::retrieveRay(name, *ray, data, r))
  {
    return false;
  }

  // copy contents of r into output
  RayLoopData *rl = (RayLoopData *)output;
  rl->transferData(r);

  // do the filter
  rl->qscale(scale, topv, lowv, subtractFromOne);
  return true;
}
