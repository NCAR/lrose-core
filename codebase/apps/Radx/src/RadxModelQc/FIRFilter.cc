/**
 * @file FiltFIR.cc
 */
#include "FIRFilter.hh"
#include "RayData.hh"
#include <Radx/RayxData.hh>
#include <toolsa/LogStream.hh>
#include <cmath>

//------------------------------------------------------------------
FIRFilter::FIRFilter()
{
  _coeff = {
  0.0230,
  -0.0358,
  -0.0014,
  0.0288,
  -0.0146,
  -0.0418,
  0.0356,
  0.0481,
  -0.0889,
  -0.0550,
  0.3117,
  0.5561,
  0.3117,
  -0.0550,
  -0.0889,
  0.0481,
  0.0356,
  -0.0418,
  -0.0146,
  0.0288,
  -0.0014,
  -0.0358,
  0.0230
  };
}

//------------------------------------------------------------------
FIRFilter::~FIRFilter()
{
}

//------------------------------------------------------------------
bool FIRFilter::filter(const std::string &name, const std::string &firType,
		       const RadxRay *ray, const std::vector<RayLoopData> &data,
		       RayLoopData *output)
{
  RayxData locdata;
  if (!RayData::retrieveRay(name, *ray, data, locdata))
  {
    return false;
  }

  RayxData::FirFilter_t _edge;
  if (firType == "USE_FIRST_DATA")
    _edge = RayxData::FIR_EDGE_CLOSEST;
  else if (firType == "MIRROR")
    _edge = RayxData::FIR_EDGE_MIRROR;
  else if (firType == "MEAN")
    _edge = RayxData::FIR_EDGE_MEAN;
  else if (firType == "INTERP")
    _edge = RayxData::FIR_EDGE_INTERP;
 else
 {
   LOG(ERROR) << "FIR edge type not known " << firType;
   return false;
 }    
  

  // copy contents of vel into output
  RayLoopData *rl = (RayLoopData *)output;
  rl->transferData(locdata);

  // make another object for quality (not used for anything so far)
  RayxData quality = *rl;

  rl->FIRfilter(_coeff, _edge, quality);
  return true;

}
