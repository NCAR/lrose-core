//
// SpreadSheetModel provides data in basic form, as ints, floats, strings
// SpreadSheetModel provides the interface to Radx utilities, and file I/O

#include <stdio.h>

#include "SpreadSheetModel.hh"
//#include "spreadsheet.hh"
//#include "spreadsheetdelegate.hh"
//#include "spreadsheetitem.hh"


SpreadSheetModel::SpreadSheetModel()
{

}

SpreadSheetModel::SpreadSheetModel(RadxRay *closestRay, RadxVol dataVolume)
{
  _closestRay = closestRay;
  if (_closestRay == NULL) 
    cout << "in SpreadSheetModel, closestRay is NULL" << endl;
  else
   cout << "in SpreadSheetModel, closestRay is NOT  NULL" << endl;
  _vol = dataVolume;
}

/*
//////////////////////////////////////////////////                                                             
// set up read                                                                                                 
void SpreadSheetModel::_setupVolRead(RadxFile *file)
{


  //if (_params.debug >= Params::DEBUG_VERBOSE) {
  //  file.setDebug(true);
  //}
  //if (_params.debug >= Params::DEBUG_EXTRA) {
  file->setDebug(true);
  file->setVerbose(true);
  //}

  // TODO: we want to read the fields that are there; not have a predetermined list of fields
  //
  //for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
  //  const DisplayField *field = _fields[ifield];
  //  file.addReadField(field->getName());
  //}
  //

}
*/

///////////////////////////// 
// get data in archive mode
// returns 0 on success, -1 on failure
/*
int SpreadSheetModel::_getArchiveData(string inputPath)
{

  // set up file object for reading

  RadxFile file;
  _vol.clear();
  _setupVolRead(&file);


  //if (_archiveScanIndex >= 0 &&
  //    _archiveScanIndex < (int) _archiveFileList.size()) {

  //  string inputPath = _archiveFileList[_archiveScanIndex];

  //  if(_params.debug) {
  //    cerr << "  reading data file path: " << inputPath << endl;
  //    cerr << "  archive file index: " << _archiveScanIndex << endl;
  //  }
  //
  if (file.readFromPath(inputPath, _vol)) {
    string errMsg = "ERROR - Cannot retrieve archive data\n";
    errMsg += "PolarManager::_getArchiveData\n";
    errMsg += file.getErrStr() + "\n";
    errMsg += "  path: " + inputPath + "\n";
    cerr << errMsg;
    return -1;
  }

  //  }


  // set number of gates constant if requested 
   _vol.setNGatesConstant();
   _vol.loadFieldsFromRays();
  // compute the fixed angles from the rays   

  //_vol.computeFixedAnglesFromRays();


  //  if (_params.debug) {
  cerr << "----------------------------------------------------" << endl;
  cerr << "perform archive retrieval" << endl;
  cerr << "  read file: " << _vol.getPathInUse() << endl;
  cerr << "  nSweeps: " << _vol.getNSweeps() << endl;
  //cerr << "  guiIndex, fixedAngle: "
  //     << _sweepManager.getGuiIndex() << ", "
  //     << _sweepManager.getSelectedAngle() << endl;
  cerr << "----------------------------------------------------" << endl;
  //}

  //  _platform = _vol.getPlatform();

  return 0;

}

void SpreadSheetModel::initData(string fileName)
{
  _getArchiveData(fileName);
}
*/

vector<string> SpreadSheetModel::getFields()
{
  vector<string> fieldNames;
  if (_closestRay != NULL) {
    _closestRay->loadFieldNameMap();

    RadxRay::FieldNameMap fieldNameMap = _closestRay->getFieldNameMap();
    RadxRay::FieldNameMapIt it;
    for (it = fieldNameMap.begin(); it != fieldNameMap.end(); it++) {
      fieldNames.push_back(it->first);
      cout << it->first << ':' << it->second << endl;
    }
  } else {
    _vol.loadFieldsFromRays();
    fieldNames = _vol.getUniqueFieldNameList();
  }
  return fieldNames;
}

// return a list of data values
vector<float> SpreadSheetModel::getSampleData()
{

  vector<float> data;
  data.push_back(1.0);
  data.push_back(3.0);
  data.push_back(5.0);
  data.push_back(10.0);
  return data;
}


// return a list of data values for the given
// field name
vector<double> SpreadSheetModel::getData(string fieldName)
{
  int cols;
  int rows;

  vector <double> dataVector;
  const RadxField *field;
  field = _vol.getFieldFromRay(fieldName);  // <--- Why is this returning NULL
  // because the type is 
  // from debugger:  *((vol.getFieldFromRay("VEL"))->getDataSi16()+1)
  if (field == NULL) {
    cerr << "no RadxField found " <<  endl;
    return dataVector;
  } 
  // Radx::fl32 *data = field->getDataFl32();
  // TODO: how may gates?
  for (int i=0; i<20; i++) { 
    double value = field->getDoubleValue(i);
    cout << value << " ";
    dataVector.push_back(value); // data[i]);
  }
  cout << endl;
  // convert data to vector
  return dataVector;
}


/*
void SpreadSheetModel::actionSum()
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

void SpreadSheetModel::actionMath_helper(const QString &title, const QString &op)
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

void SpreadSheetModel::actionAdd()
{
    actionMath_helper(tr("Addition"), "+");
}

void SpreadSheetModel::actionSubtract()
{
    actionMath_helper(tr("Subtraction"), "-");
}

void SpreadSheetModel::actionMultiply()
{
    actionMath_helper(tr("Multiplication"), "*");
}
void SpreadSheetModel::actionDivide()
{
    actionMath_helper(tr("Division"), "/");
}

void SpreadSheetModel::clear()
{
    foreach (QTableWidgetItem *i, table->selectedItems())
        i->setText("");
}
*/

