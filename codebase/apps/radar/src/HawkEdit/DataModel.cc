#include "DataModel.hh"  
#include <toolsa/LogStream.hh>
#include <Radx/RadxFile.hh>


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

}

void DataModel::update() {

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

size_t DataModel::getNRays(string fieldName, double sweepAngle) {
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

// TODO: remove RadxPlatform and return base types
const RadxPlatform &DataModel::getPlatform() {
  return _vol.getPlatform();
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


