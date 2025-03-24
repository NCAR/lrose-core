#include "DataModel.hh"  
#include <toolsa/LogStream.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxCfactors.hh>
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
    string msg = "DataModel::GetData rayIdx outside sweep ";
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
const vector<float> *DataModel::GetData(string fieldName,
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

void DataModel::SetDataByIndex(string &fieldName, 
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

void DataModel::SetData(RadxVol *vol, string &fieldName,
            int rayIdx, RadxSweep *sweep, vector<float> *fieldData) {

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;

  vol->loadRaysFromFields(); // loadFieldsFromRays();

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;

  RadxField *field;

  //  get the ray for this field
  const vector<RadxRay *>  &rays = vol->getRays();
  if (rays.size() <= 0) {
    LOG(DEBUG) <<  "ERROR - no rays found";
  }
  RadxRay *ray = rays.at(rayIdx); // startRayIndex + rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "no ray found";
  }

  // get the data (in) and create space for new data (out)
  field = fetchDataField(vol, ray, fieldName);
  size_t nGates = ray->getNGates();

  if (field == NULL) {
    throw std::invalid_argument("no RadxField found ");
  }

  vector<float> deref = *fieldData;
  const Radx::fl32 *radxData = &deref[0];
  bool isLocal = true;  //?? not sure about this
  field->setDataFl32(nGates, radxData, isLocal);
  
  LOG(DEBUG) << "exit ";
}

void DataModel::SetData(string &fieldName, 
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

void DataModel::SetData(string &fieldName, 
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
  int result = _vol->removeField(fieldName);
  if (result != 0) {
    string msg = "failed to remove field: ";
    msg.append(fieldName); 
    throw std::invalid_argument(msg);
  }
}

// remove field from ray
void DataModel::RemoveField(size_t rayIdx, string &fieldName) {
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

void DataModel::regularizeRays() {
  bool nFieldsConstantPerRay = true;
  _vol->loadFieldsFromRays(nFieldsConstantPerRay);
  _vol->loadRaysFromFields();
}

RadxVol *DataModel::getRadarVolume(string path, vector<string> *fieldNames,
  bool debug_verbose, bool debug_extra) {


  // Is this a new path? or has it been read before?
  //if (_currentFilePath.compare(path) == 0) {
  //  return;  <<====  
  //}

  LOG(DEBUG) << "enter";
  // set up file object for reading

  RadxVol *vol = new RadxVol();

  cerr << "before " << endl;
  RadxFile file;

  _setupVolRead(file, *fieldNames, debug_verbose, debug_extra);
   
  LOG(DEBUG) << "  reading data file path: " << path;    
    
  if (file.readFromPath(path, *vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "DataModel::readData\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + path + "\n";
      cerr << errMsg;
      throw errMsg;
  } 
  cerr << "after " << endl;

  vol->convertToFl32();

  // adjust angles for elevation surveillance if needed
    //  if (ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
    //  ray->setAnglesForElevSurveillance();
    //}
  
  //vol->setAnglesForElevSurveillance();
  
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
RadxVol *DataModel::getRadarVolume(string path, vector<string> *fieldNames,
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
      errMsg += "DataModel::getRadarVolume\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + path + "\n";
      cerr << errMsg;
      throw errMsg;
  } 
  cerr << "after " << endl;

  vol->convertToFl32();

  // adjust angles for elevation surveillance if needed
  
   //   if (ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
   //   ray->setAnglesForElevSurveillance();
   // }
  //vol->setAnglesForElevSurveillance();
  
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

void DataModel::getRayData(string path, vector<string> &fieldNames,
  int sweepNumber) {
  // before reading, see if we have the information already ...

  readData(path, fieldNames, sweepNumber);
}

//void DataModel::applyCorrectionFactors() {
  //adjustAnglesForElevationSurveillance(_vol);
//}

//void DataModel::withdrawCorrectionFactors() {
  //resetAnglesForElevationSurveillance(_vol);
//}

/*
void DataModel::resetAnglesForElevationSurveillance(RadxVol *_vol) {
  // adjust angles for elevation surveillance if needed
  RadxRay *ray = getRay(0);
  if (ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
    size_t nRays = getNRays();
    for (size_t iray = 0; iray < nRays; iray++) {
      RadxRay *ray = getRay(iray);
      LOG(DEBUG) << iray << ": adjusting az " << ray->getAzimuthDeg() << " to "; 
      //ray->setAnglesForElevSurveillance();

//----  copied this code from ray->setAnglesForElevSurveillance();
      // because the cfactors were always NULL for the ray,
      // but not for the volume.
   // TODO: THIS IS NOT CORRECT, BECAUSE THE AZ and EL were overwritten when cfacs applied!!!
      const RadxGeoref *georef = ray->getGeoreference();
      if (georef != NULL) {
        double rollCorr = 0.0;
        double rotCorr = 0.0;
        double tiltCorr = 0.0;
        const RadxCfactors *cfactors = _vol->getCfactors();
        if (cfactors != NULL) {
          rollCorr = cfactors->getRollCorr();
          rotCorr = cfactors->getRotationCorr();
          tiltCorr = cfactors->getTiltCorr();
        }
        double rotation = georef->getRotation() - rotCorr;
        double roll = georef->getRoll() - rollCorr;
        double tilt = georef->getTilt() - tiltCorr;
        double newAz = rotation - roll;
        while (newAz < 0) {
          newAz += 360.0;
        }
        while (newAz > 360.0) {
          newAz -= 360.0;
        }
        ray->setAzimuthDeg(newAz);
        ray->setElevationDeg(tilt);
      }
    }
  }
}
*/
/*
void DataModel::adjustAnglesForElevationSurveillance(RadxVol *_vol) {

  // adjust angles for elevation surveillance if needed
  RadxRay *ray = getRay(0);
  if (ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
    size_t nRays = getNRays();
    for (size_t iray = 0; iray < nRays; iray++) {
      RadxRay *ray = getRay(iray);
      LOG(DEBUG) << iray << ": adjusting az " << ray->getAzimuthDeg() << " to "; 
      //ray->setAnglesForElevSurveillance();

//----  copied this code from ray->setAnglesForElevSurveillance();
      // because the cfactors were always NULL for the ray,
      // but not for the volume.
   
   // TODO: THIS IS DIFFERENT FROM RadxRay::applyGeoref

    const RadxGeoref *georef = ray->getGeoreference();
    if (georef != NULL) {
      double rollCorr = 0.0;
      double rotCorr = 0.0;
      double tiltCorr = 0.0;
      const RadxCfactors *cfactors = _vol->getCfactors();
      if (cfactors != NULL) {
        rollCorr = cfactors->getRollCorr();
        rotCorr = cfactors->getRotationCorr();
        tiltCorr = cfactors->getTiltCorr();
      }
      double rotation = georef->getRotation() + rotCorr;
      double roll = georef->getRoll() + rollCorr;
      double tilt = georef->getTilt() + tiltCorr;
      double newAz = rotation + roll;
      while (newAz < 0) {
        newAz += 360.0;
      }
      while (newAz > 360.0) {
        newAz -= 360.0;
      }
      //TODO: where to handle this?? when making mods for survellance? or somehwere else??
      //  mod everything by 360?
      ray->setAzimuthDeg(newAz);
      ray->setElevationDeg(tilt);
      // cerr << ray->getAzimuthDeg() << endl;
    }
//----


     // loadSweepInfoFromRays();
    }
    //_vol->setAnglesForElevSurveillance();
      //ray->setAnglesForElevSurveillance();
  }
}
*/

void DataModel::readData(string path, vector<string> &fieldNames,
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
      errMsg += "DataModel::readData\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + path + "\n";
      cerr << errMsg;
      throw errMsg;
  } 
  cerr << "after " << endl;

  if (_vol == NULL) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "DataModel::readData\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + path + "\n";
      cerr << errMsg;
      throw errMsg;  
  }

  // check for fields read
  //bool nFieldsConstantPerRay = true;
  //_vol->loadFieldsFromRays(nFieldsConstantPerRay);

  /*
  vector<string>::iterator it;
  for (it=fieldNames.begin(); it != fieldNames.end(); ++it) {
    RadxField *field = _vol->getField(*it);
    if (field == NULL) {
        string errMsg = "ERROR - No field read\n";
        errMsg += "DataModel::readData\n";
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

  // adjustAnglesForElevationSurveillance(_vol);

  // adjust angles for elevation surveillance if needed
  //if (_vol->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
  //    _vol->setAnglesForElevSurveillance();
      //ray->setAnglesForElevSurveillance();
  //}
  

  
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

  //_sanityCheckVolume();

  LOG(DEBUG) << "exit";
}

void DataModel::sanityCheckVolume(string &warningMsg) {

  // accumulate warning or error information, then send
  // the appropriate level of information.
  // fatal errors, throw a string exception
  string fatalErrorMsg;
  bool errors = false;
  // warnings, throw a std::invalide_argument exception
  //string warningMsg;
  bool warnings = false;

  if (getPrimaryAxis() == Radx::PRIMARY_AXIS_Y_PRIME) {

      // check if georeferences have been applied for the first ray
      bool areGeorefsApplied = getGeoreferenceApplied(0);
      if (!areGeorefsApplied) {
        // issue a warning ...
        warningMsg.append("HawkEdit will display the data as-is, still allow any edits, ");
        warningMsg.append("and examination of the data. However, these data are NOT considered ready and it is ");
        warningMsg.append("recommended to run RadxConvert, to prepare the data for ");
        warningMsg.append("HawkEdit display and editing.");
        warnings = true;
      }

      // get georefs on first ray
      const RadxGeoref *georef = getGeoreference(0);
      if (georef == NULL) {
        warningMsg.append("HawkEdit detected a Y-Prime radar. ");
        warningMsg.append("Georeference information is missing.  ");
        warningMsg.append("This information is needed to properly display the field data. ");
        warningMsg.append("Azimuth will be used in the PPI display, instead of rotation. ");
        warnings = true;
        // TODO: should this be a warning? and just use the azimuth since there is no rotation?
      } else {
        double az = georef->getTrackRelRot();
        if (az == Radx::missingMetaDouble) {
          // this is alright, we will just use the rotation from the georefs block.

          // Where to check this? Detect in DataModel? 
          // if PRIMARY_AXIS_Y_PRIME, and georef is null, or if georef->getTrackRelRot is missing,
          //   send message to PolarManager.  PolarManager must display the message.
          // So, it must propogate up that far.
          // warningMsg.append("HawkEdit detected the track-relative rotation is missing for a Y-Prime radar. ");
          // warningMsg.append("This information is needed to properly display the field data.  ");
          // warningMsg.append("If the displayed data do not appear as expected, ");
          // warningMsg.append("please exit HawkEdit and use RadxConvert with the parameter ");
          // warningMsg.append("apply_georeference_corrections = TRUE to fix this issue.");
          // warnings = true;
        }
      }
  }

  //if (warnings) {
  //  throw std::invalid_argument(warningMsg);
  //} 
  //if (errors) {
  //  throw std::invalid_argument(fatalErrorMsg);
  //}

}


// TODO: Do I need a DataController? to switch between Radx* data types and
// standard C++/Qt data types?

RadxTime DataModel::getStartTimeSecs() {
	return _vol->getStartTimeSecs();
}

RadxTime DataModel::getEndTimeSecs() {
  return _vol->getEndTimeSecs();
}

// TODO: this could be faster
void DataModel::_selectFieldsNotInVolume(vector<string> *allFieldNames) {
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

void DataModel::_selectFieldsNotInCurrentVersion(
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

Radx::PrimaryAxis_t DataModel::getPrimaryAxis() {
  return _vol->getPrimaryAxis();
}

// Working here ...
// What options do we want when merging data fields?
// We don't want to corrupt our internal working version of the radar volume.
// Maybe we work with a temporary, accumulator volume that is the merge
// of the original and the working version.
// I can do incremental reads of the files; yes, but it is awkward.
// So, don't do this. There is a concern, this may be optimal
// so that there are not big files in memory at the same time.
// Read the metadata, read sweeps as needed, not the entire file.


// merge edited fields (those read in memory) with
// those fields in the original data file
/*TODO: maybe just send a pointer to the rays and the number of rays? No.
void DataModel::mergeDataFields(RadxVol *dstVol,
                                    size_t dstStartRayIndex
                                    size_t dstEndRayIndex
                                    RadxVol *srcVol,
                                    size_t srcStartRayIndex
                                    size_t srcEndRayIndex
                                    working here

                                    //string originalSourcePath
) {
  int error = 0;
  size_t dstNRays = dstEndRayIndex - dstStartRayIndex + 1;
  size_t srcNRays = srcEndRayIndex - srcStartRayIndex + 1;
  
  if (dstNRays != srcNRays) {
    error = 1;
    return error;
  }
  
  // This won't work! I need to add the fields with all their metadata!
  // Well, it will work if the fields are NOT new fields
  // So, add the new fields.  And bulk import the existing fields.
    
  vector<size_t> rayNGates;
  rayNGates.reserve(dstNRays);
  size_t i;
  for (i = dstStartRayIndex; i<dstEndRayIndex; i++) {
    RadxRay *dstRay = dstVol->getRay.at(i);
    // TODO: if dstNGates != srcNGates ==> error
    rayNGates.push_back(dstRay->getNGates);
  }

  dstVol->loadFieldsFromRays();
  srcVol->loadFieldsFromRays();
  // read the source_path into a separate volume, then merge the fields and
*/
/*
    Need to add sweeps from secondary file to primary file;
    script edits with apply to all sweeps generates a complete version file in undo/redo stack
    script edits to current sweep generate single sweep version file?
    spreadsheet edits generate single sweep version file in undo/redo stack
    to reconstitute the complete edits, need to walk up/down the version stack
 
  // compare to the original file
  size_t nsweeps_primary = primaryVol->getNSweeps();
  size_t nsweeps_secondary = secondaryVol->getNSweeps();
    
  // if the number of sweeps is the same,
  // then we can use the latest version in the stack
  if (nsweeps_primary == nsweeps_secondary) {
    useLatestVersion();
  } else {
    // if the number of sweeps is different, then we have incremental edits and
    // and need to walk up/down the version stack and assemble a complete file
    // with all the sweeps.
    assembleFromVersionStack(primaryVol, secondaryVol);
  }
*/
/*
  // now merge any fields that were not imported from the
  // original file.
  int maxSweepNum = 0;
  //vector<RadxRay *> &pRays = primaryVol->getRays();
  //const vector<RadxRay *> &sRays = secondaryVol->getRays();
  //if (pRays.size() != sRays.size()) {
    // report an error!
  //  cout << "ERROR!!! unequal number of rays while merging for file save." << endl;
  //}
  
  // The number of rays for each sweep will stay constant.
  // The fields can vary from sweep to sweep.
  // The sweeps may or may not be in each file version (RadxVol).
  
  // So, let's make the fields continuous in memory.
  // Then we can add fields quickly in one chunk.
  
  
  
  // data is a linear chunk of memory with data for a field
  // the dimensions are
  // ray1Gate0 ... ray1GateN_1
  // ray2Gate0 ... ray2GateN_2
  // ...
  // rayRGate0 ... rayRGateN_R
  //
  // rayNGates = {N_1, N_2, ... N_R}  the number of gates for each ray
  //
  void RadxField::setDataFl32(const vector<size_t> &rayNGates,
                              const Radx::fl32 *data)
  
  vector<RadxRay *> dstRays = getRays(RadxVol *dstVol)
  vector<RadxRay *> srcRays = getRays(RadxVol *srcVol)
  
  for (size_t offset = 0; offset < dstNRays; offset++) {
    
    RadxRay *dstRay = dstRays+offset;
    RadxRay *srcRay = srcRays+offset;
    // for each field in source vol
    Do the rays have field info?
    Yes,
    vector<RadxField *> srcFields = srcVol->getFields();
    // TODO: use iterator here ...
    for (size_t ifield = 0; ifield < srcFields->size(); ifield++) {
      string fieldName = srcFields->at(ifield);
      const RadxField *sfield = sRay->getField(fieldName);
      
      RadxField *copyField = new RadxField();
      *copyField = *sfield;
      // Add a previously-created field to the ray. The field must have
      // been dynamically allocted using new(). Memory management for
      //  this field passes to the ray, which will free the field object
      // using delete().
      // void addField(RadxField *field);
      RadxField *dstField = dstRay->getField(fieldName);
      RadxField *getField(const string &name);
      // delete the field in the destination, if it exists
      // If isLocal is true, the data will be copied to the
      // local buffer.
      //if (field exists in dstRay)
      if (dstField != NULL) {
        //copy src data to dst field
        // for a vector of rays
        // rayNGates[0] = 50  # of gates for ray 0
        // rayNGates[1] = 50  # of gates for ray 1
        // ...
        // rayNGates[nRays] = 60
        // data must be
        // ray0gate0, ray0gate1, ... ray0gate49
        // ray1gate0, ...
        // ...
        // rayNgate0, ...            rayNgate59
        dstField->setDataFl32(const vector<size_t> &rayNGates,
                                    const Radx::fl32 *data)
      } else {
        // add new field to ray with data
      }
      
      bool isLocal?
      dstRay->addField(copyField);
      
      
      void RadxField::setDataRemote(const RadxField &other,
                                    const void *data,
                                    size_t nGates)
      
      
      -----
      
      RadxRay *ray = rays.at(rayIdx); // startRayIndex + rayIdx);
      if (ray == NULL) {
        LOG(DEBUG) << "ERROR - ray is NULL";
        throw "no ray found";
      }

      // get the data (in) and create space for new data (out)
      field = fetchDataField(vol, ray, fieldName);
      size_t nGates = ray->getNGates();

      if (field == NULL) {
        throw std::invalid_argument("no RadxField found ");
      }

      vector<float> deref = *fieldData;
      const Radx::fl32 *radxData = &deref[0];
      bool isLocal = true;  //?? not sure about this
      field->setDataFl32(nGates, radxData, isLocal);
      
      --
      
      
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
*/


// fill the fieldNames vector with field names
// in the vol.
void DataModel::getFieldNames(RadxVol &vol, vector<string> *fieldNames) {

  vol.loadFieldsFromRays();
  vector<RadxField *> lookAheadFieldsTemp = vol.getFields();

  size_t nFields = lookAheadFieldsTemp.size();
  fieldNames->reserve(nFields);
  for (vector<RadxField *>::const_iterator iter = lookAheadFieldsTemp.begin(); iter != lookAheadFieldsTemp.end(); ++iter)
  {
    RadxField *field = *iter;
    string name = field->getName();
      fieldNames->push_back(name);
  }
}

// change the radxVol sent.
// There is another version of this function, that
// creates and returns a new RadxVol.
//
// merge fields in radxVol (currently in memory) with
// those fields in the original data file
// the result of the merge is returned in radxVol
void DataModel::mergeDataFields(RadxVol *radxVol, string originalSourcePath) {

  // read the original source into a separate volume,
  // then add fields from the original that are NOT in
  // the radxVol

  vector<string> currentVersionFieldNames;
  getFieldNames(*radxVol, &currentVersionFieldNames);
  vector<string> *allPossibleFieldNames = getPossibleFieldNames(originalSourcePath);
  _selectFieldsNotInCurrentVersion(&currentVersionFieldNames, allPossibleFieldNames);

  // allPossibleFieldNames now contains only the fields
  // NOT in the current version of file

  bool debug_verbose = false;
  bool debug_extra = false;

  RadxVol *originalVol = getRadarVolume(originalSourcePath, allPossibleFieldNames,
     debug_verbose, debug_extra);
    
  // add secondary rays to primary vol

  int maxSweepNum = 0;
  vector<RadxRay *> &pRays = radxVol->getRays();
  const vector<RadxRay *> &sRays = originalVol->getRays();
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

  radxVol->setPackingFromRays();
  radxVol->loadVolumeInfoFromRays();
  radxVol->loadSweepInfoFromRays();
  radxVol->remapToPredomGeom();
  
  delete allPossibleFieldNames;
  currentVersionFieldNames.clear();
  delete originalVol;

}

// change the radxVol sent.
// There is another version of this function, that
// creates and returns a new RadxVol.
//
// merge fields in radxVol (currently in memory) with
// those fields in the original data file
// the result of the merge is returned in radxVol
void DataModel::mergeDataFields2(RadxVol *radxVol, string originalSourcePath) {

  // read the original source into a separate volume,
  // then add fields from the original that are NOT in
  // the radxVol

  vector<string> currentVersionFieldNames;
  getFieldNames(*radxVol, &currentVersionFieldNames);
  vector<string> *allPossibleFieldNames = getPossibleFieldNames(originalSourcePath);
  _selectFieldsNotInCurrentVersion(&currentVersionFieldNames, allPossibleFieldNames);

  // allPossibleFieldNames now contains only the fields
  // NOT in the current version of file

  bool debug_verbose = false;
  bool debug_extra = false;

  RadxVol *originalVol = getRadarVolume(originalSourcePath, allPossibleFieldNames,
     debug_verbose, debug_extra);
    
  // add secondary rays to primary vol

  int maxSweepNum = 0;
  vector<RadxRay *> &pRays = radxVol->getRays();
  const vector<RadxRay *> &sRays = originalVol->getRays();
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

  radxVol->setPackingFromRays();
  radxVol->loadVolumeInfoFromRays();
  radxVol->loadSweepInfoFromRays();
  radxVol->remapToPredomGeom();
  
  delete allPossibleFieldNames;
  currentVersionFieldNames.clear();
  delete originalVol;

}
/*
// add field to destination (dst) rays from source (src) rays
void DataModel::addFieldWithData(RadxVol *dstVol,
                                 RadxVol *srcVol,
                                 string &fieldName,
                                 vector<RadxRay *> *dstRays,
                                 vector<RadxRay *> *srcRays
                                 ) {
  
  if (dstRays->size() != srcRays->size())
    throw "Error: DataModel::addFieldWithData unequal number of rays";
  // if fieldName !exist in srcRays ==> error
  // if fieldName exists in dstRays ==> error
  
  for (size_t iray = 0; iray < srcRays.size(); iray++) {
    RadxRay srcRay = srcRays->at(iray);
    RadxRay dstRay = dstRays->at(iray);
    
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
// returns merged radar volume
RadxVol *DataModel::mergeDataFields(string currentVersionPath, string originalSourcePath) {


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

// primaryVol is the accumulator
// secondaryVol is the original volume
// need access to the undo/redo stack
// This should be an UndoRedoController method! 
//void DataModel::assembleFromVersionStack(RadxVol *primaryVol,
//                                         RadxVol *secondaryVol) {
  // start at the most recent changes and walk to the oldest changes
  // overwrite sweep changes
  
  // Do we have to assemble the entire RadxVol in memory before
  // writing it to a file? Or can we write incrementally to
  // a file one sweep at a time? Nope.  The Radx lib only writes
  // a complete file.
  
  // Probably need a temporary accumulating RadxVol,
  // so that the working volume is not corrupted.
  // primaryVol should be a copy of the working volume.
  
  
//}


/*
// use to merge data with currently selected data file
void DataModel::writeWithMergeData(string outputPath, string originalSourcePath) {

    RadxVol *mergedVolume = mergeDataFields(originalSourcePath);
    writeData(outputPath, mergedVolume);
    delete mergedVolume;
}
 */

/*
// use to merge data with a file NOT currently selected
void DataModel::writeWithMergeData(string outputPath, RadxVol *radxVol, string originalSourcePath) {

  mergeSweeps(radxVol, originalSourcePath);
  mergeDataFields(radxVol, originalSourcePath);
  writeData(outputPath, radxVol);
}
*/

void DataModel::_mergeSweepsFromFileVersions(RadxVol *accumulator,
                                   queue<string> *listOfVersions,
                                   string justFilename) {
  
  //queue<string>::iterator versionFileName;
  //for (versionFileName = listOfVersions->begin();
  //     versionFileName != listOfVersions->end(); ++versionFileName)
  vector<bool> sweepUpdated;
  size_t nSweeps = accumulator->getNSweeps();
  sweepUpdated.resize(nSweeps, false);
  while (!listOfVersions->empty()) {
    string filePath = listOfVersions->front();
    filePath.append("/");  // TODO: this should be path separator!!! use RadxPath for this
    filePath.append(justFilename);
    mergeSweeps(accumulator, filePath, &sweepUpdated);
    listOfVersions->pop();
  }
}

int DataModel::mergeSelectedFileVersions(string dest_path,
                                         string originalPath,
                                         queue<string> *listOfVersions,
                                         string justFilename) {
  LOG(DEBUG) << "enter";
  int success = 0;
  RadxVol accumulator;
  //TODO: I think the accumulator needs to be the complete file, with all the  sweeps and all the rays.  If we use the working version, we may need to add sweeps, which might be weird compared to overwriting sweeps??
  
  
  RadxFile file;

  accumulator.clear();

  file.setReadMetadataOnly(false);
  string inputPath = originalPath;
  
  LOG(DEBUG) << "  reading data file path: " << inputPath;
    
  if (file.readFromPath(inputPath, accumulator)) {
    string errMsg = "ERROR - Cannot retrieve previous version of file\n";
    errMsg += "DataModel::mergeDataFileVersions\n";
    errMsg += file.getErrStr() + "\n";
    errMsg += "  path: " + inputPath + "\n";
    cerr << errMsg;
    throw std::invalid_argument(errMsg);
  }
  accumulator.loadSweepInfoFromRays();

  // TODO: then, no need to pass around the original path!!!!
  //*accumulator = *_vol;  // duplicate the current volume as the accumulator
  //_mergeFileVersions(accumulator, listOfVersions, justFilename);
  _mergeWrite(&accumulator, listOfVersions, justFilename,
              dest_path);
  //delete accumulator;
  LOG(DEBUG) << "exit";
  return success;
}

// listOfVersions, with the latest version at the END,
// and the oldest version at the front.
int DataModel::mergeFileVersions(string outputPath, string sourcePath,
                                 // TODO: what is sourcePath???
                                     string originalPath,
                                     queue<string> *listOfVersions,
                                     string justFilename) {
  
  LOG(DEBUG) << "enter";
  
  int success = 0;
  // go through each version starting with the latest
  // if there is a sweep in the version that is NOT in the
  // current version, then add it to the current version.
  // Otherwise, the sweep is old, discard it.
  
  RadxVol *accumulator;
  //if (isCurrentFile) {
  //  accumulatorFile = *_vol;  // TODO: test!!! is this a copy? or the original?
  //} else {
  // start with the latest version of this file. get it into memorym
  // and use it as an accumulator
  // remove the last element
  //string currentVersionPath = listOfVersions->front();
  //listOfVersions->pop();
  vector<string> *fieldNames = getPossibleFieldNames(originalPath);
  bool debug_verbose = false;
  bool debug_extra = false;
  accumulator = getRadarVolume(originalPath, fieldNames,
       debug_verbose, debug_extra);
  _mergeWrite(accumulator, listOfVersions, justFilename,
              outputPath);
  //_mergeFileVersions(accumulator, listOfVersions, justFilename);
  
  // now merge the data fields
  //writeWithMergeData(outputPath, accumulator, originalPath);
  LOG(DEBUG) << "exit";
  return success;
}

// GOOD
void DataModel::_mergeWrite(RadxVol *accumulator,
                        queue<string> *listOfVersions,
                        string justFilename,
                        string outputPath) {
  _mergeSweepsFromFileVersions(accumulator, listOfVersions, justFilename);
  //mergeDataFields(accumulator, originalSourcePath);
  writeData(outputPath, accumulator);
}

// merge sweeps from dataFilePath into the radxVol
// overwrite the sweep/ray/field data in radxVol, if the
// sweep has NOT been updated.
// radxVol in/out
// sweepUpdated in/out
// dataFilePath in
void DataModel::mergeSweeps(RadxVol *dstVol, string &dataFilePath,
                            vector<bool> *sweepUpdated) {
  // set up file object for reading
  
  RadxFile file;
  RadxVol previousVersion;

  previousVersion.clear();

  file.setReadMetadataOnly(false);
  string inputPath = dataFilePath;
  
  LOG(DEBUG) << "  reading data file path: " << inputPath;
    
  if (file.readFromPath(inputPath, previousVersion)) {
    string errMsg = "ERROR - Cannot retrieve previous version of file\n";
    errMsg += "DataModel::mergeDataFileVersions\n";
    errMsg += file.getErrStr() + "\n";
    errMsg += "  path: " + inputPath + "\n";
    cerr << errMsg;
    throw std::invalid_argument(errMsg);
  }
  previousVersion.loadSweepInfoFromRays();
  const vector<RadxSweep *> sweeps = previousVersion.getSweepsAsInFile();
  size_t nSweeps = sweeps.size();
  if (nSweeps <= 0) {
    throw std::invalid_argument("no sweeps found in previous version of file");
  }
  
  for (vector<RadxSweep *>::const_iterator iter = sweeps.begin(); iter != sweeps.end(); ++iter)
  {
    RadxSweep *sweep = *iter;
    // TODO: sometimes, the sweep numbers are NOT sequential AND
    // sometimes the sweep numbers are not even available,
    // so I need to find a sweep index, or find the sweep by angle?
    double sweepAngle = sweep->getFixedAngleDeg();
    int sweepNumber = sweep->getSweepNumber();
    int dstSweepIndex = getSweepIndexInVolume(dstVol, sweepAngle, sweepNumber);
    cout << "sweep num " << sweepNumber << " angle " << sweepAngle << endl;
    // find the sweep
    //RadxSweep *accumulatorSweep = radxVol->getSweepByNumber(sweepNumber);
      //radxVol->addSweep(sweep);

    if (!sweepUpdated->at(dstSweepIndex)) {
      overwriteSweepRays(dstVol, dstSweepIndex, &previousVersion, sweep);
      sweepUpdated->at(sweepNumber) = true;
    }
  }
}

// GOOD
// srcSweep is from an edited version
void DataModel::overwriteSweepRays(RadxVol *dstVol, int dstSweepIndex, RadxVol *srcVol, RadxSweep *srcSweep) {
  // the number of rays will be the same, but the fields may be different
  // We need to reconcile the fields
  
  if (!srcSweep->getRaysAreIndexed()) {
    throw "Error DataModel::overwriteSweepRays rays are NOT indexed in sweep";
  }
  size_t dstStartRayIndex = getFirstRayIndex(dstVol, dstSweepIndex);
  size_t dstEndRayIndex = getLastRayIndex(dstVol, dstSweepIndex);
  
  size_t srcStartRayIndex = srcSweep->getStartRayIndex(); // getFirstRayIndex(srcVol, srcSweepIndex);
  size_t srcEndRayIndex = srcSweep->getEndRayIndex(); // getLastRayIndex(srcVol, srcSweepIndex);
  
  vector<RadxRay *> srcRays = srcVol->getRays();
  vector<RadxRay *> dstRays = dstVol->getRays();
  
  // Do we just want to insert all the rays from the source into the
  // destination? The ray indexes seem to stay the same. Yes.
  // This is probably the easiest.
 
  // go by offset, from 0 to srcEndRayIndex - srcStartRayIndex + 1;
  // because the indexes may be in different locations within the rays
  // The ray order should be the same between the source and the destination
  //for each source ray
  size_t nRaysInSweep = srcEndRayIndex - srcStartRayIndex + 1;
  // sanity check the ray indexes
  if (srcStartRayIndex + nRaysInSweep - 1 > srcEndRayIndex)
    throw "Error source indexes are not compatible";
  if (dstStartRayIndex + nRaysInSweep - 1 > dstEndRayIndex)
    throw "Error destination indexes are not compatible";
  if (nRaysInSweep != dstEndRayIndex - dstStartRayIndex + 1)
    throw "Error DataModel::overwriteSweepRays incompatible number of rays while merging sweeps";
  
  RadxRay *firstRay = srcRays.at(srcStartRayIndex);
  const vector<RadxField *> srcFields = firstRay->getFields();
  RadxRay *dstFirstRay = dstRays.at(dstStartRayIndex);
  vector<RadxField *> dstFields = dstFirstRay->getFields();
  
  for (int offset = 0; offset < nRaysInSweep; offset += 1) {
    // working here ...
    // for each field in src
    vector<RadxField *>::const_iterator srcIter;
    for (srcIter = srcFields.begin(); srcIter != srcFields.end(); ++srcIter ) {
      RadxField *srcField = *srcIter;
      string srcFieldName = srcField->getName();
      RadxRay *srcRay = srcRays.at(srcStartRayIndex + offset);
      RadxRay *dstRay = dstRays.at(dstStartRayIndex + offset);
      
      RadxField *srcFieldInCurrentRay = srcRay->getField(srcFieldName);
      if (srcField == NULL)
        throw "DataModel::overwriteSweepRays WHOA! We have a major error";
      string units = srcFieldInCurrentRay->getUnits();
      size_t nGates = srcRay->getNGates();
      Radx::fl32 missingValue = srcFieldInCurrentRay->getMissingFl32();
      Radx::fl32 *data = srcFieldInCurrentRay->getDataFl32();
      bool isLocal=true;
      
      // if field exists in dst
      RadxField *foundField = dstRay->getField(srcFieldName);
      if (foundField != NULL) {
          //vector<float> deref = *data;
          //const Radx::fl32 *radxData = &deref[0];
          if (nGates != dstRay->getNGates())
            throw "DataModel::overwriteSweepRays Another major error";
          foundField->setDataFl32(nGates, data, isLocal);
          
      } else {
        // add field to destination
        dstRay->addField(srcFieldName, units, nGates, missingValue, data, isLocal);
      }
    }
  }
}

// Make write no compression the default,
//  then turn compression on with final save of files.
// NOTE: side effect of changing the class variable _currentFilePath
void DataModel::writeData(string path, bool compressed) {
    RadxFile outFile;


    //outFile.setWriteCompressed(compressed);

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
void DataModel::writeData(string path, RadxVol *vol, bool compressed) {
    RadxFile outFile;

    //outFile.setWriteCompressed(compressed);

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
int DataModel::mergeDataFiles(string dest_path, string source_path, string original_path) {

  writeWithMergeData(dest_path, source_path, original_path);
  return 0;

}
*/
void DataModel::update() {

}

void DataModel::renameField(string currentName, string newName) {
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

void DataModel::renameField(size_t rayIdx, string currentName, string newName) {
  RadxRay *ray = getRay(rayIdx);

  // renameField(oldName, newName);
  ray->renameField(currentName, newName);
  // loadFieldNameMap
  ray->loadFieldNameMap();

}

// copy from one field to another field 
// overwrite destination
void DataModel::copyField(size_t rayIdx, string fromFieldName, string toFieldName) {
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
void DataModel::copyField2(size_t rayIdx, string fromFieldName, string toFieldName) {
  RadxRay *ray = getRay(rayIdx);
  RadxField *srcField = fetchDataField(ray, fromFieldName);
  if (srcField != NULL) {
    RadxField *copy = new RadxField(*srcField);
    copy->setName(toFieldName);
    ray->addField(copy);
  }
}

bool DataModel::fieldExists(size_t rayIdx, string fieldName) {
  RadxRay *ray = getRay(rayIdx);
  try {
    RadxField *field = fetchDataField(ray, fieldName);
    if (field != NULL) return true;
    else return false;
  } catch (std::invalid_argument &ex) {
    return false;
  }
}

// TODO: combine with other form of the same method
RadxField *DataModel::fetchDataField(RadxVol *vol, RadxRay *ray, string &fieldName) {
  if (fieldName.length() <= 0) {
    cerr << "fieldName is empty!!" << endl;
  }

  if (vol == NULL) {
    throw std::invalid_argument("volume is NULL; no data found");
  }
  vol->loadRaysFromFields();

  vector<RadxField *> fields;
  try {
    fields = ray->getFields();
  
  // dataField = ray->getField(fieldName);  often this doesn't work, and
  // throws an vector exception that cannot be trapped.  Sometimes the
  // field name map is built and sometimes not, so avoid using this function.
  } catch (std::exception &ex) {
    string msg = "DataModel::fetchDataField unknown error occurred: ";
    msg.append(fieldName);
    throw std::invalid_argument(msg);
  }

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
    string msg = "DataModel::fetchDataField No field found in ray: ";
    msg.append(fieldName);
    throw std::invalid_argument(msg);
  }

  return dataField;
}

RadxField *DataModel::fetchDataField(RadxRay *ray, string &fieldName) {
  if (fieldName.length() <= 0) {
    cerr << "fieldName is empty!!" << endl;
  }
  //const vector<RadxRay *> rays = _vol->getRays();
  //if ((ray < rays.at(0) || (ray > rays.at(rays.size()-1)))) {
    //throw std::invalid_argument("ray is out of bounds!");
  //  cerr << "ray is out of bounds!";
  //}
  if (_vol == NULL) {
    throw std::invalid_argument("volume is NULL; no data found");
  }
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
    string msg = "DataModel::fetchDataField unknown error occurred: ";
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
    string msg = "DataModel::fetchDataField No field found in ray: ";
    msg.append(fieldName);
    throw std::invalid_argument(msg);
  }

  return dataField; 
}
/*
const RadxField *DataModel::fetchDataField(const RadxRay *ray, string &fieldName) {
  ray->loadFieldNameMap();
  RadxField *foundField = NULL;
  vector<RadxField *> fields = ray->getFields();
  for (size_t ii = 0; ii < fields.size(); ii++) {
    string name = fields[ii]->getName();
    if (name.compare(fieldName) == 0)
       foundField = fields[ii];
   }
   if (foundField == NULL) {
      string msg = "DataModel::fetchDataField No field found in ray: ";
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
    string msg = "DataModel::fetchDataField unknown error occurred: ";
    msg + fieldName;
    throw std::invalid_argument(msg);
  }
  if (dataField == NULL) {
    string msg = "DataModel::fetchDataField No field found in ray: ";
    msg + fieldName;
    throw std::invalid_argument(msg);
  }
  return dataField; 
  
}
*/

const float *DataModel::fetchData(RadxRay *ray, string &fieldName) {

    RadxField *dataField = fetchDataField(ray, fieldName);
    if (dataField != NULL)
      return dataField->getDataFl32();
    else
      return NULL;
}

// total number of rays in volume, for all sweeps
size_t DataModel::getNRays() { // string fieldName, double sweepAngle) {
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
size_t DataModel::getNRays(int sweepNumber) {
  _vol->loadRaysFromFields();
  RadxSweep *sweep = _vol->getSweepByNumber(sweepNumber);
  if (sweep == NULL) {
    throw std::invalid_argument("DataModel::getNRays: no sweep found");
  }
  size_t nRays = sweep->getNRays();
  return nRays;
}

// get the number of rays for a sweep
size_t DataModel::getNRaysSweepIndex(int sweepIndex) {
  _vol->loadRaysFromFields();
  const vector<RadxSweep *> sweeps = _vol->getSweeps();
  RadxSweep *sweep = sweeps.at(sweepIndex); 
  if (sweep == NULL) {
    throw std::invalid_argument("DataModel::getNRaysSweepIndex: bad sweep index");
  }
  size_t nRays = sweep->getNRays();
  return nRays;
}

// get the first ray for a sweep
size_t DataModel::getFirstRayIndex(int sweepIndex) {
  return getFirstRayIndex(_vol, sweepIndex);
}

// get the first ray for a sweep
size_t DataModel::getFirstRayIndex(RadxVol *vol, int sweepIndex) {
  if (sweepIndex < 0) {
    throw std::invalid_argument("DataModel::getFirstRayIndex: bad sweep index < 0");
  }
  vol->loadRaysFromFields();
  
  const vector<RadxSweep *> sweeps = vol->getSweeps();
  if (sweepIndex >= sweeps.size()) {
    throw std::invalid_argument("DataModel::getFirstRayIndex: sweep index > number of sweeps");
  }
  RadxSweep *sweep = sweeps.at(sweepIndex);  
  if (sweep == NULL) {
    throw std::invalid_argument("DataModel::getFirstRayIndex: bad sweep index");
  }
  size_t firstRayIndex = sweep->getStartRayIndex();
  return firstRayIndex;
}

// get the last ray for a sweep
size_t DataModel::getLastRayIndex(int sweepIndex) {
  return getLastRayIndex(_vol, sweepIndex);
}


size_t DataModel::getLastRayIndex(RadxVol *vol, int sweepIndex) {
  vol->loadRaysFromFields();
  
  const vector<RadxSweep *> sweeps = vol->getSweeps();
  if ((sweepIndex < 0) || (sweepIndex >= sweeps.size())) {
    string msg = "DataModel::getLastRayIndex sweepIndex out of bounds ";
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
int DataModel::getSweepNumber(int sweepIndex) {

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

double DataModel::getRayAzimuthDeg(size_t rayIdx) {
  _vol->loadRaysFromFields();
  const vector<RadxRay *>  &rays = _vol->getRays();
  RadxRay *ray = rays.at(rayIdx);
  return ray->getAzimuthDeg();
}

double DataModel::getRayNyquistVelocityMps(size_t rayIdx) {
  _vol->loadRaysFromFields();
  const vector<RadxRay *>  &rays = _vol->getRays();
  RadxRay *ray = rays.at(rayIdx);
  return ray->getNyquistMps();
}

vector<RadxRay *> &DataModel::getRays() {
  //const vector<RadxRay *>  &rays = vol->getRays();
	return _vol->getRays();
}

vector<RadxRay *> &DataModel::getRays(RadxVol *vol) {
  //const vector<RadxRay *>  &rays = vol->getRays();
  return vol->getRays();
}

RadxRay *DataModel::getRay(size_t rayIdx) {
  _vol->loadRaysFromFields();
  
  //  get the ray for this field 
  vector<RadxRay *>  &rays = _vol->getRays();

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
  _vol->loadRaysFromFields();
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol->getRays(); 
  LOG(DEBUG) << "first 20 rays in order ...";
  for (int i=0; i<20; i++) {
    RadxRay *ray = rays.at(i);
    LOG(DEBUG) << "ray Az = " << ray->getAzimuthDeg();
  }
}


int DataModel::getSweepNumber(float elevation) {
  LOG(DEBUG) << "enter: elevation = " << elevation;
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
  delete sweepAngles;

  if (!found) {
    throw std::invalid_argument("DataModel::getSweepNumber no sweep found for elevation ");
  }

  // use the index, i, to find the sweep number, because
  // the index may be different than the number, which is a label for a sweep.
  int sweepNumber = 0;
  if (_vol == NULL) {
    if (!_cacheMetaDataValid) {
      throw std::invalid_argument("DataModel::getSweepNumber _vol is NULL; cache not valid");
    } 
    LOG(DEBUG) << "_vol is NULL; cache is valid ";
    if ((i < 0) || (i >= _cacheSweepNumbers.size())) {
      throw std::invalid_argument("DataModel::getSweepNumber index out of bounds for cache");
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

int DataModel::getSweepIndexFromSweepNumber(int sweepNumber) {
  vector<RadxSweep *> sweeps = _vol->getSweeps();
  int idx = -1;
  for (int i = 0; i<sweeps.size(); i++) {
    if (sweeps.at(i)->getSweepNumber() == sweepNumber) {
      idx = i;
    }
  }
  if (idx < 0) {
    stringstream ss;
    ss << "DataModel::getSweepIndex: no sweep found with sweep number " << sweepNumber << endl;
    throw std::invalid_argument(ss.str());
  } else {
    return idx;
  }
  
}

int DataModel::getSweepIndexFromSweepAngle(float elevation) {
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
    ss << "DataModel::getSweepIndexFromSweepAngle no sweep found for elevation " <<
      elevation << endl;
    throw std::invalid_argument(ss.str());
  }
  return i;  
}

size_t DataModel::getSweepIndexInVolume(RadxVol *vol,
                                        float sweepAngle,
                                        int sweepNumber) {
  double elevation = sweepAngle;
  
  // -- use the sweep number
  vector<RadxSweep *> sweeps = vol->getSweeps();
  int idx = -1;
  for (int i = 0; i<sweeps.size(); i++) {
    if (sweeps.at(i)->getSweepNumber() == sweepNumber) {
      idx = i;
    }
  }
  if (idx < 0) {
    // -- use the sweep angle
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
      ss << "DataModel::getSweepIndexInVolume no sweep found for elevation " <<
        elevation << " or sweepNumber " << sweepNumber << endl;
      throw std::invalid_argument(ss.str());
    }
    idx = i;
  }
  return idx;
}

double DataModel::getSweepAngleFromSweepNumber(int sweepNumber) {
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
    ss << "DataModel::getSweepAngleFromSweepNumber no sweep found for sweep number " <<
      sweepNumber << endl;
    throw std::invalid_argument(ss.str());
  }

  return sweepAngles->at(i);    
}



vector<float> *DataModel::getRayData(size_t rayIdx, string fieldName) { // , int sweepHeight) {
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

size_t DataModel::getNGates(size_t rayIdx, string fieldName, double sweepHeight) {

// TODO: which sweep? 
  _vol->loadRaysFromFields();
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = _vol->getRays();
  if (rayIdx >= rays.size()) {
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

float DataModel::getMissingFl32(string fieldName) {
  //_vol->loadFieldsFromRays();
  if (_vol == NULL) return Radx::missingFl32;
  const RadxField *field = _vol->getFieldFromRay(fieldName);
  if (field == NULL) return Radx::missingFl32;
  
  Radx::fl32 missingValue = field->getMissingFl32();
  return (float) missingValue;
}
  
DataModel::DataModel() {
	init();
}

void DataModel::init() {
  _vol = NULL;
  _cacheMetaDataValid = false;
}

// ----- 

void DataModel::clearVolume() {
  delete _vol;
  _vol = NULL;
  _cacheMetaDataValid = false;

}

void DataModel::getLookAhead(string fileName) {

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
      errMsg += "DataModel::getLookAhead\n";
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

    getFieldNames(vol, &_lookAheadFieldNames);
    
    LOG(DEBUG) << "exit";
}  

void DataModel::deleteLookAhead() {
  _lookAheadSweepNumbers.clear();
  _lookAheadSweepAngles.clear();
  _lookAheadFields.clear();    
}

void DataModel::moveToLookAhead() {
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
const string &DataModel::getPathInUse() {
	return _vol->getPathInUse();
}

size_t DataModel::getNSweeps() {
	return _vol->getNSweeps();
}

/*
vector<double> *DataModel::getSweepAngles(string fileName) {
  if (_vol == NULL) {
    // we don't have an active volume. read the metadata for the file
    return getPossibleSweepAngles(fileName);
  } else {
    return getSweepAngles();
  }
}
*/

vector<double> *DataModel::getSweepAngles() {

  vector<double> *sweepAngles = new vector<double>;
  if (_vol == NULL) {
    // pull info from cache metadata
    if (!_cacheMetaDataValid) {
      throw std::invalid_argument("DataModel::getSweepAngles _vol is null; cache is invalid; no info on sweeps");
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

vector<int> *DataModel::getSweepNumbers() {

  vector<int> *sweepNumbers = new vector<int>;
  if (_vol == NULL) {
    // pull info from cache metadata
    if (!_cacheMetaDataValid) {
      throw std::invalid_argument("DataModel::getSweepNumbers _vol is null; cache is invalid; no info on sweeps");
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
const RadxPlatform &DataModel::getPlatform() {
  return _vol->getPlatform();
} 

void DataModel::getPredomRayGeom(double *startRangeKm, double *gateSpacingKm) {
  double startRange;
  double gateSpace;
  _vol->getPredomRayGeom(startRange, gateSpace);
  *startRangeKm = startRange;
  *gateSpacingKm = gateSpace;  
}


vector<string> *DataModel::getUniqueFieldNameList() {
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

float DataModel::getLatitudeDeg() {
  return _vol->getLatitudeDeg();
}

float DataModel::getLongitudeDeg() {
  return _vol->getLongitudeDeg();
}

float DataModel::getAltitudeKm() {
  return _vol->getAltitudeKm();
}

double DataModel::getRadarBeamWidthDegV() {
  return _vol->getRadarBeamWidthDegV();
}

double DataModel::getCfactorRotationCorr() {
  RadxCfactors *cfactors;
  if ((cfactors = _vol->getCfactors()) != NULL)
    return cfactors->getRotationCorr();
  else return 0.0;
}

void DataModel::getCfactors(double *rollCorr, double *rotCorr, 
  double *tiltCorr) {

  *rollCorr = 0.0;
  *rotCorr = 0.0;
  *tiltCorr = 0.0;
  const RadxCfactors *cfactors = _vol->getCfactors();
  if (cfactors != NULL) {
    *rollCorr = cfactors->getRollCorr();
    *rotCorr = cfactors->getRotationCorr();
    *tiltCorr = cfactors->getTiltCorr();
  }
}

/*
double DataModel::getAltitudeKmAgl() {

}
*/

const RadxGeoref *DataModel::getGeoreference(size_t rayIdx) {

  RadxRay *ray = getRay(rayIdx);
  // get the winds from the aircraft platform
  const RadxGeoref *georef = ray->getGeoreference();

  if (georef == NULL) {
    LOG(DEBUG) << "ERROR - georef is NULL";
    LOG(DEBUG) << "      trying to recover ...";
    _vol->setLocationFromStartRay();
    georef = ray->getGeoreference();
    if (georef == NULL) {
      throw "could not recover from missing georeference information";
    }
  } 
  return georef;
}

bool DataModel::getGeoreferenceApplied(size_t rayIdx) {
  RadxRay *ray = getRay(rayIdx);
  return ray->getGeorefApplied(); 
}

void DataModel::_setupVolRead(RadxFile &file, vector<string> &fieldNames,
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

// reads all sweeps
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
  
}

/*
vector<double> *DataModel::getPossibleSweepAngles(string fileName)
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
      errMsg += "DataModel::getPossibleSweepAngles\n";
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

vector<string> *DataModel::getPossibleFieldNames(string fileName)
{

  // read meta data
  LOG(DEBUG) << "enter";

  deleteLookAhead();
  getLookAhead(fileName);
 
  vector<string> *allFieldNames = new vector<string>;
  for (vector<string>::iterator iter = _lookAheadFieldNames.begin(); iter != _lookAheadFieldNames.end(); ++iter)
  {
    //string *field = *iter;
    //cout << field->getName() << endl;
    allFieldNames->push_back(*iter);
  }

    LOG(DEBUG) << "exit";
    return allFieldNames;
}


// may not be used; may be the same as getLookAhead??
void DataModel::readFileMetaData(string fileName)
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
      errMsg += "DataModel::getPossibleFieldNames\n";
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


size_t DataModel::findClosestRay(float azimuth, int sweepNumber) { // float elevation) {
  LOG(DEBUG) << "enter azimuth = " << azimuth << " sweepNumber = " << sweepNumber;

  DataModel *dataModel = DataModel::Instance();

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

