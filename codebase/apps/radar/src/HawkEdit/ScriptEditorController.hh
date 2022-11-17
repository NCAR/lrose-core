

#ifndef SCRIPTEDITORCONTROLLER_HH
#define SCRIPTEDITORCONTROLLER_HH

#include <QObject>
#include <QJSEngine>
#include <QStringList>
#include "ScriptEditorView.hh"
#include "ScriptEditorModel.hh"
#include "SoloFunctionsModel.hh"
//#include "PolarWidget.hh"
//#include <Radx/RadxVol.hh>
#include "SoloFunctionsController.hh"

#include <vector>

class ScriptEditorView;

using namespace std;

class ScriptEditorController : public QObject
{

  Q_OBJECT

public:

  ScriptEditorController(ScriptEditorView *view);
  ScriptEditorController(ScriptEditorView *view, ScriptEditorModel *model);

  vector<string> *getFieldNames();
  // vector<float> *getData(string fieldName);
  //void setData(string fieldName, vector<float> *data);

  void openData(string fileName);
  void writeData(string &path);

  ScriptEditorModel *getDataModel() {return _currentModel;};
  void volumeUpdated(QStringList newFieldNames);

  void printQJSEngineContext();
  void addVariableToScriptEditor(QString name, QJSValue value);

  void initProgress(int nFiles);
  void updateProgress(int currentIndex, int lastIndex);
  void batchEditComplete();



signals:
  void scriptChangedVolume(QStringList newFieldNames); // const RadxVol &radarDataVolume);
  void scriptComplete();

public slots:
  void needFieldNames();
  // void needDataForField(string fieldName, int r, int c);
  void getVolumeChanges();
  bool notDefined(QString &fieldName, std::map<QString, QString> &previousVariableContext);
  void runOneTimeOnlyScript(QString script);
  void runForEachRayScript(QString script, bool useBoundary, vector<Point> &boundaryPoints,
    string dataFileName, bool updateVolume);  
  void runForEachRayScript(QString script, int currentSweepNumber,
    bool useBoundary, vector<Point> &boundaryPoints,
    string dataFileName, bool updateVolume); 
  void runForEachRayScriptOLD(QString script, int currentSweepIndex,
    bool useBoundary, vector<Point> &boundaryPoints, string dataFileName);

  void runMultipleArchiveFiles(vector<string> &archiveFiles, 
    QString script, bool useBoundary,
    vector<Point> &boundaryPoints, string saveDirectoryPath,
    vector<string> &fieldNames, bool debug_verbose, bool debug_extra);

private:


  ScriptEditorModel *_currentModel;
  ScriptEditorView *_currentView;
  SoloFunctionsController *_soloFunctionsController;

  ScriptsDataController *_scriptsDataController;

  QJSEngine *engine;

  vector<string> *initialFieldNames;

  bool _cancelPressed;

  void reset();
  void _resetDataFile(string &dataFileName, bool debug_verbose, bool debug_extra);
  void _resetDataFile(string &dataFileName, bool debug_verbose, bool debug_extra,
    vector<string> &fieldNamesInScript);
  void _resetDataFile(string &dataFileName, int sweepNumber, bool debug_verbose, bool debug_extra,
    vector<string> &fieldNamesInScript);

  void setupBoundaryArray();
  void setupFieldArrays();
  void saveFieldArrays(std::map<QString, QString> &previousVariableContext);
  void saveFieldVariableAssignments(std::map<QString, QString> &previousVariableContext);
  QStringList *findNewFieldNames(std::map<QString, QString> &previousVariableContext);
  void setupSoloFunctions(SoloFunctionsController *soloFunctions);
  void fieldNamesProvided(vector<string> *fieldNames);
  void _assign(string tempName, string userDefinedName);
  void _assign(string tempName, string userDefinedName,
    int sweepIndex);
  void _assignByRay(string tempName, string userDefinedName);

  void _copy(string tempName, string userDefinedName);
  void _copy(string tempName, string userDefinedName,
    int sweepIndex);
  
  void regularizeRays();
  //void _addFieldNameVectorsToContext(vector<string> &fieldNames, 
  //  std::map<QString, QString> *currentVariableContext);
  vector<bool> *getListOfFieldsReferencedInScript(
    vector<string> &fields, string script);
  string lowerIt(string s);

};


#endif // SCRIPTEDITORCONTROLLER_H
