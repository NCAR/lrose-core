/**
 * @file GriddedData.cc
 */

//------------------------------------------------------------------
#include <FiltAlgVirtVol/GriddedData.hh>


//------------------------------------------------------------------
GriddedData::GriddedData(void)
{
}

//------------------------------------------------------------------
GriddedData::GriddedData(const Grid2d &g) : Grid2d(g)
{
}

//------------------------------------------------------------------
GriddedData::~GriddedData(void)
{
}

//------------------------------------------------------------------
int GriddedData::numData(void) const
{
  return Grid2d::getNdata();
}

//------------------------------------------------------------------
bool GriddedData::nameMatch(const std::string &n) const
{
  return Grid2d::nameEquals(n);
}

//------------------------------------------------------------------
bool GriddedData::getVal(int ipt, double &v) const
{
  return Grid2d::getValue(ipt, v);
}

//------------------------------------------------------------------
void GriddedData::setVal(int ipt, double v)
{
  Grid2d::setValue(ipt, v);
}

//------------------------------------------------------------------
void GriddedData::setMissing(int ipt)
{
  Grid2d::setMissing(ipt);
}

//------------------------------------------------------------------
double GriddedData::getMissingValue(void) const
{
  return Grid2d::getMissing();
}


//------------------------------------------------------------------
MathLoopData *GriddedData::clone(void) const
{
  GriddedData *newg = new GriddedData(*this);
  return (MathLoopData *)newg;
}
  
//------------------------------------------------------------------
void GriddedData::print(void) const
{
  Grid2d::print();
}

//------------------------------------------------------------------
void GriddedData::setAllToValue(double v)
{
  Grid2d::setAllToValue(v);
}

