// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
/////////////////////////////////////////////////////////////
// VsectionRoute.hh
//
// User-selected route for an MDV vertical section
///////////////////////////////////////////////////////////////

#ifndef VsectionRoute_HH
#define VsectionRoute_HH

#include <vector>

#include <QMetaType>

class VsectionRoute {

public:

  struct Point {
    double x;
    double y;
    double lat;
    double lon;
    double cumulativeDistanceKm;
  };

  void clear();
  void append(double x, double y, double lat, double lon);
  void removeLast();

  bool empty() const { return _points.empty(); }
  size_t size() const { return _points.size(); }
  double totalLengthKm() const;

  const Point &back() const { return _points.back(); }
  const std::vector<Point> &points() const { return _points; }

private:

  std::vector<Point> _points;

};

Q_DECLARE_METATYPE(VsectionRoute)

#endif
