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
//   CartIG -input input_filename -output output_filename -field field_name -color_scale colorscale -field field_name -color_scale colorscale ....

// i.e. the normal unix convention.

// RadxPrint is an example of an app that supports multiple fields on the command line


// CartIG <data file name> <output base dir> <field1> <color_scale1> <field2> <color_scale2> ...
int main(int argc, char *argv[])
{
    char *inputFile;
    char *outputDir;
    char *fieldName;
    char *colorScaleFileOrName;
    
    // TODO: integrate the tdrp args, etc.
    if (argc < 3) {
        cout << "wrong usage" << endl;
    } 

  inputFile = argv[1];
  outputDir = argv[2];




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
    PrintMdv printMdv(inputFile, outputDir);

    // or construct two lists color scale name, field name;
    // then send both lists??
    // if we had the list of field names when we are reading ..
    // or just open the file, read the field, plot it.
    if (argc > 3) {
        // generate images for the specified fields
        for (int i = 3; i < argc; i+=2 ) {
            fieldName = argv[i];
            colorScaleFileOrName = argv[i+1]; 
            printMdv.plotField(fieldName, colorScaleFileOrName);       
        }
        
    } else {
        // generate images for all the fields
        printMdv.plotAllFields();
    }

 

    return 0;    
}
