#include <FiltAlgVirtVol/VolumeTime.hh>

//------------------------------------------------------------------
VolumeTime::VolumeTime(const time_t &t) : _time(t)
{
}

//-------------------------------------------------------
bool VolumeTime::getFloat(double &v) const
{
  v = 0;
  return false;
}

