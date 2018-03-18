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

//////////////////////////////////////////////////////////
// DsServerMsg.cc
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
//
// Jan 1999
//
////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/URL.hh>
#include <toolsa/GetHost.hh>
#include <didss/DsMsgPart.hh>
#include <dsserver/DsServerMsg.hh>
#include <iostream>
#include <cstring>
#include <cstdlib>
using namespace std;

//////////////
// constructor
//
// Memory model defaults to local copy

DsServerMsg::DsServerMsg(memModel_t mem_model /* = CopyMem */) :
  DsMessage(mem_model)
{
  _category = Generic;
}

/////////////////////////////
// Copy constructor
//

DsServerMsg::DsServerMsg(const DsServerMsg &rhs)

{
  if (this != &rhs) {
    _copy(rhs);
  }
}

/////////////
// destructor

DsServerMsg::~DsServerMsg()
{
}

//////////////////////////////
// Assignment
//

DsServerMsg &DsServerMsg::operator=(const DsServerMsg &rhs)
  
{
  return _copy(rhs);
}

//////////////////////////                                                    
// decode a message header
//
// This is used if you just want to peek at the header before       
// deciding how to handle the message.
//
// If msg_len is set, checks that the msg is big enough
//   to hold at least a DsMsgHdr_t. Otherwise, assumes            
//   that the message is big enough and blindly copies            
//   memory.
//
// Subclasses *must* call this version if they define
//   this method and want checking.
//                                        
// Returns: -1 If msg_len is set and is smaller than a DsMsgHdr_t.
//               or if the category of the message doesn't match
//               one of the valid categories.
//           0 Otherwise.          
// Virtual.
int DsServerMsg::decodeHeader(const void *in_msg, ssize_t msg_len /* = -1*/ )
{
  int status = DsMessage::decodeHeader(in_msg, msg_len);
  if (status < 0) {
    return -1;
  }

  if (_category < Generic || _category > LastCategory) {
    return -1;
  }

  return 0;
}

/////////////////////////////////////
// set the message header attributes
//
// These overwrite the existing attributes.

void DsServerMsg::setHdrAttr(const int type,
			     const int sub_type /* = -1*/,
			     const int mode /* = -1*/,
			     const int flags /* = 0*/,
			     const int major_version /* = 1*/,
			     const int minor_version /* = 0*/,
			     const int serial_num /* = -1*/,
			     const category_t category /* = Generic*/ )

{
  _type = type;
  _subType = sub_type;
  _mode = mode;
  _flags = flags;
  _majorVersion = major_version;
  _minorVersion = minor_version;
  _serialNum = serial_num;
  _category = (int) category;
}

// Get the first URL string in the message, if any.
// Returns filled string on success, empty string on failure.

string DsServerMsg::getFirstURLStr() const
{
  if (!partExists(DS_URL)) {
    return ("");
  }
  
  DsMsgPart * part = getPartByType(DS_URL);
  if (part == NULL) {
    return ("");
  }
  
  // URL string in part must be null-terminated.
  char * urlStr = (char *) part->getBuf();
  if ((int)strlen(urlStr) >= part->getLength()) {
    // String must not be null-terminated.
    return ("");
  }
  
  return (urlStr);

}

// Get the first URL in the message, if any.
//   Returns: NULL if there is no url.
//            Pointer to a DsURL if successful:
//              o Returned DsURL may be invalid, if srcString was corrupt.
//              o Caller is responsible for deleting returned DsURL.
// 
DsURL * DsServerMsg::getFirstURL() const
{
    string urlString(getFirstURLStr());
    if (urlString == "") {
      return NULL;
    }
    DsURL * url = new DsURL(urlString);
    return url; // Must be checked for validity by caller!
}

string DsServerMsg::getFirstErrString() const
{
    string errString("");

    if (!partExists(DS_ERR_STRING)) {
        return errString;
    }

    DsMsgPart * part = getPartByType(DS_ERR_STRING);
    if (part == NULL) {
        return errString;
    }

    // String in part must be null-terminated.
    char * rawString = (char *) part->getBuf();
    if ((int)strlen(rawString) >= part->getLength()) {
        // String is not be null-terminated.
        return errString;
    }

    errString = rawString;
    return errString;
}

string DsServerMsg::getFirstString() const
{
    string thisString("");

    if (!partExists(DS_STRING)) {
        return thisString;
    }

    DsMsgPart * part = getPartByType(DS_STRING);
    if (part == NULL) {
        return thisString;
    }

    // String in part must be null-terminated.
    char * rawString = (char *) part->getBuf();
    if ((int)strlen(rawString) >= part->getLength()) {
        // String is not be null-terminated.
        return thisString;
    }

    thisString = rawString;
    return thisString;
}

