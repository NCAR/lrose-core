#include <iostream>
#include <QtWidgets>
#include <QChar>

#include "Dialog.hh"
#include "YourButton.hh"

using namespace std;

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{

    createRotatableGroupBox();
    createOptionsGroupBox();
    createButtonBox();

    mainLayout = new QGridLayout;
    mainLayout->addWidget(rotatableGroupBox, 0, 0);
    mainLayout->addWidget(optionsGroupBox, 1, 0);
    mainLayout->addWidget(buttonBox, 2, 0);
    setLayout(mainLayout);

    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

    setWindowTitle(tr("Dynamic Layouts"));

    initFieldDisplayList();


}

void Dialog::buttonsOrientationChanged(int index)
{
    mainLayout->setSizeConstraint(QLayout::SetNoConstraint);
    setMinimumSize(0, 0);

    Qt::Orientation orientation = Qt::Orientation(
            buttonsOrientationComboBox->itemData(index).toInt());

    if (orientation == buttonBox->orientation())
        return;

    mainLayout->removeWidget(buttonBox);

    int spacing = mainLayout->spacing();

    QSize oldSizeHint = buttonBox->sizeHint() + QSize(spacing, spacing);
    buttonBox->setOrientation(orientation);
    QSize newSizeHint = buttonBox->sizeHint() + QSize(spacing, spacing);

    if (orientation == Qt::Horizontal) {
        mainLayout->addWidget(buttonBox, 2, 0);
        resize(size() + QSize(-oldSizeHint.width(), newSizeHint.height()));
    } else {
        mainLayout->addWidget(buttonBox, 0, 3, 2, 1);
        resize(size() + QSize(newSizeHint.width(), -oldSizeHint.height()));
    }

    mainLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
}
/*
void Dialog::dealTheFields() {

  // TODO: just remove and move the affected widgets 

  vector <QWidget *> aRow;

  // remove the old widgets
  for (int r=0; r<getNRows(); r++) {
    aRow = fieldDisplays.at[r];
    for (int c=0; c<getNCols; c++) {
      rotatableLayout->addWidget(aRow.at[c], r, c);
    }
  }
    foreach (QWidget *widget, rotatableWidgets)
        rotatableLayout->removeWidget(widget);

  // insert the new widget order
  for (int r=0; r<getNRows(); r++) {
    aRow = fieldDisplays.at[r];
    for (int c=0; c<getNCols; c++) {
      rotatableLayout->addWidget(aRow.at[c], r, c);
    }
  }
}


void Dialog::shiftDown(unsigned int from) {

  if ((from >= maxRowsColumns) || (from < 0)) {
    QMessageBox::error(this, tr("Cannot insert row beyond boundaries"));
    return;
  }

  // TODO: just remove and move the affected widgets 

  vector <QWidget *> aRow;

  // remove the old widgets
  for (unsigned int r=from; r<getNRows(); r++) {
    aRow = fieldDisplays.at[r];
    for (int c=0; c<getNCols; c++) {
      rotatableLayout->removeWidget(aRow.at[c]);
    }
  }
    foreach (QWidget *widget, rotatableWidgets)
        rotatableLayout->removeWidget(widget);

  // insert the new widget order
  for (int r=0; r<getNRows(); r++) {
    aRow = fieldDisplays.at[r];
    for (int c=0; c<getNCols; c++) {
      rotatableLayout->addWidget(aRow.at[c], r, c);
    }
  }
}
*/

void Dialog::addAfter(unsigned int rowNum) {

  if (rowNum >= maxRowsColumns) {
    QMessageBox::warning(this, tr("Dynamic Layout Warning"),  
			 tr("Cannot insert row beyond boundaries"));
    //    QMessageBox::information(this, tr("Dynamic Layouts Help"),
    //                           tr("This example shows how to change layouts "
    //                              "dynamically."));


    return;
  }

  cerr << "adding row after " << rowNum << endl;

  // TODO: just remove and move the affected widgets 

  //vector <QWidget *> aRow;

  vector<FieldDisplay *>::iterator it;

  cerr << "removing affected widgets" << endl;

  for (it=fieldDisplayList.begin(); it != fieldDisplayList.end(); it++) {
    FieldDisplay *fieldDisplay = *it;
    unsigned int r = fieldDisplay->row;
    unsigned int c = fieldDisplay->col;
    QWidget *widget = fieldDisplay->widget;
    if (r > rowNum) {
      // move the widget 
      rotatableLayout->removeWidget(widget);
      fieldDisplay->row += 1;
      rotatableLayout->addWidget(widget, fieldDisplay->row, c);
    }
  }

  cerr << "adding new widget " << endl;

  // add the new widgets to the end of the list
  for (unsigned int c=0; c<currentNCols; c++) {
    QWidget *widget = new QLabel(QChar(lastNameUsed));
    lastNameUsed += 1;
    fieldDisplayList.push_back(new FieldDisplay(widget, rowNum, c));
    rotatableLayout->addWidget(widget, rowNum, c);    
  }

  currentNRows += 1;

  cerr << "number of rows " << currentNRows << endl << endl;
  /*
  // remove the old widgets
  for (unsigned int r=from; r<getNRows(); r++) {
    aRow = fieldDisplays.at[r];
    for (int c=0; c<getNCols; c++) {
      rotatableLayout->removeWidget(aRow.at[c]);
    }
  }

  // insert the new widgets into the store
  // insert the new widgets into the display

  

  for (int r=0; r<getNRows(); r++) {
    aRow = fieldDisplays.at[r];
    for (int c=0; c<getNCols; c++) {
      rotatableLayout->addWidget(aRow.at[c], r, c);
    }
  }
  */
}

