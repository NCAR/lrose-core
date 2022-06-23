#ifndef RayLocationModel_HH
#define RayLocationModel_HH


#include <vector>


#include "RayLoc.hh"


class RayLocationModel {

public:

RayLocationModel();
~RayLocationModel();

void init();
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

RadxRay *getClosestRay(double azDeg);

bool isRayLocationSetup();

private:
	vector<RayLoc> ray_loc;

	// use as a mutex (until a real mutex is necessary)
	// to prevent access to the rayLocation array until 
	// the rays have been sorted.
	bool _rayLocationSetup;

};

#endif