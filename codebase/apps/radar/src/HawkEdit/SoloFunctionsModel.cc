
#include <vector>
#include <iostream>
#include <regex>
#include <stdio.h>
#include <string.h>
#include <float.h>

#include "SoloFunctionsModel.hh"
//#include "RemoveAcMotion.cc" // This comes from an external library
#include "BoundaryPointEditor.hh"
#include "ScriptsDataController.hh"


#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxCfactors.hh>
#include <Radx/RadxGeoref.hh>
#include <toolsa/LogStream.hh>
#include <Solo/SoloFunctionsApi.hh>

using namespace std;


SoloFunctionsModel::SoloFunctionsModel() {
  _boundaryMask = NULL;
  _boundaryMaskSet = false;
  _boundaryMaskLength = 0;
}

void SoloFunctionsModel::reset(ScriptsDataController *scriptsDataController) {
  _scriptsDataController = scriptsDataController;
}

/*
void SoloFunctionsModel::reset(string &inputPath, bool debug_verbose, bool debug_extra,
  vector<string> *fieldNames) {
  try {
    // create the ScriptsDataModel, read the data, etc. 
    _scriptsDataController->readData(inputPath, *fieldNames, debug_verbose, debug_extra);
    //_lastRayIdx = _scriptsDataController->getNRays();
  } catch (std::invalid_argument &ex) {
    std::cout << ex.what() << std::endl;
  }  

}

void SoloFunctionsModel::writeData(string &path) {
  try {
    _scriptsDataController->writeData(path);
  } catch (std::invalid_argument &ex) {
    std::cout << ex.what() << std::endl;
  }  
}
*/

int SoloFunctionsModel::ConvertRadxPlatformToSoloRadarType(Radx::PlatformType_t platform) {

  int soloRadarType = 0; // default is ground
  switch (platform) {
  // PLATFORM_TYPE_NOT_SET = 0, ///< Initialized but not yet set
  //  PLATFORM_TYPE_FIXED = 1, ///< Radar is in a fixed location
  //  PLATFORM_TYPE_VEHICLE = 2, ///< Radar is mounted on a land vehicle
  //  PLATFORM_TYPE_SHIP = 3, ///< Radar is mounted on a ship
  //  PLATFORM_TYPE_AIRCRAFT = 4, ///< Foreward looking on aircraft
  //  PLATFORM_TYPE_AIRCRAFT_FORE = 5, ///< Foreward looking on aircraft
    case Radx::PLATFORM_TYPE_AIRCRAFT_AFT: // = 6, ///< Backward looking on aircraft
      soloRadarType = 2; // AIR_AFT
      break;
    case Radx::PLATFORM_TYPE_AIRCRAFT_TAIL: //  = 7, ///< Tail - e.g. ELDORA
      soloRadarType = 3; // AIR_TAIL
      break;
  //  PLATFORM_TYPE_AIRCRAFT_BELLY = 8, ///< Belly radar on aircraft
  //  PLATFORM_TYPE_AIRCRAFT_ROOF = 9, ///< Roof radar on aircraft
  //  PLATFORM_TYPE_AIRCRAFT_NOSE = 10, ///< radar in nose radome on aircraft
  //  PLATFORM_TYPE_SATELLITE_ORBIT = 11, ///< orbiting satellite
  //  PLATFORM_TYPE_SATELLITE_GEOSTAT = 12, ///< geostationary satellite  
  //  RadxPlatform platform _scriptsDataController::getPlatform();
    default:
      soloRadarType = 0; // GROUND

  }
  // convert from RadxPlatform to Solo radar type;
  // or just convert Solo radar types to Radx
  
  // use DoradeData.hh radar_type_t or lidar_type_t
/* Dorade radar types */
/*
# define           GROUND 0
# define         AIR_FORE 1
# define          AIR_AFT 2
# define         AIR_TAIL 3
# define           AIR_LF 4
# define             SHIP 5
# define         AIR_NOSE 6         
# define        SATELLITE 7
# define     LIDAR_MOVING 8
# define      LIDAR_FIXED 9
*/ 
  return soloRadarType;
}

void SoloFunctionsModel::ClearBoundaryMask() {
  delete[] _boundaryMask;
  _boundaryMask = NULL;
  _boundaryMaskLength = 0;
}

// call this for each new ray, since the azimuth changes each time the ray changes
void SoloFunctionsModel::SetBoundaryMask(//RadxVol *vol,
					 size_t rayIdx, //int sweepIdx, 
           bool useBoundaryMask,
           vector<Point> &boundaryPoints) {

  _boundaryMaskSet = true;

  bool determineMask = false;
  if (useBoundaryMask)
    determineMask = true;

  CheckForDefaultMask(rayIdx, //sweepIdx, 
    determineMask, boundaryPoints);
  //  SetBoundaryMaskOriginal(vol, rayIdx, sweepIdx);
}

const vector<bool> *SoloFunctionsModel::GetBoundaryMask() {
  vector<bool> *dataVector = new vector<bool>(_boundaryMaskLength);
  dataVector->assign(_boundaryMask, _boundaryMask+_boundaryMaskLength);
  return dataVector;
}


void SoloFunctionsModel::CheckForDefaultMask(size_t rayIdx, //int sweepIdx, 
  bool determineMask,
  vector<Point> &boundaryPoints) {

  
  LOG(DEBUG) << "enter";
  LOG(DEBUG) << " radIdx=" << rayIdx;
	     //<< " sweepIdx=" << sweepIdx;
 
  short *boundary;

  // TODO: make this a call to BoundaryPointModel?
  //BoundaryPointEditor *bpe = BoundaryPointEditor::Instance();
  //vector<Point> boundaryPoints = bpe->getWorldPoints();
  // vector<Point> myPoints = BoundaryPointEditor::Instance()->getBoundaryPoints("/media/sf_lrose/ncswp_SPOL_RHI_.nc", 0, 4, "Boundary1");  TODO
  
  //map boundaryPoints to a list of short/boolean the same size as the ray->datafield->ngates
  int nBoundaryPoints = boundaryPoints.size();
  LOG(DEBUG) << "nBoundaryPoints = " << nBoundaryPoints;

  // need nGates & do NOT delete unless the number of gates changes

  // can we reuse the boundary mask?  If it is all true, we can reuse it
  // The boundary mask will be all true if the nBoundaryPoints < 3
  // if we have less than three points, then it is NOT a boundary
  if ((nBoundaryPoints < 3) || !determineMask) { 
    // set default (all true) boundary mask; 
    SetDefaultMask(rayIdx);
  } else {
    DetermineBoundaryMask(rayIdx, boundaryPoints);
  } 
  LOG(DEBUG) << "exit";
}

void SoloFunctionsModel::SetDefaultMask(size_t rayIdx) {
  //vol->loadRaysFromFields();

  const RadxField *field;

  //  get the ray for this field 
  // for the boundary mask, all we care about is the geometry of the ray,
  // NOT the data values for each field.

  //const vector<RadxRay *>  &rays = ScriptsDataModel->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  //if (ray == NULL) {
  //  LOG(DEBUG) << "ERROR - ray is NULL";
  //  throw "Ray is null";
  //} 

  //if (rayIdx == 720) {
  //  cerr << "HERE!!! " << endl;
  //}

  //field = ray->getField(fieldName);
  size_t nGates = _scriptsDataController->getNGates(rayIdx); 
  LOG(DEBUG) << "there are nGates " << nGates;

  //if (nGates != _boundaryMaskLength) {
  //  // clear old mask
  //  if (_boundaryMask != NULL) {
  //    delete[] _boundaryMask;
  //  }
  
    // allocate new mask
    _boundaryMaskLength = nGates;
    _boundaryMask = new bool[_boundaryMaskLength];
  //}

  for (size_t i=0; i<nGates; i++) {
    _boundaryMask[i] = true;
  } 
}


void SoloFunctionsModel::DetermineBoundaryMask(size_t rayIdx, //int sweepIdx,
  vector<Point> &boundaryPoints) {
  SetBoundaryMaskOriginal(rayIdx, boundaryPoints);
}


// call this for each new ray, since the azimuth changes each time the ray changes
void SoloFunctionsModel::SetBoundaryMaskOriginal(size_t rayIdx, //int sweepIdx,
  vector<Point> &boundaryPoints) {
  
  LOG(DEBUG) << "enter";
  LOG(DEBUG) << " radIdx=" << rayIdx;
 
  short *boundary;

  // TODO: make this a call to BoundaryPointModel?
  //BoundaryPointEditor *bpe = BoundaryPointEditor::Instance();
  //vector<Point> boundaryPoints = bpe->getWorldPoints();
  // vector<Point> myPoints = BoundaryPointEditor::Instance()->getBoundaryPoints("/media/sf_lrose/ncswp_SPOL_RHI_.nc", 0, 4, "Boundary1");  TODO
  
  //map boundaryPoints to a list of short/boolean the same size as the ray->datafield->ngates
  int nBoundaryPoints = boundaryPoints.size();
  LOG(DEBUG) << "nBoundaryPoints = " << nBoundaryPoints;

  //---------  HERE -------
  // need nGates & do NOT delete unless the number of gates changes

  // can we reuse the boundary mask?  

  //if (_boundaryMask != NULL) {
  //  delete[] _boundaryMask;
  //}
 
  //--------- END HERE ----------


  long *xpoints = new long[nBoundaryPoints];
  long *ypoints = new long[nBoundaryPoints];

  // convert data models  ...  
  // and change coordinate systems from World points to
  // Solo/Data points.  This requires just a change of scale
  // from kilometers to meters.
  vector<Point>::iterator it;
  int i = 0;
  for (it = boundaryPoints.begin(); it != boundaryPoints.end(); it++) {
    xpoints[i] = (long) it->x * 1000;
    ypoints[i] = (long) it->y * 1000;
    i += 1;
  }

  //  we want the boundary algorithm to be outside of the individual f(x).  Because it applies to 
  //       all the f(x) in a script
  
  // Wait! The SoloFunctionsApi doesn't store state!!!! So, we need to 
  // pass around the boundary mask.  
  // get the boundary mask for this ray

  SoloFunctionsApi soloFunctionsApi;

  // TODO: Actually, I want to move this boundary stuff to a couple levels up 
  // because the boundary is for a particular ray; then the boundary
  // can be used with multiple functions ?? maybe NOT!

  // we need the cfactors applied BEFORE accessing this info!!!


  // wrestle this information out of the ray and radar volume ...

  float radar_origin_latitude = _scriptsDataController->getLatitudeDeg();
  float radar_origin_longitude = _scriptsDataController->getLongitudeDeg();
  float radar_origin_altitude = _scriptsDataController->getAltitudeKm() * 1000.0;
  float boundary_origin_tilt = 0.0;
  float boundary_origin_latitude = radar_origin_latitude; // 0.0;
  float boundary_origin_longitude = radar_origin_longitude; // 0.0;
  float boundary_origin_altitude = radar_origin_altitude; // 0.0;
  
  // =======

  //vol->loadRaysFromFields();

  //const RadxField *field;
  /*
  field = vol->getFieldFromRay(fieldName);
  if (field == NULL) {
    LOG(DEBUG) << "no RadxField found in volume";
    throw "No data field with name " + fieldName;;
  }
  */

  //  get the ray for this field 
  // for the boundary mask, all we care about is the geometry of the ray,
  // NOT the data values for each field.

  RadxRay *ray = _scriptsDataController->getRay(rayIdx); 
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  //field = ray->getField(fieldName);
  size_t nGates = ray->getNGates(); 
  LOG(DEBUG) << "there are nGates " << nGates;

  // =======

  _boundaryMaskLength = nGates;

  float gateSize = ray->getGateSpacingKm() * 1000.0;
  float distanceToCellNInMeters = ray->getStartRangeKm() * 1000.0;

  // need to do some conversions here ...
  // TODO: get these from SoloLibrary::dd_math.h
  Radx::PlatformType_t platform = _scriptsDataController->getPlatform().getPlatformType();
  // convert from RadxPlatform to Solo radar type;
  // or just convert Solo radar types to Radx
  
  // use DoradeData.hh radar_type_t or lidar_type_t
/* Dorade radar types */
/*
# define           GROUND 0
# define         AIR_FORE 1
# define          AIR_AFT 2
# define         AIR_TAIL 3
# define           AIR_LF 4
# define             SHIP 5
# define         AIR_NOSE 6         
# define        SATELLITE 7
# define     LIDAR_MOVING 8
# define      LIDAR_FIXED 9
*/ 
  int radar_scan_mode = 1; // PPI; // TODO: need to get this: either RHI or PPI? 
  int radar_type = ConvertRadxPlatformToSoloRadarType(platform);
 
  float tilt_angle = 0.0; // TODO: It should be this ... ray->getElevationDeg();

  /* TODO: if Y-Prime radar, then send the track-relative rotation for the azimuth.
      PRIMARY_AXIS_Z = 0, ///< vertical
    PRIMARY_AXIS_Y = 1, ///< longitudinal axis of platform
    PRIMARY_AXIS_X = 2, ///< lateral axis of platform
    PRIMARY_AXIS_Z_PRIME = 3, ///< inverted vertical
    PRIMARY_AXIS_Y_PRIME = 4, ///< ELDORA, HRD tail
    PRIMARY_AXIS_X_PRIME = 5  ///< translated lateral
  */ 
  Radx::PrimaryAxis_t primary_axis = _scriptsDataController->getPrimaryAxis();
  
  
  float azimuth = ray->getAzimuthDeg();
  if (primary_axis == Radx::PRIMARY_AXIS_Y_PRIME) {
    RadxGeoref *georef = ray->getGeoreference();
    if (georef == NULL) {
      LOG(DEBUG) << "ERROR - georef for ray is NULL";
      string msg = "Georef for ray is null. Cannot apply boundary";
      throw msg;
    }
    azimuth = georef->getTrackRelRot();
    if (azimuth == Radx::missingMetaDouble) {
      LOG(DEBUG) << "ERROR - track-relative rotation is missing";
      string msg = "Track-relative rotation is missing.  Cannot apply boundary";
      throw msg;
    }
  }

  // TODO: need to fix this!  sending bool*, expecting short*
  _boundaryMask = new bool[_boundaryMaskLength];
 
  soloFunctionsApi.GetBoundaryMask(xpoints, ypoints, nBoundaryPoints,
                         radar_origin_latitude,
                         radar_origin_longitude,
                         radar_origin_altitude,
                         boundary_origin_tilt,
                         boundary_origin_latitude,
                         boundary_origin_longitude,
                         boundary_origin_altitude,
				   nGates,
				   gateSize,
				   distanceToCellNInMeters,
				   azimuth,
				   radar_scan_mode,
				   radar_type,
				   tilt_angle,
				   _boundaryMask);
 
  printBoundaryMask();

  delete[] xpoints;
  delete[] ypoints;

  LOG(DEBUG) << "exit"; 

}

