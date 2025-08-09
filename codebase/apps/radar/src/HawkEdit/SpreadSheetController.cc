
//
// SpreadSheetController converts Model data to format for display in View
//  mostly from float, int to string based on format requested.
//

#include <stdio.h>
#include <QtWidgets>

#include "SpreadSheetController.hh"
#include "SpreadSheetModel.hh"
#include <toolsa/LogStream.hh>

SpreadSheetController::SpreadSheetController(SpreadSheetView *view)
{
  _currentView = view;
  _currentModel = new SpreadSheetModel();
}


SpreadSheetController::SpreadSheetController(SpreadSheetView *view, SpreadSheetModel *model,
  RayLocationController *rayLocationController)
{
  // int rows;
  // int cols;


  _currentView = view;

  _currentModel = model;

  _rayLocationController = rayLocationController;

  //  functionsModel = new SoloFunctionsModel(_currentModel);
  //_soloFunctions = new SoloFunctions(_currentModel->_vol);
  //_currentView->setupSoloFunctions(_soloFunctions);

  // connect view signals to controller slots

  connect(_currentView, SIGNAL(needFieldNames()), this, SLOT(needFieldNames()));
  connect(_currentView, SIGNAL(needRangeData(size_t)), this, SLOT(needRangeData(size_t)));
  connect(_currentView, SIGNAL(needDataForField(string, int, int)), 
	  this, SLOT(needDataForField(string, int, int)));
  connect(_currentView, SIGNAL(needAzimuthForRay(int, int, string)), 
    this, SLOT(needAzimuthForRay(int, int, string)));
  connect(_currentView, SIGNAL(needNyquistVelocityForRay(int, int, string)), 
    this, SLOT(needNyquistVelocityForRay(int, int, string)));
  // TODO: need to know which sweep!!!
  connect(_currentView, SIGNAL(applyVolumeEdits(string, float, int, vector<float> *)), 
	  this, SLOT(getVolumeChanges(string, float, int, vector<float> *)));

  connect(_currentView, SIGNAL(signalRayAzimuthChange(float, int)), this, SLOT(switchRay(float, int)));

 // FYI: connect(PolarManager, newSweepData, this, SLOT(displaySweepData());

    // connect controller slots to model signals 

    // connect model signals to controller slots 
  /*
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetController::updateStatus);
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetController::updateColor);
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetController::updateLineEdit);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheetController::updateStatus);
    connect(formulaInput, &QLineEdit::returnPressed, this, &SpreadSheetController::returnPressed);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheetController::updateLineEdit);
  */
}


void SpreadSheetController::moveToLocation(string fieldName, int sweepNumber,
    float azimuth) {
  //_currentView->setAzimuth(azimuth);
  //switchRay(azimuth, sweepNumber);
}

void SpreadSheetController::moveToLocation(string fieldName, int sweepNumber,
    float azimuth, float range) {

  //moveToLocation(fieldName, sweepNumber, azimuth);
  _currentView->highlightClickedData(fieldName, azimuth, sweepNumber, range);
}



void SpreadSheetController::switchRay(float azimuth, int sweepNumber) {
  LOG(DEBUG) << "enter";
  //if ()

  //HERE not switching azimuth; the azimuth sent is from the polar manager -- not pulled from dialog!!
  emit selectSweep(sweepNumber);
  // QCoreApplication::processEvents();
  LOG(DEBUG) << "exit";
}
      
// Keep the rayLoc index in one place, the rayLocationMVC
// since we can get the closest ray data fast using the rayLocation classes,
// there is no need to store the indexes here.

