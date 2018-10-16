/**
 * @file ClumpRegion.cc
 */

#include "ClumpRegion.hh"

//------------------------------------------------------------------------
void ClumpRegion::setMissing(Grid2d &out) const
{
  for (size_t i=0; i<_pt.size(); ++i)
  {
    out.setMissing(_pt[i].first, _pt[i].second);
  }
}

//------------------------------------------------------------------------
void ClumpRegion::setToValue(double value, Grid2d &out) const
{
  for (size_t i=0; i<_pt.size(); ++i)
  {
    out.setValue(_pt[i].first, _pt[i].second, value);
  }
}
  

