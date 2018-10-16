/**
 * @file FiltFIR.cc
 */
#include "FIRFilter.hh"
#include <euclid/GridAlgs.hh>
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
bool FIRFilter::filter(const Grid2d &inp, GridAlgs &out) const
{
  out.dataCopy(inp);
  out.FIRfilter(_coeff);
  return true;
}
