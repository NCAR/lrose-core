//
// SpreadSheetModel provides data in basic form, as ints, floats, strings
// SpreadSheetModel provides the interface to Radx utilities, and file I/O

#include <stdio.h>
#include <stdexcept>
#include "SpreadSheetModel.hh"
#include <toolsa/LogStream.hh>
#include <fstream>
//#include "spreadsheet.hh"
//#include "spreadsheetdelegate.hh"
//#include "spreadsheetitem.hh"


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
  closestRay->print(cout);
  _vol = dataVolume;
  _vol->loadFieldsFromRays();
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

// return a list of data values
vector<float> SpreadSheetModel::getSampleData()
{

  vector<float> data;
  data.push_back(1.0);
  data.push_back(3.0);
  data.push_back(5.0);
  data.push_back(10.0);
  return data;
}


// return a list of data values for the given
// field name
// vector<double>
vector<float> *SpreadSheetModel::getData(string fieldName)
{

  // vector <double> *dataVector = NULL;
  const RadxField *field;
  //  field = _vol.getFieldFromRay(fieldName);  // <--- Why is this returning NULL
  // because the type is 
  // from debugger:  *((vol.getFieldFromRay("VEL"))->getDataSi16()+1)
  field = _closestRay->getField(fieldName);

  if (field == NULL) {
    throw std::invalid_argument("no RadxField found "); //  <<  endl;
    //return NULL; // dataVector;
  } 
  // Radx::fl32 *data = field->getDataFl32();
  // how may gates?
  size_t nPoints = field->getNPoints();
  LOG(DEBUG_VERBOSE) << "nGates = " << nPoints;
  float *data = (float *) field->getDataFl32();
  LOG(DEBUG_VERBOSE) << data << ", " << data+1 << ", " << data +2;
  vector<float> *dataVector = new vector<float>(data, data + nPoints);
  //  LOG(DEBUG_VERBOSE) << dataVector.at[0] << ", " << dataVector.at[1] << ", " << dataVector.at[2];

  //vector<float> *dataVectorPtr = &dataVector;
  for (int i=0; i<3; i++) {
    float value = dataVector->at(i);
    LOG(DEBUG_VERBOSE) << value;
  }

  return dataVector;

  /*
  for (size_t i=0; i<nPoints; i++) { 
    //for (size_t i=0; i<200; i++) { 
    double value = field->getDoubleValue(i);
    //cout << value << " ";
    dataVector.push_back(value); // data[i]);
  }
  cout << endl;
  // convert data to vector
  return dataVector;
  */
}

/*
RadxVol SpreadSheetModel::getVolume() {
  return _vol;
}
*/

// set data values for the field in the Volume 
void SpreadSheetModel::setData(string fieldName, vector<float> *data)
{
  LOG(DEBUG_VERBOSE) << "fieldName=" << fieldName;

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

}