// Should this move to ScriptsDataModel???  Yes.
/*
// return data for the field, at the sweep and ray index
const vector<float> *SoloFunctionsModel::GetData(string fieldName,  RadxVol *vol,
              int rayIdx, int sweepIdx)  {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx
       << " sweepIdx=" << sweepIdx;

  vol->loadRaysFromFields();
  
  const RadxField *field;

  //  get the ray for this field 
  const vector<RadxRay *>  &rays = vol->getRays();
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

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();

  vector<float> *dataVector = new vector<float>(nGates);
  dataVector->assign(data, data+nGates);

  return dataVector;
}

void SoloFunctionsModel::SetData(string &fieldName, RadxVol *vol,
            int rayIdx, int sweepIdx, vector<float> *fieldData) { 

  // What is being returned? the name of the new field in the model that
  // contains the results.

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;

  // gather data from context -- most of the data are in a DoradeRadxFile object

  // TODO: convert the context RadxVol to DoradeRadxFile and DoradeData format;
  //RadxVol vol = context->_vol;
  // make sure the radar angles have been calculated.

  vol->loadRaysFromFields(); // loadFieldsFromRays();

  const RadxField *field;
  //  field = vol->getFieldFromRay(fieldName);
  //  if (field == NULL) {
  //    LOG(DEBUG) << "no RadxField found in volume";
  //    throw "No data field with name " + fieldName;;
  //  }
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = vol->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  RadxRay *ray = rays.at(rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 
*/
  /*
  const RadxGeoref *georef = ray->getGeoreference();
  if (georef == NULL) {
    LOG(DEBUG) << "ERROR - georef is NULL";
    LOG(DEBUG) << "      trying to recover ...";
    vol->setLocationFromStartRay();
    georef = ray->getGeoreference();
    if (georef == NULL) {
      throw "Georef is null";
    }
  } 
  */
/*
  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  //field = fetchDataField(ray, fieldName);
  //size_t nGates = ray->getNGates(); 
  
  //float *newData = new float[nGates];

  int nGates = fieldData->size();
  // cerr << "there arenGates " << nGates;

  Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = true;
  const float *flatData = fieldData->data();

  //RadxField *newField = new RadxField(newFieldName, "m/s");
  //newField->copyMetaData(*field);
  //newField->addDataFl32(nGates, newData);
  RadxField *field1 = ray->addField(fieldName, "m/s", nGates, missingValue, flatData, isLocal);

  //string tempFieldName = field1->getName();
  //tempFieldName.append("#");

  LOG(DEBUG) << "exit ";

}
*/

//
// When a new field or ray is added, 
// isLocal = true, to manage the memory in this class.
// once, the script is complete, use loadFieldsFromRays (in ScriptsDataModel::writeData)
// to move all the new fields to the volume for writing.
//

