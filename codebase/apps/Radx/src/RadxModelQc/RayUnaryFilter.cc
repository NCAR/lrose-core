#include "RayUnaryFilter.hh"
#include "RayData.hh"
#include <Radx/RadxRay.hh>
#include <Radx/RayxData.hh>
#include <Radx/RadxFuzzyF.hh>
#include <radar/RadxApp.hh>
#include <toolsa/LogStream.hh>
#include <vector>
#include <string>

//----------------------------------------------------------------------
RayUnaryFilter::RayUnaryFilter(void) {}

//----------------------------------------------------------------------
RayUnaryFilter::~RayUnaryFilter(void) {}

//----------------------------------------------------------------------
bool RayUnaryFilter::smooth(const std::string &name, int nx, int ny,
			    const RadxRay *ray,
			    const std::vector<RayLoopData> &_data,
			    MathData *output)
{
  // // get the field from the center ray
  // RayxData r;
  // if (!RayData::retrieveRay(name, *ray, _data, r))
  // {
    return false;
  // }

  // // copy contents of named field into output
  // RayLoopData *rl = (RayLoopData *)output;
  // rl->transferData(r);

  // rl->smooth(nx, ny);
  // return true;
}

//----------------------------------------------------------------------
bool RayUnaryFilter::fuzzy(const std::string &name,
			   const RadxFuzzyF &f,
			   const RadxRay *ray,
			   const std::vector<RayLoopData> &data,
			   MathData *output)
{
  // get the field from the center ray
  RayxData r;
  if (!RayData::retrieveRay(name, *ray, data, r))
  {
    return false;
  }

  // copy contents of named field into output
  RayLoopData *rl = (RayLoopData *)output;
  rl->fuzzyRemap(f);
  return true;
}
  
