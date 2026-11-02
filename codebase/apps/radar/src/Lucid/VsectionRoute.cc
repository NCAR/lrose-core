// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
/////////////////////////////////////////////////////////////
// VsectionRoute.cc
//
// User-selected route for an MDV vertical section
///////////////////////////////////////////////////////////////

#include "VsectionRoute.hh"

#include <toolsa/pjg.h>

void VsectionRoute::clear()
{
  _points.clear();
}

void VsectionRoute::append(double x, double y, double lat, double lon)
{
  double cumulativeDistanceKm = 0.0;
  if (!_points.empty()) {
    double segmentLengthKm = 0.0;
    double azimuthDeg = 0.0;
    const Point &previous = _points.back();
    PJGLatLon2RTheta(previous.lat, previous.lon, lat, lon,
                    &segmentLengthKm, &azimuthDeg);
    cumulativeDistanceKm =
      previous.cumulativeDistanceKm + segmentLengthKm;
  }

  Point point = {x, y, lat, lon, cumulativeDistanceKm};
  _points.push_back(point);
}

void VsectionRoute::removeLast()
{
  if (!_points.empty()) {
    _points.pop_back();
  }
}

double VsectionRoute::totalLengthKm() const
{
  return _points.empty() ? 0.0 : _points.back().cumulativeDistanceKm;
}
