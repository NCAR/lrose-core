
#ifndef SPREADSHEETVIEW_HH
#define SPREADSHEETVIEW_HH

#include <QMainWindow>
#include "TextEdit.hh"
#include "SpreadSheetUtils.hh"
#include "SpreadSheetController.hh"
#include "SpreadSheetDelegate.hh"
//#include "CustomTableHeader.hh"

#include <vector>

#include <QWidget>
#include <QAction>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QString>
#include <QUndoStack>
#include <QUndoView>

using namespace std;

class SpreadSheetView : public QMainWindow
{
    Q_OBJECT

public:

  //  SpreadSheetView(std::string fileName, QWidget *parent = 0);
  SpreadSheetView(QWidget *parent = 0, float rayAzimuth = 0.0,
    int sweepNumber = 0);

  //  void setController(SpreadSheetController *controller);

  void init();

  void newDataReady();

  int getSweepNumber();
  float getAzimuth();
  void setAzimuth(float azimuth);

  float myPow();

  vector<string> *getVariablesFromSpreadSheet();
  vector<float> *getDataForVariableFromSpreadSheet(int column); // , string fieldName);

  void setSelectionToValue(QString value);

  void highlightClickedData(string fieldName, float azimuth,
    int sweepNumber, float range);

  void closeEvent();

  
  int getNFieldsToDisplay() { return _nFieldsToDisplay; };
  int getNRaysToDisplay() { return _nRays; };
  //string data_format = "%g";
  float getMissingDataValue() { return _missingDataValue; };
  vector<std::string> getFieldNamesToDisplay() { return _fieldNames; };


public slots:
    void updateStatus(QTableWidgetItem *item);
    void updateColor(QTableWidgetItem *item);
    //void updateTextEdit(QTableWidgetItem *item);
    void returnPressed();
    //void acceptFormulaInput();
    void cancelFormulaInput();
    void clear();
    void showAbout();

    void print();
    QString open();

  void actionDisplayCellValues();
  void actionDisplayRayInfo();
  void actionDisplayMetadata();
  void actionDisplayEditHist();
  void deleteRay();
  void deleteSelection();
  void subtractNyquistFromRay();
  void addNyquistFromRay();
  void subtractNyquistFromSelection();
  void addNyquistToSelection();
  void subtractNyquistFromSelectionToEnd();
  void addNyquistFromSelectionToEnd();
  void adjustNyquistFromRay(float factor, int top);
  void adjustNyquistFromSelection(float factor);
  void adjustNyquistFromSelectionToEnd(float factor);
  void adjustNyquistGeneral(float factor, bool fromSelection, int startRow);

  void setRangeToAllSelectedColumns(int startRow, int fromSelection);

  void notImplementedMessage();


  //  void setupSoloFunctions(SoloFunctions *soloFunctions);

  void fieldNamesSelected(vector<string> fieldNames);
  void fieldNamesProvided(vector<string> *fieldNames);
  void fieldDataSent(vector<float> *data, int useless, int c);
  void azimuthForRaySent(float azimuth, int offsetFromClosestRay,
    int fieldIdx, string fieldName);
  void nyquistVelocitySent(float nyquistVelocity, int offsetFromClosestRay,
    int fieldIdx, string fieldName); 
  void setHeader(int baseColumn, int fieldIdx, float azimuth,
    string fieldName);

  void applyChanges();
  void applyEdits();
  void changeAzEl(float azimuth, int sweepNumber);
  void changeMissingValue(float currentMissingValue);
  void updateLocationInVolume(float azimuth, int sweepNumber);
  void setTheWindowTitle(float rayAzimuth, int sweepNumber);

  void rangeDataSent(size_t nGates, float startingKm, float gateSize);

  void columnHeaderClicked(int index);

  void updateNavigation(string fieldName, float azimuth, int sweepNumber);

signals:

  void needFieldNames();
  void needDataForField(string fieldName, int r, int c);
  void needAzimuthForRay(int offsetFromClosestRay, int fieldIdx, string fieldName);
  void needNyquistVelocityForRay(int rayIdx, int fieldIdx, string fieldName);
  void applyVolumeEdits(string fieldName, float rayAzimuth, int sweepNumber, vector<float> *data);
  void signalRayAzimuthChange(float rayAzimuth, int sweepNumber);
  void needRangeData(size_t nPoints);
  void setDataMissing(string fieldName, float missingDataValue);
  void replotRequested();
  void spreadSheetClosed();
  void dataChanged();

protected:
    void setupContextMenu();
    void setupContents();
    void setupMenuBar();
    void createActions();
    //void addVariableToSpreadSheet(QString name, QJSValue value);

    bool runFunctionDialog();

  void criticalMessage(std::string message);

  string getFieldName(QString text);
  float getAzimuth(QString text);
  void replot();

  bool isMissing(QString textValue);

private:

  //SpreadSheetController *_controller;
    vector<std::string> _fieldNames;
    //float _currentAzimuth;
    //float _currentElevation;

    int _nFieldsToDisplay;
    int _nRays;
    string data_format = "%g";
    float _missingDataValue;
    string _missingDataString = "--";


    QToolBar *toolBar;
    QAction *colorAction;
    QAction *fontAction;
    QAction *firstSeparator;
    QAction *cell_deleteAction;
    QAction *cell_negFoldAction;
    QAction *cell_plusFoldAction;
    QAction *cell_deleteRayAction;
    QAction *cell_negFoldRayAction;
    QAction *cell_plusFoldRayAction;
    QAction *cell_negFoldRayGreaterAction;
    QAction *cell_plusFoldRayGreaterAction;
    QAction *cell_plusFoldRangeAction;
    QAction *cell_zapGndSpdAction;

    QAction *secondSeparator;
    QAction *clearAction;
    QAction *aboutSpreadSheet;
    QAction *exitAction;
    QAction *openAction;
    QAction *undoAction = nullptr;
    QAction *redoAction = nullptr;

  QAction *display_cellValuesAction;
  QAction *display_rayInfoAction;
  QAction *display_metadataAction;
  QAction *display_editHistAction;

  QPushButton *applyEditsButton;
  QPushButton *refreshButton;

  QLineEdit *rayLineEdit;
  QLineEdit *sweepLineEdit;
  QLineEdit *raysLineEdit;
  QLineEdit *missingDataValueLineEdit;
  QLabel *rangeLineEdit;

    QAction *printAction;
    QAction *replotAction;
    QAction *applyEditsAction;
    QAction *clearEditsAction;

    QLabel *nyquistVelocityLabel;

    QLabel *cellLabel;
    QTableWidget *table;
    //CustomTableHeader *h;

    TextEdit *formulaInput;
    //QTextEdit *formulaInput;
  // SpreadSheetDelegate *formulaInput;

    QListWidget *fieldListWidget;
    QUndoStack *undoStack = nullptr;
    QUndoView *undoView = nullptr;

    bool _unAppliedEdits = false;

    float _startGateKm = 0.0;

  
  //  const char *LogFileName = "/tmp/HawkEye_log.txt";
 
const char *htmlText =
"<HTML>"
"<p><b>"
"Help info .."
"<ul>"
"<li>Doing something.</li>"
"<li>number of cells.</li>"
  "</HTML>";
};



#endif // SPREADSHEETVIEW_H
