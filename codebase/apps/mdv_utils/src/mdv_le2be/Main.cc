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
// converter for little-endian mdv files.
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1999
//
///////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <csignal>

#include <toolsa/os_config.h>
#include <toolsa/port.h>

#include "Args.hh"
#include "Params.hh"

#include <dataport/bigend.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
using namespace std;
//#include <mdv/MdvFile.hh>
//#include <mdv/MdvField.hh>
//#include <euclid/TypeGrid.hh>

//
// Prototypes for static functions.
//

static void tidy_and_exit (int sig);


/*****************************************************************
 * main() - main program.
 */

Args * _args = NULL;
Params *_params = NULL;
char * _progName = (char *) "mdv_le2be";
char * _paramsPath = (char *) "unknown";

int main(int argc, char **argv)
{
  // set signal handling
  
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  _args = new Args(argc, argv, _progName);
  if (!_args->OK || argc != 5) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    tidy_and_exit(1);
  }

  _params = new Params();
  if (_params->loadFromArgs(argc, argv,
                            _args->override.list,
                            &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    tidy_and_exit(1);
  }


  BE_reverse();
  DsMdvx in_mdv;
  in_mdv.clearRead();
  in_mdv.clearReadFields();
  in_mdv.clearReadPath();
  in_mdv.setReadPath(_params->inFile);
  in_mdv.readAllHeaders();
  Mdvx::master_header_t masterHdr = in_mdv.getMasterHeaderFile();

  for(int i=0; i<masterHdr.n_fields; ++i) {
    in_mdv.addReadField(i);
  }
  if ( in_mdv.readVolume() < 0 ) {
    cerr << "ERROR: " << endl;
    cerr << " failed to read fields. " << endl;
    cerr << in_mdv.getErrStr() << endl;
    cerr << "*** Exiting ***" << endl;
    return 0;
  }


  BE_reverse();
  if (in_mdv.writeToPath(_params->outFile) != 0) {
    cerr << "WARNING: " << endl;
    cerr << "Error writing data for time ";
    cerr << utimstr(in_mdv.getMasterHeader().time_centroid);
    cerr << " to file " << _params->outFile << endl;
    cerr << "*** Exiting ***" << endl << endl;
  }

#ifdef NOTNOW


  MdvFile mdvFile;
  TypeGrid<float> floatGrid(Grid::FLOAT_GRID);
  MdvField protoField("Prototype Field", floatGrid);
  string errString;

  int status = mdvFile.readAllFields(_params->inFile, protoField, errString);
  if (status < 0) {
    cerr << "Could not read mdv file: " << _params->inFile << endl;
    tidy_and_exit(1);
  }

  fp = fopen(_params->outFile, "wb");
  if (fp == NULL) {
    cerr << "Could not open output file: " << _params->outFile << endl;
    tidy_and_exit(1);
  }

  status = mdvFile.write(fp, MDV_FLOAT32);
  if (status < 0) {
    cerr << "Could not write mdv file: " << _params->outFile << endl;
    tidy_and_exit(1);
  }
#endif
  

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

