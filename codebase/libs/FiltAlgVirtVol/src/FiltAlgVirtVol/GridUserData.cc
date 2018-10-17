/**
 * @file GridUserData.hh
 */
#include <FiltAlgVirtVol/GridUserData.hh>

GridUserData::GridUserData(void) :
  MathUserData(), Grid2d()
{
}


GridUserData::GridUserData(const Grid2d &grid, const std::string &name) :
  MathUserData(), Grid2d(grid)
{
  Grid2d::setName(name);
}

GridUserData::~GridUserData(void)
{
}

bool GridUserData::getFloat(double &v) const
{
  return 0.0;
}
