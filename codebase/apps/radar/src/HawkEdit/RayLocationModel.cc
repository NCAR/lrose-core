
#include "RayLocationModel.hh"
#include "DataModel.hh"
#include <Radx/RadxCfactors.hh>
#include <toolsa/LogStream.hh>
#include <cmath>

RayLocationModel::RayLocationModel() {
  //_ppiRays = new RayLoc[RayLoc::RAY_LOC_N];
  //ray_loc = _ppiRays + RayLoc::RAY_LOC_OFFSET;
  ray_loc.resize(RayLoc::RAY_LOC_N);
  _rayLocationSetup = false;
}

RayLocationModel::~RayLocationModel() {}

void RayLocationModel::init() {
  _rayLocationSetup = false;
  for (size_t ii = 0; ii < RayLoc::RAY_LOC_N; ii++) {
    ray_loc[ii].ray = NULL;
    ray_loc[ii].active = false;
    ray_loc[ii].startIndex = 0;
    ray_loc[ii].endIndex = 0;    
    //LOG(DEBUG) << "ray_loc[" << i << "].startIdx = " << ray_loc[i].startIndex;
    //LOG(DEBUG) << "  ray_loc[" << i << "].endIdx = " << ray_loc[i].endIndex;
  }
}

// call when new data file is read, or when switching to new sweep?
void RayLocationModel::sortRaysIntoRayLocations(float ppi_rendering_beam_width,
  int sweepNumber) {
  LOG(DEBUG) << "enter";

  DataModel *dataModel = DataModel::Instance();
  if (dataModel->getPrimaryAxis() == Radx::PRIMARY_AXIS_Y_PRIME) {
    // _platform.getPrimaryAxis()
    _sortRaysIntoRayLocationsUsingRotation(ppi_rendering_beam_width,
      sweepNumber);
  } else {
    _sortRaysIntoRayLocationsUsingAzimuth(ppi_rendering_beam_width,
      sweepNumber);
  }
  LOG(DEBUG) << "exit";
}

// call when new data file is read, or when switching to new sweep?
void RayLocationModel::_sortRaysIntoRayLocationsUsingAzimuth(
  float ppi_rendering_beam_width,
  int sweepNumber) {
  LOG(DEBUG) << "enter";
  init();
//	_storeRayLoc(const RadxRay *ray, const double az,
//                                const double beam_width, RayLoc *ray_loc)

  DataModel *dataModel = DataModel::Instance();
  vector<RadxRay *> &listOfRays = dataModel->getRays();

  // the rendering beam width should be the minimum distance between rays.
  // this may result in gaps between rays, but at least the rays won't
  // overlap, and hide data.

  double minDistance = 99999999.0;
  double previousAz = 0.0;
  size_t minDistanceRayIdx = 0;
  vector<RadxRay *>::const_iterator rayItr;
  for (rayItr = listOfRays.begin(); rayItr != listOfRays.end(); ++rayItr) {
    RadxRay *ray = *rayItr;
    if (ray->getSweepNumber() == sweepNumber) {   
      double az = ray->getAzimuthDeg();
      if (rayItr != listOfRays.begin()) { 
        // what if the rays are NOT in sorted order by az? It will be close enough
        double distance = fabs(az - previousAz);
        LOG(DEBUG) << "az= " << az << " distance=" << distance;
        if (distance < minDistance) {
          minDistance = distance;
          minDistanceRayIdx = rayItr - listOfRays.begin();
        }
      }
      previousAz = az;
    }
  }
  LOG(DEBUG) << "using minimum ray distance of " << minDistance;
  if (minDistance < 1) {
    minDistance = 1.0;
    cerr << "min ray distance less than 1.0, setting to 1.0" << endl;
  }
  //double half_angle = ppi_rendering_beam_width / 2.0;
  double half_angle = minDistance / 2.0;


  LOG(DEBUG) << "sorting " << listOfRays.size() << " rays";

  // vector<RadxRay *>::const_iterator rayItr;
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
    RadxRay *ray = *rayItr;
         
    if (ray->getSweepNumber() == sweepNumber) {   


      double az = ray->getAzimuthDeg();
      while (az < 0) {
        az += 360.0;  
        //TODO: where to handle this?? when making mods for survellance? or somehwere else??
        // mod everything by 360?
      }
      while (az > 360) {
        az = az - 360;
      }
      double startAz = az - half_angle; // - 0.1;
      double endAz = az + half_angle; // + 0.1;

    // store

      int startIndex = (int) (startAz * RayLoc::RAY_LOC_RES);
      int endIndex = (int) (endAz * RayLoc::RAY_LOC_RES); //  + 1);
      LOG(DEBUG) << "startIndex " << startIndex << " to " << endIndex;
      if (startIndex < 0) startIndex = 0;
      if (endIndex >= RayLoc::RAY_LOC_N) endIndex = RayLoc::RAY_LOC_N - 1;   

    // Clear out any rays in the locations list that are overlapped by the
    // new ray
      
    //_clearRayOverlap(startIndex, endIndex, ray_loc);

    // Set the locations associated with this ray

      if (endIndex < startIndex) {
      	LOG(DEBUG) << "ERROR endIndex: " << endIndex << " < startIndex: " << startIndex;
      } else {
  	    for (int ii = startIndex; ii <= endIndex; ii++) {
  	      ray_loc[ii].ray = ray;
  	      ray_loc[ii].active = true;
  	      ray_loc[ii].startIndex = startIndex;
  	      ray_loc[ii].endIndex = endIndex;
  	    }
      }
    }
  }
  _rayLocationSetup = true;

  //for (int i = 0; i< RayLoc::RAY_LOC_N; i++) {
  	//LOG(DEBUG) << "ray_loc[" << i << "].startIdx = " << ray_loc[i].startIndex;
  	//LOG(DEBUG) << "  ray_loc[" << i << "].endIdx = " << ray_loc[i].endIndex;
  //}

  LOG(DEBUG) << "exit";
}

