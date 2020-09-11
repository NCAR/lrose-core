
#include <stdio.h>
#include <QtWidgets>
#include <QMessageBox>
#include <QModelIndex>
#include <QJSEngine>
#include <QJSValue>
#include <QJSValueIterator>
#include <QCheckBox>
#include <vector>
#include <iostream>
#include <toolsa/LogStream.hh>
//#include "TextEdit.hh"
#include "ScriptEditorView.hh"
//#include "ScriptEditorDelegate.hh"
//#include "ScriptEditorItem.hh"
#include "SoloFunctionsController.hh"
#include "DataField.hh"


using namespace std;

Q_DECLARE_METATYPE(QVector<int>)
Q_DECLARE_METATYPE(QVector<double>)


// ScriptEditorView will emit signals that are followed by the controller
//
//


  ScriptEditorView::ScriptEditorView(QWidget *parent)
  : QMainWindow(parent)
{
  LOG(DEBUG) << "in ScriptEditorView constructor";
  //  initScriptEditor();
  int rows;
  int cols;

  cols = 3; // (int) fieldNames.size();
  rows = 200;

    addToolBar(toolBar = new QToolBar());
    formulaInput = new TextEdit(this);
    // get the font to determine height of one row
    QFontMetrics m(formulaInput->font());
    int rowHeight = m.lineSpacing();
    //formulaInput->setFixedHeight(3*rowHeight);

    formulaInputForEachRay = new TextEdit(this);
    // get the font to determine height of one row
    QFontMetrics m2(formulaInputForEachRay->font());
    //int rowHeight = m2.lineSpacing();
    //formulaInputForEachRay->setFixedHeight(3*rowHeight);

    cellLabel = new QLabel(toolBar);
    //cellLabel->setMaximumSize(50, 10);
    //cellLabel->setMinimumSize(80, 10);


    QHBoxLayout *scriptEditLayout = new QHBoxLayout();
    QVBoxLayout *forEachLayout = new QVBoxLayout();
    QVBoxLayout *oneTimeOnlyLayout = new QVBoxLayout();

    toolBar->addWidget(cellLabel);
    oneTimeOnlyLayout->addWidget(new QLabel("One Time Only"));
    oneTimeOnlyLayout->addWidget(formulaInput);
    forEachLayout->addWidget(new QLabel("For Each Ray"));
    forEachLayout->addWidget(formulaInputForEachRay);


    QWidget *oneTimeWidget = new QWidget();
    oneTimeWidget->setLayout(oneTimeOnlyLayout);
    QWidget *forEachWidget = new QWidget();
    forEachWidget->setLayout(forEachLayout);
    
    int actionFontSize = 18;
    QVBoxLayout *actionLayout = new QVBoxLayout();
    QAction *cancelAct = new QAction(tr("&Cancel"), this);
    QFont font = cancelAct->font();
    font.setPointSize(actionFontSize);
    cancelAct->setFont(font);
    cancelAct->setStatusTip(tr("cancel changes"));
    // cancelAct->setIcon(QIcon(":/images/cancel_x.png"));
    connect(cancelAct, &QAction::triggered, this, &ScriptEditorView::cancelFormulaInput);
    toolBar->addAction(cancelAct);

    QAction *okAct = new QAction(tr("&Run"), this);
    font = okAct->font();
    font.setPointSize(actionFontSize);
    okAct->setFont(font);
    okAct->setStatusTip(tr("run script"));
    //okAct->setIcon(QIcon(":/images/ok_check.png"));
    connect(okAct, &QAction::triggered, this, &ScriptEditorView::acceptFormulaInput);
    toolBar->addAction(okAct);

    QAction *openFileAct = new QAction(tr("&Open"), this);
    font = openFileAct->font();
    font.setPointSize(actionFontSize);
    openFileAct->setFont(font);
    openFileAct->setStatusTip(tr("get script from file"));
    connect(openFileAct, &QAction::triggered, this, &ScriptEditorView::openScriptFile);
    toolBar->addAction(openFileAct);

    QAction *saveFileAct = new QAction(tr("&Save"), this);
    font = saveFileAct->font();
    font.setPointSize(actionFontSize);
    saveFileAct->setFont(font);
    saveFileAct->setStatusTip(tr("save script to file"));
    connect(saveFileAct, &QAction::triggered, this, &ScriptEditorView::saveScriptFile);
    toolBar->addAction(saveFileAct);

/*
    QAction *applyAct = new QAction(tr("&Apply"), this);
    applyAct->setStatusTip(tr("Apply changes to display"));
    applyAct->setIcon(QIcon(":/images/apply.png"));
    QFont applyFont = applyAct->font();
    applyFont.setBold(true);
    applyFont.setPointSize(actionFontSize);
    applyAct->setFont(applyFont);
    connect(applyAct, &QAction::triggered, this, &ScriptEditorView::applyChanges);
    toolBar->addAction(applyAct);
*/

    useBoundaryWidget = new QCheckBox("Use Boundary", this);
    useBoundaryWidget->setChecked(true);

    //scriptEditLayout->addWidget(actionWidget);
    scriptEditLayout->addWidget(forEachWidget);
    // scriptEditLayout->addWidget(oneTimeWidget);
    scriptEditLayout->addWidget(useBoundaryWidget);
    QWidget *scriptEditWidget = new QWidget();
    scriptEditWidget->setLayout(scriptEditLayout);


    createActions();
    LOG(DEBUG) << "Action created\n";
    //updateColor(0);
    //LOG(DEBUG) << "update Color\n";
    setupMenuBar();
    LOG(DEBUG) << "setupMenuBar\n";
    //setupContentsBlank();
    //cout << "setupContentsBlank\n";
    setupContextMenu();
    LOG(DEBUG) << "setupContextMenu";
    //setCentralWidget(table);
    LOG(DEBUG) << "setCentralWidgets";

    QPushButton *firstSweepButton = new QPushButton("First Sweep");
    QPushButton *lastSweepButton = new QPushButton("Last Sweep");
    TextEdit *dateTimeFirstSweepInput = new TextEdit(this);
    TextEdit *dateTimeLastSweepInput = new TextEdit(this);
    QFontMetrics m3(dateTimeFirstSweepInput->font());
    int rowHeight3 = m3.lineSpacing();
    dateTimeFirstSweepInput->setFixedHeight(1.5*rowHeight3);
    dateTimeLastSweepInput->setFixedHeight(1.5*rowHeight3);


    QHBoxLayout *startTimeLayout = new QHBoxLayout();
    startTimeLayout->addWidget(new QLabel("Start Time"));
    startTimeLayout->addWidget(dateTimeFirstSweepInput);
    startTimeLayout->addWidget(firstSweepButton);
    QHBoxLayout *stopTimeLayout = new QHBoxLayout();
    stopTimeLayout->addWidget(new QLabel("Stop Time"));
    stopTimeLayout->addWidget(dateTimeLastSweepInput);
    stopTimeLayout->addWidget(lastSweepButton);

    QWidget *startTimeWidget = new QWidget();
    QWidget *stopTimeWidget = new QWidget();
    startTimeWidget->setLayout(startTimeLayout);
    stopTimeWidget->setLayout(stopTimeLayout);
    
    //    QVBoxLayout *startStopTimeLayout = new QVBoxLayout();
    //startStopTimeLayout->addWidget(startTimeWidget);
    //startStopTimeLayout->addWidget(stopTimeWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(scriptEditWidget);
    mainLayout->addWidget(startTimeWidget);
    mainLayout->addWidget(stopTimeWidget);

    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);

    statusBar();

    QString title("Script Editor");

    setWindowTitle(title);

    LOG(DEBUG) << "after setup";

}


