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
// Base and derived classes for dealing with different TAF inputs.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2000
//
///////////////////////////////////////////////////////////////

#include "Input.hh"
#include <cerrno>
#include <string>
#include <vector>
#include <toolsa/file_io.h>
#include <toolsa/TaArray.hh>
using namespace std;

//////////////////////
// Input - base class

// constructor
  
Input::Input(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{

  _in = NULL;
  _inTafBlock = false;
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
  _inTafBlock = false;
  _line[0] = '\0';
  _saved[0] = '\0';
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
  strncpy(_saved, buffer, MaxLine-1);
}

////////////////////////////////////////
// gets either from file or saved buffer

char *Input::_fgets(char *buffer)
{
  if(*_saved != '\0')
  {
    strncpy(buffer, _saved, MaxLine);
    *_saved = '\0';
    return buffer;
  }

  char *line = fgets(buffer, MaxLine, _in);
  return line;
}

////////////////////////////////////////////////////////
// Input::readNext() - generic
//
// read the next TAF from AFTN or GTS file
//
// returns 0 on success, -1 on failure (no more TAFS)

int Input::readNext(string &tafStr)

{
  
  tafStr = "";
  memset(_line, 0, MaxLine);
  
  while (_fgets(_line) != NULL) {
    
    char *line = _line;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "input --> " << line;
    }
    
    // NIL line - ignore
    
    if (strstr(line, "NIL=")) {
      continue;
    }

    // look for ^C or ^A
    
    if(strchr(line, '\003') != NULL || strchr(line, '\001') != NULL)  {
      _inTafBlock = false;
      continue;
    }

    // tokenize the line
    
    vector<string> toks;
    TaArray<char> lineCopy_;
    char *lineCopy = lineCopy_.alloc(strlen(line) + 1);
    strcpy(lineCopy, line);
    char *tok = strtok(lineCopy, " ");
    while (tok) {
      toks.push_back(tok);
      tok = strtok(NULL, " ");
    }

    // check for ^BFT
    
    if (strlen(line) >= 13 && strncmp(line, "\002FT", 3) == 0) {
      
      _inTafBlock = true;
      _headStr = line + 1;
      tafStr = "";
      continue;
      
    } else {
      
      // check for "TAF" start
      
      if(strlen(line) >= 3 && strncmp(line, "TAF", 3) == 0) {
	_inTafBlock = true;
	tafStr = "";
      } else if (strlen(line) >= 4 && strncmp(line + 1, "TAF", 3) == 0) {
	_inTafBlock = true;
	tafStr = "";
	line++;
      }
      
    } // if (strlen(line) >= 13 ...
    
    if(_inTafBlock) {
      
      // check for empty line - indicates break
      
      bool empty = true;
      for (int i = 0; i < (int) strlen(line); i++) {
	if (!isspace(line[i])) {
	  empty = false;
	  break;
	}
      }
      if (empty) {
	_inTafBlock = false;
	if (tafStr.size() > 0) {
	  return 0;
	} else {
	  continue;
	}
      }

      // add this line to the TAF
      
      tafStr += line;
      
      // check for equals - indicates end of TAF
      
      if (strchr(line, '=') != NULL) {
	return 0;
      }
      
    } // if(_inTafBlock)

  } // while
  
  _inTafBlock = false;
  return -1;
  
}

////////////////////////////////////////////////////////////////  
// NnnnInput constructor
  
NnnnInput::NnnnInput(const string &prog_name, const Params &params) :
  Input(prog_name, params)

{

}

////////////////////////////////////////////////////////
// NnnnInput::readNext()
//
// read the next TAF from the NNNN file
//
// returns 0 on success, -1 on failure (no more TAFS)

int NnnnInput::readNext(string &tafStr)

{

  _headStr = "";
  tafStr = "";
  memset(_line, 0, MaxLine);
  
  while (_fgets(_line) != NULL) {

    char *line = _line;
    
    // empty line - ignore
    
    bool empty = true;
    for (int i = 0; i < (int) strlen(line); i++) {
      if (!isspace(line[i])) {
	empty = false;
	break;
      }
    }
    if (empty) {
      continue;
    }

    // NIL line - ignore
    
    if (strstr(line, "NIL=")) {
      continue;
    }

    // ZCZC - start of message
    
    if (strstr(line, "ZCZC") != NULL) {
      _inTafBlock = false;
      continue;
    }

    // FTCI line - optional header

    if(strncmp(line, "FTCI", 4) == 0) {
      _inTafBlock = true;
      _headStr = line;
      continue;
    }

    // TAF - start of TAF
    
    if(strncmp(line, "TAF", 3) == 0) {
      _inTafBlock = true;
      _headStr += line;
      continue;
    }
    
    if(_inTafBlock) {
      
      // check for NNNN
      
      if (strstr(line, "NNNN")) {
	_inTafBlock = false;
	if (tafStr.size() > 0) {
	  return 0;
	} else {
	  continue;
	}
      }
      
      // add this line
      
      tafStr += line;

      // check for equals - indicates end of TAF
      
      if (strchr(line, '=') != NULL) {
	return 0;
      }
      
    } // if(_inTafBlock)

  } // while
  
  _inTafBlock = false;
  return -1;
  
}

