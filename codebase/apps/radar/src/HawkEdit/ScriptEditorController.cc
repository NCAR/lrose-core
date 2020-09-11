
//
// ScriptEditorController converts Model data to format for display in View
//  mostly from float, int to string based on format requested.
//

#include <stdio.h>
#include <map>
#include <QtWidgets>
#include <QJSEngine>
#include <QJSValue>
#include <QJSValueIterator>

#include "ScriptEditorController.hh"
#include "ScriptEditorModel.hh"
#include "SoloFunctionsController.hh"
#include <toolsa/LogStream.hh>

ScriptEditorController::ScriptEditorController(ScriptEditorView *view)
{
  // int rows;
  // int cols;


  _currentView = view;
  _currentModel = new ScriptEditorModel();

  //  functionsModel = new SoloFunctionsModel(_currentModel);

    // connect controller slots to model signals 

    // connect model signals to controller slots 
  /*
    connect(table, &QTableWidget::currentItemChanged,
            this, &ScriptEditorController::updateStatus);
    connect(table, &QTableWidget::currentItemChanged,
            this, &ScriptEditorController::updateColor);
    connect(table, &QTableWidget::currentItemChanged,
            this, &ScriptEditorController::updateLineEdit);
    connect(table, &QTableWidget::itemChanged,
            this, &ScriptEditorController::updateStatus);
    connect(formulaInput, &QLineEdit::returnPressed, this, &ScriptEditorController::returnPressed);
    connect(table, &QTableWidget::itemChanged,
            this, &ScriptEditorController::updateLineEdit);
  */
}


ScriptEditorController::ScriptEditorController(ScriptEditorView *view, ScriptEditorModel *model)
{
  // int rows;
  // int cols;


  _currentView = view;

  _currentModel = model;

  //  functionsModel = new SoloFunctionsModel(_currentModel);
  _soloFunctionsController = new SoloFunctionsController(_currentModel->_vol);

  setupSoloFunctions(_soloFunctionsController);
  //setupFieldArrays();

  // connect view signals to controller slots

  connect(_currentView, SIGNAL(needFieldNames()), this, SLOT(needFieldNames()));
  //connect(_currentView, SIGNAL(needDataForField(string, int, int)), 
  //	  this, SLOT(needDataForField(string, int, int)));
  connect(_currentView, SIGNAL(applyVolumeEdits()), 
	  this, SLOT(getVolumeChanges()));


  connect(_currentView, SIGNAL(runOneTimeOnlyScript(QString)),
	  this, SLOT(runOneTimeOnlyScript(QString)));
  connect(_currentView, SIGNAL(runForEachRayScript(QString, bool)),
	  this, SLOT(runForEachRayScript(QString, bool)));


    // connect controller slots to model signals 

    // connect model signals to controller slots 
  /*
    connect(table, &QTableWidget::currentItemChanged,
            this, &ScriptEditorController::updateStatus);
    connect(table, &QTableWidget::currentItemChanged,
            this, &ScriptEditorController::updateColor);
    connect(table, &QTableWidget::currentItemChanged,
            this, &ScriptEditorController::updateLineEdit);
    connect(table, &QTableWidget::itemChanged,
            this, &ScriptEditorController::updateStatus);
    connect(formulaInput, &QLineEdit::returnPressed, this, &ScriptEditorController::returnPressed);
    connect(table, &QTableWidget::itemChanged,
            this, &ScriptEditorController::updateLineEdit);
  */
}



vector<string>  ScriptEditorController::getFieldNames()
{
  vector<string> names = _currentModel->getFields();
  cout << " In ScriptEditorController::getFieldNames, there are " << names.size() << " field names" << endl;
  return names;
}

/*
vector<float> *ScriptEditorController::getData(string fieldName)
{

  LOG(DEBUG) << "getting values for " << fieldName;

  
  //return _currentModel->getData(fieldName);
  
  //  vector<float> ScriptEditorModel::getData(string fieldName)
  vector<float> *data = _currentModel->getData(fieldName);

  LOG(DEBUG) << " found " << data->size() << " data values ";

  return data;
 
}
*/
/*
void ScriptEditorController::setData(string fieldName, vector<float> *data)
{
  LOG(DEBUG) << "setting values for " << fieldName;
  _currentModel->setData(fieldName, data);
}
*/

void  ScriptEditorController::needFieldNames() {
  fieldNamesProvided(getFieldNames());
}

