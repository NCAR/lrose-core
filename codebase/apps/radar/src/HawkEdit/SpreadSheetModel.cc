//
// SpreadSheetModel provides data in basic form, as ints, floats, strings
// SpreadSheetModel provides the interface to Radx utilities, and file I/O

#include <stdio.h>
#include <stdexcept>
#include "SpreadSheetModel.hh"
#include "DataModel.hh"
#include <toolsa/LogStream.hh>
#include <fstream>
#include <cmath>

SpreadSheetModel::SpreadSheetModel()
{

}

SpreadSheetModel::SpreadSheetModel(RadxRay *closestRay) // , RadxVol *dataVolume)
{
  _closestRay = closestRay;
  if (_closestRay == NULL) 
    cout << "in SpreadSheetModel, closestRay is NULL" << endl;
  else
   cout << "in SpreadSheetModel, closestRay is NOT  NULL" << endl;
  // closestRay->print(cout);
  //_vol = dataVolume;
  //_vol->loadFieldsFromRays();
}

void SpreadSheetModel::getRangeGeom(float *startRangeKm, float *gateSpacingKm) { 
  double startRange;
  double gateSpace;
    // TODO: maybe on construction, map to finest range geometry? then we can call 
  DataModel *dataModel = DataModel::Instance();
  dataModel->getPredomRayGeom(&startRange, &gateSpace);
  *startRangeKm = startRange;
  *gateSpacingKm = gateSpace;
}

vector<string> *SpreadSheetModel::getFields()
{
  vector<string> *fieldNames;
  if (_closestRay != NULL) {
    _closestRay->loadFieldNameMap();

    RadxRay::FieldNameMap fieldNameMap = _closestRay->getFieldNameMap();
    RadxRay::FieldNameMapIt it;
    fieldNames = new vector<string>;
    for (it = fieldNameMap.begin(); it != fieldNameMap.end(); it++) {
      fieldNames->push_back(it->first);
      cout << it->first << ':' << it->second << endl;
    }
  } else {
    DataModel *dataModel = DataModel::Instance();
    fieldNames = dataModel->getUniqueFieldNameList();
    //fieldNames = const_cast <vector<string>> (fieldNamesC);
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

    DataModel *dataModel = DataModel::Instance();
    /*
    size_t nRays = dataModel->getNRays();
    //vector<RadxRay *> rays = const_cast <vector<RadxRay *>> (dataModel->getRays());
    // ( size_t = (size_t + int) % size_t)  ==> TROUBLE!!!
    //int closestRay = (int) _closestRayIdx;

    size_t rayIdx = dataModel->getRayIndex(_closestRayIdx,
      offsetFromClosest, _currentSweepNumber); 

    //if (idx < 0) throw std::invalid_argument("requested ray index < 0"); 
    LOG(DEBUG) << "closestRayIdx=" << _closestRayIdx << " offsetFromClosest=" << offsetFromClosest 
      << " rayIdx=" << rayIdx;
      */
    size_t rayIdx = _getRayIdx(offsetFromClosest);      
    azimuth = dataModel->getRayAzimuthDeg(rayIdx);  
    //RadxRay *ray = rays.at(idx);
    //azimuth = ray->getAzimuthDeg(); 
  }
  return azimuth;
}

float SpreadSheetModel::getNyquistVelocityForRay(int offsetFromClosest) {
  float nyquistVelocity = -1.0;  // negative value indicates no value found
  if (offsetFromClosest == 0) {
    nyquistVelocity = _closestRay->getAzimuthDeg();
  } else {
    
    DataModel *dataModel = DataModel::Instance();
    /*
    size_t nRays = dataModel->getNRays();
    //vector<RadxRay *> rays = const_cast <vector<RadxRay *>> (dataModel->getRays());
    // ( size_t = (size_t + int) % size_t)  ==> TROUBLE!!!
    //int closestRay = (int) _closestRayIdx;

    size_t rayIdx = dataModel->getRayIndex(_closestRayIdx,
      offsetFromClosest, _currentSweepNumber); 

    //if (idx < 0) throw std::invalid_argument("requested ray index < 0"); 
    LOG(DEBUG) << "closestRayIdx=" << _closestRayIdx << " offsetFromClosest=" << offsetFromClosest 
      << " rayIdx=" << rayIdx;
      */
    size_t rayIdx = _getRayIdx(offsetFromClosest);
    nyquistVelocity = dataModel->getRayNyquistVelocityMps(rayIdx);  
  }
  return nyquistVelocity;
}

