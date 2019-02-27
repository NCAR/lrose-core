/**
 * @file RadxAppRayLoopData.cc
 */
#include <radar/RadxAppRayLoopData.hh>
#include <cstdio>

//-----------------------------------------------------------
RadxAppRayLoopData::RadxAppRayLoopData(void) : RayxData()
{
}

//-----------------------------------------------------------
RadxAppRayLoopData::RadxAppRayLoopData(const RayxData &r) : RayxData(r)
{
}

//-----------------------------------------------------------
RadxAppRayLoopData::~RadxAppRayLoopData(void)
{
}

//-----------------------------------------------------------
int RadxAppRayLoopData::numData(void) const
{
  return RayxData::getNpoints();
}

//-----------------------------------------------------------
bool RadxAppRayLoopData::nameMatch(const std::string &n) const
{
  return RayxData::namesMatch(n);
}

//-----------------------------------------------------------
bool RadxAppRayLoopData::getVal(int ipt, double &v) const
{
  return RayxData::getV(ipt, v);
}

//-----------------------------------------------------------
void RadxAppRayLoopData::setVal(int ipt, double v)
{
  RayxData::setV(ipt, v);
}

//-----------------------------------------------------------
void RadxAppRayLoopData::setMissing(int ipt)
{
  RayxData::setV(ipt, RayxData::getMissing());
}

//-----------------------------------------------------------
double RadxAppRayLoopData::getMissingValue(void) const
{
  return RayxData::getMissing();
}

//-----------------------------------------------------------
MathLoopData *RadxAppRayLoopData::clone(void) const
{
  RadxAppRayLoopData *ret = new RadxAppRayLoopData(*this);
  return (MathLoopData *)ret;
}

//-----------------------------------------------------------
void RadxAppRayLoopData::setAllToValue(double v)
{
  for (int i=0; i<getNpoints(); ++i)
  {
    setVal(i, v);
  }
}

//-----------------------------------------------------------
void RadxAppRayLoopData::print(void) const
{
  printf("Ray for %s\n", RayxData::getName().c_str());
}

