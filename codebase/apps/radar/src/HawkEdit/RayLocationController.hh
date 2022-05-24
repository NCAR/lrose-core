#ifndef RayLocationController_HH
#define RayLocationController_HH


#include <vector>


#include "RayLocationModel.hh"


class RayLocationController {
	
public:

RayLocationController();
~RayLocationController();

void sortRaysIntoRayLocations(float ppi_rendering_beam_width,
	int sweepNumber);

size_t getNRayLocations();
double getStartRangeKm(size_t rayIdx);
double getGateSpacingKm(size_t rayIdx);
double getMaxRangeKm();
size_t getEndIndex(size_t rayIdx);
double getStartAngle(size_t rayIdx);
double getStopAngle(size_t rayIdx);

vector <float> *getRayData(size_t rayIdx, string fieldName);

const RadxRay *getClosestRay(double azDeg);

private:
	RayLocationModel *_model;

};

#endif