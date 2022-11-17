#include "ScriptsDataController.hh"  
#include <toolsa/LogStream.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxCfactors.hh>
#include <cmath>

using namespace std;

// read all fields and all sweeps
ScriptsDataController::ScriptsDataController(
  string &dataFileName, bool debug_verbose, bool debug_extra) {
  init();
  openRead(dataFileName, debug_verbose, debug_extra);
}

ScriptsDataController::ScriptsDataController(
  string &dataFileName, bool debug_verbose, bool debug_extra,
  vector<string> &fieldNamesInScript) {
  init();
  openRead(dataFileName, fieldNamesInScript, debug_verbose, debug_extra);
}

ScriptsDataController::ScriptsDataController(
  string &dataFileName, int sweepNumber, bool debug_verbose, bool debug_extra,
  vector<string> &fieldNamesInScript) {
  init();
  openRead(dataFileName, sweepNumber, fieldNamesInScript, debug_verbose, debug_extra);
}

ScriptsDataController::~ScriptsDataController() {}

void ScriptsDataController::init() {
  //_vol = NULL;
  _cacheMetaDataValid = false;
  // TODO: set all indexes to zero ...
}

int ScriptsDataController::openRead(string &inputPath,
 bool debug_verbose, bool debug_extra) {

  LOG(DEBUG) << "enter " << inputPath;
   
  try {
    _scriptsDataModel.readData(inputPath, 
      debug_verbose, debug_extra);
  } catch (const string &errMsg) {
    return -1;
  }
  init();
  LOG(DEBUG) << "exit";
  return 0;
}

int ScriptsDataController::openRead(string &inputPath, 
  vector<string> &fieldNames, bool debug_verbose, bool debug_extra) {

  LOG(DEBUG) << "enter " << inputPath;
   
  try {
    //int sweep_number = _sweepController->getSelectedNumber();
    _scriptsDataModel.readData(inputPath, fieldNames, 
      debug_verbose, debug_extra);
  } catch (const string &errMsg) {
    return -1;
  }
  init();
  LOG(DEBUG) << "exit";
  return 0;
}

int ScriptsDataController::openRead(string &inputPath, int sweepNumber,
  vector<string> &fieldNames, bool debug_verbose, bool debug_extra) {

  LOG(DEBUG) << "enter " << inputPath;
   
  try {
    //int sweep_number = _sweepController->getSelectedNumber();
    _scriptsDataModel.readData(inputPath, fieldNames, sweepNumber,
      debug_verbose, debug_extra);
  } catch (const string &errMsg) {
    return -1;
  }
  init(); 
  LOG(DEBUG) << "exit";
  return 0;
}


// return data for the field, at the sweep and ray index
const vector<float> *ScriptsDataController::GetData_NOTUSED(string fieldName,
              int rayIdx, int sweepIdx)  {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx
       << " sweepIdx=" << sweepIdx;

  return _scriptsDataModel.GetData(fieldName, rayIdx, sweepIdx);
  /*

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
    string msg = "ScriptsDataController::GetData rayIdx outside sweep ";
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
  */
}

/*
const vector<float> *ScriptsDataController::GetData(string fieldName,
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

void ScriptsDataController::setDataByIndex(string &fieldName, 
            int rayIdx, int sweepIdx, vector<float> *fieldData) { 

  // What is being returned? the name of the new field in the model that
  // contains the results.

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;

  _scriptsDataModel.SetDataByIndex(fieldName, rayIdx, sweepIdx, fieldData);

/*
  _vol->loadRaysFromFields(); // loadFieldsFromRays();


    LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx
       << " sweepIdx=" << sweepIdx;

  // sweep numbers are 1-based in RadxVol, not zero based, so, add one to the index.
  sweepIdx += 1;

  RadxSweep *sweep = _vol->getSweepByNumber(sweepIdx);


  SetData(fieldName, rayIdx, sweep, fieldData);
  */
  LOG(DEBUG) << "exit ";
}

void ScriptsDataController::setData(string &fieldName, vector<float> *fieldData) { 

  _scriptsDataModel.SetData(fieldName, _currentRayIdx, _currentSweepIdx, fieldData);

  LOG(DEBUG) << "exit ";
}

