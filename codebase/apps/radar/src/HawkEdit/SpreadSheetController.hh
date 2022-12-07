

#ifndef SPREADSHEETCONTROLLER_HH
#define SPREADSHEETCONTROLLER_HH

#include <QObject>
#include "SpreadSheetView.hh"
#include "SpreadSheetModel.hh"
#include "RayLocationController.hh"

#include <vector>

class SpreadSheetView;

using namespace std;

class SpreadSheetController : public QObject
{

  Q_OBJECT

public:

  SpreadSheetController(SpreadSheetView *view);
  SpreadSheetController(SpreadSheetView *view, SpreadSheetModel *model,
    RayLocationController *rayLocationController);



  vector<string> *getFieldNames();
  //vector<float> *getData(string fieldName, int offsetFromClosest);
  float getAzimuthForRay(int offsetFromClosest);
  void getRangeData(float *startingRangeKm, float *gateSpacingKm);
  float getNyquistVelocity(int offsetFromClosest);

  void setData(string fieldName, float azimuth, 
    int sweepNumber, vector<float> *data);

  void open(string fileName);

  //void newElevation(float elevation);
  //void changeAzEl(float azimuth, float elevation);
  void moveToLocation(string fieldName, int sweepNumber,
    float azimuth);
  void moveToLocation(string fieldName, int sweepNumber,
    float azimuth, float range);  

  SpreadSheetModel *getDataModel() {return _currentModel;};
  void volumeUpdated();

signals:
  void volumeChanged(); // const RadxVol &radarDataVolume);
  void selectSweep(int sweepNumber);

public slots:
  void setDataMissing(string fieldName, float missingDataValue);
  void needFieldNames();
  //void needDataForField(string fieldName, int r, int c);
  void needAzimuthForRay(int offsetFromClosest, int fieldIdx, string fieldName);
  void needNyquistVelocityForRay(int offsetFromClosest, int fieldIdx, string fieldName);
  void needRangeData(size_t nGates);
  void getVolumeChanges(string fieldName, float azimuth, 
    int sweepNumber, vector<float> *data);
  void switchRay(float azimuth, int sweepNumber);
  //void newSweepData(int sweepNumber);
  void displaySweepData(int sweepNumber);
  
private:

  SpreadSheetModel *_currentModel;
  SpreadSheetView *_currentView;
  RayLocationController *_rayLocationController;

};


#endif // SPREADSHEETCONTROLLER_H
