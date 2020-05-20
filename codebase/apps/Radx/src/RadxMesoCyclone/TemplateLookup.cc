/**
 * @file TemplateLookup.cc
 */

#include "TemplateLookup.hh"
#include <cmath>

//------------------------------------------------------------------
TemplateLookup::TemplateLookup(int centerIndex)
{
  _centerIndex = centerIndex;
  _centerIndexA = 0;
}

//------------------------------------------------------------------
TemplateLookup::TemplateLookup(double x, double y, double yOff, 
			       double centerR, int centerIndex,
			       int ngates, double startRangeKm,
			       double deltaGateKm, double deltaAzDeg)
{
  _centerIndex = centerIndex;
  _centerIndexA = 0;

  // we construct two options for xOff, one each way
  _points = TemplateLookup1(_centerIndex, x, ngates,
			     startRangeKm, deltaGateKm, deltaAzDeg, yOff, y);
}

//------------------------------------------------------------------
TemplateLookup::~TemplateLookup(void)
{

}

//------------------------------------------------------------------
void TemplateLookup::print(void) const
{
  printf("[(%03d,%03d] -  %s\n",
	 _centerIndex,
	 _centerIndexA, _points.sprint().c_str());
}
