
#include <stdio.h>
#include <QtWidgets>
#include <QModelIndex>
#include <QJSEngine>
#include <QJSValue>
#include <QJSValueIterator>
#include <vector>
#include <iostream>

//#include "TextEdit.hh"
#include "SpreadSheetView.hh"
#include "spreadsheetdelegate.hh"
#include "spreadsheetitem.hh"
#include "SoloFunctions.hh"
#include "DataField.hh"


using namespace std;

Q_DECLARE_METATYPE(QVector<int>)
Q_DECLARE_METATYPE(QVector<double>)

/*
static QScriptValue getSetFoo(QScriptContext *context, QScriptEngine *engine)
{
  QScriptValue callee = context->callee();
  if (context->argumentCount() == 1) { // writing?
    callee.setProperty("value", context->argument(0));
  }
  return callee.property("value");
}
*/
  /*
  QVector<int> v = qscriptvalue_cast<QVector<int> >(engine->evaluate("[5, 1, 3, 2]"));
qSort(v.begin(), v.end());
QScriptValue a = engine->toScriptValue(v);
qDebug() << a.toString(); // outputs "[1, 2, 3, 5]"     
  */



SpreadSheetView::SpreadSheetView(QWidget *parent)
        : QMainWindow(parent)
{
  cerr << "in SpreadSheetView constructor" << endl;
  //  initSpreadSheet();
  int rows;
  int cols;

  _controller = new SpreadSheetController(this);
  //_controller->open(fileName);
  //vector<std::string> fieldNames = _controller->getFieldNames();
  //cols = displayInfo.getNumFields();
  // vector<std::string> fieldNames = vol.getUniqueFieldNameList();
  cols = 3; // (int) fieldNames.size();
  rows = 20;

  //_volumeData = vol;
    addToolBar(toolBar = new QToolBar());
    formulaInput = new TextEdit(this);
    // formulaInput = new QTextEdit();

    cellLabel = new QLabel(toolBar);
    cellLabel->setMinimumSize(80, 0);

    toolBar->addWidget(cellLabel);
    toolBar->addWidget(formulaInput);
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
    connect(cancelAct, &QAction::triggered, this, &SpreadSheetView::cancelFormulaInput);
    toolBar->addAction(cancelAct);

    QAction *okAct = new QAction(tr("&Ok"), this);
    cancelAct->setStatusTip(tr("Accept changes"));
    connect(okAct, &QAction::triggered, this, &SpreadSheetView::acceptFormulaInput);
    toolBar->addAction(okAct);

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
    table->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    // set the column headers to the data fields
    
    
   
    for (int c=0; c<cols; c++) {
      QString the_name(" ");
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

    setWindowTitle(tr("Spreadsheet"));

    setupSoloFunctions();
}


SpreadSheetView::SpreadSheetView(std::string fileName, QWidget *parent)
        : QMainWindow(parent)
{
  int rows;
  int cols;

  _controller = new SpreadSheetController(this);
  _controller->open(fileName);
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

    setupSoloFunctions();
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


    QMenu *optionsMenu = menuBar()->addMenu(tr("&Options"));
    QMenu *replotMenu = menuBar()->addMenu(tr("&Replot"));

    menuBar()->addSeparator();

    QMenu *aboutMenu = menuBar()->addMenu(tr("&Help"));
    aboutMenu->addAction(aboutSpreadSheet);
}

void SpreadSheetView::updateStatus(QTableWidgetItem *item)
{
    if (item && item == table->currentItem()) {
        statusBar()->showMessage(item->data(Qt::StatusTipRole).toString(), 1000);
        cellLabel->setText(tr("Cell: (%1)").arg(SpreadSheetUtils::encode_pos(table->row(item), table->column(item))));
    }
}

void SpreadSheetView::updateColor(QTableWidgetItem *item)
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
    cerr << "text entered: " << text.toStdString() << endl;

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


void SpreadSheetView::setupSoloFunctions() {
  
    QJSValue myExt = engine.newQObject(new SoloFunctions());
    engine.globalObject().setProperty("cat", myExt.property("cat"));
    engine.globalObject().setProperty("sqrt", myExt.property("sqrt"));
    engine.globalObject().setProperty("REMOVE_AIRCRAFT_MOTION", myExt.property("REMOVE_AIRCRAFT_MOTION"));
    engine.globalObject().setProperty("add", myExt.property("add"));

}

void SpreadSheetView::acceptFormulaInput()
{
    QString text = formulaInput->getText();
    cerr << "text entered: " << text.toStdString() << endl;
    
    //QJSEngine engine;  // moved to class header; member variable
    // ********

    // engine->checkSyntax(text);

    // how to use vectors ...
    //    Q_DECLARE_METATYPE(QVector<int>)
    //qScriptRegisterSequenceMetaType<QVector<int> >(&engine);
    //qScriptRegisterSequenceMetaType<QVector<int> >(&engine);

    /* maybe this part goes into function??
QVector<int> v = qscriptvalue_cast<QVector<int> >(engine->evaluate("[5, 1, 3, 2]"));
qSort(v.begin(), v.end());
QScriptValue a = engine->toScriptValue(v);
qDebug() << a.toString(); // outputs "[1, 2, 3, 5]"
    */
    // end how to use vectors


    //QObject *soloFunctions = new SoloFunctions(_controller);
    //QScriptValue objectValue = engine.newQObject(soloFunctions);
    //var fun = function() { print("999.9"); };
    //engine.globalObject().setProperty("solo", objectValue);
    
    // try to set a global function  ...
    //QScriptValue object = engine.newObject();
    /*    engine.globalObject().setProperty("foo", engine.newFunction(getSetFoo),
		       QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    engine.globalObject().setProperty("bar", engine.newFunction(getSetFoo),
		       QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    */
    /* use like this ...
       foo=4
       foo=foo+2
       foo
       -----
       the result is 6 ... pretty nice!
    */
    //    engine.globalObject().setProperty("REMOVE_AIRCRAFT_MOTION", engine.newFunction(REMOVE_AIRCRAFT_MOTION),
    //		       QScriptValue::PropertyGetter | QScriptValue::PropertySetter);

    //QScriptValue fun = engine.newFunction(REMOVE_AIRCRAFT_MOTION, 1);
    /*
    engine.globalObject().setProperty("REMOVE_AIRCRAFT_MOTION", 
				      engine.newFunction(REMOVE_AIRCRAFT_MOTION, 1),
				      QScriptValue::ReadOnly | QScriptValue::Undeletable);


    engine.globalObject().setProperty("vectorop", 
				      engine.newFunction(VectorOp, 1),
				      QScriptValue::ReadOnly | QScriptValue::Undeletable);
    */

    /* moved to separate method
    QJSValue myExt = engine.newQObject(new SoloFunctions());
    engine.globalObject().setProperty("cat", myExt.property("cat"));
    engine.globalObject().setProperty("sqrt", myExt.property("sqrt"));
    engine.globalObject().setProperty("REMOVE_AIRCRAFT_MOTION", myExt.property("REMOVE_AIRCRAFT_MOTION"));
    engine.globalObject().setProperty("add", myExt.property("add"));
    */
    // end of ... try to set a global function

    // try to set global field values

    /*
    QVector<int> threes(3);
    threes[0]=3;     threes[1]=3;     threes[2]=3;
    QJSValue objectValue = engine.newQObject(new DataField(threes));
    engine.globalObject().setProperty("VEL1", objectValue.property("values"));
    */
    /* moved to open File
    // for each field in model (RadxVol)
    
    //=======
    if (_controller != NULL) {
    vector<string> fieldNames = _controller->getFieldNames();

    int c = 0;
    vector<string>::iterator it;
    for(it = fieldNames.begin(); it != fieldNames.end(); it++) {
      QString fieldName(QString::fromStdString(*it));
      vector<double> data = _controller->getData(*it);
      QVector<double> qData = QVector<double>::fromStdVector(data);
      QJSValue objectValue = engine.newQObject(new DataField(qData));
      engine.globalObject().setProperty(fieldName, objectValue.property("values"));
    }
    }
    // end set global field values
    */
    // *******

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

    int row = table->currentRow();
    int col = table->currentColumn();
    QTableWidgetItem *item = table->item(row, col);
    if (!item)
      table->setItem(row, col, new SpreadSheetItem(result.toString())); // text));
    else
      item->setData(Qt::EditRole, result.toString()); // text);
    table->viewport()->update();
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

void SpreadSheetView::selectColor()
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

/*
void SpreadSheetView::setupContentsBlank()
{
    QColor titleBackground(Qt::lightGray);
    QFont titleFont = table->font();
    titleFont.setBold(true);


    int index;
    index = 0;

    // vector<string> fieldNames = {"one", "two"}; //  
    vector<string> fieldNames = _controller->getFieldNames();

    int c = 0;
    int r = 0;
    vector<string>::iterator it; 
    for(it = fieldNames.begin(); it != fieldNames.end(); it++, c++) {
      QString the_name(QString::fromStdString(*it));
      cerr << *it << endl;
      table->setHorizontalHeaderItem(c, new QTableWidgetItem(the_name));
       
      vector<double> data = _controller->getData(*it);

      cerr << "number of data values = " << data.size() << endl;

      for (r=0; r<20; r++) {
        string format = "%g";
        char formattedData[250];
        //    sprintf(formattedData, format, data[0]);
        sprintf(formattedData, "%g", data[r]); 
        table->setItem(r, c, new SpreadSheetItem(formattedData));
      }
    }
}
*/

void SpreadSheetView::setupContents()
{
    QColor titleBackground(Qt::lightGray);
    QFont titleFont = table->font();
    titleFont.setBold(true);


    int index;
    index = 0;

    vector<string> fieldNames = _controller->getFieldNames();
    table->setColumnCount(fieldNames.size());

    int c = 0;
    vector<string>::iterator it; 
    for(it = fieldNames.begin(); it != fieldNames.end(); it++) {
      QString the_name(QString::fromStdString(*it));
      cerr << *it << endl;
      table->setHorizontalHeaderItem(c, new QTableWidgetItem(the_name));
       
      vector<double> data = _controller->getData(*it);

      cerr << "number of data values = " << data.size() << endl;

      string format = "%g";
      char formattedData[250];

      for (int r=0; r<20; r++) {
        //    sprintf(formattedData, format, data[0]);
        sprintf(formattedData, "%g", data.at(r));
        cerr << "setting " << r << "," << c << "= " << formattedData << endl; 
        table->setItem(r, c, new SpreadSheetItem(formattedData));
      }
      c += 1;
    }


    //======
    
    // for each field in model (RadxVol)
    
    if (_controller != NULL) {
    vector<string> fieldNames = _controller->getFieldNames();

    int c = 0;
    vector<string>::iterator it;
    for(it = fieldNames.begin(); it != fieldNames.end(); it++) {
      QString fieldName(QString::fromStdString(*it));
      vector<double> data = _controller->getData(*it);
      QVector<double> qData = QVector<double>::fromStdVector(data);
      QJSValue objectValue = engine.newQObject(new DataField(qData));
      engine.globalObject().setProperty(fieldName, objectValue.property("values"));
    }
    }
    // end set global field values

    //==========

    /* TODO: each of these columns and data must come from RadxVol
    // column 0
    table->setItem(0, 0, new SpreadSheetItem("Item"));
    table->item(0, 0)->setBackgroundColor(titleBackground);
    table->item(0, 0)->setToolTip("This column shows the purchased item/service");
    table->item(0, 0)->setFont(titleFont);

    table->setItem(1, 0, new SpreadSheetItem("AirportBus"));
    table->setItem(2, 0, new SpreadSheetItem("Flight (Munich)"));
    table->setItem(3, 0, new SpreadSheetItem("Lunch"));
    table->setItem(4, 0, new SpreadSheetItem("Flight (LA)"));
    table->setItem(5, 0, new SpreadSheetItem("Taxi"));
    table->setItem(6, 0, new SpreadSheetItem("Dinner"));
    table->setItem(7, 0, new SpreadSheetItem("Hotel"));
    table->setItem(8, 0, new SpreadSheetItem("Flight (Oslo)"));
    table->setItem(9, 0, new SpreadSheetItem("Total:"));

    table->item(9, 0)->setFont(titleFont);
    table->item(9, 0)->setBackgroundColor(Qt::lightGray);

    // column 1
    table->setItem(0, 1, new SpreadSheetItem("Date"));
    table->item(0, 1)->setBackgroundColor(titleBackground);
    table->item(0, 1)->setToolTip("This column shows the purchase date, double click to change");
    table->item(0, 1)->setFont(titleFont);

    table->setItem(1, 1, new SpreadSheetItem("15/6/2006"));
    table->setItem(2, 1, new SpreadSheetItem("15/6/2006"));
    table->setItem(3, 1, new SpreadSheetItem("15/6/2006"));
    table->setItem(4, 1, new SpreadSheetItem("21/5/2006"));
    table->setItem(5, 1, new SpreadSheetItem("16/6/2006"));
    table->setItem(6, 1, new SpreadSheetItem("16/6/2006"));
    table->setItem(7, 1, new SpreadSheetItem("16/6/2006"));
    table->setItem(8, 1, new SpreadSheetItem("18/6/2006"));

    table->setItem(9, 1, new SpreadSheetItem());
    table->item(9, 1)->setBackgroundColor(Qt::lightGray);

    // column 2
    table->setItem(0, 2, new SpreadSheetItem("Price"));
    table->item(0, 2)->setBackgroundColor(titleBackground);
    table->item(0, 2)->setToolTip("This column shows the price of the purchase");
    table->item(0, 2)->setFont(titleFont);

    table->setItem(1, 2, new SpreadSheetItem("150"));
    table->setItem(2, 2, new SpreadSheetItem("2350"));
    table->setItem(3, 2, new SpreadSheetItem("-14"));
    table->setItem(4, 2, new SpreadSheetItem("980"));
    table->setItem(5, 2, new SpreadSheetItem("5"));
    table->setItem(6, 2, new SpreadSheetItem("120"));
    table->setItem(7, 2, new SpreadSheetItem("300"));
    table->setItem(8, 2, new SpreadSheetItem("1240"));

    table->setItem(9, 2, new SpreadSheetItem());
    table->item(9, 2)->setBackgroundColor(Qt::lightGray);

    // column 3
    table->setItem(0, 3, new SpreadSheetItem("Currency"));
    table->item(0, 3)->setBackgroundColor(titleBackground);
    table->item(0, 3)->setToolTip("This column shows the currency");
    table->item(0, 3)->setFont(titleFont);

    table->setItem(1, 3, new SpreadSheetItem("NOK"));
    table->setItem(2, 3, new SpreadSheetItem("NOK"));
    table->setItem(3, 3, new SpreadSheetItem("EUR"));
    table->setItem(4, 3, new SpreadSheetItem("EUR"));
    table->setItem(5, 3, new SpreadSheetItem("USD"));
    table->setItem(6, 3, new SpreadSheetItem("USD"));
    table->setItem(7, 3, new SpreadSheetItem("USD"));
    table->setItem(8, 3, new SpreadSheetItem("USD"));

    table->setItem(9, 3, new SpreadSheetItem());
    table->item(9,3)->setBackgroundColor(Qt::lightGray);

    // column 4
    table->setItem(0, 4, new SpreadSheetItem("Ex. Rate"));
    table->item(0, 4)->setBackgroundColor(titleBackground);
    table->item(0, 4)->setToolTip("This column shows the exchange rate to NOK");
    table->item(0, 4)->setFont(titleFont);

    table->setItem(1, 4, new SpreadSheetItem("1"));
    table->setItem(2, 4, new SpreadSheetItem("1"));
    table->setItem(3, 4, new SpreadSheetItem("8"));
    table->setItem(4, 4, new SpreadSheetItem("8"));
    table->setItem(5, 4, new SpreadSheetItem("7"));
    table->setItem(6, 4, new SpreadSheetItem("7"));
    table->setItem(7, 4, new SpreadSheetItem("7"));
    table->setItem(8, 4, new SpreadSheetItem("7"));

    table->setItem(9, 4, new SpreadSheetItem());
    table->item(9,4)->setBackgroundColor(Qt::lightGray);

    // column 5
    table->setItem(0, 5, new SpreadSheetItem("NOK"));
    table->item(0, 5)->setBackgroundColor(titleBackground);
    table->item(0, 5)->setToolTip("This column shows the expenses in NOK");
    table->item(0, 5)->setFont(titleFont);

    table->setItem(1, 5, new SpreadSheetItem("* C2 E2"));
    table->setItem(2, 5, new SpreadSheetItem("* C3 E3"));
    table->setItem(3, 5, new SpreadSheetItem("* C4 E4"));
    table->setItem(4, 5, new SpreadSheetItem("* C5 E5"));
    table->setItem(5, 5, new SpreadSheetItem("* C6 E6"));
    table->setItem(6, 5, new SpreadSheetItem("* C7 E7"));
    table->setItem(7, 5, new SpreadSheetItem("* C8 E8"));
    table->setItem(8, 5, new SpreadSheetItem("* C9 E9"));

    table->setItem(9, 5, new SpreadSheetItem("sum F2 F9"));
    table->item(9,5)->setBackgroundColor(Qt::lightGray);
    */
}

void SpreadSheetView::addVariableToSpreadSheet(QString name, QJSValue value) {


    string format = "%g";
    char formattedData[250];


  if (value.isArray()) {
    qDebug() << "variable isArray " << name << endl;
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
    qDebug() << "variable isBool " << name << endl;
  }
  if (value.isCallable()) {
    qDebug() << "variable isCallable " << name << endl;
  }
  if (value.isDate()) {
    qDebug() << "variable isDate " << name << endl;
  }
  if (value.isError()) {
    qDebug() << "variable isError " << name << endl;
  }
  if (value.isNull()) {
    qDebug() << "variable isNull " << name << endl;
  }
  if (value.isNumber()) {
    qDebug() << "variable isNumber " << name << endl;
  }
  if (value.isObject()) {
    qDebug() << "variable isObject " << name << endl;
    //    QVector<double> myv = value.property("values");
    //qDebug() << myv.at(0) << ";" << myv.at(1) << endl;
    table->setColumnCount(table->columnCount() + 1);

    int c = table->columnCount() - 1;
    table->setHorizontalHeaderItem(c, new QTableWidgetItem(name));

    QJSValueIterator it(value);
    while (it.hasNext()) {
      it.next();
      qDebug() << it.name() << ": " << it.value().toString();
    }

    for (int r=0; r<value.property("length").toInt(); r++) {
      //qDebug() << it.name() << ": " << it.value().toString();
      QString valueAsString = value.property(r).toString();
      //      sprintf(formattedData, "%g", value.property(r).toInt());
      //table->setItem(r, c, new SpreadSheetItem(formattedData));
      table->setItem(r,c, new QTableWidgetItem(valueAsString));
    }

  }
  if (value.isQMetaObject()) {
    qDebug() << "variable isQMetaObject " << name << endl;
  }
  if (value.isQObject()) {
    qDebug() << "variable isQObject " << name << endl;
  }
  if (value.isRegExp()) {
    qDebug() << "variable isRegExp " << name << endl;
  }
  if (value.isString()) {
    qDebug() << "variable isString " << name << endl;
  }
  if (value.isUndefined()) {
    qDebug() << "variable isUndefined " << name << endl;
  }
  if (value.isVariant()) {
    qDebug() << "variable isVariant " << name << endl;
  }

}


void SpreadSheetView::showAbout()
{
    QMessageBox::about(this, "About Spreadsheet", htmlText);
}


void SpreadSheetView::print()
{
  cerr << "ERROR - Print not supported" << endl;
  /*
#if QT_CONFIG(printpreviewdialog)
    QPrinter printer(QPrinter::ScreenResolution);
    QPrintPreviewDialog dlg(&printer);
    PrintView view;
    view.setModel(table->model());
    connect(&dlg, &QPrintPreviewDialog::paintRequested, &view, &PrintView::print);
    dlg.exec();
#endif
  */
}

QString SpreadSheetView::open()
{
  QString fileName;
  fileName = QFileDialog::getOpenFileName(this,
     tr("Open Image"), "/tmp", tr("Sweep Files (*.*)"));


  // signal the controller to read new data file

  _controller->open(fileName.toStdString());

  // load new data
  newDataReady();

  return fileName;

}

void  SpreadSheetView::newDataReady()
{

  setupContents();
}
