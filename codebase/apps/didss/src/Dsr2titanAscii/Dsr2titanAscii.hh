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
// Dsr2titanAscii.hh
//
// Dsr2titanAscii object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
///////////////////////////////////////////////////////////////
//
// Dsr2titanAscii prints out info from a DsRadar FMQ
//
///////////////////////////////////////////////////////////////////////

#ifndef Dsr2titanAscii_HH
#define Dsr2titanAscii_HH

#include <string>
#include <rapformats/DsRadarMsg.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

class Socket;

////////////////////////
// This class

class Dsr2titanAscii {
  
public:

  // constructor

  Dsr2titanAscii (int argc, char **argv);

  // destructor
  
  ~Dsr2titanAscii();

  // run 

  int Run();
  

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  
  int _run();

  void _processMessage(int contents, const DsRadarMsg &radarMsg);

  FILE *_ofp;
  void _openOutputFile(time_t msgTime);
  void _closeOutputFile();


};

#endif

