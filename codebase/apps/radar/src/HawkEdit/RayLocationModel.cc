
#include "RayLocationModel.hh"
#include "DataModel.hh"
#include <toolsa/LogStream.hh>

RayLocationModel::RayLocationModel() {
  //_ppiRays = new RayLoc[RayLoc::RAY_LOC_N];
  //ray_loc = _ppiRays + RayLoc::RAY_LOC_OFFSET;
  ray_loc.resize(RayLoc::RAY_LOC_N);
}

RayLocationModel::~RayLocationModel() {}

// call when new data file is read, or when switching to new sweep?
void RayLocationModel::sortRaysIntoRayLocations(float ppi_rendering_beam_width,
  int sweepNumber) {
  LOG(DEBUG) << "enter";
//	_storeRayLoc(const RadxRay *ray, const double az,
//                                const double beam_width, RayLoc *ray_loc)

  double half_angle = ppi_rendering_beam_width / 2.0;

  DataModel *dataModel = DataModel::Instance();
  const vector<RadxRay *> &listOfRays = dataModel->getRays();
  vector<RadxRay *>::const_iterator rayItr;
  for (rayItr = listOfRays.begin(); rayItr != listOfRays.end(); ++rayItr) {
  // for each ray in file,
	// sort into RayLoc structure based on ray azimuth

  // Determine the extent of this ray: _startAz & _endAz
/*
  if (ppi_override_rendering_beam_width) {
    double half_angle = ppi_rendering_beam_width / 2.0;
    _startAz = az - half_angle - 0.1;
    _endAz = az + half_angle + 0.1;
  } else if (ray->getIsIndexed()) {
    double half_angle = ray->getAngleResDeg() / 2.0;
    _startAz = az - half_angle - 0.1;
    _endAz = az + half_angle + 0.1;
  } else {
    double max_half_angle = beam_width / 2.0;
    double prev_offset = max_half_angle;
    if (_prevAz >= 0.0) {
      double az_diff = az - _prevAz;
      if (az_diff < 0.0)
	      az_diff += 360.0;
      double half_az_diff = az_diff / 2.0;
	
      if (prev_offset > half_az_diff)
	      prev_offset = half_az_diff;
    }
    _startAz = az - prev_offset - 0.1;
    _endAz = az + max_half_angle + 0.1;
  }
 */
    const RadxRay *ray = *rayItr;

    // Sweep numbers start at 1 not Zero, so offset ...
         
    if (ray->getSweepNumber() == sweepNumber+1) {   


      double az = ray->getAzimuthDeg();
      double startAz = az - half_angle - 0.1;
      double endAz = az + half_angle + 0.1;

    // store
      
      int startIndex = (int) (startAz * RayLoc::RAY_LOC_RES);
      int endIndex = (int) (endAz * RayLoc::RAY_LOC_RES + 1);

      if (startIndex < 0) startIndex = 0;
      if (endIndex >= RayLoc::RAY_LOC_N) endIndex = RayLoc::RAY_LOC_N - 1;   

    // Clear out any rays in the locations list that are overlapped by the
    // new ray
      
    //_clearRayOverlap(startIndex, endIndex, ray_loc);

    // Set the locations associated with this ray

      if (endIndex < startIndex) {
      	LOG(DEBUG) << "ERROR endIndex: " << endIndex << " < startIndex: " << startIndex;
      } else {
  	    for (int ii = startIndex; ii < endIndex; ii++) {
  	      ray_loc[ii].ray = ray;
  	      ray_loc[ii].active = true;
  	      ray_loc[ii].startIndex = startIndex;
  	      ray_loc[ii].endIndex = endIndex;
  	    }
      }
    }
  }

  for (int i = 0; i< RayLoc::RAY_LOC_N; i++) {
  	LOG(DEBUG) << "ray_loc[" << i << "].startIdx = " << ray_loc[i].startIndex;
  	LOG(DEBUG) << "  ray_loc[" << i << "].endIdx = " << ray_loc[i].endIndex;
  }

  LOG(DEBUG) << "exit";
}

size_t RayLocationModel::getNRayLocations() {
  return (size_t) RayLoc::RAY_LOC_N;
};

double RayLocationModel::getStartRangeKm(size_t rayIdx) {
	const RadxRay *ray = ray_loc.at(rayIdx).ray;
	return ray->getStartRangeKm();
}

double RayLocationModel::getGateSpacingKm(size_t rayIdx) {
	const RadxRay *ray = ray_loc.at(rayIdx).ray;
	return ray->getGateSpacingKm();
}

size_t RayLocationModel::getEndIndex(size_t rayIdx) {
	return ray_loc.at(rayIdx).endIndex;
}

double RayLocationModel::getStartAngle(size_t rayIdx) {
	double resolution = 1.0/(double) RayLoc::RAY_LOC_RES;
	double az = ray_loc.at(rayIdx).startIndex * resolution;
	return az;
}

double RayLocationModel::getStopAngle(size_t rayIdx) {
	double resolution = 1.0/(double) RayLoc::RAY_LOC_RES;
	double az = ray_loc.at(rayIdx).endIndex * resolution;
	return az;
}

vector <float> *RayLocationModel::getRayData(size_t rayIdx, string fieldName) {
  vector<float> *dataVector = new vector<float>(0);
	// get the ray 
  const RadxRay *ray = ray_loc.at(rayIdx).ray;
  if (ray != NULL)  {// throw std::invalid_argument("rayIdx has no ray data");
  size_t nGates = ray->getNGates(); 

  // get the field data
  const RadxField *field = ray->getField(fieldName);

  // cerr << "there arenGates " << nGates;

  //field->convertToFl32();
  //convertToType(Radx::Fl32);
  
  const float *data = field->getDataFl32();
  dataVector->resize(nGates);
  dataVector->assign(data, data+nGates);
  }
// TODO: have calling method free the memory
  return dataVector;
}

const RadxRay *RayLocationModel::getClosestRay(double azDeg) {
	int rayIndex = (int) (azDeg * RayLoc::RAY_LOC_RES);
    if ((rayIndex < 0) || (rayIndex >= RayLoc::RAY_LOC_N)) {
    	throw "azimuth out of range";
    }
	return ray_loc.at(rayIndex).ray;
}