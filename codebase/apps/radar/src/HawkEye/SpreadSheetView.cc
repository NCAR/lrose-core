
#include <stdio.h>
#include <QtWidgets>
#include <QMessageBox>
#include <QModelIndex>
#include <QJSEngine>
#include <QJSValue>
#include <QJSValueIterator>
#include <vector>
#include <iostream>
#include <toolsa/LogStream.hh>
//#include "TextEdit.hh"
#include "SpreadSheetView.hh"
#include "SpreadSheetDelegate.hh"
#include "SpreadSheetItem.hh"
#include "SoloFunctions.hh"
#include "DataField.hh"


using namespace std;

Q_DECLARE_METATYPE(QVector<int>)
Q_DECLARE_METATYPE(QVector<double>)


// SpreadSheetView will emit signals that are followed by the controller
//
//

// Qt::WindowMinMaxButtonsHint

  SpreadSheetView::SpreadSheetView(QWidget *parent, float rayAzimuth)
  : QMainWindow(parent)
{
  LOG(DEBUG_VERBOSE) << "in SpreadSheetView constructor";
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

  //_volumeData = vol;
    addToolBar(toolBar = new QToolBar());
    formulaInput = new TextEdit(this);
    //QSize sizeHint = formulaInput->viewportSizeHint();
    // get the font to determine height of one row
    QFontMetrics m(formulaInput->font());
    int rowHeight = m.lineSpacing();
    formulaInput->setFixedHeight(3*rowHeight);
    cellLabel = new QLabel(toolBar);
    //cellLabel->setMaximumSize(50, 10);
    //cellLabel->setMinimumSize(80, 10);

    toolBar->addWidget(cellLabel);
    toolBar->addWidget(formulaInput);

    int actionFontSize = 14;

    // =======

    //QPushButton cancelButton(tr("Cancel"), this);
    //connect(&cancelButton, &QAbstractButton::clicked, &functionDialog, &QDialog::reject);

    //QPushButton okButton(tr("OK"), this);
    //okButton.setDefault(true);
    //connect(&okButton, &QAbstractButton::clicked, &functionDialog, &QDialog::accept);
    //    addWidget(&cancelButton);
    //addWidget(&okButton);
    QAction *cancelAct = new QAction(tr("&Cancel"), this);
    cancelAct->setStatusTip(tr("Cancel changes"));
    cancelAct->setIcon(QIcon(":/images/cancel_x.png"));
    //QFont cancelFont = cancelAct->font();
    //cancelFont.setBold(true);
    //cancelFont.setPointSize(actionFontSize);
    //cancelAct->setFont(cancelFont);
    connect(cancelAct, &QAction::triggered, this, &SpreadSheetView::cancelFormulaInput);
    toolBar->addAction(cancelAct);

    QAction *okAct = new QAction(tr("&Ok"), this);
    okAct->setStatusTip(tr("Accept changes"));
    okAct->setIcon(QIcon(":/images/ok_check.png"));
    //QFont okFont = okAct->font();
    //okFont.setBold(true);
    //okFont.setPointSize(actionFontSize);
    //okAct->setFont(okFont);
    connect(okAct, &QAction::triggered, this, &SpreadSheetView::acceptFormulaInput);
    toolBar->addAction(okAct);

    QAction *applyAct = new QAction(tr("&Apply"), this);
    applyAct->setStatusTip(tr("Apply changes to display"));
    applyAct->setIcon(QIcon(":/images/apply.png"));
    QFont applyFont = applyAct->font();
    applyFont.setBold(true);
    applyFont.setPointSize(actionFontSize);
    applyAct->setFont(applyFont);
    connect(applyAct, &QAction::triggered, this, &SpreadSheetView::applyChanges);
    toolBar->addAction(applyAct);

    /*
    toolBar->addAction(okAct);
    toolBar->addAction(applyAct);
    toolBar->addAction(cancelAct);
    */

    /*
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(&okButton);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(&cancelButton);

    QHBoxLayout *dialogLayout = new QHBoxLayout(&functionDialog);
    dialogLayout->addWidget(&textEditArea);
    dialogLayout->addStretch(1);
    dialogLayout->addItem(buttonsLayout);
    */

    // ============
    table = new QTableWidget(rows, cols, this);
    QHeaderView* header = table->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);
    // table->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    // set the column headers to the data fields
    
    
   
    for (int c=0; c<cols; c++) {
      QString the_name(" ");
      table->setHorizontalHeaderItem(c, new QTableWidgetItem(the_name));
    }
    
    LOG(DEBUG_VERBOSE) << "creating table";
    table->setItemPrototype(table->item(rows - 1, cols - 1));
    table->setItemDelegate(new SpreadSheetDelegate());

    createActions();
    LOG(DEBUG_VERBOSE) << "Action created\n";
    updateColor(0);
    LOG(DEBUG_VERBOSE) << "update Color\n";
    setupMenuBar();
    LOG(DEBUG_VERBOSE) << "setupMenuBar\n";
    //setupContentsBlank();
    //cout << "setupContentsBlank\n";
    setupContextMenu();
    cout << "setupContextMenu\n";
    setCentralWidget(table);
    cout << "setCentralWidgets\n";

    statusBar();
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetView::updateStatus);
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetView::updateColor);
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetView::updateTextEdit);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheetView::updateStatus);
    //    connect(formulaInput, &QTextEdit::returnPressed, this, &SpreadSheetView::returnPressed);
    // connect(formulaInput, &TextEdit::Pressed, this, &SpreadSheetView::returnPressed);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheetView::updateTextEdit);


    QString title("Spreadsheet Editor for Ray ");
    title.append(QString::number(rayAzimuth, 'f', 2));
    title.append(" degrees");
    setWindowTitle(title);

    //setupSoloFunctions();
}

