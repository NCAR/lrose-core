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
// TaStr.cc
//
// Toolsa STL string utility routines
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2000
//
//////////////////////////////////////////////////////////

#include <cstdio>
#include <toolsa/TaStr.hh>
using namespace std;


///////////////////////////////////////////
// add integer value to target string,
// with optional following carriage return.

void TaStr::AddInt(string &target,
		   int iarg,
		   bool cr)
{
  AddInt(target, "", iarg, cr);
}

///////////////////////////////////////////////
// add labelled integer value to target string,
// with optional following carriage return.

void TaStr::AddInt(string &target,
		   string label,
		   int iarg,
		   bool cr)
{
  target += label;
  char str[32];
  sprintf(str, "%d", iarg);
  target += str;
  if (cr) {
    target += "\n";
  }
}

///////////////////////////////////////////
// add long integer value to target string,
// with optional following carriage return.

void TaStr::AddLong(string &target,
                    long larg,
                    bool cr)
{
  AddLong(target, "", larg, cr);
}

/////////////////////////////////////////////////////
// add labelled long integer value to target string,
// with optional following carriage return.

void TaStr::AddLong(string &target,
                    string label,
                    long larg,
                    bool cr)
{
  target += label;
  char str[32];
  sprintf(str, "%ld", larg);
  target += str;
  if (cr) {
    target += "\n";
  }
}

///////////////////////////////////////////
// add double value to target string,
// with optional following carriage return.
// Default format is %g.

void TaStr::AddDbl(string &target,
		   double darg,
		   string format,
		   bool cr)

{
  AddDbl(target, "", darg, format, cr);
}

///////////////////////////////////////////////
// add labelled double value to target string,
// with optional following carriage return.
// Default format is %g.

void TaStr::AddDbl(string &target,
		   string label,
		   double darg,
		   string format,
		   bool cr)

{
  target += label;
  char str[128];
  sprintf(str, format.c_str(), darg);
  target += str;
  if (cr) {
    target += "\n";
  }
}

////////////////////////////////////////
// add labelled string to target string
// with optional following carriage return.

void TaStr::AddStr(string &target,
		   string label,
		   string strarg,
		   bool cr)

{
  target += label;
  target += strarg;
  if (cr) {
    target += "\n";
  }
}

//////////////////////////////////////////////
// tokenize a string into a vector of strings
// given a string of spacer characters.
//
// If there are multiple 'spacer' characters sequentially,
// these will be treated as one spacer, and no tokens
// will be created within that set of spacer characters.
//
// For example, if the spacer is ',', and the string is:
//     ,,word1,word2,,,,word3,,word4,,
// the tokens will be
//     word1,word2,word3,word4

void TaStr::tokenize(const string &str,
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

//////////////////////////////////////////////
// tokenize a string into a vector of strings
// given a spacer character.
//
// If a pair of 'spacer' characters exist sequentially,
// an empty token will be created for that pair.
//
// If the string starts with a space, an empty token will
// be created at the start.
//
// If the string ends with a space, an empty token will
// be created at the end.
//
// For example, if the spacer is ',', and the string is:
//     ,word1,word2,,,word3,,word4,,
// the tokens will be
//     empty,word1,word2,empty,empty,word3,empty,word4,empty,empty

void TaStr::tokenizeAllowEmpty(const string &str,
                               char spacer,
                               vector<string> &toks)
  
{
    
  toks.clear();

  // check start
  
  size_t start = 0;
  if (str[0] == spacer) {
    // start with empty token
    string tok;
    toks.push_back(tok);
    start = 1;
  }

  while (start <= str.size()) {
    string tok;
    size_t end = str.find_first_of(spacer, start);
    if (end == string::npos) {
      string tok(str.substr(start));
      toks.push_back(tok);
      break;
    } else {
      int len = (int) end - (int) start;
      string tok(str.substr(start, len));
      toks.push_back(tok);
    }
    start = end + 1;
  }

}
