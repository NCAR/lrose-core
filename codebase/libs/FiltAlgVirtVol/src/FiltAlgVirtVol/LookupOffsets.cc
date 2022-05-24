/**
 * @file LookupOffsets.cc
 */
#include <FiltAlgVirtVol/LookupOffsets.hh>
#include <cmath>
#include <cstdio>

//------------------------------------------------------------------
LookupOffsets::LookupOffsets(int xCenterIndex, int nx,
			   double x0Km, double dxKm, double dyDeg,
			     double circleRadiusKm)
{
  // min and max x
  int minXindex = xCenterIndex - (int)(circleRadiusKm/dxKm);
  int maxXindex = xCenterIndex + (int)(circleRadiusKm/dxKm);
  for (int x=minXindex; x<=maxXindex; ++x)
  {
    if (x >= 0 && x < nx)
    {
      _add(x, x0Km, dxKm, dyDeg, circleRadiusKm);
    }
  }
}

//------------------------------------------------------------------
void LookupOffsets::print(int index) const
{
  printf("Lookups[%d]\n", index);
  for (size_t i=0; i< _offsets.size(); ++i)
  {
    printf("%d,%d\n", _offsets[i].first, _offsets[i].second);
  }
}

//------------------------------------------------------------------
void LookupOffsets::_add(int x, double x0Km, double dxKm, double dyDeg,
			 double circleRadiusKm)
{
  double ri = x0Km + dxKm*(double)x;
  double conversion = 180.0/3.14159/ri;
  double minAngle, maxAngle;
  int minAindex, maxAindex;

  minAngle = -1*conversion*circleRadiusKm;
  maxAngle = conversion*circleRadiusKm;
  minAindex = (int)(minAngle/dyDeg);
  maxAindex = (int)(maxAngle/dyDeg);
  if (fabs(maxAngle-minAngle) > 45.0)
  {
    // do not update offsets, too close to radar
    return;
  }
  for (int a=minAindex; a<maxAindex; ++a)
  {
    _offsets.push_back(std::pair<int,int>(x, a));
  }
}


