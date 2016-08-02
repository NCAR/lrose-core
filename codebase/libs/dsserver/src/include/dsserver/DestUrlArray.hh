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
// DestUrlArray.hh
//
// DestUrlArray object
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////
//
// Creates an array of destination DsURL objects.
//
// The array can be created by:
//   (a) specifying URL strings to add, or
//   (b) combining a list of hosts in a file with a template URL.
//   (c) reading in a list of complete URLs from a file
//
// The constructor creates an array with no entries.
//
// To use method (a), use add() to add a DsURL by specifying the
// URL string.
//
// To use method (b), use load() to specify the host list file and
// the template.
//   The host list file format has one host per line.
//   Comments start with '#'.
//
// To use method (c), use load() to specify the host list file 
//   The url list file format has one url per line.
//   Comments start with '#'.
//
// If ignore_local is true in the constructor, the local host is
// filtered from the list.
//
// Use erase() to erase the list.
//
// Use setDebugOn() and setDebugOff() to control debugging messages.
//
///////////////////////////////////////////////////////////////

#ifndef DestUrlArray_HH
#define DestUrlArray_HH

#include <didss/DsURL.hh>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class DestUrlArray {
  
public:

  // Constructor
  // If ignore_local is true, a hostname which is local will
  // be ignored in constructing the array.
  
  DestUrlArray(const string &prog_name = "",
	       const bool ignore_local = true);

  // destructor
  
  virtual ~DestUrlArray();

  // erase
  
  void erase();

  // set strict local check.
  // If set, then strict checking is imposed for determining whether
  // a host is local.
  // WARNING: this is NOT THREAD SAFE.

  void setStrictCheckForLocalHost(bool strict) { _strictLocalCheck = strict; }
  
  // Load up the destination URL array from the host list file and 
  // the url template.
  //
  // Returns 0 on success, -1 on failure.
  
  int load(const string &dest_host_list_file_path,
	   const string &dest_url_template_str);

  // Load up the destination URL array from the URL list file
  //
  // Returns 0 on success, -1 on failure.
  
  int load(const string &dest_url_list_file_path);

  // Add a URL to the array
  //
  // Returns 0 on success, -1 on failure.

  int add(const string &url_str);

  // data access after load
  
  size_t size() { return (_destUrls.size()); }
  string &getUrl(size_t i) { return (_destUrls[i]); }
  string &operator[](size_t i) { return (_destUrls[i]); }

  // debugging

  void setDebugOn() { _debug = true; }
  void setDebugOff() { _debug = false; }

  // override the directory path of the URLs
  
  void overrideDir(const string &dir_path);
  
  // Compute the final directory from the relative path
  // given in the URL, if a dir was specified in the url
  
  void setDirRelative(const string &rel_path);

  // print

  void print(ostream &out);

  // error string
  
  const string &getErrStr() { return (_errStr); }
  const string &getErrorStr() { return (_errStr); }
  const string &getErrorString() { return (_errStr); }

protected:
  
  string _progName;
  bool _ignoreLocal;
  bool _strictLocalCheck;
  bool _debug;
  string _errStr;
  vector<string> _destUrls;

  void _clearErrStr() { _errStr = ""; }
  void _addIntErr(const char *err_str, const int iarg);
  void _addStrErr(const char *err_str, const string &sarg);

private:

};

#endif
