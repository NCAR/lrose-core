
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
#include <QJSEngine>

using namespace std;

class SpreadSheetView : public QMainWindow
{
    Q_OBJECT

public:

  //  SpreadSheetView(std::string fileName, QWidget *parent = 0);
  SpreadSheetView(QWidget *parent = 0, float rayAzimuth = 0.0);

  //  void setController(SpreadSheetController *controller);

  void init();

  void newDataReady();


  float myPow();

  vector<string> *getVariablesFromSpreadSheet();
  vector<float> *getDataForVariableFromSpreadSheet(int column, string fieldName);

  void setSelectionToValue(QString value);

  void highlightClickedData(string fieldName, float azimuth, float range);

  void closeEvent();


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
  void deleteField();

  void notImplementedMessage();


  //  void setupSoloFunctions(SoloFunctions *soloFunctions);

  void fieldNamesSelected(vector<string> fieldNames);
  void fieldNamesProvided(vector<string> *fieldNames);
  void fieldDataSent(vector<float> *data, int useless, int c);
  void azimuthForRaySent(float azimuth, int offsetFromClosestRay,
    int fieldIdx, string fieldName);
  void setHeader(int baseColumn, int fieldIdx, float azimuth,
    string fieldName);

  void applyChanges();
  void applyEdits();
  void changeAzEl(float azimuth, float elevation);
  void changeMissingValue(float currentMissingValue);
  void newElevation(float elevation);
  void newAzimuth(float azimuth);
  void setTheWindowTitle(float rayAzimuth);

  void rangeDataSent(size_t nGates, float startingKm, float gateSize);

  void printQJSEngineContext();

signals:

  void needFieldNames();
  void needDataForField(string fieldName, int r, int c);
  void needAzimuthForRay(int offsetFromClosestRay, int fieldIdx, string fieldName);
  void applyVolumeEdits();
  void signalRayAzimuthChange(float rayAzimuth, float elevation);
  void needRangeData(size_t nPoints);
  void setDataMissing(string fieldName, float missingDataValue);
  void spreadSheetClosed();

protected:
    void setupContextMenu();
    void setupContents();
    void setupMenuBar();
    void createActions();
    //void addVariableToSpreadSheet(QString name, QJSValue value);



    void actionMath_helper(const QString &title, const QString &op);
    bool runFunctionDialog();

    bool runInputDialog(const QString &title,
                        const QString &c1Text,
                        const QString &c2Text,
                        const QString &opText,
                        const QString &outText,
                        QString *cell1, QString *cell2, QString *outCell);

  void criticalMessage(std::string message);

private:

  //SpreadSheetController *_controller;
    vector<std::string> _fieldNames;
    //float _currentAzimuth;
    float _currentElevation;

    int _nFieldsToDisplay;
    int _nRays;
    string data_format = "%g";
    float _missingDataValue;


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
    QAction *cell_zapGndSpdAction;

    QAction *secondSeparator;
    QAction *clearAction;
    QAction *aboutSpreadSheet;
    QAction *exitAction;
    QAction *openAction;

  QAction *display_cellValuesAction;
  QAction *display_rayInfoAction;
  QAction *display_metadataAction;
  QAction *display_editHistAction;

  QPushButton *applyEditsButton;
  QLineEdit *rayLineEdit;
  QLineEdit *raysLineEdit;
  QLineEdit *missingDataValueLineEdit;

    QAction *printAction;

    QLabel *cellLabel;
    QTableWidget *table;
    //CustomTableHeader *h;

    TextEdit *formulaInput;
    //QTextEdit *formulaInput;
  // SpreadSheetDelegate *formulaInput;

    QListWidget *fieldListWidget;

  QJSEngine engine;

  
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



#endif // SPREADSHEETVIEW_H
