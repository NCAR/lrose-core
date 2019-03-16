#ifndef SOLOFUNCTIONSMODEL_H
#define SOLOFUNCTIONSMODEL_H

#include <stdio.h>

#include <vector>
#include <iostream>

#include "SpreadSheetModel.hh"

using namespace std;


class SoloFunctionsModel
{


public:
  SoloFunctionsModel() {}

  vector<double> RemoveAircraftMotion(vector<double>, SpreadSheetModel *context);
  vector<double> RemoveAircraftMotion(string fieldName, SpreadSheetModel *context);
 
private:

  //  SpreadSheetModel *dataModel;
};


#endif
