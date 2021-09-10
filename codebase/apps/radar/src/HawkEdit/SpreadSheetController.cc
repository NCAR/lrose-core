
//
// SpreadSheetController converts Model data to format for display in View
//  mostly from float, int to string based on format requested.
//

#include <stdio.h>
#include <QtWidgets>

#include "SpreadSheetController.hh"
//#include "SpreadSheetDelegate.hh"
//#include "SpreadSheetItem.hh"
#include "SpreadSheetModel.hh"
#include <toolsa/LogStream.hh>

SpreadSheetController::SpreadSheetController(SpreadSheetView *view)
{
  // int rows;
  // int cols;


  _currentView = view;
  _currentModel = new SpreadSheetModel();

  //  functionsModel = new SoloFunctionsModel(_currentModel);

    // connect controller slots to model signals 

    // connect model signals to controller slots 
  /*
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetController::updateStatus);
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetController::updateColor);
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetController::updateLineEdit);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheetController::updateStatus);
    connect(formulaInput, &QLineEdit::returnPressed, this, &SpreadSheetController::returnPressed);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheetController::updateLineEdit);
  */


}


SpreadSheetController::SpreadSheetController(SpreadSheetView *view, SpreadSheetModel *model)
{
  // int rows;
  // int cols;


  _currentView = view;

  _currentModel = model;

  //  functionsModel = new SoloFunctionsModel(_currentModel);
  //_soloFunctions = new SoloFunctions(_currentModel->_vol);
  //_currentView->setupSoloFunctions(_soloFunctions);

  // connect view signals to controller slots

  connect(_currentView, SIGNAL(needFieldNames()), this, SLOT(needFieldNames()));
  connect(_currentView, SIGNAL(needRangeData(size_t)), this, SLOT(needRangeData(size_t)));
  connect(_currentView, SIGNAL(needDataForField(string, int, int)), 
	  this, SLOT(needDataForField(string, int, int)));
  connect(_currentView, SIGNAL(needAzimuthForRay(int, int, string)), 
    this, SLOT(needAzimuthForRay(int, int, string)));
  connect(_currentView, SIGNAL(needNyquistVelocityForRay(int, int, string)), 
    this, SLOT(needNyquistVelocityForRay(int, int, string)));
  // TODO: need to know which sweep!!!
  connect(_currentView, SIGNAL(applyVolumeEdits(string, float, vector<float> *)), 
	  this, SLOT(getVolumeChanges(string, float, vector<float> *)));

  connect(_currentView, SIGNAL(signalRayAzimuthChange(float, float)), this, SLOT(switchRay(float, float)));

    // connect controller slots to model signals 

    // connect model signals to controller slots 
  /*
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetController::updateStatus);
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetController::updateColor);
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetController::updateLineEdit);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheetController::updateStatus);
    connect(formulaInput, &QLineEdit::returnPressed, this, &SpreadSheetController::returnPressed);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheetController::updateLineEdit);
  */
}


void SpreadSheetController::moveToLocation(string fieldName, float elevation,
    float azimuth) {

  switchRay(azimuth, elevation);
}

void SpreadSheetController::moveToLocation(string fieldName, float elevation,
    float azimuth, float range) {

  moveToLocation(fieldName, elevation, azimuth);
  _currentView->highlightClickedData(fieldName, azimuth, range);
}

void SpreadSheetController::switchRay(float azimuth, float elevation) {
  LOG(DEBUG) << "enter";
  //try {
    _currentModel->setClosestRay(azimuth, elevation);
    LOG(DEBUG) << "switching to ray " << azimuth;
    //_currenView->newElevation(elevation);
    _currentView->updateLocationInVolume(azimuth, elevation);
  //} catch (std::invalid_argument &ex) {
  //  LOG(DEBUG) << "ERROR: " << ex.what();
    //_currentView->criticalMessage(ex.what());
  //}
  LOG(DEBUG) << "exit";
}

vector<string>  *SpreadSheetController::getFieldNames()
{
  vector<string> *names = _currentModel->getFields();
  LOG(DEBUG) << " In SpreadSheetController::getFieldNames, there are " << names->size() << " field names";
  return names;
}

vector<float> *SpreadSheetController::getData(string fieldName, int offsetFromClosest)
{

  LOG(DEBUG) << "getting values for " << fieldName;

  
  //return _currentModel->getData(fieldName);
  
  //  vector<float> SpreadSheetModel::getData(string fieldName)
  vector<float> *data = _currentModel->getData(fieldName, offsetFromClosest);

  LOG(DEBUG) << " found " << data->size() << " data values ";

  return data;
 
}

