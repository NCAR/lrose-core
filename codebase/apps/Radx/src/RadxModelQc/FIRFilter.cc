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
  // initialize coefficents
  _coeff.push_back(0.0230);
  _coeff.push_back(-0.0358);
  _coeff.push_back(-0.0014);
  _coeff.push_back(0.0288);
  _coeff.push_back(-0.0146);
  _coeff.push_back(-0.0418);
  _coeff.push_back(0.0356);
  _coeff.push_back(0.0481);
  _coeff.push_back(-0.0889);
  _coeff.push_back(-0.0550);
  _coeff.push_back(0.3117);
  _coeff.push_back(0.5561);
  _coeff.push_back(0.3117);
  _coeff.push_back(-0.0550);
  _coeff.push_back(-0.0889);
  _coeff.push_back(0.0481);
  _coeff.push_back(0.0356);
  _coeff.push_back(-0.0418);
  _coeff.push_back(-0.0146);
  _coeff.push_back(0.0288);
  _coeff.push_back(-0.0014);
  _coeff.push_back(-0.0358);
  _coeff.push_back(0.0230);
}

//------------------------------------------------------------------
FIRFilter::~FIRFilter()
{
}

//------------------------------------------------------------------
bool FIRFilter::filter(const RayxData &data, const std::string &firType,
		       RayLoopData *output)
{
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
  output->transferData(data);

  // make another object for quality (not used for anything so far)
  RayxData quality = *output;

  output->FIRfilter(_coeff, _edge, quality);
  return true;

}
