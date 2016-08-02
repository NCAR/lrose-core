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

#ifndef URL_INCLUDED
#define URL_INCLUDED

#include <string>
#include <iostream>
#include <vector>
using namespace std;

// Abstract URL base class.
// 
//   Subclasses implement: setURLStr() and getURLStr()
//                         which take care of specific protocols.

class URL {

public:

  // default constructor
  
  URL();

  // constructor from URL

  URL(const string &urlStr);
  
  // NOTE: copy constructor and assignment are automatically
  // supplied by compiler since all members are OK for copy and
  // assignment
  
  // destructor - virtual
  
  virtual ~URL();
  
  // Set the URL string, and decode it into its component parts.
  // sets valid flag appropriately
  // returns 0 on success, -1 on failure
  // this is pure virtual - must be provided on subclasses
  
  virtual int setURLStr(const string &urlStr) = 0;
  
  // Methods to set individual members of the class.
  // 
  // Using these methods sets the validity of the URL to false.
  //   This is not reset until the string representation is retrieved
  //   via getURLStr().

  virtual inline void setProtocol(const string &protocol) {
    _isValid = false;
    _protocol = protocol;
  }
  
  void inline setHost(const string &host) {
    _isValid = false;
    _host = host;            
  }
  
  void inline setPort(int port) {
    _isValid = false;
    _port = port;             
  }
  
  void inline setFile(const string &file) {
    _isValid = false;
    _file = file;             
  }

  void inline setArgs(const string &args) {
    _isValid = false;
    _args = args;             
  }

  // Get methods for properties

  const string &getProtocol() const   { return _protocol;   }
  const string &getHost() const { return _host; }
  int getPort() const { return _port; }
  const string &getFile() const { return _file; }
  const string &getArgs() const { return _args; }
  const vector<string> &getArgList () const { return _argList; }
  const vector<pair<string, string> > &getArgPairs () const {
    return _argPairs;
  }

  // Get the string representation of the URL.
  // Computes string from previously stored components
  // Returns:
  //   On success: a string representing the URL
  //      sets _isValid to true
  //   On error:
  //      An empty string if the URL has no protocol,
  //      or if the subclass cannot encode the protocol-specific part.
  //      sets _isValid to false

  virtual string getURLStr() const = 0;

  // Determine if the object is a valid URL.
  //   Use getErrString() to determine reason, if this returns false.
  // 
  // Note that if building a URL from parts, rather than from a string,
  //   the validity flag is not updated until the string representation
  //   is asked for by calling getURLStr().

  bool isValid() const { return _isValid; }

  // Check the URL for consistency.
  // If URL member is not valid, try to create a valid
  // one from the stored properties.
  // Sets the _isValid flag appropriately.

  virtual bool checkValid() = 0;
  
  // Get the error string for the object, which is set when
  //   _isValid is false and on error returns.

  const string &getErrString() const { return _errString; }

  // print

  virtual void print(ostream &out) const = 0;

protected:

  static const string Ampersand;
  static const string Colon;
  static const string Equals;
  static const string QuestionMark;
  static const string Slash;
  static const string SlashSlash;
  static const string ColonSlashSlash;

  mutable string _urlStr;
  string _protocol;
  string _host;
  int _port;
  string _file;
  string _args;
  vector<string> _argList;
  vector<pair<string, string> > _argPairs;

  // Object status members are never const b/c they are updated by
  //   const methods that do validity checking.

  mutable string _errString;
  mutable bool _isValid;

  void _loadArgPairs(const string &args);

  // initialize

  virtual void _init();

};

#endif
