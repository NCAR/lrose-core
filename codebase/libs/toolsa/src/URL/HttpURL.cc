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
/////////////////////////////////////////////////////////
// HttpURL derived class - F. Hage Feb 1999
/////////////////////////////////////////////////////////

#include <toolsa/HttpURL.hh>
#include <string>
#include <cstdio>
#include <cstdlib>
using namespace std;

//////////////
// constructor

HttpURL::HttpURL() : URL()
{
  
}

HttpURL::HttpURL(const string &urlStr) : URL()
{
  setURLStr(urlStr);
}

///////////////////////
// virtual destructor

HttpURL::~HttpURL()
{

}

//////////////////////
// set the URL string

int HttpURL::setURLStr(const string &urlStr)
{

  // initialize

  _init();
  _urlStr = urlStr;
  
  // get protocol

  size_t currPos = 0;
  size_t nextPos = _urlStr.find(ColonSlashSlash);
  if (nextPos == string::npos) {
    // no protocol found
    _errString  = "HttpURL string has no protocol.";
    _errString += "Protocol must be http or https";
    _errString += _urlStr;
    _protocol = "no-protocol";
    return -1;
  }
  
  _protocol.assign(_urlStr, 0, nextPos - currPos);

  // check protocol
  
  if (_protocol != "http" && _protocol != "https") {
    _errString  = "HttpURL string has illegal protocol.";
    _errString += "Protocol must be http or https";
    _errString += _urlStr;
    _protocol = "no-protocol";
    return -1;
  }
  
  currPos = nextPos + ColonSlashSlash.size();
  if (currPos >= _urlStr.size()) {
    _errString  = "URL string has nothing following ://";
    _errString += _urlStr;
    _host = "no-host";
    _file = "no-file";
    return -1;
  }
  
  // Pull Out the host and port info
  // Search for the slash at end of host,
  // plus optional colon for alternate port

  _port = 80;
  size_t colonPos = _urlStr.find(Colon, currPos);
  size_t slashPos = _urlStr.find(Slash, currPos);
  if (slashPos == string::npos) {
    _errString  = "No host part found";
    _errString += "URL: ";
    _errString += _urlStr;
    _host = "no-host";
    return -1;
  }

  if (colonPos != string::npos && colonPos < slashPos) {
    _host.assign(_urlStr, currPos, colonPos - currPos);
    currPos = colonPos + Colon.size();
    string portString(_urlStr, currPos, slashPos - currPos);
    if (sscanf(portString.c_str(), "%d", &_port) != 1) {
      _port = 80;
    }
    currPos = slashPos + Slash.size();
  } else {
    _host.assign(_urlStr, currPos, slashPos - currPos);
    currPos = slashPos + Slash.size();
  }
  
  // File/Resource part
  
  if (currPos >= _urlStr.size()) {
    _file = "/";
    return 0 ;
  }

  // look for args
  
  string args;
  nextPos = _urlStr.find(QuestionMark, currPos);
  if (nextPos == string::npos) {
    _file.assign(_urlStr, currPos, nextPos - currPos);
  } else {
    _file.assign(_urlStr, currPos, nextPos - currPos);
    currPos = nextPos + Ampersand.size();
    if (currPos >= _urlStr.size()) {
      _errString  = "Args delimiter present, but no args: ";
      _errString += _urlStr;
      _args = "no-args";
      return -1;
    } else {
      // The rest of the string is args.
      args.assign(_urlStr, currPos, string::npos);
    }
  }

  // load up args

  _loadArgPairs(args);

  _isValid = true;
  return 0;

}

/////////////////////////////////////////////////////
// get URL string
//
// Computes string from previously stored components

string HttpURL::getURLStr() const
{

  // if already valid, return URL that was used to
  // set the object state

  if (_isValid) {
    return _urlStr;
  }

  // if not valid, then individual properties have been set
  // and we need to construct the URL

  _urlStr = _protocol;
  _urlStr += ColonSlashSlash;
  _urlStr += _host;
  
  if (_port != 80) {
    _urlStr += Colon;
    char buf[20];
    sprintf(buf, "%d", _port);
    _urlStr += buf;
  }
  
  _urlStr += Slash;
  _urlStr += _file;
  
  if (_args.size() > 0) {
    _urlStr += QuestionMark;
    _urlStr += _args;
  }
  
  // Update the validity flag.
  _isValid = true;

  return _urlStr;
  
}

///////////////////////////////////////////////////////
// Check the URL for consistency.
// If URL member is not valid, try to create a valid
// one from the stored properties.
// Sets the _isValid flag appropriately.

bool HttpURL::checkValid()
{

  // already valid?
  
  if (_isValid) {
    return true;
  }

  // construct the URL string from individual properties

  getURLStr();
  if (_isValid) {
    return true;
  } else {
    return false;
  }
}

///////////////////////////////////////////////
// Check a URL string for consistency.
// Static method. Can be called without object.

bool HttpURL::checkValid(string url)
{
  HttpURL test(url);
  if (test.isValid()) {
    return true;
  } else {
    return false;
  }
}

///////////////////////////////////////////////  
// print

void HttpURL::print(ostream &out) const
{
  out << "  urlStr: " << _urlStr << endl;
  out << "  isValid: " << _isValid << endl;
  out << "  protocol: " << _protocol << endl;
  out << "  host: " << _host << endl;
  out << "  port: " << _port << endl;
  out << "  file: " << _file << endl;
  out << "  args: " << _args << endl;
  out << "  nArgs: " << _argList.size() << endl;
  if (_argList.size() > 0) {
    out << "  argList: " << endl;
    for (size_t ii = 0; ii < _argPairs.size(); ii++) {
      out << "    " << _argList[ii] << endl;
    }
  }
  out << "  nArgPairs: " << _argPairs.size() << endl;
  if (_argPairs.size() > 0) {
    out << "  argPairs: " << endl;
    for (size_t ii = 0; ii < _argPairs.size(); ii++) {
      out << "    " << _argPairs[ii].first
	  << " = " << _argPairs[ii].second << endl;
    }
  }
  out << "computed urlStr: " << getURLStr() << endl;
  
}

