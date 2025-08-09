
#include <stdio.h>
#include <string.h>
#include <QtWidgets>
#include <QMessageBox>
#include <QModelIndex>
#include <QJSEngine>
#include <QJSValue>
#include <QJSValueIterator>
#include <QStringList>
#include <vector>
#include <iostream>
#include <toolsa/LogStream.hh>
//#include "TextEdit.hh"
#include "SpreadSheetView.hh"
#include "SpreadSheetDelegate.hh"
#include "SpreadSheetItem.hh"
//#include "SoloFunctions.hh"
#include "DataField.hh"
//#include "CustomTableHeader.hh"


using namespace std;

Q_DECLARE_METATYPE(QVector<int>)
Q_DECLARE_METATYPE(QVector<double>)


// SpreadSheetView will emit signals that are followed by the controller
//
//

// Qt::WindowMinMaxButtonsHint

// Create a Dialog for the spreadsheet settings
// Create a separate window that contains the spreadsheet 

  SpreadSheetView::SpreadSheetView(QWidget *parent, 
    float rayAzimuth, int sweepNumber)
  : QMainWindow(parent)
{
  LOG(DEBUG) << "in SpreadSheetView constructor";
  //  initSpreadSheet();
  int rows;
  int cols;

  // _controller = new SpreadSheetController(this);
  //_controller->open(fileName);
  //vector<std::string> fieldNames = _controller->getFieldNames();
  //cols = displayInfo.getNumFields();
  // vector<std::string> fieldNames = vol.getUniqueFieldNameList();
  cols = 3; // (int) fieldNames.size();
  rows = 200;

  _nFieldsToDisplay = 1;
  _nRays = 1;

  //_volumeData = vol;

    addToolBar(toolBar = new QToolBar());
    //formulaInput = new TextEdit(this);
    //QSize sizeHint = formulaInput->viewportSizeHint();
    // get the font to determine height of one row
    //QFontMetrics m(formulaInput->font());
    //int rowHeight = m.lineSpacing();
    //formulaInput->setFixedHeight(3*rowHeight);
    //cellLabel = new QLabel(toolBar);
    //cellLabel->setMaximumSize(50, 10);
    //cellLabel->setMinimumSize(80, 10);

    // Add a layout
    //QGridLayout layout;
    //addLayout
    // Add the input field widgets to the layout

    //toolBar->addWidget(cellLabel);
    //toolBar->addWidget(formulaInput);

    //createActions();


    QDockWidget *dock = new QDockWidget(tr("Spreadsheet Navigation"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QGroupBox *echoGroup = new QGroupBox(tr(""));

    QLabel *echoLabel = new QLabel(tr("Fields"));
    fieldListWidget = new QListWidget();
    fieldListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QGridLayout *echoLayout = new QGridLayout;
    echoLayout->addWidget(echoLabel, 0, 0);
    echoLayout->addWidget(fieldListWidget, 0, 1);
    //echoLayout->addWidget(echoLineEdit, 1, 0, 1, 2);

    QLabel *formatLabel = new QLabel(tr("Format"));
    QLineEdit *formatLineEdit = new QLineEdit;
    formatLineEdit->setPlaceholderText("6.1f");
    echoLayout->addWidget(formatLabel, 1, 0);
    echoLayout->addWidget(formatLineEdit, 1, 1);

    QLineEdit *changesLineEdit = new QLineEdit;
    changesLineEdit->setPlaceholderText("0");
    echoLayout->addWidget(new QLabel(tr("Changes")), 2, 0);
    echoLayout->addWidget(changesLineEdit, 2, 1);

    rangeLineEdit = new QLabel(tr("N/A"));;
    //rangeLineEdit->setPlaceholderText("0.15");
    echoLayout->addWidget(new QLabel(tr("Range")), 3, 0);
    echoLayout->addWidget(rangeLineEdit, 3, 1);

    QLineEdit *nyqVelLineEdit = new QLineEdit;
    nyqVelLineEdit->setPlaceholderText("0");
    echoLayout->addWidget(new QLabel(tr("Nyq Vel")), 4, 0);
    //echoLayout->addWidget(nyqVelLineEdit, 4, 1);
    nyquistVelocityLabel = new QLabel(tr("N/A"));
    echoLayout->addWidget(nyquistVelocityLabel, 4, 1);

    missingDataValueLineEdit = new QLineEdit;
    missingDataValueLineEdit->setText(QString::number(Radx::missingFl32));
    echoLayout->addWidget(new QLabel(tr("Missing Data")), 5, 0);
    echoLayout->addWidget(missingDataValueLineEdit, 5, 1);

    //QFileDialog *logDirDialog = new QFileDialog; 
    echoLayout->addWidget(new QLabel(tr("Log Dir")), 6, 0);
    //echoLayout->addWidget(logDirDialog, 5, 1);

    sweepLineEdit = new QLineEdit;
    QString sweepString = QString::number(sweepNumber);
    echoLayout->addWidget(new QLabel(tr("Sweep")), 0, 2);
    echoLayout->addWidget(sweepLineEdit, 0, 3);

    rayLineEdit = new QLineEdit;
    QString azString = QString::number(rayAzimuth);
    //rayLineEdit->setPlaceholderText(azString);

    //rayLineEdit->setValidator(new QDoubleValidator(0.0,
    //        360.0, 2, rayLineEdit));
    //rayLineEdit->insert(azString);


    
    undoStack = new QUndoStack(this);
    // QLabel *undoLabel = new QLabel(tr("Command List \n Undo/Redo"));
    // QUndoView *undoView = new QUndoView(undoStack);

/*
    echoLayout->addWidget(undoLabel, 0, 2);
    echoLayout->addWidget(undoView, 0, 3);
//---
*/


    echoLayout->addWidget(new QLabel(tr("Ray")), 1, 2);
    echoLayout->addWidget(rayLineEdit, 1, 3);

    raysLineEdit = new QLineEdit;
    raysLineEdit->setText(QString::number(_nRays));
    echoLayout->addWidget(new QLabel(tr("Rays")), 2, 2);
    echoLayout->addWidget(raysLineEdit, 2, 3);

    QLineEdit *cellLineEdit = new QLineEdit;
    cellLineEdit->setPlaceholderText("0");
    echoLayout->addWidget(new QLabel(tr("Cell")), 3, 2);
    echoLayout->addWidget(cellLineEdit, 3, 3);

    //QPushButton *clearEditsButton = new QPushButton("Clear Edits", this);
    //echoLayout->addWidget(clearEditsButton, 6, 0);

    //QPushButton *undoButton = new QPushButton("Undo", this);
    //undoButton->setDisabled(true); 
    //echoLayout->addWidget(undoButton, 6, 1);

    //applyEditsButton = new QPushButton("Apply Edits", this);
    //echoLayout->addWidget(applyEditsButton, 6, 2);
    //connect(applyEditsButton, SIGNAL (released()), this, SLOT (applyEdits())); 

    refreshButton = new QPushButton("Refresh", this);
    refreshButton->setDisabled(false);
    echoLayout->addWidget(refreshButton, 6, 3);
    connect(refreshButton, SIGNAL (released()), this, SLOT (applyEdits()));     

    echoGroup->setLayout(echoLayout);

    //QGridLayout *layout = new QGridLayout;
    //layout->addWidget(echoGroup, 1, 1);

    //HERE  ... not sure of the next two lines ... 
    //QDialog *myList = new QDialog(dock);
    dock->setWidget(echoGroup);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    // int actionFontSize = 14;

    // ============
    table = new QTableWidget(rows, cols, this);
    QHeaderView* header = table->horizontalHeader();
    //-----  custom header ... 
    // QTableWidget *tw = new QTableWidget(5, 5);
    //CustomTableHeader *h = new CustomTableHeader(table->horizontalHeader());
    //CustomTableHeader *h = new CustomTableHeader(header);
    //----

    header->setSectionResizeMode(QHeaderView::Interactive);
    // table->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    // set the column headers to the data fields
   
    for (int c=0; c<cols; c++) {
      QString the_name(" ");
      table->setHorizontalHeaderItem(c, new QTableWidgetItem(the_name));
    }
    
    LOG(DEBUG) << "creating table";
    table->setItemPrototype(table->item(rows - 1, cols - 1));
    table->setItemDelegate(new SpreadSheetDelegate());

    table->setSelectionBehavior(QAbstractItemView::SelectItems); // SelectColumns);

    createActions();
    LOG(DEBUG) << "Action created\n";
    //updateColor(0);
    //LOG(DEBUG) << "update Color\n";
    setupMenuBar();
    LOG(DEBUG) << "setupMenuBar\n";
    //setupContentsBlank();
    //cout << "setupContentsBlank\n";
    //setupContextMenu();
    //cout << "setupContextMenu\n";
    setCentralWidget(table);
    cout << "setCentralWidgets\n";

    statusBar();
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetView::updateStatus);

   // connect(table, SIGNAL(QTableWidget::horizontalHeader().sectionClicked(int)),
   //     this, SLOT(SpreadSheetView::columnHeaderClicked(int)));
    //connect(table, &QTableWidget::currentItemChanged,
    //        this, &SpreadSheetView::updateColor);
    //connect(table, &QTableWidget::currentItemChanged,
    //        this, &SpreadSheetView::updateTextEdit);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheetView::updateStatus);
    //    connect(formulaInput, &QTextEdit::returnPressed, this, &SpreadSheetView::returnPressed);
    // connect(formulaInput, &TextEdit::Pressed, this, &SpreadSheetView::returnPressed);
    //connect(table, &QTableWidget::itemChanged,
    //        this, &SpreadSheetView::updateTextEdit);

    // setTheWindowTitle(rayAzimuth);
    updateLocationInVolume(rayAzimuth, sweepNumber);
}



void SpreadSheetView::setTheWindowTitle(float rayAzimuth, int sweepNumber) {
    QString title("Spreadsheet Editor for Ray ");
    title.append(QString::number(rayAzimuth, 'f', 2));
    title.append(" degrees");
    title.append(" sweep number");
    title.append(QString::number(sweepNumber, 'd', 0));
    setWindowTitle(title);
}

void SpreadSheetView::init()
{
  LOG(DEBUG) << "emitting signal to get field names";
  //  emit a signal to the controller to get the data for display
  emit needFieldNames();
  
}

void SpreadSheetView::createActions()
{

    undoAction = undoStack->createUndoAction(this, tr("&Undo"));
    undoAction->setShortcuts(QKeySequence::Undo);

    redoAction = undoStack->createRedoAction(this, tr("&Redo"));
    redoAction->setShortcuts(QKeySequence::Redo);

    cell_deleteAction = new QAction(tr("Delete"), this);
    cell_deleteAction->setToolTip(
          QString("set selected gates to missing data value"));
    connect(cell_deleteAction, &QAction::triggered, this, &SpreadSheetView::deleteSelection);

    cell_negFoldAction = new QAction(tr("&- Fold"), this);
    cell_negFoldAction->setToolTip(
          QString("subtract Nyquist velocity from selected gates"));
    connect(cell_negFoldAction, &QAction::triggered, this, &SpreadSheetView::subtractNyquistFromSelection);

    cell_plusFoldAction = new QAction(tr("&+ Fold"), this);
    cell_plusFoldAction->setToolTip(
          QString("add Nyquist velocity to selected gates"));
    connect(cell_plusFoldAction, &QAction::triggered, this, &SpreadSheetView::addNyquistToSelection);

    cell_deleteRayAction = new QAction(tr("&Delete Ray"), this);
    cell_deleteRayAction->setToolTip(
          QString("set entire ray to missing data value"));
    connect(cell_deleteRayAction, &QAction::triggered, this, &SpreadSheetView::deleteRay);

    cell_negFoldRayAction = new QAction(tr("&- Fold Ray"), this);
    cell_negFoldRayAction->setToolTip(
          QString("subtract Nyquist velocity from entire ray"));
    connect(cell_negFoldRayAction, &QAction::triggered, this, &SpreadSheetView::subtractNyquistFromRay);

    cell_plusFoldRayAction = new QAction(tr("&+ Fold Ray"), this);
    cell_plusFoldRayAction->setToolTip(
          QString("add Nyquist velocity to entire ray"));
    connect(cell_plusFoldRayAction, &QAction::triggered, this, &SpreadSheetView::addNyquistFromRay);

    cell_negFoldRayGreaterAction = new QAction(tr("&- Fold Ray >"), this);
    cell_negFoldRayGreaterAction->setToolTip(
          QString("subtract Nyquist velocity from selected gate to end gate"));
    connect(cell_negFoldRayGreaterAction, &QAction::triggered, this, &SpreadSheetView::subtractNyquistFromSelectionToEnd);

    cell_plusFoldRayGreaterAction = new QAction(tr("&+ Fold Ray >"), this);
    cell_plusFoldRayGreaterAction->setToolTip(
          QString("add Nyquist velocity starting at selected gate to end gate"));
    connect(cell_plusFoldRayGreaterAction, &QAction::triggered, this, &SpreadSheetView::addNyquistFromSelectionToEnd);

    cell_plusFoldRangeAction = new QAction(tr("&+ Fold Range"), this);
    connect(cell_plusFoldRangeAction, &QAction::triggered, this, &SpreadSheetView::notImplementedMessage);

    cell_zapGndSpdAction = new QAction(tr("Zap Gnd Spd"), this);
    connect(cell_zapGndSpdAction, &QAction::triggered, this, &SpreadSheetView::notImplementedMessage);

    /* TODO:
    cell_MinusFoldRayAction = new QAction(tr("&- Fold Ray"), this);
    //cell_MinusFoldRayAction->setShortcut(Qt::CTRL | Qt::Key_division);
    connect(cell_MinusFoldRayAction, &QAction::triggered, this, &SpreadSheetView::actionMinusFoldRay);
    

    cell_removeAircraftMotionAction = new QAction(tr("&- Aircraft Motion >"), this);
    //cell_divAction->setShortcut(Qt::CTRL | Qt::Key_division);
    connect(cell_removeAircraftMotionAction, &QAction::triggered, this, &SpreadSheetView::actionRemoveAircraftMotion);
    */   
        
    
    display_cellValuesAction = new QAction(tr("&Cell Values"), this);
    //cell_divAction->setShortcut(Qt::CTRL | Qt::Key_division);
    connect(display_cellValuesAction, &QAction::triggered, this, &SpreadSheetView::actionDisplayCellValues);

    display_rayInfoAction = new QAction(tr("&Ray Info"), this);
    //cell_divAction->setShortcut(Qt::CTRL | Qt::Key_division);
    connect(display_rayInfoAction, &QAction::triggered, this, &SpreadSheetView::actionDisplayRayInfo);

    display_metadataAction = new QAction(tr("&Metadata"), this);
    //cell_divAction->setShortcut(Qt::CTRL | Qt::Key_division);
    connect(display_metadataAction, &QAction::triggered, this, &SpreadSheetView::actionDisplayMetadata);

    display_editHistAction = new QAction(tr("&Edit Hist"), this);
    //cell_divAction->setShortcut(Qt::CTRL | Qt::Key_division);
    connect(display_editHistAction, &QAction::triggered, this, &SpreadSheetView::actionDisplayEditHist);
     
    //fontAction = new QAction(tr("Font ..."), this);
    //fontAction->setShortcut(Qt::CTRL | Qt::Key_F);
    //connect(fontAction, &QAction::triggered, this, &SpreadSheetView::selectFont);
    
    //colorAction = new QAction(QPixmap(16, 16), tr("Background &Color..."), this);
    //connect(colorAction, &QAction::triggered, this, &SpreadSheetView::selectColor);
    

    clearAction = new QAction(tr("Delete"), this);
    clearAction->setShortcut(Qt::Key_Delete);
    connect(clearAction, &QAction::triggered, this, &SpreadSheetView::clear);

    aboutSpreadSheet = new QAction(tr("About Spreadsheet"), this);
    connect(aboutSpreadSheet, &QAction::triggered, this, &SpreadSheetView::showAbout);

    exitAction = new QAction(tr("Cell,Ray Labels"), this);
    connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    openAction = new QAction(tr("&Az,Rng Labels"), this);
    connect(openAction, &QAction::triggered, this, &SpreadSheetView::open);

    printAction = new QAction(tr("&Logging Active"), this);
    connect(printAction, &QAction::triggered, this, &SpreadSheetView::print);

    firstSeparator = new QAction(this);
    firstSeparator->setSeparator(true);

    secondSeparator = new QAction(this);
    secondSeparator->setSeparator(true);


    replotAction = new QAction(tr("&Replot All"), this);
    connect(replotAction, &QAction::triggered, this, &SpreadSheetView::replot);

    applyEditsAction = new QAction(tr("&Apply Edits"), this);
    connect(applyEditsAction, &QAction::triggered, this, &SpreadSheetView::applyChanges);

    clearEditsAction = new QAction(tr("&Clear Edits"), this);
    connect(clearEditsAction, &QAction::triggered, this, &SpreadSheetView::applyEdits);

}

void SpreadSheetView::setupMenuBar()
{

    QMenu *displayMenu = menuBar()->addMenu(tr("&Display"));
    displayMenu->addAction(display_cellValuesAction);
    displayMenu->addAction(display_rayInfoAction);
    displayMenu->addAction(display_metadataAction);
    displayMenu->addAction(display_editHistAction);
    

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAction);
    editMenu->addAction(redoAction);
    editMenu->addSeparator();
    editMenu->addAction(cell_deleteAction);
    editMenu->addAction(cell_negFoldAction);
    editMenu->addAction(cell_plusFoldAction);
    editMenu->addAction(cell_deleteRayAction);
    editMenu->addAction(cell_negFoldRayAction);
    editMenu->addAction(cell_plusFoldRayAction);
    editMenu->addAction(cell_negFoldRayGreaterAction);
    editMenu->addAction(cell_plusFoldRayGreaterAction);
    editMenu->addAction(cell_zapGndSpdAction);

    //editMenu->addSeparator();

    menuBar()->addSeparator();

    QMenu *OptionsMenu = menuBar()->addMenu(tr("&Options"));
    OptionsMenu->addAction(openAction);
    OptionsMenu->addAction(printAction);
    OptionsMenu->addAction(exitAction);

    QMenu *ReplotMenu = menuBar()->addMenu(tr("&Replot"));
    ReplotMenu->addAction(replotAction);
    ReplotMenu->addAction(applyEditsAction);
    ReplotMenu->addAction(clearEditsAction);

    //OptionsMenu->addAction(openAction);
    //OptionsMenu->addAction(printAction);
    //OptionsMenu->addAction(exitAction);

    QMenu *aboutMenu = menuBar()->addMenu(tr("&Help"));
    aboutMenu->addAction(aboutSpreadSheet);

    // QMenu *cancelMenu = menuBar()->addMenu(tr("&Cancel"));

}

