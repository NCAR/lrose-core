/**
 * @file VolumeInfo.cc
 */
#include <FiltAlgVirtVol/VolumeInfo.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
#include <cstdio>

//------------------------------------------------------------------
VolumeInfo::VolumeInfo()
{
  _nx = 0;
  _ny = 0;
  _dx = 0;
  _dy = 0;
  _x0 = _y0 = 0;
  _proj = 0;
  _lat = _lon = 0;
  _hasAltitude = false;
  _altitudeKm = 0;
  _hasWavelength = false;
  _wavelengthCm = 0;
  
}

//------------------------------------------------------------------
VolumeInfo::VolumeInfo(int nx, int ny, double dx, double dy, double x0,
		       double y0, int proj, double lat,
		       double lon, bool hasWavelength, double wavelengthCm,
		       double altitude, const std::vector<double> &vlevels)
{
  _nx = nx;
  _ny = ny;
  _dx = dx;
  _dy = dy;
  _x0 = x0;
  _y0 = y0;
  _proj = proj;
  _lat = lat;
  _lon = lon;
  _hasAltitude = true;
  _altitudeKm = altitude;
  _hasWavelength = hasWavelength;
  _wavelengthCm = wavelengthCm;
  _vlevels = vlevels;
}

//------------------------------------------------------------------
VolumeInfo::~VolumeInfo()
{
}

//------------------------------------------------------------------
bool VolumeInfo::operator==(const VolumeInfo &g) const
{
  if (_hasAltitude != g._hasAltitude)
  {
    return false;
  }
  if (_hasWavelength != g._hasWavelength)
  {
    return false;
  }
  if (_hasAltitude && _altitudeKm != g._altitudeKm)
  {
    return false;
  }
  if (_hasWavelength && _wavelengthCm != g._wavelengthCm)
  {
    return false;
  }
  
  return (_nx == g._nx && _ny == g._ny &&
	  _dx == g._dx && _dy == g._dy &&
	  _x0 == g._x0 && _y0 == g._y0 &&
	  _proj == g._proj &&
	  _lat == g._lat && _lon == g._lon &&
	  _vlevels == g._vlevels);
}

//------------------------------------------------------------------
bool VolumeInfo::operator!=(const VolumeInfo &g) const
{
  if (_hasAltitude != g._hasAltitude)
  {
    return true;
  }
  if (_hasWavelength != g._hasWavelength)
  {
    return true;
  }
  if (_hasAltitude && _altitudeKm != g._altitudeKm)
  {
    return true;
  }
  if (_hasWavelength && _wavelengthCm != g._wavelengthCm)
  {
    return true;
  }
  
  return (_nx != g._nx || _ny != g._ny ||
	  _dx != g._dx || _dy != g._dy ||
	  _x0 != g._x0 || _y0 != g._y0 ||
	  _proj != g._proj ||
	  _lat != g._lat || _lon != g._lon ||
	  _vlevels != g._vlevels);
}

//------------------------------------------------------------------
bool VolumeInfo::equalGrids(const VolumeInfo &g) const
{
  return (_nx == g._nx && _ny == g._ny &&
	  _dx == g._dx && _dy == g._dy &&
	  _x0 == g._x0 && _y0 == g._y0 &&
	  _proj == g._proj &&
	  _lat == g._lat && _lon == g._lon &&
	  //fabs(_lat - g._lat) < 0.02 && fabs(_lon - g._lon) < 0.02 &&
	  _vlevels == g._vlevels);
}

//------------------------------------------------------------------
bool VolumeInfo::equalSizes(const VolumeInfo &g) const
{
  return (_nx == g._nx && _ny == g._ny &&
	  _dx == g._dx && _dy == g._dy &&
	  _vlevels == g._vlevels);
}

//------------------------------------------------------------------
void VolumeInfo::print(void) const
{
  printf("nx,ny=%d,%d dx,dy=%lf,%lf  x0,y0=%lf,%lf proj(lat,long):%lf,%lf\n",
	 _nx, _ny, _dx, _dy, _x0, _y0, _lat, _lon);
  for (size_t i=0; i<_vlevels.size(); ++i)
  {
    printf("Vlevel:%lf\n", _vlevels[i]);
  }
}

