#ifndef SPECIAL0_FILTER_H
#define SPECIAL0_FILTER_H
#include <string>
#include <vector>

class RayLoopData;
class RadxRay;
class RayxData;


class Special0Filter
{
public:
  inline Special0Filter() {}
  inline ~Special0Filter() {}
  
  bool filter(const RayxData &width, double meanPrt, double meanNsamples,
	      RayLoopData *output);
    
private:
};

#endif