// return the temporary name for the new field in the volume
string SoloFunctionsModel::ZeroMiddleThird(string fieldName,  // RadxVol *vol,
					   size_t rayIdx, // int sweepIdx,
					   string newFieldName) {
  LOG(DEBUG) << "entry with fieldName ... " << fieldName;
  // << " radIdx=" << rayIdx
	//     << " sweepIdx=" << sweepIdx;

  RadxRay *ray = _scriptsDataController->getRay(rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  const RadxField *field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 
  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  float *newData = new float[nGates];
  for (int i=0; i<10; i++)
    newData[i] = data[i];   
  for (int i=10; i<30; i++)
    newData[i] = 0;
  for (int i=30; i<nGates; i++)
    newData[i] = data[i];   

  // insert new field into RadxVol                                                                             
  LOG(DEBUG) << "result = ";
  for (int i=0; i<50; i++)
    LOG(DEBUG) << newData[i] << ", ";
  
  // I have the ray, can't I just add a field to it?
                                                                  
  Radx::fl32 missingValue = Radx::missingFl32;
  bool isLocal = true;

  RadxField *field1 = ray->addField(newFieldName, "m/s", nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::CopyField(string fieldName,
             size_t rayIdx, // int sweepIdx,
             string newFieldName) {
  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
     //  << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 
  
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  const float *data = field->getDataFl32();
  float *newData = new float[nGates];
  memcpy(newData, data, nGates*sizeof(float));
/*
  for (int i=0; i<10; i++)
    newData[i] = data[i];   
  for (int i=10; i<30; i++)
    newData[i] = 0;
  for (int i=30; i<nGates; i++)
    newData[i] = data[i];   
*/
  // insert new field into RadxVol                                                                             
  LOG(DEBUG) << "result = ";
  for (int i=0; i<50; i++)
    LOG(DEBUG) << newData[i] << ", ";
  
  // I have the ray, can't I just add a field to it?
                                                                     
  Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = true;
  string field_units = field->getUnits();
  RadxField *field1 = ray->addField(newFieldName, field_units, nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return ""; // tempFieldName;
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::ZeroInsideBoundary(string fieldName,  //RadxVol *vol,
					   size_t rayIdx, // int sweepIdx,
					   string newFieldName) {
  LOG(DEBUG) << "entry with fieldName ... " << fieldName; 

  RadxRay *ray = _scriptsDataController->getRay(rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  const RadxField *field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  float *newData = new float[nGates];

  // =======
  // data, _boundaryMask, and newData should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {

    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";

    // cerr << "there arenGates " << nGates;
    const float *data = field->getDataFl32();
  
    // perform the function ...
    soloFunctionsApi.ZeroInsideBoundary(data, _boundaryMask, newData, nGates);
  /*
    if (_boundaryMaskLength != nGates) 
      throw "Error: boundaryMaskLength not equal to nGates (ZeroInsideBoundary)";
    for (int i=0; i<_boundaryMaskLength; i++) {
      if (_boundaryMask[i]) 
	newData[i] = data[i];
      else
	newData[i] = 0.0;
    }
  */
  } else {  // no _boundaryMaskSet

    const float *data = field->getDataFl32();
    soloFunctionsApi.ZeroInsideBoundary(data, NULL, newData, nGates);

  }

  // insert new field into RadxVol                                                                             
  LOG(DEBUG) << "result = ";
  for (int i=0; i<50; i++)
    LOG(DEBUG) << newData[i] << ", ";
  
  Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = true;

  RadxField *field1 = ray->addField(newFieldName, "m/s", nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}


// return the temporary name for the new field in the volume
string SoloFunctionsModel::Despeckle(string fieldName,  //RadxVol *vol,
				     size_t rayIdx, //int sweepIdx,
				     size_t speckle_length,
				     size_t clip_gate,
				     float bad_data_value,
				     string newFieldName) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName;
  //  << " radIdx=" << rayIdx
	//     << " sweepIdx=" << sweepIdx;

  //vol->loadRaysFromFields();
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = ScriptsDataModel->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  //   field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  float *newData = new float[nGates];

  // data, _boundaryMask, and newData should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;


  if (_boundaryMaskSet) { //  && _boundaryMaskLength >= 3) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";

  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();

  Radx::fl32 missingValue = field->getMissingFl32();
  if (bad_data_value == FLT_MIN) {
    bad_data_value = missingValue;
  }  
  
  // perform the function ...
  soloFunctionsApi.Despeckle(data,  newData, nGates, bad_data_value, speckle_length,
			     clip_gate, _boundaryMask);

  // insert new field into RadxVol                                                                             
  LOG(DEBUG) << "result = ";
  for (int i=0; i<50; i++)
    LOG(DEBUG) << newData[i] << ", ";


  // Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = true;

  //RadxField *newField = new RadxField(newFieldName, "m/s");
  //newField->copyMetaData(*field);
  //newField->addDataFl32(nGates, newData);
  RadxField *field1 = ray->addField(newFieldName, "m/s", nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

string SoloFunctionsModel::RemoveAircraftMotion(string fieldName, //RadxVol *vol,
						size_t rayIdx, // int sweepIdx,
						float nyquist_velocity,
            bool use_radar_angles,
						size_t clip_gate,
						float bad_data_value,
						string newFieldName) { 

  // What is being returned? the name of the new field in the model that
  // contains the results.

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;

  // gather data from context -- most of the data are in a DoradeRadxFile object

  // TODO: convert the context RadxVol to DoradeRadxFile and DoradeData format;
  //RadxVol vol = context->_vol;
  // make sure the radar angles have been calculated.

  //vol->loadRaysFromFields(); // loadFieldsFromRays();

  const RadxField *field;

  //  field = vol->getFieldFromRay(fieldName);
  //  if (field == NULL) {
  //    LOG(DEBUG) << "no RadxField found in volume";
  //    throw "No data field with name " + fieldName;;
  //  }
  
  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx); 
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  /*
  const RadxGeoref *georef = ray->getGeoreference();
  if (georef == NULL) {
    LOG(DEBUG) << "ERROR - georef is NULL";
    LOG(DEBUG) << "      trying to recover ...";
    vol->setLocationFromStartRay();
    georef = ray->getGeoreference();
    if (georef == NULL) {
      throw "Remove Aircraft Motion: Georef is null. Cannot find vert_velocity, ew_velocity, ns_velocity.";
    }
  }
  */ 

  SoloFunctionsApi soloFunctionsApi;

  const RadxGeoref *georef = _scriptsDataController->getGeoreference(rayIdx);
  if (georef == NULL) {
    throw "Remove Aircraft Motion: Georef is null. Cannot find vert_velocity, ew_velocity, ns_velocity.";
  } 

  float tilt = georef->getTrackRelTilt() * Radx::DegToRad;
  // TODO: elevation changes with different rays/fields how to get the current one???
  float elevation = georef->getTrackRelEl() * Radx::DegToRad;

  float dds_radd_eff_unamb_vel = ray->getNyquistMps(); // doradeData.eff_unamb_vel;
  float seds_nyquist_velocity = nyquist_velocity; // TODO: what is this value?

  //==========  

  float new_az = 0.0;
  
  if (use_radar_angles) {
    // these are input args to radar angles calculation
    float asib_roll = georef->getRoll();
    float asib_pitch = georef->getPitch();
    float asib_heading = georef->getHeading();
    float asib_drift_angle = georef->getDrift();
    float asib_rotation_angle = georef->getRotation();
    float asib_tilt = georef->getTilt();
    float cfac_pitch_corr = _scriptsDataController->getCfactorPitchCorr();
    float cfac_heading_corr = _scriptsDataController->getCfactorHeadingCorr();
    float cfac_drift_corr = _scriptsDataController->getCfactorDriftCorr();
    float cfac_roll_corr = _scriptsDataController->getCfactorRollCorr();
    float cfac_elevation_corr = _scriptsDataController->getCfactorElevationCorr();
    float cfac_azimuth_corr = _scriptsDataController->getCfactorAzimuthCorr();
    float cfac_rot_angle_corr = _scriptsDataController->getCfactorRotationCorr();
    float cfac_tilt_corr = _scriptsDataController->getCfactorTiltCorr();

    if (_scriptsDataController->getPlatform().getPlatformType() != Radx::PLATFORM_TYPE_AIRCRAFT_TAIL) {
      throw "SoloII radar_angles only works with Tail radar";
    } 
    int radar_type =  3; // hard code to TAIL    // from dgi->dds->radd->radar_type
    // TODO: convert the Radx::Platform enum to the SoloII int for the radar type
    //switch (_scriptsDataController->getPlatform().getPlatformType()) {
    //case ...
    //}

    bool use_Wen_Chaus_algorithm = true;
    float dgi_dds_ryib_azimuth = ray->getAzimuthDeg();
    float dgi_dds_ryib_elevation = ray->getElevationDeg();
    // these are output args to radar angles calculation
    float  ra_x;
    float  ra_y;
    float  ra_z;
    float  ra_rotation_angle;
    float  ra_tilt;
    float  ra_azimuth;
    float  ra_elevation;
    float  ra_psi;  

    soloFunctionsApi.CalculateRadarAngles( 
       asib_roll,
       asib_pitch,
       asib_heading,
       asib_drift_angle,
       asib_rotation_angle,
       asib_tilt,
       cfac_pitch_corr,
       cfac_heading_corr,
       cfac_drift_corr,
       cfac_roll_corr,
       cfac_elevation_corr,
       cfac_azimuth_corr,
       cfac_rot_angle_corr,
       cfac_tilt_corr,
       radar_type,  // from dgi->dds->radd->radar_type
       use_Wen_Chaus_algorithm,
       dgi_dds_ryib_azimuth,
       dgi_dds_ryib_elevation,
       &ra_x,
       &ra_y,
       &ra_z,
       &ra_rotation_angle,
       &ra_tilt,
       &ra_azimuth,
       &ra_elevation,
       &ra_psi
    );
    // adjust the azimuth to earth-relative coordinates Solo::radar_angles does this ...
    //float T = asib_heading + cfac_heading_corr + asib_drift_angle + cfac_drift_corr;
    //ra_azimuth = ra_azimuth * Radx::RadToDeg + T;
    new_az = ra_azimuth * Radx::RadToDeg;
    tilt = ra_tilt; //  * Radx::DegToRad;
    elevation = ra_elevation; //  * Radx::DegToRad; // doradeData.elevation; // fl32;
    //ray->setElevationDeg(ra_elevation * Radx::RadToDeg);
    //dds_ra_elevation = ra_elevation; // not sure if in degrees or radians:  * M_PI / 180.00; 
    float other_az; //  = new_az; // atan2(ra_y, ra_x) * Radx::RadToDeg;

    //if ((ray->getAzimuthDeg() > 90.0) && (ray->getAzimuthDeg() < 180.0)) {
       //other_az = 360 - new_az;
    //}


    float T_deg = (georef->getHeading() + cfac_heading_corr)  + 
              (georef->getDrift() + cfac_drift_corr);
    float T = T_deg * Radx::DegToRad;
    float ra_x2 = cos(T) * ra_x + sin(T) * ra_y;
    float ra_y2 = - sin(T) * ra_x + cos(T) * ra_y;

    if (ra_x2 == 0.0) {
      if (ra_y2 > 0.0) {
        other_az = 90.0;
      } else {
        other_az = 270.0;
      }
    } else {
      double tg = ra_y2/ra_x2;
      other_az = atan(tg) * Radx::RadToDeg;
      //other_az = atan(ra_y2, ra_x2) * Radx::RadToDeg; //  + T_deg;
      if (ra_x2 > 0.0) { // all good no changes 
      } 
      if ((ra_x2 < 0.0) && (ra_y2 >= 0.0)) {
        other_az += 180.0;
      }
      if ((ra_x2 < 0.0) && (ra_y2 < 0.0)) {
        other_az -= 180.0;
      }
    }
    //other_az = other_az - 2 * T_deg;

    
    cerr << "new_az= " << new_az << " ra_x= " << ra_x << " ra_y= " << ra_y << 
          " ra_x2= " << ra_x2 << " ra_y2= " << ra_y2 <<
          " orig_az= " << ray->getAzimuthDeg() <<
          " T_deg= " << T_deg <<
          " other_az= " << other_az
          << endl;

    new_az = ra_rotation_angle * Radx::RadToDeg;
  }

 
  float vert_velocity = georef->getVertVelocity();  // fl32
  float ew_velocity = georef->getEwVelocity(); // fl32
  float ns_velocity = georef->getNsVelocity(); // fl32;

  float ew_gndspd_corr = 0.0; // I don't know what this is or where to find it.
  const RadxCfactors *cfactors = ray->getCfactors();
  if (cfactors != NULL) {
    ew_gndspd_corr = cfactors->getEwVelCorr(); // ?? _gndspd_corr; // fl32;
  }
 
  //  cerr << "sizeof(short) = " << sizeof(short);
  //if (sizeof(short) != 16) 
  //  throw "FATAL ERROR: short is NOT 16 bits! Exiting.";
  LOG(DEBUG) << "args: ";
  LOG(DEBUG) << "vert_velocity " << vert_velocity;
  LOG(DEBUG) <<   "ew_velocity " << ew_velocity;
  LOG(DEBUG) <<   "ns_velocity " << ns_velocity;
  LOG(DEBUG) <<   "ew_gndspd_corr " << ew_gndspd_corr;
  LOG(DEBUG) <<   "tilt " << tilt;
  LOG(DEBUG) <<   "elevation " << elevation;
  //LOG(DEBUG) <<   "bad " << bad;
  //  LOG(DEBUG) <<   "parameter_scale " << parameter_scale;
  // LOG(DEBUG) <<   "dgi_clip_gate " << dgi_clip_gate;
  LOG(DEBUG) <<   "dds_radd_eff_unamb_vel " << dds_radd_eff_unamb_vel;
  LOG(DEBUG) <<   "seds_nyquist_velocity " << "??";


  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  float *newData = new float[nGates];

  if (_boundaryMaskSet) { //  && _boundaryMaskLength >= 3) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";

  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();

  Radx::fl32 missingValue = field->getMissingFl32();
  if (bad_data_value == FLT_MIN) {
    bad_data_value = missingValue;
  }
  
  //==========

  // TODO: data, _boundaryMask, and newData should have all the same dimensions = nGates


  /*						     
  soloFunctionsApi.RemoveAircraftMotion(vert_velocity, ew_velocity, ns_velocity,
						     ew_gndspd_corr, tilt, elevation,
						     fakeData,
						     bad, parameter_scale, parameter_bias, dgi_clip_gate,
						     dds_radd_eff_unamb_vel, seds_nyquist_velocity,
						     _boundaryMask);
  */
  // perform the function ...
  //  soloFunctionsApi.Despeckle(data,  newData, nGates, bad_data_value, speckle_length,
  //                             clip_gate, _boundaryMask);
  // all angles should be in radians
  soloFunctionsApi.RemoveAircraftMotion(vert_velocity, ew_velocity, ns_velocity,
					ew_gndspd_corr, tilt, elevation,
					data, newData, nGates,
					bad_data_value, clip_gate,
					dds_radd_eff_unamb_vel, seds_nyquist_velocity,
					_boundaryMask);

  // insert new field into RadxVol   
  
  //Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = true;

  //RadxField *newField = new RadxField(newFieldName, "m/s");
  //newField->copyMetaData(*field);
  //newField->addDataFl32(nGates, newData);
  RadxField *field1 = ray->addField(newFieldName, "m/s", nGates, missingValue, newData, isLocal);

  // replace azimuth with rotation only for display
  if (use_radar_angles) {
     ray->setAzimuthDeg(new_az);
  }
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  LOG(DEBUG) << "exit ";

  return tempFieldName;
}

string SoloFunctionsModel::RemoveOnlySurface(string fieldName,
            size_t rayIdx, //int sweepIdx,

     float optimal_beamwidth,      // script parameter; origin seds->optimal_beamwidth
     int seds_surface_gate_shift,       // script parameter; origin seds->surface_gate_shift
     bool getenv_ALTERNATE_GECHO,  // script parameter
     double d, // used for min_grad, if getenv_ALTERNATE_GECHO is true
               // d = ALTERNATE_GECHO environment variable

            size_t clip_gate,
            float bad_data_value,
            string newFieldName) { 

  // What is being returned? the name of the new field in the model that
  // contains the results.

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;

  // gather data from context -- most of the data are in a DoradeRadxFile object

  // TODO: convert the context RadxVol to DoradeRadxFile and DoradeData format;
  //RadxVol vol = context->_vol;


  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx); 
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  /* make sure the radar angles have been calculated.  
  // this does NOT see to be functioning
  if (!ray->getGeorefApplied()) {  
    LOG(DEBUG) << "ERROR - georefs/cfac have not been applied";
    throw "Georefs have not been applied";
  } 
  */

  //Radx::PrimaryAxis_t primary_axis = _scriptsDataController->getPrimaryAxis();
  //bool force = true;
  //ray->applyGeoref(primary_axis, force);

  /*
  const RadxGeoref *georef = ray->getGeoreference();
  if (georef == NULL) {
    LOG(DEBUG) << "ERROR - georef is NULL";
    LOG(DEBUG) << "      trying to recover ...";
    vol->setLocationFromStartRay();
    georef = ray->getGeoreference();
    if (georef == NULL) {
      throw "Remove Aircraft Motion: Georef is null. Cannot find vert_velocity, ew_velocity, ns_velocity.";
    }
  }
  */ 

  const RadxGeoref *georef = _scriptsDataController->getGeoreference(rayIdx);
  if (georef == NULL) {
    throw "Remove Only Surface: Georef is null. Cannot find asib_altitude_agl.";
  }  
 
// ----  need these values from cfac/georef

  // get from platform
  // TODO: should these be in radians?
     double dds_asib_rotation_angle = georef->getRotation(); // origin dds->asib->rotation_angle;  asib is struct platform_i
     double dds_asib_roll = georef->getRoll();          // origin dds->asib->roll
     float asib_altitude_agl = georef->getAltitudeKmAgl();     // altitude angle ??? from platform??

  // get from cfac info
  //const RadxCfactors *cfactors = ray->getCfactors();
  double dds_cfac_rot_angle_corr = _scriptsDataController->getCfactorRotationCorr(); // origin dds->cfac->rot_angle_corr; cfac is struct correction_d

// ---

  //float vert_velocity = georef->getVertVelocity();  // fl32

  // TODO: elevation changes with different rays/fields how to get the current one???
  float elevation = ray->getElevationDeg(); // doradeData.elevation; // fl32;
  float dds_ra_elevation = elevation * Radx::DegToRad; // radar angles!! requires cfac values and calculation
                           // origin dds->ra->elevation, ra = radar_angles
                           // get this from RadxRay::_elev if RadxRay::_georefApplied == true
                           // 1/4/2023 I don't believe that cfacs need to be applied to elevation!!
  float vert_beam_width = _scriptsDataController->getRadarBeamWidthDegV(); // from platform radarBeamWidthDegV; origin dgi->dds->radd->vert_beam_width
  float radar_latitude = _scriptsDataController->getLatitudeDeg(); // radar->latitude 

  LOG(DEBUG) << "args: ";
  LOG(DEBUG) << "ra_elevation (radians) " << dds_ra_elevation; 
  LOG(DEBUG) <<   "radar_latitude " << radar_latitude;
  LOG(DEBUG) <<   "vert_beam_width (degrees) " << vert_beam_width;
  //LOG(DEBUG) <<   "bad " << bad;
  //  LOG(DEBUG) <<   "parameter_scale " << parameter_scale;
  // LOG(DEBUG) <<   "dgi_clip_gate " << dgi_clip_gate;

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  double startRange;
  double gateSpace;
  _scriptsDataController->getPredomRayGeom(&startRange, &gateSpace);

  float gate_size = gateSpace;
  float distance_to_first_gate = startRange;
  double max_range = ray->getMaxRangeKm();    // internal value; origin dds->celvc_dist_cells[dgi_clip_gate];

  float *newData = new float[nGates];

  if (_boundaryMaskSet) { //  && _boundaryMaskLength >= 3) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";

  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();

  Radx::fl32 missingValue = field->getMissingFl32();
  if (bad_data_value == FLT_MIN) {
    bad_data_value = missingValue;
  }

  SoloFunctionsApi soloFunctionsApi; 

  //==========  
  
  if (false) { // TODO: add this arg ... use_radar_angles) {
    // these are input args to radar angles calculation
    float asib_roll = dds_asib_roll;
    float asib_pitch = georef->getPitch();
    float asib_heading = georef->getHeading();
    float asib_drift_angle = georef->getDrift();
    float asib_rotation_angle = dds_asib_rotation_angle;
    float asib_tilt = georef->getTilt();
    float cfac_pitch_corr = _scriptsDataController->getCfactorPitchCorr();
    float cfac_heading_corr = _scriptsDataController->getCfactorHeadingCorr();
    float cfac_drift_corr = _scriptsDataController->getCfactorDriftCorr();
    float cfac_roll_corr = _scriptsDataController->getCfactorRollCorr();
    float cfac_elevation_corr = _scriptsDataController->getCfactorElevationCorr();
    float cfac_azimuth_corr = _scriptsDataController->getCfactorAzimuthCorr();
    float cfac_rot_angle_corr = dds_cfac_rot_angle_corr;
    float cfac_tilt_corr = _scriptsDataController->getCfactorTiltCorr();

    int radar_type =  6; // TODO: FIX!!! hard-coding to AIR_NOSE    // from dgi->dds->radd->radar_type
    // TODO: convert the Radx::Platform enum to the SoloII int for the radar type
    //switch (_scriptsDataController->getPlatform().getPlatformType()) {
    //case ...
    //}

    // radar_type == AIR_LF || radar_type == AIR_NOSE
    // this must translate to Radx::PlatformType_t
    // PLATFORM_TYPE_AIRCRAFT_NOSE = 10,
    // 

    bool use_Wen_Chaus_algorithm = false;
    float dgi_dds_ryib_azimuth = ray->getAzimuthDeg();
    float dgi_dds_ryib_elevation = ray->getElevationDeg();
    // these are output args to radar angles calculation
    float  ra_x;
    float  ra_y;
    float  ra_z;
    float  ra_rotation_angle;
    float  ra_tilt;
    float  ra_azimuth;
    float  ra_elevation;
    float  ra_psi;  



    soloFunctionsApi.CalculateRadarAngles( 
       asib_roll,
       asib_pitch,
       asib_heading,
       asib_drift_angle,
       asib_rotation_angle,
       asib_tilt,
       cfac_pitch_corr,
       cfac_heading_corr,
       cfac_drift_corr,
       cfac_roll_corr,
       cfac_elevation_corr,
       cfac_azimuth_corr,
       cfac_rot_angle_corr,
       cfac_tilt_corr,
       radar_type,  // from dgi->dds->radd->radar_type
       use_Wen_Chaus_algorithm,
       dgi_dds_ryib_azimuth,
       dgi_dds_ryib_elevation,
       &ra_x,
       &ra_y,
       &ra_z,
       &ra_rotation_angle,
       &ra_tilt,
       &ra_azimuth,
       &ra_elevation,
       &ra_psi
    );
    dds_ra_elevation = ra_elevation; // not sure if in degrees or radians:  * M_PI / 180.00; 
  }

  //soloFunctionsApi.RemoveAircraftMotion(vert_velocity, ew_velocity, ns_velocity,
  //        ew_gndspd_corr, tilt, elevation,
  //        data, newData, nGates,
  //        bad_data_value, clip_gate,
  //        dds_radd_eff_unamb_vel, seds_nyquist_velocity,
  //        _boundaryMask);
  
  soloFunctionsApi.RemoveOnlySurface(
    optimal_beamwidth,      // script parameter; origin seds->optimal_beamwidth
    seds_surface_gate_shift,       // script parameter; origin seds->surface_gate_shift
    vert_beam_width,        // from radar angles???; origin dgi->dds->radd->vert_beam_width
    asib_altitude_agl,      // altitude angle ???
    dds_ra_elevation,       // radar angles!! requires cfac values and calculation
                           // origin dds->ra->elevation, ra = radar_angles
                           // get this from RadxRay::_elev if RadxRay::_georefApplied == true
    getenv_ALTERNATE_GECHO,  // script parameter
    d, // used for min_grad, if getenv_ALTERNATE_GECHO is true
               // d = ALTERNATE_GECHO environment variable
    dds_asib_rotation_angle,  // origin dds->asib->rotation_angle;  asib is struct platform_i
    dds_asib_roll,            // origin dds->asib->roll
    dds_cfac_rot_angle_corr,  // origin dds->cfac->rot_angle_corr; cfac is struct correction_d
    radar_latitude,  // radar->latitude 
    data,     // internal value
    newData,       // internal value
    nGates,         // internal value
    gate_size,
    distance_to_first_gate,
    max_range,      // internal value; origin dds->celvc_dist_cells[dgi_clip_gate];
    bad_data_value,  // default value
    clip_gate,  // default value
    _boundaryMask);

  // insert new field into RadxVol     
  bool isLocal = true;

  string field_units = field->getUnits();

  RadxField *field1 = ray->addField(newFieldName, field_units, nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  LOG(DEBUG) << "exit ";

  return tempFieldName;
}




string SoloFunctionsModel::BBUnfoldFirstGoodGate(string fieldName, //RadxVol *vol,
						size_t rayIdx, //int sweepIdx,
						float nyquist_velocity,
						int max_pos_folds,
						int max_neg_folds,
						size_t ngates_averaged,
						size_t clip_gate,
						    float bad_data_value, // TODO: pull this from data file?
						string newFieldName) { 

  // find the ray and data

  // get the missing data value

  // What is being returned? the name of the new field in the model that
  // contains the results.

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;

  //vol->loadRaysFromFields(); // loadFieldsFromRays();

  const RadxField *field;

  //  field = vol->getFieldFromRay(fieldName);
  //  if (field == NULL) {
  //    LOG(DEBUG) << "no RadxField found in volume";
  //    throw "No data field with name " + fieldName;;
  //  }
  
  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "WARNING - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx); 
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  float dds_radd_eff_unamb_vel = ray->getNyquistMps(); // doradeData.eff_unamb_vel;
  int seds_nyquist_velocity = nyquist_velocity; //  what is this value? It is the nyquist velocity set by "one time only" commands

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  float *newData = new float[nGates];

  // data, _boundaryMask, and newData should have all the same dimensions = nGates
  if (_boundaryMaskSet) { //  && _boundaryMaskLength >= 3) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();

  // manage the last good v0, initialized from first good gate in a sweep;
  // and perpetuated for each ray in the sweep
  static float last_good_v0;
  float missingValue = field->getMissingFl32();

  bool firstRayInSweep = rayIdx == 0;
  float last_good_v0_passable = missingValue;
  if (firstRayInSweep) {
    // reset the running average?
    last_good_v0 = missingValue;
  }
  // if bad data value is not set, i.e. still the default value
  // then set the bad data value to the missing value from the data
  if (bad_data_value == FLT_MIN) {
    bad_data_value = missingValue;
  }
 
  LOG(DEBUG) << "args: ";
  LOG(DEBUG) << "nyquist_velocity=" << nyquist_velocity;
  LOG(DEBUG) << "dds_radd_eff_unamb_vel=" << dds_radd_eff_unamb_vel;
  LOG(DEBUG) << "ngates_averaged= " << ngates_averaged;
  LOG(DEBUG) << "max_pos_folds=" << max_pos_folds;
  LOG(DEBUG) << "max_neg_folds=" << max_neg_folds;
  LOG(DEBUG) << "clip_gate=" << clip_gate;
  LOG(DEBUG) << "bad_data_value=" << bad_data_value;
  LOG(DEBUG) << "missingValue=" << missingValue;
  LOG(DEBUG) << "last_good_v0=" << last_good_v0;

  SoloFunctionsApi soloFunctionsApi;

  soloFunctionsApi.BBUnfoldFirstGoodGate(data, newData, nGates,
						nyquist_velocity, dds_radd_eff_unamb_vel,
						max_pos_folds, max_neg_folds,
						ngates_averaged,
						&last_good_v0_passable,
						bad_data_value, clip_gate, _boundaryMask);

  LOG(DEBUG) << "returning ... last_good_v0=" << last_good_v0;

  // insert new field into RadxVol                                                                             
  LOG(DEBUG) << "result = ";
  for (int i=0; i<10; i++)
    LOG(DEBUG) << newData[i] << ", ";

  //Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = true;
  string field_units = field->getUnits();
  //RadxField *newField = new RadxField(newFieldName, "m/s");
  //newField->copyMetaData(*field);
  //newField->addDataFl32(nGates, newData);
  RadxField *field1 = ray->addField(newFieldName, field_units, nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  LOG(DEBUG) << "exit ";

  return tempFieldName;

}

string SoloFunctionsModel::BBUnfoldAircraftWind(string fieldName, //RadxVol *vol,
            size_t rayIdx, //int sweepIdx,
            float nyquist_velocity,
            int max_pos_folds,
            int max_neg_folds,
            size_t ngates_averaged,
            size_t clip_gate,
            float bad_data_value, // TODO: pull this from data file?
            string newFieldName) { 

  // find the ray and data

  // get the missing data value

  // What is being returned? the name of the new field in the model that
  // contains the results.

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;

  //vol->loadRaysFromFields(); // loadFieldsFromRays();

  const RadxField *field;

  RadxRay *ray = _scriptsDataController->getRay(rayIdx);

  /*
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = ScriptsDataModel->getRays();
  if (rays.size() > 1) {
    LOG(DEBUG) <<  "WARNING - more than one ray; expected only one";
  }
  RadxRay *ray = rays.at(rayIdx);
  */
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  float dds_radd_eff_unamb_vel = ray->getNyquistMps(); // doradeData.eff_unamb_vel;
  int seds_nyquist_velocity = nyquist_velocity; //  what is this value? It is the nyquist velocity set by "one time only" commands

  float elevation_angle_degrees = ray->getElevationDeg();
  float azimuth_angle_degrees = ray->getAzimuthDeg();

  const RadxGeoref *georef = _scriptsDataController->getGeoreference(rayIdx);
  if (georef == NULL) {
    throw "BBUnfoldAircraftWind: Georef is null. Cannot find ew_wind, ns_wind, vert_wind";
  }
  /* ---
  // get the winds from the aircraft platform
  const RadxGeoref *georef = ray->getGeoreference();
  move this to ScriptsDataModel?
  if (georef == NULL) {
    LOG(DEBUG) << "ERROR - georef is NULL";
    LOG(DEBUG) << "      trying to recover ...";
    ScriptsDataModel->setLocationFromStartRay();
    georef = ray->getGeoreference();
    if (georef == NULL) {
      throw "BBUnfoldAircraftWind: Georef is null. Cannot find ew_wind, ns_wind, vert_wind";
    }
  } 
  */
 
  float vert_wind = georef->getVertWind();  // fl32
  float ew_wind = georef->getEwWind(); // fl32
  float ns_wind = georef->getNsWind(); // fl32;
  //-- 

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  float *newData = new float[nGates];

  // data, _boundaryMask, and newData should have all the same dimensions = nGates
  if (_boundaryMaskSet) { //  && _boundaryMaskLength >= 3) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();

  float missingValue = field->getMissingFl32();
  if (bad_data_value == FLT_MIN) {
    bad_data_value = missingValue;
  }
 
  LOG(DEBUG) << "args: ";
  LOG(DEBUG) << "nyquist_velocity=" << nyquist_velocity;
  LOG(DEBUG) << "dds_radd_eff_unamb_vel=" << dds_radd_eff_unamb_vel;
  LOG(DEBUG) << "ew_wind=" << ew_wind;
  LOG(DEBUG) << "nw_wind=" << ns_wind;
  LOG(DEBUG) << "vert_wind=" << vert_wind;
  LOG(DEBUG) << "azimuth_angle_degrees=" << azimuth_angle_degrees;
  LOG(DEBUG) << "elevation_angle_degrees=" << elevation_angle_degrees;
  LOG(DEBUG) << "ngates_averaged= " << ngates_averaged;
  LOG(DEBUG) << "max_pos_folds=" << max_pos_folds;
  LOG(DEBUG) << "max_neg_folds=" << max_neg_folds;
  LOG(DEBUG) << "clip_gate=" << clip_gate;
  LOG(DEBUG) << "bad_data_value=" << bad_data_value;
  LOG(DEBUG) << "missingValue=" << missingValue;

  SoloFunctionsApi soloFunctionsApi;

  soloFunctionsApi.BBUnfoldAircraftWind(data, newData, nGates,
                       nyquist_velocity, dds_radd_eff_unamb_vel,
                       azimuth_angle_degrees, elevation_angle_degrees,
                       ew_wind, ns_wind, vert_wind,
                       max_pos_folds, max_neg_folds,
                       ngates_averaged,
                       bad_data_value, clip_gate, _boundaryMask);

  // insert new field into RadxVol                                                                             
  LOG(DEBUG) << "result = ";
  for (int i=0; i<10; i++)
    LOG(DEBUG) << newData[i] << ", ";

  //Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = true;
  string field_units = field->getUnits();
  //RadxField *newField = new RadxField(newFieldName, "m/s");
  //newField->copyMetaData(*field);
  //newField->addDataFl32(nGates, newData);
  RadxField *field1 = ray->addField(newFieldName, field_units, nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  LOG(DEBUG) << "exit ";

  return tempFieldName;

}

string SoloFunctionsModel::BBUnfoldLocalWind(string fieldName, // RadxVol *vol,
            size_t rayIdx, //int sweepIdx,
            float nyquist_velocity,
            int max_pos_folds,
            int max_neg_folds,
            size_t ngates_averaged,
            float ew_wind, float ns_wind,
            size_t clip_gate,
            float bad_data_value, // TODO: pull this from data file?
            string newFieldName) { 

  // find the ray and data

  // get the missing data value

  // What is being returned? the name of the new field in the model that
  // contains the results.

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;

  const RadxField *field;
  //  field = vol->getFieldFromRay(fieldName);
  //  if (field == NULL) {
  //    LOG(DEBUG) << "no RadxField found in volume";
  //    throw "No data field with name " + fieldName;;
  //  }
  
  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "WARNING - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx); 
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  float dds_radd_eff_unamb_vel = ray->getNyquistMps(); // doradeData.eff_unamb_vel;
  int seds_nyquist_velocity = nyquist_velocity; //  what is this value? It is the nyquist velocity set by "one time only" commands

  float elevation_angle_degrees = ray->getElevationDeg();
  float azimuth_angle_degrees = ray->getAzimuthDeg();

  /* get the winds from the aircraft platform
  const RadxGeoref *georef = ray->getGeoreference();
  if (georef == NULL) {
    LOG(DEBUG) << "ERROR - georef is NULL";
    LOG(DEBUG) << "      trying to recover ...";
    vol->setLocationFromStartRay();
    georef = ray->getGeoreference();
    if (georef == NULL) {
      throw "BBUnfoldLocalWind: Georef is null. Cannot find ud_wind";
    }
  } 
 
  float ud_wind = georef->getVertWind();  // fl32
  */
  float ud_wind = 0.0; // TODO: this is not used; remove it from args.

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  float *newData = new float[nGates];

  // data, _boundaryMask, and newData should have all the same dimensions = nGates
  if (_boundaryMaskSet) { //  && _boundaryMaskLength >= 3) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  const float *data = field->getDataFl32();

  // manage the last good v0, initialized from first good gate in a sweep;
  // and perpetuated for each ray in the sweep
  static float last_good_v0;
  float missingValue = field->getMissingFl32();
  if (bad_data_value == FLT_MIN) {
    bad_data_value = missingValue;
  } 

  LOG(DEBUG) << "args: ";
  LOG(DEBUG) << "nyquist_velocity=" << nyquist_velocity;
  LOG(DEBUG) << "dds_radd_eff_unamb_vel=" << dds_radd_eff_unamb_vel;
  LOG(DEBUG) << "ew_wind=" << ew_wind;
  LOG(DEBUG) << "nw_wind=" << ns_wind;
  //LOG(DEBUG) << "ud_wind=" << ud_wind;
  LOG(DEBUG) << "azimuth_angle_degrees=" << azimuth_angle_degrees;
  LOG(DEBUG) << "elevation_angle_degrees=" << elevation_angle_degrees;
  LOG(DEBUG) << "ngates_averaged= " << ngates_averaged;
  LOG(DEBUG) << "max_pos_folds=" << max_pos_folds;
  LOG(DEBUG) << "max_neg_folds=" << max_neg_folds;  
  LOG(DEBUG) << "clip_gate=" << clip_gate;
  LOG(DEBUG) << "bad_data_value=" << bad_data_value;
  LOG(DEBUG) << "missingValue=" << missingValue;

  SoloFunctionsApi soloFunctionsApi;

  soloFunctionsApi.BBUnfoldLocalWind(data, newData, nGates,
                       nyquist_velocity, dds_radd_eff_unamb_vel,
                       azimuth_angle_degrees, elevation_angle_degrees,
                       ew_wind, ns_wind, ud_wind,
                       max_pos_folds, max_neg_folds,
                       ngates_averaged,
                       bad_data_value, clip_gate, _boundaryMask);

  // insert new field into RadxVol                                                                             
  LOG(DEBUG) << "result = ";
  for (int i=0; i<10; i++)
    LOG(DEBUG) << newData[i] << ", ";

  //Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = true;
  string field_units = field->getUnits();
  //RadxField *newField = new RadxField(newFieldName, "m/s");
  //newField->copyMetaData(*field);
  //newField->addDataFl32(nGates, newData);
  RadxField *field1 = ray->addField(newFieldName, field_units, nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  LOG(DEBUG) << "exit ";

  return tempFieldName;

}


  // TODO: where are we getting the bad_flag_mask?
  // TODO: is bad_flag_mask a variable/vector from the environment? or 
  // is it held internally, like the boundary mask?
  // I guess it depends on how we use use?  If we never need to return the mask plus 
  // something else, then the mask can be a variable just like any other data vector?

// return the temporary name for the new field in the volume
string SoloFunctionsModel::_flaggedAddMultiply(string fieldName,  // RadxVol *vol,
				     size_t rayIdx, //int sweepIdx,
				      bool multiply,
				     float constant,
				     size_t clip_gate,
				     float bad_data_value,
				     string flagFieldName) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
//	     << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx); 
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  float *newData = new float[nGates];

  // data, _boundaryMask, and newData should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) { //  && _boundaryMaskLength >= 3) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";

  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();

    RadxField *flagField = fetchDataField(ray, flagFieldName);
  const bool *bad_flag_mask = (const bool *) flagField->getDataSi08(); 
  
  // perform the function ...
  //bool multiply = false;
  soloFunctionsApi.FlaggedAdd(constant, multiply, data,  newData, nGates, 
			      bad_data_value, clip_gate, _boundaryMask,
			      bad_flag_mask);  

  // insert new field into RadxVol        
  string field_units = field->getUnits();
  Radx::fl32 missingValue = field->getMissingFl32();
  bool isLocal = true;

  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::FlaggedAdd(string fieldName,  // RadxVol *vol,
				      size_t rayIdx, //int sweepIdx,
				      float constant,
				      size_t clip_gate,
				      float bad_data_value,
				      string flagFieldName) {
  bool multiply = false;
  return _flaggedAddMultiply(fieldName, rayIdx, multiply, constant,
			    clip_gate, bad_data_value, flagFieldName);
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::FlaggedMultiply(string fieldName,  // RadxVol *vol,
				      size_t rayIdx, //int sweepIdx,
				      float constant,
				      size_t clip_gate,
				      float bad_data_value,
				      string flagFieldName) {
  bool multiply = true;
  return _flaggedAddMultiply(fieldName, rayIdx, multiply, constant,
			    clip_gate, bad_data_value, flagFieldName);
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::SetBadFlagsAbove(string fieldName,  // RadxVol *vol,
					    size_t rayIdx, //int sweepIdx,
					    float lower_threshold, 
					    size_t clip_gate,
					    float bad_data_value,
					    string badFlagMaskFieldName) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
//	     << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create bad_flag_mask for return 
  bool *bad_flag_mask = new bool[nGates];

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.SetBadFlagsAbove(lower_threshold,  
			       data, nGates, 
			       bad_data_value, clip_gate,
			       _boundaryMask, bad_flag_mask);
  // Q: where are we getting the bad_flag_mask? create it here.
  // Q: is bad_flag_mask a variable/vector from the environment? or 
  // is it held internally, like the boundary mask?
  // I guess it depends on how we use use?  If we never need to return the mask plus 
  // something else, then the mask can be a variable just like any other data vector?
  // Yes, the bad_flag_mask is a boolean variable like any other field vector.
  // insert new field into RadxVol                                                                             
  //cerr << "result = ";
  //for (int i=0; i<50; i++)
  //  cerr << bad_flag_mask[i] << ", ";
  //cerr << endl;

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = true;

  //badFlagMaskFieldName.append("_BAD");
  // I suppose the boolean mask should probably be kept as a Si08
  RadxField *field1 = ray->addField(badFlagMaskFieldName, "units", nGates, missingValue,
				    (Radx::si08 *) bad_flag_mask, 
				    1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::SetBadFlagsBelow(string fieldName,  //RadxVol *vol,
					    size_t rayIdx, //int sweepIdx,
					    float lower_threshold, 
					    size_t clip_gate,
					    float bad_data_value,
					    string badFlagMaskFieldName) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
//	     << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create bad_flag_mask for return 
  bool *bad_flag_mask = new bool[nGates];

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.SetBadFlagsBelow(lower_threshold,  
			       data, nGates, 
			       bad_data_value, clip_gate,
			       _boundaryMask, bad_flag_mask);
  // Q: where are we getting the bad_flag_mask? create it here.
  // Q: is bad_flag_mask a variable/vector from the environment? or 
  // is it held internally, like the boundary mask?
  // I guess it depends on how we use use?  If we never need to return the mask plus 
  // something else, then the mask can be a variable just like any other data vector?
  // Yes, the bad_flag_mask is a boolean variable like any other field vector.
  // insert new field into RadxVol                                                                             
  //cerr << "result = ";
  //for (int i=0; i<50; i++)
  //  cerr << bad_flag_mask[i] << ", ";
  //cerr << endl;

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = true;

  // badFlagMaskFieldName.append("_BAD");
  // I suppose the boolean mask should probably be kept as a Si08
  RadxField *field1 = ray->addField(badFlagMaskFieldName, "units", nGates, missingValue,
				    (Radx::si08 *) bad_flag_mask, 
				    1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::SetBadFlagsBetween(string fieldName,  // RadxVol *vol,
					      size_t rayIdx, //int sweepIdx,
					      float lower_threshold,
					      float upper_threshold,
					      size_t clip_gate,
					      float bad_data_value,
					      string badFlagMaskFieldName) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
//	     << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create bad_flag_mask for return 
  bool *bad_flag_mask = new bool[nGates];

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.SetBadFlagsBetween(lower_threshold, upper_threshold,  
				      data, nGates,
				      bad_data_value, clip_gate,
				      _boundaryMask, bad_flag_mask);
  // Q: where are we getting the bad_flag_mask? create it here.
  // Q: is bad_flag_mask a variable/vector from the environment? or 
  // is it held internally, like the boundary mask?
  // I guess it depends on how we use use?  If we never need to return the mask plus 
  // something else, then the mask can be a variable just like any other data vector?
  // Yes, the bad_flag_mask is a boolean variable like any other field vector.
  // insert new field into RadxVol                                                                             
  //cerr << "result = ";
  //for (int i=0; i<50; i++)
  //  cerr << bad_flag_mask[i] << ", ";
  //cerr << endl;

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = true;

  //badFlagMaskFieldName.append("_BAD");
  // I suppose the boolean mask should probably be kept as a Si08
  RadxField *field1 = ray->addField(badFlagMaskFieldName, "units", nGates, missingValue,
				    (Radx::si08 *) bad_flag_mask, 
				    1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

// ---- REMOVE RING ------
// return the temporary name for the new field in the volume
string SoloFunctionsModel::RemoveRing(string fieldName,  // RadxVol *vol,
                size_t rayIdx, //int sweepIdx,
                float lower_threshold, // in km
                float upper_threshold, // in km
                size_t clip_gate,
                float bad_data_value,
                string newFieldName) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
//       << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  //---- begin insert ...
  float *newData = new float[nGates];

  if (_boundaryMaskSet) { //  && _boundaryMaskLength >= 3) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // -- 
  // translate upper and lower threshold from km to a gate index
  double startRange;
  double gateSpace;
  _scriptsDataController->getPredomRayGeom(&startRange, &gateSpace);
  size_t from_gate = 0;
  if (lower_threshold > startRange) {
    from_gate = ceil((lower_threshold - startRange) / gateSpace);
    if (from_gate > nGates) {
      LOG(DEBUG) << "RemoveRing: lower_threshold exceeds number of gates; setting to max number of gates";
      from_gate = nGates;
      //throw std::invalid_argument(msg);
    }
  }
  size_t to_gate = 0;
  if (upper_threshold > startRange) {
    to_gate = ceil((upper_threshold - startRange) / gateSpace);
    if (to_gate > nGates) {
      LOG(DEBUG) << "RemoveRing: upper_threshold exceeds number of gates; setting to max number of gates";
      to_gate = nGates;
      //throw std::invalid_argument(msg);
    }
  }
  //----

  // // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  Radx::fl32 missingValue = field->getMissingFl32();

  // TODO: data, _boundaryMask, and newData should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;
  
  //---- end insert ...

  // perform the function ...
  soloFunctionsApi.RemoveRing(from_gate, to_gate,  
              data, newData, nGates,
              missingValue, clip_gate,
              _boundaryMask);

  //Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = true;

  //RadxField *newField = new RadxField(newFieldName, "m/s");
  //newField->copyMetaData(*field);
  //newField->addDataFl32(nGates, newData);
  RadxField *field1 = ray->addField(newFieldName, "m/s", nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  LOG(DEBUG) << "exit ";

  return tempFieldName;
}

// ---- ASSERT CLEAR COMPLEMENT ----

// return the temporary name for the new field in the volume
string SoloFunctionsModel::AssertBadFlags(string fieldName,  // RadxVol *vol,
					      size_t rayIdx, //int sweepIdx,
					      size_t clip_gate,
					      float bad_data_value,
					      string badFlagMaskFieldName) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
//	     << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)
  //fixUpFieldName(fieldName);  
  //field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create newData for return 
  float *newData = new float[nGates];

  // get the bad flag mask data
  RadxField *badDataField = fetchDataField(ray, badFlagMaskFieldName);

  //RadxField *badDataField = ray->getField(badFlagMaskFieldName);
  size_t nGatesMask = ray->getNGates(); 
  if (nGatesMask != nGates)
      throw "Error: bad flag mask and field gate dimension are not equal (SoloFunctionsModel)";
  const bool *bad_flag_mask = (bool *) badDataField->getDataSi08(); 

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.AssertBadFlags(data, newData, nGates,
				  bad_data_value, clip_gate,
				  _boundaryMask, bad_flag_mask);

  // NOTE: we are adding a new data field! NOT a new mask!!! 

  string field_units = field->getUnits();
  Radx::fl32 missingValue = field->getMissingFl32();
  bool isLocal = true;

  RadxField *field1 = ray->addField(fieldName, field_units, nGates, 
				    missingValue, newData, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  LOG(DEBUG) << "exit ";

  return tempFieldName;
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::ClearBadFlags(string badFlagMaskFieldName,  // RadxVol *vol,
					 size_t rayIdx) { // , int sweepIdx) {

  LOG(DEBUG) << "entry with fieldName ... " << badFlagMaskFieldName << " radIdx=" << rayIdx;
//	     << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  // field = ray->getField(badFlagMaskFieldName);
  field = fetchDataField(ray, badFlagMaskFieldName);
  size_t nGates = ray->getNGates(); 

  // create bad_flag_mask for return 
  bool *bad_flag_mask = new bool[nGates];

  // data, _boundaryMask, and bad flag mask should all have the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  // if (_boundaryMaskSet) {
  //  // verify dimensions on data in/out and boundary mask
  //  if (nGates > _boundaryMaskLength)
  //    throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  //}

  // cerr << "there arenGates " << nGates;
  //  const bool *bad_flag_mask = (bool *) field->getDataSi08();
  
  // TODO: is this function really useful? aren't we just creating a new field with
  // all false values?  Not sure how to deal with this function.
  // perform the function ...
  soloFunctionsApi.ClearBadFlags(bad_flag_mask, nGates);

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = true;

  //badFlagMaskFieldName.append("_BAD");
  // I suppose the boolean mask should probably be kept as a Si08
  RadxField *field1 = ray->addField(badFlagMaskFieldName, "units", nGates, missingValue,
				    (Radx::si08 *) bad_flag_mask, 
				    1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::ComplementBadFlags(string fieldName,  // RadxVol *vol,
					      size_t rayIdx){ // , int sweepIdx) {
			
  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
//	     << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  //string tempFieldName =
  field = fetchDataField(ray, fieldName);
  //field = ray->getField(fieldName);
  size_t nGates = ray->getNGates(); 

  // create bad_flag_mask for return 
  bool *complement_mask = new bool[nGates];

  // get the bad flag mask data
  //RadxField *badDataField = ray->getField(badFlagMaskFieldName);
  //size_t nGatesMask = ray->getNGates(); 
  //if (nGatesMask != nGates)
  //   throw "Error: bad flag mask and field gate dimension are not equal (SoloFunctionsModel)";
  const bool *bad_flag_mask = (bool *) field->getDataSi08();


  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  //if (_boundaryMaskSet) {
  //  // verify dimensions on data in/out and boundary mask
  //  if (nGates > _boundaryMaskLength)
  //    throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  //}

  // cerr << "there arenGates " << nGates;
  //const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.ComplementBadFlags(bad_flag_mask, complement_mask, nGates);

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = true;

  // I suppose the boolean mask should probably be kept as a Si08
  RadxField *field1 = ray->addField(fieldName, "units", nGates, missingValue,
				    (Radx::si08 *) complement_mask, 
				    1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

// ----

// return the temporary name for the new field in the volume
string SoloFunctionsModel::SetBadFlags(string fieldName,  // RadxVol *vol,
				       size_t rayIdx, //int sweepIdx,
				       string where,
				       float lower_threshold, float upper_threshold,
				       size_t clip_gate,
				       float bad_data_value,
				       string badFlagMaskFieldName) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
//	     << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  //field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create bad_flag_mask for return 
  bool *bad_flag_mask = new bool[nGates];

  // data, _boundaryMask, and newData should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  /*
  // perform the function ...
  soloFunctionsApi.SetBadFlags(where.c_str(), lower_threshold, upper_threshold, 
			       data, nGates, 
			       bad_data_value, clip_gate,
			       _boundaryMask, bad_flag_mask);
  // Q: where are we getting the bad_flag_mask? create it here.
  // Q: is bad_flag_mask a variable/vector from the environment? or 
  // is it held internally, like the boundary mask?
  // I guess it depends on how we use use?  If we never need to return the mask plus 
  // something else, then the mask can be a variable just like any other data vector?
  // Yes, the bad_flag_mask is a boolean variable like any other field vector.
  // insert new field into RadxVol                                                                      */
  //cerr << "result = ";
  //for (int i=0; i<50; i++)
  //  cerr << bad_flag_mask[i] << ", ";
  //cerr << endl;

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = true;

  // badFlagMaskFieldName.append("_BAD");
  //RadxField *newField = new RadxField(newFieldName, "m/s");
  //newField->copyMetaData(*field);
  //newField->addDataFl32(nGates, newData);
  RadxField *field1 = ray->addField(badFlagMaskFieldName, "units", nGates, missingValue,
				    (Radx::si08 *) bad_flag_mask, 1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

// ----
  string SoloFunctionsModel::AndBadFlagsAbove(string fieldName,  //RadxVol *vol, 
                         size_t rayIdx, //int sweepIdx,
                         float constant, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {
    SoloFunctionsApi api;
    return _generalLogicalFx(fieldName, rayIdx, //sweepIdx,
			     constant,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::AndBadFlagsAbove, api);

 }
  string SoloFunctionsModel::AndBadFlagsBelow(string fieldName,  //RadxVol *vol, 
                         size_t rayIdx, //int sweepIdx,
                         float constant, size_t clip_gate, float bad_data_value,
                         string maskFieldName) { 
    SoloFunctionsApi api;
    return _generalLogicalFx(fieldName, rayIdx, //sweepIdx,
			     constant,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::AndBadFlagsBelow, api);

  }
  string SoloFunctionsModel::AndBadFlagsBetween(string fieldName,  size_t rayIdx, //int sweepIdx,
                         float constantLower, float constantUpper, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {
    SoloFunctionsApi api;
    return _generalLogicalFx2(fieldName, rayIdx, //sweepIdx,
			     constantLower, constantUpper,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::AndBadFlagsBetween, api);

  }

  string SoloFunctionsModel::OrBadFlagsAbove(string fieldName,  size_t rayIdx, //int sweepIdx,
                         float constant, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {
    SoloFunctionsApi api;
    return _generalLogicalFx(fieldName, rayIdx, //sweepIdx,
			     constant,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::OrBadFlagsAbove, api);

  }
  string SoloFunctionsModel::OrBadFlagsBelow(string fieldName, size_t rayIdx, //int sweepIdx,
                         float constant, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {
    SoloFunctionsApi api;
    return _generalLogicalFx(fieldName, rayIdx, //sweepIdx,
			     constant,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::OrBadFlagsBelow, api);

  }
  string SoloFunctionsModel::OrBadFlagsBetween(string fieldName,  size_t rayIdx, //int sweepIdx,
                         float constantLower, float constantUpper, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {
    SoloFunctionsApi api;
    return _generalLogicalFx2(fieldName, rayIdx, //sweepIdx,
			      constantLower, constantUpper,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::OrBadFlagsBetween, api);

  }

  string SoloFunctionsModel::XorBadFlagsAbove(string fieldName,  size_t rayIdx, //int sweepIdx,
                         float constant, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {

    SoloFunctionsApi api;
    return _generalLogicalFx(fieldName, rayIdx, //sweepIdx,
			     constant,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::XorBadFlagsAbove, api);
  }
  string SoloFunctionsModel::XorBadFlagsBelow(string fieldName,   size_t rayIdx, //int sweepIdx,
                         float constant, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {

    SoloFunctionsApi api;
    return _generalLogicalFx(fieldName, rayIdx, //sweepIdx,
			     constant,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::XorBadFlagsBelow, api);
 }

 string SoloFunctionsModel::XorBadFlagsBetween(string fieldName,  size_t rayIdx, //int sweepIdx,
					      float constantLower, float constantUpper, 
					      size_t clip_gate, float bad_data_value,
					      string maskFieldName) {
    SoloFunctionsApi api;
    return _generalLogicalFx2(fieldName, rayIdx, //sweepIdx,
			      constantLower, constantUpper,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::XorBadFlagsBetween, api);
  }


/* 
void CopyBadFlags(const float *data, size_t nGates,
		  float bad, size_t dgi_clip_gate,
		  bool *boundary_mask, bool *bad_flag_mask);
*/
 string SoloFunctionsModel::CopyBadFlags(string fieldName,  size_t rayIdx, //int sweepIdx,
					 size_t clip_gate, float bad_data_value) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
//	     << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create bad_flag_mask for return 
  bool *new_mask = new bool[nGates];

  // get the bad flag mask
  //RadxField *maskField = fetchDataField(ray, maskFieldName);
  // TODO: fix up this check ...
  // size_t nGatesMask = ray->getNGates(); 
  //if (nGatesMask != nGates)
  //   throw "Error: bad flag mask and field gate dimension are not equal (SoloFunctionsModel)";
  // const bool *bad_flag_mask = (bool *) maskField->getDataSi08();


  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.CopyBadFlags(data, nGates, bad_data_value, clip_gate,
				_boundaryMask, new_mask);

  /* --
  CopyBadFlags(const float *data, size_t nGates,                                                              
	       float bad, size_t dgi_clip_gate,                                                               
	       bool *boundary_mask, bool *bad_flag_mask);  
  // --
  */

// TODO: should all of this work with the Radx stuff be via the _scriptsDataController??
  // AND ISLOCAL NEEDS TO BE TRUE
  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = true;

  // I suppose the boolean mask should probably be kept as a Si08
  RadxField *field1 = ray->addField(fieldName, "units", nGates, missingValue,
				    (Radx::si08 *) new_mask, 
				    1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

string SoloFunctionsModel::FlaggedAssign(string fieldName,  size_t rayIdx, //int sweepIdx,
					 float constant,
					 size_t clip_gate, string maskFieldName) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
	 //    << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create new data for return 
  float *newData = new float[nGates];

  // get the bad flag mask
  RadxField *maskField = fetchDataField(ray, maskFieldName);
  // TODO: fix up this check ...
  // size_t nGatesMask = ray->getNGates(); 
  //if (nGatesMask != nGates)
  //   throw "Error: bad flag mask and field gate dimension are not equal (SoloFunctionsModel)";
  const bool *bad_flag_mask = (bool *) maskField->getDataSi08();

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.FlaggedAssign(constant, data, newData, nGates, clip_gate,
				_boundaryMask, bad_flag_mask);

  Radx::fl32 missingValue = field->getMissingFl32(); 
  bool isLocal = true;
  string field_units = field->getUnits();
  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;

}

string SoloFunctionsModel::FlaggedCopy(string fieldName,  size_t rayIdx, //int sweepIdx,
		   size_t clip_gate, string maskFieldName) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
//	     << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create new data for return 
  float *newData = new float[nGates];
  // get the bad flag mask

  RadxField *maskField = fetchDataField(ray, maskFieldName);
  // TODO: fix up this check ...
  // size_t nGatesMask = ray->getNGates(); 
  //if (nGatesMask != nGates)
  //   throw "Error: bad flag mask and field gate dimension are not equal (SoloFunctionsModel)";
  const bool *bad_flag_mask = (bool *) maskField->getDataSi08();

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.FlaggedCopy(data, newData, nGates, clip_gate,
				_boundaryMask, bad_flag_mask);

  Radx::fl32 missingValue = field->getMissingFl32(); 
  bool isLocal = true;
  string field_units = field->getUnits();
  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;

}

string SoloFunctionsModel::FlagFreckles(string fieldName,   size_t rayIdx, //int sweepIdx,
		    float freckle_threshold, size_t freckle_avg_count,
		    size_t clip_gate, float bad_data_value) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
//	     << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx); 
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create bad_flag_mask for return 
  bool *new_mask = new bool[nGates];

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.FlagFreckles(freckle_threshold, freckle_avg_count,
				data, nGates, bad_data_value, clip_gate,
				_boundaryMask, new_mask);

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = true;

  // I suppose the boolean mask should probably be kept as a Si08
  RadxField *field1 = ray->addField(fieldName, "units", nGates, missingValue,
				    (Radx::si08 *) new_mask, 
				    1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;

}


string SoloFunctionsModel::FlagGlitches(string fieldName,   size_t rayIdx, //int sweepIdx,
		    float deglitch_threshold, int deglitch_radius,
		    int deglitch_min_gates,
					size_t clip_gate, float bad_data_value) {
  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
//	     << " sweepIdx=" << sweepIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  //const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  //if (rays.size() > 1) {
  //  LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  //}
  //RadxRay *ray = rays.at(rayIdx);
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create bad_flag_mask for return 
  bool *new_mask = new bool[nGates];

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.FlagGlitches(deglitch_threshold, deglitch_radius,
				deglitch_min_gates,
				data, nGates, bad_data_value, clip_gate,
				_boundaryMask, new_mask);

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = true;

  // I suppose the boolean mask should probably be kept as a Si08
  RadxField *field1 = ray->addField(fieldName, "units", nGates, missingValue,
				    (Radx::si08 *) new_mask, 
				    1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

// TODO: remove bad_flag_mask_field_name because it is not used 
string SoloFunctionsModel::ThresholdFieldAbove(string fieldName,  size_t rayIdx, //int sweepIdx,
					       float scaled_thr,
					       int first_good_gate, string threshold_field,
					       float threshold_bad_data_value,
					       size_t clip_gate, float bad_data_value) {
					       // string bad_flag_mask_field_name) {
  SoloFunctionsApi api;
  string bad_flag_mask_field_name = "";
  return _generalThresholdFx(fieldName, rayIdx, //sweepIdx,
			      scaled_thr,
			      first_good_gate, threshold_field,
			      threshold_bad_data_value,
			      clip_gate, bad_data_value,
			      bad_flag_mask_field_name,
			      &SoloFunctionsApi::ThresholdFieldAbove, api);
}

string SoloFunctionsModel::ThresholdFieldBelow(string fieldName,  size_t rayIdx, //int sweepIdx,
					       float scaled_thr,
					       int first_good_gate, string threshold_field,
					       float threshold_bad_data_value,
					       size_t clip_gate, float bad_data_value) {
  SoloFunctionsApi api;
  string bad_flag_mask_field_name = "";
  return _generalThresholdFx(fieldName, rayIdx, //sweepIdx,
			      scaled_thr,
			      first_good_gate, threshold_field,
			      threshold_bad_data_value,
			      clip_gate, bad_data_value,
			      bad_flag_mask_field_name,
			      &SoloFunctionsApi::ThresholdFieldBelow, api);
}

string SoloFunctionsModel::ThresholdFieldBetween(string fieldName,  size_t rayIdx, //int sweepIdx,
						 float lower_threshold, float upper_threshold,
						 int first_good_gate, string threshold_field,
						 float threshold_bad_data_value,
						 size_t clip_gate, float bad_data_value,
						 string maskFieldName) {

   SoloFunctionsApi api;

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create new data field for return 
  float *newData = new float[nGates];

  // get the bad flag mask
  RadxField *maskField = fetchDataField(ray, maskFieldName);
  // TODO: fix up this check ...
  // size_t nGatesMask = ray->getNGates(); 
  //if (nGatesMask != nGates)
  //   throw "Error: bad flag mask and field gate dimension are not equal (SoloFunctionsModel)";
  const bool *bad_flag_mask = (bool *) maskField->getDataSi08();

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();

  const RadxField *field_thr;
  field_thr = fetchDataField(ray, threshold_field);
  const float *threshold_data = field_thr->getDataFl32();
  float thr_bad_data_value = field_thr->getMissingFl32();

  // perform the function ...
  soloFunctionsApi.ThresholdFieldBetween(lower_threshold, upper_threshold, 
					 first_good_gate, data, threshold_data, nGates,
					 newData, bad_data_value, thr_bad_data_value,
					 clip_gate, _boundaryMask, bad_flag_mask);

  bool isLocal = true;
  string field_units = field->getUnits();
  Radx::fl32 missingValue = field->getMissingFl32();
  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;

}



string SoloFunctionsModel::ForceUnfolding(string fieldName,   size_t rayIdx, //int sweepIdx,
		      float nyquist_velocity,
		      float center,
		      float bad_data_value, size_t dgi_clip_gate) {

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;

  const RadxField *field;
  
  //  get the ray for this field 
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  float dds_radd_eff_unamb_vel = ray->getNyquistMps(); // doradeData.eff_unamb_vel;
  int seds_nyquist_velocity = nyquist_velocity; //  what is this value? It is the nyquist velocity set by "one time only" commands

  // get the data (in) and create space for new data (out)  
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  float *newData = new float[nGates];

  // data, _boundaryMask, and newData should have all the same dimensions = nGates
  if (_boundaryMaskSet) { //  && _boundaryMaskLength >= 3) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
 
  LOG(DEBUG) << "args: ";
  LOG(DEBUG) << "nyquist_velocity=" << nyquist_velocity;
  LOG(DEBUG) << "dds_radd_eff_unamb_vel=" << dds_radd_eff_unamb_vel;
  LOG(DEBUG) << "clip_gate=" << dgi_clip_gate;
  LOG(DEBUG) << "bad_data_value=" << bad_data_value;

  SoloFunctionsApi soloFunctionsApi;

  soloFunctionsApi.ForceUnfolding(data, newData, nGates,
				  nyquist_velocity, dds_radd_eff_unamb_vel,
				  center,
				  bad_data_value, dgi_clip_gate, _boundaryMask);

  // insert new field into RadxVol                                                                             
  LOG(DEBUG) << "result = ";
  for (int i=0; i<10; i++)
    LOG(DEBUG) << newData[i] << ", ";

  bool isLocal = true;
  string field_units = field->getUnits();
  Radx::fl32 missingValue = field->getMissingFl32();
  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  LOG(DEBUG) << "exit ";

  return tempFieldName;

}

string SoloFunctionsModel::UnconditionalDelete(string fieldName,  size_t rayIdx, //int sweepIdx,
             size_t clip_gate, float bad_data_value) {

   SoloFunctionsApi api;

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  RadxRay *ray = _scriptsDataController->getRay(rayIdx); 
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create new data field for return 
  float *newData = new float[nGates];

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();

  Radx::fl32 missingValue = field->getMissingFl32();

  // perform the function ...
  soloFunctionsApi.UnconditionalDelete(data, newData, nGates,
          (float) missingValue, 
          clip_gate, _boundaryMask);

  bool isLocal = true;
  string field_units = field->getUnits();

  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;

}

string SoloFunctionsModel::AssignValue(string fieldName,  size_t rayIdx,
             float data_value, size_t clip_gate) {

   SoloFunctionsApi api;

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  RadxRay *ray = _scriptsDataController->getRay(rayIdx); 
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create new data field for return 
  float *newData = new float[nGates];

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  const float *data = field->getDataFl32();

  Radx::fl32 missingValue = field->getMissingFl32();

  // perform the function ...
  soloFunctionsApi.AssignValue(data, newData, nGates,
          data_value, 
          clip_gate, _boundaryMask);

  bool isLocal = true;
  string field_units = field->getUnits();

  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;

}


// Private methods 

string SoloFunctionsModel::_generalLogicalFx(string fieldName,   size_t rayIdx, //int sweepIdx,
					     float constant, 
					      size_t clip_gate, float bad_data_value,
					     string maskFieldName,
					     void (SoloFunctionsApi::*function) (float, const float *, size_t, float, size_t, bool *, const bool *, bool *), SoloFunctionsApi& api) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  RadxRay *ray = _scriptsDataController->getRay(rayIdx);   
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create bad_flag_mask for return 
  bool *new_mask = new bool[nGates];

  // get the bad flag mask
  RadxField *maskField = fetchDataField(ray, maskFieldName);
  // TODO: fix up this check ...
  // size_t nGatesMask = ray->getNGates(); 
  //if (nGatesMask != nGates)
  //   throw "Error: bad flag mask and field gate dimension are not equal (SoloFunctionsModel)";
  const bool *bad_flag_mask = (bool *) maskField->getDataSi08();


  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  //SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  //soloFunctionsApi.XorBadFlagsBetween(constantLower, constantUpper,
  //				      data, nGates, bad_data_value, clip_gate,
  //				      _boundaryMask, bad_flag_mask, new_mask);
  (api.*function)(constant, data, nGates, bad_data_value, clip_gate, _boundaryMask, bad_flag_mask, new_mask);

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = true;

  // I suppose the boolean mask should probably be kept as a Si08
  RadxField *field1 = ray->addField(maskFieldName, "units", nGates, missingValue,
				    (Radx::si08 *) new_mask, 
				    1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

// for upper and lower thresholds
string SoloFunctionsModel::_generalLogicalFx2(string fieldName,   size_t rayIdx, //int sweepIdx,
					      float constantLower, float constantUpper, 
					      size_t clip_gate, float bad_data_value,
					      string maskFieldName,
					      void (SoloFunctionsApi::*function) (float, float, const float *, size_t, float, size_t, bool *, const bool *, bool *), SoloFunctionsApi& api) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
  
  const RadxField *field;

  //  get the ray for this field 
  RadxRay *ray = _scriptsDataController->getRay(rayIdx); 
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  // get the data (in) and create space for new data (out)  
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create bad_flag_mask for return 
  bool *new_mask = new bool[nGates];

  // get the bad flag mask
  RadxField *maskField = fetchDataField(ray, maskFieldName);
  // TODO: fix up this check ...
  // size_t nGatesMask = ray->getNGates(); 
  //if (nGatesMask != nGates)
  //   throw "Error: bad flag mask and field gate dimension are not equal (SoloFunctionsModel)";
  const bool *bad_flag_mask = (bool *) maskField->getDataSi08();


  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  //SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  // cerr << "there arenGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  //soloFunctionsApi.XorBadFlagsBetween(constantLower, constantUpper,
  //				      data, nGates, bad_data_value, clip_gate,
  //				      _boundaryMask, bad_flag_mask, new_mask);
  (api.*function)(constantLower, constantUpper, data, nGates, bad_data_value, clip_gate, _boundaryMask, bad_flag_mask, new_mask);

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = true;

  // I suppose the boolean mask should probably be kept as a Si08
  RadxField *field1 = ray->addField(maskFieldName, "units", nGates, missingValue,
				    (Radx::si08 *) new_mask, 
				    1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

// for upper and lower thresholds
string SoloFunctionsModel::_generalThresholdFx(string fieldName,  size_t rayIdx, //int sweepIdx,
						float threshold, 
						int first_good_gate, string threshold_field,
						float threshold_bad_data_value,
						size_t clip_gate, float bad_data_value,
						string maskFieldName,
					       void (SoloFunctionsApi::*function) (float, int,  const float *, const float *, size_t, float *, float, float, size_t, bool *, const bool *), SoloFunctionsApi& api) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx;
  
  //  get the ray for this field 
  RadxRay *ray = _scriptsDataController->getRay(rayIdx); 
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  const RadxField *field = NULL;
  field = fetchDataField(ray, fieldName);

  // get the data (in) and create space for new data (out)  
  const float *fieldData = fetchData(ray, fieldName);
  const float *thrdata = fetchData(ray, threshold_field);

  //field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 

  // create new data field for return 
  float *newData = new float[nGates];

  // get the bad flag mask
  //RadxField *maskField = fetchDataField(ray, maskFieldName);
  // TODO: fix up this check ...
  // size_t nGatesMask = ray->getNGates(); 
  //if (nGatesMask != nGates)
  //   throw "Error: bad flag mask and field gate dimension are not equal (SoloFunctionsModel)";
  const bool *bad_flag_mask = NULL; // (bool *) maskField->getDataSi08();


  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  //SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  const float *threshold_data = thrdata; // field_thr->getDataFl32();

  const RadxField *field_thr;
  field_thr = fetchDataField(ray, threshold_field);
  float thr_bad_data_value = Radx::missingFl32;
  if (field_thr != NULL) {
    thr_bad_data_value = field_thr->getMissingFl32();
  }

  // cerr << "there arenGates " << nGates;
  const float *data = fieldData; // field->getDataFl32();

  Radx::fl32 missingValue = field->getMissingFl32();
  if (bad_data_value == FLT_MIN) {
    bad_data_value = missingValue;
  }

  // perform the function ...
  //soloFunctionsApi.XorBadFlagsBetween(constantLower, constantUpper,
  //				      data, nGates, bad_data_value, clip_gate,
  //				      _boundaryMask, bad_flag_mask, new_mask);
  (api.*function)(threshold, first_good_gate, data, threshold_data, nGates, 
		  newData, bad_data_value, thr_bad_data_value, 
		  clip_gate, _boundaryMask, bad_flag_mask);

  bool isLocal = true;
  string field_units = "";
  //Radx::fl32 missingValue = Radx::missingFl32;
  if (field != NULL) {
    field_units = field->getUnits();
    missingValue = field->getMissingFl32();
  }
  if (field == NULL) {
    fieldName = "NULL";
  }
  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}



// ---


// ----



// These are not used.  The code is saved as a way to return a vector of data

vector<double> SoloFunctionsModel::RemoveAircraftMotion(vector<double> data) { // SpreadSheetModel *context) {

  // TODO: what is being returned? the name of the new field in the model that
  // contains the results.
  // since the std::vector<double> data has to be copied to QVector anyway, 
  // go ahead and format it as a string?
  // maybe return a pointer to std::vector<double> ?? then when presenting the data, we can convert it to string,
  // but maintain the precision in the model (RadxVol)??

  cerr << "RemoveAircraftMotion ... "  << endl;

  // gather data from context -- most of the data are in a DoradeRadxFile object

  // TODO: convert the context RadxVol to DoradeRadxFile and DoradeData format;
  //RadxVol vol = context->_vol;
  // make sure the radar angles have been calculated.

  /*
  const RadxField *field;
  field = vol.getFieldFromRay(fieldName);
  if (field == NULL) {
    cerr << "no RadxField found " <<  endl;
    throw "No data field with name " + fieldName;;
  }
  */

  // TODO: get the ray for this field 
  const vector<RadxRay *>  &rays = _scriptsDataController->getRays();
  if (rays.size() > 1) {
    cerr << "ERROR - more than one ray; expected only one\n";
  }

  RadxRay *ray = rays.at(0);
  if (ray == NULL) {
    cerr << "ERROR - first ray is NULL" << endl;
  } 
  //const RadxGeoref *georef = ray->getGeoreference();

  // float vert_velocity = georef->getVertVelocity();  // fl32
  // float ew_velocity = georef->getEwVelocity(); // fl32
  // float ns_velocity = georef->getNsVelocity(); // fl32;

  float ew_gndspd_corr = 0.0; 
  const RadxCfactors *cfactors = ray->getCfactors();
  if (cfactors != NULL) {
    ew_gndspd_corr = cfactors->getEwVelCorr(); // ?? _gndspd_corr; // fl32;
  }
  LOG(DEBUG) << "ew_gndspd_corr: " << ew_gndspd_corr;
 
  // float tilt = georef->getTilt(); // fl32; 
  // TODO: elevation changes with different rays/fields how to get the current one???
  // float elevation = ray->getElevationDeg(); // doradeData.elevation; // fl32;

  // TODO:  look up the dataField and get the associated values
  // look through DoradeRadxFile::_ddParms for a parameter_t type that has parameter_name that matches
  // the dataField.
  //short *data; // data is in and out parameter
  //data = 
  //  double scale = 1.0 / parm.parameter_scale;
  //  double bias = (-1.0 * parm.parameter_bias) / parm.parameter_scale;

  // related to field->setTypeSi32(parm.bad_data, scale, bias)
  // RadxField::_scale;  RadxField::_offset = bias; RadxField::_missingSi32 = bad_data
  // 
  /* TODO: need to find the field. How to do this????
  short bad = field->getMissingSi16(); // doradeData.bad_data;
  float parameter_scale = 1.0 / field->getScale(); // doradeData.parameter_scale; 
  float parameter_bias = -1.0 * field->getOffset() * field->getScale(); // doradeData.parameter_bias; 
  int dgi_clip_gate = 7; // field->num_samples; // or number_cells
  short dds_radd_eff_unamb_vel = ray->getNyquistMps(); // doradeData.eff_unamb_vel;
  int seds_nyquist_velocity; // TODO: what is this value?

  cerr << "args: " << endl <<
    "vert_velocity " << vert_velocity << endl <<
    "ew_velocity " << ew_velocity << endl <<
    "ns_velocity " << ns_velocity << endl <<
    "ew_gndspd_corr " << ew_gndspd_corr << endl <<
    "tilt " << tilt << endl <<
    "elevation " << elevation <<
    endl;
  */
  //SoloFunctionsApi soloFunctionsApi;
  /*  int result = soloFunctionsApi.se_remove_ac_motion(vert_velocity, ew_velocity, ns_velocity,
     ew_gndspd_corr, tilt, elevation,
     data, bad, parameter_scale, parameter_bias, dgi_clip_gate,
     dds_radd_eff_unamb_vel, seds_nyquist_velocity);
  */

  // TODO: We are converting from short to double!!!  <=====
  vector<double> newData; // (data, dgi_clip_gate+1);
  for (vector<double>::iterator it = data.begin(); it != data.end(); ++it)
    newData.push_back(*it * 2.0);
  return newData;
}



void SoloFunctionsModel::printBoundaryMask() {
  LOG(DEBUG) << "Boundary Mask ... Length = " << _boundaryMaskLength;
  for (int i=0; i<_boundaryMaskLength; i++)
    LOG(DEBUG) << _boundaryMask[i] << ", ";
}

RadxField *SoloFunctionsModel::fetchDataField(RadxRay *ray, string &fieldName) {

  // the new field names are not yet available,
  // so use the original name, until they are available
  size_t endpt = fieldName.size() - 1;
  if (fieldName[endpt] == '#') { // retrieve the field name without the special symbol #
    fieldName.erase(endpt, 1);
  }
  RadxField *dataField = ray->getField(fieldName);
  /*
  if (dataField == NULL) {
    char msg[180];
    sprintf(msg, "ERROR - ray field not found %s\n", fieldName.c_str());
    string msgs;
    msgs.append("no field ");
    msgs.append(fieldName);
    throw std::invalid_argument(msgs);
  }
  */
  return dataField; 
}

float *SoloFunctionsModel::convertValueStringToFloatPtr(string &listOfValues) {
  vector<float> *values = new vector<float>;

  size_t nextPos = 0;
  size_t idx = 0;
  const char *s = listOfValues.c_str();
  size_t length = strlen(s);
  bool done = false;
  while ((nextPos < length) && !done) {
    float value;
    const char *substring = &s[nextPos];
    if (sscanf(substring, "%g", &value) <= 0) { done = true; }
    else {
      //printf("value[%lu] = \t%g\n", idx, value);
      values->push_back(value);
      const char *ptr = strchr(substring, ',');
      nextPos = (ptr + 1) - s;
      idx += 1;
    }
  }

  return &(*values)[0];

}

const float *SoloFunctionsModel::fetchData(RadxRay *ray, string &fieldName) {

  /*
  std::regex values_regex("[-,\\s\\.[:digit:]]+",
            std::regex_constants::ECMAScript);
  std::regex variable_regex("[_|\\s|[:alpha:]]+",
            std::regex_constants::ECMAScript);
            */
  if (strchr(fieldName.c_str(),',') != NULL) {
    cout << "We have a pass by value argument" << endl;
    return convertValueStringToFloatPtr(fieldName);
  } else {
    RadxField *dataField = fetchDataField(ray, fieldName);
    if (dataField != NULL)
      return dataField->getDataFl32();
    else
      return NULL;
  }
}
