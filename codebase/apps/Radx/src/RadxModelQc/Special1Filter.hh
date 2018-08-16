#ifndef SPECIAL1_FILTER_H
#define SPECIAL1_FILTER_H
#include <string>
#include <vector>

class RayLoopData;
class RadxRay;
class RayxData;

class Special1Filter
{
public:
  inline Special1Filter() {}
  inline ~Special1Filter() {}
  
  bool filter(const std::string &widthName, double meanPrt, double meanNsamples,
	      const RadxRay *_ray,
	      const std::vector<RayLoopData> &_data, RayLoopData *output);
    
private:
};

#endif
