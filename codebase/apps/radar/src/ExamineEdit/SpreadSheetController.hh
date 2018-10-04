

#ifndef SPREADSHEETCONTROLLER_HH
#define SPREADSHEETCONTROLLER_HH

#include "SpreadSheetModel.hh"
// #include "SpreadSheetView.hh"

#include <vector>

class SpreadSheetView;

using namespace std;

class SpreadSheetController
{

public:

  SpreadSheetController(SpreadSheetView *view);

  vector<string> getFieldNames();
  vector<double> getData(string fieldName);

  void open(string fileName);

private:

  SpreadSheetModel *_currentModel;
  SpreadSheetView *_currentView;

};


#endif // SPREADSHEETCONTROLLER_H
