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
//  Driver.hh
//  Driver for TsFmq2Tcp application class
//
//  Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
//  Feb 2009
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
class TsFmq2Tcp;


class Driver
{
public:
  
  Driver();
  ~Driver();
  
  // Initialization

  int init(int argc, char **argv);
  
  // Execution

  int run();
  
  const string &getProgName() { return _progName; }

private:
  
  string _progName;
  Params _params;
  char *_paramPath;
  tdrp_override_t _tdrpOverride;
  TsFmq2Tcp *_server;
  
  void _usage(ostream &out);
  int _processArgs(int argc, char **argv);
  int _readParams(int argc, char **argv);


};

#endif