void ScriptEditorView::init()
{
  LOG(DEBUG) << "emitting signal to get field names";
  //  emit a signal to the controller to get the data for display
  emit needFieldNames();
  
  //setupSoloFunctions();
}

void ScriptEditorView::createActions()
{
}

void ScriptEditorView::setupMenuBar()
{
}

void ScriptEditorView::updateStatus(QTableWidgetItem *item)
{
  //    if (item && item == table->currentItem()) {
  //      statusBar()->showMessage(item->data(Qt::StatusTipRole).toString(), 1000);
        //cellLabel->setText(tr("Cell: (%1)").arg(ScriptEditorUtils::encode_pos(table->row(item), table->column(item))));
  //  }
}


void ScriptEditorView::updateTextEdit(QTableWidgetItem *item)
{
  //    if (item != table->currentItem())
  //      return;
    if (item)
        formulaInput->setText(item->data(Qt::EditRole).toString());
    else
        formulaInput->clear();
}

void ScriptEditorView::returnPressed()
{
    QString scriptOneTimeOnly = formulaInput->getText();
    LOG(DEBUG) << "OneTimeOnly text entered: " << scriptOneTimeOnly.toStdString();

    QString scriptForEachRay = formulaInputForEachRay->getText();
    LOG(DEBUG) << "ForEachRay text entered: " << scriptForEachRay.toStdString();

    //    int row = table->currentRow();
    //int col = table->currentColumn();
    //QTableWidgetItem *item = table->item(row, col);
    //if (!item)
    //    table->setItem(row, col, new ScriptEditorItem(text));
    //else
    //    item->setData(Qt::EditRole, text);
    //table->viewport()->update();
}