void SpreadSheetView::nyquistVelocitySent(float nyquistVelocity, int offsetFromClosestRay,
    int fieldIdx, string fieldName) {
    QString nyquistAsString("N/A");
    if (nyquistVelocity < 0.0) {
        // assume not available and display N/A
    } else {
      nyquistAsString.setNum(nyquistVelocity);
    }
    nyquistVelocityLabel->setText(nyquistAsString);
}

void SpreadSheetView::updateStatus(QTableWidgetItem *item)
{
    if (item && item == table->currentItem()) {
        statusBar()->showMessage(item->data(Qt::StatusTipRole).toString(), 1000);
        //cellLabel->setText(tr("Cell: (%1)").arg(SpreadSheetUtils::encode_pos(table->row(item), table->column(item))));
        // int whichColumn = item->column();

      int rayIdx = 0;
      int fieldIdx = 0;

      //decode(whichColumn, &rayIdx, &fieldIdx);
      rayIdx = 0;
      int offsetFromClosest = rayIdx;
      string fieldName;
      emit needNyquistVelocityForRay(offsetFromClosest, fieldIdx, fieldName);

      //---  get the Nyquist Velocity for this ray; if not available, display N/A
    }

}

// TODO: this is the same as updateLocation?? may not need this ... 
// but updateLocation only changes az and el, not the selected field
void SpreadSheetView::updateNavigation(string fieldName, float azimuth, int sweepNumber) {

    // set selected field
    QList<QListWidgetItem *> matchingFields = 
      fieldListWidget->findItems(QString(fieldName.c_str()),
        Qt::MatchExactly);
    if (matchingFields.size() < 0) {
        string msg = "no field found matching ";
        msg.append(fieldName);
        throw std::invalid_argument(msg);
    }
    QListWidgetItem *item = matchingFields.at(0);
    fieldListWidget->setCurrentItem(item);

    int fieldIdx = 0; // unused
    int offsetFromClosest = 0;
    emit needNyquistVelocityForRay(offsetFromClosest, fieldIdx, fieldName);
    

    //emit refreshButton->released();

    // set azimuth
    //QString azString = QString::number(azimuth);
    //rayLineEdit->setText(azString);

    // set sweep
    //sweepLineEdit->setText(QString.setNum(elevation));
}

