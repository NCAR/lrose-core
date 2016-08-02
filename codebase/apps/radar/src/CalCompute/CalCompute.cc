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
////////////////////////////////////////////////////////////////////////
// CalCompute.cc
//
// CalCompute object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2006
//
///////////////////////////////////////////////////////////////
//
// CalCompute computes radar cal from a cal data file
//
///////////////////////////////////////////////////////////////////////

#include <dirent.h>
#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include "CalCompute.hh"
using namespace std;

// Constructor

CalCompute::CalCompute(int argc, char **argv)

{

  isOK = true;
  _maxColNum = 0;

  // set programe name

  _progName = "CalCompute";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
    return;
  }

  // load up channel objects
  // compute max column number

  for (int ii = 0; ii < _params.input_channels_n; ii++) {
    if (_params._input_channels[ii].column_num > _maxColNum) {
      _maxColNum = _params._input_channels[ii].column_num;
    }
    Channel *chan = new Channel(_params,
                                _params._input_channels[ii].short_label,
                                _params._input_channels[ii].long_label,
                                _params._input_channels[ii].coupling_loss_db,
                                _params._input_channels[ii].peak_power_w);
    _channels.push_back(chan);
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "_maxColNum: " << _maxColNum << endl;
  }

  return;

}

// destructor

CalCompute::~CalCompute()

{
  for (int ii = 0; ii < (int) _channels.size(); ii++) {
    delete _channels[ii];
  }
  _channels.clear();
}

//////////////////////////////////////////////////
// Run

int CalCompute::Run ()
{

  // read in cal data

  if (_readCalFile(_params.input_path)) {
    return -1;
  }

  // compute cal fit

  for (int ii = 0; ii < (int) _channels.size(); ii++) {
    if (_channels[ii]->computeFit()) {
      return -1;
    }
  }
  
  // print

  cout << "Calibration file: " << _params.input_path << endl;
  for (int ii = 0; ii < (int) _channels.size(); ii++) {
    cout << "==============================" << endl;
    cout << "Channel: " << ii << endl;
    _channels[ii]->print(cout);
  }
  cout << "==============================" << endl;

  return 0;
  
}

////////////////////
// process a file

int CalCompute::_readCalFile(const char *file_path)
  
{
  
  if (_params.debug) {
    cerr << "Reading in cal file: " << file_path << endl;
  }
  
  // Open the file
  
  FILE *in;
  if((in = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - CalCompute::_readCalFile" << endl;
    cerr << "  Cannot open catalog file: "
	 << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // read though the file, reading available lines
  
  char line[5000];
  int iret = 0;
  while(fgets(line, 5000, in) != NULL) {

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << line;
    }

    // skip comments

    if (line[0] == '#') {
      continue;
    }

    // tokenize the line, using spaces as the delimiters
    
    vector<string> toks;
    _tokenize(line, " ", toks);

    int minToksExpected = _maxColNum + 1;
    
    if ((int) toks.size() < minToksExpected) {
      if (_params.debug) {
        cerr << "ERROR - CalCompute::_readCalFile" << endl;
        cerr << "  Not enough columns in line: " << line << endl;
        cerr << "  Need at least: " << minToksExpected << endl;
        cerr << "  Found only: " << toks.size() << endl;
        cerr << "  File: " << file_path << endl;
      }
      continue;
    }

    // scan the tokens, set the data fields

    double siggenDbm;
    if (sscanf(toks[0].c_str(), "%lg", &siggenDbm) != 1) {
      cerr << "ERROR - CalCompute::_readCalFile" << endl;
      cerr << "  Cannot read siggen dbm" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }

    for (int ii = 0; ii < (int) _channels.size(); ii++) {

      int col = _params._input_channels[ii].column_num;
      double ifdDbm;
      if (sscanf(toks[col].c_str(), "%lg", &ifdDbm) != 1) {
        cerr << "ERROR - CalCompute::_readCalFile" << endl;
        cerr << "  Cannot read ifd dbm" << endl;
        cerr << "  line: " << line << endl;
        iret = -1;
        continue;
      }

      _channels[ii]->addDataPoint(siggenDbm, ifdDbm);
      
    } // ii

  } // while
    
  fclose(in);
  
  return iret;

}

//////////////////////////////////////////////
// tokenize a string into a vector of strings

void CalCompute::_tokenize(const string &str,
                           const string &spacer,
                           vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  while (true) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      return;
    } else if (end == string::npos) {
      string tok;
      tok.assign(str, start, string::npos);
      toks.push_back(tok);
      return;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}

