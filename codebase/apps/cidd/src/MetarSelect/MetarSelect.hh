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
// MetarSelect.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2000
//
///////////////////////////////////////////////////////////////
//
// MetarSelect reads user key-clicks from the display, and
// prints the closest metars to stdout
//
///////////////////////////////////////////////////////////////

#ifndef MetarSelect_H
#define MetarSelect_H

#include <rapformats/coord_export.h>
#include <rapformats/WxObs.hh>
#include <Spdb/DsSpdb.hh>
#include <Fmq/RemoteUIQueue.hh>
#include <map>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class MetarSelect {
  
public:

  typedef multimap<double, int, less<double> > distmap_t;
  typedef pair<const double, int > distpair_t;

  // constructor

  MetarSelect (int argc, char **argv);

  // destructor
  
  ~MetarSelect();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  coord_export_t *_coordShmem;
  DsSpdb _spdb;

  void _runShmemMode();
  void _runFmqMode();
  void _doPrint(time_t data_time, double click_lat, double click_lon);
  void _printWsddmHeader();
  void _printWsddmHeaderEng();
  void _printWsddmHeaderEngHtml();
  void _printAoawsHeader();
  void _printAoawsHeader_2();
  void _printWsddmMetar(WxObs &metar);
  void _printWsddmMetarEng(WxObs &metar);
  void _printWsddmMetarEngHtml(WxObs &metar);
  void _printAoawsMetar(WxObs &metar);
  double _nearest(double target, double delta);
  string _truncateWxStr(const string &wxStr, int nToks);
  void _tokenize(const string &str,
                 const string &spacer,
                 vector<string> &toks);
 
  RemoteUIQueue *_remote_ui;   // Remote User Interface/Command class

  char _output_filename[4096]; 

  FILE *_ofile; 
  
};

#endif
