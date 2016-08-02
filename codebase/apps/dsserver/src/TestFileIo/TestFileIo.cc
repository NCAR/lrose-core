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
// TestFileIo.cc
//
// TestFileIo object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#include "TestFileIo.hh"
#include "Args.hh"
#include "Params.hh"
#include <dsserver/DsFileIo.hh>

#include <toolsa/umisc.h>
#include <toolsa/str.h>
using namespace std;

// Constructor

TestFileIo::TestFileIo(int argc, char **argv)

{

  OK = TRUE;
  _progName = NULL;
  _args = NULL;
  _params = NULL;

  // set programe name

  _progName = STRdup("TestFileIo");

  // get command line args
  
  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _params = new Params();
  _paramsPath = const_cast<char*>(string("unknown").c_str());
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  return;

}

// destructor

TestFileIo::~TestFileIo()

{

  // free up

  if (_progName) {
    STRfree(_progName);
  }
  if (_args) {
    delete(_args);
  }
  if (_params) {
    delete(_params);
  }

}

//////////////////////////////////////////////////
// Run

int TestFileIo::Run()
{

  if (_params->mode == Params::ASCII_COPY_MODE) {
    if (_asciiCopy()) {
      cerr << "Ascii copy failed" << endl;
    }
  } else if (_params->mode == Params::BINARY_COPY_MODE) {
    if (_binaryCopy()) {
      cerr << "Binary copy failed" << endl;
    }
  } else if (_params->mode == Params::PRINTF_MODE) {
    if (_printf()) {
      cerr << "Printf failed" << endl;
    }
  } else {
    cerr << "Unknown mode: " << endl;
  }

  return (0);

}

////////////////////
// _asciiCopy()
//

int TestFileIo::_asciiCopy()

{

  if (_params->debug) {
    cerr << "----> Ascii copy" << endl;
  }

  if (!strcmp(_params->source_url, "null") ||
      !strcmp(_params->dest_url, "null")) {
    cerr << "ERROR - ASCII_COPY_MODE" << endl;
    cerr << "Must specify both source and destination URL." << endl;
    return (-1);
  }

  DsFileIo source;
  DsFileIo dest;

  // open the source

  if (source.fOpen(_params->source_url, "r")) {
    cerr << "ERROR - cannot fOpen source url " << _params->source_url << endl;
    return (-1);
  }
  
  // open the dest

  if (dest.fOpen(_params->dest_url, "w")) {
    cerr << "ERROR - cannot fOpen dest url " << _params->dest_url << endl;
    source.fClose();
    return (-1);
  }

  if (_params->debug) {
    source.fSeek(0, SEEK_SET);
    dest.fSeek(0, SEEK_SET);
  }

  // copy the contents over

  char line[2048];
  int count = 0;
  while (source.fGets(line, 2048) != NULL) {
    count++;
    line[2047] = '\0';
    if (dest.fPuts(line)) {
      cerr << "ERROR - fPuts from " << _params->source_url << endl;
      cerr << "              to   " << _params->dest_url << endl;
      cerr << "Line:" << endl;
      cerr << line << endl;
      source.fClose();
      dest.fClose();
      return (-1);
    }
  }

  if (_params->debug) {
    cerr << "Source file pos: " << source.fTell() << endl;
    cerr << "Dest file pos: " << dest.fTell() << endl;
  }

  // close
  
  source.fClose();
  dest.fClose();

  return (0);

}

////////////////////
// _binaryCopy()
//

#define NBLOCK 8196

int TestFileIo::_binaryCopy()

{

  if (_params->debug) {
    cerr << "----> Binary copy" << endl;
  }

  if (!strcmp(_params->source_url, "null") ||
      !strcmp(_params->dest_url, "null")) {
    cerr << "ERROR - BINARY_COPY_MODE" << endl;
    cerr << "Must specify both source and destination URL." << endl;
    return (-1);
  }

  DsFileIo source;
  DsFileIo dest;

  // open the source

  if (source.fOpen(_params->source_url, "r")) {
    cerr << "ERROR - cannot fOpen source url " << _params->source_url << endl;
    return (-1);
  }
  
  // stat the source

  off_t fileSize;
  if (source.fStat(&fileSize)) {
    cerr << "ERROR - cannot fStat source url " << _params->source_url << endl;
    source.fClose();
    return (-1);
  }
  
  // open the dest

  if (dest.fOpen(_params->dest_url, "w")) {
    cerr << "ERROR - cannot fOpen dest url " << _params->dest_url << endl;
    source.fClose();
    return (-1);
  }

  if (_params->debug) {
    source.fSeek(0, SEEK_SET);
    dest.fSeek(0, SEEK_SET);
  }

  // copy the contents over

  ui08 buf[NBLOCK];
  unsigned int nleft = fileSize;
  while (nleft > 0) {

    unsigned int ncopy;
    if (nleft > NBLOCK) {
      ncopy = NBLOCK;
    } else {
      ncopy = nleft;
    }

    if (source.fRead(buf, 1, ncopy) != ncopy) {
      cerr << "ERROR - fRead for " << ncopy << " bytes from "
	   << _params->source_url << endl;
      source.fClose();
      dest.fClose();
      return (-1);
    }

    if (dest.fWrite(buf, 1, ncopy) != ncopy) {
      cerr << "ERROR - fWrite for " << ncopy << " bytes to "
	   << _params->dest_url << endl;
      source.fClose();
      dest.fClose();
      return (-1);
    }

    nleft -= ncopy;

  } // while

  if (_params->debug) {
    cerr << "Source file pos: " << source.fTell() << endl;
    cerr << "Dest file pos: " << dest.fTell() << endl;
  }

  // close
  
  source.fClose();
  dest.fClose();

  return (0);

}

////////////////////
// _printf()
//

int TestFileIo::_printf()

{

  if (_params->debug) {
    cerr << "----> printf" << endl;
  }
  
  if (!strcmp(_params->dest_url, "null")) {
    cerr << "ERROR - PRINTF_MODE" << endl;
    cerr << "Must specify destination URL." << endl;
    return (-1);
  }

  DsFileIo dest;

  // open the dest

  if (dest.fOpen(_params->dest_url, "w")) {
    cerr << "ERROR - cannot fOpen dest url " << _params->dest_url << endl;
    return (-1);
  }

  if (_params->debug) {
    cerr << "URL " << _params->dest_url << " opened." << endl;
  }

  if (_params->debug) {
    dest.fSeek(0, SEEK_SET);
  }

  // print params to file

  dest.fPrintf("*************************************\n");
  dest.fPrintf("Test fPrintf at time %s\n", utimstr(time(NULL)));
  dest.fPrintf("debug: %d, mode: %d\n", _params->debug, _params->mode);
  dest.fPrintf("Source: %s\n", _params->source_url);
  dest.fPrintf("Dest: %s\n", _params->dest_url);
  dest.fPrintf("*************************************\n");

  if (_params->debug) {
    cerr << "Dest file pos: " << dest.fTell() << endl;
  }

  // close
  
  dest.fClose();

  return (0);

}