void SpreadSheetController::displaySweepData(int sweepNumber) {
  LOG(DEBUG) << "enter";

  //if (sweepNumber == _currentView->getSweepNumber()) {
    float azimuth = _currentView->getAzimuth();
    int _nRays = _currentView->getNRaysToDisplay();

    _currentView->updateLocationInVolume(azimuth, sweepNumber);    

    //size_t closestRayIdx = _rayLocationController->getRayIdx(azimuth);
    // for each display/selected field 
    vector<string> displayFields = _currentView->getFieldNamesToDisplay();

    // rayIdx goes from 0 to nRays; map to -nRays/2 ... 0 ... nRays/2
    for (int rayIdx= - _nRays/2; rayIdx <= _nRays/2; rayIdx++) {
      int offsetFromClosest = rayIdx;
      for (int fieldIdx=0; fieldIdx < (int) displayFields.size(); fieldIdx++) {  
    
        // use RayLoc to find the closest ray
        // int idx = offsetFromClosest + _nRays/2;
        //_currentModel->setRay(idx,
        //  _rayLocationController->getClosestRay(azimuth, offsetFromClosest));
        vector <float> *data = 
          //_currentModel->getData(displayFields.at(fieldIdx), offsetFromClosest);
          _rayLocationController->getRayDataOffset(azimuth, offsetFromClosest, 
            displayFields.at(fieldIdx));
    //_currentModel->setClosestRay(closestRayIdx, sweepNumber);
    //LOG(DEBUG) << "switching to ray " << azimuth;
    //_currenView->newElevation(elevation);
    // _currentView->updateLocationInVolume(azimuth, sweepNumber);
  //} catch (std::invalid_argument &ex) {
  //  LOG(DEBUG) << "ERROR: " << ex.what();
    //_currentView->criticalMessage(ex.what());
        _currentView->fieldDataSent(data, offsetFromClosest, fieldIdx); 
      }      
    //}

/* ----
HERE!!!
    // rayIdx goes from 0 to nRays; map to -nRays/2 ... 0 ... nRays/2
    for (int rayIdx= - _nRays/2; rayIdx <= _nRays/2; rayIdx++) {
        int fieldIdx = 0;

        vector<string>::iterator it; 
        for(it = fieldNames.begin(); it != fieldNames.end(); it++) {
          QString the_name(QString::fromStdString(*it));
          LOG(DEBUG) << *it;
          _fieldNames.push_back(the_name);
          for (int i=0; i<_nRays; i++) {
            // this ultimately calls setHeader; we need to send the info needed for setHeader
            emit needAzimuthForRay(rayIdx, fieldIdx, *it);
            // needAzimuthForRay(int offsetFromClosest, 

            //table->setHorizontalHeaderItem(c + (i*_nFieldsToDisplay), 
             //   new QTableWidgetItem(the_name));
            // TODO: what about setHorizontalHeaderLabels(const QStringList &labels) instead? would it be faster?
          }
          //emit needDataForField(*it, rayIdx, fieldIdx);  // need to push this, not pull it.
          //emit needAzimuthForRay(rayIdx);     
          fieldIdx += 1;
        }
    }

    // ----
    */

  }
  LOG(DEBUG) << "exit";
}

vector<string>  *SpreadSheetController::getFieldNames()
{
  vector<string> *names = _currentModel->getFields();
  LOG(DEBUG) << " In SpreadSheetController::getFieldNames, there are " << names->size() << " field names";
  return names;
}

/*
vector<float> *SpreadSheetController::getData(string fieldName, int offsetFromClosest)
{

  LOG(DEBUG) << "getting values for " << fieldName;

  
  //return _currentModel->getData(fieldName);
  
  //  vector<float> SpreadSheetModel::getData(string fieldName)
  //vector<float> *data = _currentModel->getData(fieldName, offsetFromClosest);
  cout << "MAJOR ERROR HERE!!! getData is needed" << endl;

  LOG(DEBUG) << " found " << data->size() << " data values ";

  return data;
 
}
*/

