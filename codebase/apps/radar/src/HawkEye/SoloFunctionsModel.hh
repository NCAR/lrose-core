#ifndef SOLOFUNCTIONSMODEL_H
#define SOLOFUNCTIONSMODEL_H

#include <stdio.h>

#include <vector>
#include <iostream>

#include "Radx/RadxVol.hh"

using namespace std;


class SoloFunctionsModel
{


public:
  SoloFunctionsModel() {}

  vector<double> RemoveAircraftMotion(vector<double>, RadxVol &vol); // SpreadSheetModel *context);
  vector<double> RemoveAircraftMotion(string fieldName, RadxVol &vol); // SpreadSheetModel *context);
 
private:

  //  SpreadSheetModel *dataModel;
};


#endif