void SpreadSheetView::updateColor(QTableWidgetItem *item)
{
    QPixmap pix(16, 16);
    QColor col;
    if (item)
      col = item->background().color();
    if (!col.isValid())
        col = palette().base().color();

    QPainter pt(&pix);
    pt.fillRect(0, 0, 16, 16, col);

    QColor lighter = col.lighter();
    pt.setPen(lighter);
    QPoint lightFrame[] = { QPoint(0, 15), QPoint(0, 0), QPoint(15, 0) };
    pt.drawPolyline(lightFrame, 3);

    pt.setPen(col.darker());
    QPoint darkFrame[] = { QPoint(1, 15), QPoint(15, 15), QPoint(15, 1) };
    pt.drawPolyline(darkFrame, 3);

    pt.end();

    colorAction->setIcon(pix);
}

/*
void SpreadSheetView::updateTextEdit(QTableWidgetItem *item)
{
    if (item != table->currentItem())
        return;
    if (item)
        formulaInput->setText(item->data(Qt::EditRole).toString());
    else
        formulaInput->clear();
}
*/

void SpreadSheetView::returnPressed()
{
    QString text = formulaInput->getText();
    LOG(DEBUG) << "text entered: " << text.toStdString();

    int row = table->currentRow();
    int col = table->currentColumn();
    QTableWidgetItem *item = table->item(row, col);
    if (!item)
        table->setItem(row, col, new SpreadSheetItem(text));
    else
        item->setData(Qt::EditRole, text);
    table->viewport()->update();
}

