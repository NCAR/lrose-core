//
// ScriptEditorModel provides data in basic form, as ints, floats, strings
// ScriptEditorModel provides the interface to Radx utilities, and file I/O

#include <stdio.h>
#include <stdexcept>
#include "ScriptEditorModel.hh"
#include <toolsa/LogStream.hh>
#include <fstream>


ScriptEditorModel::ScriptEditorModel()
{

}

ScriptEditorModel::ScriptEditorModel(RadxVol *dataVolume)
{
  _vol = dataVolume;
  _vol->loadFieldsFromRays();
}



vector<string> ScriptEditorModel::getFields()
{
  vector<string> fieldNames;
  /*
  if (_closestRay != NULL) {
    _closestRay->loadFieldNameMap();

    RadxRay::FieldNameMap fieldNameMap = _closestRay->getFieldNameMap();
    RadxRay::FieldNameMapIt it;
    for (it = fieldNameMap.begin(); it != fieldNameMap.end(); it++) {
      fieldNames.push_back(it->first);
      cout << it->first << ':' << it->second << endl;
    }
  } else {
  */
    _vol->loadFieldsFromRays();
    fieldNames = _vol->getUniqueFieldNameList();
    //}
  return fieldNames;
}

// TODO: we'll need some way of looping through the rays; "for each ray"
// like getNextRay(); initForEachRay();  endOfRays(); 


/*
// return a list of data values for the given
// field name
// vector<double>
vector<float> *ScriptEditorModel::getData(string fieldName)
{

  const RadxField *field;
  field = _closestRay->getField(fieldName);

  if (field == NULL) {
    throw std::invalid_argument("no RadxField found "); //  <<  endl;
  } 
  size_t nPoints = field->getNPoints();
  LOG(DEBUG) << "nGates = " << nPoints;
  float *data = (float *) field->getDataFl32();
  LOG(DEBUG) << data << ", " << data+1 << ", " << data +2;
  vector<float> *dataVector = new vector<float>(data, data + nPoints);
  for (int i=0; i<3; i++) {
    float value = dataVector->at(i);
    LOG(DEBUG) << value;
  }

  return dataVector;

}


// set data values for the field in the Volume 
void ScriptEditorModel::setData(string fieldName, vector<float> *data)
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
*/
