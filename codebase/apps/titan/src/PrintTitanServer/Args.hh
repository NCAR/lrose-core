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
// Args.hh: Command line object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2001
//
/////////////////////////////////////////////////////////////

#ifndef ARGS_H
#define ARGS_H

#include <iostream>
#include <string>
#include <titan/TitanServer.hh>
using namespace std;

class Args {
  
public:

  // constructor

  Args();

  // destructor

  virtual ~Args();

  // parse

  int parse(int argc, char **argv, string &prog_name);

  // public data

  string url;

  tserver_read_time_mode_t readTimeMode;
  tserver_track_set_t trackSet;
  time_t requestTime;
  time_t startTime;
  time_t endTime;
  int readTimeMargin;
  int requestComplexNum;

  bool debug;
  bool readLprops;
  bool readDbzHist;
  bool readRuns;
  bool readProjRuns;
  bool readCompressed;
  bool printAsXml;

protected:
  
private:

  string _progName;
  void _usage(ostream &out);
  
};

#endif

