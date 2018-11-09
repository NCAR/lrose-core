
#include <vector>
#include <iostream>

#include "SoloFunctions.hh"

using namespace std;
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
