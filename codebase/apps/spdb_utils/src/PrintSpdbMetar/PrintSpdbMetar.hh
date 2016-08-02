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
// PrintSpdbMetar.h
//
// PrintSpdbMetar object
//
// Modified from SpdbQuery by Mike Dixon
// RAP, NCAR P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1999
//
///////////////////////////////////////////////////////////////

#ifndef PrintSpdbMetar_H
#define PrintSpdbMetar_H

#include "Args.hh"
#include <cstdio>
#include <rapformats/WxObs.hh>
using namespace std;

class PrintSpdbMetar {
  
public:

  // constructor

  PrintSpdbMetar (int argc, char **argv);

  // destructor
  
  ~PrintSpdbMetar();

  // run 

  int Run();

  // data members

  bool OK;

protected:
  
private:

  string _progName;
  Args _args;

  int _doDataType(si32 dataType, bool &headerPrinted);
  void _printWsddmHeader();
  void _printAoawsHeader();
  void _printAoawsWideHeader();
  void _printWsddmMetar(WxObs &metar,
			bool add_carriage_return = true);
  void _printAoawsMetar(WxObs &metar,
			bool add_carriage_return = true);
  double _nearest(double target, double delta);
  string _truncateWxStr(const string &wxStr, int nToks);
  void _tokenize(const string &str,
                 const string &spacer,
                 vector<string> &toks);

};

#endif



