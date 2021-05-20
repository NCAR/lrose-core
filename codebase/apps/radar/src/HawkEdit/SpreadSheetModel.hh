

#ifndef SPREADSHEETMODEL_HH
#define SPREADSHEETMODEL_HH

#include <Radx/RadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>

class SpreadSheetModel
{

public:

  SpreadSheetModel();
  SpreadSheetModel(RadxRay *closestRay); // , RadxVol *dataVolume);
  
  //void initData(string fileName);
  void getRangeGeom(float *startRangeKm, float *gateSpacingKm);
    // TODO: maybe on construction, map to finest range geometry? then we can call 

  // return lists of data
  vector<float> *getData(string fieldName, int offsetFromClosest);
  vector<string> *getFields();
  float getAzimuthForRay(int offsetFromClosest);
  //  RadxVol getVolume(); 

  void setData(string fieldName, vector<float> *data);
  void findClosestRay(float azimuth, float elevation);

  //RadxVol *_vol;
  RadxRay *_closestRay;
  size_t _closestRayIdx;

  /*
  void _setupVolRead(RadxFile *file);
  int _getArchiveData(string inputPath);
  */
};

#endif // SPREADSHEETMODEL_H