/*
void  ScriptEditorController::needDataForField(string fieldName, int r, int c) {

  _currentView->fieldDataSent(getData(fieldName), r, c);
}
*/


void ScriptEditorController::getVolumeChanges() {

  LOG(DEBUG) << "enter";
  //  QStringList *fieldNames = _currentView->getVariablesFromScriptEditor();
  //int column = 0;
  //for(vector<string>::iterator s = fields->begin(); s != fields->end(); s++) {
    //vector<float> *data = _currentView->getDataForVariableFromScriptEditor(column, *s);
    //setData(*s, data);
    //column++;
  //}
  //volumeUpdated(fieldNames);
  LOG(DEBUG) << "exit";
}

void ScriptEditorController::volumeUpdated(QStringList newFieldNames) {
  //QStringList newFieldNamesFAKE = {"VEL_xyz"};
  //  QStringList newFieldNamesFAKE = {"DBZ", "WIDTH", "ZDR"};
  emit scriptChangedVolume(newFieldNames); // FAKE); // _currentModel->getVolume());
}


void ScriptEditorController::open(string fileName)
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

void ScriptEditorController::setupBoundaryArray() {
  const vector<bool> *boundaryMaskForRay = _soloFunctionsController->GetBoundaryMask();

  QJSValue fieldArray = engine.newArray(boundaryMaskForRay->size());
  QString vectorName("BOUNDARY");    


  //std::vector<float> fieldData = getData(fieldName);
  vector<bool>::const_iterator itData;
  int idx = 0;
  for (itData=boundaryMaskForRay->begin(); itData != boundaryMaskForRay->end(); ++itData) {
    if (*itData)
      fieldArray.setProperty(idx, true);
    else 
      fieldArray.setProperty(idx, false);
    idx += 1;
  }

  engine.globalObject().setProperty(vectorName, fieldArray);
  LOG(DEBUG) << "adding vector form " << vectorName.toStdString();
  //boundaryMaskForRay->clear();
  delete boundaryMaskForRay;
} 

void ScriptEditorController::setupFieldArrays() {
    
    vector<string>::iterator it;

    for(it = initialFieldNames.begin(); it != initialFieldNames.end(); it++) {
      QString fieldName(QString::fromStdString(*it));

          // ===== set field to array of numbers; begin =====
            // get the field data, for the current (sweep, ray)
      const vector<float> *fieldData = _soloFunctionsController->getData(*it);  

      QJSValue fieldArray = engine.newArray(fieldData->size());
      QString vectorName = fieldName; //  + "_v";    


      //std::vector<float> fieldData = getData(fieldName);
      vector<float>::const_iterator itData;
      int idx = 0;
      for (itData=fieldData->begin(); itData != fieldData->end(); ++itData) {
        fieldArray.setProperty(idx, *itData);
        idx += 1;
      }

      //for (int i=0; i<fieldData.size(); i++) {
      //  fieldArray.setProperty(i, fieldData.at(i));
      //}
      LOG(DEBUG) << "adding vector form " << vectorName.toStdString();
      engine.globalObject().setProperty(vectorName, fieldArray);
      LOG(DEBUG) << "end adding vector form " << vectorName.toStdString();
      // ===== set field to array of numbers; end ====
    } 
}

void ScriptEditorController::_addFieldNameVectorsToContext(vector<string> &fieldNames, 
  std::map<QString, QString> *currentVariableContextPtr) {
  std::map<QString, QString> currentVariableContext = *currentVariableContextPtr;
  vector<string>::iterator nameItr;
  for (nameItr = fieldNames.begin(); nameItr != fieldNames.end(); ++nameItr) {
    QString vectorName(nameItr->c_str());
    //vectorName.append("_v");
    currentVariableContext[vectorName] = vectorName;
  }

}

bool ScriptEditorController::notDefined(QString &fieldName, std::map<QString, QString> &previousVariableContext) {
      return (previousVariableContext.find(fieldName) == previousVariableContext.end());
}

