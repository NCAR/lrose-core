
#include <vector>
#include <iostream>

#include <string>
#include <sstream>
#include <iterator>

#include "SoloFunctions.hh"
#include "SoloFunctionsModel.hh"
// #include "SpreadSheetModel.hh"

using namespace std;

template<typename Out>
void SoloFunctions::split(const string &s, char delim, Out result) {
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

vector<double> SoloFunctions::splitDouble(const string &s, char delim) {
  vector<double> elems;
  split(s, delim, back_inserter(elems));
  return elems;
}


// TODO:  parameters should be DataField ??
QString  SoloFunctions::REMOVE_AIRCRAFT_MOTION(QString field) { 

  SoloFunctionsModel soloFunctionsModel;

  // the value of the field has been substituted; If the field is a vector,
  // then the QString contains all the values as a comma separated list in a string
  // parse the field data into a vector 
  //vector<string> x = split(field.toStdString(), ',');
  /*vector<double> x = splitDouble(field.toStdString(), ',');

  cerr << "list of field values: " << endl;
  for (vector<double>::iterator it = x.begin(); it != x.end(); ++it) 
    cerr << ' ' << *it;
  cerr << endl;
  */

  vector<double> result = soloFunctionsModel.RemoveAircraftMotion(field.toStdString(), _data);
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