float SpreadSheetController::getAzimuthForRay(int offsetFromClosest)
{

  LOG(DEBUG) << "getting azimuth for ray: offset from closest =" << offsetFromClosest;

  float azimuth = _currentModel->getAzimuthForRay(offsetFromClosest);

  LOG(DEBUG) << " found: azimuth=" << azimuth;

  return azimuth;
 
}

float SpreadSheetController::getNyquistVelocity(int offsetFromClosest)
{

  LOG(DEBUG) << "getting nyquist velocity for ray: offset from closest =" << offsetFromClosest;
  // TODO: fix this ... 
  //float nyquistVelocity = _currentModel->getNyquistVelocityForRay(offsetFromClosest);

  //LOG(DEBUG) << " found: nyq vel =" << nyquistVelocity;

  return 0.0; // nyquistVelocity;
 
}

void SpreadSheetController::getRangeData(float *startingRangeKm, float *gateSpacingKm)
{
  _currentModel->getRangeGeom(startingRangeKm, gateSpacingKm);
  cout << " In SpreadSheetController::getRangeGeom, startingRangeKm = " 
    << *startingRangeKm << ", gateSpacingKm = " << *gateSpacingKm << endl;
}

void SpreadSheetController::setData(string fieldName, float azimuth, vector<float> *data)
{
  LOG(DEBUG) << "setting values for " << fieldName;
  _currentModel->setData(fieldName, azimuth, data);
}

void SpreadSheetController::setDataMissing(string fieldName, float missingDataValue) {
  _currentModel->setDataMissing(fieldName, missingDataValue);
}

void  SpreadSheetController::needFieldNames() {
  _currentView->fieldNamesProvided(getFieldNames());
}

void  SpreadSheetController::needDataForField(string fieldName, int offsetFromClosest, int c) {

  int useless = 0;
  _currentView->fieldDataSent(getData(fieldName, offsetFromClosest), offsetFromClosest, c);
}

void  SpreadSheetController::needAzimuthForRay(int offsetFromClosest, 
  int fieldIdx, string fieldName) {

//azimuthForRaySent(float azimuth, int offsetFromClosestRay, int fieldIdx, string fieldName)
  _currentView->azimuthForRaySent(getAzimuthForRay(offsetFromClosest), offsetFromClosest,
    fieldIdx, fieldName);
}

void  SpreadSheetController::needNyquistVelocityForRay(int offsetFromClosest, 
  int fieldIdx, string fieldName) {
  //_currentView->nyquistVelocitySent(getNyquistVelocityForRay(offsetFromClosest), offsetFromClosest,
  //  fieldIdx, fieldName);
}

void  SpreadSheetController::needRangeData(size_t nGates) {
  float startingKm;
  float gateSpacingKm;
  getRangeData(&startingKm, &gateSpacingKm);
  _currentView->rangeDataSent(nGates, startingKm, gateSpacingKm);
}

// persist the changes in the spreadsheet to the model, which is the data volume
void SpreadSheetController::getVolumeChanges(string fieldName, float azimuth, vector<float> *data) {

  LOG(DEBUG) << "enter";
  //vector<string> *fields = _currentView->getVariablesFromSpreadSheet();

  // update the model
  //int column = 0;
  //for(vector<string>::iterator s = fields->begin(); s != fields->end(); s++) {
  //  vector<float> *data = _currentView->getDataForVariableFromSpreadSheet(column, *s);
  setData(fieldName, azimuth, data);

  //  column++;
  //}
  // announce changes have been made to the data
  // volumeUpdated();
  LOG(DEBUG) << "exit";
}

void SpreadSheetController::volumeUpdated() {
  emit volumeChanged(); // _currentModel->getVolume());
}



