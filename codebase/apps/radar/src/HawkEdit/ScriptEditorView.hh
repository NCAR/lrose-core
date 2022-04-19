
#ifndef SCRIPTEDITORVIEW_HH
#define SCRIPTEDITORVIEW_HH

#include <QMainWindow>
#include "TextEdit.hh"
#include "ScriptEditorController.hh"
//#include "ScriptEditorDelegate.hh"
#include "SoloFunctionsController.hh"

//#include "PolarManager.hh"

#include <QWidget>
#include <QAction>
#include <QLabel>
#include <QTextEdit>
#include <QToolBar>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QTreeView>
#include <QString>
// #include <QJSEngine>

class ScriptEditorView : public QMainWindow
{
    Q_OBJECT

public:

  //  ScriptEditorView(std::string fileName, QWidget *parent = 0);
  ScriptEditorView(QWidget *parent = 0);
  virtual ~ScriptEditorView();

  //  void setController(ScriptEditorController *controller);

  void init();

  void newDataReady();


  float myPow();

  vector<string> *getVariablesFromScriptEditor();
  vector<float> *getDataForVariableFromScriptEditor(int column, string fieldName);

  string getSaveEditsDirectory();

  void initProgress(int nFiles);
  void updateProgress(int currentIndex, int lastIndex);
  void batchEditComplete();

  //void setSelectionToValue(QString value);
  void closeEvent();

public slots:
    void updateStatus(QTableWidgetItem *item);
    void updateTextEdit(QTableWidgetItem *item);
    void returnPressed();
    void acceptFormulaInput();
    void cancelFormulaInput();
    void displayHelp();
    void scriptComplete();
    void cancelScriptRun();
    //void undoEdits();
    //void redoEdits();
    //void clear();

    void notImplementedMessage();

    //void setupSoloFunctions(SoloFunctions *soloFunctions);

    void fieldNamesProvided(vector<string> fieldNames);
    //void fieldDataSent(vector<float> *data, int useless, int c);

    void applyChanges();
    void currentSweepClicked(bool checked);
    //void allSweepsClicked(bool checked);
    //void timeRangeClicked(bool checked);
    void changeOutputLocation(bool checked);


  //void hideTimeRangeEdits();
  //void showTimeRangeEdits();

  //  void printQJSEngineContext();

signals:

  void needFieldNames();
  //void needDataForField(string fieldName, int r, int c);
  void applyVolumeEdits();
  void runOneTimeOnlyScript(QString oneTimeOnlyScript);
  void runForEachRayScript(QString forEachRayScript, bool useBoundary,
    bool useAllSweeps);
  void runScriptBatchMode(QString script, bool useBoundary, 
    bool useAllSweeps, bool useTimeRange);
  void cancelScriptRunRequest();
  void undoScriptEdits();
  void redoScriptEdits();  
  void scriptEditorClosed();

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
  void scriptCompleteMessage();

  void openScriptFile();
  void importScriptFile();
  void saveScriptFile();
  void saveEditDirectory();



private:

  //ScriptEditorController *_controller;
    vector<std::string> _fieldNames;

    QToolBar *toolBar;

    QLabel *cellLabel;
    TextEdit *formulaInput;
    TextEdit *formulaInputForEachRay;
    QPushButton *useBoundaryWidget;
    //QRadioButton *applyToCurrentSweep;
    //QRadioButton *applyToAllSweeps;
    QTreeView *helpView;
    QHBoxLayout *scriptEditLayout;

    //QPushButton *currentSweepToggleButton;
    QPushButton *allSweepsToggleButton;
    QGroupBox *scriptModifiers;

    //QPushButton *currentTimeToggleButton;
    //QPushButton *timeRangeToggleButton;
    QDateTimeEdit *_archiveStartTimeEdit;
    QDateTimeEdit *_archiveEndTimeEdit;
    QLabel *saveEditsDirectory;
    QPushButton *browseDirectoryButton;

    QVBoxLayout *checkBoxLayout;
    QWidget *scriptEditWidget;

    QVBoxLayout *helpViewLayout;
    QWidget *helpWidget;

    //QTextEdit *formulaInput;
  // ScriptEditorDelegate *formulaInput;

    QProgressBar *progressBar;
};

#endif // SCRIPTEDITORVIEW_H
