

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
  float getNyquistVelocityForRay(int offsetFromClosest);
  //  RadxVol getVolume(); 



  void setData(string fieldName, float azimuth, vector<float> *data);
  void setDataMissing(string fieldName, float missingDataValue);
  void setClosestRay(float azimuth, int sweepNumber);


private:


  size_t _getRayIdx(int offsetFromClosest);
  //void _getSweepNumber(float elevation);
  void _setSweepNumber(int sweepNumber);

  //RadxVol *_vol;
  RadxRay *_closestRay;
  size_t _closestRayIdx;
  float _currentSweepNumber;  

  /*
  void _setupVolRead(RadxFile *file);
  int _getArchiveData(string inputPath);
  */
};

#endif // SPREADSHEETMODEL_H