float  ScriptEditorView::myPow()
{
  return(999.9);
}




void ScriptEditorView::openScriptFile() {
  //QString fileNameQ = QFileDialog::getOpenFileName(this,
  //  tr("Open Script from File"), ".", tr("Script Files (*.*)"));
  //string fileName = fileNameQ.toStdString();
  //LOG(DEBUG) << "fileName is " << fileName;

  //QString fileContent("Script Content Here");

/*
  auto fileContentReady = [](const QString &fileName, const QByteArray &fileContent) {
    if (fileName.isEmpty()) {
        // No file was selected
    } else {
        // Use fileName and fileContent
    }
  };
*/
  // takes zero arguments and returns nothing
  std::function<void(const QString &, const QByteArray &)> fileContentReady;
  fileContentReady = [this](const QString &fileName, const QByteArray &fileContent ) {
    if (fileName.isEmpty()) {
        // No file was selected
    } else {
        // Use fileName and fileContent
      this->formulaInputForEachRay->setText(fileContent);
    }
  };
  QFileDialog::getOpenFileContent("Images (*.png *.xpm *.jpg)",  fileContentReady);

  // formulaInputForEachRay->setText(fileContent);

}

void ScriptEditorView::saveScriptFile() {
  QString fileNameQ = QFileDialog::getSaveFileName(this,
    tr("Save Script to File"), ".", tr("Script Files (*.*)"));
  string fileName = fileNameQ.toStdString();
  LOG(DEBUG) << "fileName is " << fileName;

  QString forEachRayScript = formulaInputForEachRay->getText();

  //QFileDialog::saveFileContent(forEachRayScript.toUtf8(), fileNameQ);
  QFile file(fileNameQ); 
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "title", "Cannot write to file");
  } else {
    QTextStream out(&file);
    out << forEachRayScript.toUtf8();
    // out.close();
  }

}

void ScriptEditorView::applyChanges()
{
  // TODO: send a list of the variables in the GlobalObject of the
  // QJSEngine to the model (via the controller?)
  // emit applyVolumeEdits();
}

void ScriptEditorView::acceptFormulaInput()
{
    QString oneTimeOnlyScript = formulaInput->getText();
    cerr << "OneTimeOnly text entered: " << oneTimeOnlyScript.toStdString() << endl;

    QString forEachRayScript = formulaInputForEachRay->getText();
    cerr << "ForEachRay text entered: " << forEachRayScript.toStdString() << endl;
    
    bool useBoundary = useBoundaryWidget->isChecked(); 

    // Send the scripts to the controller for processing
    try {
      emit runOneTimeOnlyScript(oneTimeOnlyScript);
      emit runForEachRayScript(forEachRayScript, useBoundary);
    /*
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

      LOG(DEBUG) << it.name().toStdString() << ": " << theValue.toStdString(); // it.value().toString().truncate(100);
      currentVariableContext[it.name()] = it.value().toString();
    }
      // ======
    try {
      QJSValue result = engine.evaluate(oneTimeOnly);
      if (result.isError()) {
        QString message;
        message.append(result.toString());
        message.append(" on line number ");
        message.append(result.property("lineNumber").toString());
        criticalMessage(message.toStdString()); 
        LOG(DEBUG)
	  << "Uncaught exception at line"
  	  << result.property("lineNumber").toInt()
	  << ":" << result.toString().toStdString();
      } else {

	LOG(DEBUG) << " the result is " << result.toString().toStdString();

	if (result.isArray()) {
	  cerr << " the result is an array\n"; 
	} 
        if (result.isNumber()) {
	  cerr << " the result is a number " << result.toString().toStdString() << endl;
          //setSelectionToValue(result.toString());
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
	  LOG(DEBUG) << it2.name().toStdString() << ": " << theValue.toStdString();
	  if (currentVariableContext.find(it2.name()) == currentVariableContext.end()) {
	    // we have a newly defined variable
	    LOG(DEBUG) << "NEW VARIABLE " << it2.name().toStdString() <<  ": " << theValue.toStdString();
	    addVariableToScriptEditor(it2.name(), it2.value());
	  }
	}
	// ======
      }

    */
    } catch (const std::exception& ex) {
      criticalMessage(ex.what());
    } catch (const std::string& ex) {
      criticalMessage(ex);
    } catch (const char*  ex) {
      criticalMessage(ex);
    } catch (...) {
      criticalMessage("Error occurred during evaluation");
    }

}