// use when a new file is opened ...
/*
SpreadSheetView::SpreadSheetView(std::string fileName, QWidget *parent)
        : QMainWindow(parent)
{
  int rows;
  int cols;

  //_controller = new SpreadSheetController(this);
  //_controller->open(fileName);
  vector<std::string> fieldNames = _controller->getFieldNames();
  //cols = displayInfo.getNumFields();
  // vector<std::string> fieldNames = vol.getUniqueFieldNameList();
  cols = (int) fieldNames.size();
  rows = 20;

  //_volumeData = vol;
    addToolBar(toolBar = new QToolBar());
    //formulaInput = new TextEdit(this); // parent);
    //SpreadSheetDelegate *formulaEditor = new SpreadSheetDelegate(this); // new QTextEdit();
    //formulaInput = formulaEditor->createEditor(this, ); // new QTextEdit();

    cellLabel = new QLabel(toolBar);
    cellLabel->setMinimumSize(80, 0);

    toolBar->addWidget(cellLabel);
    //toolBar->addWidget(formulaInput);

    table = new QTableWidget(rows, cols, this);
    table->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    // set the column headers to the data fields
    
    
    int c = 0;
    vector<std::string>::iterator it; 
    for(it = fieldNames.begin(); it != fieldNames.end(); it++, c++) {
      QString the_name(QString::fromStdString(*it));
      //cerr << *it << endl;
      table->setHorizontalHeaderItem(c, new QTableWidgetItem(the_name));
    }
    
    table->setItemPrototype(table->item(rows - 1, cols - 1));
    table->setItemDelegate(new SpreadSheetDelegate());

    createActions();
    cout << "Action created\n";
    updateColor(0);
    cout << "update Color\n";
    setupMenuBar();
    cout << "setupMenuBar\n";
    setupContents();
    cout << "setupContents\n";
    setupContextMenu();
    cout << "setupContextMenu\n";
    setCentralWidget(table);
    cout << "setCentralWidgets\n";

    statusBar();
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetView::updateStatus);
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetView::updateColor);
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetView::updateTextEdit);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheetView::updateStatus);
    // connect(formulaInput, &QTextEdit::returnPressed, this, &SpreadSheetView::returnPressed);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheetView::updateTextEdit);

    setWindowTitle(tr("Spreadsheet"));

    //setupSoloFunctions();
}
*/



void SpreadSheetView::init()
{
  LOG(DEBUG_VERBOSE) << "emitting signal to get field names";
  //  emit a signal to the controller to get the data for display
  emit needFieldNames();
  
  /*  int rows;
  int cols;

  cols = 5; 
  rows = 20;

    addToolBar(toolBar = new QToolBar());

    cellLabel = new QLabel(toolBar);
    cellLabel->setMinimumSize(80, 0);

    toolBar->addWidget(cellLabel);

    table = new QTableWidget(rows, cols, this);
    table->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    // set the column headers to the data fields

    table->setItemPrototype(table->item(rows - 1, cols - 1));
    table->setItemDelegate(new SpreadSheetDelegate());

    createActions();
    cout << "Actions created\n";
    updateColor(0);
    cout << "update Color\n";
    setupMenuBar();
    cout << "setupMenuBar\n";
    setupContents();
    cout << "setupContents\n";
    setupContextMenu();
    cout << "setupContextMenu\n";
    setCentralWidget(table);
    cout << "setCentralWidgets\n";

    statusBar();
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetView::updateStatus);
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetView::updateColor);
    connect(table, &QTableWidget::currentItemChanged,
            this, &SpreadSheetView::updateTextEdit);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheetView::updateStatus);
    // connect(formulaInput, &QTextEdit::returnPressed, this, &SpreadSheetView::returnPressed);
    connect(table, &QTableWidget::itemChanged,
            this, &SpreadSheetView::updateTextEdit);

    setWindowTitle(tr("Spreadsheet"));
  */
    //setupSoloFunctions();
}