// call when new data file is read, or when switching to new sweep?
void RayLocationModel::_sortRaysIntoRayLocationsUsingRotation(
  float ppi_rendering_beam_width,
  int sweepNumber) {
  LOG(DEBUG) << "enter";
  init();
//  _storeRayLoc(const RadxRay *ray, const double az,
//                                const double beam_width, RayLoc *ray_loc)

  DataModel *dataModel = DataModel::Instance();
  vector<RadxRay *> &listOfRays = dataModel->getRays();

  //RadxCfactors *cfac = dataModel->getCfactors();

  // the rendering beam width should be the minimum distance between rays.
  // this may result in gaps between rays, but at least the rays won't
  // overlap, and hide data.

  double minDistance = 99999999.0;
  double previousAz = 0.0;
  size_t minDistanceRayIdx = 0;
  vector<RadxRay *>::const_iterator rayItr;
  for (rayItr = listOfRays.begin(); rayItr != listOfRays.end(); ++rayItr) {
    RadxRay *ray = *rayItr;
    if (ray->getSweepNumber() == sweepNumber) {   
      /*
      RadxGeoref *georef = ray->getGeoreference();
      if (georef == NULL) {
        throw std::invalid_argument("georefence is NULL cannot sort rays");
      }

      //float T_deg = (georef->getHeading() + cfac->getHeadingCorr())  + 
      //        (georef->getDrift() + cfac->getDriftCorr());
      //float T_deg = (georef->getHeading())  + 
      //        (georef->getDrift());              
      //float T = T_deg * Radx::DegToRad;
      // CfRadial has the track-relative rotation,
      // but Dorade files, don't have this value, it is missing.
      // what to use for Dorade? azimuth - T? or Rotation - Track?
      double az = georef->getTrackRelRot(); // georef->getRotation()  - T_deg; // getTrackRelEl();
      if (az == Radx::missingMetaDouble) {
        // use the rotation angle for the az; this is the best guess in this
        // situation.
        az = georef->getRotation();
      }*/

      float az = _getTrackRelativeRotation(ray);
      if (rayItr != listOfRays.begin()) { 
        // what if the rays are NOT in sorted order by az? It will be close enough
        double distance = fabs(az - previousAz);
        LOG(DEBUG) << "az= " << az << " distance=" << distance;
        if (distance < minDistance) {
          minDistance = distance;
          minDistanceRayIdx = rayItr - listOfRays.begin();
        }
      }
      previousAz = az;
    }
  }
  LOG(DEBUG) << "using minimum ray distance of " << minDistance;
  if (minDistance < 1) {
    minDistance = 1.0;
    cerr << "min ray distance less than 1.0, setting to 1.0" << endl;
  }
  //double half_angle = ppi_rendering_beam_width / 2.0;
  double half_angle = minDistance / 2.0;


  LOG(DEBUG) << "sorting " << listOfRays.size() << " rays";

  // vector<RadxRay *>::const_iterator rayItr;
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
    RadxRay *ray = *rayItr;
         
    if (ray->getSweepNumber() == sweepNumber) {   

      RadxGeoref *georef = ray->getGeoreference();
      if (georef == NULL) {
        throw std::invalid_argument("georefence is NULL cannot sort rays2");
      }
     // RadxCfactors *cfac = ray->getCfactors();
    //float T_deg = (georef->getHeading() + cfac->getHeadingCorr())  + 
    //          (georef->getDrift() + cfac->getDriftCorr());
    //float T_deg = (georef->getHeading())  + 
    //          (georef->getDrift());                
    //float T = T_deg * Radx::DegToRad;
      double az = georef->getTrackRelRot(); // ray->getGeoreference()->getTrackRelRot();
       if (az == Radx::missingMetaDouble) {
        // use the rotation angle for the az; this is the best guess in this
        // situation.
        az = georef->getRotation();
      }
      while (az < 0) {
        az += 360.0;  
        //TODO: where to handle this?? when making mods for survellance? or somehwere else??
        // mod everything by 360?
      }
      while (az > 360) {
        az = az - 360;
      }
      double startAz = az - half_angle; // - 0.1;
      double endAz = az + half_angle; // + 0.1;

    // store

      int startIndex = (int) (startAz * RayLoc::RAY_LOC_RES);
      int endIndex = (int) (endAz * RayLoc::RAY_LOC_RES); //  + 1);
      LOG(DEBUG) << "startIndex " << startIndex << " to " << endIndex;
      if (startIndex < 0) startIndex = 0;
      if (endIndex >= RayLoc::RAY_LOC_N) endIndex = RayLoc::RAY_LOC_N - 1;   

    // Clear out any rays in the locations list that are overlapped by the
    // new ray
      
    //_clearRayOverlap(startIndex, endIndex, ray_loc);

    // Set the locations associated with this ray

      if (endIndex < startIndex) {
        LOG(DEBUG) << "ERROR endIndex: " << endIndex << " < startIndex: " << startIndex;
      } else {
        for (int ii = startIndex; ii <= endIndex; ii++) {
          ray_loc[ii].ray = ray;
          ray_loc[ii].active = true;
          ray_loc[ii].startIndex = startIndex;
          ray_loc[ii].endIndex = endIndex;
        }
      }
    }
  }
  _rayLocationSetup = true;

  //for (int i = 0; i< RayLoc::RAY_LOC_N; i++) {
    //LOG(DEBUG) << "ray_loc[" << i << "].startIdx = " << ray_loc[i].startIndex;
    //LOG(DEBUG) << "  ray_loc[" << i << "].endIdx = " << ray_loc[i].endIndex;
  //}

  LOG(DEBUG) << "exit";
}


