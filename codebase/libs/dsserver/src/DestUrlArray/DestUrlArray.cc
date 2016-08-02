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
// DestUrlArray.cc
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
// If ignore_local is true in the constructor, the local host is
// filtered from the list.
//
// Use setDebugOn() and setDebugOff() to control debugging messages.
//
///////////////////////////////////////////////////////////////

#include <dsserver/DestUrlArray.hh>
#include <didss/RapDataDir.hh>
#include <toolsa/GetHost.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <cstdio>
#include <cerrno>
using namespace std;

///////////////////////////////////////////////////////////////
// Constructor

DestUrlArray::DestUrlArray(const string &prog_name /* = ""*/,
			   const bool ignore_local /* = true*/ )

{

  _progName = prog_name;
  _ignoreLocal = ignore_local;
  _strictLocalCheck = false;
  _debug = false;

}

// destructor

DestUrlArray::~DestUrlArray()

{

}

// erase the array

void DestUrlArray::erase()

{
  _destUrls.erase(_destUrls.begin(), _destUrls.end());
}

///////////////////////////////////////////////////////////////a
// load()
//
// Load up the destination URL array from the host list file and 
// the url template.
//
// Returns 0 on success, -1 on failure.

int DestUrlArray::load(const string &dest_host_list_file_path,
		       const string &dest_url_template_str)

{

  _clearErrStr();
  _errStr += "ERROR - ";
  _errStr += _progName;
  _errStr += "::DestUrlArray::load()\n";
  TaStr::AddStr(_errStr, "  ", DateTime::str());

  erase();
  
  string destHostListFilePath(dest_host_list_file_path);
  string destUrlTemplateStr(dest_url_template_str);

  // decode template URL string

  DsURL templateUrl(destUrlTemplateStr.c_str());
  if (!templateUrl.isValid()) {
    _addStrErr("  Invalid template URL: ", destUrlTemplateStr);
    return (-1);
  }

  // open host list file

  FILE *hostFile;
  if ((hostFile = fopen(destHostListFilePath.c_str(), "r")) == NULL) {
    int errNum = errno;
    _addStrErr("  Cannot open host list file: ", destHostListFilePath);
    _addStrErr("  ", strerror(errNum));
    return (-1);
  }

  // read in host entries


  GetHost getHost;
  char line[256];
  while (fgets(line, 256, hostFile) != NULL) {
    
    if (line[0] == '#') continue; // comment line

    char hostName[256];
    if (sscanf(line, "%s", hostName) != 1) continue; // cannot parse

    if (strlen(hostName) == 0) continue; // zero-length hostname

    if (_ignoreLocal) {
      if (_strictLocalCheck) {
	if (getHost.hostIsLocal2(hostName)) continue;
      } else {
	if (getHost.hostIsLocal(hostName)) continue;
      }
    }

    // not a comment, not zero-length, not-local (if checking)
    // so compute the  url from the host and template
    // and add to array
    
    DsURL url(templateUrl);
    url.setHost(hostName);
    if (url.checkValid()) {
      string urlStr(url.getURLStr());
      _destUrls.push_back(urlStr);
    }

  } // while

  fclose(hostFile);

  if (_debug && _destUrls.size() == 0) {
    cerr << "WARNING - " << _progName << ":DestUrlArray::load()" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  No valid hosts in file: " << destHostListFilePath << endl;
  }

  return (0);

}

///////////////////////////////////////////////////////////////a
// load()
//
// Load up the destination URL array from the URL list file
//
// Returns 0 on success, -1 on failure.

int DestUrlArray::load(const string &dest_url_list_file_path)

