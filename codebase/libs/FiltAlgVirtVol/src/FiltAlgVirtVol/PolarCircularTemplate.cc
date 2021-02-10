#include <FiltAlgVirtVol/PolarCircularTemplate.hh>
#include <Mdv/MdvxProj.hh>
#include <euclid/Grid2d.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
#include <algorithm>

//------------------------------------------------------------------
PolarCircularTemplate::PolarCircularTemplate(double radiusKm,
					     const MdvxProj &proj,
					     double minRadiusKm) :
  _radiusKm(radiusKm)
{
  Mdvx::coord_t coord = proj.getCoord();

  _ngates=coord.nx;
  _deltaGateKm = coord.dx;
  _deltaAzDeg= coord.dy;
  _startRangeKm = coord.minx;
  _circular = coord.ny*coord.dy >= 358;
  
  // use values gotten to build lookup tables
  for (int i=0; i<_ngates; ++i)
  {
    double R = (double)(i)*_deltaGateKm + _startRangeKm;
    //if (R <= 2.0)
    if (R <= minRadiusKm)
    {
      _lookup.push_back(LookupOffsets());
    }
    else
    {
      LookupOffsets l(i, _ngates, _startRangeKm, _deltaGateKm,
		      _deltaAzDeg, radiusKm);
      _lookup.push_back(l);
      LOG(DEBUG_VERBOSE) << "Lookup[" << i << "] R=" << R << " npt=" << l.num();
    }
  }
}

//------------------------------------------------------------------
void PolarCircularTemplate::print(void) const
{
  printf("Ngates:%d  deltaR:%.2lf  deltaAz:%.2lf    Number of lookups:%d\n",
	 _ngates, _deltaGateKm, _deltaAzDeg, (int)_lookup.size());
}


//------------------------------------------------------------------
void PolarCircularTemplate::printLookups(void) const
{
  print();
  for (size_t i=0; i<_lookup.size(); ++i)
  {
    _lookup[i].print(i);
  }
}

//-------------------------------------------------------
std::vector<double>
PolarCircularTemplate::dataInsideCircle(int x, int y,
					const Grid2d &data) const
{
  std::vector<double> ret;
  
  if (x < 0 || x > (int)_lookup.size())
  {
    return ret;
  }
  return _dataInsideCircle(_lookup[x], y, data);
}

//-------------------------------------------------------
std::vector<double>
PolarCircularTemplate::_dataInsideCircle(const LookupOffsets &lx, int y,
					 const Grid2d &data) const
{
  std::vector<double> ret;
  for (int j=0; j< lx.num(); ++j)
  {
    int rj = lx.ithIndexR(j);
    int aj = lx.ithIndexA(j);
    if (rj >= 0 && rj < data.getNx())
    {
      int ny = data.getNy();
      int ia = y + aj;
      while (ia < 0)
      {
	if (_circular)
	{
	  ia += ny;
	}
	else
	{
	  continue;
	}
      }
      while (ia >= ny)
      {
	if (_circular)
	{
	  ia -= ny;
	}
	else
	{
	  continue;
	}
      }
      double v;
      if (data.getValue(rj, ia, v))
      {
	ret.push_back(v);
      }
    }
  }
  return ret;
}

//-------------------------------------------------------
bool PolarCircularTemplate::getFloat(double &v) const
{
  v = 0;
  return false;
}
