#include "DataModel.hh"  
#include <toolsa/LogStream.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxSweep.hh>
#include <cmath>

using namespace std;

DataModel *DataModel::_instance = NULL;

DataModel *DataModel::Instance() {
  	if (_instance == NULL) {
  		_instance = new DataModel();
  	}
  	return _instance;
}

DataModel::~DataModel() {}

/*
void DataModel::setData(RadxVol *vol) {
  _vol = vol;
}
*/

// return data for the field, at the sweep and ray index
const vector<float> *DataModel::GetData(string fieldName,
              int rayIdx, int sweepIdx)  {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx
       << " sweepIdx=" << sweepIdx;

  // sweep numbers are 1-based in RadxVol, not zero based, so, add one to the index.
  //sweepIdx += 1;

  _vol.loadRaysFromFields();

  //RadxSweep *sweep = _vol.getSweepByNumber(sweepIdx); // NOT by index!! Grrh!
  vector<RadxSweep *> volSweeps = _vol.getSweeps();
  RadxSweep *sweep = volSweeps.at(sweepIdx);
  if (sweep == NULL)
    throw std::invalid_argument("bad sweep index");

  size_t startRayIndex = sweep->getStartRayIndex();

  const RadxField *field;

  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol.getRays();
  if (rays.size() > 1) {
    LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  }
  RadxRay *ray = rays.at(startRayIndex + rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();

  vector<float> *dataVector = new vector<float>(nGates);
  dataVector->assign(data, data+nGates);

  return dataVector;
}

/*
const vector<float> *DataModel::GetData(string fieldName,
              int rayIdx, int sweepIdx)  {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx
       << " sweepIdx=" << sweepIdx;

  _vol.loadRaysFromFields();
  
  const RadxField *field;

  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol.getRays();
  if (rays.size() > 1) {
    LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  }
  RadxRay *ray = rays.at(rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();

  vector<float> *dataVector = new vector<float>(nGates);
  dataVector->assign(data, data+nGates);

  return dataVector;
}
*/

void DataModel::SetDataByIndex(string &fieldName, 
            int rayIdx, int sweepIdx, vector<float> *fieldData) { 

  // What is being returned? the name of the new field in the model that
  // contains the results.

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;



  _vol.loadRaysFromFields(); // loadFieldsFromRays();


    LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx
       << " sweepIdx=" << sweepIdx;

  // sweep numbers are 1-based in RadxVol, not zero based, so, add one to the index.
  sweepIdx += 1;

  RadxSweep *sweep = _vol.getSweepByNumber(sweepIdx);


  SetData(fieldName, rayIdx, sweep, fieldData);
  /* 
  if (sweep == NULL)
    throw std::invalid_argument("bad sweep index");

  size_t startRayIndex = sweep->getStartRayIndex();

  const RadxField *field;

  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol.getRays();
  if (rays.size() > 1) {
    LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  }
  RadxRay *ray = rays.at(startRayIndex + rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  int nGates = fieldData->size();

  Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = false;
  const string units = field->getUnits();

  ray->removeField(fieldName);

  const float *flatData = fieldData->data();
  RadxField *field1 = ray->addField(fieldName, units, nGates, missingValue, flatData, isLocal);
*/
  LOG(DEBUG) << "exit ";
}

void DataModel::SetData(string &fieldName, 
            int rayIdx, RadxSweep *sweep, vector<float> *fieldData) { 

  // What is being returned? the name of the new field in the model that
  // contains the results.

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;

  _vol.loadRaysFromFields(); // loadFieldsFromRays();

    LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;

  //if (sweep == NULL)
  //  throw std::invalid_argument("bad sweep index");

  //size_t startRayIndex = sweep->getStartRayIndex();

  //const 
  RadxField *field;

  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol.getRays();
  if (rays.size() <= 0) {
    LOG(DEBUG) <<  "ERROR - no rays found";
  }
  RadxRay *ray = rays.at(rayIdx); // startRayIndex + rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "no ray found";
  } 

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  //int nGates = fieldData->size();

//  Radx::fl32 missingValue = Radx::missingFl32; 
//  bool isLocal = false;
//  const string units = field->getUnits();

//  ray->removeField(fieldName);

//  const float *flatData = fieldData->data();
//  RadxField *field1 = ray->addField(fieldName, units, nGates, missingValue, flatData, isLocal);

//  LOG(DEBUG) << "ray after change ...";
//  ray->print(cerr);

//--- here ...
    // TODO: get the correct azimuth, and sweep number from  elevation
//  RadxField *field = _closestRay->getField(fieldName);

  if (field == NULL) {
    throw std::invalid_argument("no RadxField found ");
  } 

  vector<float> deref = *fieldData;
  const Radx::fl32 *radxData = &deref[0];
  bool isLocal = true;  //?? not sure about this 
  field->setDataFl32(nGates, radxData, isLocal);
  
  // make sure the new data are there ...
  // field->printWithData(cout);
  
  // data should be copied, so free the memory
  // delete data;
  
  // again, make sure the data are there
//  _closestRay->printWithFieldData(cout);
  
  //_vol->loadRaysFromFields();
  
  //std::ofstream outfile("/tmp/voldebug.txt");
  // finally, make sure the data are there
  //_vol->printWithFieldData(outfile);

  // end here 

  LOG(DEBUG) << "exit ";
}

