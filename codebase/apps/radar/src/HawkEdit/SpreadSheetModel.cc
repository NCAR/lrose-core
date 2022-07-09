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

SpreadSheetModel::SpreadSheetModel(RadxRay *closestRay)
{
  _closestRay = closestRay;
  if (_closestRay == NULL) 
    cout << "in SpreadSheetModel, closestRay is NULL" << endl;
  else
   cout << "in SpreadSheetModel, closestRay is NOT  NULL" << endl;

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

/*
// return the azimuth of the ray offset from the closest ray
float SpreadSheetModel::getAzimuthForRay(int offsetFromClosest)
{
  float azimuth = 0.0;
  if (offsetFromClosest == 0) {
    azimuth = _closestRay->getAzimuthDeg();
  } else {

    DataModel *dataModel = DataModel::Instance();

    size_t rayIdx = _getRayIdx(offsetFromClosest);      
    azimuth = dataModel->getRayAzimuthDeg(rayIdx);  

  }
  return azimuth;
}
*/
/*
float SpreadSheetModel::getNyquistVelocityForRay(RayLocationController *rayLocationController,
  int offsetFromClosest) {
  float nyquistVelocity = -1.0;  // negative value indicates no value found
  if (offsetFromClosest == 0) {
    nyquistVelocity = _closestRay->getNyquistMps();
  } else {
    
    DataModel *dataModel = DataModel::Instance();

    size_t rayIdx = _getRayIdx(offsetFromClosest);
    nyquistVelocity = dataModel->getRayNyquistVelocityMps(rayIdx);  
  }
  return nyquistVelocity;
}
*/

/*
size_t SpreadSheetModel::_getRayIdx(int offsetFromClosest) {
    DataModel *dataModel = DataModel::Instance();
    size_t nRays = dataModel->getNRays();
    //vector<RadxRay *> rays = const_cast <vector<RadxRay *>> (dataModel->getRays());
    // ( size_t = (size_t + int) % size_t)  ==> TROUBLE!!!
    //int closestRay = (int) _closestRayIdx;

    // Use RayLoc!

    size_t rayIdx = dataModel->getRayIndex(_closestRayIdx,
      offsetFromClosest, _currentSweepNumber); 

    //if (idx < 0) throw std::invalid_argument("requested ray index < 0"); 
    LOG(DEBUG) << "closestRayIdx=" << _closestRayIdx << " offsetFromClosest=" << offsetFromClosest 
      << " rayIdx=" << rayIdx;
    return rayIdx;
}
*/
void SpreadSheetModel::_setSweepNumber(int sweepNumber) {
  _currentSweepNumber = sweepNumber;
}

/*
// idx is zero based
void SpreadSheetModel::setRay(int idx, RadxRay *ray) { 
  if (idx < 0) {}
  if (idx >= _raysToDisplay.size()) {
    _raysToDisplay.resize(idx+1);
  }
  _raysToDisplay.at(idx) = ray;
}
*/

// Remember, rayLocation has pointers to the rays in the volume held in the DataModel. 
/*
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

  LOG(DEBUG) << "exit";
  return dataVector;

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

  LOG(DEBUG) << "exit";

}


// set data values for the field in the Volume (for all rays? for all sweeps?)
void SpreadSheetModel::setDataMissing(string fieldName, float missingDataValue)
{
  LOG(DEBUG) << "fieldName=" << fieldName << " setting to missing value " << missingDataValue;


  DataModel *dataModel = DataModel::Instance();
  float dummy = 0.0;
  dataModel->SetData(fieldName, dummy);

}


/*

// find the closest ray in the volume and set the internal variable _closestRay
// and the internal variable _closestRayIdx
//
void SpreadSheetModel::setClosestRay(size_t rayIdx, int sweepNumber) {
  LOG(DEBUG) << "enter azimuth = " << azimuth;


// Why not use rayLocation?  It is already sorted by azimuth for the current sweep???

  DataModel *dataModel = DataModel::Instance();

  _setSweepNumber(sweepNumber);
  size_t closestRayIdx = dataModel->findClosestRay(azimuth, _currentSweepNumber);

  _closestRay = dataModel->getRay(closestRayIdx);
  _closestRayIdx = closestRayIdx;
  LOG(DEBUG) << "_closestRayIdx = " << _closestRayIdx 
    << " azimuth = " << _closestRay->getAzimuthDeg() << " vs. requested az " << azimuth;
  LOG(DEBUG) << "exit";
}
*/
/*
void SpreadSheetModel::setClosestRay(float azimuth, int sweepNumber) {
  LOG(DEBUG) << "enter azimuth = " << azimuth;

  DataModel *dataModel = DataModel::Instance();

  _setSweepNumber(sweepNumber);
  size_t closestRayIdx = dataModel->findClosestRay(azimuth, _currentSweepNumber);

  _closestRay = dataModel->getRay(closestRayIdx);
  _closestRayIdx = closestRayIdx;
  LOG(DEBUG) << "_closestRayIdx = " << _closestRayIdx 
    << " azimuth = " << _closestRay->getAzimuthDeg() << " vs. requested az " << azimuth;
  LOG(DEBUG) << "exit";
}
*/