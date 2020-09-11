
#ifndef SCRIPTEDITORVIEW_HH
#define SCRIPTEDITORVIEW_HH

#include <QMainWindow>
#include "TextEdit.hh"
#include "ScriptEditorController.hh"
//#include "ScriptEditorDelegate.hh"
#include "SoloFunctionsController.hh"

#include <QWidget>
#include <QAction>
#include <QLabel>
#include <QTextEdit>
#include <QToolBar>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QString>
// #include <QJSEngine>

class ScriptEditorView : public QMainWindow
{
    Q_OBJECT

public:

  //  ScriptEditorView(std::string fileName, QWidget *parent = 0);
  ScriptEditorView(QWidget *parent = 0);

  //  void setController(ScriptEditorController *controller);

  void init();

  void newDataReady();


  float myPow();

  vector<string> *getVariablesFromScriptEditor();
  vector<float> *getDataForVariableFromScriptEditor(int column, string fieldName);

  //void setSelectionToValue(QString value);


public slots:
    void updateStatus(QTableWidgetItem *item);
    void updateTextEdit(QTableWidgetItem *item);
    void returnPressed();
    void acceptFormulaInput();
    void cancelFormulaInput();
  //void clear();

  void notImplementedMessage();

  //void setupSoloFunctions(SoloFunctions *soloFunctions);

  void fieldNamesProvided(vector<string> fieldNames);
  //void fieldDataSent(vector<float> *data, int useless, int c);

  void applyChanges();

  //  void printQJSEngineContext();

signals:

  void needFieldNames();
  //void needDataForField(string fieldName, int r, int c);
  void applyVolumeEdits();
  void runOneTimeOnlyScript(QString oneTimeOnlyScript);
  void runForEachRayScript(QString forEachRayScript, bool useBoundary);

protected:
    void setupContextMenu();
    void setupContents();
    void setupMenuBar();
    void createActions();
  //    void addVariableToScriptEditor(QString name, QJSValue value);

  /*
    bool runInputDialog(const QString &title,
                        const QString &c1Text,
                        const QString &c2Text,
                        const QString &opText,
                        const QString &outText,
                        QString *cell1, QString *cell2, QString *outCell);
  */
  void criticalMessage(std::string message);

  void openScriptFile();
  void saveScriptFile();

private:

  //ScriptEditorController *_controller;
    vector<std::string> _fieldNames;

    QToolBar *toolBar;

    QLabel *cellLabel;
    TextEdit *formulaInput;
    TextEdit *formulaInputForEachRay;
    QCheckBox *useBoundaryWidget;
    //QTextEdit *formulaInput;
  // ScriptEditorDelegate *formulaInput;

  //  QJSEngine engine;

  
  //  const char *LogFileName = "/tmp/HawkEye_log.txt";
 
const char *htmlText =
"<HTML>"
"<p><b>"
"Some useful info .."
"<ul>"
"<li>Adding two cells.</li>"
"<li>Subtracting one cell from another.</li>"
"<li>Multiplying two cells.</li>"
"<li>Dividing one cell with another.</li>"
"<li>Summing the contents of an arbitrary number of cells.</li>"
  "</HTML>";
};



#endif // SCRIPTEDITORVIEW_H