bool RayLocationModel::isRayLocationSetup() {
  return _rayLocationSetup;
}

size_t RayLocationModel::getNRayLocations() {
  return (size_t) RayLoc::RAY_LOC_N;
}

double RayLocationModel::getStartRangeKm(size_t rayIdx) {
	RadxRay *ray = ray_loc.at(rayIdx).ray;
	return ray->getStartRangeKm();
}

double RayLocationModel::getGateSpacingKm(size_t rayIdx) {
	RadxRay *ray = ray_loc.at(rayIdx).ray;
	return ray->getGateSpacingKm();
}

// over all the rays, find the maximum range
double RayLocationModel::getMaxRangeKm() {

  double max = 0.0;

  DataModel *dataModel = DataModel::Instance();
  vector<RadxRay *> &listOfRays = dataModel->getRays();
  vector<RadxRay *>::const_iterator rayItr;
  for (rayItr = listOfRays.begin(); rayItr != listOfRays.end(); ++rayItr) {
    RadxRay *ray = *rayItr;

    double fieldRange = ray->getStartRangeKm() + 
      (double) ray->getNGates() * ray->getGateSpacingKm();
    if (fieldRange > max) {
      max = fieldRange;
    }
  }
  return max;
}

size_t RayLocationModel::getEndIndex(size_t rayIdx) {
	return ray_loc.at(rayIdx).endIndex;
}

size_t RayLocationModel::getStartIndex(size_t rayIdx) {
  return ray_loc.at(rayIdx).startIndex;
}

double RayLocationModel::getStartAngle(size_t rayIdx) {
	double resolution = 1.0/(double) RayLoc::RAY_LOC_RES;
	double az = (ray_loc.at(rayIdx).startIndex-0.5) * resolution;
	return az;
}

double RayLocationModel::getStopAngle(size_t rayIdx) {
	double resolution = 1.0/(double) RayLoc::RAY_LOC_RES;
	double az = (ray_loc.at(rayIdx).endIndex+0.5) * resolution;
	return az;
}