{

  _clearErrStr();
  _errStr += "ERROR - ";
  _errStr += _progName;
  _errStr += "::DestUrlArray::load()\n";
  TaStr::AddStr(_errStr, "  ", DateTime::str());

  erase();
  
  string destUrlListFilePath(dest_url_list_file_path);

  // open url list file
  
  FILE *urlFile;
  if ((urlFile = fopen(destUrlListFilePath.c_str(), "r")) == NULL) {
    int errNum = errno;
    _addStrErr("  Cannot open url list file: ", destUrlListFilePath);
    _addStrErr("  ", strerror(errNum));
    return (-1);
  }

  // read in entries


  char line[256];
  while (fgets(line, 256, urlFile) != NULL) {
    
    if (line[0] == '#') continue; // comment line
    
    char url[256];
    if (sscanf(line, "%s", url) != 1) continue; // cannot parse
    if (strlen(url) == 0) continue; // zero-length

    // add

    add(url);

  } // while

  fclose(urlFile);
  
  if (_debug && _destUrls.size() == 0) {
    cerr << "WARNING - " << _progName << ":DestUrlArray::load()" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  No valid urls in file: " << destUrlListFilePath << endl;
  }

  return (0);


}

///////////////////////////////////////////////////////////////a
// add()
//
// Add a URL to the array
//
// Returns 0 on success, -1 on failure.

int DestUrlArray::add(const string &url_str)

{

  _clearErrStr();
  _errStr += "ERROR - ";
  _errStr += _progName;
  _errStr += "::DestUrlArray::add()\n";
  TaStr::AddStr(_errStr, "  ", DateTime::str());

  // is string a valid URL?
  
  DsURL url(url_str);
  if (!url.checkValid()) {
    _addStrErr("  Invalid URL: ", url_str);
    return (-1);
  }

  // Check for local host if the file is not set, to
  // prevent recursive puts.
  // In the URL the file points to the target directory.
  // If the directory is set, then the URL is probably
  // pointing to a different directory, and therefore
  // we do not need the host check.

  if (_ignoreLocal &&
      (url.getPort() < 0) &&
      (url.getFile().size() == 0)) {
    const string &host = url.getHost();
    GetHost getHost;
    if (_strictLocalCheck) {
      if (getHost.hostIsLocal2(host)) {
	return 0;
      }
    } else {
      if (getHost.hostIsLocal(host)) {
	return 0;
      }
    }
  }

  // OK, add to array

  _destUrls.push_back(url.getURLStr());

  return (0);

}

///////////////////////////////////////////////////////////////a
// override the directory path of the URLs

void DestUrlArray::overrideDir(const string &dir_path)

{

  // get the dir name from the dir path by stripping off RAP_DATA_DIR

  string dirName;
  RapDataDir.stripPath(dir_path, dirName);

  // for each URL, replace the dir with the one passed in

  for (size_t ii = 0; ii < _destUrls.size(); ii++) {
    // cerr << "***** url before: " << _destUrls[ii] << endl;
    DsURL url(_destUrls[ii].c_str());
    url.setFile(dirName);
    _destUrls[ii] = url.getURLStr();
    // cerr << "***** url after: " << _destUrls[ii] << endl;
  } // ii

}

///////////////////////////////////////////////////////////////a
// Compute the final directory from the relative path
// given in the URL, if a dir was specified in the url

void DestUrlArray::setDirRelative(const string &rel_path)

{

  // for each URL, replace the dir with the one passed in

  for (size_t ii = 0; ii < _destUrls.size(); ii++) {
    // cerr << "***** url before: " << _destUrls[ii] << endl;
    DsURL url(_destUrls[ii].c_str());
    const string subDir = url.getFile();
    if (subDir.size() > 0) {
      string dir = subDir;
      dir += PATH_DELIM;
      dir += rel_path;
      url.setFile(dir);
      _destUrls[ii] = url.getURLStr();
    }
    // cerr << "***** url after: " << _destUrls[ii] << endl;
  } // ii

}

///////////
// print()
//

void DestUrlArray::print(ostream &out)

{

  cout << "---------------------------" << endl;
  for (size_t i = 0; i < _destUrls.size(); i++) {
    cout << _destUrls[i] << endl;
  }
  cout << "---------------------------" << endl;

}
  
/////////////////////////////////////
// add error string with int argument

void DestUrlArray::_addIntErr(const char *err_str, const int iarg)
{
  _errStr += err_str;
  char str[32];
  sprintf(str, "%d\n", iarg);
  _errStr += str;
}

////////////////////////////////////////
// add error string with string argument

void DestUrlArray::_addStrErr(const char *err_str, const string &sarg)
{
  _errStr += err_str;
  _errStr += ": ";
  _errStr += sarg;
  _errStr += "\n";
}


