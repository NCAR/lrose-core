/**
 * @file CircularLookupHandler.cc
 */
#include "CircularLookupHandler.hh"
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
#include <algorithm>

//------------------------------------------------------------------
CircularLookupHandler::CircularLookupHandler(double r, const RadxVol &vol)
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
      _state.push_back(CircularLookup(i));
    }
    else
    {
      CircularLookup l(r, R, i, _ngates, _startRangeKm, 
		       _deltaGateKm, _deltaAzDeg);
      _state.push_back(l);
    }
  }
}

//------------------------------------------------------------------
void CircularLookupHandler::print(void) const
{
  printf("Ngates:%d  deltaR:%.2lf  deltaAz:%.2lf    Number of lookups:%d\n",
	 _ngates, _deltaGateKm, _deltaAzDeg, (int)_state.size());
}


//------------------------------------------------------------------
void CircularLookupHandler::printLookups(void) const
{
  print();
  for (size_t i=0; i<_state.size(); ++i)
  {
    _state[i].print();
  }
}

//------------------------------------------------------------------
void CircularLookupHandler::_setState(const RadxVol &vol, const RadxSweep &s,
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
double CircularLookupHandler::_deltaAngleDegrees(const RadxVol &vol,
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

  
