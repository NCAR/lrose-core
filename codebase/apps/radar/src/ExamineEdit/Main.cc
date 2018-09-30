

#include "SpreadSheetView.hh"
#include "SpreadSheetController.hh"
#include <iostream>
#include <QApplication>
#include <QLayout>

using namespace std;

int main(int argc, char** argv) {

  cerr << "Hello There, Brenda" << endl;

  // Q_INIT_RESOURCE(spreadsheet);
    QApplication app(argc, argv);


    // read in the data file 
    // get data                                                                                                                
    SpreadSheetView *sheetView = new SpreadSheetView(argv[1]); 
    //    SpreadSheetModel sheetModel(argv[1]);
    SpreadSheetController sheetControl(sheetView);  
    //try {
      //_getArchiveData(argv[1]);
      //} catch (FileIException ex) {
      //      this->setCursor(Qt::ArrowCursor);
      //_timeControl->setCursor(Qt::ArrowCursor);
      //    return -1;
      //}

      // SpreadSheetView sheet(_vol); // , _displayInfo);
      sheetView->setWindowIcon(QPixmap(":/images/interview.png"));
      sheetView->show();
      sheetView->layout()->setSizeConstraint(QLayout::SetFixedSize);
      return app.exec();
}