vector <float> *RayLocationModel::getRayData(size_t rayIdx, string fieldName) {
  //cout << "RayLocationModel::getRayData for rayIdx " << rayIdx << endl;
  vector<float> *dataVector;
	// get the ray 
  RadxRay *ray = ray_loc.at(rayIdx).ray;
  if (ray != NULL)  { // throw std::invalid_argument("rayIdx has no ray data");
    size_t nGates = ray->getNGates(); 
    //dataVector->resize(nGates);

    // get the field data
    DataModel *dataModel = DataModel::Instance();
    RadxField *field = NULL;
    try  {
      field = dataModel->fetchDataField(ray, fieldName);
    } catch (std::invalid_argument &ex) {
      cerr << "RayLocationModel::getRayData catching exception no field found\n";
      field = NULL;
    }
    //const RadxField *field = ray->getField(fieldName);
    if (field == NULL) {
      //string msg = "no data for field in ray ";
      //msg.append(fieldName);
      //throw std::invalid_argument(msg);
      //delete dataVector;
      // create vector; initialize to missing
      dataVector = new vector<float>(nGates, Radx::missingFl32);   //issue with memory!!! who will free?
    } else {
      // cerr << "there arenGates " << nGates;

      field->convertToFl32();
      //convertToType(Radx::Fl32);
    
      float *data = field->getDataFl32();
      dataVector = new vector<float>(nGates);
      dataVector->assign(data, data+nGates);
    }
  } else {
    dataVector = new vector<float>(0);
  }
  // have calling method free the memory
  return dataVector;
}

size_t RayLocationModel::getClosestRayIdx(float azDeg, int offset) {
  int rayIdx = (int) (azDeg * RayLoc::RAY_LOC_RES);
    if ((rayIdx < 0) || (rayIdx >= RayLoc::RAY_LOC_N)) {
      throw "azimuth out of range";
    }
  // now find the ray that is offset
  if (offset < 0) { 
    // move through the startIndex
    int offsetIdx = offset;
    while (offsetIdx < 0) {
      rayIdx = getStartIndex(rayIdx);
      // move to next active ray
      do {
        rayIdx -= 1;
        if (rayIdx < 0) {
          rayIdx = RayLoc::RAY_LOC_N - 1;
        }
      } while (!ray_loc.at(rayIdx).active);      
      offsetIdx += 1;
    }
  } 
  if (offset > 0) {
    // move through the endIndex
    int offsetIdx = offset;
    while (offsetIdx > 0) {
      rayIdx = getEndIndex(rayIdx);
      // move to next active ray
      do {
        rayIdx += 1;
        if (rayIdx >= RayLoc::RAY_LOC_N) {
          rayIdx = 0;
        }
      } while (!ray_loc.at(rayIdx).active);
      offsetIdx -= 1;
    }
  }  
  return rayIdx;
}

vector <float> *RayLocationModel::getRayDataOffset(float azimuth, 
  int offsetFromClosest, string fieldName) {
  // find the rayIdx
  size_t rayIdx = getClosestRayIdx(azimuth, offsetFromClosest);
  return getRayData(rayIdx, fieldName);

}

RadxRay *RayLocationModel::getClosestRay(double azDeg) {
	int rayIndex = (int) (azDeg * RayLoc::RAY_LOC_RES);

  // find the closest ray_loc that is active
  // search in positive direction first
  bool activeFound = false;
  int nsteps = 0;
  int indexPlus = rayIndex;
  while ((indexPlus >= 0) && (indexPlus < RayLoc::RAY_LOC_N)
    && !activeFound && (nsteps < 50)) {
    if (ray_loc.at(indexPlus).active) {
      activeFound = true;
    } else {
      indexPlus += 1;
      nsteps += 1;
      if (indexPlus >= RayLoc::RAY_LOC_N)
        indexPlus = 0; 
    }
  }

  // search in the negative direction
  activeFound = false;
  nsteps = 0;
  int indexNeg = rayIndex;
  while ((indexNeg >= 0) && (indexNeg < RayLoc::RAY_LOC_N)
    && !activeFound && (nsteps < 50)) {
    if (ray_loc.at(indexNeg).active) {
      activeFound = true;
    } else {
      indexNeg -= 1;
      nsteps += 1;
      if (indexNeg < 0)
        indexNeg = RayLoc::RAY_LOC_N-1; 
    }
  }
  if (nsteps >= 50) throw "Way to many steps to find closest ray!!";

  int distPlus = abs(indexPlus - rayIndex);
  int distNeg = abs(indexNeg - rayIndex);
  int closestIdx = indexNeg;
  if (distPlus < distNeg) {
    closestIdx =  indexPlus;
  }

    //if ((rayIndex < 0) || (rayIndex >= RayLoc::RAY_LOC_N)) {
    //	throw "azimuth out of range";
    //}
	return ray_loc.at(closestIdx).ray;
}

