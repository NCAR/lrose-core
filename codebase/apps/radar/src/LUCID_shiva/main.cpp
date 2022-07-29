//#include "mainwindow.h"
//#include <QApplication>
#include <cstdio>
#include <iostream>
#include <cstdlib>
//#include "Args.hh"
//#include "Params.hh"
#include "PrintMdv.hh"

#include <iostream>



using namespace std;

// args should be ...
// BestLittleCartImageGenerator  input-file output-file color-scale-[file? or name?]

int main(int argc, char *argv[])
{

    
    //readX11ColorTables();
    PrintMdv printMdv(argv[1]);
    printMdv.Run();

    return 0;

    /*
    QApplication a(argc, argv);
    std::cout << argv[1] << std::endl;
    MainWindow w;

    //------------------------------------------
    //this portion was recently added to include Args and Params.
    string _progName;
    //Params _params;
    Args _args("LUCID");

    */

    // get command line args
    /*if (_args.parse(argc, const_cast<const char **>(argv))) {
        cerr << "ERROR: " << _progName << endl;
        cerr << "Problem with command line args" << endl;
      }*/

    // load TDRP params from command line
    //char *paramsPath = const_cast<char *>("unknown");
    /*if (_params.loadFromArgs(argc, argv,
                   _args.override.list,
                   &paramsPath)) {
        cerr << "ERROR: " << _progName << endl;
        cerr << "Problem with TDRP parameters." << endl;
      }*/

    /*if (_params.fields_n < 1) {
        cerr << "ERROR: " << _progName << endl;
        cerr << "  0 fields specified" << endl;
        cerr << "  At least 1 field is required" << endl;
      }*/


    /*if (_params.debug > Params::DEBUG_NORM)
        cerr << "Debug is on" << endl;*/
    //--------------------------------------------

    //w.show();

    //I'm using the below line to show parameter values, delete at a later date.
    // std::cout << _params. << std::endl;

    //return a.exec();
}