void ScriptEditorController::saveFieldArrays(std::map<QString, QString> &previousVariableContext) {
  // go through the context and save any new fields (as arrays)

  LOG(DEBUG) << "current QJSEngine context ...";

  std::map<QString, QString> currentVariableContext;
  QJSValue theGlobalObject = engine.globalObject();

  QJSValueIterator it2(theGlobalObject);
  while (it2.hasNext()) {
    it2.next();
    QJSValue value = it2.value();
    if (value.isArray()) {
      QString name = it2.name();
      string fieldName = name.toStdString();
      bool newField = notDefined(name, previousVariableContext);
      if (newField) {

        LOG(DEBUG) << fieldName << " is an Array ";
      
        // save to temporary name
        fieldName.append("#");
        vector <float> *floatData;
        QJSValue jsArray = it2.value();
        //QVector<int> integers;
        const unsigned int length = jsArray.property("length").toUInt();
        floatData = new vector<float>(length);
        for (unsigned int i = 0; i < length; ++i) {
            float theValue = (float) jsArray.property(i).toNumber();
            floatData->at(i) = theValue;
        }
        _soloFunctionsController->setData(fieldName, floatData); 

        // just some debugging stuff
        QString resultString = it2.value().toString();
        resultString.truncate(25);
        LOG(DEBUG) << it2.name().toStdString() << ": " << resultString.toStdString();
      }
    }
    // currentVariableContext[it2.name()] = it2.value().toString();
  }

  LOG(DEBUG) << "end current QJSEngine context";
}

/*
 Then the Javascript map operator will work for user defined functions on the radar fields, e.g.

  QString fworks1 = "function incr(val) { return val + 1 }; [8,9,10].map(incr)"; // works
  QString fworks2 = "function incr(val) { return val + 1 }; message.qilist.map(incr)";
  QString f = "function incr(val) { return val + 100 }; VEL.map(incr)";
  QJSValue result = myEngine.evaluate(f);
  std::cout << "result = " << result.toString().toStdString() << std::endl;

 */

// SoloFunctions object comes in with data model already attached 
void ScriptEditorController::setupSoloFunctions(SoloFunctionsController *soloFunctions)
{
  
  QJSValue myExt = engine.newQObject(soloFunctions); // new SoloFunctions());
  
  engine.globalObject().setProperty("sqrt", myExt.property("sqrt"));
  engine.globalObject().setProperty("REMOVE_AIRCRAFT_MOTION", myExt.property("REMOVE_AIRCRAFT_MOTION"));
  engine.globalObject().setProperty("BB_UNFOLDING_FIRST_GOOD_GATE", myExt.property("BB_UNFOLDING_FIRST_GOOD_GATE"));
  engine.globalObject().setProperty("ZERO_MIDDLE_THIRD", myExt.property("ZERO_MIDDLE_THIRD"));
  engine.globalObject().setProperty("ZERO_INSIDE_BOUNDARY", myExt.property("ZERO_INSIDE_BOUNDARY"));
  engine.globalObject().setProperty("add", myExt.property("add"));
  engine.globalObject().setProperty("DESPECKLE", myExt.property("DESPECKLE"));
  engine.globalObject().setProperty("SET_BAD_FLAGS_ABOVE", myExt.property("SET_BAD_FLAGS_ABOVE"));
  engine.globalObject().setProperty("SET_BAD_FLAGS_BELOW", myExt.property("SET_BAD_FLAGS_BELOW"));
  engine.globalObject().setProperty("SET_BAD_FLAGS_BETWEEN", myExt.property("SET_BAD_FLAGS_BETWEEN"));
  engine.globalObject().setProperty("COMPLEMENT_BAD_FLAGS", myExt.property("COMPLEMENT_BAD_FLAGS"));
  engine.globalObject().setProperty("CLEAR_BAD_FLAGS", myExt.property("CLEAR_BAD_FLAGS"));
  engine.globalObject().setProperty("ASSERT_BAD_FLAGS", myExt.property("ASSERT_BAD_FLAGS"));

  engine.globalObject().setProperty("AND_BAD_FLAGS_ABOVE", myExt.property("AND_BAD_FLAGS_ABOVE"));
  engine.globalObject().setProperty("AND_BAD_FLAGS_BELOW", myExt.property("AND_BAD_FLAGS_BELOW"));
  engine.globalObject().setProperty("AND_BAD_FLAGS_BETWEEN", myExt.property("AND_BAD_FLAGS_BETWEEN"));
  
  engine.globalObject().setProperty("OR_BAD_FLAGS_ABOVE", myExt.property("OR_BAD_FLAGS_ABOVE"));
  engine.globalObject().setProperty("OR_BAD_FLAGS_BELOW", myExt.property("OR_BAD_FLAGS_BELOW"));
  engine.globalObject().setProperty("OR_BAD_FLAGS_BETWEEN", myExt.property("OR_BAD_FLAGS_BETWEEN"));

  engine.globalObject().setProperty("XOR_BAD_FLAGS_ABOVE", myExt.property("XOR_BAD_FLAGS_ABOVE"));
  engine.globalObject().setProperty("XOR_BAD_FLAGS_BELOW", myExt.property("XOR_BAD_FLAGS_BELOW"));
  engine.globalObject().setProperty("XOR_BAD_FLAGS_BETWEEN", myExt.property("XOR_BAD_FLAGS_BETWEEN"));
  engine.globalObject().setProperty("COPY_BAD_FLAGS", myExt.property("COPY_BAD_FLAGS"));
  engine.globalObject().setProperty("FLAGGED_ADD", myExt.property("FLAGGED_ADD"));
  engine.globalObject().setProperty("FLAGGED_MULTIPLY", myExt.property("FLAGGED_MULTIPLY"));

  engine.globalObject().setProperty("FLAG_FRECKLES", myExt.property("FLAG_FRECKLES"));
  engine.globalObject().setProperty("FLAG_GLITCHES", myExt.property("FLAG_GLITCHES"));
  engine.globalObject().setProperty("~+", myExt.property("FLAGGED_ADD"));

  // print the context ...
  // printQJSEngineContext();
  
} 

