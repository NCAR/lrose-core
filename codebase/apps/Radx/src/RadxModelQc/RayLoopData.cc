#include <RayLoopData.hh>

RayLoopData::RayLoopData(const RayxData &r) : RayxData(r)
{
}

RayLoopData::~RayLoopData(void) {}

int RayLoopData::numData(void) const
{
  return RayxData::getNpoints();
}

bool RayLoopData::nameMatch(const std::string &n) const
{
  return RayxData::namesMatch(n);
}

bool RayLoopData::getVal(int ipt, double &v) const
{
  return RayxData::getV(ipt, v);
}

void RayLoopData::setVal(int ipt, double v)
{
  RayxData::setV(ipt, v);
}

void RayLoopData::setMissing(int ipt)
{
  RayxData::setV(ipt, RayxData::getMissing());
}

double RayLoopData::getMissingValue(void) const
{
  return RayxData::getMissing();
}

MathLoopData *RayLoopData::clone(void) const
{
  RayLoopData *ret = new RayLoopData(*this);
  return (MathLoopData *)ret;
}

void RayLoopData::setAllToValue(double v)
{
  for (int i=0; i<getNpoints(); ++i)
  {
    setVal(i, v);
  }
}

void RayLoopData::print(void) const
{
  printf("Ray for %s\n", RayxData::getName().c_str());
}

