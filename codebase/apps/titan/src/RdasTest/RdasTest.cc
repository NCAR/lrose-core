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
///////////////////////////////////////////////////////////////
// RdasTest.cc
//
// RdasTest object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
///////////////////////////////////////////////////////////////

#include "RdasTest.hh"
#include <dataport/bigend.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/Socket.hh>
#include <cstdio>
#include <unistd.h>
using namespace std;

// Constructor

RdasTest::RdasTest(int argc, char **argv)
  
{

  OK = true;

  // set programe name

  _progName = "RdasTest";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = false;
    return;
  }
  
  return;

}

// destructor

RdasTest::~RdasTest()

{


}

//////////////////////////////////////////////////
// Run

int RdasTest::Run()

{

  int iret = 0;
  while (true) {
    if (_contactRdas()) {
      iret = -1;
    }
  }

  return iret;
  
}

//////////////////////////////////////////////////
// contact RDAS
//
// Returns 0 on success, -1 on failure

int RdasTest::_contactRdas()
  
{

  // open socket to Rdas

  Socket ss;
  if (ss.open(_args.host.c_str(), _args.port, 1000)) {
    cerr << "Cannot contact RDAS, host: "
	 << _args.host << ", port: " << _args.port << endl;
    umsleep(1000);
    return -1;
  }

  // write commands to RDAS

  while (true) {

    ui32 buf[10];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x5B5B5B5B;
    buf[1] = 999;
    buf[2] = 28;
    
    BE_from_array_32(buf, sizeof(buf));

    if (ss.writeBuffer(buf, sizeof(buf), 1000)) {
      cerr << "Cannot write message to RDAS" << endl;
      ss.close();
      return -1;
    }

  } // while

  return 0;
  
}

