#ifndef RayLocationModel_HH
#define RayLocationModel_HH


#include <vector>


#include "RayLoc.hh"


// TODO: this class should become a base class with
// inherited classes (or abstract class)
// that implement these functions for the
// specific primary axis:
//  * sortRaysIntoRayLocations
//  * getRayDataOffset
//  * getClosestRay
//  * getClsoestRayIdx
//  * getAzimuthForRay
//

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
size_t getStartIndex(size_t rayIdx);
double getStartAngle(size_t rayIdx);
double getStopAngle(size_t rayIdx);

vector <float> *getRayData(size_t rayIdx, string fieldName);
vector <float> *getRayDataOffset(float azimuth, 
  int offsetFromClosest, string fieldName);

RadxRay *getClosestRay(double azDeg);
size_t getClosestRayIdx(double azDeg);
size_t getClosestRayIdx(float azDeg, int offset);
float getNyquistVelocityForRay(double azDeg, int offset);
float getAzimuthForRay(double azDeg, int offset);
float getAzimuthForRay(const RadxRay *ray);
RadxRay *findClosestNonNullRay(size_t rayIndex);
size_t validateRayLocIndex(size_t index);

bool isRayLocationSetup();

private:
	vector<RayLoc> ray_loc;

	// use as a mutex (until a real mutex is necessary)
	// to prevent access to the rayLocation array until 
	// the rays have been sorted.
	bool _rayLocationSetup;

	void _sortRaysIntoRayLocationsUsingAzimuth(float ppi_rendering_beam_width,
		int sweepNumber);
	void _sortRaysIntoRayLocationsUsingRotation(float ppi_rendering_beam_width,
		int sweepNumber);

	float _getAzimuthForRayYPrimeAxis(double angleDeg, int offset);
	float _getAzimuthForRayUsingAzimuth(double azDeg, int offset);
	float _getTrackRelativeRotation(const RadxRay *ray);

};

#endif