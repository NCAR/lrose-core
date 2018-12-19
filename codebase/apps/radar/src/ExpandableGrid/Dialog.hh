#ifndef DIALOG_H
#define DIALOG_H

#include <vector>

#include <QDialog>
#include <QQueue>
#include "FieldDisplay.hh"
#include "HiddenContextMenu.hh"

class QComboBox;
class QDialogButtonBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QPushButton;
class QToolButton;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);

    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void buttonsOrientationChanged(int index);
    void rotateWidgets();
    void help();

private:
    void createRotatableGroupBox();
    void createOptionsGroupBox();
    void createButtonBox();
    void initFieldDisplayList();
    void addAfter(unsigned int rowNum);
    void addColumnAfter(unsigned int colNum);
    void addColumnWidget();
    void revealButton();
    void coverButton();

    QGroupBox *rotatableGroupBox;
    QQueue<QWidget *> rotatableWidgets;

    // the field displays must always be square or rectangular
  std::vector<FieldDisplay *> fieldDisplayList;

    QGroupBox *optionsGroupBox;
    QLabel *buttonsOrientationLabel;
    QComboBox *buttonsOrientationComboBox;

    QDialogButtonBox *buttonBox;
    QPushButton *closeButton;
    QPushButton *helpButton;
    QPushButton *rotateWidgetsButton;
    QPushButton *addColumnButton;
    HiddenContextMenu *hiddenContextPushButton;
    QPushButton *theButton;
    QToolButton *toolButton;

    QGridLayout *mainLayout;
    QGridLayout *rotatableLayout;
    QGridLayout *optionsLayout;

    char lastNameUsed = 'A';

    const unsigned int maxRowsColumns = 6;
    //vector <unsigned int> currentOrderRows;
    //vector <unsigned int> currentOrderCols;
    unsigned int currentNRows;
    unsigned int currentNCols;
};

#endif // DIALOG_H
