
#include "RayLocationController.hh"
#include <toolsa/LogStream.hh>


RayLocationController::RayLocationController() {
  LOG(DEBUG) << "enter";
  _model = new RayLocationModel();
  LOG(DEBUG) << "exit";
}

RayLocationController::~RayLocationController() {}

// call when new data file is read, or when switching to new sweep?
void RayLocationController::sortRaysIntoRayLocations(float ppi_rendering_beam_width,
  int sweepNumber) {
  _model->sortRaysIntoRayLocations(ppi_rendering_beam_width, sweepNumber);	
}

bool RayLocationController::isRayLocationSetup() {
  return _model->isRayLocationSetup();
}

size_t RayLocationController::getNRayLocations() {
  return _model->getNRayLocations();
};

double RayLocationController::getStartRangeKm(size_t rayIdx) {
	return _model->getStartRangeKm(rayIdx);
}

double RayLocationController::getGateSpacingKm(size_t rayIdx) {
	return _model->getGateSpacingKm(rayIdx);
}

double RayLocationController::getMaxRangeKm() {
  return _model->getMaxRangeKm();
}

size_t RayLocationController::getEndIndex(size_t rayIdx) {
	return _model->getEndIndex(rayIdx);
}

double RayLocationController::getStartAngle(size_t rayIdx) {
	return _model->getStartAngle(rayIdx);
}

double RayLocationController::getStopAngle(size_t rayIdx) {
  return _model->getStopAngle(rayIdx);
}

vector <float> *RayLocationController::getRayData(size_t rayIdx, string fieldName) {
  return _model->getRayData(rayIdx, fieldName);
}

vector <float> *RayLocationController::getRayDataOffset(float azimuth, 
  int offsetFromClosest, string fieldName) {
  return _model->getRayDataOffset(azimuth, offsetFromClosest, fieldName);
}


const RadxRay *RayLocationController::getClosestRay(double azDeg) {
	return _model->getClosestRay(azDeg);
}

float RayLocationController::getNyquistVelocityForRay(float azDeg, int offset) {
  return _model->getNyquistVelocityForRay(azDeg, offset);
}

float RayLocationController::getAzimuthForRay(float azDeg, int offset) {
  return _model->getAzimuthForRay(azDeg, offset);
}