void SpreadSheetController::open(string fileName)
{

  // _currentModel->initData(fileName);

  // signal the view to pull the data
  // for each fieldName ...
  // _currentView->newDataReady();
  //  while (_currentModel->moreData()) {  
  //  vector <float> data = _currentModel->getData(fieldName);
  // update display
  //  _currentView->setupContents(data, fieldName);  
  //}
}
/*
void SpreadSheetController::setupSoloFunctions()
{
  
  QJSValue myExt = engine.newQObject(new SoloFunctions());
  engine.globalObject().setProperty("cat", myExt.property("cat"));
  engine.globalObject().setProperty("sqrt", myExt.property("sqrt"));
  engine.globalObject().setProperty("REMOVE_AIRCRAFT_MOTION", myExt.property("REMOVE_AIRCRAFT_MOTION"));
  engine.globalObject().setProperty("add", myExt.property("add"));
  
}
*/

 /*
void SpreadSheetController::processFormula(QString formula)
{

  // Grab the context before evaluating the formula  
  // ======
  // TODO: YES! This works.  The new global variables are listed here;
  // just find them and add them to the spreadsheet and to the Model??
  // HERE!!!
  // try iterating over the properties of the globalObject to find new variables

  std::map<QString, QString> currentVariableContext;
  QJSValue theGlobalObject = engine.globalObject();

  QJSValueIterator it(theGlobalObject);
  while (it.hasNext()) {
    it.next();
    qDebug() << it.name() << ": " << it.value().toString();
    currentVariableContext[it.name()] = it.value().toString();
  }
  // ======                                                                                                                                    

  QJSValue result = engine.evaluate(text);
  if (result.isArray()) {
    cerr << " the result is an array\n";
    //vector<int> myvector;                                                                                                                      
    //myvector = engine.fromScriptValue(result);                                                                                                 
  }
  cerr << " the result is " << result.toString().toStdString() << endl;

  // ====== 
  // TODO: YES! This works.  The new global variables are listed here;
  // just find them and add them to the spreadsheet and to the Model?? 
  // HERE!!!
  // try iterating over the properties of the globalObject to find new variables                                                                 
  QJSValue newGlobalObject = engine.globalObject();

  QJSValueIterator it2(newGlobalObject);
  while (it2.hasNext()) {
    it2.next();
    qDebug() << it2.name() << ": " << it2.value().toString();
    if (currentVariableContext.find(it2.name()) == currentVariableContext.end()) {
      // we have a newly defined variable                                                                                                      
      qDebug() << "NEW VARIABLE " << it2.name() <<  ": " << it2.value().toString();
      addVariableToSpreadSheet(it2.name(), it2.value());
    }
  }
  // ======    



}
 */

