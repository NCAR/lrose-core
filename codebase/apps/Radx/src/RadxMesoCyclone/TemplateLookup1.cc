#include "TemplateLookup1.hh"
#include <cmath>

//------------------------------------------------------------------
TemplateLookup1::TemplateLookup1(void)
{
}

//------------------------------------------------------------------
TemplateLookup1::TemplateLookup1(double x, double y, double yOff,
				 int xCenterIndex, int ngates,
				 double startRangeKm, double deltaGateKm,
				 double deltaAzDeg)
{   
  // min and max radius will be on the 0
  int minRindex = xCenterIndex - (int)(x/2.0/deltaGateKm);
  int maxRindex = xCenterIndex + (int)(x/2.0/deltaGateKm);

  for (int r=minRindex; r<=maxRindex; ++r)
  {
    if (r >= 0 && r < ngates)
    {
      _addToOffsets(r, ngates, startRangeKm, deltaGateKm, deltaAzDeg,
		    yOff, y, false, _offsets1);
    }
  }

  for (int r=minRindex; r<=maxRindex; ++r)
  {
    if (r >= 0 && r < ngates)
    {
      _addToOffsets(r, ngates, startRangeKm, deltaGateKm, deltaAzDeg, yOff, y,
		    true, _offsets2);
    }
  }
}

//------------------------------------------------------------------
void TemplateLookup1::print(void) const
{
  printf("%d,%d points\n", (int)(_offsets1.size()), (int)(_offsets2.size()));
}

//------------------------------------------------------------------
std::string TemplateLookup1::sprint(void) const
{
  char buf[1000];
  sprintf(buf, "%d,%d points",
	  (int)(_offsets1.size()), (int)(_offsets2.size()));
  std::string s= buf;
  return s;
}

//------------------------------------------------------------------
void TemplateLookup1::_addToOffsets(int r, int ngates, double startRangeKm,
				    double deltaGateKm, double deltaAzDeg,
				    double yOff, double y, bool negative,
				    std::vector<std::pair<int,int> > &offsets)
{
  double ri = startRangeKm + deltaGateKm*(double)r;

  double conversion = 180.0/3.14159/ri;

  double minAngle;
  double maxAngle;
  int minAindex;
  int maxAindex;
  if (negative)
  {    
    minAngle = -1*conversion*(yOff+y);
    maxAngle = -1*conversion*(yOff);
    minAindex = (int)(minAngle/deltaAzDeg);
    maxAindex = (int)(maxAngle/deltaAzDeg);
  }
  else
  {
    minAngle = conversion*yOff;
    maxAngle = conversion*(yOff+y);
    minAindex = (int)(minAngle/deltaAzDeg);
    maxAindex = (int)(maxAngle/deltaAzDeg);
  }
  if (fabs(maxAngle-minAngle) > 45.0)
  {
    // do not update offsets, too close to radar
    return;
  }
  
  for (int a=minAindex; a<maxAindex; ++a)
  {
    offsets.push_back(std::pair<int,int>(r, a));
  }
}

