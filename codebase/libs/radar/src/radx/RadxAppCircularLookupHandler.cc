/**
 * @file RadxAppCircularLookupHandler.cc
 */
#include <radar/RadxAppCircularLookupHandler.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxVol.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
#include <algorithm>

//------------------------------------------------------------------
RadxAppCircularLookupHandler::RadxAppCircularLookupHandler(double r, const RadxVol &vol)
{
  const vector<RadxRay *> rays = vol.getRays();
  const vector<RadxSweep *> s = vol.getSweeps();

  // need representative delta azimuth and delta range, and max range
  _ngates=0;

  // assume constant
  _deltaGateKm = rays[0]->getGateSpacingKm();
  _deltaAzDeg=0;
  _startRangeKm = rays[0]->getStartRangeKm();
  
  for (size_t i=0; i<s.size(); ++i)
  {
    _setState(vol, *s[i], _ngates, _deltaAzDeg, i==0);
  }

  // use values gotten to build lookup tables
  for (int i=0; i<_ngates; ++i)
  {
    double R = (double)(i)*_deltaGateKm + _startRangeKm;
    if (R <= 10.0)
    {
      _state.push_back(RadxAppCircularLookup(i));
    }
    else
    {
      RadxAppCircularLookup l(r, R, i, _ngates, _startRangeKm, 
		       _deltaGateKm, _deltaAzDeg);
      _state.push_back(l);
    }
  }
}

//------------------------------------------------------------------
void RadxAppCircularLookupHandler::print(void) const
{
  printf("Ngates:%d  deltaR:%.2lf  deltaAz:%.2lf    Number of lookups:%d\n",
	 _ngates, _deltaGateKm, _deltaAzDeg, (int)_state.size());
}


//------------------------------------------------------------------
void RadxAppCircularLookupHandler::printLookups(void) const
{
  print();
  for (size_t i=0; i<_state.size(); ++i)
  {
    _state[i].print();
  }
}

//------------------------------------------------------------------
void RadxAppCircularLookupHandler::_setState(const RadxVol &vol, const RadxSweep &s,
				      int &ngates, double &dangles, bool first)
{
  // figure out delta azimuth
  double da = _deltaAngleDegrees(vol, s, ngates);
  if (first)
  {
    dangles = da;
  }
  else
  {
    if (da != dangles)
    {
      LOG(WARNING) << "Uneven angles in lookup, not yet implemented";
      exit(0);
    }
  }
}

//------------------------------------------------------------------
double RadxAppCircularLookupHandler::_deltaAngleDegrees(const RadxVol &vol,
						 const RadxSweep &s,
						 int &ngates) const
{
  const vector<RadxRay *> rays = vol.getRays();

  vector<double> angles;
  for (int i = s.getStartRayIndex(); 
       i <= static_cast<int>(s.getEndRayIndex()); ++i)
  {
    double a = rays[i]->getAzimuthDeg();
    angles.push_back(a);
    int ng = rays[i]->getNGates();
    if (ng > ngates)
    {
      ngates = ng;
    }    
  }

  vector<double> sangles(angles);
  sort(sangles.begin(), sangles.end());
  double da = 0;
  for (size_t ia=0; ia<sangles.size()-1; ++ia)
  {
    double dai = sangles[ia+1] - sangles[ia];
    if (ia == 0)
    {
      da = dai;
    }
    else
    {
      if (da != dai)
      {
	LOG(WARNING) << "Uneven angle spacing " << dai << " going with " << da;
      }
    }
  }
  return da;
}    

  
//------------------------------------------------------------------
RadxAppCircularLookup::RadxAppCircularLookup(int centerIndex)
{
  _centerIndexR = centerIndex;
  _centerIndexA = 0;
}

//------------------------------------------------------------------
RadxAppCircularLookup::RadxAppCircularLookup(double circleRadius, double centerR,
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
RadxAppCircularLookup::~RadxAppCircularLookup(void)
{

}

//------------------------------------------------------------------
void RadxAppCircularLookup::print(void) const
{
  printf("[%03d,%03d] -  npt=%d\n", _centerIndexR, _centerIndexA,
	 (int)_offsets.size());
}