int DsServerMsg::getFirstInt() const
{
    int thisInt(-1);

    if (!partExists(DS_INT)) {
        return thisInt;
    }

    DsMsgPart * part = getPartByType(DS_INT);
    if (part == NULL) {
        return thisInt;
    }

    // Int in part must be big enough.
    if (part->getLength() != sizeof(int)) {
        return thisInt;
    }

    const unsigned char * rawInt = part->getBuf();
    return BE_to_si32( *((int*)rawInt) );
}

int DsServerMsg::addURL(const string & url_str) {
    if (url_str.size() <= 0) {
        return -1;
    }
    addPart(DS_URL, url_str.size() + 1, url_str.c_str());
    return 0;
}

int DsServerMsg::addURL(const DsURL & url) {
    string urlString = url.getURLStr();
    if (urlString.size() <= 0) {
        return -1;
    }

    addPart(DS_URL, urlString.size() + 1, urlString.c_str());
    return 0;
}

// add URL, stripping off any forwarding info

int DsServerMsg::addURLNoFwd(const DsURL & url) {
  string urlString = url.getURLStrNoFwd();
  if (urlString.size() <= 0) {
    return -1;
  }
  
  addPart(DS_URL, urlString.size() + 1, urlString.c_str());
  return 0;

}

int DsServerMsg::addErrString(const string & err) {
    if (err.size() <= 0) {
        return -1;
    }

    addPart(DS_ERR_STRING, err.size() + 1, err.c_str());
    return 0;
}

int DsServerMsg::addString(const string & str)
{
    if (str.size() <= 0) {
        return -1;
    }

    addPart(DS_STRING, str.size() + 1, str.c_str());
    return 0;
}

int DsServerMsg::addInt(const int & integer)
{
    int BEint = BE_from_si32(integer);
    addPart(DS_INT, sizeof(integer), &BEint);
    return 0;
}

int DsServerMsg::addClientUser() {
  const char *user = getenv("USER");
  if (user == NULL) {
    user = "unknown";
  }
  addPart(DS_CLIENT_USER, strlen(user) + 1, user);
  return 0;
}

string DsServerMsg::getClientUser() const
{
  string thisString("");
    
  if (!partExists(DS_CLIENT_USER)) {
    return thisString;
  }
  
  DsMsgPart * part = getPartByType(DS_CLIENT_USER);
  if (part == NULL) {
    return thisString;
  }
  
  // String in part must be null-terminated.
  char * rawString = (char *) part->getBuf();
  if ((int)strlen(rawString) >= part->getLength()) {
    // String is not be null-terminated.
    return thisString;
  }
  
  thisString = rawString;
  return thisString;
}

string DsServerMsg::getClientHost() const
{
  string thisString("");
    
  if (!partExists(DS_CLIENT_HOST)) {
    return thisString;
  }
  
  DsMsgPart * part = getPartByType(DS_CLIENT_HOST);
  if (part == NULL) {
    return thisString;
  }
  
  // String in part must be null-terminated.
  char * rawString = (char *) part->getBuf();
  if ((int)strlen(rawString) >= part->getLength()) {
    // String is not be null-terminated.
    return thisString;
  }
  
  thisString = rawString;
  return thisString;
}

int DsServerMsg::addClientHost() {
  GetHost get;
  string host = get.localHostNameFull();
  if (host.size() == 0) {
    host = "unknown";
  }
  addPart(DS_CLIENT_HOST, host.size() + 1, host.c_str());
  return 0;
}

string DsServerMsg::getClientIpaddr() const
{
  string thisString("");
    
  if (!partExists(DS_CLIENT_IPADDR)) {
    return thisString;
  }
  
  DsMsgPart * part = getPartByType(DS_CLIENT_IPADDR);
  if (part == NULL) {
    return thisString;
  }
  
  // String in part must be null-terminated.
  char * rawString = (char *) part->getBuf();
  if ((int)strlen(rawString) >= part->getLength()) {
    // String is not be null-terminated.
    return thisString;
  }
  
  thisString = rawString;
  return thisString;
}

int DsServerMsg::addClientIpaddr() {
  GetHost get;
  string ipaddr = get.localIpAddr();
  if (ipaddr.size() == 0) {
    ipaddr = "unknown";
  }
  addPart(DS_CLIENT_IPADDR, ipaddr.size() + 1, ipaddr.c_str());
  return 0;
}

//////////////////////////////////////
// set everything to its initial state
//
void DsServerMsg::clearAll()
{
    DsMessage::clearAll();
    _category = Generic;
}

//////////////////////////////////////////
// print out main header and parts headers
//