float  SpreadSheetView::myPow()
{
  return(999.9);
}

void SpreadSheetView::replot() {
  LOG(DEBUG) << "enter";
  // ??? applyChanges();
  // send signal to replot data ...
  emit replotRequested();

  LOG(DEBUG) << "exit";  
}

void SpreadSheetView::applyChanges()
{
  // TODO: send a list of the variables in the GlobalObject of the
  // QJSEngine to the model (via the controller?)
  LOG(DEBUG) << "enter";


  // int column = 0;
  for(int column = 0; column < table->columnCount(); column++) {
    vector<float> *data = getDataForVariableFromSpreadSheet(column);

    QTableWidgetItem *tableWidgetItem = table->horizontalHeaderItem(column);
    QString label = tableWidgetItem->text();
    string fieldName = getFieldName(label); 

    float rayAzimuth = getAzimuth(label);
    LOG(DEBUG) << "column " << label.toStdString() << ": extracted field " << fieldName
      << ", azimuth " << rayAzimuth; 
    int sweepNumber = getSweepNumber(); 
    emit applyVolumeEdits(fieldName, rayAzimuth, sweepNumber, data);

    _unAppliedEdits = false;
  }

  emit dataChanged();
  // emit applyVolumeEdits();
  LOG(DEBUG) << "exit";
}

void SpreadSheetView::cancelFormulaInput()
{
  
  //QString text = formulaInput->getText();
  cerr << "cancelling formula changes" << endl;
    /*
    int row = table->currentRow();
    int col = table->currentColumn();
    QTableWidgetItem *item = table->item(row, col);
    if (!item)
        table->setItem(row, col, new SpreadSheetItem(text));
    else
        item->setData(Qt::EditRole, text);
    table->viewport()->update();
    */
}


// TODO: I have to be careful here ...
// addField will always work, it just renames the field if it already
// exists.  If the field hasn't changed, then don't send it in the list.
// Hmmm, how to detect and keep track of changes?  
// 1) Just always add new field and then allow a delete of a field? 
//     Uck, this requires another dialog.
// 2) Select which fields to write to save file?
//     Uck, same ... requires another dialog
// 3) Save them all; use RadxConvert to select fields.
//     Easy, simple. Do it!
//
// Hmm, maybe we want to get the information from the spreadsheet table?
// Yes, get them from spreadsheet (QTableWidget) NOT from the QJSEngine.
// QJSEngine has a bunch of context variables and functions, which we don't want.
vector<string> *SpreadSheetView::getVariablesFromSpreadSheet() {

  vector<string> *names = new vector<string>;

  for (int c=0; c < table->columnCount(); c++) {
    QTableWidgetItem *tableWidgetItem = table->horizontalHeaderItem(c);
    string fieldName = getFieldName(tableWidgetItem->text()); 
    LOG(DEBUG) << fieldName; 
    //if (currentVariableContext.find(it2.name()) == currentVariableContext.end()) {
    //  // we have a newly defined variable
    //  qDebug() << "NEW VARIABLE " << it2.name() <<  ": " << theValue; // it2.value().toString().truncate(100);
    //  addVariableToSpreadSheet(it2.name(), it2.value());
    //}
    names->push_back(fieldName);
  }
	
  return names;
}

