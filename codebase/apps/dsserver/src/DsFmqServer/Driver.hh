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
////////////////////////////////////////////////////////////////////////////////
//
//  Driver for DsFmqServer application class
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  March 1999
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _FMQ_DRIVER_INC_
#define _FMQ_DRIVER_INC_

#include <string>
#include <toolsa/Path.hh>
#include "Params.hh"
using namespace std;

//
// Forward class declarations
//
class DsFmqServer;


class Driver
{
public:
  
  Driver();
  ~Driver();
  
  //
  // Initialization
  //
  static const string version;
  int                 init( int argc, char **argv );
  const string&       getProgramName(){ return progName; }
  
  //
  // Execution
  //
  int                 run();
  
private:
  
  //
  // Initialization
  //
  string             progName;
  
  void               usage(ostream &out);
  int                processArgs( int argc, char **argv );
  
  //
  // Parameter processing
  //
  Params             params;
  char               *paramPath;
  tdrp_override_t    tdrpOverride;
  
  int                readParams(  int argc, char **argv );
  
  //
  // Server execution
  //
  DsFmqServer       *server;

};

#endif
