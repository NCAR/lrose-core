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
////////////////////////////////////////////////////////////////////////
// $Id: EdgePkt.hh,v 1.3 2016/03/06 23:53:42 dixon Exp $
//
// Edge Packet base class
///////////////////////////////////////////////////////////////////////
#ifndef _EDGE_PKT_
#define _EDGE_PKT_

#include <cstdio>
#include <netinet/in.h>
#include <dataport/port_types.h>
using namespace std;

//
// Forward class declarations
//
class Status;
class Ray;

class EdgePkt {

public:

   enum compressionType { NONE, LZW, RUN_LEN, UNKNOWN };

   EdgePkt( int portNum, char* logFileName );
   ~EdgePkt();

   int    init();
   int    readUdp();
   int    printInfo();

   static const int MAX_PKT_SIZE;
   static const int HEADER_SIZE;
   
private:
   
   double           azimuth;
   double           elevation;
   int              checkSum;
   int              uncompressedLen;
   int              compressedLen;
   int              moment;
   compressionType  compType;

   Status          *statusInfo;
   Ray             *beamData;

   char            *buffer;
   int              bufLen;
   int              packetLen;

   int              port;
   int              udpFd;
   sockaddr_in      udpAddress;

   char*            logName;
   bool             logEnabled;

   int              readMsg( int packetLen );
   int              readHdr();
   void             printHdr( FILE* stream );
   int              uncompress();

};

#endif

   