//
// Get data from spreadsheet/table because we need to capture individual cell edits
//
vector<float> *SpreadSheetView::getDataForVariableFromSpreadSheet(int column) { // , string fieldName) {

  vector<float> *data = new vector<float>;

  //int c = 0;
  // QTableWidgetItem *tableWidgetItem = table->horizontalHeaderItem(c);
  // TODO; verify fieldName and matches expected name
  LOG(DEBUG) << "getting data for column " << column; // << ", " << fieldName;;
  // go through the rows and put the data into a vector

  float value;
  QString missingQ(_missingDataString.c_str());

  for (int r = 0; r < table->rowCount(); r++) {
    if (r == 468) {
        cerr << "HERE" << endl;
    }
    QTableWidgetItem *tableWidgetItem = table->item(r, column);
    QString content = tableWidgetItem->text();
    if (content.contains(missingQ)) {
      value = _missingDataValue;
    } else {
        bool ok;
        value = content.toFloat(&ok);
        if (ok) {
          LOG(DEBUG) << value;
        } else {
          value = _missingDataValue;
          LOG(DEBUG) << "Could not convert to number " << 
          tableWidgetItem->text().toStdString() << " at r,c " << r << "," << column;
          //QMessageBox::warning(this, tr(tableWidgetItem->text().toStdString().c_str()),
          //                     tr("Could not convert to number.\n"),
          //                     QMessageBox::Abort);
        }
    }
    data->push_back(value);
  }
  /*
    for (int r=0; r<value.property("length").toInt(); r++) {
      //qDebug() << it.name() << ": " << it.value().toString();
      QString valueAsString = value.property(r).toString();
      //      sprintf(formattedData, "%g", value.property(r).toInt());
      //table->setItem(r, c, new SpreadSheetItem(formattedData));
      QTableWidgetItem *tableItem = table->item(r,c);
    }
  */
	
  return data;
}


void SpreadSheetView::setSelectionToValue(QString value)
{
  //    QTableWidgetItem *item = table->currentItem();
  //    QColor col = item ? item->backgroundColor() : table->palette().base().color();
  //    col = QColorDialog::getColor(col, this);
  //    if (!col.isValid())
  //        return;

    QList<QTableWidgetItem*> selected = table->selectedItems();
    if (selected.count() == 0)
        return;

    foreach (QTableWidgetItem *i, selected) {
        if (i)
          i->setText(value); // setBackgroundColor(col);
    }

    //updateColor(table->currentItem());
}


void SpreadSheetView::notImplementedMessage() {
      QMessageBox::information(this, "Not Implemented", "Not Implemented");
}

void SpreadSheetView::actionDisplayCellValues()
{
  notImplementedMessage();
}
void SpreadSheetView::actionDisplayRayInfo()
{
  notImplementedMessage();
}
void SpreadSheetView::actionDisplayMetadata()
{
  notImplementedMessage();
}
void SpreadSheetView::actionDisplayEditHist()
{
  notImplementedMessage();
}



void SpreadSheetView::clear()
{
    foreach (QTableWidgetItem *i, table->selectedItems())
        i->setText("");
}

void SpreadSheetView::setupContextMenu()
{
    addAction(cell_deleteAction);
    addAction(cell_negFoldAction);
    addAction(cell_plusFoldAction);
    addAction(cell_deleteRayAction);
    addAction(cell_negFoldRayAction);
    addAction(cell_plusFoldRayAction);
    addAction(cell_negFoldRayGreaterAction);
    addAction(cell_plusFoldRayGreaterAction);
    addAction(cell_zapGndSpdAction);

    setContextMenuPolicy(Qt::ActionsContextMenu);
}


void SpreadSheetView::setupContents()
{
    QColor titleBackground(Qt::lightGray);
    QFont titleFont = table->font();
    titleFont.setBold(true);

}

void SpreadSheetView::rangeDataSent(size_t nGates, float startingKm, float gateSize) {
  //table->setHorizontalHeaderItem(0, new QTableWidgetItem("Km"));   
  if (table->rowCount() != (int) nGates) {
    table->setRowCount(nGates);
  }
  char rangeFormatted[15];
  for (size_t r=0; r<nGates; r++) {
    sprintf(rangeFormatted, "%8.2f Km", gateSize*r + startingKm);
    table->setVerticalHeaderItem(r, new QTableWidgetItem(rangeFormatted));
  }

  QString n;
  n.setNum(gateSize);
  //rangeLineEdit->clear();
  rangeLineEdit->setText(n);
  _startGateKm = startingKm;
}

// TODO: addm missing value as an argument
// request filled by Controller in response to needFieldData 
void SpreadSheetView::fieldDataSent(vector<float> *data, int offsetFromClosest, int c) {
  size_t nPoints = data->size();
  LOG(DEBUG) << "number of data values = " << nPoints;

  // We have the number of data points, now, request the range geometry
  needRangeData(nPoints); // TODO: what if the range geom is different for each variable? 
  //rangeDataSent(nPoints, 1.5);  // TODO: fix this up.  Not sure where/when to request this.

      string format = "%g";
      char formattedData[250];
      // char dashes[] = _missingDataString.c_str(); // " -- ";
      float MISSING = _missingDataValue;

      int startingColumn = 0; // leave room for the range/gate values

      // TODO: get the globalObject for this field and set the values
      int nthClosest = offsetFromClosest + (_nRays/2);
      cout << "nthClosest = " << nthClosest << endl;
      int c2 = nthClosest * _nFieldsToDisplay + c + startingColumn;
      cout << "c2 = " << c2 << endl;
      //------
      QTableWidgetItem *headerItem = table->horizontalHeaderItem(c);
      QString fieldName = headerItem->text();
      cout << "fieldName = " << fieldName.toStdString() << endl;
      //QJSValue fieldArray = engine.newArray(nPoints);
      QString vectorName = fieldName;
      if (table->rowCount() != (int) nPoints) {
        table->setRowCount(nPoints);
      }
      vector<float> dataVector = *data;
      float *dp = &dataVector[0];

      int startingRow = 0; // leave room for the azimuth header
      for (int r=startingRow; r < startingRow + (int) nPoints; r++) {
      // 752019 for (std::size_t r=0; r<data.size(); r++) {
        //    sprintf(formattedData, format, data[0]);
        if (*dp == MISSING) {
            sprintf(formattedData, "%10s",  _missingDataString.c_str()); // dashes);
        } else {
          sprintf(formattedData, "%g", *dp); // data->at(r));
        }
        // LOG(DEBUG) << "setting " << r << "," << c2 << "= " << formattedData; 
        table->setItem(r, c2, new SpreadSheetItem(formattedData));
        //fieldArray.setProperty(r, *dp); // data.at(r));
        dp++;
      }
      //LOG(DEBUG) << "adding vector form " << vectorName.toStdString();
      //engine.globalObject().setProperty(vectorName, fieldArray);
      //LOG(DEBUG) << "end adding vector form " << vectorName.toStdString();

}

// baseColumn is the first column of the fields for this ray
//  az 0    az 1     az 2
// VEL RF  VEL RF   VEL RF
//         ^  base Column
// 
void SpreadSheetView::setHeader(int baseColumn, int fieldIdx, float azimuth,
    string fieldName) {

    char formattedData[250];
    sprintf(formattedData, "%6.2f\n%s", azimuth, fieldName.c_str()); 
    //cout << "formatted header ..." << endl; 
    //cout << formattedData << endl;

    int column = baseColumn + fieldIdx;
    table->setHorizontalHeaderItem(column, new QTableWidgetItem(formattedData));
}