void Dialog::addColumnAfter(unsigned int colNum) {

  cerr << "adding column after " << colNum << endl;


  if (colNum >= maxRowsColumns) {
    QMessageBox::warning(this, tr("Dynamic Layout Warning"),  
			 tr("Cannot insert column beyond boundaries"));
    //    QMessageBox::information(this, tr("Dynamic Layouts Help"),
    //                           tr("This example shows how to change layouts "
    //                              "dynamically."));


    return;
  }

  vector<FieldDisplay *>::iterator it;

  cerr << "removing affected widgets" << endl;

  for (it=fieldDisplayList.begin(); it != fieldDisplayList.end(); it++) {
    FieldDisplay *fieldDisplay = *it;
    unsigned int r = fieldDisplay->row;
    unsigned int c = fieldDisplay->col;
    QWidget *widget = fieldDisplay->widget;
    if (c > colNum) {
      // move the widget 
      rotatableLayout->removeWidget(widget);
      fieldDisplay->col += 1;
      rotatableLayout->addWidget(widget, r, fieldDisplay->col);
    }
  }

  cerr << "adding new widget " << endl;

  // add the new widgets to the end of the list
  for (unsigned int r=0; r<currentNRows; r++) {
    QWidget *widget = new QLabel(QChar(lastNameUsed));
    lastNameUsed += 1;
    fieldDisplayList.push_back(new FieldDisplay(widget, r, colNum));
    rotatableLayout->addWidget(widget, r, colNum);    
  }

  currentNCols += 1;

  cerr << "number of cols " << currentNCols << endl << endl;
}

/*
void Dialog::insertNewRow(int afterHere)
{
  int nRows = getNRows();

  // validate arguments
  if ((afterHere > nRows) || (afterHere < 0)i || (afterHere > maxRows)) {
    QMessageBox::error(this, tr("Cannot insert row beyond boundaries"),
  }

  // how many elements in each row?
  int nCols = getNCols();

  // create a new blank vector
  for (int i=0; i<nCols; i++) {
    newVector.pushBack(new QLabel());
  }

    foreach (QWidget *widget, rotatableWidgets)
        rotatableLayout->removeWidget(widget);

    rotatableWidgets.enqueue(rotatableWidgets.dequeue());

    const int n = rotatableWidgets.count();
    for (int i = 0; i < n / 2; ++i) {
        rotatableLayout->addWidget(rotatableWidgets[n - i - 1], 0, i);
        rotatableLayout->addWidget(rotatableWidgets[i], 1, i);
    }
}
*/

void Dialog::rotateWidgets()
{
  /*
    Q_ASSERT(rotatableWidgets.count() % 2 == 0);

    foreach (QWidget *widget, rotatableWidgets)
        rotatableLayout->removeWidget(widget);

    rotatableWidgets.enqueue(rotatableWidgets.dequeue());

    const int n = rotatableWidgets.count();
    for (int i = 0; i < n / 2; ++i) {
        rotatableLayout->addWidget(rotatableWidgets[n - i - 1], 0, i);
        rotatableLayout->addWidget(rotatableWidgets[i], 1, i);
    }
  */

  addAfter(currentNRows);
}

void Dialog::addColumnWidget()
{
  addColumnAfter(currentNCols);
}




void Dialog::initFieldDisplayList() {

  QLabel *widget = new QLabel(QChar(lastNameUsed));
  fieldDisplayList.push_back(new FieldDisplay(widget, 0, 0));
  lastNameUsed += 1;
  currentNRows = 1;
  currentNCols = 1;
  rotatableLayout->addWidget(widget, 0, 0);    

  // add initial widget
    //rotateWidgets();
    //currentNCols += 1;

}
void Dialog::help()
{
    QMessageBox::information(this, tr("Dynamic Layouts Help"),
                               tr("This example shows how to change layouts "
                                  "dynamically."));
}

