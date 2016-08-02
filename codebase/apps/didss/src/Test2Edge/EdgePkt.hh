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
///////////////////////////////////////////////
// $Id: EdgePkt.hh,v 1.10 2016/03/06 23:53:41 dixon Exp $
//
// Edge Packet base class
//////////////////////////////////////////////
#ifndef _EDGE_PKT_
#define _EDGE_PKT_

#include <cstdio>
#include <netinet/in.h>
#include <dataport/port_types.h>
using namespace std;

class EdgePkt {

public:

   enum compressionType { NONE, LZW, RUN_LEN };

   EdgePkt( int portNum, char* logName );
   virtual ~EdgePkt();

   virtual void simulate( time_t now ) = 0;
   virtual int  broadcast() = 0;

   static const int MAX_PKT_SIZE;
   static const int HEADER_SIZE;
   static const int STATUS_MOMENT;
   
protected:
   
   unsigned short   azimuth;
   unsigned short   elevation;
   int              checkSum;
   int              uncompressedLen;
   int              compressedLen;
   ui16             moment;
   compressionType  compType;

   char            *buffer;
   int              bufLen;

   int              port;
   int              udpFd;
   sockaddr_in      localAddress;
   sockaddr_in      outputAddress;

   char            *logFileName;

   int              loadHdr();

   int              initUdp( char *broadcastAddress );
   int              sendUdp();

};

#endif

   