// TODO: does this become setHeader?
void SpreadSheetView::azimuthForRaySent(float azimuth, int offsetFromClosestRay,
    int fieldIdx, string fieldName) {

      //cout << "--------" << endl;

      int nthClosest = offsetFromClosestRay + (_nRays/2);
      //cout << "nthClosest = " << nthClosest << endl;
      int baseColumn = nthClosest * _nFieldsToDisplay;
      //cout << "baseColumn = " << baseColumn << endl;

      setHeader(baseColumn, fieldIdx, azimuth, fieldName);
      //table->setHorizontalHeaderItem(5, new QTableWidgetItem("ha ha"));

      //cout << "++++++++++" << endl;
}

void SpreadSheetView::applyEdits() {
  bool ok;
  bool carryOn = true;

  QString nRaysToDisplay = raysLineEdit->text();
  int newNRays = nRaysToDisplay.toInt(&ok);

  if (ok) {
    _nRays = newNRays;
    // TODO: what to do when the number of rays to display changes?
  } else {
    criticalMessage("number of rays to display must be between 0 to 10 rays");
  }

  // get new missing data value, if needed
  QString missingDataValue = missingDataValueLineEdit->text();
  LOG(DEBUG) << "missing data entered " << missingDataValue.toStdString();

  float currentMissingValue = missingDataValue.toFloat(&ok);

  if (ok) {
    // signal the controller to update the display
    changeMissingValue(currentMissingValue);  
  } else {
    criticalMessage("missing data must be numeric");
    carryOn = false;
  }

  // get a new azimuth if needed
  QString rayAz = rayLineEdit->text();
  LOG(DEBUG) << "ray az entered " << rayAz.toStdString();

  float currentRayAzimuth = rayAz.toFloat(&ok);

  if (!ok) {
    criticalMessage("ray azimuth must be between 0.0 and 360.0");
    carryOn = false;
  }

  // get a new sweep number if needed
  QString sweepNumber = sweepLineEdit->text();
  LOG(DEBUG) << "sweep entered " << sweepNumber.toStdString();

  float currentSweepNumber = sweepNumber.toInt(&ok);

  if (!ok) {
    criticalMessage("sweep number not recognized");
    carryOn = false;
  }

  if (carryOn) {
    // signal the controller to update the model
    changeAzEl(currentRayAzimuth, currentSweepNumber);  
  } else {
    criticalMessage("no changes made");
  }
}

int SpreadSheetView::getSweepNumber() {

  QString elevation = sweepLineEdit->text();
  LOG(DEBUG) << "sweep entered " << elevation.toStdString();
  bool ok;
  int currentSweepNumber = elevation.toInt(&ok);

  if (!ok) {
    criticalMessage("sweep must be an integer");
    return 0; // ??? 
  } else {
    return currentSweepNumber;
  }
}

float SpreadSheetView::getAzimuth() {

  QString azimuth = rayLineEdit->text();
  LOG(DEBUG) << "azimuth entered " << azimuth.toStdString();
  bool ok;
  float currentAzimuth = azimuth.toFloat(&ok);

  if (!ok) {
    criticalMessage("set azimuth between 0.0 and 360.0");
    return 0; // ??? 
  } else {
    return currentAzimuth;
  }
}

/*
void SpreadSheetView::setAzimuth(float azimuth) {
    QString n;
    n.setNum(azimuth);
    rayLineEdit->clear();
    rayLineEdit->insert(n);
}*/

void SpreadSheetView::changeMissingValue(float currentMissingValue) {
    _missingDataValue = currentMissingValue;
}

void SpreadSheetView::changeAzEl(float azimuth, int sweepNumber) {
  if ((azimuth < 0) || (azimuth > 360)) {
    criticalMessage("ray azimuth must be between 0.0 and 360.0");
    return;
  }

  // signal the controller to update the model
  try {

    // model updated; request new data for display
    const QList<QListWidgetItem *> selectedFields = fieldListWidget->selectedItems();
    vector<string> selectedNames;
    QList<QListWidgetItem *>::const_iterator item;
    for (item = selectedFields.constBegin(); item != selectedFields.constEnd(); ++item) {
      QListWidgetItem *i = *item;
      QString theText = i->text();
      selectedNames.push_back(theText.toStdString());
    }
    fieldNamesSelected(selectedNames);
    updateLocationInVolume(azimuth, sweepNumber);

    signalRayAzimuthChange(azimuth, sweepNumber);

    //needRangeData();
  } catch (std::invalid_argument &ex) {
    criticalMessage(ex.what());
  }
}

// update spreadsheet location in volume
void SpreadSheetView::updateLocationInVolume(float azimuth, int sweepNumber) {

    QString n;
    n.setNum(azimuth);
    rayLineEdit->clear();
    rayLineEdit->insert(n);

    n.setNum(sweepNumber);
    sweepLineEdit->clear();
    sweepLineEdit->insert(n);

    setTheWindowTitle(azimuth, sweepNumber);    
}

