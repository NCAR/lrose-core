
#ifndef SPREADSHEETVIEW_HH
#define SPREADSHEETVIEW_HH

#include <QMainWindow>
#include "TextEdit.hh"
#include "SpreadSheetUtils.hh"
#include "SpreadSheetController.hh"
#include "SpreadSheetDelegate.hh"

#include <vector>

#include <QWidget>
#include <QAction>
#include <QLabel>
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


public slots:
    void updateStatus(QTableWidgetItem *item);
    void updateColor(QTableWidgetItem *item);
    void updateTextEdit(QTableWidgetItem *item);
    void returnPressed();
    void acceptFormulaInput();
    void cancelFormulaInput();
    void selectColor();
    void selectFont();
    void clear();
    void showAbout();

    void print();
    QString open();

    void actionSum();
    void actionSubtract();
    void actionAdd();
    void actionMultiply();
    void actionDivide();

  void actionDisplayCellValues();
  void actionDisplayRayInfo();
  void actionDisplayMetadata();
  void actionDisplayEditHist();

  void notImplementedMessage();


  //  void setupSoloFunctions(SoloFunctions *soloFunctions);

  void fieldNamesProvided(vector<string> fieldNames);
  void fieldDataSent(vector<float> *data, int useless, int c);

  void applyChanges();

  void printQJSEngineContext();

signals:

  void needFieldNames();
  void needDataForField(string fieldName, int r, int c);
  void applyVolumeEdits();

protected:
    void setupContextMenu();
    void setupContents();
    void setupMenuBar();
    void createActions();
    void addVariableToSpreadSheet(QString name, QJSValue value);



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

    QToolBar *toolBar;
    QAction *colorAction;
    QAction *fontAction;
    QAction *firstSeparator;
    QAction *cell_sumAction;
    QAction *cell_addAction;
    QAction *cell_subAction;
    QAction *cell_mulAction;
    QAction *cell_divAction;
    QAction *secondSeparator;
    QAction *clearAction;
    QAction *aboutSpreadSheet;
    QAction *exitAction;
    QAction *openAction;

  QAction *display_cellValuesAction;
  QAction *display_rayInfoAction;
  QAction *display_metadataAction;
  QAction *display_editHistAction;

    QAction *printAction;

    QLabel *cellLabel;
    QTableWidget *table;
    TextEdit *formulaInput;
    //QTextEdit *formulaInput;
  // SpreadSheetDelegate *formulaInput;

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
