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
// Polygon2Xml.hh
//
// Sue Dettling, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2006
//
///////////////////////////////////////////////////////////////

#ifndef POLYGON2XML_HH
#define POLYGON2XML_HH

#include <tdrp/tdrp.h>
#include <string>
#include <Spdb/DsSpdb.hh>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/DateTime.hh>
#include "Args.hh"
#include "Params.hh"

using namespace std;

class Args;
class Params;

class Polygon2Xml {
  
public:

  // constructor

  Polygon2Xml (int argc, char **argv);

  // destructor
  
  ~Polygon2Xml();

  // run 

  int Run();

  // data members

  int isOK;


protected:
  
private:

  int _convert(time_t start_time, time_t end_time);
  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

};

#endif