void SpreadSheetView::highlightClickedData(string fieldName, float azimuth,
    int sweepNumber, float range) {
    // map the fieldName, azimuth, and range to a cell in the table
    // get the column labels, find the fieldName, then the azimuth
    // get the row labels, find the closest range

    // This method should get the number of rays from the navigation,
    // change the navigation azimuth to the argument azimuth,
    // update the rows and columns of the spreadsheet to the fieldName, azimuth, range, and # of rows
    // highlight/select the fieldName in the list of fields
    // only allow one field at a time?
    // 
    // call applyEdits()

    LOG(DEBUG) << "enter";

    updateNavigation(fieldName, azimuth, sweepNumber);  // also selects the field in list
    updateLocationInVolume(azimuth, sweepNumber);
    applyEdits();
    //return; 
    // find the row
    bool ok;

    float rangeSize = rangeLineEdit->text().toFloat(&ok);
    if (!ok) {
        throw std::invalid_argument("cannot determine range from line edit");
    }
    //needRayGeom???
    float rowD = (range - _startGateKm) / rangeSize;
    int row = (int) rowD;
 

    int nRays = raysLineEdit->text().toInt(&ok);
    if (!ok) {
        throw std::invalid_argument("cannot determine number of rays from line edit");
    }
    int column = nRays / 2;

    //int left = column;
    //int top = row;

    /*
    QString label1 = table->verticalHeaderItem(row)->text();
    QString label2 = table->verticalHeaderItem(row+1)->text();

    string label1_string = label1.toStdString();
    string label2_string = label2.toStdString();

    LOG(DEBUG) << "label1 = " << label1_string;
    LOG(DEBUG) << "label2 = " << label2_string;
   
    float gate1 = 0;
    float gate2 = 0;
    sscanf(label1_string.c_str(), "%f", &gate1);
    sscanf(label2_string.c_str(), "%f", &gate2);  
    LOG(DEBUG) << "gate1 = " << gate1;
    LOG(DEBUG) << "gate2 = " << gate2;      

    float gateSpacing = gate2 - gate1;
    int top = 1;
    if (gateSpacing > 0) {
      top = (int) (range - gate1) / gateSpacing;
    }
    LOG(DEBUG) << "top = " << top;
    if ((top <= 0) || (top > table->rowCount())) top = 1;
    LOG(DEBUG) << "top = " << top;

    // find the column
    int nColumns = table->columnCount();
    LOG(DEBUG) << "nColumns = " << nColumns;
    //bool found = false;
    int column = 0;
    float minDiff = 9e+33;
    int left = 0;
    QString fieldNameQ(fieldName.c_str());
    while ((column < nColumns)) {
        QString labelQ = table->horizontalHeaderItem(column)->text();
        string label_string = labelQ.toStdString();
        LOG(DEBUG) << "label = " << label_string;
        
        if (labelQ.contains(fieldNameQ)) {
            // look for the closest azimuth
            float label_azimuth = 0.0;
            sscanf(label_string.c_str(), "%f", &label_azimuth);  
            LOG(DEBUG) << "label azimuth = " << label_azimuth;
            float diff = fabs(label_azimuth - azimuth);
            if (diff < minDiff) {
                minDiff = diff;
                left = column; 
            }
        }
        column += 1;
    }
    LOG(DEBUG) << "left = " << left;
    */


    //bool selected = true;
    //int bottom = top; int right = left;
    table->setCurrentCell(row, column, QItemSelectionModel::SelectCurrent);
    //const QTableWidgetSelectionRange selectionRange(top, left, bottom, right);
    //table->setRangeSelected(selectionRange, selected);
    QTableWidgetItem * selectedItem = table->item(row, column);
    table->scrollToItem(selectedItem, QAbstractItemView::PositionAtCenter);
    LOG(DEBUG) << "exit";
}



// display the fields selected and their data
void SpreadSheetView::fieldNamesSelected(vector<string> fieldNames) {

  // int useless = 0; // this becomes iterator over nRays
  table->clear(); // clearContents(); 
  _fieldNames.clear();

  // fill everything that needs the fieldNames ...
    _nFieldsToDisplay = fieldNames.size();
    _fieldNames.reserve(_nFieldsToDisplay);

    int somethingHideous = 0;
    table->setColumnCount(_nFieldsToDisplay * _nRays + somethingHideous);
    LOG(DEBUG) << "there are " << fieldNames.size() << " field namess";

    // move the rest of this to the controller to push the data to the view ... 
    int fieldIdx = 0;    
    vector<string>::iterator it; 
    for(it = fieldNames.begin(); it != fieldNames.end(); it++) {
      QString the_name(QString::fromStdString(*it));
      LOG(DEBUG) << *it;
      _fieldNames.push_back(*it);    
        // rayIdx goes from 0 to nRays; map to -nRays/2 ... 0 ... nRays/2
        for (int rayIdx= - _nRays/2; rayIdx <= _nRays/2; rayIdx++) {
          //for (int i=0; i<_nRays; i++) {
            // this ultimately calls setHeader; we need to send the info needed for setHeader
            emit needAzimuthForRay(rayIdx, fieldIdx, *it);
            // needAzimuthForRay(int offsetFromClosest, 

            //table->setHorizontalHeaderItem(c + (i*_nFieldsToDisplay), 
             //   new QTableWidgetItem(the_name));
            // TODO: what about setHorizontalHeaderLabels(const QStringList &labels) instead? would it be faster?
          //}
          //emit needDataForField(*it, rayIdx, fieldIdx);  // need to push this, not pull it.
          //emit needAzimuthForRay(rayIdx);     

        }
        fieldIdx += 1;
    }
}

// request filled by Controller in response to needFieldNames signal
// fill the list of fields to choose
void SpreadSheetView::fieldNamesProvided(vector<string> *fieldNames) {

//  fieldListWidget->addItems(new QStringList(fieldNames));
/*
  // fill everything that needs the fieldNames ...

    table->setColumnCount(fieldNames.size());
    LOG(DEBUG) << " there are " << fieldNames.size() << " field namess";
*/
  // int c = 0;
    vector<string>::iterator it; 
    for(it = fieldNames->begin(); it != fieldNames->end(); it++) {
      QString the_name(QString::fromStdString(*it));
      LOG(DEBUG) << *it;
      fieldListWidget->addItem(the_name); // , new QTableWidgetItem(the_name));
    }
    delete fieldNames;
    LOG(DEBUG) << "exit";
}

// not used 
void SpreadSheetView::columnHeaderClicked(int index) {

  criticalMessage(to_string(index));
}

// fromSelection means from the selected row to the last gate
void SpreadSheetView::setRangeToAllSelectedColumns(int startRow, int fromSelection) {
    QList<QTableWidgetItem *> indexList = table->selectedItems();

    QList<QTableWidgetItem *>::iterator i;
    for (i = indexList.begin(); i != indexList.end(); ++i) {     
        QTableWidgetItem *selection = *i;
        int column = selection->column();
        int top;
        if (fromSelection) {
            int row = selection->row();
            top = row;
        } else {
            top = startRow;
        }
        int currentColumn = column; // table->currentColumn();
        //QTableWidgetSelectionRange::QTableWidgetSelectionRange(t)
        //int top = 1;
        int left = currentColumn;
        int bottom = table->rowCount() - 1;
        int right = currentColumn;
         
        QTableWidgetSelectionRange range(top, left, bottom, right);
        bool select = true;
        table->setRangeSelected(range, select);
    }
}

void SpreadSheetView::deleteRay() {
    LOG(DEBUG) << "enter";
    // set the Range to all selected columns
    int startRow = 0;
    bool fromRowToEnd = false;
    setRangeToAllSelectedColumns(startRow, fromRowToEnd);
    setSelectionToValue(QString(_missingDataString.c_str()));
    LOG(DEBUG) << "exit";

}

void SpreadSheetView::subtractNyquistFromSelection() {
  float factor = -1.0;
  adjustNyquistFromSelection(factor);
}

void SpreadSheetView::addNyquistToSelection() {
  float factor = 1.0;
  adjustNyquistFromSelection(factor); 
}

