//
// SpreadSheetModel provides data in basic form, as ints, floats, strings
// SpreadSheetModel provides the interface to Radx utilities, and file I/O

#include <stdio.h>
#include <stdexcept>
#include "SpreadSheetModel.hh"
#include <toolsa/LogStream.hh>
#include <fstream>

SpreadSheetModel::SpreadSheetModel()
{

}

SpreadSheetModel::SpreadSheetModel(RadxRay *closestRay, RadxVol *dataVolume)
{
  _closestRay = closestRay;
  if (_closestRay == NULL) 
    cout << "in SpreadSheetModel, closestRay is NULL" << endl;
  else
   cout << "in SpreadSheetModel, closestRay is NOT  NULL" << endl;
  // closestRay->print(cout);
  _vol = dataVolume;
  _vol->loadFieldsFromRays();
}

void SpreadSheetModel::getRangeGeom(float *startRangeKm, float *gateSpacingKm) { 
  double startRange;
  double gateSpace;
    // TODO: maybe on construction, map to finest range geometry? then we can call 
  _vol->getPredomRayGeom(startRange, gateSpace);
  *startRangeKm = startRange;
  *gateSpacingKm = gateSpace;
}

vector<string> SpreadSheetModel::getFields()
{
  vector<string> fieldNames;
  if (_closestRay != NULL) {
    _closestRay->loadFieldNameMap();

    RadxRay::FieldNameMap fieldNameMap = _closestRay->getFieldNameMap();
    RadxRay::FieldNameMapIt it;
    for (it = fieldNameMap.begin(); it != fieldNameMap.end(); it++) {
      fieldNames.push_back(it->first);
      cout << it->first << ':' << it->second << endl;
    }
  } else {
    _vol->loadFieldsFromRays();
    fieldNames = _vol->getUniqueFieldNameList();
  }
  return fieldNames;
}

// return the azimuth of the ray offset from the closest ray
float SpreadSheetModel::getAzimuthForRay(int offsetFromClosest)
{
  float azimuth = 0.0;
  if (offsetFromClosest == 0) {
    azimuth = _closestRay->getAzimuthDeg();
  } else {
    vector<RadxRay *> rays = _vol->getRays();
    size_t idx = (_closestRayIdx + offsetFromClosest) % rays.size();
    cout << "closestRayIdx=" << _closestRayIdx << " offsetFromClosest=" << offsetFromClosest 
      << " idx=" << idx << endl;
    RadxRay *ray = rays.at(idx);
    azimuth = ray->getAzimuthDeg(); 
  }
  return azimuth;
}

// return a list of data values for the given
// field name
// vector<double>
vector<float> *SpreadSheetModel::getData(string fieldName, int offsetFromClosest)
{

  // vector <double> *dataVector = NULL;
  const RadxField *field;

  if (offsetFromClosest == 0) {
    //  field = _vol.getFieldFromRay(fieldName);  // <--- Why is this returning NULL
    // because the type is 
    // from debugger:  *((vol.getFieldFromRay("VEL"))->getDataSi16()+1)
    field = _closestRay->getField(fieldName);
  } else {
    vector<RadxRay *> rays = _vol->getRays();
    size_t idx = (_closestRayIdx + offsetFromClosest) % rays.size();
    RadxRay *ray = rays.at(idx);
    field = ray->getField(fieldName); 
  }

  if (field == NULL) {
    throw std::invalid_argument("no RadxField found "); //  <<  endl;
    //return NULL; // dataVector;
  } 
  // Radx::fl32 *data = field->getDataFl32();
  // how may gates?
  size_t nPoints = field->getNPoints();
  LOG(DEBUG) << "nGates = " << nPoints;
  float *data = (float *) field->getDataFl32();
  LOG(DEBUG) << data << ", " << data+1 << ", " << data +2;
  vector<float> *dataVector = new vector<float>(data, data + nPoints);
  //  LOG(DEBUG) << dataVector.at[0] << ", " << dataVector.at[1] << ", " << dataVector.at[2];

  //vector<float> *dataVectorPtr = &dataVector;
  for (int i=0; i<3; i++) {
    float value = dataVector->at(i);
    LOG(DEBUG) << value;
  }

  return dataVector;

}


/*
RadxVol SpreadSheetModel::getVolume() {
  return _vol;
}
*/

