

#ifndef SPREADSHEETMODEL_HH
#define SPREADSHEETMODEL_HH

#include <Radx/RadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>

class SpreadSheetModel
{

public:

  SpreadSheetModel();
  SpreadSheetModel(RadxRay *closestRay, RadxVol dataVolume);
  
  //void initData(string fileName);

  // return lists of data
  vector<float> *getData(string fieldName);
  vector<float> getSampleData();
  vector<string> getFields();
  RadxVol getVolume(); 

  void setData(string fieldName, vector<double> *data);
  RadxVol _vol;
  RadxRay *_closestRay;

  /*
  void _setupVolRead(RadxFile *file);
  int _getArchiveData(string inputPath);
  */
};

#endif // SPREADSHEETMODEL_H
