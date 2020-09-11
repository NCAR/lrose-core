
#include <vector>
#include <iostream>

#include <string>
#include <sstream>
#include <iterator>

#include <Radx/RadxRay.hh>
#include <toolsa/LogStream.hh>

#include "SoloFunctionsController.hh"
// #include "SpreadSheetModel.hh"

using namespace std;

SoloFunctionsController::SoloFunctionsController(RadxVol *data, QObject *parent) : QObject(parent) {
  _data = data;
  _nRays = _data->getNRays();
  _nSweeps = _data->getNSweeps();

}



template<typename Out>
void SoloFunctionsController::split(const string &s, char delim, Out result) {
  stringstream ss(s);
  string item;
  double value;
  while (getline(ss, item, delim)) {
    value = stod(item);
    *(result++) = value;
  }
}

/*
vector<string> SoloFunctions::split(const string &s, char delim) {
  vector<string> elems;
  split(s, delim, back_inserter(elems));
  return elems;
}
*/

vector<double> SoloFunctionsController::splitDouble(const string &s, char delim) {
  vector<double> elems;
  split(s, delim, back_inserter(elems));
  return elems;
}
/*
QVector<double> add(QVector<double> v, QVector<double> v2) {
  int size = v.size(); 
  if (size != v2.size()) {
    // TODO: throw exception
    // return the size of the shorter?
    QVector<double> v0(0);
    return v0;
  }
    //    QVector<double> v3 = v + v2; return v3; } // this appends one v2 elements to v                          
    QVector<double> v3(size);
    for (int i=0; i<size; i++) 
      v3[i]=v[i]+v2[i];
    return v3;
}
*/


QString  SoloFunctionsController::REMOVE_AIRCRAFT_MOTION(QString field, float nyquist, float bad_data,
					   size_t clip_gate) { 

  string tempFieldName = soloFunctionsModel.RemoveAircraftMotion(field.toStdString(), _data,
						     _currentRayIdx, _currentSweepIdx,
						      nyquist,
						      clip_gate,
						      bad_data,
						     field.toStdString());

  // returns name of new field in RadxVol
  return QString::fromStdString(tempFieldName);
}


QString  SoloFunctionsController::BB_UNFOLDING_FIRST_GOOD_GATE(QString field, float nyquist, 
							       int max_pos_folds,
							       int max_neg_folds,
							       size_t ngates_averaged,
							       float bad_data,
							       size_t clip_gate) {

  string tempFieldName = soloFunctionsModel.BBUnfoldFirstGoodGate(field.toStdString(), _data,
								 _currentRayIdx, _currentSweepIdx,
								 nyquist,
								 max_pos_folds,
								 max_neg_folds,
								 ngates_averaged,
								 clip_gate,
								 bad_data,
								 field.toStdString());

  // returns name of new field in RadxVol                                                                                       
  return QString::fromStdString(tempFieldName);
}


/*
// How to return a vector
QString  SoloFunctionsController::REMOVE_AIRCRAFT_MOTION(QString field) { 


  //SoloFunctionsModel soloFunctionsModel;

  // the value of the field has been substituted; If the field is a vector,
  // then the QString contains all the values as a comma separated list in a string
  // parse the field data into a vector 
  //vector<string> x = split(field.toStdString(), ',');

  //  vector<double> result = soloFunctionsModel.RemoveAircraftMotion(field.toStdString(), _data);
    vector<double> result = soloFunctionsModel.RemoveAircraftMotion(field.toStdString(), _data,
  								  _currentRayIdx, _currentSweepIdx);
 // ,
								  //								  _currentSweep, _currentRay);
  //  vector<double> result = soloFunctionsModel.RemoveAircraftMotion(x, dataModel);

  // TODO: what is being returned? the name of the new field in the model that
  // contains the results.
  // since the std::vector<double> data has to be copied to QVector anyway, 
  // go ahead and format it as a string?
  // maybe return a pointer to std::vector<double> ?? then when presenting the data, we can convert it to string,
  // but maintain the precision in the model (RadxVol)??
  //QString newFieldName = field + "2";
  // TODO: dataModel->addField(newFieldName.toStdString(), result);

  QString newData;
  cerr << "list of result values: " << endl;
  for (vector<double>::iterator it = result.begin(); it != result.end(); ++it) {
    cerr << ' ' << *it;
    newData.append(QString::number(*it));
    newData.append(',');
  }
  cerr << endl;

  return newData;
}
*/

// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::ZERO_MIDDLE_THIRD(QString field) { 

  //SoloFunctionsModel soloFunctionsModel;

  string tempFieldName = soloFunctionsModel.ZeroMiddleThird(field.toStdString(), _data,
						     _currentRayIdx, _currentSweepIdx,
						     field.toStdString()); // "VEL_xyz");

  // TODO: returns name of new field in RadxVol

  return QString::fromStdString(tempFieldName); // QString("zero middle result");
}


// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::ZERO_INSIDE_BOUNDARY(QString field) { 

  //SoloFunctionsModel soloFunctionsModel;

  string tempFieldName = soloFunctionsModel.ZeroInsideBoundary(field.toStdString(), _data,
						     _currentRayIdx, _currentSweepIdx,
						     field.toStdString()); // "VEL_xyz");

  // TODO: returns name of new field in RadxVol

  return QString::fromStdString(tempFieldName); // QString("zero middle result");
}


// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::DESPECKLE(QString field, size_t speckle_length, float bad_data,
					   size_t clip_gate) { 

  //SoloFunctionsModel soloFunctionsModel;

  string tempFieldName = soloFunctionsModel.Despeckle(field.toStdString(), _data,
						     _currentRayIdx, _currentSweepIdx,
						      speckle_length,
						      clip_gate,
						      bad_data,
						     field.toStdString());

  // returns name of new field in RadxVol
  return QString::fromStdString(tempFieldName);
}

// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::FLAGGED_ADD(QString field, float constant, float bad_data,
					     size_t clip_gate, QString bad_flag_field) { 

  // updated bad_flag_field is returned in tempFieldName
  string tempFieldName = soloFunctionsModel.FlaggedAdd(field.toStdString(), _data,
						       _currentRayIdx, _currentSweepIdx,
						       constant,
						       clip_gate,
						       bad_data,
						       bad_flag_field.toStdString());
  return QString::fromStdString(tempFieldName);
}

// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::FLAGGED_MULTIPLY(QString field, float constant, float bad_data,
					   size_t clip_gate, QString bad_flag_field) { 

  string tempFieldName = soloFunctionsModel.FlaggedMultiply(field.toStdString(), _data,
						     _currentRayIdx, _currentSweepIdx,
						      constant,
						      clip_gate,
						      bad_data,
						     bad_flag_field.toStdString());
  // returns name of new field in RadxVol
  return QString::fromStdString(tempFieldName); 
}



// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::AND_BAD_FLAGS_ABOVE(QString field, float constant, 
						     QString mask_field, float bad_data,
						     size_t clip_gate) {
  // last arg is field name, which will be used to create  bad_flag_field returned in tempFieldName
  string tempFieldName = soloFunctionsModel.AndBadFlagsAbove(field.toStdString(), _data,
							     _currentRayIdx, _currentSweepIdx,
							     constant,
							     clip_gate,
							     bad_data,
							     mask_field.toStdString());
  return QString::fromStdString(tempFieldName);
} 

QString SoloFunctionsController::AND_BAD_FLAGS_BELOW(QString field, float constant, 
						     QString mask_field, float bad_data,
						     size_t clip_gate) {
  // last arg is field name, which will be used to create  bad_flag_field returned in tempFieldName
  string tempFieldName = soloFunctionsModel.AndBadFlagsBelow(field.toStdString(), _data,
							     _currentRayIdx, _currentSweepIdx,
							     constant,
							     clip_gate,
							     bad_data,
							     mask_field.toStdString());
  return QString::fromStdString(tempFieldName);
} 

QString SoloFunctionsController::AND_BAD_FLAGS_BETWEEN(QString field, float constantLower,
                                                       float constantUpper, 
						       QString mask_field, float bad_data,
					               size_t clip_gate) {
  // last arg is field name, which will be used to create  bad_flag_field returned in tempFieldName
  string tempFieldName = soloFunctionsModel.AndBadFlagsBetween(field.toStdString(), _data,
							     _currentRayIdx, _currentSweepIdx,
							     constantLower, constantUpper,
							     clip_gate,
							     bad_data,
							     mask_field.toStdString());
  return QString::fromStdString(tempFieldName);
} 

// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::OR_BAD_FLAGS_ABOVE(QString field, float constant, 
						    QString mask_field, float bad_data,
						    size_t clip_gate) {
  // last arg is field name, which will be used to create  bad_flag_field returned in tempFieldName
  string tempFieldName = soloFunctionsModel.AndBadFlagsAbove(field.toStdString(), _data,
							     _currentRayIdx, _currentSweepIdx,
							     constant,
							     clip_gate,
							     bad_data,
							     mask_field.toStdString());
  return QString::fromStdString(tempFieldName);
} 

