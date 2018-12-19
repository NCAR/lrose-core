/**
 * @file ClumpRegions.cc
 */

#include "ClumpRegions.hh"

//------------------------------------------------------------------------
ClumpRegions::ClumpRegions(const Grid2d &g)
{
  Grid2dClump clump(g);
  int nx = g.getNx();
  int ny = g.getNy();

  std::vector<clump::Region_t> r = clump.buildRegions();
  for (size_t i=0; i<r.size(); ++i)
  {
    _regions.push_back(PointList(nx, ny, r[i]));//ClumpRegion(r[i]));
  }
}

//------------------------------------------------------------------------
ClumpRegions::~ClumpRegions(void)
{
}
   
//------------------------------------------------------------------------
void ClumpRegions::setValues(int index, double color, Grid2d &out) const
{
  _regions[index].toGrid(out, color);//setToValue(color, out);

  // for (size_t j=0; j<_regions[index].size(); ++j)
  // {
  //   out.setValue(_regions[index].x(j), _regions[index].y(j), color);
  // }
}

//------------------------------------------------------------------------
void ClumpRegions::removeSmallClump(int index, int minPt, Grid2d &out) const
{
  if (_regions[index].size() < minPt)
  {
    double mv = out.getMissing();
    _regions[index].toGrid(out, mv);
    // for (size_t j=0; j<_regions[index].size(); ++j)
    // {
    //   out.setMissing(_regions[index].x(j), _regions[index].y(j));
    // }
  }
}

//------------------------------------------------------------------------
bool ClumpRegions::getFloat(double &f) const
{
  return false;
}