void ScriptsDataController::setData(string &fieldName, 
            int rayIdx, RadxSweep *sweep, vector<float> *fieldData) { 

  _scriptsDataModel.SetData(fieldName, rayIdx, sweep, fieldData);

  LOG(DEBUG) << "exit ";
}

void ScriptsDataController::setData(string &fieldName, 
            float azimuth, float sweepAngle, vector<float> *fieldData) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " ray =" << azimuth
       << " sweep =" << sweepAngle;

  // this is the closest ray for the sweep angle
  int rayIdx = (int) findClosestRay(azimuth, sweepAngle);
  //_vol->loadRaysFromFields();

  //RadxSweep *sweep = _vol->getSweepByFixedAngle(sweepAngle);
  RadxSweep *dummyValue = nullptr;

  setData(fieldName, rayIdx, dummyValue, fieldData);
  LOG(DEBUG) << "exit";
}

void ScriptsDataController::setData(string &fieldName, float value) {
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
void ScriptsDataController::RemoveField(string &fieldName) {
  _scriptsDataModel.RemoveField(fieldName);
  /*
  int result = _vol->removeField(fieldName);
  if (result != 0) {
    string msg = "failed to remove field: ";
    msg.append(fieldName); 
    throw std::invalid_argument(msg);
  }
  */
}

// remove field from ray
void ScriptsDataController::RemoveField(size_t rayIdx, string &fieldName) {
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

RadxVol *ScriptsDataController::getRadarVolume(string path, vector<string> *fieldNames,
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
      errMsg += "ScriptsDataController::readData\n";
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
RadxVol *ScriptsDataController::getRadarVolume(string path, vector<string> *fieldNames,
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
      errMsg += "ScriptsDataController::getRadarVolume\n";
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

void ScriptsDataController::getRayData(string path, vector<string> &fieldNames,
  int sweepNumber) {
  readData(path, fieldNames, sweepNumber);
}

void ScriptsDataController::readData(string path, vector<string> &fieldNames,
  bool debug_verbose, bool debug_extra) {

  LOG(DEBUG) << "enter";
  _scriptsDataModel.readData(path, fieldNames, debug_verbose, debug_extra);

  LOG(DEBUG) << "exit";
}



void ScriptsDataController::readData(string path, vector<string> &fieldNames,
  int sweepNumber,
	bool debug_verbose, bool debug_extra) {

  LOG(DEBUG) << "enter";
  _scriptsDataModel.readData(path, fieldNames, sweepNumber, debug_verbose, debug_extra);

  LOG(DEBUG) << "exit";
}

RadxTime ScriptsDataController::getStartTimeSecs() {
	return _scriptsDataModel.getStartTimeSecs();
}

RadxTime ScriptsDataController::getEndTimeSecs() {
  return _scriptsDataModel.getEndTimeSecs();
}

// TODO: this could be faster
void ScriptsDataController::_selectFieldsNotInVolume(vector<string> *allFieldNames) {
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

void ScriptsDataController::_selectFieldsNotInCurrentVersion(
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

Radx::PrimaryAxis_t ScriptsDataController::getPrimaryAxis() {
  return _scriptsDataModel.getPrimaryAxis();
}


/* append the edited sweep (those fields read in memory) to
// tho current version of the data file
// Q: What if the fields are different? maybe merge fields before calling append??
void ScriptsDataController::appendSweep(string currentVersionPath) {


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


/*
// merge edited fields (those read in memory) with
// those fields in the original data file
RadxVol *ScriptsDataController::mergeDataFields(string originalSourcePath) {


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
RadxVol *ScriptsDataController::mergeDataFields(string currentVersionPath, string originalSourcePath) {


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
void ScriptsDataController::writeWithMergeData(string outputPath, string originalSourcePath) {

    RadxVol *mergedVolume = mergeDataFields(originalSourcePath);
    writeData(outputPath, mergedVolume);
    delete mergedVolume;
}

// use to merge data with a data file NOT currently selected
void ScriptsDataController::writeWithMergeData(string outputPath, string currentVersionPath, string originalSourcePath) {

    RadxVol *mergedVolume = mergeDataFields(currentVersionPath, originalSourcePath);
    writeData(outputPath, mergedVolume);
    delete mergedVolume;
}
*/

// NOTE: side effect of changing the class variable _currentFilePath
void ScriptsDataController::writeData(string path) {
    _scriptsDataModel.writeData(path);
    /* KEEP  THIS ... BECAUSE IT WORKS!!!

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
    */
}

// no side effects, just writes the radar volume to the path
void ScriptsDataController::writeData(string path, RadxVol *vol) {
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

/*
int ScriptsDataController::mergeDataFiles(string dest_path, string source_path, string original_path) {

  writeWithMergeData(dest_path, source_path, original_path);
  return 0;
}
*/

void ScriptsDataController::update() {

}

void ScriptsDataController::renameField(string currentName, string newName) {
  _scriptsDataModel.renameField(currentName, newName);
}

void ScriptsDataController::renameField(size_t rayIdx, string currentName, string newName) {
  RadxRay *ray = getRay(rayIdx);

  // renameField(oldName, newName);
  ray->renameField(currentName, newName);
  // loadFieldNameMap
  ray->loadFieldNameMap();

}

/* move to ScriptDataModel
// copy from one field to another field 
// overwrite destination
void ScriptsDataController::copyField(size_t rayIdx, string fromFieldName, string toFieldName) {
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
void ScriptsDataController::copyField2(size_t rayIdx, string fromFieldName, string toFieldName) {
  RadxRay *ray = getRay(rayIdx);
  RadxField *srcField = fetchDataField(ray, fromFieldName);
  if (srcField != NULL) {
    RadxField *copy = new RadxField(*srcField);
    copy->setName(toFieldName);
    ray->addField(copy);
  }
}
*/

bool ScriptsDataController::fieldExists(size_t rayIdx, string fieldName) {
  RadxRay *ray = getRay(rayIdx);
  try {
    RadxField *field = fetchDataField(ray, fieldName);
    if (field != NULL) return true;
    else return false;
  } catch (std::invalid_argument &ex) {
    return false;
  }
}

RadxField *ScriptsDataController::fetchDataField(RadxRay *ray, string &fieldName) {
  return _scriptsDataModel.fetchDataField(ray, fieldName);
  /*
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
    string msg = "ScriptsDataController::fetchDataField unknown error occurred: ";
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
    string msg = "ScriptsDataController::fetchDataField No field found in ray: ";
    msg.append(fieldName);
    throw std::invalid_argument(msg);
  }

  return dataField; 
  */
}
/*
const RadxField *ScriptsDataController::fetchDataField(const RadxRay *ray, string &fieldName) {
  ray->loadFieldNameMap();
  RadxField *foundField = NULL;
  vector<RadxField *> fields = ray->getFields();
  for (size_t ii = 0; ii < fields.size(); ii++) {
    string name = fields[ii]->getName();
    if (name.compare(fieldName) == 0)
       foundField = fields[ii];
   }
   if (foundField == NULL) {
      string msg = "ScriptsDataController::fetchDataField No field found in ray: ";
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
    string msg = "ScriptsDataController::fetchDataField unknown error occurred: ";
    msg + fieldName;
    throw std::invalid_argument(msg);
  }
  if (dataField == NULL) {
    string msg = "ScriptsDataController::fetchDataField No field found in ray: ";
    msg + fieldName;
    throw std::invalid_argument(msg);
  }
  return dataField; 
  
}
*/

const float *ScriptsDataController::fetchData(RadxRay *ray, string &fieldName) {

    RadxField *dataField = fetchDataField(ray, fieldName);
    if (dataField != NULL)
      return dataField->getDataFl32();
    else
      return NULL;
}

// total number of rays in volume, for all sweeps
size_t ScriptsDataController::getNRays() { // string fieldName, double sweepAngle) {
  return _scriptsDataModel.getNRays();
  /*
  size_t nRays = 0;
  if (_vol != NULL) {
    _vol->loadRaysFromFields();
    //const RadxField *field;
    vector<RadxRay *>  &rays = _vol->getRays();
    nRays = rays.size();
  }
  return nRays;
  */
}

// get the number of rays for a sweep
size_t ScriptsDataController::getNRays(int sweepNumber) {
  return _scriptsDataModel.getNRays(sweepNumber);
  /*
  _vol->loadRaysFromFields();
  RadxSweep *sweep = _vol->getSweepByNumber(sweepNumber);
  if (sweep == NULL) {
    throw std::invalid_argument("ScriptsDataController::getNRays: no sweep found");
  }
  size_t nRays = sweep->getNRays();
  return nRays;
  */
}

// get the number of rays for a sweep
size_t ScriptsDataController::getNRaysSweepIndex(int sweepIndex) {
  return _scriptsDataModel.getNRaysSweepIndex(sweepIndex);
  /*
  _vol->loadRaysFromFields();
  const vector<RadxSweep *> sweeps = _vol->getSweeps();
  RadxSweep *sweep = sweeps.at(sweepIndex); 
  if (sweep == NULL) {
    throw std::invalid_argument("ScriptsDataController::getNRaysSweepIndex: bad sweep index");
  }
  size_t nRays = sweep->getNRays();
  return nRays;
  */
}

// get the first ray for a sweep
size_t ScriptsDataController::getFirstRayIndex(int sweepIndex) {
  return _scriptsDataModel.getFirstRayIndex(sweepIndex);
}

// get the last ray for a sweep
size_t ScriptsDataController::getLastRayIndex(int sweepIndex) {
  return _scriptsDataModel.getLastRayIndex(sweepIndex);
}

/*
int ScriptsDataController::getSweepNumber(int sweepIndex) {

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

double ScriptsDataController::getRayAzimuthDeg(size_t rayIdx) {
  return _scriptsDataModel.getRayAzimuthDeg(rayIdx);
  /*
  _vol->loadRaysFromFields();
  const vector<RadxRay *>  &rays = _vol->getRays();
  RadxRay *ray = rays.at(rayIdx);
  return ray->getAzimuthDeg();
  */
}

double ScriptsDataController::getRayNyquistVelocityMps(size_t rayIdx) {
  return _scriptsDataModel.getRayNyquistVelocityMps(rayIdx);
  /*
  _vol->loadRaysFromFields();
  const vector<RadxRay *>  &rays = _vol->getRays();
  RadxRay *ray = rays.at(rayIdx);
  return ray->getNyquistMps();
  */
}

vector<RadxRay *> &ScriptsDataController::getRays() {
  //const vector<RadxRay *>  &rays = vol->getRays();
	return _scriptsDataModel.getRays();
}

RadxRay *ScriptsDataController::getRay(size_t rayIdx) {
  return _scriptsDataModel.getRay(rayIdx);
}

size_t ScriptsDataController::getCurrentRayIdx() {
  return _currentRayIdx;
}
  /*
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
  */


void ScriptsDataController::printAzimuthInRayOrder() {
  _scriptsDataModel.printAzimuthInRayOrder();
  /*
  _vol->loadRaysFromFields();
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol->getRays(); 
  LOG(DEBUG) << "first 20 rays in order ...";
  for (int i=0; i<20; i++) {
    RadxRay *ray = rays.at(i);
    LOG(DEBUG) << "ray Az = " << ray->getAzimuthDeg();
  }
  */
}


int ScriptsDataController::getSweepNumber(float elevation) {
  LOG(DEBUG) << "enter: elevation = " << elevation;
  return _scriptsDataModel.getSweepNumber(elevation);
  /*
  //ScriptsDataController *ScriptsDataController = ScriptsDataController::Instance();
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
    throw std::invalid_argument("ScriptsDataController::getSweepNumber no sweep found for elevation ");
  }

  // use the index, i, to find the sweep number, because
  // the index may be different than the number, which is a label for a sweep.
  int sweepNumber = 0;
  if (_vol == NULL) {
    if (!_cacheMetaDataValid) {
      throw std::invalid_argument("ScriptsDataController::getSweepNumber _vol is NULL; cache not valid");
    } 
    LOG(DEBUG) << "_vol is NULL; cache is valid ";
    if ((i < 0) || (i >= _cacheSweepNumbers.size())) {
      throw std::invalid_argument("ScriptsDataController::getSweepNumber index out of bounds for cache");
    }
    sweepNumber = _cacheSweepNumbers.at(i);
  } else {
    vector<RadxSweep *> sweeps = _vol->getSweeps();
    RadxSweep *sweep = sweeps.at(i);
    sweepNumber = sweep->getSweepNumber();
  }

  LOG(DEBUG) << "exit: sweepNumber is " << sweepNumber;
  return sweepNumber;
  */
}

int ScriptsDataController::getSweepIndexFromSweepNumber(int sweepNumber) {
  return _scriptsDataModel.getSweepIndexFromSweepNumber(sweepNumber);
  /*
  vector<RadxSweep *> sweeps = _vol->getSweeps();
  int idx = -1;
  for (int i = 0; i<sweeps.size(); i++) {
    if (sweeps.at(i)->getSweepNumber() == sweepNumber) {
      idx = i;
    }
  }
  if (idx < 0) {
    stringstream ss;
    ss << "ScriptsDataController::getSweepIndex: no sweep found with sweep number " << sweepNumber << endl;
    throw std::invalid_argument(ss.str());
  } else {
    return idx;
  }
  */
}

/*
int ScriptsDataController::getSweepIndexFromSweepAngle(float elevation) {
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
    ss << "ScriptsDataController::getSweepIndexFromSweepAngle no sweep found for elevation " <<
      elevation << endl;
    throw std::invalid_argument(ss.str());
  }
  return i;  
}

double ScriptsDataController::getSweepAngleFromSweepNumber(int sweepNumber) {
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
    ss << "ScriptsDataController::getSweepAngleFromSweepNumber no sweep found for sweep number " <<
      sweepNumber << endl;
    throw std::invalid_argument(ss.str());
  }

  return sweepAngles->at(i);    
}
*/

vector<float> *ScriptsDataController::getRayData(string fieldName) {
  return _scriptsDataModel.getRayData(_currentRayIdx, fieldName);
}


vector<float> *ScriptsDataController::getRayData(size_t rayIdx, string fieldName) { // , int sweepHeight) {

  return _scriptsDataModel.getRayData(rayIdx, fieldName);
  /*

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
  */
}

int ScriptsDataController::getNGates(size_t rayIdx, string fieldName, double sweepHeight) {

  return _scriptsDataModel.getNGates(rayIdx, fieldName, sweepHeight);
  /*
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
  */
}

float ScriptsDataController::getMissingFl32(string fieldName) {
  return _scriptsDataModel.getMissingFl32(fieldName);
  /*
  //_vol->loadFieldsFromRays();
  const RadxField *field = _vol->getFieldFromRay(fieldName);
  if (field == NULL) return Radx::missingFl32;
  
  Radx::fl32 missingValue = field->getMissingFl32();
  return (float) missingValue;
  */
}

// ----- 

void ScriptsDataController::clearVolume() {
  _scriptsDataModel.clearVolume();
}

void ScriptsDataController::getSweepsAndFields(string fileName) {

  LOG(DEBUG) << "enter";

  _scriptsDataModel.getSweepsAndFields(fileName);

/*
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
      errMsg += "ScriptsDataController::getSweepsAndFields\n";
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
    */ 

    LOG(DEBUG) << "exit";
}  

void ScriptsDataController::deleteLookAhead() {
  _lookAheadSweepNumbers.clear();
  _lookAheadSweepAngles.clear();
  _lookAheadFields.clear();    
}

void ScriptsDataController::moveToLookAhead() {
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
const string &ScriptsDataController::getPathInUse() {
	return _scriptsDataModel.getPathInUse();
}

int ScriptsDataController::getNSweeps() {
	return _scriptsDataModel.getNSweeps();
}

const RadxSweep &ScriptsDataController::getSweep(size_t sweepIndex) {
  return _scriptsDataModel.getSweep(sweepIndex);
}

vector<double> *ScriptsDataController::getSweepAngles() {

  return _scriptsDataModel.getSweepAngles();
  /*
  vector<double> *sweepAngles = new vector<double>;
  if (_vol == NULL) {
    // pull info from cache metadata
    if (!_cacheMetaDataValid) {
      throw std::invalid_argument("ScriptsDataController::getSweepAngles _vol is null; cache is invalid; no info on sweeps");
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
  */
}

vector<int> *ScriptsDataController::getSweepNumbers() {

  return _scriptsDataModel.getSweepNumbers();
  /*
  vector<int> *sweepNumbers = new vector<int>;
  if (_vol == NULL) {
    // pull info from cache metadata
    if (!_cacheMetaDataValid) {
      throw std::invalid_argument("ScriptsDataController::getSweepNumbers _vol is null; cache is invalid; no info on sweeps");
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
  */
}

// TODO: remove RadxPlatform and return base types
const RadxPlatform &ScriptsDataController::getPlatform() {
  return _scriptsDataModel.getPlatform();
} 

void ScriptsDataController::getPredomRayGeom(double *startRangeKm, double *gateSpacingKm) {
  //double startRange;
  //double gateSpace;
  _scriptsDataModel.getPredomRayGeom(startRangeKm, gateSpacingKm);
  //*startRangeKm = startRange;
  //*gateSpacingKm = gateSpace;  
}


vector<string> *ScriptsDataController::getUniqueFieldNameList() {
  return _scriptsDataModel.getUniqueFieldNameList();
  /*
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
    */
}

//const RadxVol& ScriptsDataController::getVolume() {
//  return *_vol;
//}

float ScriptsDataController::getLatitudeDeg() {
  return _scriptsDataModel.getLatitudeDeg();
}

float ScriptsDataController::getLongitudeDeg() {
  return _scriptsDataModel.getLongitudeDeg();
}

float ScriptsDataController::getAltitudeKm() {
  return _scriptsDataModel.getAltitudeKm();
}

double ScriptsDataController::getRadarBeamWidthDegV() {
  return _scriptsDataModel.getRadarBeamWidthDegV();
}

double ScriptsDataController::getCfactorRotationCorr() {
  return _scriptsDataModel.getCfactorRotationCorr();
  /*
  RadxCfactors *cfactors;
  if ((cfactors = _vol->getCfactors()) != NULL)
    return cfactors->getRotationCorr();
  else return 0.0;
  */
}

/*
double ScriptsDataController::getAltitudeKmAgl() {

}
*/

const RadxGeoref *ScriptsDataController::getGeoreference(size_t rayIdx) {
  return _scriptsDataModel.getGeoreference(rayIdx);
  /*
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
  */
}

void ScriptsDataController::_setupVolRead(RadxFile &file, vector<string> &fieldNames,
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

bool ScriptsDataController::sweepRaysAreIndexed(size_t sweepIndex) {

  const RadxSweep &sweep = getSweep(sweepIndex);
  //if (sweep == NULL) {
  //  throw invalid_argument("ScriptsDataController::sweepRaysAreIndexed: no sweep for index ");
  //}
  return sweep.getRaysAreIndexed(); 
}

double ScriptsDataController::getSweepAngleResDeg(size_t sweepIndex) {
  const RadxSweep &sweep = getSweep(sweepIndex);
  //if (sweep == NULL) {
  //  throw invalid_argument("ScriptsDataController::getSweepAngleResDeg: no sweep for index ");
  //}  
  return sweep.getAngleResDeg();
}

void ScriptsDataController::setCurrentSweepToFirst() {
  cerr << "entry setCurrentSweepToFirst" << endl;
  _currentSweepIdx = 0;
  cerr << "exit setCurrentSweepToFirst" << endl;

  //LOG(DEBUG) << "exit";

}

void ScriptsDataController::setCurrentSweepTo(int sweepIndex) {
  cerr << "entry setCurrentSweepToFirst" << endl;
  if (sweepIndex < 0) _currentSweepIdx = 0;
  else _currentSweepIdx = (size_t) sweepIndex;
  cerr << "exit setCurrentSweepToFirst" << endl;

  //LOG(DEBUG) << "exit";

}


void ScriptsDataController::setCurrentRayToFirst() {
  //cerr << "entry setCurrentRayToFirst" << endl;
  _currentRayIdx = 0;
  _lastRayIdx = _scriptsDataModel.getNRays();
  //applyBoundary();
  //cerr << "exit setCurrentRayToFirst" << endl;

  //LOG(DEBUG) << "exit";

}

// BE CAREFUL!! SweepIndex (int) vs. SweepNumber (int)
//              size_t vs. int
void ScriptsDataController::setCurrentRayToFirstOf(int sweepIndex) {
  // find the first ray of this sweep
  // find the last ray of this sweep

  //int sweepNumber = ScriptsDataModel->getSweepNumber(sweepIndex);
  _currentRayIdx = _scriptsDataModel.getFirstRayIndex(sweepIndex);
  _lastRayIdx = _scriptsDataModel.getLastRayIndex(sweepIndex);
}

void ScriptsDataController::setCurrentRayTo(size_t rayIdx) {
  _currentRayIdx = rayIdx;
}

void ScriptsDataController::nextRayIndependentOfSweep() {
  //LOG(DEBUG) << "entry";
  _currentRayIdx += 1;

  if ((_currentRayIdx % 100) == 0) {
      cerr << "   current ray " << _currentRayIdx << 
        " last ray index " << _lastRayIdx;
      if (moreRays()) 
         cerr << " az = " << _scriptsDataModel.getRayAzimuthDeg(_currentRayIdx);
      cerr << " current sweep index " << _currentSweepIdx << endl;
  }  
}

void ScriptsDataController::nextRay() {
  //LOG(DEBUG) << "entry";
  _currentRayIdx += 1;

  size_t lastRayIndex = _scriptsDataModel.getLastRayIndex(_currentSweepIdx);  
  if (_currentRayIdx > lastRayIndex) {
    _currentSweepIdx += 1;
  } 
  if ((_currentRayIdx % 100) == 0) {
      cerr << "   current ray " << _currentRayIdx << 
        " last ray index " << lastRayIndex;
      if (moreRays()) 
         cerr << " az = " << _scriptsDataModel.getRayAzimuthDeg(_currentRayIdx);
      cerr << " current sweep index " << _currentSweepIdx << endl;
  }  
  //  applyBoundary();
  //cerr << "exit nextRay" << endl;
  //LOG(DEBUG) << "exit";

}

bool ScriptsDataController::moreRays() {
  //  LOG(DEBUG) << "entry";
  //cerr << "entry moreRays" << endl;
  //  vector<RadxRay *> rays = _data->getRays();
  //size_t 
  // THIS DOES NOT WORK; it changes memory outside of its bounds
  //  const size_t nRays = _data->getNRays();

  //LOG(DEBUG) << "There are " <<  nRays << " rays";;
  //if (_currentRayIdx >= nRays)
  //  _data->loadFieldsFromRays();
  //return (_currentRayIdx < _data->getNRays()); // nRays);

  return (_currentRayIdx < _lastRayIdx);
}

void ScriptsDataController::nextSweep() {
  //LOG(DEBUG) << "entry" << " _currentSweepIdx = " << _currentSweepIdx;
  _currentSweepIdx += 1;
  cerr << "current sweep " <<  _currentSweepIdx << endl;
  //cerr << "exit nextSweep" << endl;
  //LOG(DEBUG) << "exit";
}

bool ScriptsDataController::moreSweeps() {
  
  size_t nSweeps = _scriptsDataModel.getNSweeps();
  return (_currentSweepIdx < nSweeps);
}



void ScriptsDataController::regularizeRays() {
  _scriptsDataModel.regularizeRays();  
}

void ScriptsDataController::copyField(size_t rayIdx, string tempName, string userDefinedName) {

  if (_scriptsDataModel.fieldExists(rayIdx, userDefinedName)) {
    throw std::invalid_argument("field name exist; cannot rename");
  }
  if (_scriptsDataModel.fieldExists(rayIdx, tempName)) {
    _scriptsDataModel.copyField2(rayIdx, tempName, userDefinedName);
  } 
}

void ScriptsDataController::assign(size_t rayIdx, string tempName, string userDefinedName) {
  //_data->loadFieldsFromRays(); // TODO: this is a costly function as it moves the data/or pointers
  // TODO: where are the field names kept? in the table map? can i just change that?
  // Because each RadxRay holds its own FieldNameMap,
  // TODO: maybe ... no longer relavant?

  // Let the ScriptsDataModel handle the changes? the renaming?
  // But, decide here if this is a rename or a copy 

  if (_scriptsDataModel.fieldExists(rayIdx, userDefinedName)) {
    // copy temp data into existing field data
    // delete temp field and data
    _scriptsDataModel.copyField(rayIdx, tempName, userDefinedName);
    _scriptsDataModel.RemoveField(rayIdx, tempName);
  } else {
    // rename the temp field 
    _scriptsDataModel.renameField(rayIdx, tempName, userDefinedName);
  }
}

void ScriptsDataController::assignByRay(string tempName, string userDefinedName) {
  assign(_currentRayIdx, tempName, userDefinedName);
}

void ScriptsDataController::assign(string tempName, string userDefinedName) {

  // for each ray ...
  size_t nRays = _scriptsDataModel.getNRays();
  for (size_t rayIdx=0; rayIdx < nRays; rayIdx++) {
    assign(rayIdx, tempName, userDefinedName);
  }
}

void ScriptsDataController::assign(string tempName, string userDefinedName,
  size_t sweepIndex) {

  // for each ray of sweep
  size_t firstRayInSweep = _scriptsDataModel.getFirstRayIndex(sweepIndex);
  size_t lastRayInSweep = _scriptsDataModel.getLastRayIndex(sweepIndex);
  for (size_t rayIdx=firstRayInSweep; rayIdx <= lastRayInSweep; rayIdx++) {
    assign(rayIdx, tempName, userDefinedName);
  }
}

void ScriptsDataController::copyField(string tempName, string userDefinedName) {

  // for each ray ...
  size_t nRays = _scriptsDataModel.getNRays();
  for (size_t rayIdx=0; rayIdx < nRays; rayIdx++) {
    copyField(rayIdx, tempName, userDefinedName);
  }
}

void ScriptsDataController::copyField(string tempName, string userDefinedName,
  size_t sweepIndex) {

  // for each ray of sweep
  size_t firstRayInSweep = _scriptsDataModel.getFirstRayIndex(sweepIndex);
  size_t lastRayInSweep = _scriptsDataModel.getLastRayIndex(sweepIndex);
  for (size_t rayIdx=firstRayInSweep; rayIdx <= lastRayInSweep; rayIdx++) {
    copyField(rayIdx, tempName, userDefinedName);
  }
}


// reads all sweeps
void ScriptsDataController::_setupVolRead(RadxFile &file, vector<string> &fieldNames,
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
vector<double> *ScriptsDataController::getPossibleSweepAngles(string fileName)
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
      errMsg += "ScriptsDataController::getPossibleSweepAngles\n";
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

vector<string> *ScriptsDataController::getPossibleFieldNames(string fileName)
{

  // read meta data
  LOG(DEBUG) << "enter";

  deleteLookAhead();
  getSweepsAndFields(fileName);
 
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
void ScriptsDataController::readFileMetaData(string fileName)
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
      errMsg += "ScriptsDataController::getPossibleFieldNames\n";
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


size_t ScriptsDataController::findClosestRay(float azimuth, int sweepNumber) { // float elevation) {
  LOG(DEBUG) << "enter azimuth = " << azimuth << " sweepNumber = " << sweepNumber;
  return _scriptsDataModel.findClosestRay(azimuth, sweepNumber);

/*
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

size_t ScriptsDataController::getRayIndex(size_t baseIndex, int offset, int sweepNumber) {
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
  */
}

size_t ScriptsDataController::calculateRayIndex_f(size_t idx, size_t start, size_t end, int offset) {

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