/*
this is interesting ...
//! [1]
QJSValue fun = myEngine.evaluate("(function(a, b) { return a + b; })");
QJSValueList args;
args << 1 << 2;
QJSValue threeAgain = fun.call(QJSValue(), args);
//! [1]
*/


void ScriptEditorController::printQJSEngineContext() {
                                                                                                           
  LOG(DEBUG) << "current QJSEngine context ...";

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



 /*
void ScriptEditorController::processFormula(QString formula)
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
      addVariableToScriptEditor(it2.name(), it2.value());
    }
  }
  // ======    



}
 */

  /*
void ScriptEditorController::returnPressed()
{
    QString text = formulaInput->text();
    int row = table->currentRow();
    int col = table->currentColumn();
    QTableWidgetItem *item = table->item(row, col);
    if (!item)
        table->setItem(row, col, new ScriptEditorControllerItem(text));
    else
        item->setData(Qt::EditRole, text);
    table->viewport()->update();
}
  */

//emit runOneTimeOnlyScript(oneTimeOnlyScript);
//emit runForEachRayScript(forEachRayScript);

void ScriptEditorController::runOneTimeOnlyScript(QString script)
{
  LOG(DEBUG) << "enter"; 

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

      LOG(DEBUG) << it.name().toStdString() << ": " << theValue.toStdString(); // it.value().toString().tr\
uncate(100);                                                                                               
      currentVariableContext[it.name()] = it.value().toString();
    }
  */
      // ======                                                                                            
    //    try {
      QJSValue result = engine.evaluate(script);
      if (result.isError()) {
        QString message;
        message.append(result.toString());
        message.append(" on line number ");
        message.append(result.property("lineNumber").toString());
        LOG(DEBUG)
          << "Uncaught exception at line"
          << result.property("lineNumber").toInt()
          << ":" << result.toString().toStdString();
        throw message.toStdString();

      } else {

        LOG(DEBUG) << " the result is " << result.toString().toStdString();

        if (result.isArray()) {
          cerr << " the result is an array\n";
        //vector<int> myvector;                                                                            
        //myvector = engine.fromScriptValue(result);                                                       
        }
        if (result.isNumber()) {
          cerr << " the result is a number " << result.toString().toStdString() << endl;
          //setSelectionToValue(result.toString());                                                        
        }
	/*
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
	*/
      }
      /*
    } catch (const std::exception& ex) {
      criticalMessage(ex.what());
    } catch (const std::string& ex) {
      criticalMessage(ex);
    } catch (...) {
      criticalMessage("Error occurred during evaluation");
    }
      */
      LOG(DEBUG) << "exit";
}