// set data values for the field in the Volume 
void SpreadSheetModel::setData(string fieldName, vector<float> *data)
{
  LOG(DEBUG) << "fieldName=" << fieldName;

  //const RadxField *field;
  //  field = _vol.getFieldFromRay(fieldName);  // <--- Why is this returning NULL
  // because the type is 
  // from debugger:  *((vol.getFieldFromRay("VEL"))->getDataSi16()+1)
  //field = _closestRay->getField(fieldName);

  // addField just modifies the name if there is a duplicate name,
  // so we can always add the field; we don't need to modify
  // an existing field.

  size_t nGates = data->size();
  size_t nGatesInRay = _closestRay->getNGates();
  if (nGates < nGatesInRay) {
    // TODO: expand, filling with missing Value
  }

  RadxField *field = _closestRay->getField(fieldName);

  if (field == NULL) {
    throw std::invalid_argument("no RadxField found ");
  } 

  //RadxField *fieldCopy = new RadxField(*field);

    vector<float> deref = *data;
    const Radx::fl32 *radxData = &deref[0];
    bool isLocal = true;  //?? not sure about this 
    field->setDataFl32(nGates, radxData, isLocal);
  
    // make sure the new data are there ...
    field->printWithData(cout);

    //cerr << "replacing data in field " <<  endl;
    //int result = _closestRay->replaceField(fieldCopy); 
    //if (result == -1)
    //  throw "Error replacing Field";
    //    _closestRay->addField("WIDTH", units, nGates, missingValue, radxData, isLocal);
    //} else {
    // replace the data values in the existing field
  // Radx::fl32 *data = field->getDataFl32();
  // how may gates?
  //  size_t nPoints = field->getNPoints();
  //  field->addDataFl64(nGates, &data[0]);
  //}

    // data should be copied, so free the memory
    // delete data;

    // again, make sure the data are there
    _closestRay->printWithFieldData(cout);

    //    _vol.addRay(_closestRay);


    _vol->loadRaysFromFields();
    //_vol->loadFieldsFromRays();
    //_vol->loadSweepInfoFromRays();

    std::ofstream outfile("/tmp/voldebug.txt");
    // finally, make sure the data are there
    _vol->printWithFieldData(outfile);

    outfile << "_vol = " << _vol << endl;
}

/* TODO ...
// set data values for the field in the Volume (for all rays? for all sweeps?)
void SpreadSheetModel::setDataMissing(string fieldName, float missingDataValue)
{
  LOG(DEBUG) << "fieldName=" << fieldName << " setting to missing value " << missingDataValue;

  //const RadxField *field;
  //  field = _vol.getFieldFromRay(fieldName);  // <--- Why is this returning NULL
  // because the type is 
  // from debugger:  *((vol.getFieldFromRay("VEL"))->getDataSi16()+1)
  //field = _closestRay->getField(fieldName);

  // addField just modifies the name if there is a duplicate name,
  // so we can always add the field; we don't need to modify
  // an existing field.

  size_t nGates = data->size();
  size_t nGatesInRay = _closestRay->getNGates();
  if (nGates < nGatesInRay) {
    // TODO: expand, filling with missing Value
  }

  RadxField *field = _closestRay->getField(fieldName);

  if (field == NULL) {
    throw std::invalid_argument("no RadxField found ");
  } 

  //RadxField *fieldCopy = new RadxField(*field);

    vector<float> deref = *data;
    const Radx::fl32 *radxData = &deref[0];
    bool isLocal = true;  //?? not sure about this 
    field->setDataFl32(nGates, radxData, isLocal);
  
    // make sure the new data are there ...
    field->printWithData(cout);

    //cerr << "replacing data in field " <<  endl;
    //int result = _closestRay->replaceField(fieldCopy); 
    //if (result == -1)
    //  throw "Error replacing Field";
    //    _closestRay->addField("WIDTH", units, nGates, missingValue, radxData, isLocal);
    //} else {
    // replace the data values in the existing field
  // Radx::fl32 *data = field->getDataFl32();
  // how may gates?
  //  size_t nPoints = field->getNPoints();
  //  field->addDataFl64(nGates, &data[0]);
  //}

    // data should be copied, so free the memory
    // delete data;

    // again, make sure the data are there
    _closestRay->printWithFieldData(cout);

    //    _vol.addRay(_closestRay);


    _vol->loadRaysFromFields();
    //_vol->loadFieldsFromRays();
    //_vol->loadSweepInfoFromRays();

    std::ofstream outfile("/tmp/voldebug.txt");
    // finally, make sure the data are there
    _vol->printWithFieldData(outfile);

    outfile << "_vol = " << _vol << endl;
}
*/

