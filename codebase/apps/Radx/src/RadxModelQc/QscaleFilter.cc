/**
 * @file QscaleFilter.cc
 */
#include "QscaleFilter.hh"
#include <radar/RadxAppRayLoopData.hh>
#include <Radx/RayxData.hh>
#include <toolsa/LogStream.hh>
#include <vector>
#include <string>

//------------------------------------------
QscaleFilter::QscaleFilter(void) {}

//------------------------------------------
QscaleFilter::~QscaleFilter(void) {}

//------------------------------------------
bool QscaleFilter::filter(const RayxData &data, double scale, double topv,
			  double lowv, bool subtractFromOne, 
			  RadxAppRayLoopData *output)
{
  output->transferData(data);

  // do the filter
  output->qscale(scale, topv, lowv, subtractFromOne);
  return true;
}