void Dialog::createRotatableGroupBox()
{
    rotatableGroupBox = new QGroupBox(tr("Rotatable Widgets"));
    /*
    rotatableWidgets.enqueue(new QSpinBox);
    rotatableWidgets.enqueue(new QSlider);
    rotatableWidgets.enqueue(new QDial);
    rotatableWidgets.enqueue(new QProgressBar);

    int n = rotatableWidgets.count();
    for (int i = 0; i < n; ++i) {
        connect(rotatableWidgets[i], SIGNAL(valueChanged(int)),
                rotatableWidgets[(i + 1) % n], SLOT(setValue(int)));
    }
    */
    rotatableLayout = new QGridLayout;
    rotatableGroupBox->setLayout(rotatableLayout);

}

void Dialog::createOptionsGroupBox()
{
    optionsGroupBox = new QGroupBox(tr("Options"));

    buttonsOrientationLabel = new QLabel(tr("Orientation of buttons:"));

    buttonsOrientationComboBox = new QComboBox;
    buttonsOrientationComboBox->addItem(tr("Horizontal"), Qt::Horizontal);
    buttonsOrientationComboBox->addItem(tr("Vertical"), Qt::Vertical);

    connect(buttonsOrientationComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &Dialog::buttonsOrientationChanged);


    // add menu to button
  QMenu *menu = new QMenu(this);
  menu->addAction("Add Row");
  menu->addAction("Add Column");
  menu->addAction("Delete Row");
  menu->addAction("Delete Column");
 
  // hiddenContextPushButton = new HiddenContextMenu();
  //hiddenContextPushButton->installEventFilter(this);
  theButton = new QPushButton(QString(""));
  theButton->installEventFilter(this);
  //YourButton *yourButton = new YourButton();
  //connect(yourButton, &YourButton::focusInEvent(QFocusEvent *), this, &Dialog::revealButton());
  //connect(yourButton, &YourButton::focusOutEvent(QFocusEvent *), this, &Dialog::coverButton());

  //theButton->setFlat(true);
  theButton->setMenu(menu);
  // connect(hiddenContextPushButton, &HiddenContextMenu::hoverOverEvent(), // &cQPushButton::focusInEvent(QFocusEvent *),
	  // this, &Dialog::revealButton());
  // connect(hiddenContextPushButton, &HiddenContextMenu::hoverOffEvent(),  // &QPushButton::focusOutEvent(QFocusEvent *),
	  // this, &Dialog::coverButton());


    optionsLayout = new QGridLayout;
    optionsLayout->addWidget(buttonsOrientationLabel, 0, 0);
    optionsLayout->addWidget(buttonsOrientationComboBox, 0, 1);
 
    //optionsLayout->addWidget(hiddenContextPushButton);
    optionsLayout->addWidget(theButton);

    optionsLayout->setColumnStretch(2, 1);
    optionsGroupBox->setLayout(optionsLayout);
}

void Dialog::revealButton() 
{
  hiddenContextPushButton->setVisible(true);

}

void Dialog::coverButton() 
{
  hiddenContextPushButton->setVisible(false);

}

void Dialog::createButtonBox()
{
    buttonBox = new QDialogButtonBox;

    closeButton = buttonBox->addButton(QDialogButtonBox::Close);
    helpButton = buttonBox->addButton(QDialogButtonBox::Help);
    rotateWidgetsButton = buttonBox->addButton(tr("Add Row"),
                                               QDialogButtonBox::ActionRole);

    connect(rotateWidgetsButton, &QPushButton::clicked, this, &Dialog::rotateWidgets);

    addColumnButton = buttonBox->addButton(tr("Add Col"),
                                               QDialogButtonBox::ActionRole);

    connect(addColumnButton, &QPushButton::clicked, this, &Dialog::addColumnWidget);

    connect(closeButton, &QPushButton::clicked, this, &Dialog::close);
    connect(helpButton, &QPushButton::clicked, this, &Dialog::help);


}

bool Dialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == toolButton) {
            cerr << "toolButton Event is " << event->type() << endl;;
	    return true;
    }
 
    if (obj == theButton) { //  && (event->type() == QEvent::Enter)) { // hiddenContextPushButton) {
       cerr << "theButton Event is " << event->type();
       if (event->type() == QEvent::Enter) {
            cerr << " Enter " << endl;;
            //theButton->setVisible(true);
            // signal event to pop up context menu

            return true;
       } 
       if (event->type() == QEvent::Leave) {
            cerr << " Leave " << endl;;
            //theButton->setVisible(false);
            return true;
       } 
       if (event->type() == QEvent::MouseButtonRelease) {
            // catch this event for selecting the row or column
            cerr << " MouseButtonRelease " << endl;;
            //theButton->setVisible(false);
            return true;
       } 
       cerr << endl;
       return false;
    } else {
        // pass the event on to the parent class
        return QDialog::eventFilter(obj, event);
    }
}
