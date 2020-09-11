
#include <vector>
#include <iostream>

#include "SoloFunctionsModel.hh"
//#include "RemoveAcMotion.cc" // This comes from an external library
#include "BoundaryPointEditor.hh"


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

// call this for each new ray, since the azimuth changes each time the ray changes
void SoloFunctionsModel::SetBoundaryMask(RadxVol *vol,
					 int rayIdx, int sweepIdx, bool useBoundaryMask) {

  _boundaryMaskSet = true;

  bool determineMask = false;
  if (useBoundaryMask)
    determineMask = true;

  CheckForDefaultMask(vol, rayIdx, sweepIdx, determineMask);
  //  SetBoundaryMaskOriginal(vol, rayIdx, sweepIdx);
}

const vector<bool> *SoloFunctionsModel::GetBoundaryMask() {
  vector<bool> *dataVector = new vector<bool>(_boundaryMaskLength);
  dataVector->assign(_boundaryMask, _boundaryMask+_boundaryMaskLength);
  return dataVector;
}


void SoloFunctionsModel::CheckForDefaultMask(RadxVol *vol, int rayIdx, int sweepIdx, bool determineMask) {

  
  LOG(DEBUG) << "enter";
  LOG(DEBUG) << " radIdx=" << rayIdx
	     << " sweepIdx=" << sweepIdx;
 
  short *boundary;

  // TODO: make this a call to BoundaryPointModel?
  BoundaryPointEditor *bpe = BoundaryPointEditor::Instance();
  vector<Point> boundaryPoints = bpe->getWorldPoints();
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
    SetDefaultMask(vol, rayIdx, sweepIdx);
  } else {
    DetermineBoundaryMask(vol, rayIdx, sweepIdx);
  } 

}

void SoloFunctionsModel::SetDefaultMask(RadxVol *vol, int rayIdx, int sweepIdx) {
  vol->loadRaysFromFields();

  const RadxField *field;

  //  get the ray for this field 
  // for the boundary mask, all we care about is the geometry of the ray,
  // NOT the data values for each field.
  const vector<RadxRay *>  &rays = vol->getRays();
  if (rays.size() > 1) {
    LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  }
  RadxRay *ray = rays.at(rayIdx);
  if (ray == NULL) {
    LOG(DEBUG) << "ERROR - ray is NULL";
    throw "Ray is null";
  } 

  //field = ray->getField(fieldName);
  size_t nGates = ray->getNGates(); 
  LOG(DEBUG) << "there are nGates " << nGates;

  if (nGates != _boundaryMaskLength) {
    // clear old mask
    if (_boundaryMask != NULL) {
      delete[] _boundaryMask;
    }

    // allocate new mask
    _boundaryMaskLength = nGates;
    _boundaryMask = new bool[_boundaryMaskLength];
  }

  for (size_t i=0; i<nGates; i++) {
    _boundaryMask[i] = true;
  } 
}


void SoloFunctionsModel::DetermineBoundaryMask(RadxVol *vol, int rayIdx, int sweepIdx) {
  SetBoundaryMaskOriginal(vol, rayIdx, sweepIdx);
}