void DataModel::SetData(string &fieldName, 
            float azimuth, float sweepAngle, vector<float> *fieldData) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " ray =" << azimuth
       << " sweep =" << sweepAngle;

  // this is the closest ray for the sweep angle
  int rayIdx = (int) findClosestRay(azimuth, sweepAngle);
  //_vol.loadRaysFromFields();

  //RadxSweep *sweep = _vol.getSweepByFixedAngle(sweepAngle);
  RadxSweep *dummyValue = nullptr;

  SetData(fieldName, rayIdx, dummyValue, fieldData);
  LOG(DEBUG) << "exit";
}

void DataModel::SetData(string &fieldName, float value) {
  RadxField *field;
  RadxRay *ray;

  size_t nRays = getNRays();
  for (size_t i = 0; i<nRays; i++) {
    ray = getRay(i);
    field = fetchDataField(ray, fieldName);
    size_t startGate, endGate;
    startGate = 0;
    endGate = ray->getNGates();
    field->setGatesToMissing(startGate, endGate);
  }
}

// remove field from volume
void DataModel::RemoveField(string &fieldName) {
  int result = _vol.removeField(fieldName);
  if (result != 0) {
    string msg = "failed to remove field: ";
    msg.append(fieldName); 
    throw std::invalid_argument(msg);
  }
}

// remove field from ray
void DataModel::RemoveField(size_t rayIdx, string &fieldName) {
  RadxRay *ray = getRay(rayIdx);
  int result = ray->removeField(fieldName);
  if (result != 0) {
    string msg = "failed to remove field from ray: ";
    msg.append(fieldName); 
    throw std::invalid_argument(msg);
  }
}