void ScriptEditorController::runForEachRayScript(QString script, bool useBoundary)
{
  LOG(DEBUG) << "enter";

  QStringList newFieldNames; //  = {"VEL_xyz"}; // , "123", "CDE"};

  
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

      LOG(DEBUG) << it.name().toStdString() << ": " << theValue.toStdString(); // it.value().toString().tr\
uncate(100);                                                                                               
      currentVariableContext[it.name()] = it.value().toString();
    }

    // set initial field names
    initialFieldNames = getFieldNames();

    // add initialFieldNames_v to currentVariableContext!
    //_addFieldNameVectorsToContext(initialFieldNames, &currentVariableContext);
    vector<string>::iterator nameItr;
    for (nameItr = initialFieldNames.begin(); nameItr != initialFieldNames.end(); ++nameItr) {
      QString vectorName(nameItr->c_str());
      //vectorName.append("_v");
      currentVariableContext[vectorName] = vectorName;
    }
  
      // ======                                                                                            
    //    try {

    // for each sweep
   _soloFunctionsController->setCurrentSweepToFirst();

   //while (_soloFunctionsController->moreSweeps()) {
    // for each ray
    _soloFunctionsController->setCurrentRayToFirst();

    while (_soloFunctionsController->moreRays()) {
      LOG(DEBUG) << "more rays ...";
      // calculate boundary mask for each ray? 
      // Yes, when the ray index changes a new boundary mask is calculated 
      // in the SoloFunctionsController
      _soloFunctionsController->applyBoundary(useBoundary);

      // TODO: set field values in javascript array? by (sweep, ray) would we apply boundary?
      setupFieldArrays(); 
      setupBoundaryArray();

      QJSValue result = engine.evaluate(script);
      if (result.isError()) {
        QString message;
        message.append(result.toString());
        message.append(" on line number ");
        message.append(result.property("lineNumber").toString());
        LOG(DEBUG)
          << "Uncaught exception at line"
          << result.property("lineNumber").toInt()
          << ":" << result.toString().toStdString();
        throw message.toStdString();

      } else {
        QString resultString = result.toString();
        resultString.truncate(25);

        LOG(DEBUG) << " the result is " << resultString.toStdString();

        if (result.isArray()) {
          cerr << " the result is an array\n";
	  //vector<int> myvector;                                                                            
	  //myvector = engine.fromScriptValue(result);                                                       
        }
        if (result.isNumber()) {
          cerr << " the result is a number " << result.toString().toStdString() << endl;
          //setSelectionToValue(result.toString());                                                        
        }
	
        saveFieldArrays(currentVariableContext);
	
      }
      _soloFunctionsController->nextRay();
    }
    //_soloFunctionsController->nextSweep();
    
    //}
      /*
    } catch (const std::exception& ex) {
      criticalMessage(ex.what());
    } catch (const std::string& ex) {
      criticalMessage(ex);
    } catch (...) {
      criticalMessage("Error occurred during evaluation");
    }
      */

	// ======                                                                                            
	//  YES! This works.  The new global variables are listed here;                                      
	// just find them and add them to the spreadsheet and to the Model??                                 
	// HERE!!!                                                                                           
	// try iterating over the properties of the globalObject to find new variables                       
    QJSValue newGlobalObject = engine.globalObject();

    QJSValueIterator it2(newGlobalObject);
    while (it2.hasNext()) {
	    it2.next();

      QJSValue value = it2.value();
      if (value.isArray()) {

  	    QString theValue = it2.value().toString();
  	    theValue.truncate(100);
  	    LOG(DEBUG) << it2.name().toStdString() << ": " << theValue.toStdString();
  	    if (currentVariableContext.find(it2.name()) == currentVariableContext.end()) {
  	      // we have a newly defined variable                                                            
  	      LOG(DEBUG) << "NEW VARIABLE " << it2.name().toStdString() <<  ": " << theValue.toStdString();
  	      // COOL! at this point, we have the new field name AND the temporary field name in the RadxVol,
  	      // so we can do an assignment now.
          string tempName = it2.name().toStdString();
  	      string userDefinedName = it2.name().toStdString();
  	      // only assign the ray data if this is a Solo Function, f(x)
          //size_t length = tempName.length();
          //if (length > 0) {
            //if (tempName[length-1] == '#') {
              //tempName.resize(length-1);
            tempName.append("#");
  	        _assign(tempName, userDefinedName);
  	        // add Variable list ToScriptEditor(it2.name(), it2.value());
  	        newFieldNames << it2.name();
  	        //}
  	      //}
  	    }
      }
    }


    volumeUpdated(newFieldNames);
    LOG(DEBUG) << "exit";
}