void SpreadSheetView::subtractNyquistFromSelectionToEnd() {
  float factor = -1.0;
  //int top = table->currentRow();
  //adjustNyquistFromRay(factor, top);  
  adjustNyquistFromSelectionToEnd(factor);
}

void SpreadSheetView::addNyquistFromSelectionToEnd() {
  float factor = 1.0;
  //int top = table->currentRow();
  //adjustNyquistFromRay(factor, top);
  adjustNyquistFromSelectionToEnd(factor);
  // --
}

void SpreadSheetView::adjustNyquistFromSelectionToEnd(float factor) {

  int top = 0;
  bool fromSelection = true;
  adjustNyquistGeneral(factor, fromSelection, top);

}

void SpreadSheetView::subtractNyquistFromRay() {
  float factor = -1.0;
  int top = 0;
  bool fromSelection = false;
  adjustNyquistGeneral(factor, fromSelection, top);
}

void SpreadSheetView::addNyquistFromRay() {
  float factor = 1.0;
  int top = 0;
  bool fromSelection = false;
  adjustNyquistGeneral(factor, fromSelection, top);
}


void SpreadSheetView::adjustNyquistGeneral(float factor, bool fromSelection, int startRow) {

    setRangeToAllSelectedColumns(startRow, fromSelection);
    // subtract the Nyquist value from the ray data 
    // for each selected cell
    // subtract Nyquist
    // set new value
    adjustNyquistFromSelection(factor);
}

// factor = -1.0 to subtract
// factor = 1.0 to add 
void SpreadSheetView::adjustNyquistFromRay(float factor, int top) {
    LOG(DEBUG) << "enter";

    int currentColumn = table->currentColumn();
    //QTableWidgetSelectionRange::QTableWidgetSelectionRange(t)
    //int top = 1;
    int left = currentColumn;
    int bottom = table->rowCount() - 1;
    int right = currentColumn;
     
    QTableWidgetSelectionRange range(top, left, bottom, right);
    bool select = true;
    table->setRangeSelected(range, select);

    // subtract the Nyquist value from the ray data 
    // for each selected cell
    // subtract Nyquist
    // set new value
    adjustNyquistFromSelection(factor);
}

void SpreadSheetView::adjustNyquistFromSelection(float factor) {

    bool ok;
    float nyquistVelocity = nyquistVelocityLabel->text().toFloat(&ok);
    if (!ok) throw std::invalid_argument("cannot determine nyquistVelocity from label");

    nyquistVelocity *= factor;

    QList<QTableWidgetItem*> selected = table->selectedItems();
    if (selected.count() == 0)
        return;

    foreach (QTableWidgetItem *i, selected) {
        if (i) {
          QString textValue = i->text();
          if (!isMissing(textValue)) {
            bool ok;
            float value = textValue.toFloat(&ok);
            if (!ok) {
                string msg = "unknown value in selected cell ";
                msg.append(textValue.toStdString().c_str());
                throw std::invalid_argument(msg);
            }
            value = value + 2.0 * nyquistVelocity;
            i->setText(QString::number(value));
          }
        }
    }

    _unAppliedEdits = true;

    LOG(DEBUG) << "exit";
}

float SpreadSheetView::getAzimuth(QString text) {
    float azimuth = 0.0;
    text.truncate(6);  // remove the field name
    sscanf(text.toStdString().c_str(), "%f", &azimuth); 
    LOG(DEBUG) << azimuth;
    return azimuth; 
}

string SpreadSheetView::getFieldName(QString text) {
   //QString fieldName("no name");
   QStringList list1 = text.split('\n');
   if (list1.size() == 2) {
      return list1.at(1).toStdString();
   } else {
      criticalMessage(text.toStdString().append(" no field name found"));
      return "";
   }
}

void SpreadSheetView::deleteSelection() {
    QString missingValue(QString(_missingDataString.c_str()));
    //missingValue.setNum(_missingDataValue);
    // int currentColumn = table->currentColumn();
    QList<QTableWidgetItem *> selectedGates = table->selectedItems();
    QList<QTableWidgetItem *>::iterator i;
    for (i = selectedGates.begin(); i != selectedGates.end(); ++i) {
        QTableWidgetItem *gate = *i;
        gate->setText(missingValue);
       // TODO: send signal with QList to controller?  NO. how to set these cells to missing?
        //   persist to the model? only when Apply or Save is clicked.
        //emit setDataMissing(currentHeader->text().toStdString(), _missingDataValue); // emit signal
    }
    _unAppliedEdits = true;
}

bool SpreadSheetView::isMissing(QString textValue) {
  return textValue.lastIndexOf(QString(_missingDataString.c_str())) > 0; 
}

void SpreadSheetView::criticalMessage(std::string message)
{
  QMessageBox::StandardButton reply =
    QMessageBox::critical(this, "QMessageBox::critical()",
                          QString::fromStdString(message),
                          QMessageBox::Ok);
  
  //  QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
  if (reply == QMessageBox::Abort)
    LOG(DEBUG) << "Abort";
    // criticalLabel->setText("Abort");
  else if (reply == QMessageBox::Retry)
    LOG(DEBUG) << "Retry";
    // criticalLabel->setText("Retry");
  else
    LOG(DEBUG) << "Ignore";
    // criticalLabel->setText("Ignore");
}

void SpreadSheetView::showAbout()
{
    QMessageBox::about(this, "About Spreadsheet", htmlText);
}


void SpreadSheetView::print()
{
  cerr << "ERROR - Print not supported" << endl;
}

QString SpreadSheetView::open()
{
  cerr << "Open not implemented" << endl;
  
  QString fileName;
  /*
  fileName = QFileDialog::getOpenFileName(this,
     tr("Open Image"), "/tmp", tr("Sweep Files (*.*)"));


  // signal the controller to read new data file

  _controller->open(fileName.toStdString());

  // load new data
  newDataReady();
  */
  return fileName;
}

void  SpreadSheetView::newDataReady()
{
  LOG(DEBUG) << "newDataReady ...";
  setupContents();
}

void SpreadSheetView::closeEvent() {
    if (_unAppliedEdits) {
        string msg = "Unsaved changes to the data. \n";
        msg.append("Use Replot->Apply Edits before closing to avoid this message. \n");
        msg.append("Do you want to apply these changes?");

        QMessageBox::StandardButton reply =
            QMessageBox::warning(this, "QMessageBox::warning()",
                          QString::fromStdString(msg),
                          QMessageBox::Apply | QMessageBox::Discard);
  
        //  QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
        if (reply == QMessageBox::Apply) {
            LOG(DEBUG) << "Apply";
            applyChanges();
            _unAppliedEdits = false;
        }

    } 
    emit spreadSheetClosed(); 
    
}