void SpreadSheetView::createActions()
{
    cell_sumAction = new QAction(tr("+ Sum"), this);
    connect(cell_sumAction, &QAction::triggered, this, &SpreadSheetView::actionSum);

    cell_addAction = new QAction(tr("&+ Fold"), this);
    cell_addAction->setShortcut(Qt::CTRL | Qt::Key_Plus);
    connect(cell_addAction, &QAction::triggered, this, &SpreadSheetView::actionAdd);

    cell_subAction = new QAction(tr("&Delete Ray"), this);
    cell_subAction->setShortcut(Qt::CTRL | Qt::Key_Minus);
    connect(cell_subAction, &QAction::triggered, this, &SpreadSheetView::actionSubtract);

    cell_mulAction = new QAction(tr("&- Fold Ray"), this);
    cell_mulAction->setShortcut(Qt::CTRL | Qt::Key_multiply);
    connect(cell_mulAction, &QAction::triggered, this, &SpreadSheetView::actionMultiply);

    cell_divAction = new QAction(tr("&+ Fold Ray"), this);
    cell_divAction->setShortcut(Qt::CTRL | Qt::Key_division);
    connect(cell_divAction, &QAction::triggered, this, &SpreadSheetView::actionDivide);
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
     
    fontAction = new QAction(tr("Font ..."), this);
    fontAction->setShortcut(Qt::CTRL | Qt::Key_F);
    connect(fontAction, &QAction::triggered, this, &SpreadSheetView::selectFont);
    
    colorAction = new QAction(QPixmap(16, 16), tr("Background &Color..."), this);
    connect(colorAction, &QAction::triggered, this, &SpreadSheetView::selectColor);
    

    clearAction = new QAction(tr("Delete"), this);
    clearAction->setShortcut(Qt::Key_Delete);
    connect(clearAction, &QAction::triggered, this, &SpreadSheetView::clear);

    aboutSpreadSheet = new QAction(tr("About Spreadsheet"), this);
    connect(aboutSpreadSheet, &QAction::triggered, this, &SpreadSheetView::showAbout);

    exitAction = new QAction(tr("E&xit"), this);
    connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    openAction = new QAction(tr("&Open"), this);
    connect(openAction, &QAction::triggered, this, &SpreadSheetView::open);

    printAction = new QAction(tr("&Print"), this);
    connect(printAction, &QAction::triggered, this, &SpreadSheetView::print);

    firstSeparator = new QAction(this);
    firstSeparator->setSeparator(true);

    secondSeparator = new QAction(this);
    secondSeparator->setSeparator(true);
}

void SpreadSheetView::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    fileMenu->addAction(printAction);
    fileMenu->addAction(exitAction);

    
    QMenu *displayMenu = menuBar()->addMenu(tr("&Display"));
    displayMenu->addAction(display_cellValuesAction);
    displayMenu->addAction(display_rayInfoAction);
    displayMenu->addAction(display_metadataAction);
    displayMenu->addAction(display_editHistAction);
    

    QMenu *cellMenu = menuBar()->addMenu(tr("&Edit"));
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


    // QMenu *optionsMenu = menuBar()->addMenu(tr("&Options"));
    // QMenu *replotMenu = menuBar()->addMenu(tr("&Replot"));

    menuBar()->addSeparator();

    QMenu *aboutMenu = menuBar()->addMenu(tr("&Help"));
    aboutMenu->addAction(aboutSpreadSheet);
}