QString SoloFunctionsController::OR_BAD_FLAGS_BELOW(QString field, float constant, 
						    QString mask_field, float bad_data,
						    size_t clip_gate) {
  // last arg is field name, which will be used to create  bad_flag_field returned in tempFieldName
  string tempFieldName = soloFunctionsModel.AndBadFlagsBelow(field.toStdString(), _data,
							     _currentRayIdx, _currentSweepIdx,
							     constant,
							     clip_gate,
							     bad_data,
							     mask_field.toStdString());
  return QString::fromStdString(tempFieldName);
} 

QString SoloFunctionsController::OR_BAD_FLAGS_BETWEEN(QString field, float constantLower,
						      float constantUpper,
						      QString mask_field, float bad_data,
						      size_t clip_gate) {
  // last arg is field name, which will be used to create  bad_flag_field returned in tempFieldName
  string tempFieldName = soloFunctionsModel.OrBadFlagsBetween(field.toStdString(), _data,
							     _currentRayIdx, _currentSweepIdx,
							     constantLower, constantUpper,
							     clip_gate,
							     bad_data,
							     mask_field.toStdString());
  return QString::fromStdString(tempFieldName);
} 

// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::XOR_BAD_FLAGS_ABOVE(QString field, float constant, 
						     QString mask_field, float bad_data,
						     size_t clip_gate) {
  // last arg is field name, which will be used to create  bad_flag_field returned in tempFieldName
  string tempFieldName = soloFunctionsModel.XorBadFlagsAbove(field.toStdString(), _data,
							     _currentRayIdx, _currentSweepIdx,
							     constant,
							     clip_gate,
							     bad_data,
							     mask_field.toStdString());
  return QString::fromStdString(tempFieldName);
} 

QString SoloFunctionsController::XOR_BAD_FLAGS_BELOW(QString field, float constant,
						     QString mask_field, 
						     float bad_data,
						     size_t clip_gate) {
  // last arg is field name, which will be used to create  bad_flag_field returned in tempFieldName
  string tempFieldName = soloFunctionsModel.XorBadFlagsBelow(field.toStdString(), _data,
							     _currentRayIdx, _currentSweepIdx,
							     constant,
							     clip_gate,
							     bad_data,
							     mask_field.toStdString());
  return QString::fromStdString(tempFieldName);
} 

// HERE <<<=== implement default values and send a bad flag mask field variable
QString SoloFunctionsController::XOR_BAD_FLAGS_BETWEEN(QString field, 
						       float constantLower, float constantUpper,
						       QString mask_field,
						       float bad_data,
					               size_t clip_gate) {
  // last arg is field name, which will be used to create  bad_flag_field returned in tempFieldName
  string tempFieldName = soloFunctionsModel.XorBadFlagsBetween(field.toStdString(), _data,
							     _currentRayIdx, _currentSweepIdx,
							     constantLower, constantUpper,
							     clip_gate,
							     bad_data,
							     mask_field.toStdString());
  return QString::fromStdString(tempFieldName);
} 
/*
QString SoloFunctionsController::OR_BAD_FLAGS_ABOVE(QString field, float constant, float bad_data,
					   size_t clip_gate) {
  return QString::fromStdString("empty");
} 

QString SoloFunctionsController::XOR_BAD_FLAGS_ABOVE(QString field, float constant, float bad_data,
					   size_t clip_gate) {
  return QString::fromStdString("empty");
} 
*/
// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::ASSERT_BAD_FLAGS(QString field, float bad_data,
						  size_t clip_gate, QString badFlagMaskFieldName) { 
  string tempFieldName = soloFunctionsModel.AssertBadFlags(field.toStdString(), _data,
							   _currentRayIdx, _currentSweepIdx,
							   clip_gate, bad_data,
							   badFlagMaskFieldName.toStdString());
							  
  return QString::fromStdString(tempFieldName);
} 

// return the name of the field in which the result is stored in the RadxVol
// TODO: What is this? Not a function, just a help message
// QString SoloFunctionsController::BAD_FLAGS_MASK(QString field, float constant, float bad_data,
//					   size_t clip_gate) { 
//  return QString::fromStdString("empty");
//} 

// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::CLEAR_BAD_FLAGS(QString field) { //float constant, float bad_data,
  // size_t clip_gate) { 
  string tempFieldName = soloFunctionsModel.ClearBadFlags(field.toStdString(), _data,
							  _currentRayIdx, _currentSweepIdx);
							  
  return QString::fromStdString(tempFieldName);
} 

// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::COMPLEMENT_BAD_FLAGS(QString field) { 

  string tempFieldName = soloFunctionsModel.ComplementBadFlags(field.toStdString(), _data,
							  _currentRayIdx, _currentSweepIdx);
							  
  return QString::fromStdString(tempFieldName);
} 


// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::COPY_BAD_FLAGS(QString field, float bad_data, size_t clip_gate) {
  string tempFieldName = soloFunctionsModel.CopyBadFlags(field.toStdString(), _data,
							 _currentRayIdx, _currentSweepIdx,
							 clip_gate, bad_data); 
  return QString::fromStdString(tempFieldName);
} 


/*

  CLIP = 35;
  BAD_DATA_VALUE = -9E+35;
  CLEAR_BAD_FLAGS(BAD_FLAG_MASK)

  // SET ==> overwriting existing values; create
  BAD_FLAG_MASK = SET_BAD_FLAGS_BETWEEN(VE, -1., 1., BAD_DATA_VALUE, CLIP)
  COPY_BAD_FLAGS ... ???

  // AND/OR/XOR ==> use existing values to determine new values; update
  BAD_FLAG_MASK = AND_BAD_FLAGS_ABOVE(DZ, 35., BAD_DATA_VALUE, CLIP,  BAD_FLAG_MASK)
  VE2 = ASSERT_BAD_FLAGS(VE, BAD_DATA_VALUE, CLIP, BAD_FLAG_MASK)
  DZ2 = ASSERT_BAD_FLAGS(DZ, BAD_DATA_VALUE, CLIP, BAD_FLAG_MASK)

*/


// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::SET_BAD_FLAGS_ABOVE(QString field, float constant, float bad_data,
					   size_t clip_gate) { 
  // last arg is field name, which will be used to create  bad_flag_field returned in tempFieldName
  string tempFieldName = soloFunctionsModel.SetBadFlagsAbove(field.toStdString(), _data,
							     _currentRayIdx, _currentSweepIdx,
							     constant,
							     clip_gate,
							     bad_data,
							     field.toStdString());
  return QString::fromStdString(tempFieldName);
} 


// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::SET_BAD_FLAGS_BELOW(QString field, float constant, float bad_data,
					   size_t clip_gate) { 
  // last arg is field name, which will be used to create  bad_flag_field returned in tempFieldName
  string tempFieldName = soloFunctionsModel.SetBadFlagsBelow(field.toStdString(), _data,
							     _currentRayIdx, _currentSweepIdx,
							     constant,
							     clip_gate,
							     bad_data,
							     field.toStdString());
  return QString::fromStdString(tempFieldName);
} 


// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::SET_BAD_FLAGS_BETWEEN(QString field, float lower_threshold,
						       float upper_threshold, float bad_data,
						       size_t clip_gate) {
  // last arg is field name, which will be used to create  bad_flag_field returned in tempFieldName
  string tempFieldName = soloFunctionsModel.SetBadFlagsBetween(field.toStdString(), _data,
							       _currentRayIdx, _currentSweepIdx,
							       lower_threshold, upper_threshold,
							       clip_gate,
							       bad_data,
							       field.toStdString());
  return QString::fromStdString(tempFieldName);
} 

/*
// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::FLAG_FRECKLES(QString field, float constant, float bad_data,
					   size_t clip_gate) { 
  return QString::fromStdString("empty");
} 

// return the name of the field in which the result is stored in the RadxVol
QString SoloFunctionsController::FLAG_GLITCHES(QString field, float constant, float bad_data,
					   size_t clip_gate) { 
  return QString::fromStdString("empty");
} 
*/

const vector<bool> *SoloFunctionsController::GetBoundaryMask() {

  return soloFunctionsModel.GetBoundaryMask();
}


void SoloFunctionsController::applyBoundary(bool useBoundaryMask) {

  //SoloFunctionsModel soloFunctionsModel;
  soloFunctionsModel.SetBoundaryMask(_data, _currentRayIdx, _currentSweepIdx,
				     useBoundaryMask);
}