// call this for each new ray, since the azimuth changes each time the ray changes
void SoloFunctionsModel::SetBoundaryMaskOriginal(RadxVol *vol,
					 int rayIdx, int sweepIdx) {
  
  LOG(DEBUG) << "enter";
  LOG(DEBUG) << " radIdx=" << rayIdx
	     << " sweepIdx=" << sweepIdx;
 
  short *boundary;

  // TODO: make this a call to BoundaryPointModel?
  BoundaryPointEditor *bpe = BoundaryPointEditor::Instance();
  vector<Point> boundaryPoints = bpe->getWorldPoints();
  // vector<Point> myPoints = BoundaryPointEditor::Instance()->getBoundaryPoints("/media/sf_lrose/ncswp_SPOL_RHI_.nc", 0, 4, "Boundary1");  TODO
  
  //map boundaryPoints to a list of short/boolean the same size as the ray->datafield->ngates
  int nBoundaryPoints = boundaryPoints.size();
  LOG(DEBUG) << "nBoundaryPoints = " << nBoundaryPoints;

  //---------  HERE -------
  // need nGates & do NOT delete unless the number of gates changes

  // can we reuse the boundary mask?  

  if (_boundaryMask != NULL) {
    delete[] _boundaryMask;
  }
 
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

  // wrestle this information out of the ray and radar volume ...
  float radar_origin_latitude = vol->getLatitudeDeg();
  float radar_origin_longitude = vol->getLongitudeDeg();
  float radar_origin_altitude = vol->getAltitudeKm() * 1000.0;
  float boundary_origin_tilt = 0.0;
  float boundary_origin_latitude = radar_origin_latitude; // 0.0;
  float boundary_origin_longitude = radar_origin_longitude; // 0.0;
  float boundary_origin_altitude = radar_origin_altitude; // 0.0;
  
  // =======

  vol->loadRaysFromFields();

  const RadxField *field;
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
  const vector<RadxRay *>  &rays = vol->getRays();
  if (rays.size() > 1) {
    LOG(DEBUG) <<  "ERROR - more than one ray; expected only one";
  }
  RadxRay *ray = rays.at(rayIdx);
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
  float azimuth = ray->getAzimuthDeg();
  // need to do some conversions here ...
  // TODO: get these from SoloLibrary::dd_math.h
  int radar_scan_mode = 1; // PPI;
  int radar_type = 0; // GROUND; 
 
  float tilt_angle = 0.0; // TODO: It should be this ... ray->getElevationDeg();
  float rotation_angle = 0.0; 


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
				   rotation_angle,
				   _boundaryMask);
 
  printBoundaryMask();

  delete[] xpoints;
  delete[] ypoints;

  LOG(DEBUG) << "exit"; 

}


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

  cerr << "there are nGates " << nGates;
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
  
  // get the data (in) and create space for new data (out)  
  //  field = ray->getField(fieldName);
  //field = fetchDataField(ray, fieldName);
  //size_t nGates = ray->getNGates(); 
  
  //float *newData = new float[nGates];

  int nGates = fieldData->size();
  cerr << "there are nGates " << nGates;

  Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = false;
  const float *flatData = fieldData->data();

  //RadxField *newField = new RadxField(newFieldName, "m/s");
  //newField->copyMetaData(*field);
  //newField->addDataFl32(nGates, newData);
  RadxField *field1 = ray->addField(fieldName, "m/s", nGates, missingValue, flatData, isLocal);

  //string tempFieldName = field1->getName();
  //tempFieldName.append("#");

  LOG(DEBUG) << "exit ";

}