void ScriptEditorView::cancelFormulaInput()
{
  // TODO: what action should cancel be?
  // close the dialog?
  // clear the script text edit?
  //
  //QString text = formulaInput->getText();
  cerr << "cancelling formula changes" << endl;
    /*
    int row = table->currentRow();
    int col = table->currentColumn();
    QTableWidgetItem *item = table->item(row, col);
    if (!item)
        table->setItem(row, col, new ScriptEditorItem(text));
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
vector<string> *ScriptEditorView::getVariablesFromScriptEditor() {

  vector<string> *names = new vector<string>;

  
  // try iterating over the properties of the globalObject to find new variables
  //QJSValue newGlobalObject = engine.globalObject();

  //QJSValueIterator it2(newGlobalObject);
  //while (it2.hasNext()) {
  //  it2.next();
    //QString theValue = it2.value().toString(); // TODO: this could be the bottle neck; try sending list of double?
    //theValue.truncate(100);

  /* TODO: we'll need to get the list of field variables from somewhere ... 
  for (int c=0; c < table->columnCount(); c++) {
    QTableWidgetItem *tableWidgetItem = table->horizontalHeaderItem(c);
    string fieldName = tableWidgetItem->text().toStdString(); 
    LOG(DEBUG) << fieldName; 
    //if (currentVariableContext.find(it2.name()) == currentVariableContext.end()) {
    //  // we have a newly defined variable
    //  qDebug() << "NEW VARIABLE " << it2.name() <<  ": " << theValue; // it2.value().toString().truncate(100);
    //  addVariableToScriptEditor(it2.name(), it2.value());
    //}
    names->push_back(fieldName);
  }
  */
  names->push_back("TODO: getFieldNames");
  return names;
}


void ScriptEditorView::notImplementedMessage() {
      QMessageBox::information(this, "Not Implemented", "Not Implemented");
}

/*
void ScriptEditorView::actionDisplayCellValues()
{
  notImplementedMessage();
}
void ScriptEditorView::actionDisplayRayInfo()
{
  notImplementedMessage();
}
void ScriptEditorView::actionDisplayMetadata()
{
  notImplementedMessage();
}
void ScriptEditorView::actionDisplayEditHist()
{
  notImplementedMessage();
}
*/

/*
void ScriptEditorView::clear()
{
    foreach (QTableWidgetItem *i, table->selectedItems())
        i->setText("");
}
*/

void ScriptEditorView::setupContextMenu()
{
  //    setContextMenuPolicy(Qt::ActionsContextMenu);
}

/*
void ScriptEditorView::setupContents()
{
    QColor titleBackground(Qt::lightGray);
    QFont titleFont = table->font();
    titleFont.setBold(true);

}
*/

 /*
// request filled by Controller in response to needFieldData 
void ScriptEditorView::fieldDataSent(vector<float> *data, int useless, int c) {
  size_t nPoints = data->size();
  LOG(DEBUG) << "number of data values = " << nPoints;

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
      for (int r=0; r<nPoints; r++) {
      // 752019 for (std::size_t r=0; r<data.size(); r++) {
        //    sprintf(formattedData, format, data[0]);
        sprintf(formattedData, "%g", *dp); // data->at(r));
        LOG(DEBUG) << "setting " << r << "," << c << "= " << formattedData; 
        table->setItem(r, c, new ScriptEditorItem(formattedData));
        fieldArray.setProperty(r, *dp); // data.at(r));
        dp++;
      }
      LOG(DEBUG) << "adding vector form " << vectorName.toStdString();
      engine.globalObject().setProperty(vectorName, fieldArray);
      LOG(DEBUG) << "end adding vector form " << vectorName.toStdString();

}
 */

 