void SoloFunctionsController::setCurrentSweepToFirst() {
  cerr << "entry setCurrentSweepToFirst" << endl;
  _currentSweepIdx = 0;
  cerr << "exit setCurrentSweepToFirst" << endl;

  //LOG(DEBUG) << "exit";

}


void SoloFunctionsController::setCurrentRayToFirst() {
  //cerr << "entry setCurrentRayToFirst" << endl;
  _currentRayIdx = 0;
  //applyBoundary();
  //cerr << "exit setCurrentRayToFirst" << endl;

  //LOG(DEBUG) << "exit";

}

void SoloFunctionsController::nextRay() {
  //LOG(DEBUG) << "entry";
  //cerr << "entry nextRay" << endl;
  _currentRayIdx += 1;
  //  applyBoundary();
  //cerr << "exit nextRay" << endl;
  //LOG(DEBUG) << "exit";

}

bool SoloFunctionsController::moreRays() {
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
  return (_currentRayIdx < _nRays);
}

void SoloFunctionsController::nextSweep() {
  //LOG(DEBUG) << "entry";
  //cerr << "entry nextSweep" << endl;
  _currentSweepIdx += 1;
  //cerr << "exit nextSweep" << endl;
  //LOG(DEBUG) << "exit";
}

bool SoloFunctionsController::moreSweeps() {
  //  LOG(DEBUG) << "entry";
  //cerr << "entry moreSweeps" << endl;
  //  const size_t nSweeps = _data->getNSweeps();
  //LOG(DEBUG) << " there are " << nSweeps << " sweeps";;
  //if (_currentSweepIdx >= nSweeps)
  //  _data->loadFieldsFromRays();

  return (_currentSweepIdx < _nSweeps);
}


void SoloFunctionsController::assign(string tempName, string userDefinedName) {
  //_data->loadFieldsFromRays(); // TODO: this is a costly function as it moves the data/or pointers
  // TODO: where are the field names kept? in the table map? can i just change that?
  // Because each RadxRay holds its own FieldNameMap,
  // TODO: maybe ...
  vector<RadxRay *> rays = _data->getRays();
  // for each ray, 
  vector<RadxRay *>::iterator it;
  for (it=rays.begin(); it != rays.end(); ++it) {
     // renameField(oldName, newName);
    (*it)->renameField(tempName, userDefinedName);
    // loadFieldNameMap
    (*it)->loadFieldNameMap();

  }
  // end for each ray
  //
  /* 
  RadxField *theField = _data->getField(tempName);
  if (theField == NULL) throw "Error: no field " + tempName + " found for " + userDefinedName + "  in data volume (SoloFunctionsController)";
  theField->setName(userDefinedName);
  theField->setLongName(userDefinedName);
  theField->setStandardName(userDefinedName);
  _data->loadRaysFromFields();
  */
}

// Return data for the field, at the current sweep and ray indexes.
const vector<float> *SoloFunctionsController::getData(string &fieldName) {

  return soloFunctionsModel.GetData(fieldName, _data, _currentRayIdx, _currentSweepIdx);

}

void SoloFunctionsController::setData(string &fieldName, vector<float> *fieldData) {
        soloFunctionsModel.SetData(fieldName, _data, _currentRayIdx, _currentSweepIdx, fieldData); 
}

/*
SoloFunctions::SoloFunctions() // SpreadSheetController *controller) 
{
  //_controller = controller;

}

QString SoloFunctions::cat(QString animal) 
{ 
  
  return animal.append(" instead of cat");
}


// TODO: make functions static, and pass in all values; DO NOT associate any
// data with the functions.
// - or - 
// wrap the function with the context which contains all the extraneous data needed
// 
QString SoloFunctions::REMOVE_AIRCRAFT_MOTION(QString field) 
{
  QString result(tr("|"));
  
  // find the field in the data?
  // return the first value of the field
  vector<string> fieldNames = _controller->getFieldNames();

  int c = 0;
  int r = 0;
  vector<string>::iterator it;
  for(it = fieldNames.begin(); it != fieldNames.end(); it++, c++) {
    QString the_name(QString::fromStdString(*it));
    cerr << *it << endl;
    if (the_name.compare(field) == 0) {

      vector<double> data = _controller->getData(*it);

      cerr << "found field; number of data values = " << data.size() << endl;

      for (r=0; r<20; r++) {
        string format = "%g";
        char formattedData[250];
      //    sprintf(formattedData, format, data[0]);
        sprintf(formattedData, "%g ", data[r]);
        result.append(formattedData);
      }
    }
  }
  
  return result;
}
*/