void DsServerMsg::print(ostream &out, const char *spacer) const
{

  printHeader(out, spacer);

  // create map of ids and labels

  PartHeaderLabelMap labels;
  labels.insert(PartHeaderLabel(DS_URL, "DS_URL"));
  labels.insert(PartHeaderLabel(DS_VERSION, "DS_VERSION"));
  labels.insert(PartHeaderLabel(DS_SERIAL, "DS_SERIAL"));
  labels.insert(PartHeaderLabel(DS_ERR_STRING, "DS_ERR_STRING"));
  labels.insert(PartHeaderLabel(DS_STRING, "DS_STRING"));
  labels.insert(PartHeaderLabel(DS_INT, "DS_INT"));
  labels.insert(PartHeaderLabel(DS_CLIENT_USER, "DS_CLIENT_USER"));
  labels.insert(PartHeaderLabel(DS_CLIENT_HOST, "DS_CLIENT_HOST"));
  labels.insert(PartHeaderLabel(DS_CLIENT_IPADDR, "DS_CLIENT_IPADDR"));

  // print parts using the labels

  printPartHeaders(out, spacer, labels);

}

////////////////////////////////
// print out the message header
//

void DsServerMsg::printHeader(ostream &out, const char *spacer) const

{

  DsMessage::printHeader(out, spacer);

  string category("");
  if (_category == Generic) {
    category = "Generic";
  } else if (_category == ServerStatus) {
    category = "ServerStatus";
  } else if (_category == StartPut) {
    category = "StartPut";
  } else if (_category == StartGet) {
    category = "StartGet";
  } else if (_category == EndSeries) {
    category = "EndSeries";
  } else {
    category = "Unknown";
  }
  out << spacer << "        category: " << category << endl;

  // do not try to interpret -1

  if (_error == -1) {
    return;
  }

  // Go through the masked errors, and remove each one.

  string errString("");
  int localError = _error;
  if (localError & BAD_MESSAGE) {
    localError &= ~BAD_MESSAGE;
    errString += "BAD_MESSAGE ";
  }
  if (localError & SERVER_ERROR) {
    localError &= ~SERVER_ERROR;
    errString += "SERVER_ERROR ";
  }
  if (localError & UNKNOWN_COMMAND) {
    localError &= ~UNKNOWN_COMMAND;
    errString += "UNKNOWN_COMMAND ";
  }
  if (localError & BAD_URL_PROVIDED) {
    localError &= ~BAD_URL_PROVIDED;
    errString += "BAD_URL_PROVIDED ";
  }
  if (localError & BAD_HOST_PROVIDED) {
    localError &= ~BAD_HOST_PROVIDED;
    errString += "BAD_HOST_PROVIDED ";
  }
  if (localError & BAD_PORT_PROVIDED) {
    localError &= ~BAD_PORT_PROVIDED;
    errString += "BAD_PORT_PROVIDED ";
  }
  if (localError & SERVICE_DENIED) {
    localError &= ~SERVICE_DENIED;
    errString += "SERVICE_DENIED ";
  }
  if (localError & NO_SERVICE_AVAIL) {
    localError &= ~NO_SERVICE_AVAIL;
    errString += "NO_SERVICE_AVAIL ";
  }

  // Check if there were any un-handled error bits.
  if (localError != 0) {
    char buf[128];
    sprintf(buf, "%d", localError);
    errString += "\n";
    errString += "Unrecognized error num: ";
    errString += buf;
  }

  out << spacer << "        errors: " << errString << endl;

}

////////////////////////////////
// backwards compatibility print

void DsServerMsg::printHeader(ostream *out, const char *spacer) const
{
  printHeader(*out, spacer);
}

////////////////////////////////////////////////////////////////
// check that a url is secure
//
// This checks for absolute paths and .. relative paths in a
// url. If these are present, the url may point to data which
// is not below DATA_DIR, making the request insecure.
//
// returns true if secure, false if not.
// Appends error information to errorStr.

bool DsServerMsg::urlIsSecure(const string &urlStr,
			      string &errorStr)
    
{

  DsURL url(urlStr);
  const string &dir = url.getFile();
  if (dir.size() < 1) {
    errorStr += "ERROR - DsServerMsg::urlIsSecure\n";
    errorStr += "  Zero-length dir\n";
    return false;
  }
  size_t firstNonWSpace = dir.find_first_not_of(" \t\n\r", 0);
  if (firstNonWSpace != string::npos) {
    if (dir[firstNonWSpace] == '/') {
      errorStr += "ERROR - DsServerMsg::urlIsSecure\n";
      errorStr += "  dir has absolute path\n";
      return false;
    }
  }
  if (dir.find("..", 0) != string::npos) {
    errorStr += "ERROR - DsServerMsg::urlIsSecure\n";
    errorStr += "  path contains '..' entries.\n";
    return false;
  }

  return true;

}
  
//////////////////////////////
// copy contents

DsServerMsg &DsServerMsg::_copy(const DsServerMsg &rhs)
  
{
  
  if (&rhs == this) {
    return *this;
  }

  // base class copy

  DsMessage::_copy(rhs);

  return *this;
  
}

