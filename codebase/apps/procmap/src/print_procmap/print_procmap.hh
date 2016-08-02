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
// print_procmap.hh
//
// print_procmap object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2000
//
///////////////////////////////////////////////////////////////

#ifndef print_procmap_H
#define print_procmap_H

#include "Args.hh"
#include <cstdio>
#include <string>
#include <map>
#include <toolsa/pmu.h>
using namespace std;

class print_procmap {
  
public:

  // constructor

  print_procmap (int argc, char **argv);

  // destructor
  
  ~print_procmap();

  // run 

  int Run();

  // data members

  bool OK;

protected:

private:

  string _progName;
  Args _args;

  // used in _clean4XML
  map< string, string > _tokenMap;

  char _basicFormat[1024];

  int _queryMapper();
  
  void _printTable(FILE *out,
                   const string &procmapHostName,
                   const string &relayHostList,
                   time_t procmapHostTime,
                   time_t procmapUpTime,
                   bool isRelay,
                   int nProcs,
                   const PROCMAP_info_t *procs);

  void _printPlain(FILE *out,
                   const string &procmapHostName,
                   time_t procmapHostTime,
                   time_t procmapUpTime,
                   int nProcs,
                   const PROCMAP_info_t *procs);

  void _printXml(FILE *out,
                 const string &procmapHostName,
                 time_t procmapHostTime,
                 time_t procmapUpTime,
                 int nProcs,
                 const PROCMAP_info_t *procs);

  void _printData(FILE *out,
                  const PROCMAP_info_t *dinfo, time_t hostTime);

  void _printHeader(FILE *out);

  string _pidString(int pid);

  string _secsStr(time_t refTime, time_t hostTime);

  string _stripHost(const char *hostname);

  static int _sortInfo(const void *p1, const void *p2);

  string _tdiffStr(int tdiff);

  void _computeFormat(int nProcs,
                      const PROCMAP_info_t *procs);
  
  // Based upon http://www.w3schools.com/xml/xml_cdata.asp
  // it is good practice to not use the characters <>&'" 
  // in XML data.  This function replaces them as follows:
  //    '<' --> '{'
  //    '>' --> '}'
  //	'&' --> 'and'
  //    ''' --> '`'
  //    '"' --> '``'
  string _clean4XML(string s);
};

#endif
