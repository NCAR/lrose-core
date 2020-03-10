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
// TaStr.hh
//
// Toolsa STL string utility routines
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2000
//
//////////////////////////////////////////////////////////

#include <string>
#include <vector>
using namespace std;


class TaStr

{

public:

  ///////////////////////////////////////////
  // add integer value to target string,
  // with optional following carriage return.
  
  static void AddInt(string &target,
		     int iarg,
		     bool cr = false);
  
  ///////////////////////////////////////////////
  // add labelled integer value to target string,
  // with optional following carriage return.
  
  static void AddInt(string &target,
		     string label,
		     const int iarg,
		     bool cr = true);

  ///////////////////////////////////////////
  // add long integer value to target string,
  // with optional following carriage return.
  
  static void AddLong(string &target,
                      long larg,
                      bool cr = false);
  
  ///////////////////////////////////////////////
  // add labelled long integer value to target string,
  // with optional following carriage return.
  
  static void AddLong(string &target,
                      string label,
                      const long larg,
                      bool cr = true);

  ///////////////////////////////////////////
  // add double value to target string,
  // with optional following carriage return.
  // Default format is %g.
  
  static void AddDbl(string &target,
		     double darg,
		     string format = "%g",
		     bool cr = false);

  ///////////////////////////////////////////////
  // add labelled double value to target string,
  // with optional following carriage return.
  // Default format is %g.

  static void AddDbl(string &target,
		     string label,
		     double darg,
		     string format = "%g",
		     bool cr = true);
  
  ////////////////////////////////////////
  // add labelled string to target string
  // with optional following carriage return.
  
  static void AddStr(string &target,
		     string label,
		     string strarg = "",
		     bool cr = true);

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
  
  static void tokenize(const string &str,
		       const string &spacer,
		       vector<string> &toks);
  
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
  
  static void tokenizeAllowEmpty(const string &str,
                                 char spacer,
                                 vector<string> &toks);
  
};
