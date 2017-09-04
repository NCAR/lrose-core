#include <Refract/RefDebug.hh>
#include <Mdv/MdvxPjg.hh>
#include <algorithm>

RefDebug::RefDebug()
{
  _lat = -1.0;
  _lon = -1.0;
  _npt = 0;
  _debugX = _debugY = -1.0;
  _debugIpt.clear();
}

RefDebug::RefDebug(double lat, double lon, int npt)
{
  _lat = lat;
  _lon = lon;
  _npt = npt;
  _debugX = _debugY = -1.0;
  _debugIpt.clear();
}

RefDebug::~RefDebug(void)
{
}

void RefDebug::setDebug(const MdvxPjg &proj)
{
  _debugX = _debugY = -1.0;
  _debugIpt.clear();
  if (_lat == -1 || _lon == -1)
  {
    return;
  }
  proj.latlon2xyIndex(_lat, _lon, _debugX, _debugY);
  for (int y=_debugY-_npt; y<=_debugY+_npt; ++y)
  {
    if (y >= 0 && y<proj.getNy())
    {
      for (int x=_debugX-_npt; x<=_debugX+_npt; ++x)
      {
	if (x >= 0 && x<proj.getNx())
	{
	  _debugIpt.push_back(proj.getNx() + x);
	}
      }
    }
  }
}

bool RefDebug::isDebugPt(int i) const
{
  if (_debugIpt.empty())
  {
    return false;
  }
  else
  {
    return find(_debugIpt.begin(), _debugIpt.end(), i) != _debugIpt.end();
  }
}