// find the closest ray in the volume and set the internal variable _closestRay
void SpreadSheetModel::findClosestRay(float azimuth, float elevation) {
  LOG(DEBUG) << "enter azimuth = " << azimuth;

  vector<RadxRay *> rays = _vol->getRays();
  // find that ray
  bool foundIt = false;
  double minDiff = 1.0e99;
  int minIdx = 0;
  double delta = 0.1;  // TODO set this to the min diff between elevations/sweeps
  RadxRay *closestRayToEdit = NULL;
  vector<RadxRay *>::iterator r;
  r=rays.begin();
  int idx = 0;
  while(r<rays.end()) {
    RadxRay *rayr = *r;
    double diff = fabs(azimuth - rayr->getAzimuthDeg());
    if (diff > 180.0) {
      diff = fabs(diff - 360.0);
    }
    if (diff < minDiff) {
      if (abs(elevation - rayr->getElevationDeg()) <= delta) {
        foundIt = true;
        closestRayToEdit = *r;
        minDiff = diff;
        minIdx = idx;
      }
    }
    r += 1;
    idx += 1;
  }  
  if (!foundIt || closestRayToEdit == NULL) {
    throw "could not find closest ray";
    // errorMessage("ExamineEdit Error", "couldn't find closest ray");
  }

  LOG(DEBUG) << "Found closest ray: index = " << minIdx << " min diff = " << minDiff; // " pointer = " << closestRayToEdit;
  closestRayToEdit->print(cout); 
  _closestRay = closestRayToEdit;
  _closestRayIdx = minIdx;
}


/*
//////////////////////////////////////////////////                                                             
// set up read                                                                                                 
void SpreadSheetModel::_setupVolRead(RadxFile *file)
{


  //if (_params.debug >= Params::DEBUG_VERBOSE) {
  //  file.setDebug(true);
  //}
  //if (_params.debug >= Params::DEBUG_EXTRA) {
  file->setDebug(true);
  file->setVerbose(true);
  //}

  // TODO: we want to read the fields that are there; not have a predetermined list of fields
  //
  //for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
  //  const DisplayField *field = _fields[ifield];
  //  file.addReadField(field->getName());
  //}
  //

}
*/

///////////////////////////// 
// get data in archive mode
// returns 0 on success, -1 on failure
/*
int SpreadSheetModel::_getArchiveData(string inputPath)
{

  // set up file object for reading

  RadxFile file;
  _vol.clear();
  _setupVolRead(&file);


  //if (_archiveScanIndex >= 0 &&
  //    _archiveScanIndex < (int) _archiveFileList.size()) {

  //  string inputPath = _archiveFileList[_archiveScanIndex];

  //  if(_params.debug) {
  //    cerr << "  reading data file path: " << inputPath << endl;
  //    cerr << "  archive file index: " << _archiveScanIndex << endl;
  //  }
  //
  if (file.readFromPath(inputPath, _vol)) {
    string errMsg = "ERROR - Cannot retrieve archive data\n";
    errMsg += "PolarManager::_getArchiveData\n";
    errMsg += file.getErrStr() + "\n";
    errMsg += "  path: " + inputPath + "\n";
    cerr << errMsg;
    return -1;
  }

  //  }


  // set number of gates constant if requested 
   _vol.setNGatesConstant();
   _vol.loadFieldsFromRays();
  // compute the fixed angles from the rays   

  //_vol.computeFixedAnglesFromRays();


  //  if (_params.debug) {
  cerr << "----------------------------------------------------" << endl;
  cerr << "perform archive retrieval" << endl;
  cerr << "  read file: " << _vol.getPathInUse() << endl;
  cerr << "  nSweeps: " << _vol.getNSweeps() << endl;
  //cerr << "  guiIndex, fixedAngle: "
  //     << _sweepManager.getGuiIndex() << ", "
  //     << _sweepManager.getSelectedAngle() << endl;
  cerr << "----------------------------------------------------" << endl;
  //}

  //  _platform = _vol.getPlatform();

  return 0;

}

void SpreadSheetModel::initData(string fileName)
{
  _getArchiveData(fileName);
}
*/
