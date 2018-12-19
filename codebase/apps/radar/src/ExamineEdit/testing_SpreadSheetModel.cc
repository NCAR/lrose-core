

#include "SpreadSheetModel.hh"

int main(int argc, char *argv[]) {

  SpreadSheetModel spreadSheet;

  spreadSheet.initData(argv[1]);

 
  vector<string> fieldNames =  spreadSheet.getFields();
  vector<string>::iterator str;
  for (str = fieldNames.begin(); str < fieldNames.end(); str++) {
    cout << *str << " ";
  }
  cout << endl;
 
  vector<double> data = spreadSheet.getData(fieldNames[0]);
  vector<double>::iterator dataPtr;
  for (dataPtr = data.begin(); dataPtr < data.end(); dataPtr++) {
    cout << *dataPtr << " ";
  }
  cout << endl;

  return 1;
}