float SpreadSheetController::getAzimuthForRay(int offsetFromClosest)
{

  LOG(DEBUG) << "getting azimuth for ray: offset from closest =" << offsetFromClosest;
  float startingAzimuth = _currentView->getAzimuth();

  //float azimuth = _currentModel->getAzimuthForRay(offsetFromClosest);
  float azimuth = _rayLocationController->getAzimuthForRay(startingAzimuth, offsetFromClosest);

  LOG(DEBUG) << " found: azimuth=" << azimuth;

  return azimuth;
 
}

float SpreadSheetController::getNyquistVelocity(int offsetFromClosest)
{
  float startingAzimuth = _currentView->getAzimuth();
  LOG(DEBUG) << "getting nyquist velocity for ray: offset from closest =" << offsetFromClosest;
  //float nyquistVelocity = _currentModel->getNyquistVelocityForRay(offsetFromClosest);
  float nyquistVelocity = _rayLocationController->getNyquistVelocityForRay(startingAzimuth, offsetFromClosest);
  LOG(DEBUG) << " found: nyq vel =" << nyquistVelocity;

  return nyquistVelocity;
 
}

void SpreadSheetController::getRangeData(float *startingRangeKm, float *gateSpacingKm)
{
  _currentModel->getRangeGeom(startingRangeKm, gateSpacingKm);
  LOG(DEBUG) << " In SpreadSheetController::getRangeGeom, startingRangeKm = " 
    << *startingRangeKm << ", gateSpacingKm = " << *gateSpacingKm;
}

void SpreadSheetController::setData(string fieldName, float azimuth, 
  int sweepNumber, vector<float> *data)
{
  LOG(DEBUG) << "setting values for " << fieldName;
  _currentModel->setData(fieldName, azimuth, sweepNumber, data);
}

void SpreadSheetController::setDataMissing(string fieldName, float missingDataValue) {
  _currentModel->setDataMissing(fieldName, missingDataValue);
}

void  SpreadSheetController::needFieldNames() {
  _currentView->fieldNamesProvided(getFieldNames());
}

/*
void  SpreadSheetController::needDataForField(string fieldName, int offsetFromClosest, int c) {

  int useless = 0;
  _currentView->fieldDataSent(getData(fieldName, offsetFromClosest), offsetFromClosest, c);
}
*/

void  SpreadSheetController::needAzimuthForRay(int offsetFromClosest, 
  int fieldIdx, string fieldName) {

//azimuthForRaySent(float azimuth, int offsetFromClosestRay, int fieldIdx, string fieldName)
  _currentView->azimuthForRaySent(getAzimuthForRay(offsetFromClosest), offsetFromClosest,
    fieldIdx, fieldName);
}

void  SpreadSheetController::needNyquistVelocityForRay(int offsetFromClosest, 
  int fieldIdx, string fieldName) {
  _currentView->nyquistVelocitySent(getNyquistVelocity(offsetFromClosest), offsetFromClosest,
    fieldIdx, fieldName);
}

void  SpreadSheetController::needRangeData(size_t nGates) {
  float startingKm;
  float gateSpacingKm;
  getRangeData(&startingKm, &gateSpacingKm);
  _currentView->rangeDataSent(nGates, startingKm, gateSpacingKm);
}

// persist the changes in the spreadsheet to the model, which is the data volume
void SpreadSheetController::getVolumeChanges(string fieldName, float azimuth, 
  int sweepNumber, vector<float> *data) {

  LOG(DEBUG) << "enter";
  //vector<string> *fields = _currentView->getVariablesFromSpreadSheet();

  // update the model
  //int column = 0;
  //for(vector<string>::iterator s = fields->begin(); s != fields->end(); s++) {
  //  vector<float> *data = _currentView->getDataForVariableFromSpreadSheet(column, *s);
  setData(fieldName, azimuth, sweepNumber, data);

  //  column++;
  //}
  // announce changes have been made to the data
  // volumeUpdated();
  LOG(DEBUG) << "exit";
}

void SpreadSheetController::volumeUpdated() {
  emit volumeChanged(); // _currentModel->getVolume());
}



void SpreadSheetController::open(string fileName)
{


}






