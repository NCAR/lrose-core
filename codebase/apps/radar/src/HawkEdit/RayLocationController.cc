
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

size_t RayLocationController::getNRayLocations() {
  return _model->getNRayLocations();
};

double RayLocationController::getStartRangeKm(size_t rayIdx) {
	return _model->getStartRangeKm(rayIdx);
}

double RayLocationController::getGateSpacingKm(size_t rayIdx) {
	return _model->getGateSpacingKm(rayIdx);
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

const RadxRay *RayLocationController::getClosestRay(double azDeg) {
	return _model->getClosestRay(azDeg);
}
