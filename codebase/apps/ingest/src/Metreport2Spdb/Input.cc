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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: Input.cc,v 1.6 2016/03/07 01:23:02 dixon Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	Input
//
// Author:	G. M. cunning
//
// Date:	June 18 2014
//
// Description: Base class for reading MET REPORT and SPECIAL bulletins 
//		in files.
//
//


// C++ include files
#include <cstring>
#include <cstdlib>
#include <vector>

// System/RAP include files
#include <toolsa/TaFile.hh>
#include <toolsa/TaStr.hh>

// Local include files
#include "Input.hh"

using namespace std;

// define any constants
const string Input::_className = "Input";


/////////////////////////////////////////////////////////////////////////
// Constructors

Input::Input(const Params *params)
{
  _params = params;
  _filePath = "";
  _soh = '\0';
  _eoh = '\0';
  _message = "";
  _inBlock = false;

}

/////////////////////////////////////////////////////////////////////////
// Destructors
  
Input::~Input()
{
  close();
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// open
int 
Input::open(const string& file_path)
{
  const string methodName = _className + string( "::open" );

  if (string(&_soh).empty() || string(&_eoh).empty()) { 
     cerr << "SOH or EOH has not been set ..." << endl;
     return 1;
   }
    
  //  this->close();

  _inBlock = false;
  _message = "";

  _inStream.open(file_path.c_str(), ifstream::in);

  if(_inStream.good() == false) {
    cerr << "Error:: " << methodName << " -- unable to open " << file_path << endl;
    return 1;
  }

  return 0;
}

 
/////////////////////////////////////////////////////////////////////////
// close
//
void 
Input::close()
{
  _inStream.close();
}

////////////////////////////////////////////////////////
// readNext
//
// try two approaches: 1) read lines, "\n" is delimiter
// 2) read message, where delimiter is Input::_eoh
int 
Input::readNext(string &msg_str)
{
  const string methodName = _className + string( "::readNext" );

  if(_inStream.eof() == true) {
    cerr << "End of file." << endl;
    return 1;
  }

  if(_inStream.is_open() == false || _inStream.good() == false) {
    cerr << "Error:: " << methodName << " -- file stream problem " << endl;
    return 1;
  }

  char eol = '\n';
  while (_inStream.eof() == false) {
    string line;
    getline(_inStream, line, _eoh);

    // make sure the message starts with _soh and strip the line containing it
    size_t where = line.find(_soh);

    if(where == string::npos) {
      continue;
    }

    // move past any interceding message block identifer lines between SOH and WMO header
    for(int i = 0; i < (_params->num_block_lines+1); i++) {
       where = line.find(eol, where) + 1;      
     }

    // move past a ^B at the start of the line and strip a '^K' and new line at end 
    where++;
    _message.assign(line, where, (line.size()-where-1));

    // filter by message type
    if(_checkType(_message, _typeIds) == false) {
      continue;
    }

    msg_str = _message;
    break;
  } // while

  // must be at end of file
  _inBlock = false;
  return 0;

}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Input::_checkType
//
bool Input::_checkType(const std::string &line, 
		       const std::vector< std::string >& type_list) 
  { 
    const string methodName = _className + string( "::_checkType" );
    
    for(size_t i = 0; i < type_list.size(); i++) {
      size_t where = line.find(type_list[i]);
      if(where == 0) { // the ID shoud be at the start of _wmoHeader
	if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_EXTRA)) {
	  cerr << "DEBUG: " << methodName << " -- found match to " << type_list[i] << endl;
	}
	return true;
      }
    }
    return false;
  }

