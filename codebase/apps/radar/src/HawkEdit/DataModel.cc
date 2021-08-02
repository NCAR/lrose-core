#include "DataModel.hh"  
#include <toolsa/LogStream.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxSweep.hh>


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

  _vol.loadRaysFromFields();

  RadxSweep *sweep = _vol.getSweepByNumber(sweepIdx);
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

void DataModel::SetData(string &fieldName, 
            int rayIdx, int sweepIdx, vector<float> *fieldData) { 

  // What is being returned? the name of the new field in the model that
  // contains the results.

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;

  _vol.loadRaysFromFields(); // loadFieldsFromRays();

  const RadxField *field;
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol.getRays();

  RadxRay *ray = rays.at(rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  int nGates = fieldData->size();

  Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = false;
  const float *flatData = fieldData->data();
  RadxField *field1 = ray->addField(fieldName, "m/s", nGates, missingValue, flatData, isLocal);
  LOG(DEBUG) << "exit ";
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
  if (result != 0) throw std::invalid_argument("failed to remove field");
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

size_t DataModel::getNRays() { // string fieldName, double sweepAngle) {
  _vol.loadRaysFromFields();
  const RadxField *field;
  const vector<RadxRay *>  &rays = _vol.getRays();
  size_t nRays = rays.size();
  return nRays;
}

double DataModel::getRayAzimuthDeg(size_t rayIdx) {
  _vol.loadRaysFromFields();
  const vector<RadxRay *>  &rays = _vol.getRays();
  RadxRay *ray = rays.at(rayIdx);
  return ray->getAzimuthDeg();
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
    throw "Ray is null";
  } else {
  	return ray;
  }
}

vector<float> *DataModel::getRayData(size_t rayIdx, string fieldName, double sweepHeight) {
// TODO: which sweep? 
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

  return dataVector;
}

int DataModel::getNGates(size_t rayIdx, string fieldName, double sweepHeight) {

// TODO: which sweep? 
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


