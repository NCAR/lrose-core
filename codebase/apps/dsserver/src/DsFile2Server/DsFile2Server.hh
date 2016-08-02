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
// DsFile2Server.hh
//
// DsFile2Server object
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2004
//
///////////////////////////////////////////////////////////////

#ifndef DsFile2Server_H
#define DsFile2Server_H

#include <string>

#include "Params.hh"
#include "IOCounter.hh"

class DsFile2Server {

public:
  
  // constructor

   DsFile2Server(int argc, char *argv[]);


  // Main method - run.

  int Run(int argc, char *argv[]);

  // This gets called once per second from  the DsInputPath Object.
  void Heartbeat(const char *label);

  // Destructor.

  ~DsFile2Server();

  // Data.

  int isOK;
  IOCounter IOC;

protected:
private:

  Params *_params;
  void _dealWithFile( string inFilename );
  void _dealWithFilecopy( unsigned char *buffer, int file_size);
  void _dealWithSpdb( unsigned char *buffer, int file_size);

};


#endif