size_t RayLocationModel::validateRayLocIndex(size_t index) {
  // ray loc is circular; it goes from 0 ... RAY_LOC_N-1 ... 0
  if (index >= RayLoc::RAY_LOC_N) {
    return 0;
  }
  if (index < 0) {
    return RayLoc::RAY_LOC_N-1;
  }
  return index;
}

// Why not just fill in the ray loc array with pointers?? from start to end??
// There are spaces in the ray location map, 
// depending on the beam width and how close the data are. 
// Remember, size_t cannot be negative, but it can be > RAY_LOC_N
RadxRay *RayLocationModel::findClosestNonNullRay(size_t rayIndex) {
  size_t startIndex = validateRayLocIndex(rayIndex);

  size_t offset = 1;
  size_t testIndex;
  bool found = false;
  //bool atStart = false;
  RadxRay *ray;
  while ((!found) && (offset < RayLoc::RAY_LOC_N/2)) {
    testIndex = validateRayLocIndex(startIndex + offset);
    //atStart = testIndex == startIndex;
    ray = ray_loc.at(testIndex).ray;
    if (ray != NULL) {
      found = true;
    } else {
      testIndex = validateRayLocIndex(startIndex - offset);
      //if (!atStart) {
      //  atStart = testIndex == startIndex;
      //}
      if (ray != NULL) {
        found = true;
      }
    }
    offset += 1;
  }
  if (found) {
    return ray;
  } else {
    throw std::invalid_argument("RayLocationModel::findClosestNonNullRay - all rays are NULL");
  }
}  

size_t RayLocationModel::getClosestRayIdx(double azDeg) {
  int rayIndex = (int) (azDeg * RayLoc::RAY_LOC_RES);
    if ((rayIndex < 0) || (rayIndex >= RayLoc::RAY_LOC_N)) {
      throw "azimuth out of range";
    }
  return rayIndex;
}

float RayLocationModel::getNyquistVelocityForRay(double azDeg,
  int offset) {

  size_t rayIdx = getClosestRayIdx(azDeg, offset);
  RadxRay *ray = findClosestNonNullRay(rayIdx); 
  return ray->getNyquistMps();;
}

float RayLocationModel::getAzimuthForRay(double azDeg,
  int offset) {

  DataModel *dataModel = DataModel::Instance();
  if (dataModel->getPrimaryAxis() == Radx::PRIMARY_AXIS_Y_PRIME) {
    // _platform.getPrimaryAxis()
    return _getAzimuthForRayYPrimeAxis(azDeg, offset);
  } else {
    return _getAzimuthForRayUsingAzimuth(azDeg, offset);
  }  
}

float RayLocationModel::getAzimuthForRay(const RadxRay *ray) {
  DataModel *dataModel = DataModel::Instance();
  if (dataModel->getPrimaryAxis() == Radx::PRIMARY_AXIS_Y_PRIME) {
    // _platform.getPrimaryAxis()
    return _getTrackRelativeRotation(ray);
  } else {
    return ray->getAzimuthDeg();
  }   
}

float RayLocationModel::_getAzimuthForRayUsingAzimuth(double azDeg,
  int offset) {

  size_t rayIdx = getClosestRayIdx(azDeg, offset);
  RadxRay *ray = findClosestNonNullRay(rayIdx); 
  return ray->getAzimuthDeg();
}

float RayLocationModel::_getAzimuthForRayYPrimeAxis(double angleDeg,
  int offset) {

  size_t rayIdx = getClosestRayIdx(angleDeg, offset);
  RadxRay *ray = findClosestNonNullRay(rayIdx); 
  return _getTrackRelativeRotation(ray);
}

float RayLocationModel::_getTrackRelativeRotation(const RadxRay *ray) {
      const RadxGeoref *georef = ray->getGeoreference();
      if (georef == NULL) {
        throw std::invalid_argument("georefence is NULL cannot retrieve\
         track relative rotation rays");
      }
      double az = georef->getTrackRelRot(); // georef->getRotation()  - T_deg; // getTrackRelEl();
      if (az == Radx::missingMetaDouble) {
        // use the rotation angle for the az; this is the best guess in this
        // situation.
        az = georef->getRotation();
      }  
      return az;
}


