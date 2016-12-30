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
// InputUdp.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2016
//
///////////////////////////////////////////////////////////////

#ifndef InputUdp_hh
#define InputUdp_hh

#include "Params.hh"
#include <string>

using namespace std;

////////////////////////
// This class

class InputUdp {
  
public:
  
  // constructor - initializes for given size
  
  InputUdp(const Params &params);
  
  // destructor
  
  ~InputUdp();

  // open and close

  int openUdp();
  void closeUdp();

  // read data from socket, puts into buffer
  // Returns number of bytes read.
  // Use getBuf() for access to buffer.
  
  int readPacket();

  // get the data

  const char *getBuf() const { return _buf; }
  int getLen() const { return _len; }

protected:
  
private:

  const Params &_params;
  int _udpFd;
  static const int maxUdpBytes = 1524;
  int _len;
  char _buf[maxUdpBytes];

};

#endif