// return the temporary name for the new field in the volume
string SoloFunctionsModel::ZeroMiddleThird(string fieldName,  RadxVol *vol,
					   int rayIdx, int sweepIdx,
					   string newFieldName) {
  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx
	     << " sweepIdx=" << sweepIdx;

  // gather data from context -- most of the data are in a DoradeRadxFile object

  // TODO: convert the context RadxVol to DoradeRadxFile and DoradeData format;
  //RadxVol vol = context->_vol;
  // make sure the radar angles have been calculated.

  //  vol->loadFieldsFromRays();
  vol->loadRaysFromFields();

  
  const RadxField *field;
  /*
  field = vol->getFieldFromRay(fieldName);
  if (field == NULL) {
    LOG(DEBUG) << "no RadxField found in volume";
    throw "No data field with name " + fieldName;;
  }
  */

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
  
  // field = ray->getField(fieldName);
  field = fetchDataField(ray, fieldName);
  size_t nGates = ray->getNGates(); 
  cerr << "there are nGates " << nGates;
  const float *data = field->getDataFl32();
  float *newData = new float[nGates];
  for (int i=0; i<10; i++)
    newData[i] = data[i];   
  for (int i=10; i<30; i++)
    newData[i] = 0;
  for (int i=30; i<nGates; i++)
    newData[i] = data[i];   

  // insert new field into RadxVol                                                                             
  cerr << "result = ";
  for (int i=0; i<50; i++)
    cerr << newData[i] << ", ";
  cerr << endl;

  // I have the ray, can't I just add a field to it?

  // ========
  /*
  RadxRay *newRay = new RadxRay();

  newRay->setVolumeNumber(1);
  newRay->setSweepNumber(sweepIdx);
  newRay->setRayNumber(rayIdx);
  */
  //      const string name = "VEL";                                                                  
  //const string units = "m/s";                                                                       
  Radx::fl32 missingValue = Radx::missingFl32; // -999;
  //      const Radx::si16 *data = &rawData[0];                                                       
  //  double scale = 1.0;
  //double offset = 0.0;
  bool isLocal = false;

  //RadxField *newField = new RadxField(newFieldName, "m/s");
  //newField->copyMetaData(*field);
  //newField->addDataFl32(nGates, newData);
  RadxField *field1 = ray->addField(newFieldName, "m/s", nGates, missingValue, newData, isLocal);
  //  RadxField *field1 = newRay->addField(newFieldName, "m/s", nGates, missingValue, newData, scale, offset, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  /*
  // to avoid this warning ...                                                                        
  // WARNING - Range geom has not been set on ray                                                     
  double startRangeKm = 3.0;
  double gateSpacingKm = 5.0;
  newRay->setRangeGeom(startRangeKm, gateSpacingKm);
  */

  //vol->addRay?Field?();  // apparently this is not needed!

  // STOPPED HERE ...  Kinda close ... adds fields VEL_xyz, VEL_xyz2, ... VEL_xyz4 one for each sweep, 
  // and the missing value is wonky 

  // ============

  //  return newData;

  return tempFieldName;
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::ZeroInsideBoundary(string fieldName,  RadxVol *vol,
					   int rayIdx, int sweepIdx,
					   string newFieldName) {
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

  float *newData = new float[nGates];

  // =======
  // data, _boundaryMask, and newData should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  //  if (!_boundaryMaskSet) 
  //  _boundaryMask = NULL;

  // =======

  if (_boundaryMaskSet) {

    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";

    cerr << "there are nGates " << nGates;
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
    cerr << "there are nGates " << nGates;
    const float *data = field->getDataFl32();

  
    /*  This is for a fixed boundary; and it works fine!
    short *fixedBnd = new short[nGates];
    for (size_t i = 0; i<nGates; i++)
      fixedBnd[i] = 0;
    for (size_t i = 500; i<nGates; i++)
      fixedBnd[i] = 1;
    soloFunctionsApi.ZeroInsideBoundary(data, fixedBnd, newData, nGates);
    delete[] fixedBnd;
    */

    soloFunctionsApi.ZeroInsideBoundary(data, NULL, newData, nGates);


    /*
    for (int i=0; i<nGates; i++) {
	newData[i] = 0.0;
    }
    */
  }
  /*
  for (int i=0; i<10; i++)
    newData[i] = data[i];   
  for (int i=10; i<30; i++)
    newData[i] = 0;
  for (int i=30; i<nGates; i++)
    newData[i] = data[i];   
  */



  // insert new field into RadxVol                                                                             
  cerr << "result = ";
  for (int i=0; i<50; i++)
    cerr << newData[i] << ", ";
  cerr << endl;

  Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = false;

  //RadxField *newField = new RadxField(newFieldName, "m/s");
  //newField->copyMetaData(*field);
  //newField->addDataFl32(nGates, newData);
  RadxField *field1 = ray->addField(newFieldName, "m/s", nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  /*
  // to avoid this warning ...                                                                        
  // WARNING - Range geom has not been set on ray                                                     
  double startRangeKm = 3.0;
  double gateSpacingKm = 5.0;
  newRay->setRangeGeom(startRangeKm, gateSpacingKm);
  */

  return tempFieldName;
}


// return the temporary name for the new field in the volume
string SoloFunctionsModel::Despeckle(string fieldName,  RadxVol *vol,
				     int rayIdx, int sweepIdx,
				     size_t speckle_length,
				     size_t clip_gate,
				     float bad_data_value,
				     string newFieldName) {

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

  cerr << "there are nGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.Despeckle(data,  newData, nGates, bad_data_value, speckle_length,
			     clip_gate, _boundaryMask);

  // insert new field into RadxVol                                                                             
  cerr << "result = ";
  for (int i=0; i<50; i++)
    cerr << newData[i] << ", ";
  cerr << endl;

  Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = false;

  //RadxField *newField = new RadxField(newFieldName, "m/s");
  //newField->copyMetaData(*field);
  //newField->addDataFl32(nGates, newData);
  RadxField *field1 = ray->addField(newFieldName, "m/s", nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

string SoloFunctionsModel::RemoveAircraftMotion(string fieldName, RadxVol *vol,
						int rayIdx, int sweepIdx,
						float nyquist_velocity,
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

  vol->loadRaysFromFields(); // loadFieldsFromRays();

  const RadxField *field;
  //  field = vol->getFieldFromRay(fieldName);
  //  if (field == NULL) {
  //    LOG(DEBUG) << "no RadxField found in volume";
  //    throw "No data field with name " + fieldName;;
  //  }
  
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
 
  float vert_velocity = georef->getVertVelocity();  // fl32
  float ew_velocity = georef->getEwVelocity(); // fl32
  float ns_velocity = georef->getNsVelocity(); // fl32;

  float ew_gndspd_corr = 0.0; 
  const RadxCfactors *cfactors = ray->getCfactors();
  if (cfactors != NULL) {
    ew_gndspd_corr = cfactors->getEwVelCorr(); // ?? _gndspd_corr; // fl32;
  }
 
  float tilt = georef->getTilt(); // fl32; 
  // TODO: elevation changes with different rays/fields how to get the current one???
  float elevation = ray->getElevationDeg(); // doradeData.elevation; // fl32;

  /*
  // TODO:  look up the dataField and get the associated values
  // look through DoradeRadxFile::_ddParms for a parameter_t type that has parameter_name that matches
  // the dataField.
  // short *data; // data is in and out parameter
  if (field->getDataType() != Radx::SI16) {
    throw  "ERROR - data is not 16-bit signed integer as expected";
  } 

  //Radx::fl32 *rawData;
  //rawData = field->getDataFl32();
  //Radx::fl32 *ptr = rawData;
  cerr << " A few data values ";
  for (int i=0; i< 10; i++) {
    cerr << field->getDoubleValue(i) << ", ";
  }
  cerr << endl;
  //vector<double> data = field->getData  Radx::fl32 *getDataFl32();   
  //  double scale = 1.0 / parm.parameter_scale;
  //  double bias = (-1.0 * parm.parameter_bias) / parm.parameter_scale;

  // related to field->setTypeSi32(parm.bad_data, scale, bias)
  // RadxField::_scale;  RadxField::_offset = bias; RadxField::_missingSi32 = bad_data
  // 
  // TODO: need to find the field. How to do this????
  short bad = field->getMissingSi16(); // doradeData.bad_data;
  float parameter_scale = 1.0 / field->getScale(); // doradeData.parameter_scale; 
  float parameter_bias = -1.0 * field->getOffset() * field->getScale(); // doradeData.parameter_bias; 

  int dgi_clip_gate = field->getNPoints(); // field->num_samples; // or number_cells
*/

  short dds_radd_eff_unamb_vel = ray->getNyquistMps(); // doradeData.eff_unamb_vel;
  int seds_nyquist_velocity = 0; // TODO: what is this value?

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

  cerr << "there are nGates " << nGates;
  const float *data = field->getDataFl32();
  
  //==========

  // TODO: data, _boundaryMask, and newData should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

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

  soloFunctionsApi.RemoveAircraftMotion(vert_velocity, ew_velocity, ns_velocity,
					ew_gndspd_corr, tilt, elevation,
					data, newData, nGates,
					bad_data_value, clip_gate,
					dds_radd_eff_unamb_vel, seds_nyquist_velocity,
					_boundaryMask);
  


  // insert new field into RadxVol                                                                             
  cerr << "result = ";
  for (int i=0; i<50; i++)
    cerr << newData[i] << ", ";
  cerr << endl;

  Radx::fl32 missingValue = Radx::missingFl32; 
  bool isLocal = false;

  //RadxField *newField = new RadxField(newFieldName, "m/s");
  //newField->copyMetaData(*field);
  //newField->addDataFl32(nGates, newData);
  RadxField *field1 = ray->addField(newFieldName, "m/s", nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  LOG(DEBUG) << "exit ";

  return tempFieldName;
}


string SoloFunctionsModel::BBUnfoldFirstGoodGate(string fieldName, RadxVol *vol,
						int rayIdx, int sweepIdx,
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

  vol->loadRaysFromFields(); // loadFieldsFromRays();

  const RadxField *field;
  //  field = vol->getFieldFromRay(fieldName);
  //  if (field == NULL) {
  //    LOG(DEBUG) << "no RadxField found in volume";
  //    throw "No data field with name " + fieldName;;
  //  }
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = vol->getRays();
  if (rays.size() > 1) {
    LOG(DEBUG) <<  "WARNING - more than one ray; expected only one";
  }
  RadxRay *ray = rays.at(rayIdx);
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

  cerr << "there are nGates " << nGates;
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
  bool isLocal = false;
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
string SoloFunctionsModel::_flaggedAddMultiply(string fieldName,  RadxVol *vol,
				     int rayIdx, int sweepIdx,
				      bool multiply,
				     float constant,
				     size_t clip_gate,
				     float bad_data_value,
				     string flagFieldName) {

  LOG(DEBUG) << "entry with fieldName ... " << fieldName << " radIdx=" << rayIdx
	     << " sweepIdx=" << sweepIdx;

  vol->loadRaysFromFields();
  
  const RadxField *field;

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

  cerr << "there are nGates " << nGates;
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
  bool isLocal = false;

  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::FlaggedAdd(string fieldName,  RadxVol *vol,
				      int rayIdx, int sweepIdx,
				      float constant,
				      size_t clip_gate,
				      float bad_data_value,
				      string flagFieldName) {
  bool multiply = false;
  return _flaggedAddMultiply(fieldName, vol, rayIdx, sweepIdx, multiply, constant,
			    clip_gate, bad_data_value, flagFieldName);
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::FlaggedMultiply(string fieldName,  RadxVol *vol,
				      int rayIdx, int sweepIdx,
				      float constant,
				      size_t clip_gate,
				      float bad_data_value,
				      string flagFieldName) {
  bool multiply = true;
  return _flaggedAddMultiply(fieldName, vol, rayIdx, sweepIdx, multiply, constant,
			    clip_gate, bad_data_value, flagFieldName);
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::SetBadFlagsAbove(string fieldName,  RadxVol *vol,
					    int rayIdx, int sweepIdx,
					    float lower_threshold, 
					    size_t clip_gate,
					    float bad_data_value,
					    string badFlagMaskFieldName) {

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

  // create bad_flag_mask for return 
  bool *bad_flag_mask = new bool[nGates];

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  cerr << "there are nGates " << nGates;
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
  cerr << "result = ";
  for (int i=0; i<50; i++)
    cerr << bad_flag_mask[i] << ", ";
  cerr << endl;

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = false;

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
string SoloFunctionsModel::SetBadFlagsBelow(string fieldName,  RadxVol *vol,
					    int rayIdx, int sweepIdx,
					    float lower_threshold, 
					    size_t clip_gate,
					    float bad_data_value,
					    string badFlagMaskFieldName) {

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

  // create bad_flag_mask for return 
  bool *bad_flag_mask = new bool[nGates];

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  cerr << "there are nGates " << nGates;
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
  cerr << "result = ";
  for (int i=0; i<50; i++)
    cerr << bad_flag_mask[i] << ", ";
  cerr << endl;

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = false;

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
string SoloFunctionsModel::SetBadFlagsBetween(string fieldName,  RadxVol *vol,
					      int rayIdx, int sweepIdx,
					      float lower_threshold,
					      float upper_threshold,
					      size_t clip_gate,
					      float bad_data_value,
					      string badFlagMaskFieldName) {

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

  // create bad_flag_mask for return 
  bool *bad_flag_mask = new bool[nGates];

  // data, _boundaryMask, and bad flag mask should have all the same dimensions = nGates
  SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  cerr << "there are nGates " << nGates;
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
  cerr << "result = ";
  for (int i=0; i<50; i++)
    cerr << bad_flag_mask[i] << ", ";
  cerr << endl;

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = false;

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

// ---- ASSERT CLEAR COMPLEMENT ----

// return the temporary name for the new field in the volume
string SoloFunctionsModel::AssertBadFlags(string fieldName,  RadxVol *vol,
					      int rayIdx, int sweepIdx,
					      size_t clip_gate,
					      float bad_data_value,
					      string badFlagMaskFieldName) {

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

  cerr << "there are nGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.AssertBadFlags(data, newData, nGates,
				  bad_data_value, clip_gate,
				  _boundaryMask, bad_flag_mask);

  // NOTE: we are adding a new data field! NOT a new mask!!! 

  string field_units = field->getUnits();
  Radx::fl32 missingValue = field->getMissingFl32();
  bool isLocal = false;

  RadxField *field1 = ray->addField(fieldName, field_units, nGates, 
				    missingValue, newData, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  LOG(DEBUG) << "exit ";

  return tempFieldName;
}

// return the temporary name for the new field in the volume
string SoloFunctionsModel::ClearBadFlags(string badFlagMaskFieldName,  RadxVol *vol,
					 int rayIdx, int sweepIdx) {

  LOG(DEBUG) << "entry with fieldName ... " << badFlagMaskFieldName << " radIdx=" << rayIdx
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

  cerr << "there are nGates " << nGates;
  //  const bool *bad_flag_mask = (bool *) field->getDataSi08();
  
  // TODO: is this function really useful? aren't we just creating a new field with
  // all false values?  Not sure how to deal with this function.
  // perform the function ...
  soloFunctionsApi.ClearBadFlags(bad_flag_mask, nGates);

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = false;

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
string SoloFunctionsModel::ComplementBadFlags(string fieldName,  RadxVol *vol,
					      int rayIdx, int sweepIdx) {
			
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

  cerr << "there are nGates " << nGates;
  //const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.ComplementBadFlags(bad_flag_mask, complement_mask, nGates);

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = false;

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
string SoloFunctionsModel::SetBadFlags(string fieldName,  RadxVol *vol,
				       int rayIdx, int sweepIdx,
				       string where,
				       float lower_threshold, float upper_threshold,
				       size_t clip_gate,
				       float bad_data_value,
				       string badFlagMaskFieldName) {

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

  cerr << "there are nGates " << nGates;
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
  cerr << "result = ";
  for (int i=0; i<50; i++)
    cerr << bad_flag_mask[i] << ", ";
  cerr << endl;

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = false;

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
  string SoloFunctionsModel::AndBadFlagsAbove(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
                         float constant, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {
    SoloFunctionsApi api;
    return _generalLogicalFx(fieldName, vol, rayIdx, sweepIdx,
			     constant,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::AndBadFlagsAbove, api);

 }
  string SoloFunctionsModel::AndBadFlagsBelow(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
                         float constant, size_t clip_gate, float bad_data_value,
                         string maskFieldName) { 
    SoloFunctionsApi api;
    return _generalLogicalFx(fieldName, vol, rayIdx, sweepIdx,
			     constant,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::AndBadFlagsBelow, api);

  }
  string SoloFunctionsModel::AndBadFlagsBetween(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
                         float constantLower, float constantUpper, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {
    SoloFunctionsApi api;
    return _generalLogicalFx2(fieldName, vol, rayIdx, sweepIdx,
			     constantLower, constantUpper,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::AndBadFlagsBetween, api);

  }

  string SoloFunctionsModel::OrBadFlagsAbove(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
                         float constant, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {
    SoloFunctionsApi api;
    return _generalLogicalFx(fieldName, vol, rayIdx, sweepIdx,
			     constant,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::OrBadFlagsAbove, api);

  }
  string SoloFunctionsModel::OrBadFlagsBelow(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
                         float constant, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {
    SoloFunctionsApi api;
    return _generalLogicalFx(fieldName, vol, rayIdx, sweepIdx,
			     constant,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::OrBadFlagsBelow, api);

  }
  string SoloFunctionsModel::OrBadFlagsBetween(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
                         float constantLower, float constantUpper, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {
    SoloFunctionsApi api;
    return _generalLogicalFx2(fieldName, vol, rayIdx, sweepIdx,
			      constantLower, constantUpper,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::OrBadFlagsBetween, api);

  }

  string SoloFunctionsModel::XorBadFlagsAbove(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
                         float constant, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {

    SoloFunctionsApi api;
    return _generalLogicalFx(fieldName, vol, rayIdx, sweepIdx,
			     constant,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::XorBadFlagsAbove, api);
  }
  string SoloFunctionsModel::XorBadFlagsBelow(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
                         float constant, size_t clip_gate, float bad_data_value,
                         string maskFieldName) {

    SoloFunctionsApi api;
    return _generalLogicalFx(fieldName, vol, rayIdx, sweepIdx,
			     constant,
			     clip_gate, bad_data_value,
			     maskFieldName,
			     &SoloFunctionsApi::XorBadFlagsBelow, api);
 }

 string SoloFunctionsModel::XorBadFlagsBetween(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
					      float constantLower, float constantUpper, 
					      size_t clip_gate, float bad_data_value,
					      string maskFieldName) {
    SoloFunctionsApi api;
    return _generalLogicalFx2(fieldName, vol, rayIdx, sweepIdx,
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
 string SoloFunctionsModel::CopyBadFlags(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
					 size_t clip_gate, float bad_data_value) {

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

  cerr << "there are nGates " << nGates;
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

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = false;

  // I suppose the boolean mask should probably be kept as a Si08
  RadxField *field1 = ray->addField(fieldName, "units", nGates, missingValue,
				    (Radx::si08 *) new_mask, 
				    1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

string SoloFunctionsModel::FlaggedAssign(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
					 float constant,
					 size_t clip_gate, string maskFieldName) {

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

  cerr << "there are nGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.FlaggedAssign(constant, data, newData, nGates, clip_gate,
				_boundaryMask, bad_flag_mask);

  Radx::fl32 missingValue = field->getMissingFl32(); 
  bool isLocal = false;
  string field_units = field->getUnits();
  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;

}

string SoloFunctionsModel::FlaggedCopy(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
		   size_t clip_gate, string maskFieldName) {

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

  cerr << "there are nGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.FlaggedCopy(data, newData, nGates, clip_gate,
				_boundaryMask, bad_flag_mask);

  Radx::fl32 missingValue = field->getMissingFl32(); 
  bool isLocal = false;
  string field_units = field->getUnits();
  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;

}

string SoloFunctionsModel::FlagFreckles(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
		    float freckle_threshold, size_t freckle_avg_count,
		    size_t clip_gate, float bad_data_value) {

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

  cerr << "there are nGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.FlagFreckles(freckle_threshold, freckle_avg_count,
				data, nGates, bad_data_value, clip_gate,
				_boundaryMask, new_mask);

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = false;

  // I suppose the boolean mask should probably be kept as a Si08
  RadxField *field1 = ray->addField(fieldName, "units", nGates, missingValue,
				    (Radx::si08 *) new_mask, 
				    1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;

}


string SoloFunctionsModel::FlagGlitches(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
		    float deglitch_threshold, int deglitch_radius,
		    int deglitch_min_gates,
					size_t clip_gate, float bad_data_value) {
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

  cerr << "there are nGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  soloFunctionsApi.FlagGlitches(deglitch_threshold, deglitch_radius,
				deglitch_min_gates,
				data, nGates, bad_data_value, clip_gate,
				_boundaryMask, new_mask);

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = false;

  // I suppose the boolean mask should probably be kept as a Si08
  RadxField *field1 = ray->addField(fieldName, "units", nGates, missingValue,
				    (Radx::si08 *) new_mask, 
				    1.0, 0.0, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}

string SoloFunctionsModel::ThresholdFieldAbove(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
					       float scaled_thr,
					       int first_good_gate, string threshold_field,
					       float threshold_bad_data_value,
					       size_t clip_gate, float bad_data_value,
					       string bad_flag_mask_field_name) {
  SoloFunctionsApi api;
  return _generalThresholdFx(fieldName, vol, rayIdx, sweepIdx,
			      scaled_thr,
			      first_good_gate, threshold_field,
			      threshold_bad_data_value,
			      clip_gate, bad_data_value,
			      bad_flag_mask_field_name,
			      &SoloFunctionsApi::ThresholdFieldAbove, api);
}

string SoloFunctionsModel::ThresholdFieldBelow(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
					       float scaled_thr,
					       int first_good_gate, string threshold_field,
					       float threshold_bad_data_value,
					       size_t clip_gate, float bad_data_value,
					       string bad_flag_mask_field_name) {
  SoloFunctionsApi api;
  return _generalThresholdFx(fieldName, vol, rayIdx, sweepIdx,
			      scaled_thr,
			      first_good_gate, threshold_field,
			      threshold_bad_data_value,
			      clip_gate, bad_data_value,
			      bad_flag_mask_field_name,
			      &SoloFunctionsApi::ThresholdFieldBelow, api);
}

string SoloFunctionsModel::ThresholdFieldBetween(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
						 float lower_threshold, float upper_threshold,
						 int first_good_gate, string threshold_field,
						 float threshold_bad_data_value,
						 size_t clip_gate, float bad_data_value,
						 string maskFieldName) {

   SoloFunctionsApi api;

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

  cerr << "there are nGates " << nGates;
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

  bool isLocal = false;
  string field_units = field->getUnits();
  Radx::fl32 missingValue = field->getMissingFl32();
  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;

}



string SoloFunctionsModel::ForceUnfolding(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
		      float nyquist_velocity,
		      float center,
		      float bad_data_value, size_t dgi_clip_gate) {

  LOG(DEBUG) << "entry with fieldName ... ";
  LOG(DEBUG) << fieldName;

  vol->loadRaysFromFields();

  const RadxField *field;
  
  //  get the ray for this field 
  const vector<RadxRay *>  &rays = vol->getRays();

  RadxRay *ray = rays.at(rayIdx);
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

  cerr << "there are nGates " << nGates;
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

  bool isLocal = false;
  string field_units = field->getUnits();
  Radx::fl32 missingValue = field->getMissingFl32();
  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  LOG(DEBUG) << "exit ";

  return tempFieldName;

}


// Private methods 

string SoloFunctionsModel::_generalLogicalFx(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
					     float constant, 
					      size_t clip_gate, float bad_data_value,
					     string maskFieldName,
					     void (SoloFunctionsApi::*function) (float, const float *, size_t, float, size_t, bool *, const bool *, bool *), SoloFunctionsApi& api) {

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

  cerr << "there are nGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  //soloFunctionsApi.XorBadFlagsBetween(constantLower, constantUpper,
  //				      data, nGates, bad_data_value, clip_gate,
  //				      _boundaryMask, bad_flag_mask, new_mask);
  (api.*function)(constant, data, nGates, bad_data_value, clip_gate, _boundaryMask, bad_flag_mask, new_mask);

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = false;

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
string SoloFunctionsModel::_generalLogicalFx2(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
					      float constantLower, float constantUpper, 
					      size_t clip_gate, float bad_data_value,
					      string maskFieldName,
					      void (SoloFunctionsApi::*function) (float, float, const float *, size_t, float, size_t, bool *, const bool *, bool *), SoloFunctionsApi& api) {

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

  cerr << "there are nGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  //soloFunctionsApi.XorBadFlagsBetween(constantLower, constantUpper,
  //				      data, nGates, bad_data_value, clip_gate,
  //				      _boundaryMask, bad_flag_mask, new_mask);
  (api.*function)(constantLower, constantUpper, data, nGates, bad_data_value, clip_gate, _boundaryMask, bad_flag_mask, new_mask);

  Radx::fl32 missingValue = Radx::missingSi08; 
  bool isLocal = false;

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
string SoloFunctionsModel::_generalThresholdFx(string fieldName,  RadxVol *vol, int rayIdx, int sweepIdx,
						float threshold, 
						int first_good_gate, string threshold_field,
						float threshold_bad_data_value,
						size_t clip_gate, float bad_data_value,
						string maskFieldName,
					       void (SoloFunctionsApi::*function) (float, int,  const float *, const float *, size_t, float *, float, float, size_t, bool *, const bool *), SoloFunctionsApi& api) {

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
  //SoloFunctionsApi soloFunctionsApi;

  if (_boundaryMaskSet) {
    // verify dimensions on data in/out and boundary mask
    if (nGates > _boundaryMaskLength)
      throw "Error: boundary mask and field gate dimension are not equal (SoloFunctionsModel)";
  }

  const RadxField *field_thr;
  field_thr = fetchDataField(ray, threshold_field);
  const float *threshold_data = field_thr->getDataFl32();
  float thr_bad_data_value = field_thr->getMissingFl32();

  cerr << "there are nGates " << nGates;
  const float *data = field->getDataFl32();
  
  // perform the function ...
  //soloFunctionsApi.XorBadFlagsBetween(constantLower, constantUpper,
  //				      data, nGates, bad_data_value, clip_gate,
  //				      _boundaryMask, bad_flag_mask, new_mask);
  (api.*function)(threshold, first_good_gate, data, threshold_data, nGates, 
		  newData, bad_data_value, thr_bad_data_value, 
		  clip_gate, _boundaryMask, bad_flag_mask);

  bool isLocal = false;
  string field_units = field->getUnits();
  Radx::fl32 missingValue = field->getMissingFl32();
  RadxField *field1 = ray->addField(fieldName, field_units, nGates, missingValue, newData, isLocal);

  // get the name that was actually inserted ...
  string tempFieldName = field1->getName();
  tempFieldName.append("#");

  return tempFieldName;
}



// ---


// ----



// These are not used.  The code is saved as a way to return a vector of data

vector<double> SoloFunctionsModel::RemoveAircraftMotion(vector<double> data, RadxVol *vol) { // SpreadSheetModel *context) {

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
  const vector<RadxRay *>  &rays = vol->getRays();
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
  cout << "Boundary Mask ... Length = " << _boundaryMaskLength << endl;
  for (int i=0; i<_boundaryMaskLength; i++)
    cout << _boundaryMask[i] << ", ";
}

RadxField *SoloFunctionsModel::fetchDataField(RadxRay *ray, string &fieldName) {

  // the new field names are not yet available,
  // so use the original name, until they are available
  size_t endpt = fieldName.size() - 1;
  if (fieldName[endpt] == '#') { // retrieve the field name without the special symbol #
    fieldName.erase(endpt, 1);
  }
  RadxField *dataField = ray->getField(fieldName);
  if (dataField == NULL) {
    char msg[180];
    sprintf(msg, "ERROR - ray field not found %s\n", fieldName.c_str());
    throw msg;
  }
  return dataField; 
}