void ScriptEditorController::_assign(string tempName, string userDefinedName) {

  // rename the field in the RadxVol
  _soloFunctionsController->assign(tempName, userDefinedName);
}


// request filled by Controller in response to needFieldNames signal                                       
void ScriptEditorController::fieldNamesProvided(vector<string> fieldNames) {

  int useless = 0;

  // fill everything that needs the fieldNames ...                                                         

    // This section of code makes every data field in volume a variable                                    
    // When the variable name is referenced in the formula bar,                                            
    // the variable name as a string is substituted.                                                       
    //                                                                                                     
    // for each field in model (RadxVol)          
    int someValue = 0;
    vector<string>::iterator it;

    for(it = fieldNames.begin(); it != fieldNames.end(); it++) {
      QString fieldName(QString::fromStdString(*it));
      // //    try {                                                                                       
      ////QJSValue objectValue = engine.newQObject(new DataField(*it));                                    
      ////engine.globalObject().setProperty(fieldName, objectValue.property("name"));    

      engine.globalObject().setProperty(fieldName.append("_V"), fieldName);                                           

      //someValue += 1;                                                                                    

      // //} catch (Exception ex) {                                                                        
      // // cerr << "ERROR - problem setting property on field " << *it << endl;                           
      // //}                                                                                               
    }
    //if (LOG_STREAM_IS_ENABLED(LogStream::DEBUG)) { // causes a segmentation fault                        
    // print the context ...                                                                              \
                                                                                                           
      LOG(DEBUG) << "current QJSEngine context ... after fieldNamesProvided";

      printQJSEngineContext();
}

// Not used? 
void ScriptEditorController::addVariableToScriptEditor(QString name, QJSValue value) {

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

    /*                                                                                                     
  for(it = value.begin(); it != value.end(); it++) {                                                       
    QString the_name(QString::fromStdString(*it));                                                         
    cerr << *it << endl;                                                                                   
    table->setHorizontalHeaderItem(c, new QTableWidgetItem(the_name));                                     
    vector<double> data = _controller->getData(*it);                                                       
    cerr << "number of data values = " << data.size() << endl;                                             
    for (int r=0; r<20; r++) {                                                                             
      //    sprintf(formattedData, format, data[0]);                                                      \
                                                                                                           
      sprintf(formattedData, "%g", data.at(r));                                                            
      cerr << "setting " << r << "," << c << "= " << formattedData << endl;                                
      table->setItem(r, c, new ScriptEditorItem(formattedData));                                           
    }                                                                                                      
    c += 1;                                                                                                
    } */
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
  if (value.isNumber()) {
    //qDebug() << "variable isNumber " << name << endl;                                                    
    LOG(DEBUG) << "variable isNumber " << name.toStdString();
  }
  if (value.isObject()) {
    LOG(DEBUG) << "variable isObject " << name.toStdString();
    //qDebug() << "variable isObject " << name << endl;                                                    
    //    QVector<double> myv = value.property("values");                                                  
    //qDebug() << myv.at(0) << ";" << myv.at(1) << endl;                                                   
    /*    table->setColumnCount(table->columnCount() + 1);    
                                                                                                           
    int c = table->columnCount() - 1;                                                                      
    table->setHorizontalHeaderItem(c, new QTableWidgetItem(name));                                         
                                                                                                           
    QJSValueIterator it(value);                                                                            
    while (it.hasNext()) {                                                                                 
      it.next();                                                                                           
      LOG(DEBUG) << it.name().toStdString() << ": " << it.value().toString().toStdString();                
    }                                                                                                      
                                                                                                           
    for (int r=0; r<value.property("length").toInt(); r++) {                                               
      //qDebug() << it.name() << ": " << it.value().toString();                                            
      QString valueAsString = value.property(r).toString();                                                
      //      sprintf(formattedData, "%g", value.property(r).toInt());                                     
      //table->setItem(r, c, new ScriptEditorItem(formattedData));                                         
      table->setItem(r,c, new QTableWidgetItem(valueAsString));                                            
    }                                                                                                      
    */

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
    /*                                                                                                     
    table->setColumnCount(table->columnCount() + 1);                                                       
                                                                                                           
    int c = table->columnCount() - 1;                                                                      
    table->setHorizontalHeaderItem(c, new QTableWidgetItem(name));                                         
    table->setItem(0,c, new QTableWidgetItem(value.toString()));                                           
    */
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