// request filled by Controller in response to needFieldNames signal
void ScriptEditorView::fieldNamesProvided(vector<string> fieldNames) {

  int useless = 0;

  // fill everything that needs the fieldNames ...

    // This section of code makes every data field in volume a variable
    // When the variable name is referenced in the formula bar,
    // the variable name as a string is substituted.
    //     
    // for each field in model (RadxVol)

    int someValue = 0;
    vector<string>::iterator it; 
    /*
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
    //      LOG(DEBUG) << "current QJSEngine context ... after fieldNamesProvided";

    //printQJSEngineContext();
}


 /*
void ScriptEditorView::addVariableToScriptEditor(QString name, QJSValue value) {

  LOG(DEBUG) << "adding variable to spreadsheet " << name.toStdString();

  string format = "%g";
  // char formattedData[250];

  int variableLength = value.property("length").toInt();
  if ( variableLength > 1) {
    // this is a vector
    LOG(DEBUG) << "variable is a vector " << name.toStdString();
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
    LOG(DEBUG) << "variable isArray " << name.toStdString();

  }
  if (value.isBool()) {
    //qDebug() << "variable isBool " << name << endl;
    LOG(DEBUG) << "variable isBool " << name.toStdString();
  }
  if (value.isCallable()) {
    //qDebug() << "variable isCallable " << name << endl;
    LOG(DEBUG) << "variable isCallable " << name.toStdString();
  }
  if (value.isDate()) {
    LOG(DEBUG) << "variable isDate " << name.toStdString();
    //qDebug() << "variable isDate " << name << endl;
  }
  if (value.isError()) {
    LOG(DEBUG) << "variable isError " << name.toStdString();
    //qDebug() << "variable isError " << name << endl;
  }
  if (value.isNull()) {
    LOG(DEBUG) << "variable isNull " << name.toStdString();
    //qDebug() << "variable isNull " << name << endl;
  }
  if (value.isNumber()) 
    //qDebug() << "variable isNumber " << name << endl;
    LOG(DEBUG) << "variable isNumber " << name.toStdString();
  }
  if (value.isObject()) {
    LOG(DEBUG) << "variable isObject " << name.toStdString();
    //qDebug() << "variable isObject " << name << endl;
    //    QVector<double> myv = value.property("values");
    //qDebug() << myv.at(0) << ";" << myv.at(1) << endl;
  }
  // if (value.isQMetaObject()) {
  //   LOG(DEBUG) << "variable isQMetaObject " << name.toStdString();
  //   qDebug() << "variable isQMetaObject " << name << endl;
  // }
  if (value.isQObject()) {
    LOG(DEBUG) << "variable isQObject " << name.toStdString();
    //qDebug() << "variable isQObject " << name << endl;
  }
  if (value.isRegExp()) {
    //qDebug() << "variable isRegExp " << name << endl;
    LOG(DEBUG) << "variable isRegExp " << name.toStdString();
  }
  if (value.isString()) {
    //qDebug() << "variable isString " << name << endl;
    LOG(DEBUG) << "variable isString " << name.toStdString();
  }
  if (value.isUndefined()) {
    //qDebug() << "variable isUndefined " << name << endl;
    LOG(DEBUG) << "variable isUndefined " << name.toStdString();
  }
  if (value.isVariant()) {
    //qDebug() << "variable isVariant " << name << endl;
    LOG(DEBUG) << "variable isVariant " << name.toStdString();
  }

}
 */

void ScriptEditorView::criticalMessage(std::string message)
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


/*
void ScriptEditorView::printQJSEngineContext() {

    // print the context ...                                                                                                   
    LOG(DEBUG) << "current QJSEngine context ...";

    // LOG(DEBUG) << "pepsi cola";
    
    std::map<QString, QString> currentVariableContext;
    QJSValue theGlobalObject = engine.globalObject();

    QJSValueIterator it2(theGlobalObject);
    while (it2.hasNext()) {
      it2.next();
      QString theValue = it2.value().toString();
      theValue.truncate(100);

      LOG(DEBUG) << it2.name().toStdString() << ": " << theValue.toStdString();
      currentVariableContext[it2.name()] = it2.value().toString();
    }
    
    LOG(DEBUG) << "end current QJSEngine context";

}
*/
