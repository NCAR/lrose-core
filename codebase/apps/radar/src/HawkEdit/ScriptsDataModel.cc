#include "ScriptsDataModel.hh"  
#include <toolsa/LogStream.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxCfactors.hh>
#include <cmath>

using namespace std;

/*ScriptsDataModel *ScriptsDataModel::_instance = NULL;

ScriptsDataModel *ScriptsDataModel::Instance() {
  	if (_instance == NULL) {
  		_instance = new ScriptsDataModel();
  	}
  	return _instance;
}
*/

ScriptsDataModel::~ScriptsDataModel() {}

/*
void ScriptsDataModel::setData(RadxVol *vol) {
  _vol = vol;
}
*/


int ScriptsDataModel::openRead(string &inputPath, int sweepNumber,
  vector<string> *fieldNames, bool debug_verbose, bool debug_extra) {

  LOG(DEBUG) << "enter " << inputPath;
   
  try {
    //int sweep_number = _sweepController->getSelectedNumber();
    readData(inputPath, *fieldNames, sweepNumber,
      debug_verbose, debug_extra);
  } catch (const string &errMsg) {
    return -1;
  }
  cerr << "ScriptsDataModel::init exit" << endl;
  LOG(DEBUG) << "exit";
  return 0;
}


bool ScriptsDataModel::okToStream() {
  return _okToStream;
}