/*
void SpreadSheetController::createActions()
{
    cell_sumAction = new QAction(tr("- Fold"), this);
    connect(cell_sumAction, &QAction::triggered, this, &SpreadSheetController::actionSum);

    cell_addAction = new QAction(tr("&+ Fold"), this);
    cell_addAction->setShortcut(Qt::CTRL | Qt::Key_Plus);
    connect(cell_addAction, &QAction::triggered, this, &SpreadSheetController::actionAdd);

    cell_subAction = new QAction(tr("&Delete Ray"), this);
    cell_subAction->setShortcut(Qt::CTRL | Qt::Key_Minus);
    connect(cell_subAction, &QAction::triggered, this, &SpreadSheetController::actionSubtract);

    cell_mulAction = new QAction(tr("&- Fold Ray"), this);
    cell_mulAction->setShortcut(Qt::CTRL | Qt::Key_multiply);
    connect(cell_mulAction, &QAction::triggered, this, &SpreadSheetController::actionMultiply);

    cell_divAction = new QAction(tr("&+ Fold Ray"), this);
    cell_divAction->setShortcut(Qt::CTRL | Qt::Key_division);
    connect(cell_divAction, &QAction::triggered, this, &SpreadSheetController::actionDivide);
  
    cell_MinusFoldRayAction = new QAction(tr("&- Fold Ray"), this);
    //cell_MinusFoldRayAction->setShortcut(Qt::CTRL | Qt::Key_division);
    connect(cell_MinusFoldRayAction, &QAction::triggered, this, &SpreadSheetController::actionMinusFoldRay);
 
    cell_divAction = new QAction(tr("&+ Fold Ray >"), this);
    cell_divAction->setShortcut(Qt::CTRL | Qt::Key_division);
    connect(cell_divAction, &QAction::triggered, this, &SpreadSheetController::actionDivide);

    cell_divAction = new QAction(tr("&- Fold Ray >"), this);
    cell_divAction->setShortcut(Qt::CTRL | Qt::Key_division);
    connect(cell_divAction, &QAction::triggered, this, &SpreadSheetController::actionDivide);

    cell_divAction = new QAction(tr("&Zap Gnd Spd"), this);
    cell_divAction->setShortcut(Qt::CTRL | Qt::Key_division);
    connect(cell_divAction, &QAction::triggered, this, &SpreadSheetController::actionDivide);
  
    fontAction = new QAction(tr("Font ..."), this);
    fontAction->setShortcut(Qt::CTRL | Qt::Key_F);
    connect(fontAction, &QAction::triggered, this, &SpreadSheetController::selectFont);

    colorAction = new QAction(QPixmap(16, 16), tr("Background &Color..."), this);
    connect(colorAction, &QAction::triggered, this, &SpreadSheetController::selectColor);

    clearAction = new QAction(tr("Delete"), this);
    clearAction->setShortcut(Qt::Key_Delete);
    connect(clearAction, &QAction::triggered, this, &SpreadSheetController::clear);

    aboutSpreadSheetController = new QAction(tr("About Spreadsheet"), this);
    connect(aboutSpreadSheetController, &QAction::triggered, this, &SpreadSheetController::showAbout);

    exitAction = new QAction(tr("E&xit"), this);
    connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    openAction = new QAction(tr("&Open"), this);
    connect(openAction, &QAction::triggered, this, &SpreadSheetController::open);

    printAction = new QAction(tr("&Print"), this);
    connect(printAction, &QAction::triggered, this, &SpreadSheetController::print);

    firstSeparator = new QAction(this);
    firstSeparator->setSeparator(true);

    secondSeparator = new QAction(this);
    secondSeparator->setSeparator(true);
}

void SpreadSheetController::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    fileMenu->addAction(printAction);
    fileMenu->addAction(exitAction);

    QMenu *cellMenu = menuBar()->addMenu(tr("&Cell/Edit"));
    cellMenu->addAction(cell_addAction);
    cellMenu->addAction(cell_subAction);
    cellMenu->addAction(cell_mulAction);
    cellMenu->addAction(cell_divAction);
    cellMenu->addAction(cell_sumAction);
    cellMenu->addSeparator();
    cellMenu->addAction(colorAction);
    cellMenu->addAction(fontAction);
    //cellMenu->addAction(clearEditsAction);
    //cellMenu->addAction(undoAction);
    //cellMenu->addAction(applyEditsAction);  // TODO: what does apply edits do?
    //cellMenu->addAction(refreshAction);


    QMenu *optionsMenu = menuBar()->addMenu(tr("&Options"));
    QMenu *replotMenu = menuBar()->addMenu(tr("&Replot"));

    menuBar()->addSeparator();

    QMenu *aboutMenu = menuBar()->addMenu(tr("&Help"));
    aboutMenu->addAction(aboutSpreadSheetController);
}

void SpreadSheetController::updateStatus(QTableWidgetItem *item)
{
    if (item && item == table->currentItem()) {
        statusBar()->showMessage(item->data(Qt::StatusTipRole).toString(), 1000);
        cellLabel->setText(tr("Cell: (%1)").arg(SpreadSheetControllerUtils::encode_pos(table->row(item), table->column(item))));
    }
}

void SpreadSheetController::updateColor(QTableWidgetItem *item)
{
    QPixmap pix(16, 16);
    QColor col;
    if (item)
        col = item->backgroundColor();
    if (!col.isValid())
        col = palette().base().color();

    QPainter pt(&pix);
    pt.fillRect(0, 0, 16, 16, col);

    QColor lighter = col.light();
    pt.setPen(lighter);
    QPoint lightFrame[] = { QPoint(0, 15), QPoint(0, 0), QPoint(15, 0) };
    pt.drawPolyline(lightFrame, 3);

    pt.setPen(col.dark());
    QPoint darkFrame[] = { QPoint(1, 15), QPoint(15, 15), QPoint(15, 1) };
    pt.drawPolyline(darkFrame, 3);

    pt.end();

    colorAction->setIcon(pix);
}

void SpreadSheetController::updateLineEdit(QTableWidgetItem *item)
{
    if (item != table->currentItem())
        return;
    if (item)
        formulaInput->setText(item->data(Qt::EditRole).toString());
    else
        formulaInput->clear();
}

void SpreadSheetController::returnPressed()
{
    QString text = formulaInput->text();
    int row = table->currentRow();
    int col = table->currentColumn();
    QTableWidgetItem *item = table->item(row, col);
    if (!item)
        table->setItem(row, col, new SpreadSheetControllerItem(text));
    else
        item->setData(Qt::EditRole, text);
    table->viewport()->update();
}

void SpreadSheetController::selectColor()
{
    QTableWidgetItem *item = table->currentItem();
    QColor col = item ? item->backgroundColor() : table->palette().base().color();
    col = QColorDialog::getColor(col, this);
    if (!col.isValid())
        return;

    QList<QTableWidgetItem*> selected = table->selectedItems();
    if (selected.count() == 0)
        return;

    foreach (QTableWidgetItem *i, selected) {
        if (i)
            i->setBackgroundColor(col);
    }

    updateColor(table->currentItem());
}

void SpreadSheetController::selectFont()
{
    QList<QTableWidgetItem*> selected = table->selectedItems();
    if (selected.count() == 0)
        return;

    bool ok = false;
    QFont fnt = QFontDialog::getFont(&ok, font(), this);

    if (!ok)
        return;
    foreach (QTableWidgetItem *i, selected) {
        if (i)
            i->setFont(fnt);
    }
}
    */

