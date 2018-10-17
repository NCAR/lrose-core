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
  
  bool filter(const RayxData &width, double meanPrt, double meanNsamples,
	      RayLoopData *output);
    
private:
};

#endif
