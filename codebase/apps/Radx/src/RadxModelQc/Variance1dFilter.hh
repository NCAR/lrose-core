#ifndef Variance1d_FILTER_H
#define Variance1d_FILTER_H

#include <string>
#include <vector>

class RayLoopData;
class RadxRay;
class RayxData;


class Variance1dFilter
{
public:
  inline Variance1dFilter(double npt, double maxPctMissing) :
    _npt(npt), _maxPctMissing(maxPctMissing) {}
  inline ~Variance1dFilter() {}
  
  bool filter(const RayxData &data, RayLoopData *output);
    
private:
  double _npt;
  double _maxPctMissing;

};

#endif