// return data for the field, at the sweep and ray index
const vector<float> *ScriptsDataModel::GetData(string fieldName,
              int rayIdx, int sweepIdx)  {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx
       << " sweepIdx=" << sweepIdx;

  // sweep numbers are 1-based in RadxVol, not zero based, so, add one to the index.
  //sweepIdx += 1;

  _vol->loadRaysFromFields();

  //RadxSweep *sweep = _vol->getSweepByNumber(sweepIdx); // NOT by index!! Grrh!
  vector<RadxSweep *> volSweeps = _vol->getSweeps();
  RadxSweep *sweep = volSweeps.at(sweepIdx);
  if (sweep == NULL)
    throw std::invalid_argument("bad sweep index");

  size_t startRayIndex = sweep->getStartRayIndex();

  const RadxField *field;

  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  
  if ((rayIdx > sweep->getEndRayIndex()) ||
      (rayIdx < startRayIndex)) {
    string msg = "ScriptsDataModel::GetData rayIdx outside sweep ";
    msg.append(std::to_string(rayIdx));
    throw msg;
  }


  RadxRay *ray = rays.at(rayIdx); // startRayIndex + rayIdx);
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
const vector<float> *ScriptsDataModel::GetData(string fieldName,
              int rayIdx, int sweepIdx)  {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx
       << " sweepIdx=" << sweepIdx;

  _vol->loadRaysFromFields();
  
  const RadxField *field;

  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol->getRays();
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

void ScriptsDataModel::SetDataByIndex(string &fieldName, 
            int rayIdx, int sweepIdx, vector<float> *fieldData) { 

  // What is being returned? the name of the new field in the model that
  // contains the results.

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;



  _vol->loadRaysFromFields(); // loadFieldsFromRays();


    LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx
       << " sweepIdx=" << sweepIdx;

  // sweep numbers are 1-based in RadxVol, not zero based, so, add one to the index.
  sweepIdx += 1;

  RadxSweep *sweep = _vol->getSweepByNumber(sweepIdx);


  SetData(fieldName, rayIdx, sweep, fieldData);
  /* 
  if (sweep == NULL)
    throw std::invalid_argument("bad sweep index");

  size_t startRayIndex = sweep->getStartRayIndex();

  const RadxField *field;

  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol->getRays();
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

void ScriptsDataModel::SetData(string &fieldName, 
            int rayIdx, RadxSweep *sweep, vector<float> *fieldData) { 

  // What is being returned? the name of the new field in the model that
  // contains the results.

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;

  _vol->loadRaysFromFields(); // loadFieldsFromRays();

    LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;

  //if (sweep == NULL)
  //  throw std::invalid_argument("bad sweep index");

  //size_t startRayIndex = sweep->getStartRayIndex();

  //const 
  RadxField *field;

  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol->getRays();
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

void ScriptsDataModel::SetData(string &fieldName, 
            float azimuth, float sweepAngle, vector<float> *fieldData) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " ray =" << azimuth
       << " sweep =" << sweepAngle;

  // this is the closest ray for the sweep angle
  int rayIdx = (int) findClosestRay(azimuth, sweepAngle);
  //_vol->loadRaysFromFields();

  //RadxSweep *sweep = _vol->getSweepByFixedAngle(sweepAngle);
  RadxSweep *dummyValue = nullptr;

  SetData(fieldName, rayIdx, dummyValue, fieldData);
  LOG(DEBUG) << "exit";
}

void ScriptsDataModel::SetData(string &fieldName, float value) {
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
void ScriptsDataModel::RemoveField(string &fieldName) {
  int result = _vol->removeField(fieldName);
  if (result != 0) {
    string msg = "failed to remove field: ";
    msg.append(fieldName); 
    throw std::invalid_argument(msg);
  }
}

// remove field from ray
void ScriptsDataModel::RemoveField(size_t rayIdx, string &fieldName) {
  RadxRay *ray = getRay(rayIdx);
  if (ray != NULL) {
    int result = ray->removeField(fieldName);
    if (result != 0) {
      string msg = "failed to remove field from ray: ";
      msg.append(fieldName); 
      throw std::invalid_argument(msg);
    }
  }
}

void ScriptsDataModel::regularizeRays() {
  bool nFieldsConstantPerRay = true;
  _vol->loadFieldsFromRays(nFieldsConstantPerRay);
  _vol->loadRaysFromFields();
}

RadxVol *ScriptsDataModel::getRadarVolume(string path, vector<string> *fieldNames,
  bool debug_verbose, bool debug_extra) {

  LOG(DEBUG) << "enter";
  // set up file object for reading

  RadxVol *vol = new RadxVol();

  cerr << "before " << endl;
  RadxFile file;

  _setupVolRead(file, *fieldNames, debug_verbose, debug_extra);
   
  LOG(DEBUG) << "  reading data file path: " << path;    
    
  if (file.readFromPath(path, *vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "ScriptsDataModel::readData\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + path + "\n";
      cerr << errMsg;
      throw errMsg;
  } 
  cerr << "after " << endl;

  vol->convertToFl32();

  // adjust angles for elevation surveillance if needed
  
  vol->setAnglesForElevSurveillance();
  
  // compute the fixed angles from the rays
  // so that we reflect reality
  
  vol->computeFixedAnglesFromRays();

    LOG(DEBUG) << "----------------------------------------------------";
    LOG(DEBUG) << "perform archive retrieval";
    LOG(DEBUG) << "  read file: " << vol->getPathInUse();
    LOG(DEBUG) << "  nSweeps: " << vol->getNSweeps();
   // LOG(DEBUG) << "  guiIndex, fixedAngle: " 
   //      << _sweepManager.getGuiIndex() << ", "
   //      << _sweepManager.getSelectedAngle();
    LOG(DEBUG) << "----------------------------------------------------";

  _cacheMetaDataValid = false;

  LOG(DEBUG) << "exit";

  return vol;
}

// add a parameter for the sweep number
RadxVol *ScriptsDataModel::getRadarVolume(string path, vector<string> *fieldNames,
  int sweepNumber,
  bool debug_verbose, bool debug_extra) {

  LOG(DEBUG) << "enter";
  // set up file object for reading

  RadxVol *vol = new RadxVol();

  cerr << "before " << endl;
  RadxFile file;

  _setupVolRead(file, *fieldNames, sweepNumber, debug_verbose, debug_extra);
   
  LOG(DEBUG) << "  reading data file path: " << path;    
    
  if (file.readFromPath(path, *vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "ScriptsDataModel::getRadarVolume\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + path + "\n";
      cerr << errMsg;
      throw errMsg;
  } 
  cerr << "after " << endl;

  vol->convertToFl32();

  // adjust angles for elevation surveillance if needed
  
  vol->setAnglesForElevSurveillance();
  
  // compute the fixed angles from the rays
  // so that we reflect reality
  
  vol->computeFixedAnglesFromRays();

    LOG(DEBUG) << "----------------------------------------------------";
    LOG(DEBUG) << "perform archive retrieval";
    LOG(DEBUG) << "  read file: " << vol->getPathInUse();
    LOG(DEBUG) << "  nSweeps: " << vol->getNSweeps();
   // LOG(DEBUG) << "  guiIndex, fixedAngle: " 
   //      << _sweepManager.getGuiIndex() << ", "
   //      << _sweepManager.getSelectedAngle();
    LOG(DEBUG) << "----------------------------------------------------";

  _cacheMetaDataValid = false;

  LOG(DEBUG) << "exit";

  return vol;
}

void ScriptsDataModel::getRayData(string path, vector<string> &fieldNames,
  int sweepNumber) {
  readData(path, fieldNames, sweepNumber);
}

void ScriptsDataModel::readData(string path, vector<string> &fieldNames,
  bool debug_verbose, bool debug_extra) {

  LOG(DEBUG) << "enter";
  // set up file object for reading

  cerr << "before " << endl;
  RadxFile file;
  if (_vol != NULL) delete _vol;
  _vol = new RadxVol();
  //_vol->clear();
  _setupVolRead(file, fieldNames, debug_verbose, debug_extra);
   
  LOG(DEBUG) << "  reading data file path: " << path;    
    
  if (file.readFromPath(path, *_vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "ScriptsDataModel::readData\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + path + "\n";
      cerr << errMsg;
      throw errMsg;
  } 
  cerr << "after " << endl;

  //if (file.getFileFormat() == RadxFile::FILE_FORMAT_NEXRAD_AR2)
  //  _okToStream = true;
  //else
  //  _okToStream = false;

  // check for fields read
  //bool nFieldsConstantPerRay = true;
  //_vol->loadFieldsFromRays(nFieldsConstantPerRay);

  /*
  vector<string>::iterator it;
  for (it=fieldNames.begin(); it != fieldNames.end(); ++it) {
    RadxField *field = _vol->getField(*it);
    if (field == NULL) {
        string errMsg = "ERROR - No field read\n";
        errMsg += "ScriptsDataModel::readData\n";
        errMsg += " field: " + *it + "\n";
        errMsg += "  path: " + path + "\n";
        cerr << errMsg;
        //throw errMsg;
    } else {
      cerr << "read field " << *it << endl;
    }
  }
  // or if no field found when requested send error message???
  */

  _vol->convertToFl32();

  // adjust angles for elevation surveillance if needed
  // TODO: this step takes FOREVER! See if it can be skipped
  //_vol->setAnglesForElevSurveillance();
  
  // compute the fixed angles from the rays
  // so that we reflect reality
  
  _vol->computeFixedAnglesFromRays();

    LOG(DEBUG) << "----------------------------------------------------";
    LOG(DEBUG) << "perform archive retrieval";
    LOG(DEBUG) << "  read file: " << _vol->getPathInUse();
    LOG(DEBUG) << "  nSweeps: " << _vol->getNSweeps();
   // LOG(DEBUG) << "  guiIndex, fixedAngle: " 
   //      << _sweepManager.getGuiIndex() << ", "
   //      << _sweepManager.getSelectedAngle();
    LOG(DEBUG) << "----------------------------------------------------";

  // printAzimuthInRayOrder();

  _currentFilePath = path;

  LOG(DEBUG) << "exit";
}



void ScriptsDataModel::readData(string path, vector<string> &fieldNames,
  int sweepNumber,
	bool debug_verbose, bool debug_extra) {

  LOG(DEBUG) << "enter";
  // set up file object for reading

  //cerr << "comparing " << path << " with \n" <<
  //        "          " << _currentFilePath << endl;

  //if (_currentFilePath.compare(path) == 0) {
  //  // don't reread the same file
  //  return;
  //}

  cerr << "before " << endl;
  RadxFile file;
  if (_vol != NULL) delete _vol;
  _vol = new RadxVol();
  //_vol->clear();
  _setupVolRead(file, fieldNames, sweepNumber, debug_verbose, debug_extra);
   
  LOG(DEBUG) << "  reading data file path: " << path;    
    
  if (file.readFromPath(path, *_vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "ScriptsDataModel::readData\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + path + "\n";
      cerr << errMsg;
      throw errMsg;
  } 
  cerr << "after " << endl;

  //if (file.getFileFormat() == RadxFile::FILE_FORMAT_NEXRAD_AR2)
  //  _okToStream = true;
  //else
  //  _okToStream = false;

  // check for fields read
  //bool nFieldsConstantPerRay = true;
  //_vol->loadFieldsFromRays(nFieldsConstantPerRay);

  /*
  vector<string>::iterator it;
  for (it=fieldNames.begin(); it != fieldNames.end(); ++it) {
    RadxField *field = _vol->getField(*it);
    if (field == NULL) {
        string errMsg = "ERROR - No field read\n";
        errMsg += "ScriptsDataModel::readData\n";
        errMsg += " field: " + *it + "\n";
        errMsg += "  path: " + path + "\n";
        cerr << errMsg;
        //throw errMsg;
    } else {
      cerr << "read field " << *it << endl;
    }
  }
  // or if no field found when requested send error message???
  */

  _vol->convertToFl32();

  // adjust angles for elevation surveillance if needed
  
  _vol->setAnglesForElevSurveillance();
  
  // compute the fixed angles from the rays
  // so that we reflect reality
  
  _vol->computeFixedAnglesFromRays();

    LOG(DEBUG) << "----------------------------------------------------";
    LOG(DEBUG) << "perform archive retrieval";
    LOG(DEBUG) << "  read file: " << _vol->getPathInUse();
    LOG(DEBUG) << "  nSweeps: " << _vol->getNSweeps();
   // LOG(DEBUG) << "  guiIndex, fixedAngle: " 
   //      << _sweepManager.getGuiIndex() << ", "
   //      << _sweepManager.getSelectedAngle();
    LOG(DEBUG) << "----------------------------------------------------";

  // printAzimuthInRayOrder();

  _currentFilePath = path;

  LOG(DEBUG) << "exit";
}


// TODO: Do I need a DataController? to switch between Radx* data types and
// standard C++/Qt data types?

RadxTime ScriptsDataModel::getStartTimeSecs() {
	return _vol->getStartTimeSecs();
}

RadxTime ScriptsDataModel::getEndTimeSecs() {
  return _vol->getEndTimeSecs();
}

// TODO: this could be faster
void ScriptsDataModel::_selectFieldsNotInVolume(vector<string> *allFieldNames) {
  vector<string> *primaryFieldNames = getUniqueFieldNameList();
  vector<string>::iterator it;
  for (it = primaryFieldNames->begin(); it != primaryFieldNames->end(); ++it) {
    int i=0; 
    bool found = false;
    while (i<allFieldNames->size() && !found) {
      if (allFieldNames->at(i).compare(*it) != string::npos) {
        found = true;
        // remove from list
        allFieldNames->erase(allFieldNames->begin()+i);
        cerr << "not reading " << *it << endl;
      }
      i++;
    }
  }
}

void ScriptsDataModel::_selectFieldsNotInCurrentVersion(
  vector<string> *currentVersionFieldNames, vector<string> *allFieldNames) {

  vector<string>::iterator it;
  for (it = currentVersionFieldNames->begin(); it != currentVersionFieldNames->end(); ++it) {
    int i=0; 
    bool found = false;
    while (i<allFieldNames->size() && !found) {
      if (allFieldNames->at(i).compare(*it) != string::npos) {
        found = true;
        // remove from list
        allFieldNames->erase(allFieldNames->begin()+i);
        cerr << "not reading " << *it << endl;
      }
      i++;
    }
  }  

}

Radx::PrimaryAxis_t ScriptsDataModel::getPrimaryAxis() {
  return _vol->getPrimaryAxis();
}


/* append the edited sweep (those fields read in memory) to
// tho current version of the data file
// Q: What if the fields are different? maybe merge fields before calling append??
void ScriptsDataModel::appendSweep(string currentVersionPath) {


  // open currentVersionPath
  // read metadata, and verify current sweep is compatible, geom, fields, etc.
  // update metadata as needed???
  // seek to end of file,
  // write sweep

  // read the source_path into a separate volume, then merge the fields and 
  //_volSecondary = read
  // write to the dest_path
  vector<string> *currentVersionFieldNames = getPossibleFieldNames(currentVersionPath);
  vector<string> *allPossibleFieldNames = getPossibleFieldNames(originalSourcePath);
  _selectFieldsNotInCurrentVersion(currentVersionFieldNames, allPossibleFieldNames);

  // allPossibleFieldNames now contains only the fields NOT in current version of file

  bool debug_verbose = false;
  bool debug_extra = false;
  
  RadxVol *primaryVol = getRadarVolume(currentVersionPath, currentVersionFieldNames,
     debug_verbose, debug_extra);

  RadxVol *secondaryVol = getRadarVolume(originalSourcePath, allPossibleFieldNames,
     debug_verbose, debug_extra);

  // ----
    // merge the primary and seconday volumes, using the primary
    // volume to hold the merged data
    
  // add secondary rays to primary vol

  int maxSweepNum = 0;
  vector<RadxRay *> &pRays = primaryVol->getRays();
  const vector<RadxRay *> &sRays = secondaryVol->getRays();
  for (size_t iray = 0; iray < pRays.size(); iray++) {
    RadxRay &pRay = *pRays[iray];
    const RadxRay *sRay = sRays[iray];
    // for each field in secondary vol
    for (size_t ifield = 0; ifield < allPossibleFieldNames->size(); ifield++) {
      string fieldName = allPossibleFieldNames->at(ifield);
      const RadxField *sfield = sRay->getField(fieldName);
      RadxField *copyField = new RadxField();
      *copyField = *sfield;
      // Add a previously-created field to the ray. The field must have
      // been dynamically allocted using new(). Memory management for
      //  this field passes to the ray, which will free the field object
      // using delete().
      // void addField(RadxField *field);
      pRay.addField(copyField);
    } // ifield
  } // iray

  // finalize the volume

  primaryVol->setPackingFromRays();
  primaryVol->loadVolumeInfoFromRays();
  primaryVol->loadSweepInfoFromRays();
  primaryVol->remapToPredomGeom();
  
  delete allPossibleFieldNames;
  delete currentVersionFieldNames;
  delete secondaryVol;

  return primaryVol;

}
*/


// merge edited fields (those read in memory) with
// those fields in the original data file
RadxVol *ScriptsDataModel::mergeDataFields(string originalSourcePath) {


  // read the source_path into a separate volume, then merge the fields and 
  //_volSecondary = read
  // write to the dest_path

  vector<string> *allPossibleFieldNames = getPossibleFieldNames(originalSourcePath);
  _selectFieldsNotInVolume(allPossibleFieldNames);
  // allPossibleFieldNames is now filtered to remove fields in the current selected file

  bool debug_verbose = false;
  bool debug_extra = false;

  // make a copy of the selected radar volume
  RadxVol *primaryVol = new RadxVol();
  *primaryVol = *_vol;

  RadxVol *secondaryVol = getRadarVolume(originalSourcePath, allPossibleFieldNames,
     debug_verbose, debug_extra);

  // ----
    // merge the primary and seconday volumes, using the primary
    // volume to hold the merged data
    
  // add secondary rays to primary vol

  int maxSweepNum = 0;
  vector<RadxRay *> &pRays = primaryVol->getRays();
  const vector<RadxRay *> &sRays = secondaryVol->getRays();
  for (size_t iray = 0; iray < pRays.size(); iray++) {
    RadxRay &pRay = *pRays[iray];
    const RadxRay *sRay = sRays[iray];
    // for each field in secondary vol
    for (size_t ifield = 0; ifield < allPossibleFieldNames->size(); ifield++) {
      string fieldName = allPossibleFieldNames->at(ifield);
      const RadxField *sfield = sRay->getField(fieldName);
      RadxField *copyField = new RadxField();
      *copyField = *sfield;
      // Add a previously-created field to the ray. The field must have
      // been dynamically allocted using new(). Memory management for
      //  this field passes to the ray, which will free the field object
      // using delete().
      // void addField(RadxField *field);
      pRay.addField(copyField);
    } // ifield
  } // iray

  // finalize the volume

  primaryVol->setPackingFromRays();
  primaryVol->loadVolumeInfoFromRays();
  primaryVol->loadSweepInfoFromRays();
  primaryVol->remapToPredomGeom();

  // --
  
  delete secondaryVol;

  delete allPossibleFieldNames;
  //delete currentVersionFieldNames;

  return primaryVol;

}

// merge edited fields (those read in memory) with
// those fields in the original data file
// returns merged radar volume
RadxVol *ScriptsDataModel::mergeDataFields(string currentVersionPath, string originalSourcePath) {


  // read the source_path into a separate volume, then merge the fields and 
  //_volSecondary = read
  // write to the dest_path
  vector<string> *currentVersionFieldNames = getPossibleFieldNames(currentVersionPath);
  vector<string> *allPossibleFieldNames = getPossibleFieldNames(originalSourcePath);
  _selectFieldsNotInCurrentVersion(currentVersionFieldNames, allPossibleFieldNames);

  // allPossibleFieldNames now contains only the fields NOT in current version of file

  bool debug_verbose = false;
  bool debug_extra = false;
  
  RadxVol *primaryVol = getRadarVolume(currentVersionPath, currentVersionFieldNames,
     debug_verbose, debug_extra);

  RadxVol *secondaryVol = getRadarVolume(originalSourcePath, allPossibleFieldNames,
     debug_verbose, debug_extra);

  // ----
    // merge the primary and seconday volumes, using the primary
    // volume to hold the merged data
    
  // add secondary rays to primary vol

  int maxSweepNum = 0;
  vector<RadxRay *> &pRays = primaryVol->getRays();
  const vector<RadxRay *> &sRays = secondaryVol->getRays();
  for (size_t iray = 0; iray < pRays.size(); iray++) {
    RadxRay &pRay = *pRays[iray];
    const RadxRay *sRay = sRays[iray];
    // for each field in secondary vol
    for (size_t ifield = 0; ifield < allPossibleFieldNames->size(); ifield++) {
      string fieldName = allPossibleFieldNames->at(ifield);
      const RadxField *sfield = sRay->getField(fieldName);
      RadxField *copyField = new RadxField();
      *copyField = *sfield;
      // Add a previously-created field to the ray. The field must have
      // been dynamically allocted using new(). Memory management for
      //  this field passes to the ray, which will free the field object
      // using delete().
      // void addField(RadxField *field);
      pRay.addField(copyField);
    } // ifield
  } // iray

  // finalize the volume

  primaryVol->setPackingFromRays();
  primaryVol->loadVolumeInfoFromRays();
  primaryVol->loadSweepInfoFromRays();
  primaryVol->remapToPredomGeom();
  
  delete allPossibleFieldNames;
  delete currentVersionFieldNames;
  delete secondaryVol;

  return primaryVol;

}

// use to merge data with currently selected data file
void ScriptsDataModel::writeWithMergeData(string outputPath, string originalSourcePath) {

    RadxVol *mergedVolume = mergeDataFields(originalSourcePath);
    writeData(outputPath, mergedVolume);
    delete mergedVolume;
}

// use to merge data with a data file NOT currently selected
void ScriptsDataModel::writeWithMergeData(string outputPath, string currentVersionPath, string originalSourcePath) {

    RadxVol *mergedVolume = mergeDataFields(currentVersionPath, originalSourcePath);
    writeData(outputPath, mergedVolume);
    delete mergedVolume;
}

// NOTE: side effect of changing the class variable _currentFilePath
void ScriptsDataModel::writeData(string path) {

    // move the data for each ray,  so that each field variable has contiguous memory.
    bool nFieldsConstantPerRay = true;
    _vol->loadFieldsFromRays(nFieldsConstantPerRay);

    RadxFile outFile;

    LOG(DEBUG) << "writing to file " << path;
    int result = outFile.writeToPath(*_vol, path);
    // Returns 0 on success, -1 on failure
    //
    // Use getErrStr() if error occurs
    // Use getPathInUse() for path written
    if (result != 0) {
      string errStr = outFile.getErrStr();
      throw std::invalid_argument(errStr);
    }
    _currentFilePath = path;
}

// no side effects, just writes the radar volume to the path
void ScriptsDataModel::writeData(string path, RadxVol *vol) {
    RadxFile outFile;

    LOG(DEBUG) << "writing to file " << path;
    int result = outFile.writeToPath(*vol, path);
    // Returns 0 on success, -1 on failure
    //
    // Use getErrStr() if error occurs
    // Use getPathInUse() for path written
    if (result != 0) {
      string errStr = outFile.getErrStr();
      throw std::invalid_argument(errStr);
    }
}

int ScriptsDataModel::mergeDataFiles(string dest_path, string source_path, string original_path) {

  writeWithMergeData(dest_path, source_path, original_path);
  return 0;
}

void ScriptsDataModel::update() {

}

void ScriptsDataModel::renameField(string currentName, string newName) {
  vector<RadxRay *> rays = _vol->getRays();	
  // for each ray, 
  vector<RadxRay *>::iterator it;
  for (it=rays.begin(); it != rays.end(); ++it) {
     // renameField(oldName, newName);
    (*it)->renameField(currentName, newName);
    // loadFieldNameMap
    (*it)->loadFieldNameMap();
  }
}

void ScriptsDataModel::renameField(size_t rayIdx, string currentName, string newName) {
  RadxRay *ray = getRay(rayIdx);

  // renameField(oldName, newName);
  ray->renameField(currentName, newName);
  // loadFieldNameMap
  ray->loadFieldNameMap();

}

// copy from one field to another field 
// overwrite destination
void ScriptsDataModel::copyField(size_t rayIdx, string fromFieldName, string toFieldName) {
  RadxRay *ray = getRay(rayIdx);
  //vector<RadxRay *> rays = _vol->getRays();  
  // for each ray, 
  //vector<RadxRay *>::iterator it;
  //for (it=rays.begin(); it != rays.end(); ++it) {
     // replaceField(RadxField *field);
    //RadxField *srcField = fetchDataField(*it, fromFieldName);
    RadxField *srcField = fetchDataField(ray, fromFieldName);
    if (srcField != NULL) {
      Radx::fl32 *src = srcField->getDataFl32();

      //RadxField *dstField = fetchDataField(*it, toFieldName);
      RadxField *dstField = fetchDataField(ray, toFieldName);
      Radx::fl32 *dst = dstField->getDataFl32();

      size_t nbytes = ray->getNGates();
      //      #include <string.h>
      // void *memcpy(void *restrict dst, const void *restrict src, size_t n);
      memcpy(dst, src, nbytes*sizeof(Radx::fl32));
    }
  //}
}

// copy from one field to another field 
void ScriptsDataModel::copyField2(size_t rayIdx, string fromFieldName, string toFieldName) {
  RadxRay *ray = getRay(rayIdx);
  RadxField *srcField = fetchDataField(ray, fromFieldName);
  if (srcField != NULL) {
    RadxField *copy = new RadxField(*srcField);
    copy->setName(toFieldName);
    ray->addField(copy);
  }
}

bool ScriptsDataModel::fieldExists(size_t rayIdx, string fieldName) {
  RadxRay *ray = getRay(rayIdx);
  try {
    RadxField *field = fetchDataField(ray, fieldName);
    if (field != NULL) return true;
    else return false;
  } catch (std::invalid_argument &ex) {
    return false;
  }
}

RadxField *ScriptsDataModel::fetchDataField(RadxRay *ray, string &fieldName) {
  if (fieldName.length() <= 0) {
    cerr << "fieldName is empty!!" << endl;
  }
  //const vector<RadxRay *> rays = _vol->getRays();
  //if ((ray < rays.at(0) || (ray > rays.at(rays.size()-1)))) {
    //throw std::invalid_argument("ray is out of bounds!");
  //  cerr << "ray is out of bounds!";
  //}
  _vol->loadRaysFromFields();
  //ray->loadFieldNameMap();
  //RadxRay::FieldNameMap fieldNameMap = ray->getFieldNameMap();
  //if (fieldNameMap.size() <= 0)
  //  throw std::invalid_argument("fieldNameMap is zero!");
  vector<RadxField *> fields;
  try {
    fields = ray->getFields();
  
  // dataField = ray->getField(fieldName);  often this doesn't work, and
  // throws an vector exception that cannot be trapped.  Sometimes the
  // field name map is built and sometimes not, so avoid using this function.
  } catch (std::exception &ex) {
    string msg = "ScriptsDataModel::fetchDataField unknown error occurred: ";
    msg.append(fieldName);
    throw std::invalid_argument(msg);
  }

//  if (fields == NULL) {
//    return NULL;
//  }
  if (fields.size() <=0) {
    return NULL;
  }

  RadxField *dataField = NULL;
  vector<RadxField *>::iterator it = fields.begin();
  bool found = false;
  while (!found && it != fields.end()) {
    RadxField *f = *it;
    if (f->getName().compare(fieldName) == 0) {
      found = true;
      dataField = f;
    }
    ++it;
  }

  if (dataField == NULL) {
    string msg = "ScriptsDataModel::fetchDataField No field found in ray: ";
    msg.append(fieldName);
    throw std::invalid_argument(msg);
  }

  return dataField; 
}
/*
const RadxField *ScriptsDataModel::fetchDataField(const RadxRay *ray, string &fieldName) {
  ray->loadFieldNameMap();
  RadxField *foundField = NULL;
  vector<RadxField *> fields = ray->getFields();
  for (size_t ii = 0; ii < fields.size(); ii++) {
    string name = fields[ii]->getName();
    if (name.compare(fieldName) == 0)
       foundField = fields[ii];
   }
   if (foundField == NULL) {
      string msg = "ScriptsDataModel::fetchDataField No field found in ray: ";
      msg + fieldName;
      throw std::invalid_argument(msg);
   }

   return foundField;
   




  std::remove_const<RadxRay *>::type ray2; 
  const FieldNameMap fieldNameMap = ray->getFieldNameMap();
  if (fieldNameMap == NULL) {
    cerr << "fieldNameMap is NULL" << endl;
  }
  ray2->loadFieldNameMap();
  const RadxField *dataField;
  try {
    dataField = ray2->getField(fieldName);
  } catch (std::exception &ex) {
    string msg = "ScriptsDataModel::fetchDataField unknown error occurred: ";
    msg + fieldName;
    throw std::invalid_argument(msg);
  }
  if (dataField == NULL) {
    string msg = "ScriptsDataModel::fetchDataField No field found in ray: ";
    msg + fieldName;
    throw std::invalid_argument(msg);
  }
  return dataField; 
  
}
*/

const float *ScriptsDataModel::fetchData(RadxRay *ray, string &fieldName) {

    RadxField *dataField = fetchDataField(ray, fieldName);
    if (dataField != NULL)
      return dataField->getDataFl32();
    else
      return NULL;
}

// total number of rays in volume, for all sweeps
size_t ScriptsDataModel::getNRays() { // string fieldName, double sweepAngle) {
  size_t nRays = 0;
  if (_vol != NULL) {
    _vol->loadRaysFromFields();
    //const RadxField *field;
    vector<RadxRay *>  &rays = _vol->getRays();
    nRays = rays.size();
  }
  return nRays;
}

// get the number of rays for a sweep
size_t ScriptsDataModel::getNRays(int sweepNumber) {
  _vol->loadRaysFromFields();
  RadxSweep *sweep = _vol->getSweepByNumber(sweepNumber);
  if (sweep == NULL) {
    throw std::invalid_argument("ScriptsDataModel::getNRays: no sweep found");
  }
  size_t nRays = sweep->getNRays();
  return nRays;
}

// get the number of rays for a sweep
size_t ScriptsDataModel::getNRaysSweepIndex(int sweepIndex) {
  _vol->loadRaysFromFields();
  const vector<RadxSweep *> sweeps = _vol->getSweeps();
  RadxSweep *sweep = sweeps.at(sweepIndex); 
  if (sweep == NULL) {
    throw std::invalid_argument("ScriptsDataModel::getNRaysSweepIndex: bad sweep index");
  }
  size_t nRays = sweep->getNRays();
  return nRays;
}

// get the first ray for a sweep
size_t ScriptsDataModel::getFirstRayIndex(int sweepIndex) {
  if (sweepIndex < 0) {
    throw std::invalid_argument("ScriptsDataModel::getFirstRayIndex: bad sweep index < 0");
  }
  _vol->loadRaysFromFields();
  
  const vector<RadxSweep *> sweeps = _vol->getSweeps();
  if (sweepIndex >= sweeps.size()) {
    throw std::invalid_argument("ScriptsDataModel::getFirstRayIndex: sweep index > number of sweeps");
  }
  RadxSweep *sweep = sweeps.at(sweepIndex);  
  if (sweep == NULL) {
    throw std::invalid_argument("ScriptsDataModel::getFirstRayIndex: bad sweep index");
  }
  size_t firstRayIndex = sweep->getStartRayIndex();
  return firstRayIndex;
}

// get the last ray for a sweep
size_t ScriptsDataModel::getLastRayIndex(int sweepIndex) {
  _vol->loadRaysFromFields();
  
  const vector<RadxSweep *> sweeps = _vol->getSweeps();
  if ((sweepIndex < 0) || (sweepIndex >= sweeps.size())) {
    string msg = "ScriptsDataModel::getLastRayIndex sweepIndex out of bounds ";
    msg.append(std::to_string(sweepIndex));
    throw std::invalid_argument(msg);
  }
  RadxSweep *sweep = sweeps.at(sweepIndex);  
  if (sweep == NULL) {
    throw std::invalid_argument("bad sweep index");
  }
  size_t lastRayIndex = sweep->getEndRayIndex();
  return lastRayIndex;
}

/*
int ScriptsDataModel::getSweepNumber(int sweepIndex) {

  const vector<RadxSweep *> sweeps = _vol->getSweeps();
  try {
    RadxSweep *sweep = sweeps.at(sweepIndex);
    int sweepNumber = sweep->getSweepNumber();
  } catch (exception??) {
    throw std::invalid_argument("bad sweep index");
  }
  return sweepNumber;
}
*/

double ScriptsDataModel::getRayAzimuthDeg(size_t rayIdx) {
  _vol->loadRaysFromFields();
  const vector<RadxRay *>  &rays = _vol->getRays();
  RadxRay *ray = rays.at(rayIdx);
  return ray->getAzimuthDeg();
}

double ScriptsDataModel::getRayNyquistVelocityMps(size_t rayIdx) {
  _vol->loadRaysFromFields();
  const vector<RadxRay *>  &rays = _vol->getRays();
  RadxRay *ray = rays.at(rayIdx);
  return ray->getNyquistMps();
}

vector<RadxRay *> &ScriptsDataModel::getRays() {
  //const vector<RadxRay *>  &rays = vol->getRays();
	return _vol->getRays();
}

RadxRay *ScriptsDataModel::getRay(size_t rayIdx) {
  _vol->loadRaysFromFields();
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol->getRays();

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

void ScriptsDataModel::printAzimuthInRayOrder() {
  _vol->loadRaysFromFields();
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol->getRays(); 
  LOG(DEBUG) << "first 20 rays in order ...";
  for (int i=0; i<20; i++) {
    RadxRay *ray = rays.at(i);
    LOG(DEBUG) << "ray Az = " << ray->getAzimuthDeg();
  }
}


int ScriptsDataModel::getSweepNumber(float elevation) {
  LOG(DEBUG) << "enter: elevation = " << elevation;
  //ScriptsDataModel *ScriptsDataModel = ScriptsDataModel::Instance();
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
  delete sweepAngles;

  if (!found) {
    throw std::invalid_argument("ScriptsDataModel::getSweepNumber no sweep found for elevation ");
  }

  // use the index, i, to find the sweep number, because
  // the index may be different than the number, which is a label for a sweep.
  int sweepNumber = 0;
  if (_vol == NULL) {
    if (!_cacheMetaDataValid) {
      throw std::invalid_argument("ScriptsDataModel::getSweepNumber _vol is NULL; cache not valid");
    } 
    LOG(DEBUG) << "_vol is NULL; cache is valid ";
    if ((i < 0) || (i >= _cacheSweepNumbers.size())) {
      throw std::invalid_argument("ScriptsDataModel::getSweepNumber index out of bounds for cache");
    }
    sweepNumber = _cacheSweepNumbers.at(i);
  } else {
    vector<RadxSweep *> sweeps = _vol->getSweeps();
    RadxSweep *sweep = sweeps.at(i);
    sweepNumber = sweep->getSweepNumber();
  }
  LOG(DEBUG) << "exit: sweepNumber is " << sweepNumber;
  return sweepNumber;
}

int ScriptsDataModel::getSweepIndexFromSweepNumber(int sweepNumber) {
  vector<RadxSweep *> sweeps = _vol->getSweeps();
  int idx = -1;
  for (int i = 0; i<sweeps.size(); i++) {
    if (sweeps.at(i)->getSweepNumber() == sweepNumber) {
      idx = i;
    }
  }
  if (idx < 0) {
    stringstream ss;
    ss << "ScriptsDataModel::getSweepIndex: no sweep found with sweep number " << sweepNumber << endl;
    throw std::invalid_argument(ss.str());
  } else {
    return idx;
  }
  
}

int ScriptsDataModel::getSweepIndexFromSweepAngle(float elevation) {
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
  if (!found) {
    stringstream ss;
    ss << "ScriptsDataModel::getSweepIndexFromSweepAngle no sweep found for elevation " <<
      elevation << endl;
    throw std::invalid_argument(ss.str());
  }
  return i;  
}

double ScriptsDataModel::getSweepAngleFromSweepNumber(int sweepNumber) {
  vector<double> *sweepAngles = getSweepAngles();
  int i = 0;
  bool found = false;  
  vector<int> *sweepNumbers = getSweepNumbers();
  while ((i < sweepNumbers->size()) && !found) {
    if (sweepNumbers->at(i) == sweepNumber) {
      found = true;
    } else {
      i += 1;
    }
  }
  if (!found) {
    stringstream ss;
    ss << "ScriptsDataModel::getSweepAngleFromSweepNumber no sweep found for sweep number " <<
      sweepNumber << endl;
    throw std::invalid_argument(ss.str());
  }

  return sweepAngles->at(i);    
}



vector<float> *ScriptsDataModel::getRayData(size_t rayIdx, string fieldName) { // , int sweepHeight) {
// TODO: which sweep? the rayIdx considers which sweep.

  LOG(DEBUG) << "enter" << " rayIdx = " << rayIdx 
    << " fieldName = " << fieldName;

  _vol->loadRaysFromFields();
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol->getRays();

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

int ScriptsDataModel::getNGates(size_t rayIdx, string fieldName, double sweepHeight) {

// TODO: which sweep? 
  _vol->loadRaysFromFields();
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol->getRays();
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
  //const RadxField *field;
  //field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 
  return nGates;
}

float ScriptsDataModel::getMissingFl32(string fieldName) {
  //_vol->loadFieldsFromRays();
  const RadxField *field = _vol->getFieldFromRay(fieldName);
  if (field == NULL) return Radx::missingFl32;
  
  Radx::fl32 missingValue = field->getMissingFl32();
  return (float) missingValue;
}
  
ScriptsDataModel::ScriptsDataModel() {
	init();
}

void ScriptsDataModel::init() {
  _vol = NULL;
  _cacheMetaDataValid = false;
}

// ----- 

void ScriptsDataModel::clearVolume() {
  delete _vol;
  _vol = NULL;
  _cacheMetaDataValid = false;

}

void ScriptsDataModel::getLookAhead(string fileName) {

  LOG(DEBUG) << "enter";

  // set up file object for reading
  
  RadxFile file;
  RadxVol vol;

  vol.clear();

  file.setReadMetadataOnly(true);
      
    string inputPath = fileName;
  
      LOG(DEBUG) << "  reading data file path: " << inputPath;
      //cerr << "  archive file index: " << _archiveScanIndex << endl;
    
    if (file.readFromPath(inputPath, vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "ScriptsDataModel::getLookAhead\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + inputPath + "\n";
      cerr << errMsg;
      throw std::invalid_argument(errMsg);
    } 

    // NOTE: since we are only reading the metadata, the rays are NOT filled.

    // load the sweeps into look ahead
    const vector<RadxSweep *> sweeps = vol.getSweepsAsInFile();
    if (sweeps.size() <= 0) {
      throw std::invalid_argument("no sweeps found in data file");
    }
    // copy sweep angles to look ahead
    // copy sweep numbers to look ahead    
    size_t nSweeps = sweeps.size();
    _lookAheadSweepNumbers.reserve(nSweeps);
    _lookAheadSweepAngles.reserve(nSweeps);
    for (vector<RadxSweep *>::const_iterator iter = sweeps.begin(); iter != sweeps.end(); ++iter)
    {
      RadxSweep *sweep = *iter;
      double sweepAngle = sweep->getFixedAngleDeg();
      int sweepNumber = sweep->getSweepNumber();
      cout << "sweep num " << sweepNumber << " angle " << sweepAngle << endl;
      _lookAheadSweepAngles.push_back(sweepAngle);
      _lookAheadSweepNumbers.push_back(sweepNumber);
    }   

    // end load sweeps into look ahead

    vol.loadFieldsFromRays();
    _lookAheadFields = vol.getFields();

    //if (file.getFileFormat() == RadxFile::FILE_FORMAT_NEXRAD_AR2)
    //  _okToStream = true;
    //else
    //  _okToStream = false;    

    LOG(DEBUG) << "exit";
}  

void ScriptsDataModel::deleteLookAhead() {
  _lookAheadSweepNumbers.clear();
  _lookAheadSweepAngles.clear();
  _lookAheadFields.clear();    
}

void ScriptsDataModel::moveToLookAhead() {
  // copy lookAhead to cache (only selected fields)
  _cacheFields.clear();
  _cacheSweepNumbers.clear();
  _cacheSweepAngles.clear();

  _cacheFields = _lookAheadFields;
  _cacheSweepAngles = _lookAheadSweepAngles;
  _cacheSweepNumbers = _lookAheadSweepNumbers;

  _cacheMetaDataValid = true;
}

// -----
const string &ScriptsDataModel::getPathInUse() {
	return _vol->getPathInUse();
}

int ScriptsDataModel::getNSweeps() {
	return _vol->getNSweeps();
}

const RadxSweep &ScriptsDataModel::getSweep(size_t sweepIndex) {
  return *_vol->getSweeps()[sweepIndex];
}
/*
vector<double> *ScriptsDataModel::getSweepAngles(string fileName) {
  if (_vol == NULL) {
    // we don't have an active volume. read the metadata for the file
    return getPossibleSweepAngles(fileName);
  } else {
    return getSweepAngles();
  }
}
*/

vector<double> *ScriptsDataModel::getSweepAngles() {

  vector<double> *sweepAngles = new vector<double>;
  if (_vol == NULL) {
    // pull info from cache metadata
    if (!_cacheMetaDataValid) {
      throw std::invalid_argument("ScriptsDataModel::getSweepAngles _vol is null; cache is invalid; no info on sweeps");
    }
    sweepAngles->reserve(_cacheSweepAngles.size());
    for (vector<double>::iterator it = _cacheSweepAngles.begin(); it != _cacheSweepAngles.end(); ++it) {
      sweepAngles->push_back(*it);
    }
  } else {
    vector<RadxSweep *> sweeps;
    sweeps = _vol->getSweeps();
    sweepAngles->reserve(sweeps.size());
    vector<RadxSweep *>::iterator it;
    for (it = sweeps.begin(); it != sweeps.end(); ++it) {
    	RadxSweep *sweep = *it;
      sweepAngles->push_back(sweep->getFixedAngleDeg());  //??? is this the right method??
    }
  }
  return sweepAngles;
}

vector<int> *ScriptsDataModel::getSweepNumbers() {

  vector<int> *sweepNumbers = new vector<int>;
  if (_vol == NULL) {
    // pull info from cache metadata
    if (!_cacheMetaDataValid) {
      throw std::invalid_argument("ScriptsDataModel::getSweepNumbers _vol is null; cache is invalid; no info on sweeps");
    }
    sweepNumbers->reserve(_cacheSweepNumbers.size());
    for (vector<int>::iterator it = _cacheSweepNumbers.begin(); it != _cacheSweepNumbers.end(); ++it) {
      sweepNumbers->push_back(*it);
    }
  } else {
    vector<RadxSweep *> sweeps;
    sweeps = _vol->getSweeps();
    sweepNumbers->reserve(sweeps.size());
    vector<RadxSweep *>::iterator it;
    for (it = sweeps.begin(); it != sweeps.end(); ++it) {
      RadxSweep *sweep = *it;
      sweepNumbers->push_back(sweep->getSweepNumber());  //??? is this the right method??
    }
  }
  return sweepNumbers;
}

// TODO: remove RadxPlatform and return base types
const RadxPlatform &ScriptsDataModel::getPlatform() {
  return _vol->getPlatform();
} 

void ScriptsDataModel::getPredomRayGeom(double *startRangeKm, double *gateSpacingKm) {
  double startRange;
  double gateSpace;
  _vol->getPredomRayGeom(startRange, gateSpace);
  *startRangeKm = startRange;
  *gateSpacingKm = gateSpace;  
}


vector<string> *ScriptsDataModel::getUniqueFieldNameList() {
    if (_vol == NULL) throw "No open archive file";
    _vol->loadFieldsFromRays();
    LOG(DEBUG) << "enter";
    const vector<string> fieldNames = _vol->getUniqueFieldNameList();
    vector<string> *fieldNamesCopy = new vector<string>;
    vector<string>::const_iterator it;
    for (it=fieldNames.begin(); it!=fieldNames.end(); ++it) {
    	fieldNamesCopy->push_back(*it);
    }
    LOG(DEBUG) << "there are " << fieldNamesCopy->size() << " fieldNames";
    LOG(DEBUG) << "exit";
    return fieldNamesCopy;
}

const RadxVol& ScriptsDataModel::getVolume() {
  return *_vol;
}

float ScriptsDataModel::getLatitudeDeg() {
  return _vol->getLatitudeDeg();
}

float ScriptsDataModel::getLongitudeDeg() {
  return _vol->getLongitudeDeg();
}

float ScriptsDataModel::getAltitudeKm() {
  return _vol->getAltitudeKm();
}

double ScriptsDataModel::getRadarBeamWidthDegV() {
  return _vol->getRadarBeamWidthDegV();
}

double ScriptsDataModel::getCfactorRotationCorr() {
  RadxCfactors *cfactors;
  if ((cfactors = _vol->getCfactors()) != NULL)
    return cfactors->getRotationCorr();
  else return 0.0;
}

/*
double ScriptsDataModel::getAltitudeKmAgl() {

}
*/

const RadxGeoref *ScriptsDataModel::getGeoreference(size_t rayIdx) {

  RadxRay *ray = getRay(rayIdx);
  // get the winds from the aircraft platform
  const RadxGeoref *georef = ray->getGeoreference();

  if (georef == NULL) {
    LOG(DEBUG) << "ERROR - georef is NULL";
    LOG(DEBUG) << "      trying to recover ...";
    _vol->setLocationFromStartRay();
    georef = ray->getGeoreference();
    if (georef == NULL) {
      throw "BBUnfoldAircraftWind: Georef is null. Cannot find ew_wind, ns_wind, vert_wind";
    }
  } 
  return georef;
}

void ScriptsDataModel::_setupVolRead(RadxFile &file, vector<string> &fieldNames,
  int sweepNumber,
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
  


// TODO: pre-read sweeps, then only request one sweep at a time
//----
    /// Set the sweep number limits to be used on read.
  ///
  /// This will limit the sweep data to be read.
  ///
  /// If strict limits are not set (see below), 1 sweep is guaranted
  /// to be returned. Even if no sweep lies within the limits the closest
  /// sweep will be returned.
  ///
  /// If strict limits are set (which is the default), then only data from
  /// sweeps within the limits will be included.

  //file.setReadSweepNumLimits(int min_sweep_num,
  //                           int max_sweep_num);

  file.setReadSweepNumLimits(sweepNumber, sweepNumber);
  file.setReadStrictAngleLimits(false);

  //-----


  //  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
  //    const DisplayField *field = _fields[ifield];
  //    file.addReadField(field->getName());
  //  }

}

bool ScriptsDataModel::sweepRaysAreIndexed(size_t sweepIndex) {

  const RadxSweep &sweep = getSweep(sweepIndex);
  //if (sweep == NULL) {
  //  throw invalid_argument("ScriptsDataModel::sweepRaysAreIndexed: no sweep for index ");
  //}
  return sweep.getRaysAreIndexed(); 
}

double ScriptsDataModel::getSweepAngleResDeg(size_t sweepIndex) {
  const RadxSweep &sweep = getSweep(sweepIndex);
  //if (sweep == NULL) {
  //  throw invalid_argument("ScriptsDataModel::getSweepAngleResDeg: no sweep for index ");
  //}  
  return sweep.getAngleResDeg();
}

// reads all sweeps
void ScriptsDataModel::_setupVolRead(RadxFile &file, vector<string> &fieldNames,
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
  
}

/*
vector<double> *ScriptsDataModel::getPossibleSweepAngles(string fileName)
{

  LOG(DEBUG) << "enter";

  // set up file object for reading
  
  RadxFile file;
  RadxVol vol;

  vol.clear();
  //_setupVolRead(file);

  file.setReadMetadataOnly(true);
      
    string inputPath = fileName;
  
    LOG(DEBUG) << "  reading data file path: " << inputPath;
    
    if (file.readFromPath(inputPath, vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "ScriptsDataModel::getPossibleSweepAngles\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + inputPath + "\n";
      cerr << errMsg;
      throw std::invalid_argument(errMsg);
    } 

  // ---

  vol.print(cerr);
  // Well, there are two lists for sweeps:
  // _sweepsAsInFile.size();
  // and _sweeps 
  // not sure which to check and when, so let's just check both
  //const vector<RadxSweep *> sweeps = vol.getSweeps();
  //if (sweeps.size() <= 0) {
  _cacheSweeps = vol.getSweepsAsInFile();
  //}
  //RadxSweep *sweep = sweeps.at(sweepIndex); 
  // ----
  if (_cacheSweeps.size() <= 0) {
    throw std::invalid_argument("no sweeps found in data file");
  }
    //vol.loadFieldsFromRays();  HERE
    //const vector<RadxField *> fields = vol.getFields();
    vector<double> *allSweepNumbers = new vector<double>;
    allSweepNumbers->resize(_cacheSweeps.size());
    for (int idx = 0; idx < _cacheSweeps.size(); idx++) 
    //for (vector<RadxSweep *>::const_iterator iter = sweeps.begin(); iter != sweeps.end(); ++iter)
    {
      //RadxSweep *sweep = *iter;
      //cout << field->getName() << endl;
      allSweepNumbers->at(idx) = _cacheSweeps[idx]->getFixedAngleDeg();
    }

    _cacheMetaDataValid = true;

    LOG(DEBUG) << "exit";
    return allSweepNumbers;
}
*/

vector<string> *ScriptsDataModel::getPossibleFieldNames(string fileName)
{

  // read meta data
  LOG(DEBUG) << "enter";

  deleteLookAhead();
  getLookAhead(fileName);
 
  vector<string> *allFieldNames = new vector<string>;
  for (vector<RadxField *>::const_iterator iter = _lookAheadFields.begin(); iter != _lookAheadFields.end(); ++iter)
  {
    RadxField *field = *iter;
    cout << field->getName() << endl;
    allFieldNames->push_back(field->getName());
  }

    LOG(DEBUG) << "exit";
    return allFieldNames;
}


// may not be used; may be the same as getLookAhead??
void ScriptsDataModel::readFileMetaData(string fileName)
{

  LOG(DEBUG) << "enter";

  // set up file object for reading
  
  RadxFile file;
  RadxVol vol;

  vol.clear();
  //_setupVolRead(file);

  file.setReadMetadataOnly(true);
      
    string inputPath = fileName;
  
      LOG(DEBUG) << "  reading data file path: " << inputPath;
      //cerr << "  archive file index: " << _archiveScanIndex << endl;
    
    if (file.readFromPath(inputPath, vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "ScriptsDataModel::getPossibleFieldNames\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + inputPath + "\n";
      cerr << errMsg;
      throw std::invalid_argument(errMsg);
    } 

    // NOTE: since we are only reading the metadata, the rays are NOT filled.

    // load the sweeps into cache
    const vector<RadxSweep *> sweeps = vol.getSweepsAsInFile();
    if (sweeps.size() <= 0) {
      throw std::invalid_argument("no sweeps found in data file");
    }
    // copy sweep angles to cache
    // copy sweep numbers to cache    
    size_t nSweeps = sweeps.size();
    _cacheSweepNumbers.reserve(nSweeps);
    _cacheSweepAngles.reserve(nSweeps);
    for (vector<RadxSweep *>::const_iterator iter = sweeps.begin(); iter != sweeps.end(); ++iter)
    {
      RadxSweep *sweep = *iter;
      double sweepAngle = sweep->getFixedAngleDeg();
      int sweepNumber = sweep->getSweepNumber();
      cout << "sweep num " << sweepNumber << " angle " << sweepAngle << endl;
      _cacheSweepAngles.push_back(sweepAngle);
      _cacheSweepNumbers.push_back(sweepNumber);
    }   

    // end load sweeps into cache

    vol.loadFieldsFromRays();
    _cacheFields = vol.getFields();
    vector<string> *allFieldNames = new vector<string>;
    for (vector<RadxField *>::const_iterator iter = _cacheFields.begin(); iter != _cacheFields.end(); ++iter)
    {
      RadxField *field = *iter;
      cout << field->getName() << endl;
      allFieldNames->push_back(field->getName());
    }

    _cacheMetaDataValid = true;

    LOG(DEBUG) << "exit";
}


size_t ScriptsDataModel::findClosestRay(float azimuth, int sweepNumber) { // float elevation) {
  LOG(DEBUG) << "enter azimuth = " << azimuth << " sweepNumber = " << sweepNumber;

  //ScriptsDataModel *ScriptsDataModel = ScriptsDataModel::Instance();

  _vol->loadRaysFromFields();

  // NOTE! Sweep Number, NOT Sweep Index!!!

  RadxSweep *sweep = _vol->getSweepByNumber(sweepNumber);
  if (sweep == NULL) {
    //string msg = "no sweep found"
    throw std::invalid_argument("no sweep found");
  }

  // RadxSweep *sweep = _vol->getSweepByFixedAngle(elevation); DOESN'T WORK
  //if (sweep == NULL) 
  //  throw std::invalid_argument("unknown sweep elevation");
  //int requestedSweepNumber = sweep->getSweepNumber();

  const vector<RadxRay *> rays = getRays();
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

size_t ScriptsDataModel::getRayIndex(size_t baseIndex, int offset, int sweepNumber) {
  RadxSweep *sweep = _vol->getSweepByNumber(sweepNumber);
  size_t startRayIndex = sweep->getStartRayIndex();
  size_t endRayIndex = sweep->getEndRayIndex();
  size_t idx = calculateRayIndex_f(baseIndex, startRayIndex, endRayIndex, offset);
  // swim around a bit to make sure we have the right ray index, 
  // just in case the end or start index is slightly off.
  // verify the sweep number 

  const vector<RadxRay *>  &rays = _vol->getRays();
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

size_t ScriptsDataModel::calculateRayIndex_f(size_t idx, size_t start, size_t end, int offset) {

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

