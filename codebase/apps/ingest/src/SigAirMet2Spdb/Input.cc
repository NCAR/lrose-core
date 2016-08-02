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
/////////////////////////////////////////////////////////////
// Input.cc
//
// Base and derived classes for dealing with different SIG/AIRMET inputs.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2003
//
///////////////////////////////////////////////////////////////

#include "Input.hh"
#include <cerrno>
#include <cstring>
#include <vector>
#include <toolsa/file_io.h>

//////////////////////
// Input - base class

// constructor
  
Input::Input(const string &prog_name, const Params &params) :
  _progName(prog_name),
  _params(params)
  
{
  
  _in = NULL;
  _line[0] = '\0';
  _saved[0] = '\0';

}

// destructor
  
Input::~Input() 
{
  Input::close();
}

// open input file
  
int Input::open(const char *file_path)

{
  Input::close();
  if ((_in = ta_fopen_uncompress((char *) file_path, "r")) == NULL) {
    cerr << "ERROR - " << _progName << ":Input::open" << endl;
    cerr << "  Cannot open input file." << endl;
    cerr << "  " << file_path << ": " << strerror(errno) << endl;
    return -1;
  }
  _line[0] = '\0';
  _saved[0] = '\0';
  _headStr = "";
  return 0;
}

// close file

void Input::close()
{
  if (_in != NULL) {
    fclose(_in);
  }
  _in = NULL;
}

//////////////////////////////////
// save the line buffer for later

void Input::_fgets_save(char *buffer)
{
  strncpy(_saved, buffer, MaxLine);
};

////////////////////////////////////////
// gets either from file or saved buffer

char *Input::_fgets(char *buffer)
{
  if(*_saved != '\0') {
    strncpy(buffer, _saved, MaxLine);
    *_saved = '\0';
    return buffer;
  }
  char *line = fgets(buffer, MaxLine, _in);
  return line;
}

////////////////////////////////////////////////////////
// Input::readNext()
//
// read the next SIGMET or AIRMET
//
// returns 0 on success, -1 on failure (no more SIGMETS)

int Input::readNext(string &report_str)

{

  bool reportFound = false;
  int blankLineNum=0;
  int lineNum=0;
  string reportStr;

  memset(_line, 0, MaxLine);
  
  while (_fgets(_line) != NULL) {

    char *line = _line;

    lineNum++;

    // NIL line - ignore
    if (strstr(line, "NIL=")) {
      continue;
    }

    // 999 line - ignore
    if (strcmp(line, "999\n") == 0) {
      continue;
    }

    // look for  and/or 
   
    // DIXON - changing to handle ^C^A which are not together
    // if(strstr(line, "\003\001")) {

    if(strchr(line, '\003') || strchr(line, '\001')) {

      // if in report, save line for next time and return
      
      if (reportFound) {
	_fgets_save(line);
	if (reportStr.size() > 0) {
	  report_str = _headStr + reportStr;
	  return 0;
	}
      }

      // not in report, re-initailize

      _headStr = "";
      reportFound = false;
      reportStr = "";
      continue;

    }

    // check for ^B - ATFN

    char *ctrlB = strchr(line, '\002');
    if (ctrlB != NULL) {
      _headStr = ctrlB + 1;
      reportFound = false;
      reportStr = "";
      continue;
    }

    // check for empty line after start of sigmet - could indicate break

    if (reportFound) {
      bool empty = true;
      for (int i = 0; i < (int) strlen(line); i++) {
	if (!isspace(line[i])) {
	  empty = false;
	}
      }
      if (empty) {
	blankLineNum=lineNum;
      }
    }
      
    // check for SIGMET or AIRMET
    
    if (strstr(line, "SIGMET") || strstr(line, "AIRMET")) {

      //      cerr << "next: line has AIRMET or SIGMET, reportFound: " << reportFound << ", lineNum: " << lineNum << ", blankLineNum: " << blankLineNum << ", line: " << line << endl;


      //      if ((reportFound) && (!strstr(line, "CNL"))) {

      if ((reportFound) && (!_ignoreKeyword(line))) {

	// already have valid sigmet, so return now. Ignore for
	// case of a CANCEL. save line for next time and return.

	_fgets_save(line);
	if (reportStr.size() > 0) {
	  report_str = _headStr + reportStr;
	  return 0;
	}

      }

      if ((reportFound) && (blankLineNum+1 == lineNum)) {

	// if in report, save line for next time and return

	_fgets_save(line);
	if (reportStr.size() > 0) {
	  report_str = _headStr + reportStr;
	  return 0;
	}
      }

      reportFound = true;
      reportStr += line;
      continue;
    }

    if (reportFound) {
      // add to report
      reportStr += line;
    } else {
      // add to header
      _headStr += line;
    }

  } // while

  if (reportFound) {
    report_str = _headStr + reportStr;
    return 0;
  }

  _headStr = "";
  return -1;
  
}

////////////////////////////////////////////////////////
// Input::ignoreKeyword
//
// Does this line have a CANCEL, AMEND, CORRECT, SEE ALSO
// 
// return true if found word to ignore, false otherwise

bool Input::_ignoreKeyword(char *line) 

{
  if (strstr(line, "CNL") ||
      strstr(line, "CANCEL") ||
      strstr(line, "AMEND") ||
      strstr(line, "CORRECT") ||
      strstr(line, "REISSUE") ||
      strstr(line, "SEE ALSO") ||
      strstr(line, "SEE INTL")) {
    return true;
  }
  return false;
}
