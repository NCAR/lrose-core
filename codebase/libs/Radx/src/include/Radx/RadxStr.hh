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
// RadxStr.hh
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


class RadxStr

{

public:

  ///////////////////////////////////////////
  // add integer value to target string,
  // with optional following carriage return.
  
  static void addInt(string &target,
		     int iarg,
		     bool cr = false);
  
  ///////////////////////////////////////////////
  // add labelled integer value to target string,
  // with optional following carriage return.
  
  static void addInt(string &target,
		     string label,
		     const int iarg,
		     bool cr = true);

  ///////////////////////////////////////////
  // add double value to target string,
  // with optional following carriage return.
  // Default format is %g.
  
  static void addDbl(string &target,
		     double darg,
		     string format = "%g",
		     bool cr = false);

  ///////////////////////////////////////////////
  // add labelled double value to target string,
  // with optional following carriage return.
  // Default format is %g.

  static void addDbl(string &target,
		     string label,
		     double darg,
		     string format = "%g",
		     bool cr = true);
  
  ////////////////////////////////////////
  // add labelled string to target string
  // with optional following carriage return.
  
  static void addStr(string &target,
		     string label,
		     string strarg = "",
		     bool cr = true);

  //////////////////////////////////////////////
  // tokenize a string into a vector of strings
  // given a spacer
  
  static void tokenize(const string &str,
		       const string &spacer,
		       vector<string> &toks);
  
};