//------------------------------------------------------------------
void VolumeInfo::printInfo(void) const
{
  LOG(PRINT) << "\t(nx,ny)=(" <<  _nx << "," << _ny << ")";
  LOG(PRINT) << "\t(dx,dy)=(" <<  _dx << "," << _dy << ")";
  LOG(PRINT) << "\t(x0,y0)=(" <<  _x0 << "," << _y0 << ")";
  LOG(PRINT) << "\t(lat,lon)=(" <<  _lat << "," << _lon << ")";
  char buf[1000];
  std::string v = "";
  for (size_t i=0; i<_vlevels.size(); ++i)
  {
    sprintf(buf, "%.2lf ", _vlevels[i]);
    v += buf;
  }
  LOG(PRINT) << "\tvlevels=" << v;
}

//------------------------------------------------------------------
std::string VolumeInfo::sprint(void) const
{
  char buf[1000];

  sprintf(buf, "nx,ny=%d,%d dx,dy=%lf,%lf  x0,y0=%lf,%lf proj(lat,long):%lf,%lf\nVlevels:",
	 _nx, _ny, _dx, _dy, _x0, _y0, _lat, _lon);
  std::string ret = buf;
  for (size_t i=0; i<_vlevels.size(); ++i)
  {
    sprintf(buf, "%.2lf ", _vlevels[i]);
    ret += buf;
  }
  return ret;
}

//------------------------------------------------------------------
bool VolumeInfo::isCircle(void) const
{
  if (_ny < 2)
  {
    return false;
  }
  double delta;
  double y1 = static_cast<double>(_ny-1)*_dy + _y0;
  delta = fabs(y1 - _y0);
  while (delta >= 360.0)
  {
    delta -= 360.0;
  }
  return (delta == fabs(_dy));
}

//------------------------------------------------------------------
double VolumeInfo::azimuth0to359(int ibeam) const
{
  double az = _y0 + (double)ibeam*_dy;
  while (az > 360)
  {
    az -= 360;
  }
  return az;
}

//------------------------------------------------------------------
double VolumeInfo::verticalLevel(int index) const
{
  if (index < 0 || index >= static_cast<int>(_vlevels.size()))
  {
    LOG(ERROR) << "Index " << index << " out of range";
    return -1.0;
  }
  return _vlevels[index];
}

//------------------------------------------------------------------
double VolumeInfo::setNext(int nextVlevel, bool &isLast, bool &isFirst) const
{
  isLast = (nextVlevel == static_cast<int>(_vlevels.size()) - 1);
  isFirst = (nextVlevel == 0);
  if (nextVlevel >= 0)
  {
    LOG(DEBUG) << "Next data = vlevel[" << nextVlevel << "]=" 
	       << _vlevels[nextVlevel];
  }
  else
  {
    LOG(DEBUG) << "Next data = vlevel[-1] = not set";
  }
  return _vlevels[nextVlevel];
}

//------------------------------------------------------------------
bool VolumeInfo::indexTooBig(int index) const
{
  return index >= static_cast<int>(_vlevels.size());
}

//------------------------------------------------------------------
bool VolumeInfo::isLastVlevel(int index) const
{
  return index+1 == static_cast<int>(_vlevels.size());
}


//------------------------------------------------------------------
double VolumeInfo::setWavelengthCm(double wavelengthDefaultCm,
				   bool overrideWavelength) const
{
  double wavelength = wavelengthDefaultCm;
  if (_hasWavelength && !overrideWavelength)
  {
    wavelength = _wavelengthCm;
  }
  return wavelength;
}

//------------------------------------------------------------------
double VolumeInfo::setHeightKm(double heightDefaultKm,
			       bool overrideHeight) const
{
  double heightKm = heightDefaultKm;
  if (_hasAltitude && !overrideHeight)
  {
    heightKm = _altitudeKm;
  }
  return heightKm;
}

//------------------------------------------------------------------
void VolumeInfo::shiftAzimuthInfo(int n)
{
  if (_proj == 0)
  {
    LOG(ERROR) << "Method won't work";
  }
  _y0 += static_cast<double>(n)*_dy;
}

//------------------------------------------------------------------
void VolumeInfo::synchRadarParams(const VolumeInfo &i)
{
  if (i._hasAltitude && _hasAltitude)
  {
    if (_altitudeKm == 0.0 && i._altitudeKm != 0.0)
    {
      // 0 means it probably is right only for the other one
      _altitudeKm = i._altitudeKm;
    }
  }
  if (i._hasAltitude != _hasAltitude)
  {
    _hasAltitude = true;
    _altitudeKm = i._altitudeKm;
  }

  if (i._hasWavelength != _hasWavelength)
  {
    _hasWavelength = true;
    _wavelengthCm = i._wavelengthCm;
  }
}

