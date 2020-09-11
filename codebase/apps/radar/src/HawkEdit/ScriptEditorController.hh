

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

  vector<string> getFieldNames();
  // vector<float> *getData(string fieldName);
  //void setData(string fieldName, vector<float> *data);

  void open(string fileName);

  ScriptEditorModel *getDataModel() {return _currentModel;};
  void volumeUpdated(QStringList newFieldNames);

  void printQJSEngineContext();
  void addVariableToScriptEditor(QString name, QJSValue value);

signals:
  void scriptChangedVolume(QStringList newFieldNames); // const RadxVol &radarDataVolume);

public slots:
  void needFieldNames();
  // void needDataForField(string fieldName, int r, int c);
  void getVolumeChanges();
  bool notDefined(QString &fieldName, std::map<QString, QString> &previousVariableContext);
  void runOneTimeOnlyScript(QString script);
  void runForEachRayScript(QString script, bool useBoundary);
private:


  ScriptEditorModel *_currentModel;
  ScriptEditorView *_currentView;
  SoloFunctionsController *_soloFunctionsController;

  QJSEngine engine;

  vector<string> initialFieldNames;

  void setupBoundaryArray();
  void setupFieldArrays();
  void saveFieldArrays(std::map<QString, QString> &previousVariableContext);
  void setupSoloFunctions(SoloFunctionsController *soloFunctions);
  void fieldNamesProvided(vector<string> fieldNames);
  void _assign(string tempName, string userDefinedName);
  void _addFieldNameVectorsToContext(vector<string> &fieldNames, 
    std::map<QString, QString> *currentVariableContext);
};


#endif // SCRIPTEDITORCONTROLLER_H