size_t SpreadSheetModel::_getRayIdx(int offsetFromClosest) {
    DataModel *dataModel = DataModel::Instance();
    size_t nRays = dataModel->getNRays();
    //vector<RadxRay *> rays = const_cast <vector<RadxRay *>> (dataModel->getRays());
    // ( size_t = (size_t + int) % size_t)  ==> TROUBLE!!!
    //int closestRay = (int) _closestRayIdx;

    size_t rayIdx = dataModel->getRayIndex(_closestRayIdx,
      offsetFromClosest, _currentSweepNumber); 

    //if (idx < 0) throw std::invalid_argument("requested ray index < 0"); 
    LOG(DEBUG) << "closestRayIdx=" << _closestRayIdx << " offsetFromClosest=" << offsetFromClosest 
      << " rayIdx=" << rayIdx;
    return rayIdx;
}

  void SpreadSheetModel::_setSweepNumber(int sweepNumber) {
    _currentSweepNumber = sweepNumber;
  }


/* moved to DataModel ...
size_t SpreadSheetModel::calculateRayIndex_f(size_t idx, size_t start, size_t end, int offset) {

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
*/
/* not used; does not work
size_t SpreadSheetModel::getRayIndex(int offsetFromClosest) {

    //  -v- HERE is the problem!! the number of rays is for the entire volume, not just one sweep!!
    size_t nRays = dataModel->getNRays(_currentSweepNumber);
    //size_t rayIdx = (_closestRayIdx + offsetFromClosest) % nRays;

    // ( size_t = (size_t + int) % size_t)  ==> TROUBLE!!!
    int closestRay = (int) _closestRayIdx;
    int idx = (closestRay + offsetFromClosest) % (int) nRays; // rays.size();
    if (idx < 0) throw std::invalid_argument("requested ray index < 0"); 
    LOG(DEBUG) << "closestRayIdx=" << _closestRayIdx << " offsetFromClosest=" << offsetFromClosest 
      << " idx=" << idx;


  return idx;
}
*/

// return a list of data values for the given
// field name
// vector<double>
vector<float> *SpreadSheetModel::getData(string fieldName, int offsetFromClosest)
{

  LOG(DEBUG) << "enter" << "offsetFromClosest = " << offsetFromClosest;
  LOG(DEBUG) << "_closestRayIdx = " << _closestRayIdx;
  DataModel *dataModel = DataModel::Instance();
  size_t rayIdx;  
  if (offsetFromClosest == 0) {
    rayIdx = _closestRayIdx;
  } else {
    //size_t nRays = dataModel->getNRays(); // fieldName, sweepAngle);
    //dataModel->getStartRayIndex
    rayIdx = dataModel->getRayIndex(_closestRayIdx,
     offsetFromClosest, _currentSweepNumber); 
  }
  vector<float> *dataVector = dataModel->getRayData(rayIdx, fieldName); // , _currentSweepNumber);

/*
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
*/
  LOG(DEBUG) << "exit";
  return dataVector;

}


/*
RadxVol SpreadSheetModel::getVolume() {
  return _vol;
}
*/

