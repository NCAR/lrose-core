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
// SpdbXml2Table.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2011
//
///////////////////////////////////////////////////////////////
//
// SpdbXml2Table reads XML entries from an SPDB data base,
// and based on the specified parameters in the file,
// it reformats these into a text table.
//
////////////////////////////////////////////////////////////////

#ifndef SpdbXml2Table_HH
#define SpdbXml2Table_HH

#include <string>
#include <toolsa/DateTime.hh>
#include <Spdb/DsSpdb.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

////////////////////////
// This class

class SpdbXml2Table {
  
public:

  // constructor

  SpdbXml2Table(int argc, char **argv);

  // destructor
  
  ~SpdbXml2Table();

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

  DateTime _startTime;
  DateTime _endTime;

  int _lineCount;

  // functions

  void _printComments(FILE *out);
  void _printLine(FILE *out, const Spdb::chunk_t &chunk);

};

#endif