void DataModel::readData(string path, vector<string> &fieldNames,
	bool debug_verbose, bool debug_extra) {

  LOG(DEBUG) << "enter";
  // set up file object for reading
  
  RadxFile file;
  _vol.clear();
  _setupVolRead(file, fieldNames, debug_verbose, debug_extra);
   
  LOG(DEBUG) << "  reading data file path: " << path;    
    
  if (file.readFromPath(path, _vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "PolarManager::_getArchiveData\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + path + "\n";
      cerr << errMsg;
      throw errMsg;
  } 

  _vol.convertToFl32();

  // adjust angles for elevation surveillance if needed
  
  _vol.setAnglesForElevSurveillance();
  
  // compute the fixed angles from the rays
  // so that we reflect reality
  
  _vol.computeFixedAnglesFromRays();

    LOG(DEBUG) << "----------------------------------------------------";
    LOG(DEBUG) << "perform archive retrieval";
    LOG(DEBUG) << "  read file: " << _vol.getPathInUse();
    LOG(DEBUG) << "  nSweeps: " << _vol.getNSweeps();
   // LOG(DEBUG) << "  guiIndex, fixedAngle: " 
   //      << _sweepManager.getGuiIndex() << ", "
   //      << _sweepManager.getSelectedAngle();
    LOG(DEBUG) << "----------------------------------------------------";

  printAzimuthInRayOrder();

  LOG(DEBUG) << "exit";
}


// TODO: Do I need a DataController? to switch between Radx* data types and
// standard C++/Qt data types?

RadxTime DataModel::getStartTimeSecs() {
	return _vol.getStartTimeSecs();
}

RadxTime DataModel::getEndTimeSecs() {
  return _vol.getEndTimeSecs();
}

void DataModel::writeData(string path) {
    RadxFile outFile;

      LOG(DEBUG) << "writing to file " << path;
      outFile.writeToPath(_vol, path);
}

void DataModel::update() {

}

void DataModel::renameField(string currentName, string newName) {
  vector<RadxRay *> rays = _vol.getRays();	
  // for each ray, 
  vector<RadxRay *>::iterator it;
  for (it=rays.begin(); it != rays.end(); ++it) {
     // renameField(oldName, newName);
    (*it)->renameField(currentName, newName);
    // loadFieldNameMap
    (*it)->loadFieldNameMap();
  }
}

void DataModel::renameField(size_t rayIdx, string currentName, string newName) {
  RadxRay *ray = getRay(rayIdx);

  // renameField(oldName, newName);
  ray->renameField(currentName, newName);
  // loadFieldNameMap
  ray->loadFieldNameMap();

}

// copy from one field to another field 
void DataModel::copyField(size_t rayIdx, string fromFieldName, string toFieldName) {
  RadxRay *ray = getRay(rayIdx);
  //vector<RadxRay *> rays = _vol.getRays();  
  // for each ray, 
  //vector<RadxRay *>::iterator it;
  //for (it=rays.begin(); it != rays.end(); ++it) {
     // replaceField(RadxField *field);
    //RadxField *srcField = fetchDataField(*it, fromFieldName);
    RadxField *srcField = fetchDataField(ray, fromFieldName);
    Radx::fl32 *src = srcField->getDataFl32();

    //RadxField *dstField = fetchDataField(*it, toFieldName);
    RadxField *dstField = fetchDataField(ray, toFieldName);
    Radx::fl32 *dst = dstField->getDataFl32();

    size_t nbytes = ray->getNGates();
    //      #include <string.h>
    // void *memcpy(void *restrict dst, const void *restrict src, size_t n);
    memcpy(dst, src, nbytes*sizeof(Radx::fl32));
  //}
}

bool DataModel::fieldExists(size_t rayIdx, string fieldName) {
  RadxRay *ray = getRay(rayIdx);
  RadxField *field = fetchDataField(ray, fieldName);
  if (field != NULL) return true;
  else return false;
}

RadxField *DataModel::fetchDataField(RadxRay *ray, string &fieldName) {

  RadxField *dataField = ray->getField(fieldName);
  return dataField; 
}

const float *DataModel::fetchData(RadxRay *ray, string &fieldName) {

    RadxField *dataField = fetchDataField(ray, fieldName);
    if (dataField != NULL)
      return dataField->getDataFl32();
    else
      return NULL;
}

// total number of rays in volume, for all sweeps
size_t DataModel::getNRays() { // string fieldName, double sweepAngle) {
  _vol.loadRaysFromFields();
  const RadxField *field;
  const vector<RadxRay *>  &rays = _vol.getRays();
  size_t nRays = rays.size();
  return nRays;
}

// get the number of rays for a sweep
size_t DataModel::getNRays(int sweepNumber) {
  _vol.loadRaysFromFields();
  RadxSweep *sweep = _vol.getSweepByNumber(sweepNumber);
  if (sweep == NULL) {
    throw std::invalid_argument("no sweep found");
  }
  size_t nRays = sweep->getNRays();
  return nRays;
}

double DataModel::getRayAzimuthDeg(size_t rayIdx) {
  _vol.loadRaysFromFields();
  const vector<RadxRay *>  &rays = _vol.getRays();
  RadxRay *ray = rays.at(rayIdx);
  return ray->getAzimuthDeg();
}

double DataModel::getRayNyquistVelocityMps(size_t rayIdx) {
  _vol.loadRaysFromFields();
  const vector<RadxRay *>  &rays = _vol.getRays();
  RadxRay *ray = rays.at(rayIdx);
  return ray->getNyquistMps();
}

const vector<RadxRay *> &DataModel::getRays() {
  //const vector<RadxRay *>  &rays = vol->getRays();
	return _vol.getRays();
}

RadxRay *DataModel::getRay(size_t rayIdx) {
  _vol.loadRaysFromFields();
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol.getRays();

  RadxRay *ray = rays.at(rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    string msg = "bad ray index ";
    msg.append(to_string(rayIdx));
    throw std::invalid_argument(msg);
  } else {
  	return ray;
  }
}

void DataModel::printAzimuthInRayOrder() {
  _vol.loadRaysFromFields();
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol.getRays(); 
  LOG(DEBUG) << "first 20 rays in order ...";
  for (int i=0; i<20; i++) {
    RadxRay *ray = rays.at(i);
    LOG(DEBUG) << "ray Az = " << ray->getAzimuthDeg();
  }
}


int DataModel::getSweepNumber(float elevation) {
  //DataModel *dataModel = DataModel::Instance();
  vector<double> *sweepAngles = getSweepAngles();
  int i = 0;
  float delta = 0.01;
  bool found = false;  
  while ((i < sweepAngles->size()) && !found) {
    if (fabs(sweepAngles->at(i) - elevation) < delta) {
      found = true;
    } else {
      i += 1;
    }
  }
  if (!found) throw std::invalid_argument("no sweep found for elevation");

  // use the index, i, to find the sweep number, because
  // the index may be different than the number, which is a label for a sweep.
  vector<RadxSweep *> sweeps = _vol.getSweeps();
  RadxSweep *sweep = sweeps.at(i);
  int sweepNumber = sweep->getSweepNumber();
  return sweepNumber;
}




vector<float> *DataModel::getRayData(size_t rayIdx, string fieldName) { // , int sweepHeight) {
// TODO: which sweep? the rayIdx considers which sweep.

  LOG(DEBUG) << "enter" << " rayIdx = " << rayIdx 
    << " fieldName = " << fieldName;

  _vol.loadRaysFromFields();
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol.getRays();

  RadxRay *ray = rays.at(rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  const RadxField *field;
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // cerr << "there arenGates " << nGates;
  //field->convertToFl32();
  const float *data = field->getDataFl32();

  vector<float> *dataVector = new vector<float>(nGates);
  dataVector->assign(data, data+nGates);

  LOG(DEBUG) << "ray Az = " << ray->getAzimuthDeg()
    << "data[0-5] = " << data[0] << ", "  << data[1] << ", "  << data[2] << ", "  << data[3] << ", "  << data[4];

  return dataVector;
}

int DataModel::getNGates(size_t rayIdx, string fieldName, double sweepHeight) {

// TODO: which sweep? 
  _vol.loadRaysFromFields();
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol.getRays();
  if (rayIdx > rays.size()) {
    string msg = "rayIdx is out of bounds: ";
    msg.append(std::to_string(rayIdx));
    throw std::invalid_argument(msg);
  }
  RadxRay *ray = rays.at(rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  const RadxField *field;
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 
  return nGates;
}

float DataModel::getMissingFl32(string fieldName) {
  //_vol.loadFieldsFromRays();
  const RadxField *field = _vol.getFieldFromRay(fieldName);
  Radx::fl32 missingValue = field->getMissingFl32();
  return (float) missingValue;
}
  
DataModel::DataModel() {
	init();
}

void DataModel::init() {

}


const string &DataModel::getPathInUse() {
	return _vol.getPathInUse();
}

int DataModel::getNSweeps() {
	return _vol.getNSweeps();
}

vector<double> *DataModel::getSweepAngles() {
  vector<RadxSweep *> sweeps = _vol.getSweeps();
  vector<double> *sweepAngles = new vector<double>;
  vector<RadxSweep *>::iterator it;
  for (it = sweeps.begin(); it != sweeps.end(); ++it) {
  	RadxSweep *sweep = *it;
    sweepAngles->push_back(sweep->getFixedAngleDeg());
  }
  return sweepAngles;
}

// TODO: remove RadxPlatform and return base types
const RadxPlatform &DataModel::getPlatform() {
  return _vol.getPlatform();
} 

void DataModel::getPredomRayGeom(double *startRangeKm, double *gateSpacingKm) {
  double startRange;
  double gateSpace;
  _vol.getPredomRayGeom(startRange, gateSpace);
  *startRangeKm = startRange;
  *gateSpacingKm = gateSpace;  
}


vector<string> *DataModel::getUniqueFieldNameList() {
    _vol.loadFieldsFromRays();
    LOG(DEBUG) << "enter";
    const vector<string> fieldNames = _vol.getUniqueFieldNameList();
    vector<string> *fieldNamesCopy = new vector<string>;
    vector<string>::const_iterator it;
    for (it=fieldNames.begin(); it!=fieldNames.end(); ++it) {
    	fieldNamesCopy->push_back(*it);
    }
    LOG(DEBUG) << "there are " << fieldNamesCopy->size() << " fieldNames";
    LOG(DEBUG) << "exit";
    return fieldNamesCopy;
}

float DataModel::getLatitudeDeg() {
  return _vol.getLatitudeDeg();
}

float DataModel::getLongitudeDeg() {
  return _vol.getLongitudeDeg();
}

float DataModel::getAltitudeKm() {
  return _vol.getAltitudeKm();
}

const RadxGeoref *DataModel::getGeoreference(size_t rayIdx) {

  RadxRay *ray = getRay(rayIdx);
  // get the winds from the aircraft platform
  const RadxGeoref *georef = ray->getGeoreference();

  if (georef == NULL) {
    LOG(DEBUG) << "ERROR - georef is NULL";
    LOG(DEBUG) << "      trying to recover ...";
    _vol.setLocationFromStartRay();
    georef = ray->getGeoreference();
    if (georef == NULL) {
      throw "BBUnfoldAircraftWind: Georef is null. Cannot find ew_wind, ns_wind, vert_wind";
    }
  } 
  return georef;
}

void DataModel::_setupVolRead(RadxFile &file, vector<string> &fieldNames,
	bool debug_verbose, bool debug_extra)
{

  if (debug_verbose) {
    file.setDebug(true);
  }
  if (debug_extra) {
    file.setDebug(true);
    file.setVerbose(true);
  }

  //vector<string> fieldNames = _displayFieldController->getFieldNames();
  vector<string>::iterator it;
  for (it = fieldNames.begin(); it != fieldNames.end(); it++) {
    file.addReadField(*it);
  }
  
  //  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
  //    const DisplayField *field = _fields[ifield];
  //    file.addReadField(field->getName());
  //  }

}

size_t DataModel::findClosestRay(float azimuth, int sweepNumber) { // float elevation) {
  LOG(DEBUG) << "enter azimuth = " << azimuth << " sweepNumber = " << sweepNumber;

  DataModel *dataModel = DataModel::Instance();

  _vol.loadRaysFromFields();

  // NOTE! Sweep Number, NOT Sweep Index!!!

  RadxSweep *sweep = _vol.getSweepByNumber(sweepNumber);
  if (sweep == NULL) {
    //string msg = "no sweep found"
    throw std::invalid_argument("no sweep found");
  }

  // RadxSweep *sweep = _vol.getSweepByFixedAngle(elevation); DOESN'T WORK
  //if (sweep == NULL) 
  //  throw std::invalid_argument("unknown sweep elevation");
  //int requestedSweepNumber = sweep->getSweepNumber();

  const vector<RadxRay *> rays = dataModel->getRays();
  // find that ray
  bool foundIt = false;
  double minDiff = 1.0e99;
  size_t minIdx = 0;
  double delta = 0.1;  // TODO set this to the min diff between elevations/sweeps
  RadxRay *closestRayToEdit = NULL;
  //vector<RadxRay *>::const_iterator r;
  //r=rays.begin();
  size_t startIdx = sweep->getStartRayIndex();
  size_t endIdx = sweep->getEndRayIndex();
  //size_t idx = 0;
  size_t r = startIdx;
  while(r<=endIdx) {
    RadxRay *rayr = rays.at(r);
    double diff = fabs(azimuth - rayr->getAzimuthDeg());
    if (diff > 180.0) {
      diff = fabs(diff - 360.0);
    }
    if (diff < minDiff) {
      if (sweepNumber == rayr->getSweepNumber()) {
        foundIt = true;
        closestRayToEdit = rayr;
        minDiff = diff;
        minIdx = r; //idx;
      }
    }
    r += 1;
    //idx += 1;
  }  
  if (!foundIt || closestRayToEdit == NULL) {
    throw std::invalid_argument("could not find closest ray");
    // errorMessage("ExamineEdit Error", "couldn't find closest ray");
  }

  LOG(DEBUG) << "Found closest ray: index = " << minIdx << " min diff = " << minDiff;
   // << " elevation found = " << closestRayToEdit->getElevationDeg()
   // << " vs. requested = " << elevation; // " pointer = " << closestRayToEdit;
  //closestRayToEdit->print(cout); 
  //_closestRay = const_cast <RadxRay *> (closestRayToEdit);
  size_t closestRayIdx = minIdx;
  return closestRayIdx;
}

size_t DataModel::getRayIndex(size_t baseIndex, int offset, int sweepNumber) {
  RadxSweep *sweep = _vol.getSweepByNumber(sweepNumber);
  size_t startRayIndex = sweep->getStartRayIndex();
  size_t endRayIndex = sweep->getEndRayIndex();
  size_t idx = calculateRayIndex_f(baseIndex, startRayIndex, endRayIndex, offset);
  // swim around a bit to make sure we have the right ray index, 
  // just in case the end or start index is slightly off.
  // verify the sweep number 

  const vector<RadxRay *>  &rays = _vol.getRays();
  RadxRay *ray = rays.at(idx);
  //size_t forSureIdx;
  //forSureIdx = idx;
  while (ray->getSweepNumber() > sweepNumber) {
    idx -= 1;
    ray = rays.at(idx);
  }
  while (ray->getSweepNumber() < sweepNumber) {
    idx += 1;
    ray = rays.at(idx);
  }
  

  return idx;
}

size_t DataModel::calculateRayIndex_f(size_t idx, size_t start, size_t end, int offset) {

  size_t new_idx;

  float raw = (float) idx + offset;
  if (raw > end) {
    new_idx = start + (raw - end) - 1;
  } else if (raw < start) {
    new_idx = end - (start - raw) + 1;
  } else {
    new_idx = raw;
  }
  return new_idx;
}