// set data values for the field in the Volume 
void SpreadSheetModel::setData(string fieldName, float azimuth, vector<float> *data)
{
  LOG(DEBUG) << "fieldName=" << fieldName << ", azimuth = " << azimuth;

  // addField just modifies the name if there is a duplicate name,
  // so we can always add the field; we don't need to modify
  // an existing field.

  size_t nGates = data->size();

  DataModel *dataModel = DataModel::Instance();
  dataModel->SetData(fieldName, 
            azimuth, _currentSweepNumber, data);

  bool debug = true;
  if (debug) {
    size_t rayIdx = dataModel->findClosestRay(azimuth, _currentSweepNumber);
    vector<float> *dataVector = dataModel->getRayData(rayIdx, fieldName); //  _currentSweepNumber);
  }

  /*
  size_t nGatesInRay = _closestRay->getNGates();
  if (nGates < nGatesInRay) {
    // TODO: expand, filling with missing Value
  }

  // TODO: get the correct azimuth, and sweep number from  elevation
  RadxField *field = _closestRay->getField(fieldName);

  if (field == NULL) {
    throw std::invalid_argument("no RadxField found ");
  } 

  vector<float> deref = *data;
  const Radx::fl32 *radxData = &deref[0];
  bool isLocal = true;  //?? not sure about this 
  field->setDataFl32(nGates, radxData, isLocal);
  
  // make sure the new data are there ...
  field->printWithData(cout);
  
  // data should be copied, so free the memory
  // delete data;
  
  // again, make sure the data are there
  _closestRay->printWithFieldData(cout);
  
  //_vol->loadRaysFromFields();
  
  //std::ofstream outfile("/tmp/voldebug.txt");
  // finally, make sure the data are there
  //_vol->printWithFieldData(outfile);
  
  //outfile << "_vol = " << _vol << endl;
  */
  LOG(DEBUG) << "exit";

}


// set data values for the field in the Volume (for all rays? for all sweeps?)
void SpreadSheetModel::setDataMissing(string fieldName, float missingDataValue)
{
  LOG(DEBUG) << "fieldName=" << fieldName << " setting to missing value " << missingDataValue;


  DataModel *dataModel = DataModel::Instance();
  float dummy = 0.0;
  dataModel->SetData(fieldName, dummy);
/*
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

    vector<float> deref = *data;
    const Radx::fl32 *radxData = &deref[0];
    bool isLocal = true;  //?? not sure about this 
    field->setDataFl32(nGates, radxData, isLocal);
  
    // make sure the new data are there ...
    field->printWithData(cout);

    // data should be copied, so free the memory
    // delete data;

    // again, make sure the data are there
    _closestRay->printWithFieldData(cout);

    _vol->loadRaysFromFields();

    std::ofstream outfile("/tmp/voldebug.txt");
    // finally, make sure the data are there
    _vol->printWithFieldData(outfile);

    outfile << "_vol = " << _vol << endl;
*/
}




// find the closest ray in the volume and set the internal variable _closestRay
// and the internal variable _closestRayIdx
//
void SpreadSheetModel::setClosestRay(float azimuth, float elevation) {
  LOG(DEBUG) << "enter azimuth = " << azimuth;

  DataModel *dataModel = DataModel::Instance();
  int sweepNumber = dataModel->getSweepNumber(elevation);
  _setSweepNumber(sweepNumber);
  size_t closestRayIdx = dataModel->findClosestRay(azimuth, _currentSweepNumber); // elevation);

  /* moved to DataModel ...
  const vector<RadxRay *> rays = dataModel->getRays();
  // find that ray
  bool foundIt = false;
  double minDiff = 1.0e99;
  int minIdx = 0;
  double delta = 0.1;  // TODO set this to the min diff between elevations/sweeps
  RadxRay *closestRayToEdit = NULL;
  vector<RadxRay *>::const_iterator r;
  r=rays.begin();
  int idx = 0;
  while(r<rays.end()) {
    RadxRay *rayr = *r;
    double diff = fabs(azimuth - rayr->getAzimuthDeg());
    if (diff > 180.0) {
      diff = fabs(diff - 360.0);
    }
    if (diff < minDiff) {
      if (abs(_currentElevation - rayr->getElevationDeg()) <= delta) {
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
    throw std::invalid_argument("could not find closest ray");
    // errorMessage("ExamineEdit Error", "couldn't find closest ray");
  }

  LOG(DEBUG) << "Found closest ray: index = " << minIdx << " min diff = " << minDiff; // " pointer = " << closestRayToEdit;
  closestRayToEdit->print(cout); 
  */

  // _closestRay = const_cast <RadxRay *> (closestRayToEdit);
  _closestRay = dataModel->getRay(closestRayIdx);
  _closestRayIdx = closestRayIdx;
  LOG(DEBUG) << "_closestRayIdx = " << _closestRayIdx 
    << " azimuth = " << _closestRay->getAzimuthDeg() << " vs. requested az " << azimuth;
  LOG(DEBUG) << "exit";
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
