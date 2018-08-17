/**
 * @file CircularLookup.cc
 */

#include "CircularLookup.hh"
#include <cmath>

//------------------------------------------------------------------
CircularLookup::CircularLookup(int centerIndex)
{
  _centerIndexR = centerIndex;
  _centerIndexA = 0;
}

//------------------------------------------------------------------
CircularLookup::CircularLookup(double circleRadius, double centerR,
			       int centerIndex, int ngates, double startRangeKm,
			       double deltaGateKm, double deltaAzDeg)
{
  _centerIndexR = centerIndex;
  _centerIndexA = 0;

  // min and max radius will be on the 0
  int minRindex = centerIndex - (int)(circleRadius/deltaGateKm);
  int maxRindex = centerIndex + (int)(circleRadius/deltaGateKm);

  double x0 = centerR;
  double y0 = 0;
  double circleRadiusSq = circleRadius*circleRadius;

  int maxAindex = (int)(90.0/deltaAzDeg);  // at most 90 degree offset

  for (int r=minRindex; r<=maxRindex; ++r)
  {
    if (r > 0 && r < ngates)
    {
      _offsets.push_back(std::pair<int,int>(r, 0));
      double ri = startRangeKm + deltaGateKm*(double)r;

      // now keep  trying azimuths offset until nothing
      for (int a=1; a<maxAindex; ++a)
      {
	double theta = deltaAzDeg*3.14159/180.0*(double)a;
	double y = ri*sin(theta);
	double x = ri*cos(theta);
	double d2 = (x-x0)*(x-x0) + (y-y0)*(y-y0);
	if (d2 <= circleRadiusSq)
	{
	  _offsets.push_back(std::pair<int,int>(r, a));
	  if (a != 0)
	  {
	    _offsets.push_back(std::pair<int,int>(r, -a));
	  }
	}
      }
    }
  }
}

//------------------------------------------------------------------
CircularLookup::~CircularLookup(void)
{

}

//------------------------------------------------------------------
void CircularLookup::print(void) const
{
  printf("[%03d,%03d] -  npt=%d\n", _centerIndexR, _centerIndexA,
	 (int)_offsets.size());
}
