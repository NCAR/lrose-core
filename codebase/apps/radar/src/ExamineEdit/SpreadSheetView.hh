

#ifndef SPREADSHEETVIEW_HH
#define SPREADSHEETVIEW_HH

#include <QMainWindow>
#include "SpreadSheetUtils.hh"
//#include "SpreadSheetController.hh"

#include <QWidget>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QToolBar>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QString>

class SpreadSheetView : public QMainWindow
{
    Q_OBJECT

public:

  SpreadSheetView(std::string fileName, QWidget *parent = 0);
  SpreadSheetView(QWidget *parent = 0);

  void newDataReady();

public slots:
    void updateStatus(QTableWidgetItem *item);
    void updateColor(QTableWidgetItem *item);
    void updateLineEdit(QTableWidgetItem *item);
    void returnPressed();
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

protected:
    void setupContextMenu();
    void setupContents();

    void setupMenuBar();
    void createActions();

    void actionMath_helper(const QString &title, const QString &op);
    bool runInputDialog(const QString &title,
                        const QString &c1Text,
                        const QString &c2Text,
                        const QString &opText,
                        const QString &outText,
                        QString *cell1, QString *cell2, QString *outCell);
private:


  //  SpreadSheetController _controller;


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

    QAction *printAction;

    QLabel *cellLabel;
    QTableWidget *table;
    QLineEdit *formulaInput;


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
