#include "mainwindow.h"
#include <QApplication>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include "Args.hh"
#include "Params.hh"





using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;


    string _progName;
    Params _params;
    Args _args("LUCID");


    // get command line args


    if (_args.parse(argc, const_cast<const char **>(argv))) {
        cerr << "ERROR: " << _progName << endl;
        cerr << "Problem with command line args" << endl;


      }

      // load TDRP params from command line

    char *paramsPath = const_cast<char *>("unknown");
    if (_params.loadFromArgs(argc, argv,
                   _args.override.list,
                   &paramsPath)) {
        cerr << "ERROR: " << _progName << endl;
        cerr << "Problem with TDRP parameters." << endl;

      }

      if (_params.fields_n < 1) {
        cerr << "ERROR: " << _progName << endl;
        cerr << "  0 fields specified" << endl;
        cerr << "  At least 1 field is required" << endl;

      }


    if (_params.debug > Params::DEBUG_NORM)
        cerr << "Debug is on" << endl;

    w.show();





    return a.exec();
}
