#include "Volume.hh"
#include "TemplateLookupMgr.hh"
// #include <Radx/RadxSweep.hh>
// #include <Radx/RadxRay.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
#include <algorithm>

//------------------------------------------------------------------
TemplateLookupMgr::TemplateLookupMgr(double x, double y, double yoff,
				     const Volume &v) :
  _x(x), _y(y), _yoff(yoff)
{
  // need representative delta azimuth and delta range, and max range
  _ngates=v.getNGates();
  _deltaGateKm = v.getDeltaGateKm();
  _deltaAzDeg=v.getDeltaAzDeg();
  _startRangeKm = v.getStartRangeKm();
  
  // use values gotten to build lookup tables
  for (int i=0; i<_ngates; ++i)
  {
    double R = (double)(i)*_deltaGateKm + _startRangeKm;
    if (R <= 2.0)
    {
      _state.push_back(TemplateLookup(i));
    }
    else
    {
      TemplateLookup l(x, y, yoff, R, i, _ngates, _startRangeKm, 
		       _deltaGateKm, _deltaAzDeg);
      _state.push_back(l);
    }
  }
}

//------------------------------------------------------------------
void TemplateLookupMgr::print(void) const
{
  printf("Ngates:%d  deltaR:%.2lf  deltaAz:%.2lf    Number of lookups:%d\n",
	 _ngates, _deltaGateKm, _deltaAzDeg, (int)_state.size());
}


//------------------------------------------------------------------
void TemplateLookupMgr::printLookups(void) const
{
  print();
  for (size_t i=0; i<_state.size(); ++i)
  {
    _state[i].print();
  }
}

//-------------------------------------------------------
bool TemplateLookupMgr::getFloat(double &v) const
{
  v = 0;
  return false;
}
