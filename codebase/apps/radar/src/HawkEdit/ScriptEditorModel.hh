

#ifndef SCRIPTEDITORMODEL_HH
#define SCRIPTEDITORMODEL_HH

#include <Radx/RadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>

class ScriptEditorModel
{

public:

  ScriptEditorModel();
  //ScriptEditorModel(RadxVol *dataVolume);
  
  // Hmm, not sure what needs to be in the model ...
  // maybe just a link/pointer to the current radar volume?
  // Oh, eventually, the script file, info?
  // 
  // return lists of data
  //vector<float> *getData(string fieldName);
  //vector<float> getSampleData();
  //vector<string> *getFields();
  //  RadxVol getVolume(); 

  //void setData(string fieldName, vector<float> *data);
  //RadxVol *_vol;

};

#endif // SCRIPTEDITORMODEL_H
