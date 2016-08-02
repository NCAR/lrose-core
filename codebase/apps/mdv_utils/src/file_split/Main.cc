// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
//
// Puts each field in it's own mdv file.
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1999
//
///////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <signal.h>

#include <toolsa/os_config.h>
#include <toolsa/port.h>

#include <mdv/MdvFile.hh>
#include <mdv/MdvField.hh>
#include <euclid/TypeGrid.hh>
using namespace std;

//
// Prototypes for static functions.
//

static void tidy_and_exit (int sig);


/*****************************************************************
 * main() - main program.
 */

int main(int argc, char **argv)
{
  // set signal handling
  
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  if (argc != 3) {
    cerr << "Wrong number of args." << endl;
    tidy_and_exit(1);
  }

  char * infile = argv[1];

  // Todo: Use stat().
  // FILE * fp = fopen(_params->inFile, "rb");
  // if (fp == NULL) {
  //   cerr << "Could not open file: " << _params->inFile << endl;
  //   tidy_and_exit(1);
  // }
  // fclose(fp);

  MdvFile mdvFile;
  TypeGrid<float> floatGrid(Grid::FLOAT_GRID);
  MdvField protoField("Prototype Field", floatGrid);

  int status;
  string errString;
  status = mdvFile.readAllFields(infile, protoField, errString);
  if (status < 0) {
    cerr << "Could not read mdv filed from file: " << infile
         << ": " << errString
         << endl;
    tidy_and_exit(1);
  }

  // fp = fopen(_params->outFile, "wb");
  // if (fp == NULL) {
  //   cerr << "Could not open output file: " << _params->outFile << endl;
  //   tidy_and_exit(1);
  // }
  // 
  // status = mdvFile.write(fp, MDV_FLOAT32);
  // if (status < 0) {
  //   cerr << "Could not write mdv file: " << _params->outFile << endl;
  //   tidy_and_exit(1);
  // }
  
  Grid * grid0 = NULL;
  MdvField * field0 = NULL;
  for (int i = 0; i < mdvFile.getNumFields(); i++) {
    MdvField * currField = mdvFile.getField(i);
    Grid * currGrid = currField->getGrid();

    // Have to set the value defs to zero so that they end up
    //   as zero in the char data. This seems like a bug!
    //   (Conversion to chars won't catch bad values!).
    // 
    // ((TypeGrid<float> *) currGrid)->setValueDefs(0.0, 0.0, 0.0);

    char buf[10];
    MdvFile currFile;
    string fileName(argv[2]);

    // Skip field 0, combine it with field 1.
    if (i == 0) {
      grid0  = currGrid;
      field0 = currField;
    }
    else if (i == 1) {
      MdvField * tmp = currFile.addField(field0->getName(), *grid0);
      MDV_field_header_t hdr = field0->getInfo();
      hdr.bad_data_value = 0.0;
      hdr.missing_data_value = 0.0;
      tmp->setInfo(hdr);

      // Create special file name.
      fileName += ".fields0and1.mdv";
    }
    else {
      // Create file name based on the field number.
      sprintf(buf, ".field%d.mdv", i);
      fileName += buf;
    }

    MdvField * newField = currFile.addField(currField->getName(), *currGrid);
    MDV_field_header_t hdr = currField->getInfo();
    hdr.bad_data_value = 0.0;
    hdr.missing_data_value = 0.0;
    newField->setInfo(hdr);
    
    FILE * fp = fopen(fileName.c_str(), "wb");
    if (fp == NULL) {
      cerr << "Could not open output file: " << fileName << endl;
      tidy_and_exit(1);
    }
    
    status = currFile.write(fp, MDV_PLANE_RLE8);
    if (status < 0) {
      cerr << "Could not write mdv file: " << fileName << endl;
      tidy_and_exit(1);
    }

    cerr << "Successfully wrote field: " << i << endl;
  }

  tidy_and_exit(0);
  return (0);
}


/*****************************************************************
 * tidy_and_exit() - Clean up memory and exit from the program.
 */

static void tidy_and_exit(int sig)
{
  exit(sig);
}

