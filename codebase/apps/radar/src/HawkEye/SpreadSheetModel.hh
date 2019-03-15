

#ifndef SPREADSHEETMODEL_HH
#define SPREADSHEETMODEL_HH

#include <Radx/RadxFile.hh>
#include <Radx/RadxVol.hh>

class SpreadSheetModel
{

public:

  SpreadSheetModel();

  void initData(string fileName);

  // return lists of data
  vector<double> getData(string fieldName);
  vector<float> getSampleData();
  vector<string> getFields();

  RadxVol _vol;

  void _setupVolRead(RadxFile *file);
  int _getArchiveData(string inputPath);
};

#endif // SPREADSHEETMODEL_H
