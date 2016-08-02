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


#include <toolsa/URL.hh>
#include <string>
#include <cstdio>
#include <cstdlib>
using namespace std;

const string URL::Ampersand = "&";
const string URL::Colon = ":";
const string URL::Equals = "=";
const string URL::QuestionMark = "?";
const string URL::Slash = "/";
const string URL::SlashSlash = "//";
const string URL::ColonSlashSlash = "://";

///////////////
// constructor

URL::URL()
{
  _init();
}

URL::URL(const string &urlStr)
{

  _init();
  _urlStr = urlStr;
  
}

/////////////////////
// virtual destructor

URL::~URL()
{

}

/////////////////////
// initialize

void URL::_init()
{

  _urlStr.clear();
  _protocol.clear();
  _host.clear();
  _port = -1;
  _file.clear();
  _args.clear();
  _argList.clear();
  _argPairs.clear();
  _errString.clear();
  _isValid = false;

}

//////////////////////////////////////////////
// look for name=val pairs in the args

void URL::_loadArgPairs(const string &args)

{

  _args = args;
  _argPairs.clear();
  size_t startOfNext = 0;
  
  // loop through looking for args

  while (startOfNext != string::npos) {
    
    size_t endOfArg = _args.find(Ampersand, startOfNext);
    size_t argLen = endOfArg - startOfNext;
    string arg = _args.substr(startOfNext, argLen);
    _argList.push_back(arg);

    // Is this a name+value pair?

    size_t pairDelim = arg.find(Equals, 0);

    if (pairDelim != string::npos) {
      pair<string, string> pr;
      pr.first = arg.substr(0, pairDelim);
      pr.second = arg.substr(pairDelim + 1);
      _argPairs.push_back(pr);
    }

    if (endOfArg == string::npos) {
      break;
    }

    startOfNext = endOfArg + 1;

  } // while

}

