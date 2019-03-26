

#ifndef SPREADSHEETCONTROLLER_HH
#define SPREADSHEETCONTROLLER_HH

#include <QObject>
#include "SpreadSheetView.hh"
#include "SpreadSheetModel.hh"
#include "SoloFunctionsModel.hh"
//#include "PolarWidget.hh"
//#include <Radx/RadxVol.hh>
#include "SoloFunctions.hh"

#include <vector>

//class SpreadSheetView;

using namespace std;

class SpreadSheetController : QObject
{

  Q_OBJECT

public:

  SpreadSheetController(SpreadSheetView *view);
  SpreadSheetController(SpreadSheetView *view, SpreadSheetModel *model);

  vector<string> getFieldNames();
  vector<double> getData(string fieldName);

  void open(string fileName);

  SpreadSheetModel *getDataModel() {return _currentModel;};

public slots:
  void needFieldNames();
  void needDataForField(string fieldName, int r, int c);

private:

  SpreadSheetModel *_currentModel;
  SpreadSheetView *_currentView;
  SoloFunctions *_soloFunctions;

};


#endif // SPREADSHEETCONTROLLER_H