void SpreadSheetView::updateStatus(QTableWidgetItem *item)
{
    if (item && item == table->currentItem()) {
        statusBar()->showMessage(item->data(Qt::StatusTipRole).toString(), 1000);
        //cellLabel->setText(tr("Cell: (%1)").arg(SpreadSheetUtils::encode_pos(table->row(item), table->column(item))));
    }
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

void SpreadSheetView::updateTextEdit(QTableWidgetItem *item)
{
    if (item != table->currentItem())
        return;
    if (item)
        formulaInput->setText(item->data(Qt::EditRole).toString());
    else
        formulaInput->clear();
}

void SpreadSheetView::returnPressed()
{
    QString text = formulaInput->getText();
    LOG(DEBUG_VERBOSE) << "text entered: " << text.toStdString();

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

// SoloFunctions object comes in with data model already attached
void SpreadSheetView::setupSoloFunctions(SoloFunctions *soloFunctions) {

  LOG(DEBUG_VERBOSE) << "defining REMOVE_AIRCRAFT_MOTION function";
  //  emit radarVolumeDataRequest();  make the request for the data inside the SoloFunctions object

  //    QJSValue myExt = engine.newQObject(new SoloFunctions(_controller));
  QJSValue myExt = engine.newQObject(soloFunctions); // new SoloFunctions());
    engine.globalObject().setProperty("cat", myExt.property("cat"));
    engine.globalObject().setProperty("sqrt", myExt.property("sqrt"));
    engine.globalObject().setProperty("REMOVE_AIRCRAFT_MOTION", myExt.property("REMOVE_AIRCRAFT_MOTION"));
    engine.globalObject().setProperty("add", myExt.property("add"));

    // print the context ...
    printQJSEngineContext();

    /*
    LOG(DEBUG_VERBOSE) << "current QJSEngine context ...";

    std::map<QString, QString> currentVariableContext;
    QJSValue theGlobalObject = engine.globalObject();

      QJSValueIterator it(theGlobalObject);
      while (it.hasNext()) {
	it.next();
        QString theValue = it.value().toString();
        theValue.truncate(100);

	qDebug() << it.name() << ": " << theValue; // it.value().toString().truncate(100);
        currentVariableContext[it.name()] = it.value().toString();
      }
      LOG(DEBUG_VERBOSE) << "end current QJSEngine context";
    */
  
}

void SpreadSheetView::applyChanges()
{
  // TODO: send a list of the variables in the GlobalObject of the
  // QJSEngine to the model (via the controller?)
  emit applyVolumeEdits();
}

void SpreadSheetView::acceptFormulaInput()
{
    QString text = formulaInput->getText();
    cerr << "text entered: " << text.toStdString() << endl;
    
    // Grab the context before evaluating the formula
    //  YES! This works.  The new global variables are listed here;
    // just find them and add them to the spreadsheet and to the Model??
    // HERE!!!
    // try iterating over the properties of the globalObject to find new variables
    std::map<QString, QString> currentVariableContext;
    QJSValue theGlobalObject = engine.globalObject();

    QJSValueIterator it(theGlobalObject);
    while (it.hasNext()) {
      it.next();
      QString theValue = it.value().toString();
      theValue.truncate(100);

      LOG(DEBUG_VERBOSE) << it.name().toStdString() << ": " << theValue.toStdString(); // it.value().toString().truncate(100);
      currentVariableContext[it.name()] = it.value().toString();
    }
      // ======
    try {
      QJSValue result = engine.evaluate(text);
      if (result.isError()) {
        QString message;
        message.append(result.toString());
        message.append(" on line number ");
        message.append(result.property("lineNumber").toString());
        criticalMessage(message.toStdString()); 
        LOG(DEBUG_VERBOSE)
	  << "Uncaught exception at line"
  	  << result.property("lineNumber").toInt()
	  << ":" << result.toString().toStdString();
      } else {

	LOG(DEBUG_VERBOSE) << " the result is " << result.toString().toStdString();

	if (result.isArray()) {
	  cerr << " the result is an array\n"; 
        //vector<int> myvector;
        //myvector = engine.fromScriptValue(result);
	} 
        if (result.isNumber()) {
          setSelectionToValue(result.toString());
        }

      // ======
      //  YES! This works.  The new global variables are listed here;
      // just find them and add them to the spreadsheet and to the Model??
      // HERE!!!
      // try iterating over the properties of the globalObject to find new variables
	QJSValue newGlobalObject = engine.globalObject();

	QJSValueIterator it2(newGlobalObject);
	while (it2.hasNext()) {
	  it2.next();
          QString theValue = it2.value().toString();
          theValue.truncate(100);
	  LOG(DEBUG_VERBOSE) << it2.name().toStdString() << ": " << theValue.toStdString();
	  if (currentVariableContext.find(it2.name()) == currentVariableContext.end()) {
	    // we have a newly defined variable
	    LOG(DEBUG_VERBOSE) << "NEW VARIABLE " << it2.name().toStdString() <<  ": " << theValue.toStdString();
	    addVariableToSpreadSheet(it2.name(), it2.value());
	  }
	}
	// ======
        /*
	int row = table->currentRow();
	int col = table->currentColumn();
	QTableWidgetItem *item = table->item(row, col);
	if (!item) {
          LOG(DEBUG_VERBOSE) << "considered not item ";
	  table->setItem(row, col, new SpreadSheetItem(result.toString())); // text));
        } else {
          LOG(DEBUG_VERBOSE) << "considered item";
	  item->setData(Qt::EditRole, result.toString()); // text);
        }
        */
	table->viewport()->update();
      }
    } catch (const std::exception& ex) {
      criticalMessage(ex.what());
    } catch (const std::string& ex) {
      criticalMessage(ex);
    } catch (...) {
      criticalMessage("Error occurred during evaluation");
    }

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

  
  // try iterating over the properties of the globalObject to find new variables
  //QJSValue newGlobalObject = engine.globalObject();

  //QJSValueIterator it2(newGlobalObject);
  //while (it2.hasNext()) {
  //  it2.next();
    //QString theValue = it2.value().toString(); // TODO: this could be the bottle neck; try sending list of double?
    //theValue.truncate(100);

  for (int c=0; c < table->columnCount(); c++) {
    QTableWidgetItem *tableWidgetItem = table->horizontalHeaderItem(c);
    string fieldName = tableWidgetItem->text().toStdString(); 
    LOG(DEBUG_VERBOSE) << fieldName; 
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
vector<float> *SpreadSheetView::getDataForVariableFromSpreadSheet(int column, string fieldName) {

  vector<float> *data = new vector<float>;

  int c = 0;
  // QTableWidgetItem *tableWidgetItem = table->horizontalHeaderItem(c);
  // TODO; verify fieldName and matches expected name
  LOG(DEBUG_VERBOSE) << "getting data for column " << column << ", " << fieldName;;
  // go through the rows and put the data into a vector
  for (int r = 0; r < table->rowCount(); r++) {
    QTableWidgetItem *tableWidgetItem = table->item(r, c);
    bool ok;
    float value = tableWidgetItem->text().toFloat(&ok);
    if (ok) {
      data->push_back(value);
      LOG(DEBUG_VERBOSE) << value;
    } else {
      QMessageBox::warning(this, tr("HawkEye"),
                           tr("Could not convert to number.\n"),
                           QMessageBox::Abort);
    }
    
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


void SpreadSheetView::selectColor()
{
    QTableWidgetItem *item = table->currentItem();
    QColor col = item ? item->background().color() : table->palette().base().color();
    col = QColorDialog::getColor(col, this);
    if (!col.isValid())
        return;

    QList<QTableWidgetItem*> selected = table->selectedItems();
    if (selected.count() == 0)
        return;

    foreach (QTableWidgetItem *i, selected) {
        if (i)
            i->setBackground(col);
    }

    updateColor(table->currentItem());
}


void SpreadSheetView::selectFont()
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
/*
bool SpreadSheetView::runFunctionDialog(const QString &title,
                                 const QString &c1Text,
                                 const QString &c2Text,
                                 const QString &opText,
                                 const QString &outText,
                                 QString *cell1, QString *cell2, QString *outCell)
{
    QStringList rows, cols;
    for (int c = 0; c < table->columnCount(); ++c)
        cols << QChar('A' + c);
    for (int r = 0; r < table->rowCount(); ++r)
        rows << QString::number(1 + r);

    QDialog functionDialog(this);
    //functionDialog.setWindowTitle(title);

    TextEdit textEditArea();

    QPushButton cancelButton(tr("Cancel"), &functionDialog);
    connect(&cancelButton, &QAbstractButton::clicked, &functionDialog, &QDialog::reject);

    QPushButton okButton(tr("OK"), &functionDialog);
    okButton.setDefault(true);
    connect(&okButton, &QAbstractButton::clicked, &functionDialog, &QDialog::accept);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(&okButton);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(&cancelButton);

    QHBoxLayout *dialogLayout = new QHBoxLayout(&functionDialog);
    dialogLayout->addWidget(&textEditArea);
    dialogLayout->addStretch(1);
    dialogLayout->addItem(buttonsLayout);

    if (addDialog.exec()) {
        QString formula = textEditArea.getText();
        return true;
    }

    return false;
}
*/

bool SpreadSheetView::runInputDialog(const QString &title,
                                 const QString &c1Text,
                                 const QString &c2Text,
                                 const QString &opText,
                                 const QString &outText,
                                 QString *cell1, QString *cell2, QString *outCell)
{
    QStringList rows, cols;
    for (int c = 0; c < table->columnCount(); ++c)
        cols << QChar('A' + c);
    for (int r = 0; r < table->rowCount(); ++r)
        rows << QString::number(1 + r);

    QDialog addDialog(this);
    addDialog.setWindowTitle(title);

    QGroupBox group(title, &addDialog);
    group.setMinimumSize(250, 100);

    QLabel cell1Label(c1Text, &group);
    QComboBox cell1RowInput(&group);
    int c1Row, c1Col;
    SpreadSheetUtils::decode_pos(*cell1, &c1Row, &c1Col);
    cell1RowInput.addItems(rows);
    cell1RowInput.setCurrentIndex(c1Row);

    QComboBox cell1ColInput(&group);
    cell1ColInput.addItems(cols);
    cell1ColInput.setCurrentIndex(c1Col);

    QLabel operatorLabel(opText, &group);
    operatorLabel.setAlignment(Qt::AlignHCenter);

    QLabel cell2Label(c2Text, &group);
    QComboBox cell2RowInput(&group);
    int c2Row, c2Col;
    SpreadSheetUtils::decode_pos(*cell2, &c2Row, &c2Col);
    cell2RowInput.addItems(rows);
    cell2RowInput.setCurrentIndex(c2Row);
    QComboBox cell2ColInput(&group);
    cell2ColInput.addItems(cols);
    cell2ColInput.setCurrentIndex(c2Col);

    QLabel equalsLabel("=", &group);
    equalsLabel.setAlignment(Qt::AlignHCenter);

    QLabel outLabel(outText, &group);
    QComboBox outRowInput(&group);
    int outRow, outCol;
    SpreadSheetUtils::decode_pos(*outCell, &outRow, &outCol);
    outRowInput.addItems(rows);
    outRowInput.setCurrentIndex(outRow);
    QComboBox outColInput(&group);
    outColInput.addItems(cols);
    outColInput.setCurrentIndex(outCol);

    QPushButton cancelButton(tr("Cancel"), &addDialog);
    connect(&cancelButton, &QAbstractButton::clicked, &addDialog, &QDialog::reject);

    QPushButton okButton(tr("OK"), &addDialog);
    okButton.setDefault(true);
    connect(&okButton, &QAbstractButton::clicked, &addDialog, &QDialog::accept);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(&okButton);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(&cancelButton);

    QVBoxLayout *dialogLayout = new QVBoxLayout(&addDialog);
    dialogLayout->addWidget(&group);
    dialogLayout->addStretch(1);
    dialogLayout->addItem(buttonsLayout);

    QHBoxLayout *cell1Layout = new QHBoxLayout;
    cell1Layout->addWidget(&cell1Label);
    cell1Layout->addSpacing(10);
    cell1Layout->addWidget(&cell1ColInput);
    cell1Layout->addSpacing(10);
    cell1Layout->addWidget(&cell1RowInput);

    QHBoxLayout *cell2Layout = new QHBoxLayout;
    cell2Layout->addWidget(&cell2Label);
    cell2Layout->addSpacing(10);
    cell2Layout->addWidget(&cell2ColInput);
    cell2Layout->addSpacing(10);
    cell2Layout->addWidget(&cell2RowInput);

    QHBoxLayout *outLayout = new QHBoxLayout;
    outLayout->addWidget(&outLabel);
    outLayout->addSpacing(10);
    outLayout->addWidget(&outColInput);
    outLayout->addSpacing(10);
    outLayout->addWidget(&outRowInput);

    QVBoxLayout *vLayout = new QVBoxLayout(&group);
    vLayout->addItem(cell1Layout);
    vLayout->addWidget(&operatorLabel);
    vLayout->addItem(cell2Layout);
    vLayout->addWidget(&equalsLabel);
    vLayout->addStretch(1);
    vLayout->addItem(outLayout);

    if (addDialog.exec()) {
        *cell1 = cell1ColInput.currentText() + cell1RowInput.currentText();
        *cell2 = cell2ColInput.currentText() + cell2RowInput.currentText();
        *outCell = outColInput.currentText() + outRowInput.currentText();
        return true;
    }

    return false;
}

void SpreadSheetView::actionSum()
{
    int row_first = 0;
    int row_last = 0;
    int row_cur = 0;

    int col_first = 0;
    int col_last = 0;
    int col_cur = 0;

    QList<QTableWidgetItem*> selected = table->selectedItems();

    if (!selected.isEmpty()) {
        QTableWidgetItem *first = selected.first();
        QTableWidgetItem *last = selected.last();
        row_first = table->row(first);
        row_last = table->row(last);
        col_first = table->column(first);
        col_last = table->column(last);
    }

    QTableWidgetItem *current = table->currentItem();

    if (current) {
        row_cur = table->row(current);
        col_cur = table->column(current);
    }

    QString cell1 = SpreadSheetUtils::encode_pos(row_first, col_first);
    QString cell2 = SpreadSheetUtils::encode_pos(row_last, col_last);
    QString out = SpreadSheetUtils::encode_pos(row_cur, col_cur);

    if (runInputDialog(tr("Sum cells"), tr("First cell:"), tr("Last cell:"),
                       QString("%1").arg(QChar(0x03a3)), tr("Output to:"),
                       &cell1, &cell2, &out)) {
        int row;
        int col;
        SpreadSheetUtils::decode_pos(out, &row, &col);
        table->item(row, col)->setText(tr("sum %1 %2").arg(cell1, cell2));
    }
}

void SpreadSheetView::actionDivide()
{
  //    actionMath_helper(tr("Division"), "/");
    // TODO: get the selected cells
  QItemSelectionModel *select = table->selectionModel();
  
  //  select->hasSelection() //check if has selection
  //  select->selectedRows() // return selected row(s)
  //  select->selectedColumns() // return selected column(s)

  QList<QModelIndex> indexes = select->selectedIndexes();
  // returns rows and columns from the table, zero-based
  cout << "selected (row,column):" << endl;
  for (QModelIndex index : indexes) {
    cout << "(" << index.row() << ", " << index.column() << ")" << endl;
  }    

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

void SpreadSheetView::actionMath_helper(const QString &title, const QString &op)
{
    QString cell1 = "C1";
    QString cell2 = "C2";
    QString out = "C3";

    QTableWidgetItem *current = table->currentItem();
    if (current)
        out = SpreadSheetUtils::encode_pos(table->currentRow(), table->currentColumn());

    if (runInputDialog(title, tr("Cell 1"), tr("Cell 2"), op, tr("Output to:"),
                       &cell1, &cell2, &out)) {
        int row, col;
        SpreadSheetUtils::decode_pos(out, &row, &col);
        table->item(row, col)->setText(tr("%1 %2 %3").arg(op, cell1, cell2));
    }
}

void SpreadSheetView::actionAdd()
{
    actionMath_helper(tr("Addition"), "+");
}

void SpreadSheetView::actionSubtract()
{
    actionMath_helper(tr("Subtraction"), "-");
}

void SpreadSheetView::actionMultiply()
{
    actionMath_helper(tr("Multiplication"), "*");
}

void SpreadSheetView::clear()
{
    foreach (QTableWidgetItem *i, table->selectedItems())
        i->setText("");
}

void SpreadSheetView::setupContextMenu()
{
    addAction(cell_addAction);
    addAction(cell_subAction);
    addAction(cell_mulAction);
    addAction(cell_divAction);
    addAction(cell_sumAction);
    addAction(firstSeparator);
    addAction(colorAction);
    addAction(fontAction);
    addAction(secondSeparator);
    addAction(clearAction);
    setContextMenuPolicy(Qt::ActionsContextMenu);
}


void SpreadSheetView::setupContents()
{
    QColor titleBackground(Qt::lightGray);
    QFont titleFont = table->font();
    titleFont.setBold(true);

}

// request filled by Controller in response to needFieldData 
void SpreadSheetView::fieldDataSent(vector<float> *data, int useless, int c) {
  size_t nPoints = data->size();
  LOG(DEBUG_VERBOSE) << "number of data values = " << nPoints;

      string format = "%g";
      char formattedData[250];

      // TODO: get the globalObject for this field and set the values

      //------
      QTableWidgetItem *headerItem = table->horizontalHeaderItem(c);
      QString fieldName = headerItem->text();
      QJSValue fieldArray = engine.newArray(nPoints);
      QString vectorName = fieldName;

      table->setRowCount(nPoints);
      vector<float> dataVector = *data;
      float *dp = &dataVector[0];
      for (int r=0; r<(int)nPoints; r++) {
      // 752019 for (std::size_t r=0; r<data.size(); r++) {
        //    sprintf(formattedData, format, data[0]);
        sprintf(formattedData, "%g", *dp); // data->at(r));
        LOG(DEBUG_VERBOSE) << "setting " << r << "," << c << "= " << formattedData; 
        table->setItem(r, c, new SpreadSheetItem(formattedData));
        fieldArray.setProperty(r, *dp); // data.at(r));
        dp++;
      }
      LOG(DEBUG_VERBOSE) << "adding vector form " << vectorName.toStdString();
      engine.globalObject().setProperty(vectorName, fieldArray);
      LOG(DEBUG_VERBOSE) << "end adding vector form " << vectorName.toStdString();

}

// request filled by Controller in response to needFieldNames signal
void SpreadSheetView::fieldNamesProvided(vector<string> fieldNames) {

  int useless = 0;

  // fill everything that needs the fieldNames ...

    table->setColumnCount(fieldNames.size());
    LOG(DEBUG_VERBOSE) << "In SpreadSheetView::fieldNamesProvided, there are " << fieldNames.size() << " field namess";

    int c = 0;
    vector<string>::iterator it; 
    for(it = fieldNames.begin(); it != fieldNames.end(); it++) {
      QString the_name(QString::fromStdString(*it));
      LOG(DEBUG_VERBOSE) << *it;
      table->setHorizontalHeaderItem(c, new QTableWidgetItem(the_name));
      // TODO: what about setHorizontalHeaderLabels(const QStringList &labels) instead? would it be faster?
      emit needDataForField(*it, useless, c);
      c += 1;
    }

    // test: adding some missing code
    // TODO: magic number 20 = number of rows
    //table->setItemPrototype(table->item(20 - 1, c - 1));
    table->setItemPrototype(table->item(20 - 1, c - 1));
    table->setItemDelegate(new SpreadSheetDelegate());
    // end test: adding some missing code

    // This section of code makes every data field in volume a variable
    // When the variable name is referenced in the formula bar,
    // the variable name as a string is substituted.
    //     
    // for each field in model (RadxVol)

    /* int someValue = 0;
    for(it = fieldNames.begin(); it != fieldNames.end(); it++) {
      QString fieldName(QString::fromStdString(*it));
      // //    try {
      ////QJSValue objectValue = engine.newQObject(new DataField(*it));
      ////engine.globalObject().setProperty(fieldName, objectValue.property("name"));
      //engine.globalObject().setProperty(fieldName, fieldName);

      QJSValue fieldArray = engine.newArray(20);
      QString vectorName = fieldName; //  + "_VECTOR";
      for (int i=0; i<20; i++) {
        fieldArray.setProperty(i, someValue);
      }
      cout << "adding vector form " << vectorName.toStdString() << endl;
      engine.globalObject().setProperty(vectorName, fieldArray);
      cout << "end adding vector form " << vectorName.toStdString() << endl;

      //someValue += 1;

      // //} catch (Exception ex) {
      // // cerr << "ERROR - problem setting property on field " << *it << endl;
      // //}
    }
    */
    
    //if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG)) { // causes a segmentation fault
    // print the context ...                                                                                                   
      LOG(DEBUG_VERBOSE) << "current QJSEngine context ... after fieldNamesProvided";

      printQJSEngineContext();
      /*
    std::map<QString, QString> currentVariableContext;
    QJSValue theGlobalObject = engine.globalObject();

    QJSValueIterator it2(theGlobalObject);
    while (it2.hasNext()) {
      it2.next();
      QString theValue = it2.value().toString();
      theValue.truncate(100);

      //      LOG(DEBUG_VERBOSE) << it2.name().toStdString() << ": " << theValue;
      qDebug() << it2.name() << ": " << theValue;
      currentVariableContext[it2.name()] = it2.value().toString();
    }
      
      LOG(DEBUG_VERBOSE) << "end current QJSEngine context";
      */
      //}
    

}

void SpreadSheetView::addVariableToSpreadSheet(QString name, QJSValue value) {

  LOG(DEBUG_VERBOSE) << "adding variable to spreadsheet " << name.toStdString();

  string format = "%g";
  // char formattedData[250];

  int variableLength = value.property("length").toInt();
  if ( variableLength > 1) {
    // this is a vector
    LOG(DEBUG_VERBOSE) << "variable is a vector " << name.toStdString();
      QJSValue fieldArray = engine.newArray(variableLength);
      QString vectorName = name;
      for (int i=0; i<variableLength; i++) {
        fieldArray.setProperty(i, value.property(i).toInt());
      }
      cout << "adding vector form " << vectorName.toStdString() << endl;
      engine.globalObject().setProperty(vectorName, fieldArray);
      cout << "end adding vector form " << vectorName.toStdString() << endl;
  }

  if (value.isArray()) {
    //qDebug() << "variable isArray " << name << endl;
    LOG(DEBUG_VERBOSE) << "variable isArray " << name.toStdString();

    /*
  for(it = value.begin(); it != value.end(); it++) {
    QString the_name(QString::fromStdString(*it));
    cerr << *it << endl;
    table->setHorizontalHeaderItem(c, new QTableWidgetItem(the_name));
    vector<double> data = _controller->getData(*it);
    cerr << "number of data values = " << data.size() << endl;
    for (int r=0; r<20; r++) {
      //    sprintf(formattedData, format, data[0]);                                                                                             
      sprintf(formattedData, "%g", data.at(r));
      cerr << "setting " << r << "," << c << "= " << formattedData << endl;
      table->setItem(r, c, new SpreadSheetItem(formattedData));
    }
    c += 1;
    } */
  }
  if (value.isBool()) {
    //qDebug() << "variable isBool " << name << endl;
    LOG(DEBUG_VERBOSE) << "variable isBool " << name.toStdString();
  }
  if (value.isCallable()) {
    //qDebug() << "variable isCallable " << name << endl;
    LOG(DEBUG_VERBOSE) << "variable isCallable " << name.toStdString();
  }
  if (value.isDate()) {
    LOG(DEBUG_VERBOSE) << "variable isDate " << name.toStdString();
    //qDebug() << "variable isDate " << name << endl;
  }
  if (value.isError()) {
    LOG(DEBUG_VERBOSE) << "variable isError " << name.toStdString();
    //qDebug() << "variable isError " << name << endl;
  }
  if (value.isNull()) {
    LOG(DEBUG_VERBOSE) << "variable isNull " << name.toStdString();
    //qDebug() << "variable isNull " << name << endl;
  }
  if (value.isNumber()) {
    //qDebug() << "variable isNumber " << name << endl;
    LOG(DEBUG_VERBOSE) << "variable isNumber " << name.toStdString();
  }
  if (value.isObject()) {
    LOG(DEBUG_VERBOSE) << "variable isObject " << name.toStdString();
    //qDebug() << "variable isObject " << name << endl;
    //    QVector<double> myv = value.property("values");
    //qDebug() << myv.at(0) << ";" << myv.at(1) << endl;
    table->setColumnCount(table->columnCount() + 1);

    int c = table->columnCount() - 1;
    table->setHorizontalHeaderItem(c, new QTableWidgetItem(name));

    QJSValueIterator it(value);
    while (it.hasNext()) {
      it.next();
      LOG(DEBUG_VERBOSE) << it.name().toStdString() << ": " << it.value().toString().toStdString();
    }

    for (int r=0; r<value.property("length").toInt(); r++) {
      //qDebug() << it.name() << ": " << it.value().toString();
      QString valueAsString = value.property(r).toString();
      //      sprintf(formattedData, "%g", value.property(r).toInt());
      //table->setItem(r, c, new SpreadSheetItem(formattedData));
      table->setItem(r,c, new QTableWidgetItem(valueAsString));
    }

  }
  // if (value.isQMetaObject()) {
  //   LOG(DEBUG_VERBOSE) << "variable isQMetaObject " << name.toStdString();
  //   qDebug() << "variable isQMetaObject " << name << endl;
  // }
  if (value.isQObject()) {
    LOG(DEBUG_VERBOSE) << "variable isQObject " << name.toStdString();
    //qDebug() << "variable isQObject " << name << endl;
  }
  if (value.isRegExp()) {
    //qDebug() << "variable isRegExp " << name << endl;
    LOG(DEBUG_VERBOSE) << "variable isRegExp " << name.toStdString();
  }
  if (value.isString()) {
    //qDebug() << "variable isString " << name << endl;
    LOG(DEBUG_VERBOSE) << "variable isString " << name.toStdString();
    table->setColumnCount(table->columnCount() + 1);

    int c = table->columnCount() - 1;
    table->setHorizontalHeaderItem(c, new QTableWidgetItem(name));
    table->setItem(0,c, new QTableWidgetItem(value.toString()));

  }
  if (value.isUndefined()) {
    //qDebug() << "variable isUndefined " << name << endl;
    LOG(DEBUG_VERBOSE) << "variable isUndefined " << name.toStdString();
  }
  if (value.isVariant()) {
    //qDebug() << "variable isVariant " << name << endl;
    LOG(DEBUG_VERBOSE) << "variable isVariant " << name.toStdString();
  }

}

void SpreadSheetView::criticalMessage(std::string message)
{
  QMessageBox::StandardButton reply =
    QMessageBox::critical(this, "QMessageBox::critical()",
                          QString::fromStdString(message),
                          QMessageBox::Ok);
  
  //  QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
  if (reply == QMessageBox::Abort)
    LOG(DEBUG_VERBOSE) << "Abort";
    // criticalLabel->setText("Abort");
  else if (reply == QMessageBox::Retry)
    LOG(DEBUG_VERBOSE) << "Retry";
    // criticalLabel->setText("Retry");
  else
    LOG(DEBUG_VERBOSE) << "Ignore";
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
  LOG(DEBUG_VERBOSE) << "newDataReady ...";
  setupContents();
}

void SpreadSheetView::printQJSEngineContext() {

    // print the context ...                                                                                                   
    LOG(DEBUG_VERBOSE) << "current QJSEngine context ...";

    LOG(DEBUG_VERBOSE) << "pepsi cola";
    /*
    std::map<QString, QString> currentVariableContext;
    QJSValue theGlobalObject = engine.globalObject();

    QJSValueIterator it2(theGlobalObject);
    while (it2.hasNext()) {
      it2.next();
      QString theValue = it2.value().toString();
      theValue.truncate(100);

      LOG(DEBUG_VERBOSE) << it2.name() << ": " << theValue;
      currentVariableContext[it2.name()] = it2.value().toString();
    }
    */
    LOG(DEBUG_VERBOSE) << "end current QJSEngine context";

}
