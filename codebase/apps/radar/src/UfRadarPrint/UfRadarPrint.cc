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
// UfRadarPrint.cc
//
// UfRadarPrint object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////
//
// Print out Universal Format radar files
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <dataport/bigend.h>
#include <toolsa/mem.h>
#include <toolsa/ucopyright.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaFile.hh>
#include <toolsa/TaArray.hh>
#include <rapformats/UfRecord.hh>
#include "UfRadarPrint.hh"
using namespace std;

// Constructor

UfRadarPrint::UfRadarPrint(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "UfRadarPrint";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that file list is set
  
  if (_args.inputFileList.size() == 0) {
    cerr << "ERROR: UfRadarPrint::UfRadarPrint." << endl;
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  return;

}

// destructor

UfRadarPrint::~UfRadarPrint()

{

}

//////////////////////////////////////////////////
// Run

int UfRadarPrint::Run ()
{

  int iret = 0;
  
  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {

    const char *inputPath = _args.inputFileList[ii].c_str();
    
    if (_processFile(inputPath)) {
      cerr << "ERROR = UfRadarPrint::Run" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      iret = -1;
    }

  } // ii

  return iret;

}

///////////////////////////////
// process file

int UfRadarPrint::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // open file - file closes automatically when inFile goes
  // out of scope
  
  TaFile inFile;
  FILE *in;
  if ((in = inFile.fopen(input_path, "r")) == NULL) {
    cerr << "ERROR - UfRadarPrint::_processFile" << endl;
    cerr << "  Cannot open input file: " << input_path << endl;
    perror("  ");
    return -1;
  }
  
  // read through the records in the file

  while (!feof(in)) {

    // read rec header - this is a 4-byte integer FORTRAN uses

    ui32 nbytes;
    if (fread(&nbytes, sizeof(ui32), 1, in) != 1) {
      continue;
    }
    BE_to_array_32(&nbytes, sizeof(ui32));
    
    // read data record

    TaArray<ui08> record_;
    ui08 *record = record_.alloc(nbytes);
    if (fread(record, sizeof(ui08), nbytes, in) != nbytes) {
      break;
    }

    if (!_params.trailing_fortran_record_missing) {

      // read record trailer - this is a 4-byte integer FORTRAN uses
      
      ui32 nbytesTrailer;
      if (fread(&nbytesTrailer, sizeof(ui32), 1, in) != 1) {
        break;
      }
      BE_to_array_32(&nbytesTrailer, sizeof(ui32));
      
      if (nbytesTrailer != nbytes) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Read record - nbytes: " << nbytes << endl;
          cerr << "WARNING - UfRadarPrint" << endl;
          cerr << "  Header record len differs from trailer len" << endl;
          cerr << "  Header  len: " << nbytes << endl;
          cerr << "  Trailer len: " << nbytesTrailer << endl;
        }
        // seek back since this is not a size record
        
        fseek(in, -sizeof(ui32), SEEK_CUR);
      }

    }

    // load up record object

    UfRecord uf;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      uf.setDebug(true);
    }
    if (uf.disassemble(record, nbytes)) {
      cerr << "Error - cannot load UF record from raw data" << endl;
      continue;
    }
    
    // print
    
    uf.print(cout, true, _params.print_data);
    
  } // while
  
  if (_params.debug) {
    cerr << "End of file" << endl;
  }

  return 0;

}

